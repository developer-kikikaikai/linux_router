#ifndef DHCP_CONFIGURE_IMPLE_HPP
#define DHCP_CONFIGURE_IMPLE_HPP

#include "DHCPConfigurator.hpp"

class DHCPConfiguratorImple : public DHCPConfigurator {
private:
	const char * _dhcp_conf;
	/*for dhcp conf method*/
	void _add_conf(std::string &srcstr, const char * key, const char * val, const char * last);
	void _add_conf_option(std::string &srcstr, const char * key, const char *val1, const char * val2);
	void _write_dhcp_conf(const char * area_start, const char * area_end, const char *gw, const char *netmask, const char * leasetime);
	int _start_dhcpd(const char * interface);
public:
	DHCPConfiguratorImple(const char * lanif);
	~DHCPConfiguratorImple() {stop();};
};
#endif
