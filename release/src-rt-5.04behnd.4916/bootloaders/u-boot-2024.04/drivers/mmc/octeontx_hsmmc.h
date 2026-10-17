/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */
#ifndef __OCTEONTX_HSMMC_H__
#define __OCTEONTX_HSMMC_H__
#include <asm/gpio.h>

/** Name of our driver */
#define OCTEONTX_MMC_DRIVER_NAME	"octeontx-hsmmc"

/** Maximum supported MMC slots */
#define OCTEONTX_MAX_MMC_SLOT		3

#define POWER_ON_TIME			40 /** See SD 4.1 spec figure 6-5 */

/**
 * Timeout used when waiting for commands to complete.  We need to keep this
 * above the hardware watchdog timeout which is usually limited to 1000ms
 */
#define WATCHDOG_COUNT			(1100)	/* in msecs */

/**
 * Long timeout for commands which might take a while to complete.
 */
#define MMC_TIMEOUT_LONG		1000

/**
 * Short timeout used for most commands in msecs
 */
#define MMC_TIMEOUT_SHORT		20

#define MAX_NO_OF_TAPS			64

#define EXT_CSD_POWER_CLASS		187	/* R/W */

/* default HS400 tuning block number */
#define DEFAULT_HS400_TUNING_BLOCK	1

struct octeontx_mmc_host;

/** MMC/SD slot data structure */
struct octeontx_mmc_slot {
	struct mmc		mmc;
	struct mmc_config	cfg;
	struct octeontx_mmc_host *host;
	struct udevice		*dev;
	void			*base_addr;	/** Same as host base_addr */
	u64			clock;
	int			bus_id;		/** slot number */
	uint			bus_width;
	uint			max_width;
	int			hs200_tap_adj;
	int			hs400_tap_adj;
	int			hs400_tuning_block;
	struct gpio_desc	cd_gpio;
	struct gpio_desc	wp_gpio;
	struct gpio_desc	power_gpio;
	enum bus_mode		mode;
	union mio_emm_switch	cached_switch;
	union mio_emm_switch	want_switch;
	union mio_emm_rca	cached_rca;
	union mio_emm_timing	taps;	/* otx2: MIO_EMM_TIMING */
	union mio_emm_timing	hs200_taps;
	union mio_emm_timing	hs400_taps;
	/* These are used to see if our tuning is still valid or not */
	enum bus_mode		last_mode;
	u32			last_clock;
	u32			block_len;
	u32			block_count;
	int			cmd_clk_skew;
	int			dat_clk_skew;
	uint			cmd_cnt;	/* otx: sample cmd in delay */
	uint			dat_cnt;	/* otx: sample data in delay */
	uint			drive;		/* Current drive */
	uint			slew;		/* clock skew */
	uint			cmd_out_hs200_delay;
	uint			data_out_hs200_delay;
	uint			cmd_out_hs400_delay;
	uint			data_out_hs400_delay;
	uint			clk_period;
	bool			valid:1;
	bool			is_acmd:1;
	bool			tuned:1;
	bool			hs200_tuned:1;
	bool			hs400_tuned:1;
	bool			is_1_8v:1;
	bool			is_3_3v:1;
	bool			is_ddr:1;
	bool			is_asim:1;
	bool			is_emul:1;
	bool			cd_inverted:1;
	bool			wp_inverted:1;
	bool			disable_ddr:1;
	bool			non_removable:1;
};

struct octeontx_mmc_cr_mods {
	u8 ctype_xor;
	u8 rtype_xor;
};

struct octeontx_mmc_cr {
	u8 c;
	u8 r;
};

struct octeontx_sd_mods {
	struct octeontx_mmc_cr mmc;
	struct octeontx_mmc_cr sd;
	struct octeontx_mmc_cr sdacmd;
};

/** Host controller data structure */
struct octeontx_mmc_host {
	struct		udevice *dev;
	void		*base_addr;
	struct octeontx_mmc_slot slots[OCTEONTX_MAX_MMC_SLOT + 1];
	pci_dev_t	pdev;
	u64		sys_freq;
	union mio_emm_cfg emm_cfg;
	u64		timing_taps;
	struct mmc	*last_mmc;	/** Last mmc used */
	ofnode		node;
	int		cur_slotid;
	int		last_slotid;
	int		max_width;
	uint		per_tap_delay;
	uint		num_slots;
	uint		dma_wait_delay;	/* Delay before polling DMA in usecs */
	bool		initialized:1;
	bool		timing_calibrated:1;
	bool		is_asim:1;
	bool		is_emul:1;
	bool		calibrate_glitch:1;
	bool		cond_clock_glitch:1;
	bool		tap_requires_noclk:1;
	bool		hs400_skew_needed:1;
};

/*
 * NOTE: This was copied from the Linux kernel.
 *
 * MMC status in R1, for native mode (SPI bits are different)
 * Type
 *	e:error bit
 *	s:status bit
 *	r:detected and set for the actual command response
 *	x:detected and set during command execution. the host must poll
 *	    the card by sending status command in order to read these bits.
 * Clear condition
 *	a:according to the card state
 *	b:always related to the previous command. Reception of
 *	    a valid command will clear it (with a delay of one command)
 *	c:clear by read
 */
#define R1_OUT_OF_RANGE		BIT(31)		/* er, c */
#define R1_ADDRESS_ERROR	BIT(30)		/* erx, c */
#define R1_BLOCK_LEN_ERROR	BIT(29)		/* er, c */
#define R1_ERASE_SEQ_ERROR	BIT(28)		/* er, c */
#define R1_ERASE_PARAM          BIT(27)		/* ex, c */
#define R1_WP_VIOLATION		BIT(26)		/* erx, c */
#define R1_CARD_IS_LOCKED	BIT(25)		/* sx, a */
#define R1_LOCK_UNLOCK_FAILED	BIT(24)		/* erx, c */
#define R1_COM_CRC_ERROR	BIT(23)		/* er, b */
/*#define R1_ILLEGAL_COMMAND	BIT(22)*/		/* er, b */
#define R1_CARD_ECC_FAILED	BIT(21)		/* ex, c */
#define R1_CC_ERROR		BIT(20)		/* erx, c */
#define R1_ERROR		BIT(19)		/* erx, c */
#define R1_UNDERRUN		BIT(18)		/* ex, c */
#define R1_OVERRUN		BIT(17)		/* ex, c */
#define R1_CID_CSD_OVERWRITE	BIT(16)		/* erx, c, CID/CSD overwrite */
#define R1_WP_ERASE_SKIP	BIT(15)		/* sx, c */
#define R1_CARD_ECC_DISABLED	BIT(14)		/* sx, a */
#define R1_ERASE_RESET		BIT(13)		/* sr, c */
#define R1_STATUS(x)		((x) & 0xFFFFE000)
#define R1_CURRENT_STATE(x)	(((x) & 0x00001E00) >> 9) /* sx, b (4 bits) */
#define R1_READY_FOR_DATA	BIT(8)		/* sx, a */
#define R1_SWITCH_ERROR		BIT(7)		/* sx, c */

#define R1_BLOCK_READ_MASK	R1_OUT_OF_RANGE |	\
				R1_ADDRESS_ERROR |	\
				R1_BLOCK_LEN_ERROR |	\
				R1_CARD_IS_LOCKED |	\
				R1_COM_CRC_ERROR |	\
				R1_ILLEGAL_COMMAND |	\
				R1_CARD_ECC_FAILED |	\
				R1_CC_ERROR |		\
				R1_ERROR
#define R1_BLOCK_WRITE_MASK	R1_OUT_OF_RANGE |	\
				R1_ADDRESS_ERROR |	\
				R1_BLOCK_LEN_ERROR |	\
				R1_WP_VIOLATION |	\
				R1_CARD_IS_LOCKED |	\
				R1_COM_CRC_ERROR |	\
				R1_ILLEGAL_COMMAND |	\
				R1_CARD_ECC_FAILED |	\
				R1_CC_ERROR |		\
				R1_ERROR |		\
				R1_UNDERRUN |		\
				R1_OVERRUN

#endif /* __OCTEONTX_HSMMC_H__ */
