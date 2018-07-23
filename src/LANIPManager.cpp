#include "LANManager.hpp"

LANIPManager::LANIPManager(const json_t * lan_info) {
	_lan_info = lan_info;
	_dhcp = new DHCPConfigurator();
	_gw = new GWConfigurator(JsonParser::get_string(_lan_info, "name"));
}

int LANIPManager::_set_gw(void) {
	const char * netmask = JsonParser::get_string(_lan_info, "netmask");
	const char * ipv4 = JsonParser::get_string(_lan_info, "ipv4");
	_gw->setIP(ipv4, netmask);
	return 0;
}

int LANIPManager::_set_dhcp(void) {
	const char * start = JsonParser::get_string(_lan_info, "dhcpv4start");
	const char * end = JsonParser::get_string(_lan_info, "dhcpv4end");
	_dhcp->start(start, end);
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

int LANIPManager::unset(void) {
	std::cout << "LANIPManager::start, setting" << std::endl;
	if(_dhcp != NULL) _dhcp->stop();
	if(_gw != NULL) _gw->unsetIP();
	return 0;
}
