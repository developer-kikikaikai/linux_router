#ifndef LAN_IP_MANAGER_HPP
#define LAN_IP_MANAGER_HPP

#include <iostream>
#include <jansson.h>
#include "DHCPConfigurator.hpp"
#include "GWConfigurator.hpp"

class LANIPManager {
private:
	DHCPConfigurator * _dhcp;
	GWConfigurator * _gw;
	const json_t * _lan_info;

	int _set_gw(void);
	int _set_dhcp(void);
public:
	LANIPManager(const json_t * lan_info);
	~LANIPManager() {
		std::cout << "LANIPManager"  << std::endl;
		if(_dhcp) delete _dhcp;
		if(_gw) delete _gw;
	}

	int set(void);
	int unset(void);
};

#endif
