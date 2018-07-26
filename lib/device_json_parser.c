#include <jansson.h>
/*cpp side, it is not set "static"*/
const char * json_get_string(const json_t * object, const char * key) {
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
