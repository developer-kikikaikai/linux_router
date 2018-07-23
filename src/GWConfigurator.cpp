#include "GWConfigurator.hpp"

#include <iostream>

int GWConfigurator::setIP(const char *ip, const char * netmask) {
	std::cout << "GWConfiguration, set IP " << ip << ",netmask " << netmask << " to " << _bridgeif << std::endl;
	return 0;
}

int GWConfigurator::unsetIP() {
	std::cout << "GWConfiguration, unset IP" << std::endl;
	return 0;
}
