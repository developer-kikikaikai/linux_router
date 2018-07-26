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

int GWConfigurator::set_ip(const char *ip, const char * netmask) {
	std::cout << "GWConfiguration, set IP " << ip << ",netmask " << netmask << " to " << _bridgeif << std::endl;
	std::cout << "Get OS default GW information" << std::endl;
	_get_gw_ip();

	std::cout << "GW, IF=" << _gwif << ", ip=" << _gwip << std::endl;

	/*setup bridge LAN interface*/
	_add_bridge(ip, netmask);
	return 0;
}

int GWConfigurator::unset_ip() {
	std::cout << "GWConfiguration, unset IP" << std::endl;
	_del_bridge();
	return 0;
}

void GWConfigurator::add_device(const char *name) {
	const char *args[] = {"brctl", "addif", _bridgeif, name, NULL};
	_call_cmd(args);
	
}

void GWConfigurator::del_device(const char *name) {
	const char *args[] = {"brctl", "delif", _bridgeif, name, NULL};
	_call_cmd(args);
}

/****private *****/
void GWConfigurator::_add_bridge(const char *ip, const char * netmask) {
	/*set bridge*/
	const char *args[] = {"brctl", "addbr", _bridgeif, NULL};
	_call_cmd(args);

	/*up and set ip*/
	int fd;
	struct ifreq ifr;
	fd = _get_ifsock();
	if(fd<0) return;

	/* access _gwif */
	strncpy(ifr.ifr_name, _bridgeif, IFNAMSIZ-1);

	/*get status*/
	ioctl(fd, SIOCGIFFLAGS, &ifr);
	/* add up and running */
	ifr.ifr_flags |= (IFF_UP | IFF_RUNNING | IFF_DYNAMIC);
	ioctl(fd, SIOCSIFFLAGS, &ifr);

	/* set ip address */
	_ifr_ip(&ifr);
	_ifr_setaddr(ip, fd, &ifr, SIOCSIFADDR);

	/*set netmask*/
	_ifr_setaddr(ip, fd, &ifr, SIOCSIFNETMASK);

	close(fd);
}

void GWConfigurator::_del_bridge() {
	/*down device*/
	int fd;
	struct ifreq ifr;
	fd = _get_ifsock();
	if(fd<0) return;

	/* access _gwif */
	strncpy(ifr.ifr_name, _bridgeif, IFNAMSIZ-1);

	/*get status*/
	ioctl(fd, SIOCGIFFLAGS, &ifr);
	/* add up and running */
	ifr.ifr_flags = ~(IFF_UP | IFF_RUNNING);
	ioctl(fd, SIOCSIFFLAGS, &ifr);

	const char *args[] = {"brctl", "delbr", _bridgeif, NULL};
	_call_cmd(args);
}

/** for set_ip ***/
void GWConfigurator::_get_gw_if(){

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

int GWConfigurator::_is_gw_if() {
	return (_gwif[0] == 0);
}

void GWConfigurator::_get_gw_ip() {

	memset(_gwip, 0, sizeof(_gwip));
	/*get gw name*/
	_get_gw_if();
	if(_is_gw_if()) return;

	int fd;
	/*struct to get interface information*/
	struct ifreq ifr;

	fd = _get_ifsock();
	if(fd < 0) return;

	_ifr_ip(&ifr);

	/* access _gwif */
	strncpy(ifr.ifr_name, _gwif, IFNAMSIZ-1);
	/* get address */
	ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);

	snprintf(_gwip, sizeof(_gwip), "%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}

void GWConfigurator::_call_cmd(const char ** in_cmd) {
	std::string cmd;

	int i=0;
	while(in_cmd[i]) {
		cmd += " ";
		cmd += in_cmd[i++];
	}

	FILE *fp = popen(cmd.c_str(), "r");
	pclose(fp);
}

void GWConfigurator::_ifr_ip(struct ifreq * ifr) {
	/* IP addr */
	ifr->ifr_addr.sa_family = AF_INET;
}

int GWConfigurator::_ifr_setaddr(const char * ip, int fd, struct ifreq * ifr, int addrflag) {
	struct sockaddr_in * s_in = (struct sockaddr_in *)&ifr->ifr_addr;
	inet_pton(AF_INET, ip, &s_in->sin_addr.s_addr);
	return ioctl(fd, addrflag, ifr);
}

int GWConfigurator::_get_ifsock() {
	return socket(AF_INET, SOCK_DGRAM, 0);
}
