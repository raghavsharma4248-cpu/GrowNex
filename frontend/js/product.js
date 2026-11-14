/* product.js - product details plus the enquiry form.
 * Only a logged-in customer sees the form; everyone else sees what to do next. */
(async function () {
  const { api, esc, param, price, stock, icon, initials, ready, setBusy, showFormError, clearFormError } = GrowNex;
  const main = document.getElementById("product-main");
  const id = param("id");

  if (!/^\d+$/.test(id)) {
    main.innerHTML = '<div class="empty-state"><h3>No product selected</h3><p>Choose a product from the list.</p><a class="btn btn-primary" href="explore.html">Browse products</a></div>';
    return;
  }

  let product, business;
  try {
    ({ product, business } = await api("/api/products/" + id));
  } catch (error) {
    main.innerHTML = '<div class="empty-state"><h3>Product not found</h3><p>' + esc(error.message) + '</p><a class="btn btn-primary" href="explore.html">Browse products</a></div>';
    return;
  }

  document.title = product.name + " | GrowNex";
  document.getElementById("crumbs").innerHTML =
    '<li><a href="explore.html">Products</a></li>' +
    '<li><a href="business.html?id=' + product.businessId + '">' + esc(product.businessName) + "</a></li>" +
    '<li><span aria-current="page">' + esc(product.name) + "</span></li>";

  main.innerHTML =
    '<span class="badge">' + esc(product.category) + "</span>" +
    '<h1 style="margin-top: var(--space-3)">' + esc(product.name) + "</h1>" +
    '<p class="product-price price">' + price(product.price) + "</p>" +
    stock(product.quantity) +
    '<dl class="product-facts">' +
    "<dt>Sold by</dt><dd><a href=\"business.html?id=" + product.businessId + '">' + esc(product.businessName) + "</a></dd>" +
    "<dt>Category</dt><dd>" + esc(product.category) + "</dd>" +
    "<dt>Quantity listed</dt><dd>" + Number(product.quantity).toLocaleString("en-IN") + "</dd>" +
    "<dt>Last updated</dt><dd>" + esc(product.updatedAt) + "</dd>" +
    "</dl>" +
    "<h2 style=\"font-size: var(--text-lg)\">About this product</h2>" +
    '<p class="product-description">' + (product.description ? esc(product.description) : '<span class="muted">The seller has not added a description.</span>') + "</p>" +
    '<p class="panel-note">Stock is the quantity the seller has listed. GrowNex does not take orders, so confirm availability with the business.</p>';

  if (business) {
    document.getElementById("seller-panel").innerHTML =
      '<div class="seller-box"><span class="monogram" aria-hidden="true">' + esc(initials(business.businessName)) + "</span>" +
      '<div><a href="business.html?id=' + business.id + '">' + esc(business.businessName) + "</a>" +
      '<div class="meta-line"><span>' + icon.pin + esc(business.city) + "</span>" +
      "<span>" + icon.phone + esc(business.phone) + "</span></div></div></div>";
  }
  document.getElementById("side").hidden = false;

  // ---- enquiry area depends on who is logged in ----
  const area = document.getElementById("enquiry-area");
  const user = await ready();
  const here = encodeURIComponent("product.html?id=" + id);

  if (!user) {
    area.innerHTML =
      "<p>Log in with a customer account to ask " + esc(product.businessName) + " about this product.</p>" +
      '<div style="display:flex; gap: var(--space-2); flex-wrap: wrap">' +
      '<a class="btn btn-primary" href="login.html?next=' + here + '">Log in to ask</a>' +
      '<a class="btn btn-secondary" href="register.html?role=customer&next=' + here + '">Create account</a></div>';
    return;
  }
  if (user.role !== "customer") {
    area.innerHTML = '<p class="panel-note">You are logged in as a business owner. Enquiries are sent from customer accounts, so the form is hidden here.</p>';
    return;
  }

  area.innerHTML =
    '<form class="form" id="enquiry-form" novalidate>' +
    '<div class="form-error" role="alert" tabindex="-1"></div>' +
    '<input type="hidden" name="productId" value="' + product.id + '">' +
    '<div class="field"><label for="message">Your question <span class="required" aria-hidden="true">*</span></label>' +
    '<textarea class="textarea" id="message" name="message" required minlength="10" maxlength="500" aria-describedby="message-hint"></textarea>' +
    '<p class="hint" id="message-hint">For example: size, colour, delivery area or bulk price. 10 to 500 characters.</p></div>' +
    '<p class="panel-note" style="margin:0">' + esc(product.businessName) + " will see your name (" + esc(user.name) + ") and email (" + esc(user.email) + ") so they can reply.</p>" +
    '<button class="btn btn-primary btn-block" type="submit">Send enquiry</button>' +
    "</form>";

  const form = document.getElementById("enquiry-form");
  const message = document.getElementById("message");
  form.addEventListener("submit", async (event) => {
    event.preventDefault();
    clearFormError(form);
    message.removeAttribute("aria-invalid");
    if (message.value.trim().length < 10) {
      message.setAttribute("aria-invalid", "true");
      showFormError(form, "Write at least 10 characters so the business understands your question.");
      return;
    }
    const button = form.querySelector('button[type="submit"]');
    setBusy(button, true, "Sending");
    try {
      await api("/api/enquiries", { method: "POST", data: new FormData(form) });
      area.innerHTML =
        '<div class="success-box" role="status"><strong>Enquiry sent</strong>' +
        esc(product.businessName) + " will reply to " + esc(user.email) + ".</div>" +
        '<p style="margin-top: var(--space-4)"><a href="customer-dashboard.html">See all your enquiries</a></p>';
    } catch (error) {
      setBusy(button, false);
      showFormError(form, error.message);
    }
  });
})();
