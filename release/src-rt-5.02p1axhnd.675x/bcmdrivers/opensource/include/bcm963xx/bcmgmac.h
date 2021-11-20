/*
<:copyright-BRCM:2012:DUAL/GPL:standard

   Copyright (c) 2012 Broadcom 
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

/***********************************************************************/
/*                                                                     */
/*   file:  bcmgmac.h                                                  */
/*                                                                     */
/***********************************************************************/
#ifndef _BCM_GMAC_H_
#define _BCM_GMAC_H_

#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>

#define GMAC_VERSION            "v0.1"
#define GMAC_VER_STR            GMAC_VERSION
#define GMAC_MODNAME            "Broadcom GMAC"
#define GMAC_DRV_MAJOR          326 /* GMAC Character Device */

#define GMAC_ERROR              (-1)
#define GMAC_SUCCESS            0

#if (ENET_RX_CHANNELS_MAX != 2)
#error "ERROR - (ENET_RX_CHANNELS_MAX != 2)"
#endif

#define GMAC_PHY_CHAN           0
#define GMAC_LOG_CHAN           (ENET_RX_CHANNELS_MAX-1)
#define GMAC_ROBO_PHY_CHAN      0   //PKTDMA_ETH_DS_IUDMA
#define GMAC_ROBO_LOG_CHAN      GMAC_ROBO_PHY_CHAN


#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#define GMAC_LOCK_BH()              spin_lock_bh( &gmac_lock_g )
#define GMAC_UNLOCK_BH()            spin_unlock_bh( &gmac_lock_g )
#else
#define GMAC_LOCK_BH()              local_bh_disable()
#define GMAC_UNLOCK_BH()            local_bh_enable()
#endif

typedef struct gmac_info
{
    uint32_t enabled;               /* GMAC is enabled for this chip. 
                                       0 for 63268C0, 1 for 63268D0 */
    uint32_t active;                /* 1 when GMAC is the WAN device */
                                    /* 0 when ROBO port is the WAN device */
    uint32_t wan;                   /* Port eth3 is configured as WAN */
#define GMAC_MODE_ROBO_PORT     0   /* Always use ROBO, e.g. C0 part */
#define GMAC_MODE_LINK_SPEED    1   /* 1G->GMAC, 10/100M->ROBO port */
#define GMAC_MODE_DEF           GMAC_MODE_ROBO_PORT
    uint32_t mode;
    uint32_t link_speed;            /* Current link speed */
    uint32_t link_up;               /* Current link status */
    uint32_t duplex;                /* Current link duplex when speed is not 1G */
    uint32_t log_chan;              /* Logical Channel for RX and TX */
    uint32_t phy_chan;              /* Physical Channel for RX and TX */
    uint32_t trans;
    uint32_t chip_id;               
    uint32_t rev_id;                /* Chip revision id */
} gmac_info_t;

#define GMAC_RB_BP_THRESH_MAX       0x400
#define GMAC_RB_BP_THRESH_LO_DEF    0x200
#define GMAC_RB_BP_THRESH_HI_DEF    0x300

#define GMAC_MAX_FRM_LEN            1518
#define GMAC_MAX_JUMBO_FRM_LEN      ENET_MAX_MTU_SIZE

extern uint32_t gmac_print_ena_g;  
extern gmac_info_t gmac_info_g,  *gmac_info_pg;

extern void gmac_set_wan_port( int add );
extern int gmac_set_active( void );
extern int gmac_set_inactive( void );
extern void gmac_set_mode( int mode );
extern void gmac_link_status_changed(int link_status, int speed, int duplex);
extern void gmac_hw_stats( int port,  struct rtnl_link_stats64 *stats );
extern int gmac_dump_mib( int port, int type );
extern void gmac_reset_mib( void );
extern FN_HANDLER_RT bcm63xx_gmac_isr( int irq, void * dev_id );
extern int gmac_init( void );
extern int gmac_is_gmac_supported( void );
extern int gmac_is_gmac_port( int port );
extern void gmac_intf_set_max_pkt_size( int frm_len );
#endif /* _BCM_GMAC_H_ */

