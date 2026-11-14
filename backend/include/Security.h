// Security.h
// SHA-256 hashing and password hashing.
//
// Passwords are never stored as plain text. For each user we create a random
// "salt", then hash (salt + password) and repeat the hash 10,000 times.
// The salt means two users with the same password get different hashes, and
// the repetition makes guessing passwords by brute force much slower.
#ifndef GROWNEX_SECURITY_H
#define GROWNEX_SECURITY_H

#include <string>

class Sha256 {
public:
    // Returns the SHA-256 digest of the input as 64 hexadecimal characters.
    static std::string hash(const std::string& input);
};

class PasswordHasher {
public:
    static const int ITERATIONS = 10000;

    static std::string makeSalt();
    static std::string hash(const std::string& password, const std::string& salt);
    static bool verify(const std::string& password, const std::string& salt,
                       const std::string& expectedHash);
};

#endif
