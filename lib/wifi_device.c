#include <stdio.h>
#include <string.h>
#include <lower_layer_builder.h>
#include "lan_interface.h"
#include "device_json_parser.h"

struct wifi_lan_device_t {
LAN_DEVICE_INTERFACE
	const char*  _ifname;
	const char*  _ssid;
	const char*  _pass;
};

typedef struct wifi_lan_device_t * WifiLANDevice;

static void wifi_lan_clean(LANDeviceInterface this);
static const char * wifi_lan_getname(LANDeviceInterface this);
static void wifi_fail_safe_type_wifi(const json_t * inparam);

static void wifi_lan_clean(LANDeviceInterface this) {
	WifiLANDevice wifi = (WifiLANDevice)this;
	printf("Wifi:clean, if=%s\n", wifi->_ifname);
}

static const char * wifi_lan_getname(LANDeviceInterface this) {
	WifiLANDevice wifi = (WifiLANDevice)this;
	return wifi->_ifname;
}

static void wifi_fail_safe_type_wifi(const json_t * inparam) {
	if(strcmp(json_get_string(inparam, "type") , "wifi") != 0) {
		fprintf(stderr, "type is invalid, wifi");
		exit(1);
	}
}

void * lower_layer_builder_instance_new(void) {
	printf("Wifi:new\n");
	WifiLANDevice instance = calloc(1, sizeof(*instance));
	if(!instance) return NULL;

	/*set interface*/
	instance->clean = wifi_lan_clean;
	instance->getname = wifi_lan_getname;
	return instance;
}

void lower_layer_builder_instance_free(void * interfaceClass) {
	printf("Wifi:free\n");
	free(interfaceClass);
}

int lan_if_load(void * initial_parameter) {
	LANDeviceInitParam param = (LANDeviceInitParam) initial_parameter;
	wifi_fail_safe_type_wifi(param->conf_handle);

	WifiLANDevice instance = (WifiLANDevice)param->class_instance;
	instance->_ifname = json_get_string(param->conf_handle, "name");
	instance->_ssid = json_get_string(param->conf_handle, "ssid");
	instance->_pass = json_get_string(param->conf_handle, "pass");
	printf("Wifi:up, if=%s\n",   instance->_ifname);
	printf("Wifi:up, ssid=%s\n", instance->_ssid);
	printf("Wifi:up, pass=%s\n", instance->_pass);
	return LL_BUILDER_SUCCESS;
}

int lan_if_up(void * initial_parameter) {
	LANDeviceInitParam param = (LANDeviceInitParam) initial_parameter;
	WifiLANDevice instance = (WifiLANDevice)param->class_instance;

	printf("Wifi:up, if=%s\n", instance->_ifname);
	return LL_BUILDER_SUCCESS;
}
