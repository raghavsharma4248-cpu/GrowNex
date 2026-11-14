// main.cpp - starting point of the GrowNex program.
//
// Usage (run from the GrowNex project folder):
//   grownex            start the server on http://localhost:8080
//   grownex --seed     add demo businesses, products and a demo customer first
//   grownex --port 9090
#include <iostream>
#include <string>

#include "Server.h"

int main(int argc, char* argv[]) {
    bool seed = false;
    int port = 8080;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--seed") {
            seed = true;
        } else if (arg == "--port" && i + 1 < argc) {
            try {
                port = std::stoi(argv[++i]);
            } catch (...) {
                std::cerr << "The port must be a number, for example --port 8080\n";
                return 1;
            }
        } else {
            std::cerr << "Unknown option: " << arg << "\n"
                      << "Usage: grownex [--seed] [--port 8080]\n";
            return 1;
        }
    }

    // The frontend and data folders are relative to where the program is run.
    Server server("frontend", "data");
    if (!server.initialise()) return 1;
    if (seed) server.seedDemoData();

    // "127.0.0.1" means only this computer can open the site (localhost only).
    if (!server.run("127.0.0.1", port)) {
        std::cerr << "Could not start on port " << port
                  << ". Another program may be using it. Try: grownex --port 9090\n";
        return 1;
    }
    return 0;
}
