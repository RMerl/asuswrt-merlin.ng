#ifndef _wlinfo_utils_h_
#define _wlinfo_utils_h_

#if defined(RTCONFIG_RALINK)
#include <netinet/ether.h>	//struct ether_addr

extern char *wl_ether_etoa(const struct ether_addr *n);

#endif

#endif /* _wlif_utils_h_ */

