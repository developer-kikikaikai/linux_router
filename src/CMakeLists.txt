cmake_minimum_required(VERSION 2.8)
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_VERBOSE_MAKEFILE 1)

link_directories(${DPPATH})
add_definitions("-std=c++11")

#modify path of GWConfigurator.cpp conf to PREFIX
add_custom_command(OUTPUT  LANManager.cpp COMMAND  "sed" "-e" "\"s@PREFIX@${CMAKE_INSTALL_PREFIX}@g\"" "LANManager_base.cpp" ">" "LANManager.cpp")

#for dhcpd
set(SRCS_BASE DHCPConfigurator.cpp GWConfigurator.cpp LANIPManager.cpp LANManager.cpp main.cpp JsonParser.cpp)
set(SRCS_DHCPD ${SRCS_BASE} DHCPConfigurator_dhcpd.cpp)
#for dnsmasq
set(SRCS_DNSMASQ ${SRCS_BASE} DHCPConfigurator_dnsmasq.cpp)
add_executable(lan_router_dhcpd ${SRCS_DHCPD})
add_executable(lan_router ${SRCS_DNSMASQ})
target_link_libraries(lan_router_dhcpd lower_layer_director dputil jansson)
target_link_libraries(lan_router lower_layer_director dputil jansson)

#install
INSTALL(TARGETS lan_router_dhcpd RUNTIME DESTINATION bin)
INSTALL(TARGETS lan_router RUNTIME DESTINATION bin)
