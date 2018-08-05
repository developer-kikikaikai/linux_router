cmake_minimum_required(VERSION 2.8)

add_custom_target(create_setting ALL COMMAND "sed" "-e" "\"s@PREFIX@${CMAKE_INSTALL_PREFIX}@g\"" "setting.json.ini" ">" "setting.json")

#install
INSTALL(FILES lan_interface.conf DESTINATION conf PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
INSTALL(FILES setting.json DESTINATION conf PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
