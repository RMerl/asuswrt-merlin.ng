/*
   <:copyright-BRCM:2015:DUAL/GPL:standard

      Copyright (c) 2015 Broadcom
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

/*
 *  Created on: Dec/2016
 *      Author: steven.hsieh@broadcom.com
 */

#include <bcmnet.h>
#include "phy_drv_sf2.h"
#include "crossbar_dev.h"
#include <bcm/bcmswapitypes.h>

#include <linux/bcm_skb_defines.h>
#include <linux/rtnetlink.h>
#include <linux/spinlock.h>

#include "linux/bcm_log.h"

#ifdef PKTC
#include <osl.h>
#endif

#include "port.h"
#include "enet.h"
#include "mux_index.h"
#include "sf2.h"
#include "sf2_common.h"
#include "pmc_switch.h"


#include "bcmenet_common.h"

#undef OFFSETOF
#define OFFSETOF(STYPE, MEMBER)     ((size_t) &((STYPE *)0)->MEMBER)

#include "bcm_map_part.h"
#include "boardparms.h"
#include "bcmmii.h"
#include "bcmmii_xtn.h"


#define DATA_TYPE_HOST_ENDIAN   (0x00<<24)
#define DATA_TYPE_BYTE_STRING   (0x01<<24)
#define DATA_TYPE_VID_MAC       (0x02<<24)

// =========== global/static variables ====================
struct semaphore bcm_link_handler_config;

enetx_port_t *sf2_sw;   /* external SF2 switch */

#define SF2SW_RREG      extsw_rreg_wrap
#define SF2SW_WREG      extsw_wreg_wrap
extern spinlock_t       sf2_reg_config;

#include "sf2_platform.h"

#if defined(CONFIG_NET_SWITCHDEV)
#define SW_SET_HW_FWD(s)      (s)->n.flags |= PORT_FLAG_HW_FWD
#define SW_CLR_HW_FWD(s)      (s)->n.flags &= ~PORT_FLAG_HW_FWD
#define SW_IS_HW_FWD(s)       ((s)->n.flags & PORT_FLAG_HW_FWD)
#else
static uint8_t  sf2_hw_switching_state = HW_SWITCHING_ENABLED;
#define SW_SET_HW_FWD(s)      sf2_hw_switching_state = HW_SWITCHING_ENABLED
#define SW_CLR_HW_FWD(s)      sf2_hw_switching_state = HW_SWITCHING_DISABLED
#define SW_IS_HW_FWD(s)       sf2_hw_switching_state
#endif

/* ETHSWPRIOCONTROL, ETHSWQUEMAP related variables */
static int sf2LanUpPorts_g, sf2WanUpPorts_g;

#if defined(CONFIG_BCM947622)
#define DefaultWANQueBitMap 0xff
#define DefaultQueNoRemap 0x76543210 /* No remapping constant */
static uint32_t wanQueMap = DefaultWANQueBitMap;
static uint32_t queRemap = DefaultQueNoRemap;
#else
#define DefaultWANQueBitMap 0xaa
#define DefaultQueRemap 0x77553311 /* Default Map CPU Traffic from Queue 0 to 1 to get basic WAN gurantee */
static uint32_t wanQueMap = DefaultWANQueBitMap;
static uint32_t queRemap = DefaultQueRemap;
#endif

#define MaxStreamNumber 40                                              // MaxStreamNumber
static uint32_t maxStreamNumber = MaxStreamNumber, curStreams, queThreConfMode;
#if !defined(SF2_EXTERNAL)
static uint16_t sf2_sw_port_thred[FC_THRED_TOTAL_TYPES][FC_LAN_TXQ_QUEUES];     // sf2_sw_port_thred
static uint16_t sf2_imp0_thred[FC_THRED_TOTAL_TYPES][FC_LAN_TXQ_QUEUES];        // sf2_imp0_thred
static uint16_t sf2_wan_imp1_thred[FC_THRED_TOTAL_TYPES][FC_LAN_TXQ_QUEUES];    // sf2_wan_imp1_thred
#endif

/* 
    ACTIVE_QUEUE_NUMBER_PER_PORT defines actuall actively used 
    queue number in run time for threshold calculation 
*/
#define ACTIVE_QUEUE_NUMBER_PER_PORT (2)
static int active_queue_number_per_port = ACTIVE_QUEUE_NUMBER_PER_PORT;


/* Deep Green Mode enabled flag currently tied to the WebGUI's "Advanced Setup -> Power Management -> Ethernet Auto Power Down & Sleep" checkbox */
#if defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE)
static int deep_green_mode_enabled = 1;      //Keep track of whether Deep Green Mode feature is enabled/disabled
static int deep_green_mode_activated = 0;    //Keep track of whether Deep Green Mode feature is activated/deactivated (DGM is activated when the feature is enabled and all ports are disconnected)

static uint32 reg_low_power_exp1;  /* Store REG_LOW_POWER_EXP1 register value before enabling Deep Green Mode so that we can restore value when disabling Deep Green Mode */
#endif

// ----------- static SF2 functions -----------------------
static inline void dumpHexData1(uint8_t *pHead, uint32_t len)
{
    uint32_t i;
    uint8_t *c = pHead;
    for (i = 0; i < len; ++i) {
        if (i % 16 == 0)
            printk("\n");
        printk("0x%02X, ", *c++);
    }
    printk("\n");
}

static void _fast_age_start_done_ext(uint8_t ctrl)
{
    uint8_t timeout = 100;

    SF2SW_WREG(PAGE_CONTROL, REG_FAST_AGING_CTRL, &ctrl, 1);
    SF2SW_RREG(PAGE_CONTROL, REG_FAST_AGING_CTRL, &ctrl, 1);
    while (ctrl & FAST_AGE_START_DONE) {
        mdelay(1);
        SF2SW_RREG(PAGE_CONTROL, REG_FAST_AGING_CTRL, &ctrl, 1);
        if (!timeout--) {
            printk("Timeout of fast aging \n");
            break;
        }
    }

    /* Restore DYNAMIC bit for normal aging */
    ctrl = FAST_AGE_DYNAMIC;
    SF2SW_WREG(PAGE_CONTROL, REG_FAST_AGING_CTRL, &ctrl, 1);
}

static int _ethsw_phy_access(struct ethswctl_data *e, enetx_port_t *port)
{
    // based on shared\bcmswaccess.c:bcmsw_phy_access()
    uint16 phy_reg_val;
    uint32_t data = 0;
    int reg_offset;
    phy_dev_t *phy = get_active_phy(port->p.phy);

    reg_offset = (e->offset >> PHY_REG_S) & PHY_REG_M;

    if (e->type == TYPE_GET) {
        down(&sf2_sw->s.conf_sem);
        if (IsC45Phy(phy))
            phy_bus_c45_read32(phy, reg_offset, &phy_reg_val);
        else
            phy_bus_read(phy, reg_offset, &phy_reg_val);
        data = phy_reg_val;
        up(&sf2_sw->s.conf_sem);
        data = __le32_to_cpu(data);
        e->length = 4;
        memcpy((void*)(&e->data), (void*)&data, e->length);
    } else {
        memcpy((void *)&phy_reg_val, (void *)e->data, 2);
        phy_reg_val = __cpu_to_le16(phy_reg_val);
        down(&sf2_sw->s.conf_sem);
        if (IsC45Phy(phy))
            phy_bus_c45_write32(phy, reg_offset, phy_reg_val);
        else
            phy_bus_write(phy, reg_offset, phy_reg_val);
        up(&sf2_sw->s.conf_sem);
    }

    return 0;
}

static int _alignment_check(struct ethswctl_data *e)
{
    static u16 misalign[][2] = {{0x581, 2}, {0x583, 4}, {0x4001, 4}, {0x4005, 2}};
    int i;

    for(i=0; i < ARRAY_SIZE(misalign); i++)
    {
        if (e->offset == misalign[i][0] && e->length == misalign[i][1]) return 1;
    }

    if (((e->offset % 2) && (e->length == 2 || e->length == 6)) ||
            ((e->offset % 4) && (e->length == 4 || e->length == 8))) return 0;

    return 1;
}

#if !defined(SF2_EXTERNAL)
static int _get_next_queue(int cur_que, int for_wan, int reverse)
{
    int q;

    if(reverse == 0)
    {
        q = 0;
        if (cur_que != -1) q = cur_que + 1;

        for (; q < FC_LAN_TXQ_QUEUES; q++)
        {
            if (for_wan)
            {
                if (wanQueMap & (1<<q)) return q;
            }
            else
            {
                if ((wanQueMap & (1<<q)) == 0) return q;
            }
        }
    }
    else
    {
        q = FC_LAN_TXQ_QUEUES - 1;
        if (cur_que != -1) q = cur_que - 1;
        for (; q >= 0; q--)
        {
            if (for_wan)
            {
                if (wanQueMap & (1<<q)) return q;
            }
            else
            {
                if ((wanQueMap & (1<<q)) == 0) return q;
            }
        }
    }
    return -1;
}
#define GET_NEXT_WAN_QUE(CUR_QUE) _get_next_queue(CUR_QUE, 1, 0)
#define GET_NEXT_LAN_QUE(CUR_QUE) _get_next_queue(CUR_QUE, 0, 0)
#define GET_PREV_WAN_QUE(CUR_QUE) _get_next_queue(CUR_QUE, 1, 1)
#define GET_PREV_LAN_QUE(CUR_QUE) _get_next_queue(CUR_QUE, 0, 1)

static void _sf2_conf_thred_2reg(int page, int reg, uint16_t tbl[][FC_LAN_TXQ_QUEUES])
{
    int t, q;

    for (t = 0; t < FC_THRED_TOTAL_TYPES; t++)
    {
        for (q = 0; q < FC_LAN_TXQ_QUEUES; q++)
        {
            SF2SW_WREG(page, reg + t*0x10 + q*2, &tbl[t][q], sizeof(tbl[0][0]));
        }
    }
}
#endif //!SF2_EXTERNAL

int speed_macro_2_mbps(phy_speed_t spd)
{
    static int speed[PHY_SPEED_10000 + 1] =
    {
        [PHY_SPEED_10] = 10,
        [PHY_SPEED_100] = 100,
        [PHY_SPEED_1000] = 1000,
        [PHY_SPEED_2500] = 2500,
        [PHY_SPEED_5000] = 5000,
        [PHY_SPEED_10000] = 10000,
    };

    if (spd >= ARRAY_SIZE(speed)) return 0;
    return speed[spd];
}

static void _port_sf2_print_status(enetx_port_t *p, int speed_change)
{
    phy_dev_t *phy = get_active_phy(p->p.phy);
    if (phy->link)
    {
        printk((KERN_CRIT "%s (%s switch port: %d) (Logical Port: %d) (phyId: %x) Link %s %d mbps %s duplex\n"),
                p->dev->name, PORT_ON_ROOT_SW(p)?"Int":"Ext", p->p.mac->mac_id, 
                PHYSICAL_PORT_TO_LOGICAL_PORT(p->p.mac->mac_id, PORT_ON_ROOT_SW(p)?0:1), phy->addr,
                speed_change? "Speed/Duplex changed to": "Up at",
                speed_macro_2_mbps(phy->speed), (phy->duplex==PHY_DUPLEX_FULL)?"full":"half");
    }
    else
    {
        printk((KERN_CRIT "%s (%s switch port: %d) (Logical Port: %d) (phyId: %x) Link DOWN.\n"),
                p->dev->name, PORT_ON_ROOT_SW(p)?"Int":"Ext", p->p.mac->mac_id, 
                    PHYSICAL_PORT_TO_LOGICAL_PORT(p->p.mac->mac_id, PORT_ON_ROOT_SW(p)?0:1), phy->addr);
    }
}

void port_sf2_print_status(enetx_port_t *p)
{
    _port_sf2_print_status(p, 0);
}

static void _sf2_conf_que_thred(void);
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
static void _extsw_set_port_imp_map_2_5g(void);
static void _extsw_set_port_imp_map_non_2_5g(void);
#endif

void _sf2_set_mac_eee_by_phy(enetx_port_t *p)
{
    mac_dev_t *mac_dev = p->p.mac;
    phy_dev_t *phy_dev = get_active_phy(p->p.phy);
    phy_dev_t *end_phy = cascade_phy_get_last(phy_dev);

    int enabled = 0;

    if (end_phy->link)
    {
        msleep(1000);
        phy_dev_eee_resolution_get(end_phy, &enabled);
    }

    mac_dev_eee_set(mac_dev, enabled);
}

extern u8 eth_internal_pause_addr[];
/*
 * handle_link_status_change
 */
void link_change_handler(enetx_port_t *port, int linkstatus, int speed, int duplex)
{
    phy_dev_t *phy_dev = get_active_phy(port->p.phy);
    phy_dev_t *end_phy = cascade_phy_get_last(phy_dev);
    mac_dev_t *mac_dev = port->p.mac;
    mac_cfg_t mac_cfg = {};
    int i, old_link;
    phy_duplex_t old_duplex;
    phy_speed_t old_speed;
#if defined(CONFIG_BCM_ETH_PWRSAVE)
    int phyId = 0; // 0 is a valid phy_id but not an external phy_id. So we are OK initializing it to 0.
#endif
    static char *color[] ={"Brown", "Blue", "Green", "Orange"};
    static char *results[] = {"Invalid", "Good", "Open", "Intra Pair Short", "Inter Pair Short"};

    if (!phy_dev)
        return;

    mac_dev_disable(mac_dev);

#if defined(CONFIG_BCM_ETH_PWRSAVE)
    if (!PORT_ON_EXTERNAL_SW(port))
    {
        phyId = phy_dev->meta_id;
    }
#endif

#if defined(ARCHER_DEVICE)
    {
        bcmFun_t *enet_phy_speed_set = bcmFun_get(BCM_FUN_ID_ENET_PHY_SPEED_SET);
        bcmSysport_PhySpeed_t info;

        if (enet_phy_speed_set && linkstatus)
        {
            info.dev = port->dev;
            info.kbps = speed*1000;
            enet_phy_speed_set(&info);
        }
    }
#endif // ARCHER_DEVICE

    down(&bcm_link_handler_config);

    old_link = netif_carrier_ok(port->dev);
    old_speed = phy_dev->speed;
    old_duplex = phy_dev->duplex;
    phy_dev->link = linkstatus;
    if (linkstatus) {

#if defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE)
        port_sf2_deep_green_mode_handler();
#endif
        // bcmeapi_link_check(port, linkstatus, speed);
        if (speed == 10000)
        {
            mac_cfg.speed = MAC_SPEED_10000;
            phy_dev->speed = PHY_SPEED_10000;
        }
        else if (speed == 2500)
        {
            mac_cfg.speed = MAC_SPEED_2500;
            phy_dev->speed = PHY_SPEED_2500;
        }
        else if (speed == 1000)
        {
            mac_cfg.speed = MAC_SPEED_1000;
            phy_dev->speed = PHY_SPEED_1000;
        }
        else if (speed == 100)
        {
            mac_cfg.speed = MAC_SPEED_100;
            phy_dev->speed = PHY_SPEED_100;
        }
        else if (speed == 200)
        {
        }
        else
        {
            phy_dev->speed = PHY_SPEED_10;
            mac_cfg.speed = MAC_SPEED_10;
        }

        mac_cfg.duplex = phy_dev->duplex = duplex? PHY_DUPLEX_FULL: PHY_DUPLEX_HALF;
        mac_dev_cfg_set(mac_dev, &mac_cfg);
        mac_dev_pause_set(mac_dev, phy_dev->pause_rx, phy_dev->pause_tx, port->dev ? port->dev->dev_addr : eth_internal_pause_addr);

#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
        if (speed == 2500)
        {
            _extsw_set_port_imp_map_2_5g();
        }
#endif

        if (!old_link && phy_dev_cable_diag_is_supported(end_phy) && phy_dev_cable_diag_is_enabled(end_phy))
        {
            int result, pair_len[4];
            phy_dev_cable_diag_run(end_phy, &result, pair_len);
            if (result == CD_INVALID)
                printk("Cable Diagnosis Not Successful - Skipped\n");
            else {
                printk("Connected Cable Length: %d.%d meter\n", pair_len[0]/100, pair_len[0]%100);
                if (result != CD_ALL_PAIR_OK) {
                    for (i=0; i<4; i++)
                        if (CD_CODE_PAIR_GET(result, i) != CD_OK)
                            printk("    Pair %s: %s;", color[i], results[CD_CODE_PAIR_GET(result, i)]);
                    printk("\n");
                }
            }

            if (!phy_dev->link) /* If Cable Diag Causes Link Down, skip this round operation */
                goto end;
        }

        /* notify linux after we have finished setting our internal state */
        if (!old_link || old_speed != phy_dev->speed || old_duplex != phy_dev->duplex)
        {
            if (!old_link && port->p.parent_sw == sf2_sw) 
            {
                if(PORT_ROLE_IS_WAN(port))
                    sf2WanUpPorts_g++;
                else
                    sf2LanUpPorts_g++;
                _sf2_conf_que_thred();
            }

            if (netif_carrier_ok(port->dev) == 0) {
                char link[16];
                char *env[] = {link, NULL};
                snprintf(link, sizeof(link), "LINK=up");
                kobject_uevent_env(&port->dev->dev.kobj, KOBJ_CHANGE, env);
                netif_carrier_on(port->dev);
            }

            _port_sf2_print_status(port, old_link == phy_dev->link);
        }

        mac_dev_enable(mac_dev);

    } else {
        /* also flush ARL for link down port */
        if (port->p.ops->fast_age)
            port->p.ops->fast_age(port);

#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
        if (speed == 2500)
        {
            _extsw_set_port_imp_map_non_2_5g();
        }
#endif /* CONFIG_BCM_ENET_MULTI_IMP_SUPPORT */

#if defined(CONFIG_BCM_GMAC)
        if (IsGmacPort( sw_port ) )
        {
            volatile GmacEEE_t *gmacEEEp = GMAC_EEE;
            gmacEEEp->eeeCtrl.linkUp = 0;
            gmacEEEp->eeeCtrl.enable = 0;
        }
#endif
        // ethsw_eee_port_enable(sw_port, 0, 0);
        /* notify linux after we have finished setting our internal state */
        if (netif_carrier_ok(port->dev) != 0)
        {
            char link[16];
            char *env[] = {link, NULL};
            snprintf(link, sizeof(link), "LINK=down");

            if (port->p.parent_sw == sf2_sw) 
            {
                if (PORT_ROLE_IS_WAN(port))
                    sf2WanUpPorts_g--;
                else
                    sf2LanUpPorts_g--;
                _sf2_conf_que_thred();
            }
            kobject_uevent_env(&port->dev->dev.kobj, KOBJ_CHANGE, env);
            netif_carrier_off(port->dev);
            port_sf2_print_status(port);
        }

        mac_dev_cfg_set(mac_dev, &mac_cfg);

        if (phy_dev_cable_diag_is_supported(end_phy) && phy_dev_cable_diag_is_enabled(end_phy))
        {
            int result, pair_len[4];
            phy_dev_cable_diag_run(end_phy, &result, pair_len);
            if (result == CD_INVALID)
                printk("Cable Dignosis Not successful - Skipped.\n");
            else
            {
                printk("PHY(address %d) Links Down due to: ", end_phy->addr);
                switch(result)
                {
                    case CD_ALL_PAIR_OK:
                        printk("Port on other end powered off; Cable length: %d.%d meter.\n",
                                pair_len[0]/100, pair_len[0]%100);
                        break;
                    case CD_ALL_PAIR_OPEN:
                        if ((pair_len[0] + pair_len[1] + pair_len[2] + pair_len[3]) == 0)
                        {
                            printk("Cable is Unplugged on local port.\n");
                        }
                        else if (pair_len[0] == pair_len[1] && pair_len[0] == pair_len[2] && pair_len[0] == pair_len[3])
                        {
                            printk("Cable is Unplugged on remote port with length: %d.%d meter.\n",
                                    pair_len[0]/100, pair_len[0]%100);
                        }
                        else 
                        {
                            printk("Cable Open at Pair Br:%d.%d Bl:%d.%d Gr:%d.%d Or%d.%d meters\n",
                                pair_len[0]/100, pair_len[0]%100, pair_len[1]/100, pair_len[1]%100, 
                                pair_len[2]/100, pair_len[2]%100, pair_len[3]/100, pair_len[3]%100);
                        }
                        break;
                    default:
                        printk("\n");
                        for(i=0; i<4; i++)
                        {
                            if (CD_CODE_PAIR_GET(result, i) == CD_INVALID)
                            {
                                printk("    Pair %s: Cable Diagnosis Failed - Skipped\n", color[i]);
                                continue;
                            }
                                
                            printk("    Pair %s: Cable is %s %s %d.%d meters\n", color[i], 
                                results[CD_CODE_PAIR_GET(result, i)],
                                CD_CODE_PAIR_GET(result, i)==CD_OK? "with": "at",
                                pair_len[i]/100, pair_len[i]%100);
                        }
                        break;
                }
            }
        }
    }

end:
    /* update EEE settings based on link status */
    enetx_queue_work(port, _sf2_set_mac_eee_by_phy);

    up(&bcm_link_handler_config);
}

static int tr_port_for_imp_cnt(enetx_port_t *port, void *ctx)
{
    int *count = ctx;

    if (port->p.port_cap == PORT_CAP_MGMT)
        (*count)++;
    return 0;
}

static int tr_port_for_nonimp_cnt(enetx_port_t *port, void *ctx)
{
    int *count = ctx;

    if (port->has_interface && port->p.port_cap != PORT_CAP_MGMT)
        (*count)++;
    return 0;
}

static int sf2_get_imp_port_cnt(void)
{
    int cnt = 0;
    port_traverse_ports(sf2_sw, tr_port_for_imp_cnt, PORT_CLASS_PORT, &cnt);
    return cnt;
}

static int sf2_get_nonimp_port_cnt(void)
{
    int cnt = 0;
    port_traverse_ports(sf2_sw, tr_port_for_nonimp_cnt, PORT_CLASS_PORT, &cnt);
    return cnt;
}

int inline sf2_imp_port_cnt (void)
{
    static int imp_port_cnt;

    if (imp_port_cnt == 0)
        imp_port_cnt = sf2_get_imp_port_cnt();
    return imp_port_cnt;
}

int inline sf2_nonimp_port_cnt (void)
{
    static int nonimp_port_cnt;

    if (nonimp_port_cnt == 0)
        nonimp_port_cnt = sf2_get_nonimp_port_cnt(); 
    return nonimp_port_cnt;
}

#if defined(SF2_EXTERNAL)
static void _sf2_conf_que_thred(void)
{
    static int sf2_fc_configured = 0;
    uint16_t val;
    int page, offset;
    int p, t, q;
    
    if (sf2_fc_configured) return;

    SF2SW_RREG(PAGE_MANAGEMENT, REG_DEV_ID, &val, sizeof(val));

    if (val==DEV_ID_BCM53134_A0 || val==DEV_ID_BCM53134_B0_B1)
    {
        for (p =0; p < 3; p++)
        {
            page = (p==0)?PAGE_FC_LAN_TXQ:(p==1)?PAGE_FC_IMP0_TXQ:PAGE_FC_IMP1_TXQ;
            for (t = 0; t < FC_THRED_TOTAL_TYPES; t++)
            {
                for (q = 0; q < FC_LAN_TXQ_QUEUES; q++)
                {
                    offset = REG_FC_LAN_TXQ_THD_RSV_QN0 + t*0x10 + q*2;
                    // override chip default with calculated nums from Predrag
                    switch (t) {
                    case FC_THRED_QUE_RSRVD_TYPE:   val = 18;       break;
                    case FC_THRED_QUE_HYSTR_TYPE:   val = 54 + 4*q; break;
                    case FC_THRED_QUE_PAUSE_TYPE:   val = 108+ 8*q; break;
                    case FC_THRED_QUE_DROP_TYPE:    val = 498;      break;
                    case FC_THRED_TTL_HYSTR_TYPE:   val = 179+ 4*q; break;
                    case FC_THRED_TTL_PAUSE_TYPE:   val = 357+ 8*q; break;
                    case FC_THRED_TTL_DROP_TYPE:    val = 498;      break;
                    }
                    SF2SW_WREG(page, offset, &val, sizeof(val));
                }
            }
        }
    }

    sf2_fc_configured = 1;
}
#else //!SF2_EXTERNAL
static uint32_t acb_xoff_threshold;
static int acb_port_xoff_threshold;

#define TTL_DROP_MAX_FACTOR  53/100     /* Max Percentage of Drop Thred to total buffer */
#define PAUSE_DELAY_BUF_IN_PAGE  33     /* IEEE requested Pause buffering */
#define TTL_PAUSE_MAX_FACTOR  10/100    /* Max Total Pause thread reduction from Drop Total Pause Thread in percentage of total buffer */
#define PORT_PAUSE_MAX_FACTOR 3/100    /* Max Port Pause Thread reduction from Port Pause Thread in percentage of total buffer */
#define TXQ_RSRVD_MAX_FACTOR  5/100     /* Max TXQ Reserved Thread percentage to total buffer */

static void _sf2_conf_que_thred(void)
{
    int q, q1, t;
    int thredBase[FC_THRED_TOTAL_TYPES],
        maxFrameLength = 4096, lastLanQue, lastWanQue, wanUpPorts, lanUpPorts;
    int available_switch_buf = SF2_MAX_BUFFER_IN_PAGE;
    int impPortCnt;
    int acb_xoff = acb_port_xoff_threshold > acb_xoff_threshold? acb_port_xoff_threshold: acb_xoff_threshold;

    /*
       The percentage of various factors for different condition
       from base computation result. Don't use parantheses to avoid underflow.
       The design goal is:
       o WAN queues have higher thredsholds than all LAN queues.
       o Higher queues have higher thresholds than lower queues in the same group.
     */

    /* Define IMP Port over LAN port threshold factor */
#define IMP_OVER_WAN_QUE_FACTOR 100/100

    /*
       Define Threshold increment factor with higher priority queue to
       guarantee higher priority queue get slight more chance of resource.
       The value should not caused WAN/LAN threshold reserved when scaled up.
     */
#define LOW_QUE_OVER_HIGH_QUE_FACTOR 100/101

    /* Define WAN Queue over LAN queu factor so that WAN queue gets higher resource chance */
#define LAN_QUE_OVER_WAN_QUE_FACTOR 100/105

    lanUpPorts = sf2LanUpPorts_g;
    wanUpPorts = sf2WanUpPorts_g;

    impPortCnt = sf2_imp_port_cnt();
    if (impPortCnt > lanUpPorts)
        impPortCnt = lanUpPorts;

    /* curStreams is used to calculat Total Pause(Not Drop) to
       reserve head room in buffer to guarantee the minimum buffer by queue reserved threshold
       when stream number is below this number. Term "stream" means traffic to ONE queue. */
    if (queThreConfMode == ThreModeDynamic)
    {
        curStreams = (lanUpPorts + wanUpPorts + impPortCnt) * active_queue_number_per_port;
    }
    else
    {
        curStreams = (sf2_nonimp_port_cnt() + sf2_imp_port_cnt()) * active_queue_number_per_port;
    }

    /* Set a system level cap of max stream number to matching real world */
    if (curStreams > maxStreamNumber)
    {
        curStreams = maxStreamNumber;
    }

    /* Unify all computation to page unit */
    maxFrameLength /= SF2_BYTES_PER_PAGE;

    /* Set Reserved Threshold to two frame size to create hard guarantee for each queue */
    thredBase[FC_THRED_QUE_RSRVD_TYPE] = maxFrameLength * 2;
    if (thredBase[FC_THRED_QUE_RSRVD_TYPE] > SF2_MAX_BUFFER_IN_PAGE * TXQ_RSRVD_MAX_FACTOR)
        thredBase[FC_THRED_QUE_RSRVD_TYPE] = SF2_MAX_BUFFER_IN_PAGE * TXQ_RSRVD_MAX_FACTOR;
        

    /*
       Total Drop Threshold:
       When total queue length exceeds Total Drop, all packet will be dropped even
       for queues under Reserved Threshold. This can only happen when external device
       ignores Pause frame. As the values for protocol violation case or misconfiguration case,
       the value is designed as high as possible to minimum the impact of overrun above
       the total Pause and as final guard to total buffer.  Thus set to to one packet size room
       below hard limits.
     */
    available_switch_buf -= acb_xoff * (lanUpPorts + wanUpPorts);
    //thredBase[FC_THRED_TTL_DROP_TYPE] = available_switch_buf - maxFrameLength;
    thredBase[FC_THRED_TTL_DROP_TYPE] = available_switch_buf;

    if ( thredBase[FC_THRED_TTL_DROP_TYPE] < SF2_MAX_BUFFER_IN_PAGE * TTL_DROP_MAX_FACTOR)
        thredBase[FC_THRED_TTL_DROP_TYPE] = SF2_MAX_BUFFER_IN_PAGE * TTL_DROP_MAX_FACTOR;


    /*
       Compute Total Pause Threshold:
       Need to guarantee hardware reserved threshold in EACH queuue.
       The value depends on simultaneous buffer requesting streams and frame length.
       The value is set to guarantee no drop
     */
    thredBase[FC_THRED_TTL_PAUSE_TYPE] = thredBase[FC_THRED_TTL_DROP_TYPE] - PAUSE_DELAY_BUF_IN_PAGE * curStreams;
    if ((thredBase[FC_THRED_TTL_DROP_TYPE] - thredBase[FC_THRED_TTL_PAUSE_TYPE]) >
        (SF2_MAX_BUFFER_IN_PAGE * TTL_PAUSE_MAX_FACTOR))
        thredBase[FC_THRED_TTL_PAUSE_TYPE] = thredBase[FC_THRED_TTL_DROP_TYPE] -
            (SF2_MAX_BUFFER_IN_PAGE * TTL_PAUSE_MAX_FACTOR);

    /*
       Total Hysteresis:
       Hysteresis will reflect the hop count from this device to the source and
       the latency of each hop's resume operation. Set this as high as possible
       related to PAUSE, set it to half of TTL_PAUSE
     */
    thredBase[FC_THRED_TTL_HYSTR_TYPE] = thredBase[FC_THRED_TTL_PAUSE_TYPE]/2;
    if (thredBase[FC_THRED_TTL_HYSTR_TYPE] < thredBase[FC_THRED_QUE_RSRVD_TYPE])
        thredBase[FC_THRED_TTL_HYSTR_TYPE] = thredBase[FC_THRED_QUE_RSRVD_TYPE] + maxFrameLength/2;

    /* Set per-queue drop threshold to 1/3 of total drop threshold */
    thredBase[FC_THRED_QUE_DROP_TYPE] = thredBase[FC_THRED_TTL_DROP_TYPE]/3;

    /* Set per-queue pause threshould one frame size lower than per-q drop threshold */
    thredBase[FC_THRED_QUE_PAUSE_TYPE] = thredBase[FC_THRED_QUE_DROP_TYPE] - PAUSE_DELAY_BUF_IN_PAGE;
    if ((thredBase[FC_THRED_QUE_DROP_TYPE] - thredBase[FC_THRED_QUE_PAUSE_TYPE]) >
        SF2_MAX_BUFFER_IN_PAGE * PORT_PAUSE_MAX_FACTOR)
    thredBase[FC_THRED_QUE_PAUSE_TYPE] = thredBase[FC_THRED_QUE_DROP_TYPE] - SF2_MAX_BUFFER_IN_PAGE * PORT_PAUSE_MAX_FACTOR;

    /* Set per queue's Hysteresis to be helf of per-q pause threshold */
    thredBase[FC_THRED_QUE_HYSTR_TYPE] = thredBase[FC_THRED_QUE_PAUSE_TYPE]/2;

    /* Compute base WAN queue threashold */
    lastWanQue = GET_PREV_WAN_QUE(-1);

    /* Now Scale WAN Queue (ACB Queue) Thredshold up */
    /* ACB queue should never exceed Hardware Reserved Thread because ACB is monitoring the queuing */
    q1 = lastWanQue;
    for (q = q1; q != -1; q1 = q, q = GET_PREV_WAN_QUE(q))
    {
        for (t = FC_THRED_TOTAL_TYPES - 1; t >= 0; t--)
        {
            switch (t)
            {
                case FC_THRED_QUE_RSRVD_TYPE:
                    /* Set reserve threshold to be the same */
                    sf2_sw_port_thred[t][q] = thredBase[FC_THRED_QUE_RSRVD_TYPE];
                    break;
                default:
                    /* Set all rest thredsholds to 2 frames lower than maximum buffer. */
                    sf2_sw_port_thred[t][q] = SF2_MAX_BUFFER_IN_PAGE - 2 * maxFrameLength;
            }
        }
    }

    /* Now Scale LAN Queue Thredshold down */
    lastLanQue = GET_PREV_LAN_QUE(-1);
    q1 = lastLanQue;
    for (q = q1; q != -1; q1 = q, q = GET_PREV_LAN_QUE(q1))
    {
        for (t = FC_THRED_TOTAL_TYPES - 1; t >= 0; t--)
        {
            switch (t)
            {
                case FC_THRED_QUE_RSRVD_TYPE:
                    /* Set reserve threshold to be the same */
                    sf2_sw_port_thred[t][q] = thredBase[FC_THRED_QUE_RSRVD_TYPE];
                    break;
                default:
                    if (q == lastLanQue)
                    {
                        sf2_sw_port_thred[t][q] = thredBase[t];

                        /* If there is WAN Queue configured, scale down LAN queue with a factor */
                        if (lastWanQue != -1)
                        {
                            sf2_sw_port_thred[t][q] = sf2_sw_port_thred[t][q] * LAN_QUE_OVER_WAN_QUE_FACTOR;
                        }
                    }
                    else
                    {
                        sf2_sw_port_thred[t][q] = sf2_sw_port_thred[t][q1] * LOW_QUE_OVER_HIGH_QUE_FACTOR;
                    }
            }
        }
    }

    /* Configure IMP port */
    for (q = FC_LAN_TXQ_QUEUES - 1; q >= 0; q--)
    {
        for (t = FC_THRED_TOTAL_TYPES - 1; t >= 0; t--)
        {
            switch (t)
            {
                case FC_THRED_QUE_RSRVD_TYPE:
                    sf2_imp0_thred[t][q] = thredBase[t];
                    break;
                default:
                    if (q == FC_LAN_TXQ_QUEUES - 1)
                    {
                        sf2_imp0_thred[t][q] = thredBase[t] * IMP_OVER_WAN_QUE_FACTOR;
                    }
                    else
                    {
                        sf2_imp0_thred[t][q] = sf2_imp0_thred[t][q + 1] * LOW_QUE_OVER_HIGH_QUE_FACTOR;
                    }
            }
        }
    }

    /* Configure IMP1 port for WAN port if it is used or Dual IMP port is used */
    for (q = FC_LAN_TXQ_QUEUES - 1; q >= 0; q--)
    {
        for (t = FC_THRED_TOTAL_TYPES - 1; t >= 0; t--)
        {
            switch (t)
            {
                case FC_THRED_QUE_RSRVD_TYPE:
                    /* Set reserve threshold to be the same */
                    sf2_wan_imp1_thred[t][q] = thredBase[FC_THRED_QUE_RSRVD_TYPE];
                    break;
                default:
                    /* Set all rest thredsholds to 2 frames lower than maximum buffer. */
                    sf2_wan_imp1_thred[t][q] = SF2_MAX_BUFFER_IN_PAGE - 2 * maxFrameLength;
            }
        }
    }

    _sf2_conf_thred_2reg(PAGE_FC_LAN_TXQ, REG_FC_LAN_TXQ_THD_RSV_QN0, sf2_sw_port_thred);
    _sf2_conf_thred_2reg(PAGE_FC_IMP0_TXQ, REG_FC_IMP0_TXQ_THD_RSV_QN0, sf2_imp0_thred);
    _sf2_conf_thred_2reg(PAGE_FC_IMP1_TXQ, REG_FC_IMP0_TXQ_THD_RSV_QN0, sf2_wan_imp1_thred);
}
#endif //!SF2_EXTERNAL

#define IMP_PORTS_ENABLE    1
#define IMP_PORTS_DISABLE   2
#define IMP_PORTS_SETUP     3

static int tr_imp_ports_op(enetx_port_t *port, void *ctx)
{
    int *op = ctx;
    int id = port->p.mac->mac_id;
    uint8_t val8, reg, bit;

#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
    if (port->p.port_cap != PORT_CAP_MGMT)
        return 0;
#else
    if ((port->p.port_cap != PORT_CAP_MGMT) || (id != IMP_PORT_ID))
        return 0;
#endif
    switch (*op) 
    {
    case IMP_PORTS_SETUP:
        // enable BRCM TAG
        reg = (id < P5_PORT_ID) ? REG_BRCM_HDR_CTRL2 : REG_BRCM_HDR_CTRL;
        bit = (id < P5_PORT_ID) ? 1 << id : (id == IMP_PORT_ID) ? BRCM_HDR_EN_IMP_PORT : (id == P7_PORT_ID) ? BRCM_HDR_EN_GMII_PORT_7 : BRCM_HDR_EN_GMII_PORT_5;
        spin_lock_bh(&sf2_reg_config);
        SF2SW_RREG(PAGE_MANAGEMENT, reg, &val8, sizeof(val8));
        val8 |= bit;
        SF2SW_WREG(PAGE_MANAGEMENT, reg, &val8, sizeof(val8));
        spin_unlock_bh(&sf2_reg_config);
        
        // enable link - port override register
        if (id == IMP_PORT_ID)
            val8 = IMP_LINK_OVERRIDE_2000FDX /*| REG_CONTROL_MPSO_FLOW_CONTROL*/; /* FIXME : Enabling flow control creates some issues */
        else 
            val8 = (id == P4_PORT_ID) ? LINK_OVERRIDE_1000FDX : LINK_OVERRIDE_1000FDX | REG_PORT_GMII_SPEED_UP_2G;
        SF2SW_WREG(PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(id), &val8, sizeof(val8));
        
        // intentional fall thru to _ENABLE
    case IMP_PORTS_ENABLE:
        spin_lock_bh(&sf2_reg_config);
        SF2SW_RREG(PAGE_CONTROL, REG_PORT_CTRL+id, &val8, 1);
        val8 &= ~REG_PORT_CTRL_DISABLE;
        SF2SW_WREG(PAGE_CONTROL, REG_PORT_CTRL+id, &val8, 1);
        spin_unlock_bh(&sf2_reg_config);
        break;
    case IMP_PORTS_DISABLE:
        spin_lock_bh(&sf2_reg_config);
        SF2SW_RREG(PAGE_CONTROL, REG_PORT_CTRL+id, &val8, 1);
        val8 |= REG_PORT_CTRL_DISABLE;
        SF2SW_WREG(PAGE_CONTROL, REG_PORT_CTRL+id, &val8, 1);
        spin_unlock_bh(&sf2_reg_config);
        break;
    }
    return 0;
}

static void _imp_ports_op(int op)
{
    int operation = op;
    port_traverse_ports(sf2_sw, tr_imp_ports_op, PORT_CLASS_PORT,  &operation);
}

static void _extsw_setup_imp_ports(void)
{
    // based on impl5:extsw_setup_imp_ports()
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT) || !defined(CONFIG_BCM_SWMDK)
    uint8_t  val8;
    uint16_t val16;
#endif /* CONFIG_BCM_ENET_MULTI_IMP_SUPPORT */
    /* Assumption : External switch is always in MANAGED Mode w/ TAG enabled.
     * BRCM TAG enable in external switch is done via MDK as well
     * but it is not deterministic when the userspace app for external switch
     * will run. When it gets delayed and the device is already getting traffic,
     * all those packets are sent to CPU without external switch TAG.
     * To avoid the race condition - it is better to enable BRCM_TAG during driver init. */
    _imp_ports_op(IMP_PORTS_SETUP);

#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)  || !defined(CONFIG_BCM_SWMDK)
    /* NOTE : Forcing these setting here; SWMDK doesn't setup IMP when multiple IMP ports in-use */

    /* Enable IMP Port */
    val8 = ENABLE_MII_PORT | RECEIVE_BPDU;
    SF2SW_WREG(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &val8, sizeof(val8));

    /* management mode, enable forwarding */
    spin_lock_bh(&sf2_reg_config);
    SF2SW_RREG(PAGE_CONTROL, REG_SWITCH_MODE, &val8, sizeof(val8));
    val8 |= REG_SWITCH_MODE_FRAME_MANAGE_MODE | REG_SWITCH_MODE_SW_FWDG_EN;
    SF2SW_WREG(PAGE_CONTROL, REG_SWITCH_MODE, &val8, sizeof(val8));
    spin_unlock_bh(&sf2_reg_config);

    /* enable rx bcast, ucast and mcast of imp port */
    val8 = REG_MII_PORT_CONTROL_RX_UCST_EN | REG_MII_PORT_CONTROL_RX_MCST_EN |
           REG_MII_PORT_CONTROL_RX_BCST_EN;
    SF2SW_WREG(PAGE_CONTROL, REG_MII_PORT_CONTROL, &val8, sizeof(val8));

    /* Forward lookup failure to use ULF/MLF/IPMC lookup fail registers */
    val8 = REG_PORT_FORWARD_MCST | REG_PORT_FORWARD_UCST | REG_PORT_FORWARD_IP_MCST;
    SF2SW_WREG(PAGE_CONTROL, REG_PORT_FORWARD, &val8, sizeof(val8));

    /* Forward unlearned unicast and unresolved mcast to the MIPS */
    val16 = PBMAP_MIPS;
    SF2SW_WREG(PAGE_CONTROL, REG_UCST_LOOKUP_FAIL, &val16, sizeof(val16));
    SF2SW_WREG(PAGE_CONTROL, REG_MCST_LOOKUP_FAIL, &val16, sizeof(val16));
    SF2SW_WREG(PAGE_CONTROL, REG_IPMC_LOOKUP_FAIL, &val16, sizeof(val16));

    /* Disable learning on MIPS*/
    val16 = PBMAP_MIPS;
    SF2SW_WREG(PAGE_CONTROL, REG_DISABLE_LEARNING, &val16, sizeof(val16));

    /* NOTE : All regular setup for P8 IMP is done above ; Same as what SWMDK would do*/
#endif /* CONFIG_BCM_ENET_MULTI_IMP_SUPPORT */
}

#include "bcm_chip_arch.h"

static void _extsw_set_pbvlan(int port, uint16_t fwdMap)
{
    SF2SW_WREG(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (port * 2), (uint8_t *)&fwdMap, 2);
}

#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)

static uint16_t _extsw_get_pbvlan(int port)
{
    uint16_t val16;

    SF2SW_RREG(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (port * 2), (uint8_t *)&val16, 2);
    return val16;
}

/*
    Use CFP to force Reserved Multicast Address to be received by
    IMP port correctly overriding Port Based VLAN set for load balancing.
*/
static int _bcmsw_add_cfp_rsvd_multicast_support(void)
{
    // based on impl5:bcmsw_add_cfp_rsvd_multicast_support()
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

    return ioctl_extsw_cfp(e);
}

static void _bcmsw_print_imp_port_grouping(unsigned long port_map, int port_imp_map[])
{
    // based on impl5:bcmsw_print_imp_port_grouping()
    int port, imp_port, new_grp = 0;
    printk("NOTE: Using Port Grouping for IMP ports : ");
    for (imp_port = 0; imp_port <= BP_MAX_SWITCH_PORTS; imp_port++)
    {
        /* Not an IMP port -- continue */
        if (! ( (1<<imp_port) & DEFAULT_IMP_PBMAP ) ) continue;
        new_grp = 1;
        for (port = 0; port < BP_MAX_SWITCH_PORTS; port++) 
        {
            if ( ((1<<port) & port_map) && 
                 port_imp_map[port] == imp_port )
            {
                if (new_grp)
                {
                    pr_cont("[");
                    new_grp = 0;
                }
                else
                {
                    pr_cont(",");
                }
                pr_cont(" %d",port);
            }
        }
        if (!new_grp)
        {
            pr_cont(" --> %d ] ",imp_port);
        }
    }
    pr_cont("\n");
}

static int *cur_port_imp_map;
static void _extsw_cfg_port_imp_grouping(int port_imp_map[])
{
    // based on impl5:extsw_cfg_port_imp_grouping()
    unsigned char port;
    unsigned long port_map;
    uint16 v16;
    const ETHERNET_MAC_INFO *EnetInfo = BpGetEthernetMacInfoArrayPtr();
    const ETHERNET_MAC_INFO *info = &EnetInfo[SF2_ETHSWCTL_UNIT];

    if (cur_port_imp_map == port_imp_map) return;

    cur_port_imp_map = port_imp_map;
    /* Configure forwarding based on Port Grouping
     * By default all port's pbvlan is 0x1FF */
    port_map = info->sw.port_map; /* Get port map for external switch */
    /* NOTE : ports are scanned to cover last IMP port as well -- see details below */
    for (port = 0; port < BP_MAX_SWITCH_PORTS+1; port++)
    {
        v16 = 0;
        if ( !(DEFAULT_IMP_PBMAP & (1<<port)) && (port_imp_map[port] != -1) && ( (1<<port) & port_map ) )
        {
            v16 = _extsw_get_pbvlan(port) & chip_arch_all_portmap[1]; /* Get current PBVLAN Map */
            v16 &= ~(imp_pbmap[1]); /* Remove all IMP Ports from PBVLAN Map for external switch*/
            v16 |= (1<<port_imp_map[port]); /* Add back the desired IMP Port */
            _extsw_set_pbvlan(port, v16);
            port_imp_emac_map[port] = imp_to_emac[port_imp_map[port]];
        }
        else if ( DEFAULT_IMP_PBMAP & (1<<port) ) 
        { /* IMP Port - Block IMP to IMP forwarding */
            /* As such there is no need to block IMP-IMP forwarding because it should NEVER happen
             * But during initial runner development, it was noticed that runner was adding incorrect
             * Broadcom tag (that has destination port as other IMP), this results in packet getting 
             * looped back; In order to avoid this issue temporarily, following is done. 
             * Below change could be kept as permanent, though not needed. */ 
            v16 = _extsw_get_pbvlan(port) & chip_arch_all_portmap[1]; /* Get current PBVLAN Map */
            v16 &= ~(imp_pbmap[1]); /* Remove all IMP Ports from PBVLAN Map for external switch*/
            v16 |= (1<<port); /* Add back this IMP Port - Not required though */
            _extsw_set_pbvlan(port, v16);
        }
    }
    
    _bcmsw_print_imp_port_grouping(port_map, port_imp_map);
}

static void _extsw_set_port_imp_map_2_5g(void)
{
    _extsw_cfg_port_imp_grouping(port_imp_map_2_5g);
}

static void _extsw_set_port_imp_map_non_2_5g(void)
{
    _extsw_cfg_port_imp_grouping(port_imp_map_non_2_5g);
}


static void _extsw_setup_imp_fwding(void)
{
    // based on impl5:extsw_setup_imp_fwding()
    uint16 v16;

    /* Configure the Lookup failure registers to all IMP ports */
    v16 = DEFAULT_IMP_PBMAP;
    SF2SW_WREG(PAGE_CONTROL, REG_UCST_LOOKUP_FAIL, &v16, sizeof(v16));
    SF2SW_WREG(PAGE_CONTROL, REG_MCST_LOOKUP_FAIL, &v16, sizeof(v16));
    SF2SW_WREG(PAGE_CONTROL, REG_IPMC_LOOKUP_FAIL, &v16, sizeof(v16));
    /* Disable learning on all IMP ports */
    SF2SW_WREG(PAGE_CONTROL, REG_DISABLE_LEARNING, &v16, sizeof(v16));

    _extsw_set_port_imp_map_non_2_5g(); /* By default we start with assuming no 2.5G port */

    _bcmsw_add_cfp_rsvd_multicast_support();
}
#else
static void _extsw_setup_imp_fwding(void) {}
#endif //!defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)

static rgmii_registers_t *get_rgmii_regs(int ext_port)
{
    int i;

#if defined(CONFIG_BCM947622)
    // 47622 only has one RGMII, however some boarparams don't use crossbar, so just use the one
    ext_port = BP_CROSSBAR_PORT_BASE+1;
#endif
    for (i=0; i<ARRAY_SIZE(rgmii_port_regs); i++)
    {
        if (rgmii_port_regs[i].ext_physical_port == ext_port) 
            return &rgmii_port_regs[i];
    }
    return NULL;
}

static void phy_config_rgmii(phy_dev_t *phy_dev, int physical_port)
{
    rgmii_registers_t *rgmii_regs;
    uint32 rgmii_ctrl_v, rgmii_rx_clk_delay_v;

    rgmii_regs = get_rgmii_regs(physical_port);
    if (rgmii_regs == NULL) {
        enet_err("**** ERROR: physical port %d not a RGMII port\n", physical_port);
        return;
    }

    if (IsRGMII_1P8V(phy_dev->meta_id))
        *rgmii_regs->pad_ctrl = (*rgmii_regs->pad_ctrl & ~MISC_XMII_PAD_MODEHV) | MISC_XMII_PAD_AMP_EN;
    else if (IsRGMII_2P5V(phy_dev->meta_id))
        *rgmii_regs->pad_ctrl = (*rgmii_regs->pad_ctrl | MISC_XMII_PAD_MODEHV) & ~MISC_XMII_PAD_SEL_GMII & ~MISC_XMII_PAD_AMP_EN;
    else if (IsRGMII_3P3V(phy_dev->meta_id))
        *rgmii_regs->pad_ctrl = (*rgmii_regs->pad_ctrl | MISC_XMII_PAD_MODEHV | MISC_XMII_PAD_SEL_GMII) & ~MISC_XMII_PAD_AMP_EN;

    rgmii_ctrl_v = (*rgmii_regs->ctrl & ~ETHSW_RC_MII_MODE_MASK) | ETHSW_RC_RGMII_EN | ETHSW_RC_ID_MODE_DIS;
    rgmii_ctrl_v |= ETHSW_RC_EXT_GPHY;

    rgmii_rx_clk_delay_v = *rgmii_regs->rx_clk_delay;

    /* Phy Tx clk Delay is on by default. phy reg 24, shadow reg 7
     * works with Rx delay -- ON by default. phy reg 28, shadow reg 3
     * No action on the phy side unless specified in boardparms
     */

    if (!phy_dev->delay_tx)
        rgmii_ctrl_v &= ~ETHSW_RC_ID_MODE_DIS; /* Clear TX_ID_DIS */

    if (!phy_dev->delay_rx)
        rgmii_rx_clk_delay_v &= ~(ETHSW_RXCLK_IDDQ|ETHSW_RXCLK_BYPASS); /* Clear Rx bypass */
 
    *rgmii_regs->rx_clk_delay = rgmii_rx_clk_delay_v;
    *rgmii_regs->ctrl = rgmii_ctrl_v;
}

static int tr_port_for_rgmii(enetx_port_t *p, void *ctx)
{
    phy_dev_t *phy_dev;
    int sw_override = 0;

    if (phy_is_crossbar(p->p.phy))
    {
        // this port is connected to cross bar, traverse the phys that is connected
        for (phy_dev = crossbar_phy_dev_first(p->p.phy); phy_dev; phy_dev = crossbar_phy_dev_next(phy_dev))
        {
            if (IsRGMII(phy_dev->meta_id))
                phy_config_rgmii(phy_dev, BP_CROSSBAR_PORT_TO_PHY_PORT(crossbar_external_endpoint(phy_dev)));
        }
        // turn on SW_OVERRIDE when port is connected to crossbar
        sw_override = 1;
    }
    else
    {
        phy_dev = p->p.phy;
        if (phy_dev)
        {
            if (IsRGMII(phy_dev->meta_id))
            {
                phy_config_rgmii(phy_dev, p->p.mac->mac_id);
                // turn on SW_OVERRIDE when phy is RGMII
                sw_override = 1;
            }
        }
    }

    if (sw_override && PORT_ON_EXTERNAL_SW(p))
    {
        uint8_t v8;
        // set override, and clear old link info
        v8 = REG_PORT_STATE_OVERRIDE;
        SF2SW_WREG(PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(p->p.mac->mac_id), &v8, 1);
    }
    return 0;
}

void sf2_rgmii_config(void)
{
    platform_init_rgmii_regs_array();
    port_traverse_ports(root_sw, tr_port_for_rgmii, PORT_CLASS_PORT, NULL);
}

static void _extsw_port_trunk_init(void)
{
    // based on impl5:extsw_port_runk_init()
    int enable_trunk = 0;
#if defined(CONFIG_BCM_KERNEL_BONDING)
    enable_trunk |= 1;
#endif

    if (enable_trunk)
    {
        unsigned char v8;
        spin_lock_bh(&sf2_reg_config);
        SF2SW_RREG(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        v8 |= ( (1 & TRUNK_EN_LOCAL_M) << TRUNK_EN_LOCAL_S ); /* Enable Trunking */
        v8 |= ( (TRUNK_HASH_DA_SA_VID & TRUNK_HASH_SEL_M) << TRUNK_HASH_SEL_S ); /* Default VID+DA+SA Hash */
        SF2SW_WREG(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        SF2SW_RREG(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        spin_unlock_bh(&sf2_reg_config);
        printk("LAG/Trunking enabled <0x%02x>\n",v8);
    }
}

static void _sf2_tc_to_cos_default(void)
{
    int i, j;
    uint16_t reg_addr;
    uint32_t val32;

    for (i = 0; i <= SF2_IMP0_PORT; i++) // all ports except 6
    {
        if (i == SF2_INEXISTANT_PORT) continue; // skip port 6
        reg_addr = SF2_REG_PORTN_TC_TO_COS + i * 4;
        val32 = 0;
        for (j = 0; j <= SF2_QOS_TC_MAX; j++) // all TC s
        {
            //  TC to COS one-one mapping
            val32 |= (j & SF2_QOS_COS_MASK) << (j * SF2_QOS_COS_SHIFT);
        }
        SF2SW_WREG(PAGE_QOS, reg_addr, &val32, 4);
    }
}

static void _sf2_qos_default(void)
{
    uint32_t val32;
    uint32_t port;
    spin_lock_bh(&sf2_reg_config);
    /* Set Global QoS Control */
    SF2SW_RREG(PAGE_QOS, SF2_REG_QOS_GLOBAL_CTRL, &val32, 4);
    val32 |= SF2_QOS_P8_AGGREGATION_MODE;
    SF2SW_WREG(PAGE_QOS, SF2_REG_QOS_GLOBAL_CTRL, &val32, 4);
    /* set SP scheduling on all ports (including IMP) by default */
    for (port=0; port <= BP_MAX_SWITCH_PORTS;  port++)
    {
        if (port == SF2_INEXISTANT_PORT) continue;
        SF2SW_RREG(PAGE_QOS_SCHEDULER, REG_PN_QOS_PRI_CTL_PORT_0 + (port), &val32, 4);
        val32 &= ~(PN_QOS_SCHED_SEL_M<<PN_QOS_SCHED_SEL_S); /* Clear Bits */
        val32 |= (SF2_ALL_Q_SP<<PN_QOS_SCHED_SEL_S); /* Set SP for all */
        SF2SW_WREG(PAGE_QOS_SCHEDULER, REG_PN_QOS_PRI_CTL_PORT_0 + (port), &val32, 4);
    }
    spin_unlock_bh(&sf2_reg_config);
    /* Set default TC to COS config */
    _sf2_tc_to_cos_default();
}

static void config_wan_queue_acb_map(void)
{
#if defined(SF2_ACB_CONTROL_QUE_MAP_0)
    int q;
    volatile uint32_t *sf2_acb_control_que_map_0 = (void *)SF2_ACB_CONTROL_QUE_MAP_0;
    volatile uint32_t *sf2_acb_control_que_map_1 = (void *)SF2_ACB_CONTROL_QUE_MAP_1;
    /* 
       Enable all port's and WAN queues for PORT Based ACB packet count
     */
    *sf2_acb_control_que_map_0 = *sf2_acb_control_que_map_1 = 0;
    for (q=0; q<FC_LAN_TXQ_QUEUES; q++) {
        if ((wanQueMap & (1<<q)) == 0) continue;
        *sf2_acb_control_que_map_0 = *sf2_acb_control_que_map_1 |= 
            (1<<q)|(1<<(q+8))|(1<<(q+16))|(1<<(q+24));
    }
#endif
}

#if !defined(SF2_EXTERNAL)
typedef struct acb_config_s {
    uint16 total_xon_hyst;
    uint16 xon_hyst;
    acb_queue_config_t acb_queue_config[1/*64*/];
} acb_config_t;

static acb_config_t acb_profiles [] = {
    // profile 1
    {
        .total_xon_hyst = 6,
        .xon_hyst = 4,
        {
            // queue 0, (port 0) for LAN->LAN
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
            {
                .pessimistic_mode = 0,
                .total_xon_en = 0,
                .pkt_len = 0,
                .xoff_threshold = 16,
            },
#else // For 148
            {
                .pessimistic_mode = 1,
                .total_xon_en = 1,
                .pkt_len = 6,
                .xoff_threshold = 16,
            },
#endif
        },

    },
};

void _sf2_conf_acb_conges_profile(int profile)
{
    /* based on impl5:sf2_conf_acb_conges_profile() */
    volatile uint32_t *sf2_acb_control    = (void *) (SF2_ACB_CONTROL_REG);
    volatile uint32_t *sf2_acb_xon_thresh = (void *) (sf2_acb_control + 1);
    volatile uint32_t *sf2_acb_que_config = (void *) (sf2_acb_control + 2);
#if defined(SF2_ACB_PORT0_CONFIG_REG)
    volatile uint32_t *sf2_acb_port_config = (void *) (SF2_ACB_PORT0_CONFIG_REG);
#endif
    acb_config_t *p = &acb_profiles [profile];
    int q, val32;
    acb_queue_config_t *qp;

    // acb disable
    *sf2_acb_control &= ~SF2_ACB_EN;
    *sf2_acb_xon_thresh = (p->total_xon_hyst & SF2_ACB_BUFS_THRESH_M) << SF2_ACB_TOTAL_XON_BUFS_S |
                          (p->xon_hyst & SF2_ACB_BUFS_THRESH_M) << SF2_ACB_XON_BUFS_S;

    // compute proper acb_xoff_threshold based on MTU size a buffer is 256 bytes
#if defined(CONFIG_BCM963178)
    acb_xoff_threshold = 32;    // fixed threshold as 178 has limited number of buffers
    acb_port_xoff_threshold = acb_xoff_threshold * 3/2;     /* Scale up per port XOFF thread to 1.5 times of per queue */
#else
    acb_xoff_threshold = (ENET_MAX_MTU_SIZE + SF2_BYTES_PER_PAGE - 1) / SF2_BYTES_PER_PAGE;  // get number of buffers needed
    acb_xoff_threshold = ((acb_xoff_threshold < 8) ? 8 : acb_xoff_threshold) * 2;   // need to be at least 2 times max packet
    acb_port_xoff_threshold = 0; /* For switch threadshold calculation */
#endif


    for (q = 0; q <= SF2_ACB_QUE_MAX; q++)
    {

        /*
         * We have made room to configure each of the 64 queues with differently
         * defined per q values.
         * Here, we are however, duplicating q0 profiled ACB  config on every queue so we do not
         * leave queues congested for ever when ACB is enabled by default.
         */
        val32 = 0;
        qp = &p->acb_queue_config[0];
        val32 |= (qp->pessimistic_mode & SF2_ACB_QUE_PESSIMISTIC_M)
                                        << SF2_ACB_QUE_PESSIMISTIC_S;
        val32 |= (qp->total_xon_en & SF2_ACB_QUE_TOTAL_XON_M) << SF2_ACB_QUE_TOTAL_XON_S;
        val32 |= (qp->xon_en & SF2_ACB_QUE_XON_M) << SF2_ACB_QUE_XON_S;
        val32 |= (qp->total_xoff_en & SF2_ACB_QUE_TOTAL_XOFF_M) << SF2_ACB_QUE_TOTAL_XOFF_S;
        val32 |= (qp->pkt_len & SF2_ACB_QUE_PKT_LEN_M) << SF2_ACB_QUE_PKT_LEN_S;
        val32 |= (acb_xoff_threshold/*qp->xoff_threshold*/ & SF2_ACB_BUFS_THRESH_M) << SF2_ACB_QUE_XOFF_BUFS_S;
        *(sf2_acb_que_config + q) =  val32;
    }
    // acb enable
#if defined(ACB_ALGORITHM2) && !defined(CONFIG_BCM963178)   // For 138, Algorithm2 is applied
    *sf2_acb_control |= SF2_ACB_EN | (SF2_ACB_ALGORITHM_M << SF2_ACB_ALGORITHM_S);
#else
    *sf2_acb_control |= SF2_ACB_EN;
#endif

#if defined(SF2_ACB_PORT0_CONFIG_REG)
    for (q = 0; q < BP_MAX_SWITCH_PORTS; q++) {
        // due to jira37197 strict priorty use acb_xoff_threshold * 3 queues + extra as port xoff limit
        // instead of using acb_port_xoff_threshold which is still used in Congestion calculation.
#if defined(PORT_WITH_8TXQ)
        if (q == PORT_WITH_8TXQ)
            // for port with 8 txqs use 7.5 multiplier to support 8 SP.
            *(sf2_acb_port_config + q) = SF2_ACB_PORT_XOFF_EN | (acb_xoff_threshold*15/2);
        else
#endif
            *(sf2_acb_port_config + q) = SF2_ACB_PORT_XOFF_EN | (acb_xoff_threshold*7/2);
    }
#endif

    config_wan_queue_acb_map();
}
#endif //!SF2_EXTERNAL

void sf2_rreg(int page, int reg, void *data_out, int len);
void sf2_wreg(int page, int reg, void *data_in, int len);

void extsw_rreg_wrap(int page, int reg, void *vptr, int len)
{
    uint8 val[8];
    uint8 *data = (uint8*)vptr;
    int type = len & SWAP_TYPE_MASK;

    len &= ~(SWAP_TYPE_MASK);

    /* Lower level driver always returnes in Little Endian data from history */
    sf2_rreg(page, reg, val, len);

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
}

void extsw_wreg_wrap(int page, int reg, void *vptr, int len)
{
    uint8 val[8];
    uint8 *data = (uint8*)vptr;
    int type = len & SWAP_TYPE_MASK;

    len  &= ~(SWAP_TYPE_MASK);

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
    sf2_wreg(page, reg, val, len);
}

// =========== public ioctl functions =====================


// ----------- SIOCETHSWCTLOPS ETHSWCOSSCHED functions ---
/*
 * Get/Set StarFighter port scheduling policy
 *** Input params
 * e->type  GET/SET
 * e->port_qos_sched.num_spq  Tells SP/WRR policy to use on the port's queues
 * e->port_qos_sched.wrr_type Granularity packet or 256 byte
 * e->port  per port
 *** Output params
 * e->val has current sched policy - GET
 * Returns 0 for Success, Negative value for failure.
 */
int ioctl_extsw_cosq_sched(enetx_port_t *self, struct ethswctl_data *e)
{
    // based on impl5:sf2_cosq_sched()
    int reg;
    int i, j;
    uint8_t data[8];
    uint8_t val8 = 0;

    down(&self->p.parent_sw->s.conf_sem);

    reg = REG_PN_QOS_PRI_CTL_PORT_0 + e->port * REG_PN_QOS_PRI_CTL_SZ;
    SF2SW_RREG(PAGE_QOS_SCHEDULER, reg, &val8, REG_PN_QOS_PRI_CTL_SZ);
    if (e->type == TYPE_GET) {
        switch ((val8 >> PN_QOS_SCHED_SEL_S ) & PN_QOS_SCHED_SEL_M)
        {
            case 0:
                e->port_qos_sched.sched_mode = BCM_COSQ_STRICT;
                break;
            case 5:
                e->port_qos_sched.sched_mode = BCM_COSQ_WRR;
                break;
            default:
                e->port_qos_sched.sched_mode = BCM_COSQ_COMBO;
                e->port_qos_sched.num_spq    = (val8 & PN_QOS_SCHED_SEL_M) << PN_QOS_SCHED_SEL_S;
                break;
        }
        e->port_qos_sched.port_qos_caps = QOS_SCHED_SP_CAP | QOS_SCHED_WRR_CAP | QOS_SCHED_WDR_CAP |
                                        QOS_SCHED_COMBO | QOS_PORT_SHAPER_CAP | QOS_QUEUE_SHAPER_CAP;
        e->port_qos_sched.max_egress_q = NUM_EGRESS_QUEUES;
        e->port_qos_sched.max_egress_spq = MAX_EGRESS_SPQ;
        e->port_qos_sched.wrr_type = (val8 >> PN_QOS_WDRR_GRAN_S) & PN_QOS_WDRR_GRAN_M?
                       QOS_ENUM_WRR_PKT: QOS_ENUM_WDRR_PKT;
        SF2SW_RREG(PAGE_QOS_SCHEDULER, REG_PN_QOS_WEIGHT_PORT_0 +
                       e->port * REG_PN_QOS_WEIGHTS, data, DATA_TYPE_HOST_ENDIAN|REG_PN_QOS_WEIGHTS);
        for (i = 0; i < BCM_COS_COUNT; i++) {
            e->weights[i] = data[i];
        }
    } else { // TYPE_SET
        val8 &= ~(PN_QOS_SCHED_SEL_M << PN_QOS_SCHED_SEL_S);
        if (e->port_qos_sched.sched_mode == BCM_COSQ_WRR) {
                val8 |= (SF2_ALL_Q_WRR & PN_QOS_SCHED_SEL_M) << PN_QOS_SCHED_SEL_S;
        } else if ((e->port_qos_sched.sched_mode == BCM_COSQ_SP) &&
                           (e->port_qos_sched.num_spq == 0)) {
                val8 |= (SF2_ALL_Q_SP & PN_QOS_SCHED_SEL_M) << PN_QOS_SCHED_SEL_S;

        } else {
            switch (e->port_qos_sched.num_spq) {
                case 1:
                    val8 |= (SF2_Q7_SP & PN_QOS_SCHED_SEL_M) << PN_QOS_SCHED_SEL_S;
                    break;
                case 2:
                    val8 |= (SF2_Q7_Q6_SP & PN_QOS_SCHED_SEL_M) << PN_QOS_SCHED_SEL_S;
                    break;
                case 3:
                    val8 |= (SF2_Q7_Q5_SP & PN_QOS_SCHED_SEL_M) << PN_QOS_SCHED_SEL_S;
                    break;
                case 4:
                    val8 |= (SF2_Q7_Q4_SP & PN_QOS_SCHED_SEL_M) << PN_QOS_SCHED_SEL_S;
                    break;
                default:
                    enet_dbg("Incorrect num_spq param %d", e->port_qos_sched.num_spq);
                    up(&self->p.parent_sw->s.conf_sem);
                    return -BCM_E_PARAM;
                    break;
            }
        }
        if (e->port_qos_sched.wrr_type == QOS_ENUM_WRR_PKT) {
            val8 |= SF2_WRR_PKT << PN_QOS_WDRR_GRAN_S;
        } else if (e->port_qos_sched.wrr_type == QOS_ENUM_WDRR_PKT) {
            val8 &= ~(SF2_WRR_PKT << PN_QOS_WDRR_GRAN_S);
        }
        SF2SW_WREG(PAGE_QOS_SCHEDULER, reg, &val8, REG_PN_QOS_PRI_CTL_SZ);
 // programming queue weights.
        if (e->port_qos_sched.num_spq || e->port_qos_sched.sched_mode == BCM_COSQ_WRR) {
                      // some or all queues in weighted mode.
            SF2SW_RREG(PAGE_QOS_SCHEDULER, REG_PN_QOS_WEIGHT_PORT_0 +
                           e->port * REG_PN_QOS_WEIGHTS, data, DATA_TYPE_HOST_ENDIAN | REG_PN_QOS_WEIGHTS);
            i = e->port_qos_sched.weights_upper? (BCM_COS_COUNT/2): 0;
            for (j = 0; j < BCM_COS_COUNT/2; i++, j++) {
                data[i] = e->weights[i];
            }
            SF2SW_WREG(PAGE_QOS_SCHEDULER, REG_PN_QOS_WEIGHT_PORT_0 + e->port * REG_PN_QOS_WEIGHTS,
                            data, DATA_TYPE_HOST_ENDIAN | REG_PN_QOS_WEIGHTS);
        }
    } // SET
    up(&self->p.parent_sw->s.conf_sem);
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWEXTPHYLINKSTATUS functions ---
int ioctl_ext_phy_link_set(enetx_port_t *port, int link)
{
    phy_dev_t *phy = get_active_phy(port->p.phy);

    if (phy && phy->phy_drv->phy_type == PHY_TYPE_MAC2MAC) {
        phy->link = link;
        link_change_handler(port, phy->link, speed_macro_2_mbps(phy->speed), phy->duplex == PHY_DUPLEX_FULL);
#if defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE)
        port_sf2_deep_green_mode_handler();
#endif
        return 0;
    }
    return -EOPNOTSUPP;
}

#if !defined(SF2_EXTERNAL)
// ----------- SIOCETHSWCTLOPS ETHSWDUMPPAGE functions ---
void ioctl_extsw_dump_page0(void)
{
    // based on impl5:sf2_dump_page0()
    // TODO_DSL? Do we want to change memory mapped to register read access?
    int i = 0, page = 0;
    volatile EthernetSwitchCore *e = ETHSW_CORE;
    EthernetSwitchCore *f = (void *)NULL;

#define SHOW_REG_FIELD(mb) (int)(((long)&(f->mb)/sizeof(f->mb))&0xff), ((uint32)(e->mb)), ((uint32)(e->mb))
    printk("#The Page0 Registers \n");
    for (i=0; i<9; i++)
        printk("%02x %02x = 0x%02x (%u) \n", page, SHOW_REG_FIELD(port_traffic_ctrl[i])); /* 0x00 - 0x08 */

    printk("%02x %02x 0x%02x (%u) \n", page, SHOW_REG_FIELD(switch_mode));
}
#endif //!SF2_EXTERNAL

typedef struct unit_to_sw_s {
    int unit; 
    int cnt;
    enetx_port_t *sw;
} unit_to_sw_t;

static int tr_get_sw_from_unit(enetx_port_t *port, void *ctx)
{
    unit_to_sw_t *us = ctx;    

    if (us->unit == us->cnt) {
        us->sw = port;
        return 1;
    }
    us->cnt++;
    return 0;
}

static enetx_port_t *enet_get_sw_from_unit(int unit)
{
    int rc;
    unit_to_sw_t us = {unit};
    rc = port_traverse_ports(root_sw, tr_get_sw_from_unit, PORT_CLASS_SW, &us);
    if (rc)
        return us.sw;
    return NULL;
}

int tr_mdk_phy_pbmap(enetx_port_t *port, void *ctx)
{
    int *phy_map = ctx;
    phy_dev_t *phy = get_active_phy(port->p.phy);
    
    if (phy && IsPhyConnected(phy->meta_id) &&
            !port->p.handle_phy_link_change)
        *phy_map |= (1<<port->p.mac->mac_id);
    return 0;
}

static int enet_get_phy_mdk_pbmap(enetx_port_t *sw)
{
    int phy_map = 0;
    _port_traverse_ports(sw, tr_mdk_phy_pbmap, PORT_CLASS_PORT, &phy_map, 1);
    return phy_map;
}

static int tr_enet_link_pbmap(enetx_port_t *port, void *ctx)
{
    int *phy_map = ctx;

    if (port->p.handle_phy_link_change)
        *phy_map |= (1<<port->p.mac->mac_id);
    return 0;
}

static int enet_get_enet_link_pmap(enetx_port_t *sw)
{
    int enet_link_pmap = 0;
    _port_traverse_ports(sw, tr_enet_link_pbmap, PORT_CLASS_PORT, &enet_link_pmap, 1);
    return enet_link_pmap;
}

// ----------- SIOCETHSWCTLOPS ETHSWPSEUDOMDIOACCESS functions ---
int ioctl_extsw_pmdioaccess(struct ethswctl_data *e)
{
    int page, reg;
    uint8 data[8] = {0};

    page = (e->offset >> 8) & 0xFF;
    reg = e->offset & 0xFF;
    if (e->type == TYPE_GET) {
        sf2_pseudo_mdio_switch_read(page, reg, data, e->length);
        memcpy((void*)(e->data), (void*)data, e->length);
    } else {
        sf2_pseudo_mdio_switch_write(page, reg, e->data, e->length);
    }
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWINFO functions ---
int ioctl_extsw_info(struct ethswctl_data *e)
{
    // based on shared\bcmswaccess.c:enet_ioctl_ethsw_info()
    int bus_type = MBUS_NONE;
    ETHERNET_MAC_INFO *EnetInfo = (ETHERNET_MAC_INFO *)BpGetEthernetMacInfoArrayPtr();
    ETHERNET_MAC_INFO *info;
    unsigned int vend_id = 0, dev_id = 0, rev_id = 0;
    int i;
    enetx_port_t *port;
    phy_dev_t *phy_dev;
    phy_drv_t *phy_drv;
    u16 v16;
    enetx_port_t *sw;

    if (e->val > BP_MAX_ENET_MACS) {
        e->ret_val = bus_type;
        return BCM_E_NONE;
    }

    info = &EnetInfo[e->val];
    if ((info->ucPhyType == BP_ENET_EXTERNAL_SWITCH) ||
            (info->ucPhyType == BP_ENET_SWITCH_VIA_INTERNAL_PHY)) {

        switch (info->usConfigType) {
            case BP_ENET_CONFIG_MMAP:
                bus_type = MBUS_MMAP;   /* SF2 based bus type is always MMAP */
                break;
            case BP_ENET_CONFIG_MDIO_PSEUDO_PHY:
            case BP_ENET_CONFIG_GPIO_MDIO:
            case BP_ENET_CONFIG_MDIO:
                bus_type = MBUS_MDIO;  
                break;
            default:
                break;
        }
    }

    if (e->val == SF2_ETHSWCTL_UNIT) {
        for (i = 0; i < BP_MAX_SWITCH_PORTS; i++) {
            uint32_t meta_id;

            port = unit_port_array[e->val][i];
            phy_dev = get_active_phy(port->p.phy);
            meta_id = phy_dev ? phy_dev->meta_id : 0;
            if(((1<<i) & info->sw.port_map) == 0 || !IsPhyConnected(meta_id) ||
                    IsExtPhyId(meta_id) || IsSerdes(meta_id) || !port) {
                continue;
            }

            phy_drv = phy_dev->phy_drv;
            phy_bus_read(phy_dev, 2, &v16);
            vend_id = v16;
            vend_id = __le32_to_cpu(vend_id);
            phy_bus_read(phy_dev, 3, &v16);
            dev_id = v16;
            dev_id = __le32_to_cpu(dev_id);
            enet_dbgv("vendor=%x dev=%x\n", vend_id, dev_id);
            if (dev_id >= 0xb000) {
                rev_id = dev_id & 0xF;
                dev_id &= 0xFFF0;
            }
            break;
        }

        if(i == BP_MAX_SWITCH_PORTS) {
            enet_err("Error: No integrated PHY defined for device ID in this board design.\n");
            return -EFAULT;
        }
    }

    e->ret_val = bus_type;
    e->vendor_id = vend_id;
    e->dev_id = dev_id;
    e->rev_id = rev_id;

    sw = enet_get_sw_from_unit(e->val);
    e->phy_portmap = enet_get_phy_mdk_pbmap(sw);
    e->port_map = info->sw.port_map & (~enet_get_enet_link_pmap(sw));

    return BCM_E_NONE;
}


// ----------- SIOCETHSWCTLOPS ETHSWREGACCESS functions ---
int ioctl_extsw_regaccess(struct ethswctl_data *e, enetx_port_t *port)
{
    // based on shared\bcmswaccess.c:enet_ioctl_ethsw_regaccess()
    unsigned char data[8] = {0};

    if (e->offset & IS_PHY_ADDR_FLAG)
        return _ethsw_phy_access(e, port);

    if ( ((e->length != 1) && (e->length % 2)) || (e->length > 8)) {
        enet_err("Invalid length");
        return -EINVAL;
    }
    if (e->unit != SF2_ETHSWCTL_UNIT)
                       /* Offset checks are not valid below for SF2 because all offsets
                       * are converted to 4-byte accesses later down the road. */
    {
        /*
            We do have unlignment register access: 0x0583 4 bytes and 0x0581 2bytes for VLAN configuration.
            So unligned access is needed and driver underneath is working correctly.
            Make exaption for 0x0583 and 0x0581
        */
        if (!_alignment_check(e))
        {
            enet_err("len = %d offset = 0x%04x alignment error !! \n", e->length, e->offset);
            return -EINVAL;
        }
    }

    if (e->type == TYPE_GET) {
        if (e->unit == SF2_ETHSWCTL_UNIT) {
            SF2SW_RREG(((e->offset & 0xFF00)>>8), (e->offset & 0xFF), data, e->length);
        }
        else
        {
            enet_err("internal switch reg read not supported.!! (offset=%x, len=%d)\n", e->offset, e->length);
            return -EINVAL;
            //ethsw_read_reg(e->offset, data, e->length);
        }
        memcpy((void*)(&e->data), (void*)&data, e->length);

    } else {
        if (e->unit == SF2_ETHSWCTL_UNIT) {
            SF2SW_WREG(((e->offset & 0xFF00)>>8), (e->offset & 0xFF), e->data, e->length);
        }
        else
        {
            enet_err("internal switch reg write not supported.!! (offset=%x, len=%d)\n", e->offset, e->length);
            return -EINVAL;
            //ethsw_write_reg(e->offset, e->data, e->length);
        }
    }

    return BCM_E_NONE;

}

// ----------- SIOCETHSWCTLOPS ETHSWMIRROR functions ---
int ioctl_extsw_port_mirror_ops(struct ethswctl_data *e)
{
    /* based on impl5:bcmsw_port_mirror_get/set() */
    uint16_t v16;

    if (e->type == TYPE_GET) {
        e->port_mirror_cfg.tx_port = -1;
        e->port_mirror_cfg.rx_port = -1;

        SF2SW_RREG(PAGE_MANAGEMENT, REG_MIRROR_CAPTURE_CTRL, &v16, sizeof(v16));
        if (v16 & REG_MIRROR_ENABLE)
        {
            e->port_mirror_cfg.enable = 1;
            e->port_mirror_cfg.mirror_port = v16 & REG_CAPTURE_PORT_M;
            e->port_mirror_cfg.blk_no_mrr = v16 & REG_BLK_NOT_MIRROR;
            SF2SW_RREG(PAGE_MANAGEMENT, REG_MIRROR_INGRESS_CTRL, &v16, sizeof(v16));
            e->port_mirror_cfg.ing_pmap = v16 & REG_INGRESS_MIRROR_M;
            SF2SW_RREG(PAGE_MANAGEMENT, REG_MIRROR_EGRESS_CTRL, &v16, sizeof(v16));
            e->port_mirror_cfg.eg_pmap = v16 & REG_EGRESS_MIRROR_M;
        }
        else
        {
            e->port_mirror_cfg.enable = 0;
        }
    } else {
        if (e->port_mirror_cfg.enable)
        {
            v16 = REG_MIRROR_ENABLE;
            v16 |= (e->port_mirror_cfg.mirror_port & REG_CAPTURE_PORT_M);
            v16 |= e->port_mirror_cfg.blk_no_mrr?REG_BLK_NOT_MIRROR:0;

            SF2SW_WREG(PAGE_MANAGEMENT, REG_MIRROR_CAPTURE_CTRL, &v16, sizeof(v16));
            v16 = e->port_mirror_cfg.ing_pmap & REG_INGRESS_MIRROR_M;
            SF2SW_WREG(PAGE_MANAGEMENT, REG_MIRROR_INGRESS_CTRL, &v16, sizeof(v16));
            v16 = e->port_mirror_cfg.eg_pmap & REG_INGRESS_MIRROR_M;
            SF2SW_WREG(PAGE_MANAGEMENT, REG_MIRROR_EGRESS_CTRL, &v16, sizeof(v16));
        }
        else
        {
            v16  = 0;
            SF2SW_WREG(PAGE_MANAGEMENT, REG_MIRROR_CAPTURE_CTRL, &v16, sizeof(v16));
        }
    }

    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWPORTTRUNK functions ---
int ioctl_extsw_port_trunk_ops(struct ethswctl_data *e)
{
    /* based on impl5:bcmsw_port_trunk_get/set() */
    uint16_t v16;
    uint8_t v8;

    if (e->type == TYPE_GET) {
        SF2SW_RREG(PAGE_MAC_BASED_TRUNK, REG_TRUNK_GRP_CTL , &v16, 2);
        e->port_trunk_cfg.grp0_pmap = (v16 >> TRUNK_EN_GRP_S) & TRUNK_EN_GRP_M ;
        SF2SW_RREG(PAGE_MAC_BASED_TRUNK, REG_TRUNK_GRP_CTL+2 , &v16, 2);
        e->port_trunk_cfg.grp1_pmap = (v16 >> TRUNK_EN_GRP_S) & TRUNK_EN_GRP_M ;

        SF2SW_RREG(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        e->port_trunk_cfg.enable = (v8 >> TRUNK_EN_LOCAL_S) & TRUNK_EN_LOCAL_M;
        e->port_trunk_cfg.hash_sel = (v8 >> TRUNK_HASH_SEL_S) & TRUNK_HASH_SEL_M;
    } else {
        SF2SW_RREG(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        v8 &= ~(TRUNK_HASH_SEL_M<<TRUNK_HASH_SEL_S); /* Clear old hash selection first */
        v8 |= ( (e->port_trunk_cfg.hash_sel & TRUNK_HASH_SEL_M) << TRUNK_HASH_SEL_S ); /* Set Hash Selection */
        SF2SW_WREG(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        printk("LAG/Trunking hash selection changed <0x%01x>\n",v8);
    }

    return 0;
}


// ----------- SIOCETHSWCTLOPS ETHSWCONTROL functions ---
/*
 * Get/Set StarFighter flow control options.
 *** Input params
 * e->type  GET/SET
 * e->sub_type Flow control type
 * e->val enable/disable above flow control type
 *** Output params
 * e->val has result for GET
 * Returns 0 for Success, Negative value for failure.
 */
static int _sf2_pause_drop_ctrl(struct ethswctl_data *e)
{
    /* based on impl5:sf2_pause_drop_ctrl() */
    uint16_t val = 0;
    uint16_t val2 = 0;
    if (e->type == TYPE_SET)    { // SET
        SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_PAUSE_DROP_CTRL, &val, 2);
        switch (e->sub_type) {
            case bcmSwitchFcMode:
                val2 = e->val? FC_CTRL_MODE_PORT: 0;
                SF2SW_WREG(PAGE_FLOW_CTRL_XTN, REG_FC_CTRL_MODE, &val2, 2);
                return 0;
            case bcmSwitchQbasedpauseEn:
                val &= ~FC_QUEUE_BASED_PAUSE_EN;
                val |= e->val? FC_QUEUE_BASED_PAUSE_EN: 0;
                break;
            case bcmSwitchTxBasedFc:
                val &= ~FC_TX_BASED_CTRL_EN;
                val |= e->val? FC_TX_BASED_CTRL_EN: 0;
                val &= ~FC_RX_BASED_CTRL_EN;
                val |= e->val? 0: FC_RX_BASED_CTRL_EN;
                val &= ~FC_RX_DROP_EN;
                val |= e->val? 0: FC_RX_DROP_EN;
                break;
            case bcmSwitchTxQdropEn:
                val &= ~FC_TX_TXQ_DROP_EN;
                val |= e->val? FC_TX_TXQ_DROP_EN: 0;
                break;
            case bcmSwitchTxTotdropEn:
                val &= ~FC_TX_TOTAL_DROP_EN;
                val |= e->val? FC_TX_TOTAL_DROP_EN: 0;
                break;
            case bcmSwitchTxQpauseEn:
                val &= ~FC_TX_TXQ_PAUSE_EN;
                val |= e->val? FC_TX_TXQ_PAUSE_EN: 0;
                break;
            case bcmSwitchTxTotPauseEn:
                val &= ~FC_TX_TOTAL_PAUSE_EN;
                val |= e->val? FC_TX_TOTAL_PAUSE_EN: 0;
                break;
            case bcmSwitchTxQpauseEnImp0:
                val &= ~FC_TX_IMP0_TXQ_PAUSE_EN;
                val |= e->val? FC_TX_IMP0_TXQ_PAUSE_EN: 0;
                break;
            case bcmSwitchTxTotPauseEnImp0:
                val &= ~FC_TX_IMP0_TOTAL_PAUSE_EN;
                val |= e->val? FC_TX_IMP0_TOTAL_PAUSE_EN: 0;
                break;
            default:
                printk("%s unknown fc type %u \n", __FUNCTION__, (unsigned int)e->sub_type);
                return -BCM_E_ERROR;
                break;
        }
        SF2SW_WREG(PAGE_FLOW_CTRL_XTN, REG_FC_PAUSE_DROP_CTRL, &val, 2);
    } else {
        //   GET
        val2 = 0;
        SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_PAUSE_DROP_CTRL, &val, 2);
        val2 |= val & FC_QUEUE_BASED_PAUSE_EN? 1 << bcmSwitchQbasedpauseEn: 0;
        val2 |= val & FC_TX_BASED_CTRL_EN? 1 << bcmSwitchTxBasedFc: 0;
        val2 |= val & FC_TX_TXQ_DROP_EN? 1 << bcmSwitchTxQdropEn: 0;
        val2 |= val & FC_TX_TOTAL_DROP_EN? 1 << bcmSwitchTxTotdropEn: 0;
        val2 |= val & FC_TX_TXQ_PAUSE_EN? 1 << bcmSwitchTxQpauseEn: 0;
        val2 |= val & FC_TX_TOTAL_PAUSE_EN? 1 << bcmSwitchTxTotPauseEn: 0;
        val2 |= val & FC_TX_IMP0_TXQ_PAUSE_EN? 1 << bcmSwitchTxQpauseEnImp0: 0;
        val2 |= val & FC_TX_IMP0_TOTAL_PAUSE_EN? 1 << bcmSwitchTxTotPauseEnImp0: 0;

        SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_CTRL_MODE, &val, 2);
        val2 |= val & FC_CTRL_MODE_PORT? 1 << bcmSwitchFcMode: 0;
        e->val = val2;
        //enet_dbg("%s: val2 = 0x%x \n", __FUNCTION__, val2);
    }
    return 0;
}

int ioctl_extsw_control(struct ethswctl_data *e)
{
    /* based on impl5:bcmeapi_ioctl_extsw_control() */
    int ret = 0;
    uint8_t val8 = 0;
    unsigned int val;
    switch (e->sw_ctrl_type) {
        case bcmSwitchBufferControl:

            if ((ret = _sf2_pause_drop_ctrl(e)) >= 0) {
                if (e->type == TYPE_GET) {
                    e->ret_val = e->val;
                    //enet_dbg("e->ret_val is = %4x\n", e->ret_val);
                 }
            }
            break;

        case bcmSwitch8021QControl:
            /* Read the 802.1Q control register */
            SF2SW_RREG(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_8021Q, &val8, 1);
            if (e->type == TYPE_GET) {
                val = (val8 >> VLAN_EN_8021Q_S) & VLAN_EN_8021Q_M;
                if (val && ((val8 >> VLAN_IVL_SVL_S) & VLAN_IVL_SVL_M))
                    val = 2; // IVL mode
                e->val  = val;
                //enet_dbg("e->val is = %4x\n", e->val);
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
                SF2SW_WREG(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_8021Q, &val8, 1);
            }
            break;

        default:
            //up(&bcm_ethlock_switch_config);
            ret = -BCM_E_PARAM;
            break;
    } //switch
    return ret;
}


// ----------- SIOCETHSWCTLOPS ETHSWPRIOCONTROL functions ---
/*
 * Get/Set StarFighter switch Flowcontrol thresholds.
 *** Input params
 * e->type  GET/SET
 * e->sw_ctrl_type buffer threshold type
 * e->port that determines LAN/IMP0/IMP1 to pick the register set
 * e->val  buffer threshold value to write
 *** Output params
 * e->val has buffer threshold value read for GET
 * Returns 0 for Success, Negative value for failure.
 */
static int _sf2_prio_control(struct ethswctl_data *e)
{
    /* based on impl5:sf2_prio_control() */
    uint16_t val16;
    int reg = 0, page, curThreConfMode, curMaxStreamNumber;

    down(&sf2_sw->s.conf_sem);
    switch (e->sw_ctrl_type)
    {
        case bcmSwitchTxQThresholdConfigMode:
        case bcmSwitchTotalPorts:
        case bcmSwitchLinkUpLanPorts:
        case bcmSwitchLinkUpWanPorts:
        case bcmSwitchMaxStreams:
        case bcmSwitchCurStreams:
        case bcmSwitchActQuePerPort:
            break;
        case bcmSwitchTxQHiReserveThreshold:
            reg = REG_FC_LAN_TXQ_THD_RSV_QN0 + (e->priority * 2);
            break;
        case bcmSwitchTxQHiHysteresisThreshold:
            reg = REG_FC_LAN_TXQ_THD_HYST_QN0 + (e->priority * 2);
            break;
        case bcmSwitchTxQHiPauseThreshold:
            reg = REG_FC_LAN_TXQ_THD_PAUSE_QN0 + (e->priority * 2);
            break;
        case bcmSwitchTxQHiDropThreshold:
            reg = REG_FC_LAN_TXQ_THD_DROP_QN0 + (e->priority * 2);
            break;
        case bcmSwitchTotalHysteresisThreshold:
            reg = REG_FC_LAN_TOTAL_THD_HYST_QN0 + (e->priority * 2);
            break;
        case bcmSwitchTotalPauseThreshold:
            reg = REG_FC_LAN_TOTAL_THD_PAUSE_QN0 + (e->priority * 2);
            break;
        case bcmSwitchTotalDropThreshold:
            reg = REG_FC_LAN_TOTAL_THD_DROP_QN0 + (e->priority * 2);
            break;
        default:
            enet_err("Unknown threshold type \n");
            up(&sf2_sw->s.conf_sem);
            return -BCM_E_PARAM;
    }

    if(e->port == SF2_IMP0_PORT)
    {
        page = PAGE_FC_IMP0_TXQ;
    } else if(e->port == SF2_WAN_IMP1_PORT) {
        page = PAGE_FC_IMP1_TXQ;
    } else if ((e->port < SF2_IMP0_PORT) && (e->port != SF2_INEXISTANT_PORT)) {
        page = PAGE_FC_LAN_TXQ;
    } else {
        enet_err("port # %d error \n", e->port);
        up(&sf2_sw->s.conf_sem);
        return -BCM_E_PARAM;
    }

    //enet_dbg("Threshold: page %d  register offset = %#4x", page, reg);
    /* select port if port based threshold configuration in force */
    if (page == PAGE_FC_LAN_TXQ)
    {
        SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_CTRL_MODE, &val16, 2);
        if (val16 & FC_CTRL_MODE_PORT) {
            /* port number to port select register */
            val16 = 1 << (REG_FC_CTRL_PORT_P0 + e->port);
            SF2SW_WREG(PAGE_FLOW_CTRL_XTN, REG_FC_CTRL_PORT_SEL, &val16, 2);
        }
    }

    if (e->type == TYPE_GET)
    {
        switch (e->sw_ctrl_type)
        {
            case bcmSwitchTxQThresholdConfigMode:
                e->val = queThreConfMode;
                break;
            case bcmSwitchTotalPorts:
                e->val = sf2_nonimp_port_cnt() + sf2_imp_port_cnt();
                break;
            case bcmSwitchLinkUpLanPorts:
                e->val = sf2LanUpPorts_g;
                break;
            case bcmSwitchLinkUpWanPorts:
                e->val = sf2WanUpPorts_g;
                break;
            case bcmSwitchMaxStreams:
                e->val = maxStreamNumber;
                break;
            case bcmSwitchCurStreams:
                e->val = curStreams;
                break;
            case bcmSwitchActQuePerPort:
                e->val = active_queue_number_per_port;
                break;
            default:
                SF2SW_RREG(page, reg, &val16, 2);
                //enet_dbg("Threshold read = %4x", val16);
                e->val = val16;
        }
    }
    else
    {
        switch (e->sw_ctrl_type)
        {
            case bcmSwitchTxQThresholdConfigMode:
                curThreConfMode = queThreConfMode;
                queThreConfMode = (uint32_t)e->val;
                if (curThreConfMode != queThreConfMode && queThreConfMode != ThreModeManual )
                {
                    _sf2_conf_que_thred();
                }
                break;
            case bcmSwitchMaxStreams:
                curMaxStreamNumber = maxStreamNumber;
                maxStreamNumber = e->val;
                if (maxStreamNumber == -1) maxStreamNumber = MaxStreamNumber;
                if (curMaxStreamNumber != maxStreamNumber && queThreConfMode != ThreModeManual )
                {
                    _sf2_conf_que_thred();
                }
                break;
            case bcmSwitchActQuePerPort:
                if(active_queue_number_per_port != e->val)
                {
                    active_queue_number_per_port = e->val;
                    _sf2_conf_que_thred();
                }
                break;
            default:
                val16 = (uint32_t)e->val;
                //ent_dbg("e->val is = %4x", e->val);
                SF2SW_WREG(page, reg, &val16, 2);
        }
    }
    up(&sf2_sw->s.conf_sem);
    return 0;
}

int ioctl_extsw_prio_control(struct ethswctl_data *e)
{
    /* based on impl5:bcmeapi_ioctl_extsw_prio_control() */
    int ret = 0;
    if ((ret =  _sf2_prio_control(e)) >= 0) {
        if (e->type == TYPE_GET) {
            e->ret_val = e->val;
            //enet_dbg("e->ret_val is = %4x", e->ret_val);
        }
    }
    return ret;
}

// ----------- SIOCETHSWCTLOPS ETHSWQUEMAP functions ---
#if defined(ARCHER_DEVICE)
static int tr_port_q_map(enetx_port_t *port, void *ctxt)
{
    uint32_t *queRemap = (uint32_t *)ctxt;
    bcmFun_t *enet_sysport_q_map = bcmFun_get(BCM_FUN_ID_ENET_SYSPORT_QUEUE_MAP);
    int ndx;
    bcmSysport_QueueMap_t map;

    if (!port->has_interface) return 0;

    if (!enet_sysport_q_map)
        return -EOPNOTSUPP;

    map.blog_chnl = port->n.blog_chnl;

    for (ndx = 0; ndx < BCM_ENET_SYSPORT_QUEUE_MAP_PRIORITY_MAX; ndx++) {
#if defined(PORT_WITH_8TXQ)
        if ((port->p.mac->mac_id == PORT_WITH_8TXQ) &&
            (port->n.port_netdev_role == PORT_NETDEV_ROLE_WAN))
            map.priority_to_switch_queue[ndx] = ndx;
        else
#endif
            map.priority_to_switch_queue[ndx] = (*queRemap >> (ndx *4)) & 0xf;
    }

    return enet_sysport_q_map(&map) ? -EINVAL : 0;
}

static int bcmenet_sysport_q_map(uint32_t queRemap)
{
    uint32_t map = queRemap;
    return port_traverse_ports(root_sw, tr_port_q_map, PORT_CLASS_PORT, &map);
}
#endif // ARCHER_DEVICE

int ioctl_extsw_que_map(struct ethswctl_data *e)
{
    /* based on impl5:bcmeapi_ioctl_que_map() */
    if (e->type == TYPE_SET) {
        if (e->val != -1) {
            wanQueMap = e->val;
            config_wan_queue_acb_map();
        }
        if (e->priority != -1) {
#if defined(ARCHER_DEVICE)
            int res = bcmenet_sysport_q_map(e->priority);
            if (res) return res;
#endif
            queRemap = e->priority;
        }
        _sf2_conf_que_thred();
    }
    e->val = wanQueMap;
    e->priority = queRemap;

    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWQUEMON functions ---
static void _check_que_mon_port(int port)
{
    /* based on impl5: check_que_mon_port() */
    static uint16 last_port = -1;

    if (last_port == port) return;
    last_port = port;
    SF2SW_WREG(PAGE_FLOW_CTRL_XTN, REG_FC_DIAG_CTRL, &last_port, 2);
    return;
}

int ioctl_extsw_que_mon(struct ethswctl_data *e)
{
    /* based on impl5:bcmeapi_ioctl_que_mon() */
    int port = e->port,
        que = e->priority,
        type = e->sw_ctrl_type,
        val, err = 0;
    uint16_t v16 = 0;

    switch(type)
    {
        case QUE_CUR_COUNT:
            _check_que_mon_port(port);
            SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_QUE_CUR_COUNT + que*2, &v16, 2);
            break;
        case QUE_PEAK_COUNT:
            _check_que_mon_port(port);
            SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_QUE_PEAK_COUNT + que*2, &v16, 2);
            break;
        case SYS_TOTAL_PEAK_COUNT:
            SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_SYS_TOTAL_PEAK_COUNT, &v16, 2);
            break;
        case SYS_TOTAL_USED_COUNT:
            SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_SYS_TOTAL_USED_COUNT, &v16, 2);
            break;

        case PORT_PEAK_RX_BUFFER:
            _check_que_mon_port(port);
            SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_PORT_PEAK_RX_BUFFER, &v16, 2);
            break;
        case QUE_FINAL_CONGESTED_STATUS:
            SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_QUE_FINAL_CONG_STAT + 2*port, &v16, 2);
            break;
        case PORT_PAUSE_HISTORY:
            SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_PORT_PAUSE_HISTORY, &v16, 2);
            break;
        case PORT_PAUSE_QUAN_HISTORY:
            SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_PORT_PAUSE_QUAN_HISTORY, &v16, 2);
            break;

        case PORT_RX_BASE_PAUSE_HISTORY:
            SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_PORT_RXBASE_PAUSE_HISTORY, &v16, 2);
            break;
        case PORT_RX_BUFFER_ERROR_HISTORY:
            SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_PORT_RX_BUFFER_ERR_HISTORY, &v16, 2);
            break;
        case QUE_CONGESTED_STATUS:
            SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_QUE_CONG_STATUS + 2*port, &v16, 2);
            break;
        case QUE_TOTAL_CONGESTED_STATUS:
            SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_QUE_TOTAL_CONG_STATUS + 2*port, &v16, 2);
            break;
    }

    val = v16;
    e->val = val;
    e->ret_val = err;
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWMACLMT functions ---
int ioctl_extsw_maclmt(struct ethswctl_data *e)
{
    int port = e->port,
        type = e->sw_ctrl_type;
    uint32_t val32;

    if (type < GLOBAL_LIMIT && (port < 0 || port > SF2_IMP0_PORT) )
        return -BCM_E_PARAM;
    if (e->type == TYPE_SET) {      // SET
        switch(type) {
        case PORT_LIMIT_EN:
            SF2SW_RREG(PAGE_SA_LIMIT, REG_SA_LIMIT_EN, &val32, 4);
            val32 = (e->val)? val32 | (1<<port) : val32 & ~(1<<port);
            SF2SW_WREG(PAGE_SA_LIMIT, REG_SA_LIMIT_EN, &val32, 4);
            break;
        case PORT_LIMIT:
            SF2SW_RREG(PAGE_SA_LIMIT, REG_SA_LIMIT_CTRL_P(port), &val32, 4);
            val32 &= ~(MACLMT_LIMIT_M <<MACLMT_LIMIT_S);
            val32 |= (e->val & MACLMT_LIMIT_M) << MACLMT_LIMIT_S;
            SF2SW_WREG(PAGE_SA_LIMIT, REG_SA_LIMIT_CTRL_P(port), &val32, 4);
            break;
       case PORT_ACTION:
            SF2SW_RREG(PAGE_SA_LIMIT, REG_SA_LIMIT_CTRL_P(port), &val32, 4);
            val32 &= ~(MACLMT_ACTION_M <<MACLMT_ACTION_S);
            val32 |= (e->val & MACLMT_ACTION_M) << MACLMT_ACTION_S;
            SF2SW_WREG(PAGE_SA_LIMIT, REG_SA_LIMIT_CTRL_P(port), &val32, 4);
            break;
        case PORT_RST_OVER_LIMIT_PKT_COUNT:
            val32 = 1 << port;
            SF2SW_WREG(PAGE_SA_LIMIT, REG_SA_OVRLIMIT_CNTR_RST, &val32, 4);
            break;
        case GLOBAL_RST_OVER_LIMIT_PKT_COUNT:
            val32 = (1 << SF2_IMP0_PORT)-1;
            SF2SW_WREG(PAGE_SA_LIMIT, REG_SA_OVRLIMIT_CNTR_RST, &val32, 4);
            break;
        case GLOBAL_LIMIT:
            SF2SW_RREG(PAGE_SA_LIMIT, REG_TOTAL_SA_LIMIT_CTL, &val32, 4);
            val32 &= ~(MACLMT_LIMIT_M <<MACLMT_LIMIT_S);
            val32 |= (e->val & MACLMT_LIMIT_M) << MACLMT_LIMIT_S;
            SF2SW_WREG(PAGE_SA_LIMIT, REG_TOTAL_SA_LIMIT_CTL, &val32, 4);
            break;
        default: return -BCM_E_PARAM;
        }
        
    } else {                        // GET
        switch(type) {
        case PORT_LIMIT_EN:
            SF2SW_RREG(PAGE_SA_LIMIT, REG_SA_LIMIT_EN, &val32, 4);
            e->val = (val32 & 1<<port) ? 1:0;
            break;
        case PORT_LIMIT:
            SF2SW_RREG(PAGE_SA_LIMIT, REG_SA_LIMIT_CTRL_P(port), &val32, 4);
            e->val = (val32 >> MACLMT_LIMIT_S) & MACLMT_LIMIT_M;
            break;
        case PORT_ACTION:
            SF2SW_RREG(PAGE_SA_LIMIT, REG_SA_LIMIT_CTRL_P(port), &val32, 4);
            e->val = (val32 >> MACLMT_ACTION_S) & MACLMT_ACTION_M;
            break;
        case PORT_LEARNED_COUNT:
            SF2SW_RREG(PAGE_SA_LIMIT, REG_SA_LRN_CNTR_P(port), &val32, 4);
            e->val = (val32 >> MACLMT_LIMIT_S) & MACLMT_LIMIT_M;
            break;
        case PORT_OVER_LIMIT_PKT_COUNT:
            SF2SW_RREG(PAGE_SA_LIMIT, REG_SA_OVRLIMIT_CNTR_P(port), &val32, 4);
            e->val = val32;
            break;
        case GLOBAL_LIMIT:
            SF2SW_RREG(PAGE_SA_LIMIT, REG_TOTAL_SA_LIMIT_CTL, &val32, 4);
            e->val = (val32 >> MACLMT_LIMIT_S) & MACLMT_LIMIT_M;
            break;
        case GLOBAL_LEARNED_COUNT:
            SF2SW_RREG(PAGE_SA_LIMIT, REG_TOTAL_SA_LRN_CNTR, &val32, 4);
            e->val = (val32 >> MACLMT_LIMIT_S) & MACLMT_LIMIT_M;
            break;
        default: return -BCM_E_PARAM;
        }
    }
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWPBVLAN functions ---
int ioctl_extsw_pbvlan(struct ethswctl_data *e)
{
    /* based on impl5:bcmeapi_ioctl_extsw_pbvlan() */
    uint16_t val16;

    //enet_dbg("Given Port: 0x%02x \n ", e->port);
    if (e->port >= TOTAL_SWITCH_PORTS) {
        printk("Invalid Switch Port \n");
        return BCM_E_ERROR;
    }

    if (e->type == TYPE_GET) {
        down(&sf2_sw->s.conf_sem);
        SF2SW_RREG(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (e->port*2), &val16, 2);
        up(&sf2_sw->s.conf_sem);
        e->fwd_map = val16;
        //enet_dbg("get e->fwd_map is = %4x\n", e->fwd_map);
    } else {
        val16 = (uint32_t)e->fwd_map;
        //enet_dbg("set e->fwd_map is = %4x\n", e->fwd_map);
        down(&sf2_sw->s.conf_sem);
        SF2SW_WREG(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (e->port*2), &val16, 2);
        up(&sf2_sw->s.conf_sem);
    }

    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWCOSPORTMAP functions ---
/*
 * Get/Set cos(queue) mapping, given priority (TC)
 *** Input params
 * e->type  GET/SET
 * e->queue - target queue
 * e->port  per port
 *** Output params
 * Returns 0 for Success, Negative value for failure.
 */
int ioctl_extsw_cosq_port_mapping(struct ethswctl_data *e)
{
    /* based on impl5:bcmeapi_ioctl_extsw_cosq_port_mapping() */
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

    //enet_dbg("Given port: %02d priority: %02d \n ", e->port, e->priority);
    if (e->port >= TOTAL_SWITCH_PORTS || e->port == SF2_INEXISTANT_PORT) {
        printk("Invalid Switch Port %02d \n", e->port);
        return -BCM_E_ERROR;
    }
    if ((e->priority > MAX_PRIORITY_VALUE) || (e->priority < 0)) {
        printk("Invalid Priority \n");
        return -BCM_E_ERROR;
    }
    reg_addr  = SF2_REG_PORTN_TC_TO_COS + e->port * 4;
    cos_shift = SF2_QOS_COS_SHIFT;
    cos_mask  = SF2_QOS_COS_MASK;
    reg_len   = 4;

    down(&sf2_sw->s.conf_sem);

    if (e->type == TYPE_GET) {
        SF2SW_RREG(PAGE_QOS, reg_addr, (uint8_t *)&val, reg_len);
        //enet_dbg("REG_QOS_PORT_PRIO_MAP_Px = %px\n", (void*)&val);
        /* Get the queue */
        queue = (val.val32 >> (e->priority * cos_shift)) & cos_mask;
        retval = queue & SF2_QOS_COS_MASK;
        //enet_dbg("%s queue is = %4x\n", __FUNCTION__, retval);
    } else {
        //enet_dbg("Given queue: 0x%02x \n ", e->queue);
        SF2SW_RREG(PAGE_QOS, reg_addr, (uint8_t *)&val, reg_len);
        val.val32 &= ~(cos_mask << (e->priority * cos_shift));
        val.val32 |= (e->queue & cos_mask) << (e->priority * cos_shift);
        SF2SW_WREG(PAGE_QOS, reg_addr, (uint8_t *)&val, reg_len);
    }
    up(&sf2_sw->s.conf_sem);
    return retval;
}

#if !defined(SF2_EXTERNAL)
// ----------- SIOCETHSWCTLOPS ETHSWACBCONTROL functions ---
#if defined(ACB_ALGORITHM2) && !defined(CONFIG_BCM963178)
static void _sf2_set_acb_algorithm(int algorithm)
{
    volatile uint32_t *sf2_acb_control    = (void *) (SF2_ACB_CONTROL_REG);
    volatile uint32_t *sf2_acb_que_config = (void *) (sf2_acb_control + 2);
    volatile uint32_t *sf2_acb_que_in_flight = (void *)(SF2_ACB_QUE0_PKTS_IN_FLIGHT);
    uint32_t val32, q;

    if (algorithm) // ACB_ALGORITHM2
    {
        *sf2_acb_control &= ~(SF2_ACB_EN|(SF2_ACB_ALGORITHM_M<<SF2_ACB_ALGORITHM_S)|
                (SF2_ACB_FLUSH_M<<SF2_ACB_FLUSH_S)|(SF2_ACB_EOP_DELAY_M<<SF2_ACB_EOP_DELAY_S));

        for (q = 0; q <= SF2_ACB_QUE_MAX; q++)
        {
            val32 = *(sf2_acb_que_config + q);
            val32 &= ~((SF2_ACB_QUE_PESSIMISTIC_M << SF2_ACB_QUE_PESSIMISTIC_S)|
                    (SF2_ACB_QUE_PKT_LEN_M<<SF2_ACB_QUE_PKT_LEN_S));
            *(sf2_acb_que_config + q) =  val32;
            *(sf2_acb_que_in_flight + q) = 0;
        }

        *sf2_acb_control |= SF2_ACB_EN|(SF2_ACB_ALGORITHM_M<<SF2_ACB_ALGORITHM_S)|
            (0x32<<SF2_ACB_EOP_DELAY_S);
    }
    else
    {
        *sf2_acb_control &= ~(SF2_ACB_EN|(SF2_ACB_ALGORITHM_M<<SF2_ACB_ALGORITHM_S)|
                (SF2_ACB_FLUSH_M<<SF2_ACB_FLUSH_S)|(SF2_ACB_EOP_DELAY_M<<SF2_ACB_EOP_DELAY_S));
        *sf2_acb_control &= ~SF2_ACB_EN;
        for (q = 0; q <= SF2_ACB_QUE_MAX; q++)
        {
            val32 = *(sf2_acb_que_config + q);
            val32 &= ~(SF2_ACB_QUE_PKT_LEN_M << SF2_ACB_QUE_PKT_LEN_S);
            val32 |= 6 << SF2_ACB_QUE_PKT_LEN_S;
            val32 |= (SF2_ACB_QUE_PESSIMISTIC_M << SF2_ACB_QUE_PESSIMISTIC_S);
            *(sf2_acb_que_config + q) =  val32;
            *(sf2_acb_que_in_flight + q) = 0;
        }
        *sf2_acb_control |= SF2_ACB_EN;
    }
}
#endif /* defined(ACB_ALGORITHM2) */

static void dump_sf2_fc_info(void)
{
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
    volatile uint32_t *sf2_acb_que_in_flight = (void *)(SF2_ACB_QUE0_PKTS_IN_FLIGHT);
#endif
    uint32_t diag_ctrl, val, i, j;
    SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_DIAG_CTRL, &diag_ctrl, 4);  // save current diag control

    printk("Note: Congested Status fields: F-final(@0a-60) C-congest(@0a-80) T-total(@0a-9a)\n"
           "                       level: .< reserved_thd < R < hyst_thd < pause_thd < P < drop_thd < D\n\n");
    SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_SYS_TOTAL_PEAK_COUNT, &i, 4);
    SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_SYS_TOTAL_USED_COUNT, &j, 4);
    printk(
#if defined(ACB_ALGORITHM2)
           "        in flight         |    mon TX    total=%4d|    peak TX        total=%4d| Congested Status FCT\n"
           "   q0 q1 q2 q3 q4 q5 q6 q7| q0 q1 q2 q3 q4 q5 q6 q7| q0 q1 q2 q3 q4 q5 q6 q7 port| q0  q1  q2  q3  q4  q5  q6  q7\n",
#else
           "        mon TX    total=%4d|    peak TX        total=%4d| Congested Status FCT\n"
           "   q0 q1 q2 q3 q4 q5 q6 q7| q0 q1 q2 q3 q4 q5 q6 q7 port| q0  q1  q2  q3  q4  q5  q6  q7\n",
#endif
           j & 0x7ff, i & 0x7ff);

    for (i = 0; i <= 8; i++)
    {
        uint32_t final, congest, total;
#if defined(ACB_ALGORITHM2)
        uint32_t inflight[8];
#endif
        uint32_t mon[8];

        SF2SW_WREG(PAGE_FLOW_CTRL_XTN, REG_FC_DIAG_CTRL, &i, 4);
        printk("p%d", i);
        // read in flight & mon values right next to each other to get more accurate values
        for (j = 0; j < 8; j++)
        {
#if defined(ACB_ALGORITHM2)
            if (i < 8)
                inflight[j]= *(sf2_acb_que_in_flight + i*8 + j);
#endif
            SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_QUE_CUR_COUNT+j*2, &(mon[j]), 4);
        }
#if defined(ACB_ALGORITHM2)
        // dump in flight regs
        for (j = 0; j < 8; j++)
        {
            if (i<8)
            {
                pr_cont("%3d", inflight[j] & 0x7ff);
            }
            else
                pr_cont("   ");
        }
        pr_cont("|");
#endif
        // dump monitor tx regs
        for (j = 0; j < 8; j++)
        {
            pr_cont("%3d", mon[j] & 0x7ff);
        }
        pr_cont("|");
        // dump peak tx regs
        for (j = 0; j < 8; j++)
        {
            SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_QUE_PEAK_COUNT+j*2, &val, 4);
            pr_cont("%3d", val & 0x7ff);
        }
        SF2SW_RREG(PAGE_FLOW_CTRL_XTN, REG_FC_PORT_PEAK_RX_BUFFER, &val, 4);
        pr_cont(" %4d|", val & 0x7ff);

        SF2SW_RREG(PAGE_FLOW_CTRL_XTN,REG_FC_QUE_FINAL_CONG_STAT+i*2, &final, 4);
        SF2SW_RREG(PAGE_FLOW_CTRL_XTN,REG_FC_QUE_CONG_STATUS+i*2, &congest, 4);
        SF2SW_RREG(PAGE_FLOW_CTRL_XTN,REG_FC_QUE_TOTAL_CONG_STATUS+i*2, &total, 4);
        for (j = 0; j < 8; j++)
        {
            char *lvl_str=".RPD";
            #define LVL_CHAR(v,q) lvl_str[(v>>q*2) & 3]
            pr_cont("%c%c%c ", LVL_CHAR(final,j), LVL_CHAR(congest,j), LVL_CHAR(total,j));
        }
        pr_cont("\n");
    }

    SF2SW_WREG(PAGE_FLOW_CTRL_XTN, REG_FC_DIAG_CTRL, &diag_ctrl, 4);  // restore current diag control
}

int ioctl_extsw_cfg_acb(struct ethswctl_data *e)
{
    /* based on impl5:sf2_config_acb() */
    uint32_t val32, val;
    acb_q_params_t acb_conf_info;
    volatile uint32_t *sf2_acb_control    = (void *) (SF2_ACB_CONTROL_REG);
    volatile uint32_t *sf2_acb_xon_thresh = (void *) (sf2_acb_control + 1);
    volatile uint32_t *sf2_acb_que_config = (void *) (sf2_acb_control + 2);
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
    volatile uint32_t *sf2_acb_que_in_flight = (void *)(SF2_ACB_QUE0_PKTS_IN_FLIGHT);
#endif

    if (e->queue < 0 || e->queue > SF2_ACB_QUE_MAX) {
        printk("%s parameter error, queue 0x%x \n", 	__FUNCTION__, e->queue);
        return BCM_E_PARAM;
    }
    val   = *sf2_acb_xon_thresh;
    val32 = *(sf2_acb_que_config + e->queue);
    if (e->type == TYPE_GET) {
        switch (e->sw_ctrl_type) {
            case acb_en:
                acb_conf_info.acb_en =  *sf2_acb_control & SF2_ACB_EN;
                break;
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
            case acb_eop_delay:
                acb_conf_info.eop_delay =  (*sf2_acb_control >> SF2_ACB_EOP_DELAY_S) & SF2_ACB_EOP_DELAY_M;
                break;
            case acb_flush:
                acb_conf_info.flush =  (*sf2_acb_control >> SF2_ACB_FLUSH_S) & SF2_ACB_FLUSH_M;
                break;
            case acb_algorithm:
#if !defined(CONFIG_BCM963178)
                acb_conf_info.algorithm =  (*sf2_acb_control >> SF2_ACB_ALGORITHM_S) & SF2_ACB_ALGORITHM_M;
#else
                acb_conf_info.algorithm = 1;
#endif
                break;
#endif
            case acb_tot_xon_hyst:
                acb_conf_info.total_xon_hyst =  (val >> SF2_ACB_TOTAL_XON_BUFS_S)
                    & SF2_ACB_BUFS_THRESH_M;
                break;
            case acb_xon_hyst:
                acb_conf_info.xon_hyst =  (val >> SF2_ACB_XON_BUFS_S)
                                                        & SF2_ACB_BUFS_THRESH_M;
                break;
            case acb_q_pessimistic_mode:
                acb_conf_info.acb_queue_config.pessimistic_mode = (val32  >> SF2_ACB_QUE_PESSIMISTIC_S)
                                                        & SF2_ACB_QUE_PESSIMISTIC_M;
                break;
            case acb_q_total_xon_en:
                acb_conf_info.acb_queue_config.total_xon_en = (val32  >> SF2_ACB_QUE_TOTAL_XON_S)
                                                         & SF2_ACB_QUE_TOTAL_XON_M;
                break;
            case acb_q_xon_en:
                acb_conf_info.acb_queue_config.xon_en = (val32  >> SF2_ACB_QUE_XON_S)
                                                         & SF2_ACB_QUE_XON_M;
                break;
            case acb_q_total_xoff_en:
                acb_conf_info.acb_queue_config.total_xoff_en = (val32  >> SF2_ACB_QUE_TOTAL_XOFF_S)
                                                         & SF2_ACB_QUE_TOTAL_XOFF_M;
                break;
            case acb_q_pkt_len:
                acb_conf_info.acb_queue_config.pkt_len = (val32  >> SF2_ACB_QUE_PKT_LEN_S)
                                                         & SF2_ACB_QUE_PKT_LEN_M;
                break;
            case acb_q_tot_xoff_thresh:
                acb_conf_info.acb_queue_config.total_xoff_threshold = (val32  >> SF2_ACB_QUE_TOTOAL_XOFF_BUFS_S)
                                                         & SF2_ACB_BUFS_THRESH_M;
                break;
            case acb_q_xoff_thresh:
                acb_conf_info.acb_queue_config.xoff_threshold = (val32  >> SF2_ACB_QUE_XOFF_BUFS_S)
                                                         & SF2_ACB_BUFS_THRESH_M;
                break;
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
            case acb_q_pkts_in_flight:
                acb_conf_info.pkts_in_flight = *(sf2_acb_que_in_flight + e->queue) & SF2_ACB_QUE_PKTS_IN_FLIGHT_M;
                break;
#endif
            case acb_parms_all:
                acb_conf_info.acb_en =  *sf2_acb_control & SF2_ACB_EN;
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
                acb_conf_info.eop_delay =  (*sf2_acb_control >> SF2_ACB_EOP_DELAY_S) & SF2_ACB_EOP_DELAY_M;
                acb_conf_info.flush =  (*sf2_acb_control >> SF2_ACB_FLUSH_S) & SF2_ACB_FLUSH_M;
#if !defined(CONFIG_BCM963178)
                acb_conf_info.algorithm =  (*sf2_acb_control >> SF2_ACB_ALGORITHM_S) & SF2_ACB_ALGORITHM_M;
#else
                acb_conf_info.algorithm = 1;
#endif
#endif
                acb_conf_info.total_xon_hyst =  (val >> SF2_ACB_TOTAL_XON_BUFS_S)
                                                        & SF2_ACB_BUFS_THRESH_M;
                acb_conf_info.xon_hyst =  (val >> SF2_ACB_XON_BUFS_S)
                                                        & SF2_ACB_BUFS_THRESH_M;
                acb_conf_info.acb_queue_config.pessimistic_mode = (val32  >> SF2_ACB_QUE_PESSIMISTIC_S)
                                                        & SF2_ACB_QUE_PESSIMISTIC_M;
                acb_conf_info.acb_queue_config.total_xon_en = (val32  >> SF2_ACB_QUE_TOTAL_XON_S)
                                                         & SF2_ACB_QUE_TOTAL_XON_M;
                acb_conf_info.acb_queue_config.xon_en = (val32  >> SF2_ACB_QUE_XON_S)
                                                         & SF2_ACB_QUE_XON_M;
                acb_conf_info.acb_queue_config.total_xoff_en = (val32  >> SF2_ACB_QUE_TOTAL_XOFF_S)
                                                         & SF2_ACB_QUE_TOTAL_XOFF_M;
                acb_conf_info.acb_queue_config.pkt_len = (val32  >> SF2_ACB_QUE_PKT_LEN_S)
                                                         & SF2_ACB_QUE_PKT_LEN_M;
                acb_conf_info.acb_queue_config.total_xoff_threshold = (val32  >> SF2_ACB_QUE_TOTOAL_XOFF_BUFS_S)
                                                         & SF2_ACB_BUFS_THRESH_M;
                acb_conf_info.acb_queue_config.xoff_threshold = (val32  >> SF2_ACB_QUE_XOFF_BUFS_S)
                                                         & SF2_ACB_BUFS_THRESH_M;
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
                acb_conf_info.pkts_in_flight = *(sf2_acb_que_in_flight + e->queue) & SF2_ACB_QUE_PKTS_IN_FLIGHT_M;
#endif
                break;
            case 100:
                dump_sf2_fc_info();
                break;
            default:
                printk("%s: Get op %#x Unsupported \n", __FUNCTION__, e->sw_ctrl_type);
                return BCM_E_PARAM;
                break;
        }
        if (copy_to_user (e->vptr, &acb_conf_info, sizeof(acb_q_params_t))) {
            return -EFAULT;
        }
    } else {  // SET
        switch (e->sw_ctrl_type) {
            case acb_en:
                if (e->val)
                    *sf2_acb_control |= SF2_ACB_EN;
                else
                    *sf2_acb_control &= ~SF2_ACB_EN;
                return 0;
                break;
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
            case acb_eop_delay:
                *sf2_acb_control &= ~SF2_ACB_EN;
                *sf2_acb_control &= ~(SF2_ACB_EOP_DELAY_M << SF2_ACB_EOP_DELAY_S);
                *sf2_acb_control |= (e->val & SF2_ACB_EOP_DELAY_M) << SF2_ACB_EOP_DELAY_S;
                *sf2_acb_control |= SF2_ACB_EN;
                return 0;
            case acb_flush:
                *sf2_acb_control &= ~SF2_ACB_EN;
                *sf2_acb_control &= ~(SF2_ACB_FLUSH_M << SF2_ACB_FLUSH_S);
                *sf2_acb_control |= (e->val & SF2_ACB_FLUSH_M) << SF2_ACB_FLUSH_S;
                *sf2_acb_control |= SF2_ACB_EN;
                return 0;
            case acb_algorithm:
#if !defined(CONFIG_BCM963178)
                _sf2_set_acb_algorithm(e->val);
#endif
                return 0;
#endif
            case acb_tot_xon_hyst:
                *sf2_acb_control &= ~SF2_ACB_EN;
                *sf2_acb_xon_thresh = val | (e->val & SF2_ACB_BUFS_THRESH_M) << SF2_ACB_TOTAL_XON_BUFS_S;
                *sf2_acb_control |= SF2_ACB_EN;
                return 0;
                break;
            case acb_xon_hyst:
                *sf2_acb_control &= ~SF2_ACB_EN;
                *sf2_acb_xon_thresh = val | (e->val & SF2_ACB_BUFS_THRESH_M) << SF2_ACB_XON_BUFS_S;
                *sf2_acb_control |= SF2_ACB_EN;
                return 0;
                break;
            case acb_q_pessimistic_mode:
                val32 &= ~(SF2_ACB_QUE_PESSIMISTIC_M << SF2_ACB_QUE_PESSIMISTIC_S);
                val32 |= (e->val & SF2_ACB_QUE_PESSIMISTIC_M) << SF2_ACB_QUE_PESSIMISTIC_S;
                break;
            case acb_q_total_xon_en:
                val32 &= ~(SF2_ACB_QUE_TOTAL_XON_M << SF2_ACB_QUE_TOTAL_XON_S);
                val32 |= (e->val & SF2_ACB_QUE_TOTAL_XON_M) << SF2_ACB_QUE_TOTAL_XON_S;
                break;
            case acb_q_xon_en:
                val32 &= ~(SF2_ACB_QUE_XON_M << SF2_ACB_QUE_XON_S);
                val32 |= (e->val & SF2_ACB_QUE_XON_M) << SF2_ACB_QUE_XON_S;
                break;
            case acb_q_total_xoff_en:
                val32 &= ~(SF2_ACB_QUE_TOTAL_XOFF_M << SF2_ACB_QUE_TOTAL_XOFF_S);
                val32 |= (e->val & SF2_ACB_QUE_TOTAL_XON_M) << SF2_ACB_QUE_TOTAL_XON_S;
                break;
            case acb_q_pkt_len:
                val32 |=  (e->val & SF2_ACB_QUE_PKT_LEN_M) << SF2_ACB_QUE_PKT_LEN_S;
                break;
            case acb_q_tot_xoff_thresh:
                val32 |=  (e->val & SF2_ACB_BUFS_THRESH_M) << SF2_ACB_QUE_TOTOAL_XOFF_BUFS_S;
                break;
            case acb_q_xoff_thresh:
                val32 |=  (e->val & SF2_ACB_BUFS_THRESH_M) << SF2_ACB_QUE_XOFF_BUFS_S;
                break;
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
            case acb_q_pkts_in_flight:
                printk("Warning: This register should only be set by HW, but set here.\n");
                *sf2_acb_control &= ~SF2_ACB_EN;
                *(sf2_acb_que_in_flight + e->queue) = e->val & SF2_ACB_QUE_PKTS_IN_FLIGHT_M;
                *sf2_acb_control |= SF2_ACB_EN;
                return BCM_E_PARAM;
#endif
            default:
                printk("%s: Set op %#x Unsupported \n", __FUNCTION__, e->sw_ctrl_type);
                return BCM_E_PARAM;
                break;
        }
        *sf2_acb_control &= ~SF2_ACB_EN;
        *(sf2_acb_que_config + e->queue) = val32;
        *sf2_acb_control |= SF2_ACB_EN;
        return 0;
    }// Set
    return 0;
}
#endif //!SF2_EXTERNAL

// ----------- SIOCETHSWCTLOPS ETHSWVLAN functions ---
static int _read_vlan_table(bcm_vlan_t vid, uint32_t *val32)
{
    uint8_t val8;
    int i, timeout = 200;

    SF2SW_WREG(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_INDX_531xx, (uint8_t *)&vid, 2);
    val8 = 0x81;
    SF2SW_WREG(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL_531xx, (uint8_t *)&val8, 1);

    for (i = 0; i < timeout; i++) {
        SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL_531xx, (uint8_t *)&val8, 1);
        if ((val8 & 0x80) == 0) {
            SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_ENTRY_531xx, (uint8_t *)val32, 4);
            return 0;
        }
        udelay(100);
    }
    return -BCM_E_ERROR;
}

static int _write_vlan_table(bcm_vlan_t vid, uint32_t val32)
{
    uint8_t val8;
    int i, timeout = 200;

    SF2SW_WREG(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_INDX_531xx, (uint8_t *)&vid, 2);
    SF2SW_WREG(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_ENTRY_531xx, (uint8_t *)&val32, 4);
    val8 = 0x80;
    SF2SW_WREG(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL_531xx, (uint8_t *)&val8, 1);

    for (i = 0; i < timeout; i++) {
        SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_VLAN_TBL_CTRL_531xx, (uint8_t *)&val8, 1);
        if ((val8 & 0x80) == 0) {
            return 0;
        }
        udelay(100);
    }
    return -BCM_E_ERROR;
}

int ioctl_extsw_vlan(struct ethswctl_data *e)
{
    bcm_vlan_t vid;
    uint32_t val32;

    down(&sf2_sw->s.conf_sem);
    vid = e->vid & BCM_NET_VLAN_VID_M;
    if (e->type == TYPE_GET) {
        if (_read_vlan_table(vid, &val32)) {
            up(&sf2_sw->s.conf_sem);
            enet_err("SF2 VLAN Table read failed\n");
            return -BCM_E_ERROR;
        }
        e->fwd_map = val32 & VLAN_FWD_MAP_M;
        e->untag_map = (val32 >> VLAN_UNTAG_MAP_S) & VLAN_UNTAG_MAP_M;
    } else {
        val32 = e->fwd_map | (e->untag_map << VLAN_UNTAG_MAP_S);
        if (_write_vlan_table(vid, val32)) {
            up(&sf2_sw->s.conf_sem);
            enet_err("SF2 VLAN Table write failed\n");
            return -BCM_E_ERROR;
        }
    }
    up(&sf2_sw->s.conf_sem);
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWARLACCESS functions ---
#define _enet_arl_read( mac, vid, val ) FALSE
#define _enet_arl_write(mac, vid, val) {}
#define _enet_arl_dump() {}  /* This should return status actually ?? */
#define _enet_arl_dump_multiport_arl() {}

static inline int enet_arl_remove(char *mac) {return 0;}

// based on bcmsw.h
int _enet_arl_search_ext(uint8_t *mac, uint32_t *vid, uint32_t *val, int op);
#define _enet_arl_dump_ext() _enet_arl_search_ext(0, 0, 0, TYPE_DUMP)
#define _enet_arl_read_ext(mc, vd, vl) _enet_arl_search_ext(mc, vd, vl, TYPE_GET)
#define _enet_arl_remove_ext(mc) _enet_arl_search_ext(mc, 0, 0, TYPE_SET)


static int _enet_arl_access_reg_op(uint8_t v8)
{
    int timeout;

    SF2SW_WREG(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
    for ( timeout = 10, SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            (v8 & ARL_TBL_CTRL_START_DONE) && timeout;
            --timeout, SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1))
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
int _enet_arl_write_ext(uint8_t *mac, uint16_t vid, uint32_t v32)
{
    uint8_t mac_vid[8];
    uint32_t cur_v32;
    uint16_t ent_vid;
    int bin, empty_bin = -1;

    if (!(v32 & (1<<31))) v32 = ((v32 & 0xfc00) << 1) | (v32 & 0x1ff);  /* If it is raw, shift valid bit left */
    v32 &= ~(1<<31);

    /* Write the MAC Address and VLAN ID */
    SF2SW_WREG(PAGE_AVTBL_ACCESS, REG_ARL_MAC_INDX_LO, mac, 6|DATA_TYPE_BYTE_STRING);
    SF2SW_WREG(PAGE_AVTBL_ACCESS, REG_ARL_VLAN_INDX, &vid, 2);
    if (!_enet_arl_access_reg_op(ARL_TBL_CTRL_START_DONE | ARL_TBL_CTRL_READ)) return 0;

    for (bin = 0; bin < REG_ARL_BINS_PER_HASH; bin++)
    {
        /* Read transaction complete - get the MAC + VID */
        SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_ARL_MAC_LO_ENTRY + bin*0x10, &mac_vid[0], 8|DATA_TYPE_VID_MAC);
        SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_ARL_DATA_ENTRY + bin*0x10,(uint8_t *)&cur_v32, 4);
        ent_vid = *(uint16_t*)mac_vid;

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
        SF2SW_WREG(PAGE_AVTBL_ACCESS, REG_ARL_MAC_LO_ENTRY + bin*0x10, mac_vid, 8|DATA_TYPE_VID_MAC);
        SF2SW_WREG(PAGE_AVTBL_ACCESS, REG_ARL_DATA_ENTRY + bin*0x10,(uint8_t *)&v32, 4);

        /* Initiate a write transaction */
        if (!_enet_arl_access_reg_op(ARL_TBL_CTRL_START_DONE)) return 0;
        return 1;
    }
    enet_err("Error - can't find the requested ARL entry\n");
    return 0;
}

int _enet_arl_entry_op(uint8_t *mac, uint32_t *vid, uint32_t *val, int op, int *count, u8 *mac_vid, u32 data)
{
    switch(op)
    {
        case TYPE_DUMP:
            if (*count == 0) printk("\nExternal Switch ARL Dump:\n");
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
                _enet_arl_write_ext(mac, *(u16*)mac_vid, 0);
                (*count)++;
            }
            break;
        case TYPE_GET:
            if (memcmp(&mac[0], &mac_vid[2], 6) == 0 &&
                    (*vid == -1 || *vid == *(u16*)mac_vid))
            {
                /* entry found */
                *vid = *(uint16_t*)mac_vid;
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

int _enet_arl_search_ext(uint8_t *mac, uint32_t *vid, uint32_t *val, int op)
{
    int timeout = 1000, count = 0, hash_ent;
    uint32_t cur_data;
    uint8_t v8, mac_vid[8];

    v8 = ARL_SRCH_CTRL_START_DONE;
    SF2SW_WREG(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1);

    for( SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1);
            (v8 & ARL_SRCH_CTRL_START_DONE);
            SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1))
    {
        /* Now read the Search Ctrl Reg Until :
         * Found Valid ARL Entry --> ARL_SRCH_CTRL_SR_VALID, or
         * ARL Search done --> ARL_SRCH_CTRL_START_DONE */
        for(timeout = 1000;
                (v8 & ARL_SRCH_CTRL_SR_VALID) == 0 && (v8 & ARL_SRCH_CTRL_START_DONE) && timeout-- > 0;
                mdelay(1),
                SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1));

        if ((v8 & ARL_SRCH_CTRL_SR_VALID) == 0 || timeout <= 0) break;

        /* Found a valid entry */
        for (hash_ent = 0; hash_ent < REG_ARL_SRCH_HASH_ENTS; hash_ent++)
        {
            SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_MAC_LO_ENTRY0_531xx + hash_ent*0x10,&mac_vid[0], 8|DATA_TYPE_VID_MAC);
            SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_DATA_ENTRY0_531xx + hash_ent*0x10,(uint8_t *)&cur_data, 4);

//            enet_dbg("ARL_SRCH_result (%02x%02x%02x%02x%02x%02x%02x%02x) \n",
//                    mac_vid[0],mac_vid[1],mac_vid[2],mac_vid[3],mac_vid[4],mac_vid[5],mac_vid[6],mac_vid[7]);
//            enet_dbg("ARL_SRCH_DATA = 0x%08x \n", cur_data);

            if ((cur_data & ARL_DATA_ENTRY_VALID_531xx))
            {
                if (_enet_arl_entry_op(mac, vid, val, op, &count, mac_vid, cur_data)) return TRUE;
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

void _enet_arl_dump_ext_multiport_arl(void)
{
    uint16 v16;
    uint8 addr[8];
    int i, enabled;
    uint32 vect;
    static char *cmp_type[] = {"Disabled", "Etype", "MAC Addr", "MAC Addr & Etype"};

    SF2SW_RREG(PAGE_ARLCTRL, REG_MULTIPORT_CTRL, &v16, 2);
    enabled = v16 & ((MULTIPORT_CTRL_EN_M << (5*2))| (MULTIPORT_CTRL_EN_M << (4*2))| (MULTIPORT_CTRL_EN_M << (3*2))|
            (MULTIPORT_CTRL_EN_M << (2*2))| (MULTIPORT_CTRL_EN_M << (1*2))| (MULTIPORT_CTRL_EN_M << (0*2)));

    printk("\nExternal Switch Multiport Address Dump: Function %s\n", enabled? "Enabled": "Disabled");
    if (!enabled) return;

    printk("  Mapping to ARL matching: %s\n", v16 & (1<<MULTIPORT_CTRL_DA_HIT_EN)? "Lookup Hit": "Lookup Failed");
    for (i=0; i<6; i++)
    {
        enabled = (v16 & (MULTIPORT_CTRL_EN_M << (5*i)));
        SF2SW_RREG(PAGE_ARLCTRL, REG_MULTIPORT_ADDR1_LO + i*16, (uint8 *)&addr, sizeof(addr)|DATA_TYPE_VID_MAC);
        SF2SW_RREG(PAGE_ARLCTRL, REG_MULTIPORT_VECTOR1 + i*16, (uint8 *)&vect, sizeof(vect));
        printk("Mport Eth Type: 0x%04x, Mport Addrs: %02x:%02x:%02x:%02x:%02x:%02x, Port Map %04x, Cmp Type: %s\n",
                *(uint16 *)(addr),
                addr[2],
                addr[3],
                addr[4],
                addr[5],
                addr[6],
                addr[7],
                (int)vect, cmp_type[enabled & MULTIPORT_CTRL_EN_M]);
    }
    printk("External Switch Multiport Address Dump Done\n");
}

int ioctl_extsw_arl_access(struct ethswctl_data *e)
{
    // based on impl5:bcmeapi_ioctl_ethsw_arl_access()
    int ret;

    switch(e->type)
    {
        case TYPE_GET:
            enet_dbg("get e->mac: %02x %02x %02x %02x %02x %02x e->vid: %d\n", e->mac[5],
                    e->mac[4], e->mac[3], e->mac[2], e->mac[1], e->mac[0], e->vid);

            if (e->unit == SF2_ETHSWCTL_UNIT)
            {
                ret = _enet_arl_read_ext(e->mac, &e->vid, &e->val );
            }
            else
            {
                if ((ret = _enet_arl_read( e->mac, &e->vid, &e->val )) == FALSE)
                {
                    ret = _enet_arl_read_ext(e->mac, &e->vid, &e->val );
                }
            }

            if (ret == FALSE)
            {
                return BCM_E_ERROR;
            }
            break;

        case TYPE_SET:
            enet_dbg("set e->mac: %02x %02x %02x %02x %02x %02x e->vid: %d\n", e->mac[5],
                    e->mac[4], e->mac[3], e->mac[2], e->mac[1], e->mac[0], e->vid);

            if (e->unit == SF2_ETHSWCTL_UNIT)
            {
                if(e->vid == 0xffff && (e->val & ARL_DATA_ENTRY_VALID) == 0)
                {
                    _enet_arl_remove_ext(e->mac);
                }
                else
                {
                    _enet_arl_write_ext(e->mac, e->vid, e->val);
                }
            }
            else
            {
                if(e->vid == 0xffff && (e->val & ARL_DATA_ENTRY_VALID) == 0)
                {
                    enet_arl_remove(e->mac);
                }
                else
                {
                    _enet_arl_write(e->mac, e->vid, e->val);
                }
            }
            break;

        case TYPE_DUMP:
            _enet_arl_dump();
            _enet_arl_dump_multiport_arl();

            _enet_arl_dump_ext();                           // TODO_DSL? probably should move arl access to per switch object
            _enet_arl_dump_ext_multiport_arl();
            break;

        case TYPE_FLUSH:
            /* Flush the ARL table */
            sf2_sw->s.ops->fast_age(sf2_sw);
            break;

        default:
            return BCM_E_PARAM;
    }
    return BCM_E_NONE;
}

// add by Andrew
int _enet_arl_entry_op_us(struct ethswctl_data *e, int *count, u8 *mac_vid, u32 data)
{
    ethsw_mac_entry *me;

    if (*count > MAX_MAC_ENTRY) 
        return FALSE;

    me = &(e->mac_table.entry[*count]);
    memcpy(&me->mac, &mac_vid[2], 6);

    me->port = data&0x1ff;

    (*count)++;

    // return TRUE to terminate the loop
    return FALSE;
}

int _enet_arl_dump_ext_us(struct ethswctl_data *e)
{
    int timeout = 1000, count = 0, hash_ent;
    uint32_t cur_data;
    uint8_t v8, mac_vid[8];

    v8 = ARL_SRCH_CTRL_START_DONE;
    SF2SW_WREG(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1);
    for( SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1);
            (v8 & ARL_SRCH_CTRL_START_DONE);
            SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1))
    {
        /* Now read the Search Ctrl Reg Until :
         * Found Valid ARL Entry --> ARL_SRCH_CTRL_SR_VALID, or
         * ARL Search done --> ARL_SRCH_CTRL_START_DONE */
        for(timeout = 1000;
                (v8 & ARL_SRCH_CTRL_SR_VALID) == 0 && (v8 & ARL_SRCH_CTRL_START_DONE) && timeout-- > 0;
                mdelay(1),
                SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1));

        if ((v8 & ARL_SRCH_CTRL_SR_VALID) == 0 || timeout <= 0) break;

        /* Found a valid entry */
        for (hash_ent = 0; hash_ent < REG_ARL_SRCH_HASH_ENTS; hash_ent++)
        {
            SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_MAC_LO_ENTRY0_531xx + hash_ent*0x10,&mac_vid[0], 8|DATA_TYPE_VID_MAC);
            SF2SW_RREG(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_DATA_ENTRY0_531xx + hash_ent*0x10,(uint8_t *)&cur_data, 4);

            if ((cur_data & ARL_DATA_ENTRY_VALID_531xx))
            {
                if (_enet_arl_entry_op_us(e, &count, mac_vid, cur_data)) 
                    return TRUE;
            }
        }
    }
    e->mac_table.count = count;
    e->mac_table.len = count*sizeof(ethsw_mac_entry) + 8;

    return TRUE;
}

int ioctl_extsw_arl_dump(struct ethswctl_data *e)
{
    if (e->type != TYPE_DUMP) 
        return BCM_E_PARAM;

    _enet_arl_dump_ext_us(e); 

    return BCM_E_NONE;
}
// end of add

int remove_arl_entry_wrapper(void *ptr)
{
    int ret = 0;
    ret = enet_arl_remove(ptr);         /* remove entry from internal switch */
    ret = _enet_arl_remove_ext(ptr);    /* remove entry from external switch */
    return ret;
}

// ----------- SIOCETHSWCTLOPS ETHSWCOSPRIORITYMETHOD functions ---
/* This function just serves Star Fighter. Legacy External switch
 * goes with Robo.
 *
 * Get/Set StarFighter cos priority method
 *** Input params
 * e->type  GET/SET
 * e->pkt_type_mask - ipv4/ipv6:802.1p:static mac destination or port Id based
 * e->val - ingress classifier TC src selection -- DSCP, vlan pri,
 *        -  MAC addr, PORT based (default vlan tag)
 * e->port  per port
 *** Output params
 * Returns 0 for Success, Negative value for failure.
 */
static int _qos_dscp_is_enabled(int port)
{
    u16 val16;
    SF2SW_RREG(PAGE_QOS, REG_QOS_DSCP_EN, (void *)&val16, 2);
    return (val16 >> port) & 1;
}

static int _qos_8021p_is_enabled(int port)
{
    u16 val16;
    SF2SW_RREG(PAGE_QOS, REG_QOS_8021P_EN, (void *)&val16, 2);
    return (val16 >> port) & 1;
}

static void _enable_dscp_qos(int port, int enable)
{
    u16 val16;
    SF2SW_RREG(PAGE_QOS, REG_QOS_DSCP_EN, (void *)&val16, 2);
    val16 &= ~(1 << port);
    val16 |= enable << port;
    SF2SW_WREG(PAGE_QOS, REG_QOS_DSCP_EN, (void *)&val16, 2);
}

static void _enable_8021p_qos(int port, int enable)
{
    u16 val16;
    SF2SW_RREG(PAGE_QOS, REG_QOS_8021P_EN, (void *)&val16, 2);
    val16 &= ~(1 << port);
    val16 |= enable << port;
    SF2SW_WREG(PAGE_QOS, REG_QOS_8021P_EN, (void *)&val16, 2);
}

/* Note: Method values are UAPI definition */
static int _isQoSMethodEnabled(int port, int method)
{
    switch(method)
    {
        case PORT_QOS:
        case MAC_QOS:
            return 1;
        case IEEE8021P_QOS:
            return _qos_8021p_is_enabled(port);
        case DIFFSERV_QOS:
            return _qos_dscp_is_enabled(port);
    }
    return 0;
}

/* Note: Method values are UAPI definition */
static void _enableQosMethod(int port, int method, int enable)
{
    switch(method)
    {
        case MAC_QOS:
        case PORT_QOS:
            return;
        case IEEE8021P_QOS:
            if (enable)
            {
                _enable_8021p_qos(port, 1); // Enable PCP for the port
            }
            else
            {
                _enable_8021p_qos(port, 0);
            }
            return;
        case DIFFSERV_QOS:
            if (enable)
            {
                _enable_dscp_qos(port, 1); // Enable DSCP for the port
            }
            else
            {
                _enable_dscp_qos(port, 0);
            }
            return;
    }
}

#define QOS_METHOD_CNVT_UAPI_AND_REG(regQoS)  (~(regQoS) & SF2_QOS_TC_SRC_SEL_VAL_MASK)
#define QOS_METHODS_CNVT_UAPI_AND_REG(regQoSPorts)  (~(regQoSPorts) & 0xffff)
int ioctl_extsw_cos_priority_method_cfg(struct ethswctl_data *e)
{
    // based on impl5:sf2_cos_priority_method_config()
    uint16_t val16, reg_addr, pkt_type_mask, tc_sel_src ;
    uint32_t val32;
    int i, enable_qos;

    down(&sf2_sw->s.conf_sem);

    //enet_dbg("%s port %d pkt_type 0x%x Given method: %02d \n ",__FUNCTION__,
    //        e->port, e->pkt_type_mask, e->val);
    if (e->port < 0 || e->port > SF2_IMP0_PORT ||  e->port == SF2_INEXISTANT_PORT) {
        enet_dbg("parameter error, port %d \n", e->port);
        return -BCM_E_PARAM;
    }
    reg_addr = SF2_REG_PORTN_TC_SELECT_TABLE + e->port * 2;
    pkt_type_mask = e->pkt_type_mask;
    if (e->type == TYPE_GET) {
        SF2SW_RREG(PAGE_QOS, reg_addr, &val16, 2);
        if (e->pkt_type_mask == SF2_QOS_TC_SRC_SEL_PKT_TYPE_ALL)
        {
            val32 = QOS_METHODS_CNVT_UAPI_AND_REG(val16);
            for (i = 0; i < e->pkt_type_mask; i++)
            {
                tc_sel_src = (val16 >> (i * 2)) & SF2_QOS_TC_SRC_SEL_VAL_MASK;
                enable_qos = _isQoSMethodEnabled(e->port, QOS_METHOD_CNVT_UAPI_AND_REG(tc_sel_src));
                val32 |= !enable_qos << (16+i);
            }
        } else {
            pkt_type_mask &=  SF2_QOS_TC_SRC_SEL_PKT_TYPE_MASK;
            val16 =  (val16 >> (pkt_type_mask * 2)) & SF2_QOS_TC_SRC_SEL_VAL_MASK;
            val16 = QOS_METHOD_CNVT_UAPI_AND_REG(val16);
            enable_qos = _isQoSMethodEnabled(e->port, val16);
            val32 = (!enable_qos << 16) | val16;
        }
        // bits programmed in TC Select Table registers and software notion are bit inversed.
        e->ret_val = val32;
    } else { // TYPE_SET
        reg_addr = SF2_REG_PORTN_TC_SELECT_TABLE + e->port * 2;

        /* when pkt_type_mask is NOT SF2_QOS_TC_SRC_SEL_PKT_TYPE_ALL,
            tc_sel_src b0-1 2 bit show of TC.  b16-17 2bits contains disable bit.  */
        if (e->pkt_type_mask != SF2_QOS_TC_SRC_SEL_PKT_TYPE_ALL)
        {
            tc_sel_src = QOS_METHOD_CNVT_UAPI_AND_REG(e->val);
            SF2SW_RREG(PAGE_QOS, reg_addr, &val16, 2);
            pkt_type_mask = e->pkt_type_mask & SF2_QOS_TC_SRC_SEL_PKT_TYPE_MASK;
            val16 &= ~(SF2_QOS_TC_SRC_SEL_VAL_MASK << (pkt_type_mask * 2));
            val16 |=  (tc_sel_src & SF2_QOS_TC_SRC_SEL_VAL_MASK ) << (pkt_type_mask * 2);
            enable_qos = !((e->val >> 16) & 1);
            //enet_dbg("%s: Write to: len %d page 0x%x reg 0x%x val 0x%x\n",
            //        __FUNCTION__, 2, PAGE_QOS, reg_addr, val16);
            _enableQosMethod(e->port, e->val & SF2_QOS_TC_SRC_SEL_VAL_MASK, enable_qos);
        }
        else    /* SF2_QOS_TC_SRC_SEL_PKT_TYPE_ALL */
        /* when pkt_type_mask is SF2_QOS_TC_SRC_SEL_PKT_TYPE_ALL,
            tc_sel_src's lower 16 bits contains TC selections for all 8 packet types.
            higher 8 bits contains disable bit for corresponding methods in lower 16bits.  */
        {
            val16 = QOS_METHODS_CNVT_UAPI_AND_REG(e->val);
            for (i = 0; i < e->pkt_type_mask; i++)
            {
                tc_sel_src = ((e->val) >> (i * 2)) & SF2_QOS_TC_SRC_SEL_VAL_MASK;
                enable_qos = !((e->val >> (16 + i)) & 1);
                _enableQosMethod(e->port, tc_sel_src, enable_qos);
            }
        }
        SF2SW_WREG(PAGE_QOS, reg_addr, &val16, 2);
    }

    up(&sf2_sw->s.conf_sem);
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWJUMBO functions ---
static uint32 _ConfigureJumboPort(uint32 regVal, int portVal, unsigned int configVal) // bill
{
    // based on impl5:ConfigureJumboPort()
    UINT32 controlBit;

    // Test for valid port specifier.
    if ((portVal >= ETHSWCTL_JUMBO_PORT_GPHY_0) && (portVal <= ETHSWCTL_JUMBO_PORT_ALL))
    {
        // Switch on port ID.
        switch (portVal)
        {
            case ETHSWCTL_JUMBO_PORT_MIPS:
                controlBit = ETHSWCTL_JUMBO_PORT_MIPS_MASK;
                break;
            case ETHSWCTL_JUMBO_PORT_GPON:
                controlBit = ETHSWCTL_JUMBO_PORT_GPON_MASK;
                break;
            case ETHSWCTL_JUMBO_PORT_USB:
                controlBit = ETHSWCTL_JUMBO_PORT_USB_MASK;
                break;
            case ETHSWCTL_JUMBO_PORT_MOCA:
                controlBit = ETHSWCTL_JUMBO_PORT_MOCA_MASK;
                break;
            case ETHSWCTL_JUMBO_PORT_GPON_SERDES:
                controlBit = ETHSWCTL_JUMBO_PORT_GPON_SERDES_MASK;
                break;
            case ETHSWCTL_JUMBO_PORT_GMII_2:
                controlBit = ETHSWCTL_JUMBO_PORT_GMII_2_MASK;
                break;
            case ETHSWCTL_JUMBO_PORT_GMII_1:
                controlBit = ETHSWCTL_JUMBO_PORT_GMII_1_MASK;
                break;
            case ETHSWCTL_JUMBO_PORT_GPHY_1:
                controlBit = ETHSWCTL_JUMBO_PORT_GPHY_1_MASK;
                break;
            case ETHSWCTL_JUMBO_PORT_GPHY_0:
                controlBit = ETHSWCTL_JUMBO_PORT_GPHY_0_MASK;
                break;
            default: // ETHSWCTL_JUMBO_PORT_ALL:
                controlBit = ETHSWCTL_JUMBO_PORT_MASK_VAL;  // ALL bits
                break;
        }

        // Test for accept JUMBO frames.
        if (configVal != 0)
        {
            // Setup register value to accept JUMBO frames.
            regVal |= controlBit;
        }
        else
        {
            // Setup register value to reject JUMBO frames.
            regVal &= ~controlBit;
        }
    }

    // Return new JUMBO configuration control register value.
    return regVal;
}


int ioctl_extsw_port_jumbo_control(struct ethswctl_data *e)
{
    // based on impl5:bcmeapi_ioctl_extsw_port_jumbo_control()
    uint32_t val32;

    if (e->type == TYPE_GET)
    {
        // Read & log current JUMBO configuration control register.
        SF2SW_RREG(PAGE_JUMBO, REG_JUMBO_PORT_MASK, (uint8 *)&val32, 4);
        enet_dbg("JUMBO_PORT_MASK = 0x%08X", (unsigned int)val32);
        e->ret_val = val32;
    }
    else
    {
        // Read & log current JUMBO configuration control register.
        SF2SW_RREG(PAGE_JUMBO, REG_JUMBO_PORT_MASK, (uint8 *)&val32, 4);
        enet_dbg("Old JUMBO_PORT_MASK = 0x%08X", (unsigned int)val32);

        // Setup JUMBO configuration control register.
        val32 = _ConfigureJumboPort(val32, e->port, e->val);
        SF2SW_WREG(PAGE_JUMBO, REG_JUMBO_PORT_MASK, (uint8 *)&val32, 4);

        // Attempt to transfer register write value to user space & test for success.
        e->ret_val = val32;
    }
    return BCM_E_NONE;
}

// ----------- SIOCETHSWCTLOPS ETHSWCOSPCPPRIOMAP functions ---
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
int ioctl_extsw_pcp_to_priority_mapping(struct ethswctl_data *e)
{
    // based on impl5:bcmeapi_ioctl_extsw_pcp_to_priority_mapping()
    uint32_t val32;
    uint16_t reg_addr;

    enet_dbg("Given pcp: %02d \n ", e->val);
    if (e->val > MAX_PRIORITY_VALUE) {
        enet_err("Invalid PCP Value %02d \n", e->val);
        return BCM_E_ERROR;
    }

    if (e->port < 0 || e->port > SF2_IMP0_PORT ||  e->port == SF2_INEXISTANT_PORT) {
        enet_err("Invalid port number %02d \n", e->port);
        return BCM_E_ERROR;
    }
    reg_addr = e->port == SF2_IMP0_PORT? SF2_REG_QOS_PCP_IMP0:
               e->port == SF2_P7? SF2_REG_QOS_PCP_P7:
                          REG_QOS_8021P_PRIO_MAP + e->port * QOS_PCP_MAP_REG_SZ;

    down(&sf2_sw->s.conf_sem);

    if (e->type == TYPE_GET) {
        SF2SW_RREG(PAGE_QOS, reg_addr, (void *)&val32, 4);
        e->priority = (val32 >> (e->val * QOS_TC_S)) & QOS_TC_M;
        enet_dbg("pcp %d is mapped to priority: %d \n ", e->val, e->priority);
    } else {
        enet_dbg("Given pcp: %02d priority: %02d \n ", e->val, e->priority);
        if ((e->priority > MAX_PRIORITY_VALUE) || (e->priority < 0)) {
            enet_err("Invalid Priority \n");
            up(&sf2_sw->s.conf_sem);
            return BCM_E_ERROR;
        }
        SF2SW_RREG(PAGE_QOS, reg_addr, (void *)&val32, 4);
        val32 &= ~(QOS_TC_M << (e->val * QOS_TC_S));
        val32 |= (e->priority & QOS_TC_M) << (e->val * QOS_TC_S);
        SF2SW_WREG(PAGE_QOS, reg_addr, (void *)&val32, 4);
    }

    up(&sf2_sw->s.conf_sem);
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWCOSPIDPRIOMAP functions ---
/*
 * Get/Set PID to TC mapping Table entry given ingress port
 * and mapped priority
 *** Input params
 * e->type  GET/SET
 * e->priority - mapped TC value, case of SET
 *** Output params
 * e->priority - mapped TC value, case of GET
 * Returns 0 for Success, Negative value for failure.
 */
int ioctl_extsw_pid_to_priority_mapping(struct ethswctl_data *e)
{
    // based on impl5:sf2_pid_to_priority_mapping()
    uint32_t val32;

    enet_dbg("Given uint %02d port %02d \n ", e->unit, e->port);

    if (e->port < 0 || e->port > SF2_IMP0_PORT ||  e->port == SF2_INEXISTANT_PORT) {
        enet_err("Invalid port number %02d \n", e->port);
        return BCM_E_ERROR;
    }
    down(&sf2_sw->s.conf_sem);

    if (e->type == TYPE_GET) {
        SF2SW_RREG(PAGE_QOS, SF2_REG_PORT_ID_PRIO_MAP, (void *)&val32, 4);
        e->priority = (val32 >> (e->port * QOS_TC_S)) & QOS_TC_M;
        enet_dbg("port %d is mapped to priority: %d \n ", e->port, e->priority);
    } else {
        enet_dbg("Given port: %02d priority: %02d \n ", e->port, e->priority);
        SF2SW_RREG(PAGE_QOS, SF2_REG_PORT_ID_PRIO_MAP, (void *)&val32, 4);
        val32 &= ~(QOS_TC_M << (e->port * QOS_TC_S));
        val32 |= (e->priority & QOS_TC_M) << (e->port * QOS_TC_S);
        SF2SW_WREG(PAGE_QOS, SF2_REG_PORT_ID_PRIO_MAP, (void *)&val32, 4);
    }

    up(&sf2_sw->s.conf_sem);
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWCOSDSCPPRIOMAP functions ---
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
int ioctl_extsw_dscp_to_priority_mapping(struct ethswctl_data *e)
{
    // based on impl5:bcmeapi_ioctl_extsw_dscp_to_priority_mapping()
    uint64 val64 = 0;
    uint32_t mapnum;
    int dscplsbs;

    enet_dbg("Given dscp: %02d \n ", e->val);
    if (e->val > QOS_DSCP_M) {
        enet_err("Invalid DSCP Value \n");
        return BCM_E_ERROR;
    }

    down(&sf2_sw->s.conf_sem);

    dscplsbs = e->val & QOS_DSCP_MAP_LSBITS_M;
    mapnum = (e->val >> QOS_DSCP_MAP_S) & QOS_DSCP_MAP_M;

    if (e->type == TYPE_GET) {
        SF2SW_RREG(PAGE_QOS, REG_QOS_DSCP_PRIO_MAP0LO + mapnum * QOS_DSCP_MAP_REG_SZ,
                                 (void *)&val64, QOS_DSCP_MAP_REG_SZ | DATA_TYPE_HOST_ENDIAN);
        e->priority = (val64 >> (dscplsbs * QOS_TC_S)) & QOS_TC_M;
        enet_dbg("dscp %d is mapped to priority: %d \n ", e->val, e->priority);
    } else {
        enet_dbg("Given priority: %02d \n ", e->priority);
        if ((e->priority > MAX_PRIORITY_VALUE) || (e->priority < 0)) {
            enet_err("Invalid Priority \n");
            up(&sf2_sw->s.conf_sem);
            return BCM_E_ERROR;
        }
        // LE assumptions below, TODO
        SF2SW_RREG(PAGE_QOS, REG_QOS_DSCP_PRIO_MAP0LO + mapnum * QOS_DSCP_MAP_REG_SZ,
                                     (void *)&val64, QOS_DSCP_MAP_REG_SZ | DATA_TYPE_HOST_ENDIAN);
        val64 &= ~(((uint64)(QOS_TC_M)) << (dscplsbs * QOS_TC_S));
        val64 |= ((uint64)(e->priority & QOS_TC_M)) << (dscplsbs * QOS_TC_S);
        enet_dbg(" @ addr %#x val64 to write = 0x%llx \n",
                                (REG_QOS_DSCP_PRIO_MAP0LO + mapnum * QOS_DSCP_MAP_REG_SZ),
                                (uint64) val64);

        SF2SW_WREG(PAGE_QOS, REG_QOS_DSCP_PRIO_MAP0LO + mapnum * QOS_DSCP_MAP_REG_SZ,
                                            (void *)&val64, QOS_DSCP_MAP_REG_SZ | DATA_TYPE_HOST_ENDIAN);
    }

    up(&sf2_sw->s.conf_sem);
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWPORTRXRATE functions ---
/*
 *  Set the burst size and rate limit value of the selected port ingress rate.
 *      limit  :   rate limit value. (Kbits)
 *      burst_size  :   max burst size. (Kbits)
 */
int ioctl_extsw_port_irc_set(struct ethswctl_data *e)
{
    // based on impl5:bcmeapi_ioctl_ethsw_port_irc_set()
    uint32_t  val32, bs = 0, rf = 0;

    down(&sf2_sw->s.conf_sem);

    if (e->limit == 0) { /* Disable ingress rate control */
        SF2SW_RREG(PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_RATE_PORT_0 + e->port * 4, &val32, 4);
        val32 &= ~REG_PN_BUCK1_ENABLE_MASK;
        SF2SW_WREG(PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_RATE_PORT_0 + e->port * 4, &val32, 4);
    } else {    /* Enable ingress rate control */
        bs = e->burst_size / 8;
        if (bs <= 4)
        {
            bs = 0;
        }
        else if (bs <= 8)
        {
            bs = 1;
        }
        else if (bs <= 16)
        {
            bs = 2;
        }
        else if (bs <= 32)
        {
            bs = 3;
        }
        else if (bs <= 64)
        {
            bs = 4;
        }
        else
        {
            bs = REG_PN_BUCK1_SIZE_M;
        }

        rf = e->limit;
        if (rf <= 1800)
        {
            rf = rf * 125 / 8 / 1000;
            if (rf == 0) rf = 1;
        }
        else if (rf <= 100000)
        {
            rf = rf / 1000 + 27;
        }
        else
        {
            rf = rf / 1000 / 8 + 115;
            if ( rf > 240) rf = 240;
        }

        SF2SW_RREG(PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_RATE_PORT_0 + e->port * 4, &val32, 4);
        val32 &= ~((REG_PN_BUCK1_SIZE_M << REG_PN_BUCK1_SIZE_S)| (REG_PN_BUCK1_REF_CNT_M << REG_PN_BUCK1_REF_CNT_S));
        val32 |= REG_PN_BUCK1_ENABLE_MASK | REG_PN_BUCK1_MODE_MASK; // use bucket 1
        val32 |= (rf & REG_PN_BUCK1_REF_CNT_M) << REG_PN_BUCK1_REF_CNT_S;
        val32 |= bs  << REG_PN_BUCK1_SIZE_S;
        SF2SW_WREG(PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_RATE_PORT_0 + e->port * 4, &val32, 4);

        SF2SW_RREG(PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_CFG_PORT_0 + e->port * 2, &val32, 4);
	    val32 |= REG_PN_BUCK1_IFG_BYTES_MASK | (REG_PN_BUCK1_PKT_SEL_M << REG_PN_BUCK1_PKT_SEL_S);
        SF2SW_WREG(PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_CFG_PORT_0 + e->port * 2, &val32, 4);
    }

    up(&sf2_sw->s.conf_sem);
    return BCM_E_NONE;
}

/*
 *   Get the burst size and rate limit value of the selected port ingress rate.
 */
int ioctl_extsw_port_irc_get(struct ethswctl_data *e)
{
    // based on impl5:bcmeapi_ioctl_ethsw_port_irc_get()
    uint32_t  val32, rf;

    down(&sf2_sw->s.conf_sem);

    SF2SW_RREG(PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_RATE_PORT_0 + e->port * 4, &val32, 4);
    if ((val32 & REG_PN_BUCK1_ENABLE_MASK) == 0)
    {
        e->limit = 0;
    }
    else
    {
        rf  = (val32 >> REG_PN_BUCK1_REF_CNT_S) & REG_PN_BUCK1_REF_CNT_M;
        if (rf <= 28)
        {
            e->limit = rf * 8000 / 125;
        }
        else if (rf <= 127)
        {
            e->limit = (rf - 27 ) * 1000;
        }
        else
        {
            if (rf > 240) rf = 240;
            e->limit = (rf - 115) * 1000 * 8;
        }

        e->burst_size = 4 << ((val32 >> REG_PN_BUCK1_SIZE_S) & REG_PN_BUCK1_SIZE_M);
        if (e->burst_size > 64) e->burst_size = 488;
        e->burst_size *= 8;
    }

    up(&sf2_sw->s.conf_sem);
    return BCM_E_NONE;
}

// ----------- SIOCETHSWCTLOPS ETHSWPORTTXRATE functions ---
/*
 * Get/Set StarFighter Egress shaper control
 *** Input params
 * e->port  egress port that is configured
 * e->unit  switch unit
 * e->type  GET/SET
 * e->limit egress rate control limit in
 *          64 kbps(Byte mode) 125 pps packet mode
 * e->burst_size egress burst in 64 Byte units(Byte mode)
 *          or in packets (packet mode)
 * e->queue egress queue if specified (per queue config)
 *          or -1 is not specified
 *** Output params
 * e->vptr has result copied for GET
 * Returns 0 for Success, Negative value for failure.
 */
int ioctl_extsw_port_erc_config(struct ethswctl_data *e)
{
    // based on impl5:sf2_port_erc_config()
    uint16_t val16, page, reg;
    uint32_t val32;
    uint32_t pkt_flag;
    unsigned char q_shaper = e->queue >= 0;

    if (e->type == TYPE_SET) {

        /* find queue or port page*/
        page = q_shaper? PAGE_Q0_EGRESS_SHAPER + e->queue:
                                    PAGE_PORT_EGRESS_SHAPER;
        /* find the shaper mode */
        pkt_flag =  (e->sub_type == SHAPER_PACKET_MODE) ? SHAPER_PACKET_MODE : 0;

        /* configure shaper rate limit; limit = 0 means disable shaper */
        val32 = 0; /* reset rate limiting by default */
        if (e->limit)
        {
            if (pkt_flag == SHAPER_PACKET_MODE)
            {
                val32 = e->limit/125; /* shaper rate config in 125pps units */
            }
            else {
                val32 = e->limit/64; /* shaper rate config in 64Kbps units */
            }
            if (!val32) {
                val32 = 1; /* At least 64Kbps */
            }
            val32 = val32 & SHAPER_RATE_BURST_VAL_MASK;
        }
        reg  =   pkt_flag == SHAPER_PACKET_MODE? SF2_REG_PN_SHAPER_RATE_PKT:
                                                  SF2_REG_PN_SHAPER_RATE_BYTE;
        SF2SW_WREG(page, reg + e->port * 4, &val32, 4);

        /* configure shaper burst size */
        val32 = 0; /* reset burst size by default */
        if (e->limit) { /* Only set burst size if shaper is getting enabled */
            if (pkt_flag == SHAPER_PACKET_MODE) {
                val32 = e->burst_size; /* shaper burst config in 1 packet units */
            }
            else {
                val32 = (e->burst_size /* Kbits */ * 1000)/(8*64); /* shaper burst config in 64Byte units */
            }
            if (!val32) {
                val32 = 1;
            }
            val32 = val32 & SHAPER_RATE_BURST_VAL_MASK;
        }
        reg  =   pkt_flag == SHAPER_PACKET_MODE? SF2_REG_PN_SHAPER_BURST_SZ_PKT:
                                                  SF2_REG_PN_SHAPER_BURST_SZ_BYTE;
        SF2SW_WREG(page, reg + e->port * 4, &val32, 4);

        /* enable shaper for byte mode or pkt mode as the case may be. */
        SF2SW_RREG(page, SF2_REG_SHAPER_ENB_PKT_BASED, &val16, 2);
        val16 &= ~(1 << e->port);
        val16 |= pkt_flag == SHAPER_PACKET_MODE? (1 << e->port): 0;
        SF2SW_WREG(page, SF2_REG_SHAPER_ENB_PKT_BASED, &val16, 2);

        /* Enable/disable shaper */
        SF2SW_RREG(page, SF2_REG_SHAPER_ENB, &val16, 2);
        val16 &= ~(1 << e->port); /* Disable Shaper */
        val16 |= e->limit? (1 << e->port): 0; /* Enable Shaper, if needed */
        SF2SW_WREG(page, SF2_REG_SHAPER_ENB, &val16, 2);

        return 0;
    } else {
        /* Egress shaper stats*/
        egress_shaper_stats_t stats;

        page = q_shaper? PAGE_Q0_EGRESS_SHAPER + e->queue:
                                    PAGE_PORT_EGRESS_SHAPER;
        SF2SW_RREG(page, SF2_REG_SHAPER_ENB_PKT_BASED, &val16, 2);
        pkt_flag = (val16 & (1 << e->port))? 1: 0;
        stats.egress_shaper_flags = 0;
        stats.egress_shaper_flags |= pkt_flag? SHAPER_RATE_PACKET_MODE: 0;

        reg = pkt_flag? SF2_REG_PN_SHAPER_RATE_PKT: SF2_REG_PN_SHAPER_RATE_BYTE;
        SF2SW_RREG(page, reg + e->port * 4, &val32, 4);
        stats.egress_rate_cfg = val32 & SHAPER_RATE_BURST_VAL_MASK;

        reg = pkt_flag? SF2_REG_PN_SHAPER_BURST_SZ_PKT: SF2_REG_PN_SHAPER_BURST_SZ_BYTE;
        SF2SW_RREG(page, reg + e->port * 4, &val32, 4);
        stats.egress_burst_sz_cfg = val32 & SHAPER_RATE_BURST_VAL_MASK;

        reg = SF2_REG_PN_SHAPER_STAT;
        SF2SW_RREG(page, reg + e->port * 4, &val32, 4);
        stats.egress_cur_tokens = val32 & SHAPER_STAT_COUNT_MASK;
        stats.egress_shaper_flags |= val32 & SHAPER_STAT_OVF_MASK? SHAPER_OVF_FLAG: 0;
        stats.egress_shaper_flags |= val32 & SHAPER_STAT_INPF_MASK? SHAPER_INPF_FLAG: 0;

        SF2SW_RREG(page, SF2_REG_SHAPER_ENB, &val16, 2);
        stats.egress_shaper_flags |= (val16 & (1 << e->port))? SHAPER_ENABLE: 0;

        SF2SW_RREG(page, SF2_REG_SHAPER_BLK_CTRL_ENB, &val16, 2);
        stats.egress_shaper_flags |= (val16 & (1 << e->port))? SHAPER_BLOCKING_MODE: 0;

        // applies only for port shaper
        if (!q_shaper) {
            SF2SW_RREG(page, SF2_REG_SHAPER_INC_IFG_CTRL, &val16, 2);
            stats.egress_shaper_flags |= (val16 & (1 << e->port))? SHAPER_INCLUDE_IFG: 0;
        }

        /* Convert the return values based on mode */
        if (pkt_flag)
        {
            stats.egress_rate_cfg *= 125; /* Shaper rate in 125pps unit */
            /* stats.egress_burst_sz_cfg  - burst unit in packets */
        }
        else {
            stats.egress_rate_cfg *= 64; /* Shaper rate in 64Kbps unit */
            stats.egress_burst_sz_cfg = (stats.egress_burst_sz_cfg*8*64)/1000; /* Shaper burst is in 64Byte unit - convert into kbits */
        }
        if (e->vptr) {
            if (copy_to_user (e->vptr, &stats, sizeof(egress_shaper_stats_t))) {
                return -EFAULT;
            }
        } else {
            // Just support Legacy API
            e->limit = stats.egress_rate_cfg;
            e->burst_size =  stats.egress_burst_sz_cfg;
        }
    }
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWPORTSHAPERCFG functions ---
inline static void _extsw_reg16_bit_ops(uint16 page, uint16 reg, int bit, int on)
{
    uint16 val16;

    SF2SW_RREG(page, reg, &val16, 2);
    val16 &= ~(1 << bit);
    val16 |= on << bit;
    SF2SW_WREG(page, reg, &val16, 2);
}


/*
 * As part of setting StarFighter Egress shaper
 * configuration, turn on/off various shaper modes.
 *** Input params
 * e->port  egress port that is configured
 * e->unit  switch unit
 * e->type  SET
 * e->queue egress queue if specified (per queue config)
 *          or -1 is not specified
 * e->sub_type - Or'ed Flags
 * e->val = 1 | 0  for On or Off
 *  Output params None
 * Returns 0 for Success, Negative value for failure.
 */
int ioctl_extsw_port_shaper_config(struct ethswctl_data *e)
{
    // based on impl5:sf2_port_shaper_config()
    uint16 page, reg;
    unsigned char q_shaper;

    if (e->type == TYPE_SET) {
        /* confiure requested shaper parameters.
         * Notice: each q has its separate page.
         */
        q_shaper = e->queue >= 0;
        page = q_shaper? PAGE_Q0_EGRESS_SHAPER + e->queue:
                                    PAGE_PORT_EGRESS_SHAPER;
        if (e->sub_type & SHAPER_ENABLE) {
            reg = SF2_REG_SHAPER_ENB;
            _extsw_reg16_bit_ops(page, reg, e->port, e->val);
        }
        if (e->sub_type & SHAPER_RATE_PACKET_MODE) {
            reg = SF2_REG_SHAPER_ENB_PKT_BASED;
            _extsw_reg16_bit_ops(page, reg, e->port, e->val);
        }
        if (e->sub_type & SHAPER_BLOCKING_MODE) {
            reg = SF2_REG_SHAPER_BLK_CTRL_ENB;
            _extsw_reg16_bit_ops(page, reg, e->port, e->val);
        }
        if (e->sub_type & SHAPER_INCLUDE_IFG) {
            // applies only for port shaper
            if (!q_shaper) {
                reg = SF2_REG_SHAPER_INC_IFG_CTRL;
                _extsw_reg16_bit_ops(page, reg, e->port, e->val);
            }
        }
    }

    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWDOSCTRL functions ---
int ioctl_extsw_dos_ctrl(struct ethswctl_data *e)
{
    // based on impl5:enet_ioctl_ethsw_dos_ctrl()
    if (e->type == TYPE_GET)
    {
        uint32_t v32 = 0;
        uint8_t v8 = 0;

        SF2SW_RREG(PAGE_DOS_PREVENT_531xx, REG_DOS_CTRL, (uint8 *)&v32, 4);
        /* Taking short-cut : Not following BCM coding guidelines */
        if (v32 & DOS_CTRL_DA_SA_DROP_EN)  e->dosCtrl.da_eq_sa_drop_en = 1;
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

        SF2SW_RREG(PAGE_DOS_PREVENT_531xx, REG_DOS_DISABLE_LRN, (uint8 *)&v8, 1);
        if (v8 & DOS_DISABLE_LRN) e->dosCtrl.dos_disable_lrn = 1;
    }
    else if (e->type == TYPE_SET)
    {
        uint32_t v32 = 0;
        uint8_t v8 = 0;
        /* Taking short-cut : Not following BCM coding guidelines */
        if (e->dosCtrl.da_eq_sa_drop_en) v32 |= DOS_CTRL_DA_SA_DROP_EN;
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
        SF2SW_WREG(PAGE_DOS_PREVENT_531xx, REG_DOS_CTRL, (uint8 *)&v32, 4);
        if (e->dosCtrl.dos_disable_lrn)
        { /* Enable */
            v8 = DOS_DISABLE_LRN;
        }
        else
        {
            v8 = 0;
        }
        SF2SW_WREG(PAGE_DOS_PREVENT_531xx, REG_DOS_DISABLE_LRN, (uint8 *)&v8, 1);
    }
    return BCM_E_NONE;
}

// ----------- SIOCETHSWCTLOPS ETHSWHWSTP functions ---
// Set STP state into SF2 register
void _ethsw_set_stp_mode(unsigned int unit, unsigned int port, unsigned char stpState)
{
    // based on impl5:bcmeapi_ethsw_set_stp_mode()
   unsigned char portInfo;

   if(unit == SF2_ETHSWCTL_UNIT) // SF2
   {
      spin_lock_bh(&sf2_reg_config);
      SF2SW_RREG(PAGE_CONTROL, REG_PORT_CTRL + (port),
                 &portInfo, sizeof(portInfo));
      portInfo &= ~REG_PORT_STP_MASK;
      portInfo |= stpState;
      SF2SW_WREG(PAGE_CONTROL, REG_PORT_CTRL + (port),
                 &portInfo, sizeof(portInfo));
      spin_unlock_bh(&sf2_reg_config);
   }
}

int _ethsw_get_stp_state(unsigned int unit, unsigned int port)
{
   unsigned char portInfo;

   if(unit == SF2_ETHSWCTL_UNIT) // SF2
   {
      SF2SW_RREG(PAGE_CONTROL, REG_PORT_CTRL + (port),
                 &portInfo, sizeof(portInfo));
      return (portInfo & REG_PORT_STP_MASK);
   }
   return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWPORTSTORMCTRL functions ---
int ioctl_extsw_port_storm_ctrl(struct ethswctl_data *e)
{
    // based on impl5:bcmeapi_ioctl_extsw_port_storm_ctrl()
    uint32_t val32;

    down(&sf2_sw->s.conf_sem);
    if (e->type == TYPE_SET) {
        /* configure storm control rate & burst size */
        SF2SW_RREG(PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_RATE_PORT_0 + e->port * 4, &val32, 4);
        val32 |= REG_PN_BUCK0_ENABLE_MASK | REG_PN_BUCK0_MODE_MASK; // use bucket 0
        val32 |= (e->limit & REG_PN_BUCK0_REF_CNT_M) << REG_PN_BUCK0_REF_CNT_S;
        val32 |= (e->burst_size & REG_PN_BUCK0_SIZE_M) << REG_PN_BUCK0_SIZE_S;
        SF2SW_WREG(PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_RATE_PORT_0 + e->port * 4, &val32, 4);

        /* pkt type */
        SF2SW_RREG(PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_CFG_PORT_0 + e->port * 2, &val32, 4);
	    val32 &= ~(REG_PN_BUCK0_PKT_SEL_M << REG_PN_BUCK0_PKT_SEL_S);
	    val32 |= (e->pkt_type_mask & REG_PN_BUCK0_PKT_SEL_M) << REG_PN_BUCK0_PKT_SEL_S;
        SF2SW_WREG(PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_CFG_PORT_0 + e->port * 2, &val32, 4);
    } else {
        SF2SW_RREG(PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_RATE_PORT_0 + e->port * 4, &val32, 4);
        e->limit = (val32 >> REG_PN_BUCK0_REF_CNT_S) & REG_PN_BUCK0_REF_CNT_M;
        e->burst_size = (val32 >> REG_PN_BUCK0_SIZE_S) & REG_PN_BUCK0_SIZE_M;

        SF2SW_RREG(PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_CFG_PORT_0 + e->port * 2, &val32, 4);
        e->pkt_type_mask = (val32 >> REG_PN_BUCK0_PKT_SEL_S) & REG_PN_BUCK0_PKT_SEL_M;
    }

    up(&sf2_sw->s.conf_sem);
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWMULTIPORT functions ---
int ioctl_extsw_set_multiport_address(uint8_t *addr)
{
    // based on impl5:bcmsw_set_multiport_address_ext()
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
       SF2SW_RREG(PAGE_ARLCTRL, (REG_MULTIPORT_ADDR1_LO + (i * 0x10)), (uint8 *)&cur64, sizeof(cur64)|DATA_TYPE_VID_MAC);
       if ( 0 == memcmp(&v64[0], &cur64[0], 8) )
       {
           return 0;
       }
    }

    /* add new entry */
    for ( i = 0; i < MULTIPORT_CTRL_COUNT; i++ )
    {
        SF2SW_RREG(PAGE_ARLCTRL, REG_MULTIPORT_CTRL, (uint8 *)&v16, 2);
        if ( 0 == (v16 & (MULTIPORT_CTRL_EN_M << (i << 1))))
        {
            v16 |= (1<<MULTIPORT_CTRL_DA_HIT_EN) | (MULTIPORT_CTRL_ADDR_CMP << (i << 1));
            SF2SW_WREG(PAGE_ARLCTRL, REG_MULTIPORT_CTRL, (uint8 *)&v16, 2);
            *(uint16*)(&v64[0]) = 0;
            memcpy(&v64[2], addr, 6);
            SF2SW_WREG(PAGE_ARLCTRL, (REG_MULTIPORT_ADDR1_LO + (i * 0x10)), (uint8 *)&v64, sizeof(v64)|DATA_TYPE_VID_MAC);
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
            v32 = imp_pbmap[1];
#else
            v32 = PBMAP_MIPS;
#endif
            SF2SW_WREG(PAGE_ARLCTRL, (REG_MULTIPORT_VECTOR1 + (i * 0x10)), (uint8 *)&v32, sizeof(v32));

            /* Set multiport VLAN control based on U/V_FWD_MAP;
               This is required so that VLAN tagged frames matching Multiport Address are forwarded according to V/U forwarding map */
            SF2SW_RREG(PAGE_8021Q_VLAN, REG_VLAN_MULTI_PORT_ADDR_CTL, &v16, sizeof(v16));
            v16 |=  (EN_MPORT_V_FWD_MAP | EN_MPORT_U_FWD_MAP) << (i*EN_MPORT_V_U_FWD_MAP_S) ;
            SF2SW_WREG(PAGE_8021Q_VLAN, REG_VLAN_MULTI_PORT_ADDR_CTL, &v16, sizeof(v16));

            return 0;
        }
    }
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWDEEPGREENMODE functions ---
#if defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE)

static int tr_imp_port_down(enetx_port_t *self, void *_ctx)
{
    if (self->p.mac && self->p.mac->mac_id == IMP_PORT_ID && self->dev != NULL)
    {
        netif_carrier_off(self->dev);
        printk((KERN_CRIT "%s (%s switch port: %d) (Logical Port: %d) Virtual link DOWN\n"),
               self->dev->name, PORT_ON_ROOT_SW(self)?"Int":"Ext", self->p.mac->mac_id, 
               PHYSICAL_PORT_TO_LOGICAL_PORT(self->p.mac->mac_id, PORT_ON_ROOT_SW(self)?0:1));
        return 1;
    }
    return 0;
}
static int tr_imp_port_up(enetx_port_t *self, void *_ctx)
{
    if (self->p.mac && self->p.mac->mac_id == IMP_PORT_ID && self->dev != NULL)
    {
        netif_carrier_on(self->dev);
        printk((KERN_CRIT "%s (%s switch port: %d) (Logical Port: %d) Virtual link UP\n"),
               self->dev->name, PORT_ON_ROOT_SW(self)?"Int":"Ext", self->p.mac->mac_id,
               PHYSICAL_PORT_TO_LOGICAL_PORT(self->p.mac->mac_id, PORT_ON_ROOT_SW(self)?0:1));
        return 1;
    }
    return 0;
}
static void _ethsw_deep_green_mode_activate(void)
{
    // based on impl5:ethsw_deep_green_mode_activate()
    uint32 reg_val32;

    printk("===> Activate Deep Green Mode\n");

    port_traverse_ports(sf2_sw, tr_imp_port_down, PORT_CLASS_PORT, NULL);

    /* Disable IMP port */
    _imp_ports_op(IMP_PORTS_DISABLE);

    /* Disable all ports' MAC TX/RX clocks (IMPORTANT: prevent all traffic into Switch while its clock is lowered) */
    SF2SW_RREG(PAGE_CONTROL, REG_LOW_POWER_EXP1, &reg_val32, 4);
    reg_low_power_exp1 = reg_val32;    //Store register's value so that we can restore this value when we disable Deep Green Mode
    reg_val32 |= REG_LOW_POWER_EXP1_SLEEP_MACCLK_PORT_MASK;
    SF2SW_WREG(PAGE_CONTROL, REG_LOW_POWER_EXP1, &reg_val32, 4);

    platform_set_clock_slow();
}

static void _ethsw_deep_green_mode_deactivate(void)
{
    // based on impl5:ethsw_deep_green_mode_deactivate()
    uint32 reg_val32;

    printk("<=== Deactivate Deep Green Mode\n");

    platform_set_clock_normal();

    /* Enable IMP port */
    _imp_ports_op(IMP_PORTS_ENABLE);

    /* Set IMP port to link up */
    SF2SW_RREG(PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(IMP_PORT_ID), &reg_val32, 4);
    reg_val32 |= REG_CONTROL_MPSO_LINKPASS;
    SF2SW_WREG(PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(IMP_PORT_ID), &reg_val32, 4);

    /* Re-enable in-use ports' MAC TX/RX clocks (leave unused port's MAC TX/RX clock disabled) */
    reg_val32 = reg_low_power_exp1;    //Restore register's previous value from before we enabled Deep Green Mode
    SF2SW_WREG(PAGE_CONTROL, REG_LOW_POWER_EXP1, &reg_val32, 4);

    port_traverse_ports(sf2_sw, tr_imp_port_up, PORT_CLASS_PORT, NULL);
}

static int tr_phy_link_up(enetx_port_t *port, void *_ctx)
{
    int *any_link_up = (int *)_ctx;
    phy_dev_t *phy = get_active_phy(port->p.phy);

    if (phy && phy->link)
    {
        *any_link_up = 1;    // found one port with link up
        return 1;           // stop scanning
    }
    return 0;
}

void port_sf2_deep_green_mode_handler(void)
{
    // based on impl5:ethsw_deep_green_mode_handler()
    int any_link_up = 0;
    
    // check if any port is linked up?
    port_traverse_ports(root_sw, tr_phy_link_up, PORT_CLASS_PORT, &any_link_up);

    /* (special case) If user uses WebGUI to disable Deep Green Mode feature then deactivate Deep Green Mode if necessary and exit function */
    if ( !deep_green_mode_enabled )  {
        //printk("Deep Green Mode feature is disabled in WebGUI.  Do nothing.\n");
        if (deep_green_mode_activated) {
            deep_green_mode_activated = 0;
            //printk("Deep Green Mode was activated.  Deactivating Deep Green Mode now...\n");
            _ethsw_deep_green_mode_deactivate();
        }
        return;
    }

    /* Only activate Deep Green Mode if all ports are linked down and Deep Green Mode wasn't already enabled */
    if ( (!any_link_up) && (!deep_green_mode_activated) ) {
        deep_green_mode_activated = 1;
        _ethsw_deep_green_mode_activate();
    /* Only deactivate Deep Green Mode if some ports are linked up and Deep Green Mode is currently enabled */
    } else if ( any_link_up && (deep_green_mode_activated) )  {
        deep_green_mode_activated = 0;
        _ethsw_deep_green_mode_deactivate();
    }
}

int ioctl_pwrmngt_get_deepgreenmode(int mode)
{
    // based on impl5:BcmPwrMngtGetDeepGreenMode()
    if (!mode) {
        return (deep_green_mode_enabled);
    } else {
        return (deep_green_mode_activated);
    }
}

int ioctl_pwrmngt_set_deepgreenmode(int enable)
{
    // based on impl5:BcmPwrMngtSetDeepGreenMode()
    if (deep_green_mode_enabled != enable) {
        deep_green_mode_enabled = enable;
        port_sf2_deep_green_mode_handler();
        printk("Deep Green Mode feature changed to %s (DGM status: %s)\n", enable?"enabled":"disabled", deep_green_mode_activated?"activated":"deactivated");
    }
    return 0;
}

#endif /* defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE) */

// =========== sf2 public functions =======================

void extsw_set_mac_address(enetx_port_t *port)
{
    uint8_t *addr;
    // only operate if port is on sf2
    if (port != sf2_sw  && port->p.parent_sw != sf2_sw)
        return;
    addr = port->dev->dev_addr;
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
    /* Static MAC works for all scenarios, so just add multiport
     * MAC only when multiple IMP ports are in use. */
    ioctl_extsw_set_multiport_address(addr);
#else
    _enet_arl_write_ext(addr, 0, ARL_DATA_ENTRY_VALID|ARL_DATA_ENTRY_STATIC|IMP_PORT_ID);
#endif
}

#if defined(CRB_5X3_QGPHY3_WORKAROUND) /* 5x3 crossbar */
/* these functions are for qgphy3 workaround */
int sf2_set_mac_port_state(int phy_port, int link, int speed, int duplex)
{
    // based on imp5\bcmsw.c:bcmsw_set_mac_port_state()
    uint8 v8;

    v8 = REG_PORT_STATE_OVERRIDE;
    v8 |= (link != 0)? REG_PORT_STATE_LNK: 0;
    v8 |= (duplex != 0)? REG_PORT_STATE_FDX: 0;

    if (speed == 1000)
        v8 |= REG_PORT_STATE_1000;
    else if (speed == 100)
        v8 |= REG_PORT_STATE_100;

    down(&sf2_sw->s.conf_sem);
    SF2SW_WREG(PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(phy_port), &v8, 1);
    up(&sf2_sw->s.conf_sem);

    return 0;
}

void sf2_force_mac_up(int port)
{
    // based on imp5\eth_pwrmngt.c:ethsw_force_mac_up()
    uint32 reg_val32 = 0;
    
    SF2SW_RREG(PAGE_CONTROL, REG_LOW_POWER_EXP1, &reg_val32, 4);
    reg_val32 &= ~((1<<port) << REG_LOW_POWER_EXP1_SLEEP_MACCLK_PORT_SHIFT);
    SF2SW_WREG(PAGE_CONTROL, REG_LOW_POWER_EXP1, &reg_val32, 4);
#if defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE)
    reg_low_power_exp1 &= ~((1<<port) << REG_LOW_POWER_EXP1_SLEEP_MACCLK_PORT_SHIFT);
#endif
}

#endif /* defined(CRB_5X3_QGPHY3_WORKAROUND) */

//#define CC_PREPEND_DATA_TEST

#if defined(CC_PREPEND_DATA_TEST)
#define PREPEND_DATA_TEST_SIZE 16

static int __prepend_data_test(void *arg_p)
{
    BCM_runnerPrepend_t *prepend_p = arg_p;
    int i;

    for(i=0; i<PREPEND_DATA_TEST_SIZE; ++i)
    {
        prepend_p->data[i] = i;
    }

    prepend_p->size = PREPEND_DATA_TEST_SIZE;

    return 0;
}
#endif

extern void add_unspecified_ports(enetx_port_t *sw, uint32_t port_map, uint32_t imp_map);

static int tr_sw_add_unspecified_ports(enetx_port_t *sw, void *_ctx)
{
    /* this function creates all unspecified port objects when multiple IMP SF2 ports are connected to runner.
       these need to be explicitly created, since board param file does not specify them.
       These port objects are used by ioctl to access hw info.
       Only device with multiple IMP ports will call this function.
    */
    if (sw == root_sw)
    {
#if (defined(CONFIG_BCM963158) && defined(ARCHER_DEVICE))
        /* for Archer on BCM63158, only port 8 is used as IMP port */
        add_unspecified_ports(sw, chip_arch_all_portmap[1], (1<<IMP_PORT_ID));
#else
        add_unspecified_ports(sw, chip_arch_all_portmap[0], chip_arch_mgmt_portmap[0]);
#endif
    }
    else
        add_unspecified_ports(sw, chip_arch_all_portmap[1], chip_arch_mgmt_portmap[1]);
    return 0;
}


int enetxapi_post_parse(void)
{
    sema_init(&bcm_link_handler_config, 1);
    
    port_traverse_ports(root_sw, tr_sw_add_unspecified_ports, PORT_CLASS_SW, NULL);
    return 0;
}

/* To deal with limited serial port output buffer size issue */
static void console_short_printk(char *buf)
{
#define CON_BUF_SIZE 256
    char *end, *bf = buf;
    char ch;
    int slen, tlen = strlen(buf);

    for(;;) {
        slen = strlen(bf);
        if (slen < CON_BUF_SIZE-1)
            end = bf + slen;
        else
            end = bf + CON_BUF_SIZE-1;
        ch = *end;
        *end = 0;
        printk(bf);
        *end = ch;
        bf = end;
        if (bf == buf + tlen)
            break;
    }
}

#if defined(ARCHER_DEVICE)
static enetx_port_t *blog_chnl_array[BCM_ENET_SYSPORT_BLOG_CHNL_MAX] = {};

static inline enetx_port_t *port_by_blog_chnl(int blog_chnl)
{
    // Archer platforms blog_chnl is not unit_port based.
    return (blog_chnl >= BCM_ENET_SYSPORT_BLOG_CHNL_MAX) ? NULL : blog_chnl_array[blog_chnl];
}

static int tr_fill_sysp_conf(enetx_port_t *port, void *ctxt)
{
    bcmSysport_Config_t *conf = (bcmSysport_Config_t*)ctxt;

    if (port->has_interface) {
        bcmSysport_BlogChnl_t *ch = &(conf->blog_chnl[port->n.blog_chnl]);
        conf->nbr_of_blog_channels++;
#if defined(CONFIG_BCM963178) || defined(CONFIG_BCM963158)
        ch->sysport = 0;
        ch->port = port->p.mac->mac_id;
#if defined(PORT_WITH_8TXQ)
        ch->nbr_of_queues = (port->p.mac->mac_id == PORT_WITH_8TXQ) ? 8 : 4;
#else
        ch->nbr_of_queues = 4;
#endif
#elif defined(CONFIG_BCM947622)
        if (PORT_ON_ROOT_SW(port)) {
            ch->sysport = port->p.mac->mac_id;
            ch->port = 0;
        } else {
            ch->sysport = port->p.parent_sw->s.parent_port->p.mac->mac_id;
            ch->port = port->p.mac->mac_id;
        }
        ch->nbr_of_queues = 8;
#endif
        blog_chnl_array[port->n.blog_chnl] = port;
        ch->dev = port->dev;

        enet_dbgv("dev %s nbr_of_blog_ch=%d blog_chnl=%d[%d/%d]\n", port->dev->name, conf->nbr_of_blog_channels, port->n.blog_chnl, ch->sysport, ch->port);
    }
    return 0;
}

static int bcmenet_sysport_config(void)
{
    bcmFun_t *enet_sysport_config = bcmFun_get(BCM_FUN_ID_ENET_SYSPORT_CONFIG);
    bcmSysport_Config_t conf;

    if(!enet_sysport_config)
    {
        enet_err("Sysport Configuration is not available\n");
        return -1;
    }

    memset(&conf, 0, sizeof(conf));
#if defined(CONFIG_BCM963178) || defined(CONFIG_BCM963158)
    conf.nbr_of_sysports = 1;
    conf.sysport[0].mode = BCM_ENET_SYSPORT_MODE_INTERNAL_BRCM_SW;
    enet_dbgv(" sysport0 mode= internal_sw\n");
#elif defined(CONFIG_BCM947622)
    conf.nbr_of_sysports = 2;
    conf.sysport[0].mode = unit_port_array[0][0]->p.child_sw ? BCM_ENET_SYSPORT_MODE_EXTERNAL_BRCM_SW : BCM_ENET_SYSPORT_MODE_PORT;
    conf.sysport[1].mode = unit_port_array[0][1]->p.child_sw ? BCM_ENET_SYSPORT_MODE_EXTERNAL_BRCM_SW : BCM_ENET_SYSPORT_MODE_PORT;
    enet_dbgv(" sysport0 mode= %s\n", conf.sysport[0].mode == BCM_ENET_SYSPORT_MODE_PORT ? "port" :"ext_sw" );
    enet_dbgv(" sysport1 mode= %s\n", conf.sysport[1].mode == BCM_ENET_SYSPORT_MODE_PORT ? "port" :"ext_sw" );
#endif
    port_traverse_ports(root_sw, tr_fill_sysp_conf, PORT_CLASS_PORT, &conf);

    enet_sysport_config(&conf);
    bcmenet_sysport_q_map(queRemap);
    return 0;
}
static int bcmenet_is_bonded_lan_wan_port(void *ctxt)
{
    /* based on impl5\bcmenet.c:bcmenet_is_bonded_lan_wan_port() */
    int ret_val = 0;
#if defined(CONFIG_BCM_KERNEL_BONDING)
    int blog_chnl = *((int*)ctxt);
    enetx_port_t *port = port_by_blog_chnl(blog_chnl);

    if (port && port->p.bond_grp)
    {
        if (port->p.bond_grp->is_lan_wan_cfg &&
            port->p.bond_grp->lan_wan_port == port )
        {
            ret_val = 1;
        }
    }
#endif
    return ret_val;
}

static int bcmenet_is_wan_port(void *ctxt)
{
    int blog_chnl = *((int*)ctxt);
    enetx_port_t *port = port_by_blog_chnl(blog_chnl);
    
    return (port && !bcmenet_is_bonded_lan_wan_port(ctxt)) ? PORT_ROLE_IS_WAN(port) : 0;
}
#endif //ARCHER_DEVICE

int enetxapi_post_sf2_config(void)
{
    char *buf;
    int sz = 0;
#if defined(CONFIG_BCM_IEEE1905)
    uint8_t ieee1905_multicast_mac[6] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x13};
#endif

    serdes_work_around(NULL); 

    if (sw_print_mac_phy_info(root_sw, &buf, &sz)) {
        console_short_printk(buf);
        kfree(buf);
    }

    sf2_rgmii_config();

#if defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE)
#if defined(CONFIG_BCM963178)
    /* Preprogram switch variable speed clock but don't switch to it yet */
    pmc_switch_clock_lowpower_mode (1);
#endif
    port_sf2_deep_green_mode_handler();
#endif

#if defined(CONFIG_BCM_IEEE1905)
    ioctl_extsw_set_multiport_address(ieee1905_multicast_mac);
#endif

#if defined(ARCHER_DEVICE)
    bcmFun_reg(BCM_FUN_ID_ENET_IS_WAN_PORT, bcmenet_is_wan_port);
    bcmFun_reg(BCM_FUN_ID_ENET_IS_BONDED_LAN_WAN_PORT, bcmenet_is_bonded_lan_wan_port);
    bcmenet_sysport_config();
#endif
#if defined(CC_PREPEND_DATA_TEST)
    bcmFun_reg(BCM_FUN_ID_RUNNER_PREPEND, __prepend_data_test);
#endif
    return 0;
}

#if defined(ARCHER_DEVICE) && defined(SF2_EXTERNAL)
static int bcmenet_tm_enable_set(void *ctxt)
{
    uint16_t val;
    uint8_t  val8;

    int enable = *((int*)ctxt);
    
    if (enable) {
        // turn off 53134 to 47622 flowcontrol
        SF2SW_RREG(PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(IMP_PORT_ID), &val8, sizeof(val8));
        val8 &= ~REG_PORT_STATE_TX_FLOWCTL;
        SF2SW_WREG(PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(IMP_PORT_ID), &val8, sizeof(val8));
        
    } else {
        // turn on 53134 to 47622 flowcontrol
        SF2SW_RREG(PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(IMP_PORT_ID), &val8, sizeof(val8));
        val8 |= REG_PORT_STATE_TX_FLOWCTL;
        SF2SW_WREG(PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(IMP_PORT_ID), &val8, sizeof(val8));

        // 53134 only gen IMP0 tx pause using total pause mechanism
        SF2SW_RREG(PAGE_FLOW_CTRL, REG_FC_PAUSE_DROP_CTRL, &val, sizeof(val));
        val &= ~FC_PAUSE_TX_IMP0_TXQ_EN;
        SF2SW_WREG(PAGE_FLOW_CTRL, REG_FC_PAUSE_DROP_CTRL, &val, sizeof(val));
    }
    return 0;
}
#endif

// =========== sf2 switch ops =============================
void port_sf2_sw_fast_age(enetx_port_t *sw)
{
    //enet_dbgv(" sw=%s\n", sw->obj_name);
    _fast_age_start_done_ext(FAST_AGE_START_DONE | FAST_AGE_DYNAMIC);
}

int port_sf2_sw_init(enetx_port_t *self)
{
    sf2_sw = self;    /* init sf2_sw shortcut */

    SW_SET_HW_FWD(self);	/* initially hw fwd for all ports on switch */
    platform_enable_p8_rdp_sel();
    platform_set_imp_speed();

    _extsw_setup_imp_ports();
    _extsw_setup_imp_fwding();

    // configure trunk groups if required.
    _extsw_port_trunk_init();

    // set ARL AGE_DYNAMIC bit for aging operations
    port_sf2_sw_fast_age(self);

    _sf2_qos_default();
#if !defined(SF2_EXTERNAL)
    _sf2_conf_acb_conges_profile(0);
#endif
    if (queThreConfMode != ThreModeManual)
    {
        _sf2_conf_que_thred(); // for acb testing
    }

    /* Register ARL Entry clear routine */
    bcmFun_reg(BCM_FUN_IN_ENET_CLEAR_ARL_ENTRY, remove_arl_entry_wrapper);

#if defined(ARCHER_DEVICE) && defined(SF2_EXTERNAL)
    {
        int tm_en = 0;
        bcmenet_tm_enable_set(&tm_en);
        bcmFun_reg(BCM_FUN_ID_ENET_TM_EN_SET, bcmenet_tm_enable_set);
    }
#endif
    return 0;
}

int port_sf2_sw_uninit(enetx_port_t *self)
{
    bcmFun_dereg(BCM_FUN_IN_ENET_CLEAR_ARL_ENTRY);
    return 0;
}

void port_sf2_sw_stats_clear(enetx_port_t *self)
{
    uint32_t global_cfg, rst_mib_en;

    if (self != sf2_sw)
        return;

#if defined(CONFIG_BCM_KERNEL_BONDING)
    {   // switch clear all port mib at one time, so notify each port stat clear
        bcmFun_t *bcmFun = bcmFun_get(BCM_FUN_ID_BOND_CLR_SLAVE_STAT);
        if (bcmFun) {
            int i;
            for (i = 0; i < self->s.port_count; i++) {
                enetx_port_t *port = self->s.ports[i];
                if (port && port->dev)
                    bcmFun(port->dev);
            }
        }
    }
#endif 

    rst_mib_en = 0xffff;    // enable clearing of all ports
    SF2SW_WREG(PAGE_MANAGEMENT, REG_RST_MIB_CNT_EN, (uint8_t*)&rst_mib_en, 4);

    // toggle global reset mib bit
    SF2SW_RREG(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &global_cfg, 4);
    global_cfg |= GLOBAL_CFG_RESET_MIB;
    SF2SW_WREG(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, (uint8_t*)&global_cfg, 4);
    global_cfg &= ~GLOBAL_CFG_RESET_MIB;
    SF2SW_WREG(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, (uint8_t*)&global_cfg, 4);

    udelay(50);  // hw need time to clear mibs
}


#if defined(CONFIG_BLOG) && defined(CONFIG_BCM_KERNEL_BONDING)

static int tr_update_trunk_chnl_rx(enetx_port_t *port, void *_ctx)
{
    bond_info_t *bond_grp = (bond_info_t *)_ctx;
    bcmFun_t *enet_bond_rx_port_map = bcmFun_get(BCM_FUN_ID_ENET_BOND_RX_PORT_MAP);
    bcmEnet_BondRxPortMap_t map;

    if (!bond_grp || port->p.bond_grp == bond_grp) {
        map.blog_chnl = port->n.blog_chnl;
        map.blog_chnl_rx = port->n.blog_chnl_rx;
        enet_bond_rx_port_map(&map);
    }
    return 0;
}

extern bond_info_t bond_grps[];
int sw_config_trunk_chnl_rx(enetx_port_t *sw, enetx_port_t *port, int grp_no, int add)
{
    bond_info_t *bond_grp = &bond_grps[grp_no];

    if (bcmFun_get(BCM_FUN_ID_ENET_BOND_RX_PORT_MAP)) {
        if (bond_grp->port_count)
            port_traverse_ports(root_sw, tr_update_trunk_chnl_rx, PORT_CLASS_PORT, bond_grp);

        if (!add)       // update removed member
            tr_update_trunk_chnl_rx(port, NULL);
    }
    return 0;
}
#else // !CONFIG_BLOG
int sw_config_trunk_chnl_rx(enetx_port_t *sw, enetx_port_t *port, int grp_no, int add)
{
    return 0;
}
#endif //!CONFIG_BLOG

int port_sf2_sw_config_trunk(enetx_port_t *sw, enetx_port_t *port, int grp_no, int add)
{
    /* based on impl5:bcmsw_config_trunk() */
    uint16_t v16;

    down(&sf2_sw->s.conf_sem);
    SF2SW_RREG(PAGE_MAC_BASED_TRUNK, REG_TRUNK_GRP_CTL + (2*grp_no), &v16, 2);
    if (add)
    {
        v16 |= ( ( (1<< port->p.mac->mac_id) & TRUNK_EN_GRP_M ) << TRUNK_EN_GRP_S );
        enet_dbg("ADD : port %s to group <%d>; New pmap <0x%02x>\n", port->obj_name, grp_no, v16);
    }
    else
    {
        v16 &= ~( ( (1<< port->p.mac->mac_id) & TRUNK_EN_GRP_M ) << TRUNK_EN_GRP_S );
        enet_dbg("REM : port %s to group <%d>; New pmap <0x%02x>\n", port->obj_name, grp_no, v16);
    }
    SF2SW_WREG(PAGE_MAC_BASED_TRUNK, REG_TRUNK_GRP_CTL + (2*grp_no), &v16, 2);
    up(&sf2_sw->s.conf_sem);

    return sw_config_trunk_chnl_rx(sw, port, grp_no, add);
}

int port_sf2_sw_update_pbvlan(enetx_port_t *sw, unsigned int pmap)
{
    int i;

    if (pmap == 0) return 0;    //nothing to do

    for (i=0; i < BP_MAX_SWITCH_PORTS; i++ )
    {
        if (pmap & (1<<i))
        {
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
            _extsw_set_pbvlan(i, pmap | (1 << cur_port_imp_map[i]));
#else
            _extsw_set_pbvlan(i, pmap | (1 << IMP_PORT_ID));
#endif
        }
    }
    return 0;
}

// ----------- SIOCETHSWCTLOPS ETHSWSWITCHING functions ---

static uint16_t dis_learning_ext = 0x0100; /* This default value does not matter */

int port_sf2_sw_hw_sw_state_set(enetx_port_t *sw, unsigned long state)
{
    uint16_t val;
    /* based on impl5:ethsw_set_hw_switching() */

    if (SW_IS_HW_FWD(sw) == state) {
        return 0;
    }

    down(&sw->s.conf_sem);
    if (state == HW_SWITCHING_ENABLED) {
        /* restore disable learning register */
        SF2SW_WREG(PAGE_CONTROL, REG_DISABLE_LEARNING, &dis_learning_ext, 2);
        SW_SET_HW_FWD(sw);
    }
    else {
        /* Save disable_learning_reg setting */
        SF2SW_RREG(PAGE_CONTROL, REG_DISABLE_LEARNING, &dis_learning_ext, 2);

        /* disable learning on all ports */
        val = PBMAP_ALL;
        SF2SW_WREG(PAGE_CONTROL, REG_DISABLE_LEARNING, &val, 2);

        /* flush arl table dynamic entries */
        port_sf2_sw_fast_age(sw);
        SW_CLR_HW_FWD(sw);
    }

    up(&sw->s.conf_sem);
    return 0;
}

int port_sf2_sw_hw_sw_state_get(enetx_port_t *sw)
{
    return SW_IS_HW_FWD(sw)?HW_SWITCHING_ENABLED:HW_SWITCHING_DISABLED;
}


// =========== sf2 port ops =============================

void port_sf2_fast_age(enetx_port_t *port)
{
    uint8_t ctrl;

    //enet_dbgv(" port=%s\n", port->obj_name);
    ctrl = port->p.mac->mac_id;
    SF2SW_WREG(PAGE_CONTROL, REG_FAST_AGING_PORT, &ctrl, 1);
    ctrl = FAST_AGE_START_DONE | FAST_AGE_DYNAMIC | FAST_AGE_PORT;
    _fast_age_start_done_ext(ctrl);
}

#ifdef EMBEDDED_BRCMTAG_TX_INSERT
// based on impl5:bcm63xx_fkb_put_tag()
static inline void enet_fkb_put_tag(FkBuff_t * fkb_p,
        struct net_device * dev, unsigned int port_map)
{
    int i;
    int tailroom;
    uint16 *from = (uint16*)fkb_p->data;
    BcmEnet_hdr2 *pHdr = (BcmEnet_hdr2 *)from;

    if (ntohs(pHdr->brcm_type) != BRCM_TYPE2)
    {
        uint16 * to = (uint16*)fkb_push(fkb_p, BRCM_TAG_TYPE2_LEN);
        pHdr = (BcmEnet_hdr2 *)to;
        for ( i=0; i<ETH_ALEN; *to++ = *from++, i++ ); /* memmove 2 * ETH_ALEN */
        /* set port of ingress brcm tag */
        pHdr->brcm_tag = htons(port_map);

    }
    /* set ingress brcm tag and TC bit */
    pHdr->brcm_type = htons(BRCM_TAG2_EGRESS | (SKBMARK_GET_Q_PRIO(fkb_p->mark) << 10));
    tailroom = ETH_ZLEN + BRCM_TAG_TYPE2_LEN - fkb_p->len;
    if (tailroom > 0)
    {
        fkb_pad(fkb_p, tailroom);
        fkb_p->dirty_p = _to_dptr_from_kptr_(fkb_p->data + fkb_p->len);
    }
}

// based on impl5:bcm63xx_skb_put_tag()
static struct sk_buff *enet_skb_put_tag(struct sk_buff *skb,
        struct net_device *dev, unsigned int port_map)
{
    BcmEnet_hdr2 *pHdr = (BcmEnet_hdr2 *)skb->data;
    int i, headroom;
    int tailroom;

    if (ntohs(pHdr->brcm_type) == BRCM_TYPE2)
    {
        headroom = 0;
        tailroom = ETH_ZLEN + BRCM_TAG_TYPE2_LEN - skb->len;
    }
    else
    {
        headroom = BRCM_TAG_TYPE2_LEN;
        tailroom = ETH_ZLEN - skb->len;
    }

    if (tailroom < 0)
    {
        tailroom = 0;
    }

#if defined(CONFIG_BCM_USBNET_ACCELERATION)
    if ((skb_writable_headroom(skb) < headroom) || (skb_tailroom(skb) < tailroom))
#else
    if ((skb_headroom(skb) < headroom) || (skb_tailroom(skb) < tailroom))
#endif
    {
        struct sk_buff *oskb = skb;
        skb = skb_copy_expand(oskb, headroom, tailroom, GFP_ATOMIC);
        kfree_skb(oskb);
        if (!skb)
        {
            return NULL;
        }
    }
#if defined(CONFIG_BCM_USBNET_ACCELERATION)
    else if ((headroom != 0) && (skb->clone_wr_head == NULL))
#else
    else if ((headroom != 0) && !(skb_clone_writable(skb, headroom)))
#endif
    {
        skb = skb_unshare(skb, GFP_ATOMIC);
        if (!skb)
        {
            return NULL;
        }
    }

    if (tailroom > 0)
    {
        if (skb_is_nonlinear(skb))
        {
            /* Non linear skb whose skb->len is < minimum Ethernet Packet Length
               (ETHZLEN or ETH_ZLEN + BroadcomMgmtTag Length) */
            if (skb_linearize(skb))
            {
                return NULL;
            }
        }
        memset(skb->data + skb->len, 0, tailroom);  /* padding to 0 */
        skb_put(skb, tailroom);
    }

    if (headroom != 0)
    {
        uint16 *to, *from;
        BcmEnet_hdr2 *pHdr2 = (BcmEnet_hdr2 *)skb_push(skb, headroom);
        to = (uint16*)pHdr2;
        from = (uint16*)(skb->data + headroom);
        for ( i=0; i<ETH_ALEN; *to++ = *from++, i++ ); /* memmove 2 * ETH_ALEN */
        /* set ingress brcm tag and TC bit */
        pHdr2->brcm_type = htons(BRCM_TAG2_EGRESS | (SKBMARK_GET_Q_PRIO(skb->mark) << 10));
        pHdr2->brcm_tag  = htons(port_map);
        if (skb_mac_header_was_set(skb))
            skb->mac_header -= headroom;
        /* network_header and transport_header are unchanged */
    }
    return skb;
}

/* insert broadcom tag for external switch port */
int port_sf2_tx_pkt_mod(enetx_port_t *port, pNBuff_t *pNBuff, uint8_t **data, uint32_t *len, unsigned int port_map)
{
    FkBuff_t *pFkb = 0;
    struct sk_buff *skb = 0;

    if (IS_FKBUFF_PTR(*pNBuff))
    {
        FkBuff_t * pFkbOrig = PNBUFF_2_FKBUFF(*pNBuff);

#if defined(CONFIG_BCM_ARCHER_SIM)
        if(SKBMARK_GET_IQPRIO_MARK(pFkbOrig->mark))
        {
            return 0;
        }
#endif
//enet_err("len=%d dirty=%d\n", pFkbOrig->len, (uint32)(pFkbOrig->dirty_p - pFkbOrig->data));
        pFkb = fkb_unshare(pFkbOrig);

        if (pFkb == FKB_NULL)
        {
            fkb_free(pFkbOrig);
            INC_STAT_TX_DROP(port,tx_dropped_no_fkb);
            return -1;
        }
        enet_fkb_put_tag(pFkb, port->dev, port_map); /* Portmap for external switch */
        *data = (void *)pFkb->data;
        *len  = pFkb->len;
        *pNBuff = PBUF_2_PNBUFF((void*)pFkb,FKBUFF_PTR);
    }
    else
    {
        skb = PNBUFF_2_SKBUFF(*pNBuff);
        skb = enet_skb_put_tag(skb, port->dev, port_map);    /* Portmap for external switch and also pads to 0 */
        if (skb == NULL) {
            INC_STAT_TX_DROP(port,tx_dropped_no_skb);
            return -1;
        }
        *data = (void *)skb->data;   /* Re-encode pNBuff for adjusted data and len */
        *len  = skb->len;
        *pNBuff = PBUF_2_PNBUFF((void*)skb,SKBUFF_PTR);
    }
    return 0;
}
#endif // EMBEDDED_BRCMTAG_TX_INSERT

#ifdef EMBEDDED_BRCMTAG_RX_REMOVE
static inline int bcm_mvtags_len(char *ethHdr)
{
    unsigned int end_offset = 0;
    BcmEnet_hdr2* bhd;
    uint16 brcm_type;

    bhd = (BcmEnet_hdr2*)ethHdr;
    brcm_type = ntohs(bhd->brcm_type);
    if (brcm_type == BRCM_TYPE2)
    {
        end_offset += BRCM_TAG_TYPE2_LEN;
    }

    return end_offset;
}

/* based on impl5:bcm_type_trans(), bcm_mvtags_len() */
int port_sf2_rx_pkt_mod(enetx_port_t *port, struct sk_buff *skb)
{
    unsigned int end_offset = 0, from_offset = 0;
    uint16 *to, *end, *from;

    skb_reset_mac_header(skb);
    end_offset = bcm_mvtags_len(skb->data);
    if (end_offset)
    {
        from_offset = OFFSETOF(struct ethhdr, h_proto);

        to = (uint16*)(skb->data + from_offset + end_offset) - 1;
        end = (uint16*)(skb->data + end_offset) - 1;
        from = (uint16*)(skb->data + from_offset) - 1;

        while ( to != end )
            *to-- = *from--;
    }

    skb_set_mac_header(skb, end_offset);

    skb_pull(skb, end_offset);
    return 0;
}
#endif // EMBEDDED_BRCMTAG_RX_REMOVE

int port_sf2_port_role_set(enetx_port_t *self, port_netdev_role_t role)
{
    /* based on impl5:bcmsw_config_wan() */
    uint16_t wan_port_map;
    bcmFun_t *enet_port_role_notify = bcmFun_get(BCM_FUN_ID_ENET_PORT_ROLE_NOTIFY);

   /* Configure WAN port */
    SF2SW_RREG(PAGE_CONTROL, REG_WAN_PORT_MAP, &wan_port_map, sizeof(wan_port_map));

    if (role == PORT_NETDEV_ROLE_WAN)
    {
        wan_port_map |= (1<<self->p.mac->mac_id); /* Add the WAN port in the port map */
    }
    else
    {
        wan_port_map &= ~(1<<self->p.mac->mac_id); /* remove the WAN port in the port map */
    }
    SF2SW_WREG(PAGE_CONTROL, REG_WAN_PORT_MAP, &wan_port_map, sizeof(wan_port_map));

    enet_dbg(" %s port %s as WAN; wan_pmap <0x%02x>\n", (role==PORT_NETDEV_ROLE_WAN)?"Add":"Remove", self->obj_name, wan_port_map);
    /* Disable learning */
    SF2SW_WREG(PAGE_CONTROL, REG_DISABLE_LEARNING, &wan_port_map, sizeof(wan_port_map));

    /* NOTE : No need to change the PBVLAN map -- switch logic does not care about pbvlan when the port is WAN */

    /* NOTE: For multiple IMP port products, switch will send all traffic from WAN to P8
       but we should use CFP to send it to correct port based on port grouping
       TBD */

    /* registered modules need to be aware of port role changes */
    if (enet_port_role_notify)
    {
        BCM_EnetPortRole_t port_role;

#if defined(CONFIG_BCM963178) || defined(CONFIG_BCM963158)
        port_role.sysport = 0;
        port_role.port = self->p.mac->mac_id;
#elif defined(CONFIG_BCM947622)
        port_role.sysport = self->p.parent_sw->s.parent_port->p.mac->mac_id;
        port_role.port = self->p.mac->mac_id;
#endif
        port_role.is_wan = (self->n.port_netdev_role == PORT_NETDEV_ROLE_WAN);

        enet_port_role_notify(&port_role);
#if defined(ARCHER_DEVICE)
        bcmenet_sysport_q_map(queRemap);
#endif
    }

    return 0;
}

#include "linux/if_bridge.h"
int port_sf2_port_stp_set(enetx_port_t *self, int mode, int state)
{
    // based on impl5:bcm_set_hw_stp()
    int unit = PORT_ON_ROOT_SW(self)?0:1;
    int port = self->p.mac->mac_id;
    int swPort= PHYSICAL_PORT_TO_LOGICAL_PORT(port, unit);
    unsigned char stpVal;

    //enet_dbgv("%s mode=%d state=%d\n", self->obj_name, mode, state);
    switch (mode)
    {
    case STP_MODE_ENABLE:
        root_sw->s.stpDisabledPortMap &= ~(1 << swPort);
        /* set stp state to forward in case bridge STP is off 
           if bridge STP is on, STP state machine will be forced to restart */
        _ethsw_set_stp_mode(unit, port, REG_PORT_STP_STATE_FORWARDING);
        break;
    case STP_MODE_DISABLE:
        root_sw->s.stpDisabledPortMap |= (1 << swPort);
        _ethsw_set_stp_mode(unit, port, REG_PORT_NO_SPANNING_TREE);
        break;
    default:    // STP_MODE_UNCHANGED
        if (state == STP_STATE_UNCHANGED)
            switch (_ethsw_get_stp_state(unit, port))
            {
            case REG_PORT_STP_STATE_FORWARDING: return STP_FORWARDING;
            case REG_PORT_STP_STATE_LEARNING:   return STP_LEARNING;
            case REG_PORT_STP_STATE_LISTENING:  return STP_LISTENING;
            case REG_PORT_STP_STATE_BLOCKING:   return STP_BLOCKING;
            default:                            return STP_DISABLED;
            }

        if (root_sw->s.stpDisabledPortMap & (1<<swPort))
            break;
        switch (state)
        {
        case STP_BLOCKING:     stpVal= REG_PORT_STP_STATE_BLOCKING; break;
        case STP_FORWARDING:   stpVal= REG_PORT_STP_STATE_FORWARDING; break;
        case STP_LEARNING:     stpVal= REG_PORT_STP_STATE_LEARNING; break;
        case STP_LISTENING:    stpVal= REG_PORT_STP_STATE_LISTENING; break;
        case STP_DISABLED:     //stpVal= REG_PORT_STP_STATE_DISABLED; break;
        default:               stpVal= REG_PORT_NO_SPANNING_TREE; break;
        }
        _ethsw_set_stp_mode(unit, port, stpVal);
    }
    return 0;
}

// add by Andrew
// ----------- SIOCETHSWCTLOPS ETHSWMIBDUMP functions ---
int port_sf2_mib_dump_us(enetx_port_t *self, void *ethswctl)
{
    struct ethswctl_data *e = (struct ethswctl_data *)ethswctl;
    unsigned int v32;
    uint8_t data[8] = {0};
    int port = self->p.mac->mac_id;

    {
        SF2SW_RREG(PAGE_CONTROL, REG_LOW_POWER_EXP1, &v32, 4);
        if (v32 & (1<<port)) {
            enet_err("port=%d is in low power mode - mib counters not accessible!!\n", port);    // SLEEP_SYSCLK_PORT for specified port is set
            return 0;
        }
    }

    /* Calculate Tx statistics */
    e->port_stats.txPackets = 0;
    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXUPKTS, &v32, 4);  // Get TX unicast packet count
    e->port_stats.txPackets += v32;
    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXMPKTS, &v32, 4);  // Get TX multicast packet count
    e->port_stats.txPackets += v32;
    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXBPKTS, &v32, 4);  // Get TX broadcast packet count
    e->port_stats.txPackets += v32;

    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXDROPS, &v32, 4);
    e->port_stats.txDrops = v32;

    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXOCTETS, data, DATA_TYPE_HOST_ENDIAN|8);
    e->port_stats.txBytes = *((uint64 *)data);

    /* Calculate Rx statistics */
    e->port_stats.rxPackets = 0;
    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXUPKTS, &v32, 4);  // Get RX unicast packet count
    e->port_stats.rxPackets += v32;
    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXMPKTS, &v32, 4);  // Get RX multicast packet count
    e->port_stats.rxPackets += v32;
    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXBPKTS, &v32, 4);  // Get RX broadcast packet count
    e->port_stats.rxPackets += v32;

    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXDROPS, &v32, 4);
    e->port_stats.rxDrops = v32;

    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXDISCARD, &v32, 4);
    e->port_stats.rxDiscards = v32;

    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXOCTETS, data,  DATA_TYPE_HOST_ENDIAN|8);
    e->port_stats.rxBytes = *((uint64 *)data);

    return 0;
}
// end of add

// ----------- SIOCETHSWCTLOPS ETHSWDUMPMIB functions ---

/* mib dump for ports on external SF2 switch */
int port_sf2_mib_dump(enetx_port_t *self, int all)
{
    /* based on impl5:sf2_bcmsw_dump_mib_ext() */
    unsigned int v32, errcnt;
    uint8_t data[8] = {0};
    int port = self->p.mac->mac_id;

    {
        SF2SW_RREG(PAGE_CONTROL, REG_LOW_POWER_EXP1, &v32, 4);
        if (v32 & (1<<port)) {
            enet_err("port=%d is in low power mode - mib counters not accessible!!\n", port);    // SLEEP_SYSCLK_PORT for specified port is set
            return 0;
        }
    }
    /* Display Tx statistics */
    printk("External Switch Stats : Port# %d\n",port);
    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXUPKTS, &v32, 4);  // Get TX unicast packet count
    printk("TxUnicastPkts:          %10u \n", v32);
    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXMPKTS, &v32, 4);  // Get TX multicast packet count
    printk("TxMulticastPkts:        %10u \n",  v32);
    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXBPKTS, &v32, 4);  // Get TX broadcast packet count
    printk("TxBroadcastPkts:        %10u \n", v32);
    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXDROPS, &v32, 4);
    printk("TxDropPkts:             %10u \n", v32);

    if (all)
    {
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXOCTETS, data, DATA_TYPE_HOST_ENDIAN|8);
        v32 = *((uint64 *)data);
        printk("TxOctetsLo:             %10u \n", v32);
        v32 = *((uint64 *)data) >> 32;
        printk("TxOctetsHi:             %10u \n", v32);
//
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TX64OCTPKTS, &v32, 4);
        printk("TxPkts64Octets:         %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TX127OCTPKTS, &v32, 4);
        printk("TxPkts65to127Octets:    %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TX255OCTPKTS, &v32, 4);
        printk("TxPkts128to255Octets:   %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TX511OCTPKTS, &v32, 4);
        printk("TxPkts256to511Octets:   %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TX1023OCTPKTS, &v32, 4);
        printk("TxPkts512to1023Octets:  %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXMAXOCTPKTS, &v32, 4);
        printk("TxPkts1024OrMoreOctets: %10u \n", v32);
//
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ0PKT, &v32, 4);
        printk("TxQ0Pkts:               %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ1PKT, &v32, 4);
        printk("TxQ1Pkts:               %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ2PKT, &v32, 4);
        printk("TxQ2Pkts:               %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ3PKT, &v32, 4);
        printk("TxQ3Pkts:               %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ4PKT, &v32, 4);
        printk("TxQ4Pkts:               %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ5PKT, &v32, 4);
        printk("TxQ5Pkts:               %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ6PKT, &v32, 4);
        printk("TxQ6Pkts:               %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ7PKT, &v32, 4);
        printk("TxQ7Pkts:               %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXCOL, &v32, 4);
        printk("TxCol:                  %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXSINGLECOL, &v32, 4);
        printk("TxSingleCol:            %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXMULTICOL, &v32, 4);
        printk("TxMultipleCol:          %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXDEFERREDTX, &v32, 4);
        printk("TxDeferredTx:           %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXLATECOL, &v32, 4);
        printk("TxLateCol:              %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXEXCESSCOL, &v32, 4);
        printk("TxExcessiveCol:         %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXFRAMEINDISC, &v32, 4);
        printk("TxFrameInDisc:          %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXPAUSEPKTS, &v32, 4);
        printk("TxPausePkts:            %10u \n", v32);
    }
    else
    {
        errcnt=0;
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXCOL, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXSINGLECOL, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXMULTICOL, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXDEFERREDTX, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXLATECOL, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXEXCESSCOL, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXFRAMEINDISC, &v32, 4);
        errcnt += v32;
        printk("TxOtherErrors:          %10u \n", errcnt);
    }

    /* Display Rx statistics */
    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXUPKTS, &v32, 4);  // Get RX unicast packet count
    printk("RxUnicastPkts:          %10u \n", v32);
    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXMPKTS, &v32, 4);  // Get RX multicast packet count
    printk("RxMulticastPkts:        %10u \n",v32);
    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXBPKTS, &v32, 4);  // Get RX broadcast packet count
    printk("RxBroadcastPkts:        %10u \n",v32);
    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXDROPS, &v32, 4);
    printk("RxDropPkts:             %10u \n",v32);
    SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXDISCARD, &v32, 4);
    printk("RxDiscard:              %10u \n", v32);

    if (all)
    {
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXOCTETS, data,  DATA_TYPE_HOST_ENDIAN|8);
        v32 = *((uint64 *)data);
        printk("RxOctetsLo:             %10u \n", v32);
        v32 = *((uint64 *)data) >> 32;
        printk("RxOctetsHi:             %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXGOODOCT, data,  DATA_TYPE_HOST_ENDIAN|8);
        v32 = *((uint64 *)data);
        printk("RxGoodOctetsLo:         %10u \n", v32);
        v32 = *((uint64 *)data) >> 32;
        printk("RxGoodOctetsHi:         %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXJABBERS, &v32, 4);
        printk("RxJabbers:              %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXALIGNERRORS, &v32, 4);
        printk("RxAlignErrs:            %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXFCSERRORS, &v32, 4);
        printk("RxFCSErrs:              %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXFRAGMENTS, &v32, 4);
        printk("RxFragments:            %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXOVERSIZE, &v32, 4);
        printk("RxOversizePkts:         %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXUNDERSIZEPKTS, &v32, 4);
        printk("RxUndersizePkts:        %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXPAUSEPKTS, &v32, 4);
        printk("RxPausePkts:            %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXSACHANGES, &v32, 4);
        printk("RxSAChanges:            %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXSYMBOLERRORS, &v32, 4);
        printk("RxSymbolError:          %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RX64OCTPKTS, &v32, 4);
        printk("RxPkts64Octets:         %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RX127OCTPKTS, &v32, 4);
        printk("RxPkts65to127Octets:    %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RX255OCTPKTS, &v32, 4);
        printk("RxPkts128to255Octets:   %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RX511OCTPKTS, &v32, 4);
        printk("RxPkts256to511Octets:   %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RX1023OCTPKTS, &v32, 4);
        printk("RxPkts512to1023Octets:  %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXMAXOCTPKTS, &v32, 4);
        printk("RxPkts1024OrMoreOctets: %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXJUMBOPKT , &v32, 4);
        printk("RxJumboPkts:            %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXOUTRANGEERR, &v32, 4);
        printk("RxOutOfRange:           %10u \n", v32);
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXINRANGEERR, &v32, 4);
        printk("RxInRangeErr:           %10u \n", v32);
    }
    else
    {
        errcnt=0;
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXJABBERS, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXALIGNERRORS, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXFCSERRORS, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXFRAGMENTS, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXOVERSIZE, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXUNDERSIZEPKTS, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXSYMBOLERRORS, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXOUTRANGEERR, &v32, 4);
        errcnt += v32;
        SF2SW_RREG(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXINRANGEERR, &v32, 4);
        errcnt += v32;
        printk("RxOtherErrors:          %10u \n", errcnt);

    }

    return 0;
}

uint32_t port_sf2_tx_q_remap(enetx_port_t *port, uint32_t txq)
{
    // based on impl5:bcmeapi_enet_prepare_xmit()
#if defined(PORT_WITH_8TXQ)
    // no remap if port has 8 txqs and is WAN
    if ((port->p.mac->mac_id == PORT_WITH_8TXQ) &&
        (port->n.port_netdev_role == PORT_NETDEV_ROLE_WAN))
        return txq;
#endif
    return (queRemap >> (txq * 4)) & 0xf;
}

#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
uint16_t port_sf2_tx_lb_imp(enetx_port_t *port, uint16_t port_id, void* pHdr)
{
    return port_imp_emac_map[port_id];
}
#endif /* defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT) */

void phy_link_change_cb(void *ctx);

void dslbase_phy_link_change_cb(void *ctx)
{
    phy_dev_t *phy = ctx;
    phy_dev_t *active_end_phy = get_active_phy(phy);    /* if phy is crossbar get actual phy that triggerred event */
    phy_dev_t *first_phy = cascade_phy_get_first(active_end_phy);
    enetx_port_t *p = first_phy->sw_port;

    phy_dev_status_propagate(active_end_phy);
    phy_dev_status_reverse_propagate(active_end_phy);
    p->p.phy_last_change = (jiffies * 100) / HZ;

    if (!phy->link)
        serdes_work_around(first_phy);

    if (p->dev)
    {
        /* Print new status to console */
        link_change_handler(p, active_end_phy->link, 
            speed_macro_2_mbps(active_end_phy->speed), 
            active_end_phy->duplex == PHY_DUPLEX_FULL);

#if defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE)
        port_sf2_deep_green_mode_handler();
#endif
    }
}
