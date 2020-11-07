/*
<:copyright-BRCM:2017:proprietary:standard

   Copyright (c) 2017 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*/

#include <linux/module.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/blog_rule.h>

#include "linux/bcm_skb_defines.h"
#include <linux/bcm_log.h>
#include <linux/blog.h>
#include "fcachehw.h"

#include "bcmtypes.h"
#include <rdpa_api.h>
#include "pktrunner_wlan.h"

#define MAX_NUM_OF_RADIOS 3

static int l2_llc_snap_value[MAX_NUM_OF_RADIOS]; /* -1 for not set, 0 for disable, 1 for enable */
static bdmf_object_handle dhd_helper[MAX_NUM_OF_RADIOS] = {};

void pktrunner_wlan_construct(void)
{
    int i;

    for (i = 0; i < MAX_NUM_OF_RADIOS; i++)
    {
        l2_llc_snap_value[i] = -1;
        dhd_helper[i] = NULL;
    }
}

void pktrunner_wlan_destruct(void)
{
    int i;

    for (i = 0; i < MAX_NUM_OF_RADIOS; i++)
    {
        if (dhd_helper[i])
            bdmf_put(dhd_helper[i]);
    }
}

int l2_header_need_llc_snap(uint8_t radio_idx)
{
    rdpa_dhd_init_cfg_t init_cfg;

    if (!dhd_helper[radio_idx])
    {
        if (rdpa_dhd_helper_get(radio_idx, &dhd_helper[radio_idx]))
            return 0; /* object doesn't exist */
        rdpa_dhd_helper_init_cfg_get(dhd_helper[radio_idx], &init_cfg);
        l2_llc_snap_value[radio_idx] = init_cfg.add_llcsnap_header ? 1 : 0;
    }
    return l2_llc_snap_value[radio_idx] != -1 ? l2_llc_snap_value[radio_idx] : 0;
}

/** LLCSNAP: OUI[2] setting for Bridge Tunnel (Apple ARP and Novell IPX) */
#define ETHER_TYPE_APPLE_ARP    0x80f3 /* Apple Address Resolution Protocol */
#define ETHER_TYPE_NOVELL_IPX   0x8137 /* Novel IPX Protocol */

#define BRIDGE_TUNNEL_OUI2      0xf8 /* OUI[2] value for Bridge Tunnel */

#define IS_BRIDGE_TUNNEL(et) \
	(((et) == ETHER_TYPE_APPLE_ARP) || ((et) == ETHER_TYPE_NOVELL_IPX))

/* Copy 14B ethernet header: 32bit aligned source and destination. */
#define edasacopy32(d, s) \
do { \
	((uint32 *)(d))[0] = ((const uint32 *)(s))[0]; \
	((uint32 *)(d))[1] = ((const uint32 *)(s))[1]; \
	((uint32 *)(d))[2] = ((const uint32 *)(s))[2]; \
} while (0)

static const union {
    uint32 u32;
	uint16 u16[2];
    char u8[4];
} _ctl_oui3 = { .u8 = {0x00, 0x00, 0x00, 0x03} };

void l2_header_insert_llc_snap(uint8_t *l2_header, Blog_t *blog_p)
{
    edasacopy32(l2_header , l2_header + L2_DOT11_LLC_SNAP_HDR_LEN);
    /* LLC ctl = 0x03, out[3] = { 0x00 0x00 0x00}: 32b aligned 4B copy */
    ((uint32 *)(l2_header))[4] = htonl(_ctl_oui3.u32);

	/* ethernet payload length: 2B. Will be updated by Runner FW from packet */
	((uint16 *)(l2_header))[6] = 0; /* */

	((uint16 *)(l2_header))[7] = (uint16)0xAAAA; /* no need for htons16 */

	/* Set OUI[2] for Bridge Tunnel */
	if (IS_BRIDGE_TUNNEL(blog_p->eth_type))
        ((uint8_t *)(l2_header))[15] = BRIDGE_TUNNEL_OUI2;
}

