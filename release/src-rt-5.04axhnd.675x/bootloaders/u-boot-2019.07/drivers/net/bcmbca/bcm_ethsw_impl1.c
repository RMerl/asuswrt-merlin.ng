/* SPDX-License-Identifier: GPL-2.0+
*  *
*   *  Copyright 2019 Broadcom Ltd.
*    */


#include <config.h>
#include <common.h>
#include <stdlib.h>
#include <dm.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/ioport.h>

#include "asm/arch/ethsw.h"
#include "mii_shared.h"
#include "pmc_drv.h"
#include "pmc_switch.h"
#include "bcm_ethsw.h"

#define BP_MAX_SWITCH_PORTS                     8
#define BP_MAX_ENET_MACS                        2

struct bcmbca_sf2_priv {
    bcm_ethsw_ops_t ops;
	volatile sw_mdio *switch_mdio;
	volatile EthernetSwitchCore *switch_core;

	void *sphy_ctrl;
	void *qphy_ctrl;
	void *phy_test_ctrl;
	u32 phy_wkard_timeout;
	int phy_base;
	int num_phys;
	uint32_t phy_ids[BP_MAX_SWITCH_PORTS];
};

static void bcm_ethsw_init(struct udevice *dev)
{
	unsigned int phy_ctrl;
	int i = 0;
	struct bcmbca_sf2_priv *priv = dev_get_priv(dev);

	volatile EthernetSwitchCore *ETHSW_CORE=priv->switch_core;

	printf("Initalizing switch low level hardware\n");


	/* Reset switch */
	if (ETHSW_CORE)
	{
		/* hard code to enable both zones clk temporarily. 
		   need to parse the dts and enable accordinlgy.
		*/
		pmc_switch_enable_rgmii_zone_clk(1, 1);
		/* power up the switch block */
		pmc_switch_power_up();

		printk("Software Resetting Switch ... ");
		ETHSW_CORE->software_reset |= SOFTWARE_RESET | EN_SW_RST;
		for (; ETHSW_CORE->software_reset & SOFTWARE_RESET;)
			udelay(100);
		printk("Done.\n");
		udelay(1000);
	}

	gphy_powerup(priv->phy_base, priv->phy_wkard_timeout, priv->sphy_ctrl, priv->qphy_ctrl, priv->phy_test_ctrl);


#if !defined(CONFIG_BCM6756)
	// BCM6756 switch has reverse logic port will be reset disabled
	if (ETHSW_CORE)
	{
		printk("Waiting MAC port Rx/Tx to be enabled by hardware ...");
		for (i = 0; i < BP_MAX_SWITCH_PORTS; i++) {
			/* Wait until hardware enable the ports, or we will kill the hardware */
			for (; ETHSW_CORE->port_traffic_ctrl[i] & PORT_CTRL_RX_DISABLE;
		    	 udelay(100)) ;
		}
	}
#endif

	if (priv->num_phys) {
		for (i=0; i < priv->num_phys; i++) {
			phy_advertise_caps(priv->phy_ids[i]);
		}
	}

	printk("Done\n");

	if (ETHSW_CORE)
	{
		/* disabled all MAC TX/RX. */
		printk("Disable Switch All MAC port Rx/Tx\n");
		for (i = 0; i < BP_MAX_SWITCH_PORTS; i++) {
			ETHSW_CORE->port_traffic_ctrl[i] =
		    	(ETHSW_CORE->
		     	port_traffic_ctrl[i] & 0xff) | PORT_CTRL_RXTX_DISABLE;
		}

		/* Set switch to unmanaged mode and enable forwarding */
		ETHSW_CORE->switch_mode =
	    	((ETHSW_CORE->
	      	switch_mode & 0xff) | ETHSW_SM_FORWARDING_EN |
	     	ETHSW_SM_RETRY_LIMIT_DIS) & (~ETHSW_SM_MANAGED_MODE);
		ETHSW_CORE->brcm_hdr_ctrl = 0;
		ETHSW_CORE->switch_ctrl = (ETHSW_CORE->switch_ctrl & 0xffb0) |
	    	ETHSW_SC_MII_DUMP_FORWARDING_EN | ETHSW_SC_MII2_VOL_SEL;

		ETHSW_CORE->imp_port_state =
	    	ETHSW_IPS_USE_REG_CONTENTS | ETHSW_IPS_TXFLOW_PAUSE_CAPABLE |
		    ETHSW_IPS_RXFLOW_PAUSE_CAPABLE | ETHSW_IPS_SW_PORT_SPEED_1000M_2000M
		    | ETHSW_IPS_DUPLEX_MODE | ETHSW_IPS_LINK_PASS;
	}
	return;
}

static void extsw_register_save_restore(volatile EthernetSwitchCore *ETHSW_CORE, int save)
{
	static int saved = 0;
	static uint32_t portCtrl[BP_MAX_SWITCH_PORTS],
	    pbvlan[BP_MAX_SWITCH_PORTS], reg;
	int i;
	int offset_jump = ARRAY_SIZE(ETHSW_CORE->port_vlan_ctrl) / 9;

	if (save) {
		saved = 1;
		for (i = 0; i < BP_MAX_SWITCH_PORTS; i++) {
			portCtrl[i] = ETHSW_CORE->port_traffic_ctrl[i] & 0xff;
			pbvlan[i] =
			    ETHSW_CORE->port_vlan_ctrl[offset_jump *
						       i] & 0xffff;
		}
	} else {
		if (saved) {
			for (i = 0; i < BP_MAX_SWITCH_PORTS; i++) {
				reg =
				    ETHSW_CORE->
				    port_traffic_ctrl[i] &
				    PORT_CTRL_SWITCH_RESERVE;
				reg |= portCtrl[i] & ~PORT_CTRL_SWITCH_RESERVE;
				ETHSW_CORE->port_traffic_ctrl[i] = reg;
				ETHSW_CORE->port_vlan_ctrl[offset_jump * i] =
				    pbvlan[i];
			}
		}
	}
}

/* SF2 switch init for CFE networking */
/* Only Called by CFE Command Line */
static void bcm_ethsw_open(struct udevice *dev)
{
	struct bcmbca_sf2_priv *priv = dev_get_priv(dev);

	volatile EthernetSwitchCore *ETHSW_CORE=priv->switch_core;

//#if defined(ETHSW_CORE)
	if (ETHSW_CORE) {
    	int i;
    	int offset_jump=ARRAY_SIZE(ETHSW_CORE->port_vlan_ctrl)/9;

    	/* Save MAC port and PBVLAN registers to save boot strap status */
    	extsw_register_save_restore(ETHSW_CORE, 1);

    	/* Enable MAC port Tx/Rx for CFE local traffic */
    	printk ("Enable Switch MAC Port Rx/Tx, set PBVLAN to FAN out, set switch to NO-STP. offset_jump = %d\n", offset_jump);
    	for( i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
    	{
        	/* Set Port VLAN to allow CPU traffic only */
        	ETHSW_CORE->port_vlan_ctrl[offset_jump*i] = PBMAP_MIPS;

        	/* Setting switch to NO-STP mode; enable port TX/RX. */
        	ETHSW_CORE->port_traffic_ctrl[i] = ((ETHSW_CORE->port_traffic_ctrl[i] & 0xff) & 
            	(~(PORT_CTRL_RXTX_DISABLE|PORT_CTRL_PORT_STATUS_M))) | PORT_CTRL_NO_STP;
    	}
	}

    return;
}

/* SF2 switch post process to restore switch status too boot strap status */
static void bcm_ethsw_close(struct udevice *dev)
{
	struct bcmbca_sf2_priv *priv = dev_get_priv(dev);

	volatile EthernetSwitchCore *ETHSW_CORE=priv->switch_core;
	printk("Restore Switch's MAC port Rx/Tx, PBVLAN back.\n");

	if (ETHSW_CORE)
	{
		extsw_register_save_restore(ETHSW_CORE, 0);
	}
}

#if defined(CONFIG_BCM6756)
void sf2_base_init(uintptr_t reg_base, uintptr_t core_base);
#endif

static int sf2_eth_probe(struct udevice *dev)
{
	struct resource res;
	ofnode subnode, port_node, phy_node;
	int ret, len;
	const fdt32_t *list;
	uint32_t phandle, phy_id;
	struct bcmbca_sf2_priv *priv = dev_get_priv(dev);
	const char *phy_mode;

    priv->ops.init  = bcm_ethsw_init;
    priv->ops.open  = bcm_ethsw_open;
    priv->ops.close = bcm_ethsw_close;

	priv->switch_mdio = NULL;
	ret = dev_read_resource_byname(dev, "switchmdio-base", &res);
	if (!ret) {
		priv->switch_mdio = devm_ioremap(dev, res.start, resource_size(&res));
		phy_set_mdio_base(priv->switch_mdio);
	}
	priv->switch_core = NULL;
	ret = dev_read_resource_byname(dev, "switchcore-base", &res);
	if (!ret) {
		priv->switch_core = devm_ioremap(dev, res.start, resource_size(&res));
#if defined(CONFIG_BCM6756)
	}
	ret = dev_read_resource_byname(dev, "switchreg-base", &res);
	if (!ret) {
		sf2_base_init(devm_ioremap(dev, res.start, resource_size(&res)), (void*)priv->switch_core);
#endif
	}

	priv->sphy_ctrl = NULL;
	ret = dev_read_resource_byname(dev, "sphy-ctrl", &res);
	if (!ret) {
		priv->sphy_ctrl = devm_ioremap(dev, res.start, resource_size(&res));
	}
	priv->qphy_ctrl = NULL;
	ret = dev_read_resource_byname(dev, "qphy-ctrl", &res);
	if (!ret) {
		priv->qphy_ctrl = devm_ioremap(dev, res.start, resource_size(&res));
	}
	priv->phy_test_ctrl = NULL;
	ret = dev_read_resource_byname(dev, "phy-test-ctrl", &res);
	if (!ret) {
		priv->phy_test_ctrl = devm_ioremap(dev, res.start, resource_size(&res));
	}

	priv->phy_base = dev_read_u32_default(dev, "phy_base", 1);
	priv->phy_wkard_timeout = 0;
	dev_read_u32(dev, "phy_wkard_timeout", &priv->phy_wkard_timeout);

	printf("sf2 phy_base %d phy power on workaround timeout %d\n", priv->phy_base, priv->phy_wkard_timeout);

	priv->num_phys = 0;	
	dev_for_each_subnode(subnode, dev) {

		ofnode_for_each_subnode(port_node, subnode) {

			list = ofnode_get_property (port_node, "phy-handle", &len);
			if (list) {

				phandle = fdt32_to_cpu (*list);
				phy_node = ofnode_get_by_phandle(phandle);
				if (ofnode_valid(phy_node)) {
					if (!ofnode_read_u32(phy_node, "reg", &phy_id)) {
						debug("phy_id = 0x%x phandle %d\n", phy_id, phandle);
					}
					phy_mode = ofnode_read_string(port_node, "phy-mode");
					// only worry about the GMII ports for now, add later
					if (!strcasecmp(phy_mode, "gmii")) {
						priv->phy_ids[priv->num_phys++] = (phy_id | ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID);
					}
				}
			}
		}
	}

	return 0;
}

static const struct udevice_id bcmbca_sf2_match_ids[] = {
	{ .compatible = "brcm,bcmbca-sf2"},
	{ }
};

U_BOOT_DRIVER(ethsw) = {
	.name = "brcm,ethsw",
	.id = UCLASS_NOP,
	.of_match = bcmbca_sf2_match_ids,
	.flags  = DM_REMOVE_ACTIVE_ALL,
	.probe = sf2_eth_probe,
	.priv_auto_alloc_size = sizeof(struct bcmbca_sf2_priv),
};

