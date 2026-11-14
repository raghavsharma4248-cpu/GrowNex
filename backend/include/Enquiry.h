// Enquiry.h
// A question a customer sends about a product. Business owners can only view
// enquiries; they reply to the customer directly by email.
// The product name is copied into the enquiry so the enquiry still makes sense
// even if the product is later deleted.
#ifndef GROWNEX_ENQUIRY_H
#define GROWNEX_ENQUIRY_H

#include <string>

class Enquiry {
private:
    int id = 0;
    int productId = 0;
    std::string productName;
    int businessId = 0;
    int customerId = 0;
    std::string customerName;
    std::string customerEmail;
    std::string message;
    std::string createdAt;

public:
    Enquiry() = default;
    Enquiry(int id, int productId, const std::string& productName, int businessId,
            int customerId, const std::string& customerName, const std::string& customerEmail,
            const std::string& message);

    int getId() const { return id; }
    int getProductId() const { return productId; }
    int getBusinessId() const { return businessId; }
    int getCustomerId() const { return customerId; }

    // businessName is passed in because it lives in the Business object.
    std::string toJson(const std::string& businessName) const;
    std::string serialize() const;
    static bool fromRecord(const std::string& line, Enquiry& out);
};

#endif
