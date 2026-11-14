/* business.js - shows one business profile and all of its products.
 * The business id comes from the address, for example business.html?id=2 */
(async function () {
  const { api, esc, param, icon, initials, productCard } = GrowNex;
  const info = document.getElementById("business-info");
  const grid = document.getElementById("product-grid");
  const id = param("id");

  if (!/^\d+$/.test(id)) {
    info.innerHTML = '<div class="empty-state"><h3>No business selected</h3><p>Choose a business from the list.</p><a class="btn btn-primary" href="explore.html?view=businesses">Browse businesses</a></div>';
    return;
  }

  try {
    const { business: b, products } = await api("/api/businesses/" + id);
    document.title = b.businessName + " | GrowNex";
    document.getElementById("crumb-name").textContent = b.businessName;

    info.innerHTML =
      '<div class="business-hero">' +
      '<span class="monogram monogram-lg" aria-hidden="true">' + esc(initials(b.businessName)) + "</span>" +
      "<div>" +
      "<h1>" + esc(b.businessName) + "</h1>" +
      '<div class="meta-line">' +
      "<span>" + icon.tag + esc(b.category) + "</span>" +
      "<span>" + icon.pin + esc(b.city) + "</span>" +
      "<span>" + icon.phone + '<a href="tel:' + esc(b.phone.replace(/[^\d+]/g, "")) + '">' + esc(b.phone) + "</a></span>" +
      "</div>" +
      (b.description ? '<p class="about">' + esc(b.description) + "</p>" : "") +
      '<p class="muted" style="margin:0">Run by ' + esc(b.ownerName) + ", on GrowNex since " + esc(b.memberSince) + "</p>" +
      "</div></div>";

    document.getElementById("products-title").textContent =
      products.length === 1 ? "1 product" : products.length + " products";
    grid.innerHTML = products.length
      ? products.map(productCard).join("")
      : '<div class="empty-state" style="grid-column:1/-1"><h3>No products listed yet</h3><p>This business has not added any products. Check back later.</p></div>';
  } catch (error) {
    info.innerHTML = '<div class="empty-state"><h3>Business not found</h3><p>' + esc(error.message) + '</p><a class="btn btn-primary" href="explore.html?view=businesses">Browse businesses</a></div>';
    document.getElementById("products-title").textContent = "";
  }
})();
