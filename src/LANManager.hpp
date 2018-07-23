#ifndef LAN_MANAGER_HPP
#define LAN_MANAGER_HPP

#include <vector>
#include "LANIPManager.hpp"
#include "JsonParser.hpp"
#include <lower_layer_director.h>

class LANManager {
private:
	const char * _setting;
	LANIPManager * _ipmanager;
	JsonParser *_setting_parser;
	std::vector<LowerLayerDirector> _directors;
public:
	LANManager(const char * input_setting) {
		_setting_parser = new JsonParser(input_setting);
		_ipmanager = new LANIPManager(_setting_parser->lan_info);
	}

	~LANManager() {
		delete _ipmanager;
	}

	int set_lan(void);
	void add_devices(void);
	void delete_devices(void);
	int unset_lan(void);
};
#endif
