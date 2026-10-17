/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __TI_PRUSS_H
#define __TI_PRUSS_H

/*
 * PRU_ICSS_CFG registers
 * SYSCFG, ISRP, ISP, IESP, IECP, SCRP applicable on AMxxxx devices only
 */
#define PRUSS_CFG_REVID		0x00
#define PRUSS_CFG_SYSCFG	0x04
#define PRUSS_CFG_GPCFG(x)	(0x08 + (x) * 4)
#define PRUSS_CFG_CGR		0x10
#define PRUSS_CFG_ISRP		0x14
#define PRUSS_CFG_ISP		0x18
#define PRUSS_CFG_IESP		0x1C
#define PRUSS_CFG_IECP		0x20
#define PRUSS_CFG_SCRP		0x24
#define PRUSS_CFG_PMAO		0x28
#define PRUSS_CFG_MII_RT	0x2C
#define PRUSS_CFG_IEPCLK	0x30
#define PRUSS_CFG_SPP		0x34
#define PRUSS_CFG_PIN_MX	0x40

/* PRUSS_GPCFG register bits */
#define PRUSS_GPCFG_PRU_GPO_SH_SEL		BIT(25)

#define PRUSS_GPCFG_PRU_DIV1_SHIFT		20
#define PRUSS_GPCFG_PRU_DIV1_MASK		GENMASK(24, 20)

#define PRUSS_GPCFG_PRU_DIV0_SHIFT		15
#define PRUSS_GPCFG_PRU_DIV0_MASK		GENMASK(15, 19)

#define PRUSS_GPCFG_PRU_GPO_MODE		BIT(14)
#define PRUSS_GPCFG_PRU_GPO_MODE_DIRECT		0
#define PRUSS_GPCFG_PRU_GPO_MODE_SERIAL		BIT(14)

#define PRUSS_GPCFG_PRU_GPI_SB			BIT(13)

#define PRUSS_GPCFG_PRU_GPI_DIV1_SHIFT		8
#define PRUSS_GPCFG_PRU_GPI_DIV1_MASK		GENMASK(12, 8)

#define PRUSS_GPCFG_PRU_GPI_DIV0_SHIFT		3
#define PRUSS_GPCFG_PRU_GPI_DIV0_MASK		GENMASK(7, 3)

#define PRUSS_GPCFG_PRU_GPI_CLK_MODE_POSITIVE	0
#define PRUSS_GPCFG_PRU_GPI_CLK_MODE_NEGATIVE	BIT(2)
#define PRUSS_GPCFG_PRU_GPI_CLK_MODE		BIT(2)

#define PRUSS_GPCFG_PRU_GPI_MODE_MASK		GENMASK(1, 0)
#define PRUSS_GPCFG_PRU_GPI_MODE_SHIFT		0

#define PRUSS_GPCFG_PRU_MUX_SEL_SHIFT		26
#define PRUSS_GPCFG_PRU_MUX_SEL_MASK		GENMASK(29, 26)

/* PRUSS_MII_RT register bits */
#define PRUSS_MII_RT_EVENT_EN			BIT(0)

/* PRUSS_SPP register bits */
#define PRUSS_SPP_PRU1_PAD_HP_EN		BIT(0)
#define PRUSS_SPP_XFER_SHIFT_EN			BIT(1)
#define PRUSS_SPP_XFR_BYTE_SHIFT_EN		BIT(2)
#define PRUSS_SPP_RTU_XFR_SHIFT_EN		BIT(3)

/**
 * enum pruss_gp_mux_sel - PRUSS GPI/O Mux modes for the
 * PRUSS_GPCFG0/1 registers
 *
 * NOTE: The below defines are the most common values, but there
 * are some exceptions like on 66AK2G, where the RESERVED and MII2
 * values are interchanged. Also, this bit-field does not exist on
 * AM335x SoCs
 */
enum pruss_gp_mux_sel {
	PRUSS_GP_MUX_SEL_GP = 0,
	PRUSS_GP_MUX_SEL_ENDAT,
	PRUSS_GP_MUX_SEL_RESERVED,
	PRUSS_GP_MUX_SEL_SD,
	PRUSS_GP_MUX_SEL_MII2,
	PRUSS_GP_MUX_SEL_MAX,
};

/**
 * enum pruss_gpi_mode - PRUSS GPI configuration modes, used
 *			 to program the PRUSS_GPCFG0/1 registers
 */
enum pruss_gpi_mode {
	PRUSS_GPI_MODE_DIRECT = 0,
	PRUSS_GPI_MODE_PARALLEL,
	PRUSS_GPI_MODE_28BIT_SHIFT,
	PRUSS_GPI_MODE_MII,
};

/**
 * enum pruss_pru_id - PRU core identifiers
 */
enum pruss_pru_id {
	PRUSS_PRU0 = 0,
	PRUSS_PRU1,
	PRUSS_NUM_PRUS,
};

/**
 * enum pru_ctable_idx - Configurable Constant table index identifiers
 */
enum pru_ctable_idx {
	PRU_C24 = 0,
	PRU_C25,
	PRU_C26,
	PRU_C27,
	PRU_C28,
	PRU_C29,
	PRU_C30,
	PRU_C31,
};

/**
 * enum pruss_mem - PRUSS memory range identifiers
 */
enum pruss_mem {
	PRUSS_MEM_DRAM0 = 0,
	PRUSS_MEM_DRAM1,
	PRUSS_MEM_SHRD_RAM2,
	PRUSS_MEM_MAX,
};

/**
 * struct pruss_mem_region - PRUSS memory region structure
 * @va: kernel virtual address of the PRUSS memory region
 * @pa: physical (bus) address of the PRUSS memory region
 * @size: size of the PRUSS memory region
 */
struct pruss_mem_region {
	void __iomem *va;
	phys_addr_t pa;
	size_t size;
};

/**
 * struct pruss - PRUSS parent structure
 * @dev: pruss device pointer
 * @cfg: regmap for config region
 * @mem_regions: data for each of the PRUSS memory regions
 * @mem_in_use: to indicate if memory resource is in use
 */
struct pruss {
	struct udevice *dev;
	struct regmap *cfg;
	struct pruss_mem_region mem_regions[PRUSS_MEM_MAX];
	struct pruss_mem_region *mem_in_use[PRUSS_MEM_MAX];
};

int pruss_request_tm_region(struct udevice *dev, phys_addr_t *loc);
int pruss_request_mem_region(struct udevice *dev, enum pruss_mem mem_id,
			     struct pruss_mem_region *region);
int pruss_release_mem_region(struct udevice *dev, struct pruss_mem_region *region);
int pruss_cfg_update(struct udevice *dev, unsigned int reg,
		     unsigned int mask, unsigned int val);

/**
 * pruss_cfg_gpimode() - set the GPI mode of the PRU
 * @dev: the pruss device
 * @pru_id: the rproc instance handle of the PRU
 * @mode: GPI mode to set
 *
 * Sets the GPI mode for a given PRU by programming the
 * corresponding PRUSS_CFG_GPCFGx register
 *
 * Returns 0 on success, or an error code otherwise
 */
static inline int pruss_cfg_gpimode(struct udevice *dev, enum pruss_pru_id id,
				    enum pruss_gpi_mode mode)
{
	if (id < 0)
		return -EINVAL;

	return pruss_cfg_update(dev, PRUSS_CFG_GPCFG(id),
				PRUSS_GPCFG_PRU_GPI_MODE_MASK,
				mode << PRUSS_GPCFG_PRU_GPI_MODE_SHIFT);
}

/**
 * pruss_cfg_miirt_enable() - Enable/disable MII RT Events
 * @dev: the pruss device
 * @enable: enable/disable
 *
 * Enable/disable the MII RT Events for the PRUSS.
 */
static inline int pruss_cfg_miirt_enable(struct udevice *dev, bool enable)
{
	u32 set = enable ? PRUSS_MII_RT_EVENT_EN : 0;

	return pruss_cfg_update(dev, PRUSS_CFG_MII_RT,
				PRUSS_MII_RT_EVENT_EN, set);
}

/**
 * pruss_cfg_xfr_enable() - Enable/disable XIN XOUT shift functionality
 * @dev: the pruss device
 * @enable: enable/disable
 */
static inline int pruss_cfg_xfr_enable(struct udevice *dev, bool enable)
{
	u32 set = enable ? PRUSS_SPP_XFER_SHIFT_EN : 0;

	return pruss_cfg_update(dev, PRUSS_CFG_SPP,
			PRUSS_SPP_XFER_SHIFT_EN, set);
}

/**
 * pruss_cfg_set_gpmux() - set the GPMUX value for a PRU device
 * @pruss: pruss device
 * @pru_id: PRU identifier (0-1)
 * @mux: new mux value for PRU
 */
static inline int pruss_cfg_set_gpmux(struct udevice *dev,
				      enum pruss_pru_id id, u8 mux)
{
	if (mux >= PRUSS_GP_MUX_SEL_MAX)
		return -EINVAL;

	return pruss_cfg_update(dev, PRUSS_CFG_GPCFG(id),
			PRUSS_GPCFG_PRU_MUX_SEL_MASK,
			(u32)mux << PRUSS_GPCFG_PRU_MUX_SEL_SHIFT);
}

#endif /* __TI_PRUSS_H */
