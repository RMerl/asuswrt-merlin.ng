#ifndef RDPA_MW_ARCH_H
#define RDPA_MW_ARCH_H

#include "bcm_rdp_arch.h"

#if defined(RDP_ARCH_SIM)
#include "pktrunner_rdpa_sim.h"

struct net_device;
static inline bdmf_object_handle rdpa_mw_get_port_object_by_dev_dir(struct net_device *dev,
                                                      uint32_t dir)
{
    return (bdmf_object_handle) dev;
}

static inline uint8_t rdpa_mw_root_dev2rdpa_ssid(struct net_device *root_dev) { return 0; }

#elif defined(RDP_ARCH_BOARD) || defined(RDP_ARCH_QEMU_SIM)
#include "bcm_OS_Deps.h"
#include "bcm_wlan_defs.h"
#include <linux/blog.h>
#include <linux/bcm_log.h>
#include <linux/blog_rule.h>
#include "rdpa_api.h"

extern bdmf_object_handle rdpa_mw_get_port_object_by_dev_dir(struct net_device *dev,
                                                      uint32_t dir);
extern uint8_t rdpa_mw_root_dev2rdpa_ssid(struct net_device *root_dev);

#else /* no RDP arch */
#error "no arch defined"
#endif /* RDP_ARCH_SIM */

#endif /* RDPA_MW_ARCH_H */
