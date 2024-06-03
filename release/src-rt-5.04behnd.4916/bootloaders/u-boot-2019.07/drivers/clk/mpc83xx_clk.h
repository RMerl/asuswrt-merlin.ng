/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

/**
 * enum ratio - Description of a core clock ratio
 * @RAT_UNK:      Unknown ratio
 * @RAT_BYP:      Bypass
 * @RAT_1_TO_8:   Ratio 1:8
 * @RAT_1_TO_4:   Ratio 1:4
 * @RAT_1_TO_2:   Ratio 1:2
 * @RAT_1_TO_1:   Ratio 1:1
 * @RAT_1_5_TO_1: Ratio 1.5:1
 * @RAT_2_TO_1:   Ratio 2:1
 * @RAT_2_5_TO_1: Ratio 2.5:1
 * @RAT_3_TO_1:   Ratio 3:1
 */
enum ratio {
	RAT_UNK,
	RAT_BYP,
	RAT_1_TO_8,
	RAT_1_TO_4,
	RAT_1_TO_2,
	RAT_1_TO_1,
	RAT_1_5_TO_1,
	RAT_2_TO_1,
	RAT_2_5_TO_1,
	RAT_3_TO_1
};

/**
 * struct corecnf - Description for a core clock configuration
 * @core_csb_ratio: Core clock frequency to CSB clock frequency ratio
 * @vco_divider: VCO divider (Core VCO frequency = Core frequency * VCO divider)
 */
struct corecnf {
	int core_csb_ratio;
	int vco_divider;
};

/*
 * Table with all valid Core CSB frequency ratio / VCO divider combinations as
 * indexed by the COREPLL field of the SPMR
 */
static const struct corecnf corecnf_tab[] = {
	{RAT_BYP, RAT_BYP},			/* 0x00 */
	{RAT_BYP, RAT_BYP},			/* 0x01 */
	{RAT_BYP, RAT_BYP},			/* 0x02 */
	{RAT_BYP, RAT_BYP},			/* 0x03 */
	{RAT_BYP, RAT_BYP},			/* 0x04 */
	{RAT_BYP, RAT_BYP},			/* 0x05 */
	{RAT_BYP, RAT_BYP},			/* 0x06 */
	{RAT_BYP, RAT_BYP},			/* 0x07 */
	{RAT_1_TO_1, RAT_1_TO_2},		/* 0x08 */
	{RAT_1_TO_1, RAT_1_TO_4},		/* 0x09 */
	{RAT_1_TO_1, RAT_1_TO_8},		/* 0x0A */
	{RAT_1_TO_1, RAT_1_TO_8},		/* 0x0B */
	{RAT_1_5_TO_1, RAT_1_TO_2},		/* 0x0C */
	{RAT_1_5_TO_1, RAT_1_TO_4},		/* 0x0D */
	{RAT_1_5_TO_1, RAT_1_TO_8},		/* 0x0E */
	{RAT_1_5_TO_1, RAT_1_TO_8},		/* 0x0F */
	{RAT_2_TO_1, RAT_1_TO_2},		/* 0x10 */
	{RAT_2_TO_1, RAT_1_TO_4},		/* 0x11 */
	{RAT_2_TO_1, RAT_1_TO_8},		/* 0x12 */
	{RAT_2_TO_1, RAT_1_TO_8},		/* 0x13 */
	{RAT_2_5_TO_1, RAT_1_TO_2},		/* 0x14 */
	{RAT_2_5_TO_1, RAT_1_TO_4},		/* 0x15 */
	{RAT_2_5_TO_1, RAT_1_TO_8},		/* 0x16 */
	{RAT_2_5_TO_1, RAT_1_TO_8},		/* 0x17 */
	{RAT_3_TO_1, RAT_1_TO_2},		/* 0x18 */
	{RAT_3_TO_1, RAT_1_TO_4},		/* 0x19 */
	{RAT_3_TO_1, RAT_1_TO_8},		/* 0x1A */
	{RAT_3_TO_1, RAT_1_TO_8},		/* 0x1B */
};

/**
 * enum reg_type - Register to read a field from
 * @REG_SCCR: Use the SCCR register
 * @REG_SPMR: Use the SPMR register
 */
enum reg_type {
	REG_SCCR,
	REG_SPMR,
};

/**
 * enum mode_type - Description of how to read a specific frequency value
 * @TYPE_INVALID: Unknown type, will provoke error
 * @TYPE_SCCR_STANDARD:        Read a field from the SCCR register, and use it
 *			       as a divider for the CSB clock to compute the
 *			       frequency
 * @TYPE_SCCR_ONOFF:           The field describes a bit flag that can turn the
 *			       clock on or off
 * @TYPE_SPMR_DIRECT_MULTIPLY: Read a field from the SPMR register, and use it
 *			       as a multiplier for the CSB clock to compute the
 *			       frequency
 * @TYPE_SPECIAL:              The frequency is calculated in a non-standard way
 */
enum mode_type {
	TYPE_INVALID = 0,
	TYPE_SCCR_STANDARD,
	TYPE_SCCR_ONOFF,
	TYPE_SPMR_DIRECT_MULTIPLY,
	TYPE_SPECIAL,
};

/* Map of each clock index to its human-readable name */
static const char * const names[] = {
	[MPC83XX_CLK_CORE] = "Core",
	[MPC83XX_CLK_CSB] = "Coherent System Bus",
	[MPC83XX_CLK_QE] = "QE",
	[MPC83XX_CLK_BRG] = "BRG",
	[MPC83XX_CLK_LBIU] = "Local Bus Controller",
	[MPC83XX_CLK_LCLK] = "Local Bus",
	[MPC83XX_CLK_MEM] = "DDR",
	[MPC83XX_CLK_MEM_SEC] = "DDR Secondary",
	[MPC83XX_CLK_ENC] = "SEC",
	[MPC83XX_CLK_I2C1] = "I2C1",
	[MPC83XX_CLK_I2C2] = "I2C2",
	[MPC83XX_CLK_TDM] = "TDM",
	[MPC83XX_CLK_SDHC] = "SDHC",
	[MPC83XX_CLK_TSEC1] = "TSEC1",
	[MPC83XX_CLK_TSEC2] = "TSEC2",
	[MPC83XX_CLK_USBDR] = "USB DR",
	[MPC83XX_CLK_USBMPH] = "USB MPH",
	[MPC83XX_CLK_PCIEXP1] = "PCIEXP1",
	[MPC83XX_CLK_PCIEXP2] = "PCIEXP2",
	[MPC83XX_CLK_SATA] = "SATA",
	[MPC83XX_CLK_DMAC] = "DMAC",
	[MPC83XX_CLK_PCI] = "PCI",
};

/**
 * struct clk_mode - Structure for clock mode descriiptions
 * @low:  The low bit of the data field to read for this mode (may not apply to
 *	  some modes)
 * @high: The high bit of the data field to read for this mode (may not apply to
 *	  some modes)
 * @type: The type of the mode description (one of enum mode_type)
 */
struct clk_mode {
	u8 low;
	u8 high;
	int type;
};

/**
 * set_mode() - Build a clock mode description from data
 * @mode: The clock mode description to be filled out
 * @low:  The low bit of the data field to read for this mode (may not apply to
 *	  some modes)
 * @high: The high bit of the data field to read for this mode (may not apply to
 *	  some modes)
 * @type: The type of the mode description (one of enum mode_type)
 *
 * Clock mode descriptions are a succinct description of how to read a specific
 * clock's rate from the hardware; usually by reading a specific field of a
 * register, such a s the SCCR register, but some types use different methods
 * for obtaining the clock rate.
 */
static void set_mode(struct clk_mode *mode, u8 low, u8 high, int type)
{
	mode->low = low;
	mode->high = high;
	mode->type = type;
}

/**
 * retrieve_mode() - Get the clock mode description for a specific clock
 * @clk:      The identifier of the clock for which the clock description should
 *	      be retrieved
 * @soc_type: The type of MPC83xx SoC for which the clock description should be
 *	      retrieved
 * @mode:     Pointer to a clk_mode structure to be filled with data for the
 *	      clock
 *
 * Since some clock rate are stored in different places on different MPC83xx
 * SoCs, the SoC type has to be supplied along with the clock's identifier.
 *
 * Return: 0 if OK, -ve on error
 */
static int retrieve_mode(int clk, int soc_type, struct clk_mode *mode)
{
	switch (clk) {
	case MPC83XX_CLK_CORE:
	case MPC83XX_CLK_CSB:
	case MPC83XX_CLK_QE:
	case MPC83XX_CLK_BRG:
	case MPC83XX_CLK_LCLK:
	case MPC83XX_CLK_I2C2:
		set_mode(mode, 0, 0, TYPE_SPECIAL);
		break;
	case MPC83XX_CLK_MEM:
		set_mode(mode, 1, 1, TYPE_SPMR_DIRECT_MULTIPLY);
		break;
	case MPC83XX_CLK_LBIU:
	case MPC83XX_CLK_MEM_SEC:
		set_mode(mode, 0, 0, TYPE_SPMR_DIRECT_MULTIPLY);
		break;
	case MPC83XX_CLK_TSEC1:
		set_mode(mode, 0, 1, TYPE_SCCR_STANDARD);
		break;
	case MPC83XX_CLK_TSEC2:
		if (soc_type == SOC_MPC8313) /* I2C and TSEC2 are the same register */
			set_mode(mode, 2, 3, TYPE_SCCR_STANDARD);
		else /* FIXME(mario.six@gdsys.cc): This has separate enable/disable bit! */
			set_mode(mode, 0, 1, TYPE_SCCR_STANDARD);
		break;
	case MPC83XX_CLK_SDHC:
		set_mode(mode, 4, 5, TYPE_SCCR_STANDARD);
		break;
	case MPC83XX_CLK_ENC:
		set_mode(mode, 6, 7, TYPE_SCCR_STANDARD);
		break;
	case MPC83XX_CLK_I2C1:
		if (soc_type == SOC_MPC8349)
			set_mode(mode, 2, 3, TYPE_SCCR_STANDARD);
		else /* I2C and ENC are the same register */
			set_mode(mode, 6, 7, TYPE_SCCR_STANDARD);
		break;
	case MPC83XX_CLK_PCIEXP1:
		set_mode(mode, 10, 11, TYPE_SCCR_STANDARD);
		break;
	case MPC83XX_CLK_PCIEXP2:
		set_mode(mode, 12, 13, TYPE_SCCR_STANDARD);
		break;
	case MPC83XX_CLK_USBDR:
		if (soc_type == SOC_MPC8313 || soc_type == SOC_MPC8349)
			set_mode(mode, 10, 11, TYPE_SCCR_STANDARD);
		else
			set_mode(mode, 8, 9, TYPE_SCCR_STANDARD);
		break;
	case MPC83XX_CLK_USBMPH:
		set_mode(mode, 8, 9, TYPE_SCCR_STANDARD);
		break;
	case MPC83XX_CLK_PCI:
		set_mode(mode, 15, 15, TYPE_SCCR_ONOFF);
		break;
	case MPC83XX_CLK_DMAC:
		set_mode(mode, 26, 27, TYPE_SCCR_STANDARD);
		break;
	case MPC83XX_CLK_SATA:
		/* FIXME(mario.six@gdsys.cc): All SATA controllers must have the same clock ratio */
		if (soc_type == SOC_MPC8379) {
			set_mode(mode, 24, 25, TYPE_SCCR_STANDARD);
			set_mode(mode, 26, 27, TYPE_SCCR_STANDARD);
			set_mode(mode, 28, 29, TYPE_SCCR_STANDARD);
			set_mode(mode, 30, 31, TYPE_SCCR_STANDARD);
		} else {
			set_mode(mode, 18, 19, TYPE_SCCR_STANDARD);
			set_mode(mode, 20, 21, TYPE_SCCR_STANDARD);
		}
		break;
	case MPC83XX_CLK_TDM:
		set_mode(mode, 26, 27, TYPE_SCCR_STANDARD);
		break;
	default:
		debug("%s: Unknown clock type %d on soc type %d\n",
		      __func__, clk, soc_type);
		set_mode(mode, 0, 0, TYPE_INVALID);
		return -EINVAL;
	}

	return 0;
}

/**
 * get_spmr() - Read the SPMR (System PLL Mode Register)
 * @im: Pointer to the MPC83xx main register map in question
 *
 * Return: The SPMR value as a 32-bit number.
 */
static inline u32 get_spmr(immap_t *im)
{
	u32 res = in_be32(&im->clk.spmr);

	return res;
}

/**
 * get_sccr() - Read the SCCR (System Clock Control Register)
 * @im: Pointer to the MPC83xx main register map in question
 *
 * Return: The SCCR value as a 32-bit number.
 */
static inline u32 get_sccr(immap_t *im)
{
	u32 res = in_be32(&im->clk.sccr);

	return res;
}

/**
 * get_lcrr() - Read the LCRR (Clock Ratio Register)
 * @im: Pointer to the MPC83xx main register map in question
 *
 * Return: The LCRR value as a 32-bit number.
 */
static inline u32 get_lcrr(immap_t *im)
{
	u32 res = in_be32(&im->im_lbc.lcrr);

	return res;
}

/**
 * get_pci_sync_in() - Read the PCI synchronization clock speed
 * @im: Pointer to the MPC83xx main register map in question
 *
 * Return: The PCI synchronization clock speed value as a 32-bit number.
 */
static inline u32 get_pci_sync_in(immap_t *im)
{
	u8 clkin_div;

	clkin_div = (get_spmr(im) & SPMR_CKID) >> SPMR_CKID_SHIFT;
	return CONFIG_SYS_CLK_FREQ / (1 + clkin_div);
}

/**
 * get_csb_clk() - Read the CSB (Coheren System Bus) clock speed
 * @im: Pointer to the MPC83xx main register map in question
 *
 * Return: The CSB clock speed value as a 32-bit number.
 */
static inline u32 get_csb_clk(immap_t *im)
{
	u8 spmf;

	spmf = (get_spmr(im) & SPMR_SPMF) >> SPMR_SPMF_SHIFT;
	return CONFIG_SYS_CLK_FREQ * spmf;
}

/**
 * spmr_field() - Read a specific SPMR field
 * @im:   Pointer to the MPC83xx main register map in question
 * @mask: A bitmask that describes the bitfield to be read
 *
 * Return: The value of the bit field as a 32-bit number.
 */
static inline uint spmr_field(immap_t *im, u32 mask)
{
	/* Extract shift from bitmask */
	uint shift = mask ? ffs(mask) - 1 : 0;

	return (get_spmr(im) & mask) >> shift;
}

/**
 * sccr_field() - Read a specific SCCR field
 * @im:   Pointer to the MPC83xx main register map in question
 * @mask: A bitmask that describes the bitfield to be read
 *
 * Return: The value of the bit field as a 32-bit number.
 */
static inline uint sccr_field(immap_t *im, u32 mask)
{
	/* Extract shift from bitmask */
	uint shift = mask ? ffs(mask) - 1 : 0;

	return (get_sccr(im) & mask) >> shift;
}

/**
 * lcrr_field() - Read a specific LCRR field
 * @im:   Pointer to the MPC83xx main register map in question
 * @mask: A bitmask that describes the bitfield to be read
 *
 * Return: The value of the bit field as a 32-bit number.
 */
static inline uint lcrr_field(immap_t *im, u32 mask)
{
	/* Extract shift from bitmask */
	uint shift = mask ? ffs(mask) - 1 : 0;

	return (get_lcrr(im) & mask) >> shift;
}
