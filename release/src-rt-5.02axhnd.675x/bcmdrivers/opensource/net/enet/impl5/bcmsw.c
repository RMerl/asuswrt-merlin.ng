/*
    <:copyright-BRCM:2016:DUAL/GPL:standard    
    
       Copyright (c) 2016 Broadcom 
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

#define _BCMENET_LOCAL_

#include "bcm_OS_Deps.h"
#include "board.h"
#include "spidevices.h"
#include <bcm_map_part.h>
#include "bcm_intr.h"
#include "bcmmii.h"
#include "ethsw_phy.h"
#include "bcmswdefs.h"
#include "bcmenet.h"
#include "bcmsw.h"
/* Exports for other drivers */
#include "bcmsw_api.h"
#include "bcmswshared.h"
#include "bcmPktDma_defines.h"
#include "boardparms.h"
#if defined(CONFIG_BCM_GMAC)
#include "bcmgmac.h"
#endif
#include "bcmswaccess.h"
#include "eth_pwrmngt.h"

#ifndef SINGLE_CHANNEL_TX
/* for enet driver txdma channel selection logic */
extern int channel_for_queue[NUM_EGRESS_QUEUES];
/* for enet driver txdma channel selection logic */
extern int use_tx_dma_channel_for_priority;
#endif /*SINGLE_CHANNEL_TX*/

extern extsw_info_t extSwInfo;
extern uint32_t logicalport_to_imp_map[];
extern uint32_t imp_pbmap[];
extern uint32_t g_imp_use_lag;

#if defined(CONFIG_BCM_GMAC)
void gmac_hw_stats( int port,  struct rtnl_link_stats64 *stats );
int gmac_dump_mib(int port, int type);
void gmac_reset_mib( void );
#endif

#define SWITCH_ADDR_MASK                   0xFFFF
#define ALL_PORTS_MASK                     0x1FF
#define ONE_TO_ONE_MAP                     0x00FAC688
#define MOCA_QUEUE_MAP                     0x0091B492
#define DEFAULT_FC_CTRL_VAL                0x1F
/* Tx: 0->0, 1->1, 2->2, 3->3. */
#define DEFAULT_IUDMA_QUEUE_SEL            0x688


/* Forward declarations */
extern spinlock_t bcm_extsw_access;
uint8_t  port_in_loopback_mode[TOTAL_SWITCH_PORTS] = {0};

/*  
    Maxmum Garanteed Streams, total buffers will support 96 streams of Jumbo frames. 
*/

/************************/
/* Ethernet Switch APIs */
/************************/

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

#if defined(CONFIG_BCM_EXT_SWITCH)

void extsw_setup_imp_ports(void)
{
     uint8_t  val8;
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
     uint16_t val16;
#endif /* CONFIG_BCM_ENET_MULTI_IMP_SUPPORT */
    /* Assumption : External switch is always in MANAGED Mode w/ TAG enabled.
     * BRCM TAG enable in external switch is done via MDK as well
     * but it is not deterministic when the userspace app for external switch
     * will run. When it gets delayed and the device is already getting traffic,
     * all those packets are sent to CPU without external switch TAG.
     * To avoid the race condition - it is better to enable BRCM_TAG during driver init. */
    extsw_rreg_wrap(PAGE_MANAGEMENT, REG_BRCM_HDR_CTRL, &val8, sizeof(val8));
    val8 &= (~(BRCM_HDR_EN_GMII_PORT_5|BRCM_HDR_EN_IMP_PORT)); /* Reset HDR_EN bit on both ports */
    val8 |= BRCM_HDR_EN_IMP_PORT; /* Set only for IMP Port */
    extsw_wreg_wrap(PAGE_MANAGEMENT, REG_BRCM_HDR_CTRL, &val8, sizeof(val8));


#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
    /* NOTE : Forcing these setting here; SWMDK doesn't setup IMP when multiple IMP ports in-use */

    /* Enable IMP Port */
    val8 = ENABLE_MII_PORT | RECEIVE_BPDU;
    extsw_wreg_wrap(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &val8, sizeof(val8));

    /* management mode, enable forwarding */
    extsw_rreg_wrap(PAGE_CONTROL, REG_SWITCH_MODE, &val8, sizeof(val8));
    val8 |= REG_SWITCH_MODE_FRAME_MANAGE_MODE | REG_SWITCH_MODE_SW_FWDG_EN;
    extsw_wreg_wrap(PAGE_CONTROL, REG_SWITCH_MODE, &val8, sizeof(val8));

    /* enable rx bcast, ucast and mcast of imp port */
    val8 = REG_MII_PORT_CONTROL_RX_UCST_EN | REG_MII_PORT_CONTROL_RX_MCST_EN |
           REG_MII_PORT_CONTROL_RX_BCST_EN;
    extsw_wreg_wrap(PAGE_CONTROL, REG_MII_PORT_CONTROL, &val8, sizeof(val8));

    /* Forward lookup failure to use ULF/MLF/IPMC lookup fail registers */
    val8 = REG_PORT_FORWARD_MCST | REG_PORT_FORWARD_UCST | REG_PORT_FORWARD_IP_MCST;
    extsw_wreg_wrap(PAGE_CONTROL, REG_PORT_FORWARD, &val8, sizeof(val8));

    /* Forward unlearned unicast and unresolved mcast to the MIPS */
    val16 = PBMAP_MIPS;
    extsw_wreg_wrap(PAGE_CONTROL, REG_UCST_LOOKUP_FAIL, &val16, sizeof(val16));
    extsw_wreg_wrap(PAGE_CONTROL, REG_MCST_LOOKUP_FAIL, &val16, sizeof(val16));
    extsw_wreg_wrap(PAGE_CONTROL, REG_IPMC_LOOKUP_FAIL, &val16, sizeof(val16));

    /* Disable learning on MIPS*/
    val16 = PBMAP_MIPS;
    extsw_wreg_wrap(PAGE_CONTROL, REG_DISABLE_LEARNING, &val16, sizeof(val16));

    /* NOTE : All regular setup for P8 IMP is done above ; Same as what SWMDK would do*/

    /* Enable BRCM TAG on P5 */
    extsw_rreg_wrap(PAGE_MANAGEMENT, REG_BRCM_HDR_CTRL, &val8, sizeof(val8));
    val8 |= BRCM_HDR_EN_GMII_PORT_5; /* Enable BRCM TAG on P5 */
    extsw_wreg_wrap(PAGE_MANAGEMENT, REG_BRCM_HDR_CTRL, &val8, sizeof(val8));
    /* Enale Link - port override register */
    extsw_rreg_wrap(PAGE_CONTROL, REG_PORT_STATE+5, &val8, sizeof(val8));
    val8 = LINK_OVERRIDE_1000FDX; /* Enable 1000FDX Link */
    val8 |= REG_PORT_GMII_SPEED_UP_2G; /* Speed up to 2G */
    extsw_wreg_wrap(PAGE_CONTROL, REG_PORT_STATE+5, &val8, sizeof(val8));

    imp_pbmap[1] |= PBMAP_P5_IMP ; /* Add P5 to the list of IMP ports */

    /* Enable BRCM TAG on P4 */
    extsw_rreg_wrap(PAGE_MANAGEMENT, REG_BRCM_HDR_CTRL2, &val8, sizeof(val8));
    val8 = BRCM_HDR_EN_P4; /* Enable BRCM TAG on P4 */
    extsw_wreg_wrap(PAGE_MANAGEMENT, REG_BRCM_HDR_CTRL2, &val8, sizeof(val8));
    /* Enale Link - port override register */
    extsw_rreg_wrap(PAGE_CONTROL, REG_PORT_STATE+4, &val8, sizeof(val8));
    val8 = LINK_OVERRIDE_1000FDX; /* Enable 1000FDX Link */
    extsw_wreg_wrap(PAGE_CONTROL, REG_PORT_STATE+4, &val8, sizeof(val8));

    imp_pbmap[1] |= PBMAP_P4_IMP ; /* Add P4 to the list of IMP ports */

#endif /* CONFIG_BCM_ENET_MULTI_IMP_SUPPORT */
}
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
void bcmsw_print_imp_port_grouping(unsigned long port_map)
{
    int port, imp_port, new_grp = 0;
    printk("NOTE: Using Port Grouping for IMP ports : ");
    for (imp_port = 0; imp_port <= BP_MAX_SWITCH_PORTS; imp_port++)
    {
        /* Not an IMP port -- continue */
        if (! ( (1<<imp_port) & (PBMAP_MIPS | PBMAP_P5_IMP | PBMAP_P4_IMP) ) ) continue;
        new_grp = 1;
        for (port = 0; port < BP_MAX_SWITCH_PORTS; port++) 
        {
            if ( ((1<<port) & port_map) && 
                 logicalport_to_imp_map[PHYSICAL_PORT_TO_LOGICAL_PORT(port,1)] == imp_port )
            {
                if (new_grp)
                {
                    printk("[");
                    new_grp = 0;
                }
                else
                {
                    printk(",");
                }
                printk(" %d",port);
            }
        }
        if (!new_grp)
        {
            printk(" --> %d ] ",imp_port);
        }
    }
    printk("\n");
}
#endif
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
/*
    Use CFP to force Reserved Multicast Address to be received by
    IMP port correctly overriding Port Based VLAN set for load balancing.
*/
static int bcmsw_add_cfp_rsvd_multicast_support(void)
{
    struct ethswctl_data _e, *e = &_e;
    cfpArg_t *cfpArg = &e->cfpArgs;
    
    memset(e, 0, sizeof(*e));

    cfpArg->da = 0x0180c2000000LL;
    cfpArg->da_mask = 0xffffff000000;
    cfpArg->argFlag |= CFP_ARG_DA_M;
    cfpArg->l3_framing= CfpL3NoIP;
    cfpArg->argFlag |= CFP_ARG_L3_FRAMING_M;
    cfpArg->op = CFPOP_APPEND;
    cfpArg->argFlag |= CFP_ARG_OP_M;
    cfpArg->chg_fpmap_ib = 2;
    cfpArg->argFlag |= CFP_ARG_CHG_FPMAP_IB_M;
    cfpArg->fpmap_ib = PBMAP_MIPS;
    cfpArg->argFlag |= CFP_ARG_FPMAP_IB_M;
    cfpArg->priority = 2;
    cfpArg->argFlag |= CFP_ARG_PRIORITY_M;

    return bcmeapi_ioctl_cfp(e);
}

/* Define the mapping here for now */


static void extsw_cfg_port_imp_grouping(int port_imp_map[])
{
    unsigned char port;
    unsigned long port_map;
    uint16 v16;
    /* Configure forwarding based on Port Grouping
     * By default all port's pbvlan is 0x1FF */
    port_map = enet_get_portmap(1); /* Get port map for external switch */
    for (port = 0; port < BP_MAX_SWITCH_PORTS; port++)
    {
        if ( (port_imp_map[port] != -1) && ( (1<<port) & port_map ) )
        {
            v16 = extsw_get_pbvlan(port); /* Get current PBVLAN Map */
            v16 &= ~(imp_pbmap[1]); /* Remove all IMP Ports from PBVLAN Map for external switch*/
            v16 |= (1<<port_imp_map[port]); /* Add back the desired IMP Port */
            extsw_set_pbvlan(port, v16);
            logicalport_to_imp_map[PHYSICAL_PORT_TO_LOGICAL_PORT(port,1)] = port_imp_map[port];
        }
    }

    bcmsw_print_imp_port_grouping(port_map);
}
#endif /* CONFIG_BCM_ENET_MULTI_IMP_SUPPORT */

void extsw_set_port_imp_map_2_5g(void)
{
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
    /* Below mapping is used when P7 comes up at 2.5G link speed */
    int port_imp_map_2_5g[BP_MAX_SWITCH_PORTS] = 
                        {
                            /* P0 */  P4_PORT_ID,
                            /* P1 */  P5_PORT_ID,
                            /* P2 */  P5_PORT_ID,
                            /* P3 */  P5_PORT_ID, /* 5 /*/
                            /* P4 */  -1,   /* IMP Port */
                            /* P5 */  -1,   /* IMP Port */
                            /* P6 */  -1,   /* Unused/undefined switch port */
                            /* P7 */  IMP_PORT_ID /* 8 */
                        };

    if (g_imp_use_lag) return; /* IMP ports are in lag group -- no port grouping needed */

    extsw_cfg_port_imp_grouping(port_imp_map_2_5g);
#endif
}
void extsw_set_port_imp_map_non_2_5g(void)
{
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
    /* Below mapping is used when P7 comes up at below 2.5G link speed */
    int port_imp_map_non_2_5g[BP_MAX_SWITCH_PORTS] = 
                        {
                            /* P0 */  P4_PORT_ID,
                            /* P1 */  P5_PORT_ID,
                            /* P2 */  P5_PORT_ID,
                            /* P3 */  IMP_PORT_ID, /* 8 /*/
                            /* P4 */  -1,   /* IMP Port */
                            /* P5 */  -1,   /* IMP Port */
                            /* P6 */  -1,   /* Unused/undefined switch port */
                            /* P7 */  IMP_PORT_ID /* 8 */
                        };

    if (g_imp_use_lag) return; /* IMP ports are in lag group -- no port grouping needed */

    extsw_cfg_port_imp_grouping(port_imp_map_non_2_5g);
#endif
}

void extsw_setup_imp_fwding(void)
{
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
    unsigned char v8;
    uint16 v16;
    uint32 v32;
#if defined(BCM_ENET_MULTI_IMP_SUPPORT_USE_LAG)
    unsigned char port;
    int crossbar_port;
    ETHERNET_MAC_INFO *EnetInfo = EnetGetEthernetMacInfo();
    ETHERNET_MAC_INFO *info = &EnetInfo[1]; /* External Switch */
#endif /* BCM_ENET_MULTI_IMP_SUPPORT_USE_LAG */

#if defined(BCM_ENET_MULTI_IMP_SUPPORT_USE_LAG)
    /* Make decision based on number of SF2 ports configured in board parameters;
       4908 supports only 5 LAN Ports on SF2 -
       P0-P3 = Quad GPHY                     
       P7 = Crossbar (1 GPHY, 1 RGMII/RvMII/TMII or 1 SERDES (2.5G) 
       Decision tree :
       1. If no crossbar LAN ports i.e. P7 is not used >>> Use port based forwarding
       2. If crossbar LAN ports i.e. P7 is in use but NOT connected to SERDES >>> Use port based forwarding
       3. TBD : if P7 is connected to SERDES and one of the other LAN ports P0-P3 are not used >>> Use Port based forwarding
       4. Otherwise use LAG */

    for (port = BP_CROSSBAR_PORT_BASE; (port < BCMENET_MAX_PHY_PORTS); port++) /* go through all the ports including crossbar */
    {
        /* No need to check BP_IS_CROSSBAR_PORT -- loop starts with CB port base */
        crossbar_port = BP_PHY_PORT_TO_CROSSBAR_PORT(port);
        if (!BP_IS_CROSSBAR_PORT_DEFINED(info->sw, crossbar_port)) continue;
        if (!IsSerdes(info->sw.crossbar[crossbar_port].phy_id)) continue;
        /* Serdes is configured - use LAG */
        g_imp_use_lag = 1;
        break;
    }
#endif /* BCM_ENET_MULTI_IMP_SUPPORT_USE_LAG */

    if (g_imp_use_lag)
    {
        int grp_no = 1; /* Switch only supports two LAG/Trunk groups - use the second one */
        extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        v8 |= ( (1 & TRUNK_EN_LOCAL_M) << TRUNK_EN_LOCAL_S ); /* Enable Trunking */
        v8 |= ( (TRUNK_HASH_DA_SA_VID & TRUNK_HASH_SEL_M) << TRUNK_HASH_SEL_S ); /* Default VID+DA+SA Hash */
        extsw_wreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);

        extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_IMP0_TRUNK_CTL, &v16, 2);
        v16 |= (PBMAP_MIPS| PBMAP_P5_IMP | PBMAP_P4_IMP );
        extsw_wreg_wrap(PAGE_MAC_BASED_TRUNK, REG_IMP0_TRUNK_CTL, &v16, 2);

        v16 = ( ( (PBMAP_MIPS | PBMAP_P5_IMP | PBMAP_P4_IMP ) & TRUNK_EN_GRP_M ) << TRUNK_EN_GRP_S );
        extsw_wreg_wrap(PAGE_MAC_BASED_TRUNK, REG_TRUNK_GRP_CTL + (2*grp_no), &v16, 2);

        /* P8/EMAC0=2.5G, P5/EMAC1=2.5G and P4/EMAC2=1.4G = 6.4G
         * Total Hash buckets = 256 --> P4=56, P5=100, P8=100 */
        v32 = HASH_WT_TRUNK_CTL_OVRD;
        v32 |= ((55 & HAS_WT_MEM_M) << HAS_WT_MEM_0_S);
        v32 |= ((155 & HAS_WT_MEM_M) << HAS_WT_MEM_1_S);
        v32 |= ((255 & HAS_WT_MEM_M) << HAS_WT_MEM_2_S);
        extsw_wreg_wrap(PAGE_MAC_BASED_TRUNK, REG_HASH_WT_TRUNK_CTL + (4*grp_no), &v32, sizeof(v32));

        printk("NOTE: Using LAG/Port-Trunking for IMP ports\n");
    }
    else
    {
        /* Configure the Lookup failure registers to P4, P5, P8 */
        v16 = (PBMAP_MIPS | PBMAP_P5_IMP | PBMAP_P4_IMP );
        extsw_wreg_wrap(PAGE_CONTROL, REG_UCST_LOOKUP_FAIL, &v16, sizeof(v16));
        extsw_wreg_wrap(PAGE_CONTROL, REG_MCST_LOOKUP_FAIL, &v16, sizeof(v16));
        extsw_wreg_wrap(PAGE_CONTROL, REG_IPMC_LOOKUP_FAIL, &v16, sizeof(v16));
        /* Disable learning on MIPS and P4/P5*/
        v16 = PBMAP_MIPS | PBMAP_P5_IMP | PBMAP_P4_IMP ;
        extsw_wreg_wrap(PAGE_CONTROL, REG_DISABLE_LEARNING, &v16, sizeof(v16));

        extsw_set_port_imp_map_non_2_5g(); /* By default we start with assuming no 2.5G port */

        bcmsw_add_cfp_rsvd_multicast_support();
    }
#endif
}

void extsw_rgmii_config(void)
{
    unsigned char data8;
    unsigned char unit = 1, port;
    uint16 v16;
    int phy_id, rgmii_ctrl;
    unsigned long port_map;
    unsigned long port_flags;

    port_map = enet_get_portmap(unit);

    for (port = 0; port <= MAX_SWITCH_PORTS; port++)
    {
        if (port == MIPS_PORT_ID) {
#if defined(CONFIG_BCM947189)
            continue;
#endif
            rgmii_ctrl = EXTSW_REG_RGMII_CTRL_IMP;
            port_flags = 0;
        }
        else {
            if (!(port_map & (1<<port))) {
                continue;
            }

            phy_id =  enet_logport_to_phyid(PHYSICAL_PORT_TO_LOGICAL_PORT(port, unit));
            if (!IsRGMII(phy_id)) {
                continue;
            }

            if (port == EXTSW_RGMII_PORT) {
                rgmii_ctrl = REG_RGMII_CTRL_P5;
            }
            else {
                continue;
            }
            port_flags = enet_get_port_flags(unit, port);
        }

        extsw_rreg_wrap( PAGE_CONTROL, rgmii_ctrl, &data8, sizeof(data8));
        data8 |= EXTSW_RGMII_TXCLK_DELAY;
        data8 &= ~EXTSW_RGMII_RXCLK_DELAY;

        /* TXID default is on so no need to check IsPortTxInternalDelay(port_flags) */
        if (IsPortRxInternalDelay(port_flags)) {
            data8 |= EXTSW_RGMII_RXCLK_DELAY;
        }

        extsw_wreg_wrap(PAGE_CONTROL, rgmii_ctrl, &data8, sizeof(data8));
        if (port == MIPS_PORT_ID) {
            continue;
        }
        if (IsPortPhyConnected(unit, port)) {

            // GTXCLK to be  off (phy reg 0x1c, shadow 3 bit 9)  - phy defaults to ON
            // RXC skew to be on (phy reg 0x18, shadow 7 bit 8)  - phy defaults to ON

            v16 = MII_1C_SHADOW_CLK_ALIGN_CTRL << MII_1C_SHADOW_REG_SEL_S;
            ethsw_phy_wreg(phy_id, MII_REGISTER_1C, &v16);
            ethsw_phy_rreg(phy_id, MII_REGISTER_1C, &v16);
            v16 &= (~GTXCLK_DELAY_BYPASS_DISABLE);
            v16 |= MII_1C_WRITE_ENABLE;
            v16 &= ~(MII_1C_SHADOW_REG_SEL_M << MII_1C_SHADOW_REG_SEL_S);
            v16 |= (MII_1C_SHADOW_CLK_ALIGN_CTRL << MII_1C_SHADOW_REG_SEL_S);
            ethsw_phy_wreg(phy_id, MII_REGISTER_1C, &v16);

            /*
             * Follow it up with any overrides to the above Defaults.
             * It may be done in two stages.
             * 1. In the interim, customers may reprogramm the above RGMII defaults on the
             *    phy, by devising  a series of bp_mdio_init_t phy init data in boardparms.
             *    Ex.,  {bp_pPhyInit,                .u.ptr = (void *)g_phyinit_xxx},
             * 2. Eventually, if phyinit stuff above qualifies to become main stream,
             *    this is the place to code Rgmii phy config overrides.
             */
        }
    }//for all ports
}

void bcmsw_config_wan(int is_add, uint8_t port)
{
    uint16_t wan_port_map;
    /* Configure WAN port */
    extsw_rreg_wrap(PAGE_CONTROL, REG_WAN_PORT_MAP, &wan_port_map, sizeof(wan_port_map));
    if (is_add)
    {
        wan_port_map |= (1<<port); /* Add the WAN port in the port map */
    }
    else
    {
        wan_port_map &= ~(1<<port); /* remove the WAN port in the port map */
    }
    extsw_wreg_wrap(PAGE_CONTROL, REG_WAN_PORT_MAP, &wan_port_map, sizeof(wan_port_map));

    printk("%s() : %s port %d as WAN; wan_pmap <0x%02x>\n",__FUNCTION__,is_add?"Add":"Remove",port,wan_port_map);

    /* Disable learning */
    extsw_rreg_wrap(PAGE_CONTROL, REG_DISABLE_LEARNING, &wan_port_map, sizeof(wan_port_map));
    if (is_add)
    {
        wan_port_map |= (1<<port); /* Add the WAN port in the port map */
    }
    else
    {
        wan_port_map &= ~(1<<port); /* remove the WAN port in the port map */
    }
    extsw_wreg_wrap(PAGE_CONTROL, REG_DISABLE_LEARNING, &wan_port_map, sizeof(wan_port_map));

    /* NOTE : No need to change the PBVLAN map -- switch logic does not care about pbvlan when the port is WAN */

    /* NOTE: For multiple IMP port products, switch will send all traffic from WAN to P8
       but we should use CFP to send it to correct port based on port grouping
       TBD */

    /* Age all dynamically learnt ARL on this port */
    extsw_fast_age_port(port, 0);
}

/* switch register configuration function to add/remove ports from a given trunk group */
int bcmsw_config_trunk(int add, int grp_no, int unit, uint16_t port)
{
    uint16_t v16;
    if (unit != 1 || grp_no >= BCM_SW_MAX_TRUNK_GROUPS)
    {
        printk("%s() : Warning - cannot %s unit=%d port=%d %s group=%d\n",
               __FUNCTION__,add?"add":"remove",unit,port,add?"to":"from",grp_no);
        return -1;
    }
    extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_TRUNK_GRP_CTL + (2*grp_no), &v16, 2);
    if (add)
    {
        v16 |= ( ( (1<<port) & TRUNK_EN_GRP_M ) << TRUNK_EN_GRP_S );
        printk("%s() : ADD : port <%d> to group <%d>; New pmap <0x%02x>\n", __FUNCTION__, port, grp_no,v16);
    }
    else
    {
        v16 &= ~( ( (1<<port) & TRUNK_EN_GRP_M ) << TRUNK_EN_GRP_S );
        printk("%s() : REM : port <%d> from group <%d>; New pmap <0x%02x>\n", __FUNCTION__, port, grp_no,v16);
    }
    extsw_wreg_wrap(PAGE_MAC_BASED_TRUNK, REG_TRUNK_GRP_CTL + (2*grp_no), &v16, 2);
    return 0;
}
/* Init time function to enable switch trunking if needed */
void extsw_port_trunk_init(void)
{
    int enable_trunk = 0;

    if (enable_trunk)
    {
        unsigned char v8;
        extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        v8 |= ( (1 & TRUNK_EN_LOCAL_M) << TRUNK_EN_LOCAL_S ); /* Enable Trunking */
        v8 |= ( (TRUNK_HASH_DA_SA_VID & TRUNK_HASH_SEL_M) << TRUNK_HASH_SEL_S ); /* Default VID+DA+SA Hash */
        extsw_wreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        printk("LAG/Trunking enabled <0x%02x>\n",v8);
    }
}

void bcmeapi_conf_que_thred(void)
{
}

extern unsigned int UtilGetChipIsLP(void);

void extsw_init_config(void)
{
    uint8_t v8;

    /* Retrieve external switch id - this can only be done after other globals have been initialized */
    if (extSwInfo.present) {
        uint8 val[4] = {0};


        extsw_rreg_wrap(PAGE_MANAGEMENT, REG_DEV_ID, (uint8 *)&val, 4);
        extSwInfo.switch_id = (*(uint32 *)val);

        extsw_setup_imp_ports();

        extsw_setup_imp_fwding();

        // RGMII
        extsw_rgmii_config();

        /* Initialize EEE on external switch */
        extsw_eee_init();

#if defined(CONFIG_BCM_ETH_PWRSAVE)
        /* Power Savings: disable Tx/Rx MACs on unused ports */
        ethsw_shutdown_unused_macs(pVnetDev0_g);
#endif

        /* Configure Trunk groups if required */
        extsw_port_trunk_init();
        /* Set ARL AGE_DYNAMIC bit for aging operations */
        v8 = FAST_AGE_DYNAMIC;
        extsw_wreg_wrap(PAGE_CONTROL, REG_FAST_AGING_CTRL, &v8, 1);
    }
}
#endif

/* Upon integration, the following function may be merged with extsw_rreg_wrap()
   and the appropriate cases that need swap will byte swap.
   Our expectation is that MBUS_MMAP case would Not need swap.
   Currently, this function always returns little endian data from history.
 */

void extsw_rreg(int page, int reg, uint8 *data, int len)
{
    if (((len != 1) && (len % 2) != 0) || len > 8)
        panic("extsw_rreg: wrong length!\n");

    switch (pVnetDev0_g->extSwitch->accessType)
    {
      case MBUS_MDIO:
          bcmsw_pmdio_rreg(page, reg, data, len);
        break;

      case MBUS_SPI:
      case MBUS_HS_SPI:
        bcmsw_spi_rreg(pVnetDev0_g->extSwitch->bus_num, pVnetDev0_g->extSwitch->spi_ss,
                       pVnetDev0_g->extSwitch->spi_cid, page, reg, data, len);
        break;
      default:
        printk("Error Access Type %d: Neither SPI nor PMDIO access in %s <page:0x%x, reg:0x%x>\n",
            pVnetDev0_g->extSwitch->accessType, __func__, page, reg);
        break;
    }
    BCM_ENET_DEBUG("%s : page=%d reg=%d Data [ 0x%02x 0x%02x 0x%02x 0x%02x ] Len = %d\n",
                    __FUNCTION__, page, reg, data[0],data[1],data[2],data[3], len);
}
/* Upon integration, the following function may be merged with extsw_rreg_wrap()
   and the appropriate cases that need swap will byte swap.
   Our expectation is that MBUS_MMAP case would Not need swap.
 */
void extsw_wreg(int page, int reg, uint8 *data, int len)
{
    if (((len != 1) && (len % 2) != 0) || len > 8)
        panic("extsw_wreg: wrong length!\n");

    BCM_ENET_DEBUG("%s : page=%d reg=%d Data [ 0x%02x 0x%02x 0x%02x 0x%02x ] Len = %d\n",
                   __FUNCTION__, page, reg, data[0],data[1],data[2],data[3], len);

    switch (pVnetDev0_g->extSwitch->accessType)
    {
      case MBUS_MDIO:
        bcmsw_pmdio_wreg(page, reg, data, len);
        break;

      case MBUS_SPI:
      case MBUS_HS_SPI:
        bcmsw_spi_wreg(pVnetDev0_g->extSwitch->bus_num, pVnetDev0_g->extSwitch->spi_ss,
                       pVnetDev0_g->extSwitch->spi_cid, page, reg, data, len);
        break;
      default:
        printk("Error Access Type %d: Neither SPI nor PMDIO access in %s <page:0x%x, reg:0x%x>\n",
            pVnetDev0_g->extSwitch->accessType, __func__, page, reg);
        break;
    }
}

void extsw_fast_age_port(uint8 port, uint8 age_static)
{
    uint8 v8;
    uint8 timeout = 100;

    v8 = FAST_AGE_START_DONE | FAST_AGE_DYNAMIC | FAST_AGE_PORT;
    if (age_static) {
        v8 |= FAST_AGE_STATIC;
    }
    extsw_wreg_wrap(PAGE_CONTROL, REG_FAST_AGING_PORT, &port, 1);

    extsw_wreg_wrap(PAGE_CONTROL, REG_FAST_AGING_CTRL, &v8, 1);
    extsw_rreg_wrap(PAGE_CONTROL, REG_FAST_AGING_CTRL, &v8, 1);
    while (v8 & FAST_AGE_START_DONE) {
        mdelay(1);
        extsw_rreg_wrap(PAGE_CONTROL, REG_FAST_AGING_CTRL, &v8, 1);
        if (!timeout--) {
            printk("Timeout of fast aging \n");
            break;
        }
    }

    /* Restore DYNAMIC bit for normal aging */
    v8 = FAST_AGE_DYNAMIC;
    extsw_wreg_wrap(PAGE_CONTROL, REG_FAST_AGING_CTRL, &v8, 1);
}

void extsw_set_wanoe_portmap(uint16 wan_port_map)
{
    int i;

    wan_port_map |= GET_PORTMAP_FROM_LOGICAL_PORTMAP(pVnetDev0_g->softSwitchingMap, 1);

    extsw_wreg_wrap(PAGE_CONTROL, REG_WAN_PORT_MAP, &wan_port_map, 2);

    /* IMP port */
    wan_port_map |= imp_pbmap[1];

    wan_port_map |= GET_PORTMAP_FROM_LOGICAL_PORTMAP(pVnetDev0_g->learningDisabledPortMap, 1);

    /* Disable learning */
    extsw_wreg_wrap(PAGE_CONTROL, REG_DISABLE_LEARNING, &wan_port_map, 2);

    for(i=0; i < TOTAL_SWITCH_PORTS; i++) {
       if((wan_port_map >> i) & 0x1) {
            extsw_fast_age_port(i, 0);
       }
    }
}

void extsw_rreg_wrap(int page, int reg, void *vptr, int len)
{
    uint8 val[8];
    uint8 *data = (uint8*)vptr;
    int type = len & SWAP_TYPE_MASK;

    len &= ~(SWAP_TYPE_MASK);

    /* Lower level driver always returnes in Little Endian data from history */
    extsw_rreg(page, reg, val, len);

    switch (len) {
        case 1:
            data[0] = val[0];
            break;
        case 2:
            *((uint16 *)data) = __le16_to_cpu(*((uint16 *)val));
            break;
        case 4:
            *((uint32 *)data) = __le32_to_cpu(*((uint32 *)val));
            break;
        case 6:
            switch (type) {
                case DATA_TYPE_HOST_ENDIAN:
                default:
                    /*
                        Value type register access
                        Input:  val:Le64 from Lower driver API
                        Output: data:Host64, a pointer to the begining of 64 bit buffer
                    */
                    *(uint64*)data = __le64_to_cpu(*(uint64 *)val);
                    break;
                case DATA_TYPE_BYTE_STRING:
                    /*
                        Byte array for MAC address
                        Input:  val:Mac[5...0] from lower driver
                        Output: data:Mac[0...5]
                    */
                    *(uint64 *)val = __swab64(*(uint64*)val);
                    memcpy(data, val+2, 6);
                    break;
            }
            break;
        case 8:
            switch (type) {
                case DATA_TYPE_HOST_ENDIAN:
                default:
                    /*
                        Input:  val: Le64 for lower driver API
                        Output: data: Host64
                    */
                    *(uint64 *)data = __le64_to_cpu(*(uint64*)val);
                    break;
                case DATA_TYPE_BYTE_STRING:
                    /*
                        Input:  val:byte[7...0] from lower driver API
                        Output: data:byte[0...7]
                    */
                    *(uint64 *)data = __swab64(*(uint64*)val);
                    break;
                case DATA_TYPE_VID_MAC:
                    /*
                        VID-MAC type;
                        Input:  val:Mac[5...0]|VidLeWord from Lower Driver API
                        Output: data:VidHostWord|Mac[0...5] for Caller
                    */
                    /* [Mac[5-0]]|[LEWord]; First always swap all bytes */
                    *((uint64 *)data) = __swab64(*((uint64 *)val));
                    /* Now is [BEWord]|[Mac[0-5]]; Conditional Swap 2 bytes */
                    *((uint16 *)&data[0]) = __be16_to_cpu(*((uint16 *)&data[0]));
                    /* Now is HostEndianWord|Mac[0-5] */
                    break;
            } // switch type
            break;
        default:
            printk("Length %d not supported\n", len);
            break;
    }
    BCM_ENET_DEBUG(" page=%d reg=%d Data [ 0x%02x 0x%02x 0x%02x 0x%02x ] Len = %d\n",
            page, reg, data[0],data[1],data[2],data[3], len);
    if (len > 4)
        BCM_ENET_DEBUG(" page=%d reg=%d Data [ 0x%02x 0x%02x 0x%02x 0x%02x ] Len = %d\n",
                page, reg, data[4],data[5],data[6],data[7], len);
}

void extsw_wreg_wrap(int page, int reg, void *vptr, int len)
{
    uint8 val[8];
    uint8 *data = (uint8*)vptr;
    int type = len & SWAP_TYPE_MASK;

    len  &= ~(SWAP_TYPE_MASK);
    BCM_ENET_DEBUG("%s : page=%d reg=%d Data [ 0x%02x 0x%02x 0x%02x 0x%02x ] Len = %d\n",
            __FUNCTION__, page, reg, data[0],data[1],data[2],data[3], len);

    switch (len) {
        case 1:
            val[0] = data[0];
            break;
        case 2:
            *((uint16 *)val) = __cpu_to_le16(*((uint16 *)data));
            break;
        case 4:
            *((uint32 *)val) = __cpu_to_le32(*((uint32 *)data));
            break;
        case 6:
            switch(type) {
                case DATA_TYPE_HOST_ENDIAN:
                default:
                    /*
                        Value type register access
                        Input:  data:Host64, a pointer to the begining of 64 bit buffer
                        Output: val:Le64
                    */
                    *(uint64 *)val = __cpu_to_le64(*(uint64 *)data);
                    break;
                case DATA_TYPE_BYTE_STRING:
                    /*
                        Byte array for MAC address
                        Input:  data:MAC[0...5] from Host
                        Output: val:Mac[5...0] for lower driver API
                    */
                    memcpy(val+2, data, 6);
                    *(uint64 *)val = __swab64(*(uint64*)val);
                    break;
            }
            break;
        case 8:
            switch (type)
            {
                case DATA_TYPE_HOST_ENDIAN:
                default:
                    /*
                        Input: data:Host64
                        Output:  val:Le64 for lower driver API
                    */
                    *(uint64 *)val = __cpu_to_le64(*(uint64*)data);
                    break;
                case DATA_TYPE_BYTE_STRING:
                    /*
                        Input:  data:byte[0...7]
                        Output: val:byte[7...0] for lower driver API
                    */
                    *(uint64 *)val = __swab64(*(uint64*)data);
                    break;
                case DATA_TYPE_VID_MAC:
                    /*
                        VID-MAC type;
                        Input:  VidHostWord|Mac[0...5]
                        Output: Mac[5..0.]|VidLeWord for Lower Driver API
                    */
                    /* Contains HostEndianWord|MAC[0-5] Always swap first*/
                    *((uint64 *)val) = __swab64(*((uint64 *)data));
                    /* Now it is MAC[5-0]|SwappedHostEndianWord */
                    /* Convert the SwappedHostEndianWord to Little Endian; thus BE */
                    *((uint16 *)&val[6]) = __cpu_to_be16(*((uint16 *)&val[6]));
                    /* Now is MAC[5-0]|LEWord as requested by HW */
                    break;
            } // switch type
            break;
        default:
            printk("Length %d not supported\n", len);
            break;
    } // switch len
    extsw_wreg(page, reg, val, len);
}

static void fast_age_start_done_ext(uint8_t ctrl)
{
    uint8_t timeout = 100;

    extsw_wreg_wrap(PAGE_CONTROL, REG_FAST_AGING_CTRL, (uint8_t *)&ctrl, 1);
    extsw_rreg_wrap(PAGE_CONTROL, REG_FAST_AGING_CTRL, (uint8_t *)&ctrl, 1);
    while (ctrl & FAST_AGE_START_DONE) {
        mdelay(1);
        extsw_rreg_wrap(PAGE_CONTROL, REG_FAST_AGING_CTRL, (uint8_t *)&ctrl, 1);
        if (!timeout--) {
            printk("Timeout of fast aging \n");
            break;
        }
    }

    /* Restore DYNAMIC bit for normal aging */
    ctrl = FAST_AGE_DYNAMIC;
    extsw_wreg_wrap(PAGE_CONTROL, REG_FAST_AGING_CTRL, (uint8_t *)&ctrl, 1);
}

void fast_age_all_ext(uint8_t age_static)
{
    uint8_t v8;

    v8 = FAST_AGE_START_DONE | FAST_AGE_DYNAMIC;
    if (age_static) {
        v8 |= FAST_AGE_STATIC;
    }

    fast_age_start_done_ext(v8);
}

static int enet_arl_access_reg_op(u8 v8)
{
    int timeout;

    extsw_wreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
    for ( timeout = 10, extsw_rreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            (v8 & ARL_TBL_CTRL_START_DONE) && timeout;
            --timeout, extsw_rreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1))
    {
        mdelay(1);
    }

    if (timeout <= 0)
    {
        printk("Error: ARL Operation Timeout\n");
        return 0;
    }
    return 1;
}

/* v32: b31 is raw bit,
    If raw: register format; etherwise: b15 is Valid bit */
int enet_arl_write_ext( uint8_t *mac, uint16_t vid, uint32_t v32)
{
    u8 mac_vid[8];
    u32 cur_v32;
    u16 ent_vid;
    int bin, empty_bin = -1;

    if (!(v32 & (1<<31))) v32 = ((v32 & 0xfc00) << 1) | (v32 & 0x1ff);  /* If it is raw, shift valid bit left */
    v32 &= ~(1<<31);

    /* Write the MAC Address and VLAN ID */
    extsw_wreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_MAC_INDX_LO, mac, 6|DATA_TYPE_BYTE_STRING);
    extsw_wreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_VLAN_INDX, &vid, 2);
    if (!enet_arl_access_reg_op(ARL_TBL_CTRL_START_DONE | ARL_TBL_CTRL_READ)) return 0;

    for (bin = 0; bin < REG_ARL_BINS_PER_HASH; bin++)
    {
        /* Read transaction complete - get the MAC + VID */
        extsw_rreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_MAC_LO_ENTRY + bin*0x10, &mac_vid[0], 8|DATA_TYPE_VID_MAC);
        extsw_rreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_DATA_ENTRY + bin*0x10,(uint8_t *)&cur_v32, 4);
        ent_vid = *(u16*)mac_vid;

        if (!(v32 & ARL_DATA_ENTRY_VALID_531xx))
        {
            /* If it is del op, find the matched bin */
            if (memcmp(&mac[0], &mac_vid[2], 6) != 0 || ent_vid != vid) continue;
        }
        else
        {
            /* If it is a modification or addition,
               find a matching entry, empty slot or last slot */
            if (memcmp(&mac[0], &mac_vid[2], 6) == 0 && vid == ent_vid) goto found_slot;
            if (!(cur_v32 & ARL_DATA_ENTRY_VALID_531xx) && empty_bin == -1) empty_bin = bin;  /* save empty bin for non matching case */
            if (bin < REG_ARL_BINS_PER_HASH-1) continue;  /* Continue to find next bin for matching if it not the last */
            /* No matching found here, if there is empty bin, use empty_bin or use last bin */
            if (empty_bin != -1) bin = empty_bin;
        }

        found_slot:

        /* Modify the data entry for this ARL */
        *(uint16 *)(&mac_vid[0]) = (vid & 0xFFF);
        memcpy(&mac_vid[2], &mac[0], 6);
        extsw_wreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_MAC_LO_ENTRY + bin*0x10, mac_vid, 8|DATA_TYPE_VID_MAC);
        extsw_wreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_DATA_ENTRY + bin*0x10,(uint8_t *)&v32, 4);

        /* Initiate a write transaction */
        if (!enet_arl_access_reg_op(ARL_TBL_CTRL_START_DONE)) return 0;
        return 1;
    }
    printk("Error - can't find the requested ARL entry\n");
    return 0;
}

int enet_arl_entry_op(uint8_t *mac, uint32_t *vid, uint32_t *val, int op, int *count, u8 *mac_vid, u32 data)
{
    switch(op)
    {
        case TYPE_DUMP:
            if (*count == 0) printk("\nExternal Switch %x ARL Dump:\n", extSwInfo.switch_id);
            if ((((*count)++) % 10)==0) 
            {
                printk("  No: VLAN  MAC          DATA" "(15:Valid,14:Static,13:Age,12-10:Pri,8-0:Port/Pmap)\n");
            }

            printk("%4d: %04d  %02x%02x%02x%02x%02x%02x 0x%04x\n",
                    *count, *(uint16 *)&mac_vid[0],
                    mac_vid[2], mac_vid[3], mac_vid[4], mac_vid[5], mac_vid[6], mac_vid[7],
                    ((data & 0x1f800)>>1)|(data&0x1ff));
            break;
        case TYPE_SET:
            if (memcmp(&mac[0], &mac_vid[2], 6) == 0)
            {
                enet_arl_write_ext(mac, *(u16*)mac_vid, 0);
                (*count)++;
            }
            break;
        case TYPE_GET:
            if (memcmp(&mac[0], &mac_vid[2], 6) == 0 &&
                    (*vid == -1 || *vid == *(u16*)mac_vid))
            {
                /* entry found */
                *vid = *(u16*)mac_vid;
                if (*val & (1<<31)) /* Raw flag passed down from users space */
                {
                    *val = data;
                }
                else
                {
                    *val = ((data & 0x1f800)>>1)|(data & 0x1ff);
                }
                /* Return FALSE to terminate loop */
                return TRUE;
            }
            break;
    }
    return FALSE;
}

int enet_arl_search_ext(uint8_t *mac, uint32_t *vid, uint32_t *val, int op)
{
    int timeout = 1000, count = 0, hash_ent;
    uint32_t cur_data;
    uint8_t v8, mac_vid[8];

    v8 = ARL_SRCH_CTRL_START_DONE;
    extsw_wreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1);

    for( extsw_rreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1);
            (v8 & ARL_SRCH_CTRL_START_DONE);
            extsw_rreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1))
    {
        /* Now read the Search Ctrl Reg Until :
         * Found Valid ARL Entry --> ARL_SRCH_CTRL_SR_VALID, or
         * ARL Search done --> ARL_SRCH_CTRL_START_DONE */
        for(timeout = 1000;
                (v8 & ARL_SRCH_CTRL_SR_VALID) == 0 && (v8 & ARL_SRCH_CTRL_START_DONE) && timeout-- > 0;
                mdelay(1),
                extsw_rreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1));

        if ((v8 & ARL_SRCH_CTRL_SR_VALID) == 0 || timeout <= 0) break;

        /* Found a valid entry */
        for (hash_ent = 0; hash_ent < REG_ARL_SRCH_HASH_ENTS; hash_ent++)
        {
            extsw_rreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_MAC_LO_ENTRY0_531xx + hash_ent*0x10,&mac_vid[0], 8|DATA_TYPE_VID_MAC);
            extsw_rreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_DATA_ENTRY0_531xx + hash_ent*0x10,(uint8_t *)&cur_data, 4);

            BCM_ENET_DEBUG("ARL_SRCH_result (%02x%02x%02x%02x%02x%02x%02x%02x) \n",
                    mac_vid[0],mac_vid[1],mac_vid[2],mac_vid[3],mac_vid[4],mac_vid[5],mac_vid[6],mac_vid[7]);
            BCM_ENET_DEBUG("ARL_SRCH_DATA = 0x%08x \n", cur_data);

            if ((cur_data & ARL_DATA_ENTRY_VALID_531xx))
            {
                if (enet_arl_entry_op(mac, vid, val, op, &count, mac_vid, cur_data)) return TRUE;
            }
        }
    }

    if (timeout <= 0)
    {
        printk("ARL Search Timeout for Valid to be 1 \n");
    }

    if (op == TYPE_DUMP) printk("Done: Total %d entries\n", count);
    if (op == TYPE_GET) return FALSE;
    return TRUE;
}

void enet_arl_dump_ext_multiport_arl(void)
{
    uint16 v16;
    uint8 addr[8];
    int i, enabled;
    uint32 vect;
    static char *cmp_type[] = {"Disabled", "Etype", "MAC Addr", "MAC Addr & Etype"}; 

    extsw_rreg_wrap(PAGE_ARLCTRL, REG_MULTIPORT_CTRL, &v16, 2);
    enabled = v16 & ((MULTIPORT_CTRL_EN_M << (5*2))| (MULTIPORT_CTRL_EN_M << (4*2))| (MULTIPORT_CTRL_EN_M << (3*2))|
            (MULTIPORT_CTRL_EN_M << (2*2))| (MULTIPORT_CTRL_EN_M << (1*2))| (MULTIPORT_CTRL_EN_M << (0*2)));

    printk("\nExternal Switch Multiport Address Dump: Function %s\n", enabled? "Enabled": "Disabled");
    if (!enabled) return;

    printk("  Mapping to ARL matching: %s\n", v16 & (1<<MULTIPORT_CTRL_DA_HIT_EN)? "Lookup Hit": "Lookup Failed");
    for (i=0; i<6; i++)
    {
        enabled = (v16 >> (i*2)) & MULTIPORT_CTRL_EN_M;
        extsw_rreg_wrap(PAGE_ARLCTRL, REG_MULTIPORT_ADDR1_LO + i*16, (uint8 *)&addr, sizeof(addr)|DATA_TYPE_VID_MAC);
        extsw_rreg_wrap(PAGE_ARLCTRL, REG_MULTIPORT_VECTOR1 + i*16, (uint8 *)&vect, sizeof(vect));
        printk("Mport Eth Type: 0x%04x, Mport Addrs: %02x:%02x:%02x:%02x:%02x:%02x, Port Map %04x, Cmp Type: %s\n",
                *(uint16 *)(addr),
                addr[2],
                addr[3],
                addr[4],
                addr[5],
                addr[6],
                addr[7],
                (int)vect, cmp_type[enabled]);
    }
    printk("External Switch Multiport Address Dump Done\n");
}

int remove_arl_entry_wrapper(void *ptr)
{
    int ret = 0;
    ret = enet_arl_remove(ptr); /* remove entry from internal switch */
    if (bcm63xx_enet_isExtSwPresent())
    {
        ret = enet_arl_remove_ext(ptr); /* remove entry from internal switch */
    }
    return ret;
}

void bcmeapi_reset_mib_ext(void)
{
#ifdef REPORT_HARDWARE_STATS
    uint8_t val8;
    if (pVnetDev0_g->extSwitch->present) {
        extsw_rreg_wrap(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &val8, 1);
        val8 |= GLOBAL_CFG_RESET_MIB;
        extsw_wreg_wrap(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &val8, 1);
        val8 &= (~GLOBAL_CFG_RESET_MIB);
        extsw_wreg_wrap(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &val8, 1);
    }

#endif
    return ;
}
void  bcmsw_dump_page_ext(int page)
{
    switch(page) {

        default:
            break;
    }
}
int bcmsw_dump_mib_ext(int port, int type)
{
    unsigned int v32, errcnt;
    unsigned long port_map;
    uint8 data[8] = {0};
    ETHERNET_MAC_INFO *EnetInfo = EnetGetEthernetMacInfo();
    ETHERNET_MAC_INFO *info;

    info = &EnetInfo[1];
    if (!((info->ucPhyType == BP_ENET_EXTERNAL_SWITCH) ||
         (info->ucPhyType == BP_ENET_SWITCH_VIA_INTERNAL_PHY)))
    {
        printk("No External switch connected\n");
        return -ENODEV;
    }

    info = &EnetInfo[1];
    port_map = info->sw.port_map;
    port_map |= imp_pbmap[1];

    if (!(port_map & (1<<port))) /* Only IMP port and ports in the port_map are allowed */
    {
        printk("Invalid/Unused External switch port %d\n",port);
        return -ENODEV;
    }

    /* Display Tx statistics */
    printk("External Switch Stats : Port# %d\n",port);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXUPKTS, &v32, 4);  // Get TX unicast packet count
    printk("TxUnicastPkts:          %10u \n", v32);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXMPKTS, &v32, 4);  // Get TX multicast packet count
    printk("TxMulticastPkts:        %10u \n",  v32);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXBPKTS, &v32, 4);  // Get TX broadcast packet count
    printk("TxBroadcastPkts:        %10u \n", v32);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXDROPS, &v32, 4);
    printk("TxDropPkts:             %10u \n", v32);

    if (type)
    {
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXOCTETS, data, DATA_TYPE_HOST_ENDIAN|8);
        v32 = *((uint64 *)data);
        printk("TxOctetsLo:             %10u \n", v32);
        v32 = *((uint64 *)data) >> 32;
        printk("TxOctetsHi:             %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXQ0PKT, &v32, 4);
        printk("TxQ0Pkts:               %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXQ1PKT, &v32, 4);
        printk("TxQ1Pkts:               %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXQ2PKT, &v32, 4);
        printk("TxQ2Pkts:               %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXQ3PKT, &v32, 4);
        printk("TxQ3Pkts:               %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXQ4PKT, &v32, 4);
        printk("TxQ4Pkts:               %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXQ5PKT, &v32, 4);
        printk("TxQ5Pkts:               %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXCOL, &v32, 4);
        printk("TxCol:                  %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXSINGLECOL, &v32, 4);
        printk("TxSingleCol:            %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXMULTICOL, &v32, 4);
        printk("TxMultipleCol:          %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXDEFERREDTX, &v32, 4);
        printk("TxDeferredTx:           %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXLATECOL, &v32, 4);
        printk("TxLateCol:              %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXEXCESSCOL, &v32, 4);
        printk("TxExcessiveCol:         %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXFRAMEINDISC, &v32, 4);
        printk("TxFrameInDisc:          %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXPAUSEPKTS, &v32, 4);
        printk("TxPausePkts:            %10u \n", v32);
    }
    else
    {
        errcnt = 0;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXCOL, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXSINGLECOL, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXMULTICOL, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXDEFERREDTX, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXLATECOL, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXEXCESSCOL, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXFRAMEINDISC, &v32, 4);
        errcnt += v32;
        printk("TxOtherErrors:          %10u \n", errcnt);
    }
    /* Display Rx statistics */
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXUPKTS, &v32, 4);  // Get RX unicast packet count
    printk("RxUnicastPkts:          %10u \n", v32);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXMPKTS, &v32, 4);  // Get RX multicast packet count
    printk("RxMulticastPkts:        %10u \n",v32);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXBPKTS, &v32, 4);  // Get RX broadcast packet count
    printk("RxBroadcastPkts:        %10u \n",v32);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXDROPS, &v32, 4);
    printk("RxDropPkts:             %10u \n",v32);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXDISCARD, &v32, 4);
    printk("RxDiscard:              %10u \n", v32);

    if (type)
    {
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXOCTETS, data, DATA_TYPE_HOST_ENDIAN|8);
        v32 = *((uint64 *)data);
        printk("RxOctetsLo:             %10u \n", v32);
        v32 = *((uint64 *)data) >> 32;
        printk("RxOctetsHi:             %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXGOODOCT, data, DATA_TYPE_HOST_ENDIAN|8);
        v32 = *((uint64 *)data);
        printk("RxGoodOctetsLo:         %10u \n", v32);
        v32 = *((uint64 *)data) >> 32;
        printk("RxGoodOctetsHi:         %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXJABBERS, &v32, 4);
        printk("RxJabbers:              %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXALIGNERRORS, &v32, 4);
        printk("RxAlignErrs:            %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXFCSERRORS, &v32, 4);
        printk("RxFCSErrs:              %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXFRAGMENTS, &v32, 4);
        printk("RxFragments:            %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXOVERSIZE, &v32, 4);
        printk("RxOversizePkts:         %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXUNDERSIZEPKTS, &v32, 4);
        printk("RxUndersizePkts:        %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXPAUSEPKTS, &v32, 4);
        printk("RxPausePkts:            %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXSACHANGES, &v32, 4);
        printk("RxSAChanges:            %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXSYMBOLERRORS, &v32, 4);
        printk("RxSymbolError:          %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RX64OCTPKTS, &v32, 4);
        printk("RxPkts64Octets:         %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RX127OCTPKTS, &v32, 4);
        printk("RxPkts65to127Octets:    %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RX255OCTPKTS, &v32, 4);
        printk("RxPkts128to255Octets:   %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RX511OCTPKTS, &v32, 4);
        printk("RxPkts256to511Octets:   %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RX1023OCTPKTS, &v32, 4);
        printk("RxPkts512to1023Octets:  %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXMAXOCTPKTS, &v32, 4);
        printk("RxPkts1024OrMoreOctets: %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXJUMBOPKT , &v32, 4);
        printk("RxJumboPkts:            %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXOUTRANGEERR, &v32, 4);
        printk("RxOutOfRange:           %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXINRANGEERR, &v32, 4);
        printk("RxInRangeErr:           %10u \n", v32);
    }
    else
    {
        errcnt=0;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXJABBERS, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXALIGNERRORS, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXFCSERRORS, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXOVERSIZE, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXUNDERSIZEPKTS, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXSYMBOLERRORS, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXOUTRANGEERR, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXINRANGEERR, &v32, 4);
        errcnt += v32;
        printk("RxOtherErrors:          %10u \n", errcnt);
    }

    return 0;
}


int bcmsw_set_multiport_address_ext(uint8_t* addr)
{
    if (bcm63xx_enet_isExtSwPresent())
    {
        int i;
        uint32 v32;
        uint16 v16;
        uint8 v64[8];
        uint8 cur64[8];

        *(uint16*)(&v64[0]) = 0;
        memcpy(&v64[2], addr, 6);
        /* check if address is set already */
        for ( i = 0; i < MULTIPORT_CTRL_COUNT; i++ )
        {
           extsw_rreg_wrap(PAGE_ARLCTRL, (REG_MULTIPORT_ADDR1_LO + (i * 0x10)), (uint8 *)&cur64, sizeof(cur64)|DATA_TYPE_VID_MAC);
           if ( 0 == memcmp(&v64[0], &cur64[0], 8) )
           {
               return 0;
           }
        }

        /* add new entry */
        for ( i = 0; i < MULTIPORT_CTRL_COUNT; i++ )
        {
            extsw_rreg_wrap(PAGE_ARLCTRL, REG_MULTIPORT_CTRL, (uint8 *)&v16, 2);
            if ( 0 == (v16 & (MULTIPORT_CTRL_EN_M << (i << 1))))
            {
                v16 |= (1<<MULTIPORT_CTRL_DA_HIT_EN) | (MULTIPORT_CTRL_ADDR_CMP << (i << 1));
                extsw_wreg_wrap(PAGE_ARLCTRL, REG_MULTIPORT_CTRL, (uint8 *)&v16, 2);
                *(uint16*)(&v64[0]) = 0;
                memcpy(&v64[2], addr, 6);
                extsw_wreg_wrap(PAGE_ARLCTRL, (REG_MULTIPORT_ADDR1_LO + (i * 0x10)), (uint8 *)&v64, sizeof(v64)|DATA_TYPE_VID_MAC);
                v32 = imp_pbmap[1];
                extsw_wreg_wrap(PAGE_ARLCTRL, (REG_MULTIPORT_VECTOR1 + (i * 0x10)), (uint8 *)&v32, sizeof(v32));

                /* Set multiport VLAN control based on U/V_FWD_MAP;
                   This is required so that VLAN tagged frames matching Multiport Address are forwarded according to V/U forwarding map */
                extsw_rreg_wrap(PAGE_8021Q_VLAN, REG_VLAN_MULTI_PORT_ADDR_CTL, &v16, sizeof(v16));
                v16 |=  (EN_MPORT_V_FWD_MAP | EN_MPORT_U_FWD_MAP) << (i*EN_MPORT_V_U_FWD_MAP_S) ;
                extsw_wreg_wrap(PAGE_8021Q_VLAN, REG_VLAN_MULTI_PORT_ADDR_CTL, &v16, sizeof(v16));

                return 0;
            }
        }
    }

    return -1;
}

int bcmeapi_ioctl_set_multiport_address(struct ethswctl_data *e)
{
    if (e->unit == 0) {
       if (e->type == TYPE_GET) {
           return BCM_E_NONE;
       } else if (e->type == TYPE_SET) {
           bcmeapi_set_multiport_address(e->mac);
           bcmsw_set_multiport_address_ext(e->mac);
           return BCM_E_PARAM;
       }
    }
    return BCM_E_NONE;
}

#ifdef REPORT_HARDWARE_STATS
int bcmsw_get_hw_stats(int port, int extswitch, struct rtnl_link_stats64 *stats)
{
    uint64 ctr64 = 0;           // Running 64 bit counter
    uint8 data[8] = {0};

    if (extswitch) {

        // Track RX unicast, multicast, and broadcast packets
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXUPKTS, data, 4);  // Get RX unicast packet count
        ctr64 = (*(uint32 *)data);                                // Keep running count

        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXMPKTS, data, 4);  // Get RX multicast packet count
        stats->multicast = (*(uint32 *)data);                                   // Save away count
        ctr64 += (*(uint32 *)data);                                             // Keep running count

        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXBPKTS, data, 4);  // Get RX broadcast packet count
        stats->rx_broadcast_packets = (*(uint32 *)data);                        // Save away count
        ctr64 += (*(uint32 *)data);                                             // Keep running count
        stats->rx_packets = ctr64;

        // Dump RX debug data if needed
        BCM_ENET_DEBUG("read data = %02x %02x %02x %02x \n",
            data[0], data[1], data[2], data[3]);
        BCM_ENET_DEBUG("ctr64 = %x \n", (unsigned int)ctr64);

        // Track RX byte count
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXOCTETS, data, DATA_TYPE_HOST_ENDIAN|8);
        stats->rx_bytes = *((uint64 *)data);

        // Track RX packet errors
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXDROPS, data, 4);
        stats->rx_dropped = (*(uint32 *)data);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXDISCARD, data, 4);
        stats->rx_dropped += (*(uint32 *)data);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXFCSERRORS, data, 4);
        stats->rx_errors = (*(uint32 *)data);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXSYMBOLERRORS, data, 4);
        stats->rx_errors += (*(uint32 *)data);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXALIGNERRORS, data, 4);
        stats->rx_errors += (*(uint32 *)data);

        // Track TX unicast, multicast, and broadcast packets
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXUPKTS, data, 4);  // Get TX unicast packet count
        ctr64 = (*(uint32 *)data);                                // Keep running count

        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXMPKTS, data, 4);  // Get TX multicast packet count
        stats->tx_multicast_packets = (*(uint32 *)data);                        // Save away count
        ctr64 += (*(uint32 *)data);                                             // Keep running count

        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXBPKTS, data, 4);  // Get TX broadcast packet count
        stats->tx_broadcast_packets = (*(uint32 *)data);                        // Save away count
        ctr64 += (*(uint32 *)data);                                             // Keep running count
        stats->tx_packets = ctr64;

        // Track TX byte count
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXOCTETS, data, DATA_TYPE_HOST_ENDIAN|8);
        stats->tx_bytes = *((uint64 *)data);

        // Track TX packet errors
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXDROPS, data, 4);
        stats->tx_dropped = (*(uint32 *)data);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXFRAMEINDISC, data, 4);
        stats->tx_dropped += (*(uint32 *)data);
    } else
    {
       ethsw_get_hw_stats(port, stats);
    }
    return 0;
}
#endif

int bcmeapi_ioctl_extsw_port_jumbo_control(struct ethswctl_data *e)
{
    uint32 val32;

    if (e->type == TYPE_GET)
    {
        // Read & log current JUMBO configuration control register.
        extsw_rreg_wrap(PAGE_JUMBO, REG_JUMBO_PORT_MASK, (uint8 *)&val32, 4);
        BCM_ENET_DEBUG("JUMBO_PORT_MASK = 0x%08X", (unsigned int)val32);
        e->ret_val = val32;
    }
    else
    {
        // Read & log current JUMBO configuration control register.
        extsw_rreg_wrap(PAGE_JUMBO, REG_JUMBO_PORT_MASK, (uint8 *)&val32, 4);
        BCM_ENET_DEBUG("Old JUMBO_PORT_MASK = 0x%08X", (unsigned int)val32);

        // Setup JUMBO configuration control register.
        val32 = ConfigureJumboPort(val32, e->port, e->val);
        extsw_wreg_wrap(PAGE_JUMBO, REG_JUMBO_PORT_MASK, (uint8 *)&val32, 4);

        // Attempt to transfer register write value to user space & test for success.
        e->ret_val = val32;
    }
    return BCM_E_NONE;
}

static uint16_t dis_learning_ext = 0x0100; /* This default value does not matter */


int bcmsw_enable_hw_switching(void)
{
    u8 i;

    /* restore disable learning register */
    extsw_wreg_wrap(PAGE_CONTROL, REG_DISABLE_LEARNING, &dis_learning_ext, 2);

    i = 0;
    while (vnet_dev[i])
    {
        if (LOGICAL_PORT_TO_UNIT_NUMBER(VPORT_TO_LOGICAL_PORT(i)) != 1) /* Not External switch port */
        {
            i++;  /* Go to next port */
            continue;
        }
        /* When hardware switching is enabled, enable the Linux bridge to
          not to forward the bcast packets on hardware ports */
        vnet_dev[i++]->priv_flags |= IFF_HW_SWITCH;
    }

    return 0;
}

int bcmsw_disable_hw_switching(void)
{
    u8 i;
    u16 reg_value;

    /* Save disable_learning_reg setting */
    extsw_rreg_wrap(PAGE_CONTROL, REG_DISABLE_LEARNING, &dis_learning_ext, 2);
    /* disable learning on all ports */
    reg_value = PBMAP_ALL;
    extsw_wreg_wrap(PAGE_CONTROL, REG_DISABLE_LEARNING, &reg_value, 2);

    i = 0;
    while (vnet_dev[i])
    {
        if (LOGICAL_PORT_TO_UNIT_NUMBER(VPORT_TO_LOGICAL_PORT(i)) != 1) /* Not External switch port */
        {
            i++;  /* Go to next port */
            continue;
        }
        /* When hardware switching is disabled, enable the Linux bridge to
          forward the bcast on hardware ports as well */
        vnet_dev[i++]->priv_flags &= ~IFF_HW_SWITCH;
    }

    /* Flush arl table dynamic entries */
    fast_age_all_ext(0);
    return 0;
}

int bcmeapi_ioctl_extsw_pid_to_priority_mapping(struct ethswctl_data *e)
{

    BCM_ENET_ERROR("No handler! \n");
    return 0;
}


/* This works for internal ROBO and external 53125 switches */
int bcmeapi_ioctl_ethsw_cos_priority_method_config(struct ethswctl_data *e)
{
    uint32_t v32;
    uint16_t v16 = 0;
    uint8_t v8, u8 = 0;
    uint8_t port_qos_en, qos_layer_sel;
    uint16_t port_dscp_en, port_pcp_en;

    down(&bcm_ethlock_switch_config);

    BCM_ENET_DEBUG(" type:  %02d\n ", e->type);
    if (e->type == TYPE_GET) {
        SW_READ_REG(e->unit, PAGE_QOS, REG_QOS_GLOBAL_CTRL, (void *)&v8, 1);
        port_qos_en = (v8 >> PORT_QOS_EN_S) & PORT_QOS_EN_M;
        qos_layer_sel = (v8 >> QOS_LAYER_SEL_S) & QOS_LAYER_SEL_M;
        SW_READ_REG(e->unit, PAGE_QOS, REG_QOS_8021P_EN, (void *)&v16, 2);
        port_pcp_en = v16 & (1UL << e->port);
        SW_READ_REG(e->unit, PAGE_QOS, REG_QOS_DSCP_EN, (void *)&v16, 2);
        port_dscp_en = v16 & (1UL << e->port);
        BCM_ENET_DEBUG(" port %d v16 %#x qos_layer_sel %d port_pcp_en %#x "
                        "port_dscp_en %#x port_qos_en %#x \n",
                        e->port, v16, qos_layer_sel, port_pcp_en, port_dscp_en, port_qos_en);

        if (port_qos_en) {
            switch (qos_layer_sel)
            {
                case 3:
                    // ? when your are here, port based QOS is always enabled.
                    v32 = PORT_QOS;
                    break;
                default:
                    v32 = PORT_QOS;
                    break;
            }
        } else {
            switch (qos_layer_sel)
            {
                case 3:
                    if (port_pcp_en) {
                        v32 = IEEE8021P_QOS;
                    } else if (port_dscp_en) {
                        v32 = DIFFSERV_QOS;
                    } else {
                        v32 = MAC_QOS;
                    }
                    break;
                // When we program, we set qos_layer == 3. So, following are moot.
                case 2:
                    if (port_dscp_en) {
                        v32 = DIFFSERV_QOS;
                    } else if (port_pcp_en) {
                        v32 = IEEE8021P_QOS;
                    } else {
                        v32 = MAC_QOS;
                    }
                    break;
                case 1:
                    if (port_dscp_en) {
                        v32 = DIFFSERV_QOS;
                    } else {
                        v32 = TC_ZERO_QOS;
                    }
                    break;
                case 0:
                    if (port_pcp_en) {
                        v32 = IEEE8021P_QOS;
                    } else {
                        v32 = MAC_QOS;
                    }
                    break;
                default:
                    break;
            }
        }
        e->ret_val = v32;
        BCM_ENET_DEBUG("e->ret_val is = %02d", e->ret_val);
    // SET
    } else {
        BCM_ENET_DEBUG("port %d Given method: %02d ADD \n ", e->port, e->val);
        SW_READ_REG(e->unit, PAGE_QOS, REG_QOS_GLOBAL_CTRL, (void *)&v8, 1);
        v8 &= ~(PORT_QOS_EN_M << PORT_QOS_EN_S);
        u8 = QOS_LAYER_SEL_M;
        if (e->val == MAC_QOS) {
           // disable per port .1p qos, & dscp

            SW_READ_REG(e->unit, PAGE_QOS, REG_QOS_8021P_EN, (void *)&v16, 2);
            v16 &= ~(1 << e->port);
            SW_WRITE_REG(e->unit, PAGE_QOS, REG_QOS_8021P_EN, (void *)&v16, 2);

            SW_READ_REG(e->unit, PAGE_QOS, REG_QOS_DSCP_EN, (void *)&v16, 2);
            v16 &= ~(1 << e->port);
            SW_WRITE_REG(e->unit, PAGE_QOS, REG_QOS_DSCP_EN, (void *)&v16, 2);
        } else if (e->val == IEEE8021P_QOS) {
           // enable .1p qos and  disable dscp

            SW_READ_REG(e->unit, PAGE_QOS, REG_QOS_8021P_EN, (void *)&v16, 2);
            v16 |= (1 << e->port);
            SW_WRITE_REG(e->unit, PAGE_QOS, REG_QOS_8021P_EN, (void *)&v16, 2);

            SW_READ_REG(e->unit, PAGE_QOS, REG_QOS_DSCP_EN, (void *)&v16, 2);
            v16 &= ~(1 << e->port);
            SW_WRITE_REG(e->unit, PAGE_QOS, REG_QOS_DSCP_EN, (void *)&v16, 2);
        } else if (e->val == DIFFSERV_QOS) {  // DSCP QOS
           // enable dscp qos and disable .1p

            SW_READ_REG(e->unit, PAGE_QOS, REG_QOS_DSCP_EN, (void *)&v16, 2);
            v16 |= (1 << e->port);
            SW_WRITE_REG(e->unit, PAGE_QOS, REG_QOS_DSCP_EN, (void *)&v16, 2);

            SW_READ_REG(e->unit, PAGE_QOS, REG_QOS_8021P_EN, (void *)&v16, 2);
            v16 &= ~(1 << e->port);
            SW_WRITE_REG(e->unit, PAGE_QOS, REG_QOS_8021P_EN, (void *)&v16, 2);
        } else {
            v8 |= (PORT_QOS_EN_M << PORT_QOS_EN_S);
        }
        v8 |= u8 << QOS_LAYER_SEL_S;
        SW_WRITE_REG(e->unit, PAGE_QOS, REG_QOS_GLOBAL_CTRL, (void *)&v8, 1);
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}
/*
 * Get/Set PID to TC mapping Tabe entry given ingress port
 * and mapped priority
 *** Input params
 * e->type  GET/SET
 * e->priority - mapped TC value, case of SET
 *** Output params
 * e->priority - mapped TC value, case of GET
 * Returns 0 for Success, Negative value for failure.
 */
int bcmeapi_ioctl_ethsw_pid_to_priority_mapping(struct ethswctl_data *e)
{

    uint32_t val16;
    BCM_ENET_DEBUG("Given uint %02d port %02d \n ", e->unit, e->port);

    if (e->port < 0 || (e->unit == 1 &&  e->port >= MAX_EXT_SWITCH_PORTS) ||
                          (e->unit == 0 &&  e->port >= TOTAL_SWITCH_PORTS))
    {
        printk("Invalid port number %02d \n", e->port);
        return BCM_E_ERROR;
    }

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        SW_READ_REG(e->unit, PAGE_8021Q_VLAN, REG_VLAN_DEFAULT_TAG + e->port * 2, (void *)&val16, 2);
        e->priority = (val16 >> DEFAULT_TAG_PRIO_S) & DEFAULT_TAG_PRIO_M;
        BCM_ENET_DEBUG("port %d is mapped to priority: %d \n ", e->port, e->priority);
    } else {
        BCM_ENET_DEBUG("Given port: %02d priority: %02d \n ", e->port, e->priority);
        SW_READ_REG(e->unit, PAGE_8021Q_VLAN, REG_VLAN_DEFAULT_TAG + e->port * 2, (void *)&val16, 2);
        val16 &= ~(DEFAULT_TAG_PRIO_M << DEFAULT_TAG_PRIO_S);
        val16 |= (e->priority & DEFAULT_TAG_PRIO_M) << DEFAULT_TAG_PRIO_S;
        SW_WRITE_REG(e->unit, PAGE_8021Q_VLAN, REG_VLAN_DEFAULT_TAG + e->port * 2, (void *)&val16, 2);
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}
int bcmeapi_ioctl_extsw_cos_priority_method_config(struct ethswctl_data *e)
{

    BCM_ENET_ERROR("No handler! \n");
    return 0;
}

/*
 * This applies only for Star Fighter switch
 */
int bcmeapi_ioctl_extsw_port_shaper_config(struct ethswctl_data *e)
{
    BCM_ENET_ERROR("No handler! \n");
    return 0;

}

int bcmeapi_ioctl_extsw_port_erc_config(struct ethswctl_data *e)
{
    BCM_ENET_ERROR("No handler! \n");
    return 0;

}

/*
 * Get/Set PCP to TC mapping Tabe entry given 802.1p priotity (PCP)
 * and mapped priority
 *** Input params
 * e->type  GET/SET
 * e->val -  pcp
 * e->priority - mapped TC value, case of SET
 *** Output params
 * e->priority - mapped TC value, case of GET
 * Returns 0 for Success, Negative value for failure.
 */
int bcmeapi_ioctl_extsw_pcp_to_priority_mapping(struct ethswctl_data *e)
{

    uint32_t val32;
    uint16_t reg_addr;

    BCM_ENET_DEBUG("Given pcp: %02d \n ", e->val);
    if (e->val > MAX_PRIORITY_VALUE) {
        printk("Invalid PCP Value %02d \n", e->val);
        return BCM_E_ERROR;
    }

    if (e->port < 0 || (e->port > MAX_EXT_SWITCH_PORTS &&  e->port != IMP_PORT_ID)) {
        printk("Invalid port number %02d \n", e->port);
        return BCM_E_ERROR;
    }
    reg_addr = e->port == IMP_PORT_ID? REG_QOS_8021P_PRIO_MAP_IP:
                          REG_QOS_8021P_PRIO_MAP + e->port * QOS_PCP_MAP_REG_SZ;

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        extsw_rreg_wrap(PAGE_QOS, reg_addr, (void *)&val32, 4);
        e->priority = (val32 >> (e->val * QOS_TC_S)) & QOS_TC_M;
        BCM_ENET_DEBUG("pcp %d is mapped to priority: %d \n ", e->val, e->priority);
    } else {
        BCM_ENET_DEBUG("Given pcp: %02d priority: %02d \n ", e->val, e->priority);
        if ((e->priority > MAX_PRIORITY_VALUE) || (e->priority < 0)) {
            printk("Invalid Priority \n");
            up(&bcm_ethlock_switch_config);
            return BCM_E_ERROR;
        }
        extsw_rreg_wrap(PAGE_QOS, reg_addr, (void *)&val32, 4);
        val32 &= ~(QOS_TC_M << (e->val * QOS_TC_S));
        val32 |= (e->priority & QOS_TC_M) << (e->val * QOS_TC_S);
        extsw_wreg_wrap(PAGE_QOS, reg_addr, (void *)&val32, 4);
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}
/*
 * Get/Set DSCP to TC mapping Tabe entry given dscp value and priority
 *** Input params
 * e->type  GET/SET
 * e->val -  dscp
 * e->priority - mapped TC value, case of SET
 *** Output params
 * e->priority - mapped TC value, case of GET
 * Returns 0 for Success, Negative value for failure.
 */
int bcmeapi_ioctl_extsw_dscp_to_priority_mapping(struct ethswctl_data *e)
{

    uint64 val64 = 0;
    uint32_t mapnum;
    int dscplsbs;

    BCM_ENET_DEBUG("Given dscp: %02d \n ", e->val);
    if (e->val > QOS_DSCP_M) {
        printk("Invalid DSCP Value \n");
        return BCM_E_ERROR;
    }

    down(&bcm_ethlock_switch_config);

    dscplsbs = e->val & QOS_DSCP_MAP_LSBITS_M;
    mapnum = (e->val >> QOS_DSCP_MAP_S) & QOS_DSCP_MAP_M;

    if (e->type == TYPE_GET) {
        extsw_rreg_wrap(PAGE_QOS, REG_QOS_DSCP_PRIO_MAP0LO + mapnum * QOS_DSCP_MAP_REG_SZ,
                                 (void *)&val64, QOS_DSCP_MAP_REG_SZ | DATA_TYPE_HOST_ENDIAN);
        e->priority = (val64 >> (dscplsbs * QOS_TC_S)) & QOS_TC_M;
        BCM_ENET_DEBUG("dscp %d is mapped to priority: %d \n ", e->val, e->priority);
    } else {
        BCM_ENET_DEBUG("Given priority: %02d \n ", e->priority);
        if ((e->priority > MAX_PRIORITY_VALUE) || (e->priority < 0)) {
            printk("Invalid Priority \n");
            up(&bcm_ethlock_switch_config);
            return BCM_E_ERROR;
        }
        // LE assumptions below, TODO
        extsw_rreg_wrap(PAGE_QOS, REG_QOS_DSCP_PRIO_MAP0LO + mapnum * QOS_DSCP_MAP_REG_SZ,
                                     (void *)&val64, QOS_DSCP_MAP_REG_SZ | DATA_TYPE_HOST_ENDIAN);
        val64 &= ~(((uint64)(QOS_TC_M)) << (dscplsbs * QOS_TC_S));
        val64 |= ((uint64)(e->priority & QOS_TC_M)) << (dscplsbs * QOS_TC_S);
        BCM_ENET_DEBUG(" @ addr %#x val64 to write = 0x%llx \n",
                                (REG_QOS_DSCP_PRIO_MAP0LO + mapnum * QOS_DSCP_MAP_REG_SZ),
                                (uint64) val64);

        extsw_wreg_wrap(PAGE_QOS, REG_QOS_DSCP_PRIO_MAP0LO + mapnum * QOS_DSCP_MAP_REG_SZ,
                                            (void *)&val64, QOS_DSCP_MAP_REG_SZ | DATA_TYPE_HOST_ENDIAN);
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}
/*
 * Get/Set cos(queue) mapping, given priority (TC)
 *** Input params
 * e->type  GET/SET
 * e->queue - target queue
 * e->port  per port
 *** Output params
 * Returns 0 for Success, Negative value for failure.
 */
int bcmeapi_ioctl_extsw_cosq_port_mapping(struct ethswctl_data *e)
{
    union {
        uint32_t val32;
        uint16_t val16;
    }val;
    int queue;
    int retval = 0;
    uint16_t reg_addr;
    uint16_t cos_shift;
    uint16_t cos_mask;
    uint16_t reg_len;

    BCM_ENET_DEBUG("Given port: %02d priority: %02d \n ", e->port, e->priority);
    if (e->port >= TOTAL_SWITCH_PORTS
       ) {
        printk("Invalid Switch Port %02d \n", e->port);
        return -BCM_E_ERROR;
    }
    if ((e->priority > MAX_PRIORITY_VALUE) || (e->priority < 0)) {
        printk("Invalid Priority \n");
        return -BCM_E_ERROR;
    }
    reg_addr  = REG_QOS_PORT_PRIO_MAP_EXT;
    cos_shift = REG_QOS_PRIO_TO_QID_SEL_BITS;
    cos_mask  = REG_QOS_PRIO_TO_QID_SEL_M;
    reg_len   = 2;

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        extsw_rreg_wrap(PAGE_QOS, reg_addr, (uint8_t *)&val, reg_len);
        BCM_ENET_DEBUG("REG_QOS_PORT_PRIO_MAP_Px = %p",
                (void*)&val);
        /* Get the queue */
        val.val32 = val.val16;
        queue = (val.val32 >> (e->priority * cos_shift)) & cos_mask;
        retval = queue & REG_QOS_PRIO_TO_QID_SEL_M;
        BCM_ENET_DEBUG("%s queue is = %4x", __FUNCTION__, retval);
    } else {
        BCM_ENET_DEBUG("Given queue: 0x%02x \n ", e->queue);
        extsw_rreg_wrap(PAGE_QOS, reg_addr, (uint8_t *)&val, reg_len);
        /* Other External switches define 16 bit TC to COS */
        val.val16 &= ~(cos_mask << (e->priority * cos_shift));
        val.val16 |= (e->queue & cos_mask) << (e->priority * cos_shift);
        extsw_wreg_wrap(PAGE_QOS, reg_addr, (uint8_t *)&val, reg_len);
    }
    up(&bcm_ethlock_switch_config);
    return retval;
}

#define MAX_WRR_WEIGHT 0x31
static int extsw_cosq_sched(struct ethswctl_data *e)
{
    uint8_t  val8, txq_mode;
    int i, val, sched;

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        extsw_rreg_wrap(PAGE_QOS, REG_QOS_TXQ_CTRL, &val8, 1);
        BCM_ENET_DEBUG("REG_QOS_TXQ_CTRL = 0x%2x", val8);
        txq_mode = (val8 >> TXQ_CTRL_TXQ_MODE_EXT_S) & TXQ_CTRL_TXQ_MODE_EXT_M;
        if (txq_mode) {
            if(txq_mode == 3) {
                sched = BCM_COSQ_STRICT;
            } else {
                sched = BCM_COSQ_COMBO;
                e->val = txq_mode;
            }
        } else {
            sched = BCM_COSQ_WRR;
        }

        e->scheduling = sched;

        /* Get the weights */
        if(sched != BCM_COSQ_STRICT) {
            for (i=0; i < NUM_EGRESS_QUEUES; i++) {
                extsw_rreg_wrap(PAGE_QOS, REG_QOS_TXQ_WEIGHT_Q0 + i, &val8, 1);
                BCM_ENET_DEBUG("Weight[%2d] = %02d ", i, val8);
                val = val8;
                e->weights[i] = val;
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
        extsw_rreg_wrap(PAGE_QOS, REG_QOS_TXQ_CTRL, &val8, 1);
        BCM_ENET_DEBUG("REG_QOS_TXQ_CTRL = 0x%02x", val8);
        txq_mode = (val8 >> TXQ_CTRL_TXQ_MODE_EXT_S) & TXQ_CTRL_TXQ_MODE_EXT_M;
        /* Set the scheduling mode */
        if (e->scheduling == BCM_COSQ_WRR) {
            // Set TXQ_MODE bits for 4 queue mode.  Leave high
            // queue preeempt bit cleared so queue weighting will be used.
            val8 = 0;  // ALL WRR queues in ext switch port
        } else if ((e->scheduling == BCM_COSQ_STRICT) ||
                   (e->scheduling == BCM_COSQ_COMBO)){
            if (e->scheduling == BCM_COSQ_STRICT) {
                txq_mode = 3;
            } else {
                txq_mode = e->queue;
            }
            val8 &= (~(TXQ_CTRL_TXQ_MODE_EXT_M << TXQ_CTRL_TXQ_MODE_EXT_S));
            val8 |= ((txq_mode & TXQ_CTRL_TXQ_MODE_EXT_M) << TXQ_CTRL_TXQ_MODE_EXT_S);
        } else {
            BCM_ENET_DEBUG("Invalid scheduling mode %02d", e->scheduling);
            up(&bcm_ethlock_switch_config);
            return BCM_E_PARAM;
        }
        BCM_ENET_DEBUG("Writing 0x%02x to REG_QOS_TXQ_CTRL", val8);
        extsw_wreg_wrap(PAGE_QOS, REG_QOS_TXQ_CTRL, &val8, 1);
        /* Set the weights if WRR or COMBO */
        if(e->scheduling != BCM_COSQ_STRICT) {
            for (i=0; i < NUM_EGRESS_QUEUES; i++) {
                BCM_ENET_DEBUG("Weight[%2d] = %02d ", i, e->weights[i]);
                val8 =  e->weights[i];
                extsw_wreg_wrap(PAGE_QOS, REG_QOS_TXQ_WEIGHT_Q0 + i, &val8, 1);
            }
        }
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}
int bcmeapi_ioctl_extsw_cosq_sched(struct ethswctl_data *e)
{
    int ret = 0;
    ret =  extsw_cosq_sched(e);
    if (ret >= 0) {
        if (e->type == TYPE_GET) {
            e->ret_val = e->val;
            BCM_ENET_DEBUG("e->ret_val is = %4x", e->ret_val);
        }
    }
    return ret;
}

uint16_t extsw_get_pbvlan(int port)
{
    uint16_t val16;

    extsw_rreg_wrap(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (port * 2),
               (uint8_t *)&val16, 2);
    return val16;
}
void extsw_set_pbvlan(int port, uint16_t fwdMap)
{
    extsw_wreg_wrap(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (port * 2),
               (uint8_t *)&fwdMap, 2);
}

int bcmeapi_ioctl_extsw_pbvlan(struct ethswctl_data *e)
{
    uint16_t val16;

    BCM_ENET_DEBUG("Given Port: 0x%02x \n ", e->port);
    if (e->port >= TOTAL_SWITCH_PORTS) {
        printk("Invalid Switch Port \n");
        return BCM_E_ERROR;
    }

    if (e->type == TYPE_GET) {
        down(&bcm_ethlock_switch_config);
        val16 = extsw_get_pbvlan(e->port); 
        up(&bcm_ethlock_switch_config);
        BCM_ENET_DEBUG("Threshold read = %4x", val16);
        e->fwd_map = val16;
        BCM_ENET_DEBUG("e->fwd_map is = %4x", e->fwd_map);
    } else {
        val16 = (uint32_t)e->fwd_map;
        BCM_ENET_DEBUG("e->fwd_map is = %4x", e->fwd_map);
        down(&bcm_ethlock_switch_config);
        extsw_set_pbvlan(e->port, val16);
        up(&bcm_ethlock_switch_config);
    }

    return 0;
}

int bcmeapi_ioctl_extsw_prio_control(struct ethswctl_data *e)
{
    int ret = 0;
    BCM_ENET_ERROR("No handler! \n");
    return ret;
}

// Default buffer thresholds need to be arrived at and configured at switch init
// calling this function.
int bcmeapi_ioctl_extsw_control(struct ethswctl_data *e)
{
    int ret = 0;
    uint8_t val8 = 0;
    unsigned int val;
    switch (e->sw_ctrl_type) {
        case bcmSwitchBufferControl:

            BCM_ENET_ERROR("No handler! \n");
            break;

        case bcmSwitch8021QControl:
            /* Read the 802.1Q control register */
            extsw_rreg_wrap(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_8021Q, &val8, 1);
            if (e->type == TYPE_GET) {
                val = (val8 >> VLAN_EN_8021Q_S) & VLAN_EN_8021Q_M;
                if (val && ((val8 >> VLAN_IVL_SVL_S) & VLAN_IVL_SVL_M))
                    val = 2; // IVL mode
                e->val  = val;
                BCM_ENET_DEBUG("e->val is = %4x", e->val);
            } else {  // 802.1Q SET
                /* Enable/Disable the 802.1Q */
                if (e->val == 0)
                    val8 &= (~(VLAN_EN_8021Q_M << VLAN_EN_8021Q_S));
                else {
                    val8 |= (VLAN_EN_8021Q_M << VLAN_EN_8021Q_S);
                    if (e->val == 1) // SVL
                        val8 &= (~(VLAN_IVL_SVL_M << VLAN_IVL_SVL_S));
                    else if (e->val == 2) // IVL
                        val8 |= (VLAN_IVL_SVL_M << VLAN_IVL_SVL_S);
                }
                extsw_wreg_wrap(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_8021Q, &val8, 1);
            }
            break;

        default:
            //up(&bcm_ethlock_switch_config);
            ret = -BCM_E_PARAM;
            break;
    } //switch
    return ret;
}
int bcmeapi_ioctl_extsw_config_acb(struct ethswctl_data *e)
{
    return 0;

}

int enet_ioctl_ethsw_dos_ctrl(struct ethswctl_data *e)
{
    if (e->unit != 1)
    {
        return BCM_E_PARAM;
    }
    if (e->type == TYPE_GET)
    {
        if (bcm63xx_enet_isExtSwPresent())
        {
            uint32 v32 = 0;
            uint8 v8 = 0;

            extsw_rreg_wrap(PAGE_DOS_PREVENT_531xx, REG_DOS_CTRL, (uint8 *)&v32, 4);
            /* Taking short-cut : Not following BCM coding guidelines */
            if (v32 & IP_LAN_DROP_EN)  e->dosCtrl.ip_lan_drop_en = 1;
            if (v32 & TCP_BLAT_DROP_EN)  e->dosCtrl.tcp_blat_drop_en = 1;
            if (v32 & UDP_BLAT_DROP_EN)  e->dosCtrl.udp_blat_drop_en = 1;
            if (v32 & TCP_NULL_SCAN_DROP_EN)  e->dosCtrl.tcp_null_scan_drop_en = 1;
            if (v32 & TCP_XMAS_SCAN_DROP_EN)  e->dosCtrl.tcp_xmas_scan_drop_en = 1;
            if (v32 & TCP_SYNFIN_SCAN_DROP_EN)  e->dosCtrl.tcp_synfin_scan_drop_en = 1;
            if (v32 & TCP_SYNERR_SCAN_DROP_EN)  e->dosCtrl.tcp_synerr_drop_en = 1;
            if (v32 & TCP_SHORTHDR_SCAN_DROP_EN)  e->dosCtrl.tcp_shorthdr_drop_en = 1;
            if (v32 & TCP_FRAGERR_SCAN_DROP_EN)  e->dosCtrl.tcp_fragerr_drop_en = 1;
            if (v32 & ICMPv4_FRAG_DROP_EN)  e->dosCtrl.icmpv4_frag_drop_en = 1;
            if (v32 & ICMPv6_FRAG_DROP_EN)  e->dosCtrl.icmpv6_frag_drop_en = 1;
            if (v32 & ICMPv4_LONGPING_DROP_EN)  e->dosCtrl.icmpv4_longping_drop_en = 1;
            if (v32 & ICMPv6_LONGPING_DROP_EN)  e->dosCtrl.icmpv6_longping_drop_en = 1;

            extsw_rreg_wrap(PAGE_DOS_PREVENT_531xx, REG_DOS_DISABLE_LRN, (uint8 *)&v8, 1);
            if (v8 & DOS_DISABLE_LRN) e->dosCtrl.dos_disable_lrn = 1;
        }
        else
        {
            return BCM_E_EXISTS;
        }
    }
    else if (e->type == TYPE_SET)
    {
        if (bcm63xx_enet_isExtSwPresent())
        {
            uint32 v32 = 0;
            uint8 v8 = 0;
            /* Taking short-cut : Not following BCM coding guidelines */
            if (e->dosCtrl.ip_lan_drop_en) v32 |= IP_LAN_DROP_EN;
            if (e->dosCtrl.tcp_blat_drop_en) v32 |= TCP_BLAT_DROP_EN;
            if (e->dosCtrl.udp_blat_drop_en) v32 |= UDP_BLAT_DROP_EN;
            if (e->dosCtrl.tcp_null_scan_drop_en) v32 |= TCP_NULL_SCAN_DROP_EN;
            if (e->dosCtrl.tcp_xmas_scan_drop_en) v32 |= TCP_XMAS_SCAN_DROP_EN;
            if (e->dosCtrl.tcp_synfin_scan_drop_en) v32 |= TCP_SYNFIN_SCAN_DROP_EN;
            if (e->dosCtrl.tcp_synerr_drop_en) v32 |= TCP_SYNERR_SCAN_DROP_EN;
            if (e->dosCtrl.tcp_shorthdr_drop_en) v32 |= TCP_SHORTHDR_SCAN_DROP_EN;
            if (e->dosCtrl.tcp_fragerr_drop_en) v32 |= TCP_FRAGERR_SCAN_DROP_EN;
            if (e->dosCtrl.icmpv4_frag_drop_en) v32 |= ICMPv4_FRAG_DROP_EN;
            if (e->dosCtrl.icmpv6_frag_drop_en) v32 |= ICMPv6_FRAG_DROP_EN;
            if (e->dosCtrl.icmpv4_longping_drop_en) v32 |= ICMPv4_LONGPING_DROP_EN;
            if (e->dosCtrl.icmpv6_longping_drop_en) v32 |= ICMPv6_LONGPING_DROP_EN;

            /* Enable DOS attack blocking functions) */
            extsw_wreg_wrap(PAGE_DOS_PREVENT_531xx, REG_DOS_CTRL, (uint8 *)&v32, 4);
            if (e->dosCtrl.dos_disable_lrn)
            { /* Enable */
                v8 = DOS_DISABLE_LRN;
            }
            else
            {
                v8 = 0;
            }
            extsw_wreg_wrap(PAGE_DOS_PREVENT_531xx, REG_DOS_DISABLE_LRN, (uint8 *)&v8, 1);
        }
        else
        {
            return BCM_E_EXISTS;
        }
    }
    return BCM_E_NONE;
}
void bcmsw_port_mirror_get(int *enable, int *mirror_port, unsigned int *ing_pmap, unsigned int *eg_pmap, unsigned int *blk_no_mrr)
{
    uint16 v16;
    extsw_rreg_wrap(PAGE_MANAGEMENT, REG_MIRROR_CAPTURE_CTRL, &v16, sizeof(v16));
    if (v16 & REG_MIRROR_ENABLE)
    {
        *enable = 1;
        *mirror_port = v16 & REG_CAPTURE_PORT_M;
        *blk_no_mrr = v16 & REG_BLK_NOT_MIRROR;
        extsw_rreg_wrap(PAGE_MANAGEMENT, REG_MIRROR_INGRESS_CTRL, &v16, sizeof(v16));
        *ing_pmap = v16 & REG_INGRESS_MIRROR_M;
        extsw_rreg_wrap(PAGE_MANAGEMENT, REG_MIRROR_EGRESS_CTRL, &v16, sizeof(v16));
        *eg_pmap = v16 & REG_EGRESS_MIRROR_M;
    }
    else
    {
        *enable = 0;
    }
}
void bcmsw_port_trunk_set(unsigned int hash_sel)
{
    uint8 v8;

    extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
    v8 &= ~(TRUNK_HASH_SEL_M<<TRUNK_HASH_SEL_S); /* Clear old hash selection first */
    v8 |= ( (hash_sel & TRUNK_HASH_SEL_M) << TRUNK_HASH_SEL_S ); /* Set Hash Selection */
    extsw_wreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
    printk("LAG/Trunking hash selection changed <0x%01x>\n",v8);
}
void bcmsw_port_trunk_get(int *enable, unsigned int *hash_sel, unsigned int *grp0_pmap, unsigned int *grp1_pmap)
{
    uint16 v16;
    uint8 v8;

    extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_TRUNK_GRP_CTL , &v16, 2);
    *grp0_pmap = (v16 >> TRUNK_EN_GRP_S) & TRUNK_EN_GRP_M ;
    extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_TRUNK_GRP_CTL+2 , &v16, 2);
    *grp1_pmap = (v16 >> TRUNK_EN_GRP_S) & TRUNK_EN_GRP_M ;

    extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
    *enable = (v8 >> TRUNK_EN_LOCAL_S) & TRUNK_EN_LOCAL_M;
    *hash_sel = (v8 >> TRUNK_HASH_SEL_S) & TRUNK_HASH_SEL_M;
}
void bcmsw_port_mirror_set (int enable, int mirror_port, unsigned int ing_pmap, unsigned int eg_pmap, unsigned int blk_no_mrr)
{
    uint16 v16;
    if (enable)
    {
        v16 = REG_MIRROR_ENABLE;
        v16 |= (mirror_port & REG_CAPTURE_PORT_M);
        v16 |= blk_no_mrr?REG_BLK_NOT_MIRROR:0;

        extsw_wreg_wrap(PAGE_MANAGEMENT, REG_MIRROR_CAPTURE_CTRL, &v16, sizeof(v16));
        v16 = ing_pmap & REG_INGRESS_MIRROR_M;
        extsw_wreg_wrap(PAGE_MANAGEMENT, REG_MIRROR_INGRESS_CTRL, &v16, sizeof(v16));
        v16 = eg_pmap & REG_INGRESS_MIRROR_M;
        extsw_wreg_wrap(PAGE_MANAGEMENT, REG_MIRROR_EGRESS_CTRL, &v16, sizeof(v16));
    }
    else
    {
        v16  = 0;
        extsw_wreg_wrap(PAGE_MANAGEMENT, REG_MIRROR_CAPTURE_CTRL, &v16, sizeof(v16));
    }
}

inline static void extsw_reg16_bit_ops(uint16 page, uint16 reg, int bit, int on)
{
    uint16 val16;

    extsw_rreg_wrap(page, reg, &val16, 2);
    val16 &= ~(1 << bit);
    val16 |= on << bit;
    extsw_wreg_wrap(page, reg, &val16, 2);
}

int bcmsw_set_mac_port_state(int unit, int phy_port, int link, int speed, int duplex)
{
    uint8 v8, v8cur;


    if (IsExternalSwitchUnit(unit)) 
    {
        extsw_rreg_wrap(PAGE_CONTROL, REG_PORT_STATE + phy_port, &v8cur, 1);
    } 
    else
    {
        ethsw_rreg(PAGE_CONTROL, REG_PORT_STATE + phy_port, &v8cur, 1);
    }

    if (link && speed)  /* If link is up and speed is known, set all */
    {
        v8 = REG_PORT_STATE_OVERRIDE | REG_PORT_STATE_LNK | (duplex? REG_PORT_STATE_FDX: 0) |
            (speed==1000? REG_PORT_STATE_1000: speed==100? REG_PORT_STATE_100: 0) |
            (v8cur & REG_PORT_GMII_SPEED_UP_2G);
    }
    else    /* If link is down or speed is unknow(0), change the link status only */
    {
        v8 = (v8cur & ~REG_PORT_STATE_RESERVE_0 & ~REG_PORT_STATE_LNK) | 
                REG_PORT_STATE_OVERRIDE | (link? REG_PORT_STATE_LNK: 0);
    }

    down(&bcm_ethlock_switch_config);

    if (IsExternalSwitchUnit(unit)) 
    {
        extsw_wreg_wrap(PAGE_CONTROL, REG_PORT_STATE + phy_port, &v8, 1);
    } 
    else
    {
        ethsw_wreg(PAGE_CONTROL, REG_PORT_STATE + phy_port, &v8, 1);
    }

    up(&bcm_ethlock_switch_config);

    return 0;
}

int bcmsw_mac_rxtx_op(int unit, int phy_port, int get, int *disable)
{
    int rc = 0;
    u8 v8 = 0;

    if (IsExternalSwitchUnit(unit)) 
    {
        if (get)
        {
            extsw_rreg_wrap(PAGE_CONTROL, REG_PORT_CTRL + phy_port, &v8, 1);
            *disable = v8 & REG_PORT_CTRL_DISABLE;
        }
        else /* set */
        {
            extsw_rreg_wrap(PAGE_CONTROL, REG_PORT_CTRL + phy_port, &v8, 1);
            v8 &= ~REG_PORT_CTRL_DISABLE;
            v8 |= (*disable & REG_PORT_CTRL_DISABLE);
            extsw_wreg_wrap(PAGE_CONTROL, REG_PORT_CTRL + phy_port, &v8, 1);
        }
    } 
    else
    {
        if (get)
        {
            ethsw_rreg(PAGE_CONTROL, REG_PORT_CTRL + phy_port, &v8, 1);
            *disable = v8 & REG_PORT_CTRL_DISABLE;
        }
        else
        {
            ethsw_rreg(PAGE_CONTROL, REG_PORT_CTRL + phy_port, &v8, 1);
            v8 &= ~REG_PORT_CTRL_DISABLE;
            v8 |= (*disable & REG_PORT_CTRL_DISABLE);
            ethsw_wreg(PAGE_CONTROL, REG_PORT_CTRL + phy_port, &v8, 1);
        }
    }

    return rc;
}

void bcmsw_enable_all_macs_rxtx(int enable)
{
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);
    int lgp, lgpMap, portMap = pDevCtrl->allPortMap;

    for (lgp = 0; lgp < MAX_TOTAL_SWITCH_PORTS && portMap; lgp++)
    {
        lgpMap = 1 << lgp;
        if ((lgpMap & portMap) == 0) continue;
        portMap &= ~(lgpMap);

        /* Skip enabling WAN Only Port that has not be configured as WAN port */
        if (enable && (pDevCtrl->wanOnlyPorts & ~pDevCtrl->wanPort & lgpMap) != 0) continue;

        bcmsw_enable_mac_rxtx_log(lgp, enable);
    }
}

int bcmeapi_ioctl_ethsw_port_traffic_control(struct ethswctl_data *e)
{
    bcmsw_mac_rxtx_op(e->unit, e->port, e->type==TYPE_GET, 
        e->type==TYPE_GET?&e->ret_val:&e->val);
    if (e->type == TYPE_SET && !(e->val & REG_PORT_CTRL_DISABLE))
    {
        PHY_STAT ps = {0};

        /* update link status bit in port state override sw register */
        ps = ethsw_phyid_stat(enet_sw_port_to_phyid(e->unit, e->port));
        if(ps.lnk) ethsw_set_mac_hw(PHYSICAL_PORT_TO_LOGICAL_PORT(e->unit, e->port), ps);
    }

    return BCM_E_NONE;
}


int bcmeapi_ioctl_extsw_port_storm_ctrl(struct ethswctl_data *e)
{
    BCM_ENET_ERROR("No handler! \n");
    return 0;

}

static int read_vlan_table_ext(bcm_vlan_t vid, uint32_t *val32)
{
    uint8_t val8;
    int i, timeout = 200;

    extsw_wreg_wrap(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_INDX_531xx, (uint8_t *)&vid, 2);
    val8 = 0x81;
    extsw_wreg_wrap(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL_531xx, (uint8_t *)&val8, 1);

    for (i = 0; i < timeout; i++) {
        extsw_rreg_wrap(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL_531xx, (uint8_t *)&val8, 1);
        if (((val8) & 0x80) == 0) {
            extsw_rreg_wrap(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_ENTRY_531xx,
                       (uint8_t *)val32, 4);
            return 0;
        }
        udelay(100);
    }

    printk("Timeout reading VLAN table\n");
    return BCM_E_ERROR;
}

static int write_vlan_table_ext(bcm_vlan_t vid, uint32_t val32)
{
    uint8_t val8;
    int i, timeout = 200;

    extsw_wreg_wrap(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_INDX_531xx, (uint8_t *)&vid, 2);
    extsw_wreg_wrap(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_ENTRY_531xx, (uint8_t *)&val32, 4);
    val8 = 0x80;
    extsw_wreg_wrap(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL_531xx, (uint8_t *)&val8, 1);

    for (i = 0; i < timeout; i++) {
        extsw_rreg_wrap(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL_531xx, (uint8_t *)&val8, 1);
        if (((val8) & 0x80) == 0) {
            return 0;
        }
        udelay(100);
    }

    printk("Timeout writing to VLAN table\n");
    return BCM_E_ERROR;
}

int bcmeapi_ioctl_extsw_vlan(struct ethswctl_data *e)
{
    bcm_vlan_t vid;
    uint32_t val32, tmp;

    down(&bcm_ethlock_switch_config);
    if (e->type == TYPE_GET) {
        vid = e->vid & BCM_NET_VLAN_VID_M;
        if (read_vlan_table_ext(vid, &val32)) {
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
        if (write_vlan_table_ext(vid, val32)) {
            up(&bcm_ethlock_switch_config);
            return BCM_E_ERROR;
        }
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}
