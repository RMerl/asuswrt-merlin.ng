/*************************************************************************
 *
 * ivi_portmap.h :
 *
 * This file is the header file for the 'ivi_portmap.c' file.
 *
 * Contributions:
 *
 * This file is part of MAP-T/MAP-E Kernel Module.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * You should have received a copy of the GNU General Public License 
 * along with MAP-T/MAP-E Kernel Module. If not, see 
 * <http://www.gnu.org/licenses/>.
 *
 * LIC: GPLv2
 *
 ************************************************************************/


#ifndef IVI_PORTMAP_H
#define IVI_PORTMAP_H

#include <net/ip.h>
#include "ivi_config.h"
#include <linux/brcm_dll.h>

#define MAPPORTMAP_HTABLE_SIZE 32
#define MAPPORTMAP_MAX_ENTRIES 128
#define MAPPORTMAP_IX_INVALID 0
#define MAPPORTMAP_NULL ((MapPortmap_t*)NULL)

#define MAPPORTMAP_INVALID_ADDRESS 0xFFFFFFFF

#define MAPPORTMAP_MODE_ADD		0
#define MAPPORTMAP_MODE_FIND	1
#define MAPPORTMAP_MODE_DEL		2

#define MAPPORTMAP_PROTO_TCP	0
#define MAPPORTMAP_PROTO_UDP	1
#define MAPPORTMAP_PROTO_ICMP	2

typedef struct mapportmap_t {
	struct dll_t node;
	u32 idx;
	struct mapportmap_t *chain_p;
	struct portmap_info info;
} __attribute__ ((packed)) MapPortmap_t;

extern u32 mapportmap_lookup( u32 *lanAddr, u32 wanAddr, u32 port, u32 *intPort, u32 proto, int mode );
extern void mapportmap_delete( u32 idx, u32 proto );
extern int mapportmap_port( u16 port, int type );
extern int init_mapportmap_list(void);

#endif /* IVI_PORTMAP_H */
