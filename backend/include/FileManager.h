// FileManager.h
// FileManager is the only class that reads or writes the .txt data files.
// Each kind of record has its own file inside the data folder:
//   businesses.txt  customers.txt  products.txt  enquiries.txt
// Every record is one line; fields are separated by '|'.
#ifndef GROWNEX_FILEMANAGER_H
#define GROWNEX_FILEMANAGER_H

#include <string>
#include <vector>

#include "Business.h"
#include "Customer.h"
#include "Enquiry.h"
#include "Product.h"

class FileManager {
private:
    std::string dataFolder;

    std::string pathFor(const std::string& fileName) const;
    std::vector<std::string> readLines(const std::string& fileName) const;
    bool writeLines(const std::string& fileName, const std::vector<std::string>& lines) const;

    // Generic loader: works for any class that has a static fromRecord().
    template <typename T>
    std::vector<T> loadAll(const std::string& fileName) const {
        std::vector<T> items;
        for (const std::string& line : readLines(fileName)) {
            T item;
            if (T::fromRecord(line, item)) items.push_back(item);
        }
        return items;
    }

    // Generic saver: works for any class that has serialize().
    template <typename T>
    bool saveAll(const std::string& fileName, const std::vector<T>& items) const {
        std::vector<std::string> lines;
        for (const T& item : items) lines.push_back(item.serialize());
        return writeLines(fileName, lines);
    }

public:
    explicit FileManager(const std::string& folder);

    // Creates the data folder and empty files if they do not exist yet.
    bool prepare() const;

    std::vector<Business> loadBusinesses() const { return loadAll<Business>("businesses.txt"); }
    std::vector<Customer> loadCustomers() const { return loadAll<Customer>("customers.txt"); }
    std::vector<Product> loadProducts() const { return loadAll<Product>("products.txt"); }
    std::vector<Enquiry> loadEnquiries() const { return loadAll<Enquiry>("enquiries.txt"); }

    bool saveBusinesses(const std::vector<Business>& v) const { return saveAll("businesses.txt", v); }
    bool saveCustomers(const std::vector<Customer>& v) const { return saveAll("customers.txt", v); }
    bool saveProducts(const std::vector<Product>& v) const { return saveAll("products.txt", v); }
    bool saveEnquiries(const std::vector<Enquiry>& v) const { return saveAll("enquiries.txt", v); }
};

#endif
