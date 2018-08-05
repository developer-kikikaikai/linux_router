#include "LANManager.hpp"

LANIPManager::LANIPManager(const json_t * lan_info) {
	_lan_info = lan_info;
	const char * lanif = getlanif();
	_dhcp = new DHCPConfiguratorImple(lanif);
	_gw = new GWConfigurator(lanif);
}

int LANIPManager::_set_gw(void) {
	const char * netmask = JsonParser::get_string(_lan_info, "netmask");
	const char * ipv4 = JsonParser::get_string(_lan_info, "ipv4");
	_gw->set_ip(ipv4, netmask);
	return 0;
}

int LANIPManager::_set_dhcp(void) {
	const char * start = JsonParser::get_string(_lan_info, "dhcpv4start");
	const char * end = JsonParser::get_string(_lan_info, "dhcpv4end");
	const char * netmask = JsonParser::get_string(_lan_info, "netmask");
	const char * ipv4 = JsonParser::get_string(_lan_info, "ipv4");
	const char * leasetime = JsonParser::get_string(_lan_info, "lease-time");
	_dhcp->start(start, end, ipv4, netmask, leasetime);
	return 0;
}

int LANIPManager::set(void) {
	std::cout << "LANIPManager::start" << std::endl;

	/* Start GW*/
	_set_gw();

	/* Start DHCP*/
	_set_dhcp();

	return 0;
}

const char * LANIPManager::getlanif(void) {
	return JsonParser::get_string(_lan_info, "name");
}

int LANIPManager::add_if(const char * name) {
	_gw->add_device(name);
	return 0;
}

int LANIPManager::del_if(const char * name) {
	_gw->del_device(name);
	return 0;
}

int LANIPManager::unset(void) {
	std::cout << "LANIPManager::start, setting" << std::endl;
	if(_dhcp != NULL) _dhcp->stop();
	if(_gw != NULL) _gw->unset_ip();
	return 0;
}
