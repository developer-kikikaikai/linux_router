#include "GWConfigurator.hpp"
#include <unistd.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <spawn.h>
#include <sys/wait.h>

int GWConfigurator::setIP(const char *ip, const char * netmask) {
	std::cout << "GWConfiguration, set IP " << ip << ",netmask " << netmask << " to " << _bridgeif << std::endl;
	std::cout << "Get OS default GW information" << std::endl;
	_getGWIP();

	std::cout << "GW, IF=" << _gwif << ", ip=" << _gwip << std::endl;

	/*setup bridge LAN interface*/
	_addBridge(ip, netmask);
	return 0;
}

int GWConfigurator::unsetIP() {
	std::cout << "GWConfiguration, unset IP" << std::endl;
	_delBridge();
	return 0;
}

void GWConfigurator::addDevice(const char *name) {
	const char *args[] = {"brctl", "addif", _bridgeif, name, NULL};
	_callCmd(args);
	
}

void GWConfigurator::delDevice(const char *name) {
	const char *args[] = {"brctl", "delif", _bridgeif, name, NULL};
	_callCmd(args);
}

/****private *****/
void GWConfigurator::_addBridge(const char *ip, const char * netmask) {
	/*set bridge*/
	const char *args[] = {"brctl", "addbr", _bridgeif, NULL};
	_callCmd(args);

	/*up and set ip*/
	int fd;
	struct ifreq ifr;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd<0) return;

	/* access _gwif */
	strncpy(ifr.ifr_name, _bridgeif, IFNAMSIZ-1);

	/*get status*/
	ioctl(fd, SIOCGIFFLAGS, &ifr);
	/* add up and running */
	ifr.ifr_flags |= (IFF_UP | IFF_RUNNING | IFF_DYNAMIC);
	ioctl(fd, SIOCSIFFLAGS, &ifr);

	/* set ip */
	ifr.ifr_addr.sa_family = AF_INET;
	struct sockaddr_in * s_in = (struct sockaddr_in *)&ifr.ifr_addr;
	s_in->sin_addr.s_addr = inet_addr(ip);
	ioctl(fd, SIOCSIFADDR, &ifr);

	/*set netmask*/
	s_in->sin_addr.s_addr = inet_addr(netmask);
	ioctl(fd, SIOCSIFNETMASK, &ifr);

	close(fd);
}

void GWConfigurator::_delBridge() {
	/*down device*/
	int fd;
	struct ifreq ifr;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd<0) return;

	/* access _gwif */
	strncpy(ifr.ifr_name, _bridgeif, IFNAMSIZ-1);

	/*get status*/
	ioctl(fd, SIOCGIFFLAGS, &ifr);

	/* add up and running */
	ifr.ifr_flags = ~(IFF_UP | IFF_RUNNING);
	ioctl(fd, SIOCSIFFLAGS, &ifr);

	const char *args[] = {"brctl", "delbr", _bridgeif, NULL};
	_callCmd(args);
}

/** for setIP ***/
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

void GWConfigurator::_callCmd(const char ** in_cmd) {
	std::string cmd;

	int i=0;
	while(in_cmd[i]) {
		cmd += " ";
		cmd += in_cmd[i++];
	}

	FILE *fp = popen(cmd.c_str(), "r");
	pclose(fp);
}
