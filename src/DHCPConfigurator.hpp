#ifndef DHCP_CONFIGURE_HPP
#define DHCP_CONFIGURE_HPP

class DHCPConfigurator {
private:
	const char * _lanif;
	const char * _dhcp_conf;
	int _dhcpd_pid;
	/*for dhcp conf method*/
	void _add_conf(std::string &srcstr, const char * key, const char * val, const char * last);
	void _add_conf_option(std::string &srcstr, const char * key, const char *val1, const char * val2);
	const char * _get_subnet(const char *gw, const char *netmask);
	void _write_dhcp_conf(const char * area_start, const char * area_end, const char *gw, const char *netmask, const char * leasetime);
	void _start_dhcpd(const char * interface);
public:
	DHCPConfigurator(const char * lanif);
	~DHCPConfigurator();
	
	int start(const char * area_start, const char * area_end, const char *gw, const char *netmask, const char * leasetime);
	int stop();
};
#endif
