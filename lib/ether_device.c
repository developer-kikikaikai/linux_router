#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lan_interface.h"
#include "device_json_parser.h"
#include <lower_layer_builder.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

struct eth_lan_device_t {
LAN_DEVICE_INTERFACE
	const char*  _ifname;
};

typedef struct eth_lan_device_t * EthLANDevice;

static void eth_lan_clean(LANDeviceInterface this);
static const char * eth_lan_getname(LANDeviceInterface this);
static void eth_fail_safe_type_ethernet(const json_t * inparam);

static void eth_lan_clean(LANDeviceInterface this) {
	EthLANDevice eth = (EthLANDevice)this;
	printf("Ether:clean, if=%s\n", eth->_ifname);
	
	/* access if, down to reset IP and up */
	struct ifreq ifr;
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd<0) return;
	
	/*get status*/
	ioctl(fd, SIOCGIFFLAGS, &ifr);
	/*down*/
	ifr.ifr_flags = ~(IFF_UP | IFF_RUNNING);
	ioctl(fd, SIOCSIFFLAGS, &ifr);
	close(fd);
}

static const char * eth_lan_getname(LANDeviceInterface this) {
	EthLANDevice eth = (EthLANDevice)this;
	return eth->_ifname;
}

static void eth_fail_safe_type_ethernet(const json_t * inparam) {
	if(strcmp(json_get_string(inparam, "type") , "ethernet") != 0) {
		fprintf(stderr, "type is invalid, eth");
		exit(1);
	}
}

void * lower_layer_builder_instance_new(void) {
	printf("Ether:new\n");
	EthLANDevice instance = calloc(1, sizeof(*instance));
	if(!instance) return NULL;

	/*set interface*/
	instance->clean = eth_lan_clean;
	instance->getname = eth_lan_getname;
	return instance;
}

void lower_layer_builder_instance_free(void * interfaceClass) {
	printf("Ether:free\n");
	free(interfaceClass);
}

int lan_if_up(void * initial_parameter) {
	LANDeviceInitParam param = (LANDeviceInitParam) initial_parameter;
	eth_fail_safe_type_ethernet(param->conf_handle);

	EthLANDevice instance = (EthLANDevice)param->class_instance;

	instance->_ifname = json_get_string(param->conf_handle, "name");
	printf("Ether:up, if=%s\n", instance->_ifname);

	struct ifreq ifr;
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd<0) return LL_BUILDER_FAILED;

	/* access if, down to reset IP and up */
	strncpy(ifr.ifr_name, instance->_ifname, IFNAMSIZ-1);
	/*get status*/
	ioctl(fd, SIOCGIFFLAGS, &ifr);

	/*set release IP flag*/
	ifr.ifr_flags |= IFF_DYNAMIC;
	ioctl(fd, SIOCGIFFLAGS, &ifr);

	/*down*/
	ifr.ifr_flags = ~(IFF_UP | IFF_RUNNING);
	ioctl(fd, SIOCSIFFLAGS, &ifr);

	/*up*/
	ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
	ioctl(fd, SIOCSIFFLAGS, &ifr);

	close(fd);
	return LL_BUILDER_SUCCESS;
}
