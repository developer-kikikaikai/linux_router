#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <lower_layer_builder.h>
#include "lan_interface.h"
#include "device_json_parser.h"
#include <signal.h>
#include <spawn.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <net/if.h>

#define SUPPLICANT_CONF "/usr/local/etc/wpa_supplicant.conf"
#define CTRL_IF "/var/run/wpa_supplicant_lan"
#define MAX_CNT (20)
struct wifi_lan_device_t {
LAN_DEVICE_INTERFACE
	const char*  _ifname;
	const char* _freq;
	const char*  _ssid;
	const char*  _pass;
	int _pid;
};

typedef struct wifi_lan_device_t * WifiLANDevice;

/*@name for lan interface implement*/
/*@{*/
static void wifi_lan_clean(LANDeviceInterface this);
static const char * wifi_lan_getname(LANDeviceInterface this);
static void wifi_fail_safe_type_wifi(const json_t * inparam);
/*@}*/

/*@name for device initialization to support builder interface*/
static void conf_fwrite(const char * val, FILE *fp);
static void write_supplicant_conf(WifiLANDevice this);
static int is_wifi_ap_running(WifiLANDevice instance);
static int is_wifi_if_up(WifiLANDevice instance);
/*@}*/

/*@name for lan interface implement*/
/*@{*/
static void wifi_lan_clean(LANDeviceInterface this) {
	WifiLANDevice wifi = (WifiLANDevice)this;
	printf("Wifi:clean, if=%s\n", wifi->_ifname);
	kill(wifi->_pid, SIGTERM);
	waitpid(wifi->_pid, NULL, 0);
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
/*@}*/

/*@name lower layer interface implement*/
/*@{*/
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
/*@}*/

/*@name lower layer interface implement*/
/*@{*/

/**write conf*/
static void conf_fwrite(const char * val, FILE *fp) {
	fwrite(val, strlen(val), 1, fp);
	fwrite("\n", strlen("\n"), 1, fp);
}

/**write conf macro*/
#define CWRITE_NON(a) conf_fwrite((a), fp);
#define CWRITE_OPT(a, str) sprintf(line,"%s%s", a, str);CWRITE_NON(line);
#define CWRITE_OP2(a, str, str2) sprintf(line,"%s%s%s", a, str, str2);CWRITE_NON(line);

/**write wpa_supplicant.conf*/
static void write_supplicant_conf(WifiLANDevice this) {
	FILE *fp = fopen(SUPPLICANT_CONF, "w");
	if(!fp) return;

	char line[64]={0};
	CWRITE_OPT("ctrl_interface=", CTRL_IF );
	CWRITE_NON("network={")
	CWRITE_OP2("    ssid=\"", this->_ssid, "\"");
	CWRITE_NON("    mode=2");
	CWRITE_NON("    proto=WPA2");
	CWRITE_OP2("    psk=\"", this->_pass, "\"");
	CWRITE_OPT("    frequency=", this->_freq);
        CWRITE_NON("}");
	fclose(fp);
}

/** check wifi ap start status */
static int is_wifi_ap_running(WifiLANDevice instance) {
	/*check statuc*/
	char wpa_cli_cmd[100];
	snprintf(wpa_cli_cmd, sizeof(wpa_cli_cmd), "wpa_cli -p %s  -i %s  status | grep wpa_state", CTRL_IF, instance->_ifname);
	FILE *wpa_cli;
	char result[64];
	int retry_cnt=0;
	while(retry_cnt<MAX_CNT) {
		retry_cnt++;
		memset(result, 0, sizeof(result));
		wpa_cli = popen(wpa_cli_cmd, "r");
		fgets(result, sizeof(result), wpa_cli);
		pclose(wpa_cli);
		if(strcmp(result, "wpa_state=COMPLETED\n") == 0) {
			printf("AP start Completed!!\n");
			return 1;
		}
		sleep(1);
	}
	return 0;
}

static int is_wifi_if_up(WifiLANDevice instance) {
	int ret = 0;
	/*up if*/
	int retry_cnt=0;
	/*check if up*/
	struct ifreq ifr;
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name, instance->_ifname, IFNAMSIZ-1);
	while(retry_cnt<MAX_CNT) {
		retry_cnt++;
		ifr.ifr_flags = 0;
		ioctl(fd, SIOCGIFFLAGS, &ifr);
		if((ifr.ifr_flags & IFF_UP) && (ifr.ifr_flags & IFF_RUNNING)) {
			printf("completed IF!!\n");
			ret = 1;
			break;
		}
		sleep(1);
	}
	close(fd);
	return ret;
}

/** lan_if_load implement (now it doesn't need because OS load device automatically in own env) */
int lan_if_load(void * initial_parameter) {
	LANDeviceInitParam param = (LANDeviceInitParam) initial_parameter;
	wifi_fail_safe_type_wifi(param->conf_handle);

	WifiLANDevice instance = (WifiLANDevice)param->class_instance;
	instance->_ifname = json_get_string(param->conf_handle, "name");
	instance->_freq = json_get_string(param->conf_handle, "freq");
	instance->_ssid = json_get_string(param->conf_handle, "ssid");
	instance->_pass = json_get_string(param->conf_handle, "pass");
	printf("Wifi:up, if=%s\n",   instance->_ifname);
	printf("Wifi:up, freq=%s\n", instance->_freq);
	printf("Wifi:up, ssid=%s\n", instance->_ssid);

	printf("Please load driver if you need\n");
	return LL_BUILDER_SUCCESS;
}

int lan_if_up(void * initial_parameter) {
	LANDeviceInitParam param = (LANDeviceInitParam) initial_parameter;
	WifiLANDevice instance = (WifiLANDevice)param->class_instance;

	printf("Wifi:up, if=%s\n", instance->_ifname);
	write_supplicant_conf(instance);

	/*write supplicant.conf*/
	write_supplicant_conf(instance);
	const char *args[] = {
		"/sbin/wpa_supplicant",
		"-i",
		 instance->_ifname,
		"-b",
		 param->lanif,
		"-c",
		 SUPPLICANT_CONF,
		"-d",
		"nl80211",
		NULL
	};
	posix_spawn( &instance->_pid, args[0], NULL, NULL, (char * const* __restrict__)args, NULL );		

	/*check statuc*/
	if(!is_wifi_ap_running(instance)) {
		fprintf(stderr, "AP start failed. Please check setting.\n");
		wifi_lan_clean((LANDeviceInterface)instance);
		return LL_BUILDER_FAILED;
	}

	printf("AP start Completed!! Check IF %s up.\n", instance->_ifname);

	/*up if*/
	if(!is_wifi_if_up(instance)) {
		fprintf(stderr, "IF %s up failed. Please check setting.\n", instance->_ifname);
		wifi_lan_clean((LANDeviceInterface)instance);
		return LL_BUILDER_FAILED;
	}

	printf("All is Completed!!\n");

	return LL_BUILDER_FAILED;
}
