// Enquiry.cpp
#include "Enquiry.h"
#include "Utils.h"

Enquiry::Enquiry(int id, int productId, const std::string& productName, int businessId,
                 int customerId, const std::string& customerName,
                 const std::string& customerEmail, const std::string& message)
    : id(id), productId(productId), productName(productName), businessId(businessId),
      customerId(customerId), customerName(customerName), customerEmail(customerEmail),
      message(message), createdAt(utils::currentDateTime()) {}

std::string Enquiry::toJson(const std::string& businessName) const {
    return "{\"id\":" + std::to_string(id) +
           ",\"productId\":" + std::to_string(productId) +
           ",\"productName\":" + utils::jsonString(productName) +
           ",\"businessId\":" + std::to_string(businessId) +
           ",\"businessName\":" + utils::jsonString(businessName) +
           ",\"customerName\":" + utils::jsonString(customerName) +
           ",\"customerEmail\":" + utils::jsonString(customerEmail) +
           ",\"message\":" + utils::jsonString(message) +
           ",\"createdAt\":" + utils::jsonString(createdAt) + "}";
}

// File format (enquiries.txt), one enquiry per line:
// id|productId|productName|businessId|customerId|customerName|customerEmail|message|createdAt
std::string Enquiry::serialize() const {
    return utils::joinRecord({std::to_string(id), std::to_string(productId), productName,
                              std::to_string(businessId), std::to_string(customerId),
                              customerName, customerEmail, message, createdAt});
}

bool Enquiry::fromRecord(const std::string& line, Enquiry& out) {
    std::vector<std::string> f = utils::splitRecord(line);
    if (f.size() != 9) return false;
    try {
        out.id = std::stoi(f[0]);
        out.productId = std::stoi(f[1]);
        out.businessId = std::stoi(f[3]);
        out.customerId = std::stoi(f[4]);
    } catch (...) {
        return false;
    }
    out.productName = f[2];
    out.customerName = f[5];
    out.customerEmail = f[6];
    out.message = f[7];
    out.createdAt = f[8];
    return true;
}
