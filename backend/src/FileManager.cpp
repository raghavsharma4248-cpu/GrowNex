// FileManager.cpp
#include "FileManager.h"

#include <filesystem>
#include <fstream>
#include <iostream>

FileManager::FileManager(const std::string& folder) : dataFolder(folder) {}

std::string FileManager::pathFor(const std::string& fileName) const {
    return (std::filesystem::path(dataFolder) / fileName).string();
}

bool FileManager::prepare() const {
    std::error_code error;
    std::filesystem::create_directories(dataFolder, error);
    if (error) {
        std::cerr << "Could not create data folder '" << dataFolder << "': " << error.message() << "\n";
        return false;
    }
    for (const char* name : {"businesses.txt", "customers.txt", "products.txt", "enquiries.txt"}) {
        std::string path = pathFor(name);
        if (!std::filesystem::exists(path)) {
            std::ofstream create(path);   // creates an empty file
            if (!create) {
                std::cerr << "Could not create " << path << "\n";
                return false;
            }
        }
    }
    return true;
}

std::vector<std::string> FileManager::readLines(const std::string& fileName) const {
    std::vector<std::string> lines;
    std::ifstream file(pathFor(fileName));
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (!line.empty()) lines.push_back(line);
    }
    return lines;
}

bool FileManager::writeLines(const std::string& fileName,
                             const std::vector<std::string>& lines) const {
    // std::ios::trunc clears the old contents; we write the full, updated list.
    std::ofstream file(pathFor(fileName), std::ios::trunc);
    if (!file) {
        std::cerr << "Could not write " << pathFor(fileName) << "\n";
        return false;
    }
    for (const std::string& line : lines) file << line << '\n';
    return static_cast<bool>(file);
}
