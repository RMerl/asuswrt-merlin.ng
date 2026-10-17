// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <asm/io.h>
#include <cpu_func.h>
#include <malloc.h>
#include <miiphy.h>
#include <net.h>
#include <netdev.h>
#include <pci.h>
#include <linux/delay.h>

/* Ethernet chip registers. */
#define SCB_STATUS		0	/* Rx/Command Unit Status *Word* */
#define SCB_INT_ACK_BYTE	1	/* Rx/Command Unit STAT/ACK byte */
#define SCB_CMD			2	/* Rx/Command Unit Command *Word* */
#define SCB_INTR_CTL_BYTE	3	/* Rx/Command Unit Intr.Control Byte */
#define SCB_POINTER		4	/* General purpose pointer. */
#define SCB_PORT		8	/* Misc. commands and operands. */
#define SCB_FLASH		12	/* Flash memory control. */
#define SCB_EEPROM		14	/* EEPROM memory control. */
#define SCB_CTRL_MDI		16	/* MDI interface control. */
#define SCB_EARLY_RX		20	/* Early receive byte count. */
#define SCB_GEN_CONTROL		28	/* 82559 General Control Register */
#define SCB_GEN_STATUS		29	/* 82559 General Status register */

/* 82559 SCB status word defnitions */
#define SCB_STATUS_CX		0x8000	/* CU finished command (transmit) */
#define SCB_STATUS_FR		0x4000	/* frame received */
#define SCB_STATUS_CNA		0x2000	/* CU left active state */
#define SCB_STATUS_RNR		0x1000	/* receiver left ready state */
#define SCB_STATUS_MDI		0x0800	/* MDI read/write cycle done */
#define SCB_STATUS_SWI		0x0400	/* software generated interrupt */
#define SCB_STATUS_FCP		0x0100	/* flow control pause interrupt */

#define SCB_INTACK_MASK		0xFD00	/* all the above */

#define SCB_INTACK_TX		(SCB_STATUS_CX | SCB_STATUS_CNA)
#define SCB_INTACK_RX		(SCB_STATUS_FR | SCB_STATUS_RNR)

/* System control block commands */
/* CU Commands */
#define CU_NOP			0x0000
#define CU_START		0x0010
#define CU_RESUME		0x0020
#define CU_STATSADDR		0x0040	/* Load Dump Statistics ctrs addr */
#define CU_SHOWSTATS		0x0050	/* Dump statistics counters. */
#define CU_ADDR_LOAD		0x0060	/* Base address to add to CU commands */
#define CU_DUMPSTATS		0x0070	/* Dump then reset stats counters. */

/* RUC Commands */
#define RUC_NOP			0x0000
#define RUC_START		0x0001
#define RUC_RESUME		0x0002
#define RUC_ABORT		0x0004
#define RUC_ADDR_LOAD		0x0006	/* (seems not to clear on acceptance) */
#define RUC_RESUMENR		0x0007

#define CU_CMD_MASK		0x00f0
#define RU_CMD_MASK		0x0007

#define SCB_M			0x0100	/* 0 = enable interrupt, 1 = disable */
#define SCB_SWI			0x0200	/* 1 - cause device to interrupt */

#define CU_STATUS_MASK		0x00C0
#define RU_STATUS_MASK		0x003C

#define RU_STATUS_IDLE		(0 << 2)
#define RU_STATUS_SUS		(1 << 2)
#define RU_STATUS_NORES		(2 << 2)
#define RU_STATUS_READY		(4 << 2)
#define RU_STATUS_NO_RBDS_SUS	((1 << 2) | (8 << 2))
#define RU_STATUS_NO_RBDS_NORES ((2 << 2) | (8 << 2))
#define RU_STATUS_NO_RBDS_READY ((4 << 2) | (8 << 2))

/* 82559 Port interface commands. */
#define I82559_RESET		0x00000000	/* Software reset */
#define I82559_SELFTEST		0x00000001	/* 82559 Selftest command */
#define I82559_SELECTIVE_RESET	0x00000002
#define I82559_DUMP		0x00000003
#define I82559_DUMP_WAKEUP	0x00000007

/* 82559 Eeprom interface. */
#define EE_SHIFT_CLK		0x01	/* EEPROM shift clock. */
#define EE_CS			0x02	/* EEPROM chip select. */
#define EE_DATA_WRITE		0x04	/* EEPROM chip data in. */
#define EE_WRITE_0		0x01
#define EE_WRITE_1		0x05
#define EE_DATA_READ		0x08	/* EEPROM chip data out. */
#define EE_ENB			(0x4800 | EE_CS)
#define EE_CMD_BITS		3
#define EE_DATA_BITS		16

/* The EEPROM commands include the alway-set leading bit. */
#define EE_EWENB_CMD(addr_len)	(4 << (addr_len))
#define EE_WRITE_CMD(addr_len)	(5 << (addr_len))
#define EE_READ_CMD(addr_len)	(6 << (addr_len))
#define EE_ERASE_CMD(addr_len)	(7 << (addr_len))

/* Receive frame descriptors. */
struct eepro100_rxfd {
	u16 status;
	u16 control;
	u32 link;		/* struct eepro100_rxfd * */
	u32 rx_buf_addr;	/* void * */
	u32 count;

	u8 data[PKTSIZE_ALIGN];
};

#define RFD_STATUS_C		0x8000	/* completion of received frame */
#define RFD_STATUS_OK		0x2000	/* frame received with no errors */

#define RFD_CONTROL_EL		0x8000	/* 1=last RFD in RFA */
#define RFD_CONTROL_S		0x4000	/* 1=suspend RU after receiving frame */
#define RFD_CONTROL_H		0x0010	/* 1=RFD is a header RFD */
#define RFD_CONTROL_SF		0x0008	/* 0=simplified, 1=flexible mode */

#define RFD_COUNT_MASK		0x3fff
#define RFD_COUNT_F		0x4000
#define RFD_COUNT_EOF		0x8000

#define RFD_RX_CRC		0x0800	/* crc error */
#define RFD_RX_ALIGNMENT	0x0400	/* alignment error */
#define RFD_RX_RESOURCE		0x0200	/* out of space, no resources */
#define RFD_RX_DMA_OVER		0x0100	/* DMA overrun */
#define RFD_RX_SHORT		0x0080	/* short frame error */
#define RFD_RX_LENGTH		0x0020
#define RFD_RX_ERROR		0x0010	/* receive error */
#define RFD_RX_NO_ADR_MATCH	0x0004	/* no address match */
#define RFD_RX_IA_MATCH		0x0002	/* individual address does not match */
#define RFD_RX_TCO		0x0001	/* TCO indication */

/* Transmit frame descriptors */
struct eepro100_txfd {		/* Transmit frame descriptor set. */
	u16 status;
	u16 command;
	u32 link;		/* void * */
	u32 tx_desc_addr;	/* Always points to the tx_buf_addr element. */
	s32 count;

	u32 tx_buf_addr0;	/* void *, frame to be transmitted. */
	s32 tx_buf_size0;	/* Length of Tx frame. */
	u32 tx_buf_addr1;	/* void *, frame to be transmitted. */
	s32 tx_buf_size1;	/* Length of Tx frame. */
};

#define TXCB_CMD_TRANSMIT	0x0004	/* transmit command */
#define TXCB_CMD_SF		0x0008	/* 0=simplified, 1=flexible mode */
#define TXCB_CMD_NC		0x0010	/* 0=CRC insert by controller */
#define TXCB_CMD_I		0x2000	/* generate interrupt on completion */
#define TXCB_CMD_S		0x4000	/* suspend on completion */
#define TXCB_CMD_EL		0x8000	/* last command block in CBL */

#define TXCB_COUNT_MASK		0x3fff
#define TXCB_COUNT_EOF		0x8000

/* The Speedo3 Rx and Tx frame/buffer descriptors. */
struct descriptor {		/* A generic descriptor. */
	u16 status;
	u16 command;
	u32 link;		/* struct descriptor * */

	unsigned char params[0];
};

#define CFG_SYS_CMD_SUSPEND		0x4000
#define CFG_SYS_CMD_IAS		0x0001	/* individual address setup */
#define CFG_SYS_CMD_CONFIGURE	0x0002	/* configure */

#define CFG_SYS_STATUS_C		0x8000
#define CFG_SYS_STATUS_OK		0x2000

/* Misc. */
#define NUM_RX_DESC		PKTBUFSRX
#define NUM_TX_DESC		1	/* Number of TX descriptors */

#define TOUT_LOOP		1000000

/*
 * The parameters for a CmdConfigure operation.
 * There are so many options that it would be difficult to document
 * each bit. We mostly use the default or recommended settings.
 */
static const char i82558_config_cmd[] = {
	22, 0x08, 0, 1, 0, 0, 0x22, 0x03, 1,	/* 1=Use MII  0=Use AUI */
	0, 0x2E, 0, 0x60, 0x08, 0x88,
	0x68, 0, 0x40, 0xf2, 0x84,		/* Disable FC */
	0x31, 0x05,
};

struct eepro100_priv {
	/* RX descriptor ring */
	struct eepro100_rxfd	rx_ring[NUM_RX_DESC];
	/* TX descriptor ring */
	struct eepro100_txfd	tx_ring[NUM_TX_DESC];
	/* RX descriptor ring pointer */
	int			rx_next;
	u16			rx_stat;
	/* TX descriptor ring pointer */
	int			tx_next;
	int			tx_threshold;
	struct udevice		*devno;
	char			*name;
	void __iomem		*iobase;
	u8			*enetaddr;
};

#define bus_to_phys(dev, a)	dm_pci_mem_to_phys((dev), (a))
#define phys_to_bus(dev, a)	dm_pci_phys_to_mem((dev), (a))

static int INW(struct eepro100_priv *priv, u_long addr)
{
	return le16_to_cpu(readw(addr + priv->iobase));
}

static void OUTW(struct eepro100_priv *priv, int command, u_long addr)
{
	writew(cpu_to_le16(command), addr + priv->iobase);
}

static void OUTL(struct eepro100_priv *priv, int command, u_long addr)
{
	writel(cpu_to_le32(command), addr + priv->iobase);
}

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
static int INL(struct eepro100_priv *priv, u_long addr)
{
	return le32_to_cpu(readl(addr + priv->iobase));
}

static int get_phyreg(struct eepro100_priv *priv, unsigned char addr,
		      unsigned char reg, unsigned short *value)
{
	int timeout = 50;
	int cmd;

	/* read requested data */
	cmd = (2 << 26) | ((addr & 0x1f) << 21) | ((reg & 0x1f) << 16);
	OUTL(priv, cmd, SCB_CTRL_MDI);

	do {
		udelay(1000);
		cmd = INL(priv, SCB_CTRL_MDI);
	} while (!(cmd & (1 << 28)) && (--timeout));

	if (timeout == 0)
		return -1;

	*value = (unsigned short)(cmd & 0xffff);

	return 0;
}

static int set_phyreg(struct eepro100_priv *priv, unsigned char addr,
		      unsigned char reg, unsigned short value)
{
	int timeout = 50;
	int cmd;

	/* write requested data */
	cmd = (1 << 26) | ((addr & 0x1f) << 21) | ((reg & 0x1f) << 16);
	OUTL(priv, cmd | value, SCB_CTRL_MDI);

	while (!(INL(priv, SCB_CTRL_MDI) & (1 << 28)) && (--timeout))
		udelay(1000);

	if (timeout == 0)
		return -1;

	return 0;
}

/*
 * Check if given phyaddr is valid, i.e. there is a PHY connected.
 * Do this by checking model value field from ID2 register.
 */
static int verify_phyaddr(struct eepro100_priv *priv, unsigned char addr)
{
	unsigned short value, model;
	int ret;

	/* read id2 register */
	ret = get_phyreg(priv, addr, MII_PHYSID2, &value);
	if (ret) {
		printf("%s: mii read timeout!\n", priv->name);
		return ret;
	}

	/* get model */
	model = (value >> 4) & 0x003f;
	if (!model) {
		printf("%s: no PHY at address %d\n", priv->name, addr);
		return -EINVAL;
	}

	return 0;
}

static int eepro100_miiphy_read(struct mii_dev *bus, int addr, int devad,
				int reg)
{
	struct eepro100_priv *priv = bus->priv;
	unsigned short value = 0;
	int ret;

	ret = verify_phyaddr(priv, addr);
	if (ret)
		return ret;

	ret = get_phyreg(priv, addr, reg, &value);
	if (ret) {
		printf("%s: mii read timeout!\n", bus->name);
		return ret;
	}

	return value;
}

static int eepro100_miiphy_write(struct mii_dev *bus, int addr, int devad,
				 int reg, u16 value)
{
	struct eepro100_priv *priv = bus->priv;
	int ret;

	ret = verify_phyaddr(priv, addr);
	if (ret)
		return ret;

	ret = set_phyreg(priv, addr, reg, value);
	if (ret) {
		printf("%s: mii write timeout!\n", bus->name);
		return ret;
	}

	return 0;
}
#endif

static void init_rx_ring(struct eepro100_priv *priv)
{
	struct eepro100_rxfd *rx_ring = priv->rx_ring;
	int i;

	for (i = 0; i < NUM_RX_DESC; i++) {
		rx_ring[i].status = 0;
		rx_ring[i].control = (i == NUM_RX_DESC - 1) ?
				     cpu_to_le16 (RFD_CONTROL_S) : 0;
		rx_ring[i].link =
			cpu_to_le32(phys_to_bus(priv->devno,
						(u32)&rx_ring[(i + 1) %
						NUM_RX_DESC]));
		rx_ring[i].rx_buf_addr = 0xffffffff;
		rx_ring[i].count = cpu_to_le32(PKTSIZE_ALIGN << 16);
	}

	flush_dcache_range((unsigned long)rx_ring,
			   (unsigned long)rx_ring +
			   (sizeof(*rx_ring) * NUM_RX_DESC));

	priv->rx_next = 0;
}

static void purge_tx_ring(struct eepro100_priv *priv)
{
	struct eepro100_txfd *tx_ring = priv->tx_ring;

	priv->tx_next = 0;
	priv->tx_threshold = 0x01208000;
	memset(tx_ring, 0, sizeof(*tx_ring) * NUM_TX_DESC);

	flush_dcache_range((unsigned long)tx_ring,
			   (unsigned long)tx_ring +
			   (sizeof(*tx_ring) * NUM_TX_DESC));
}

/* Wait for the chip get the command. */
static int wait_for_eepro100(struct eepro100_priv *priv)
{
	int i;

	for (i = 0; INW(priv, SCB_CMD) & (CU_CMD_MASK | RU_CMD_MASK); i++) {
		if (i >= TOUT_LOOP)
			return 0;
	}

	return 1;
}

static int eepro100_txcmd_send(struct eepro100_priv *priv,
			       struct eepro100_txfd *desc)
{
	u16 rstat;
	int i = 0;

	flush_dcache_range((unsigned long)desc,
			   (unsigned long)desc + sizeof(*desc));

	if (!wait_for_eepro100(priv))
		return -ETIMEDOUT;

	OUTL(priv, phys_to_bus(priv->devno, (u32)desc), SCB_POINTER);
	OUTW(priv, SCB_M | CU_START, SCB_CMD);

	while (true) {
		invalidate_dcache_range((unsigned long)desc,
					(unsigned long)desc + sizeof(*desc));
		rstat = le16_to_cpu(desc->status);
		if (rstat & CFG_SYS_STATUS_C)
			break;

		if (i++ >= TOUT_LOOP) {
			printf("%s: Tx error buffer not ready\n", priv->name);
			return -EINVAL;
		}
	}

	invalidate_dcache_range((unsigned long)desc,
				(unsigned long)desc + sizeof(*desc));
	rstat = le16_to_cpu(desc->status);

	if (!(rstat & CFG_SYS_STATUS_OK)) {
		printf("TX error status = 0x%08X\n", rstat);
		return -EIO;
	}

	return 0;
}

/* SROM Read. */
static int read_eeprom(struct eepro100_priv *priv, int location, int addr_len)
{
	unsigned short retval = 0;
	int read_cmd = location | EE_READ_CMD(addr_len);
	int i;

	OUTW(priv, EE_ENB & ~EE_CS, SCB_EEPROM);
	OUTW(priv, EE_ENB, SCB_EEPROM);

	/* Shift the read command bits out. */
	for (i = 12; i >= 0; i--) {
		short dataval = (read_cmd & (1 << i)) ? EE_DATA_WRITE : 0;

		OUTW(priv, EE_ENB | dataval, SCB_EEPROM);
		udelay(1);
		OUTW(priv, EE_ENB | dataval | EE_SHIFT_CLK, SCB_EEPROM);
		udelay(1);
	}
	OUTW(priv, EE_ENB, SCB_EEPROM);

	for (i = 15; i >= 0; i--) {
		OUTW(priv, EE_ENB | EE_SHIFT_CLK, SCB_EEPROM);
		udelay(1);
		retval = (retval << 1) |
			 !!(INW(priv, SCB_EEPROM) & EE_DATA_READ);
		OUTW(priv, EE_ENB, SCB_EEPROM);
		udelay(1);
	}

	/* Terminate the EEPROM access. */
	OUTW(priv, EE_ENB & ~EE_CS, SCB_EEPROM);
	return retval;
}

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
static int eepro100_initialize_mii(struct eepro100_priv *priv)
{
	/* register mii command access routines */
	struct mii_dev *mdiodev;
	int ret;

	mdiodev = mdio_alloc();
	if (!mdiodev)
		return -ENOMEM;

	strlcpy(mdiodev->name, priv->name, MDIO_NAME_LEN);
	mdiodev->read = eepro100_miiphy_read;
	mdiodev->write = eepro100_miiphy_write;
	mdiodev->priv = priv;

	ret = mdio_register(mdiodev);
	if (ret < 0) {
		mdio_free(mdiodev);
		return ret;
	}

	return 0;
}
#else
static int eepro100_initialize_mii(struct eepro100_priv *priv)
{
	return 0;
}
#endif

static struct pci_device_id supported[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82557) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82559) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82559ER) },
	{ }
};

static void eepro100_get_hwaddr(struct eepro100_priv *priv)
{
	u16 sum = 0;
	int i, j;
	int addr_len = read_eeprom(priv, 0, 6) == 0xffff ? 8 : 6;

	for (j = 0, i = 0; i < 0x40; i++) {
		u16 value = read_eeprom(priv, i, addr_len);

		sum += value;
		if (i < 3) {
			priv->enetaddr[j++] = value;
			priv->enetaddr[j++] = value >> 8;
		}
	}

	if (sum != 0xBABA) {
		memset(priv->enetaddr, 0, ETH_ALEN);
		debug("%s: Invalid EEPROM checksum %#4.4x, check settings before activating this device!\n",
		      priv->name, sum);
	}
}

static int eepro100_init_common(struct eepro100_priv *priv)
{
	struct eepro100_rxfd *rx_ring = priv->rx_ring;
	struct eepro100_txfd *tx_ring = priv->tx_ring;
	struct eepro100_txfd *ias_cmd, *cfg_cmd;
	int ret, status = -1;
	int tx_cur;

	/* Reset the ethernet controller */
	OUTL(priv, I82559_SELECTIVE_RESET, SCB_PORT);
	udelay(20);

	OUTL(priv, I82559_RESET, SCB_PORT);
	udelay(20);

	if (!wait_for_eepro100(priv)) {
		printf("Error: Can not reset ethernet controller.\n");
		goto done;
	}
	OUTL(priv, 0, SCB_POINTER);
	OUTW(priv, SCB_M | RUC_ADDR_LOAD, SCB_CMD);

	if (!wait_for_eepro100(priv)) {
		printf("Error: Can not reset ethernet controller.\n");
		goto done;
	}
	OUTL(priv, 0, SCB_POINTER);
	OUTW(priv, SCB_M | CU_ADDR_LOAD, SCB_CMD);

	/* Initialize Rx and Tx rings. */
	init_rx_ring(priv);
	purge_tx_ring(priv);

	/* Tell the adapter where the RX ring is located. */
	if (!wait_for_eepro100(priv)) {
		printf("Error: Can not reset ethernet controller.\n");
		goto done;
	}

	/* RX ring cache was already flushed in init_rx_ring() */
	OUTL(priv, phys_to_bus(priv->devno, (u32)&rx_ring[priv->rx_next]),
	     SCB_POINTER);
	OUTW(priv, SCB_M | RUC_START, SCB_CMD);

	/* Send the Configure frame */
	tx_cur = priv->tx_next;
	priv->tx_next = ((priv->tx_next + 1) % NUM_TX_DESC);

	cfg_cmd = &tx_ring[tx_cur];
	cfg_cmd->command = cpu_to_le16(CFG_SYS_CMD_SUSPEND |
				       CFG_SYS_CMD_CONFIGURE);
	cfg_cmd->status = 0;
	cfg_cmd->link = cpu_to_le32(phys_to_bus(priv->devno,
						(u32)&tx_ring[priv->tx_next]));

	memcpy(((struct descriptor *)cfg_cmd)->params, i82558_config_cmd,
	       sizeof(i82558_config_cmd));

	ret = eepro100_txcmd_send(priv, cfg_cmd);
	if (ret) {
		if (ret == -ETIMEDOUT)
			printf("Error---CFG_SYS_CMD_CONFIGURE: Can not reset ethernet controller.\n");
		goto done;
	}

	/* Send the Individual Address Setup frame */
	tx_cur = priv->tx_next;
	priv->tx_next = ((priv->tx_next + 1) % NUM_TX_DESC);

	ias_cmd = &tx_ring[tx_cur];
	ias_cmd->command = cpu_to_le16(CFG_SYS_CMD_SUSPEND |
				       CFG_SYS_CMD_IAS);
	ias_cmd->status = 0;
	ias_cmd->link = cpu_to_le32(phys_to_bus(priv->devno,
						(u32)&tx_ring[priv->tx_next]));

	memcpy(((struct descriptor *)ias_cmd)->params, priv->enetaddr, 6);

	ret = eepro100_txcmd_send(priv, ias_cmd);
	if (ret) {
		if (ret == -ETIMEDOUT)
			printf("Error: Can not reset ethernet controller.\n");
		goto done;
	}

	status = 0;

done:
	return status;
}

static int eepro100_send_common(struct eepro100_priv *priv,
				void *packet, int length)
{
	struct eepro100_txfd *tx_ring = priv->tx_ring;
	struct eepro100_txfd *desc;
	int ret, status = -1;
	int tx_cur;

	if (length <= 0) {
		printf("%s: bad packet size: %d\n", priv->name, length);
		goto done;
	}

	tx_cur = priv->tx_next;
	priv->tx_next = (priv->tx_next + 1) % NUM_TX_DESC;

	desc = &tx_ring[tx_cur];
	desc->command = cpu_to_le16(TXCB_CMD_TRANSMIT | TXCB_CMD_SF |
				    TXCB_CMD_S | TXCB_CMD_EL);
	desc->status = 0;
	desc->count = cpu_to_le32(priv->tx_threshold);
	desc->link = cpu_to_le32(phys_to_bus(priv->devno,
					     (u32)&tx_ring[priv->tx_next]));
	desc->tx_desc_addr = cpu_to_le32(phys_to_bus(priv->devno,
						     (u32)&desc->tx_buf_addr0));
	desc->tx_buf_addr0 = cpu_to_le32(phys_to_bus(priv->devno,
						     (u_long)packet));
	desc->tx_buf_size0 = cpu_to_le32(length);

	ret = eepro100_txcmd_send(priv, &tx_ring[tx_cur]);
	if (ret) {
		if (ret == -ETIMEDOUT)
			printf("%s: Tx error ethernet controller not ready.\n",
			       priv->name);
		goto done;
	}

	status = length;

done:
	return status;
}

static int eepro100_recv_common(struct eepro100_priv *priv, uchar **packetp)
{
	struct eepro100_rxfd *rx_ring = priv->rx_ring;
	struct eepro100_rxfd *desc;
	int length;
	u16 status;

	priv->rx_stat = INW(priv, SCB_STATUS);
	OUTW(priv, priv->rx_stat & SCB_STATUS_RNR, SCB_STATUS);

	desc = &rx_ring[priv->rx_next];
	invalidate_dcache_range((unsigned long)desc,
				(unsigned long)desc + sizeof(*desc));
	status = le16_to_cpu(desc->status);

	if (!(status & RFD_STATUS_C))
		return 0;

	/* Valid frame status. */
	if (status & RFD_STATUS_OK) {
		/* A valid frame received. */
		length = le32_to_cpu(desc->count) & 0x3fff;
		/* Pass the packet up to the protocol layers. */
		*packetp = desc->data;
		return length;
	}

	/* There was an error. */
	printf("RX error status = 0x%08X\n", status);
	return -EINVAL;
}

static void eepro100_free_pkt_common(struct eepro100_priv *priv)
{
	struct eepro100_rxfd *rx_ring = priv->rx_ring;
	struct eepro100_rxfd *desc;
	int rx_prev;

	desc = &rx_ring[priv->rx_next];

	desc->control = cpu_to_le16(RFD_CONTROL_S);
	desc->status = 0;
	desc->count = cpu_to_le32(PKTSIZE_ALIGN << 16);
	flush_dcache_range((unsigned long)desc,
			   (unsigned long)desc + sizeof(*desc));

	rx_prev = (priv->rx_next + NUM_RX_DESC - 1) % NUM_RX_DESC;
	desc = &rx_ring[rx_prev];
	desc->control = 0;
	flush_dcache_range((unsigned long)desc,
			   (unsigned long)desc + sizeof(*desc));

	/* Update entry information. */
	priv->rx_next = (priv->rx_next + 1) % NUM_RX_DESC;

	if (!(priv->rx_stat & SCB_STATUS_RNR))
		return;

	printf("%s: Receiver is not ready, restart it !\n", priv->name);

	/* Reinitialize Rx ring. */
	init_rx_ring(priv);

	if (!wait_for_eepro100(priv)) {
		printf("Error: Can not restart ethernet controller.\n");
		return;
	}

	/* RX ring cache was already flushed in init_rx_ring() */
	OUTL(priv, phys_to_bus(priv->devno, (u32)&rx_ring[priv->rx_next]),
	     SCB_POINTER);
	OUTW(priv, SCB_M | RUC_START, SCB_CMD);
}

static void eepro100_halt_common(struct eepro100_priv *priv)
{
	/* Reset the ethernet controller */
	OUTL(priv, I82559_SELECTIVE_RESET, SCB_PORT);
	udelay(20);

	OUTL(priv, I82559_RESET, SCB_PORT);
	udelay(20);

	if (!wait_for_eepro100(priv)) {
		printf("Error: Can not reset ethernet controller.\n");
		goto done;
	}
	OUTL(priv, 0, SCB_POINTER);
	OUTW(priv, SCB_M | RUC_ADDR_LOAD, SCB_CMD);

	if (!wait_for_eepro100(priv)) {
		printf("Error: Can not reset ethernet controller.\n");
		goto done;
	}
	OUTL(priv, 0, SCB_POINTER);
	OUTW(priv, SCB_M | CU_ADDR_LOAD, SCB_CMD);

done:
	return;
}

static int eepro100_start(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_plat(dev);
	struct eepro100_priv *priv = dev_get_priv(dev);

	memcpy(priv->enetaddr, plat->enetaddr, sizeof(plat->enetaddr));

	return eepro100_init_common(priv);
}

static void eepro100_stop(struct udevice *dev)
{
	struct eepro100_priv *priv = dev_get_priv(dev);

	eepro100_halt_common(priv);
}

static int eepro100_send(struct udevice *dev, void *packet, int length)
{
	struct eepro100_priv *priv = dev_get_priv(dev);
	int ret;

	ret = eepro100_send_common(priv, packet, length);

	return ret ? 0 : -ETIMEDOUT;
}

static int eepro100_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct eepro100_priv *priv = dev_get_priv(dev);

	return eepro100_recv_common(priv, packetp);
}

static int eepro100_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct eepro100_priv *priv = dev_get_priv(dev);

	eepro100_free_pkt_common(priv);

	return 0;
}

static int eepro100_read_rom_hwaddr(struct udevice *dev)
{
	struct eepro100_priv *priv = dev_get_priv(dev);

	eepro100_get_hwaddr(priv);

	return 0;
}

static int eepro100_bind(struct udevice *dev)
{
	static int card_number;
	char name[16];

	sprintf(name, "eepro100#%u", card_number++);

	return device_set_name(dev, name);
}

static int eepro100_probe(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_plat(dev);
	struct eepro100_priv *priv = dev_get_priv(dev);
	u16 command, status;
	u32 iobase;
	int ret;

	dm_pci_read_config32(dev, PCI_BASE_ADDRESS_0, &iobase);
	iobase &= ~0xf;

	debug("eepro100: Intel i82559 PCI EtherExpressPro @0x%x\n", iobase);

	priv->devno = dev;
	priv->enetaddr = plat->enetaddr;
	priv->iobase = (void __iomem *)bus_to_phys(dev, iobase);

	command = PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
	dm_pci_write_config16(dev, PCI_COMMAND, command);
	dm_pci_read_config16(dev, PCI_COMMAND, &status);
	if ((status & command) != command) {
		printf("eepro100: Couldn't enable IO access or Bus Mastering\n");
		return -EINVAL;
	}

	ret = eepro100_initialize_mii(priv);
	if (ret)
		return ret;

	dm_pci_write_config8(dev, PCI_LATENCY_TIMER, 0x20);

	return 0;
}

static const struct eth_ops eepro100_ops = {
	.start		= eepro100_start,
	.send		= eepro100_send,
	.recv		= eepro100_recv,
	.stop		= eepro100_stop,
	.free_pkt	= eepro100_free_pkt,
	.read_rom_hwaddr = eepro100_read_rom_hwaddr,
};

U_BOOT_DRIVER(eth_eepro100) = {
	.name	= "eth_eepro100",
	.id	= UCLASS_ETH,
	.bind	= eepro100_bind,
	.probe	= eepro100_probe,
	.ops	= &eepro100_ops,
	.priv_auto	= sizeof(struct eepro100_priv),
	.plat_auto	= sizeof(struct eth_pdata),
};

U_BOOT_PCI_DEVICE(eth_eepro100, supported);
