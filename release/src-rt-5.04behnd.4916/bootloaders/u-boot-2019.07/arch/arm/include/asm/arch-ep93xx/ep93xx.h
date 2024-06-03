/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Cirrus Logic EP93xx register definitions.
 *
 * Copyright (C) 2013
 * Sergey Kostanbaev <sergey.kostanbaev <at> fairwaves.ru>
 *
 * Copyright (C) 2009
 * Matthias Kaehlcke <matthias@kaehlcke.net>
 *
 * Copyright (C) 2006
 * Dominic Rath <Dominic.Rath@gmx.de>
 *
 * Copyright (C) 2004, 2005
 * Cory T. Tusar, Videon Central, Inc., <ctusar@videon-central.com>
 *
 * Based in large part on linux/include/asm-arm/arch-ep93xx/regmap.h, which is
 *
 * Copyright (C) 2004 Ray Lehtiniemi
 * Copyright (C) 2003 Cirrus Logic, Inc
 * Copyright (C) 1999 ARM Limited.
 */

#define EP93XX_AHB_BASE			0x80000000
#define EP93XX_APB_BASE			0x80800000

/*
 * 0x80000000 - 0x8000FFFF: DMA
 */
#define DMA_OFFSET			0x000000
#define DMA_BASE			(EP93XX_AHB_BASE | DMA_OFFSET)

#ifndef __ASSEMBLY__
struct dma_channel {
	uint32_t control;
	uint32_t interrupt;
	uint32_t ppalloc;
	uint32_t status;
	uint32_t reserved0;
	uint32_t remain;
	uint32_t reserved1[2];
	uint32_t maxcnt0;
	uint32_t base0;
	uint32_t current0;
	uint32_t reserved2;
	uint32_t maxcnt1;
	uint32_t base1;
	uint32_t current1;
	uint32_t reserved3;
};

struct dma_regs {
	struct dma_channel m2p_channel_0;
	struct dma_channel m2p_channel_1;
	struct dma_channel m2p_channel_2;
	struct dma_channel m2p_channel_3;
	struct dma_channel m2m_channel_0;
	struct dma_channel m2m_channel_1;
	struct dma_channel reserved0[2];
	struct dma_channel m2p_channel_5;
	struct dma_channel m2p_channel_4;
	struct dma_channel m2p_channel_7;
	struct dma_channel m2p_channel_6;
	struct dma_channel m2p_channel_9;
	struct dma_channel m2p_channel_8;
	uint32_t channel_arbitration;
	uint32_t reserved[15];
	uint32_t global_interrupt;
};
#endif

/*
 * 0x80010000 - 0x8001FFFF: Ethernet MAC
 */
#define MAC_OFFSET			0x010000
#define MAC_BASE			(EP93XX_AHB_BASE | MAC_OFFSET)

#ifndef __ASSEMBLY__
struct mac_queue {
	uint32_t badd;
	union { /* deal with half-word aligned registers */
		uint32_t blen;
		union {
			uint16_t filler;
			uint16_t curlen;
		};
	};
	uint32_t curadd;
};

struct mac_regs {
	uint32_t rxctl;
	uint32_t txctl;
	uint32_t testctl;
	uint32_t reserved0;
	uint32_t miicmd;
	uint32_t miidata;
	uint32_t miists;
	uint32_t reserved1;
	uint32_t selfctl;
	uint32_t inten;
	uint32_t intstsp;
	uint32_t intstsc;
	uint32_t reserved2[2];
	uint32_t diagad;
	uint32_t diagdata;
	uint32_t gt;
	uint32_t fct;
	uint32_t fcf;
	uint32_t afp;
	union {
		struct {
			uint32_t indad;
			uint32_t indad_upper;
		};
		uint32_t hashtbl;
	};
	uint32_t reserved3[2];
	uint32_t giintsts;
	uint32_t giintmsk;
	uint32_t giintrosts;
	uint32_t giintfrc;
	uint32_t txcollcnt;
	uint32_t rxmissnct;
	uint32_t rxruntcnt;
	uint32_t reserved4;
	uint32_t bmctl;
	uint32_t bmsts;
	uint32_t rxbca;
	uint32_t reserved5;
	struct mac_queue rxdq;
	uint32_t rxdqenq;
	struct mac_queue rxstsq;
	uint32_t rxstsqenq;
	struct mac_queue txdq;
	uint32_t txdqenq;
	struct mac_queue txstsq;
	uint32_t reserved6;
	uint32_t rxbufthrshld;
	uint32_t txbufthrshld;
	uint32_t rxststhrshld;
	uint32_t txststhrshld;
	uint32_t rxdthrshld;
	uint32_t txdthrshld;
	uint32_t maxfrmlen;
	uint32_t maxhdrlen;
};
#endif

#define SELFCTL_RWP		(1 << 7)
#define SELFCTL_GPO0		(1 << 5)
#define SELFCTL_PUWE		(1 << 4)
#define SELFCTL_PDWE		(1 << 3)
#define SELFCTL_MIIL		(1 << 2)
#define SELFCTL_RESET		(1 << 0)

#define INTSTS_RWI		(1 << 30)
#define INTSTS_RXMI		(1 << 29)
#define INTSTS_RXBI		(1 << 28)
#define INTSTS_RXSQI		(1 << 27)
#define INTSTS_TXLEI		(1 << 26)
#define INTSTS_ECIE		(1 << 25)
#define INTSTS_TXUHI		(1 << 24)
#define INTSTS_MOI		(1 << 18)
#define INTSTS_TXCOI		(1 << 17)
#define INTSTS_RXROI		(1 << 16)
#define INTSTS_MIII		(1 << 12)
#define INTSTS_PHYI		(1 << 11)
#define INTSTS_TI		(1 << 10)
#define INTSTS_AHBE		(1 << 8)
#define INTSTS_OTHER		(1 << 4)
#define INTSTS_TXSQ		(1 << 3)
#define INTSTS_RXSQ		(1 << 2)

#define BMCTL_MT		(1 << 13)
#define BMCTL_TT		(1 << 12)
#define BMCTL_UNH		(1 << 11)
#define BMCTL_TXCHR		(1 << 10)
#define BMCTL_TXDIS		(1 << 9)
#define BMCTL_TXEN		(1 << 8)
#define BMCTL_EH2		(1 << 6)
#define BMCTL_EH1		(1 << 5)
#define BMCTL_EEOB		(1 << 4)
#define BMCTL_RXCHR		(1 << 2)
#define BMCTL_RXDIS		(1 << 1)
#define BMCTL_RXEN		(1 << 0)

#define BMSTS_TXACT		(1 << 7)
#define BMSTS_TP		(1 << 4)
#define BMSTS_RXACT		(1 << 3)
#define BMSTS_QID_MASK		0x07
#define BMSTS_QID_RXDATA	0x00
#define BMSTS_QID_TXDATA	0x01
#define BMSTS_QID_RXSTS		0x02
#define BMSTS_QID_TXSTS		0x03
#define BMSTS_QID_RXDESC	0x04
#define BMSTS_QID_TXDESC	0x05

#define AFP_MASK		0x07
#define AFP_IAPRIMARY		0x00
#define AFP_IASECONDARY1	0x01
#define AFP_IASECONDARY2	0x02
#define AFP_IASECONDARY3	0x03
#define AFP_TX			0x06
#define AFP_HASH		0x07

#define RXCTL_PAUSEA		(1 << 20)
#define RXCTL_RXFCE1		(1 << 19)
#define RXCTL_RXFCE0		(1 << 18)
#define RXCTL_BCRC		(1 << 17)
#define RXCTL_SRXON		(1 << 16)
#define RXCTL_RCRCA		(1 << 13)
#define RXCTL_RA		(1 << 12)
#define RXCTL_PA		(1 << 11)
#define RXCTL_BA		(1 << 10)
#define RXCTL_MA		(1 << 9)
#define RXCTL_IAHA		(1 << 8)
#define RXCTL_IA3		(1 << 3)
#define RXCTL_IA2		(1 << 2)
#define RXCTL_IA1		(1 << 1)
#define RXCTL_IA0		(1 << 0)

#define TXCTL_DEFDIS		(1 << 7)
#define TXCTL_MBE		(1 << 6)
#define TXCTL_ICRC		(1 << 5)
#define TXCTL_TPD		(1 << 4)
#define TXCTL_OCOLL		(1 << 3)
#define TXCTL_SP		(1 << 2)
#define TXCTL_PB		(1 << 1)
#define TXCTL_STXON		(1 << 0)

#define MIICMD_REGAD_MASK	(0x001F)
#define MIICMD_PHYAD_MASK	(0x03E0)
#define MIICMD_OPCODE_MASK	(0xC000)
#define MIICMD_PHYAD_8950	(0x0000)
#define MIICMD_OPCODE_READ	(0x8000)
#define MIICMD_OPCODE_WRITE	(0x4000)

#define MIISTS_BUSY		(1 << 0)

/*
 * 0x80020000 - 0x8002FFFF: USB OHCI
 */
#define USB_OFFSET			0x020000
#define USB_BASE			(EP93XX_AHB_BASE | USB_OFFSET)

/*
 * 0x80030000 - 0x8003FFFF: Raster engine
 */
#if (defined(CONFIG_EP9307) || defined(CONFIG_EP9312) || defined(CONFIG_EP9315))
#define RASTER_OFFSET			0x030000
#define RASTER_BASE			(EP93XX_AHB_BASE | RASTER_OFFSET)
#endif

/*
 * 0x80040000 - 0x8004FFFF: Graphics accelerator
 */
#if defined(CONFIG_EP9315)
#define GFX_OFFSET			0x040000
#define GFX_BASE			(EP93XX_AHB_BASE | GFX_OFFSET)
#endif

/*
 * 0x80050000 - 0x8005FFFF: Reserved
 */

/*
 * 0x80060000 - 0x8006FFFF: SDRAM controller
 */
#define SDRAM_OFFSET			0x060000
#define SDRAM_BASE			(EP93XX_AHB_BASE | SDRAM_OFFSET)

#ifndef __ASSEMBLY__
struct sdram_regs {
	uint32_t reserved;
	uint32_t glconfig;
	uint32_t refrshtimr;
	uint32_t bootsts;
	uint32_t devcfg0;
	uint32_t devcfg1;
	uint32_t devcfg2;
	uint32_t devcfg3;
};
#endif

#define SDRAM_DEVCFG_EXTBUSWIDTH	(1 << 2)
#define SDRAM_DEVCFG_BANKCOUNT		(1 << 3)
#define SDRAM_DEVCFG_SROMLL		(1 << 5)
#define SDRAM_DEVCFG_CASLAT_2		0x00010000
#define SDRAM_DEVCFG_RASTOCAS_2		0x00200000

#define SDRAM_OFF_GLCONFIG		0x0004
#define SDRAM_OFF_REFRSHTIMR		0x0008

#define SDRAM_OFF_DEVCFG0		0x0010
#define SDRAM_OFF_DEVCFG1		0x0014
#define SDRAM_OFF_DEVCFG2		0x0018
#define SDRAM_OFF_DEVCFG3		0x001C

#define SDRAM_DEVCFG0_BASE		0xC0000000
#define SDRAM_DEVCFG1_BASE		0xD0000000
#define SDRAM_DEVCFG2_BASE		0xE0000000
#define SDRAM_DEVCFG3_ASD0_BASE		0xF0000000
#define SDRAM_DEVCFG3_ASD1_BASE		0x00000000

#define GLCONFIG_INIT			(1 << 0)
#define GLCONFIG_MRS			(1 << 1)
#define GLCONFIG_SMEMBUSY		(1 << 5)
#define GLCONFIG_LCR			(1 << 6)
#define GLCONFIG_REARBEN		(1 << 7)
#define GLCONFIG_CLKSHUTDOWN		(1 << 30)
#define GLCONFIG_CKE			(1 << 31)

#define EP93XX_SDRAMCTRL			0x80060000
#define EP93XX_SDRAMCTRL_GLOBALCFG_INIT		0x00000001
#define EP93XX_SDRAMCTRL_GLOBALCFG_MRS		0x00000002
#define EP93XX_SDRAMCTRL_GLOBALCFG_SMEMBUSY	0x00000020
#define EP93XX_SDRAMCTRL_GLOBALCFG_LCR		0x00000040
#define EP93XX_SDRAMCTRL_GLOBALCFG_REARBEN	0x00000080
#define EP93XX_SDRAMCTRL_GLOBALCFG_CLKSHUTDOWN	0x40000000
#define EP93XX_SDRAMCTRL_GLOBALCFG_CKE		0x80000000

#define EP93XX_SDRAMCTRL_REFRESH_MASK		0x0000FFFF

#define EP93XX_SDRAMCTRL_BOOTSTATUS_WIDTH_32	0x00000002
#define EP93XX_SDRAMCTRL_BOOTSTATUS_WIDTH_16	0x00000001
#define EP93XX_SDRAMCTRL_BOOTSTATUS_WIDTH_8	0x00000000
#define EP93XX_SDRAMCTRL_BOOTSTATUS_WIDTH_MASK	0x00000003
#define EP93XX_SDRAMCTRL_BOOTSTATUS_MEDIA	0x00000004

#define EP93XX_SDRAMCTRL_DEVCFG_EXTBUSWIDTH	0x00000004
#define EP93XX_SDRAMCTRL_DEVCFG_BANKCOUNT	0x00000008
#define EP93XX_SDRAMCTRL_DEVCFG_SROM512		0x00000010
#define EP93XX_SDRAMCTRL_DEVCFG_SROMLL		0x00000020
#define EP93XX_SDRAMCTRL_DEVCFG_2KPAGE		0x00000040
#define EP93XX_SDRAMCTRL_DEVCFG_SFCONFIGADDR	0x00000080
#define EP93XX_SDRAMCTRL_DEVCFG_CASLAT_MASK	0x00070000
#define EP93XX_SDRAMCTRL_DEVCFG_CASLAT_2	0x00010000
#define EP93XX_SDRAMCTRL_DEVCFG_CASLAT_3	0x00020000
#define EP93XX_SDRAMCTRL_DEVCFG_CASLAT_4	0x00030000
#define EP93XX_SDRAMCTRL_DEVCFG_CASLAT_5	0x00040000
#define EP93XX_SDRAMCTRL_DEVCFG_CASLAT_6	0x00050000
#define EP93XX_SDRAMCTRL_DEVCFG_CASLAT_7	0x00060000
#define EP93XX_SDRAMCTRL_DEVCFG_CASLAT_8	0x00070000
#define EP93XX_SDRAMCTRL_DEVCFG_WBL		0x00080000
#define EP93XX_SDRAMCTRL_DEVCFG_RASTOCAS_MASK	0x00300000
#define EP93XX_SDRAMCTRL_DEVCFG_RASTOCAS_2	0x00200000
#define EP93XX_SDRAMCTRL_DEVCFG_RASTOCAS_3	0x00300000
#define EP93XX_SDRAMCTRL_DEVCFG_AUTOPRECHARGE	0x01000000

/*
 * 0x80070000 - 0x8007FFFF: Reserved
 */

/*
 * 0x80080000 - 0x8008FFFF: SRAM controller & PCMCIA
 */
#define SMC_OFFSET			0x080000
#define SMC_BASE			(EP93XX_AHB_BASE | SMC_OFFSET)

#ifndef __ASSEMBLY__
struct smc_regs {
	uint32_t bcr0;
	uint32_t bcr1;
	uint32_t bcr2;
	uint32_t bcr3;
	uint32_t reserved0[2];
	uint32_t bcr6;
	uint32_t bcr7;
#if defined(CONFIG_EP9315)
	uint32_t pcattribute;
	uint32_t pccommon;
	uint32_t pcio;
	uint32_t reserved1[5];
	uint32_t pcmciactrl;
#endif
};
#endif

#define EP93XX_OFF_SMCBCR0		0x00
#define EP93XX_OFF_SMCBCR1		0x04
#define EP93XX_OFF_SMCBCR2		0x08
#define EP93XX_OFF_SMCBCR3		0x0C
#define EP93XX_OFF_SMCBCR6		0x18
#define EP93XX_OFF_SMCBCR7		0x1C

#define SMC_BCR_IDCY_SHIFT	0
#define SMC_BCR_WST1_SHIFT	5
#define SMC_BCR_BLE		(1 << 10)
#define SMC_BCR_WST2_SHIFT	11
#define SMC_BCR_MW_SHIFT	28

/*
 * 0x80090000 - 0x8009FFFF: Boot ROM
 */

/*
 * 0x800A0000 - 0x800AFFFF: IDE interface
 */

/*
 * 0x800B0000 - 0x800BFFFF: VIC1
 */

/*
 * 0x800C0000 - 0x800CFFFF: VIC2
 */

/*
 * 0x800D0000 - 0x800FFFFF: Reserved
 */

/*
 * 0x80800000 - 0x8080FFFF: Reserved
 */

/*
 * 0x80810000 - 0x8081FFFF: Timers
 */
#define TIMER_OFFSET		0x010000
#define TIMER_BASE		(EP93XX_APB_BASE | TIMER_OFFSET)

#ifndef __ASSEMBLY__
struct timer {
	uint32_t load;
	uint32_t value;
	uint32_t control;
	uint32_t clear;
};

struct timer4 {
	uint32_t value_low;
	uint32_t value_high;
};

struct timer_regs {
	struct timer timer1;
	uint32_t reserved0[4];
	struct timer timer2;
	uint32_t reserved1[12];
	struct timer4 timer4;
	uint32_t reserved2[6];
	struct timer timer3;
};
#endif

/*
 * 0x80820000 - 0x8082FFFF: I2S
 */
#define I2S_OFFSET		0x020000
#define I2S_BASE		(EP93XX_APB_BASE | I2S_OFFSET)

/*
 * 0x80830000 - 0x8083FFFF: Security
 */
#define SECURITY_OFFSET		0x030000
#define SECURITY_BASE		(EP93XX_APB_BASE | SECURITY_OFFSET)

#define EXTENSIONID		(SECURITY_BASE + 0x2714)

/*
 * 0x80840000 - 0x8084FFFF: GPIO
 */
#define GPIO_OFFSET		0x040000
#define GPIO_BASE		(EP93XX_APB_BASE | GPIO_OFFSET)

#ifndef __ASSEMBLY__
struct gpio_int {
	uint32_t inttype1;
	uint32_t inttype2;
	uint32_t eoi;
	uint32_t inten;
	uint32_t intsts;
	uint32_t rawintsts;
	uint32_t db;
};

struct gpio_regs {
	uint32_t padr;
	uint32_t pbdr;
	uint32_t pcdr;
	uint32_t pddr;
	uint32_t paddr;
	uint32_t pbddr;
	uint32_t pcddr;
	uint32_t pdddr;
	uint32_t pedr;
	uint32_t peddr;
	uint32_t reserved0[2];
	uint32_t pfdr;
	uint32_t pfddr;
	uint32_t pgdr;
	uint32_t pgddr;
	uint32_t phdr;
	uint32_t phddr;
	uint32_t reserved1;
	uint32_t finttype1;
	uint32_t finttype2;
	uint32_t reserved2;
	struct gpio_int pfint;
	uint32_t reserved3[10];
	struct gpio_int paint;
	struct gpio_int pbint;
	uint32_t eedrive;
};
#endif

#define EP93XX_LED_DATA		0x80840020
#define EP93XX_LED_GREEN_ON	0x0001
#define EP93XX_LED_RED_ON	0x0002

#define EP93XX_LED_DDR		0x80840024
#define EP93XX_LED_GREEN_ENABLE	0x0001
#define EP93XX_LED_RED_ENABLE	0x00020000

/*
 * 0x80850000 - 0x8087FFFF: Reserved
 */

/*
 * 0x80880000 - 0x8088FFFF: AAC
 */
#define AAC_OFFSET		0x080000
#define AAC_BASE		(EP93XX_APB_BASE | AAC_OFFSET)

/*
 * 0x80890000 - 0x8089FFFF: Reserved
 */

/*
 * 0x808A0000 - 0x808AFFFF: SPI
 */
#define SPI_OFFSET		0x0A0000
#define SPI_BASE		(EP93XX_APB_BASE | SPI_OFFSET)

/*
 * 0x808B0000 - 0x808BFFFF: IrDA
 */
#define IRDA_OFFSET		0x0B0000
#define IRDA_BASE		(EP93XX_APB_BASE | IRDA_OFFSET)

/*
 * 0x808C0000 - 0x808CFFFF: UART1
 */
#define UART1_OFFSET		0x0C0000
#define UART1_BASE		(EP93XX_APB_BASE | UART1_OFFSET)

/*
 * 0x808D0000 - 0x808DFFFF: UART2
 */
#define UART2_OFFSET		0x0D0000
#define UART2_BASE		(EP93XX_APB_BASE | UART2_OFFSET)

/*
 * 0x808E0000 - 0x808EFFFF: UART3
 */
#define UART3_OFFSET		0x0E0000
#define UART3_BASE		(EP93XX_APB_BASE | UART3_OFFSET)

/*
 * 0x808F0000 - 0x808FFFFF: Key Matrix
 */
#define KEY_OFFSET		0x0F0000
#define KEY_BASE		(EP93XX_APB_BASE | KEY_OFFSET)

/*
 * 0x80900000 - 0x8090FFFF: Touchscreen
 */
#define TOUCH_OFFSET		0x900000
#define TOUCH_BASE		(EP93XX_APB_BASE | TOUCH_OFFSET)

/*
 * 0x80910000 - 0x8091FFFF: Pulse Width Modulation
 */
#define PWM_OFFSET		0x910000
#define PWM_BASE		(EP93XX_APB_BASE | PWM_OFFSET)

/*
 * 0x80920000 - 0x8092FFFF: Real time clock
 */
#define RTC_OFFSET		0x920000
#define RTC_BASE		(EP93XX_APB_BASE | RTC_OFFSET)

/*
 * 0x80930000 - 0x8093FFFF: Syscon
 */
#define SYSCON_OFFSET		0x930000
#define SYSCON_BASE		(EP93XX_APB_BASE | SYSCON_OFFSET)

/* Security */
#define SECURITY_EXTENSIONID	0x80832714

#ifndef __ASSEMBLY__
struct syscon_regs {
	uint32_t pwrsts;
	uint32_t pwrcnt;
	uint32_t halt;
	uint32_t stby;
	uint32_t reserved0[2];
	uint32_t teoi;
	uint32_t stfclr;
	uint32_t clkset1;
	uint32_t clkset2;
	uint32_t reserved1[6];
	uint32_t scratch0;
	uint32_t scratch1;
	uint32_t reserved2[2];
	uint32_t apbwait;
	uint32_t bustmstrarb;
	uint32_t bootmodeclr;
	uint32_t reserved3[9];
	uint32_t devicecfg;
	uint32_t vidclkdiv;
	uint32_t mirclkdiv;
	uint32_t i2sclkdiv;
	uint32_t keytchclkdiv;
	uint32_t chipid;
	uint32_t reserved4;
	uint32_t syscfg;
	uint32_t reserved5[8];
	uint32_t sysswlock;
};
#else
#define SYSCON_SCRATCH0		(SYSCON_BASE + 0x0040)
#endif

#define SYSCON_OFF_CLKSET1			0x0020
#define SYSCON_OFF_SYSCFG			0x009c

#define SYSCON_PWRCNT_UART_BAUD			(1 << 29)
#define SYSCON_PWRCNT_USH_EN			(1 << 28)

#define SYSCON_CLKSET_PLL_X2IPD_SHIFT		0
#define SYSCON_CLKSET_PLL_X2FBD2_SHIFT		5
#define SYSCON_CLKSET_PLL_X1FBD1_SHIFT		11
#define SYSCON_CLKSET_PLL_PS_SHIFT		16
#define SYSCON_CLKSET1_PCLK_DIV_SHIFT		18
#define SYSCON_CLKSET1_HCLK_DIV_SHIFT		20
#define SYSCON_CLKSET1_NBYP1			(1 << 23)
#define SYSCON_CLKSET1_FCLK_DIV_SHIFT		25

#define SYSCON_CLKSET2_PLL2_EN			(1 << 18)
#define SYSCON_CLKSET2_NBYP2			(1 << 19)
#define SYSCON_CLKSET2_USB_DIV_SHIFT		28

#define SYSCON_CHIPID_REV_MASK			0xF0000000
#define SYSCON_DEVICECFG_SWRST			(1 << 31)

#define SYSCON_SYSCFG_LASDO			0x00000020

/*
 * 0x80930000 - 0x8093FFFF: Watchdog Timer
 */
#define WATCHDOG_OFFSET		0x940000
#define WATCHDOG_BASE		(EP93XX_APB_BASE | WATCHDOG_OFFSET)

/*
 * 0x80950000 - 0x9000FFFF: Reserved
 */

/*
 * During low_level init we store memory layout in memory at specific location
 */
#define UBOOT_MEMORYCNF_BANK_SIZE		0x2000
#define UBOOT_MEMORYCNF_BANK_MASK		0x2004
#define UBOOT_MEMORYCNF_BANK_COUNT		0x2008
