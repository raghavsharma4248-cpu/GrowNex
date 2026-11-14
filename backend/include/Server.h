// Server.h
// Server wraps cpp-httplib. It:
//   1. serves the HTML/CSS/JS files from the frontend folder, and
//   2. answers the /api/... requests sent by the browser with JSON.
//
// All data is kept in memory in vectors and written to the .txt files through
// FileManager every time something changes, so nothing is lost on restart.
#ifndef GROWNEX_SERVER_H
#define GROWNEX_SERVER_H

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "FileManager.h"
#include "httplib.h"

class Server {
private:
    struct Session {
        std::string role;   // "business" or "customer"
        int userId = 0;
    };

    httplib::Server http;
    FileManager files;
    std::string frontendFolder;

    std::vector<Business> businesses;
    std::vector<Customer> customers;
    std::vector<Product> products;
    std::vector<Enquiry> enquiries;

    std::map<std::string, Session> sessions;   // session token -> logged-in user

    // cpp-httplib answers requests on several threads at once, so every
    // handler locks this mutex before reading or changing the vectors.
    std::mutex dataMutex;

    // ---- setup ----
    void setupRoutes();

    // ---- helpers ----
    static void sendJson(httplib::Response& res, int status, const std::string& body);
    static void sendError(httplib::Response& res, int status, const std::string& message);
    static std::string field(const httplib::Request& req, const std::string& name);
    static std::string readCookie(const httplib::Request& req, const std::string& name);

    const Session* currentSession(const httplib::Request& req) const;
    User* findUserByEmail(const std::string& email);
    User* findUser(const Session& session);
    Business* findBusiness(int id);
    Product* findProduct(int id);
    std::string businessNameOf(int businessId);
    int productCountOf(int businessId) const;

    template <typename T>
    static int nextId(const std::vector<T>& items) {
        int maxId = 0;
        for (const T& item : items)
            if (item.getId() > maxId) maxId = item.getId();
        return maxId + 1;
    }

    // ---- request handlers ----
    void handleRegister(const httplib::Request& req, httplib::Response& res);
    void handleLogin(const httplib::Request& req, httplib::Response& res);
    void handleLogout(const httplib::Request& req, httplib::Response& res);
    void handleMe(const httplib::Request& req, httplib::Response& res);

    void handleCategories(const httplib::Request& req, httplib::Response& res);
    void handleListBusinesses(const httplib::Request& req, httplib::Response& res);
    void handleGetBusiness(const httplib::Request& req, httplib::Response& res);
    void handleListProducts(const httplib::Request& req, httplib::Response& res);
    void handleGetProduct(const httplib::Request& req, httplib::Response& res);

    void handleUpdateProfile(const httplib::Request& req, httplib::Response& res);
    void handleOwnerProducts(const httplib::Request& req, httplib::Response& res);
    void handleCreateProduct(const httplib::Request& req, httplib::Response& res);
    void handleUpdateProduct(const httplib::Request& req, httplib::Response& res);
    void handleDeleteProduct(const httplib::Request& req, httplib::Response& res);
    void handleOwnerEnquiries(const httplib::Request& req, httplib::Response& res);

    void handleCreateEnquiry(const httplib::Request& req, httplib::Response& res);
    void handleCustomerEnquiries(const httplib::Request& req, httplib::Response& res);

public:
    Server(const std::string& frontendFolder, const std::string& dataFolder);

    // Loads the .txt files. Returns false if the folders are missing.
    bool initialise();

    // Adds three demo businesses with products (only if there are none yet).
    void seedDemoData();

    // Starts listening. Blocks until the program is stopped (Ctrl + C).
    bool run(const std::string& host, int port);
};

#endif
