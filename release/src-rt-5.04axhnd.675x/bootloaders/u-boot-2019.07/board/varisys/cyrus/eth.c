// SPDX-License-Identifier: GPL-2.0+
/*
 * Author Adrian Cox
 * Based somewhat on board/freescale/corenet_ds/eth_hydra.c
 */

#include <common.h>
#include <netdev.h>
#include <asm/fsl_serdes.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <malloc.h>
#include <fdt_support.h>
#include <fsl_dtsec.h>

#ifdef CONFIG_FMAN_ENET

#define FIRST_PORT_ADDR 3
#define SECOND_PORT_ADDR 7

#ifdef CONFIG_ARCH_P5040
#define FIRST_PORT FM1_DTSEC5
#define SECOND_PORT FM2_DTSEC5
#else
#define FIRST_PORT FM1_DTSEC4
#define SECOND_PORT FM1_DTSEC5
#endif

#define IS_VALID_PORT(p)  ((p) == FIRST_PORT || (p) == SECOND_PORT)

static void cyrus_phy_tuning(int phy)
{
	/*
	 * Enable RGMII delay on Tx and Rx for CPU port
	 */
	printf("Tuning PHY @ %d\n", phy);

	/* sets address 0x104 or reg 260 for writing */
	miiphy_write(DEFAULT_FM_MDIO_NAME, phy, 0xb, 0x8104);
	/* Sets RXC/TXC to +0.96ns and TX_CTL/RX_CTL to -0.84ns */
	miiphy_write(DEFAULT_FM_MDIO_NAME, phy, 0xc, 0xf0f0);
	/* sets address 0x105 or reg 261 for writing */
	miiphy_write(DEFAULT_FM_MDIO_NAME, phy, 0xb, 0x8105);
	/* writes to address 0x105 , RXD[3..0] to -0. */
	miiphy_write(DEFAULT_FM_MDIO_NAME, phy, 0xc, 0x0000);
	/* sets address 0x106 or reg 261 for writing */
	miiphy_write(DEFAULT_FM_MDIO_NAME, phy, 0xb, 0x8106);
	/* writes to address 0x106 , TXD[3..0] to -0.84ns */
	miiphy_write(DEFAULT_FM_MDIO_NAME, phy, 0xc, 0x0000);
	/* force re-negotiation */
	miiphy_write(DEFAULT_FM_MDIO_NAME, phy, 0x0, 0x1340);
}
#endif

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_FMAN_ENET
	struct fsl_pq_mdio_info dtsec_mdio_info;
	unsigned int i;

	printf("Initializing Fman\n");


	/* Register the real 1G MDIO bus */
	dtsec_mdio_info.regs =
		(struct tsec_mii_mng *)CONFIG_SYS_FM1_DTSEC1_MDIO_ADDR;
	dtsec_mdio_info.name = DEFAULT_FM_MDIO_NAME;

	fsl_pq_mdio_init(bis, &dtsec_mdio_info);


	fm_info_set_phy_address(FIRST_PORT, FIRST_PORT_ADDR);
	fm_info_set_mdio(FIRST_PORT,
			miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME));
	fm_info_set_phy_address(SECOND_PORT, SECOND_PORT_ADDR);
	fm_info_set_mdio(SECOND_PORT,
			miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME));

	/* Never disable DTSEC1 - it controls MDIO */
	for (i = FM1_DTSEC2; i < FM1_DTSEC1 + CONFIG_SYS_NUM_FM1_DTSEC; i++) {
		if (!IS_VALID_PORT(i))
			fm_disable_port(i);
	}

#ifdef CONFIG_ARCH_P5040
	for (i = FM2_DTSEC2; i < FM2_DTSEC1 + CONFIG_SYS_NUM_FM2_DTSEC; i++) {
		if (!IS_VALID_PORT(i))
			fm_disable_port(i);
	}
#endif

	cpu_eth_init(bis);

	cyrus_phy_tuning(FIRST_PORT_ADDR);
	cyrus_phy_tuning(SECOND_PORT_ADDR);
#endif

	return pci_eth_init(bis);
}
