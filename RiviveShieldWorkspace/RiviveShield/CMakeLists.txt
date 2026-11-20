add_executable(RiviveShield WIN32 "RiviveShield.cpp" "RiviveShield.h")

find_package(wxWidgets CONFIG REQUIRED)
target_link_libraries(RiviveShield PRIVATE wx::core wx::base)

find_package(llhttp CONFIG REQUIRED)
target_link_libraries(RiviveShield PRIVATE llhttp::llhttp_static)

find_package(PcapPlusPlus CONFIG REQUIRED)
target_link_libraries(RiviveShield PRIVATE PcapPlusPlus::Pcap++)