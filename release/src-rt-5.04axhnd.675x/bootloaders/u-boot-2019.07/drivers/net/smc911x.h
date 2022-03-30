/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * SMSC LAN9[12]1[567] Network driver
 *
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
 */

#ifndef _SMC911X_H_
#define _SMC911X_H_

#include <linux/types.h>

#define DRIVERNAME "smc911x"

#if defined (CONFIG_SMC911X_32_BIT) && \
	defined (CONFIG_SMC911X_16_BIT)
#error "SMC911X: Only one of CONFIG_SMC911X_32_BIT and \
	CONFIG_SMC911X_16_BIT shall be set"
#endif

#if defined (CONFIG_SMC911X_32_BIT)
static inline u32 __smc911x_reg_read(struct eth_device *dev, u32 offset)
{
	return *(volatile u32*)(dev->iobase + offset);
}
u32 smc911x_reg_read(struct eth_device *dev, u32 offset)
	__attribute__((weak, alias("__smc911x_reg_read")));

static inline void __smc911x_reg_write(struct eth_device *dev,
					u32 offset, u32 val)
{
	*(volatile u32*)(dev->iobase + offset) = val;
}
void smc911x_reg_write(struct eth_device *dev, u32 offset, u32 val)
	__attribute__((weak, alias("__smc911x_reg_write")));
#elif defined (CONFIG_SMC911X_16_BIT)
static inline u32 smc911x_reg_read(struct eth_device *dev, u32 offset)
{
	volatile u16 *addr_16 = (u16 *)(dev->iobase + offset);
	return ((*addr_16 & 0x0000ffff) | (*(addr_16 + 1) << 16));
}
static inline void smc911x_reg_write(struct eth_device *dev,
					u32 offset, u32 val)
{
	*(volatile u16 *)(dev->iobase + offset) = (u16)val;
	*(volatile u16 *)(dev->iobase + offset + 2) = (u16)(val >> 16);
}
#else
#error "SMC911X: undefined bus width"
#endif /* CONFIG_SMC911X_16_BIT */

/* Below are the register offsets and bit definitions
 * of the Lan911x memory space
 */
#define RX_DATA_FIFO		 		0x00

#define TX_DATA_FIFO		 		0x20
#define	TX_CMD_A_INT_ON_COMP			0x80000000
#define	TX_CMD_A_INT_BUF_END_ALGN		0x03000000
#define	TX_CMD_A_INT_4_BYTE_ALGN		0x00000000
#define	TX_CMD_A_INT_16_BYTE_ALGN		0x01000000
#define	TX_CMD_A_INT_32_BYTE_ALGN		0x02000000
#define	TX_CMD_A_INT_DATA_OFFSET		0x001F0000
#define	TX_CMD_A_INT_FIRST_SEG			0x00002000
#define	TX_CMD_A_INT_LAST_SEG			0x00001000
#define	TX_CMD_A_BUF_SIZE			0x000007FF
#define	TX_CMD_B_PKT_TAG			0xFFFF0000
#define	TX_CMD_B_ADD_CRC_DISABLE		0x00002000
#define	TX_CMD_B_DISABLE_PADDING		0x00001000
#define	TX_CMD_B_PKT_BYTE_LENGTH		0x000007FF

#define RX_STATUS_FIFO				0x40
#define	RX_STS_PKT_LEN				0x3FFF0000
#define	RX_STS_ES				0x00008000
#define	RX_STS_BCST				0x00002000
#define	RX_STS_LEN_ERR				0x00001000
#define	RX_STS_RUNT_ERR				0x00000800
#define	RX_STS_MCAST				0x00000400
#define	RX_STS_TOO_LONG				0x00000080
#define	RX_STS_COLL				0x00000040
#define	RX_STS_ETH_TYPE				0x00000020
#define	RX_STS_WDOG_TMT				0x00000010
#define	RX_STS_MII_ERR				0x00000008
#define	RX_STS_DRIBBLING			0x00000004
#define	RX_STS_CRC_ERR				0x00000002
#define RX_STATUS_FIFO_PEEK			0x44
#define TX_STATUS_FIFO				0x48
#define	TX_STS_TAG				0xFFFF0000
#define	TX_STS_ES				0x00008000
#define	TX_STS_LOC				0x00000800
#define	TX_STS_NO_CARR				0x00000400
#define	TX_STS_LATE_COLL			0x00000200
#define	TX_STS_MANY_COLL			0x00000100
#define	TX_STS_COLL_CNT				0x00000078
#define	TX_STS_MANY_DEFER			0x00000004
#define	TX_STS_UNDERRUN				0x00000002
#define	TX_STS_DEFERRED				0x00000001
#define TX_STATUS_FIFO_PEEK			0x4C
#define ID_REV					0x50
#define	ID_REV_CHIP_ID				0xFFFF0000  /* RO */
#define	ID_REV_REV_ID				0x0000FFFF  /* RO */

#define INT_CFG					0x54
#define	INT_CFG_INT_DEAS			0xFF000000  /* R/W */
#define	INT_CFG_INT_DEAS_CLR			0x00004000
#define	INT_CFG_INT_DEAS_STS			0x00002000
#define	INT_CFG_IRQ_INT				0x00001000  /* RO */
#define	INT_CFG_IRQ_EN				0x00000100  /* R/W */
					/* R/W Not Affected by SW Reset */
#define	INT_CFG_IRQ_POL				0x00000010
					/* R/W Not Affected by SW Reset */
#define	INT_CFG_IRQ_TYPE			0x00000001

#define INT_STS					0x58
#define	INT_STS_SW_INT				0x80000000  /* R/WC */
#define	INT_STS_TXSTOP_INT			0x02000000  /* R/WC */
#define	INT_STS_RXSTOP_INT			0x01000000  /* R/WC */
#define	INT_STS_RXDFH_INT			0x00800000  /* R/WC */
#define	INT_STS_RXDF_INT			0x00400000  /* R/WC */
#define	INT_STS_TX_IOC				0x00200000  /* R/WC */
#define	INT_STS_RXD_INT				0x00100000  /* R/WC */
#define	INT_STS_GPT_INT				0x00080000  /* R/WC */
#define	INT_STS_PHY_INT				0x00040000  /* RO */
#define	INT_STS_PME_INT				0x00020000  /* R/WC */
#define	INT_STS_TXSO				0x00010000  /* R/WC */
#define	INT_STS_RWT				0x00008000  /* R/WC */
#define	INT_STS_RXE				0x00004000  /* R/WC */
#define	INT_STS_TXE				0x00002000  /* R/WC */
/*#define	INT_STS_ERX		0x00001000*/  /* R/WC */
#define	INT_STS_TDFU				0x00000800  /* R/WC */
#define	INT_STS_TDFO				0x00000400  /* R/WC */
#define	INT_STS_TDFA				0x00000200  /* R/WC */
#define	INT_STS_TSFF				0x00000100  /* R/WC */
#define	INT_STS_TSFL				0x00000080  /* R/WC */
/*#define	INT_STS_RXDF		0x00000040*/  /* R/WC */
#define	INT_STS_RDFO				0x00000040  /* R/WC */
#define	INT_STS_RDFL				0x00000020  /* R/WC */
#define	INT_STS_RSFF				0x00000010  /* R/WC */
#define	INT_STS_RSFL				0x00000008  /* R/WC */
#define	INT_STS_GPIO2_INT			0x00000004  /* R/WC */
#define	INT_STS_GPIO1_INT			0x00000002  /* R/WC */
#define	INT_STS_GPIO0_INT			0x00000001  /* R/WC */
#define INT_EN					0x5C
#define	INT_EN_SW_INT_EN			0x80000000  /* R/W */
#define	INT_EN_TXSTOP_INT_EN			0x02000000  /* R/W */
#define	INT_EN_RXSTOP_INT_EN			0x01000000  /* R/W */
#define	INT_EN_RXDFH_INT_EN			0x00800000  /* R/W */
/*#define	INT_EN_RXDF_INT_EN		0x00400000*/  /* R/W */
#define	INT_EN_TIOC_INT_EN			0x00200000  /* R/W */
#define	INT_EN_RXD_INT_EN			0x00100000  /* R/W */
#define	INT_EN_GPT_INT_EN			0x00080000  /* R/W */
#define	INT_EN_PHY_INT_EN			0x00040000  /* R/W */
#define	INT_EN_PME_INT_EN			0x00020000  /* R/W */
#define	INT_EN_TXSO_EN				0x00010000  /* R/W */
#define	INT_EN_RWT_EN				0x00008000  /* R/W */
#define	INT_EN_RXE_EN				0x00004000  /* R/W */
#define	INT_EN_TXE_EN				0x00002000  /* R/W */
/*#define	INT_EN_ERX_EN			0x00001000*/  /* R/W */
#define	INT_EN_TDFU_EN				0x00000800  /* R/W */
#define	INT_EN_TDFO_EN				0x00000400  /* R/W */
#define	INT_EN_TDFA_EN				0x00000200  /* R/W */
#define	INT_EN_TSFF_EN				0x00000100  /* R/W */
#define	INT_EN_TSFL_EN				0x00000080  /* R/W */
/*#define	INT_EN_RXDF_EN			0x00000040*/  /* R/W */
#define	INT_EN_RDFO_EN				0x00000040  /* R/W */
#define	INT_EN_RDFL_EN				0x00000020  /* R/W */
#define	INT_EN_RSFF_EN				0x00000010  /* R/W */
#define	INT_EN_RSFL_EN				0x00000008  /* R/W */
#define	INT_EN_GPIO2_INT			0x00000004  /* R/W */
#define	INT_EN_GPIO1_INT			0x00000002  /* R/W */
#define	INT_EN_GPIO0_INT			0x00000001  /* R/W */

#define BYTE_TEST				0x64
#define FIFO_INT				0x68
#define	FIFO_INT_TX_AVAIL_LEVEL			0xFF000000  /* R/W */
#define	FIFO_INT_TX_STS_LEVEL			0x00FF0000  /* R/W */
#define	FIFO_INT_RX_AVAIL_LEVEL			0x0000FF00  /* R/W */
#define	FIFO_INT_RX_STS_LEVEL			0x000000FF  /* R/W */

#define RX_CFG					0x6C
#define	RX_CFG_RX_END_ALGN			0xC0000000  /* R/W */
#define		RX_CFG_RX_END_ALGN4		0x00000000  /* R/W */
#define		RX_CFG_RX_END_ALGN16		0x40000000  /* R/W */
#define		RX_CFG_RX_END_ALGN32		0x80000000  /* R/W */
#define	RX_CFG_RX_DMA_CNT			0x0FFF0000  /* R/W */
#define	RX_CFG_RX_DUMP				0x00008000  /* R/W */
#define	RX_CFG_RXDOFF				0x00001F00  /* R/W */
/*#define	RX_CFG_RXBAD			0x00000001*/  /* R/W */

#define TX_CFG					0x70
/*#define	TX_CFG_TX_DMA_LVL		0xE0000000*/	 /* R/W */
						 /* R/W Self Clearing */
/*#define	TX_CFG_TX_DMA_CNT		0x0FFF0000*/
#define	TX_CFG_TXS_DUMP				0x00008000  /* Self Clearing */
#define	TX_CFG_TXD_DUMP				0x00004000  /* Self Clearing */
#define	TX_CFG_TXSAO				0x00000004  /* R/W */
#define	TX_CFG_TX_ON				0x00000002  /* R/W */
#define	TX_CFG_STOP_TX				0x00000001  /* Self Clearing */

#define HW_CFG					0x74
#define	HW_CFG_TTM				0x00200000  /* R/W */
#define	HW_CFG_SF				0x00100000  /* R/W */
#define	HW_CFG_TX_FIF_SZ			0x000F0000  /* R/W */
#define	HW_CFG_TR				0x00003000  /* R/W */
#define	HW_CFG_PHY_CLK_SEL			0x00000060  /* R/W */
#define	HW_CFG_PHY_CLK_SEL_INT_PHY		0x00000000 /* R/W */
#define	HW_CFG_PHY_CLK_SEL_EXT_PHY		0x00000020 /* R/W */
#define	HW_CFG_PHY_CLK_SEL_CLK_DIS		0x00000040 /* R/W */
#define	HW_CFG_SMI_SEL				0x00000010  /* R/W */
#define	HW_CFG_EXT_PHY_DET			0x00000008  /* RO */
#define	HW_CFG_EXT_PHY_EN			0x00000004  /* R/W */
#define	HW_CFG_32_16_BIT_MODE			0x00000004  /* RO */
#define	HW_CFG_SRST_TO				0x00000002  /* RO */
#define	HW_CFG_SRST				0x00000001  /* Self Clearing */

#define RX_DP_CTRL				0x78
#define	RX_DP_CTRL_RX_FFWD			0x80000000  /* R/W */
#define	RX_DP_CTRL_FFWD_BUSY			0x80000000  /* RO */

#define RX_FIFO_INF				0x7C
#define	 RX_FIFO_INF_RXSUSED			0x00FF0000  /* RO */
#define	 RX_FIFO_INF_RXDUSED			0x0000FFFF  /* RO */

#define TX_FIFO_INF				0x80
#define	TX_FIFO_INF_TSUSED			0x00FF0000  /* RO */
#define	TX_FIFO_INF_TDFREE			0x0000FFFF  /* RO */

#define PMT_CTRL				0x84
#define	PMT_CTRL_PM_MODE			0x00003000  /* Self Clearing */
#define	PMT_CTRL_PHY_RST			0x00000400  /* Self Clearing */
#define	PMT_CTRL_WOL_EN				0x00000200  /* R/W */
#define	PMT_CTRL_ED_EN				0x00000100  /* R/W */
					/* R/W Not Affected by SW Reset */
#define	PMT_CTRL_PME_TYPE			0x00000040
#define	PMT_CTRL_WUPS				0x00000030  /* R/WC */
#define	PMT_CTRL_WUPS_NOWAKE			0x00000000  /* R/WC */
#define	PMT_CTRL_WUPS_ED			0x00000010  /* R/WC */
#define	PMT_CTRL_WUPS_WOL			0x00000020  /* R/WC */
#define	PMT_CTRL_WUPS_MULTI			0x00000030  /* R/WC */
#define	PMT_CTRL_PME_IND			0x00000008  /* R/W */
#define	PMT_CTRL_PME_POL			0x00000004  /* R/W */
					/* R/W Not Affected by SW Reset */
#define	PMT_CTRL_PME_EN				0x00000002
#define	PMT_CTRL_READY				0x00000001  /* RO */

#define GPIO_CFG				0x88
#define	GPIO_CFG_LED3_EN			0x40000000  /* R/W */
#define	GPIO_CFG_LED2_EN			0x20000000  /* R/W */
#define	GPIO_CFG_LED1_EN			0x10000000  /* R/W */
#define	GPIO_CFG_GPIO2_INT_POL			0x04000000  /* R/W */
#define	GPIO_CFG_GPIO1_INT_POL			0x02000000  /* R/W */
#define	GPIO_CFG_GPIO0_INT_POL			0x01000000  /* R/W */
#define	GPIO_CFG_EEPR_EN			0x00700000  /* R/W */
#define	GPIO_CFG_GPIOBUF2			0x00040000  /* R/W */
#define	GPIO_CFG_GPIOBUF1			0x00020000  /* R/W */
#define	GPIO_CFG_GPIOBUF0			0x00010000  /* R/W */
#define	GPIO_CFG_GPIODIR2			0x00000400  /* R/W */
#define	GPIO_CFG_GPIODIR1			0x00000200  /* R/W */
#define	GPIO_CFG_GPIODIR0			0x00000100  /* R/W */
#define	GPIO_CFG_GPIOD4				0x00000010  /* R/W */
#define	GPIO_CFG_GPIOD3				0x00000008  /* R/W */
#define	GPIO_CFG_GPIOD2				0x00000004  /* R/W */
#define	GPIO_CFG_GPIOD1				0x00000002  /* R/W */
#define	GPIO_CFG_GPIOD0				0x00000001  /* R/W */

#define GPT_CFG					0x8C
#define	GPT_CFG_TIMER_EN			0x20000000  /* R/W */
#define	GPT_CFG_GPT_LOAD			0x0000FFFF  /* R/W */

#define GPT_CNT					0x90
#define	GPT_CNT_GPT_CNT				0x0000FFFF  /* RO */

#define ENDIAN					0x98
#define FREE_RUN				0x9C
#define RX_DROP					0xA0
#define MAC_CSR_CMD				0xA4
#define	 MAC_CSR_CMD_CSR_BUSY			0x80000000  /* Self Clearing */
#define	 MAC_CSR_CMD_R_NOT_W			0x40000000  /* R/W */
#define	 MAC_CSR_CMD_CSR_ADDR			0x000000FF  /* R/W */

#define MAC_CSR_DATA				0xA8
#define AFC_CFG					0xAC
#define		AFC_CFG_AFC_HI			0x00FF0000  /* R/W */
#define		AFC_CFG_AFC_LO			0x0000FF00  /* R/W */
#define		AFC_CFG_BACK_DUR		0x000000F0  /* R/W */
#define		AFC_CFG_FCMULT			0x00000008  /* R/W */
#define		AFC_CFG_FCBRD			0x00000004  /* R/W */
#define		AFC_CFG_FCADD			0x00000002  /* R/W */
#define		AFC_CFG_FCANY			0x00000001  /* R/W */

#define E2P_CMD					0xB0
#define		E2P_CMD_EPC_BUSY		0x80000000  /* Self Clearing */
#define		E2P_CMD_EPC_CMD			0x70000000  /* R/W */
#define		E2P_CMD_EPC_CMD_READ		0x00000000  /* R/W */
#define		E2P_CMD_EPC_CMD_EWDS		0x10000000  /* R/W */
#define		E2P_CMD_EPC_CMD_EWEN		0x20000000  /* R/W */
#define		E2P_CMD_EPC_CMD_WRITE		0x30000000  /* R/W */
#define		E2P_CMD_EPC_CMD_WRAL		0x40000000  /* R/W */
#define		E2P_CMD_EPC_CMD_ERASE		0x50000000  /* R/W */
#define		E2P_CMD_EPC_CMD_ERAL		0x60000000  /* R/W */
#define		E2P_CMD_EPC_CMD_RELOAD		0x70000000  /* R/W */
#define		E2P_CMD_EPC_TIMEOUT		0x00000200  /* RO */
#define		E2P_CMD_MAC_ADDR_LOADED		0x00000100  /* RO */
#define		E2P_CMD_EPC_ADDR		0x000000FF  /* R/W */

#define E2P_DATA				0xB4
#define	E2P_DATA_EEPROM_DATA			0x000000FF  /* R/W */
/* end of LAN register offsets and bit definitions */

/* MAC Control and Status registers */
#define MAC_CR			0x01  /* R/W */

/* MAC_CR - MAC Control Register */
#define MAC_CR_RXALL			0x80000000
/* TODO: delete this bit? It is not described in the data sheet. */
#define MAC_CR_HBDIS			0x10000000
#define MAC_CR_RCVOWN			0x00800000
#define MAC_CR_LOOPBK			0x00200000
#define MAC_CR_FDPX			0x00100000
#define MAC_CR_MCPAS			0x00080000
#define MAC_CR_PRMS			0x00040000
#define MAC_CR_INVFILT			0x00020000
#define MAC_CR_PASSBAD			0x00010000
#define MAC_CR_HFILT			0x00008000
#define MAC_CR_HPFILT			0x00002000
#define MAC_CR_LCOLL			0x00001000
#define MAC_CR_BCAST			0x00000800
#define MAC_CR_DISRTY			0x00000400
#define MAC_CR_PADSTR			0x00000100
#define MAC_CR_BOLMT_MASK		0x000000C0
#define MAC_CR_DFCHK			0x00000020
#define MAC_CR_TXEN			0x00000008
#define MAC_CR_RXEN			0x00000004

#define ADDRH			0x02	  /* R/W mask 0x0000FFFFUL */
#define ADDRL			0x03	  /* R/W mask 0xFFFFFFFFUL */
#define HASHH			0x04	  /* R/W */
#define HASHL			0x05	  /* R/W */

#define MII_ACC			0x06	  /* R/W */
#define MII_ACC_PHY_ADDR		0x0000F800
#define MII_ACC_MIIRINDA		0x000007C0
#define MII_ACC_MII_WRITE		0x00000002
#define MII_ACC_MII_BUSY		0x00000001

#define MII_DATA		0x07	  /* R/W mask 0x0000FFFFUL */

#define FLOW			0x08	  /* R/W */
#define FLOW_FCPT			0xFFFF0000
#define FLOW_FCPASS			0x00000004
#define FLOW_FCEN			0x00000002
#define FLOW_FCBSY			0x00000001

#define VLAN1			0x09	  /* R/W mask 0x0000FFFFUL */
#define VLAN1_VTI1			0x0000ffff

#define VLAN2			0x0A	  /* R/W mask 0x0000FFFFUL */
#define VLAN2_VTI2			0x0000ffff

#define WUFF			0x0B	  /* WO */

#define WUCSR			0x0C	  /* R/W */
#define WUCSR_GUE			0x00000200
#define WUCSR_WUFR			0x00000040
#define WUCSR_MPR			0x00000020
#define WUCSR_WAKE_EN			0x00000004
#define WUCSR_MPEN			0x00000002

/* Chip ID values */
#define CHIP_89218	0x218a
#define CHIP_9115	0x115
#define CHIP_9116	0x116
#define CHIP_9117	0x117
#define CHIP_9118	0x118
#define CHIP_9211	0x9211
#define CHIP_9215	0x115a
#define CHIP_9216	0x116a
#define CHIP_9217	0x117a
#define CHIP_9218	0x118a
#define CHIP_9220	0x9220
#define CHIP_9221	0x9221

struct chip_id {
	u16 id;
	char *name;
};

static const struct chip_id chip_ids[] =  {
	{ CHIP_89218, "LAN89218" },
	{ CHIP_9115, "LAN9115" },
	{ CHIP_9116, "LAN9116" },
	{ CHIP_9117, "LAN9117" },
	{ CHIP_9118, "LAN9118" },
	{ CHIP_9211, "LAN9211" },
	{ CHIP_9215, "LAN9215" },
	{ CHIP_9216, "LAN9216" },
	{ CHIP_9217, "LAN9217" },
	{ CHIP_9218, "LAN9218" },
	{ CHIP_9220, "LAN9220" },
	{ CHIP_9221, "LAN9221" },
	{ 0, NULL },
};

static u32 smc911x_get_mac_csr(struct eth_device *dev, u8 reg)
{
	while (smc911x_reg_read(dev, MAC_CSR_CMD) & MAC_CSR_CMD_CSR_BUSY)
		;
	smc911x_reg_write(dev, MAC_CSR_CMD,
			MAC_CSR_CMD_CSR_BUSY | MAC_CSR_CMD_R_NOT_W | reg);
	while (smc911x_reg_read(dev, MAC_CSR_CMD) & MAC_CSR_CMD_CSR_BUSY)
		;

	return smc911x_reg_read(dev, MAC_CSR_DATA);
}

static void smc911x_set_mac_csr(struct eth_device *dev, u8 reg, u32 data)
{
	while (smc911x_reg_read(dev, MAC_CSR_CMD) & MAC_CSR_CMD_CSR_BUSY)
		;
	smc911x_reg_write(dev, MAC_CSR_DATA, data);
	smc911x_reg_write(dev, MAC_CSR_CMD, MAC_CSR_CMD_CSR_BUSY | reg);
	while (smc911x_reg_read(dev, MAC_CSR_CMD) & MAC_CSR_CMD_CSR_BUSY)
		;
}

static int smc911x_detect_chip(struct eth_device *dev)
{
	unsigned long val, i;

	val = smc911x_reg_read(dev, BYTE_TEST);
	if (val == 0xffffffff) {
		/* Special case -- no chip present */
		return -1;
	} else if (val != 0x87654321) {
		printf(DRIVERNAME ": Invalid chip endian 0x%08lx\n", val);
		return -1;
	}

	val = smc911x_reg_read(dev, ID_REV) >> 16;
	for (i = 0; chip_ids[i].id != 0; i++) {
		if (chip_ids[i].id == val) break;
	}
	if (!chip_ids[i].id) {
		printf(DRIVERNAME ": Unknown chip ID %04lx\n", val);
		return -1;
	}

	dev->priv = (void *)&chip_ids[i];

	return 0;
}

static void smc911x_reset(struct eth_device *dev)
{
	int timeout;

	/*
	 *  Take out of PM setting first
	 *  Device is already wake up if PMT_CTRL_READY bit is set
	 */
	if ((smc911x_reg_read(dev, PMT_CTRL) & PMT_CTRL_READY) == 0) {
		/* Write to the bytetest will take out of powerdown */
		smc911x_reg_write(dev, BYTE_TEST, 0x0);

		timeout = 10;

		while (timeout-- &&
			!(smc911x_reg_read(dev, PMT_CTRL) & PMT_CTRL_READY))
			udelay(10);
		if (timeout < 0) {
			printf(DRIVERNAME
				": timeout waiting for PM restore\n");
			return;
		}
	}

	/* Disable interrupts */
	smc911x_reg_write(dev, INT_EN, 0);

	smc911x_reg_write(dev, HW_CFG, HW_CFG_SRST);

	timeout = 1000;
	while (timeout-- && smc911x_reg_read(dev, E2P_CMD) & E2P_CMD_EPC_BUSY)
		udelay(10);

	if (timeout < 0) {
		printf(DRIVERNAME ": reset timeout\n");
		return;
	}

	/* Reset the FIFO level and flow control settings */
	smc911x_set_mac_csr(dev, FLOW, FLOW_FCPT | FLOW_FCEN);
	smc911x_reg_write(dev, AFC_CFG, 0x0050287F);

	/* Set to LED outputs */
	smc911x_reg_write(dev, GPIO_CFG, 0x70070000);
}

#endif
