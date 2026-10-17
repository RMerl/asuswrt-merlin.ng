/*
 * GE b1x5v2 - QMX6 SPL
 *
 * Copyright 2013, Adeneo Embedded <www.adeneo-embedded.com>
 * Copyright 2018-2020 GE Inc.
 * Copyright 2018-2020 Collabora Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <init.h>
#include <spi.h>
#include <spi_flash.h>
#include <spl.h>

#if defined(CONFIG_SPL_BUILD)

#include <asm/arch/mx6-ddr.h>

#define IMX6DQ_DRIVE_STRENGTH_40_OHM		0x30
#define IMX6DQ_DRIVE_STRENGTH_48_OHM		0x28
#define IMX6DQ_DRIVE_STRENGTH			IMX6DQ_DRIVE_STRENGTH_40_OHM

#define QMX6_DDR_PKE_DISABLED			0x00000000
#define QMX6_DDR_ODT_60_OHM			(2 << 16)
#define QMX6_DDR_TYPE_DDR3			0x000c0000

#define QMX6_DRAM_SDCKE_PULLUP_100K		0x00003000
#define QMX6_DRAM_SDBA2_PULLUP_NONE		0x00000000

#define SPI_PAD_CTRL (PAD_CTL_HYS |				\
	PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define SPI1_CS0 IMX_GPIO_NR(3, 19)
#define POWEROFF IMX_GPIO_NR(4, 25)

static iomux_v3_cfg_t const poweroff_pads[] = {
	IOMUX_PADS(PAD_DISP0_DAT4__GPIO4_IO25 | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static iomux_v3_cfg_t const uart2_pads[] = {
	IOMUX_PADS(PAD_EIM_D26__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D27__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

static iomux_v3_cfg_t const uart3_pads[] = {
	IOMUX_PADS(PAD_EIM_D24__UART3_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D25__UART3_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

static iomux_v3_cfg_t const ecspi1_pads[] = {
	IOMUX_PADS(PAD_EIM_D16__ECSPI1_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D17__ECSPI1_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D18__ECSPI1_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D19__GPIO3_IO19 | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static struct mx6dq_iomux_ddr_regs mx6q_ddr_ioregs = {
	.dram_sdclk_0 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdclk_1 = IMX6DQ_DRIVE_STRENGTH,
	.dram_cas = IMX6DQ_DRIVE_STRENGTH,
	.dram_ras = IMX6DQ_DRIVE_STRENGTH,
	.dram_reset = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdcke0 = QMX6_DRAM_SDCKE_PULLUP_100K,
	.dram_sdcke1 = QMX6_DRAM_SDCKE_PULLUP_100K,
	.dram_sdba2 = QMX6_DRAM_SDBA2_PULLUP_NONE,
	.dram_sdodt0 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdodt1 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs0 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs1 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs2 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs3 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs4 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs5 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs6 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs7 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm0 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm1 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm2 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm3 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm4 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm5 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm6 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm7 = IMX6DQ_DRIVE_STRENGTH,
};

static const struct mx6sdl_iomux_ddr_regs mx6dl_ddr_ioregs = {
	.dram_sdclk_0 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdclk_1 = IMX6DQ_DRIVE_STRENGTH,
	.dram_cas = IMX6DQ_DRIVE_STRENGTH,
	.dram_ras = IMX6DQ_DRIVE_STRENGTH,
	.dram_reset = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdcke0 = QMX6_DRAM_SDCKE_PULLUP_100K,
	.dram_sdcke1 = QMX6_DRAM_SDCKE_PULLUP_100K,
	.dram_sdba2 = QMX6_DRAM_SDBA2_PULLUP_NONE,
	.dram_sdodt0 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdodt1 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs0 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs1 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs2 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs3 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs4 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs5 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs6 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs7 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm0 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm1 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm2 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm3 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm4 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm5 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm6 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm7 = IMX6DQ_DRIVE_STRENGTH,
};

static struct mx6dq_iomux_grp_regs mx6q_grp_ioregs = {
	.grp_ddr_type = QMX6_DDR_TYPE_DDR3,
	.grp_ddrmode_ctl = QMX6_DDR_ODT_60_OHM,
	.grp_ddrpke = QMX6_DDR_PKE_DISABLED,
	.grp_addds = IMX6DQ_DRIVE_STRENGTH,
	.grp_ctlds = IMX6DQ_DRIVE_STRENGTH,
	.grp_ddrmode = QMX6_DDR_ODT_60_OHM,
	.grp_b0ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b1ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b2ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b3ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b4ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b5ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b6ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b7ds = IMX6DQ_DRIVE_STRENGTH,
};

static const struct mx6sdl_iomux_grp_regs mx6sdl_grp_ioregs = {
	.grp_ddr_type = QMX6_DDR_TYPE_DDR3,
	.grp_ddrmode_ctl = QMX6_DDR_ODT_60_OHM,
	.grp_ddrpke = QMX6_DDR_PKE_DISABLED,
	.grp_addds = IMX6DQ_DRIVE_STRENGTH,
	.grp_ctlds = IMX6DQ_DRIVE_STRENGTH,
	.grp_ddrmode = QMX6_DDR_ODT_60_OHM,
	.grp_b0ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b1ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b2ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b3ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b4ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b5ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b6ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b7ds = IMX6DQ_DRIVE_STRENGTH,
};

const struct mx6_mmdc_calibration mx6q_mmcd_calib = {
	.p0_mpwldectrl0 =  0x0016001A,
	.p0_mpwldectrl1 =  0x0023001C,
	.p1_mpwldectrl0 =  0x0028003A,
	.p1_mpwldectrl1 =  0x001F002C,
	.p0_mpdgctrl0 =  0x43440354,
	.p0_mpdgctrl1 =  0x033C033C,
	.p1_mpdgctrl0 =  0x43300368,
	.p1_mpdgctrl1 =  0x03500330,
	.p0_mprddlctl =  0x3228242E,
	.p1_mprddlctl =  0x2C2C2636,
	.p0_mpwrdlctl =  0x36323A38,
	.p1_mpwrdlctl =  0x42324440,
};

const struct mx6_mmdc_calibration mx6q_2g_mmcd_calib = {
	.p0_mpwldectrl0 =  0x00080016,
	.p0_mpwldectrl1 =  0x001D0016,
	.p1_mpwldectrl0 =  0x0018002C,
	.p1_mpwldectrl1 =  0x000D001D,
	.p0_mpdgctrl0 =    0x43200334,
	.p0_mpdgctrl1 =    0x0320031C,
	.p1_mpdgctrl0 =    0x0344034C,
	.p1_mpdgctrl1 =    0x03380314,
	.p0_mprddlctl =    0x3E36383A,
	.p1_mprddlctl =    0x38363240,
	.p0_mpwrdlctl =	   0x36364238,
	.p1_mpwrdlctl =    0x4230423E,
};

const struct mx6_mmdc_calibration mx6q_4g_mmcd_calib = {
	.p0_mpwldectrl0 =  0x00180018,
	.p0_mpwldectrl1 =  0x00220018,
	.p1_mpwldectrl0 =  0x00330046,
	.p1_mpwldectrl1 =  0x002B003D,
	.p0_mpdgctrl0 =    0x4344034C,
	.p0_mpdgctrl1 =    0x033C033C,
	.p1_mpdgctrl0 =    0x03700374,
	.p1_mpdgctrl1 =    0x03600338,
	.p0_mprddlctl =    0x443E3E40,
	.p1_mprddlctl =    0x423E3E48,
	.p0_mpwrdlctl =	   0x3C3C4442,
	.p1_mpwrdlctl =    0x46384C46,
};

static const struct mx6_mmdc_calibration mx6s_mmcd_calib = {
	.p0_mpwldectrl0 =  0x00480049,
	.p0_mpwldectrl1 =  0x00410044,
	.p0_mpdgctrl0 =    0x42480248,
	.p0_mpdgctrl1 =    0x023C023C,
	.p0_mprddlctl =    0x40424644,
	.p0_mpwrdlctl =    0x34323034,
};

static const struct mx6_mmdc_calibration mx6s_2g_mmcd_calib = {
	.p0_mpwldectrl0 =  0x00450048,
	.p0_mpwldectrl1 =  0x003B003F,
	.p0_mpdgctrl0 =    0x424C0248,
	.p0_mpdgctrl1 =    0x0234023C,
	.p0_mprddlctl =    0x40444848,
	.p0_mpwrdlctl =    0x38363232,
};

static const struct mx6_mmdc_calibration mx6dl_mmcd_calib = {
	.p0_mpwldectrl0 =  0x0043004B,
	.p0_mpwldectrl1 =  0x003A003E,
	.p1_mpwldectrl0 =  0x0047004F,
	.p1_mpwldectrl1 =  0x004E0061,
	.p0_mpdgctrl0 =    0x42500250,
	.p0_mpdgctrl1 =	   0x0238023C,
	.p1_mpdgctrl0 =    0x42640264,
	.p1_mpdgctrl1 =    0x02500258,
	.p0_mprddlctl =    0x40424846,
	.p1_mprddlctl =    0x46484842,
	.p0_mpwrdlctl =    0x38382C30,
	.p1_mpwrdlctl =    0x34343430,
};

static const struct mx6_mmdc_calibration mx6dl_2g_mmcd_calib = {
	.p0_mpwldectrl0 =  0x00450045,
	.p0_mpwldectrl1 =  0x00390043,
	.p1_mpwldectrl0 =  0x0049004D,
	.p1_mpwldectrl1 =  0x004E0061,
	.p0_mpdgctrl0 =    0x4240023C,
	.p0_mpdgctrl1 =	   0x0228022C,
	.p1_mpdgctrl0 =    0x02400244,
	.p1_mpdgctrl1 =    0x02340238,
	.p0_mprddlctl =    0x42464648,
	.p1_mprddlctl =    0x4446463C,
	.p0_mpwrdlctl =    0x3C38323A,
	.p1_mpwrdlctl =    0x34323430,
};

static struct mx6_ddr3_cfg mem_ddr_2g = {
	.mem_speed = 1600,
	.density = 2,
	.width = 16,
	.banks = 8,
	.rowaddr = 14,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1310,
	.trcmin = 4875,
	.trasmin = 3500,
};

static struct mx6_ddr3_cfg mem_ddr_4g = {
	.mem_speed = 1600,
	.density = 4,
	.width = 16,
	.banks = 8,
	.rowaddr = 15,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1310,
	.trcmin = 4875,
	.trasmin = 3500,
};

static struct mx6_ddr3_cfg mem_ddr_8g = {
	.mem_speed = 1600,
	.density = 8,
	.width = 16,
	.banks = 8,
	.rowaddr = 16,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1310,
	.trcmin = 4875,
	.trasmin = 3500,
};

static void spl_dram_init(u8 width, u32 memsize) {
	struct mx6_ddr_sysinfo sysinfo = {
		 /* width of data bus: 0=16, 1=32, 2=64 */
		.dsize		= width / 32,
		/* config for full 4GB range so that get_mem_size() works */
		.cs_density	= 32,	/* 32Gb per CS */

		.ncs = 1,
		.cs1_mirror = 0,
		.rtt_wr = 2,
		.rtt_nom = 2,
		.walat = 0,
		.ralat = 5,
		.mif3_mode = 3,
		.bi_on = 1,
		.sde_to_rst = 0x0d,
		.rst_to_cke = 0x20,
	};

	if (is_cpu_type(MXC_CPU_MX6SOLO)) {
		sysinfo.walat = 1;
		mx6sdl_dram_iocfg(width, &mx6dl_ddr_ioregs, &mx6sdl_grp_ioregs);

		switch(memsize) {
		case 512:
			mx6_dram_cfg(&sysinfo, &mx6s_2g_mmcd_calib, &mem_ddr_2g);
			break;
		default:
			mx6_dram_cfg(&sysinfo, &mx6s_mmcd_calib, &mem_ddr_4g);
			break;
		}
	} else if (is_cpu_type(MXC_CPU_MX6DL)) {
		sysinfo.walat = 1;
		mx6sdl_dram_iocfg(width, &mx6dl_ddr_ioregs, &mx6sdl_grp_ioregs);

		switch(memsize) {
		case 2048:
			mx6_dram_cfg(&sysinfo, &mx6dl_2g_mmcd_calib, &mem_ddr_4g);
			break;
		default:
			mx6_dram_cfg(&sysinfo, &mx6dl_mmcd_calib, &mem_ddr_2g);
			break;
		}
	} else if (is_cpu_type(MXC_CPU_MX6Q)) {
		mx6dq_dram_iocfg(width, &mx6q_ddr_ioregs, &mx6q_grp_ioregs);

		switch(memsize) {
		case 4096:
			sysinfo.cs_density = 16;
			sysinfo.ncs = 2;
			mx6_dram_cfg(&sysinfo, &mx6q_4g_mmcd_calib, &mem_ddr_8g);
			break;
		case 2048:
			mx6_dram_cfg(&sysinfo, &mx6q_2g_mmcd_calib, &mem_ddr_4g);
			break;
		default:
			mx6_dram_cfg(&sysinfo, &mx6q_mmcd_calib, &mem_ddr_2g);
			break;
		}
	}
}

/* Define a minimal structure so that the part number can be read via SPL */
#define CFG_MFG_ADDR_OFFSET	(spi->size - SZ_16K)
struct mfgdata {
	unsigned char tsize;
	/* size of checksummed part in bytes */
	unsigned char ckcnt;
	/* checksum corrected byte */
	unsigned char cksum;
	/* decimal serial number, packed BCD */
	unsigned char serial[6];
	 /* part number, right justified, ASCII */
	unsigned char pn[16];
};

static void conv_ascii(unsigned char *dst, unsigned char *src, int len)
{
	int remain = len;
	unsigned char *sptr = src;
	unsigned char *dptr = dst;

	while (remain) {
		if (*sptr) {
			*dptr = *sptr;
			dptr++;
		}
		sptr++;
		remain--;
	}
	*dptr = 0x0;
}

/*
 * Returns the total size of the memory [in MB] the board is equipped with
 *
 * This is determined via the partnumber which is stored in the
 * congatec manufacturing area
 */
static int get_boardmem_size(struct spi_flash *spi)
{
	int ret;
	int i;
	int arraysize;
	char buf[sizeof(struct mfgdata)];
	struct mfgdata *data = (struct mfgdata *)buf;
	unsigned char outbuf[32];
	char partnumbers_2g[4][7] = { "016104", "016105", "016304", "016305" };
	char partnumbers_4g[2][7] = { "016308", "016318" };
	char partnumbers_512m[2][7] = { "016203", "616300" };

	ret = spi_flash_read(spi, CFG_MFG_ADDR_OFFSET, sizeof(struct mfgdata),
			     buf);
	if (ret)
		return 1024; /* default to 1GByte in case of error */

	conv_ascii(outbuf, data->pn, sizeof(data->pn));

	printf("Detected Congatec QMX6 SOM: %s\n", outbuf);

	/* congatec PN 016104, 016105, 016304, 016305 have 2GiB of RAM */
	arraysize = sizeof(partnumbers_2g) / sizeof(partnumbers_2g[0]);
	for (i=0; i < arraysize; i++) {
		if (!memcmp(outbuf,partnumbers_2g[i],6))
			return 2048;
	}

	/* congatec PN 016308, 016318 have 4GiB of RAM */
	arraysize = sizeof(partnumbers_4g) / sizeof(partnumbers_4g[0]);
	for (i=0; i < arraysize; i++) {
		if (!memcmp(outbuf,partnumbers_4g[i],6))
			return 4096;
	}

	/* congatec PN 016203, 616300 has 512MiB of RAM */
	arraysize = sizeof(partnumbers_512m) / sizeof(partnumbers_512m[0]);
	for (i=0; i < arraysize; i++) {
		if (!memcmp(outbuf,partnumbers_512m[i],6))
			return 512;
	}

	/* default to 1GByte */
	return 1024;
}

void reset_cpu(void)
{
}

int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	if (bus == 0 && cs == 0)
		return (SPI1_CS0);
	else
		return -1;
}

static void memory_init(void) {
	struct spi_flash *spi;
	u8 width;
	u32 size;

	SETUP_IOMUX_PADS(ecspi1_pads);
	gpio_direction_output(SPI1_CS0, 0);

	spi = spi_flash_probe(CONFIG_ENV_SPI_BUS,
			      CONFIG_ENV_SPI_CS,
			      CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
	if (!spi)
		panic("Cannot identify board type: SPI-NOR flash module not detected\n");

	/* lock manufacturer area */
	spi_flash_protect(spi, CFG_MFG_ADDR_OFFSET, SZ_16K, true);

	width = is_cpu_type(MXC_CPU_MX6SOLO) ? 32 : 64;
	size = get_boardmem_size(spi);
	printf("Detected Memory Size: %u\n", size);

	spl_dram_init(width, size);
}

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	static const uint32_t ccgr0 =
		MXC_CCM_CCGR0_AIPS_TZ1_MASK |
		MXC_CCM_CCGR0_AIPS_TZ2_MASK |
		MXC_CCM_CCGR0_APBHDMA_MASK |
		MXC_CCM_CCGR0_CAAM_SECURE_MEM_MASK |
		MXC_CCM_CCGR0_CAAM_WRAPPER_ACLK_MASK |
		MXC_CCM_CCGR0_CAAM_WRAPPER_IPG_MASK |
		MXC_CCM_CCGR0_CHEETAH_DBG_CLK_MASK;

	static const uint32_t ccgr1 =
		MXC_CCM_CCGR1_ECSPI1S_MASK |
		MXC_CCM_CCGR1_ENET_MASK |
		MXC_CCM_CCGR1_EPIT1S_MASK |
		MXC_CCM_CCGR1_EPIT2S_MASK |
		MXC_CCM_CCGR1_GPT_BUS_MASK;

	static const uint32_t ccgr2 =
		MXC_CCM_CCGR2_IOMUX_IPT_CLK_IO_MASK |
		MXC_CCM_CCGR2_IPMUX1_MASK |
		MXC_CCM_CCGR2_IPMUX2_MASK |
		MXC_CCM_CCGR2_IPMUX3_MASK |
		MXC_CCM_CCGR2_IPSYNC_IP2APB_TZASC1_IPGS_MASK |
		MXC_CCM_CCGR2_IPSYNC_IP2APB_TZASC2_IPG_MASK |
		MXC_CCM_CCGR2_IPSYNC_VDOA_IPG_MASTER_CLK_MASK;

	static const uint32_t ccgr3 =
		MXC_CCM_CCGR3_MMDC_CORE_ACLK_FAST_CORE_P0_MASK |
		MXC_CCM_CCGR3_MMDC_CORE_ACLK_FAST_CORE_P1_MASK |
		MXC_CCM_CCGR3_MMDC_CORE_IPG_CLK_P0_MASK |
		MXC_CCM_CCGR3_MMDC_CORE_IPG_CLK_P1_MASK |
		MXC_CCM_CCGR3_OCRAM_MASK;

	static const uint32_t ccgr4 =
		MXC_CCM_CCGR4_PL301_MX6QFAST1_S133_MASK |
		MXC_CCM_CCGR4_PL301_MX6QPER1_BCH_MASK |
		MXC_CCM_CCGR4_PL301_MX6QPER2_MAINCLK_ENABLE_MASK |
		MXC_CCM_CCGR4_PWM1_MASK |
		MXC_CCM_CCGR4_PWM2_MASK |
		MXC_CCM_CCGR4_PWM3_MASK |
		MXC_CCM_CCGR4_PWM4_MASK;

	static const uint32_t ccgr5 =
		MXC_CCM_CCGR5_ROM_MASK |
		MXC_CCM_CCGR5_SDMA_MASK |
		MXC_CCM_CCGR5_UART_MASK |
		MXC_CCM_CCGR5_UART_SERIAL_MASK;

	static const uint32_t ccgr6 =
		MXC_CCM_CCGR6_USBOH3_MASK |
		MXC_CCM_CCGR6_USDHC1_MASK |
		MXC_CCM_CCGR6_USDHC2_MASK |
		MXC_CCM_CCGR6_SIM1_CLK_MASK |
		MXC_CCM_CCGR6_SIM2_CLK_MASK;

	writel(ccgr0, &ccm->CCGR0);
	writel(ccgr1, &ccm->CCGR1);
	writel(ccgr2, &ccm->CCGR2);
	writel(ccgr3, &ccm->CCGR3);
	writel(ccgr4, &ccm->CCGR4);
	writel(ccgr5, &ccm->CCGR5);
	writel(ccgr6, &ccm->CCGR6);
}

void board_init_f(ulong dummy)
{
	/* setup clock gating */
	ccgr_init();

	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	/* setup AXI */
	gpr_init();

	/*
	 * setup poweroff GPIO. This controls system power regulator. Once
	 * the power button is released this must be enabled to keep system
	 * running. Not enabling it (or disabling it later) will turn off
	 * the main system regulator and instantly poweroff the system. We
	 * do this very early, to reduce the time users have to press the
	 * power button.
	 */
	SETUP_IOMUX_PADS(poweroff_pads);
	gpio_direction_output(POWEROFF, 1);

	/* setup GP timer */
	timer_init();

	/* iomux */
	if (CFG_MXC_UART_BASE == UART2_BASE)
		SETUP_IOMUX_PADS(uart2_pads);
	else if (CFG_MXC_UART_BASE == UART3_BASE)
		SETUP_IOMUX_PADS(uart3_pads);

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* Needed for malloc() [used by SPI] to work in SPL prior to board_init_r() */
	spl_init();

	/* DDR initialization */
	memory_init();
}

void spl_board_prepare_for_boot(void)
{
	printf("Load normal U-Boot...\n");
}
#endif
