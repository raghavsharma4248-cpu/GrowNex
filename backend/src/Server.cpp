// Server.cpp
// Every /api/... route is registered in setupRoutes() and handled by one
// member function below. Handlers:
//   1. read form fields from the request,
//   2. validate them with Validator,
//   3. change the in-memory vectors,
//   4. save the changed vector through FileManager,
//   5. reply with JSON.
#include "Server.h"
#include "Security.h"
#include "Utils.h"

#include <algorithm>
#include <filesystem>
#include <iostream>

namespace {
const char* COOKIE_NAME = "grownex_session";
}

Server::Server(const std::string& frontendFolder, const std::string& dataFolder)
    : files(dataFolder), frontendFolder(frontendFolder) {}

bool Server::initialise() {
    if (!std::filesystem::exists(std::filesystem::path(frontendFolder) / "index.html")) {
        std::cerr << "Cannot find " << frontendFolder << "/index.html.\n"
                  << "Run the program from the GrowNex project folder.\n";
        return false;
    }
    if (!files.prepare()) return false;
    businesses = files.loadBusinesses();
    customers = files.loadCustomers();
    products = files.loadProducts();
    enquiries = files.loadEnquiries();
    std::cout << "Loaded " << businesses.size() << " businesses, " << customers.size()
              << " customers, " << products.size() << " products and " << enquiries.size()
              << " enquiries." << std::endl;
    setupRoutes();
    return true;
}

bool Server::run(const std::string& host, int port) {
    // Reserve the port first, so we only say "running" when it really is.
    if (!http.bind_to_port(host, port)) return false;
    std::cout << "GrowNex is running at http://localhost:" << port << "\n"
              << "Press Ctrl + C to stop the server." << std::endl;
    return http.listen_after_bind();
}

// ===================================================================
// Routes
// ===================================================================

void Server::setupRoutes() {
    // cpp-httplib normally lets several programs share one port. We do not
    // want that: two copies of GrowNex would overwrite each other's files.
    // These options make a second copy fail with "port in use" instead.
    http.set_socket_options([](socket_t sock) {
#ifdef _WIN32
        httplib::set_socket_opt(sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, 1);
#else
        httplib::set_socket_opt(sock, SOL_SOCKET, SO_REUSEADDR, 1);
#endif
    });

    // Static files: http://localhost:8080/ serves frontend/index.html, etc.
    http.set_mount_point("/", frontendFolder);

    // Accounts
    http.Post("/api/register", [this](const httplib::Request& q, httplib::Response& s) { handleRegister(q, s); });
    http.Post("/api/login",    [this](const httplib::Request& q, httplib::Response& s) { handleLogin(q, s); });
    http.Post("/api/logout",   [this](const httplib::Request& q, httplib::Response& s) { handleLogout(q, s); });
    http.Get("/api/me",        [this](const httplib::Request& q, httplib::Response& s) { handleMe(q, s); });

    // Public browsing (no login needed)
    http.Get("/api/categories",            [this](const httplib::Request& q, httplib::Response& s) { handleCategories(q, s); });
    http.Get("/api/businesses",            [this](const httplib::Request& q, httplib::Response& s) { handleListBusinesses(q, s); });
    http.Get(R"(/api/businesses/(\d+))",   [this](const httplib::Request& q, httplib::Response& s) { handleGetBusiness(q, s); });
    http.Get("/api/products",              [this](const httplib::Request& q, httplib::Response& s) { handleListProducts(q, s); });
    http.Get(R"(/api/products/(\d+))",     [this](const httplib::Request& q, httplib::Response& s) { handleGetProduct(q, s); });

    // Business owner only
    http.Post("/api/owner/profile",                    [this](const httplib::Request& q, httplib::Response& s) { handleUpdateProfile(q, s); });
    http.Get("/api/owner/products",                    [this](const httplib::Request& q, httplib::Response& s) { handleOwnerProducts(q, s); });
    http.Post("/api/owner/products",                   [this](const httplib::Request& q, httplib::Response& s) { handleCreateProduct(q, s); });
    http.Post(R"(/api/owner/products/(\d+)/update)",   [this](const httplib::Request& q, httplib::Response& s) { handleUpdateProduct(q, s); });
    http.Post(R"(/api/owner/products/(\d+)/delete)",   [this](const httplib::Request& q, httplib::Response& s) { handleDeleteProduct(q, s); });
    http.Get("/api/owner/enquiries",                   [this](const httplib::Request& q, httplib::Response& s) { handleOwnerEnquiries(q, s); });

    // Customer only
    http.Post("/api/enquiries",          [this](const httplib::Request& q, httplib::Response& s) { handleCreateEnquiry(q, s); });
    http.Get("/api/customer/enquiries",  [this](const httplib::Request& q, httplib::Response& s) { handleCustomerEnquiries(q, s); });

    // If any handler throws an exception, reply with a JSON error instead of crashing.
    http.set_exception_handler([](const httplib::Request&, httplib::Response& res, std::exception_ptr) {
        sendError(res, 500, "Something went wrong on the server. Please try again.");
    });
}

// ===================================================================
// Helpers
// ===================================================================

void Server::sendJson(httplib::Response& res, int status, const std::string& body) {
    res.status = status;
    res.set_header("Cache-Control", "no-store");
    res.set_content(body, "application/json; charset=utf-8");
}

void Server::sendError(httplib::Response& res, int status, const std::string& message) {
    sendJson(res, status, "{\"ok\":false,\"error\":" + utils::jsonString(message) + "}");
}

std::string Server::field(const httplib::Request& req, const std::string& name) {
    return req.has_param(name) ? utils::trim(req.get_param_value(name)) : "";
}

std::string Server::readCookie(const httplib::Request& req, const std::string& name) {
    // The Cookie header looks like: "a=1; grownex_session=abc123; b=2"
    std::string header = req.get_header_value("Cookie");
    std::string key = name + "=";
    std::size_t pos = 0;
    while (pos < header.size()) {
        std::size_t end = header.find(';', pos);
        if (end == std::string::npos) end = header.size();
        std::string part = utils::trim(header.substr(pos, end - pos));
        if (part.compare(0, key.size(), key) == 0) return part.substr(key.size());
        pos = end + 1;
    }
    return "";
}

const Server::Session* Server::currentSession(const httplib::Request& req) const {
    auto it = sessions.find(readCookie(req, COOKIE_NAME));
    return it == sessions.end() ? nullptr : &it->second;
}

// Returns a base-class pointer to either a Business or a Customer.
// The caller does not need to know which: it simply calls the virtual
// functions (getRole, dashboardPage, toJson) - this is polymorphism.
User* Server::findUserByEmail(const std::string& email) {
    std::string wanted = utils::toLower(email);
    for (Business& b : businesses)
        if (utils::toLower(b.getEmail()) == wanted) return &b;
    for (Customer& c : customers)
        if (utils::toLower(c.getEmail()) == wanted) return &c;
    return nullptr;
}

User* Server::findUser(const Session& session) {
    if (session.role == "business") return findBusiness(session.userId);
    for (Customer& c : customers)
        if (c.getId() == session.userId) return &c;
    return nullptr;
}

Business* Server::findBusiness(int id) {
    for (Business& b : businesses)
        if (b.getId() == id) return &b;
    return nullptr;
}

Product* Server::findProduct(int id) {
    for (Product& p : products)
        if (p.getId() == id) return &p;
    return nullptr;
}

std::string Server::businessNameOf(int businessId) {
    Business* b = findBusiness(businessId);
    return b ? b->getBusinessName() : "Unknown business";
}

int Server::productCountOf(int businessId) const {
    return static_cast<int>(std::count_if(products.begin(), products.end(),
        [businessId](const Product& p) { return p.getBusinessId() == businessId; }));
}

// ===================================================================
// Accounts
// ===================================================================

void Server::handleRegister(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(dataMutex);

    std::string role = field(req, "role");
    std::string name = field(req, "name");
    std::string email = utils::toLower(field(req, "email"));
    std::string password = req.has_param("password") ? req.get_param_value("password") : "";
    std::string city = field(req, "city");

    if (role != "business" && role != "customer")
        return sendError(res, 400, "Choose whether you are a business owner or a customer.");

    std::string error = Validator::personName(name);
    if (error.empty()) error = Validator::email(email);
    if (error.empty()) error = Validator::password(password);
    if (error.empty()) error = Validator::city(city);
    if (!error.empty()) return sendError(res, 400, error);

    if (findUserByEmail(email))
        return sendError(res, 409, "An account with this email already exists. Try logging in.");

    Session session;
    session.role = role;

    if (role == "business") {
        std::string businessName = field(req, "businessName");
        std::string category = field(req, "category");
        std::string phone = field(req, "phone");
        error = Validator::businessName(businessName);
        if (error.empty()) error = Validator::category(category);
        if (error.empty()) error = Validator::phone(phone);
        if (!error.empty()) return sendError(res, 400, error);

        Business owner(nextId(businesses), name, email, businessName, category, city, phone);
        owner.setPassword(password);
        businesses.push_back(owner);
        if (!files.saveBusinesses(businesses)) return sendError(res, 500, "Could not save your account.");
        session.userId = owner.getId();
    } else {
        Customer customer(nextId(customers), name, email, city);
        customer.setPassword(password);
        customers.push_back(customer);
        if (!files.saveCustomers(customers)) return sendError(res, 500, "Could not save your account.");
        session.userId = customer.getId();
    }

    // Log the new user in straight away.
    std::string token = utils::randomHex(32);
    sessions[token] = session;
    res.set_header("Set-Cookie", std::string(COOKIE_NAME) + "=" + token +
                                     "; Path=/; HttpOnly; SameSite=Strict");
    User* user = findUser(session);
    sendJson(res, 201, "{\"ok\":true,\"redirect\":" + utils::jsonString(user->dashboardPage()) +
                           ",\"user\":" + user->toJson() + "}");
}

void Server::handleLogin(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(dataMutex);

    std::string email = utils::toLower(field(req, "email"));
    std::string password = req.has_param("password") ? req.get_param_value("password") : "";
    if (email.empty() || password.empty())
        return sendError(res, 400, "Enter your email and password.");

    User* user = findUserByEmail(email);
    // Same message for "no such email" and "wrong password", so the page
    // does not reveal which emails are registered.
    if (!user || !user->checkPassword(password))
        return sendError(res, 401, "Email or password is incorrect.");

    Session session;
    session.role = user->getRole();      // virtual call
    session.userId = user->getId();
    std::string token = utils::randomHex(32);
    sessions[token] = session;

    res.set_header("Set-Cookie", std::string(COOKIE_NAME) + "=" + token +
                                     "; Path=/; HttpOnly; SameSite=Strict");
    sendJson(res, 200, "{\"ok\":true,\"redirect\":" + utils::jsonString(user->dashboardPage()) +
                           ",\"user\":" + user->toJson() + "}");
}

void Server::handleLogout(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(dataMutex);
    sessions.erase(readCookie(req, COOKIE_NAME));
    res.set_header("Set-Cookie", std::string(COOKIE_NAME) +
                                     "=; Path=/; Max-Age=0; HttpOnly; SameSite=Strict");
    sendJson(res, 200, "{\"ok\":true}");
}

void Server::handleMe(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(dataMutex);
    const Session* session = currentSession(req);
    User* user = session ? findUser(*session) : nullptr;
    if (!user) return sendJson(res, 200, "{\"ok\":true,\"loggedIn\":false}");
    sendJson(res, 200, "{\"ok\":true,\"loggedIn\":true,\"user\":" + user->toJson() + "}");
}

// ===================================================================
// Public browsing
// ===================================================================

void Server::handleCategories(const httplib::Request&, httplib::Response& res) {
    std::string body = "{\"ok\":true,\"categories\":[";
    const auto& list = utils::categories();
    for (std::size_t i = 0; i < list.size(); ++i) {
        if (i > 0) body += ",";
        body += utils::jsonString(list[i]);
    }
    sendJson(res, 200, body + "]}");
}

void Server::handleListBusinesses(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(dataMutex);
    std::string q = field(req, "q");
    std::string category = field(req, "category");

    std::string body = "{\"ok\":true,\"businesses\":[";
    bool first = true;
    for (const Business& b : businesses) {
        if (!category.empty() && b.getCategory() != category) continue;
        if (!q.empty() && !utils::containsIgnoreCase(b.getBusinessName(), q) &&
            !utils::containsIgnoreCase(b.getCity(), q) &&
            !utils::containsIgnoreCase(b.getDescription(), q))
            continue;
        if (!first) body += ",";
        body += b.publicJson(productCountOf(b.getId()));
        first = false;
    }
    sendJson(res, 200, body + "]}");
}

void Server::handleGetBusiness(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(dataMutex);
    int id = std::stoi(req.matches[1]);
    Business* b = findBusiness(id);
    if (!b) return sendError(res, 404, "This business could not be found.");

    std::string list;
    for (auto it = products.rbegin(); it != products.rend(); ++it) {
        if (it->getBusinessId() != id) continue;
        if (!list.empty()) list += ",";
        list += it->toJson(b->getBusinessName());
    }
    sendJson(res, 200, "{\"ok\":true,\"business\":" + b->publicJson(productCountOf(id)) +
                           ",\"products\":[" + list + "]}");
}

void Server::handleListProducts(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(dataMutex);
    std::string q = field(req, "q");
    std::string category = field(req, "category");
    int limit = 0;
    try {
        if (!field(req, "limit").empty()) limit = std::stoi(field(req, "limit"));
    } catch (...) {
        limit = 0;
    }

    std::string body = "{\"ok\":true,\"products\":[";
    int count = 0;
    // Newest products first (they are stored in the order they were added).
    for (auto it = products.rbegin(); it != products.rend(); ++it) {
        if (!category.empty() && it->getCategory() != category) continue;
        std::string businessName = businessNameOf(it->getBusinessId());
        if (!q.empty() && !it->matches(q) && !utils::containsIgnoreCase(businessName, q)) continue;
        if (count > 0) body += ",";
        body += it->toJson(businessName);
        if (++count == limit) break;
    }
    sendJson(res, 200, body + "]}");
}

void Server::handleGetProduct(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(dataMutex);
    Product* p = findProduct(std::stoi(req.matches[1]));
    if (!p) return sendError(res, 404, "This product could not be found. It may have been removed.");
    Business* b = findBusiness(p->getBusinessId());
    std::string businessJson = b ? b->publicJson(productCountOf(b->getId())) : "null";
    sendJson(res, 200, "{\"ok\":true,\"product\":" + p->toJson(businessNameOf(p->getBusinessId())) +
                           ",\"business\":" + businessJson + "}");
}

// ===================================================================
// Business owner
// ===================================================================

// Every owner handler starts with this check.
#define REQUIRE_ROLE(wantedRole, message)                                   \
    const Session* session = currentSession(req);                          \
    if (!session) return sendError(res, 401, "Please log in to continue."); \
    if (session->role != wantedRole) return sendError(res, 403, message);

void Server::handleUpdateProfile(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(dataMutex);
    REQUIRE_ROLE("business", "Only business owners can edit a business profile.");

    std::string businessName = field(req, "businessName");
    std::string category = field(req, "category");
    std::string city = field(req, "city");
    std::string phone = field(req, "phone");
    std::string description = field(req, "description");

    std::string error = Validator::businessName(businessName);
    if (error.empty()) error = Validator::category(category);
    if (error.empty()) error = Validator::city(city);
    if (error.empty()) error = Validator::phone(phone);
    if (error.empty()) error = Validator::description(description, 600);
    if (!error.empty()) return sendError(res, 400, error);

    Business* b = findBusiness(session->userId);
    if (!b) return sendError(res, 404, "Your business account could not be found.");
    b->updateProfile(businessName, category, city, phone, description);
    if (!files.saveBusinesses(businesses)) return sendError(res, 500, "Could not save your profile.");
    sendJson(res, 200, "{\"ok\":true,\"user\":" + b->toJson() + "}");
}

void Server::handleOwnerProducts(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(dataMutex);
    REQUIRE_ROLE("business", "Only business owners have a product list.");

    std::string name = businessNameOf(session->userId);
    std::string list;
    for (auto it = products.rbegin(); it != products.rend(); ++it) {
        if (it->getBusinessId() != session->userId) continue;
        if (!list.empty()) list += ",";
        list += it->toJson(name);
    }
    sendJson(res, 200, "{\"ok\":true,\"products\":[" + list + "]}");
}

namespace {
// Reads and validates the product form. Used by both create and update.
std::string readProductForm(const httplib::Request& req, std::string& name, std::string& category,
                            std::string& description, double& price, int& quantity) {
    auto get = [&req](const char* key) {
        return req.has_param(key) ? utils::trim(req.get_param_value(key)) : std::string();
    };
    name = get("name");
    category = get("category");
    description = get("description");
    std::string error = Validator::productName(name);
    if (error.empty()) error = Validator::category(category);
    if (error.empty()) error = Validator::description(description, 800);
    if (error.empty()) error = Validator::price(get("price"), price);
    if (error.empty()) error = Validator::quantity(get("quantity"), quantity);
    return error;
}
} // namespace

void Server::handleCreateProduct(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(dataMutex);
    REQUIRE_ROLE("business", "Only business owners can add products.");

    std::string name, category, description;
    double price = 0;
    int quantity = 0;
    std::string error = readProductForm(req, name, category, description, price, quantity);
    if (!error.empty()) return sendError(res, 400, error);

    // A new id must not match a deleted product that an old enquiry still
    // points to, so we also look at the product ids stored in enquiries.
    int newId = nextId(products);
    for (const Enquiry& e : enquiries)
        if (e.getProductId() >= newId) newId = e.getProductId() + 1;

    Product product(newId, session->userId, name, category, description, price, quantity);
    products.push_back(product);
    if (!files.saveProducts(products)) return sendError(res, 500, "Could not save the product.");
    sendJson(res, 201, "{\"ok\":true,\"product\":" + product.toJson(businessNameOf(session->userId)) + "}");
}

void Server::handleUpdateProduct(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(dataMutex);
    REQUIRE_ROLE("business", "Only business owners can edit products.");

    Product* product = findProduct(std::stoi(req.matches[1]));
    // An owner may only change their own products.
    if (!product || product->getBusinessId() != session->userId)
        return sendError(res, 404, "This product could not be found in your list.");

    std::string name, category, description;
    double price = 0;
    int quantity = 0;
    std::string error = readProductForm(req, name, category, description, price, quantity);
    if (!error.empty()) return sendError(res, 400, error);

    product->update(name, category, description, price, quantity);
    if (!files.saveProducts(products)) return sendError(res, 500, "Could not save the product.");
    sendJson(res, 200, "{\"ok\":true,\"product\":" + product->toJson(businessNameOf(session->userId)) + "}");
}

void Server::handleDeleteProduct(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(dataMutex);
    REQUIRE_ROLE("business", "Only business owners can delete products.");

    int id = std::stoi(req.matches[1]);
    auto it = std::find_if(products.begin(), products.end(), [&](const Product& p) {
        return p.getId() == id && p.getBusinessId() == session->userId;
    });
    if (it == products.end()) return sendError(res, 404, "This product could not be found in your list.");
    products.erase(it);
    if (!files.saveProducts(products)) return sendError(res, 500, "Could not delete the product.");
    sendJson(res, 200, "{\"ok\":true}");
}

void Server::handleOwnerEnquiries(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(dataMutex);
    REQUIRE_ROLE("business", "Only business owners receive enquiries.");

    std::string name = businessNameOf(session->userId);
    std::string list;
    for (auto it = enquiries.rbegin(); it != enquiries.rend(); ++it) {
        if (it->getBusinessId() != session->userId) continue;
        if (!list.empty()) list += ",";
        list += it->toJson(name);
    }
    sendJson(res, 200, "{\"ok\":true,\"enquiries\":[" + list + "]}");
}

// ===================================================================
// Customer
// ===================================================================

void Server::handleCreateEnquiry(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(dataMutex);
    REQUIRE_ROLE("customer", "Enquiries are sent from a customer account.");

    int productId = 0;
    try {
        productId = std::stoi(field(req, "productId"));
    } catch (...) {
        return sendError(res, 400, "Choose a product to ask about.");
    }
    Product* product = findProduct(productId);
    if (!product) return sendError(res, 404, "This product could not be found. It may have been removed.");

    std::string message = field(req, "message");
    std::string error = Validator::enquiryMessage(message);
    if (!error.empty()) return sendError(res, 400, error);

    User* customer = findUser(*session);
    if (!customer) return sendError(res, 401, "Please log in to continue.");

    Enquiry enquiry(nextId(enquiries), product->getId(), product->getName(),
                    product->getBusinessId(), customer->getId(), customer->getName(),
                    customer->getEmail(), message);
    enquiries.push_back(enquiry);
    if (!files.saveEnquiries(enquiries)) return sendError(res, 500, "Could not send the enquiry.");
    sendJson(res, 201, "{\"ok\":true,\"enquiry\":" +
                           enquiry.toJson(businessNameOf(product->getBusinessId())) + "}");
}

void Server::handleCustomerEnquiries(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(dataMutex);
    REQUIRE_ROLE("customer", "Only customers have sent enquiries.");

    std::string list;
    for (auto it = enquiries.rbegin(); it != enquiries.rend(); ++it) {
        if (it->getCustomerId() != session->userId) continue;
        if (!list.empty()) list += ",";
        list += it->toJson(businessNameOf(it->getBusinessId()));
    }
    sendJson(res, 200, "{\"ok\":true,\"enquiries\":[" + list + "]}");
}

#undef REQUIRE_ROLE

// ===================================================================
// Demo data
// ===================================================================

void Server::seedDemoData() {
    std::lock_guard<std::mutex> lock(dataMutex);
    if (!businesses.empty() || !customers.empty()) {
        std::cout << "Demo data skipped: the data files already contain accounts.\n";
        return;
    }
    const std::string demoPassword = "Demo@1234";

    struct DemoBusiness {
        const char *owner, *email, *name, *category, *city, *phone, *about;
    };
    const DemoBusiness demo[] = {
        {"Meera Kulkarni", "meera@example.com", "Meera's Millet Kitchen", "Food & Beverages", "Pune",
         "98220 11234",
         "A home kitchen turned small brand. We bake and pack millet snacks in small batches "
         "with no maida and no preservatives."},
        {"Arjun Rao", "arjun@example.com", "Kaveri Handloom Studio", "Clothing & Textiles", "Mysuru",
         "99001 45678",
         "Four weavers working on pit looms. Every piece is woven by hand from cotton yarn "
         "dyed in our own workshop."},
        {"Sana Qureshi", "sana@example.com", "Leafline Repairs", "Electronics", "Jaipur",
         "94140 98765",
         "Phone and laptop repairs with a 90-day service warranty, plus tested refurbished "
         "accessories."},
        {"Tenzin Dorje", "tenzin@example.com", "Bodhi Paper Co.", "Stationery & Books", "Bhopal",
         "97540 22110",
         "Handmade paper from cotton rag and recycled office paper. Notebooks, cards and "
         "gift wrap."},
    };

    for (const DemoBusiness& d : demo) {
        Business b(nextId(businesses), d.owner, d.email, d.name, d.category, d.city, d.phone);
        b.updateProfile(d.name, d.category, d.city, d.phone, d.about);
        b.setPassword(demoPassword);
        businesses.push_back(b);
    }

    struct DemoProduct {
        int businessId;
        const char *name, *category, *about;
        double price;
        int quantity;
    };
    const DemoProduct items[] = {
        {1, "Ragi Jaggery Cookies (250 g)", "Food & Beverages",
         "Crisp finger millet cookies sweetened with jaggery. Baked fresh every Tuesday and Friday.", 180, 40},
        {1, "Foxtail Millet Upma Mix (500 g)", "Food & Beverages",
         "Roasted foxtail millet with curry leaves and mustard. Add hot water and cook for five minutes.", 220, 25},
        {1, "Jowar Flakes (400 g)", "Food & Beverages",
         "Lightly toasted sorghum flakes for breakfast. Next batch expected next week.", 160, 0},
        {2, "Handwoven Cotton Saree", "Clothing & Textiles",
         "Six yards of soft cotton with a contrast border. Natural indigo dye, 47 inch width.", 2450, 6},
        {2, "Cotton Stole, Checked Weave", "Clothing & Textiles",
         "Lightweight everyday stole, 2 m long. Available in rust, indigo and olive.", 650, 18},
        {2, "Table Runner (6 seater)", "Home & Living",
         "Handloom table runner, 72 by 14 inches. Machine washable on a gentle cycle.", 890, 9},
        {3, "65W Laptop Charger, Refurbished", "Electronics",
         "Tested original-brand charger with a new cable. Works with most Dell and HP laptops.", 950, 12},
        {3, "Phone Screen Replacement", "Services",
         "Screen replacement for most Android phones, done the same day. Price shown is labour only.", 1200, 30},
        {4, "Handmade Paper Notebook (A5)", "Stationery & Books",
         "96 unruled pages of cotton rag paper, thread bound with a soft cover.", 240, 60},
        {4, "Seed Paper Greeting Cards (pack of 5)", "Stationery & Books",
         "Plantable cards with tulsi and marigold seeds pressed into the paper.", 90, 120},
    };
    for (const DemoProduct& p : items) {
        products.emplace_back(nextId(products), p.businessId, p.name, p.category, p.about, p.price, p.quantity);
    }

    Customer ravi(1, "Ravi Menon", "ravi@example.com", "Kochi");
    ravi.setPassword(demoPassword);
    customers.push_back(ravi);

    enquiries.emplace_back(1, 4, "Handwoven Cotton Saree", 2, 1, "Ravi Menon", "ravi@example.com",
                           "Is this saree available in a maroon border? I would like two pieces for a "
                           "wedding next month.");

    files.saveBusinesses(businesses);
    files.saveCustomers(customers);
    files.saveProducts(products);
    files.saveEnquiries(enquiries);
    std::cout << "Demo data added. Log in with any demo email and the password " << demoPassword << "\n";
}
