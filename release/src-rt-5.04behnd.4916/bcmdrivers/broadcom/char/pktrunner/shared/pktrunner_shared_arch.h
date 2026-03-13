#ifndef PKTRUNNER_SHARED_ARCH_H
#define PKTRUNNER_SHARED_ARCH_H

#ifndef CONFIG_BCM_ARCHER
#include "bcm_rdp_arch.h"
#endif /* CONFIG_BCM_ARCHER */

#if defined(RDP_ARCH_SIM)
#include "pktrunner_rdpa_sim.h"
#include "rdpa_blog.h"

#if defined(RDP_UFC)
#define CAN_GET_VTAGS_FROM_ENCAP
#endif /* RDP_UFC */

#define PKTRUNNER_LLC_SNAP PROTO_MAX /* no LLC_SNAP enum in SIM */

#elif defined(RDP_ARCH_BOARD) || defined(CONFIG_BCM_ARCHER) || defined (RDP_ARCH_QEMU_SIM)
#define CC_PKTRUNNER_PROCFS
#include <linux/seq_file.h>
#include <linux/module.h>

#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/blog_rule.h>

#include <bcmenet_common.h>
#include "linux/bcm_skb_defines.h"
#include <linux/bcm_log.h>
#include <linux/blog.h>
#include <net/ipv6.h>
#include <net/ip6_checksum.h>
#include "bcmtypes.h"
#include "bcm_vlan.h"
#include "pktHdr.h"

#if defined(CONFIG_BCM_FHW)
#include "fcachehw.h"
#else
#define FHW_TUPLE_INVALID BLOG_FLOW_HW_INVALID
#endif

#define CAN_GET_VTAGS_FROM_ENCAP

#define PKTRUNNER_LLC_SNAP LLC_SNAP

#else /* no RDP arch */
#error "no arch defined"
#endif /* RDP_ARCH_SIM */

#endif /* PKTRUNNER_SHARED_ARCH_H */
