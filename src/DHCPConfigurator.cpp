//fork
#include <unistd.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <signal.h>
#include "DHCPConfigurator.hpp"

const char * DHCPConfigurator::_get_subnet(const char *gw, const char *netmask) {

	/*get gw and netmask bytearray*/
	uint32_t gw_binary;
	uint32_t netmask_binary;
	inet_pton(AF_INET, gw, &gw_binary);
	inet_pton(AF_INET, netmask, &netmask_binary);

	/*get subnet*/
	struct in_addr sin_addr;
	sin_addr.s_addr = gw_binary & netmask_binary;
	/*change to string*/
	return inet_ntoa(sin_addr);
}

DHCPConfigurator::DHCPConfigurator(const char * lanif) {
	_lanif = lanif;
	_dhcpd_pid = 0;
}

int DHCPConfigurator::start(const char * area_start, const char * area_end, const char *gw, const char *netmask, const char * leasetime) {
	std::cout << "DHCPConfigurator::start, area_start is " << area_start<< ", end is " << area_end << std::endl;

	_write_dhcp_conf(area_start, area_end, gw, netmask, leasetime);;

	_dhcpd_pid = _start_dhcpd(_lanif);
        return 0;
}

int DHCPConfigurator::stop(void) {
	std::cout << "DHCPConfigurator::stop" << std::endl;
	if(_dhcpd_pid != 0) {
		/*stop dhcpd*/
		kill(_dhcpd_pid, SIGTERM);
		waitpid(_dhcpd_pid, NULL, 0);
		_dhcpd_pid=0;
	}
	return 0;
}
