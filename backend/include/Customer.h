// Customer.h
// A Customer is a User who browses businesses and sends product enquiries.
#ifndef GROWNEX_CUSTOMER_H
#define GROWNEX_CUSTOMER_H

#include "User.h"

class Customer : public User {
private:
    std::string city;

public:
    Customer() = default;
    Customer(int id, const std::string& name, const std::string& email, const std::string& city);

    const std::string& getCity() const { return city; }

    // Overrides of the virtual functions declared in User.
    std::string getRole() const override { return "customer"; }
    std::string dashboardPage() const override { return "/customer-dashboard.html"; }
    std::string toJson() const override;
    std::string serialize() const override;

    // Rebuilds a Customer from one line of customers.txt.
    static bool fromRecord(const std::string& line, Customer& out);
};

#endif
