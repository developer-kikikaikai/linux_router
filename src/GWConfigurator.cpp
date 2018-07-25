#include "GWConfigurator.hpp"
#include <unistd.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int GWConfigurator::setIP(const char *ip, const char * netmask) {
	std::cout << "GWConfiguration, set IP " << ip << ",netmask " << netmask << " to " << _bridgeif << std::endl;
	std::cout << "Get OS default GW information" << std::endl;
	_getGWIP();

	std::cout << "GW, IF=" << _gwif << ", ip=" << _gwip << std::endl;

	return 0;
}

int GWConfigurator::unsetIP() {
	std::cout << "GWConfiguration, unset IP" << std::endl;
	return 0;
}

/****private *****/
void GWConfigurator::_getGWIF(){

	memset(_gwif, 0, sizeof(_gwif));

	/* only support ip command, try ip command
	 * if you want to you route command, please use
	 * route  -n | grep "^0.0.0.0" | awk -F" " '{print $8}'
	*/
	FILE *fp = popen("ip route show default | awk -F' ' '{print $5}'", "r");
	if(!fp) {
		return;
	}

	fgets(_gwif, sizeof(_gwif), fp);
	_gwif[strlen(_gwif)-1]=0;
	pclose(fp);
}

int GWConfigurator::_isGWIF() {
	return (_gwif[0] == 0);
}

void GWConfigurator::_getGWIP() {

	memset(_gwip, 0, sizeof(_gwip));
	/*get gw name*/
	_getGWIF();
	if(_isGWIF()) return;

	int fd;
	/*struct to get interface information*/
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd < 0) return;

	/* IP addr */
	ifr.ifr_addr.sa_family = AF_INET;

	/* access _gwif */
	strncpy(ifr.ifr_name, _gwif, IFNAMSIZ-1);
	/* get address */
	ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);

	snprintf(_gwip, sizeof(_gwip), "%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}
