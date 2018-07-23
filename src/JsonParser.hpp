#ifndef JSON_PARSER_HPP
#define JSON_PARSER_HPP

#include <string.h>
#include <map>
#include <jansson.h>

class JsonParser {
private:
	std::string _lan_name="bridgeif";
	json_t * _setting_master;
public:
	const json_t * lan_info;
	std::map<std::string,json_t *> plugin_map;

	JsonParser(const char * input_setting);
	~JsonParser();
	static const char * get_string(const json_t * object, const char * key);
};
#endif
