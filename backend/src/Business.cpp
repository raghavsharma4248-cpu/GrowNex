// Business.cpp
#include "Business.h"
#include "Utils.h"

Business::Business(int id, const std::string& ownerName, const std::string& email,
                   const std::string& businessName, const std::string& category,
                   const std::string& city, const std::string& phone)
    : User(id, ownerName, email), businessName(businessName), category(category),
      city(city), phone(phone) {}

void Business::updateProfile(const std::string& newBusinessName, const std::string& newCategory,
                             const std::string& newCity, const std::string& newPhone,
                             const std::string& newDescription) {
    businessName = newBusinessName;
    category = newCategory;
    city = newCity;
    phone = newPhone;
    description = newDescription;
}

std::string Business::toJson() const {
    return "{" + baseJsonFields() +
           ",\"businessName\":" + utils::jsonString(businessName) +
           ",\"category\":" + utils::jsonString(category) +
           ",\"city\":" + utils::jsonString(city) +
           ",\"phone\":" + utils::jsonString(phone) +
           ",\"description\":" + utils::jsonString(description) + "}";
}

std::string Business::publicJson(int productCount) const {
    return "{\"id\":" + std::to_string(id) +
           ",\"ownerName\":" + utils::jsonString(name) +
           ",\"businessName\":" + utils::jsonString(businessName) +
           ",\"category\":" + utils::jsonString(category) +
           ",\"city\":" + utils::jsonString(city) +
           ",\"phone\":" + utils::jsonString(phone) +
           ",\"description\":" + utils::jsonString(description) +
           ",\"memberSince\":" + utils::jsonString(createdAt.substr(0, 10)) +
           ",\"productCount\":" + std::to_string(productCount) + "}";
}

// File format (businesses.txt), one business per line:
// id|ownerName|email|passwordHash|salt|businessName|category|city|phone|description|createdAt
std::string Business::serialize() const {
    return utils::joinRecord({std::to_string(id), name, email, passwordHash, salt,
                              businessName, category, city, phone, description, createdAt});
}

bool Business::fromRecord(const std::string& line, Business& out) {
    std::vector<std::string> f = utils::splitRecord(line);
    if (f.size() != 11) return false;
    try {
        out.id = std::stoi(f[0]);
    } catch (...) {
        return false;
    }
    out.name = f[1];
    out.email = f[2];
    out.passwordHash = f[3];
    out.salt = f[4];
    out.businessName = f[5];
    out.category = f[6];
    out.city = f[7];
    out.phone = f[8];
    out.description = f[9];
    out.createdAt = f[10];
    return true;
}
