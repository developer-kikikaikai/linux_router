//fork
#include <unistd.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include "DHCPConfiguratorImple.hpp"

int DHCPConfiguratorImple::_start_dhcpd(const char * interface) {
	int pid = fork();
	/*child, call dhcpd*/
	if(pid == 0) {
		/*start dns and dhcp*/
		std::string cmd;
		cmd = "/sbin/dnsmasq --no-daemon -C ";
		cmd += _dhcp_conf;
		cmd += " -i ";
		cmd += interface;
		std::cout << "DHCP start, cmd="  << cmd << std::endl;
		FILE *fp = popen(cmd.c_str(), "r");
		pclose(fp);
		std::cout << "DHCP stop"  << std::endl;
		return 0;
	} else {
		return pid;
	}
}

void DHCPConfiguratorImple::_write_dhcp_conf(const char * area_start, const char * area_end, const char *gw, const char *netmask, const char * leasetime) {

	std::string dhcp_conf;
	const char * sepa=",", *end="\n";

	/*range + lease*/
	dhcp_conf+="dhcp-range=";
	_add_conf(dhcp_conf, sepa, {area_start, area_end, leasetime}, end);

	/*netmask*/	
	dhcp_conf+="dhcp-option=";
	_add_conf(dhcp_conf, sepa, {"1", netmask}, end);

	/*dhcp server(gw)*/	
	dhcp_conf+="dhcp-option=";
	_add_conf(dhcp_conf, sepa, {"3", gw}, end);

	/*dns server(gw)*/	
	dhcp_conf+="dhcp-option=";
	_add_conf(dhcp_conf, sepa, {"6", gw}, end);

	/*fix*/
	dhcp_conf+="cache-size=0\n";
	std::cout << dhcp_conf << std::endl;

	/*write file*/
	std::ofstream fout;
	fout.open(_dhcp_conf);
	fout << dhcp_conf;
	fout.close();
}

DHCPConfiguratorImple::DHCPConfiguratorImple(const char * lanif) : DHCPConfigurator(lanif) {
	_dhcp_conf = "/usr/local/etc/dnsmasq.conf";
}

