/*
 * Copyright (c) 2013 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/bitops.h>
#include <linux/netdevice.h>
#include <linux/proc_fs.h>
#include <linux/ctype.h>
#include <linux/stddef.h>
#include <linux/phy.h>
#include <linux/brcmphy.h>
#include <linux/module.h>

#define BCM53125_PHY_ID		0x03625f20
#define BCM53101_PHY_ID		0x03625ed0

static unsigned char swdata[16];
static struct proc_dir_entry *p;

#define REG_PSEUDO_PHY_MII_REG16                          0x10
    #define REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT        8
    #define REG_PPM_REG16_MDIO_ENABLE                     0x01
#define REG_PSEUDO_PHY_MII_REG17                          0x11

    #define REG_PPM_REG17_REG_NUMBER_SHIFT                8
    #define REG_PPM_REG17_OP_DONE                         0x00
    #define REG_PPM_REG17_OP_WRITE                        0x01
    #define REG_PPM_REG17_OP_READ                         0x02

#define REG_PSEUDO_PHY_MII_REG24                          0x18
#define REG_PSEUDO_PHY_MII_REG25                          0x19
#define REG_PSEUDO_PHY_MII_REG26                          0x1a
#define REG_PSEUDO_PHY_MII_REG27                          0x1b

#define PAGE_CONTROL                                      0x00
    #define REG_PORT_CTRL                                 0x00
    #define REG_IMP_PORT_CONTROL                          0x08

        #define REG_IMP_PORT_CONTROL_RX_UCST_EN           0x10
        #define REG_IMP_PORT_CONTROL_RX_MCST_EN           0x08
        #define REG_IMP_PORT_CONTROL_RX_BCST_EN           0x04
    #define REG_SWITCH_MODE                               0x0b

        #define REG_SWITCH_MODE_FRAME_MANAGE_MODE         0x01
        #define REG_SWITCH_MODE_SW_FWDG_EN                0x02
    #define REG_CONTROL_IMP_PORT_STATE_OVERRIDE           0x0e

        #define REG_CONTROL_MPSO_MII_SW_OVERRIDE          0x80
        #define REG_CONTROL_MPSO_TX_FLOW_CONTROL          0x20
        #define REG_CONTROL_MPSO_RX_FLOW_CONTROL          0x10
        #define REG_CONTROL_MPSO_SPEED1000                0x08
        #define REG_CONTROL_MPSO_SPEED100                 0x04
        #define REG_CONTROL_MPSO_FDX                      0x02
        #define REG_CONTROL_MPSO_LINKPASS                 0x01

    #define REG_SWITCH_CTRL				  0x20
	#define REG_MII_DUMB_FWD_EN			  0x01 /* 53101 only */

    #define REG_PORT_FORWARD                              0x21 /* 5397 only */

        #define REG_PORT_FORWARD_MCST                     0x80
        #define REG_PORT_FORWARD_UCST                     0x40
        #define REG_PORT_FORWARD_IPMC                     0x01

    #define REG_UCST_LOOKUP_FAIL                          0x32

    #define REG_RGMII_CTRL_IMP                            0x60

    #define REG_RGMII_CTRL_P5                             0x65

        #define REG_RGMII_CTRL_ENABLE_GMII                0x80
        #define REG_RGMII_CTRL_TIMING_SEL                 0x04
        #define REG_RGMII_CTRL_DLL_RXC                    0x02
        #define REG_RGMII_CTRL_DLL_TXC                    0x01

#define SWITCH_PORT_MAP_IMP                               0x0100

#define PAGE_MANAGEMENT                                   0x02

    #define REG_GLOBAL_CONFIG                             0x00
#define PAGE_PORT_BASED_VLAN                              0x31

    #define REG_VLAN_CTRL_P0                              0x00
    #define REG_VLAN_CTRL_P1                              0x02
    #define REG_VLAN_CTRL_P2                              0x04
    #define REG_VLAN_CTRL_P3                              0x06
    #define REG_VLAN_CTRL_P4                              0x08
    #define REG_VLAN_CTRL_P5                              0x0a
    #define REG_VLAN_CTRL_P6                              0x0c
    #define REG_VLAN_CTRL_P7                              0x0e
    #define REG_VLAN_CTRL_P8                              0x10

#define BE64(x) \
({ \
	uint64_t __x = (x); \
	((uint64_t)( \
		(((uint64_t)(__x) & (uint64_t)0x00000000000000ffUL) << 56) | \
		(((uint64_t)(__x) & (uint64_t)0x000000000000ff00UL) << 40) | \
		(((uint64_t)(__x) & (uint64_t)0x0000000000ff0000UL) << 24) | \
		(((uint64_t)(__x) & (uint64_t)0x00000000ff000000UL) << 8) | \
		(((uint64_t)(__x) & (uint64_t)0x000000ff00000000UL) >> 8) | \
		(((uint64_t)(__x) & (uint64_t)0x0000ff0000000000UL) >> 24) | \
		(((uint64_t)(__x) & (uint64_t)0x00ff000000000000UL) >> 40) | \
		(((uint64_t)(__x) & (uint64_t)0xff00000000000000UL) >> 56) )); \
})
#define BE32(x) \
({ \
	uint32_t __x = (x); \
	((uint32_t)( \
		(((uint32_t)(__x) & (uint32_t)0x000000ffUL) << 24) | \
		(((uint32_t)(__x) & (uint32_t)0x0000ff00UL) <<  8) | \
		(((uint32_t)(__x) & (uint32_t)0x00ff0000UL) >>  8) | \
		(((uint32_t)(__x) & (uint32_t)0xff000000UL) >> 24) )); \
})
#define BE16(x) \
({ \
	uint16_t __x = (x); \
	((uint16_t)( \
		(((uint16_t)(__x) & (uint16_t)0x00ffUL) << 8) | \
		(((uint16_t)(__x) & (uint16_t)0xff00UL) >>  8) )); \
})

static void ethsw_mdio_rreg(struct phy_device *phydev, int page, int reg,
				unsigned char *data, int len)
{
	int phy_id;
	int v, vm[4];
	int i;

	phy_id = BRCM_PSEUDO_PHY_ADDR;
	v = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT) | REG_PPM_REG16_MDIO_ENABLE;
	mdiobus_write(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG16, v);

	v = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_READ;
	mdiobus_write(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG17, v);

	for (i = 0; i < 5; i++)
	{
		v = mdiobus_read(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG17);

		if ((v & (REG_PPM_REG17_OP_WRITE | REG_PPM_REG17_OP_READ)) == REG_PPM_REG17_OP_DONE)
			break;

		udelay(10);
	}

	if (i == 5)
	{
		printk("ethsw_mdio_rreg: timeout!\n");
		return;
	}

	switch (len)
	{
		case 1:
			v = mdiobus_read(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG24);
			data[0] = (unsigned char)v;
			break;
		case 2:
			v = mdiobus_read(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG24);
			((unsigned short *)data)[0] = (unsigned short)v;
			break;
		case 4:
			vm[0] = mdiobus_read(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG24);
			vm[1] = mdiobus_read(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG25);
			((unsigned short *)data)[0] = (unsigned short)vm[0];
			((unsigned short *)data)[1] = (unsigned short)vm[1];
			break;
		case 6:
			vm[0] = mdiobus_read(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG26);
			vm[1] = mdiobus_read(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG25);
			vm[2] = mdiobus_read(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG24);
			((unsigned short *)data)[0] = (unsigned short)vm[2];
			((unsigned short *)data)[1] = (unsigned short)vm[1];
			((unsigned short *)data)[2] = (unsigned short)vm[0];
			break;
		case 8:
			vm[0] = mdiobus_read(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG27);
			vm[1] = mdiobus_read(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG26);
			vm[2] = mdiobus_read(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG25);
			vm[3] = mdiobus_read(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG24);
			((unsigned short *)data)[0] = (unsigned short)vm[3];
			((unsigned short *)data)[1] = (unsigned short)vm[2];
			((unsigned short *)data)[2] = (unsigned short)vm[1];
			((unsigned short *)data)[3] = (unsigned short)vm[0];
			break;
	}
	v = page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT;
	mdiobus_write(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG16, v);
}

static void ethsw_mdio_wreg(struct phy_device *phydev, int page, int reg,
				unsigned char *data, int len)
{
	int phy_id;
	int v;
	int i;

	phy_id = BRCM_PSEUDO_PHY_ADDR;
	v = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT) | REG_PPM_REG16_MDIO_ENABLE;
	mdiobus_write(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG16, v);

	switch (len)
	{
		case 1:
			v = data[0];
			mdiobus_write(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG24, v);
			break;
		case 2:
			v = ((unsigned short *)data)[0];
			mdiobus_write(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG24, v);
			break;
		case 4:
			v = ((unsigned short *)data)[0];
			mdiobus_write(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG24, v);
			v = ((unsigned short *)data)[1];
			mdiobus_write(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG25, v);
			break;
		case 6:
			v = ((unsigned short *)data)[0];
			mdiobus_write(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG24, v);
			v = ((unsigned short *)data)[1];
			mdiobus_write(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG25, v);
			v = ((unsigned short *)data)[2];
			mdiobus_write(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG26, v);
			break;
		case 8:
			v = ((unsigned short *)data)[0];
			mdiobus_write(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG24, v);
			v = ((unsigned short *)data)[1];
			mdiobus_write(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG25, v);
			v = ((unsigned short *)data)[2];
			mdiobus_write(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG26, v);
			v = ((unsigned short *)data)[3];
			mdiobus_write(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG27, v);
			break;
	}

	v = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_WRITE;
	mdiobus_write(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG17, v);

	for (i = 0; i < 5; i++)
	{
		v = mdiobus_read(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG17);

		if ((v & (REG_PPM_REG17_OP_WRITE | REG_PPM_REG17_OP_READ)) == REG_PPM_REG17_OP_DONE)
			break;

		udelay(10);
	}

	if (i == 5)
		printk("ethsw_mdio_wreg: timeout!\n");

	v = page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT;
	mdiobus_write(phydev->mdio.bus, phy_id, REG_PSEUDO_PHY_MII_REG16, v);
}

static void ethsw_rreg(struct phy_device *phydev, int page, int reg,
			unsigned char *data, int len)
{
	if (((len != 1) && (len % 2) != 0) || len > 8)
		panic("ethsw_rreg: wrong length!\n");

	ethsw_mdio_rreg(phydev, page, reg, data, len);
}

static void ethsw_wreg(struct phy_device *phydev, int page, int reg,
			unsigned char *data, int len)
{
	if (((len != 1) && (len % 2) != 0) || len > 8)
		panic("ethsw_wreg: wrong length!\n");

	ethsw_mdio_wreg(phydev, page, reg, data, len);
}

static int ethsw_config_learning(struct phy_device *phydev)
{
	unsigned char v8;
	unsigned short v16;

	// Forward lookup failure to CPU
	v8 = (REG_PORT_FORWARD_MCST | REG_PORT_FORWARD_UCST | REG_PORT_FORWARD_IPMC);
	ethsw_wreg(phydev, PAGE_CONTROL, REG_PORT_FORWARD, (unsigned char *)&v8, sizeof(v8));
	// Forward unlearned unicast frames to the MIPS
	v16 = SWITCH_PORT_MAP_IMP;
	ethsw_wreg(phydev, PAGE_CONTROL, REG_UCST_LOOKUP_FAIL, (unsigned char *)&v16, sizeof(v16));

	return 0;
}

static void ethsw_switch_unmanage_mode(struct phy_device *phydev)
{
	unsigned char v8;
	unsigned short v16;

	ethsw_rreg(phydev, PAGE_CONTROL, REG_SWITCH_MODE, &v8, sizeof(v8));
	v8 &= ~REG_SWITCH_MODE_FRAME_MANAGE_MODE;
	v8 |= REG_SWITCH_MODE_SW_FWDG_EN;
	ethsw_wreg(phydev, PAGE_CONTROL, REG_SWITCH_MODE, &v8, sizeof(v8));

	/* BCM53101 requires this MII_DUMB_FWD_EN to be set to forward frames
	 * towards the host CPU
	 */
	if ((phydev->drv->phy_id & phydev->drv->phy_id_mask) == BCM53101_PHY_ID) {
		v8 = REG_MII_DUMB_FWD_EN;
		ethsw_wreg(phydev, PAGE_CONTROL, REG_SWITCH_CTRL, &v8, sizeof(v8));
	}

	v8 = 0;
	ethsw_wreg(phydev, PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &v8, sizeof(v8));

	/* Delete port-based VLAN */
	v16 = 0x01ff;
	ethsw_wreg(phydev, PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0, (unsigned char *)&v16, sizeof(v16));
	ethsw_wreg(phydev, PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P1, (unsigned char *)&v16, sizeof(v16));
	ethsw_wreg(phydev, PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P2, (unsigned char *)&v16, sizeof(v16));
	ethsw_wreg(phydev, PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P3, (unsigned char *)&v16, sizeof(v16));
	ethsw_wreg(phydev, PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P4, (unsigned char *)&v16, sizeof(v16));
	ethsw_wreg(phydev, PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P8, (unsigned char *)&v16, sizeof(v16));
}

static inline bool ethsw_gigabit(struct phy_device *phydev)
{
	int ret = 0;

	ret = linkmode_test_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
				phydev->advertising);
	ret |= linkmode_test_bit(ETHTOOL_LINK_MODE_1000baseT_Half_BIT,
				 phydev->advertising);

	return !!ret;
}

static inline bool ethsw_fast_ethernet(struct phy_device *phydev)
{
	int ret = 0;

	ret = linkmode_test_bit(ETHTOOL_LINK_MODE_100baseT_Full_BIT,
				phydev->advertising);
	ret |= linkmode_test_bit(ETHTOOL_LINK_MODE_100baseT_Half_BIT,
				 phydev->advertising);

	return !!ret;
}

static int ethsw_force_speed(struct phy_device *phydev)
{
	unsigned char v8;

	v8 = 0;
	v8 |= (REG_CONTROL_MPSO_MII_SW_OVERRIDE|REG_CONTROL_MPSO_LINKPASS);
	if (ethsw_gigabit(phydev))
		v8 |= REG_CONTROL_MPSO_SPEED1000;
	else if (ethsw_fast_ethernet(phydev))
		v8 |= REG_CONTROL_MPSO_SPEED100;
	else
		v8 &= ~(REG_CONTROL_MPSO_SPEED1000 | REG_CONTROL_MPSO_SPEED100);
	v8 |= (REG_CONTROL_MPSO_RX_FLOW_CONTROL|REG_CONTROL_MPSO_FDX);
	ethsw_wreg(phydev, PAGE_CONTROL, REG_CONTROL_IMP_PORT_STATE_OVERRIDE, &v8, sizeof(v8));

	/* checking Switch functional */
	v8 = 0;
	ethsw_rreg(phydev, PAGE_CONTROL, REG_CONTROL_IMP_PORT_STATE_OVERRIDE, &v8, sizeof(v8));
	if ((v8 & (REG_CONTROL_MPSO_MII_SW_OVERRIDE|REG_CONTROL_MPSO_LINKPASS)) !=
		(REG_CONTROL_MPSO_MII_SW_OVERRIDE|REG_CONTROL_MPSO_LINKPASS) ||
		(v8 == 0xff)) {
		phydev_err(phydev, "error on Ethernet Switch setup 0x%x\n", v8);
		return -EIO;
	}

	return 0;
}

static int ethsw_reset_ports(struct phy_device *phydev)
{
	unsigned long flags;
	int i;
	unsigned char v8;

	local_irq_save(flags);

	/* MAC level reset */
	v8 = 0; /* No spanning tree and tx/rx enable */
	for (i = 0; i < 5; i++)
	{
		ethsw_wreg(phydev, PAGE_CONTROL, REG_PORT_CTRL + i, &v8, 1);
	}

	/* Config IMP port RGMII clock delay by DLL disabled and tx_clk aligned
	 * timing (restoring to reset defaults)
	 */
	ethsw_rreg(phydev, PAGE_CONTROL, REG_RGMII_CTRL_IMP, &v8, sizeof(v8));
	v8 &= ~(REG_RGMII_CTRL_DLL_RXC | REG_RGMII_CTRL_DLL_TXC);
	v8 &= ~REG_RGMII_CTRL_TIMING_SEL;

	/* PHY_INTERFACE_MODE_RGMII_TXID means TX internal delay, make sure
	 * that we enable the IMP Port TX clock internal delay to account for
	 * this internal delay that is inserted, otherwise the switch won't be
	 * able to receive correctly.
	 *
	 * PHY_INTERFACE_MODE_RGMII means that we are not introducing any delay
	 * neither on transmission nor reception, so the BCM53125 must also be
	 * configured accordingly to account for the lack of delay and
	 * introduce
	 *
	 * The BCM53125 switch has its RX clock and TX clock control swapped,
	 * hence the reason why we modify the TX clock path in the "RGMII" case
	 */
	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID)
		v8 |= REG_RGMII_CTRL_DLL_TXC;
	if (phydev->interface == PHY_INTERFACE_MODE_RGMII)
		v8 |= REG_RGMII_CTRL_DLL_TXC | REG_RGMII_CTRL_DLL_RXC;
	v8 |= REG_RGMII_CTRL_TIMING_SEL;
	ethsw_wreg(phydev, PAGE_CONTROL, REG_RGMII_CTRL_IMP, &v8, sizeof(v8));

	/* Set IMP port RMII mode */
	v8 = 0;
	v8 |= REG_IMP_PORT_CONTROL_RX_UCST_EN;
	v8 |= REG_IMP_PORT_CONTROL_RX_MCST_EN;
	v8 |= REG_IMP_PORT_CONTROL_RX_BCST_EN;
	ethsw_wreg(phydev, PAGE_CONTROL, REG_IMP_PORT_CONTROL, &v8, 1);

	local_irq_restore(flags);

	return 0;
}


static void ethsw_print_reg(struct phy_device *phydev, int reg_page, int reg_addr, int reg_len, char *reg_name)
{
	unsigned char swdata[8];
	unsigned int val;
	int i;

	ethsw_rreg(phydev, reg_page, reg_addr, swdata, reg_len);
	printk("%s [%02x:%02x] = ", reg_name, reg_page, reg_addr);
	switch(reg_len) {
		case 2:
			*(uint16_t *)&swdata[0] = BE16(*(uint16_t *)&swdata[0]);
			break;
		case 4:
			*(uint32_t *)&swdata[0] = BE32(*(uint32_t *)&swdata[0]);
			break;
		case 8:
			val = BE32(*(uint32_t *)&swdata[0]);
			*(uint32_t *)&swdata[0] = BE32(*(uint32_t *)&swdata[4]);
			*(uint32_t *)&swdata[4] = val;
			break;
	}
	for (i = 0; i < reg_len; i ++)
		printk("%02x ", swdata[i]);
	printk("\n");
}

static int ethsw_dump_mib(struct phy_device *phydev, int port)
{
	int reg_page;

	if (port > 8) {
		printk("Invalid port number \n");
		return -1;
	}

	printk("\n");
	printk("dump port %d MIB counter\n", port);
	reg_page = 0x20 + port;
	/* Display Tx statistics */
	ethsw_print_reg(phydev, reg_page, 0, 8, "TxOctets");
	ethsw_print_reg(phydev, reg_page, 8, 4, "TxDropPkts");
	ethsw_print_reg(phydev, reg_page, 0x10, 4, "TxBroadcastPkts");
	ethsw_print_reg(phydev, reg_page, 0x14, 4, "TxMulticastPkts");
	ethsw_print_reg(phydev, reg_page, 0x18, 4, "TxUnicastPkts");
	ethsw_print_reg(phydev, reg_page, 0x1c, 4, "TxCollisions");
	ethsw_print_reg(phydev, reg_page, 0x58, 4, "RxUndersizePkts");
	ethsw_print_reg(phydev, reg_page, 0x60, 4, "Pkt64Octets");
	ethsw_print_reg(phydev, reg_page, 0x64, 4, "Pkt65to127Octets");
	ethsw_print_reg(phydev, reg_page, 0x68, 4, "Pkt128to255Octets");
	ethsw_print_reg(phydev, reg_page, 0x6C, 4, "Pkt256to511Octets");
	ethsw_print_reg(phydev, reg_page, 0x70, 4, "Pkt512to1023Octets");
	ethsw_print_reg(phydev, reg_page, 0x74, 4, "Pkts1024toMaxPktOctets");
	ethsw_print_reg(phydev, reg_page, 0x78, 4, "RxOversizePkts");
	ethsw_print_reg(phydev, reg_page, 0x80, 4, "RxAlignmentErros");
	ethsw_print_reg(phydev, reg_page, 0x84, 4, "RXFCSErrors");
	ethsw_print_reg(phydev, reg_page, 0x88, 4, "RxGoodOctets");
	ethsw_print_reg(phydev, reg_page, 0x90, 4, "RxDropPkts");
	ethsw_print_reg(phydev, reg_page, 0x94, 4, "RxUnicastPkts");
	ethsw_print_reg(phydev, reg_page, 0x98, 4, "RxMulticastPkts");
	ethsw_print_reg(phydev, reg_page, 0x9C, 4, "RxBroadcastPkts");
	ethsw_print_reg(phydev, reg_page, 0xA8, 4, "JumboPkt");
	ethsw_print_reg(phydev, reg_page, 0xAC, 4, "RXSymbolError");
	ethsw_print_reg(phydev, reg_page, 0xB4, 4, "OutofRangeError");

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

static int proc_get_sw_param(struct seq_file *m, void *v)
{
	struct phy_device *phydev = m->private;
	int reg_page  = swdata[0];
	int reg_addr  = swdata[1];
	int reg_len   = swdata[2];
	int i = 0;
	unsigned int val;

	if (reg_len == 0)
		return 0;

	if (!v)
		return 0;

#if 0
	if (reg_page == 0xfe) {
		if (reg_len == 6)
			show_arl((struct phy_device *)data, swdata[1], swdata + 3);
		else
			show_arl((struct phy_device *)data, swdata[1], NULL);
		return 0;
	}
#endif
	if (reg_page == 0xff) {
		for (i = 0; i < 8; i++) {
			if ((0x01 << i) & reg_addr)
				ethsw_dump_mib(phydev, i);
		}
		ethsw_dump_mib(phydev, 8);
		return 0;
	}
	ethsw_rreg(phydev, reg_page, reg_addr, swdata + 3, reg_len);

	seq_printf(m, "[%02x:%02x] = ", swdata[0], swdata[1]);
	switch(reg_len) {
		case 2:
			*(uint16_t *)&swdata[3] = BE16(*(uint16_t *)&swdata[3]);
			break;
		case 4:
			*(uint32_t *)&swdata[3] = BE32(*(uint32_t *)&swdata[3]);
			break;
		case 6:
			val = BE32(*(uint32_t *)&swdata[5]);
			*(uint16_t *)&swdata[7] = BE16(*(uint32_t *)&swdata[3]);
			*(uint32_t *)&swdata[3] = val;
			break;
		case 8:
			val = BE32(*(uint32_t *)&swdata[3]);
			*(uint32_t *)&swdata[3] = BE32(*(uint32_t *)&swdata[7]);
			*(uint32_t *)&swdata[7] = val;
			break;
	}

	for (i = 0; i < reg_len; i ++)
		seq_printf(m, "%02x ", swdata[3 + i]);

	seq_printf(m, "\n");

	return 0;
}

static ssize_t proc_set_sw_param(struct file *f, const char *buf, size_t cnt, loff_t *data)
{
	struct phy_device *phydev = PDE_DATA(file_inode(f));
	char input[32];
	int i;
	int r;
	int num_of_octets;

	int reg_page;
	int reg_addr;
	int reg_len;
	unsigned int val;

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

	if ((num_of_octets > 3) && (reg_page < 0xfe)) {
		switch(reg_len) {
			case 2:
				*(uint16_t *)&swdata[3] = BE16(*(uint16_t *)&swdata[3]);
				break;
			case 4:
				*(uint32_t *)&swdata[3] = BE32(*(uint32_t *)&swdata[3]);
				break;
			case 6:
				val = BE32(*(uint32_t *)&swdata[5]);
				*(uint16_t *)&swdata[7] = BE16(*(uint32_t *)&swdata[3]);
				*(uint32_t *)&swdata[3] = val;
				break;
			case 8:
				val = BE32(*(uint32_t *)&swdata[3]);
				*(uint32_t *)&swdata[3] = BE32(*(uint32_t *)&swdata[7]);
				*(uint32_t *)&swdata[7] = val;
				break;
		}
		ethsw_wreg(phydev, reg_page, reg_addr, swdata + 3, reg_len);
	}
	return cnt;
}

static int ethsw_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_get_sw_param, PDE_DATA(inode));
}

static struct proc_ops ethsw_fops = {
	.proc_open	= ethsw_open,
	.proc_read	= seq_read,
	.proc_write	= proc_set_sw_param,
	.proc_lseek	= seq_lseek,
	.proc_release	= seq_release,
};

static int ethsw_add_proc_files(struct phy_device *phydev)
{
	p = proc_create_data("switch", 0644, NULL, &ethsw_fops, phydev);
	if (p == NULL) {
		pr_err("%s: unable to create proc entry\n", __func__);
		return -1;
	}

	memset(swdata, 0, sizeof(swdata));

	return 0;
}

static int bcm53x25_ethsw_init(struct phy_device *phydev)
{
	ethsw_reset_ports(phydev);
	ethsw_switch_unmanage_mode(phydev);
	ethsw_config_learning(phydev);

	return 0;
}

static int bcm53x25_ethsw_resume(struct phy_device *phydev)
{
	bcm53x25_ethsw_init(phydev);
	return ethsw_force_speed(phydev);
}

static int bcm53x25_config_aneg(struct phy_device *phydev)
{
	return ethsw_force_speed(phydev);
}

static int bcm53x25_read_status(struct phy_device *phydev)
{
	phydev->duplex = DUPLEX_FULL;

	if (ethsw_gigabit(phydev))
		phydev->speed = SPEED_1000;
	else if (ethsw_fast_ethernet(phydev))
		phydev->speed = SPEED_100;
	else
		phydev->speed = SPEED_10;

	phydev->link = 1;

	return 0;
}

static int bcm53x25_probe(struct phy_device *phydev)
{
	/* Make sure that we match either the pseudo-PHY addreess or
	 * address 0 as this is the default PHY addressing supported
	 * for BCM53xxx switches
	 */
	if (phydev->mdio.addr != BRCM_PSEUDO_PHY_ADDR &&
	    phydev->mdio.addr != 0) {
		phydev_err(phydev, "invalid PHY address %d\n", phydev->mdio.addr);
		return -ENODEV;
	}

	if (p) {
		remove_proc_entry("switch", NULL);
		p = NULL;
	}
	ethsw_add_proc_files(phydev);

	/* Tell the MDIO bus controller that this switch's pseudo-PHY has a
	 * broken turn-around and it should ignore read failures. See ING JIRA
	 * SF-357.
	 */
	phydev->mdio.bus->phy_ignore_ta_mask |= 1 << BRCM_PSEUDO_PHY_ADDR;

	return 0;
}

static void bcm53x25_remove(struct phy_device *phydev)
{
	if (p) {
		remove_proc_entry("switch", NULL);
		p = NULL;
	}
}

static struct phy_driver bcm53x25_driver[] = {
	{
		/* Match all BCM53125 revisions */
		.phy_id		= BCM53125_PHY_ID,
		.phy_id_mask 	= 0x1ffffff0,
		.name		= "Broadcom BCM53125",
		.features	= PHY_GBIT_FEATURES,
		.probe		= bcm53x25_probe,
		.remove		= bcm53x25_remove,
		.config_init	= bcm53x25_ethsw_init,
		.config_aneg	= bcm53x25_config_aneg,
		.read_status	= bcm53x25_read_status,
		.resume		= bcm53x25_ethsw_resume,
	},
	{
		.phy_id		= BCM53101_PHY_ID,
		.phy_id_mask	= 0x1ffffff0,
		.name		= "Broadcom BCM53101",
		.features	= PHY_BASIC_FEATURES,
		.probe		= bcm53x25_probe,
		.remove		= bcm53x25_remove,
		.config_init	= bcm53x25_ethsw_init,
		.config_aneg	= bcm53x25_config_aneg,
		.read_status	= bcm53x25_read_status,
		.resume		= bcm53x25_ethsw_resume,
	},
};

static struct mdio_device_id __maybe_unused bcm53x25_tbl[] = {
	{ BCM53125_PHY_ID, 0x1ffffff0 },
	{ BCM53101_PHY_ID, 0x1ffffff0 },
	{ }
};

module_phy_driver(bcm53x25_driver);

MODULE_DEVICE_TABLE(mdio, bcm53x25_tbl);

MODULE_DESCRIPTION("Broadcom BCM53x25 switch driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom Corporation");
