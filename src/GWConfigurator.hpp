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
	void _addBridge(const char *ip, const char * netmask);
	void _delBridge();

	/*for setIP*/
	void _getGWIF();
	int _isGWIF();
	void _getGWIP();

	/*common*/
	void _callCmd(const char ** cmd);
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
	void addDevice(const char *name);
	void delDevice(const char *name);
};
#endif
