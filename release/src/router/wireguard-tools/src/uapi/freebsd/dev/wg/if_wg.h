#ifndef __IF_WG_H__
#define __IF_WG_H__

#include <net/if.h>
#include <netinet/in.h>

struct wg_data_io {
	char wgd_name[IFNAMSIZ];
	void *wgd_data;
	size_t wgd_size;
};

#define SIOCSWG _IOWR('i', 210, struct wg_data_io)
#define SIOCGWG _IOWR('i', 211, struct wg_data_io)

#endif
