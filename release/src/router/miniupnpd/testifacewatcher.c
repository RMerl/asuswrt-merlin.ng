/* $Id: testifacewatcher.c,v 1.3 2025/03/30 22:36:05 nanard Exp $ */

#include <syslog.h>
#include <signal.h>

#include "ifacewatcher.h"
#include "miniupnpdtypes.h"

int runtime_flags = 0;
time_t startup_time = 0;
struct lan_addr_list lan_addrs;
const char * ext_if_name;
volatile sig_atomic_t should_send_public_address_change_notif = 0;

int main(int argc, char * * argv)
{
#ifndef USE_IFACEWATCHER
	fprintf(stderr, "build without USE_IFACEWATCHER\n");
	return 1;
#else
	int s;

	ext_if_name = (const char *)0;
	if (argc > 1) {
		ext_if_name = argv[1];
	}

	openlog("testifacewatcher", LOG_CONS|LOG_PERROR, LOG_USER);

	s = OpenAndConfInterfaceWatchSocket();
	if (s < 0) {
		syslog(LOG_ERR, "OpenAndConfInterfaceWatchSocket() failed");
		return 1;
	}
	syslog(LOG_DEBUG, "Socket %d open. waiting", s);
	for(;;) {
		if(should_send_public_address_change_notif) {
			syslog(LOG_DEBUG, "should_send_public_address_change_notif !");
			should_send_public_address_change_notif = 0;
		}
		ProcessInterfaceWatchNotify(s);
	}
	closelog();
	return 0;
#endif
}
