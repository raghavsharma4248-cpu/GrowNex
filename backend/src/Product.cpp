// Product.cpp
#include "Product.h"
#include "Utils.h"

Product::Product(int id, int businessId, const std::string& name, const std::string& category,
                 const std::string& description, double price, int quantity)
    : id(id), businessId(businessId), name(name), category(category),
      description(description), price(price), quantity(quantity),
      updatedAt(utils::currentDateTime()) {}

void Product::update(const std::string& newName, const std::string& newCategory,
                     const std::string& newDescription, double newPrice, int newQuantity) {
    name = newName;
    category = newCategory;
    description = newDescription;
    price = newPrice;
    quantity = newQuantity;
    updatedAt = utils::currentDateTime();
}

bool Product::matches(const std::string& word) const {
    return utils::containsIgnoreCase(name, word) ||
           utils::containsIgnoreCase(category, word) ||
           utils::containsIgnoreCase(description, word);
}

std::string Product::toJson(const std::string& businessName) const {
    return "{\"id\":" + std::to_string(id) +
           ",\"businessId\":" + std::to_string(businessId) +
           ",\"businessName\":" + utils::jsonString(businessName) +
           ",\"name\":" + utils::jsonString(name) +
           ",\"category\":" + utils::jsonString(category) +
           ",\"description\":" + utils::jsonString(description) +
           ",\"price\":" + utils::formatPrice(price) +
           ",\"quantity\":" + std::to_string(quantity) +
           ",\"updatedAt\":" + utils::jsonString(updatedAt) + "}";
}

// File format (products.txt), one product per line:
// id|businessId|name|category|description|price|quantity|updatedAt
std::string Product::serialize() const {
    return utils::joinRecord({std::to_string(id), std::to_string(businessId), name, category,
                              description, utils::formatPrice(price), std::to_string(quantity),
                              updatedAt});
}

bool Product::fromRecord(const std::string& line, Product& out) {
    std::vector<std::string> f = utils::splitRecord(line);
    if (f.size() != 8) return false;
    try {
        out.id = std::stoi(f[0]);
        out.businessId = std::stoi(f[1]);
        out.price = std::stod(f[5]);
        out.quantity = std::stoi(f[6]);
    } catch (...) {
        return false;
    }
    out.name = f[2];
    out.category = f[3];
    out.description = f[4];
    out.updatedAt = f[7];
    return true;
}
