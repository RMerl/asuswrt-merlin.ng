#ifndef CMDLIST_ARCH_H
#define CMDLIST_ARCH_H

#ifndef CONFIG_BCM_ARCHER
#include "bcm_rdp_arch.h"
#endif /* CONFIG_BCM_ARCHER */

#if defined(RDP_ARCH_SIM)
#include "pktrunner_rdpa_sim.h"
#define MODULE_VERSION(val1)

#define CMDLIST_STATS_PRINT(sf, bytes, fmt, arg...)     \
    do {                                                \
        bytes += printf(fmt,##arg);             \
    } while(0);

#define blog_ethernet_offset() OFFSETOF(BlogEthHdr_t, ethType)

#elif defined(RDP_ARCH_BOARD) || defined(CONFIG_BCM_ARCHER) || defined(RDP_ARCH_QEMU_SIM)

#include <linux/module.h>
#include <linux/bcm_log.h>
#include <linux/blog.h>
#include <linux/nbuff.h>
#include <net/ipv6.h>
#include <bcmtypes.h>
#include "pktHdr.h"
#include "bcmxtmcfg.h"
#include "bcmenet_common.h"
#include "bcm_vlan.h"

#define CMDLIST_STATS_PRINT(sf, bytes, fmt, arg...)     \
    do {                                                \
        if(sf) seq_printf(sf, fmt, ##arg);              \
        else bytes += bcm_print(fmt,##arg);             \
    } while(0);

#define blog_ethernet_offset() OFFSETOF(BlogEthHdr, ethType)

#else
#error "no arch defined"
#endif /* RDP_ARCH_SIM */

#endif /* CMDLIST_ARCH_H */
