#include <stdio.h>
#include <signal.h>
#include "LANManager.hpp"

static int running;
static void handler(int signum) {
	running=0;
}

int main(int argc, const char *argv[]) {
	if(argc != 2) {
		std::cout << "usage: " << argv[0] << " confname" << std::endl;
		return 0;
	}

	LANManager * lan_manager = new LANManager(argv[1]);
	lan_manager->set_lan();
	lan_manager->add_devices();

	signal(SIGINT, handler);

	running = 1;

	//for main loop
	struct timeval timeval;
	while(running) {
		timeval.tv_sec = 1;
		timeval.tv_usec = 0;
		select(1, NULL, NULL, NULL, &timeval);
	}

	lan_manager->delete_devices();
	lan_manager->unset_lan();
	return 0;
}
