#ifndef LAN_INTERFACE_H
#define LAN_INTERFACE_H
#include <jansson.h>

struct lan_device_interface_t;
typedef struct lan_device_interface_t lan_device_interface_t, *LANDeviceInterface;
struct lan_device_interface_t {
	void (*clean)(LANDeviceInterface instance);
	const char * (*getname)(LANDeviceInterface instance);
};

#define LAN_DEVICE_INTERFACE \
	void (*clean)(LANDeviceInterface instance);\
	const char * (*getname)(LANDeviceInterface instance);

/*for builder interface initial parameter*/
struct lan_device_init_param_t;
typedef struct lan_device_init_param_t lan_device_init_param_t, *LANDeviceInitParam;
struct lan_device_init_param_t {
	const json_t * conf_handle;
	LANDeviceInterface class_instance;
};
//int lan_if_load(LANDeviceInitParam * input_param);
//int lan_if_up(LANDeviceInitParam * input_param);

#endif
