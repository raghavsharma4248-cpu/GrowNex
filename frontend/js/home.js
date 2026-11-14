/* home.js - fills the "Recently listed" ledger, the category links and the
 * business grid on the home page with live data from the C++ server. */
(async function () {
  const { api, esc, price, stock, businessCard } = GrowNex;

  // 1. Latest five products in the ledger
  const list = document.getElementById("ledger-list");
  const count = document.getElementById("ledger-count");
  try {
    const data = await api("/api/products?limit=5");
    if (data.products.length === 0) {
      list.innerHTML = '<li class="ledger-empty">No products yet. Business owners can add the first one from their dashboard.</li>';
      count.textContent = "";
    } else {
      list.innerHTML = data.products
        .map(
          (p) =>
            '<li><a class="ledger-row" href="product.html?id=' + p.id + '">' +
            '<span class="item">' + esc(p.name) + "</span>" +
            '<span class="price">' + price(p.price) + "</span>" +
            '<span class="seller">' + esc(p.businessName) + "</span>" +
            stock(p.quantity) +
            "</a></li>"
        )
        .join("");
      count.textContent = "Updated live";
    }
  } catch (error) {
    list.innerHTML = '<li class="ledger-empty">' + esc(error.message) + "</li>";
    count.textContent = "";
  }

  // 2. Category links
  try {
    const data = await api("/api/categories");
    document.getElementById("category-strip").innerHTML = data.categories
      .map((c) => '<a class="chip" href="explore.html?category=' + encodeURIComponent(c) + '">' + esc(c) + "</a>")
      .join("");
  } catch (error) {
    /* the strip simply stays empty */
  }

  // 3. Up to six businesses
  const grid = document.getElementById("business-grid");
  try {
    const data = await api("/api/businesses");
    grid.innerHTML = data.businesses.length
      ? data.businesses.slice(0, 6).map(businessCard).join("")
      : '<div class="empty-state"><h3>No businesses yet</h3><p>Be the first to set up a storefront on GrowNex.</p><a class="btn btn-primary" href="register.html?role=business">List your business</a></div>';
  } catch (error) {
    grid.innerHTML = '<p class="loading">' + esc(error.message) + "</p>";
  }
})();
