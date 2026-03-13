#ifndef PKTRUNNER_ARCH_H
#define PKTRUNNER_ARCH_H

#include "bcm_rdp_arch.h"

#if defined(RDP_ARCH_SIM)
#include "pktrunner_rdpa_sim.h"
#include "rdpa_blog.h"

#define blog_error(X, P)

static inline int blog_num_of_vtags(Blog_t *blog_p) { return 0; }
static inline uint32_t make_mcast_key_exclude_fields(void) { return rdpa_mcast_flow_key_exclude_etype_field; }
static inline void pktrunner_bind(int enable) {}

/* In simulation environment the Runner (i.e the simulator) is not running at this point
   On real board it l2_header_need_llc_snap() returns true except when,
   during debugging only, WiFi is configured to add llc internally.
*/
static inline int l2_header_need_llc_snap(uint8_t radio_idx) { return 1; }

#elif defined(RDP_ARCH_BOARD)
#include <bcmenet_common.h>
#include <linux/blog.h>
#include <linux/bcm_log.h>
#include <linux/blog_rule.h>
#include <linux/if.h>
#include "fcachehw.h"
#include "linux/bcm_skb_defines.h"
#include "idx_pool_util.h"
#include <bcmdpi.h>
#include "pktrunner_common.h"

#define blog_error(X, P) __logError(X, P ? ((struct net_device *) P)->name:"NULL")

static inline int blog_num_of_vtags(Blog_t *blog_p) {
    uint16_t etype;
    uint32_t vtag0, vtag1;

    return blog_hdr_get_vtags_from_encap(&blog_p->tx, &etype, &vtag0, &vtag1, 0);
}

static inline uint32_t make_mcast_key_exclude_fields(void) {
    return rdpa_mcast_flow_key_exclude_pbit_field |
        rdpa_mcast_flow_key_exclude_dei_field | rdpa_mcast_flow_key_exclude_etype_field; /* not supported yet in BLOG */

}

extern void pktrunner_bind(int enable);
extern int l2_header_need_llc_snap(uint8_t radio_idx);

#else /* no RDP arch */
#error "no arch defined"
#endif /* RDP_ARCH_SIM */

#endif /* PKTRUNNER_ARCH_H */
