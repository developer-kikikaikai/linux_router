#include<cstring>
#include "LANManager.hpp"
#include "lan_interface.h"
#include <unistd.h>

extern "C" {
#include "dp_debug.h"
}

int LANManager::set_lan(void) {
	std::cout << "LANManager::start" << std::endl;
	return _ipmanager->set();
}

void LANManager::add_devices(void) {
	std::cout << "LANManager::add_device"  << std::endl;
	std::cout << "Load device by using LowerLayerInerface" << std::endl;

	/*load plugin*/
	LowerLayerDirector director;
	lan_device_init_param_t device_init;
	char * plugin_name;
	const char * key;
	const json_t * value;
	device_init.lanif = _ipmanager->getlanif();
	for(auto itr = _setting_parser->plugin_map.begin(); itr != _setting_parser->plugin_map.end(); ++itr) {
		std::cout << itr->first << std::endl;
		json_object_foreach(itr->second, key, value)  {
			std::cout << "    " <<  key << ":" << json_string_value(value) << std::endl;
		}
		plugin_name = (char *)itr->first.c_str();
		director = lower_layer_director_new(plugin_name , (char*)"../conf/lan_interface.conf");
		if(director == NULL) {
			fprintf(stderr, "Failed to load %s\n", plugin_name);
			exit(1);
		}
		/* keep plugin instance */
		_directors.push_back(director);
		device_init.conf_handle = itr->second;
		device_init.class_instance = (LANDeviceInterface)director->lower_layer_interface;

		/*up interface*/
		lower_layer_director_construct(director, &device_init, NULL);

		/*add device*/
		_ipmanager->add_if(device_init.class_instance->getname(device_init.class_instance));
	}
}

void LANManager::delete_devices(void ){
	std::cout << "LANManager::delete_device" << std::endl;
	LANDeviceInterface class_instance;
	for(auto itr = _directors.begin(); itr != _directors.end(); ++itr) {
		
		/*del device*/
		class_instance = (LANDeviceInterface) (*itr)->lower_layer_interface;
		_ipmanager->del_if(class_instance->getname(class_instance));
		/*down interface*/
		lower_layer_director_free(*itr);
	}

	/*clear data*/
	std::vector<LowerLayerDirector> v;
	_directors.swap(v);
}

int LANManager::unset_lan() {
	std::cout << "LANManager::unset_lan" << std::endl;
	return _ipmanager->unset();
}
