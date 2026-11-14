@echo off
REM Build GrowNex on Windows (MinGW-w64 g++ from MSYS2).
REM Run this from the GrowNex folder:  build.bat
REM -lws2_32 links the Windows networking library that cpp-httplib needs.
REM -D_WIN32_WINNT=0x0A00 tells cpp-httplib we are on Windows 10 or newer.

echo Compiling GrowNex... (the first build can take up to a minute)
g++ -std=c++17 -O2 -Wall -D_WIN32_WINNT=0x0A00 -Ibackend/include -Ibackend/lib backend/src/main.cpp backend/src/Server.cpp backend/src/FileManager.cpp backend/src/User.cpp backend/src/Business.cpp backend/src/Customer.cpp backend/src/Product.cpp backend/src/Enquiry.cpp backend/src/Security.cpp backend/src/Utils.cpp -o grownex.exe -lws2_32
if errorlevel 1 (
    echo.
    echo Build failed. Read the first error message above.
    exit /b 1
)
echo.
echo Build finished. Start the server with:  grownex.exe
echo First time? Add demo data with:          grownex.exe --seed
