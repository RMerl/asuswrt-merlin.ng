/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 NXP
 */

#include <phy_interface.h>
#include <linux/bitops.h>

/* Core registers */

#define EQOS_MAC_REGS_BASE 0x000
struct eqos_mac_regs {
	u32 configuration;				/* 0x000 */
	u32 unused_004[(0x070 - 0x004) / 4];	/* 0x004 */
	u32 q0_tx_flow_ctrl;			/* 0x070 */
	u32 unused_070[(0x090 - 0x074) / 4];	/* 0x074 */
	u32 rx_flow_ctrl;				/* 0x090 */
	u32 unused_094;				/* 0x094 */
	u32 txq_prty_map0;				/* 0x098 */
	u32 unused_09c;				/* 0x09c */
	u32 rxq_ctrl0;				/* 0x0a0 */
	u32 unused_0a4;				/* 0x0a4 */
	u32 rxq_ctrl2;				/* 0x0a8 */
	u32 unused_0ac[(0x0dc - 0x0ac) / 4];	/* 0x0ac */
	u32 us_tic_counter;			/* 0x0dc */
	u32 unused_0e0[(0x11c - 0x0e0) / 4];	/* 0x0e0 */
	u32 hw_feature0;				/* 0x11c */
	u32 hw_feature1;				/* 0x120 */
	u32 hw_feature2;				/* 0x124 */
	u32 unused_128[(0x200 - 0x128) / 4];	/* 0x128 */
	u32 mdio_address;				/* 0x200 */
	u32 mdio_data;				/* 0x204 */
	u32 unused_208[(0x300 - 0x208) / 4];	/* 0x208 */
	u32 address0_high;				/* 0x300 */
	u32 address0_low;				/* 0x304 */
};

#define EQOS_MAC_CONFIGURATION_GPSLCE			BIT(23)
#define EQOS_MAC_CONFIGURATION_CST			BIT(21)
#define EQOS_MAC_CONFIGURATION_ACS			BIT(20)
#define EQOS_MAC_CONFIGURATION_WD			BIT(19)
#define EQOS_MAC_CONFIGURATION_JD			BIT(17)
#define EQOS_MAC_CONFIGURATION_JE			BIT(16)
#define EQOS_MAC_CONFIGURATION_PS			BIT(15)
#define EQOS_MAC_CONFIGURATION_FES			BIT(14)
#define EQOS_MAC_CONFIGURATION_DM			BIT(13)
#define EQOS_MAC_CONFIGURATION_LM			BIT(12)
#define EQOS_MAC_CONFIGURATION_TE			BIT(1)
#define EQOS_MAC_CONFIGURATION_RE			BIT(0)

#define EQOS_MAC_Q0_TX_FLOW_CTRL_PT_SHIFT		16
#define EQOS_MAC_Q0_TX_FLOW_CTRL_PT_MASK		0xffff
#define EQOS_MAC_Q0_TX_FLOW_CTRL_TFE			BIT(1)

#define EQOS_MAC_RX_FLOW_CTRL_RFE			BIT(0)

#define EQOS_MAC_TXQ_PRTY_MAP0_PSTQ0_SHIFT		0
#define EQOS_MAC_TXQ_PRTY_MAP0_PSTQ0_MASK		0xff

#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_SHIFT			0
#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_MASK			3
#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_NOT_ENABLED		0
#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB		2
#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_AV		1

#define EQOS_MAC_RXQ_CTRL2_PSRQ0_SHIFT			0
#define EQOS_MAC_RXQ_CTRL2_PSRQ0_MASK			0xff

#define EQOS_MAC_HW_FEATURE0_MMCSEL_SHIFT		8
#define EQOS_MAC_HW_FEATURE0_HDSEL_SHIFT		2
#define EQOS_MAC_HW_FEATURE0_GMIISEL_SHIFT		1
#define EQOS_MAC_HW_FEATURE0_MIISEL_SHIFT		0

#define EQOS_MAC_HW_FEATURE1_TXFIFOSIZE_SHIFT		6
#define EQOS_MAC_HW_FEATURE1_TXFIFOSIZE_MASK		0x1f
#define EQOS_MAC_HW_FEATURE1_RXFIFOSIZE_SHIFT		0
#define EQOS_MAC_HW_FEATURE1_RXFIFOSIZE_MASK		0x1f

#define EQOS_MAC_HW_FEATURE3_ASP_SHIFT			28
#define EQOS_MAC_HW_FEATURE3_ASP_MASK			0x3

#define EQOS_MAC_MDIO_ADDRESS_PA_SHIFT			21
#define EQOS_MAC_MDIO_ADDRESS_RDA_SHIFT			16
#define EQOS_MAC_MDIO_ADDRESS_CR_SHIFT			8
#define EQOS_MAC_MDIO_ADDRESS_CR_100_150		1
#define EQOS_MAC_MDIO_ADDRESS_CR_20_35			2
#define EQOS_MAC_MDIO_ADDRESS_CR_250_300		5
#define EQOS_MAC_MDIO_ADDRESS_SKAP			BIT(4)
#define EQOS_MAC_MDIO_ADDRESS_GOC_SHIFT			2
#define EQOS_MAC_MDIO_ADDRESS_GOC_READ			3
#define EQOS_MAC_MDIO_ADDRESS_GOC_WRITE			1
#define EQOS_MAC_MDIO_ADDRESS_C45E			BIT(1)
#define EQOS_MAC_MDIO_ADDRESS_GB			BIT(0)

#define EQOS_MAC_MDIO_DATA_GD_MASK			0xffff

#define EQOS_MTL_REGS_BASE 0xd00
struct eqos_mtl_regs {
	u32 txq0_operation_mode;			/* 0xd00 */
	u32 unused_d04;				/* 0xd04 */
	u32 txq0_debug;				/* 0xd08 */
	u32 unused_d0c[(0xd18 - 0xd0c) / 4];	/* 0xd0c */
	u32 txq0_quantum_weight;			/* 0xd18 */
	u32 unused_d1c[(0xd30 - 0xd1c) / 4];	/* 0xd1c */
	u32 rxq0_operation_mode;			/* 0xd30 */
	u32 unused_d34;				/* 0xd34 */
	u32 rxq0_debug;				/* 0xd38 */
};

#define EQOS_MTL_TXQ0_OPERATION_MODE_TQS_SHIFT		16
#define EQOS_MTL_TXQ0_OPERATION_MODE_TQS_MASK		0x1ff
#define EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_SHIFT	2
#define EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_MASK		3
#define EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_ENABLED	2
#define EQOS_MTL_TXQ0_OPERATION_MODE_TSF		BIT(1)
#define EQOS_MTL_TXQ0_OPERATION_MODE_FTQ		BIT(0)

#define EQOS_MTL_TXQ0_DEBUG_TXQSTS			BIT(4)
#define EQOS_MTL_TXQ0_DEBUG_TRCSTS_SHIFT		1
#define EQOS_MTL_TXQ0_DEBUG_TRCSTS_MASK			3

#define EQOS_MTL_RXQ0_OPERATION_MODE_RQS_SHIFT		20
#define EQOS_MTL_RXQ0_OPERATION_MODE_RQS_MASK		0x3ff
#define EQOS_MTL_RXQ0_OPERATION_MODE_RFD_SHIFT		14
#define EQOS_MTL_RXQ0_OPERATION_MODE_RFD_MASK		0x3f
#define EQOS_MTL_RXQ0_OPERATION_MODE_RFA_SHIFT		8
#define EQOS_MTL_RXQ0_OPERATION_MODE_RFA_MASK		0x3f
#define EQOS_MTL_RXQ0_OPERATION_MODE_EHFC		BIT(7)
#define EQOS_MTL_RXQ0_OPERATION_MODE_RSF		BIT(5)

#define EQOS_MTL_RXQ0_DEBUG_PRXQ_SHIFT			16
#define EQOS_MTL_RXQ0_DEBUG_PRXQ_MASK			0x7fff
#define EQOS_MTL_RXQ0_DEBUG_RXQSTS_SHIFT		4
#define EQOS_MTL_RXQ0_DEBUG_RXQSTS_MASK			3

#define EQOS_DMA_REGS_BASE 0x1000
struct eqos_dma_regs {
	u32 mode;					/* 0x1000 */
	u32 sysbus_mode;				/* 0x1004 */
	u32 unused_1008[(0x1100 - 0x1008) / 4];	/* 0x1008 */
	u32 ch0_control;				/* 0x1100 */
	u32 ch0_tx_control;			/* 0x1104 */
	u32 ch0_rx_control;			/* 0x1108 */
	u32 unused_110c;				/* 0x110c */
	u32 ch0_txdesc_list_haddress;		/* 0x1110 */
	u32 ch0_txdesc_list_address;		/* 0x1114 */
	u32 ch0_rxdesc_list_haddress;		/* 0x1118 */
	u32 ch0_rxdesc_list_address;		/* 0x111c */
	u32 ch0_txdesc_tail_pointer;		/* 0x1120 */
	u32 unused_1124;				/* 0x1124 */
	u32 ch0_rxdesc_tail_pointer;		/* 0x1128 */
	u32 ch0_txdesc_ring_length;		/* 0x112c */
	u32 ch0_rxdesc_ring_length;		/* 0x1130 */
};

#define EQOS_DMA_MODE_SWR				BIT(0)

#define EQOS_DMA_SYSBUS_MODE_RD_OSR_LMT_SHIFT		16
#define EQOS_DMA_SYSBUS_MODE_RD_OSR_LMT_MASK		0xf
#define EQOS_DMA_SYSBUS_MODE_EAME			BIT(11)
#define EQOS_DMA_SYSBUS_MODE_BLEN16			BIT(3)
#define EQOS_DMA_SYSBUS_MODE_BLEN8			BIT(2)
#define EQOS_DMA_SYSBUS_MODE_BLEN4			BIT(1)

#define EQOS_DMA_CH0_CONTROL_DSL_SHIFT			18
#define EQOS_DMA_CH0_CONTROL_DSL_MASK			0x7
#define EQOS_DMA_CH0_CONTROL_PBLX8			BIT(16)

#define EQOS_DMA_CH0_TX_CONTROL_TXPBL_SHIFT		16
#define EQOS_DMA_CH0_TX_CONTROL_TXPBL_MASK		0x3f
#define EQOS_DMA_CH0_TX_CONTROL_OSP			BIT(4)
#define EQOS_DMA_CH0_TX_CONTROL_ST			BIT(0)

#define EQOS_DMA_CH0_RX_CONTROL_RXPBL_SHIFT		16
#define EQOS_DMA_CH0_RX_CONTROL_RXPBL_MASK		0x3f
#define EQOS_DMA_CH0_RX_CONTROL_RBSZ_SHIFT		1
#define EQOS_DMA_CH0_RX_CONTROL_RBSZ_MASK		0x3fff
#define EQOS_DMA_CH0_RX_CONTROL_SR			BIT(0)

/* These registers are Tegra186-specific */
#define EQOS_TEGRA186_REGS_BASE 0x8800
struct eqos_tegra186_regs {
	u32 sdmemcomppadctrl;			/* 0x8800 */
	u32 auto_cal_config;			/* 0x8804 */
	u32 unused_8808;				/* 0x8808 */
	u32 auto_cal_status;			/* 0x880c */
};

#define EQOS_SDMEMCOMPPADCTRL_PAD_E_INPUT_OR_E_PWRD	BIT(31)

#define EQOS_AUTO_CAL_CONFIG_START			BIT(31)
#define EQOS_AUTO_CAL_CONFIG_ENABLE			BIT(29)

#define EQOS_AUTO_CAL_STATUS_ACTIVE			BIT(31)

/* Descriptors */
#define EQOS_DESCRIPTORS_TX	4
#define EQOS_DESCRIPTORS_RX	4
#define EQOS_DESCRIPTORS_NUM	(EQOS_DESCRIPTORS_TX + EQOS_DESCRIPTORS_RX)
#define EQOS_BUFFER_ALIGN	ARCH_DMA_MINALIGN
#define EQOS_MAX_PACKET_SIZE	ALIGN(1568, ARCH_DMA_MINALIGN)
#define EQOS_RX_BUFFER_SIZE	(EQOS_DESCRIPTORS_RX * EQOS_MAX_PACKET_SIZE)

struct eqos_desc {
	u32 des0;
	u32 des1;
	u32 des2;
	u32 des3;
};

#define EQOS_DESC3_OWN		BIT(31)
#define EQOS_DESC3_FD		BIT(29)
#define EQOS_DESC3_LD		BIT(28)
#define EQOS_DESC3_BUF1V	BIT(24)

#define EQOS_AXI_WIDTH_32	4
#define EQOS_AXI_WIDTH_64	8
#define EQOS_AXI_WIDTH_128	16

struct eqos_config {
	bool reg_access_always_ok;
	int mdio_wait;
	int swr_wait;
	int config_mac;
	int config_mac_mdio;
	unsigned int axi_bus_width;
	phy_interface_t (*interface)(const struct udevice *dev);
	struct eqos_ops *ops;
};

struct eqos_ops {
	void (*eqos_inval_desc)(void *desc);
	void (*eqos_flush_desc)(void *desc);
	void (*eqos_inval_buffer)(void *buf, size_t size);
	void (*eqos_flush_buffer)(void *buf, size_t size);
	int (*eqos_probe_resources)(struct udevice *dev);
	int (*eqos_remove_resources)(struct udevice *dev);
	int (*eqos_stop_resets)(struct udevice *dev);
	int (*eqos_start_resets)(struct udevice *dev);
	int (*eqos_stop_clks)(struct udevice *dev);
	int (*eqos_start_clks)(struct udevice *dev);
	int (*eqos_calibrate_pads)(struct udevice *dev);
	int (*eqos_disable_calibration)(struct udevice *dev);
	int (*eqos_set_tx_clk_speed)(struct udevice *dev);
	int (*eqos_get_enetaddr)(struct udevice *dev);
	ulong (*eqos_get_tick_clk_rate)(struct udevice *dev);
};

struct eqos_priv {
	struct udevice *dev;
	const struct eqos_config *config;
	fdt_addr_t regs;
	struct eqos_mac_regs *mac_regs;
	struct eqos_mtl_regs *mtl_regs;
	struct eqos_dma_regs *dma_regs;
	struct eqos_tegra186_regs *tegra186_regs;
	void *eqos_qcom_rgmii_regs;
	struct reset_ctl reset_ctl;
	struct gpio_desc phy_reset_gpio;
	struct clk clk_master_bus;
	struct clk clk_rx;
	struct clk clk_ptp_ref;
	struct clk clk_tx;
	struct clk clk_ck;
	struct clk clk_slave_bus;
	struct mii_dev *mii;
	struct phy_device *phy;
	ofnode phy_of_node;
	u32 max_speed;
	void *tx_descs;
	void *rx_descs;
	int tx_desc_idx, rx_desc_idx;
	unsigned int desc_size;
	unsigned int desc_per_cacheline;
	void *tx_dma_buf;
	void *rx_dma_buf;
	bool started;
	bool reg_access_ok;
	bool clk_ck_enabled;
	unsigned int tx_fifo_sz, rx_fifo_sz;
	u32 reset_delays[3];
};

void eqos_inval_desc_generic(void *desc);
void eqos_flush_desc_generic(void *desc);
void eqos_inval_buffer_generic(void *buf, size_t size);
void eqos_flush_buffer_generic(void *buf, size_t size);
int eqos_null_ops(struct udevice *dev);

extern struct eqos_config eqos_imx_config;
extern struct eqos_config eqos_rockchip_config;
extern struct eqos_config eqos_qcom_config;
extern struct eqos_config eqos_jh7110_config;
