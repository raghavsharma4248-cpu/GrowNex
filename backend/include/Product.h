// Product.h
// A product listed by a business. The quantity is shown to customers but is
// never reduced automatically (GrowNex has no ordering system).
#ifndef GROWNEX_PRODUCT_H
#define GROWNEX_PRODUCT_H

#include <string>

class Product {
private:
    int id = 0;
    int businessId = 0;
    std::string name;
    std::string category;
    std::string description;
    double price = 0.0;
    int quantity = 0;
    std::string updatedAt;

public:
    Product() = default;
    Product(int id, int businessId, const std::string& name, const std::string& category,
            const std::string& description, double price, int quantity);

    int getId() const { return id; }
    int getBusinessId() const { return businessId; }
    const std::string& getName() const { return name; }
    const std::string& getCategory() const { return category; }
    const std::string& getDescription() const { return description; }
    double getPrice() const { return price; }
    int getQuantity() const { return quantity; }

    void update(const std::string& newName, const std::string& newCategory,
                const std::string& newDescription, double newPrice, int newQuantity);

    // True when the search word appears in the name, category or description.
    bool matches(const std::string& word) const;

    std::string toJson(const std::string& businessName) const;
    std::string serialize() const;
    static bool fromRecord(const std::string& line, Product& out);
};

#endif
