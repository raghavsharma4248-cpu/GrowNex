/* app.js - shared code loaded on every page.
 *
 * How the frontend talks to the C++ backend:
 *   1. A page calls api("/api/products") or api("/api/login", { method: "POST", data: form }).
 *   2. fetch() sends an HTTP request to the C++ server on localhost.
 *   3. The C++ Server class handles it and replies with JSON.
 *   4. We read the JSON and update the page.
 * The login session is kept in an HttpOnly cookie that the browser sends
 * automatically with every request, so JavaScript never sees the token.
 */

const GrowNex = (() => {
  // ---------- API helper ----------
  async function api(path, options = {}) {
    const request = { method: options.method || "GET", headers: {}, credentials: "same-origin" };

    if (options.data) {
      // Send form fields as application/x-www-form-urlencoded.
      // cpp-httplib reads them into req.params on the server.
      const body = options.data instanceof FormData ? new URLSearchParams(options.data) : new URLSearchParams(options.data);
      request.body = body;
      request.headers["Content-Type"] = "application/x-www-form-urlencoded";
    }

    let response;
    try {
      response = await fetch(path, request);
    } catch (networkError) {
      throw new Error("Cannot reach the GrowNex server. Check that grownex is running, then try again.");
    }

    let json;
    try {
      json = await response.json();
    } catch (parseError) {
      throw new Error("The server sent an unexpected reply (HTTP " + response.status + ").");
    }
    if (!response.ok || json.ok === false) {
      const error = new Error(json.error || "Request failed (HTTP " + response.status + ").");
      error.status = response.status;
      throw error;
    }
    return json;
  }

  // ---------- formatting ----------

  // Escape text before putting it into HTML so user input can never
  // inject tags or scripts into the page.
  function esc(value) {
    return String(value ?? "")
      .replace(/&/g, "&amp;")
      .replace(/</g, "&lt;")
      .replace(/>/g, "&gt;")
      .replace(/"/g, "&quot;")
      .replace(/'/g, "&#39;");
  }

  function price(value) {
    const amount = Number(value);
    const whole = Number.isInteger(amount);
    return new Intl.NumberFormat("en-IN", {
      style: "currency",
      currency: "INR",
      minimumFractionDigits: whole ? 0 : 2,
      maximumFractionDigits: 2,
    }).format(amount);
  }

  function stock(quantity) {
    const q = Number(quantity);
    if (q <= 0) return '<span class="stock stock-out">Out of stock</span>';
    if (q <= 5) return '<span class="stock stock-low">Only ' + q + " left</span>";
    return '<span class="stock stock-in">' + q.toLocaleString("en-IN") + " in stock</span>";
  }

  function initials(name) {
    const words = String(name).replace(/[^A-Za-z\s]/g, "").trim().split(/\s+/).filter(Boolean);
    if (words.length === 0) return "G";
    return (words[0][0] + (words[1] ? words[1][0] : "")).toUpperCase();
  }

  function param(name) {
    return new URLSearchParams(window.location.search).get(name) || "";
  }

  // ---------- small inline icons (SVG, never emoji) ----------
  const icon = {
    pin: '<svg viewBox="0 0 24 24" width="16" height="16" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="M12 21s-7-6.2-7-11.5A7 7 0 0 1 19 9.5C19 14.8 12 21 12 21z"/><circle cx="12" cy="9.5" r="2.5"/></svg>',
    phone: '<svg viewBox="0 0 24 24" width="16" height="16" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="M5 4h3.5l1.7 4.3-2.2 1.4a11 11 0 0 0 6.3 6.3l1.4-2.2L20 15.5V19a1.5 1.5 0 0 1-1.6 1.5A16.5 16.5 0 0 1 3.5 5.6 1.5 1.5 0 0 1 5 4z"/></svg>',
    tag: '<svg viewBox="0 0 24 24" width="16" height="16" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="M3 12V4h8l10 10-8 8L3 12z"/><circle cx="7.5" cy="8.5" r="1.3"/></svg>',
    box: '<svg viewBox="0 0 24 24" width="16" height="16" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="M3 7.5 12 3l9 4.5v9L12 21l-9-4.5v-9z"/><path d="M3 7.5 12 12l9-4.5M12 12v9"/></svg>',
    mail: '<svg viewBox="0 0 24 24" width="16" height="16" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><rect x="3" y="5" width="18" height="14" rx="2"/><path d="m4 7 8 6 8-6"/></svg>',
  };

  // ---------- reusable card templates ----------
  function productCard(p) {
    return (
      '<article class="card product-card">' +
      '<span class="category">' + esc(p.category) + "</span>" +
      '<h3><a href="product.html?id=' + p.id + '">' + esc(p.name) + "</a></h3>" +
      '<p class="seller">by ' + esc(p.businessName) + "</p>" +
      '<div class="card-foot"><span class="price">' + price(p.price) + "</span>" + stock(p.quantity) + "</div>" +
      "</article>"
    );
  }

  function businessCard(b) {
    const count = b.productCount === 1 ? "1 product" : b.productCount + " products";
    return (
      '<article class="card business-card">' +
      '<div class="top"><span class="monogram" aria-hidden="true">' + esc(initials(b.businessName)) + "</span>" +
      '<div><h3><a href="business.html?id=' + b.id + '">' + esc(b.businessName) + "</a></h3>" +
      '<div class="meta-line"><span>' + icon.pin + esc(b.city) + "</span><span>" + esc(b.category) + "</span></div></div></div>" +
      (b.description ? '<p class="about">' + esc(b.description) + "</p>" : "") +
      '<div class="meta-line"><span>' + icon.box + count + "</span></div>" +
      "</article>"
    );
  }

  // ---------- toast messages ----------
  let toastTimer;
  function toast(message, isError = false) {
    let box = document.getElementById("toast");
    if (!box) {
      box = document.createElement("div");
      box.id = "toast";
      box.className = "toast";
      box.setAttribute("role", "status");
      box.setAttribute("aria-live", "polite");
      document.body.appendChild(box);
    }
    box.textContent = message;
    box.classList.toggle("is-error", isError);
    box.classList.add("is-visible");
    clearTimeout(toastTimer);
    toastTimer = setTimeout(() => box.classList.remove("is-visible"), 4000);
  }

  // ---------- form helpers ----------
  function showFormError(form, message) {
    const box = form.querySelector(".form-error");
    if (!box) return toast(message, true);
    box.textContent = message;
    box.classList.add("is-visible");
    box.focus();
  }

  function clearFormError(form) {
    const box = form.querySelector(".form-error");
    if (box) {
      box.textContent = "";
      box.classList.remove("is-visible");
    }
  }

  function setBusy(button, busy, busyText) {
    if (!button) return;
    if (busy) {
      button.dataset.label = button.textContent;
      button.textContent = busyText || "Please wait";
      button.setAttribute("aria-busy", "true");
      button.disabled = true;
    } else {
      button.textContent = button.dataset.label || button.textContent;
      button.removeAttribute("aria-busy");
      button.disabled = false;
    }
  }

  // Adds Show / Hide buttons to every password field on the page.
  function setupPasswordToggles() {
    document.querySelectorAll("[data-password-toggle]").forEach((button) => {
      const input = document.getElementById(button.getAttribute("aria-controls"));
      button.addEventListener("click", () => {
        const show = input.type === "password";
        input.type = show ? "text" : "password";
        button.textContent = show ? "Hide" : "Show";
        button.setAttribute("aria-pressed", String(show));
      });
    });
  }

  async function fillCategorySelect(select, placeholder) {
    const data = await api("/api/categories");
    const current = select.dataset.value || select.value;
    select.innerHTML =
      (placeholder ? '<option value="">' + esc(placeholder) + "</option>" : "") +
      data.categories.map((c) => '<option value="' + esc(c) + '">' + esc(c) + "</option>").join("");
    if (current) select.value = current;
    return data.categories;
  }

  // ---------- header: mobile menu and account links ----------
  let currentUser = null;

  async function loadSession() {
    try {
      const data = await api("/api/me");
      currentUser = data.loggedIn ? data.user : null;
    } catch (error) {
      currentUser = null;
    }
    renderAccount();
    return currentUser;
  }

  function dashboardFor(user) {
    return user.role === "business" ? "owner-dashboard.html" : "customer-dashboard.html";
  }

  function renderAccount() {
    const area = document.getElementById("nav-account");
    if (!area) return;
    if (!currentUser) {
      area.innerHTML =
        '<a class="btn btn-quiet" href="login.html">Log in</a>' +
        '<a class="btn btn-primary" href="register.html">Create account</a>';
      return;
    }
    area.innerHTML =
      '<a class="btn btn-secondary" href="' + dashboardFor(currentUser) + '">Dashboard</a>' +
      '<button class="btn btn-quiet" type="button" id="logout-button">Log out</button>';
    document.getElementById("logout-button").addEventListener("click", logout);
  }

  async function logout() {
    try {
      await api("/api/logout", { method: "POST" });
    } finally {
      window.location.href = "index.html";
    }
  }

  function setupMenu() {
    const toggle = document.querySelector(".nav-toggle");
    const nav = document.getElementById("site-nav");
    if (!toggle || !nav) return;
    toggle.addEventListener("click", () => {
      const open = nav.classList.toggle("is-open");
      toggle.setAttribute("aria-expanded", String(open));
    });
  }

  // Every page checks the session once when it loads; pages can wait for it.
  let sessionPromise = null;
  function ready() {
    if (!sessionPromise) sessionPromise = loadSession();
    return sessionPromise;
  }

  // Sends people who are not logged in (or have the wrong role) to login.
  async function requireRole(role) {
    const user = await ready();
    if (!user) {
      window.location.href = "login.html?next=" + encodeURIComponent(location.pathname.replace(/^\//, ""));
      return null;
    }
    if (user.role !== role) {
      window.location.href = dashboardFor(user);
      return null;
    }
    return user;
  }

  document.addEventListener("DOMContentLoaded", () => {
    setupMenu();
    setupPasswordToggles();
    ready();
    // Highlight the current section in the main navigation.
    const section = document.body.dataset.nav;
    if (section) {
      const link = document.querySelector('.nav-links [data-nav="' + section + '"]');
      if (link) link.setAttribute("aria-current", "page");
    }
    const year = document.getElementById("year");
    if (year) year.textContent = new Date().getFullYear();
  });

  return {
    api, esc, price, stock, initials, param, icon, productCard, businessCard,
    toast, showFormError, clearFormError, setBusy, fillCategorySelect,
    ready, requireRole, dashboardFor,
    get user() { return currentUser; },
  };
})();
