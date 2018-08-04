cmake_minimum_required(VERSION 2.8)
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_VERBOSE_MAKEFILE 1)

link_directories(/usr/local/lib)
add_definitions("-std=c++11")
set(SRCS DHCPConfigurator.cpp DHCPConfigurator_dhcpd.cpp GWConfigurator.cpp LANIPManager.cpp LANManager.cpp main.cpp JsonParser.cpp)
add_executable(lan_router ${SRCS})
target_link_libraries(lan_router lower_layer_director dputil jansson)
