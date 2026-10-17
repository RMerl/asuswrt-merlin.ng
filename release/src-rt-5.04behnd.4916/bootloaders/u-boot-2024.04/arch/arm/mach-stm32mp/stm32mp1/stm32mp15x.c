// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2021, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY LOGC_ARCH

#include <common.h>
#include <env.h>
#include <log.h>
#include <asm/io.h>
#include <asm/arch/bsec.h>
#include <asm/arch/stm32.h>
#include <asm/arch/sys_proto.h>
#include <dm/device.h>
#include <dm/uclass.h>

/* RCC register */
#define RCC_TZCR		(STM32_RCC_BASE + 0x00)
#define RCC_BDCR		(STM32_RCC_BASE + 0x0140)
#define RCC_MP_APB5ENSETR	(STM32_RCC_BASE + 0x0208)
#define RCC_MP_AHB5ENSETR	(STM32_RCC_BASE + 0x0210)
#define RCC_DBGCFGR		(STM32_RCC_BASE + 0x080C)

#define RCC_BDCR_VSWRST		BIT(31)
#define RCC_BDCR_RTCSRC		GENMASK(17, 16)

#define RCC_DBGCFGR_DBGCKEN	BIT(8)

/* DBGMCU register */
#define DBGMCU_IDC		(STM32_DBGMCU_BASE + 0x00)
#define DBGMCU_APB4FZ1		(STM32_DBGMCU_BASE + 0x2C)
#define DBGMCU_APB4FZ1_IWDG2	BIT(2)

/* Security register */
#define ETZPC_TZMA1_SIZE	(STM32_ETZPC_BASE + 0x04)
#define ETZPC_DECPROT0		(STM32_ETZPC_BASE + 0x10)

#define TZC_GATE_KEEPER		(STM32_TZC_BASE + 0x008)
#define TZC_REGION_ATTRIBUTE0	(STM32_TZC_BASE + 0x110)
#define TZC_REGION_ID_ACCESS0	(STM32_TZC_BASE + 0x114)

#define TAMP_CR1		(STM32_TAMP_BASE + 0x00)

#define PWR_CR1			(STM32_PWR_BASE + 0x00)
#define PWR_MCUCR		(STM32_PWR_BASE + 0x14)
#define PWR_CR1_DBP		BIT(8)
#define PWR_MCUCR_SBF		BIT(6)

/* GPIOZ registers */
#define GPIOZ_SECCFGR		0x54004030

/* DBGMCU register */
#define DBGMCU_IDC		(STM32_DBGMCU_BASE + 0x00)
#define DBGMCU_IDC_DEV_ID_MASK	GENMASK(11, 0)
#define DBGMCU_IDC_DEV_ID_SHIFT	0
#define DBGMCU_IDC_REV_ID_MASK	GENMASK(31, 16)
#define DBGMCU_IDC_REV_ID_SHIFT	16

/* boot interface from Bootrom
 * - boot instance = bit 31:16
 * - boot device = bit 15:0
 */
#define BOOTROM_PARAM_ADDR	0x2FFC0078
#define BOOTROM_MODE_MASK	GENMASK(15, 0)
#define BOOTROM_MODE_SHIFT	0
#define BOOTROM_INSTANCE_MASK	 GENMASK(31, 16)
#define BOOTROM_INSTANCE_SHIFT	16

/* Device Part Number (RPN) = OTP_DATA1 lower 8 bits */
#define RPN_SHIFT	0
#define RPN_MASK	GENMASK(7, 0)

/* Package = bit 27:29 of OTP16 => STM32MP15_PKG defines
 * - 100: LBGA448 (FFI) => AA = LFBGA 18x18mm 448 balls p. 0.8mm
 * - 011: LBGA354 (LCI) => AB = LFBGA 16x16mm 359 balls p. 0.8mm
 * - 010: TFBGA361 (FFC) => AC = TFBGA 12x12mm 361 balls p. 0.5mm
 * - 001: TFBGA257 (LCC) => AD = TFBGA 10x10mm 257 balls p. 0.5mm
 * - others: Reserved
 */
#define PKG_SHIFT	27
#define PKG_MASK	GENMASK(2, 0)

static void security_init(void)
{
	/* Disable the backup domain write protection */
	/* the protection is enable at each reset by hardware */
	/* And must be disable by software */
	setbits_le32(PWR_CR1, PWR_CR1_DBP);

	while (!(readl(PWR_CR1) & PWR_CR1_DBP))
		;

	/* If RTC clock isn't enable so this is a cold boot then we need
	 * to reset the backup domain
	 */
	if (!(readl(RCC_BDCR) & RCC_BDCR_RTCSRC)) {
		setbits_le32(RCC_BDCR, RCC_BDCR_VSWRST);
		while (!(readl(RCC_BDCR) & RCC_BDCR_VSWRST))
			;
		clrbits_le32(RCC_BDCR, RCC_BDCR_VSWRST);
	}

	/* allow non secure access in Write/Read for all peripheral */
	writel(GENMASK(25, 0), ETZPC_DECPROT0);

	/* Open SYSRAM for no secure access */
	writel(0x0, ETZPC_TZMA1_SIZE);

	/* enable TZC1 TZC2 clock */
	writel(BIT(11) | BIT(12), RCC_MP_APB5ENSETR);

	/* Region 0 set to no access by default */
	/* bit 0 / 16 => nsaid0 read/write Enable
	 * bit 1 / 17 => nsaid1 read/write Enable
	 * ...
	 * bit 15 / 31 => nsaid15 read/write Enable
	 */
	writel(0xFFFFFFFF, TZC_REGION_ID_ACCESS0);
	/* bit 30 / 31 => Secure Global Enable : write/read */
	/* bit 0 / 1 => Region Enable for filter 0/1 */
	writel(BIT(0) | BIT(1) | BIT(30) | BIT(31), TZC_REGION_ATTRIBUTE0);

	/* Enable Filter 0 and 1 */
	setbits_le32(TZC_GATE_KEEPER, BIT(0) | BIT(1));

	/* RCC trust zone deactivated */
	writel(0x0, RCC_TZCR);

	/* TAMP: deactivate the internal tamper
	 * Bit 23 ITAMP8E: monotonic counter overflow
	 * Bit 20 ITAMP5E: RTC calendar overflow
	 * Bit 19 ITAMP4E: HSE monitoring
	 * Bit 18 ITAMP3E: LSE monitoring
	 * Bit 16 ITAMP1E: RTC power domain supply monitoring
	 */
	writel(0x0, TAMP_CR1);

	/* GPIOZ: deactivate the security */
	writel(BIT(0), RCC_MP_AHB5ENSETR);
	writel(0x0, GPIOZ_SECCFGR);
}

/*
 * Debug init
 */
void dbgmcu_init(void)
{
	/*
	 * Freeze IWDG2 if Cortex-A7 is in debug mode
	 * done in TF-A for TRUSTED boot and
	 * DBGMCU access is controlled by BSEC_DENABLE.DBGSWENABLE
	 */
	if (bsec_dbgswenable()) {
		setbits_le32(RCC_DBGCFGR, RCC_DBGCFGR_DBGCKEN);
		setbits_le32(DBGMCU_APB4FZ1, DBGMCU_APB4FZ1_IWDG2);
	}
}

void spl_board_init(void)
{
	struct udevice *dev;
	int ret;

	dbgmcu_init();

	/* force probe of BSEC driver to shadow the upper OTP */
	ret = uclass_get_device_by_driver(UCLASS_MISC, DM_DRIVER_GET(stm32mp_bsec), &dev);
	if (ret)
		log_warning("BSEC probe failed: %d\n", ret);
}

/* get bootmode from ROM code boot context: saved in TAMP register */
static void update_bootmode(void)
{
	u32 boot_mode;
	u32 bootrom_itf = readl(BOOTROM_PARAM_ADDR);
	u32 bootrom_device, bootrom_instance;

	/* enable TAMP clock = RTCAPBEN */
	writel(BIT(8), RCC_MP_APB5ENSETR);

	/* read bootrom context */
	bootrom_device =
		(bootrom_itf & BOOTROM_MODE_MASK) >> BOOTROM_MODE_SHIFT;
	bootrom_instance =
		(bootrom_itf & BOOTROM_INSTANCE_MASK) >> BOOTROM_INSTANCE_SHIFT;
	boot_mode =
		((bootrom_device << BOOT_TYPE_SHIFT) & BOOT_TYPE_MASK) |
		((bootrom_instance << BOOT_INSTANCE_SHIFT) &
		 BOOT_INSTANCE_MASK);

	/* save the boot mode in TAMP backup register */
	clrsetbits_le32(TAMP_BOOT_CONTEXT,
			TAMP_BOOT_MODE_MASK,
			boot_mode << TAMP_BOOT_MODE_SHIFT);
}

/* weak function: STM32MP15x mach init for boot without TFA */
void stm32mp_cpu_init(void)
{
	if (IS_ENABLED(CONFIG_SPL_BUILD)) {
		security_init();
		update_bootmode();
	}

	/* reset copro state in SPL, when used, or in U-Boot */
	if (!IS_ENABLED(CONFIG_SPL) || IS_ENABLED(CONFIG_SPL_BUILD)) {
		/* Reset Coprocessor state unless it wakes up from Standby power mode */
		if (!(readl(PWR_MCUCR) & PWR_MCUCR_SBF)) {
			writel(TAMP_COPRO_STATE_OFF, TAMP_COPRO_STATE);
			writel(0, TAMP_COPRO_RSC_TBL_ADDRESS);
		}
	}
}

static u32 read_idc(void)
{
	/* DBGMCU access is controlled by BSEC_DENABLE.DBGSWENABLE */
	if (bsec_dbgswenable()) {
		setbits_le32(RCC_DBGCFGR, RCC_DBGCFGR_DBGCKEN);

		return readl(DBGMCU_IDC);
	}

	return CPU_DEV_STM32MP15; /* STM32MP15x and unknown revision */
}

u32 get_cpu_dev(void)
{
	return (read_idc() & DBGMCU_IDC_DEV_ID_MASK) >> DBGMCU_IDC_DEV_ID_SHIFT;
}

u32 get_cpu_rev(void)
{
	return (read_idc() & DBGMCU_IDC_REV_ID_MASK) >> DBGMCU_IDC_REV_ID_SHIFT;
}

/* Get Device Part Number (RPN) from OTP */
static u32 get_cpu_rpn(void)
{
	return get_otp(BSEC_OTP_RPN, RPN_SHIFT, RPN_MASK);
}

u32 get_cpu_type(void)
{
	return (get_cpu_dev() << 16) | get_cpu_rpn();
}

int get_eth_nb(void)
{
	return 1;
}

/* Get Package options from OTP */
u32 get_cpu_package(void)
{
	return get_otp(BSEC_OTP_PKG, PKG_SHIFT, PKG_MASK);
}

static const char * const soc_type[] = {
	"????",
	"151C", "151A", "151F", "151D",
	"153C", "153A", "153F", "153D",
	"157C", "157A", "157F", "157D"
};

static const char * const soc_pkg[] = { "??", "AD", "AC", "AB", "AA" };
static const char * const soc_rev[] = { "?", "A", "B", "Z", "Y"};

static void get_cpu_string_offsets(unsigned int *type, unsigned int *pkg,
				   unsigned int *rev)
{
	u32 cpu_type = get_cpu_type();
	u32 ct = cpu_type & ~(BIT(7) | BIT(0));
	u32 cm = ((cpu_type & BIT(7)) >> 6) | (cpu_type & BIT(0));

	/* Bits 0 and 7 are the ACDF, 00:C 01:A 10:F 11:D */
	switch (ct) {
	case CPU_STM32MP151Cxx:
		*type = cm + 1;
		break;
	case CPU_STM32MP153Cxx:
		*type = cm + 5;
		break;
	case CPU_STM32MP157Cxx:
		*type = cm + 9;
		break;
	default:
		*type = 0;
		break;
	}

	/* Package */
	*pkg = get_cpu_package();
	if (*pkg > STM32MP15_PKG_AA_LBGA448)
		*pkg = STM32MP15_PKG_UNKNOWN;

	/* Revision */
	switch (get_cpu_rev()) {
	case CPU_REV1:
		*rev = 1;
		break;
	case CPU_REV2:
		*rev = 2;
		break;
	case CPU_REV2_1:
		*rev = 3;
		break;
	case CPU_REV2_2:
		*rev = 4;
		break;
	default:
		*rev = 0;
		break;
	}
}

void get_soc_name(char name[SOC_NAME_SIZE])
{
	unsigned int type, pkg, rev;

	get_cpu_string_offsets(&type, &pkg, &rev);

	snprintf(name, SOC_NAME_SIZE, "STM32MP%s%s Rev.%s",
		 soc_type[type], soc_pkg[pkg], soc_rev[rev]);
}

static void setup_soc_type_pkg_rev(void)
{
	unsigned int type, pkg, rev;

	get_cpu_string_offsets(&type, &pkg, &rev);

	env_set("soc_type", soc_type[type]);
	env_set("soc_pkg", soc_pkg[pkg]);
	env_set("soc_rev", soc_rev[rev]);
}

/* weak function called in arch_misc_init */
void stm32mp_misc_init(void)
{
	setup_soc_type_pkg_rev();
}
