// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2019 NXP
 *
 */

#include <common.h>
#include <hwconfig.h>
#include <command.h>
#include <netdev.h>
#include <malloc.h>
#include <fsl_mdio.h>
#include <miiphy.h>
#include <phy.h>
#include <fm_eth.h>
#include <asm/io.h>
#include <exports.h>
#include <asm/arch/fsl_serdes.h>
#include <fsl-mc/fsl_mc.h>
#include <fsl-mc/ldpaa_wriop.h>

#include "../common/qixis.h"

DECLARE_GLOBAL_DATA_PTR;

#define EMI_NONE	0
#define EMI1		1 /* Mdio Bus 1 */
#define EMI2		2 /* Mdio Bus 2 */

#if defined(CONFIG_FSL_MC_ENET)
enum io_slot {
	IO_SLOT_NONE = 0,
	IO_SLOT_1,
	IO_SLOT_2,
	IO_SLOT_3,
	IO_SLOT_4,
	IO_SLOT_5,
	IO_SLOT_6,
	IO_SLOT_7,
	IO_SLOT_8,
	EMI1_RGMII1,
	EMI1_RGMII2,
	IO_SLOT_MAX
};

struct lx2160a_qds_mdio {
	enum io_slot ioslot : 4;
	u8 realbusnum : 4;
	struct mii_dev *realbus;
};

/* structure explaining the phy configuration on 8 lanes of a serdes*/
struct serdes_phy_config {
	u8 serdes; /* serdes protocol */
	struct phy_config {
		u8 dpmacid;
		/* -1 terminated array */
		int phy_address[WRIOP_MAX_PHY_NUM + 1];
		u8 mdio_bus;
		enum io_slot ioslot;
	} phy_config[SRDS_MAX_LANES];
};

/* Table defining the phy configuration on 8 lanes of a serdes.
 * Various assumptions have been made while defining this table.
 * e.g. for serdes1 protocol 19 it is being assumed that X-M11-USXGMII
 * card is being used for dpmac 3-4. (X-M12-XFI could also have been used)
 * And also that this card is connected to IO Slot 1 (could have been connected
 * to any of the 8 IO slots (IO slot 1 - IO slot 8)).
 * similarly, it is also being assumed that MDIO 1 is selected on X-M7-40G card
 * used in serdes1 protocol 19 (could have selected MDIO 2)
 * To override these settings "dpmac" environment variable can be used after
 * defining "dpmac_override" in hwconfig environment variable.
 * This table has limited serdes protocol entries. It can be expanded as per
 * requirement.
 */
static const struct serdes_phy_config serdes1_phy_config[] = {
	{3, {{WRIOP1_DPMAC3, {AQ_PHY_ADDR1, -1},
	      EMI1, IO_SLOT_1},
	    {WRIOP1_DPMAC4, {AQ_PHY_ADDR2, -1},
	     EMI1, IO_SLOT_1},
	    {WRIOP1_DPMAC5, {AQ_PHY_ADDR3, -1},
	     EMI1, IO_SLOT_1},
	    {WRIOP1_DPMAC6, {AQ_PHY_ADDR4, -1},
	     EMI1, IO_SLOT_1} } },
	{7, {{WRIOP1_DPMAC3, {AQ_PHY_ADDR1, -1},
	      EMI1, IO_SLOT_1},
	    {WRIOP1_DPMAC4, {AQ_PHY_ADDR2, -1},
	     EMI1, IO_SLOT_1},
	    {WRIOP1_DPMAC5, {AQ_PHY_ADDR3, -1},
	     EMI1, IO_SLOT_1},
	    {WRIOP1_DPMAC6, {AQ_PHY_ADDR4, -1},
	     EMI1, IO_SLOT_1},
	    {WRIOP1_DPMAC7, {SGMII_CARD_PORT1_PHY_ADDR, -1},
	     EMI1, IO_SLOT_2},
	    {WRIOP1_DPMAC8, {SGMII_CARD_PORT2_PHY_ADDR, -1},
	     EMI1, IO_SLOT_2},
	    {WRIOP1_DPMAC9, {SGMII_CARD_PORT3_PHY_ADDR, -1},
	     EMI1, IO_SLOT_2},
	    {WRIOP1_DPMAC10, {SGMII_CARD_PORT4_PHY_ADDR, -1},
	     EMI1, IO_SLOT_2} } },
	{8, {} },
	{13, {{WRIOP1_DPMAC1, {INPHI_PHY_ADDR1, INPHI_PHY_ADDR2, -1},
	       EMI1, IO_SLOT_1},
	     {WRIOP1_DPMAC2, {INPHI_PHY_ADDR1, INPHI_PHY_ADDR2, -1},
	      EMI1, IO_SLOT_2} } },
	{15, {{WRIOP1_DPMAC1, {INPHI_PHY_ADDR1, INPHI_PHY_ADDR2, -1},
	       EMI1, IO_SLOT_1},
	     {WRIOP1_DPMAC2, {INPHI_PHY_ADDR1, INPHI_PHY_ADDR2, -1},
	      EMI1, IO_SLOT_1} } },
	{17, {{WRIOP1_DPMAC3, {INPHI_PHY_ADDR1, INPHI_PHY_ADDR2, -1},
	       EMI1, IO_SLOT_1},
	     {WRIOP1_DPMAC4, {INPHI_PHY_ADDR1, INPHI_PHY_ADDR2, -1},
	      EMI1, IO_SLOT_1},
	     {WRIOP1_DPMAC5, {INPHI_PHY_ADDR1, INPHI_PHY_ADDR2, -1},
	      EMI1, IO_SLOT_1},
	     {WRIOP1_DPMAC6, {INPHI_PHY_ADDR1, INPHI_PHY_ADDR2, -1},
	      EMI1, IO_SLOT_1} } },
	{19, {{WRIOP1_DPMAC2, {CORTINA_PHY_ADDR1, -1},
	       EMI1, IO_SLOT_2},
	     {WRIOP1_DPMAC3, {AQ_PHY_ADDR1, -1},
	      EMI1, IO_SLOT_1},
	     {WRIOP1_DPMAC4, {AQ_PHY_ADDR2, -1},
	      EMI1, IO_SLOT_1},
	     {WRIOP1_DPMAC5, {INPHI_PHY_ADDR1, INPHI_PHY_ADDR2, -1},
	      EMI1, IO_SLOT_6},
	     {WRIOP1_DPMAC6, {INPHI_PHY_ADDR1, INPHI_PHY_ADDR2, -1},
	      EMI1, IO_SLOT_6} } },
	{20, {{WRIOP1_DPMAC1, {CORTINA_PHY_ADDR1, -1},
	       EMI1, IO_SLOT_1},
	     {WRIOP1_DPMAC2, {CORTINA_PHY_ADDR1, -1},
	      EMI1, IO_SLOT_2} } }
};

static const struct serdes_phy_config serdes2_phy_config[] = {
	{2, {} },
	{3, {} },
	{5, {} },
	{11, {{WRIOP1_DPMAC12, {SGMII_CARD_PORT2_PHY_ADDR, -1},
	       EMI1, IO_SLOT_7},
	     {WRIOP1_DPMAC17, {SGMII_CARD_PORT3_PHY_ADDR, -1},
	      EMI1, IO_SLOT_7},
	     {WRIOP1_DPMAC18, {SGMII_CARD_PORT4_PHY_ADDR, -1},
	      EMI1, IO_SLOT_7},
	     {WRIOP1_DPMAC16, {SGMII_CARD_PORT2_PHY_ADDR, -1},
	      EMI1, IO_SLOT_8},
	     {WRIOP1_DPMAC13, {SGMII_CARD_PORT3_PHY_ADDR, -1},
	      EMI1, IO_SLOT_8},
	     {WRIOP1_DPMAC14, {SGMII_CARD_PORT4_PHY_ADDR, -1},
	      EMI1, IO_SLOT_8} } },
};

static const struct serdes_phy_config serdes3_phy_config[] = {
	{2, {} },
	{3, {} }
};

static inline
const struct phy_config *get_phy_config(u8 serdes,
					const struct serdes_phy_config *table,
					u8 table_size)
{
	int i;

	for (i = 0; i < table_size; i++) {
		if (table[i].serdes == serdes)
			return table[i].phy_config;
	}

	return NULL;
}

/* BRDCFG4 controls EMI routing for the board.
 * Bits    Function
 * 7-6     EMI Interface #1 Primary Routing (CFG_MUX1_EMI1) (1.8V):
 * EMI1    00= On-board PHY #1
 *         01= On-board PHY #2
 *         10= (reserved)
 *         11= Slots 1..8 multiplexer and translator.
 * 5-3     EMI Interface #1 Secondary Routing (CFG_MUX2_EMI1) (2.5V):
 * EMI1X   000= Slot #1
 *         001= Slot #2
 *         010= Slot #3
 *         011= Slot #4
 *         100= Slot #5
 *         101= Slot #6
 *         110= Slot #7
 *         111= Slot #8
 * 2-0     EMI Interface #2 Routing (CFG_MUX_EMI2):
 * EMI2    000= Slot #1 (secondary EMI)
 *         001= Slot #2 (secondary EMI)
 *         010= Slot #3 (secondary EMI)
 *         011= Slot #4 (secondary EMI)
 *         100= Slot #5 (secondary EMI)
 *         101= Slot #6 (secondary EMI)
 *         110= Slot #7 (secondary EMI)
 *         111= Slot #8 (secondary EMI)
 */
static int lx2160a_qds_get_mdio_mux_val(u8 realbusnum, enum io_slot ioslot)
{
	switch (realbusnum) {
	case EMI1:
		switch (ioslot) {
		case EMI1_RGMII1:
			return 0;
		case EMI1_RGMII2:
			return 0x40;
		default:
			return (((ioslot - 1) << BRDCFG4_EMI1SEL_SHIFT) | 0xC0);
		}
		break;
	case EMI2:
		return ((ioslot - 1) << BRDCFG4_EMI2SEL_SHIFT);
	default:
		return -1;
	}
}

static void lx2160a_qds_mux_mdio(struct lx2160a_qds_mdio *priv)
{
	u8 brdcfg4, mux_val, reg;

	brdcfg4 = QIXIS_READ(brdcfg[4]);
	reg = brdcfg4;
	mux_val = lx2160a_qds_get_mdio_mux_val(priv->realbusnum, priv->ioslot);

	switch (priv->realbusnum) {
	case EMI1:
		brdcfg4 &= ~BRDCFG4_EMI1SEL_MASK;
		brdcfg4 |= mux_val;
		break;
	case EMI2:
		brdcfg4 &= ~BRDCFG4_EMI2SEL_MASK;
		brdcfg4 |= mux_val;
		break;
	}

	if (brdcfg4 ^ reg)
		QIXIS_WRITE(brdcfg[4], brdcfg4);
}

static int lx2160a_qds_mdio_read(struct mii_dev *bus, int addr,
				 int devad, int regnum)
{
	struct lx2160a_qds_mdio *priv = bus->priv;

	lx2160a_qds_mux_mdio(priv);

	return priv->realbus->read(priv->realbus, addr, devad, regnum);
}

static int lx2160a_qds_mdio_write(struct mii_dev *bus, int addr, int devad,
				  int regnum, u16 value)
{
	struct lx2160a_qds_mdio *priv = bus->priv;

	lx2160a_qds_mux_mdio(priv);

	return priv->realbus->write(priv->realbus, addr, devad, regnum, value);
}

static int lx2160a_qds_mdio_reset(struct mii_dev *bus)
{
	struct lx2160a_qds_mdio *priv = bus->priv;

	return priv->realbus->reset(priv->realbus);
}

static struct mii_dev *lx2160a_qds_mdio_init(u8 realbusnum, enum io_slot ioslot)
{
	struct lx2160a_qds_mdio *pmdio;
	struct mii_dev *bus;
	/*should be within MDIO_NAME_LEN*/
	char dummy_mdio_name[] = "LX2160A_QDS_MDIO1_IOSLOT1";

	if (realbusnum == EMI2) {
		if (ioslot < IO_SLOT_1 || ioslot > IO_SLOT_8) {
			printf("invalid ioslot %d\n", ioslot);
			return NULL;
		}
	} else if (realbusnum == EMI1) {
		if (ioslot < IO_SLOT_1 || ioslot > EMI1_RGMII2) {
			printf("invalid ioslot %d\n", ioslot);
			return NULL;
		}
	} else {
		printf("not supported real mdio bus %d\n", realbusnum);
		return NULL;
	}

	if (ioslot == EMI1_RGMII1)
		strcpy(dummy_mdio_name, "LX2160A_QDS_MDIO1_RGMII1");
	else if (ioslot == EMI1_RGMII2)
		strcpy(dummy_mdio_name, "LX2160A_QDS_MDIO1_RGMII2");
	else
		sprintf(dummy_mdio_name, "LX2160A_QDS_MDIO%d_IOSLOT%d",
			realbusnum, ioslot);
	bus = miiphy_get_dev_by_name(dummy_mdio_name);

	if (bus)
		return bus;

	bus = mdio_alloc();
	if (!bus) {
		printf("Failed to allocate %s bus\n", dummy_mdio_name);
		return NULL;
	}

	pmdio = malloc(sizeof(*pmdio));
	if (!pmdio) {
		printf("Failed to allocate %s private data\n", dummy_mdio_name);
		free(bus);
		return NULL;
	}

	switch (realbusnum) {
	case EMI1:
		pmdio->realbus =
		  miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO1_NAME);
		break;
	case EMI2:
		pmdio->realbus =
		  miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO2_NAME);
		break;
	}

	if (!pmdio->realbus) {
		printf("No real mdio bus num %d found\n", realbusnum);
		free(bus);
		free(pmdio);
		return NULL;
	}

	pmdio->realbusnum = realbusnum;
	pmdio->ioslot = ioslot;
	bus->read = lx2160a_qds_mdio_read;
	bus->write = lx2160a_qds_mdio_write;
	bus->reset = lx2160a_qds_mdio_reset;
	strcpy(bus->name, dummy_mdio_name);
	bus->priv = pmdio;

	if (!mdio_register(bus))
		return bus;

	printf("No bus with name %s\n", dummy_mdio_name);
	free(bus);
	free(pmdio);
	return NULL;
}

static inline void do_phy_config(const struct phy_config *phy_config)
{
	struct mii_dev *bus;
	int i, phy_num, phy_address;

	for (i = 0; i < SRDS_MAX_LANES; i++) {
		if (!phy_config[i].dpmacid)
			continue;

		for (phy_num = 0;
		     phy_num < ARRAY_SIZE(phy_config[i].phy_address);
		     phy_num++) {
			phy_address = phy_config[i].phy_address[phy_num];
			if (phy_address == -1)
				break;
			wriop_set_phy_address(phy_config[i].dpmacid,
					      phy_num, phy_address);
		}
		/*Register the muxing front-ends to the MDIO buses*/
		bus = lx2160a_qds_mdio_init(phy_config[i].mdio_bus,
					    phy_config[i].ioslot);
		if (!bus)
			printf("could not get bus for mdio %d ioslot %d\n",
			       phy_config[i].mdio_bus,
			       phy_config[i].ioslot);
		else
			wriop_set_mdio(phy_config[i].dpmacid, bus);
	}
}

static inline void do_dpmac_config(int dpmac, const char *arg_dpmacid,
				   char *env_dpmac)
{
	const char *ret;
	size_t len;
	u8 realbusnum, ioslot;
	struct mii_dev *bus;
	int phy_num;
	char *phystr = "phy00";

	/*search phy in dpmac arg*/
	for (phy_num = 0; phy_num < WRIOP_MAX_PHY_NUM; phy_num++) {
		sprintf(phystr, "phy%d", phy_num + 1);
		ret = hwconfig_subarg_f(arg_dpmacid, phystr, &len, env_dpmac);
		if (!ret) {
			/*look for phy instead of phy1*/
			if (!phy_num)
				ret = hwconfig_subarg_f(arg_dpmacid, "phy",
							&len, env_dpmac);
			if (!ret)
				continue;
		}

		if (len != 4 || strncmp(ret, "0x", 2))
			printf("invalid phy format in %s variable.\n"
			       "specify phy%d for %s in hex format e.g. 0x12\n",
			       env_dpmac, phy_num + 1, arg_dpmacid);
		else
			wriop_set_phy_address(dpmac, phy_num,
					      simple_strtoul(ret, NULL, 16));
	}

	/*search mdio in dpmac arg*/
	ret = hwconfig_subarg_f(arg_dpmacid, "mdio", &len, env_dpmac);
	if (ret)
		realbusnum = *ret - '0';
	else
		realbusnum = EMI_NONE;

	if (realbusnum) {
		/*search io in dpmac arg*/
		ret = hwconfig_subarg_f(arg_dpmacid, "io", &len, env_dpmac);
		if (ret)
			ioslot = *ret - '0';
		else
			ioslot = IO_SLOT_NONE;
		/*Register the muxing front-ends to the MDIO buses*/
		bus = lx2160a_qds_mdio_init(realbusnum, ioslot);
		if (!bus)
			printf("could not get bus for mdio %d ioslot %d\n",
			       realbusnum, ioslot);
		else
			wriop_set_mdio(dpmac, bus);
	}
}

#endif

int board_eth_init(bd_t *bis)
{
#if defined(CONFIG_FSL_MC_ENET)
	struct memac_mdio_info mdio_info;
	struct memac_mdio_controller *regs;
	int i;
	const char *ret;
	char *env_dpmac;
	char dpmacid[] = "dpmac00", srds[] = "00_00_00";
	size_t len;
	struct mii_dev *bus;
	const struct phy_config *phy_config;
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 srds_s1, srds_s2, srds_s3;

	srds_s1 = in_le32(&gur->rcwsr[28]) &
		  FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_SHIFT;

	srds_s2 = in_le32(&gur->rcwsr[28]) &
		  FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_MASK;
	srds_s2 >>= FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_SHIFT;

	srds_s3 = in_le32(&gur->rcwsr[28]) &
		  FSL_CHASSIS3_RCWSR28_SRDS3_PRTCL_MASK;
	srds_s3 >>= FSL_CHASSIS3_RCWSR28_SRDS3_PRTCL_SHIFT;

	sprintf(srds, "%d_%d_%d", srds_s1, srds_s2, srds_s3);

	regs = (struct memac_mdio_controller *)CONFIG_SYS_FSL_WRIOP1_MDIO1;
	mdio_info.regs = regs;
	mdio_info.name = DEFAULT_WRIOP_MDIO1_NAME;

	/*Register the EMI 1*/
	fm_memac_mdio_init(bis, &mdio_info);

	regs = (struct memac_mdio_controller *)CONFIG_SYS_FSL_WRIOP1_MDIO2;
	mdio_info.regs = regs;
	mdio_info.name = DEFAULT_WRIOP_MDIO2_NAME;

	/*Register the EMI 2*/
	fm_memac_mdio_init(bis, &mdio_info);

	/* "dpmac" environment variable can be used after
	 * defining "dpmac_override" in hwconfig environment variable.
	 */
	if (hwconfig("dpmac_override")) {
		env_dpmac = env_get("dpmac");
		if (env_dpmac) {
			ret = hwconfig_arg_f("srds", &len, env_dpmac);
			if (ret) {
				if (strncmp(ret, srds, strlen(srds))) {
					printf("SERDES configuration changed.\n"
					       "previous: %.*s, current: %s.\n"
					       "update dpmac variable.\n",
					       (int)len, ret, srds);
				}
			} else {
				printf("SERDES configuration not found.\n"
				       "Please add srds:%s in dpmac variable\n",
				       srds);
			}

			for (i = WRIOP1_DPMAC1; i < NUM_WRIOP_PORTS; i++) {
				/* Look for dpmac1 to dpmac24(current max) arg
				 * in dpmac environment variable
				 */
				sprintf(dpmacid, "dpmac%d", i);
				ret = hwconfig_arg_f(dpmacid, &len, env_dpmac);
				if (ret)
					do_dpmac_config(i, dpmacid, env_dpmac);
			}
		} else {
			printf("Warning: environment dpmac not found.\n"
			       "DPAA network interfaces may not work\n");
		}
	} else {
		/*Look for phy config for serdes1 in phy config table*/
		phy_config = get_phy_config(srds_s1, serdes1_phy_config,
					    ARRAY_SIZE(serdes1_phy_config));
		if (!phy_config) {
			printf("%s WRIOP: Unsupported SerDes1 Protocol %d\n",
			       __func__, srds_s1);
		} else {
			do_phy_config(phy_config);
		}
		phy_config = get_phy_config(srds_s2, serdes2_phy_config,
					    ARRAY_SIZE(serdes2_phy_config));
		if (!phy_config) {
			printf("%s WRIOP: Unsupported SerDes2 Protocol %d\n",
			       __func__, srds_s2);
		} else {
			do_phy_config(phy_config);
		}
		phy_config = get_phy_config(srds_s3, serdes3_phy_config,
					    ARRAY_SIZE(serdes3_phy_config));
		if (!phy_config) {
			printf("%s WRIOP: Unsupported SerDes3 Protocol %d\n",
			       __func__, srds_s3);
		} else {
			do_phy_config(phy_config);
		}
	}

	if (wriop_get_enet_if(WRIOP1_DPMAC17) == PHY_INTERFACE_MODE_RGMII_ID) {
		wriop_set_phy_address(WRIOP1_DPMAC17, 0, RGMII_PHY_ADDR1);
		bus = lx2160a_qds_mdio_init(EMI1, EMI1_RGMII1);
		if (!bus)
			printf("could not get bus for RGMII1\n");
		else
			wriop_set_mdio(WRIOP1_DPMAC17, bus);
	}

	if (wriop_get_enet_if(WRIOP1_DPMAC18) == PHY_INTERFACE_MODE_RGMII_ID) {
		wriop_set_phy_address(WRIOP1_DPMAC18, 0, RGMII_PHY_ADDR2);
		bus = lx2160a_qds_mdio_init(EMI1, EMI1_RGMII2);
		if (!bus)
			printf("could not get bus for RGMII2\n");
		else
			wriop_set_mdio(WRIOP1_DPMAC18, bus);
	}

	cpu_eth_init(bis);
#endif /* CONFIG_FMAN_ENET */

#ifdef CONFIG_PHY_AQUANTIA
	/*
	 * Export functions to be used by AQ firmware
	 * upload application
	 */
	gd->jt->strcpy = strcpy;
	gd->jt->mdelay = mdelay;
	gd->jt->mdio_get_current_dev = mdio_get_current_dev;
	gd->jt->phy_find_by_mask = phy_find_by_mask;
	gd->jt->mdio_phydev_for_ethname = mdio_phydev_for_ethname;
	gd->jt->miiphy_set_current_dev = miiphy_set_current_dev;
#endif
	return pci_eth_init(bis);
}

#if defined(CONFIG_RESET_PHY_R)
void reset_phy(void)
{
#if defined(CONFIG_FSL_MC_ENET)
	mc_env_boot();
#endif
}
#endif /* CONFIG_RESET_PHY_R */

#if defined(CONFIG_FSL_MC_ENET)
int fdt_fixup_dpmac_phy_handle(void *fdt, int dpmac_id, int node_phandle)
{
	int offset;
	int ret;
	char dpmac_str[] = "dpmacs@00";
	const char *phy_string;

	offset = fdt_path_offset(fdt, "/soc/fsl-mc/dpmacs");

	if (offset < 0)
		offset = fdt_path_offset(fdt, "/fsl-mc/dpmacs");

	if (offset < 0) {
		printf("dpmacs node not found in device tree\n");
		return offset;
	}

	sprintf(dpmac_str, "dpmac@%x", dpmac_id);
	debug("dpmac_str = %s\n", dpmac_str);

	offset = fdt_subnode_offset(fdt, offset, dpmac_str);
	if (offset < 0) {
		printf("%s node not found in device tree\n", dpmac_str);
		return offset;
	}

	ret = fdt_appendprop_cell(fdt, offset, "phy-handle", node_phandle);
	if (ret)
		printf("%d@%s %d\n", __LINE__, __func__, ret);

	phy_string = phy_string_for_interface(wriop_get_enet_if(dpmac_id));
	ret = fdt_setprop_string(fdt, offset, "phy-connection-type",
				 phy_string);
	if (ret)
		printf("%d@%s %d\n", __LINE__, __func__, ret);

	return ret;
}

int fdt_get_ioslot_offset(void *fdt, struct mii_dev *mii_dev, int fpga_offset)
{
	char mdio_ioslot_str[] = "mdio@00";
	struct lx2160a_qds_mdio *priv;
	u64 reg;
	u32 phandle;
	int offset, mux_val;

	/*Test if the MDIO bus is real mdio bus or muxing front end ?*/
	if (strncmp(mii_dev->name, "LX2160A_QDS_MDIO",
		    strlen("LX2160A_QDS_MDIO")))
		return -1;

	/*Get the real MDIO bus num and ioslot info from bus's priv data*/
	priv = mii_dev->priv;

	debug("real_bus_num = %d, ioslot = %d\n",
	      priv->realbusnum, priv->ioslot);

	if (priv->realbusnum == EMI1)
		reg = CONFIG_SYS_FSL_WRIOP1_MDIO1;
	else
		reg = CONFIG_SYS_FSL_WRIOP1_MDIO2;

	offset = fdt_node_offset_by_compat_reg(fdt, "fsl,fman-memac-mdio", reg);
	if (offset < 0) {
		printf("mdio@%llx node not found in device tree\n", reg);
		return offset;
	}

	phandle = fdt_get_phandle(fdt, offset);
	phandle = cpu_to_fdt32(phandle);
	offset = fdt_node_offset_by_prop_value(fdt, -1, "mdio-parent-bus",
					       &phandle, 4);
	if (offset < 0) {
		printf("mdio-mux-%d node not found in device tree\n",
		       priv->realbusnum == EMI1 ? 1 : 2);
		return offset;
	}

	mux_val = lx2160a_qds_get_mdio_mux_val(priv->realbusnum, priv->ioslot);
	if (priv->realbusnum == EMI1)
		mux_val >>= BRDCFG4_EMI1SEL_SHIFT;
	else
		mux_val >>= BRDCFG4_EMI2SEL_SHIFT;
	sprintf(mdio_ioslot_str, "mdio@%x", (u8)mux_val);

	offset = fdt_subnode_offset(fdt, offset, mdio_ioslot_str);
	if (offset < 0) {
		printf("%s node not found in device tree\n", mdio_ioslot_str);
		return offset;
	}

	return offset;
}

int fdt_create_phy_node(void *fdt, int offset, u8 phyaddr, int *subnodeoffset,
			struct phy_device *phy_dev, int phandle)
{
	char phy_node_name[] = "ethernet-phy@00";
	char phy_id_compatible_str[] = "ethernet-phy-id0000.0000";
	int ret;

	sprintf(phy_node_name, "ethernet-phy@%x", phyaddr);
	debug("phy_node_name = %s\n", phy_node_name);

	*subnodeoffset = fdt_add_subnode(fdt, offset, phy_node_name);
	if (*subnodeoffset <= 0) {
		printf("Could not add subnode %s inside node %s err = %s\n",
		       phy_node_name, fdt_get_name(fdt, offset, NULL),
		       fdt_strerror(*subnodeoffset));
		return *subnodeoffset;
	}

	sprintf(phy_id_compatible_str, "ethernet-phy-id%04x.%04x",
		phy_dev->phy_id >> 16, phy_dev->phy_id & 0xFFFF);
	debug("phy_id_compatible_str %s\n", phy_id_compatible_str);

	ret = fdt_setprop_string(fdt, *subnodeoffset, "compatible",
				 phy_id_compatible_str);
	if (ret) {
		printf("%d@%s %d\n", __LINE__, __func__, ret);
		goto out;
	}

	if (phy_dev->is_c45) {
		ret = fdt_appendprop_string(fdt, *subnodeoffset, "compatible",
					    "ethernet-phy-ieee802.3-c45");
		if (ret) {
			printf("%d@%s %d\n", __LINE__, __func__, ret);
			goto out;
		}
	} else {
		ret = fdt_appendprop_string(fdt, *subnodeoffset, "compatible",
					    "ethernet-phy-ieee802.3-c22");
		if (ret) {
			printf("%d@%s %d\n", __LINE__, __func__, ret);
			goto out;
		}
	}

	ret = fdt_setprop_cell(fdt, *subnodeoffset, "reg", phyaddr);
	if (ret) {
		printf("%d@%s %d\n", __LINE__, __func__, ret);
		goto out;
	}

	ret = fdt_set_phandle(fdt, *subnodeoffset, phandle);
	if (ret) {
		printf("%d@%s %d\n", __LINE__, __func__, ret);
		goto out;
	}

out:
	if (ret)
		fdt_del_node(fdt, *subnodeoffset);

	return ret;
}

int fdt_fixup_board_phy(void *fdt)
{
	int fpga_offset, offset, subnodeoffset;
	struct mii_dev *mii_dev;
	struct list_head *mii_devs, *entry;
	int ret, dpmac_id, phandle, i;
	struct phy_device *phy_dev;
	char ethname[ETH_NAME_LEN];
	phy_interface_t	phy_iface;

	ret = 0;
	/* we know FPGA is connected to i2c0, therefore search path directly,
	 * instead of compatible property, as it saves time
	 */
	fpga_offset = fdt_path_offset(fdt, "/soc/i2c@2000000/fpga");

	if (fpga_offset < 0)
		fpga_offset = fdt_path_offset(fdt, "/i2c@2000000/fpga");

	if (fpga_offset < 0) {
		printf("i2c@2000000/fpga node not found in device tree\n");
		return fpga_offset;
	}

	phandle = fdt_alloc_phandle(fdt);
	mii_devs = mdio_get_list_head();

	list_for_each(entry, mii_devs) {
		mii_dev = list_entry(entry, struct mii_dev, link);
		debug("mii_dev name : %s\n", mii_dev->name);
		offset = fdt_get_ioslot_offset(fdt, mii_dev, fpga_offset);
		if (offset < 0)
			continue;

		// Look for phy devices attached to MDIO bus muxing front end
		// and create their entries with compatible being the device id
		for (i = 0; i < PHY_MAX_ADDR; i++) {
			phy_dev = mii_dev->phymap[i];
			if (!phy_dev)
				continue;

			// TODO: use sscanf instead of loop
			dpmac_id = WRIOP1_DPMAC1;
			while (dpmac_id < NUM_WRIOP_PORTS) {
				phy_iface = wriop_get_enet_if(dpmac_id);
				snprintf(ethname, ETH_NAME_LEN, "DPMAC%d@%s",
					 dpmac_id,
					 phy_string_for_interface(phy_iface));
				if (strcmp(ethname, phy_dev->dev->name) == 0)
					break;
				dpmac_id++;
			}
			if (dpmac_id == NUM_WRIOP_PORTS)
				continue;
			ret = fdt_create_phy_node(fdt, offset, i,
						  &subnodeoffset,
						  phy_dev, phandle);
			if (ret)
				break;

			ret = fdt_fixup_dpmac_phy_handle(fdt,
							 dpmac_id, phandle);
			if (ret) {
				fdt_del_node(fdt, subnodeoffset);
				break;
			}
			/* calculate offset again as new node addition may have
			 * changed offset;
			 */
			offset = fdt_get_ioslot_offset(fdt, mii_dev,
						       fpga_offset);
			phandle++;
		}

		if (ret)
			break;
	}

	return ret;
}
#endif // CONFIG_FSL_MC_ENET

