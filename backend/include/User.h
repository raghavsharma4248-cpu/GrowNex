// User.h
// User is an ABSTRACT BASE CLASS. It holds what every account has in common
// (id, name, email, password hash). Business and Customer INHERIT from it.
//
// OOP concepts shown here:
//   Encapsulation  - data members are protected/private; other code uses
//                    getters and checkPassword() instead of touching them.
//   Inheritance    - class Business : public User, class Customer : public User
//   Polymorphism   - getRole(), dashboardPage(), toJson() and serialize() are
//                    virtual, so a User* calls the right version at run time.
#ifndef GROWNEX_USER_H
#define GROWNEX_USER_H

#include <string>

class User {
protected:
    int id = 0;
    std::string name;
    std::string email;
    std::string passwordHash;
    std::string salt;
    std::string createdAt;

    // Shared JSON fields; derived classes add their own after these.
    std::string baseJsonFields() const;

public:
    User() = default;
    User(int id, const std::string& name, const std::string& email);
    virtual ~User() = default;

    int getId() const { return id; }
    const std::string& getName() const { return name; }
    const std::string& getEmail() const { return email; }
    const std::string& getCreatedAt() const { return createdAt; }

    void setName(const std::string& newName) { name = newName; }

    // Hashes the plain password with a new salt and stores only the hash.
    void setPassword(const std::string& plainPassword);
    bool checkPassword(const std::string& plainPassword) const;

    // ---- polymorphic behaviour (each derived class decides) ----
    virtual std::string getRole() const = 0;        // "business" or "customer"
    virtual std::string dashboardPage() const = 0;  // page to open after login
    virtual std::string toJson() const = 0;         // data sent to the browser
    virtual std::string serialize() const = 0;      // one line for the .txt file
};

#endif
