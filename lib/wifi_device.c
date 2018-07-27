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

#define SUPPLICANT_CONF "/usr/local/share/wpa_supplicant.conf"
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

static void wifi_lan_clean(LANDeviceInterface this);
static const char * wifi_lan_getname(LANDeviceInterface this);
static void wifi_fail_safe_type_wifi(const json_t * inparam);

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
	instance->_freq = json_get_string(param->conf_handle, "freq");
	instance->_ssid = json_get_string(param->conf_handle, "ssid");
	instance->_pass = json_get_string(param->conf_handle, "pass");
	printf("Wifi:up, if=%s\n",   instance->_ifname);
	printf("Wifi:up, freq=%s\n", instance->_freq);
	printf("Wifi:up, ssid=%s\n", instance->_ssid);

	printf("Please load driver if you need\n");
	return LL_BUILDER_SUCCESS;
}

static void conf_fwrite(const char * val, FILE *fp) {
	fwrite(val, strlen(val), 1, fp);
	fwrite("\n", strlen("\n"), 1, fp);
}

#define CWRITE_NON(a) conf_fwrite((a), fp);
#define CWRITE_OPT(a, str) sprintf(line,"%s%s", a, str);CWRITE_NON(line);
#define CWRITE_OP2(a, str, str2) sprintf(line,"%s%s%s", a, str, str2);CWRITE_NON(line);

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

int lan_if_up(void * initial_parameter) {
	int ret = LL_BUILDER_FAILED;
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
	posix_spawn( &instance->_pid, args[0], NULL, NULL, args, NULL );		

	/*check statuc*/
	char wpa_cli_cmd[100];
	snprintf(wpa_cli_cmd, sizeof(wpa_cli_cmd), "wpa_cli -p %s  -i %s  status | grep wpa_state", CTRL_IF, instance->_ifname);
	FILE *wpa_cli;
	char result[64];
	int retry_cnt=0;
	while(retry_cnt<MAX_CNT) {
		fprintf(stderr, "Check conpleted!!\n");
		retry_cnt++;
		memset(result, 0, sizeof(result));
		wpa_cli = popen(wpa_cli_cmd, "r");
		fgets(result, sizeof(result), wpa_cli);
		pclose(wpa_cli);
		fprintf(stderr, "err:%s", result);
		if(strcmp(result, "wpa_state=COMPLETED\n") == 0 || strcmp(result, "wpa_state=DISCONNECTED\n") == 0) {
			fprintf(stderr, "Completed!! check if u!!!!!p\n");
			ret = LL_BUILDER_SUCCESS;
			break;
		}
		sleep(1);
	}
	fprintf(stderr, "Completed!! check if up\n");

	/*up if*/
	if(ret == LL_BUILDER_SUCCESS) {
		fprintf(stderr, "Check conpleted IF!!\n");
		ret = LL_BUILDER_FAILED;
		retry_cnt=0;
		/*check if up*/
		struct ifreq ifr;
		int fd = socket(AF_INET, SOCK_DGRAM, 0);
		strncpy(ifr.ifr_name, instance->_ifname, IFNAMSIZ-1);
		while(retry_cnt<MAX_CNT) {
			retry_cnt++;
			ifr.ifr_flags = 0;
			ioctl(fd, SIOCGIFFLAGS, &ifr);
			if((ifr.ifr_flags & IFF_UP) && (ifr.ifr_flags & IFF_RUNNING)) {
				fprintf(stderr, "conpleted IF!!\n");
				ret = LL_BUILDER_SUCCESS;
				system("ifconfig");
				usleep(100000);
				break;
			}
				fprintf(stderr, "unconpleted IF!!\n");
			system("ifconfig");
			sleep(1);
		}
		close(fd);
	fprintf(stderr, "Completed!!\n");
	}
	return ret;
}
