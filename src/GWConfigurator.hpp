#ifndef GW_CONFIGURE_HPP
#define GW_CONFIGURE_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>

class GWConfigurator {
private:
	const char * _bridgeif;
	char _gwif[IFNAMSIZ];
	char _gwip[16 + 1];

	/*for up bridge*/
	void _add_bridge(const char *ip, const char * netmask);
	void _set_nat();
	void _del_bridge();

	/*for set_ip*/
	void _get_gw_if();
	int _is_gw_if();
	void _get_gw_ip();

	/*common*/
	void _call_cmd(const char ** cmd);
	void _ifr_ip(struct ifreq * ifr);
	int _ifr_setaddr(const char * ip, int fd, struct ifreq * ifr, int addrflag);
	int _get_ifsock();
public:
	GWConfigurator(const char *bridgeif) {
		if(bridgeif==NULL) {
			fputs("bridgeif is NULL", stderr);
			exit(-1);
		}
		_bridgeif = bridgeif;
	}

	int set_ip(const char * ip, const char * netmask);
	int unset_ip();
	void add_device(const char *name);
	void del_device(const char *name);
};
#endif
