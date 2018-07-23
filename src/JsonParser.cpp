#include <string.h>
#include <fstream>
#include <iostream>

#include "JsonParser.hpp"

JsonParser::JsonParser(const char * input_setting) {
	/*read file first*/
	std::string input_string;
	try {
		/*openfile*/
		std::ifstream ifs(input_setting, std::ios::binary);

		/*read all*/
		std::string buf;
		while(getline(ifs, buf)) {
			input_string += buf;
		}
	} catch (std::ifstream::failure e){ 
		exit(-1);
	}

	json_error_t error;
	_setting_master = json_loads(input_string.c_str(), 0, &error);
	if ( _setting_master == NULL ) {
		fputs(error.text, stderr);
		exit(-1);
	}

	/*add data into map*/
	const char *key;
	json_t *value;
	json_object_foreach(_setting_master, key, value) {
		std::cout << key << std::endl;
		if( key == _lan_name) {
			lan_info = value;
		} else {
			plugin_map[key] = value;
		}
	}

	std::cout << _lan_name << std::endl;
	json_object_foreach((json_t *)lan_info, key, value)  {
		 std::cout << "    " <<  key << ":" << json_string_value(value) << std::endl;
	}

	for(auto itr = plugin_map.begin(); itr != plugin_map.end(); ++itr) {
		std::cout << itr->first << std::endl;
		json_object_foreach(itr->second, key, value)  {
			std::cout << "    " <<  key << ":" << json_string_value(value) << std::endl;
		}
	}
}

JsonParser::~JsonParser() {
	json_decref(_setting_master);
}

/*cpp side, it is not set "static"*/
const char * JsonParser::get_string(const json_t * object, const char * key) {
	json_t * json_value = json_object_get(object, key);
	if(json_value == NULL) goto errend;
	{
	const char * value = json_string_value(json_value);
	if(value == NULL) goto errend;

	return value;
	}
errend:
	fprintf(stderr, "setting error, please check key %s\n", key);
	exit(-1);
}
