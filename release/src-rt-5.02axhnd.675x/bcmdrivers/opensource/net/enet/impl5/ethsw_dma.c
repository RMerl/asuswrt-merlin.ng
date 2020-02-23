/*
   Copyright 2007-2010 Broadcom Corp. All Rights Reserved.

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
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/mii.h>
#include <linux/stddef.h>
#include <linux/ctype.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <board.h>
#include "boardparms.h"
#include <bcm_map_part.h>
#include "bcm_intr.h"
#include "bcmenet.h"
#include "bcmmii.h"
#include "ethswdefs.h"
#include "ethsw.h"
#include "ethsw_phy.h"
#include "eth_pwrmngt.h"
#include "bcmsw.h"
#include "bcmSpiRes.h"
#include "bcmswaccess.h"
#include "bcmswshared.h"
#if !defined(CONFIG_BCM963138) && !defined(CONFIG_BCM963148) && !defined(CONFIG_BCM94908)
#include "bcmPktDma.h"
#endif
#if defined(_CONFIG_BCM_FAP)
#include "fap_packet.h"
#endif
#if defined(CONFIG_BCM_GMAC)
#include "bcmgmac.h"
#endif
#if defined(CONFIG_BCM963381)
#include "pmc_switch.h"
#endif


extern struct semaphore bcm_ethlock_switch_config;
extern uint8_t port_in_loopback_mode[TOTAL_SWITCH_PORTS];
extern int vport_cnt;  /* number of vports: bitcount of Enetinfo.sw.port_map */

extern BcmEnet_devctrl *pVnetDev0_g;


#if defined(CONFIG_BCM947189)
int ethsw_reset_ports(struct net_device *dev) {return 0;};
void bcmeapi_ethsw_init_config(void){};
void ethsw_port_mirror_get(int *enable, int *mirror_port, unsigned int *ing_pmap,
                           unsigned int *eg_pmap, unsigned int *blk_no_mrr,
                           int *tx_port, int *rx_port){};
void ethsw_port_mirror_set(int enable, int mirror_port, unsigned int ing_pmap, 
                           unsigned int eg_pmap, unsigned int blk_no_mrr,
                           int tx_port, int rx_port){};
void ethsw_phy_apply_init_bp(void){};
void ethsw_phy_handle_exception_cases (void){};
int ethsw_setup_led(void){return 0;};
int ethsw_setup_phys(void){return 0;};
int ethsw_add_proc_files(struct net_device *dev){return 0;};
int ethsw_del_proc_files(void){return 0;};
int ethsw_phy_intr_ctrl(int port, int on){return 0;};
int ethsw_enable_hw_switching(void){return 0;};
int ethsw_disable_hw_switching(void){return 0;};
#else
static uint16_t dis_learning = 0x0180;
static uint8_t  port_fwd_ctrl = 0xC1;
static uint16_t pbvlan_map[TOTAL_SWITCH_PORTS];

#if defined(CONFIG_BCM963268)
void ethsw_phy_advertise_all(uint32 phy_id)
{
    uint16 v16;
    /* Advertise all speed & duplex combinations */
    /* Advertise 100BaseTX FD/HD and 10BaseT FD/HD */
    ethsw_phy_rreg(phy_id, MII_ADVERTISE, &v16);
    v16 |= AN_ADV_ALL;
    ethsw_phy_wreg(phy_id, MII_ADVERTISE, &v16);
    /* Advertise 1000BaseT FD/HD */
    ethsw_phy_rreg(phy_id, MII_CTRL1000, &v16);
    v16 |= AN_1000BASET_CTRL_ADV_ALL;
    ethsw_phy_wreg(phy_id, MII_CTRL1000, &v16);
}
#endif

/* apply phy init board parameters for internal switch*/
void ethsw_phy_apply_init_bp(void)
{
    BcmEnet_devctrl *pVnetDev0 = (BcmEnet_devctrl *) netdev_priv(vnet_dev[0]);
    unsigned int portmap, i, phy_id;
    bp_mdio_init_t* phyinit;
    uint16 data;

    portmap = pVnetDev0->EnetInfo[0].sw.port_map;
    for (i = 0; i < (TOTAL_SWITCH_PORTS - 1); i++) {
        if ((portmap & (1U<<i)) != 0) {
            phy_id = pVnetDev0->EnetInfo[0].sw.phy_id[i];
            phyinit = pVnetDev0->EnetInfo[0].sw.phyinit[i];
            if( phyinit == 0 )
                continue;

            while(phyinit->u.op.op != BP_MDIO_INIT_OP_NULL)
            {
                if(phyinit->u.op.op == BP_MDIO_INIT_OP_WRITE)
                    ethsw_phy_wreg(phy_id, phyinit->u.write.reg, (uint16*)(&phyinit->u.write.data));
                else if(phyinit->u.op.op == BP_MDIO_INIT_OP_UPDATE)
                {
                    ethsw_phy_rreg(phy_id, phyinit->u.update.reg, &data);
                    data &= ~phyinit->u.update.mask;
                    data |= phyinit->u.update.data;
                    ethsw_phy_wreg(phy_id, phyinit->u.update.reg, &data);
                }
                phyinit++;
            }
        }
    }

}
/* Code to handle exceptions and chip specific cases */
void ethsw_phy_handle_exception_cases (void)
{
    /* In some chips, the GPhys do not advertise all capabilities. So, fix it first */ 
#if defined(CONFIG_BCM963268)
    ethsw_phy_advertise_all(GPHY_PORT_PHY_ID);
#endif

}

int ethsw_setup_phys(void)
{
    ethsw_shutdown_unused_phys();
    return 0;
}

void bcmeapi_ethsw_init_config(void)
{
    int i;

    /* Save the state that is restored in enable_hw_switching */
    for(i = 0; i < TOTAL_SWITCH_PORTS; i++)  {
        ethsw_rreg(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (i * 2),
                (uint8 *)&pbvlan_map[i], 2);
    }
    ethsw_rreg(PAGE_CONTROL, REG_DISABLE_LEARNING, (uint8 *)&dis_learning, 2);
    ethsw_rreg(PAGE_CONTROL, REG_PORT_FORWARD, (uint8 *)&port_fwd_ctrl, 1);

#if defined(CONFIG_BCM963268) ||  defined(CONFIG_BCM963381)
    {
        /* Disable tags for internal switch ports */
        uint32 tmp;
        ethsw_rreg(PAGE_CONTROL, REG_IUDMA_CTRL, (uint8_t *)&tmp, 4);
        tmp |= REG_IUDMA_CTRL_TX_MII_TAG_DISABLE;
        ethsw_wreg(PAGE_CONTROL, REG_IUDMA_CTRL, (uint8_t *)&tmp, 4); 
    }
#endif

}

int ethsw_setup_led(void)
{
    BcmEnet_devctrl *pVnetDev0 = (BcmEnet_devctrl *) netdev_priv(vnet_dev[0]);
    unsigned int phy_id, i;
    uint16 v16;

    /* For each port that has an internal or external PHY, configure it
       as per the required initial configuration */
    for (i = 0; i < (TOTAL_SWITCH_PORTS - 1); i++) {
        /* Check if the port is in the portmap or not */
        if ((pVnetDev0->EnetInfo[0].sw.port_map & (1U<<i)) != 0) {
            /* Check if the port is connected to a PHY or not */
            phy_id = pVnetDev0->EnetInfo[0].sw.phy_id[i];
            /* If a Phy is connected, set it up with initial config */
            /* TBD: Maintain the config for each Phy */
            if(IsPhyConnected(phy_id) && !IsExtPhyId(phy_id)) {
#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381)
                v16 = 0xa410;
                // Enable Shadow register 2
                ethsw_phy_rreg(phy_id, MII_BRCM_TEST, &v16);
                v16 |= MII_BRCM_TEST_SHADOW2_ENABLE;
                ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &v16);

#if defined(CONFIG_BCM963268)
#if defined(CONFIG_BCM963268)
                if (i != GPHY_PORT_ID) 
#else
                    if ((i != GPHY1_PORT_ID) && (i != GPHY2_PORT_ID))
#endif
                    {
                        // Set LED1 to speed. Set LED0 to blinky link
                        v16 = 0x08;
                    }
#else
                // Set LED0 to speed. Set LED1 to blinky link
                v16 = 0x71;
#endif
                ethsw_phy_wreg(phy_id, 0x15, &v16);
                // Disable Shadow register 2
                ethsw_phy_rreg(phy_id, MII_BRCM_TEST, &v16);
                v16 &= ~MII_BRCM_TEST_SHADOW2_ENABLE;
                ethsw_phy_wreg(phy_id, MII_BRCM_TEST, &v16);
#endif
            }
            if (IsExtPhyId(phy_id)) {
                /* Configure LED for link/activity */
                v16 = MII_1C_SHADOW_LED_CONTROL << MII_1C_SHADOW_REG_SEL_S;
                ethsw_phy_wreg(phy_id, MII_REGISTER_1C, &v16);
                ethsw_phy_rreg(phy_id, MII_REGISTER_1C, &v16);
                v16 |= ACT_LINK_LED_ENABLE;
                v16 |= MII_1C_WRITE_ENABLE;
                v16 &= ~(MII_1C_SHADOW_REG_SEL_M << MII_1C_SHADOW_REG_SEL_S);
                v16 |= (MII_1C_SHADOW_LED_CONTROL << MII_1C_SHADOW_REG_SEL_S);
                ethsw_phy_wreg(phy_id, MII_REGISTER_1C, &v16);

                ethsw_phy_rreg(phy_id, MII_PHYSID2, &v16);
                if ((v16 & BCM_PHYID_M) == (BCM54610_PHYID2 & BCM_PHYID_M)) {
                    /* Configure LOM LED Mode */
                    v16 = MII_1C_EXTERNAL_CONTROL_1 << MII_1C_SHADOW_REG_SEL_S;
                    ethsw_phy_wreg(phy_id, MII_REGISTER_1C, &v16);
                    ethsw_phy_rreg(phy_id, MII_REGISTER_1C, &v16);
                    v16 |= LOM_LED_MODE;
                    v16 |= MII_1C_WRITE_ENABLE;
                    v16 &= ~(MII_1C_SHADOW_REG_SEL_M << MII_1C_SHADOW_REG_SEL_S);
                    v16 |= (MII_1C_EXTERNAL_CONTROL_1 << MII_1C_SHADOW_REG_SEL_S);
                    ethsw_phy_wreg(phy_id, MII_REGISTER_1C, &v16);
                }
            }
        }
    }
    return 0;
}

int ethsw_reset_ports(struct net_device *dev)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
    int map, cnt, i;
    uint16 v16, phy_identifier;
    int phyid;
    uint8 v8;
    unsigned long port_flags;

    map = pDevCtrl->EnetInfo[0].sw.port_map;
    bitcount(cnt, map);

    if (cnt <= 0)
        return 0;

#if defined(CONFIG_BCM963268)
    if (map & (1 << (RGMII_PORT_ID + 1))) {
        GPIO->RoboswSwitchCtrl |= (RSW_MII_2_IFC_EN | (RSW_MII_SEL_2P5V << RSW_MII_2_SEL_SHIFT));
    }
#endif

    for (i = 0; i < NUM_RGMII_PORTS; i++) {
        {
            phyid = pDevCtrl->EnetInfo[0].sw.phy_id[RGMII_PORT_ID + i];
            ethsw_phy_rreg(phyid, MII_PHYSID2, &phy_identifier);

            ethsw_rreg(PAGE_CONTROL, REG_RGMII_CTRL_P4 + i, &v8, 1);
#if defined(CONFIG_BCM963268)
            v8 |= REG_RGMII_CTRL_ENABLE_RGMII_OVERRIDE;
            v8 &= ~REG_RGMII_CTRL_MODE;
            if (IsRGMII(phyid)) {
                v8 |= REG_RGMII_CTRL_MODE_RGMII;
            } else if (IsRvMII(phyid)) {
                v8 |= REG_RGMII_CTRL_MODE_RvMII;
            } else if (IsGMII(phyid)) {
                v8 |= REG_RGMII_CTRL_MODE_GMII;
            } else {
                v8 |= REG_RGMII_CTRL_MODE_MII;
            }
#endif
            
#if defined(CONFIG_BCM963268)
            if ((pDevCtrl->chipRev == 0xA0) || (pDevCtrl->chipRev == 0xB0)) {
                /* RGMII timing workaround */
                v8 &= ~REG_RGMII_CTRL_TIMING_SEL;
            }
            else
#endif    
            {

                v8 |= REG_RGMII_CTRL_TIMING_SEL;
            }
            /* Enable Clock delay in RX */
            port_flags = enet_get_port_flags(0, RGMII_PORT_ID + i);
            if (IsPortRxInternalDelay(port_flags)) {
                v8 |= REG_RGMII_CTRL_DLL_RXC_BYPASS;
            }
            else if ((phy_identifier & BCM_PHYID_M) == (BCM54616_PHYID2 & BCM_PHYID_M)) {
                v8 |= REG_RGMII_CTRL_DLL_RXC_BYPASS;
            }

            ethsw_wreg(PAGE_CONTROL, REG_RGMII_CTRL_P4 + i, &v8, 1);

#if defined(CONFIG_BCM963268)
            if ((pDevCtrl->chipRev == 0xA0) || (pDevCtrl->chipRev == 0xB0)) {
                /* RGMII timing workaround */
                v8 = 0xAB;
                ethsw_wreg(PAGE_CONTROL, REG_RGMII_TIMING_P4 + i, &v8, 1);
            }
#endif

            /* No need to check the PhyID if the board params is set correctly for RGMII. However, keeping
             *   the phy id check to make it work even when customer does not set the RGMII flag in the phy_id
             *   in board params
             */
            if ((IsRGMII(phyid) && IsPhyConnected(phyid)) ||
                    ((phy_identifier & BCM_PHYID_M) == (BCM54610_PHYID2 & BCM_PHYID_M)) ||
                    ((phy_identifier & BCM_PHYID_M) == (BCM50612_PHYID2 & BCM_PHYID_M))) {

                v16 = MII_1C_SHADOW_CLK_ALIGN_CTRL << MII_1C_SHADOW_REG_SEL_S;
                ethsw_phy_wreg(phyid, MII_REGISTER_1C, &v16);
                ethsw_phy_rreg(phyid, MII_REGISTER_1C, &v16);
#if defined(CONFIG_BCM963268)
                /* Temporary work-around for MII2 port RGMII delay programming */
                if (i == 1 && ((pDevCtrl->chipRev == 0xA0) || (pDevCtrl->chipRev == 0xB0)) )
                    v16 |= GTXCLK_DELAY_BYPASS_DISABLE;
                else
#endif
                    v16 &= (~GTXCLK_DELAY_BYPASS_DISABLE);
                v16 |= MII_1C_WRITE_ENABLE;
                v16 &= ~(MII_1C_SHADOW_REG_SEL_M << MII_1C_SHADOW_REG_SEL_S);
                v16 |= (MII_1C_SHADOW_CLK_ALIGN_CTRL << MII_1C_SHADOW_REG_SEL_S);
                ethsw_phy_wreg(phyid, MII_REGISTER_1C, &v16);
                if ((phy_identifier & BCM_PHYID_M) == (BCM54616_PHYID2 & BCM_PHYID_M)) {
                    v16 = MII_REG_18_SEL(0x7);
                    ethsw_phy_wreg(phyid, MII_REGISTER_18, &v16);
                    ethsw_phy_rreg(phyid, MII_REGISTER_18, &v16);
                    /* Disable Skew */
                    v16 &= (~RGMII_RXD_TO_RXC_SKEW);
                    v16 = MII_REG_18_WR(0x7,v16);
                    ethsw_phy_wreg(phyid, MII_REGISTER_18, &v16);
                }
            }
        }
    }

    /*Remaining port reset functionality is moved into ethsw_init_hw*/

    return 0;
}

int bcmeapi_ethsw_init(void)
{
    //bcm_ethsw_init();

    return 0;
}

void bcmeapi_ethsw_init_ports()
{
    //robosw_configure_ports();
}

static uint8 swdata[16];
static uint8 miidata[16];

int ethsw_del_proc_files(void)
{
    remove_proc_entry("switch", NULL);

    remove_proc_entry("mii", NULL);
    return 0;
}

static void str_to_num(char* in, char* out, int len)
{
    int i;
    memset(out, 0, len);

    for (i = 0; i < len * 2; i ++)
    {
        if ((*in >= '0') && (*in <= '9'))
            *out += (*in - '0');
        else if ((*in >= 'a') && (*in <= 'f'))
            *out += (*in - 'a') + 10;
        else if ((*in >= 'A') && (*in <= 'F'))
            *out += (*in - 'A') + 10;
        else
            *out += 0;

        if ((i % 2) == 0)
            *out *= 16;
        else
            out ++;

        in ++;
    }
    return;
}

static int proc_get_sw_param(struct seq_file *seq, void *offset) 
{
    int reg_page  = swdata[0];
    int reg_addr  = swdata[1];
    int reg_len   = swdata[2];
    int i = 0;

    if (reg_len == 0)
        return 0;

    down(&bcm_ethlock_switch_config);
    ethsw_rreg(reg_page, reg_addr, swdata + 3, reg_len);
    up(&bcm_ethlock_switch_config);

    seq_printf(seq, "[%02x:%02x] = ", swdata[0], swdata[1]);

    for (i = 0; i < reg_len; i ++)
        seq_printf(seq, "%02x ", swdata[3 + i]);

    seq_printf(seq, "\n");
    return 0;
}

static int proc_set_sw_param(struct file *file, const char __user *buf, size_t cnt, loff_t *ppos)
{
    char input[32];
    int i;
    int r;
    int num_of_octets;

    int reg_page;
    int reg_addr;
    int reg_len;

    if (cnt > 32)
        cnt = 32;

    if (copy_from_user(input, buf, cnt) != 0)
        return -EFAULT;

    r = cnt;

    for (i = 0; i < r; i ++)
    {
        if (!isxdigit(input[i]))
        {
            memmove(&input[i], &input[i + 1], r - i - 1);
            r --;
            i --;
        }
    }

    num_of_octets = r / 2;

    if (num_of_octets < 3) // page, addr, len
        return -EFAULT;

    str_to_num(input, swdata, num_of_octets);

    reg_page  = swdata[0];
    reg_addr  = swdata[1];
    reg_len   = swdata[2];

    if (((reg_len != 1) && (reg_len % 2) != 0) || reg_len > 8)
    {
        memset(swdata, 0, sizeof(swdata));
        return -EFAULT;
    }

    if ((num_of_octets > 3) && (num_of_octets != reg_len + 3))
    {
        memset(swdata, 0, sizeof(swdata));
        return -EFAULT;
    }

    if (num_of_octets > 3) {
        down(&bcm_ethlock_switch_config);
        ethsw_wreg(reg_page, reg_addr, swdata + 3, reg_len);
        up(&bcm_ethlock_switch_config);
    }
    return cnt;
}

static int proc_get_mii_param(struct seq_file *seq, void *offset) 
{
    int mii_port  = miidata[0];
    int mii_addr  = miidata[1];

    down(&bcm_ethlock_switch_config);
    ethsw_phy_rreg(mii_port, mii_addr, (uint16 *)(miidata + 2));
    up(&bcm_ethlock_switch_config);

    seq_printf(
            seq,
            "[%02x:%02x] = %02x %02x\n",
            miidata[0], miidata[1], miidata[2], miidata[3]
            );

    return 0;
}

static int proc_set_mii_param(struct file *file, const char __user *buf, size_t cnt, loff_t *ppos)
{
    char input[32];
    int i;
    int r;
    int num_of_octets;

    int mii_port;
    int mii_addr;

    if (cnt > 32)
        cnt = 32;

    if (copy_from_user(input, buf, cnt) != 0)
        return -EFAULT;

    r = cnt;

    for (i = 0; i < r; i ++)
    {
        if (!isxdigit(input[i]))
        {
            memmove(&input[i], &input[i + 1], r - i - 1);
            r --;
            i --;
        }
    }

    num_of_octets = r / 2;

    if ((num_of_octets!= 2) && (num_of_octets != 4))
    {
        memset(miidata, 0, sizeof(miidata));
        return -EFAULT;
    }

    str_to_num(input, miidata, num_of_octets);
    mii_port  = miidata[0];
    mii_addr  = miidata[1];

    down(&bcm_ethlock_switch_config);

    if (num_of_octets > 2)
        ethsw_phy_wreg(mii_port, mii_addr, (uint16 *)(miidata + 2));

    up(&bcm_ethlock_switch_config);
    return cnt;
}

static int sw_param_open(struct inode *inode, struct file *file)
{
   return single_open(file, proc_get_sw_param, PDE_DATA(inode));
}

static const struct file_operations sw_param_fops = {
   .open    = sw_param_open,
   .read    = seq_read,
   .write   = proc_set_sw_param,
   .llseek  = seq_lseek,
   .release = seq_release,
};

static int mii_param_open(struct inode *inode, struct file *file)
{
   return single_open(file, proc_get_mii_param, PDE_DATA(inode));
}

static const struct file_operations mii_param_fops = {
   .open    = mii_param_open,
   .read    = seq_read,
   .write   = proc_set_mii_param,
   .llseek  = seq_lseek,
   .release = seq_release,
};

int ethsw_add_proc_files(struct net_device *dev)
{
   memset(swdata, 0, sizeof(swdata));
   memset(miidata, 0, sizeof(miidata));

   proc_create_data("switch", 0, NULL, &sw_param_fops, dev);
   proc_create_data("mii", 0, NULL, &mii_param_fops, dev);
   return 0;
}

int ethsw_enable_hw_switching(void)
{
    u8 i;

    /* restore pbvlan config */
    for(i = 0; i < TOTAL_SWITCH_PORTS; i++)
    {
        ethsw_wreg(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (i * 2),
                (uint8 *)&pbvlan_map[i], 2);
    }

    /* restore disable learning register */
    ethsw_wreg(PAGE_CONTROL, REG_DISABLE_LEARNING, (uint8 *)&dis_learning, 2);

    /* restore port forward control register */
    ethsw_wreg(PAGE_CONTROL, REG_PORT_FORWARD, (uint8 *)&port_fwd_ctrl, 1);

    i = 0;
    while (vnet_dev[i])
    {
        if (LOGICAL_PORT_TO_UNIT_NUMBER(VPORT_TO_LOGICAL_PORT(i)) != 0) /* Not Internal switch port */
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

int ethsw_disable_hw_switching(void)
{
    u8 i, byte_value;
    u16 reg_value;


    /* set the port-based vlan control reg of each port with fwding mask of
       only that port and MIPS. For MIPS port, set the forwarding mask of
       all the ports */
    for(i = 0; i < TOTAL_SWITCH_PORTS; i++)
    {
        ethsw_rreg(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (i * 2),
                (uint8 *)&pbvlan_map[i], 2);
        if (i == MIPS_PORT_ID)
        {
            reg_value = PBMAP_ALL;
        }
        else
        {
            reg_value = PBMAP_MIPS;
        }
        ethsw_wreg(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (i * 2),
                (uint8 *)&reg_value, 2);
    }

    /* Save disable_learning_reg setting */
    ethsw_rreg(PAGE_CONTROL, REG_DISABLE_LEARNING, (uint8 *)&dis_learning, 2);
    /* disable learning on all ports */
    reg_value = PBMAP_ALL;
    ethsw_wreg(PAGE_CONTROL, REG_DISABLE_LEARNING, (uint8 *)&reg_value, 2);

    /* Save port forward control setting */
    ethsw_rreg(PAGE_CONTROL, REG_PORT_FORWARD, (uint8 *)&port_fwd_ctrl, 1);
    /* flood unlearned packets */
    byte_value = 0x00;
    ethsw_wreg(PAGE_CONTROL, REG_PORT_FORWARD, (uint8 *)&byte_value, 1);

    i = 0;
    while (vnet_dev[i])
    {
        if (LOGICAL_PORT_TO_UNIT_NUMBER(VPORT_TO_LOGICAL_PORT(i)) != 0) /* Not Internal switch port */
        {
            i++;  /* Go to next port */
            continue;
        }
        /* When hardware switching is disabled, enable the Linux bridge to
           forward the bcast on hardware ports as well */
        vnet_dev[i++]->priv_flags &= ~IFF_HW_SWITCH;
    }


    /* Flush arl table dynamic entries */
    fast_age_all(0);
    return 0;
}


int ethsw_switch_manage_ports_leds(int led_mode)
{
#define AUX_MODE_REG 0x1d
#define LNK_LED_DIS  4 // Bit4

    uint16 v16, i;

    down(&bcm_ethlock_switch_config);

    for (i=0; i<4; i++) {
        ethsw_phy_rreg(enet_sw_port_to_phyid(0, i), AUX_MODE_REG, &v16);

        if(led_mode)
            v16 &= ~(1 << LNK_LED_DIS);
        else
            v16 |= (1 << LNK_LED_DIS);

        ethsw_phy_wreg(enet_sw_port_to_phyid(0, i), AUX_MODE_REG, &v16);
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}
EXPORT_SYMBOL(ethsw_switch_manage_ports_leds);


/* port = physical port */
int ethsw_phy_intr_ctrl(int port, int on)
{
    uint16 v16;
    int phyId = enet_sw_port_to_phyid(0, port);

    down(&bcm_ethlock_switch_config);

#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963138) | defined(CONFIG_BCM963381) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908)
    if (on != 0)
        v16 = MII_INTR_ENABLE | MII_INTR_FDX | MII_INTR_SPD | MII_INTR_LNK;
    else
        v16 = 0;

    ethsw_phy_wreg(phyId, MII_INTERRUPT, &v16);
#endif

#if defined(CONFIG_BCM963268)
#if defined(CONFIG_BCM963268)
    if (port == GPHY_PORT_ID)
#endif
        {
            if (on != 0)
                v16 = ~(MII_INTR_FDX | MII_INTR_SPD | MII_INTR_LNK);
            else
                v16 = 0xFFFF;

            ethsw_phy_wreg(phyId, MII_INTERRUPT_MASK, &v16);
        }
#endif

    up(&bcm_ethlock_switch_config);

    return 0;
}

void ethsw_port_mirror_get(int *enable, int *mirror_port, unsigned int *ing_pmap,
                           unsigned int *eg_pmap, unsigned int *blk_no_mrr,
                           int *tx_port, int *rx_port)
{
    uint16 v16;
    ethsw_rreg(PAGE_MANAGEMENT, REG_MIRROR_CAPTURE_CTRL,  (uint8*)&v16, sizeof(v16));
    if (v16 & REG_MIRROR_ENABLE)
    {
        *enable = 1;
        *mirror_port = v16 & REG_CAPTURE_PORT_M;
        *blk_no_mrr = v16 & REG_BLK_NOT_MIRROR;
        ethsw_rreg(PAGE_MANAGEMENT, REG_MIRROR_INGRESS_CTRL, (uint8*)&v16, sizeof(v16));
        *ing_pmap = v16 & REG_INGRESS_MIRROR_M;
        ethsw_rreg(PAGE_MANAGEMENT, REG_MIRROR_EGRESS_CTRL, (uint8*)&v16, sizeof(v16));
        *eg_pmap = v16 & REG_EGRESS_MIRROR_M;
    }
    else
    {
        *enable = 0;
    }
}
void ethsw_port_mirror_set(int enable, int mirror_port, unsigned int ing_pmap, 
                           unsigned int eg_pmap, unsigned int blk_no_mrr, 
                           int tx_port, int rx_port)
{
    uint16 v16;
    if (enable)
    {
        v16 = REG_MIRROR_ENABLE;
        v16 |= (mirror_port & REG_CAPTURE_PORT_M);
        v16 |= blk_no_mrr?REG_BLK_NOT_MIRROR:0;

        ethsw_wreg(PAGE_MANAGEMENT, REG_MIRROR_CAPTURE_CTRL, (uint8*)&v16, sizeof(v16));
        v16 = ing_pmap & REG_INGRESS_MIRROR_M;
        ethsw_wreg(PAGE_MANAGEMENT, REG_MIRROR_INGRESS_CTRL, (uint8*)&v16, sizeof(v16));
        v16 = eg_pmap & REG_INGRESS_MIRROR_M;
        ethsw_wreg(PAGE_MANAGEMENT, REG_MIRROR_EGRESS_CTRL, (uint8*)&v16, sizeof(v16));
    }
    else
    {
        v16  = 0;
        ethsw_wreg(PAGE_MANAGEMENT, REG_MIRROR_CAPTURE_CTRL, (uint8*)&v16, sizeof(v16));
    }
}
#endif
MODULE_LICENSE("GPL");

