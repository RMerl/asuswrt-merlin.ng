// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/*
 * 
 */

/* From boardparam.h */
#define PHY_ADV_CAP_CFG_M       0x3F
#define PHY_ADV_CAP_CFG_S       12
#define ADVERTISE_10HD          (1 << PHY_ADV_CAP_CFG_S)
#define ADVERTISE_10FD          (2 << PHY_ADV_CAP_CFG_S)
#define ADVERTISE_100HD         (4 << PHY_ADV_CAP_CFG_S)
#define ADVERTISE_100FD         (8 << PHY_ADV_CAP_CFG_S)
#define ADVERTISE_1000HD        (16 << PHY_ADV_CAP_CFG_S)
#define ADVERTISE_1000FD        (32 << PHY_ADV_CAP_CFG_S)
#define ADVERTISE_ALL_GMII      (ADVERTISE_10HD | ADVERTISE_10FD | ADVERTISE_100HD | ADVERTISE_100FD | ADVERTISE_1000HD | ADVERTISE_1000FD)
#define ADVERTISE_ALL_MII       (ADVERTISE_10HD | ADVERTISE_10FD | ADVERTISE_100HD | ADVERTISE_100FD)

#define PHY_ADV_CFG_VALID_M     1
#define PHY_ADV_CFG_VALID_S     18
#define PHY_ADV_CFG_VALID       (PHY_ADV_CFG_VALID_M << PHY_ADV_CFG_VALID_S)

#define MAC_CONN_M              0x1
#define MAC_CONN_S              21
#define MAC_CONNECTION          (MAC_CONN_M << MAC_CONN_S)
#define MAC_PHY_IF              (0 << MAC_CONN_S)
#define MAC_MAC_IF              (1 << MAC_CONN_S)

#define MAC_CONN_VALID_M        1
#define MAC_CONN_VALID_S        22
#define MAC_CONN_VALID          (MAC_CONN_VALID_M << MAC_CONN_VALID_S)

#define PHYID_LSBYTE_M           0xFF
#define BCM_PHY_ID_M             0x1F

#define IsPhyConnected(id)  (((id) & MAC_CONN_VALID)?(((id) & MAC_CONNECTION) != MAC_MAC_IF):(((id) & PHYID_LSBYTE_M) != 0xFF))
#define IsPhyAdvCapConfigValid(id) (((id) & PHY_ADV_CFG_VALID)?1:0)

#define K1CTL_REPEATER_DTE 0x400
#if !defined(K1CTL_1000BT_FDX)
#define K1CTL_1000BT_FDX 	0x200
#endif
#if !defined(K1CTL_1000BT_HDX)
#define K1CTL_1000BT_HDX 	0x100
#endif

#define ETHSW_MDIO_BUSY                       (1 << 29)
#define ETHSW_MDIO_FAIL                       (1 << 28)
#define ETHSW_MDIO_CMD_SHIFT                  26
#define ETHSW_MDIO_CMD_MASK                   (0x3<<ETHSW_MDIO_CMD_SHIFT) 
#define ETHSW_MDIO_CMD_C22_READ               2
#define ETHSW_MDIO_CMD_C22_WRITE              1
#define ETHSW_MDIO_C22_PHY_ADDR_SHIFT         21
#define ETHSW_MDIO_C22_PHY_ADDR_MASK          (0x1f<<ETHSW_MDIO_C22_PHY_ADDR_SHIFT)
#define ETHSW_MDIO_C22_PHY_REG_SHIFT          16
#define ETHSW_MDIO_C22_PHY_REG_MASK           (0x1f<<ETHSW_MDIO_C22_PHY_REG_SHIFT)
#define ETHSW_MDIO_PHY_DATA_SHIFT             0
#define ETHSW_MDIO_PHY_DATA_MASK              (0xffff<<ETHSW_MDIO_PHY_DATA_SHIFT)

typedef struct sw_mdio {
	uint32_t mdio_cmd;
	uint32_t mdio_cfg;

} sw_mdio;

typedef struct bcm_ethsw_ops_t
{
    void (*init)(struct udevice *dev);
    void (*open)(struct udevice *dev);
    void (*close)(struct udevice *dev);
} bcm_ethsw_ops_t;

uint16_t bcm_ethsw_phy_read_reg(int phy_id, int reg);
void bcm_ethsw_phy_write_reg(int phy_id, int reg, uint16_t data);



void phy_advertise_caps(unsigned int phy_id);
void gphy_powerup(int phy_base, u32 wkard_timeout, void *sphy_ctrl, void *qphy_ctrl, void *phy_test_ctrl);
void phy_set_mdio_base(volatile sw_mdio *switch_mdio);

#if defined(__UBOOT__)
#define mii_write bcm_ethsw_phy_write_reg
#define mii_read  bcm_ethsw_phy_read_reg
#endif
