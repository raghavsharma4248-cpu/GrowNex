// Customer.cpp
#include "Customer.h"
#include "Utils.h"

Customer::Customer(int id, const std::string& name, const std::string& email,
                   const std::string& city)
    : User(id, name, email), city(city) {}

std::string Customer::toJson() const {
    return "{" + baseJsonFields() + ",\"city\":" + utils::jsonString(city) + "}";
}

// File format (customers.txt), one customer per line:
// id|name|email|passwordHash|salt|city|createdAt
std::string Customer::serialize() const {
    return utils::joinRecord({std::to_string(id), name, email, passwordHash, salt, city, createdAt});
}

bool Customer::fromRecord(const std::string& line, Customer& out) {
    std::vector<std::string> f = utils::splitRecord(line);
    if (f.size() != 7) return false;
    try {
        out.id = std::stoi(f[0]);
    } catch (...) {
        return false;
    }
    out.name = f[1];
    out.email = f[2];
    out.passwordHash = f[3];
    out.salt = f[4];
    out.city = f[5];
    out.createdAt = f[6];
    return true;
}
