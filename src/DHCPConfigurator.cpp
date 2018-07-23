#include <iostream>
#include "DHCPConfigurator.hpp"

int DHCPConfigurator::start(const char * area_start, const char * area_end) {
	std::cout << "DHCPConfigurator::start, area_start is " << area_start<< ", end is " << area_end << std::endl;
	return 0;
}

int DHCPConfigurator::stop(void) {
	std::cout << "DHCPConfigurator::stop" << std::endl;
	return 0;
}
