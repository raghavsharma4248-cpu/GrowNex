// Utils.h
// Small helper functions used by every other class:
//   - text-file escaping (so a '|' or a new line inside a description
//     cannot break our one-record-per-line file format)
//   - JSON string escaping (so the browser always receives valid JSON)
//   - input validation rules shared by registration, products and enquiries
#ifndef GROWNEX_UTILS_H
#define GROWNEX_UTILS_H

#include <string>
#include <vector>

namespace utils {

// ---------- text file helpers ----------

// Turns "Tea | Coffee" into "Tea \| Coffee" so it can be stored in a
// pipe-separated line. New lines become "\n" (two characters).
std::string escapeField(const std::string& value);

// Splits one stored line back into its fields, undoing escapeField().
std::vector<std::string> splitRecord(const std::string& line);

// Joins already-raw values into one stored line (escapes each value).
std::string joinRecord(const std::vector<std::string>& fields);

// ---------- JSON helpers ----------

// Returns the value wrapped in double quotes with special characters escaped.
std::string jsonString(const std::string& value);

// ---------- general helpers ----------

std::string trim(const std::string& value);
std::string toLower(const std::string& value);
bool containsIgnoreCase(const std::string& text, const std::string& word);
std::string formatPrice(double price);          // 1250.5 -> "1250.50"
std::string currentDateTime();                  // "2026-11-13 15:42"
std::string randomHex(std::size_t byteCount);   // random token / salt

// The fixed list of categories used for businesses and products.
const std::vector<std::string>& categories();

} // namespace utils

// Validator groups every input rule in one place. Each function returns an
// empty string when the value is valid, otherwise a message for the user.
class Validator {
public:
    static std::string personName(const std::string& value);
    static std::string email(const std::string& value);
    static std::string password(const std::string& value);
    static std::string businessName(const std::string& value);
    static std::string city(const std::string& value);
    static std::string phone(const std::string& value);
    static std::string category(const std::string& value);
    static std::string description(const std::string& value, std::size_t maxLength);
    static std::string productName(const std::string& value);
    static std::string price(const std::string& value, double& parsed);
    static std::string quantity(const std::string& value, int& parsed);
    static std::string enquiryMessage(const std::string& value);
};

#endif
