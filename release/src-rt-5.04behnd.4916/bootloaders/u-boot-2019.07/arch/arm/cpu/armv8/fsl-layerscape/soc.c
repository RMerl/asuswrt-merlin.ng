// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014-2015 Freescale Semiconductor
 * Copyright 2019 NXP
 */

#include <common.h>
#include <fsl_immap.h>
#include <fsl_ifc.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>
#include <asm/io.h>
#include <asm/global_data.h>
#include <asm/arch-fsl-layerscape/config.h>
#include <asm/arch-fsl-layerscape/ns_access.h>
#include <asm/arch-fsl-layerscape/fsl_icid.h>
#ifdef CONFIG_LAYERSCAPE_NS_ACCESS
#include <fsl_csu.h>
#endif
#ifdef CONFIG_SYS_FSL_DDR
#include <fsl_ddr_sdram.h>
#include <fsl_ddr.h>
#endif
#ifdef CONFIG_CHAIN_OF_TRUST
#include <fsl_validate.h>
#endif
#include <fsl_immap.h>
#ifdef CONFIG_TFABOOT
#include <environment.h>
DECLARE_GLOBAL_DATA_PTR;
#endif

bool soc_has_dp_ddr(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 svr = gur_in32(&gur->svr);

	/* LS2085A, LS2088A, LS2048A has DP_DDR */
	if ((SVR_SOC_VER(svr) == SVR_LS2085A) ||
	    (SVR_SOC_VER(svr) == SVR_LS2088A) ||
	    (SVR_SOC_VER(svr) == SVR_LS2048A))
		return true;

	return false;
}

bool soc_has_aiop(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 svr = gur_in32(&gur->svr);

	/* LS2085A has AIOP */
	if (SVR_SOC_VER(svr) == SVR_LS2085A)
		return true;

	return false;
}

static inline void set_usb_txvreftune(u32 __iomem *scfg, u32 offset)
{
	scfg_clrsetbits32(scfg + offset / 4,
			0xF << 6,
			SCFG_USB_TXVREFTUNE << 6);
}

static void erratum_a009008(void)
{
#ifdef CONFIG_SYS_FSL_ERRATUM_A009008
	u32 __iomem *scfg = (u32 __iomem *)SCFG_BASE;

#if defined(CONFIG_ARCH_LS1043A) || defined(CONFIG_ARCH_LS1046A) || \
	defined(CONFIG_ARCH_LS1012A)
	set_usb_txvreftune(scfg, SCFG_USB3PRM1CR_USB1);
#if defined(CONFIG_ARCH_LS1043A) || defined(CONFIG_ARCH_LS1046A)
	set_usb_txvreftune(scfg, SCFG_USB3PRM1CR_USB2);
	set_usb_txvreftune(scfg, SCFG_USB3PRM1CR_USB3);
#endif
#elif defined(CONFIG_ARCH_LS2080A)
	set_usb_txvreftune(scfg, SCFG_USB3PRM1CR);
#endif
#endif /* CONFIG_SYS_FSL_ERRATUM_A009008 */
}

static inline void set_usb_sqrxtune(u32 __iomem *scfg, u32 offset)
{
	scfg_clrbits32(scfg + offset / 4,
			SCFG_USB_SQRXTUNE_MASK << 23);
}

static void erratum_a009798(void)
{
#ifdef CONFIG_SYS_FSL_ERRATUM_A009798
	u32 __iomem *scfg = (u32 __iomem *)SCFG_BASE;

#if defined(CONFIG_ARCH_LS1043A) || defined(CONFIG_ARCH_LS1046A) || \
	defined(CONFIG_ARCH_LS1012A)
	set_usb_sqrxtune(scfg, SCFG_USB3PRM1CR_USB1);
#if defined(CONFIG_ARCH_LS1043A) || defined(CONFIG_ARCH_LS1046A)
	set_usb_sqrxtune(scfg, SCFG_USB3PRM1CR_USB2);
	set_usb_sqrxtune(scfg, SCFG_USB3PRM1CR_USB3);
#endif
#elif defined(CONFIG_ARCH_LS2080A)
	set_usb_sqrxtune(scfg, SCFG_USB3PRM1CR);
#endif
#endif /* CONFIG_SYS_FSL_ERRATUM_A009798 */
}

#if defined(CONFIG_ARCH_LS1043A) || defined(CONFIG_ARCH_LS1046A) || \
	defined(CONFIG_ARCH_LS1012A)
static inline void set_usb_pcstxswingfull(u32 __iomem *scfg, u32 offset)
{
	scfg_clrsetbits32(scfg + offset / 4,
			0x7F << 9,
			SCFG_USB_PCSTXSWINGFULL << 9);
}
#endif

static void erratum_a008997(void)
{
#ifdef CONFIG_SYS_FSL_ERRATUM_A008997
#if defined(CONFIG_ARCH_LS1043A) || defined(CONFIG_ARCH_LS1046A) || \
	defined(CONFIG_ARCH_LS1012A)
	u32 __iomem *scfg = (u32 __iomem *)SCFG_BASE;

	set_usb_pcstxswingfull(scfg, SCFG_USB3PRM2CR_USB1);
#if defined(CONFIG_ARCH_LS1043A) || defined(CONFIG_ARCH_LS1046A)
	set_usb_pcstxswingfull(scfg, SCFG_USB3PRM2CR_USB2);
	set_usb_pcstxswingfull(scfg, SCFG_USB3PRM2CR_USB3);
#endif
#elif defined(CONFIG_ARCH_LS1028A)
	clrsetbits_le32(DCSR_BASE +  DCSR_USB_IOCR1,
			0x7F << 11,
			DCSR_USB_PCSTXSWINGFULL << 11);
#endif
#endif /* CONFIG_SYS_FSL_ERRATUM_A008997 */
}

#if defined(CONFIG_ARCH_LS1043A) || defined(CONFIG_ARCH_LS1046A) || \
	defined(CONFIG_ARCH_LS1012A)

#define PROGRAM_USB_PHY_RX_OVRD_IN_HI(phy)	\
	out_be16((phy) + SCFG_USB_PHY_RX_OVRD_IN_HI, USB_PHY_RX_EQ_VAL_1);	\
	out_be16((phy) + SCFG_USB_PHY_RX_OVRD_IN_HI, USB_PHY_RX_EQ_VAL_2);	\
	out_be16((phy) + SCFG_USB_PHY_RX_OVRD_IN_HI, USB_PHY_RX_EQ_VAL_3);	\
	out_be16((phy) + SCFG_USB_PHY_RX_OVRD_IN_HI, USB_PHY_RX_EQ_VAL_4)

#elif defined(CONFIG_ARCH_LS2080A) || defined(CONFIG_ARCH_LS1088A) || \
	defined(CONFIG_ARCH_LS1028A)

#define PROGRAM_USB_PHY_RX_OVRD_IN_HI(phy)	\
	out_le16((phy) + DCSR_USB_PHY_RX_OVRD_IN_HI, USB_PHY_RX_EQ_VAL_1); \
	out_le16((phy) + DCSR_USB_PHY_RX_OVRD_IN_HI, USB_PHY_RX_EQ_VAL_2); \
	out_le16((phy) + DCSR_USB_PHY_RX_OVRD_IN_HI, USB_PHY_RX_EQ_VAL_3); \
	out_le16((phy) + DCSR_USB_PHY_RX_OVRD_IN_HI, USB_PHY_RX_EQ_VAL_4)

#endif

static void erratum_a009007(void)
{
#if defined(CONFIG_ARCH_LS1043A) || defined(CONFIG_ARCH_LS1046A) || \
	defined(CONFIG_ARCH_LS1012A)
	void __iomem *usb_phy = (void __iomem *)SCFG_USB_PHY1;

	PROGRAM_USB_PHY_RX_OVRD_IN_HI(usb_phy);
#if defined(CONFIG_ARCH_LS1043A) || defined(CONFIG_ARCH_LS1046A)
	usb_phy = (void __iomem *)SCFG_USB_PHY2;
	PROGRAM_USB_PHY_RX_OVRD_IN_HI(usb_phy);

	usb_phy = (void __iomem *)SCFG_USB_PHY3;
	PROGRAM_USB_PHY_RX_OVRD_IN_HI(usb_phy);
#endif
#elif defined(CONFIG_ARCH_LS2080A) || defined(CONFIG_ARCH_LS1088A) || \
	defined(CONFIG_ARCH_LS1028A)
	void __iomem *dcsr = (void __iomem *)DCSR_BASE;

	PROGRAM_USB_PHY_RX_OVRD_IN_HI(dcsr + DCSR_USB_PHY1);
	PROGRAM_USB_PHY_RX_OVRD_IN_HI(dcsr + DCSR_USB_PHY2);
#endif /* CONFIG_SYS_FSL_ERRATUM_A009007 */
}

#if defined(CONFIG_FSL_LSCH3)
/*
 * This erratum requires setting a value to eddrtqcr1 to
 * optimal the DDR performance.
 */
static void erratum_a008336(void)
{
#ifdef CONFIG_SYS_FSL_ERRATUM_A008336
	u32 *eddrtqcr1;

#ifdef CONFIG_SYS_FSL_DCSR_DDR_ADDR
	eddrtqcr1 = (void *)CONFIG_SYS_FSL_DCSR_DDR_ADDR + 0x800;
	if (fsl_ddr_get_version(0) == 0x50200)
		out_le32(eddrtqcr1, 0x63b30002);
#endif
#ifdef CONFIG_SYS_FSL_DCSR_DDR2_ADDR
	eddrtqcr1 = (void *)CONFIG_SYS_FSL_DCSR_DDR2_ADDR + 0x800;
	if (fsl_ddr_get_version(0) == 0x50200)
		out_le32(eddrtqcr1, 0x63b30002);
#endif
#endif
}

/*
 * This erratum requires a register write before being Memory
 * controller 3 being enabled.
 */
static void erratum_a008514(void)
{
#ifdef CONFIG_SYS_FSL_ERRATUM_A008514
	u32 *eddrtqcr1;

#ifdef CONFIG_SYS_FSL_DCSR_DDR3_ADDR
	eddrtqcr1 = (void *)CONFIG_SYS_FSL_DCSR_DDR3_ADDR + 0x800;
	out_le32(eddrtqcr1, 0x63b20002);
#endif
#endif
}
#ifdef CONFIG_SYS_FSL_ERRATUM_A009635
#define PLATFORM_CYCLE_ENV_VAR	"a009635_interval_val"

static unsigned long get_internval_val_mhz(void)
{
	char *interval = env_get(PLATFORM_CYCLE_ENV_VAR);
	/*
	 *  interval is the number of platform cycles(MHz) between
	 *  wake up events generated by EPU.
	 */
	ulong interval_mhz = get_bus_freq(0) / (1000 * 1000);

	if (interval)
		interval_mhz = simple_strtoul(interval, NULL, 10);

	return interval_mhz;
}

void erratum_a009635(void)
{
	u32 val;
	unsigned long interval_mhz = get_internval_val_mhz();

	if (!interval_mhz)
		return;

	val = in_le32(DCSR_CGACRE5);
	writel(val | 0x00000200, DCSR_CGACRE5);

	val = in_le32(EPU_EPCMPR5);
	writel(interval_mhz, EPU_EPCMPR5);
	val = in_le32(EPU_EPCCR5);
	writel(val | 0x82820000, EPU_EPCCR5);
	val = in_le32(EPU_EPSMCR5);
	writel(val | 0x002f0000, EPU_EPSMCR5);
	val = in_le32(EPU_EPECR5);
	writel(val | 0x20000000, EPU_EPECR5);
	val = in_le32(EPU_EPGCR);
	writel(val | 0x80000000, EPU_EPGCR);
}
#endif	/* CONFIG_SYS_FSL_ERRATUM_A009635 */

static void erratum_rcw_src(void)
{
#if defined(CONFIG_SPL) && defined(CONFIG_NAND_BOOT)
	u32 __iomem *dcfg_ccsr = (u32 __iomem *)DCFG_BASE;
	u32 __iomem *dcfg_dcsr = (u32 __iomem *)DCFG_DCSR_BASE;
	u32 val;

	val = in_le32(dcfg_ccsr + DCFG_PORSR1 / 4);
	val &= ~DCFG_PORSR1_RCW_SRC;
	val |= DCFG_PORSR1_RCW_SRC_NOR;
	out_le32(dcfg_dcsr + DCFG_DCSR_PORCR1 / 4, val);
#endif
}

#define I2C_DEBUG_REG 0x6
#define I2C_GLITCH_EN 0x8
/*
 * This erratum requires setting glitch_en bit to enable
 * digital glitch filter to improve clock stability.
 */
#ifdef CONFIG_SYS_FSL_ERRATUM_A009203
static void erratum_a009203(void)
{
#ifdef CONFIG_SYS_I2C
	u8 __iomem *ptr;
#ifdef I2C1_BASE_ADDR
	ptr = (u8 __iomem *)(I2C1_BASE_ADDR + I2C_DEBUG_REG);

	writeb(I2C_GLITCH_EN, ptr);
#endif
#ifdef I2C2_BASE_ADDR
	ptr = (u8 __iomem *)(I2C2_BASE_ADDR + I2C_DEBUG_REG);

	writeb(I2C_GLITCH_EN, ptr);
#endif
#ifdef I2C3_BASE_ADDR
	ptr = (u8 __iomem *)(I2C3_BASE_ADDR + I2C_DEBUG_REG);

	writeb(I2C_GLITCH_EN, ptr);
#endif
#ifdef I2C4_BASE_ADDR
	ptr = (u8 __iomem *)(I2C4_BASE_ADDR + I2C_DEBUG_REG);

	writeb(I2C_GLITCH_EN, ptr);
#endif
#endif
}
#endif

void bypass_smmu(void)
{
	u32 val;
	val = (in_le32(SMMU_SCR0) | SCR0_CLIENTPD_MASK) & ~(SCR0_USFCFG_MASK);
	out_le32(SMMU_SCR0, val);
	val = (in_le32(SMMU_NSCR0) | SCR0_CLIENTPD_MASK) & ~(SCR0_USFCFG_MASK);
	out_le32(SMMU_NSCR0, val);
}
void fsl_lsch3_early_init_f(void)
{
	erratum_rcw_src();
#ifdef CONFIG_FSL_IFC
	init_early_memctl_regs();	/* tighten IFC timing */
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A009203
	erratum_a009203();
#endif
	erratum_a008514();
	erratum_a008336();
	erratum_a009008();
	erratum_a009798();
	erratum_a008997();
	erratum_a009007();
#ifdef CONFIG_CHAIN_OF_TRUST
	/* In case of Secure Boot, the IBR configures the SMMU
	* to allow only Secure transactions.
	* SMMU must be reset in bypass mode.
	* Set the ClientPD bit and Clear the USFCFG Bit
	*/
	if (fsl_check_boot_mode_secure() == 1)
		bypass_smmu();
#endif
}

/* Get VDD in the unit mV from voltage ID */
int get_core_volt_from_fuse(void)
{
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	int vdd;
	u32 fusesr;
	u8 vid;

	/* get the voltage ID from fuse status register */
	fusesr = in_le32(&gur->dcfg_fusesr);
	debug("%s: fusesr = 0x%x\n", __func__, fusesr);
	vid = (fusesr >> FSL_CHASSIS3_DCFG_FUSESR_ALTVID_SHIFT) &
		FSL_CHASSIS3_DCFG_FUSESR_ALTVID_MASK;
	if ((vid == 0) || (vid == FSL_CHASSIS3_DCFG_FUSESR_ALTVID_MASK)) {
		vid = (fusesr >> FSL_CHASSIS3_DCFG_FUSESR_VID_SHIFT) &
			FSL_CHASSIS3_DCFG_FUSESR_VID_MASK;
	}
	debug("%s: VID = 0x%x\n", __func__, vid);
	switch (vid) {
	case 0x00: /* VID isn't supported */
		vdd = -EINVAL;
		debug("%s: The VID feature is not supported\n", __func__);
		break;
	case 0x08: /* 0.9V silicon */
		vdd = 900;
		break;
	case 0x10: /* 1.0V silicon */
		vdd = 1000;
		break;
	default:  /* Other core voltage */
		vdd = -EINVAL;
		debug("%s: The VID(%x) isn't supported\n", __func__, vid);
		break;
	}
	debug("%s: The required minimum volt of CORE is %dmV\n", __func__, vdd);

	return vdd;
}

#elif defined(CONFIG_FSL_LSCH2)

static void erratum_a009929(void)
{
#ifdef CONFIG_SYS_FSL_ERRATUM_A009929
	struct ccsr_gur *gur = (void *)CONFIG_SYS_FSL_GUTS_ADDR;
	u32 __iomem *dcsr_cop_ccp = (void *)CONFIG_SYS_DCSR_COP_CCP_ADDR;
	u32 rstrqmr1 = gur_in32(&gur->rstrqmr1);

	rstrqmr1 |= 0x00000400;
	gur_out32(&gur->rstrqmr1, rstrqmr1);
	writel(0x01000000, dcsr_cop_ccp);
#endif
}

/*
 * This erratum requires setting a value to eddrtqcr1 to optimal
 * the DDR performance. The eddrtqcr1 register is in SCFG space
 * of LS1043A and the offset is 0x157_020c.
 */
#if defined(CONFIG_SYS_FSL_ERRATUM_A009660) \
	&& defined(CONFIG_SYS_FSL_ERRATUM_A008514)
#error A009660 and A008514 can not be both enabled.
#endif

static void erratum_a009660(void)
{
#ifdef CONFIG_SYS_FSL_ERRATUM_A009660
	u32 *eddrtqcr1 = (void *)CONFIG_SYS_FSL_SCFG_ADDR + 0x20c;
	out_be32(eddrtqcr1, 0x63b20042);
#endif
}

static void erratum_a008850_early(void)
{
#ifdef CONFIG_SYS_FSL_ERRATUM_A008850
	/* part 1 of 2 */
	struct ccsr_cci400 __iomem *cci = (void *)(CONFIG_SYS_IMMR +
						CONFIG_SYS_CCI400_OFFSET);
	struct ccsr_ddr __iomem *ddr = (void *)CONFIG_SYS_FSL_DDR_ADDR;

	/* Skip if running at lower exception level */
	if (current_el() < 3)
		return;

	/* disables propagation of barrier transactions to DDRC from CCI400 */
	out_le32(&cci->ctrl_ord, CCI400_CTRLORD_TERM_BARRIER);

	/* disable the re-ordering in DDRC */
	ddr_out32(&ddr->eor, DDR_EOR_RD_REOD_DIS | DDR_EOR_WD_REOD_DIS);
#endif
}

void erratum_a008850_post(void)
{
#ifdef CONFIG_SYS_FSL_ERRATUM_A008850
	/* part 2 of 2 */
	struct ccsr_cci400 __iomem *cci = (void *)(CONFIG_SYS_IMMR +
						CONFIG_SYS_CCI400_OFFSET);
	struct ccsr_ddr __iomem *ddr = (void *)CONFIG_SYS_FSL_DDR_ADDR;
	u32 tmp;

	/* Skip if running at lower exception level */
	if (current_el() < 3)
		return;

	/* enable propagation of barrier transactions to DDRC from CCI400 */
	out_le32(&cci->ctrl_ord, CCI400_CTRLORD_EN_BARRIER);

	/* enable the re-ordering in DDRC */
	tmp = ddr_in32(&ddr->eor);
	tmp &= ~(DDR_EOR_RD_REOD_DIS | DDR_EOR_WD_REOD_DIS);
	ddr_out32(&ddr->eor, tmp);
#endif
}

#ifdef CONFIG_SYS_FSL_ERRATUM_A010315
void erratum_a010315(void)
{
	int i;

	for (i = PCIE1; i <= PCIE4; i++)
		if (!is_serdes_configured(i)) {
			debug("PCIe%d: disabled all R/W permission!\n", i);
			set_pcie_ns_access(i, 0);
		}
}
#endif

static void erratum_a010539(void)
{
#if defined(CONFIG_SYS_FSL_ERRATUM_A010539) && defined(CONFIG_QSPI_BOOT)
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 porsr1;

	porsr1 = in_be32(&gur->porsr1);
	porsr1 &= ~FSL_CHASSIS2_CCSR_PORSR1_RCW_MASK;
	out_be32((void *)(CONFIG_SYS_DCSR_DCFG_ADDR + DCFG_DCSR_PORCR1),
		 porsr1);
	out_be32((void *)(CONFIG_SYS_FSL_SCFG_ADDR + 0x1a8), 0xffffffff);
#endif
}

/* Get VDD in the unit mV from voltage ID */
int get_core_volt_from_fuse(void)
{
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	int vdd;
	u32 fusesr;
	u8 vid;

	fusesr = in_be32(&gur->dcfg_fusesr);
	debug("%s: fusesr = 0x%x\n", __func__, fusesr);
	vid = (fusesr >> FSL_CHASSIS2_DCFG_FUSESR_ALTVID_SHIFT) &
		FSL_CHASSIS2_DCFG_FUSESR_ALTVID_MASK;
	if ((vid == 0) || (vid == FSL_CHASSIS2_DCFG_FUSESR_ALTVID_MASK)) {
		vid = (fusesr >> FSL_CHASSIS2_DCFG_FUSESR_VID_SHIFT) &
			FSL_CHASSIS2_DCFG_FUSESR_VID_MASK;
	}
	debug("%s: VID = 0x%x\n", __func__, vid);
	switch (vid) {
	case 0x00: /* VID isn't supported */
		vdd = -EINVAL;
		debug("%s: The VID feature is not supported\n", __func__);
		break;
	case 0x08: /* 0.9V silicon */
		vdd = 900;
		break;
	case 0x10: /* 1.0V silicon */
		vdd = 1000;
		break;
	default:  /* Other core voltage */
		vdd = -EINVAL;
		printf("%s: The VID(%x) isn't supported\n", __func__, vid);
		break;
	}
	debug("%s: The required minimum volt of CORE is %dmV\n", __func__, vdd);

	return vdd;
}

__weak int board_switch_core_volt(u32 vdd)
{
	return 0;
}

static int setup_core_volt(u32 vdd)
{
	return board_setup_core_volt(vdd);
}

#ifdef CONFIG_SYS_FSL_DDR
static void ddr_enable_0v9_volt(bool en)
{
	struct ccsr_ddr __iomem *ddr = (void *)CONFIG_SYS_FSL_DDR_ADDR;
	u32 tmp;

	tmp = ddr_in32(&ddr->ddr_cdr1);

	if (en)
		tmp |= DDR_CDR1_V0PT9_EN;
	else
		tmp &= ~DDR_CDR1_V0PT9_EN;

	ddr_out32(&ddr->ddr_cdr1, tmp);
}
#endif

int setup_chip_volt(void)
{
	int vdd;

	vdd = get_core_volt_from_fuse();
	/* Nothing to do for silicons doesn't support VID */
	if (vdd < 0)
		return vdd;

	if (setup_core_volt(vdd))
		printf("%s: Switch core VDD to %dmV failed\n", __func__, vdd);
#ifdef CONFIG_SYS_HAS_SERDES
	if (setup_serdes_volt(vdd))
		printf("%s: Switch SVDD to %dmV failed\n", __func__, vdd);
#endif

#ifdef CONFIG_SYS_FSL_DDR
	if (vdd == 900)
		ddr_enable_0v9_volt(true);
#endif

	return 0;
}

#ifdef CONFIG_FSL_PFE
void init_pfe_scfg_dcfg_regs(void)
{
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;
	u32 ecccr2;

	out_be32(&scfg->pfeasbcr,
		 in_be32(&scfg->pfeasbcr) | SCFG_PFEASBCR_AWCACHE0);
	out_be32(&scfg->pfebsbcr,
		 in_be32(&scfg->pfebsbcr) | SCFG_PFEASBCR_AWCACHE0);

	/* CCI-400 QoS settings for PFE */
	out_be32(&scfg->wr_qos1, (unsigned int)(SCFG_WR_QOS1_PFE1_QOS
		 | SCFG_WR_QOS1_PFE2_QOS));
	out_be32(&scfg->rd_qos1, (unsigned int)(SCFG_RD_QOS1_PFE1_QOS
		 | SCFG_RD_QOS1_PFE2_QOS));

	ecccr2 = in_be32(CONFIG_SYS_DCSR_DCFG_ADDR + DCFG_DCSR_ECCCR2);
	out_be32((void *)CONFIG_SYS_DCSR_DCFG_ADDR + DCFG_DCSR_ECCCR2,
		 ecccr2 | (unsigned int)DISABLE_PFE_ECC);
}
#endif

void fsl_lsch2_early_init_f(void)
{
	struct ccsr_cci400 *cci = (struct ccsr_cci400 *)(CONFIG_SYS_IMMR +
					CONFIG_SYS_CCI400_OFFSET);
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;
#if defined(CONFIG_FSL_QSPI) && defined(CONFIG_TFABOOT)
	enum boot_src src;
#endif

#ifdef CONFIG_LAYERSCAPE_NS_ACCESS
	enable_layerscape_ns_access();
#endif

#ifdef CONFIG_FSL_IFC
	init_early_memctl_regs();	/* tighten IFC timing */
#endif

#if defined(CONFIG_FSL_QSPI) && defined(CONFIG_TFABOOT)
	src = get_boot_src();
	if (src != BOOT_SOURCE_QSPI_NOR)
		out_be32(&scfg->qspi_cfg, SCFG_QSPI_CLKSEL);
#else
#if defined(CONFIG_FSL_QSPI) && !defined(CONFIG_QSPI_BOOT)
	out_be32(&scfg->qspi_cfg, SCFG_QSPI_CLKSEL);
#endif
#endif
	/* Make SEC reads and writes snoopable */
	setbits_be32(&scfg->snpcnfgcr, SCFG_SNPCNFGCR_SECRDSNP |
		     SCFG_SNPCNFGCR_SECWRSNP |
		     SCFG_SNPCNFGCR_SATARDSNP |
		     SCFG_SNPCNFGCR_SATAWRSNP);

	/*
	 * Enable snoop requests and DVM message requests for
	 * Slave insterface S4 (A53 core cluster)
	 */
	if (current_el() == 3) {
		out_le32(&cci->slave[4].snoop_ctrl,
			 CCI400_DVM_MESSAGE_REQ_EN | CCI400_SNOOP_REQ_EN);
	}

	/*
	 * Program Central Security Unit (CSU) to grant access
	 * permission for USB 2.0 controller
	 */
#if defined(CONFIG_ARCH_LS1012A) && defined(CONFIG_USB_EHCI_FSL)
	if (current_el() == 3)
		set_devices_ns_access(CSU_CSLX_USB_2, CSU_ALL_RW);
#endif
	/* Erratum */
	erratum_a008850_early(); /* part 1 of 2 */
	erratum_a009929();
	erratum_a009660();
	erratum_a010539();
	erratum_a009008();
	erratum_a009798();
	erratum_a008997();
	erratum_a009007();

#if defined(CONFIG_ARCH_LS1043A) || defined(CONFIG_ARCH_LS1046A)
	set_icids();
#endif
}
#endif

#ifdef CONFIG_QSPI_AHB_INIT
/* Enable 4bytes address support and fast read */
int qspi_ahb_init(void)
{
	u32 *qspi_lut, lut_key, *qspi_key;

	qspi_key = (void *)SYS_FSL_QSPI_ADDR + 0x300;
	qspi_lut = (void *)SYS_FSL_QSPI_ADDR + 0x310;

	lut_key = in_be32(qspi_key);

	if (lut_key == 0x5af05af0) {
		/* That means the register is BE */
		out_be32(qspi_key, 0x5af05af0);
		/* Unlock the lut table */
		out_be32(qspi_key + 1, 0x00000002);
		out_be32(qspi_lut, 0x0820040c);
		out_be32(qspi_lut + 1, 0x1c080c08);
		out_be32(qspi_lut + 2, 0x00002400);
		/* Lock the lut table */
		out_be32(qspi_key, 0x5af05af0);
		out_be32(qspi_key + 1, 0x00000001);
	} else {
		/* That means the register is LE */
		out_le32(qspi_key, 0x5af05af0);
		/* Unlock the lut table */
		out_le32(qspi_key + 1, 0x00000002);
		out_le32(qspi_lut, 0x0820040c);
		out_le32(qspi_lut + 1, 0x1c080c08);
		out_le32(qspi_lut + 2, 0x00002400);
		/* Lock the lut table */
		out_le32(qspi_key, 0x5af05af0);
		out_le32(qspi_key + 1, 0x00000001);
	}

	return 0;
}
#endif

#ifdef CONFIG_TFABOOT
#define MAX_BOOTCMD_SIZE	512

int fsl_setenv_bootcmd(void)
{
	int ret;
	enum boot_src src = get_boot_src();
	char bootcmd_str[MAX_BOOTCMD_SIZE];

	switch (src) {
#ifdef IFC_NOR_BOOTCOMMAND
	case BOOT_SOURCE_IFC_NOR:
		sprintf(bootcmd_str, IFC_NOR_BOOTCOMMAND);
		break;
#endif
#ifdef QSPI_NOR_BOOTCOMMAND
	case BOOT_SOURCE_QSPI_NOR:
		sprintf(bootcmd_str, QSPI_NOR_BOOTCOMMAND);
		break;
#endif
#ifdef XSPI_NOR_BOOTCOMMAND
	case BOOT_SOURCE_XSPI_NOR:
		sprintf(bootcmd_str, XSPI_NOR_BOOTCOMMAND);
		break;
#endif
#ifdef IFC_NAND_BOOTCOMMAND
	case BOOT_SOURCE_IFC_NAND:
		sprintf(bootcmd_str, IFC_NAND_BOOTCOMMAND);
		break;
#endif
#ifdef QSPI_NAND_BOOTCOMMAND
	case BOOT_SOURCE_QSPI_NAND:
		sprintf(bootcmd_str, QSPI_NAND_BOOTCOMMAND);
		break;
#endif
#ifdef XSPI_NAND_BOOTCOMMAND
	case BOOT_SOURCE_XSPI_NAND:
		sprintf(bootcmd_str, XSPI_NAND_BOOTCOMMAND);
		break;
#endif
#ifdef SD_BOOTCOMMAND
	case BOOT_SOURCE_SD_MMC:
		sprintf(bootcmd_str, SD_BOOTCOMMAND);
		break;
#endif
#ifdef SD2_BOOTCOMMAND
	case BOOT_SOURCE_SD_MMC2:
		sprintf(bootcmd_str, SD2_BOOTCOMMAND);
		break;
#endif
	default:
#ifdef QSPI_NOR_BOOTCOMMAND
		sprintf(bootcmd_str, QSPI_NOR_BOOTCOMMAND);
#endif
		break;
	}

	ret = env_set("bootcmd", bootcmd_str);
	if (ret) {
		printf("Failed to set bootcmd: ret = %d\n", ret);
		return ret;
	}
	return 0;
}

int fsl_setenv_mcinitcmd(void)
{
	int ret = 0;
	enum boot_src src = get_boot_src();

	switch (src) {
#ifdef IFC_MC_INIT_CMD
	case BOOT_SOURCE_IFC_NAND:
	case BOOT_SOURCE_IFC_NOR:
	ret = env_set("mcinitcmd", IFC_MC_INIT_CMD);
		break;
#endif
#ifdef QSPI_MC_INIT_CMD
	case BOOT_SOURCE_QSPI_NAND:
	case BOOT_SOURCE_QSPI_NOR:
	ret = env_set("mcinitcmd", QSPI_MC_INIT_CMD);
		break;
#endif
#ifdef XSPI_MC_INIT_CMD
	case BOOT_SOURCE_XSPI_NAND:
	case BOOT_SOURCE_XSPI_NOR:
	ret = env_set("mcinitcmd", XSPI_MC_INIT_CMD);
		break;
#endif
#ifdef SD_MC_INIT_CMD
	case BOOT_SOURCE_SD_MMC:
	ret = env_set("mcinitcmd", SD_MC_INIT_CMD);
		break;
#endif
#ifdef SD2_MC_INIT_CMD
	case BOOT_SOURCE_SD_MMC2:
	ret = env_set("mcinitcmd", SD2_MC_INIT_CMD);
		break;
#endif
	default:
#ifdef QSPI_MC_INIT_CMD
	ret = env_set("mcinitcmd", QSPI_MC_INIT_CMD);
#endif
		break;
	}

	if (ret) {
		printf("Failed to set mcinitcmd: ret = %d\n", ret);
		return ret;
	}
	return 0;
}
#endif

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_CHAIN_OF_TRUST
	fsl_setenv_chain_of_trust();
#endif
#ifdef CONFIG_TFABOOT
	/*
	 * check if gd->env_addr is default_environment; then setenv bootcmd
	 * and mcinitcmd.
	 */
#if !defined(CONFIG_ENV_ADDR) || defined(ENV_IS_EMBEDDED)
	if (gd->env_addr == (ulong)&default_environment[0]) {
#else
	if (gd->env_addr + gd->reloc_off == (ulong)&default_environment[0]) {
#endif
		fsl_setenv_bootcmd();
		fsl_setenv_mcinitcmd();
	}

	/*
	 * If the boot mode is secure, default environment is not present then
	 * setenv command needs to be run by default
	 */
#ifdef CONFIG_CHAIN_OF_TRUST
	if ((fsl_check_boot_mode_secure() == 1)) {
		fsl_setenv_bootcmd();
		fsl_setenv_mcinitcmd();
	}
#endif
#endif
#ifdef CONFIG_QSPI_AHB_INIT
	qspi_ahb_init();
#endif

	return 0;
}
#endif
