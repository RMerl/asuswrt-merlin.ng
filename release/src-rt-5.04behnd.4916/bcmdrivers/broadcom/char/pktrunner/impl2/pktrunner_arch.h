#ifndef PKTRUNNER_ARCH_H
#define PKTRUNNER_ARCH_H

#include <rdpa_api.h>
#include <rdpa_flow_idx_pool.h>

#if !defined(G9991_FC) && !defined(OPERATION_MODE_PRV)
#define CC_PKTRUNNER_WLAN
#endif

#ifndef CONFIG_BCM_ARCHER
#include "bcm_rdp_arch.h"
#endif /* CONFIG_BCM_ARCHER */

#define SIM_SET_LOG_LEVEL() do {\
    bcmLog_setLogLevel(BCM_LOG_ID_PKTRUNNER, BCM_LOG_LEVEL_DEBUG); \
    bcm_print("Initialized Runner Protocol Layer in SIMULATION MODE\n"); \
} while (0)

#if defined(RDP_ARCH_SIM)
#include "pktrunner_rdpa_sim.h"
#include "rdpa_blog.h"

#define fhwPktRunnerAddHostMac NULL
#define fhwPktRunnerDelHostMac NULL

#define disp_pool_alloc(X) kmalloc(X, GFP_KERNEL)
#define finish_init() SIM_SET_LOG_LEVEL()

static inline int __isEnetBondedLanWanPort(uint32_t logicalPort) { return 0; }
static inline rdpa_traffic_dir dpiqos_dir(Blog_t *blog_p) { return rdpa_dir_ds; }
#define lan2lan_dir rdpa_dir_us
#define debug_gdx_port(B)

#define CC_PKTRUNNER_MCAST

#define runnerHost_construct()
#define runnerHost_destruct()

#elif defined(RDP_ARCH_BOARD) || defined(CONFIG_BCM_ARCHER) || defined (RDP_ARCH_QEMU_SIM)
#include <linux/module.h>
#include <linux/bcm_log.h>
#include <linux/blog.h>
#include <net/ipv6.h>
#include <bcmdpi.h>
#include <bcmtypes.h>
#include <linux/bcm_netdevice.h>
#include "clk_rst.h"
#include "bcmxtmcfg.h"
#if (defined(XRDP) || defined(CONFIG_BCM_DSL_RDP)) && defined(CC_PKTRUNNER_WLAN)
#include "bcm_wlan_defs.h"
#endif /* XRDP */
#include "linux/bcm_skb_defines.h"
#include <port_platform.h>
#if defined(CONFIG_BCM_CMDLIST_SIM)
#define fhwPktRunnerAddHostMac NULL
#define fhwPktRunnerDelHostMac NULL
#else /* CONFIG_BCM_CMDLIST_SIM */
#include "pktrunner_host_common.h"
#define fhwPktRunnerAddHostMac pktrunner_system_add_host_mac
#define fhwPktRunnerDelHostMac pktrunner_system_delete_host_mac
#endif /* CONFIG_BCM_CMDLIST_SIM */

#define disp_pool_alloc(X) vmalloc(X)
#if !defined(CONFIG_BCM_CMDLIST_SIM) && defined(CONFIG_BCM_PMC)
#define finish_init() do { \
    unsigned int rdp_freq; \
    get_rdp_freq(&rdp_freq); \
    bcm_print("Initialized Runner Protocol Layer (%u)\n", rdp_freq); \
    } while (0)
#else /* CONFIG_BCM_PMC */
#define finish_init() SIM_SET_LOG_LEVEL()
#endif /* CONFIG_BCM_PMC */

/* Returns TRUE if LAN/SF2-Port is bonded with Runner WAN port */
static inline int __isEnetBondedLanWanPort(uint32_t logicalPort)
{
   int ret_val = FALSE ;

#if defined(CONFIG_BCM_KERNEL_BONDING) && !defined(CONFIG_BCM963158)
  bcmFun_t *enetFunc = bcmFun_get(BCM_FUN_ID_ENET_IS_BONDED_LAN_WAN_PORT);

  BCM_ASSERT(enetFunc != NULL);

  ret_val = enetFunc(&logicalPort);
#endif

   return (ret_val);
}

static inline rdpa_traffic_dir dpiqos_dir(Blog_t *blog_p) {
    struct net_device *dev = blog_p->rx_dev_p;
    return is_netdev_wan(dev) ? rdpa_dir_ds : rdpa_dir_us;
}

#define lan2lan_dir rdpa_dir_ds
#define debug_gdx_port(B) __logDebug("ucast: gdx_tx: port: %s\n", ((struct net_device*)B->rx_dev_p)->name)

#if !defined(WL4908_EAP)
#define CC_PKTRUNNER_MCAST
#endif

extern int __init runnerHost_construct(void);
extern void __exit runnerHost_destruct(void);

#else /* no RDP arch */
#error "no arch defined"
#endif /* RDP_ARCH_SIM */

#endif /* PKTRUNNER_ARCH_H */
