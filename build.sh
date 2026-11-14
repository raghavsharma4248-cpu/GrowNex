#!/usr/bin/env bash
# Build GrowNex on Linux or macOS. Run from the GrowNex folder:  ./build.sh
# -pthread is needed because cpp-httplib answers requests on several threads.
set -e
echo "Compiling GrowNex... (the first build can take up to a minute)"
g++ -std=c++17 -O2 -Wall -Ibackend/include -Ibackend/lib backend/src/main.cpp backend/src/Server.cpp backend/src/FileManager.cpp backend/src/User.cpp backend/src/Business.cpp backend/src/Customer.cpp backend/src/Product.cpp backend/src/Enquiry.cpp backend/src/Security.cpp backend/src/Utils.cpp -o grownex -pthread
echo
echo "Build finished. Start the server with:  ./grownex"
echo "First time? Add demo data with:          ./grownex --seed"
