/* owner.js - business owner dashboard.
 *   Products tab:   list, add, edit and delete products
 *   Enquiries tab:  read customer enquiries (view only)
 *   Profile tab:    edit the public business profile
 */
(async function () {
  const { api, esc, price, stock, requireRole, fillCategorySelect, setBusy, showFormError, clearFormError, toast } = GrowNex;

  const user = await requireRole("business");
  if (!user) return;

  let products = [];
  let enquiries = [];

  // ---------- header details ----------
  function showOwner(u) {
    document.getElementById("who-business").textContent = u.businessName;
    document.getElementById("who-owner").textContent = u.name + ", " + u.city;
    document.getElementById("public-link").href = "business.html?id=" + u.id;
  }
  showOwner(user);

  // ---------- tabs (driven by the # part of the address) ----------
  function showTab() {
    const tab = ["products", "enquiries", "profile"].includes(location.hash.slice(1)) ? location.hash.slice(1) : "products";
    document.querySelectorAll("[data-panel]").forEach((panel) => (panel.hidden = panel.id !== "tab-" + tab));
    document.querySelectorAll("[data-tab]").forEach((link) => {
      if (link.dataset.tab === tab) link.setAttribute("aria-current", "true");
      else link.removeAttribute("aria-current");
    });
  }
  window.addEventListener("hashchange", showTab);
  showTab();

  // ---------- categories for both selects ----------
  await Promise.all([
    fillCategorySelect(document.getElementById("f-category"), "Choose a category"),
    fillCategorySelect(document.getElementById("p-category"), "Choose a category"),
  ]);

  // ---------- products ----------
  function updateStats() {
    document.getElementById("stat-products").textContent = products.length;
    document.getElementById("stat-out").textContent = products.filter((p) => p.quantity <= 0).length;
    document.getElementById("stat-enquiries").textContent = enquiries.length;
    document.getElementById("count-products").textContent = products.length;
    document.getElementById("count-enquiries").textContent = enquiries.length;
  }

  function renderProducts() {
    const box = document.getElementById("product-table");
    updateStats();
    if (products.length === 0) {
      box.innerHTML =
        '<div class="empty-state"><h3>Add your first product</h3>' +
        "<p>Products appear on your public page and in search as soon as you save them.</p>" +
        '<button class="btn btn-primary" type="button" data-action="add">Add product</button></div>';
      return;
    }
    box.innerHTML =
      '<div class="table-wrap"><table class="table">' +
      "<caption class=\"visually-hidden\">Your products</caption>" +
      '<thead><tr><th scope="col">Product</th><th scope="col" class="num">Price</th><th scope="col">Stock</th><th scope="col">Last updated</th><th scope="col" class="actions"><span class="visually-hidden">Actions</span></th></tr></thead><tbody>' +
      products
        .map(
          (p) =>
            "<tr>" +
            '<td class="name"><a href="product.html?id=' + p.id + '">' + esc(p.name) + '</a><span class="sub">' + esc(p.category) + "</span></td>" +
            '<td class="num price" data-label="Price">' + price(p.price) + "</td>" +
            "<td>" + stock(p.quantity) + "</td>" +
            '<td class="muted" data-label="Updated">' + esc(p.updatedAt) + "</td>" +
            '<td class="actions">' +
            '<button class="btn btn-quiet btn-sm" type="button" data-action="edit" data-id="' + p.id + '">Edit<span class="visually-hidden"> ' + esc(p.name) + "</span></button> " +
            '<button class="btn btn-quiet btn-sm" type="button" data-action="delete" data-id="' + p.id + '" style="color: var(--red-700)">Delete<span class="visually-hidden"> ' + esc(p.name) + "</span></button>" +
            "</td></tr>"
        )
        .join("") +
      "</tbody></table></div>";
  }

  async function loadProducts() {
    try {
      products = (await api("/api/owner/products")).products;
      renderProducts();
    } catch (error) {
      document.getElementById("product-table").innerHTML = '<div class="empty-state"><h3>Could not load products</h3><p>' + esc(error.message) + "</p></div>";
    }
  }

  // ---------- add / edit dialog ----------
  const productDialog = document.getElementById("product-dialog");
  const productForm = document.getElementById("product-form");
  let lastTrigger = null;

  function openProductDialog(product, trigger) {
    lastTrigger = trigger;
    productForm.reset();
    clearFormError(productForm);
    productForm.querySelectorAll("[aria-invalid]").forEach((el) => el.removeAttribute("aria-invalid"));
    document.getElementById("product-dialog-title").textContent = product ? "Edit product" : "Add product";
    document.getElementById("product-submit").textContent = product ? "Save changes" : "Add product";
    document.getElementById("f-id").value = product ? product.id : "";
    if (product) {
      document.getElementById("f-name").value = product.name;
      document.getElementById("f-category").value = product.category;
      document.getElementById("f-price").value = Number(product.price) % 1 === 0 ? Number(product.price) : Number(product.price).toFixed(2);
      document.getElementById("f-quantity").value = product.quantity;
      document.getElementById("f-description").value = product.description;
    }
    productDialog.showModal();
    document.getElementById("f-name").focus();
  }

  function checkProductForm() {
    const checks = [
      ["f-name", (v) => v.length >= 2, "Enter a product name of at least 2 characters."],
      ["f-category", (v) => v !== "", "Choose a category for the product."],
      ["f-price", (v) => /^\d+(\.\d{1,2})?$/.test(v) && Number(v) > 0, "Enter a price above 0, like 250 or 99.50."],
      ["f-quantity", (v) => /^\d+$/.test(v), "Enter the quantity as a whole number, 0 or more."],
    ];
    for (const [id, test, message] of checks) {
      const input = document.getElementById(id);
      const ok = test(input.value.trim());
      input.setAttribute("aria-invalid", ok ? "false" : "true");
      if (!ok) {
        showFormError(productForm, message);
        input.focus();
        return false;
      }
    }
    return true;
  }

  productForm.addEventListener("submit", async (event) => {
    event.preventDefault();
    clearFormError(productForm);
    if (!checkProductForm()) return;
    const id = document.getElementById("f-id").value;
    const button = document.getElementById("product-submit");
    setBusy(button, true, "Saving");
    try {
      const path = id ? "/api/owner/products/" + id + "/update" : "/api/owner/products";
      await api(path, { method: "POST", data: new FormData(productForm) });
      productDialog.close();
      await loadProducts();
      toast(id ? "Product updated" : "Product added");
    } catch (error) {
      showFormError(productForm, error.message);
    } finally {
      setBusy(button, false);
    }
  });

  // ---------- delete dialog ----------
  const deleteDialog = document.getElementById("delete-dialog");
  let deleteId = null;

  document.getElementById("confirm-delete").addEventListener("click", async (event) => {
    const button = event.currentTarget;
    setBusy(button, true, "Deleting");
    try {
      await api("/api/owner/products/" + deleteId + "/delete", { method: "POST" });
      deleteDialog.close();
      await loadProducts();
      toast("Product deleted");
    } catch (error) {
      toast(error.message, true);
    } finally {
      setBusy(button, false);
    }
  });

  // Return keyboard focus to the button that opened a dialog
  [productDialog, deleteDialog].forEach((dialog) => {
    dialog.addEventListener("click", (event) => {
      if (event.target.closest("[data-close]")) dialog.close();
    });
    dialog.addEventListener("close", () => {
      if (lastTrigger && document.body.contains(lastTrigger)) lastTrigger.focus();
    });
  });

  // One click listener for every Add / Edit / Delete button on the page
  document.addEventListener("click", (event) => {
    const button = event.target.closest("[data-action]");
    if (!button) return;
    if (button.dataset.action === "add") return openProductDialog(null, button);
    const product = products.find((p) => String(p.id) === button.dataset.id);
    if (!product) return;
    if (button.dataset.action === "edit") openProductDialog(product, button);
    if (button.dataset.action === "delete") {
      lastTrigger = button;
      deleteId = product.id;
      document.getElementById("delete-title").textContent = "Delete " + product.name + "?";
      deleteDialog.showModal();
    }
  });

  // ---------- enquiries ----------
  async function loadEnquiries() {
    const box = document.getElementById("enquiry-list");
    try {
      enquiries = (await api("/api/owner/enquiries")).enquiries;
    } catch (error) {
      box.innerHTML = '<div class="empty-state"><h3>Could not load enquiries</h3><p>' + esc(error.message) + "</p></div>";
      return;
    }
    updateStats();
    if (enquiries.length === 0) {
      box.innerHTML = '<div class="empty-state"><h3>No enquiries yet</h3><p>When a customer asks about one of your products, it will appear here with their email so you can reply.</p></div>';
      return;
    }
    box.innerHTML =
      '<ul class="enquiry-list">' +
      enquiries
        .map((e) => {
          const subject = encodeURIComponent("Re: " + e.productName + " on GrowNex");
          return (
            '<li class="enquiry">' +
            '<div class="enquiry-top"><h3>' + esc(e.productName) + '</h3><time datetime="' + esc(e.createdAt.replace(" ", "T")) + '">' + esc(e.createdAt) + "</time></div>" +
            "<blockquote>" + esc(e.message) + "</blockquote>" +
            '<div class="enquiry-from"><span>From <strong>' + esc(e.customerName) + "</strong></span>" +
            "<span>" + esc(e.customerEmail) + "</span>" +
            '<a class="btn btn-secondary btn-sm" href="mailto:' + esc(e.customerEmail) + "?subject=" + subject + '">' + GrowNex.icon.mail + "Reply by email</a></div>" +
            "</li>"
          );
        })
        .join("") +
      "</ul>";
  }

  // ---------- profile ----------
  const profileForm = document.getElementById("profile-form");
  function fillProfile(u) {
    document.getElementById("p-businessName").value = u.businessName;
    document.getElementById("p-category").value = u.category;
    document.getElementById("p-city").value = u.city;
    document.getElementById("p-phone").value = u.phone;
    document.getElementById("p-description").value = u.description;
  }
  fillProfile(user);

  profileForm.addEventListener("submit", async (event) => {
    event.preventDefault();
    clearFormError(profileForm);
    const button = profileForm.querySelector('button[type="submit"]');
    setBusy(button, true, "Saving");
    try {
      const data = await api("/api/owner/profile", { method: "POST", data: new FormData(profileForm) });
      showOwner(data.user);
      fillProfile(data.user);
      toast("Profile saved");
    } catch (error) {
      showFormError(profileForm, error.message);
    } finally {
      setBusy(button, false);
    }
  });

  await Promise.all([loadProducts(), loadEnquiries()]);
})();
