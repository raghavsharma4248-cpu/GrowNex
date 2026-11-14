// User.cpp
#include "User.h"
#include "Security.h"
#include "Utils.h"

User::User(int id, const std::string& name, const std::string& email)
    : id(id), name(name), email(email), createdAt(utils::currentDateTime()) {}

void User::setPassword(const std::string& plainPassword) {
    salt = PasswordHasher::makeSalt();
    passwordHash = PasswordHasher::hash(plainPassword, salt);
}

bool User::checkPassword(const std::string& plainPassword) const {
    return PasswordHasher::verify(plainPassword, salt, passwordHash);
}

std::string User::baseJsonFields() const {
    // Note: the password hash and salt are never sent to the browser.
    return "\"id\":" + std::to_string(id) +
           ",\"name\":" + utils::jsonString(name) +
           ",\"email\":" + utils::jsonString(email) +
           ",\"role\":" + utils::jsonString(getRole()) +
           ",\"createdAt\":" + utils::jsonString(createdAt);
}
