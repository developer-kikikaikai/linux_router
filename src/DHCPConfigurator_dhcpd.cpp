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
#include "DHCPConfiguratorImple.hpp"

void DHCPConfiguratorImple::_add_conf(std::string &srcstr, const char * key, const char * val, const char * last) {
	srcstr+=key;
	srcstr+=" ";
	srcstr+=val;
	srcstr+=" ";
	srcstr+=last;
}
void DHCPConfiguratorImple::_add_conf_option(std::string &srcstr, const char * key, const char *val1, const char * val2) {
	srcstr += "  ";
	_add_conf(srcstr, key, val1, val2);
	srcstr += ";\n";
}

int DHCPConfiguratorImple::_start_dhcpd(const char * interface) {
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
		return 0;
	} else {
		return pid;
	}
}

void DHCPConfiguratorImple::_write_dhcp_conf(const char * area_start, const char * area_end, const char *gw, const char *netmask, const char * leasetime) {
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

DHCPConfiguratorImple::DHCPConfiguratorImple(const char * lanif) : DHCPConfigurator(lanif){
	_dhcp_conf = "/usr/local/etc/dhcpd.conf";
}

