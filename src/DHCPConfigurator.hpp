#ifndef DHCP_CONFIGURE_HPP
#define DHCP_CONFIGURE_HPP

class DHCPConfigurator {
private:
	const char * _lanif;
	int _dhcpd_pid;

public:
	/*for subclass*/
	const char * _get_subnet(const char *gw, const char *netmask);
	void _add_conf(std::string &srcstr, char const* separater, std::initializer_list<char const*> options);
	virtual void _write_dhcp_conf(const char * area_start, const char * area_end, const char *gw, const char *netmask, const char * leasetime) = 0;
	virtual int _start_dhcpd(const char * interface) = 0;

	/*for subclass*/
	DHCPConfigurator(const char * lanif);
	virtual ~DHCPConfigurator(){};

	int start(const char * area_start, const char * area_end, const char *gw, const char *netmask, const char * leasetime);
	int stop();
};
#endif
