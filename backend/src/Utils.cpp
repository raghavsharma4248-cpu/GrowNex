// Utils.cpp - implementation of the helper functions declared in Utils.h
#include "Utils.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <random>

namespace utils {

std::string escapeField(const std::string& value) {
    std::string out;
    for (char c : value) {
        if (c == '\\') out += "\\\\";
        else if (c == '|') out += "\\|";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') continue;          // drop Windows carriage returns
        else out += c;
    }
    return out;
}

std::vector<std::string> splitRecord(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    for (std::size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '\\' && i + 1 < line.size()) {
            char next = line[++i];
            current += (next == 'n') ? '\n' : next;
        } else if (c == '|') {
            fields.push_back(current);
            current.clear();
        } else if (c != '\r') {
            current += c;
        }
    }
    fields.push_back(current);
    return fields;
}

std::string joinRecord(const std::vector<std::string>& fields) {
    std::string line;
    for (std::size_t i = 0; i < fields.size(); ++i) {
        if (i > 0) line += '|';
        line += escapeField(fields[i]);
    }
    return line;
}

std::string jsonString(const std::string& value) {
    std::string out = "\"";
    for (unsigned char c : value) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += static_cast<char>(c);
                }
        }
    }
    out += "\"";
    return out;
}

std::string trim(const std::string& value) {
    std::size_t start = 0, end = value.size();
    while (start < end && std::isspace(static_cast<unsigned char>(value[start]))) ++start;
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) --end;
    return value.substr(start, end - start);
}

std::string toLower(const std::string& value) {
    std::string out = value;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

bool containsIgnoreCase(const std::string& text, const std::string& word) {
    if (word.empty()) return true;
    return toLower(text).find(toLower(word)) != std::string::npos;
}

std::string formatPrice(double price) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.2f", price);
    return buf;
}

std::string currentDateTime() {
    std::time_t now = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &local);
    return buf;
}

std::string randomHex(std::size_t byteCount) {
    // The generator is seeded once from the operating system's random device
    // mixed with the current time.
    static std::mt19937_64 engine = [] {
        std::random_device device;
        auto time = static_cast<unsigned>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count());
        std::seed_seq seed{device(), device(), device(), device(), time};
        return std::mt19937_64(seed);
    }();
    static const char* digits = "0123456789abcdef";
    std::uniform_int_distribution<int> byte(0, 255);
    std::string out;
    for (std::size_t i = 0; i < byteCount; ++i) {
        int b = byte(engine);
        out += digits[b >> 4];
        out += digits[b & 15];
    }
    return out;
}

const std::vector<std::string>& categories() {
    static const std::vector<std::string> list = {
        "Food & Beverages", "Clothing & Textiles", "Handicrafts", "Electronics",
        "Health & Beauty",  "Home & Living",       "Agriculture", "Services",
        "Stationery & Books", "Other"};
    return list;
}

} // namespace utils

// ---------------- Validator ----------------

namespace {
bool lengthBetween(const std::string& v, std::size_t min, std::size_t max) {
    return v.size() >= min && v.size() <= max;
}
} // namespace

std::string Validator::personName(const std::string& value) {
    if (!lengthBetween(value, 2, 60)) return "Name must be 2 to 60 characters.";
    for (unsigned char c : value) {
        if (!(std::isalpha(c) || c == ' ' || c == '.' || c == '\'' || c == '-'))
            return "Name can contain only letters, spaces, dots, hyphens and apostrophes.";
    }
    return "";
}

std::string Validator::email(const std::string& value) {
    if (value.empty() || value.size() > 100) return "Enter a valid email address.";
    std::size_t at = value.find('@');
    if (at == std::string::npos || at == 0 || value.find('@', at + 1) != std::string::npos)
        return "Enter a valid email address, for example name@example.com.";
    std::string domain = value.substr(at + 1);
    std::size_t dot = domain.rfind('.');
    if (dot == std::string::npos || dot == 0 || dot == domain.size() - 1)
        return "Enter a valid email address, for example name@example.com.";
    for (unsigned char c : value) {
        if (std::isspace(c) || c == '|' || c == '\\')
            return "Email address cannot contain spaces or special separators.";
    }
    return "";
}

std::string Validator::password(const std::string& value) {
    if (!lengthBetween(value, 8, 64)) return "Password must be 8 to 64 characters.";
    bool hasLetter = false, hasDigit = false;
    for (unsigned char c : value) {
        if (std::isalpha(c)) hasLetter = true;
        if (std::isdigit(c)) hasDigit = true;
    }
    if (!hasLetter || !hasDigit) return "Password must contain at least one letter and one number.";
    return "";
}

std::string Validator::businessName(const std::string& value) {
    if (!lengthBetween(value, 2, 80)) return "Business name must be 2 to 80 characters.";
    return "";
}

std::string Validator::city(const std::string& value) {
    if (!lengthBetween(value, 2, 50)) return "City must be 2 to 50 characters.";
    for (unsigned char c : value) {
        if (!(std::isalpha(c) || c == ' ' || c == '-' || c == '.'))
            return "City can contain only letters, spaces, dots and hyphens.";
    }
    return "";
}

std::string Validator::phone(const std::string& value) {
    std::string digits;
    for (std::size_t i = 0; i < value.size(); ++i) {
        unsigned char c = value[i];
        if (std::isdigit(c)) digits += static_cast<char>(c);
        else if (c == ' ' || c == '-' || (c == '+' && i == 0)) continue;
        else return "Phone number can contain only digits, spaces, hyphens and a leading +.";
    }
    if (digits.size() < 10 || digits.size() > 13) return "Phone number must have 10 to 13 digits.";
    return "";
}

std::string Validator::category(const std::string& value) {
    for (const auto& c : utils::categories())
        if (c == value) return "";
    return "Choose a category from the list.";
}

std::string Validator::description(const std::string& value, std::size_t maxLength) {
    if (value.size() > maxLength)
        return "Description can be at most " + std::to_string(maxLength) + " characters.";
    return "";
}

std::string Validator::productName(const std::string& value) {
    if (!lengthBetween(value, 2, 80)) return "Product name must be 2 to 80 characters.";
    return "";
}

std::string Validator::price(const std::string& value, double& parsed) {
    if (value.empty()) return "Enter a price.";
    int dots = 0, decimals = 0;
    for (unsigned char c : value) {
        if (c == '.') { if (++dots > 1) return "Price must be a number like 250 or 99.50."; }
        else if (!std::isdigit(c)) return "Price must be a number like 250 or 99.50.";
        else if (dots == 1) ++decimals;
    }
    if (decimals > 2) return "Price can have at most two decimal places.";
    try {
        parsed = std::stod(value);
    } catch (...) {
        return "Price must be a number like 250 or 99.50.";
    }
    if (parsed <= 0 || parsed > 10000000) return "Price must be more than 0 and at most 1,00,00,000.";
    return "";
}

std::string Validator::quantity(const std::string& value, int& parsed) {
    if (value.empty() || value.size() > 7) return "Quantity must be a whole number from 0 to 1,000,000.";
    for (unsigned char c : value)
        if (!std::isdigit(c)) return "Quantity must be a whole number from 0 to 1,000,000.";
    parsed = std::stoi(value);
    if (parsed > 1000000) return "Quantity must be a whole number from 0 to 1,000,000.";
    return "";
}

std::string Validator::enquiryMessage(const std::string& value) {
    if (!lengthBetween(value, 10, 500)) return "Message must be 10 to 500 characters.";
    return "";
}
