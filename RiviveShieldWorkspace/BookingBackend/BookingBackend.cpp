#include <iostream>
#include <sqlite3.h>
#include <httplib.h>

int main()
{
    std::cout << "SQLite version: " << sqlite3_libversion() << '\n';

    httplib::Server server;
    std::cout << "cpp-httplib server object created successfully.\n";

    return 0;
}