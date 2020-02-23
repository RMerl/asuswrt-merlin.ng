/*
             printf("gmac_hw_stats(port=%d, stats=0x%x)\n", port, stats);
    Copyright 2000-2010 Broadcom Corporation

    <:label-BRCM:2011:DUAL/GPL:standard
    
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


#define _BCMENET_LOCAL_

#include "bcm_OS_Deps.h"
#include "board.h"
#include "spidevices.h"
#include <bcm_map_part.h>
#include "bcm_intr.h"
#include "bcmenet.h"
#include "bcmmii.h"
#include "ethsw_phy.h"
#include "ethsw.h"
#include "eth_pwrmngt.h"
#include "bcmswdefs.h"
#include "bcmsw.h"
/* Exports for other drivers */
#include "bcmsw_api.h"
#include "bcmswshared.h"
#include "bcmPktDma_defines.h"
#include "boardparms.h"
#include "bcmPktDma.h"
#include "shared_utils.h"

#if defined(CONFIG_BCM_GMAC)
#include "bcmgmac.h"
#endif

#if defined(CONFIG_BCM947189)
#include "bcmgmac_47189.h"

int  ethsw_save_port_state(void){return 0;};
int  ethsw_restore_port_state(void){return 0;};
void bcmeapi_ethsw_dump_page(int page){};
int  bcmeapi_ethsw_dump_mib(int port, int type, int queue){gmac_dump_mib(port, type); return 0;};
int  bcmeapi_ioctl_ethsw_port_tagreplace(struct ethswctl_data *e){return 0;};
int  bcmeapi_ioctl_ethsw_port_tagmangle(struct ethswctl_data *e){return 0;};
int  bcmeapi_ioctl_ethsw_port_tagmangle_matchvid(struct ethswctl_data *e){return 0;};
int  bcmeapi_ioctl_ethsw_port_tagstrip(struct ethswctl_data *e){return 0;};
int  bcmeapi_ioctl_ethsw_control(struct ethswctl_data *e){return 0;};
int  bcmeapi_ioctl_ethsw_prio_control(struct ethswctl_data *e){return 0;};
int  bcmeapi_ioctl_ethsw_vlan(struct ethswctl_data *e){return 0;};
int  bcmeapi_ioctl_ethsw_pbvlan(struct ethswctl_data *e){return 0;};
int  bcmeapi_ioctl_ethsw_port_irc_set(struct ethswctl_data *e){return 0;};
int  bcmeapi_ioctl_ethsw_port_irc_get(struct ethswctl_data *e){return 0;};
int  bcmeapi_ioctl_ethsw_port_erc_set(struct ethswctl_data *e){return 0;};
int  bcmeapi_ioctl_ethsw_port_erc_get(struct ethswctl_data *e){return 0;};
int  bcmeapi_ioctl_ethsw_cosq_config(struct ethswctl_data *e){return 0;};
int  bcmeapi_ioctl_ethsw_cosq_sched(struct ethswctl_data *e){return 0;};
int  bcmeapi_ioctl_ethsw_cosq_port_mapping(struct ethswctl_data *e){return 0;};
void enet_park_rxdma_channel(int parkChan, int unpark){};
void bcmeapi_ethsw_cosq_rxchannel_mapping(int *channel, int queue, int set){};
int  bcmeapi_ioctl_ethsw_cosq_rxchannel_mapping(struct ethswctl_data *e){return 0;};
int  bcmeapi_ioctl_ethsw_cosq_txchannel_mapping(struct ethswctl_data *e){return 0;};
int  bcmeapi_ioctl_ethsw_cosq_txq_sel(struct ethswctl_data *e){return 0;};
int  bcmeapi_ioctl_ethsw_clear_port_stats(struct ethswctl_data *e){return 0;};
int  bcmeapi_ioctl_ethsw_clear_stats(uint32_t port_map){return 0;};
int  bcmeapi_ioctl_ethsw_counter_get(struct ethswctl_data *e){return 0;};
int  bcmeapi_ioctl_ethsw_port_default_tag_config(struct ethswctl_data *e){return 0;};
int  bcmeapi_ioctl_ethsw_port_loopback(struct ethswctl_data *e, int phy_id){return 0;};
int  bcmeapi_ioctl_ethsw_pkt_padding(struct ethswctl_data *e){return 0;};
int  bcmeapi_ioctl_ethsw_port_jumbo_control(struct ethswctl_data *e){return 0;}; // bill
void fast_age_all(uint8_t age_static){};
int bcmeapi_ioctl_ethsw_dscp_to_priority_mapping(struct ethswctl_data *e){return 0;};
int bcmeapi_ioctl_ethsw_pcp_to_priority_mapping(struct ethswctl_data *e){return 0;};
void ethsw_set_wanoe_portmap(uint16 wan_port_map){};
int bcmeapi_ethsw_init(void){return 0;};
void bcmeapi_ethsw_init_ports(void){};
void bcmeapi_ethsw_init_hw(int unit, uint32_t map, int wanPort){};
void bcmeapi_set_multiport_address(uint8_t* addr){};
uint16_t ethsw_get_pbvlan(int port){return 0;};
void ethsw_set_pbvlan(int port, uint16_t fwdMap){};

#if (defined(CONFIG_BCM_ARL) || defined(CONFIG_BCM_ARL_MODULE))
int enet_hook_for_arl_access(void *ethswctl){return 0;};
#endif
void bcmeapi_link_check(int log_port, int link, int speed){};
int bcmeapi_ioctl_debug_conf(struct ethswctl_data *e){return 0;};
int write_vlan_table(bcm_vlan_t vid, uint32_t val32){return 0;};
void enet_arl_write(uint8_t *mac, uint16_t vid, int val){};
void bcmeapi_ethsw_set_stp_mode(unsigned int unit, unsigned int port, unsigned char stpState){};

int enet_arl_search(u8 *mac, u32 *vid, u32 *val, int op ){return 0;};
void enet_arl_dump_multiport_arl(void){};
int bcmeapi_init_ext_sw_if(extsw_info_t *extSwInfo){return 0;};
void ephy_adjust_afe(unsigned int phy_id){};
void bcmeapi_update_link_status(void){};
int ethsw_counter_collect(uint32_t portmap, int discard){return 0;};
void bcmeapi_reset_mib(void){gmac_reset_mib(0);gmac_reset_mib(1);};
void fast_age_port(uint8_t port, uint8_t age_static){};
#else

#ifndef SINGLE_CHANNEL_TX
/* for enet driver txdma channel selection logic */
extern int channel_for_queue[NUM_EGRESS_QUEUES];
/* for enet driver txdma channel selection logic */
extern int use_tx_dma_channel_for_priority;
#endif /*SINGLE_CHANNEL_TX*/

extern struct semaphore bcm_ethlock_switch_config;
#if defined(CONFIG_BCM_ETH_PWRSAVE)
extern spinlock_t bcmsw_pll_control_lock;
#endif

#if defined(CONFIG_BCM_GMAC)
void gmac_hw_stats( int port,  struct rtnl_link_stats64 *stats );
int gmac_dump_mib(int port, int type);
void gmac_reset_mib( void );
#endif

#define ALL_PORTS_MASK                     0x1FF
#define ONE_TO_ONE_MAP                     0x00FAC688
#define MOCA_QUEUE_MAP                     0x0091B492
#define DEFAULT_FC_CTRL_VAL                0x1F
/* Tx: 0->0, 1->1, 2->2, 3->3. */
#define DEFAULT_IUDMA_QUEUE_SEL            0x688

static uint8_t  swpkt_ctrl_usb_saved = 0;
static uint32_t swpkt_ctrl_usb;
extern uint8_t port_in_loopback_mode[TOTAL_SWITCH_PORTS];
extern uint32_t imp_pbmap[];

extern BcmEnet_devctrl *pVnetDev0_g;

#if defined(CONFIG_BCM963268)
int remove_arls_on_port_5398_workaround(uint8_t port, uint8_t age_static);
#endif
#endif

extern spinlock_t bcm_extsw_access;

/****************************************/
/* Hardware Access Functions            */
/****************************************/
/* BCM5325E PSEUDO PHY register access through MDC/MDIO */
/* When reading or writing PSEUDO PHY registers, we must use the exact starting address and
   exact length for each register as defined in the data sheet.  In other words, for example,
   dividing a 32-bit register read into two 16-bit reads will produce wrong result.  Neither
   can we start read/write from the middle of a register.  Yet another bad example is trying
   to read a 32-bit register as a 48-bit one.  This is very important!!
*/
void bcmsw_pmdio_rreg(int page, int reg, uint8 *data, int len)
{
    uint16 v;
    int i;

    BCM_ENET_LINK_DEBUG("read op; page = %x; reg = %x; len = %d\n", 
        (unsigned int) page, (unsigned int) reg, len);
    
    spin_lock_bh(&bcm_extsw_access);
    
    v = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT) | REG_PPM_REG16_MDIO_ENABLE;
    ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG16, &v);

    v = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_READ;
    ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, &v);

    for (i = 0; i < 20; i++) {
        ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, &v);
        if ((v & (REG_PPM_REG17_OP_WRITE | REG_PPM_REG17_OP_READ)) == REG_PPM_REG17_OP_DONE)
            break;
        udelay(10);
    }

    if (i >= 20) {
        printk("bcmsw_mdio_rreg: timeout!\n");
        spin_unlock_bh(&bcm_extsw_access);
        return;
    }

    switch (len) {
        case 1:
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
            data[0] = (uint8)v;
            break;
        case 2:
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
#if defined(CONFIG_BCM947189)
            ((uint16 *)data)[0] = (uint16)v;
#else
            ((uint16 *)data)[0] = swab16((uint16)v);
#endif
            break;
        case 4:
#if defined(CONFIG_BCM947189)
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &v);
            ((uint16 *)data)[1] = (uint16)v;
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
            ((uint16 *)data)[0] = (uint16)v;            
#else
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &v);
            ((uint16 *)data)[0] = (uint16)v;
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
            ((uint16 *)data)[1] = (uint16)v;
            ((uint32 *)data)[0] = swab32(((uint32 *)data)[0]);            
#endif
            break;
        case 6:
#if defined(CONFIG_BCM947189)
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG26, &v);
            ((uint16 *)data)[2] = (uint16)v;
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &v);
            ((uint16 *)data)[1] = (uint16)v;
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
            ((uint16 *)data)[0] = (uint16)v;
#else
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG26, &v);
            ((uint16 *)data)[0] = (uint16)v;
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &v);
            ((uint16 *)data)[1] = (uint16)v;
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
            ((uint16 *)data)[2] = (uint16)v;

            // pmdio does not come from LE hosts. Ok, not being endian agnostic. 
            *(uint16 *)data = swab16(*(uint16 *)data);
            *(uint16 *)(data + 2) = swab16(*(uint16 *)(data + 2));
            *(uint16 *)(data + 4) = swab16(*(uint16 *)(data + 4));
            // swap half-words at the ends
            *((uint16 *)(data + 0)) ^= *((uint16 *)(data + 4));
            *((uint16 *)(data + 4)) ^= *((uint16 *)(data + 0));
            *((uint16 *)(data + 0)) ^= *((uint16 *)(data + 4));
#endif               
            break;
        case 8:
             /* 
               MDIO or SPI should return the same byte-order
               The caller expect it to be returned as 08:07:06:05:04:03:02:01
               Earlier code (prior to JIRA#12013 was returning word swapped - as below :
               MDIO : 04:03:02:01:08:07:06:05
              */
#if defined(CONFIG_BCM947189)
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG27, &v);
            ((uint16 *)data)[3] = (uint16)v;
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG26, &v);
            ((uint16 *)data)[2] = (uint16)v;
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &v);
            ((uint16 *)data)[1] = (uint16)v;
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
            ((uint16 *)data)[0] = (uint16)v;
#else
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG27, &v);
            ((uint16 *)data)[2] = (uint16)v;
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG26, &v);
            ((uint16 *)data)[3] = (uint16)v;
            ((uint32 *)data)[1] = swab32(((uint32 *)data)[1]);
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &v);
            ((uint16 *)data)[0] = (uint16)v;
            ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
            ((uint16 *)data)[1] = (uint16)v;
            ((uint32 *)data)[0] = swab32(((uint32 *)data)[0]);
#endif
            break;
    }

    BCM_ENET_LINK_DEBUG("read data = %02x %02x %02x %02x\n", 
        data[0], data[1], data[2], data[3]);

    spin_unlock_bh(&bcm_extsw_access);
}
void bcmsw_pmdio_wreg(int page, int reg, uint8 *data, int len)
{
    uint16 v;
    int i;

    BCM_ENET_LINK_DEBUG("write op; page = %x; reg = %x; len = %d\n", 
        (unsigned int) page, (unsigned int) reg, len);
    BCM_ENET_LINK_DEBUG("given data = %02x %02x %02x %02x\n", 
        data[0], data[1], data[2], data[3]);

    spin_lock_bh(&bcm_extsw_access);
    v = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT) | REG_PPM_REG16_MDIO_ENABLE;
    ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG16, &v);

    switch (len) {
        case 1:
            v = data[0];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
            break;
        case 2:
#if defined(CONFIG_BCM947189)
            v = ((uint16 *)data)[0];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
#else
            v = swab16(((uint16 *)data)[0]);
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
#endif
            break;
        case 4:
#if defined(CONFIG_BCM947189)
            v = ((uint16 *)data)[1];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &v);
            v = ((uint16 *)data)[0];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
#else
            ((uint32 *)data)[0] = swab32(((uint32 *)data)[0]);
            v = ((uint16 *)data)[0];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &v);
            v = ((uint16 *)data)[1];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
#endif
            break;
        case 6:
#if defined(CONFIG_BCM947189)
            v = ((uint16 *)data)[2];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG26, &v);
            v = ((uint16 *)data)[1];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &v);
            v = ((uint16 *)data)[0];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
#else
            // pmdio does not come from LE hosts. Ok, not being endian agnostic. 
            *(uint16 *)data = swab16(*(uint16 *)data);
            *(uint16 *)(data + 2) = swab16(*(uint16 *)(data + 2));
            *(uint16 *)(data + 4) = swab16(*(uint16 *)(data + 4));
            // swap half-words at the ends
            *((uint16 *)(data + 0)) ^= *((uint16 *)(data + 4));
            *((uint16 *)(data + 4)) ^= *((uint16 *)(data + 0));
            *((uint16 *)(data + 0)) ^= *((uint16 *)(data + 4));
            v = ((uint16 *)data)[0];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG26, &v);
            v = ((uint16 *)data)[1];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &v);
            v = ((uint16 *)data)[2];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
#endif
            break;
        case 8:
             /*
               The caller gives the value as 08:07:06:05:04:03:02:01(b0:..:b63)
               Earlier code (prior to JIRA#12013 was writing as word swapped (as below):
               MDIO : 04:03:02:01:08:07:06:05
              */
#if defined(CONFIG_BCM947189)
            v = ((uint16 *)data)[0];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
            v = ((uint16 *)data)[1];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &v);
            v = ((uint16 *)data)[2];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG26, &v);
            v = ((uint16 *)data)[3];
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG27, &v);
#else
            v = swab16(*(uint16 *)&data[0]);
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, &v);
            v = swab16(*(uint16 *)&data[2]);
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, &v);
            v = swab16(*(uint16 *)&data[4]);
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG26, &v);
            v = swab16(*(uint16 *)&data[6]);
            ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG27, &v);
#endif
            break;
    }

    v = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_WRITE;
    ethsw_phy_wreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, &v);

    for (i = 0; i < 20; i++) {
        ethsw_phy_rreg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, &v);
        if ((v & (REG_PPM_REG17_OP_WRITE | REG_PPM_REG17_OP_READ)) == REG_PPM_REG17_OP_DONE)
            break;
        udelay(10);
    }

    spin_unlock_bh(&bcm_extsw_access);

    if (i >= 20)
        printk("ethsw_mdio_wreg: timeout!\n");
}

#if !defined(CONFIG_BCM947189)
/*******************/
/* Local functions */
/*******************/
static void fast_age_start_done(uint8_t ctrl)
{
    uint8_t timeout = 100;

    ethsw_wreg(PAGE_CONTROL, REG_FAST_AGING_CTRL, (uint8_t *)&ctrl, 1);
    ethsw_rreg(PAGE_CONTROL, REG_FAST_AGING_CTRL, (uint8_t *)&ctrl, 1);
    while (ctrl & FAST_AGE_START_DONE) {
        mdelay(1);
        ethsw_rreg(PAGE_CONTROL, REG_FAST_AGING_CTRL, (uint8_t *)&ctrl, 1);
        if (!timeout--) {
            printk("Timeout of fast aging\n");
            break;
        }
    }

    /* Restore Dynamic for noraml aging */
    ctrl = FAST_AGE_DYNAMIC;
    ethsw_wreg(PAGE_CONTROL, REG_FAST_AGING_CTRL, (uint8_t *)&ctrl, 1);
}

void fast_age_all(uint8_t age_static)
{
    uint8_t v8;

    v8 = FAST_AGE_START_DONE | FAST_AGE_DYNAMIC;
  if (age_static) {
        v8 |= FAST_AGE_STATIC;
    }

    fast_age_start_done(v8);
}

void fast_age_port(uint8_t port, uint8_t age_static)
{
    uint8_t v8;

    v8 = FAST_AGE_START_DONE | FAST_AGE_DYNAMIC | FAST_AGE_PORT;
    if (age_static) {
        v8 |= FAST_AGE_STATIC;
    }

    ethsw_wreg(PAGE_CONTROL, REG_FAST_AGING_PORT, &port, 1);
    fast_age_start_done(v8);
#if defined(CONFIG_BCM963268)
    remove_arls_on_port_5398_workaround(port, age_static);
#endif
}

static int read_vlan_table(bcm_vlan_t vid, uint32_t *val32)
{
    uint8_t val8;
    int i, timeout = 200;

    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_INDX, (uint8_t *)&vid, 2);
    val8 = 0x81;
    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL, (uint8_t *)&val8, 1);

    for (i = 0; i < timeout; i++) {
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL, (uint8_t *)&val8, 1);
        if (((val8) & 0x80) == 0) {
            ethsw_rreg(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_ENTRY,
                       (uint8_t *)val32, 4);
            return 0;
        }
        udelay(100);
    }

    printk("Timeout reading VLAN table\n");
    return BCM_E_ERROR;
}

int write_vlan_table(bcm_vlan_t vid, uint32_t val32)
{
    uint8_t val8;
    int i, timeout = 200;

    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_INDX, (uint8_t *)&vid, 2);
    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_ENTRY, (uint8_t *)&val32, 4);
    val8 = 0x80;
    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL, (uint8_t *)&val8, 1);

    for (i = 0; i < timeout; i++) {
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL, (uint8_t *)&val8, 1);
        if (((val8) & 0x80) == 0) {
            return 0;
        }
        udelay(100);
    }

    printk("Timeout writing to VLAN table\n");
    return BCM_E_ERROR;
}


void ethsw_set_wanoe_portmap(uint16 wan_port_map)
{
    int i;
    uint16 map;
    BCM_ENET_DEBUG("wanoe port map = 0x%x", wan_port_map);

    down(&bcm_ethlock_switch_config);
    wan_port_map |= GET_PORTMAP_FROM_LOGICAL_PORTMAP(pVnetDev0_g->softSwitchingMap,0);

    /* Set WANoE port map */
    map = wan_port_map | EN_MAN_TO_WAN;
    ethsw_wreg(PAGE_CONTROL, REG_WAN_PORT_MAP, (uint8 *)&map, 2);

    /* Disable learning */
    map = imp_pbmap[0] | wan_port_map ;
    map |= GET_PORTMAP_FROM_LOGICAL_PORTMAP(pVnetDev0_g->learningDisabledPortMap, 0);
    ethsw_wreg(PAGE_CONTROL, REG_DISABLE_LEARNING, (uint8 *)&map, 2);

    for(i=0; i < TOTAL_SWITCH_PORTS; i++) {
       if((wan_port_map >> i) & 0x1) {
            fast_age_port(i, 0);
       }
    }

    up(&bcm_ethlock_switch_config);
}

/************************/
/* Ethernet Switch APIs */
/************************/


int bcmeapi_ioctl_ethsw_control(struct ethswctl_data *e)
{
    uint16_t val16;
    uint8_t val8;
    unsigned int val;

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        switch (e->sw_ctrl_type) {
            case bcmSwitchBufferControl:
                ethsw_rreg(PAGE_FLOW_CTRL, REG_FC_CTRL, (uint8_t *)&val16, 2);
                BCM_ENET_DEBUG("FC_CTRL = %4x", val16);
                e->val = val16 & QOS_PAUSE_DROP_EN_MAP;
                BCM_ENET_DEBUG("e->val is = %4x", e->val);
                break;

            case bcmSwitch8021QControl:
                /* Read the 802.1Q control register */
                ethsw_rreg(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_8021Q, &val8, 1);
                val = (val8 >> VLAN_EN_8021Q_S) & VLAN_EN_8021Q_M;
                if (val && ((val8 >> VLAN_IVL_SVL_S) & VLAN_IVL_SVL_M))
                    val = 2; // IVL mode
                e->val = val;
                BCM_ENET_DEBUG("e->val is = %4x", e->val);
                break;

            default:
                up(&bcm_ethlock_switch_config);
                return BCM_E_PARAM;
                break;
        }
    } else {
        switch (e->sw_ctrl_type) {
            case bcmSwitchBufferControl:
                /* Read the Pause/Drop control register */
                ethsw_rreg(PAGE_FLOW_CTRL, REG_FC_CTRL, (uint8_t *)&val16, 2);
                /* Modify the Pause/Drop control register as requested*/
                val16 &= ~QOS_PAUSE_DROP_EN_MAP;
                val16 |= (e->val & QOS_PAUSE_DROP_EN_MAP);
                BCM_ENET_DEBUG("Write FC_CTRL = %4x", val16);
                ethsw_wreg(PAGE_FLOW_CTRL, REG_FC_CTRL, (uint8_t *)&val16, 2);
                break;

            case bcmSwitch8021QControl:
                /* Read the 802.1Q control register */
                ethsw_rreg(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_8021Q, &val8, 1);
                /* Enable/Disable the 802.1Q */
                if (e->val == 0)
                    val8 &= (~(VLAN_EN_8021Q_M << VLAN_EN_8021Q_S));
                else {
                    val8 |= VLAN_EN_8021Q_M << VLAN_EN_8021Q_S;
                    if (e->val == 1) // SVL
                        val8 &= (~(VLAN_IVL_SVL_M << VLAN_IVL_SVL_S));
                    else if (e->val == 2) // IVL
                        val8 |= (VLAN_IVL_SVL_M << VLAN_IVL_SVL_S);
                }
                ethsw_wreg(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_8021Q, &val8, 1);
                break;

            default:
                up(&bcm_ethlock_switch_config);
                return BCM_E_PARAM;
                break;
        }
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

int bcmeapi_ioctl_ethsw_prio_control(struct ethswctl_data *e)
{
    uint16_t val16;
    int reg;

    BCM_ENET_DEBUG("e->priority = %2d", e->priority);
    if (e->priority > MAX_PRIORITY_VALUE) {
        printk("Invalid Priority\n");
        return BCM_E_ERROR;
    }

    /* */
    switch (e->sw_ctrl_type) {
        case bcmSwitchTxQHiHysteresisThreshold:
            reg = REG_FC_PRIQ_HYST + (e->priority * 2);
            break;
        case bcmSwitchTxQHiPauseThreshold:
            reg = REG_FC_PRIQ_PAUSE + (e->priority * 2);
            break;
        case bcmSwitchTxQHiDropThreshold:
            reg = REG_FC_PRIQ_DROP + (e->priority * 2);
            break;
        case bcmSwitchTotalHysteresisThreshold:
            reg = REG_FC_PRIQ_TOTAL_HYST + (e->priority * 2);
            break;
        case bcmSwitchTotalPauseThreshold:
            reg = REG_FC_PRIQ_TOTAL_PAUSE + (e->priority * 2);
            break;
        case bcmSwitchTotalDropThreshold:
            reg = REG_FC_PRIQ_TOTAL_DROP + (e->priority * 2);
            break;
        default:
            printk("unknown threshold type\n");
            return BCM_E_ERROR;
    }

    BCM_ENET_DEBUG("Threshold register offset = %4x", reg);

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        ethsw_rreg(PAGE_FLOW_CTRL, reg, (uint8_t *)&val16, 2);
        BCM_ENET_DEBUG("Threshold read = %4x", val16);
        e->ret_val = val16;
        BCM_ENET_DEBUG("e->ret_val is = %4x", e->ret_val);
    } else {
       val16 = (uint32_t)e->val;
       BCM_ENET_DEBUG("e->val is = %4x", e->val);
       ethsw_wreg(PAGE_FLOW_CTRL, reg, (uint8_t *)&val16, 2);
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

int bcmeapi_ioctl_ethsw_vlan(struct ethswctl_data *e)
{
    bcm_vlan_t vid;
    uint32_t val32, tmp;

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        vid = e->vid & BCM_NET_VLAN_VID_M;
        if (read_vlan_table(vid, &val32)) {
            up(&bcm_ethlock_switch_config);
            printk("VLAN Table Read Failed\n");
            return BCM_E_ERROR;
        }
        BCM_ENET_DEBUG("Combined fwd and untag map: 0x%08x\n",
                       (unsigned int)val32);
        tmp = val32 & VLAN_FWD_MAP_M;
        e->fwd_map = tmp;
        tmp = (val32 >> VLAN_UNTAG_MAP_S) & VLAN_UNTAG_MAP_M;
        e->untag_map = tmp;
    } else {
        vid = e->vid & BCM_NET_VLAN_VID_M;
        val32 = e->fwd_map | (e->untag_map << TOTAL_SWITCH_PORTS);
        BCM_ENET_DEBUG("VLAN_ID = %4d; fwd_map = 0x%04x; ", vid, e->fwd_map);
        BCM_ENET_DEBUG("untag_map = 0x%04x\n", e->untag_map);
        if (write_vlan_table(vid, val32)) {
            up(&bcm_ethlock_switch_config);
            printk("VLAN Table Write Failed\n");
            return BCM_E_ERROR;
        }
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}
uint16_t ethsw_get_pbvlan(int port)
{
    uint16_t val16;

    ethsw_rreg(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (port * 2),
               (uint8_t *)&val16, 2);
    return val16;
}
void ethsw_set_pbvlan(int port, uint16_t fwdMap)
{
    ethsw_wreg(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (port * 2),
               (uint8_t *)&fwdMap, 2);
}

int bcmeapi_ioctl_ethsw_pbvlan(struct ethswctl_data *e)
{
    uint16_t val16;

    BCM_ENET_DEBUG("Given Port: 0x%02x\n ", e->port);
    if (e->port >= TOTAL_SWITCH_PORTS) {
        printk("Invalid Switch Port\n");
        return BCM_E_ERROR;
    }

    if (e->type == TYPE_GET) {
        down(&bcm_ethlock_switch_config);
        val16 = ethsw_get_pbvlan(e->port); 
        up(&bcm_ethlock_switch_config);
        BCM_ENET_DEBUG("Threshold read = %4x", val16);
        e->fwd_map = val16;
        BCM_ENET_DEBUG("e->fwd_map is = %4x", e->fwd_map);
    } else {
        val16 = (uint32_t)e->fwd_map;
        BCM_ENET_DEBUG("e->fwd_map is = %4x", e->fwd_map);
        down(&bcm_ethlock_switch_config);
        ethsw_set_pbvlan(e->port, val16);
        up(&bcm_ethlock_switch_config);
    }

    return 0;
}

static int robo_ingress_rate_init_flag = 0;
#define IRC_PKT_MASK    0x3f
/*
 *  Function : bcmeapi_ioctl_ethsw_port_irc_set
 *
 *  Purpose :
 *  Set the burst size and rate limit value of the selected port ingress rate.
 *
 *  Parameters :
 *      unit   :   unit id
 *      port   :   port id.
 *      limit  :   rate limit value. (Kbits)
 *      burst_size  :   max burst size. (Kbits)
 *
 *  Return :
 *
 *  Note :
 *      Set the limit and burst size to bucket 0(storm control use bucket1).
 *
 */
int bcmeapi_ioctl_ethsw_port_irc_set(struct ethswctl_data *e)
{
    uint32_t  reg_value, burst_kbyte = 0, temp = 0;
    int rv = BCM_E_NONE;

    down(&bcm_ethlock_switch_config);

    /* Enable XLEN_EN bit to include IPG for rate limiting */
    ethsw_rreg(PAGE_BSS, REG_BSS_IRC_CONFIG, (uint8_t *)&reg_value, 4);
    reg_value |= (1 << IRC_CFG_XLENEN);
    ethsw_wreg(PAGE_BSS, REG_BSS_IRC_CONFIG, (uint8_t *)&reg_value, 4);

    /* Read the current Ingress Rate Config of the given port */
    ethsw_rreg(PAGE_BSS, REG_BSS_RX_RATE_CTRL_P0 + (e->port * 4),
               (uint8_t *)&reg_value, 4);

    if (e->limit == 0) { /* Disable ingress rate control */
         /* Disable ingress rate control
          *    - ING_RC_ENf can't be set as 0, it will stop this port's storm
          *       control rate also.
          *    - to prevent the affecting on other ports' ingress rate cotrol,
          *       global ingress rate setting is not allowed been modified on
          *       trying to disable this port's ingress rate control also.
          *    - set the REF_CNT to the MAX value means packets could
          *       be forwarded by no limit rate. (set to 0 will block all this
          *       port's traffic)
          */
         reg_value &= ~(IRC_BKT0_RATE_CNT_M << IRC_BKT0_RATE_CNT_S);
         reg_value |= 254 << IRC_BKT0_RATE_CNT_S;
    } else {    /* Enable ingress rate control */
        /* if not, config the pkt_type_mask for ingress rate control */
        if (!robo_ingress_rate_init_flag) {
            ethsw_rreg(PAGE_BSS, REG_BSS_IRC_CONFIG, (uint8_t *)&temp, 4);
            /* Enable bucket0 rate limiting for all types of traffic */
            temp |= IRC_PKT_MASK;
            /* Extended packet mask: SA lookup fail */
            temp |= 1 << IRC_CFG_PKT_MSK0_EXT_S;
            ethsw_wreg(PAGE_BSS, REG_BSS_IRC_CONFIG, (uint8_t *)&temp, 4);
            robo_ingress_rate_init_flag = 1;
        }

        burst_kbyte = e->burst_size / 8;
        if (e->burst_size > (500 * 8)) { /* 500 KB */
            return BCM_E_PARAM;
        }
        if (burst_kbyte <= 16) { /* 16KB */
            temp = 0;
        } else if (burst_kbyte <= 20) { /* 20KB */
            temp = 1;
        } else if (burst_kbyte <= 28) { /* 28KB */
            temp = 2;
        } else if (burst_kbyte <= 40) { /* 40KB */
            temp = 3;
        } else if (burst_kbyte <= 76) { /* 76KB */
            temp = 4;
        } else if (burst_kbyte <= 140){ /* 140KB */
            temp = 5;
        } else if (burst_kbyte <= 268){ /* 268KB */
            temp = 6;
        } else if (burst_kbyte <= 500){ /* 500KB */
            temp = 7;
        }

        reg_value &= ~(IRC_BKT0_SIZE_M << IRC_BKT0_SIZE_S);
        reg_value |= temp << IRC_BKT0_SIZE_S;

        /* refresh count  (fixed type)*/
        if (e->limit <= 1792) { /* 64KB ~ 1.792MB */
            temp = ((e->limit-1) / 64) +1;
        } else if (e->limit <= 100000){ /* 2MB ~ 100MB */
            temp = (e->limit /1000 ) + 27;
        } else if (e->limit <= 1000000){ /* 104MB ~ 1000MB */
            temp = (e->limit /8000) + 115;
        } else {
            temp = 255;
        }

        /* Setting ingress rate
         *    - here we defined ingress rate control will be disable if
         *       REF_CNT=255. (means no rate control)
         *    - this definition is for seperate different rate between
         *       "Ingress rate control" and "Strom rate control"
         *    - thus if the gave limit value trasfer REF_CNT is 255, we reasign
         *       REF_CNT to be 254
         */
        temp = (temp == 255) ? 254 : temp;
        reg_value &= ~(IRC_BKT0_RATE_CNT_M << IRC_BKT0_RATE_CNT_S);
        reg_value |= (temp << IRC_BKT0_RATE_CNT_S);

        /* enable ingress rate control */
        reg_value &= ~(IRC_BKT0_EN_M << IRC_BKT0_EN_S);
        reg_value |= (1 << IRC_BKT0_EN_S);
    }
    /* write register */
    ethsw_wreg(PAGE_BSS, REG_BSS_RX_RATE_CTRL_P0 + (e->port * 4),
               (uint8_t *)&reg_value, 4);

    up(&bcm_ethlock_switch_config);
    return rv;
}

/*
 *  Function : bcmeapi_ioctl_ethsw_port_irc_get
 *
 *  Purpose :
 *   Get the burst size and rate limit value of the selected port ingress rate.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      limit  :   rate limit value. (Kbits)
 *      burst_size  :   max burst size. (Kbits)
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Set the limit and burst size to bucket 0(storm control use bucket1).
 *
 */
int bcmeapi_ioctl_ethsw_port_irc_get(struct ethswctl_data *e)
{
    uint32_t  reg_value, temp;
    int     rv= BCM_E_NONE;

    down(&bcm_ethlock_switch_config);

    /* check global ingress rate control setting */
    if (robo_ingress_rate_init_flag) {
        ethsw_rreg(PAGE_BSS, REG_BSS_IRC_CONFIG, (uint8_t *)&temp, 4);
        /* if pkt_type_mask is 0, then reset robo_ingress_rate_init_flag to 0
         * so that the next irc_set will set it correctly.
         */
        temp &= (IRC_PKT_MASK | (1 << IRC_CFG_PKT_MSK0_EXT_S));
        robo_ingress_rate_init_flag = (temp == 0) ? 0 : 1;
    }

    /* Check ingress rate control
      *    - ING_RC_ENf should not be 0 in the runtime except the system been
      *       process without ingress rate setting or user manuel config
      *       register value. It will stop this port's storm control rate also.
      *    - set the REF_CNT to the MAX value means packets could
      *       be forwarded by no limit rate. (set to 0 will block all this
      *       port's traffic)
      */
    ethsw_rreg(PAGE_BSS, REG_BSS_RX_RATE_CTRL_P0 + (e->port * 4),
               (uint8_t *)&reg_value, 4);
    temp = (reg_value >> IRC_BKT0_RATE_CNT_S) & IRC_BKT0_RATE_CNT_M;


    if (temp == 254) {
        e->limit = 0;
        e->burst_size = 0;
    } else {
        temp = (reg_value >> IRC_BKT0_SIZE_S) & IRC_BKT0_SIZE_M;
        switch (temp) {
            case 0:
                e->burst_size = 16 * 8; /* 16KB */
                break;
            case 1:
                e->burst_size = 20 * 8; /* 20KB */
                break;
            case 2:
                e->burst_size = 28 * 8; /* 28KB */
                break;
            case 3:
                e->burst_size = 40 * 8; /* 40KB */
                break;
            case 4:
                e->burst_size = 76 * 8; /* 76KB */
                break;
            case 5:
                e->burst_size = 140 * 8; /* 140KB */
                break;
            case 6:
                e->burst_size = 268 * 8; /* 268KB */
                break;
            case 7:
                e->burst_size = 500 * 8; /* 500KB */
                break;

            default:
                up(&bcm_ethlock_switch_config);
                return BCM_E_INTERNAL;
        }

        temp = (reg_value >> IRC_BKT0_RATE_CNT_S) & IRC_BKT0_RATE_CNT_M;
        if (temp <= 28) {
            e->limit = temp * 64;
        } else if (temp <= 127) {
            e->limit = (temp -27) * 1000;
        } else if (temp <=243) {
            e->limit = (temp -115) * 1000 * 8;
        } else {
            rv = BCM_E_INTERNAL;
        }
    }

    up(&bcm_ethlock_switch_config);
    return rv;
}

/*
 *  Function : bcmeapi_ioctl_ethsw_port_erc_set
 *
 *  Purpose :
 *     Set the burst size and rate limit value of the selected port egress rate.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      limit  :   rate limit value.
 *      burst_size  :   max burst size.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int bcmeapi_ioctl_ethsw_port_erc_set(struct ethswctl_data *e)
{
    uint32_t  port_cfg_reg_value;
    uint16_t  rate_cfg_reg_value;
    uint32_t  temp = 0;
    uint32_t  burst_kbyte = 0;
    int     rv = BCM_E_NONE;

    down(&bcm_ethlock_switch_config);

    /* Enable XLEN_EN bit to include IPG for egress rate limiting */
    ethsw_rreg(PAGE_BSS, REG_BSS_IRC_CONFIG, (uint8_t *)&port_cfg_reg_value, 4);
    port_cfg_reg_value |= (1 << IRC_CFG_XLENEN);
    ethsw_wreg(PAGE_BSS, REG_BSS_IRC_CONFIG, (uint8_t *)&port_cfg_reg_value, 4);

    /* Read the current Egress Rate Config of the given port */
    ethsw_rreg(PAGE_BSS, REG_BSS_TX_RATE_CTRL_P0 + (e->port * 2),
               (uint8_t *)&rate_cfg_reg_value, 2);
    if (e->limit == 0) { /* Disable egress rate control */
        rate_cfg_reg_value &= ~(ERC_ERC_EN_M << ERC_ERC_EN_S);
    } else {    /* Enable egress rate control */
        /* burst size */
        burst_kbyte = e->burst_size / 8; /* Convert kbit to KByte */
        if (burst_kbyte > 500) { /* 500 KB */
            up(&bcm_ethlock_switch_config);
            return BCM_E_PARAM;
        }
        if (burst_kbyte <= 16) { /* 16KB */
            temp = 0;
        } else if (burst_kbyte <= 20) { /* 20KB */
            temp = 1;
        } else if (burst_kbyte <= 28) { /* 28KB */
            temp = 2;
        } else if (burst_kbyte <= 40) { /* 40KB */
            temp = 3;
        } else if (burst_kbyte <= 76) { /* 76KB */
            temp = 4;
        } else if (burst_kbyte <= 140) { /* 140KB */
            temp = 5;
        } else if (burst_kbyte <= 268) { /* 268KB */
            temp = 6;
        } else if (burst_kbyte <= 500) { /* 500KB */
            temp = 7;
        }

        rate_cfg_reg_value &= ~(ERC_BKT_SIZE_M << ERC_BKT_SIZE_S);
        rate_cfg_reg_value |= temp << ERC_BKT_SIZE_S;

        /* refresh count  (fixed type)*/
        if (e->limit <= 1792) { /* 64Kbps ~ 1.792Mbps */
            temp = ((e->limit-1) / 64) +1;
        } else if (e->limit <= 100000){ /* 2Mbps ~ 100Mbps */
            temp = (e->limit /1000 ) + 27;
        } else { /* 104Mbps ~ 1000Mbps */
            temp = (e->limit /8000) + 115;
        }
        rate_cfg_reg_value &= ~(ERC_RFSH_CNT_M << ERC_RFSH_CNT_S);
        rate_cfg_reg_value |= temp << ERC_RFSH_CNT_S;

        /* enable egress rate control */
        rate_cfg_reg_value &= ~(ERC_ERC_EN_M << ERC_ERC_EN_S);
        rate_cfg_reg_value |= (1 << ERC_ERC_EN_S);
    }
    /* write register */
    ethsw_wreg(PAGE_BSS, REG_BSS_TX_RATE_CTRL_P0 + (e->port * 2),
               (uint8_t *)&rate_cfg_reg_value, 2);

    up(&bcm_ethlock_switch_config);
    return rv;
}

/*
 *  Function : bcmeapi_ioctl_ethsw_port_erc_get
 *
 *  Purpose :
 *     Get the burst size and rate limit value of the selected port.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      limit  :   rate limit value.
 *      burst_size  :   max burst size.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int bcmeapi_ioctl_ethsw_port_erc_get(struct ethswctl_data *e)
{
    uint16_t reg_value;
    uint32_t temp;
    int     rv= BCM_E_NONE;

    ethsw_rreg(PAGE_BSS, REG_BSS_TX_RATE_CTRL_P0 + (e->port * 2),
               (uint8_t *)&reg_value, 2);
    temp = (reg_value >> ERC_ERC_EN_S) & ERC_ERC_EN_M;
    if (temp ==0) {
        e->limit = 0;
        e->burst_size = 0;
    } else {
        temp = (reg_value >> ERC_BKT_SIZE_S) & ERC_BKT_SIZE_M;
        switch (temp) {
            case 0:
                e->burst_size = 16 * 8; /* 16KB */
                break;
            case 1:
                e->burst_size = 20 * 8; /* 20KB */
                break;
            case 2:
                e->burst_size = 28 * 8; /* 28KB */
                break;
            case 3:
                e->burst_size = 40 * 8; /* 40KB */
                break;
            case 4:
                e->burst_size = 76 * 8; /* 76KB */
                break;
            case 5:
                e->burst_size = 140 * 8; /* 140KB */
                break;
            case 6:
                e->burst_size = 268 * 8; /* 268KB */
                break;
            case 7:
                e->burst_size = 500 * 8; /* 500KB */
                break;
            default:
                return BCM_E_INTERNAL;
        }
        temp = (reg_value >> ERC_RFSH_CNT_S) & ERC_RFSH_CNT_M;
        if (temp <= 28) {
            e->limit = temp * 64;
        } else if (temp <= 127) {
            e->limit = (temp -27) * 1000;
        } else if (temp <=243) {
            e->limit = (temp -115) * 1000 * 8;
        } else {
            return BCM_E_INTERNAL;
        }
    }

    return rv;
}

int bcmeapi_ioctl_ethsw_cosq_config(struct ethswctl_data *e)
{
    uint8_t  val8;
    int val;

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        ethsw_rreg(PAGE_QOS, REG_QOS_TXQ_CTRL, &val8, 1);
        BCM_ENET_DEBUG("REG_QOS_TXQ_CTRL = 0x%2x", val8);
        /* If TXQ_MODE is non-zero, then we have multiple egress queues */
        if ((val8 >> TXQ_CTRL_TXQ_MODE_S) & TXQ_CTRL_TXQ_MODE_M) {
            val = NUM_EGRESS_QUEUES;
        } else {
            val = 1;
        }
        e->numq = val;
        BCM_ENET_DEBUG("e->numq is = %2d", e->numq);
    } else {
        BCM_ENET_DEBUG("Given numq: 0x%02x\n ", e->numq);
        ethsw_rreg(PAGE_QOS, REG_QOS_TXQ_CTRL, &val8, 1);
        BCM_ENET_DEBUG("REG_QOS_TXQ_CTRL = 0x%02x", val8);
        if ((e->numq > 1) && (e->numq <= NUM_EGRESS_QUEUES)) {
            if ( ((val8 >> TXQ_CTRL_TXQ_MODE_S) & TXQ_CTRL_TXQ_MODE_M)
                != (e->numq - 1) ) {
                /* Set the number of queues to the given value */
                val8 &= (~(TXQ_CTRL_TXQ_MODE_M << TXQ_CTRL_TXQ_MODE_S));
                val8 |= (((e->numq - 1) & TXQ_CTRL_TXQ_MODE_M) <<
                         TXQ_CTRL_TXQ_MODE_S);
            }
        } else if (e->numq == 1){
            if ((val8 >> TXQ_CTRL_TXQ_MODE_S) & TXQ_CTRL_TXQ_MODE_M) {
                /* Set the number of queues to 1 */
                val8 &= (~(TXQ_CTRL_TXQ_MODE_M << TXQ_CTRL_TXQ_MODE_S));
            }
        } else {
            BCM_ENET_DEBUG("Invalid number of queues = %2d", e->numq);
            up(&bcm_ethlock_switch_config);
            return BCM_E_PARAM;
        }
        BCM_ENET_DEBUG("Writing 0x%02x to REG_QOS_TXQ_CTRL", val8);
        ethsw_wreg(PAGE_QOS, REG_QOS_TXQ_CTRL, &val8, 1);
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

#define MAX_WRR_WEIGHT 0x31
int bcmeapi_ioctl_ethsw_cosq_sched(struct ethswctl_data *e)
{
    uint8_t  val8, txq_mode, hq_preempt;
    int i, sched;

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        ethsw_rreg(PAGE_QOS, REG_QOS_TXQ_CTRL, &val8, 1);
        BCM_ENET_DEBUG("REG_QOS_TXQ_CTRL = 0x%2x", val8);
        txq_mode = (val8 >> TXQ_CTRL_TXQ_MODE_S) & TXQ_CTRL_TXQ_MODE_M;
        if(txq_mode == 0) {
            BCM_ENET_DEBUG("Multiple egress queues feature is not enabled");
            up(&bcm_ethlock_switch_config);
            return -1;
        }
        hq_preempt = (val8 >> TXQ_CTRL_HQ_PREEMPT_S) & TXQ_CTRL_HQ_PREEMPT_M;
        if (hq_preempt) {
            if(txq_mode == 1) {
                sched = BCM_COSQ_STRICT;
            } else {
                sched = BCM_COSQ_COMBO;
                e->queue = txq_mode;
                }
        } else {
            sched = BCM_COSQ_WRR;
        }

        e->scheduling = sched;

        /* Get the weights */
        if(sched != BCM_COSQ_STRICT) {
            for (i=0; i < NUM_EGRESS_QUEUES; i++) {
                ethsw_rreg(PAGE_QOS, REG_QOS_TXQ_WEIGHT_Q0 + i, &val8, 1);
                BCM_ENET_DEBUG("Weight[%2d] = %02d ", i, val8);
                e->weights[i] = val8;
                BCM_ENET_DEBUG("e->weight[%2d] = %02d ", i, e->weights[i]);
            }
        }
    } else {
        BCM_ENET_DEBUG("Given scheduling mode: %02d", e->scheduling);
        BCM_ENET_DEBUG("Given sp_endq: %02d", e->queue);
        for (i=0; i < NUM_EGRESS_QUEUES; i++) {
            BCM_ENET_DEBUG("Given weight[%2d] = %02d ", i, e->weights[i]);
            
            // Is this a legal weight?
            if (e->weights[i] <= 0 || e->weights[i] > MAX_WRR_WEIGHT) {
                BCM_ENET_DEBUG("Invalid weight");
                up(&bcm_ethlock_switch_config);
                return BCM_E_ERROR;
            }
        }
        ethsw_rreg(PAGE_QOS, REG_QOS_TXQ_CTRL, &val8, 1);
        BCM_ENET_DEBUG("REG_QOS_TXQ_CTRL = 0x%02x", val8);
        txq_mode = (val8 >> TXQ_CTRL_TXQ_MODE_S) & TXQ_CTRL_TXQ_MODE_M;
        if(txq_mode == 0) {
            BCM_ENET_DEBUG("Multiple egress queues feature is not enabled");
            up(&bcm_ethlock_switch_config);
            return -1;
        }
        /* Set the scheduling mode */
        if (e->scheduling == BCM_COSQ_WRR) {
            // Set TXQ_MODE bits for 4 queue mode.  Leave high
            // queue preeempt bit cleared so queue weighting will be used.
            val8 = ((0x03 & TXQ_CTRL_TXQ_MODE_M) << TXQ_CTRL_TXQ_MODE_S);
        } else if ((e->scheduling == BCM_COSQ_STRICT) ||
                   (e->scheduling == BCM_COSQ_COMBO)){
            val8 |= (TXQ_CTRL_HQ_PREEMPT_M << TXQ_CTRL_HQ_PREEMPT_S);
            if (e->scheduling == BCM_COSQ_STRICT) {
                txq_mode = 1;
            } else {
                txq_mode = e->queue;
            }
            val8 &= (~(TXQ_CTRL_TXQ_MODE_M << TXQ_CTRL_TXQ_MODE_S));
            val8 |= ((txq_mode & TXQ_CTRL_TXQ_MODE_M) << TXQ_CTRL_TXQ_MODE_S);
        } else {
            BCM_ENET_DEBUG("Invalid scheduling mode %02d", e->scheduling);
            up(&bcm_ethlock_switch_config);
            return BCM_E_PARAM;
        }
        BCM_ENET_DEBUG("Writing 0x%02x to REG_QOS_TXQ_CTRL", val8);
        ethsw_wreg(PAGE_QOS, REG_QOS_TXQ_CTRL, &val8, 1);
        /* Set the weights if WRR or COMBO */
        if(e->scheduling != BCM_COSQ_STRICT) {
            for (i=0; i < NUM_EGRESS_QUEUES; i++) {
                BCM_ENET_DEBUG("Weight[%2d] = %02d ", i, e->weights[i]);
                val8 =  e->weights[i];
                ethsw_wreg(PAGE_QOS, REG_QOS_TXQ_WEIGHT_Q0 + i, &val8, 1);
            }
        }
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

/* For TYPE_GET, return the queue value to caller. Push the copy_to_user to the next level up
   so bcmeapi_ioctl_ethsw_cosq_port_mapping can be used by the enet driver.
   Negative return values used to indicate an error - Jan 2011 */
int bcmeapi_ioctl_ethsw_cosq_port_mapping(struct ethswctl_data *e)
{
    uint32_t val32;
    uint16_t val16;
    int queue;
    int retval = 0;

    BCM_ENET_DEBUG("Given port: %02d\n ", e->port);
    if (e->port >= TOTAL_SWITCH_PORTS) {
        printk("Invalid Switch Port\n");
        return -BCM_E_ERROR;
    }
    BCM_ENET_DEBUG("Given priority: %02d\n ", e->priority);
    if ((e->priority > MAX_PRIORITY_VALUE) || (e->priority < 0)) {
        printk("Invalid Priority\n");
        return -BCM_E_ERROR;
    }

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        ethsw_rreg(PAGE_QOS, REG_QOS_PORT_PRIO_MAP_P0 + (e->port * 2),
                   (uint8_t *)&val16, 2);
        BCM_ENET_DEBUG("REG_QOS_PORT_PRIO_MAP_Px = 0x%04x", val16);
        val32 = val16;
        /* Get the queue */
        queue = val32 >> (e->priority * REG_QOS_PRIO_TO_QID_SEL_BITS);
        queue &= REG_QOS_PRIO_TO_QID_SEL_M;
        retval = queue;
        BCM_ENET_DEBUG("e->queue is = %4x", e->queue);
    } else {
        BCM_ENET_DEBUG("Given queue: 0x%02x\n ", e->queue);
        ethsw_rreg(PAGE_QOS, REG_QOS_PORT_PRIO_MAP_P0 + (e->port * 2),
                   (uint8_t *)&val16, 2);
        val32 = val16;
        val32 &= ~(REG_QOS_PRIO_TO_QID_SEL_M <<
                   (e->priority * REG_QOS_PRIO_TO_QID_SEL_BITS));
        val32 |= ((e->queue & REG_QOS_PRIO_TO_QID_SEL_M) <<
                  (e->priority * REG_QOS_PRIO_TO_QID_SEL_BITS));
        BCM_ENET_DEBUG("Writing = 0x%08x to REG_QOS_PORT_PRIO_MAP_Px",
                       (unsigned int)val32);
        val16 = val32;
        ethsw_wreg(PAGE_QOS, REG_QOS_PORT_PRIO_MAP_P0 + (e->port * 2),
                   (uint8_t *)&val16, 2);
        mapEthPortToRxIudma(e->port, e->queue);
    }

    up(&bcm_ethlock_switch_config);
    return retval;
}

int bcmeapi_ioctl_ethsw_dscp_to_priority_mapping(struct ethswctl_data *e)
{
    uint32_t val32, offsetlo, offsethi, mapnum;
    uint16_t val16;
    int priority, dscplsbs, offsetlolen = 4;

    BCM_ENET_DEBUG("Given dscp: %02d\n ", e->val);
    if (e->val > 0x3F) {
        printk("Invalid DSCP Value\n");
        return BCM_E_ERROR;
    }

    down(&bcm_ethlock_switch_config);

    dscplsbs = e->val & 0xF;
    mapnum = (e->val >> 4) & 0x3;
    switch (mapnum) {
        case 0:
            offsetlo = REG_QOS_DSCP_PRIO_MAP0LO;
            offsethi = REG_QOS_DSCP_PRIO_MAP0HI;
            break;
        case 1:
            offsetlo = REG_QOS_DSCP_PRIO_MAP1LO;
            offsethi = REG_QOS_DSCP_PRIO_MAP1HI;
            offsetlolen = 2;
            break;
        case 2:
            offsetlo = REG_QOS_DSCP_PRIO_MAP2LO;
            offsethi = REG_QOS_DSCP_PRIO_MAP2HI;
            break;
        case 3:
            offsetlo = REG_QOS_DSCP_PRIO_MAP3LO;
            offsethi = REG_QOS_DSCP_PRIO_MAP3HI;
            offsetlolen = 2;
            break;
        default:
            return -1;
    }

    if (e->type == TYPE_GET) {
        if (offsetlolen == 2) {
            if (dscplsbs <= 4) {
                ethsw_rreg(PAGE_QOS, offsetlo, (uint8_t *)&val16, 2);
                priority = (val16 >> (dscplsbs * 3)) & 0x7;
            } else if (dscplsbs > 5) {
                ethsw_rreg(PAGE_QOS, offsethi, (uint8_t *)&val32, 4);
                priority = (val32 >> (((dscplsbs - 5) * 3) - 1) ) & 0x7;
            } else {
                ethsw_rreg(PAGE_QOS, offsetlo, (uint8_t *)&val16, 2);
                ethsw_rreg(PAGE_QOS, offsethi, (uint8_t *)&val32, 4);
                priority = ((val16 >> 15) & 0x1) | ((val32 & 0x3) << 1);
            }
        } else {
            if (dscplsbs <= 9) {
                ethsw_rreg(PAGE_QOS, offsetlo, (uint8_t *)&val32, 4);
                priority = (val32 >> (dscplsbs * 3)) & 0x7;
            } else if (dscplsbs > 10) {
                ethsw_rreg(PAGE_QOS, offsethi, (uint8_t *)&val16, 2);
                priority = (val16 >> (((dscplsbs - 10) * 3) - 2) ) & 0x7;
            } else {
                ethsw_rreg(PAGE_QOS, offsetlo, (uint8_t *)&val32, 4);
                ethsw_rreg(PAGE_QOS, offsethi, (uint8_t *)&val16, 2);
                priority = ((val32 >> 30) & 0x3) | ((val16 & 0x1) << 2);
            }
        }
        e->priority = priority;
        BCM_ENET_DEBUG("dscp %d is mapped to priority: %d\n ", e->val, e->priority);
    } else {
        BCM_ENET_DEBUG("Given priority: %02d\n ", e->priority);
        if ((e->priority > MAX_PRIORITY_VALUE) || (e->priority < 0)) {
            printk("Invalid Priority\n");
            return BCM_E_ERROR;
        }
        if (offsetlolen == 2) {
            ethsw_rreg(PAGE_QOS, offsetlo, (uint8_t *)&val16, 2);
            ethsw_rreg(PAGE_QOS, offsethi, (uint8_t *)&val32, 4);
            if (dscplsbs <= 4) {
                val16 &= ~(0x7 << (dscplsbs * 3));
                val16 |= ((e->priority & 0x7) << (dscplsbs * 3));
            } else if (dscplsbs > 5) {
                val32 &= ~(0x7 << (((dscplsbs - 5) * 3) - 1));
                val32 |= ((e->priority & 0x7) << (((dscplsbs - 5) * 3) - 1));
            } else {
                val16 &= ~(1 << 15);
                val16 |= ((e->priority & 0x1) << 15);
                val32 &= ~(0x3);
                val32 |= ((e->priority >> 1) & 0x3);
            }
            ethsw_wreg(PAGE_QOS, offsetlo, (uint8_t *)&val16, 2);
            ethsw_wreg(PAGE_QOS, offsethi, (uint8_t *)&val32, 4);
        } else {
            ethsw_rreg(PAGE_QOS, offsetlo, (uint8_t *)&val32, 4);
            ethsw_rreg(PAGE_QOS, offsethi, (uint8_t *)&val16, 2);
            if (dscplsbs <= 9) {
                val32 &= ~(0x7 << (dscplsbs * 3));
                val32 |= ((e->priority & 0x7) << (dscplsbs * 3));
            } else if (dscplsbs > 10) {
                val16 &= ~(0x7 << (((dscplsbs - 10) * 3) - 2));
                val16 |= ((e->priority & 0x7) << (((dscplsbs - 10) * 3) - 2));
            } else {
                val32 &= ~(0x3 << 30);
                val32 |= ((e->priority & 0x3) << 30);
                val16 &= ~(0x1);
                val16 |= ((e->priority >> 2) & 0x1);
            }
            ethsw_wreg(PAGE_QOS, offsetlo, (uint8_t *)&val32, 4);
            ethsw_wreg(PAGE_QOS, offsethi, (uint8_t *)&val16, 2);
        }
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

/*
 * Get/Set PCP to TC mapping Tabel entry given 802.1p priotity (PCP)
 * and mapped priority
 *** Input params 
 * e->type  GET/SET
 * e->val -  pcp
 * e->priority - mapped TC value, case of SET
 *** Output params 
 * e->priority - mapped TC value, case of GET
 * Returns 0 for Success, Negative value for failure.
 */
int bcmeapi_ioctl_ethsw_pcp_to_priority_mapping(struct ethswctl_data *e)
{

    uint32_t val32;
    uint16_t reg_addr;

    BCM_ENET_DEBUG("Given pcp: %02d\n ", e->val);
    if (e->val > MAX_PRIORITY_VALUE) {
        printk("Invalid PCP Value %02d\n", e->val);
        return BCM_E_ERROR;
    }

    /* Internal switch has one instance for all ports
     * unlike external switches which have one instance
     * per port.
     */
    reg_addr = REG_QOS_8021P_PRIO_MAP;

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        ethsw_rreg(PAGE_QOS, reg_addr, (void *)&val32, 4);
        e->priority = (val32 >> (e->val * 3)) & MAX_PRIORITY_VALUE;
        BCM_ENET_DEBUG("pcp %d is mapped to priority: %d\n ", e->val, e->priority);
    } else {
        BCM_ENET_DEBUG("Given pcp: %02d priority: %02d\n ", e->val, e->priority);
        if ((e->priority > MAX_PRIORITY_VALUE) || (e->priority < 0)) {
            printk("Invalid Priority\n");
            up(&bcm_ethlock_switch_config);
            return BCM_E_ERROR;
        }
        ethsw_rreg(PAGE_QOS, reg_addr, (void *)&val32, 4);
        val32 &= ~(MAX_PRIORITY_VALUE << (e->val * 3));
        val32 |= (e->priority & MAX_PRIORITY_VALUE) << (e->val * 3);
        ethsw_wreg(PAGE_QOS, reg_addr, (void *)&val32, 4);
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

/* Original Queue Mapping before parking */
static int impQue2ChanMap[NUM_EGRESS_QUEUES] = {[0 ... (NUM_EGRESS_QUEUES-1)] = -1};
void enet_park_rxdma_channel(int parkChan, int unpark)
{
    volatile DmaRegs *newDmaReg = (DmaRegs *)(SWITCH_DMA_BASE);
    int queue, ch, newChan;

#if defined(CONFIG_BCM_GMAC)
    /* Skip GMAC channel request */
    if ((int)get_dmaCtrl( parkChan )== GMAC_DMA_BASE) return;
#endif
    
    if (!unpark)
    {
        /* Find a safe enabled channel to park queue */
        for (newChan = 0; newChan < ENET_RX_CHANNELS_MAX; newChan++)
        {
            if (newChan == parkChan) continue;
            if (newDmaReg->chcfg[RxChanTo0BasedPhyChan(newChan)].cfg & DMA_ENABLE) break;
        }

        if (newChan >= ENET_RX_CHANNELS_MAX)
        {
            printk(" No Channel has been enabled, park the channel %d to 0 first\n", parkChan);
            newChan = 0;
        }
        /* Find which queue need to park to the new channel */
        for (queue=0; queue < NUM_EGRESS_QUEUES; queue++)
        {
            bcmeapi_ethsw_cosq_rxchannel_mapping(&ch, queue, 0);

            if (ch == parkChan)
            {
                /* Record original queue mapping then map it to the new channel */
                impQue2ChanMap[queue] = parkChan;  
                bcmeapi_ethsw_cosq_rxchannel_mapping(&newChan, queue, 1);
            }
        }
    }
    else    /* Unpark */
    {
        /* Find queues originally mapped to this channel */
        for (queue=0; queue < NUM_EGRESS_QUEUES; queue++)
        {
            if (impQue2ChanMap[queue] == parkChan)
            {
                bcmeapi_ethsw_cosq_rxchannel_mapping(&parkChan, queue, 1);
            }
        }
    }
}

void bcmeapi_ethsw_cosq_rxchannel_mapping(int *channel, int queue, int set)
{
    uint32_t val32;
    volatile DmaRegs *newDmaReg = (DmaRegs *)(SWITCH_DMA_BASE);

    down(&bcm_ethlock_switch_config);

    if (!set) {
        ethsw_rreg(PAGE_CONTROL, REG_IUDMA_QUEUE_CTRL, (uint8_t *)&val32, 4);

        /* Get the channel */
        *channel = val32 >> (REG_IUDMA_Q_CTRL_RXQ_SEL_S + (queue * 2));
        *channel &= REG_IUDMA_Q_CTRL_PRIO_TO_CH_M;
    } else {
        /* If the channel to be mapped is disabled, delay queue mapping until channel is enabled */
        if (!(newDmaReg->chcfg[RxChanTo0BasedPhyChan(*channel)].cfg & DMA_ENABLE))
        {
            impQue2ChanMap[queue] = *channel;
            goto done;
        }

        ethsw_rreg(PAGE_CONTROL, REG_IUDMA_QUEUE_CTRL, (uint8_t *)&val32, 4);
        val32 &= ~(REG_IUDMA_Q_CTRL_PRIO_TO_CH_M <<
                   (REG_IUDMA_Q_CTRL_RXQ_SEL_S + (queue * 2)));
        val32 |= ((*channel & REG_IUDMA_Q_CTRL_PRIO_TO_CH_M) <<
                  (REG_IUDMA_Q_CTRL_RXQ_SEL_S + (queue * 2)));
        ethsw_wreg(PAGE_CONTROL, REG_IUDMA_QUEUE_CTRL, (uint8_t *)&val32, 4);
    }

done:
    up(&bcm_ethlock_switch_config);
}

int bcmeapi_ioctl_ethsw_cosq_rxchannel_mapping(struct ethswctl_data *e)
{
    int channel;

    BCM_ENET_DEBUG("Given queue: 0x%02x\n ", e->queue);
    if ((e->queue >= NUM_EGRESS_QUEUES) || (e->queue < 0)) {
        printk("Invalid queue\n");
        return BCM_E_ERROR;
    }

    if (e->type == TYPE_GET) {
        /* Get the channel */
        bcmeapi_ethsw_cosq_rxchannel_mapping(&channel, e->queue, 0);
        e->channel = channel;
        BCM_ENET_DEBUG("e->channel is = %4x", e->channel);
    } else {
        BCM_ENET_DEBUG("Given channel: 0x%02x\n ", e->channel);
        if ((e->channel >= ENET_RX_CHANNELS_MAX) || (e->channel < 0)) {
            printk("Invalid Channel\n");
            return BCM_E_ERROR;
        }

        channel = e->channel;
        bcmeapi_ethsw_cosq_rxchannel_mapping(&channel, e->queue, 1);
    }

    return 0;
}

int bcmeapi_ioctl_debug_conf(struct ethswctl_data *e)
{
    int logLevel = e->val;

    if (e->type == TYPE_GET) {
        e->ret_val = bcmLog_getLogLevel(BCM_LOG_ID_ENET);
    } else {
        if(logLevel >= 0 && logLevel < BCM_LOG_LEVEL_MAX)
        {
            printk("Setting Enet Driver Log level");
            bcmLog_setLogLevel(BCM_LOG_ID_ENET, logLevel);
        }
        else
        {
            BCM_ENET_ERROR("Invalid Log level %d (max %d)",
                   logLevel, BCM_LOG_LEVEL_MAX);

            return -1;
        }
    }

    return 0;
}


int bcmeapi_ioctl_ethsw_cosq_txq_sel(struct ethswctl_data *e)
{
    uint32_t v32, method = 0;

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        /* Configure the switch to use Desc priority */
        ethsw_rreg(PAGE_CONTROL, REG_IUDMA_CTRL, (uint8_t *)&v32, 4);
        if (v32 & REG_IUDMA_CTRL_USE_DESC_PRIO) {
            method = USE_TX_BD_PRIORITY;
        } else if (v32 & REG_IUDMA_CTRL_USE_QUEUE_PRIO){
            method = USE_TX_DMA_CHANNEL;
        } else {
            /* Indicate neither BD nor iuDMA queue priority are used */
            method = NONE_OF_THE_METHODS;
        }
        e->ret_val = method;
        BCM_ENET_DEBUG("e->ret_val is = %4x", e->ret_val);
    } else {
        BCM_ENET_DEBUG("Given txq_sel method: %02d\n ", e->val);

        /* Set the txq selection method as given */
        if (e->val == USE_TX_BD_PRIORITY) {
            /* Configure the switch to use Desc priority */
            ethsw_rreg(PAGE_CONTROL, REG_IUDMA_CTRL, (uint8_t *)&v32, 4);
            v32 &= ~REG_IUDMA_CTRL_USE_QUEUE_PRIO;
            v32 |= REG_IUDMA_CTRL_USE_DESC_PRIO;
            ethsw_wreg(PAGE_CONTROL, REG_IUDMA_CTRL, (uint8_t *)&v32, 4);
#ifndef SINGLE_CHANNEL_TX
            use_tx_dma_channel_for_priority = 0;
#endif /*SINGLE_CHANNEL_TX*/
        } else if (e->val == USE_TX_DMA_CHANNEL) {
            /* Configure the switch to use Desc priority */
            ethsw_rreg(PAGE_CONTROL, REG_IUDMA_CTRL, (uint8_t *)&v32, 4);
            v32 &= ~REG_IUDMA_CTRL_USE_DESC_PRIO;
            v32 |= REG_IUDMA_CTRL_USE_QUEUE_PRIO;
            ethsw_wreg(PAGE_CONTROL, REG_IUDMA_CTRL, (uint8_t *)&v32, 4);
#ifndef SINGLE_CHANNEL_TX
            use_tx_dma_channel_for_priority = 1;
#endif /*SINGLE_CHANNEL_TX*/
        } else {
            up(&bcm_ethlock_switch_config);
            printk("Invalid method\n");
            return BCM_E_ERROR;
        }
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

int ethsw_iudmaq_to_egressq_map_get(int iudmaq, int *egressq)
{
    uint32_t val32;

    if ((iudmaq >= ENET_RX_CHANNELS_MAX) || (iudmaq < 0)) {
        printk("Invalid iudma queue\n");
        return -1;
    }

    down(&bcm_ethlock_switch_config);
    ethsw_rreg(PAGE_CONTROL, REG_IUDMA_QUEUE_CTRL, (uint8_t *)&val32, 4);
    up(&bcm_ethlock_switch_config);

    *egressq = val32 >> (REG_IUDMA_Q_CTRL_TXQ_SEL_S + (iudmaq * 3));
    *egressq &= REG_IUDMA_Q_CTRL_CH_TO_PRIO_M;
    BCM_ENET_DEBUG("The iudmaq %02d is mapped to egress queue: %02d\n ", iudmaq, *egressq);

    return 0;
}
EXPORT_SYMBOL(ethsw_iudmaq_to_egressq_map_get);

int bcmeapi_ioctl_ethsw_cosq_txchannel_mapping(struct ethswctl_data *e)
{
    uint32_t val32;

    BCM_ENET_DEBUG("Given channel: %02d\n ", e->channel);
    if ((e->channel >= ENET_RX_CHANNELS_MAX) || (e->channel < 0)) {
        printk("Invalid Channel\n");
        return BCM_E_ERROR;
    }

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        ethsw_rreg(PAGE_CONTROL, REG_IUDMA_QUEUE_CTRL, (uint8_t *)&val32, 4);
        BCM_ENET_DEBUG("REG_IUDMA_QUEUE_CTRL = 0x%08x", (unsigned int)val32);
        /* Get the queue */
        e->queue = val32 >> (REG_IUDMA_Q_CTRL_TXQ_SEL_S + (e->channel * 3));
        e->queue &= REG_IUDMA_Q_CTRL_CH_TO_PRIO_M;
        BCM_ENET_DEBUG("The queue: %02d\n ", e->queue);
    } else {
        BCM_ENET_DEBUG("Given queue: %02d\n ", e->queue);
        if ((e->queue >= NUM_EGRESS_QUEUES) || (e->queue < 0)) {
            up(&bcm_ethlock_switch_config);
            printk("Invalid queue\n");
            return BCM_E_ERROR;
        }

        ethsw_rreg(PAGE_CONTROL, REG_IUDMA_QUEUE_CTRL, (uint8_t *)&val32, 4);
        val32 &= ~(REG_IUDMA_Q_CTRL_CH_TO_PRIO_M <<
                   (REG_IUDMA_Q_CTRL_TXQ_SEL_S + (e->channel * 3)));
        val32 |= ((e->queue & REG_IUDMA_Q_CTRL_CH_TO_PRIO_M) <<
                  (REG_IUDMA_Q_CTRL_TXQ_SEL_S + (e->channel * 3)));
        BCM_ENET_DEBUG("Writing = 0x%08x to REG_IUDMA_QUEUE_CTRL",
                       (unsigned int)val32);
        ethsw_wreg(PAGE_CONTROL, REG_IUDMA_QUEUE_CTRL, (uint8_t *)&val32, 4);

#ifndef SINGLE_CHANNEL_TX
        /* Maintain the channel to queue mapping information in driver */
        channel_for_queue[e->queue] = e->channel;
#endif /*SINGLE_CHANNEL_TX*/
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

/* Stats API */
typedef enum bcm_hw_stat_e {
    TXOCTETSr = 0,
    TXDROPPKTSr,
    TXQOSPKTSr,
    TXBROADCASTPKTSr,
    TXMULTICASTPKTSr,
    TXUNICASTPKTSr,
    TXCOLLISIONSr,
    TXSINGLECOLLISIONr,
    TXMULTIPLECOLLISIONr,
    TXDEFERREDTRANSMITr,
    TXLATECOLLISIONr,
    TXEXCESSIVECOLLISIONr,
    TXFRAMEINDISCr,
    TXPAUSEPKTSr,
    TXQOSOCTETSr,
    RXOCTETSr,
    RXUNDERSIZEPKTSr,
    RXPAUSEPKTSr,
    PKTS64OCTETSr,
    PKTS65TO127OCTETSr,
    PKTS128TO255OCTETSr,
    PKTS256TO511OCTETSr,
    PKTS512TO1023OCTETSr,
    PKTS1024TO1522OCTETSr,
    RXOVERSIZEPKTSr,
    RXJABBERSr,
    RXALIGNMENTERRORSr,
    RXFCSERRORSr,
    RXGOODOCTETSr,
    RXDROPPKTSr,
    RXUNICASTPKTSr,
    RXMULTICASTPKTSr,
    RXBROADCASTPKTSr,
    RXSACHANGESr,
    RXFRAGMENTSr,
    RXEXCESSSIZEDISCr,
    RXSYMBOLERRORr,
    RXQOSPKTSr,
    RXQOSOCTETSr,
    PKTS1523TO2047r,
    PKTS2048TO4095r,
    PKTS4096TO8191r,
    PKTS8192TO9728r,
    MAXNUMCNTRS,
} bcm_hw_stat_t;


typedef struct bcm_reg_info_t {
    uint8_t offset;
    uint8_t len;
} bcm_reg_info_t;

/* offsets and lengths of stats defined in bcm_hw_stat_t */
bcm_reg_info_t bcm_stat_reg_list[] =  {
    {0x00, 8}, {0x08, 4}, {0x0c, 4}, {0x10, 4}, {0x14, 4},
    {0x18, 4}, {0x1C, 4}, {0x20, 4}, {0x24, 4}, {0x28, 4},
    {0x2C, 4}, {0x30, 4}, {0x34, 4}, {0x38, 4}, {0x3c, 8},
    {0x44, 8}, {0x4c, 4}, {0x50, 4}, {0x54, 4}, {0x58, 4},
    {0x5c, 4}, {0x60, 4}, {0x64, 4}, {0x68, 4}, {0x6c, 4},
    {0x70, 4}, {0x74, 4}, {0x78, 4}, {0x7c, 8}, {0x84, 4},
    {0x88, 4}, {0x8c, 4}, {0x90, 4}, {0x94, 4}, {0x98, 4},
    {0x9c, 4}, {0xa0, 4}, {0xa4, 4}, {0xa8, 8}, {0xb0, 4},
    {0xb4, 4}, {0xb8, 4}, {0xbc, 4},
};

#define COMPILER_64_SET(dst, src_hi, src_lo) \
    ((dst) = (((uint64) (src_hi)) << 32) | ((uint64) (src_lo)))
#define COMPILER_64_ZERO(dst)       ((dst) = 0)
#define COMPILER_64_ADD_64(dst, src)  ((dst) += (src))
#define COMPILER_64_SUB_64(dst, src)  ((dst) -= (src))
#define COMPILER_64_EQ(src1, src2)      ((src1) == (src2))
#define COMPILER_64_LT(src1, src2)      ((src1) <  (src2))

static uint64 counter_sw_val[TOTAL_SWITCH_PORTS][MAXNUMCNTRS];
static uint64 counter_hw_val[TOTAL_SWITCH_PORTS][MAXNUMCNTRS];
static uint64 counter_delta[TOTAL_SWITCH_PORTS][MAXNUMCNTRS];

#define PBMP_MEMBER(bmp, port)  (((bmp) & (1U<<(port))) != 0)
#define PBMP_ITER(bmp, port) \
        for ((port) = 0; (port) < TOTAL_SWITCH_PORTS; (port)++) \
            if (PBMP_MEMBER((bmp), (port)))


/*
 * Function:
 *      ethsw_robo_counter_collect
 * Purpose:
 *      Collects and accumulates the stats
 * Parameters:
 *      discard - If true, the software counters are not updated; this
 *              results in only synchronizing the previous hardware
 *              count buffer.
 * Returns:
 *      BCM_E_XXX
 */
int ethsw_counter_collect(uint32_t portmap, int discard)
{
    int32_t         i = 0;
    uint32_t        ctr_new32, ctraddr, len;
    uint16_t        v16;
    bcm_port_t      port;
    uint64          ctr_new, ctr_prev, ctr_diff;
    bcm_reg_info_t  reg_info;

#if defined(CONFIG_BCM_ETH_PWRSAVE)
        ethsw_phy_pll_up(0);
#endif

    PBMP_ITER(portmap, port) {
        for (i = 0; i < MAXNUMCNTRS; i++) {
            /* Get the counter offset and length */
            reg_info = bcm_stat_reg_list[i];
            ctraddr = reg_info.offset;
            len = reg_info.len;

            ctr_prev = counter_hw_val[port][i];

            /* Read the counter value from H/W */
            down(&bcm_ethlock_switch_config);
            if(len == 8) {
                /* For the 64-bit counter */
                ethsw_rreg(PAGE_MIB_P0 + (port), ctraddr,
                           (uint8_t *)&ctr_new, 8);
            } else {
                /* For the 32-bit counter */
                ethsw_rreg(PAGE_MIB_P0 + (port), ctraddr,
                           (uint8_t *)&ctr_new32, 4);
                COMPILER_64_SET(ctr_new, 0, ctr_new32);
            }
            up(&bcm_ethlock_switch_config);

            BCM_ENET_LINK_DEBUG("port= %d; i = %d; The ctr_new = 0x%llx",
                                port, i, ctr_new);

            if (COMPILER_64_EQ(ctr_new, ctr_prev)) {
                COMPILER_64_ZERO(counter_delta[port][i]);
                continue;
            }

            if (discard) {
                /* Update the previous value buffer */
                counter_hw_val[port][i] = ctr_new;
                COMPILER_64_ZERO(counter_delta[port][i]);
                continue;
            }

            ctr_diff = ctr_new;
            if (COMPILER_64_LT(ctr_diff, ctr_prev)) {
                int             width;
                uint64          wrap_amt;

                /*
                 * Counter must have wrapped around.
                 * Add the proper wrap-around amount.
                 */
                width = len * 8;
                if (width < 32) {
                    COMPILER_64_SET(wrap_amt, 0, 1UL << width);
                    COMPILER_64_ADD_64(ctr_diff, wrap_amt);
                } else if (width < 64) {
                    COMPILER_64_SET(wrap_amt, 1UL << (width - 32), 0);
                    COMPILER_64_ADD_64(ctr_diff, wrap_amt);
                }
            }
            COMPILER_64_SUB_64(ctr_diff, ctr_prev);
            /*
             * For ROBO chips,
             * MIB counter TxPausePkts always counts both backpressure(half duplex) and
             * pause frames(full duplex). But this counter should not accumulate
             * when duplex is half.
             * Thus, update SW counter table only when duplex is full.
             */
            if (i == TXPAUSEPKTSr) {
                down(&bcm_ethlock_switch_config);
                ethsw_phy_rreg(ETHSW_PHY_GET_PHYID(port), MII_ASR, &v16);
                up(&bcm_ethlock_switch_config);
                if (MII_ASR_FDX(v16)) {
                    COMPILER_64_ADD_64(counter_sw_val[port][i], ctr_diff);
                    counter_delta[port][i] = ctr_diff;
                } else {
                    counter_delta[port][i] = 0;
                }
                counter_hw_val[port][i] = ctr_new;
            } else {
                COMPILER_64_ADD_64(counter_sw_val[port][i], ctr_diff);
                counter_delta[port][i] = ctr_diff;
                counter_hw_val[port][i] = ctr_new;
            }
            BCM_ENET_LINK_DEBUG("counter_sw_val = 0x%llx",
                                counter_sw_val[port][i]);
        }
    }

    return BCM_E_NONE;
}



/*
 * Function:
 *      soc_robo_counter_get
 * Purpose:
 *      Retrieves the value of a 64-bit software shadow counter.
 * Parameters:
 *      port    - RoboSwitch port number.
 *      ctr_reg - counter register to retrieve.
 *      val     - (OUT) Pointer to place the 64-bit result.
 * Returns:
 *      BCM_E_XXX.
 */
static inline int ethsw_robo_counter_get(bcm_port_t port, bcm_hw_stat_t
                                         ctr_idx, uint64 *val)
{
    BCM_ENET_INFO("port = %2d; ctr_idx = %4d", port, ctr_idx);
    *val = counter_sw_val[port][ctr_idx];
    BCM_ENET_INFO("counter = 0x%llx", counter_sw_val[port][ctr_idx]);

    return BCM_E_NONE;
}

/*
 *  Function : bcmeapi_ioctl_ethsw_counter_get
 *
 *  Purpose :
 *      Get the snmp counter value.
 *
 *  Parameters :
 *      uint    :   uint number.
 *      port        :   port number.
 *      counter_type   :   counter_type.
 *      val  :   counter val.
 *
 *  Return :
 *      BCM_E_XXX
 *
 *  Note :
 *
 */
int bcmeapi_ioctl_ethsw_counter_get(struct ethswctl_data *e)
{
    uint64  count=0, count_tmp=0;
    uint32_t port = e->port;
    uint32_t counter_type = e->counter_type;
    uint64 val;

    BCM_ENET_INFO("Port = %2d", e->port);
    BCM_ENET_INFO("Counter Type = %4d", e->counter_type);
    switch (counter_type)
    {
        /* *** RFC 1213 *** */

        case snmpIfInOctets:
            ethsw_robo_counter_get(port, RXOCTETSr, &count);
            break;

        case snmpIfInUcastPkts:
            /* Total non-error frames minus broadcast/multicast frames */
            ethsw_robo_counter_get(port, RXUNICASTPKTSr, &count);
            break;

        case snmpIfInNUcastPkts:
            /* Multicast frames plus broadcast frames */
            ethsw_robo_counter_get(port, RXMULTICASTPKTSr, &count_tmp);
            count=count_tmp;
            ethsw_robo_counter_get(port, RXBROADCASTPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpIfInDiscards:
            ethsw_robo_counter_get(port, RXDROPPKTSr, &count);
            break;

        case snmpIfInErrors:
            ethsw_robo_counter_get(port, RXALIGNMENTERRORSr, &count_tmp);
            count=count_tmp;
            ethsw_robo_counter_get(port, RXFCSERRORSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXFRAGMENTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXEXCESSSIZEDISCr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXJABBERSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpIfInUnknownProtos:
            val = 0;
            return BCM_E_UNAVAIL;
            break;

        case snmpIfOutOctets:
            ethsw_robo_counter_get(port, TXOCTETSr, &count);
            break;

        case snmpIfOutUcastPkts: /* ALL - mcast - bcast */
            ethsw_robo_counter_get(port, TXUNICASTPKTSr, &count);
            break;

        case snmpIfOutNUcastPkts:
            /* broadcast frames plus multicast frames */
            ethsw_robo_counter_get(port, TXBROADCASTPKTSr, &count_tmp);
            count=count_tmp;
            ethsw_robo_counter_get(port, TXMULTICASTPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpIfOutDiscards:
            ethsw_robo_counter_get(port, TXDROPPKTSr, &count);
            break;

        case snmpIfOutErrors:
            ethsw_robo_counter_get(port, TXEXCESSIVECOLLISIONr, &count_tmp);
            count=count_tmp;
            ethsw_robo_counter_get(port, TXLATECOLLISIONr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpIfOutQLen:
            return BCM_E_UNAVAIL;
            break;

        case snmpIpInReceives:
            return BCM_E_UNAVAIL;
            break;

        case snmpIpInHdrErrors:
            return BCM_E_UNAVAIL;
            break;

        case snmpIpForwDatagrams:
            return BCM_E_UNAVAIL;
            break;

        case snmpIpInDiscards:
            return BCM_E_UNAVAIL;
            break;

        /* *** RFC 1493 *** */

        case snmpDot1dBasePortDelayExceededDiscards:
            return BCM_E_UNAVAIL;
            break;

        case snmpDot1dBasePortMtuExceededDiscards:
            /* robo not suppport */
            return BCM_E_UNAVAIL;
            break;

        case snmpDot1dTpPortInFrames:
            ethsw_robo_counter_get(port, RXUNICASTPKTSr, &count_tmp);
            count=count_tmp;
            ethsw_robo_counter_get(port, RXMULTICASTPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXBROADCASTPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpDot1dTpPortOutFrames:
            ethsw_robo_counter_get(port, TXUNICASTPKTSr, &count_tmp);
            count=count_tmp;
            ethsw_robo_counter_get(port, TXMULTICASTPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, TXBROADCASTPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpDot1dPortInDiscards:
            ethsw_robo_counter_get(port, RXDROPPKTSr, &count);
            break;

        /* *** RFC 1757 *** */
        case snmpEtherStatsDropEvents:
            ethsw_robo_counter_get(port, TXDROPPKTSr, &count_tmp);
            count=count_tmp;
            ethsw_robo_counter_get(port, RXDROPPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpEtherStatsMulticastPkts:
            ethsw_robo_counter_get(port, RXMULTICASTPKTSr, &count);
            break;

        case snmpEtherStatsBroadcastPkts:
            ethsw_robo_counter_get(port, RXBROADCASTPKTSr, &count);
            break;

        case snmpEtherStatsUndersizePkts:
            ethsw_robo_counter_get(port, RXUNDERSIZEPKTSr, &count);
            break;

        case snmpEtherStatsFragments:
            ethsw_robo_counter_get(port, RXFRAGMENTSr,&count);
            break;

        case snmpEtherStatsPkts64Octets:
            ethsw_robo_counter_get(port, PKTS64OCTETSr,&count);
            break;

        case snmpEtherStatsPkts65to127Octets:
            ethsw_robo_counter_get(port, PKTS65TO127OCTETSr,&count);
            break;

        case snmpEtherStatsPkts128to255Octets:
            ethsw_robo_counter_get(port, PKTS128TO255OCTETSr,&count);
            break;

        case snmpEtherStatsPkts256to511Octets:
            ethsw_robo_counter_get(port, PKTS256TO511OCTETSr,&count);
            break;

        case snmpEtherStatsPkts512to1023Octets:
            ethsw_robo_counter_get(port, PKTS512TO1023OCTETSr,&count);
            break;

        case snmpEtherStatsPkts1024to1518Octets:
            ethsw_robo_counter_get(port, PKTS1024TO1522OCTETSr,&count);
            break;

        case snmpEtherStatsOversizePkts:
            ethsw_robo_counter_get(port, RXOVERSIZEPKTSr,&count);
            break;

        case snmpEtherStatsJabbers:
            ethsw_robo_counter_get(port, RXJABBERSr,&count);
            break;

        case snmpEtherStatsOctets:
            ethsw_robo_counter_get(port, RXOCTETSr,&count);
            break;

        case snmpEtherStatsPkts:
            ethsw_robo_counter_get(port, RXUNICASTPKTSr,&count_tmp);
            count=count_tmp;
            ethsw_robo_counter_get(port, RXMULTICASTPKTSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXBROADCASTPKTSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXALIGNMENTERRORSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXFCSERRORSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXFRAGMENTSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXOVERSIZEPKTSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXJABBERSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpEtherStatsCollisions:
            ethsw_robo_counter_get(port, TXCOLLISIONSr,&count);
            break;

        case snmpEtherStatsCRCAlignErrors:
            /* CRC errors + alignment errors */
            ethsw_robo_counter_get(port, RXALIGNMENTERRORSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXFCSERRORSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpEtherStatsTXNoErrors:
            ethsw_robo_counter_get(port, TXUNICASTPKTSr, &count_tmp);
            count=count_tmp;
            ethsw_robo_counter_get(port, TXMULTICASTPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, TXBROADCASTPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpEtherStatsRXNoErrors:
            ethsw_robo_counter_get(port, RXUNICASTPKTSr, &count_tmp);
            count=count_tmp;
            ethsw_robo_counter_get(port, RXMULTICASTPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXBROADCASTPKTSr, &count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        /* RFC 1643 */
        case snmpDot3StatsInternalMacReceiveErrors:
            ethsw_robo_counter_get(port, RXDROPPKTSr,&count);
            break;

        case snmpDot3StatsFrameTooLongs:
            ethsw_robo_counter_get(port, RXOVERSIZEPKTSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            ethsw_robo_counter_get(port, RXJABBERSr,&count_tmp);
            COMPILER_64_ADD_64(count, count_tmp);
            break;

        case snmpDot3StatsAlignmentErrors:  /* *** RFC 2665 *** */
            ethsw_robo_counter_get(port, RXALIGNMENTERRORSr,&count);
            break;

        case snmpDot3StatsFCSErrors:    /* *** RFC 2665 *** */
            ethsw_robo_counter_get(port, RXFCSERRORSr,&count);
            break;

        case snmpDot3StatsInternalMacTransmitErrors:
            ethsw_robo_counter_get(port, TXDROPPKTSr,&count);
            break;

        case snmpDot3StatsSingleCollisionFrames:
            /* *** RFC 2665 *** */
            ethsw_robo_counter_get(port, TXSINGLECOLLISIONr,&count);
            break;

        case snmpDot3StatsMultipleCollisionFrames:
            /* *** RFC 2665 *** */
            ethsw_robo_counter_get(port, TXMULTIPLECOLLISIONr,&count);
            break;

        case snmpDot3StatsDeferredTransmissions:
            ethsw_robo_counter_get(port, TXDEFERREDTRANSMITr,&count);
            break;

        case snmpDot3StatsLateCollisions:
            ethsw_robo_counter_get(port, TXLATECOLLISIONr,&count);
            break;

        case snmpDot3StatsExcessiveCollisions:
            ethsw_robo_counter_get(port, TXEXCESSIVECOLLISIONr,&count);
            break;

        case snmpDot3StatsCarrierSenseErrors:
            return BCM_E_UNAVAIL;
            break;

        case snmpDot3StatsSQETTestErrors:
            /* not support for BCM5380 */
            return BCM_E_UNAVAIL;
            break;


        /* *** RFC 2665 *** some object same as RFC 1643 */

        case snmpDot3StatsSymbolErrors:
            return BCM_E_UNAVAIL;
            break;

        case snmpDot3ControlInUnknownOpcodes:
            return BCM_E_UNAVAIL;
            break;

        case snmpDot3InPauseFrames:
            ethsw_robo_counter_get(port, RXPAUSEPKTSr, &count);
            break;

        case snmpDot3OutPauseFrames:
            ethsw_robo_counter_get(port, TXPAUSEPKTSr, &count);
            break;

        /*** RFC 2233 ***/
        case snmpIfHCInOctets:
            ethsw_robo_counter_get(port, RXOCTETSr,&count);
            break;

        case snmpIfHCInUcastPkts:
            ethsw_robo_counter_get(port, RXUNICASTPKTSr,&count);
            break;

        case snmpIfHCInMulticastPkts:
            ethsw_robo_counter_get(port, RXMULTICASTPKTSr,&count);
            break;

        case snmpIfHCInBroadcastPkts:
            ethsw_robo_counter_get(port, RXBROADCASTPKTSr,&count);
            break;

        case snmpIfHCOutOctets:
            ethsw_robo_counter_get(port, TXOCTETSr,&count);
            break;

        case snmpIfHCOutUcastPkts:
            ethsw_robo_counter_get(port, TXUNICASTPKTSr,&count);
            break;

        case snmpIfHCOutMulticastPkts:
            ethsw_robo_counter_get(port, TXMULTICASTPKTSr,&count);
            break;

        case snmpIfHCOutBroadcastPckts:
            ethsw_robo_counter_get(port, TXBROADCASTPKTSr,&count);
            break;

        /*** RFC 2465 ***/
        case snmpIpv6IfStatsInReceives:
        case snmpIpv6IfStatsInHdrErrors:
        case snmpIpv6IfStatsInAddrErrors:
        case snmpIpv6IfStatsInDiscards:
        case snmpIpv6IfStatsOutForwDatagrams:
        case snmpIpv6IfStatsOutDiscards:
        case snmpIpv6IfStatsInMcastPkts:
        case snmpIpv6IfStatsOutMcastPkts:
            val = 0;
            return BCM_E_UNAVAIL;
            break;

        case snmpBcmIPMCBridgedPckts:
        case snmpBcmIPMCRoutedPckts:
        case snmpBcmIPMCInDroppedPckts:
        case snmpBcmIPMCOutDroppedPckts:
            val = 0;
            return BCM_E_UNAVAIL;

        case snmpBcmEtherStatsPkts1519to1522Octets:
            val = 0;
            return BCM_E_UNAVAIL;

        case snmpBcmEtherStatsPkts1522to2047Octets:
            ethsw_robo_counter_get(port, PKTS1523TO2047r,&count);
            break;

        case snmpBcmEtherStatsPkts2048to4095Octets:
            ethsw_robo_counter_get(port, PKTS2048TO4095r,&count);
            break;

        case snmpBcmEtherStatsPkts4095to9216Octets:
            val = 0;
            return BCM_E_UNAVAIL;

        default:
            val = 0;
            BCM_ENET_DEBUG("Not supported\n");
            return BCM_E_PARAM;
    }

    BCM_ENET_INFO("count = 0x%llx", count);
    e->counter_val = count;
    BCM_ENET_INFO("e->counter_val = 0x%llx", e->counter_val);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcmeapi_ioctl_ethsw_clear_port_stats
 * Purpose:
 *      Clear the software accumulated counters for a given port
 * Parameters:
 *      e->port - StrataSwitch port #.
 * Returns:
 *      BCM_E_XXX
 */
int bcmeapi_ioctl_ethsw_clear_port_stats(struct ethswctl_data *e)
{
    bcm_port_t port = e->port;
    int i = 0;

    for (i = 0; i < MAXNUMCNTRS; i++) {

        /* Update the S/W counter */
        COMPILER_64_SET(counter_sw_val[port][i], 0, 0);
        COMPILER_64_SET(counter_delta[port][i], 0, 0);

        /* Do not update the H/W counter */
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcmeapi_ioctl_ethsw_clear_stats
 * Purpose:
 *      Clear the software accumulated counters
 * Returns:
 *      BCM_E_XXX
 */
int bcmeapi_ioctl_ethsw_clear_stats(uint32_t portmap)
{
    bcm_port_t port;
    int i = 0;

    PBMP_ITER(portmap, port) {
        for (i = 0; i < MAXNUMCNTRS; i++) {
            /* Update the S/W counter */
            COMPILER_64_SET(counter_sw_val[port][i], 0, 0);
            COMPILER_64_SET(counter_delta[port][i], 0, 0);
            /* Do not update the H/W counter */
        }
    }

    return BCM_E_NONE;
}

/*
* The two functions below allow gathering SW tx/rx counters.
* Is used by wlan driver to stop calibration
* when heavy traffic runs between Eth Port, to avoid Eth pkt loss
*/
unsigned int sw_tx, sw_rx;

/*
* This function gathers the statistics every 1 second from the ethernet
* poll function. Do not create your own polling function that would call it,
* as this would interfere with power management and reduce power savings.
*/
void __ethsw_get_txrx_imp_port_pkts(void)
{
    volatile EthSwMIBRegs *e = ((volatile EthSwMIBRegs *)
                                (SWITCH_BASE + 0x2000 + ((MIPS_PORT_ID) * 0x100)));

#if defined(CONFIG_BCM_ETH_PWRSAVE)
    unsigned long flags;
    spin_lock_irqsave(&bcmsw_pll_control_lock, flags);
    ethsw_phy_pll_up(0);
#endif

    sw_tx = e->TxUnicastPkts + e->TxMulticastPkts + e->TxBroadcastPkts;
    sw_rx = e->RxUnicastPkts + e->RxMulticastPkts + e->RxBroadcastPkts;

#if defined(CONFIG_BCM_ETH_PWRSAVE)
    spin_unlock_irqrestore(&bcmsw_pll_control_lock, flags);
#endif

    return;
}

/*
* This function provides the packet counts with up to 1 seconds delay.
*/
void ethsw_get_txrx_imp_port_pkts(unsigned int *tx, unsigned int *rx)
{
    *tx = sw_tx;
    *rx = sw_rx;

    return;
}
EXPORT_SYMBOL(ethsw_get_txrx_imp_port_pkts);

#if defined(CONFIG_BCM963268)
/*
*------------------------------------------------------------------------------
* Function   : remove_arls_on_port_5398_workaround
* Description: Removes/invalidates all ARLs on specified port (flush all on port)
* Input      : Port number where to search
* Design Note: This is a workaround because ARL entries with bit 39 set don't get
*              flushed by the HW switch when flushing on specific port
*------------------------------------------------------------------------------
*/
int remove_arls_on_port_5398_workaround(uint8_t port, uint8_t age_static)
{
    uint16_t v16;
    int timeout, first = 1;
    uint32_t first_mac_lo = 0, first_vid_mac_hi = 0;
    uint32_t cur_lo = 0, cur_hi = 0, v32=0;
    uint16_t cur_data = 0;
    uint8_t v8;

    /* Setup ARL Search */
    v16 = ARL_SRCH_CTRL_START_DONE;
    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL, (uint8_t *)&v16, 2);
    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_INDX_LO, (uint8_t *)&v16, 2);
    /* Read the complete ARL table */
    while (v16 & ARL_SRCH_CTRL_START_DONE) {
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL, (uint8_t *)&v16, 2);
        timeout = 1000;
        while((v16 & ARL_SRCH_CTRL_SR_VALID) == 0) {
            ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL, (uint8_t *)&v16, 2);
            if (v16 & ARL_SRCH_CTRL_START_DONE) {
                mdelay(1);
                if (timeout-- <= 0) {
                    return 0;
                }
            } else {
                return 0;
            }
        }
        /* Grab the lo and hi MAC entry */
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_MAC_LO_ENTRY,
                   (uint8_t *)&cur_lo, 4);
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_VID_MAC_HI_ENTRY,
                   (uint8_t *)&cur_hi, 4);
        /* Store the first MAC read */
        if (first) {
            first_mac_lo = cur_lo;
            first_vid_mac_hi = cur_hi;
            first = 0;
        } else if ((first_mac_lo == cur_lo) &&
                   (first_vid_mac_hi == cur_hi)) {
            /* Bail out if all the entries read */
            break;
        }
        /* Grab the data part of ARL entry */
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_DATA_ENTRY,
                   (uint8_t *)&cur_data, 2);

        BCM_ENET_DEBUG("cur_lo = 0x%x cur_hi = 0x%x cur_data = 0x%x\n", cur_lo,cur_hi,cur_data);
        /* Check if found the matching ARL entry (based on port # only) */
        if (port == (((cur_data & 0x1f)>>1)) && (age_static || (!age_static && !(cur_data & 0x8000))))
        { /* found a matching entry ; invalidate it */
            v16 = cur_lo & 0xFFFF; /* Re-using v16 - no problem we go out of this function */
            ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_INDX_LO, (uint8_t *)&v16, 2);
            v32 = (cur_lo >> 16) | (cur_hi << 16);
            ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_INDX_HI, (uint8_t *)&v32, 4);
            v16 = (cur_hi >> 16) & 0xFFFF;
            ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_VLAN_INDX, (uint8_t *)&v16, 2);
            v8 = ARL_TBL_CTRL_START_DONE | ARL_TBL_CTRL_READ;
            ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            timeout = 10;
            while(v8 & ARL_TBL_CTRL_START_DONE) {
                mdelay(1);
                if (timeout-- <= 0)  {
                    break;
                }
                ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            }
            ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_LO_ENTRY,
                       (uint8_t *)&cur_lo, 4);
            ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_VID_MAC_HI_ENTRY,
                       (uint8_t *)&cur_hi, 4);
            /* Invalidate the entry -> clear valid bit
             * Actually valid bit is always NOT set on read - so this is redundant */
            cur_data &= 0x7FFF; 
            ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_DATA_ENTRY,
                       (uint8_t *)&cur_data, 2);
            v8 = ARL_TBL_CTRL_START_DONE;
            ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            timeout = 10;
            ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            while(v8 & ARL_TBL_CTRL_START_DONE) {
                mdelay(1);
                if (timeout-- <= 0)  {
                    break;
                }
                ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            }
        }
    }
    
	return 0; /* Actually this is error but no-one should care the return value */
}
#endif

int enet_arl_search(u8 *mac, u32 *vid, u32 *val, int op )
{
    int timeout, count = 0;
    uint32_t v32lo, v32hi, mac_lo = 0;
    uint16_t v16, mac_hi = 0;

    if (mac)
    {
            mac_hi = __be16_to_cpu(*(uint16 *)mac);
            mac_lo = (__be16_to_cpu(*(uint16 *)(mac + 2)) << 16) |
                __be16_to_cpu(*(uint16 *)(mac + 4)); /* For mac 2 byte alignment */
    }

    v16 = ARL_SRCH_CTRL_START_DONE;
    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL, (uint8_t *)&v16, 2);

    if (op == TYPE_DUMP) printk("\nInternal Switch ARL Dump:\n");
    for(ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL, (uint8_t *)&v16, 2);
        (v16 & ARL_SRCH_CTRL_START_DONE);
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL, (uint8_t *)&v16, 2))
    {
        for(timeout = 1000; 
                (v16 & ARL_SRCH_CTRL_SR_VALID) == 0 && (v16 & ARL_SRCH_CTRL_START_DONE);
                mdelay(1), ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL, (uint8_t *)&v16, 2), --timeout);

        if (timeout <= 0)
        {
            printk("ARL Search Timeout for Valid to be 1 \n");
            return BCM_E_NONE;
        }

        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_MAC_LO_ENTRY, (uint8_t *)&v32lo, 4);
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_VID_MAC_HI_ENTRY, (uint8_t *)&v32hi, 4);
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_DATA_ENTRY, (uint8_t *)&v16, 2);

        switch(op)
        {
            case TYPE_DUMP:
                if (count % 10 == 0)
                {
                    printk("  No: VLAN  MAC          DATA"
                            "(15:Valid,14:Static,13:Age,12-10:Pri,8-0:Port/Pmap)\n");
                }
                printk("%4d: %04d  %04x%08x 0x%04x\n", (count+1), (int)(v32hi >> 16) & 0xFFFF,
                        (unsigned int)(v32hi & 0xFFFF), (unsigned int)v32lo, ((1<<15)|(v16 & 0xf800)>>1)|((v16&0x3fe)>>1));
                count++;
                break;
            case TYPE_GET:
                if ((v32hi & 0xffff) == mac_hi && v32lo == mac_lo &&
                        (*vid == -1 || ((v32hi >> 16) & 0xffff) == *vid))
                {
                    *vid = (v32hi >> 16) & 0xffff;
                    *val = v16;
                    return 1;
                }
                break;
            case TYPE_SET:
                if ((mac_hi == (v32hi & 0xFFFF)) && (mac_lo == v32lo))
                {
                    enet_arl_write(mac, (v32hi >> 16)&0xffff, 0);
                }
                break;
        }
    }
    if (op == TYPE_DUMP) printk("Internal Switch ARL Dump Done: Total %d entries\n", count);
    if (op == TYPE_GET) return 0;
    return 1;
}

static void enet_arl_access_prepare( uint8_t *mac,  u16 vid)
{
    int timeout = 100;
    uint16_t v16;
    uint8_t v8;
    uint32_t v32;

    v16 = (mac[4] << 8)| (mac[5]);
    BCM_ENET_INFO("mac_lo (16-bit) = 0x%x -> 0x502", v16);
    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_INDX_LO, (uint8_t *)&v16, 2);

    v32 = (mac[0]<<24) | (mac[1] << 16) | (mac[2] << 8) | (mac[3]);
    BCM_ENET_INFO("mac_hi (32-bit) = 0x%x -> 0x504", (unsigned int)v32);
    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_INDX_HI, (uint8_t *)&v32, 4);

    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_VLAN_INDX, (uint8_t *)&vid, 2);
    v8 = ARL_TBL_CTRL_START_DONE | ARL_TBL_CTRL_READ;
    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);

    for (ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            (v8 & ARL_TBL_CTRL_START_DONE) && --timeout > 0;
            ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1))
    {
        mdelay(1);
    }

    if (timeout <= 0)
    {
        printk("Timeout Waiting for ARL Read Access Done \n");
    }
}

void enet_arl_write(uint8_t *mac, uint16_t vid, int val)
{
    int timeout;
    uint16_t v16;
    uint8_t v8;
    uint32_t v32;

    enet_arl_access_prepare( mac, vid);

    v32 = (mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) |mac[5];
    BCM_ENET_INFO("mac_lo (32-bit) = 0x%x -> 0x510", (unsigned int)v32);
    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_MAC_LO_ENTRY, (uint8_t *)&v32, 4);

    v32 = (mac[0] << 8) | mac[1] | (vid << 16);
    BCM_ENET_INFO("mac_hi (32-bit) = 0x%x -> 0x514", (unsigned int)v32);
    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_VID_MAC_HI_ENTRY, (uint8_t *)&v32, 4);

    v16 = val & 0xffff;
    BCM_ENET_INFO("data (16-bit) = 0x%x -> 0x518", v16);
    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_DATA_ENTRY, (uint8_t *)&v16, 2);

    v8 = ARL_TBL_CTRL_START_DONE;
    ethsw_wreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);

    for(timeout = 100, ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
        (v8 & ARL_TBL_CTRL_START_DONE) && --timeout > 0;
        ethsw_rreg(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1))
    {
        mdelay(1);
    }

    if (timeout <= 0)
    {
            printk("Timeout Waiting for ARL Write Access Done\n");
    }
}

void enet_arl_dump_multiport_arl(void)
{
    uint8 v8, addr[6];
    uint32 vect;
    int i, enabled;

    /* Enable multiport addresses */
    ethsw_rreg(PAGE_ARLCTRL, REG_ARLCFG, &v8, 1);
    enabled = v8 & (MULTIPORT_ADDR_EN_M << MULTIPORT_ADDR_EN_S);
    printk("\nInternal Switch Multiport Address Dump: Function %s\n", enabled? "Enabled": "Disabled");

    if (!enabled)
    {
        return;
    }

    printk("  No:  %-12s Port Map\n", "Mac Address");
    for (i=0; i<2; i++)
    {
        ethsw_rreg(PAGE_ARLCTRL, REG_MULTIPORT_ADDR1_LO + i*16,
                addr, 6);
        ethsw_rreg(PAGE_ARLCTRL, REG_MULTIPORT_VECTOR1 + i*16, 
                (uint8 *)&vect, 4);
        printk("%4d:  %02x%02x%02x%02x%02x%02x %04x\n",
                (i+1),
                addr[0],
                addr[1],
                addr[2],
                addr[3],
                addr[4],
                addr[5],
                (int)vect);
    }
    printk("Internal Switch Multiport Address Dump Done.\n");
}



int bcmeapi_ioctl_ethsw_port_default_tag_config(struct ethswctl_data *e)
{
    uint16_t v16;

    if (e->port >= TOTAL_SWITCH_PORTS) {
        printk("Invalid Switch Port\n");
        return BCM_E_ERROR;
    }

    down(&bcm_ethlock_switch_config);

    BCM_ENET_DEBUG("Given port: %02d\n ", e->port);
    if (e->type == TYPE_GET) {
        ethsw_rreg(PAGE_8021Q_VLAN, REG_VLAN_DEFAULT_TAG + (e->port * 2),
                (uint8_t *)&v16, 2);
        e->priority = v16;
        BCM_ENET_DEBUG("e->priority is = %02d", e->priority);
    } else {
        BCM_ENET_DEBUG("Given priority: %02d\n ", e->priority);
        ethsw_rreg(PAGE_8021Q_VLAN, REG_VLAN_DEFAULT_TAG + (e->port * 2),
                (uint8_t *)&v16, 2);
        v16 &= (~(DEFAULT_TAG_PRIO_M << DEFAULT_TAG_PRIO_S));
        v16 |= ((e->priority & DEFAULT_TAG_PRIO_M) << DEFAULT_TAG_PRIO_S);
        ethsw_wreg(PAGE_8021Q_VLAN, REG_VLAN_DEFAULT_TAG + (e->port * 2),
                (uint8_t *)&v16, 2);
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}


void bcmeapi_reset_mib(void)
{
#ifdef REPORT_HARDWARE_STATS
    uint8_t val8;

    ethsw_rreg(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &val8, 1);
    val8 |= GLOBAL_CFG_RESET_MIB;
    ethsw_wreg(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &val8, 1);
    val8 &= (~GLOBAL_CFG_RESET_MIB);
    ethsw_wreg(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &val8, 1);

#if defined(CONFIG_BCM_GMAC)
    gmac_reset_mib();
#endif
#endif

    return;
}

int bcmeapi_ioctl_ethsw_port_loopback(struct ethswctl_data *e, int phy_id)
{
    uint32_t val32;
    uint8_t v8, phy_connected = 0;
    uint16_t v16;

    BCM_ENET_DEBUG("Given physical port %d", e->port);
    if (e->port >= EPHY_PORTS && e->port != USB_PORT_ID) {
        BCM_ENET_DEBUG("Invalid port %d ", e->port);
        return -EINVAL;
    }
    BCM_ENET_DEBUG("Given phy id %d", phy_id);
    if ((e->port < EPHY_PORTS) && IsPhyConnected(phy_id)) {
        phy_connected = 1;
    }

    if (e->type == TYPE_GET) {
        e->ret_val = port_in_loopback_mode[e->port];
        BCM_ENET_DEBUG("The port %d loopback status: %02d\n ", e->port, e->ret_val);
    } else {
        BCM_ENET_DEBUG("Given enable/disable control %02x\n ", e->val);
        if (e->port == USB_PORT_ID) {
            if (e->val) {
                port_in_loopback_mode[e->port] = 1;
                ethsw_rreg(PAGE_CONTROL, REG_SWPKT_CTRL_USB, (uint8_t *)&swpkt_ctrl_usb, 4);
                swpkt_ctrl_usb_saved = 1;
                val32 = USB_SWPKTBUS_LOOPBACK_VAL;
                ethsw_wreg(PAGE_CONTROL, REG_SWPKT_CTRL_USB, (uint8_t *)&val32, 4);
                v8 = LINKDOWN_OVERRIDE_VAL;
                ethsw_wreg(PAGE_CONTROL, REG_PORT_STATE + USB_PORT_ID, &v8, 1);
            } else {
                v8 = LINKDOWN_OVERRIDE_VAL & ~(REG_PORT_STATE_LNK);
                ethsw_wreg(PAGE_CONTROL, REG_PORT_STATE + e->port, &v8, 1);
                if (swpkt_ctrl_usb_saved) {
                    ethsw_wreg(PAGE_CONTROL, REG_SWPKT_CTRL_USB, (uint8_t *)&swpkt_ctrl_usb, 4);
                } else {
                    val32 = 0;
                    ethsw_wreg(PAGE_CONTROL, REG_SWPKT_CTRL_USB, (uint8_t *)&val32, 4);
                }
                port_in_loopback_mode[e->port] = 0;
            }
        } else {
            if (e->val) {
                if (phy_connected) {
                    port_in_loopback_mode[e->port] = 1;
                    ethsw_phy_rreg(phy_id, MII_BMCR, &v16);
                    v16 |= BMCR_LOOPBACK;
                    ethsw_phy_wreg(phy_id, MII_BMCR, &v16);
                } else {
                    printk("No Phy");
                }
                v8 = LINKDOWN_OVERRIDE_VAL;
                ethsw_wreg(PAGE_CONTROL, REG_PORT_STATE + e->port, &v8, 1);
            } else {
                v8 = LINKDOWN_OVERRIDE_VAL & ~(REG_PORT_STATE_LNK);
                ethsw_wreg(PAGE_CONTROL, REG_PORT_STATE + e->port, &v8, 1);
                if (phy_connected) {
                    ethsw_phy_rreg(phy_id, MII_BMCR, &v16);
                    v16 &= (~BMCR_LOOPBACK);
                    ethsw_phy_wreg(phy_id, MII_BMCR, &v16);
                } else {
                    printk("No Phy");
                }
                port_in_loopback_mode[e->port] = 0;
            }
        }
    }

    return BCM_E_NONE;
}

/************** Debug Functions ******************/
static void ethsw_dump_page0(void)
{
    int i = 0, page = 0;
    volatile EthSwRegs *e = ETHSWREG;

#if defined(CONFIG_BCM_ETH_PWRSAVE)
    unsigned long flags;
    spin_lock_irqsave(&bcmsw_pll_control_lock, flags);
    ethsw_phy_pll_up(0);
#endif

    printk("#The Page0 Registers\n");
    for (i=0; i<9; i++) {
        printk("%02x %02x = 0x%02x (%u)\n", page,
                ((int)&e->port_traffic_ctrl[i]) & 0xFF, e->port_traffic_ctrl[i],
                e->port_traffic_ctrl[i]); /* 0x00 - 0x08 */
    }
    printk("%02x %02x 0x%02x (%u)\n", page, ((int)&e->switch_mode) & 0xFF,
            e->switch_mode, e->switch_mode); /* 0x0b */
    printk("%02x %02x 0x%04x (%u)\n", page, ((int)&e->pause_quanta) & 0xFF,
            e->pause_quanta, e->pause_quanta); /*0x0c */
    printk("%02x %02x 0x%02x (%u)\n", page, ((int)&e->imp_port_state) & 0xFF,
            e->imp_port_state, e->imp_port_state); /*0x0e */
    printk("%02x %02x 0x%02x (%u)\n", page, ((int)&e->led_refresh) & 0xFF,
            e->led_refresh, e->led_refresh); /* 0x0f */
    for (i=0; i<2; i++) {
        printk("%02x %02x 0x%04x (%u)\n", page,
                ((int)&e->led_function[i]) & 0xFF, e->led_function[i],
                e->led_function[i]); /* 0x10 */
    }
    printk("%02x %02x 0x%04x (%u)\n", page, ((int)&e->led_function_map) & 0xFF,
            e->led_function_map, e->led_function_map); /* 0x14 */
    printk("%02x %02x 0x%04x (%u)\n", page, ((int)&e->led_enable_map) & 0xFF,
            e->led_enable_map, e->led_enable_map); /* 0x16 */
    printk("%02x %02x 0x%04x (%u)\n", page, ((int)&e->led_mode_map0) & 0xFF,
            e->led_mode_map0, e->led_mode_map0); /* 0x18 */
    printk("%02x %02x 0x%04x (%u)\n", page, ((int)&e->led_function_map1) & 0xFF,
            e->led_function_map1, e->led_function_map1); /* 0x1a */
    printk("%02x %02x 0x%02x (%u)\n", page, ((int)&e->reserved2[3]) & 0xFF,
            e->reserved2[3], e->reserved2[3]); /* 0x1f */
    printk("%02x %02x 0x%02x (%u)\n", page, ((int)&e->port_forward_ctrl) & 0xFF,
            e->port_forward_ctrl, e->port_forward_ctrl); /* 0x21 */
    printk("%02x %02x 0x%04x (%u)\n", page, ((int)&e->protected_port_selection)
            & 0xFF, e->protected_port_selection,
            e->protected_port_selection); /* 0x24 */
    printk("%02x %02x 0x%04x (%u)\n", page, ((int)&e->wan_port_select) & 0xFF,
            e->wan_port_select, e->wan_port_select); /* 0x26 */
    printk("%02x %02x 0x%08x (%u)\n", page, ((int)&e->pause_capability)
            & 0xFF, e->pause_capability, e->pause_capability);/*0x28*/
    printk("%02x %02x 0x%02x (%u)\n", page,
            ((int)&e->reserved_multicast_control) & 0xFF, e->reserved_multicast_control,
            e->reserved_multicast_control); /* 0x2f */
    printk("%02x %02x 0x%02x (%u)\n", page, ((int)&e->txq_flush_mode_control) &
            0xFF, e->txq_flush_mode_control, e->txq_flush_mode_control); /* 0x31 */
    printk("%02x %02x 0x%04x (%u)\n", page, ((int)&e->ulf_forward_map) & 0xFF,
            e->ulf_forward_map, e->ulf_forward_map); /* 0x32 */
    printk("%02x %02x 0x%04x (%u)\n", page, ((int)&e->mlf_forward_map) & 0xFF,
            e->mlf_forward_map, e->mlf_forward_map); /* 0x34 */
    printk("%02x %02x 0x%04x (%u)\n", page, ((int)&e->mlf_impc_forward_map) &
            0xFF, e->mlf_impc_forward_map, e->mlf_impc_forward_map); /* 0x36 */
    printk("%02x %02x 0x%04x (%u)\n", page,
            ((int)&e->pause_pass_through_for_rx) & 0xFF, e->pause_pass_through_for_rx,
            e->pause_pass_through_for_rx); /* 0x38 */
    printk("%02x %02x 0x%04x (%u)\n", page,
            ((int)&e->pause_pass_through_for_tx) & 0xFF, e->pause_pass_through_for_tx,
            e->pause_pass_through_for_tx); /* 0x3a */
    printk("%02x %02x 0x%04x (%u)\n", page, ((int)&e->disable_learning) & 0xFF,
            e->disable_learning, e->disable_learning); /* 0x3c */
    for (i=0; i<8; i++) {
        printk("%02x %02x 0x%02x (%u)\n", page,
                ((int)&e->port_state_override[i]) & 0xFF, e->port_state_override[i],
                e->port_state_override[i]); /* 0x58 - 0x5f */
    }
    printk("%02x %02x 0x%02x (%u)\n", page, ((int)&e->imp_rgmii_ctrl_p4) &
            0xFF, e->imp_rgmii_ctrl_p4, e->imp_rgmii_ctrl_p4); /* 0x64 */
#if !defined(CONFIG_BCM963381)
    /* Only 1 rgmii port 4 on 6318 */
    printk("%02x %02x 0x%02x (%u)\n", page, ((int)&e->imp_rgmii_ctrl_p5) &
            0xFF, e->imp_rgmii_ctrl_p5, e->imp_rgmii_ctrl_p5); /* 0x65 */
#endif
    printk("%02x %02x 0x%02x (%u)\n", page, ((int)&e->rgmii_timing_delay_p4) &
            0xFF, e->rgmii_timing_delay_p4, e->rgmii_timing_delay_p4); /* 0x6c */
#if !defined(CONFIG_BCM963381)
    printk("%02x %02x 0x%02x (%u)\n", page, ((int)&e->gmii_timing_delay_p5) &
            0xFF, e->gmii_timing_delay_p5, e->gmii_timing_delay_p5); /* 0x6d */
#endif
    printk("%02x %02x 0x%02x (%u)\n", page, ((int)&e->software_reset) & 0xFF,
            e->software_reset, e->software_reset); /* 0x79 */
        printk("%02x %02x 0x%02x (%u)\n", page, ((int)&e->pause_frame_detection) &
         0xFF, e->pause_frame_detection, e->pause_frame_detection); /* 0x80 */
        printk("%02x %02x 0x%02x (%u)\n", page, ((int)&e->fast_aging_ctrl) & 0xFF,
               e->fast_aging_ctrl, e->fast_aging_ctrl); /* 0x88 */
        printk("%02x %02x 0x%02x (%u)\n", page, ((int)&e->fast_aging_port) & 0xFF,
               e->fast_aging_port, e->fast_aging_port); /* 0x89 */
        printk("%02x %02x 0x%02x (%u)\n", page, ((int)&e->fast_aging_vid) & 0xFF,
               e->fast_aging_vid, e->fast_aging_vid); /* 0x8a */
#if !defined(CONFIG_BCM963381)
        printk("%02x %02x 0x%08x (%u)\n", page, ((int)&e->swpkt_ctrl_sar) & 0xFF,
               e->swpkt_ctrl_sar, e->swpkt_ctrl_sar); /*0xa0 */
        printk("%02x %02x 0x%08x (%u)\n", page, ((int)&e->swpkt_ctrl_usb) & 0xFF,
               e->swpkt_ctrl_usb, e->swpkt_ctrl_usb); /*0xa4 */
#endif /*defined(CONFIG_BCM96818)*/
        printk("%02x %02x 0x%04x (%u)\n", page, ((int)&e->iudma_ctrl) & 0xFF,
               e->iudma_ctrl, e->iudma_ctrl); /*0xa8 */
        printk("%02x %02x 0x%08x (%u)\n", page, ((int)&e->rxfilt_ctrl) & 0xFF,
               e->rxfilt_ctrl, e->rxfilt_ctrl); /*0xac */
        printk("%02x %02x 0x%08x (%u)\n", page, ((int)&e->mdio_ctrl) & 0xFF,
               e->mdio_ctrl, e->mdio_ctrl); /*0xb0 */
        printk("%02x %02x 0x%08x (%u)\n", page, ((int)&e->mdio_data) & 0xFF,
               e->mdio_data, e->mdio_data); /*0xb4 */
        printk("%02x %02x 0x%08x (%u)\n", page, ((int)&e->sw_mem_test) & 0xFF,
               e->sw_mem_test, e->sw_mem_test); /*0xe0 */

#if defined(CONFIG_BCM_ETH_PWRSAVE)
        spin_unlock_irqrestore(&bcmsw_pll_control_lock, flags);
#endif
}

int bcmeapi_ethsw_dump_mib(int port, int type, int queue)
{
    volatile EthSwMIBRegs *e;
    int base, ret = 0, i, qosCntReset = 0;
    uint8_t v8;
#if defined(CONFIG_BCM_ETH_PWRSAVE)
    unsigned long flags;
#endif

    if (port > 8) {
        printk("Invalid port number\n");
        return -1;
    }
    base = (SWITCH_BASE + 0x2000 + (port * 0x100));
    e = (volatile EthSwMIBRegs *)base;

    if (queue >= 4 && queue != -1)
    {
        printk("Invalid queue number %d\n", queue);
        return -1;
    }

    ethsw_rreg(PAGE_QOS, REG_QOS_GLOBAL_CTRL, &v8, 1);
    if (queue == -1)
    {
        queue = (v8 >> MIB_QOS_MONITOR_SET_S) & MIB_QOS_MONITOR_SET_M;
    }
    else    /* queue = 0 - 3 */
    {
        if ((qosCntReset = (queue != ((v8 >> MIB_QOS_MONITOR_SET_S) & MIB_QOS_MONITOR_SET_M))))
        {
            printk (" Warning: mornitoring queue changed from before %d to new %d.\n"
                    " All Port OoS Recieving Packet Counter will be reset now\n",
                    ((v8 >> MIB_QOS_MONITOR_SET_S) & MIB_QOS_MONITOR_SET_M), queue);
            v8 &= ~(MIB_QOS_MONITOR_SET_M << MIB_QOS_MONITOR_SET_S); 
            v8 |= queue << MIB_QOS_MONITOR_SET_S;
            ethsw_wreg(PAGE_QOS, REG_QOS_GLOBAL_CTRL, &v8, 1);

            /* Reset all ports' TxQoSPkts counter when mornitoring queue changed */
            for (i=0; i<=IMP_PORT_ID; i++)
            {
                ((EthSwMIBRegs *)(SWITCH_BASE + 0x2000 + i * 0x100))->TxQoSPkts = 0;
            }
        }
    }

    printk("\nInternal Switch Stats : Port# %d\n",port);

#if defined(CONFIG_BCM_ETH_PWRSAVE)
    spin_lock_irqsave(&bcmsw_pll_control_lock, flags);
    ethsw_phy_pll_up(0);
#endif

#if defined(CONFIG_BCM_GMAC)
    if (gmac_info_pg->enabled && (port == GMAC_PORT_ID) )
    {
        volatile GmacMIBRegs *g = (volatile GmacMIBRegs *)GMAC_MIB;

        /* Display Tx statistics */
        printk("\n");
        printk("TxUnicastPkts:          %10u\n",
                e->TxUnicastPkts + g->TxUnicastPkts);
        printk("TxMulticastPkts:        %10u\n",
                e->TxMulticastPkts + g->TxMulticastPkts);
        printk("TxBroadcastPkts:        %10u\n",
                e->TxBroadcastPkts + g->TxBroadcastPkts);
        printk("TxDropPkts:             %10u\n",
                e->TxDropPkts + (g->TxPkts - g->TxGoodPkts));

        if (type) {
            printk("TxOctetsLo:             %10u\n",
                    e->TxOctetsLo + g->TxOctetsLo);
            printk("TxOctetsHi:             %10u\n",
                    e->TxOctetsHi );
            printk("TxGoodPkts :            %10u\n",
                    g->TxGoodPkts + e->TxUnicastPkts + e->TxMulticastPkts + e->TxBroadcastPkts);
            printk("TxCol:                  %10u\n",
                    e->TxCol + g->TxCol);
            printk("TxSingleCol:            %10u\n",
                    e->TxSingleCol + g->TxSingleCol);
            printk("TxMultipleCol:          %10u\n",
                    e->TxMultipleCol + g->TxMultipleCol);
            printk("TxDeferredTx:           %10u\n",
                    e->TxDeferredTx + g->TxDeferredTx);
            printk("TxLateCol:              %10u\n",
                    e->TxLateCol + g->TxLateCol);
            printk("TxExcessiveCol:         %10u\n",
                    e->TxExcessiveCol + g->TxExcessiveCol);
            printk("TxFrameInDisc:          %10u\n",
                    e->TxFrameInDisc );
            printk("TxPausePkts:            %10u\n",
                    e->TxPausePkts + g->TxPausePkts);
            printk("TxQoSOctetsLo:          %10u\n",
                    e->TxQoSOctetsLo + g->TxOctetsLo);
            printk("TxQoSOctetsHi:          %10u\n",
                    e->TxQoSOctetsHi);
        }

        /* Display Rx statistics */
        printk("\n");
        printk("RxUnicastPkts:          %10u\n",
                e->RxUnicastPkts + g->RxUnicastPkts);
        printk("RxMulticastPkts:        %10u\n",
                e->RxMulticastPkts + g->RxMulticastPkts);
        printk("RxBroadcastPkts:        %10u\n",
                e->RxBroadcastPkts + g->RxBroadcastPkts);
        printk("RxDropPkts:             %10u\n",
                e->RxDropPkts + (g->RxPkts - g->RxGoodPkts));

        /* Display remaining rx stats only if requested */
        if (type) {
            printk("RxJabbers:              %10u\n",
                    e->RxJabbers + g->RxJabbers);
            printk("RxAlignErrs:            %10u\n",
                    e->RxAlignErrs + g->RxAlignErrs);
            printk("RxFCSErrs:              %10u\n",
                    e->RxFCSErrs + g->RxFCSErrs);
            printk("RxFragments:            %10u\n",
                    e->RxFragments + g->RxFragments);
            printk("RxOversizePkts:         %10u\n",
                    e->RxOversizePkts + g->RxOversizePkts);
            printk("RxExcessSizeDisc:       %10u\n",
                    e->RxExcessSizeDisc + g->RxExcessSizeDisc);
            printk("RxOctetsLo:             %10u\n",
                    e->RxOctetsLo + g->RxOctetsLo);
            printk("RxOctetsHi:             %10u\n",
                    e->RxOctetsHi);
            printk("RxUndersizePkts:        %10u\n",
                    e->RxUndersizePkts + g->RxUndersizePkts);
            printk("RxPausePkts:            %10u\n",
                    e->RxPausePkts + g->RxPausePkts);
            printk("RxGoodOctetsLo:         %10u\n",
                    e->RxGoodOctetsLo + g->RxOctetsLo);
            printk("RxGoodOctetsHi:         %10u\n",
                    e->RxGoodOctetsHi);
            printk("RxSAChanges:            %10u\n",
                    e->RxSAChanges);
            printk("RxSymbolError:          %10u\n",
                    e->RxSymbolError + g->RxSymbolError);
            printk("RxQoSPkts:              %10u\n",
                    e->RxQoSPkts + g->RxGoodPkts);
            printk("RxQoSOctetsLo:          %10u\n",
                    e->RxQoSOctetsLo + g->RxOctetsLo);
            printk("RxQoSOctetsHi:          %10u\n",
                    e->RxQoSOctetsHi);
            printk("RxPkts64Octets:         %10u\n",
                    e->Pkts64Octets + g->Pkts64Octets);
            printk("RxPkts65to127Octets:    %10u\n",
                    e->Pkts65to127Octets + g->Pkts65to127Octets);
            printk("RxPkts128to255Octets:   %10u\n",
                    e->Pkts128to255Octets + g->Pkts128to255Octets);
            printk("RxPkts256to511Octets:   %10u\n",
                    e->Pkts256to511Octets + g->Pkts256to511Octets);
            printk("RxPkts512to1023Octets:  %10u\n",
                    e->Pkts512to1023Octets + g->Pkts512to1023Octets);
            printk("RxPkts1024to1522Octets: %10u\n",
                    e->Pkts1024to1522Octets + 
                    (g->Pkts1024to1518Octets + g->Pkts1519to1522));
            printk("RxPkts1523to2047:       %10u\n",
                    e->Pkts1523to2047 + g->Pkts1523to2047);
            printk("RxPkts2048to4095:       %10u\n",
                    e->Pkts2048to4095 + g->Pkts2048to4095);
            printk("RxPkts4096to8191:       %10u\n",
                    e->Pkts4096to8191 + g->Pkts4096to8191);
            printk("RxPkts8192to9728:       %10u\n",
                    e->Pkts8192to9728);
        }

        goto ret;
    }
#endif

    /* Display Tx statistics */
    printk("\n");
    printk("TxUnicastPkts:          %10u\n", e->TxUnicastPkts);
    printk("TxMulticastPkts:        %10u\n",  e->TxMulticastPkts);
    printk("TxBroadcastPkts:        %10u\n", e->TxBroadcastPkts);
    printk("TxDropPkts:             %10u\n", e->TxDropPkts);

    /* Display remaining tx stats only if requested */
    if (type) {
        printk("TxOctetsLo:             %10u\n", e->TxOctetsLo);
        printk("TxOctetsHi:             %10u\n", e->TxOctetsHi);
        printk("TxSwitchQoSPkts:        %10u (Switch queue %d%s)\n", 
                e->TxQoSPkts, queue, qosCntReset? ", Note: The Counter was just reset by this command -q option":"");
        printk("TxGoodPkts :            %10u\n",
                e->TxUnicastPkts + e->TxMulticastPkts + e->TxBroadcastPkts);
        printk("TxCol:                  %10u\n", e->TxCol);
        printk("TxSingleCol:            %10u\n", e->TxSingleCol);
        printk("TxMultipleCol:          %10u\n", e->TxMultipleCol);
        printk("TxDeferredTx:           %10u\n", e->TxDeferredTx);
        printk("TxLateCol:              %10u\n", e->TxLateCol);
        printk("TxExcessiveCol:         %10u\n", e->TxExcessiveCol);
        printk("TxFrameInDisc:          %10u\n", e->TxFrameInDisc);
        printk("TxPausePkts:            %10u\n", e->TxPausePkts);
        printk("TxQoSOctetsLo:          %10u\n", e->TxQoSOctetsLo);
        printk("TxQoSOctetsHi:          %10u\n", e->TxQoSOctetsHi);
    }

    /* Display Rx statistics */
    printk("\n");
    printk("RxUnicastPkts:          %10u\n", e->RxUnicastPkts);
    printk("RxMulticastPkts:        %10u\n", e->RxMulticastPkts);
    printk("RxBroadcastPkts:        %10u\n", e->RxBroadcastPkts);
    printk("RxDropPkts:             %10u\n", e->RxDropPkts);

    /* Display remaining rx stats only if requested */
    if (type) {
        printk("RxJabbers:              %10u\n", e->RxJabbers);
        printk("RxAlignErrs:            %10u\n", e->RxAlignErrs);
        printk("RxFCSErrs:              %10u\n", e->RxFCSErrs);
        printk("RxFragments:            %10u\n", e->RxFragments);
        printk("RxOversizePkts:         %10u\n", e->RxOversizePkts);
        printk("RxExcessSizeDisc:       %10u\n", e->RxExcessSizeDisc);
        printk("RxOctetsLo:             %10u\n", e->RxOctetsLo);
        printk("RxOctetsHi:             %10u\n", e->RxOctetsHi);
        printk("RxUndersizePkts:        %10u\n", e->RxUndersizePkts);
        printk("RxPausePkts:            %10u\n", e->RxPausePkts);
        printk("RxGoodOctetsLo:         %10u\n", e->RxGoodOctetsLo);
        printk("RxGoodOctetsHi:         %10u\n", e->RxGoodOctetsHi);
        printk("RxSAChanges:            %10u\n", e->RxSAChanges);
        printk("RxSymbolError:          %10u\n", e->RxSymbolError);
        printk("RxQoSPkts:              %10u\n", e->RxQoSPkts);
        printk("RxQoSOctetsLo:          %10u\n", e->RxQoSOctetsLo);
        printk("RxQoSOctetsHi:          %10u\n", e->RxQoSOctetsHi);
        printk("RxPkts64Octets:         %10u\n", e->Pkts64Octets);
        printk("RxPkts65to127Octets:    %10u\n", e->Pkts65to127Octets);
        printk("RxPkts128to255Octets:   %10u\n", e->Pkts128to255Octets);
        printk("RxPkts256to511Octets:   %10u\n", e->Pkts256to511Octets);
        printk("RxPkts512to1023Octets:  %10u\n", e->Pkts512to1023Octets);
        printk("RxPkts1024to1522Octets: %10u\n", e->Pkts1024to1522Octets);
        printk("RxPkts1523to2047:       %10u\n", e->Pkts1523to2047);
        printk("RxPkts2048to4095:       %10u\n", e->Pkts2048to4095);
        printk("RxPkts4096to8191:       %10u\n", e->Pkts4096to8191);
        printk("RxPkts8192to9728:       %10u\n", e->Pkts8192to9728);
    }

#if defined(CONFIG_BCM_GMAC)
ret:
#endif

#if defined(CONFIG_BCM_ETH_PWRSAVE)
    spin_unlock_irqrestore(&bcmsw_pll_control_lock, flags);
#endif
    return ret;
}

void bcmeapi_ethsw_dump_page(int page)
{
    switch (page) {
        case 0:
            ethsw_dump_page0();
            break;

        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0x26:
        case 0x27:
        case 0x28:
            bcmeapi_ethsw_dump_mib(page - 0x20, 1, -1);
            break;

        default:
            printk("Invalid page or not yet implemented\n");
            break;
    }
}


int bcmeapi_ioctl_ethsw_port_jumbo_control(struct ethswctl_data *e)  // bill
{
    uint32 val32;

    if (e->type == TYPE_GET) {
        // Read & log current JUMBO configuration control register.
        ethsw_rreg(PAGE_JUMBO, REG_JUMBO_PORT_MASK_INT_SW, (uint8 *)&val32, 4);
        BCM_ENET_DEBUG("JUMBO_PORT_MASK = 0x%08X", (unsigned int)val32);
        e->ret_val = val32;
    } else {
        // Read & log current JUMBO configuration control register.
        ethsw_rreg(PAGE_JUMBO, REG_JUMBO_PORT_MASK_INT_SW, (uint8 *)&val32, 4);
        BCM_ENET_DEBUG("Old JUMBO_PORT_MASK = 0x%08X", (unsigned int)val32);

        // Setup JUMBO configuration control register.
        val32 = ConfigureJumboPort(val32, e->port, e->val);
        ethsw_wreg(PAGE_JUMBO, REG_JUMBO_PORT_MASK_INT_SW, (uint8 *)&val32, 4);
#if defined(CONFIG_BCM_GMAC)
        if (gmac_info_pg->enabled && (e->port == GMAC_PORT_ID)) {
            if (e->val) { // Jumbo ON
                gmac_intf_set_max_pkt_size(GMAC_MAX_JUMBO_FRM_LEN);
            } else {
                gmac_intf_set_max_pkt_size(GMAC_MAX_FRM_LEN);
            }
        }
#endif

        // Attempt to transfer register write value to user space & test for success.
        e->ret_val = val32;
    }

    return BCM_E_NONE;
}

#if defined(CONFIG_BCM963381)
void ethsw_reset_phy(int phy_id)
{
    int i;
    static int reg[] = {0, 4, 7, 9, 0x10, 0x18, 0x1a, 0x1b, 0x1d};
    uint16 val[ARRAY_SIZE(reg)];
    static int eee_reg[] = {3, 6, 0xe, 0xb};
    uint16 eee_val[ARRAY_SIZE(eee_reg)];
    uint16 v16;

    /* Save PHY configuration registers */
    for (i = 0; i<ARRAY_SIZE(reg); i++)
    {
        ethsw_phy_rreg(phy_id, reg[i], &val[i]);
    }
    for (i = 0; i<ARRAY_SIZE(eee_reg); i++)
    {
        ethsw_ephy_shadow_read(phy_id, 3, eee_reg[i], &eee_val[i]);
    }

    /* Reset PHY */
    v16 = val[0] | MII_CONTROL_RESET;
    ethsw_phy_wreg(phy_id, MII_CONTROL, &v16);
    msleep(1);
    ephy_adjust_afe(phy_id);
    if (val[0] & MII_CONTROL_AN_ENABLE)
    {
        val[0] |= MII_CONTROL_RESTART_AUTONEG;
        // On 63381, setting both duplex mode and autoneg causes interrupt to constantly fire
        val[0] &= ~MII_CONTROL_DUPLEX_MODE;
    }

    /* Restore PHY configuration registers, set configuration register 0 last */
    for (i = 0; i<ARRAY_SIZE(eee_reg); i++)
    {
        ethsw_ephy_shadow_write(phy_id, 3, eee_reg[i], &eee_val[i]);
    }
    for (i = ARRAY_SIZE(reg)-1; i >= 0; i--)
    {
        ethsw_phy_wreg(phy_id, reg[i], &val[i]);
    }
}

/* 
   Work around for 63381 PHY issue 
Return: 
1 indicated PHY checking needs to be rescheduled in future.
0 means port checking is done.
 */
static void bcm63381_phy_enable_rxcnt(int phy_id)
{
    uint16 v16;

    /* Enable and reset RxPkt Counter */
    v16 = 0x3;
    ethsw_ephy_shadow_write(phy_id, 3, 0x11, &v16);
    ethsw_ephy_shadow_read(phy_id, 3, 0x12, &v16);
    /* Clear link error */
    ethsw_phy_rreg(phy_id, 0x11, &v16);
}

#define SP100_CARRIER_ERROR_MASK ((1<<9)|(1<<8)|(1<<5))
static int bcm63381_phy_linkup_workaround(int phy_id)
{
    uint16 v16, pktCnt;
    PHY_STAT phys;

    /* Ensure the link status did not change */
    phys = ethsw_phyid_stat(phy_id);
    if (phys.lnk == 0 || phys.spd100 == 0)
    {
        return 0;
    }

    /* If we received a packet, PHY is working normally */    
    ethsw_ephy_shadow_read(phy_id, 3, 0x12, &pktCnt);
    if (pktCnt > 0)
    {
        ethsw_phy_rreg(phy_id, 0x11, &v16);
        return 0;
    }

    /* If Carrier error happens, reset phy */
    ethsw_phy_rreg(phy_id, 0x11, &v16);
    if ((v16 & SP100_CARRIER_ERROR_MASK) == SP100_CARRIER_ERROR_MASK)
    {
        printk ("**** Detected EPHY unusual status, reset PHY to recover\n");
        ethsw_reset_phy(phy_id);
        return 0;
    }

    /* No pkt received and no carrier error detected so reschedule the check */
    return 1;
}

static void bcm63381_phy_workaround_check(int log_port)
{
    int i, unit, phy_port, phy_id;
    static int port_checking = 0;

    /* If this is a link up event */
    if (log_port != -1)
    {
        /* Checking during Link change handling */
        unit = LOGICAL_PORT_TO_UNIT_NUMBER(log_port);
        phy_port = LOGICAL_PORT_TO_PHYSICAL_PORT(log_port);
        phy_id = enet_sw_port_to_phyid(unit, phy_port);

        if (IsExtPhyId(phy_id))
            return;

        bcm63381_phy_enable_rxcnt(phy_id);
        if (bcm63381_phy_linkup_workaround(phy_id) == 0)
        {
            port_checking &= ~(1<<phy_port);
        }
        else
        {
            port_checking |= 1<<phy_port;
        }
    }
    else
    {
        /* This is a 1 sec periodic check */
        for (i = 0; i < BP_MAX_SWITCH_PORTS && port_checking; i++)
        {
            if (port_checking & (1<<i))
            {
                phy_id = enet_sw_port_to_phyid(0, i);
                if (bcm63381_phy_linkup_workaround(phy_id) == 0)
                {
                    port_checking &= ~(1<<i);
                }
            }
        }
    }
}

void ephy_adjust_afe(unsigned int phy_id)
{
    uint32 chipRev = UtilGetChipRev();

    /* these EPHY AFE tuning only apply to A0 and A1 */
    if( (chipRev&0xf0) == 0xa0 )
    {
        //Zero out internal and external current trim values
        ethsw_ephy_write_bank2_reg(phy_id, 0x1a, 0x3b80);
        //Write analog control registers
        //AFE_RXCONFIG_0
        ethsw_ephy_write_bank3_reg(phy_id, 0xc0, 0xeb15);
        //AFE_RXCONFIG_1
        ethsw_ephy_write_bank3_reg(phy_id, 0xc1, 0x9a07);
        //AFE_TX_CONFIG
        ethsw_ephy_write_bank3_reg(phy_id, 0xc4, 0x0800);
        //AFE_RX_LP_COUNTER
        ethsw_ephy_write_bank3_reg(phy_id, 0xc3, 0x7fc4);
        //AFE_HPF_TRIM_OTHERS
        ethsw_ephy_write_bank3_reg(phy_id, 0xc8, 0x000b);
        //AFE PLLCTRL_4
        ethsw_ephy_write_bank3_reg(phy_id, 0x8c, 0x0e20);
        //Reset RC_CAL, R_CAL Engine
        ethsw_ephy_write_bank3_reg(phy_id, 0x23, 0x0006);
        //Disable Reset RC_CAL, R_CAL Engine
        ethsw_ephy_write_bank3_reg(phy_id, 0x23, 0x0000);
        //APD Workaround
        ethsw_ephy_write_bank3_reg(phy_id, 0x21, 0x0080);
    } else {
        // Set internal and external current trim values INT_trim = -2, Ext_trim =0
        ethsw_ephy_write_bank2_reg(phy_id, 0x1a, 0x3be0);
        // Cal reset
        ethsw_ephy_write_bank3_reg(phy_id, 0x23, 0x0006);
        // Cal reset disable
        ethsw_ephy_write_bank3_reg(phy_id, 0x23, 0x0000);
        //AFE_RX_CONFIG_2 NTLT = +1 code HT = +3 code
        ethsw_ephy_write_bank3_reg(phy_id, 0xc2, 0xd003);
        //AFE_TX_CONFIG, Set 100BT Cfeed=011 to improve rise/fall time
        ethsw_ephy_write_bank3_reg(phy_id, 0xc4, 0x0431);
        //AFE_VDAC_OTHERS_0, Set 1000BT Cidac=010 for all ports
        ethsw_ephy_write_bank3_reg(phy_id, 0xc7, 0xa020);
        //AFE_HPF_TRIM_OTHERS Amplitude -3.2% and HT = +3 code
        ethsw_ephy_write_bank3_reg(phy_id, 0xc8, 0x00b3);
    }
}
#endif

void bcmeapi_update_link_status(void)
{
#if defined(CONFIG_BCM963381)
    bcm63381_phy_workaround_check(-1);
#endif
}

void bcmeapi_link_check(int log_port, int link, int speed)
{
#if defined(CONFIG_BCM963381)
    if (link != 0 && speed == 100)
    {
        bcm63381_phy_workaround_check(log_port);
    }
#endif
}

void bcmeapi_ethsw_init_hw(int unit, uint32_t portMap,  int wanPortMap)
{
    uint8 v8;
#if !defined(CONFIG_BCM_SWMDK)
    int i;
    uint16 v16;
#if !defined(CONFIG_BCM_SWMDK)
    uint8 wrr_queue_weights[NUM_EGRESS_QUEUES] = {[0 ... (NUM_EGRESS_QUEUES-1)] = 0x1};
#endif
#endif
#if !defined(CONFIG_BCM_SWMDK) ||  !defined(_CONFIG_BCM_FAP)
    uint32 v32;
#endif

#if !defined(CONFIG_BCM_SWMDK)
    unsigned long port_flags;
    for (i=0; i<(TOTAL_SWITCH_PORTS - 1); i++)
    {
        v8 = ((portMap % 2) != 0)? 0: REG_PORT_CTRL_DISABLE;
        ethsw_wreg(PAGE_CONTROL, REG_PORT_CTRL + i, &v8, 1);
        portMap /= 2;
    }

    /* Set IMP port RMII mode */
    v8 = 0;
    ethsw_wreg(PAGE_CONTROL, REG_MII_PORT_CONTROL, &v8, 1);

    // Enable the GMII clocks.
    for (i = 0; i < NUM_RGMII_PORTS; i++) {
        {
            ethsw_rreg(PAGE_CONTROL, REG_RGMII_CTRL_P4 + i, &v8, 1);
            {
                v8 |= REG_RGMII_CTRL_ENABLE_GMII;
            }
            ethsw_wreg(PAGE_CONTROL, REG_RGMII_CTRL_P4 + i, &v8, 1);
        }
    }

    /* RGMII Delay Programming. Enable ID mode */
    for (i = 0; i < NUM_RGMII_PORTS; i++) {
        {
            /* Enable ID mode */
            ethsw_rreg(PAGE_CONTROL, REG_RGMII_CTRL_P4 + i, &v8, 1);
            port_flags = enet_get_port_flags(unit, (RGMII_PORT_ID + i));
            /* TXID default is on so no need to check IsPortTxInternalDelay(port_flags) */
            v8 |= REG_RGMII_CTRL_TIMING_SEL;
            if (IsPortRxInternalDelay(port_flags)) {
                v8 |= REG_RGMII_CTRL_DLL_RXC_BYPASS;
            }
#if defined(CONFIG_BCM963268)
            /* Force RGMII mode as these port support only RGMII */
            v8 |= REG_RGMII_CTRL_ENABLE_RGMII_OVERRIDE;
#endif
            ethsw_wreg(PAGE_CONTROL, REG_RGMII_CTRL_P4 + i, &v8, 1);
        }
    }

    /* Put switch in frame management mode. */
    ethsw_rreg(PAGE_CONTROL, REG_SWITCH_MODE, &v8, sizeof(v8));
    v8 |= REG_SWITCH_MODE_FRAME_MANAGE_MODE;
    v8 |= REG_SWITCH_MODE_SW_FWDG_EN;
    ethsw_wreg(PAGE_CONTROL, REG_SWITCH_MODE, &v8, sizeof(v8));

    // MII port
    v8 = 0xa0;
    v8 |= REG_MII_PORT_CONTROL_RX_UCST_EN;
    v8 |= REG_MII_PORT_CONTROL_RX_MCST_EN;
    v8 |= REG_MII_PORT_CONTROL_RX_BCST_EN;
    ethsw_wreg(PAGE_CONTROL, REG_MII_PORT_CONTROL, &v8, sizeof(v8));

    // Management port configuration
    v8 = ENABLE_MII_PORT | RECEIVE_IGMP | RECEIVE_BPDU;
    ethsw_wreg(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &v8, sizeof(v8));

    /* Configure the switch to use Desc priority */
    ethsw_rreg(PAGE_CONTROL, REG_IUDMA_CTRL, (uint8 *)&v32, 4);
    v32 |= REG_IUDMA_CTRL_USE_DESC_PRIO;
    ethsw_wreg(PAGE_CONTROL, REG_IUDMA_CTRL, (uint8 *)&v32, 4);

    /* Disable tag_status_preserve */
    v8 = 0;
    ethsw_wreg(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_CTRL5, (uint8 *)&v8, 1);

    /* Enable multiple queues and scheduling mode */
    v8 = (1 << TXQ_CTRL_TXQ_MODE_S) |
        (DEFAULT_HQ_PREEMPT_EN << TXQ_CTRL_HQ_PREEMPT_S);
    ethsw_wreg(PAGE_QOS, REG_QOS_TXQ_CTRL, &v8, 1);

    /* Set the queue weights. */
    for (i = 0; i < NUM_EGRESS_QUEUES; i++) {
        ethsw_wreg(PAGE_QOS, REG_QOS_TXQ_WEIGHT_Q0 + i,
                &wrr_queue_weights[i], sizeof(wrr_queue_weights[i]));
    }

    v16 = 0;
    ethsw_wreg(PAGE_QOS, REG_QOS_THRESHOLD_CTRL, (uint8 *)&v16, 2);

    /* Set the default flow control value as desired */
    v16 = DEFAULT_FC_CTRL_VAL;
    ethsw_wreg(PAGE_FLOW_CTRL, REG_FC_CTRL, (uint8 *)&v16, 2);

    /*Set priority queue thresholds */
    for (i = 0; i < NUM_EGRESS_QUEUES; i++) {
        v16 = DEFAULT_TXQHI_HYSTERESIS_THRESHOLD;
        ethsw_wreg(PAGE_FLOW_CTRL, REG_FC_PRIQ_HYST + (2*i), (uint8 *)&v16, sizeof(v16));
        v16 = DEFAULT_TXQHI_PAUSE_THRESHOLD;
        ethsw_wreg(PAGE_FLOW_CTRL, REG_FC_PRIQ_PAUSE + (2*i), (uint8 *)&v16, sizeof(v16));
        v16 = DEFAULT_TXQHI_DROP_THRESHOLD;
        ethsw_wreg(PAGE_FLOW_CTRL, REG_FC_PRIQ_DROP + (2*i), (uint8 *)&v16, sizeof(v16));
        v16 = DEFAULT_TOTAL_HYSTERESIS_THRESHOLD;
        ethsw_wreg(PAGE_FLOW_CTRL, REG_FC_PRIQ_TOTAL_HYST + (2*i), (uint8 *)&v16, sizeof(v16));
        v16 = DEFAULT_TOTAL_PAUSE_THRESHOLD;
        ethsw_wreg(PAGE_FLOW_CTRL, REG_FC_PRIQ_TOTAL_PAUSE + (2*i), (uint8 *)&v16, sizeof(v16));
        v16 = DEFAULT_TOTAL_DROP_THRESHOLD;
        ethsw_wreg(PAGE_FLOW_CTRL, REG_FC_PRIQ_TOTAL_DROP + (2*i), (uint8 *)&v16, sizeof(v16));
    }


    // Forward lookup failure to use ULF/MLF/IPMC lookup fail registers */
    v8 = (REG_PORT_FORWARD_MCST | REG_PORT_FORWARD_UCST | REG_PORT_FORWARD_IP_MCST);
    ethsw_wreg(PAGE_CONTROL, REG_PORT_FORWARD, (uint8 *)&v8, sizeof(v8));

    // Forward unlearned unicast and unresolved mcast to the MIPS
    v16 = PBMAP_MIPS;
    ethsw_wreg(PAGE_CONTROL, REG_UCST_LOOKUP_FAIL, (uint8 *)&v16, sizeof(v16));
    ethsw_wreg(PAGE_CONTROL, REG_MCST_LOOKUP_FAIL, (uint8 *)&v16, sizeof(v16));
    ethsw_wreg(PAGE_CONTROL, REG_IPMC_LOOKUP_FAIL, (uint8 *)&v16, sizeof(v16));



    v32 = DEFAULT_IUDMA_QUEUE_SEL;
    ethsw_wreg(PAGE_CONTROL, REG_IUDMA_QUEUE_CTRL, (uint8 *)&v32, 4);

#endif // #if !defined(CONFIG_BCM_SWMDK)

#if !defined(_CONFIG_BCM_FAP)
    /* Don't enable flow control on 6816 and FAP based systems as FAP does not burst into the switch like 
       MIPS does (due to MIPS going out for snacks (like WLAN calibration) once in a while). 
       For 6368/6328, the back-pressure is needed to pass the 100Mbps DS zero packet loss 
       test as MIPS can burst into switch without flow control. If needed, the back pressure can be replaced with 
       the mechanism of polling for availability of switch buffers before sending packets to switch 
       (adds overhead to hard_xmit function and affects throughput performance but does not break QoS)*/
    v32 = REG_PAUSE_CAPBILITY_OVERRIDE | REG_PAUSE_CAPBILITY_MIPS_TX;
    ethsw_wreg(PAGE_CONTROL, REG_PAUSE_CAPBILITY, (uint8 *)&v32, sizeof(v32));
#endif

    if (unit == 1)  /* External switch is present - decided at run time */
    {
        /* Set WAN ports on external switch */
        if (GET_PORTMAP_FROM_LOGICAL_PORTMAP(wanPortMap, 1))
        {
            extsw_set_wanoe_portmap(GET_PORTMAP_FROM_LOGICAL_PORTMAP(wanPortMap, 1));
        }
#if defined(CONFIG_BCM_PORTS_ON_INT_EXT_SW)
        /* Set WAN ports on internal switch */
        if (GET_PORTMAP_FROM_LOGICAL_PORTMAP(wanPortMap, 0))
        {
            ethsw_set_wanoe_portmap(GET_PORTMAP_FROM_LOGICAL_PORTMAP(wanPortMap, 0));
        }
#endif
    }
    else
    { /* No external switch */      
        ethsw_set_wanoe_portmap(GET_PORTMAP_FROM_LOGICAL_PORTMAP(wanPortMap, 0));
    }


    ethsw_eee_init_hw();

    /* Set ARL AGE_DYNAMIC bit for aging operations */
    v8 = FAST_AGE_DYNAMIC;
    ethsw_wreg(PAGE_CONTROL, REG_FAST_AGING_CTRL, &v8, 1);


#if defined (CONFIG_BCM_JUMBO_FRAME)
    {
        /* 
            Set MIB values for jumbo frames to reflect our maximum frame size.
            Need to set size to hardware max size, otherwise byte counter and
            frame counters in hardware will be inconsistent
        */
        uint16 v16 = MAX_HW_JUMBO_FRAME_SIZE;
        int i;

        for (i=0; i<(unit+1); i++)
        {
            if (i == 0)
            {
                ethsw_wreg(PAGE_JUMBO, REG_JUMBO_FRAME_SIZE_INT_SW, (uint8 *)&v16, 2);   
            }
            else
            {
                extsw_wreg_wrap(PAGE_JUMBO, REG_JUMBO_FRAME_SIZE, (uint8 *)&v16, 2);   
            }
        }
    } 
#endif
    /* Additional initialization needed for 6318 EPHY*/
}

void bcmeapi_set_multiport_address(uint8_t* addr)
{
    uint8 v8, was_enabled;
    uint32 v32;

    ethsw_rreg(PAGE_ARLCTRL, REG_ARLCFG, &v8, 1);
    was_enabled = v8 & (MULTIPORT_ADDR_EN_M << MULTIPORT_ADDR_EN_S);
    v8 |= (MULTIPORT_ADDR_EN_M << MULTIPORT_ADDR_EN_S);
    ethsw_wreg(PAGE_ARLCTRL, REG_ARLCFG, &v8, 1);

    /* If writing for the first time, both banks should be written to */
    if (!was_enabled) {
       ethsw_wreg(PAGE_ARLCTRL, REG_MULTIPORT_ADDR1_LO, addr, 6);
       ethsw_wreg(PAGE_ARLCTRL, REG_MULTIPORT_ADDR2_LO, addr, 6);
    v32 = PBMAP_MIPS;
    ethsw_wreg(PAGE_ARLCTRL, REG_MULTIPORT_VECTOR1, (uint8 *)&v32, 4);
    ethsw_wreg(PAGE_ARLCTRL, REG_MULTIPORT_VECTOR2, (uint8 *)&v32, 4);
    } else {
       /* Overwrite the second bank only */
       ethsw_wreg(PAGE_ARLCTRL, REG_MULTIPORT_ADDR2_LO, addr, 6);
       v32 = PBMAP_MIPS;
       ethsw_wreg(PAGE_ARLCTRL, REG_MULTIPORT_VECTOR2, (uint8 *)&v32, 4);       
    }
    return ;
}

#if defined(_CONFIG_BCM_ARL)
int enet_hook_for_arl_access(void *ethswctl)
{
    struct ethswctl_data *pEthswctl = (struct ethswctl_data *)ethswctl;
    int                   ret;

    if ( TYPE_SET == pEthswctl->type ) {
        /* if ifname is set caller needs the port, unit and val set based on
           port information */
        if (strlen(pEthswctl->ifname) > 0) {
            ret    = bcm63xx_enet_getPortFromName(pEthswctl->ifname, &pEthswctl->unit, &pEthswctl->port);
            if(ret < 0)
            {
                pEthswctl->val = 0;
                return BCM_E_PARAM;
            }
            if ( pEthswctl->unit )
            {
                pEthswctl->val = ARL_DATA_ENTRY_VALID_531xx | 
                                 ARL_DATA_ENTRY_STATIC_531xx |
                                 (1<<pEthswctl->port);
            }
            else
            {
                pEthswctl->val = ARL_DATA_ENTRY_VALID | 
                                 ARL_DATA_ENTRY_STATIC |
                                 (1<<pEthswctl->port);
            }
        }
    }
   
    return bcmeapi_ioctl_ethsw_arl_access(pEthswctl);
}
#endif

#if defined(CONFIG_BCM_GMAC)

int enet_set_port_ctrl( int port, uint32_t val32 )
{
    uint8_t v8;

    local_irq_disable();

    BCM_ENET_DEBUG("Given port: %d control: %02x\n ", port, val32);
    ethsw_rreg(PAGE_CONTROL, REG_PORT_CTRL + port, &v8, 1);
    v8 &= (~REG_PORT_CTRL_DISABLE);
    v8 |= (val32 & REG_PORT_CTRL_DISABLE);
    BCM_ENET_DEBUG("Writing = 0x%x to REG_PORT_CTRL", v8);
    ethsw_wreg(PAGE_CONTROL, REG_PORT_CTRL + port, &v8, 1);

    local_irq_enable();

    return BCM_E_NONE;
}


/* Restart Autonegotiation on the port */
void enet_restart_autoneg(int log_port)
{
    uint16_t v16;
    int phyid;

    phyid = enet_logport_to_phyid(log_port);

    local_irq_disable();

    /* read control register */
    ethsw_phy_rreg(phyid, MII_BMCR, &v16);
    BCM_ENET_DEBUG("MII_BMCR Read Value = %4x", v16);

    /* Write control register wth AN_EN and RESTART_AN bits set */
    v16 |= (BMCR_ANENABLE | BMCR_ANRESTART);
    BCM_ENET_DEBUG("MII_BMCR Written Value = %4x", v16);
    ethsw_phy_wreg(phyid, MII_BMCR, &v16);

    local_irq_enable();
}
#endif

void bcmeapi_ethsw_set_stp_mode(unsigned int unit, unsigned int port, unsigned char stpState)
{
   unsigned char portInfo;

   if ( unit )
   {
      extsw_rreg_wrap(PAGE_CONTROL, REG_PORT_CTRL + (port),
                 &portInfo, sizeof(portInfo));
      portInfo &= ~REG_PORT_STP_MASK;
      portInfo |= stpState;
      extsw_wreg_wrap(PAGE_CONTROL, REG_PORT_CTRL + (port),
                 &portInfo, sizeof(portInfo));
   }
   else
   {
      ethsw_rreg(PAGE_CONTROL, REG_PORT_CTRL + (port), 
                     &portInfo, sizeof(portInfo));
      portInfo &= ~REG_PORT_STP_MASK;
      portInfo |= stpState;
      ethsw_wreg(PAGE_CONTROL, REG_PORT_CTRL + (port), 
                     &portInfo, sizeof(portInfo));
   }
}

#ifdef REPORT_HARDWARE_STATS
int ethsw_get_hw_stats(int port, struct rtnl_link_stats64 *stats)
{
    uint32 ctr32 = 0;           // Running 32 bit counter
    uint64 ctr64 = 0;           // Running 64 bit counter
    uint64 tempctr64 = 0;       // Temp 64 bit counter

    // Track RX unicast, multicast, and broadcast packets
     ethsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXUPKTS,
            (uint8_t *)&ctr32, 4);                        // Get RX unicast packet count
     ctr64 = (uint64)ctr32;                                          // Keep running count        

     ethsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXMPKTS,
            (uint8_t *)&ctr32, 4);                        // Get RX multicast packet count
     tempctr64 = (uint64)ctr32;
     stats->multicast = tempctr64;                                   // Save away count
     ctr64 += tempctr64;                                             // Keep running count
     
     ethsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXBPKTS,
            (uint8_t *)&ctr32, 4);                        // Get RX broadcast packet count
     tempctr64 = (uint64)ctr32;
     stats->rx_broadcast_packets = tempctr64;                         // Save away count
     ctr64 += tempctr64;                                              // Keep running count
     stats->rx_packets = ctr64;
     
     // Track RX byte count
     ethsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXOCTETS,
            (uint8_t *)&ctr64, 8);
     stats->rx_bytes = ctr64;
     
     // Track RX packet errors
     ethsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXDROPS,
            (uint8_t *)&ctr32, 4);
     stats->rx_dropped = ctr32;
     ethsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXFCSERRORS,
            (uint8_t *)&ctr32, 4);
     stats->rx_errors = ctr32;
     ethsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXSYMBOLERRORS,
            (uint8_t *)&ctr32, 4);
     stats->rx_errors += ctr32;
     ethsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_RXALIGNERRORS,
            (uint8_t *)&ctr32, 4);
     stats->rx_errors += ctr32;

     // Track TX unicast, multicast, and broadcast packets
     ethsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_TXUPKTS,
            (uint8_t *)&ctr32, 4);                        // Get TX unicast packet count
     ctr64 = (uint64)ctr32;                                          // Keep running count        

     ethsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_TXMPKTS,
            (uint8_t *)&ctr32, 4);                        // Get TX multicast packet count
     tempctr64 = (uint64)ctr32;
     stats->tx_multicast_packets = tempctr64;                        // Save away count
     ctr64 += tempctr64;                                             // Keep running count
     
     ethsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_TXBPKTS,
            (uint8_t *)&ctr32, 4);                        // Get TX broadcast packet count
     tempctr64 = (uint64)ctr32;
     stats->tx_broadcast_packets = tempctr64;                         // Save away count
     ctr64 += tempctr64;                                              // Keep running count
     stats->tx_packets = ctr64;
     
     // Track TX byte count
     ethsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_TXOCTETS,
            (uint8_t *)&ctr64, 8);
     stats->tx_bytes = ctr64;
     
     // Track TX packet errors
     ethsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_TXDROPS,
            (uint8_t *)&ctr32, 4);
    stats->tx_dropped = ctr32;
    ethsw_rreg(PAGE_MIB_P0 + (port), REG_MIB_P0_TXFRAMEINDISC,
            (uint8_t *)&ctr32, 4);
    stats->tx_dropped += ctr32;

#if defined(CONFIG_BCM_GMAC)
     if ( gmac_info_pg->enabled && (port == GMAC_PORT_ID) )
         gmac_hw_stats( port, stats );
#endif /* if defined(CONFIG_BCM_GMAC) */
    return 0;
}
#endif

int bcmeapi_init_ext_sw_if(extsw_info_t *extSwInfo)
{
    int status = 0;

    if ((extSwInfo->accessType == MBUS_SPI) || (extSwInfo->accessType == MBUS_HS_SPI)) {
        status = BcmSpiReserveSlave2(extSwInfo->bus_num, extSwInfo->spi_ss, 781000, SPI_MODE_3,
                SPI_CONTROLLER_STATE_GATE_CLK_SSOFF);
        if ( SPI_STATUS_OK != status ) {
            printk("Unable to reserve slave id for ethernet switch\n");
        }
    }

    return status;
}
#else /* For 47189 */
/*
* The two functions below allow gathering SW tx/rx counters.
* Is used by wlan driver to stop calibration
* when heavy traffic runs between Eth Port, to avoid Eth pkt loss
*/
unsigned int sw_tx = 0, sw_rx = 0;

/*
* This function gathers the statistics every 1 second from the ethernet
* poll function. Do not create your own polling function that would call it,
* as this would interfere with power management and reduce power savings.
*/

void __ethsw_get_txrx_imp_port_pkts(void)
{
    ETHERNET_MAC_INFO EnetInfo[BP_MAX_ENET_MACS];
    unsigned int temp_sw_tx = 0, temp_sw_rx = 0;
    int index;

    if(BpGetEthernetMacInfo(EnetInfo, BP_MAX_ENET_MACS) != BP_SUCCESS)
    {
        return;
    }

    for (index=0; index < BP_MAX_ENET_MACS; index++)
    {
        volatile EnetCoreMib *e;
            
        if ((EnetInfo[0].sw.port_map & (1 << index)) == 0)
            continue;
        
        e = gmac_mib_regs(index);
        if(e)
        {
            temp_sw_tx += e->tx_pkts;
            temp_sw_rx += e->rx_pkts;
        }
    }

    if(temp_sw_tx || temp_sw_rx)
    {
        sw_tx = temp_sw_tx;
        sw_rx = temp_sw_rx;
    }

    return;
}

/*
* This function provides the packet counts with up to 1 seconds delay.
*/
void ethsw_get_txrx_imp_port_pkts(unsigned int *tx, unsigned int *rx)
{
    *tx = sw_tx;
    *rx = sw_rx;
    return;
}
EXPORT_SYMBOL(ethsw_get_txrx_imp_port_pkts);

#ifdef REPORT_HARDWARE_STATS
int ethsw_get_hw_stats(int port, struct rtnl_link_stats64 *stats)
{
    gmac_hw_stats( port, stats );
    return 0;
}
#endif

#endif
