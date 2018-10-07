#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <shared.h>
#include <rc.h>
#include <netinet/ether.h>
#include <roamast.h>
void get_stainfo(int bssidx, int vifidx)
{

}

#ifdef RTCONFIG_ADV_RAST
int rast_stamon_get_rssi(int bssidx, struct ether_addr *addr)
{

}

void rast_retrieve_static_maclist(int bssidx, int vifidx)
{

}

void rast_set_maclist(int bssidx, int vifidx)
{

}
#if 0
uint8 rast_get_rclass(int bssidx, int vifidx)
{

}

uint8 rast_get_channel(int bssidx, int vifidx)
{

}

int rast_send_bsstrans_req(int bssidx, int vifidx, struct ether_addr *sta_addr, struct ether_addr *nbr_bssid)
{

}
#endif
#endif
