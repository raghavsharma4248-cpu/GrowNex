/* auth.js - login and registration forms.
 * The browser checks the obvious mistakes first (empty fields, short
 * password) so people get instant feedback. The C++ server checks every
 * rule again, because browser checks can always be bypassed. */
(async function () {
  const { api, param, setBusy, showFormError, clearFormError, fillCategorySelect, ready, dashboardFor } = GrowNex;

  // Only allow redirects to our own pages, e.g. "product.html?id=3".
  function safeNext() {
    const next = param("next");
    return /^[a-z-]+\.html(\?[A-Za-z0-9=&%_-]*)?$/.test(next) ? next : "";
  }

  // Someone already logged in does not need these pages.
  ready().then((user) => {
    // Online preview: there are no accounts, so explain instead of showing a form.
    if (GrowNex.demoMode) {
      const card = document.querySelector(".auth-card");
      if (card) GrowNex.showLocalOnly(card, "Accounts are only available when running locally");
      return;
    }
    if (user) window.location.href = safeNext() || dashboardFor(user);
  });

  // Checks one field and returns an error message, or "" if it is fine.
  function checkField(input) {
    const label = document.querySelector('label[for="' + input.id + '"]');
    const name = (label ? label.childNodes[0].textContent.trim() : input.name).replace(/^Your /, "");
    const value = input.value.trim();
    if (input.required && !value) {
      return input.tagName === "SELECT" ? "Choose a " + name.toLowerCase() + "." : "Enter your " + name.toLowerCase() + ".";
    }
    if (input.type === "email" && value && !/^[^\s@|]+@[^\s@|]+\.[^\s@|]+$/.test(value))
      return "Enter a valid email address, for example name@example.com.";
    if (input.minLength > 0 && value && value.length < input.minLength)
      return name.charAt(0).toUpperCase() + name.slice(1) + " must be at least " + input.minLength + " characters.";
    return "";
  }

  function validate(form) {
    let firstError = "";
    let firstInput = null;
    form.querySelectorAll("input.input, select.select").forEach((input) => {
      if (input.closest("[hidden]")) return;
      const error = checkField(input);
      input.setAttribute("aria-invalid", error ? "true" : "false");
      if (error && !firstError) {
        firstError = error;
        firstInput = input;
      }
    });
    if (firstError) {
      showFormError(form, firstError);
      firstInput.focus();
      return false;
    }
    return true;
  }

  async function submit(form, path, busyText) {
    clearFormError(form);
    if (!validate(form)) return;
    const button = form.querySelector('button[type="submit"]');
    setBusy(button, true, busyText);
    try {
      const data = await api(path, { method: "POST", data: new FormData(form) });
      window.location.href = safeNext() || data.redirect.replace(/^\//, "");
    } catch (error) {
      setBusy(button, false);
      showFormError(form, error.message);
    }
  }

  // ---------- login page ----------
  const loginForm = document.getElementById("login-form");
  if (loginForm) {
    loginForm.addEventListener("submit", (event) => {
      event.preventDefault();
      submit(loginForm, "/api/login", "Logging in");
    });
  }

  // ---------- register page ----------
  const registerForm = document.getElementById("register-form");
  if (registerForm) {
    const businessFields = document.getElementById("business-fields");
    const button = document.getElementById("register-button");
    const businessInputs = ["businessName", "category", "phone"].map((id) => document.getElementById(id));

    // Show the business fields only for business owners (progressive disclosure).
    function updateRole() {
      const isBusiness = document.getElementById("role-business").checked;
      businessFields.hidden = !isBusiness;
      businessInputs.forEach((input) => (input.required = isBusiness));
      button.textContent = isBusiness ? "Create business account" : "Create customer account";
    }
    registerForm.querySelectorAll('input[name="role"]').forEach((radio) => radio.addEventListener("change", updateRole));

    if (param("role") === "business") document.getElementById("role-business").checked = true;
    if (param("role") !== "business") document.body.removeAttribute("data-nav");
    await fillCategorySelect(document.getElementById("category"), "Choose a category");
    // In the online preview the form has been replaced by a message by now.
    if (!document.body.contains(registerForm)) return;
    updateRole();

    registerForm.addEventListener("submit", (event) => {
      event.preventDefault();
      submit(registerForm, "/api/register", "Creating account");
    });
  }
})();
