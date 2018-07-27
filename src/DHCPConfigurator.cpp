//fork
#include <unistd.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <signal.h>
#include "DHCPConfigurator.hpp"

void DHCPConfigurator::_add_conf(std::string &srcstr, const char * key, const char * val, const char * last) {
	srcstr+=key;
	srcstr+=" ";
	srcstr+=val;
	srcstr+=" ";
	srcstr+=last;
}
void DHCPConfigurator::_add_conf_option(std::string &srcstr, const char * key, const char *val1, const char * val2) {
	srcstr += "  ";
	_add_conf(srcstr, key, val1, val2);
	srcstr += ";\n";
}

const char * DHCPConfigurator::_get_subnet(const char *gw, const char *netmask) {

	/*get gw and netmask bytearray*/
	uint32_t gw_binary;
	uint32_t netmask_binary;
	inet_pton(AF_INET, gw, &gw_binary);
	inet_pton(AF_INET, netmask, &netmask_binary);

	/*get subnet*/
	struct in_addr sin_addr;
	sin_addr.s_addr = gw_binary & netmask_binary;
	/*change to string*/
	return inet_ntoa(sin_addr);
}

void DHCPConfigurator::_start_dhcpd(const char * interface) {
	int pid = fork();
	/*child, call dhcpd*/
	if(pid == 0) {
		/*start dns first*/
		std::string cmd;
		cmd="dnsmasq";
		FILE *fp = popen(cmd.c_str(), "r");
		pclose(fp);

		cmd = "/usr/sbin/dhcpd -f -cf ";
		cmd += _dhcp_conf;
		cmd += " -user dhcpd -group dhcpd --no-pid ";
		//cmd += " -user dhcpd -group dhcpd --no-pid -d";//if you want to show debug
		cmd += interface;

		/*call dhcpd*/
		std::cout << "DHCP start, cmd="  << cmd << std::endl;
		fp = popen(cmd.c_str(), "r");
		pclose(fp);
		std::cout << "DHCP stop"  << std::endl;
	} else {
		_dhcpd_pid = pid;
	}
}

void DHCPConfigurator::_write_dhcp_conf(const char * area_start, const char * area_end, const char *gw, const char *netmask, const char * leasetime) {
	std::string dhcp_conf;
	_add_conf(dhcp_conf, "default-lease-time", leasetime, ";\n");
	_add_conf(dhcp_conf, "max-lease-time", leasetime, ";\n");
	dhcp_conf += "log-facility local7;\n";//fix
	_add_conf(dhcp_conf, "subnet", _get_subnet(gw, netmask) , " ");
	_add_conf(dhcp_conf, "netmask", netmask, " {\n");
	//option start
	_add_conf_option(dhcp_conf, "range", area_start, area_end);
	_add_conf_option(dhcp_conf, "option", "subnet-mask", netmask);
	_add_conf_option(dhcp_conf, "option", "routers", gw);
	_add_conf_option(dhcp_conf, "option", "domain-name-servers", gw);
	dhcp_conf += "}";
	std::cout << dhcp_conf << std::endl;

	/*write file*/
	std::ofstream fout;
	fout.open(_dhcp_conf);
	fout << dhcp_conf;
	fout.close();
}

DHCPConfigurator::DHCPConfigurator(const char * lanif) {
	_lanif = lanif;
	_dhcp_conf = "/usr/local/share/dhcpd.conf";
	_dhcpd_pid = 0;
}

DHCPConfigurator::~DHCPConfigurator() {
	stop();
}

int DHCPConfigurator::start(const char * area_start, const char * area_end, const char *gw, const char *netmask, const char * leasetime) {
	std::cout << "DHCPConfigurator::start, area_start is " << area_start<< ", end is " << area_end << std::endl;

	_write_dhcp_conf(area_start, area_end, gw, netmask, leasetime);;

	_start_dhcpd(_lanif);
        return 0;
}

int DHCPConfigurator::stop(void) {
	std::cout << "DHCPConfigurator::stop" << std::endl;
	if(_dhcpd_pid != 0) {
		/*stop dhcpd*/
		kill(_dhcpd_pid, SIGTERM);
		waitpid(_dhcpd_pid, NULL, 0);
		_dhcpd_pid=0;
	}
	return 0;
}
