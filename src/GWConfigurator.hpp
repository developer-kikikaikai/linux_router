#ifndef GW_CONFIGURE_HPP
#define GW_CONFIGURE_HPP

#include <stdio.h>
#include <stdlib.h>

class GWConfigurator {
private:
	const char * _bridgeif;
public:
	GWConfigurator(const char *bridgeif) {
		if(bridgeif==NULL) {
			fputs("bridgeif is NULL", stderr);
			exit(-1);
		}
		_bridgeif = bridgeif;
	}
	int setIP(const char * ip, const char * netmask);
	int unsetIP();
};
#endif
