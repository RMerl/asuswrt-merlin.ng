/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Texas Instruments GPMC Driver
 *
 * Copyright (C) 2021 Texas Instruments Incorporated - https://www.ti.com/
 */

/* GPMC register offsets */
#define GPMC_REVISION		0x00
#define GPMC_SYSCONFIG		0x10
#define GPMC_SYSSTATUS		0x14
#define GPMC_IRQSTATUS		0x18
#define GPMC_IRQENABLE		0x1c
#define GPMC_TIMEOUT_CONTROL	0x40
#define GPMC_ERR_ADDRESS	0x44
#define GPMC_ERR_TYPE		0x48
#define GPMC_CONFIG		0x50
#define GPMC_STATUS		0x54
#define GPMC_PREFETCH_CONFIG1	0x1e0
#define GPMC_PREFETCH_CONFIG2	0x1e4
#define GPMC_PREFETCH_CONTROL	0x1ec
#define GPMC_PREFETCH_STATUS	0x1f0
#define GPMC_ECC_CONFIG		0x1f4
#define GPMC_ECC_CONTROL	0x1f8
#define GPMC_ECC_SIZE_CONFIG	0x1fc
#define GPMC_ECC1_RESULT        0x200
#define GPMC_ECC_BCH_RESULT_0   0x240   /* not available on OMAP2 */
#define	GPMC_ECC_BCH_RESULT_1	0x244	/* not available on OMAP2 */
#define	GPMC_ECC_BCH_RESULT_2	0x248	/* not available on OMAP2 */
#define	GPMC_ECC_BCH_RESULT_3	0x24c	/* not available on OMAP2 */
#define	GPMC_ECC_BCH_RESULT_4	0x300	/* not available on OMAP2 */
#define	GPMC_ECC_BCH_RESULT_5	0x304	/* not available on OMAP2 */
#define	GPMC_ECC_BCH_RESULT_6	0x308	/* not available on OMAP2 */

/* GPMC ECC control settings */
#define GPMC_ECC_CTRL_ECCCLEAR		0x100
#define GPMC_ECC_CTRL_ECCDISABLE	0x000
#define GPMC_ECC_CTRL_ECCREG1		0x001
#define GPMC_ECC_CTRL_ECCREG2		0x002
#define GPMC_ECC_CTRL_ECCREG3		0x003
#define GPMC_ECC_CTRL_ECCREG4		0x004
#define GPMC_ECC_CTRL_ECCREG5		0x005
#define GPMC_ECC_CTRL_ECCREG6		0x006
#define GPMC_ECC_CTRL_ECCREG7		0x007
#define GPMC_ECC_CTRL_ECCREG8		0x008
#define GPMC_ECC_CTRL_ECCREG9		0x009

#define GPMC_CONFIG_LIMITEDADDRESS		BIT(1)

#define GPMC_STATUS_EMPTYWRITEBUFFERSTATUS	BIT(0)

#define	GPMC_CONFIG2_CSEXTRADELAY		BIT(7)
#define	GPMC_CONFIG3_ADVEXTRADELAY		BIT(7)
#define	GPMC_CONFIG4_OEEXTRADELAY		BIT(7)
#define	GPMC_CONFIG4_WEEXTRADELAY		BIT(23)
#define	GPMC_CONFIG6_CYCLE2CYCLEDIFFCSEN	BIT(6)
#define	GPMC_CONFIG6_CYCLE2CYCLESAMECSEN	BIT(7)

#define GPMC_CS0_OFFSET		0x60
#define GPMC_CS_SIZE		0x30
#define	GPMC_BCH_SIZE		0x10

/*
 * The first 1MB of GPMC address space is typically mapped to
 * the internal ROM. Never allocate the first page, to
 * facilitate bug detection; even if we didn't boot from ROM.
 * As GPMC minimum partition size is 16MB we can only start from
 * there.
 */
#define GPMC_MEM_START		0x1000000
#define GPMC_MEM_END		0x3FFFFFFF

#define GPMC_CHUNK_SHIFT	24		/* 16 MB */
#define GPMC_SECTION_SHIFT	28		/* 128 MB */

#define CS_NUM_SHIFT		24
#define ENABLE_PREFETCH		(0x1 << 7)
#define DMA_MPU_MODE		2

#define	GPMC_REVISION_MAJOR(l)		(((l) >> 4) & 0xf)
#define	GPMC_REVISION_MINOR(l)		((l) & 0xf)

#define	GPMC_HAS_WR_ACCESS		0x1
#define	GPMC_HAS_WR_DATA_MUX_BUS	0x2
#define	GPMC_HAS_MUX_AAD		0x4

#define GPMC_NR_WAITPINS		4

#define GPMC_CS_CONFIG1		0x00
#define GPMC_CS_CONFIG2		0x04
#define GPMC_CS_CONFIG3		0x08
#define GPMC_CS_CONFIG4		0x0c
#define GPMC_CS_CONFIG5		0x10
#define GPMC_CS_CONFIG6		0x14
#define GPMC_CS_CONFIG7		0x18
#define GPMC_CS_NAND_COMMAND	0x1c
#define GPMC_CS_NAND_ADDRESS	0x20
#define GPMC_CS_NAND_DATA	0x24

/* Control Commands */
#define GPMC_CONFIG_RDY_BSY	0x00000001
#define GPMC_CONFIG_DEV_SIZE	0x00000002
#define GPMC_CONFIG_DEV_TYPE	0x00000003

#define GPMC_CONFIG_WP          0x00000005

#define GPMC_CONFIG1_WRAPBURST_SUPP     BIT(31)
#define GPMC_CONFIG1_READMULTIPLE_SUPP  BIT(30)
#define GPMC_CONFIG1_READTYPE_ASYNC     (0 << 29)
#define GPMC_CONFIG1_READTYPE_SYNC      BIT(29)
#define GPMC_CONFIG1_WRITEMULTIPLE_SUPP BIT(28)
#define GPMC_CONFIG1_WRITETYPE_ASYNC    (0 << 27)
#define GPMC_CONFIG1_WRITETYPE_SYNC     BIT(27)
#define GPMC_CONFIG1_CLKACTIVATIONTIME(val) (((val) & 3) << 25)
/** CLKACTIVATIONTIME Max Ticks */
#define GPMC_CONFIG1_CLKACTIVATIONTIME_MAX 2
#define GPMC_CONFIG1_PAGE_LEN(val)      (((val) & 3) << 23)
/** ATTACHEDDEVICEPAGELENGTH Max Value */
#define GPMC_CONFIG1_ATTACHEDDEVICEPAGELENGTH_MAX 2
#define GPMC_CONFIG1_WAIT_READ_MON      BIT(22)
#define GPMC_CONFIG1_WAIT_WRITE_MON     BIT(21)
#define GPMC_CONFIG1_WAIT_MON_TIME(val) (((val) & 3) << 18)
/** WAITMONITORINGTIME Max Ticks */
#define GPMC_CONFIG1_WAITMONITORINGTIME_MAX  2
#define GPMC_CONFIG1_WAIT_PIN_SEL(val)  (((val) & 3) << 16)
#define GPMC_CONFIG1_DEVICESIZE(val)    (((val) & 3) << 12)
#define GPMC_CONFIG1_DEVICESIZE_16      GPMC_CONFIG1_DEVICESIZE(1)
/** DEVICESIZE Max Value */
#define GPMC_CONFIG1_DEVICESIZE_MAX     1
#define GPMC_CONFIG1_DEVICETYPE(val)    (((val) & 3) << 10)
#define GPMC_CONFIG1_DEVICETYPE_NOR     GPMC_CONFIG1_DEVICETYPE(0)
#define GPMC_CONFIG1_MUXTYPE(val)       (((val) & 3) << 8)
#define GPMC_CONFIG1_TIME_PARA_GRAN     BIT(4)
#define GPMC_CONFIG1_FCLK_DIV(val)      ((val) & 3)
#define GPMC_CONFIG1_FCLK_DIV2          (GPMC_CONFIG1_FCLK_DIV(1))
#define GPMC_CONFIG1_FCLK_DIV3          (GPMC_CONFIG1_FCLK_DIV(2))
#define GPMC_CONFIG1_FCLK_DIV4          (GPMC_CONFIG1_FCLK_DIV(3))
#define GPMC_CONFIG7_CSVALID		BIT(6)

#define GPMC_CONFIG7_BASEADDRESS_MASK	0x3f
#define GPMC_CONFIG7_CSVALID_MASK	BIT(6)
#define GPMC_CONFIG7_MASKADDRESS_OFFSET	8
#define GPMC_CONFIG7_MASKADDRESS_MASK	(0xf << GPMC_CONFIG7_MASKADDRESS_OFFSET)
/* All CONFIG7 bits except reserved bits */
#define GPMC_CONFIG7_MASK		(GPMC_CONFIG7_BASEADDRESS_MASK | \
					 GPMC_CONFIG7_CSVALID_MASK |     \
					 GPMC_CONFIG7_MASKADDRESS_MASK)

#define GPMC_DEVICETYPE_NOR		0
#define GPMC_DEVICETYPE_NAND		2
#define GPMC_CONFIG_WRITEPROTECT	0x00000010
#define WR_RD_PIN_MONITORING		0x00600000

/* ECC commands */
#define GPMC_ECC_READ		0 /* Reset Hardware ECC for read */
#define GPMC_ECC_WRITE		1 /* Reset Hardware ECC for write */
#define GPMC_ECC_READSYN	2 /* Reset before syndrom is read back */

#define	GPMC_NR_NAND_IRQS	2 /* number of NAND specific IRQs */

/* bool type time settings */
struct gpmc_bool_timings {
	bool cycle2cyclediffcsen;
	bool cycle2cyclesamecsen;
	bool we_extra_delay;
	bool oe_extra_delay;
	bool adv_extra_delay;
	bool cs_extra_delay;
	bool time_para_granularity;
};

/*
 * Note that all values in this struct are in nanoseconds except sync_clk
 * (which is in picoseconds), while the register values are in gpmc_fck cycles.
 */
struct gpmc_timings {
	/* Minimum clock period for synchronous mode (in picoseconds) */
	u32 sync_clk;

	/* Chip-select signal timings corresponding to GPMC_CS_CONFIG2 */
	u32 cs_on;		/* Assertion time */
	u32 cs_rd_off;		/* Read deassertion time */
	u32 cs_wr_off;		/* Write deassertion time */

	/* ADV signal timings corresponding to GPMC_CONFIG3 */
	u32 adv_on;		/* Assertion time */
	u32 adv_rd_off;		/* Read deassertion time */
	u32 adv_wr_off;		/* Write deassertion time */
	u32 adv_aad_mux_on;	/* ADV assertion time for AAD */
	u32 adv_aad_mux_rd_off;	/* ADV read deassertion time for AAD */
	u32 adv_aad_mux_wr_off;	/* ADV write deassertion time for AAD */

	/* WE signals timings corresponding to GPMC_CONFIG4 */
	u32 we_on;		/* WE assertion time */
	u32 we_off;		/* WE deassertion time */

	/* OE signals timings corresponding to GPMC_CONFIG4 */
	u32 oe_on;		/* OE assertion time */
	u32 oe_off;		/* OE deassertion time */
	u32 oe_aad_mux_on;	/* OE assertion time for AAD */
	u32 oe_aad_mux_off;	/* OE deassertion time for AAD */

	/* Access time and cycle time timings corresponding to GPMC_CONFIG5 */
	u32 page_burst_access;	/* Multiple access word delay */
	u32 access;		/* Start-cycle to first data valid delay */
	u32 rd_cycle;		/* Total read cycle time */
	u32 wr_cycle;		/* Total write cycle time */

	u32 bus_turnaround;
	u32 cycle2cycle_delay;

	u32 wait_monitoring;
	u32 clk_activation;

	/* The following are only on OMAP3430 */
	u32 wr_access;		/* WRACCESSTIME */
	u32 wr_data_mux_bus;	/* WRDATAONADMUXBUS */

	struct gpmc_bool_timings bool_timings;
};

/* Device timings in picoseconds */
struct gpmc_device_timings {
	u32 t_ceasu;	/* address setup to CS valid */
	u32 t_avdasu;	/* address setup to ADV valid */
	/* XXX: try to combine t_avdp_r & t_avdp_w. Issue is
	 * of tusb using these timings even for sync whilst
	 * ideally for adv_rd/(wr)_off it should have considered
	 * t_avdh instead. This indirectly necessitates r/w
	 * variations of t_avdp as it is possible to have one
	 * sync & other async
	 */
	u32 t_avdp_r;	/* ADV low time (what about t_cer ?) */
	u32 t_avdp_w;
	u32 t_aavdh;	/* address hold time */
	u32 t_oeasu;	/* address setup to OE valid */
	u32 t_aa;	/* access time from ADV assertion */
	u32 t_iaa;	/* initial access time */
	u32 t_oe;	/* access time from OE assertion */
	u32 t_ce;	/* access time from CS asertion */
	u32 t_rd_cycle;	/* read cycle time */
	u32 t_cez_r;	/* read CS deassertion to high Z */
	u32 t_cez_w;	/* write CS deassertion to high Z */
	u32 t_oez;	/* OE deassertion to high Z */
	u32 t_weasu;	/* address setup to WE valid */
	u32 t_wpl;	/* write assertion time */
	u32 t_wph;	/* write deassertion time */
	u32 t_wr_cycle;	/* write cycle time */

	u32 clk;
	u32 t_bacc;	/* burst access valid clock to output delay */
	u32 t_ces;	/* CS setup time to clk */
	u32 t_avds;	/* ADV setup time to clk */
	u32 t_avdh;	/* ADV hold time from clk */
	u32 t_ach;	/* address hold time from clk */
	u32 t_rdyo;	/* clk to ready valid */

	u32 t_ce_rdyz;	/* XXX: description ?, or use t_cez instead */
	u32 t_ce_avd;	/* CS on to ADV on delay */

	/* XXX: check the possibility of combining
	 * cyc_aavhd_oe & cyc_aavdh_we
	 */
	u8 cyc_aavdh_oe;/* read address hold time in cycles */
	u8 cyc_aavdh_we;/* write address hold time in cycles */
	u8 cyc_oe;	/* access time from OE assertion in cycles */
	u8 cyc_wpl;	/* write deassertion time in cycles */
	u32 cyc_iaa;	/* initial access time in cycles */

	/* extra delays */
	bool ce_xdelay;
	bool avd_xdelay;
	bool oe_xdelay;
	bool we_xdelay;
};

#define GPMC_BURST_4			4	/* 4 word burst */
#define GPMC_BURST_8			8	/* 8 word burst */
#define GPMC_BURST_16			16	/* 16 word burst */
#define GPMC_DEVWIDTH_8BIT		1	/* 8-bit device width */
#define GPMC_DEVWIDTH_16BIT		2	/* 16-bit device width */
#define GPMC_MUX_AAD			1	/* Addr-Addr-Data multiplex */
#define GPMC_MUX_AD			2	/* Addr-Data multiplex */

struct gpmc_settings {
	bool burst_wrap;	/* enables wrap bursting */
	bool burst_read;	/* enables read page/burst mode */
	bool burst_write;	/* enables write page/burst mode */
	bool device_nand;	/* device is NAND */
	bool sync_read;		/* enables synchronous reads */
	bool sync_write;	/* enables synchronous writes */
	bool wait_on_read;	/* monitor wait on reads */
	bool wait_on_write;	/* monitor wait on writes */
	u32 burst_len;		/* page/burst length */
	u32 device_width;	/* device bus width (8 or 16 bit) */
	u32 mux_add_data;	/* multiplex address & data */
	u32 wait_pin;		/* wait-pin to be used */
};
