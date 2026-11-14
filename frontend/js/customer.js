/* customer.js - customer dashboard: the list of enquiries this customer sent. */
(async function () {
  const { api, esc, requireRole } = GrowNex;

  const user = await requireRole("customer");
  if (!user) return;

  document.getElementById("who-name").textContent = user.name;
  document.getElementById("who-detail").textContent = user.city;
  document.getElementById("greeting").textContent = "Hello, " + user.name.split(" ")[0];
  document.getElementById("reply-email").textContent = user.email;

  const box = document.getElementById("enquiry-list");
  try {
    const { enquiries } = await api("/api/customer/enquiries");
    document.getElementById("count-enquiries").textContent = enquiries.length;
    if (enquiries.length === 0) {
      box.innerHTML =
        '<div class="empty-state"><h3>You have not sent any enquiries yet</h3>' +
        "<p>Open any product and use the Ask the business form. Your questions will be listed here.</p>" +
        '<a class="btn btn-primary" href="explore.html">Browse products</a></div>';
      return;
    }
    box.innerHTML =
      '<ul class="enquiry-list">' +
      enquiries
        .map(
          (e) =>
            '<li class="enquiry">' +
            '<div class="enquiry-top"><h3><a href="product.html?id=' + e.productId + '">' + esc(e.productName) + "</a></h3>" +
            '<time datetime="' + esc(e.createdAt.replace(" ", "T")) + '">' + esc(e.createdAt) + "</time></div>" +
            "<blockquote>" + esc(e.message) + "</blockquote>" +
            '<div class="enquiry-from"><span>Sent to <a href="business.html?id=' + e.businessId + '"><strong>' + esc(e.businessName) + "</strong></a></span></div>" +
            "</li>"
        )
        .join("") +
      "</ul>";
  } catch (error) {
    box.innerHTML = '<div class="empty-state"><h3>Could not load enquiries</h3><p>' + esc(error.message) + "</p></div>";
  }
})();
