add_executable(BookingBackend "BookingBackend.cpp" "BookingBackend.h")

find_package(unofficial-sqlite3 CONFIG REQUIRED)
target_link_libraries(BookingBackend PRIVATE unofficial::sqlite3::sqlite3)

find_package(httplib CONFIG REQUIRED)
target_link_libraries(BookingBackend PRIVATE httplib::httplib)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(BookingBackend PRIVATE nlohmann_json::nlohmann_json)
