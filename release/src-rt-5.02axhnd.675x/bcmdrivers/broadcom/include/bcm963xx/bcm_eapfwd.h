#if !defined(__bcm_eapfwd_h_included__)
#define __bcm_eapfwd_h_included__

#include "pktHdr.h"
#include <bcm_pktfwd.h>			/* BCM_PKTFWD && BCM_PKTLIST */

/**
 * =============================================================================
 * Section: Configuration
 * See bcm_pktfwd.h to change default configuration of abstract data types.
 * =============================================================================
 */

#define EAP_WLAN_RADIOS_MAX     (PKTFWD_DOMAINS_WLAN)
#if (EAP_WLAN_RADIOS_MAX < 2)
#error "Need a minimum of 2 WLAN radios"
#endif

#define EAP_STATIONS_MAX        (PKTFWD_ENDPOINTS_MAX)
#if (EAP_STATIONS_MAX < 256)
#error "Need a minimum of 256 WLAN stations per WLAN radio domain"
#endif

/**
 * =============================================================================
 * Section: Function declarations
 * =============================================================================
 */

int wl_eap_bind(void * wl, struct net_device * wl_dev, int _radio_idx,
                d3lut_t * lut, pktlist_context_t * wl_pktlist_context,
                HOOKP wl_pktfwd_xfer_hook);

int wl_eap_unbind(int _radio_idx);

extern void (*eap_receive_skb_hook)(struct sk_buff * skb);
extern void (*eap_xmit_schedule_hook)(void);

#endif /* __bcm_eapfwd_h_included__ */
