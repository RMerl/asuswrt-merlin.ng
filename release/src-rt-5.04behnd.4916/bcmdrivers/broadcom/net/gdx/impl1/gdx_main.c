/*
   <:copyright-BRCM:2021:DUAL/GPL:standard
   
      Copyright (c) 2021 Broadcom 
      All Rights Reserved
   
   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:
   
      As a special exception, the copyright holders of this software give
      you permission to link this software with independent modules, and
      to copy and distribute the resulting executable under terms of your
      choice, provided that you also meet, for each linked independent
      module, the terms and conditions of the license of that module.
      An independent module is a module which is not derived from this
      software.  The special exception does not apply to any modifications
      of the software.
   
   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.
   
   :>
 */

/****************************************************************************/
/**                                                                        **/
/** Generic Device Accelerator (GDX) Driver                                **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   Generic device accelerator interface.                                **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   GDX driver functions common to both hardware and software            **/
/**   acceleration                                                         **/
/**                                                                        **/
/** Allocated requirements:                                                **/
/**                                                                        **/
/** Allocated resources:                                                   **/
/**                                                                        **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/

#include "gdx.h"

extern int  gdx_priv_init(uint32_t page_size, uint32_t max_skb_frags);
extern void gdx_priv_uninit(void);

int (*gdx_hw_accel_loopbk_fn)(struct sk_buff *skb, bool l3_packet) = NULL;

int gdx_print_lvl = GDX_PRINT_LVL_ERROR;

#if !defined(CONFIG_BCM_GDX_HW)
/* GDX HW Acceleration not supported. Declare dummy functions */
int gdx_hw_init(void)
{
    return 0;
}

void gdx_hw_uninit(void)
{
    return;
}
#else
extern int  gdx_hw_init(void);
extern void gdx_hw_uninit(void);
#endif

/*
 * Check if frag gather is supported in hardware accelerator 
 * for the input device.
 */
int gdx_is_hw_frag_gather_supported(void)
{
#if defined(CONFIG_BCM_RUNNER_FRAG_GATHER)
    return 1;
#endif
    return 0;
}

/*
 * Check if there is any difference in the netdev offset between gdx 
 * binary and the opensource. Helps detect any mismatch when patches 
 * are provided to customers. 
 * Note: The fields that are checked here are the ones that are 
 * accessed by the GDX binary.
 */
int gdx_netdev_offset_check(gdx_netdev_field_offset_t *netdev_field_offset_p)
{
    /* Compare the netdev field offsets between the
       opensource and GDX binary */
    GDX_CHECK_NETDEV_OFFSET_RET(netdev_field_offset_p, tx_dropped);
    GDX_CHECK_NETDEV_OFFSET_RET(netdev_field_offset_p, type);
    GDX_CHECK_NETDEVEXT_OFFSET_RET(netdev_field_offset_p, iff_flags);
    return 0;
}

/*
 * Check if there is any difference in the skb offset between gdx 
 * binary and the opensource. Helps detect any mismatch when patches 
 * are provided to customers. 
 * Note: The fields that are checked here are the ones that are 
 * accessed by the GDX binary.
 */
int gdx_skb_offset_check(gdx_skb_field_offset_t *skb_field_offset_p)
{
    /* Compare the skb field offsets between the
       opensource and GDX binary */
    GDX_CHECK_SKB_OFFSET_RET(skb_field_offset_p, data);
    GDX_CHECK_SKB_OFFSET_RET(skb_field_offset_p, __pkt_type_offset);
    GDX_CHECK_SKB_OFFSET_RET(skb_field_offset_p, protocol);
    GDX_CHECK_SKB_OFFSET_RET(skb_field_offset_p, head);
    GDX_CHECK_SKB_OFFSET_RET(skb_field_offset_p, transport_header);
    GDX_CHECK_SKB_OFFSET_RET(skb_field_offset_p, network_header);
    GDX_CHECK_SKB_OFFSET_RET(skb_field_offset_p, dev);
    GDX_CHECK_SKB_OFFSET_RET(skb_field_offset_p, bcm_ext);
    GDX_CHECK_SKBEXT_OFFSET_RET(skb_field_offset_p, flags);
    return 0;
}

/*
 * GDX driver initialization function
 */
static int gdx_init(void)
{
    /* First do the private initialization and check whether the skb and
       netdev offsets are ok before proceeding with other initialization */
    if (gdx_priv_init(PAGE_SIZE, MAX_SKB_FRAGS) != 0)
    {
        GDX_PRINT_ERROR("gdx_priv_init failure");
        return GDX_FAILURE;
    }

    if (gdx_hw_init() != 0)
    {
        GDX_PRINT_ERROR("gdx_hw_init failure");
        return GDX_FAILURE;
    }

    return 0;
}

/*
 * GDX driver uninitialization function
 */
static void gdx_uninit(void)
{
    gdx_hw_uninit();

    gdx_priv_uninit();
}

MODULE_DESCRIPTION("Generic Device Accelerator (GDX) Driver");
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("Proprietary");

module_init(gdx_init);
module_exit(gdx_uninit);
