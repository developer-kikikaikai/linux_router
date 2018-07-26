#ifndef DEVICE_JSON_PARSER_H
#define DEVICE_JSON_PARSER_H
#include <jansson.h>
const char * json_get_string(const json_t * object, const char * key);
#endif
