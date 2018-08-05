cmake_minimum_required(VERSION 2.8)
INCLUDE_DIRECTORIES("/usr/local/include")

add_definitions("-g -O2")

ADD_LIBRARY(eth_landevice SHARED ether_device.c device_json_parser.c)
ADD_LIBRARY(wifi_landevice SHARED wifi_device.c device_json_parser.c)

target_link_libraries( eth_landevice jansson )
target_link_libraries( wifi_landevice jansson )
INSTALL(TARGETS eth_landevice LIBRARY DESTINATION lib)
INSTALL(TARGETS wifi_landevice LIBRARY DESTINATION lib)
