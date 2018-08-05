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
	const char * sepa=" ", *end=";\n", *opt="  ";
	_add_conf(dhcp_conf, sepa, {"default-lease-time", leasetime, end});
	_add_conf(dhcp_conf, sepa, {"max-lease-time", leasetime, end});
	dhcp_conf += "log-facility local7;\n";//fix
	_add_conf(dhcp_conf, " ", {"subnet", _get_subnet(gw, netmask) , "netmask", netmask, " {\n"});
	//option start
	_add_conf(dhcp_conf, sepa, {opt, "range", area_start, area_end, end});
	_add_conf(dhcp_conf, sepa, {opt, "option", "subnet-mask", netmask, end});
	_add_conf(dhcp_conf, sepa, {opt, "option", "routers", gw, ";\n"});
	_add_conf(dhcp_conf, sepa, {opt, "option", "domain-name-servers", gw, end});
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

