// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/io.h>
#include <miiphy.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <fsl_mdio.h>
#include <tsec.h>
#include <mmc.h>
#include <netdev.h>
#include <pci.h>
#include <asm/fsl_serdes.h>
#include <fsl_ifc.h>
#include <asm/fsl_pci.h>
#include <hwconfig.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

#define GPIO4_PCIE_RESET_SET		0x08000000
#define MUX_CPLD_CAN_UART		0x00
#define MUX_CPLD_TDM			0x01
#define MUX_CPLD_SPICS0_FLASH		0x00
#define MUX_CPLD_SPICS0_SLIC		0x02
#define PMUXCR1_IFC_MASK       0x00ffff00
#define PMUXCR1_SDHC_MASK      0x00fff000
#define PMUXCR1_SDHC_ENABLE    0x00555000

enum {
	MUX_TYPE_IFC,
	MUX_TYPE_SDHC,
	MUX_TYPE_SPIFLASH,
	MUX_TYPE_TDM,
	MUX_TYPE_CAN,
	MUX_TYPE_CS0_NOR,
	MUX_TYPE_CS0_NAND,
};

enum {
	I2C_READ_BANK,
	I2C_READ_PCB_VER,
};

static uint sd_ifc_mux;

struct cpld_data {
	u8 cpld_ver; /* cpld revision */
#if defined(CONFIG_TARGET_P1010RDB_PA)
	u8 pcba_ver; /* pcb revision number */
	u8 twindie_ddr3;
	u8 res1[6];
	u8 bank_sel; /* NOR Flash bank */
	u8 res2[5];
	u8 usb2_sel;
	u8 res3[1];
	u8 porsw_sel;
	u8 tdm_can_sel;
	u8 spi_cs0_sel; /* SPI CS0 SLIC/SPI Flash */
	u8 por0; /* POR Options */
	u8 por1; /* POR Options */
	u8 por2; /* POR Options */
	u8 por3; /* POR Options */
#elif defined(CONFIG_TARGET_P1010RDB_PB)
	u8 rom_loc;
#endif
};

int board_early_init_f(void)
{
	ccsr_gpio_t *pgpio = (void *)(CONFIG_SYS_MPC85xx_GPIO_ADDR);
	struct fsl_ifc ifc = {(void *)CONFIG_SYS_IFC_ADDR, (void *)NULL};
	/* Clock configuration to access CPLD using IFC(GPCM) */
	setbits_be32(&ifc.gregs->ifc_gcr, 1 << IFC_GCR_TBCTL_TRN_TIME_SHIFT);
	/*
	* Reset PCIe slots via GPIO4
	*/
	setbits_be32(&pgpio->gpdir, GPIO4_PCIE_RESET_SET);
	setbits_be32(&pgpio->gpdat, GPIO4_PCIE_RESET_SET);

	return 0;
}

int board_early_init_r(void)
{
	const unsigned int flashbase = CONFIG_SYS_FLASH_BASE;
	int flash_esel = find_tlb_idx((void *)flashbase, 1);

	/*
	 * Remap Boot flash region to caching-inhibited
	 * so that flash can be erased properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	if (flash_esel == -1) {
		/* very unlikely unless something is messed up */
		puts("Error: Could not find TLB for FLASH BASE\n");
		flash_esel = 2;	/* give our best effort to continue */
	} else {
		/* invalidate existing TLB entry for flash */
		disable_tlb(flash_esel);
	}

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS,
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
			0, flash_esel, BOOKE_PAGESZ_16M, 1);

	set_tlb(1, flashbase + 0x1000000,
			CONFIG_SYS_FLASH_BASE_PHYS + 0x1000000,
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
			0, flash_esel+1, BOOKE_PAGESZ_16M, 1);
	return 0;
}

#ifdef CONFIG_PCI
void pci_init_board(void)
{
	fsl_pcie_init_board(0);
}
#endif /* ifdef CONFIG_PCI */

int config_board_mux(int ctrl_type)
{
	ccsr_gur_t __iomem *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u8 tmp;

#if defined(CONFIG_TARGET_P1010RDB_PA)
	struct cpld_data *cpld_data = (void *)(CONFIG_SYS_CPLD_BASE);

	switch (ctrl_type) {
	case MUX_TYPE_IFC:
		i2c_set_bus_num(I2C_PCA9557_BUS_NUM);
		tmp = 0xf0;
		i2c_write(I2C_PCA9557_ADDR1, 3, 1, &tmp, 1);
		tmp = 0x01;
		i2c_write(I2C_PCA9557_ADDR1, 1, 1, &tmp, 1);
		sd_ifc_mux = MUX_TYPE_IFC;
		clrbits_be32(&gur->pmuxcr, PMUXCR1_IFC_MASK);
		break;
	case MUX_TYPE_SDHC:
		i2c_set_bus_num(I2C_PCA9557_BUS_NUM);
		tmp = 0xf0;
		i2c_write(I2C_PCA9557_ADDR1, 3, 1, &tmp, 1);
		tmp = 0x05;
		i2c_write(I2C_PCA9557_ADDR1, 1, 1, &tmp, 1);
		sd_ifc_mux = MUX_TYPE_SDHC;
		clrsetbits_be32(&gur->pmuxcr, PMUXCR1_SDHC_MASK,
				PMUXCR1_SDHC_ENABLE);
		break;
	case MUX_TYPE_SPIFLASH:
		out_8(&cpld_data->spi_cs0_sel, MUX_CPLD_SPICS0_FLASH);
		break;
	case MUX_TYPE_TDM:
		out_8(&cpld_data->tdm_can_sel, MUX_CPLD_TDM);
		out_8(&cpld_data->spi_cs0_sel, MUX_CPLD_SPICS0_SLIC);
		break;
	case MUX_TYPE_CAN:
		out_8(&cpld_data->tdm_can_sel, MUX_CPLD_CAN_UART);
		break;
	default:
		break;
	}
#elif defined(CONFIG_TARGET_P1010RDB_PB)
	uint orig_bus = i2c_get_bus_num();
	i2c_set_bus_num(I2C_PCA9557_BUS_NUM);

	switch (ctrl_type) {
	case MUX_TYPE_IFC:
		i2c_read(I2C_PCA9557_ADDR2, 0, 1, &tmp, 1);
		clrbits_8(&tmp, 0x04);
		i2c_write(I2C_PCA9557_ADDR2, 1, 1, &tmp, 1);
		i2c_read(I2C_PCA9557_ADDR2, 3, 1, &tmp, 1);
		clrbits_8(&tmp, 0x04);
		i2c_write(I2C_PCA9557_ADDR2, 3, 1, &tmp, 1);
		sd_ifc_mux = MUX_TYPE_IFC;
		clrbits_be32(&gur->pmuxcr, PMUXCR1_IFC_MASK);
		break;
	case MUX_TYPE_SDHC:
		i2c_read(I2C_PCA9557_ADDR2, 0, 1, &tmp, 1);
		setbits_8(&tmp, 0x04);
		i2c_write(I2C_PCA9557_ADDR2, 1, 1, &tmp, 1);
		i2c_read(I2C_PCA9557_ADDR2, 3, 1, &tmp, 1);
		clrbits_8(&tmp, 0x04);
		i2c_write(I2C_PCA9557_ADDR2, 3, 1, &tmp, 1);
		sd_ifc_mux = MUX_TYPE_SDHC;
		clrsetbits_be32(&gur->pmuxcr, PMUXCR1_SDHC_MASK,
				PMUXCR1_SDHC_ENABLE);
		break;
	case MUX_TYPE_SPIFLASH:
		i2c_read(I2C_PCA9557_ADDR2, 0, 1, &tmp, 1);
		clrbits_8(&tmp, 0x80);
		i2c_write(I2C_PCA9557_ADDR2, 1, 1, &tmp, 1);
		i2c_read(I2C_PCA9557_ADDR2, 3, 1, &tmp, 1);
		clrbits_8(&tmp, 0x80);
		i2c_write(I2C_PCA9557_ADDR2, 3, 1, &tmp, 1);
		break;
	case MUX_TYPE_TDM:
		i2c_read(I2C_PCA9557_ADDR2, 0, 1, &tmp, 1);
		setbits_8(&tmp, 0x82);
		i2c_write(I2C_PCA9557_ADDR2, 1, 1, &tmp, 1);
		i2c_read(I2C_PCA9557_ADDR2, 3, 1, &tmp, 1);
		clrbits_8(&tmp, 0x82);
		i2c_write(I2C_PCA9557_ADDR2, 3, 1, &tmp, 1);
		break;
	case MUX_TYPE_CAN:
		i2c_read(I2C_PCA9557_ADDR2, 0, 1, &tmp, 1);
		clrbits_8(&tmp, 0x02);
		i2c_write(I2C_PCA9557_ADDR2, 1, 1, &tmp, 1);
		i2c_read(I2C_PCA9557_ADDR2, 3, 1, &tmp, 1);
		clrbits_8(&tmp, 0x02);
		i2c_write(I2C_PCA9557_ADDR2, 3, 1, &tmp, 1);
		break;
	case MUX_TYPE_CS0_NOR:
		i2c_read(I2C_PCA9557_ADDR2, 0, 1, &tmp, 1);
		clrbits_8(&tmp, 0x08);
		i2c_write(I2C_PCA9557_ADDR2, 1, 1, &tmp, 1);
		i2c_read(I2C_PCA9557_ADDR2, 3, 1, &tmp, 1);
		clrbits_8(&tmp, 0x08);
		i2c_write(I2C_PCA9557_ADDR2, 3, 1, &tmp, 1);
		break;
	case MUX_TYPE_CS0_NAND:
		i2c_read(I2C_PCA9557_ADDR2, 0, 1, &tmp, 1);
		setbits_8(&tmp, 0x08);
		i2c_write(I2C_PCA9557_ADDR2, 1, 1, &tmp, 1);
		i2c_read(I2C_PCA9557_ADDR2, 3, 1, &tmp, 1);
		clrbits_8(&tmp, 0x08);
		i2c_write(I2C_PCA9557_ADDR2, 3, 1, &tmp, 1);
		break;
	default:
		break;
	}
	i2c_set_bus_num(orig_bus);
#endif
	return 0;
}

#ifdef CONFIG_TARGET_P1010RDB_PB
int i2c_pca9557_read(int type)
{
	u8 val;

	i2c_set_bus_num(I2C_PCA9557_BUS_NUM);
	i2c_read(I2C_PCA9557_ADDR2, 0, 1, &val, 1);

	switch (type) {
	case I2C_READ_BANK:
		val = (val & 0x10) >> 4;
		break;
	case I2C_READ_PCB_VER:
		val = ((val & 0x60) >> 5) + 1;
		break;
	default:
		break;
	}

	return val;
}
#endif

int checkboard(void)
{
	struct cpu_type *cpu;
	struct cpld_data *cpld_data = (void *)(CONFIG_SYS_CPLD_BASE);
	u8 val;

	cpu = gd->arch.cpu;
#if defined(CONFIG_TARGET_P1010RDB_PA)
	printf("Board: %sRDB-PA, ", cpu->name);
#elif defined(CONFIG_TARGET_P1010RDB_PB)
	printf("Board: %sRDB-PB, ", cpu->name);
	i2c_set_bus_num(I2C_PCA9557_BUS_NUM);
	i2c_init(CONFIG_SYS_FSL_I2C_SPEED, CONFIG_SYS_FSL_I2C_SLAVE);
	val = 0x0;  /* no polarity inversion */
	i2c_write(I2C_PCA9557_ADDR2, 2, 1, &val, 1);
#endif

#ifdef CONFIG_SDCARD
	/* switch to IFC to read info from CPLD */
	config_board_mux(MUX_TYPE_IFC);
#endif

#if defined(CONFIG_TARGET_P1010RDB_PA)
	val = (in_8(&cpld_data->pcba_ver) & 0xf);
	printf("PCB: v%x.0\n", val);
#elif defined(CONFIG_TARGET_P1010RDB_PB)
	val = in_8(&cpld_data->cpld_ver);
	printf("CPLD: v%x.%x, ", val >> 4, val & 0xf);
	printf("PCB: v%x.0, ", i2c_pca9557_read(I2C_READ_PCB_VER));
	val = in_8(&cpld_data->rom_loc) & 0xf;
	puts("Boot from: ");
	switch (val) {
	case 0xf:
		config_board_mux(MUX_TYPE_CS0_NOR);
		printf("NOR vBank%d\n", i2c_pca9557_read(I2C_READ_BANK));
		break;
	case 0xe:
		puts("SDHC\n");
		val = 0x60; /* set pca9557 pin input/output */
		i2c_write(I2C_PCA9557_ADDR2, 3, 1, &val, 1);
		break;
	case 0x5:
		config_board_mux(MUX_TYPE_IFC);
		config_board_mux(MUX_TYPE_CS0_NAND);
		puts("NAND\n");
		break;
	case 0x6:
		config_board_mux(MUX_TYPE_IFC);
		puts("SPI\n");
		break;
	default:
		puts("unknown\n");
		break;
	}
#endif
	return 0;
}

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_TSEC_ENET
	struct fsl_pq_mdio_info mdio_info;
	struct tsec_info_struct tsec_info[4];
	struct cpu_type *cpu;
	int num = 0;

	cpu = gd->arch.cpu;

#ifdef CONFIG_TSEC1
	SET_STD_TSEC_INFO(tsec_info[num], 1);
	num++;
#endif
#ifdef CONFIG_TSEC2
	SET_STD_TSEC_INFO(tsec_info[num], 2);
	num++;
#endif
#ifdef CONFIG_TSEC3
	/* P1014 and it's derivatives do not support eTSEC3 */
	if (cpu->soc_ver != SVR_P1014) {
		SET_STD_TSEC_INFO(tsec_info[num], 3);
		num++;
	}
#endif
	if (!num) {
		printf("No TSECs initialized\n");
		return 0;
	}

	mdio_info.regs = (struct tsec_mii_mng *)CONFIG_SYS_MDIO_BASE_ADDR;
	mdio_info.name = DEFAULT_MII_NAME;

	fsl_pq_mdio_init(bis, &mdio_info);

	tsec_eth_init(bis, tsec_info, num);
#endif

	return pci_eth_init(bis);
}

#if defined(CONFIG_OF_BOARD_SETUP)
void fdt_del_flexcan(void *blob)
{
	int nodeoff = 0;

	while ((nodeoff = fdt_node_offset_by_compatible(blob, 0,
				"fsl,p1010-flexcan")) >= 0) {
		fdt_del_node(blob, nodeoff);
	}
}

void fdt_del_spi_flash(void *blob)
{
	int nodeoff = 0;

	while ((nodeoff = fdt_node_offset_by_compatible(blob, 0,
				"spansion,s25sl12801")) >= 0) {
		fdt_del_node(blob, nodeoff);
	}
}

void fdt_del_spi_slic(void *blob)
{
	int nodeoff = 0;

	while ((nodeoff = fdt_node_offset_by_compatible(blob, 0,
				"zarlink,le88266")) >= 0) {
		fdt_del_node(blob, nodeoff);
	}
}

void fdt_del_tdm(void *blob)
{
	int nodeoff = 0;

	while ((nodeoff = fdt_node_offset_by_compatible(blob, 0,
				"fsl,starlite-tdm")) >= 0) {
		fdt_del_node(blob, nodeoff);
	}
}

void fdt_del_sdhc(void *blob)
{
	int nodeoff = 0;

	while ((nodeoff = fdt_node_offset_by_compatible(blob, 0,
			"fsl,esdhc")) >= 0) {
		fdt_del_node(blob, nodeoff);
	}
}

void fdt_del_ifc(void *blob)
{
	int nodeoff = 0;

	while ((nodeoff = fdt_node_offset_by_compatible(blob, 0,
				"fsl,ifc")) >= 0) {
		fdt_del_node(blob, nodeoff);
	}
}

void fdt_disable_uart1(void *blob)
{
	int nodeoff;

	nodeoff = fdt_node_offset_by_compat_reg(blob, "fsl,ns16550",
					CONFIG_SYS_NS16550_COM2);

	if (nodeoff > 0) {
		fdt_status_disabled(blob, nodeoff);
	} else {
		printf("WARNING unable to set status for fsl,ns16550 "
			"uart1: %s\n", fdt_strerror(nodeoff));
	}
}

int ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;
	struct cpu_type *cpu;

	cpu = gd->arch.cpu;

	ft_cpu_setup(blob, bd);

	base = env_get_bootm_low();
	size = env_get_bootm_size();

#if defined(CONFIG_PCI)
	FT_FSL_PCI_SETUP;
#endif

	fdt_fixup_memory(blob, (u64)base, (u64)size);

#if defined(CONFIG_HAS_FSL_DR_USB)
	fsl_fdt_fixup_dr_usb(blob, bd);
#endif

       /* P1014 and it's derivatives don't support CAN and eTSEC3 */
	if (cpu->soc_ver == SVR_P1014) {
		fdt_del_flexcan(blob);
		fdt_del_node_and_alias(blob, "ethernet2");
	}

	/* Delete IFC node as IFC pins are multiplexing with SDHC */
	if (sd_ifc_mux != MUX_TYPE_IFC)
		fdt_del_ifc(blob);
	else
		fdt_del_sdhc(blob);

	if (hwconfig_subarg_cmp("fsl_p1010mux", "tdm_can", "can")) {
		fdt_del_tdm(blob);
		fdt_del_spi_slic(blob);
	} else if (hwconfig_subarg_cmp("fsl_p1010mux", "tdm_can", "tdm")) {
		fdt_del_flexcan(blob);
		fdt_del_spi_flash(blob);
		fdt_disable_uart1(blob);
	} else {
		/*
		 * If we don't set fsl_p1010mux:tdm_can to "can" or "tdm"
		 * explicitly, defaultly spi_cs_sel to spi-flash instead of
		 * to tdm/slic.
		 */
		fdt_del_tdm(blob);
		fdt_del_flexcan(blob);
		fdt_disable_uart1(blob);
	}

	return 0;
}
#endif

#ifdef CONFIG_SDCARD
int board_mmc_init(bd_t *bis)
{
	config_board_mux(MUX_TYPE_SDHC);
		return -1;
}
#else
void board_reset(void)
{
	/* mux to IFC to enable CPLD for reset */
	if (sd_ifc_mux != MUX_TYPE_IFC)
		config_board_mux(MUX_TYPE_IFC);
}
#endif


int misc_init_r(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	if (hwconfig_subarg_cmp("fsl_p1010mux", "tdm_can", "can")) {
		clrbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_CAN1_TDM |
				MPC85xx_PMUXCR_CAN1_UART |
				MPC85xx_PMUXCR_CAN2_TDM |
				MPC85xx_PMUXCR_CAN2_UART);
		config_board_mux(MUX_TYPE_CAN);
	} else if (hwconfig_subarg_cmp("fsl_p1010mux", "tdm_can", "tdm")) {
		clrbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_CAN2_UART |
				MPC85xx_PMUXCR_CAN1_UART);
		setbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_CAN2_TDM |
				MPC85xx_PMUXCR_CAN1_TDM);
		clrbits_be32(&gur->pmuxcr2, MPC85xx_PMUXCR2_UART_GPIO);
		setbits_be32(&gur->pmuxcr2, MPC85xx_PMUXCR2_UART_TDM);
		config_board_mux(MUX_TYPE_TDM);
	} else {
		/* defaultly spi_cs_sel to flash */
		config_board_mux(MUX_TYPE_SPIFLASH);
	}

	if (hwconfig("esdhc"))
		config_board_mux(MUX_TYPE_SDHC);
	else if (hwconfig("ifc"))
		config_board_mux(MUX_TYPE_IFC);

#ifdef CONFIG_TARGET_P1010RDB_PB
	setbits_be32(&gur->pmuxcr2, MPC85xx_PMUXCR2_GPIO01_DRVVBUS);
#endif
	return 0;
}

#ifndef CONFIG_SPL_BUILD
static int pin_mux_cmd(cmd_tbl_t *cmdtp, int flag, int argc,
				char * const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;
	if (strcmp(argv[1], "ifc") == 0)
		config_board_mux(MUX_TYPE_IFC);
	else if (strcmp(argv[1], "sdhc") == 0)
		config_board_mux(MUX_TYPE_SDHC);
	else
		return CMD_RET_USAGE;
	return 0;
}

U_BOOT_CMD(
	mux, 2, 0, pin_mux_cmd,
	"configure multiplexing pin for IFC/SDHC bus in runtime",
	"bus_type (e.g. mux sdhc)"
);
#endif
