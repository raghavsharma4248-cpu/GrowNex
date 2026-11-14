// Business.h
// A Business is a User (the business owner's account) plus the public
// business profile that customers see.
#ifndef GROWNEX_BUSINESS_H
#define GROWNEX_BUSINESS_H

#include "User.h"

class Business : public User {
private:
    std::string businessName;
    std::string category;
    std::string city;
    std::string phone;
    std::string description;

public:
    Business() = default;
    Business(int id, const std::string& ownerName, const std::string& email,
             const std::string& businessName, const std::string& category,
             const std::string& city, const std::string& phone);

    const std::string& getBusinessName() const { return businessName; }
    const std::string& getCategory() const { return category; }
    const std::string& getCity() const { return city; }
    const std::string& getPhone() const { return phone; }
    const std::string& getDescription() const { return description; }

    void updateProfile(const std::string& newBusinessName, const std::string& newCategory,
                       const std::string& newCity, const std::string& newPhone,
                       const std::string& newDescription);

    // Overrides of the virtual functions declared in User.
    std::string getRole() const override { return "business"; }
    std::string dashboardPage() const override { return "/owner-dashboard.html"; }
    std::string toJson() const override;
    std::string serialize() const override;

    // Public profile for customers: no login email, plus a product count.
    std::string publicJson(int productCount) const;

    // Rebuilds a Business from one line of businesses.txt.
    static bool fromRecord(const std::string& line, Business& out);
};

#endif
