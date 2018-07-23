#ifndef DHCP_CONFIGURE_HPP
#define DHCP_CONFIGURE_HPP

class DHCPConfigurator {
private:
	const char * _area_start;
	const char * _area_end;
public:
	int start(const char * area_start, const char * area_end);
	int stop(void);
};
#endif
