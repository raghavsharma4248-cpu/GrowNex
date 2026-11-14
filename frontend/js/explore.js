/* explore.js - product and business search.
 * The same page shows either products (default) or businesses (?view=businesses).
 * Typing in the search box or changing the category asks the C++ server again. */
(async function () {
  const { api, esc, param, productCard, businessCard, fillCategorySelect } = GrowNex;

  const view = param("view") === "businesses" ? "businesses" : "products";
  const form = document.getElementById("filter-form");
  const q = document.getElementById("q");
  const category = document.getElementById("category");
  const results = document.getElementById("results");
  const count = document.getElementById("result-count");

  // Set up the page for the chosen view
  document.body.dataset.nav = view;
  document.querySelectorAll(".nav-links [data-nav]").forEach((link) => {
    if (link.dataset.nav === view) link.setAttribute("aria-current", "page");
    else link.removeAttribute("aria-current");
  });
  document.getElementById(view === "businesses" ? "view-businesses" : "view-products").setAttribute("aria-current", "page");
  if (view === "businesses") {
    document.title = "Businesses | GrowNex";
    document.getElementById("explore-title").textContent = "Explore businesses";
    document.getElementById("explore-lead").textContent =
      "Small businesses and startups with a storefront on GrowNex. Open one to see its products and contact details.";
    document.getElementById("q-label").textContent = "Search by name, city or description";
  }

  q.value = param("q");
  category.dataset.value = param("category");
  await fillCategorySelect(category, "All categories");

  async function search() {
    const query = new URLSearchParams();
    if (q.value.trim()) query.set("q", q.value.trim());
    if (category.value) query.set("category", category.value);

    // Keep the address bar in sync so the search can be bookmarked or shared
    const pageQuery = new URLSearchParams(query);
    if (view === "businesses") pageQuery.set("view", "businesses");
    history.replaceState(null, "", "explore.html" + (pageQuery.toString() ? "?" + pageQuery : ""));

    try {
      const data = await api("/api/" + view + "?" + query);
      const items = data[view];
      const noun = view === "products" ? (items.length === 1 ? "product" : "products") : items.length === 1 ? "business" : "businesses";
      count.textContent = items.length + " " + noun + (q.value.trim() ? ' matching "' + q.value.trim() + '"' : "");
      if (items.length === 0) {
        results.innerHTML =
          '<div class="empty-state" style="grid-column: 1 / -1"><h3>Nothing found</h3>' +
          "<p>Try a shorter search word, or choose All categories.</p>" +
          '<button class="btn btn-secondary" type="button" id="clear-search">Clear search</button></div>';
        document.getElementById("clear-search").addEventListener("click", () => {
          q.value = "";
          category.value = "";
          search();
        });
        return;
      }
      results.innerHTML = items.map(view === "products" ? productCard : businessCard).join("");
    } catch (error) {
      count.textContent = "";
      results.innerHTML = '<div class="empty-state" style="grid-column: 1 / -1"><h3>Could not load results</h3><p>' + esc(error.message) + "</p></div>";
    }
  }

  // Wait until the person stops typing for a moment before searching
  let timer;
  q.addEventListener("input", () => {
    clearTimeout(timer);
    timer = setTimeout(search, 250);
  });
  category.addEventListener("change", search);
  form.addEventListener("submit", (event) => {
    event.preventDefault();
    search();
  });

  search();
})();
