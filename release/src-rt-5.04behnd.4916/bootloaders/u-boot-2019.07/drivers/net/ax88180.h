/* ax88180.h: ASIX AX88180 Non-PCI Gigabit Ethernet u-boot driver */
/*
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */

#ifndef _AX88180_H_
#define _AX88180_H_

#include <asm/io.h>
#include <asm/types.h>
#include <config.h>

typedef enum _ax88180_link_state {
	INS_LINK_DOWN,
	INS_LINK_UP,
	INS_LINK_UNKNOWN
} ax88180_link_state;

struct ax88180_private {
	unsigned char BusWidth;
	unsigned char PadSize;
	unsigned short PhyAddr;
	unsigned short PhyID0;
	unsigned short PhyID1;
	unsigned short FirstTxDesc;
	unsigned short NextTxDesc;
	ax88180_link_state LinkState;
};

#define BUS_WIDTH_16			1
#define BUS_WIDTH_32			2

#define ENABLE_JUMBO			1
#define DISABLE_JUMBO			0

#define ENABLE_BURST			1
#define DISABLE_BURST			0

#define NORMAL_RX_MODE		0
#define RX_LOOPBACK_MODE		1
#define RX_INIFINIT_LOOP_MODE		2
#define TX_INIFINIT_LOOP_MODE		3

#define DEFAULT_ETH_MTU		1500

/* Jumbo packet size 4086 bytes included 4 bytes CRC*/
#define MAX_JUMBO_MTU		4072

/* Max Tx Jumbo size 4086 bytes included 4 bytes CRC */
#define MAX_TX_JUMBO_SIZE		4086

/* Max Rx Jumbo size is 15K Bytes */
#define MAX_RX_SIZE			0x3C00

#define MARVELL_ALASKA_PHYSID0	0x141
#define MARVELL_88E1118_PHYSID1	0xE40

#define CICADA_CIS8201_PHYSID0		0x000F

#define MEDIA_AUTO			0
#define MEDIA_1000FULL			1
#define MEDIA_1000HALF			2
#define MEDIA_100FULL			3
#define MEDIA_100HALF			4
#define MEDIA_10FULL			5
#define MEDIA_10HALF			6
#define MEDIA_UNKNOWN		7

#define AUTO_MEDIA			0
#define FORCE_MEDIA			1

#define TXDP_MASK			3
#define TXDP0				0
#define TXDP1				1
#define TXDP2				2
#define TXDP3				3

#define CMD_MAP_SIZE			0x100

#if defined (CONFIG_DRIVER_AX88180_16BIT)
  #define AX88180_MEMORY_SIZE		0x00004000
  #define START_BASE			0x1000

  #define RX_BUF_SIZE			0x1000
  #define TX_BUF_SIZE			0x0F00

  #define TX_BASE			START_BASE
  #define CMD_BASE			(TX_BASE + TX_BUF_SIZE)
  #define RX_BASE			(CMD_BASE + CMD_MAP_SIZE)
#else
  #define AX88180_MEMORY_SIZE	0x00010000

  #define RX_BUF_SIZE			0x8000
  #define TX_BUF_SIZE			0x7C00

  #define RX_BASE			0x0000
  #define TX_BASE			(RX_BASE + RX_BUF_SIZE)
  #define CMD_BASE			(TX_BASE + TX_BUF_SIZE)
#endif

/* AX88180 Memory Mapping Definition */
#define RXBUFFER_START			RX_BASE
  #define RX_PACKET_LEN_OFFSET	0
  #define RX_PAGE_NUM_MASK		0x7FF	/* RX pages 0~7FFh */
#define TXBUFFER_START			TX_BASE

/* AX88180 MAC Register Definition */
#define DECODE		(0)
  #define DECODE_EN		0x00000001
#define BASE		(6)
#define CMD		(CMD_BASE + 0x0000)
  #define WAKEMOD		0x00000001
  #define TXEN			0x00000100
  #define RXEN			0x00000200
  #define DEFAULT_CMD		WAKEMOD
#define IMR		(CMD_BASE + 0x0004)
  #define IMR_RXBUFFOVR	0x00000001
  #define IMR_WATCHDOG	0x00000002
  #define IMR_TX		0x00000008
  #define IMR_RX		0x00000010
  #define IMR_PHY		0x00000020
  #define CLEAR_IMR		0x00000000
  #define DEFAULT_IMR		(IMR_PHY | IMR_RX | IMR_TX |\
					 IMR_RXBUFFOVR | IMR_WATCHDOG)
#define ISR		(CMD_BASE + 0x0008)
  #define ISR_RXBUFFOVR	0x00000001
  #define ISR_WATCHDOG	0x00000002
  #define ISR_TX			0x00000008
  #define ISR_RX			0x00000010
  #define ISR_PHY		0x00000020
#define TXCFG		(CMD_BASE + 0x0010)
  #define AUTOPAD_CRC		0x00000050
  #define DEFAULT_TXCFG	AUTOPAD_CRC
#define TXCMD		(CMD_BASE + 0x0014)
  #define TXCMD_TXDP_MASK	0x00006000
  #define TXCMD_TXDP0		0x00000000
  #define TXCMD_TXDP1		0x00002000
  #define TXCMD_TXDP2		0x00004000
  #define TXCMD_TXDP3		0x00006000
  #define TX_START_WRITE	0x00008000
  #define TX_STOP_WRITE		0x00000000
  #define DEFAULT_TXCMD	0x00000000
#define TXBS		(CMD_BASE + 0x0018)
  #define TXDP0_USED		0x00000001
  #define TXDP1_USED		0x00000002
  #define TXDP2_USED		0x00000004
  #define TXDP3_USED		0x00000008
  #define DEFAULT_TXBS		0x00000000
#define TXDES0		(CMD_BASE + 0x0020)
  #define TXDPx_ENABLE		0x00008000
  #define TXDPx_LEN_MASK	0x00001FFF
  #define DEFAULT_TXDES0	0x00000000
#define TXDES1		(CMD_BASE + 0x0024)
  #define TXDPx_ENABLE		0x00008000
  #define TXDPx_LEN_MASK	0x00001FFF
  #define DEFAULT_TXDES1	0x00000000
#define TXDES2		(CMD_BASE + 0x0028)
  #define TXDPx_ENABLE		0x00008000
  #define TXDPx_LEN_MASK	0x00001FFF
  #define DEFAULT_TXDES2	0x00000000
#define TXDES3		(CMD_BASE + 0x002C)
  #define TXDPx_ENABLE		0x00008000
  #define TXDPx_LEN_MASK	0x00001FFF
  #define DEFAULT_TXDES3	0x00000000
#define RXCFG		(CMD_BASE + 0x0030)
  #define RXBUFF_PROTECT	0x00000001
  #define RXTCPCRC_CHECK	0x00000010
  #define RXFLOW_ENABLE	0x00000100
  #define DEFAULT_RXCFG	RXBUFF_PROTECT
#define RXCURT		(CMD_BASE + 0x0034)
  #define DEFAULT_RXCURT	0x00000000
#define RXBOUND	(CMD_BASE + 0x0038)
  #define DEFAULT_RXBOUND	0x7FF		/* RX pages 0~7FFh */
#define MACCFG0	(CMD_BASE + 0x0040)
  #define MACCFG0_BIT3_0	0x00000007
  #define IPGT_VAL		0x00000150
  #define TXFLOW_ENABLE	0x00001000
  #define SPEED100		0x00008000
  #define DEFAULT_MACCFG0	(IPGT_VAL | MACCFG0_BIT3_0)
#define MACCFG1	(CMD_BASE + 0x0044)
  #define RGMII_EN		0x00000002
  #define RXFLOW_EN		0x00000020
  #define FULLDUPLEX		0x00000040
  #define MAX_JUMBO_LEN	0x00000780
  #define RXJUMBO_EN		0x00000800
  #define GIGA_MODE_EN	0x00001000
  #define RXCRC_CHECK		0x00002000
  #define RXPAUSE_DA_CHECK	0x00004000

  #define JUMBO_LEN_4K		0x00000200
  #define JUMBO_LEN_15K	0x00000780
  #define DEFAULT_MACCFG1	(RXCRC_CHECK | RXPAUSE_DA_CHECK | \
				 RGMII_EN)
  #define CICADA_DEFAULT_MACCFG1	(RXCRC_CHECK | RXPAUSE_DA_CHECK)
#define MACCFG2		(CMD_BASE + 0x0048)
  #define MACCFG2_BIT15_8	0x00000100
  #define JAM_LIMIT_MASK	0x000000FC
  #define DEFAULT_JAM_LIMIT	0x00000064
  #define DEFAULT_MACCFG2	MACCFG2_BIT15_8
#define MACCFG3		(CMD_BASE + 0x004C)
  #define IPGR2_VAL		0x0000000E
  #define IPGR1_VAL		0x00000600
  #define NOABORT		0x00008000
  #define DEFAULT_MACCFG3	(IPGR1_VAL | IPGR2_VAL)
#define TXPAUT		(CMD_BASE + 0x0054)
  #define DEFAULT_TXPAUT	0x001FE000
#define RXBTHD0		(CMD_BASE + 0x0058)
  #define DEFAULT_RXBTHD0	0x00000300
#define RXBTHD1		(CMD_BASE + 0x005C)
  #define DEFAULT_RXBTHD1	0x00000600
#define RXFULTHD	(CMD_BASE + 0x0060)
  #define DEFAULT_RXFULTHD	0x00000100
#define MISC		(CMD_BASE + 0x0068)
  /* Normal operation mode */
  #define MISC_NORMAL		0x00000003
  /* Clear bit 0 to reset MAC */
  #define MISC_RESET_MAC	0x00000002
  /* Clear bit 1 to reset PHY */
  #define MISC_RESET_PHY	0x00000001
  /* Clear bit 0 and 1 to reset MAC and PHY */
  #define MISC_RESET_MAC_PHY	0x00000000
  #define DEFAULT_MISC		MISC_NORMAL
#define MACID0		(CMD_BASE + 0x0070)
#define MACID1		(CMD_BASE + 0x0074)
#define MACID2		(CMD_BASE + 0x0078)
#define TXLEN		(CMD_BASE + 0x007C)
  #define DEFAULT_TXLEN	0x000005FC
#define RXFILTER	(CMD_BASE + 0x0080)
  #define RX_RXANY		0x00000001
  #define RX_MULTICAST		0x00000002
  #define RX_UNICAST		0x00000004
  #define RX_BROADCAST	0x00000008
  #define RX_MULTI_HASH	0x00000010
  #define DISABLE_RXFILTER	0x00000000
  #define DEFAULT_RXFILTER	(RX_BROADCAST + RX_UNICAST)
#define MDIOCTRL	(CMD_BASE + 0x0084)
  #define PHY_ADDR_MASK	0x0000001F
  #define REG_ADDR_MASK	0x00001F00
  #define READ_PHY		0x00004000
  #define WRITE_PHY		0x00008000
#define MDIODP		(CMD_BASE + 0x0088)
#define GPIOCTRL	(CMD_BASE + 0x008C)
#define RXINDICATOR	(CMD_BASE + 0x0090)
  #define RX_START_READ	0x00000001
  #define RX_STOP_READ		0x00000000
  #define DEFAULT_RXINDICATOR	RX_STOP_READ
#define TXST		(CMD_BASE + 0x0094)
#define MDCCLKPAT	(CMD_BASE + 0x00A0)
#define RXIPCRCCNT	(CMD_BASE + 0x00A4)
#define RXCRCCNT	(CMD_BASE + 0x00A8)
#define TXFAILCNT	(CMD_BASE + 0x00AC)
#define PROMDP		(CMD_BASE + 0x00B0)
#define PROMCTRL	(CMD_BASE + 0x00B4)
  #define RELOAD_EEPROM	0x00000200
#define MAXRXLEN	(CMD_BASE + 0x00B8)
#define HASHTAB0	(CMD_BASE + 0x00C0)
#define HASHTAB1	(CMD_BASE + 0x00C4)
#define HASHTAB2	(CMD_BASE + 0x00C8)
#define HASHTAB3	(CMD_BASE + 0x00CC)
#define DOGTHD0	(CMD_BASE + 0x00E0)
  #define DEFAULT_DOGTHD0	0x0000FFFF
#define DOGTHD1	(CMD_BASE + 0x00E4)
  #define START_WATCHDOG_TIMER	0x00008000
  #define DEFAULT_DOGTHD1		0x00000FFF
#define SOFTRST		(CMD_BASE + 0x00EC)
  #define SOFTRST_NORMAL	0x00000003
  #define SOFTRST_RESET_MAC	0x00000002

/* Marvell 88E1111 Gigabit PHY Register Definition */
#define M88_SSR		0x0011
  #define SSR_SPEED_MASK	0xC000
  #define SSR_SPEED_1000		0x8000
  #define SSR_SPEED_100		0x4000
  #define SSR_SPEED_10		0x0000
  #define SSR_DUPLEX		0x2000
  #define SSR_MEDIA_RESOLVED_OK	0x0800

  #define SSR_MEDIA_MASK	(SSR_SPEED_MASK | SSR_DUPLEX)
  #define SSR_1000FULL		(SSR_SPEED_1000 | SSR_DUPLEX)
  #define SSR_1000HALF		SSR_SPEED_1000
  #define SSR_100FULL		(SSR_SPEED_100 | SSR_DUPLEX)
  #define SSR_100HALF		SSR_SPEED_100
  #define SSR_10FULL		(SSR_SPEED_10 | SSR_DUPLEX)
  #define SSR_10HALF		SSR_SPEED_10
#define M88_IER		0x0012
  #define LINK_CHANGE_INT	0x0400
#define M88_ISR		0x0013
  #define LINK_CHANGE_STATUS	0x0400
#define M88E1111_EXT_SCR	0x0014
  #define RGMII_RXCLK_DELAY	0x0080
  #define RGMII_TXCLK_DELAY	0x0002
  #define DEFAULT_EXT_SCR	(RGMII_TXCLK_DELAY | RGMII_RXCLK_DELAY)
#define M88E1111_EXT_SSR	0x001B
  #define HWCFG_MODE_MASK	0x000F
  #define RGMII_COPPER_MODE	0x000B

/* Marvell 88E1118 Gigabit PHY Register Definition */
#define M88E1118_CR			0x14
  #define M88E1118_CR_RGMII_RXCLK_DELAY	0x0020
  #define M88E1118_CR_RGMII_TXCLK_DELAY	0x0010
  #define M88E1118_CR_DEFAULT		(M88E1118_CR_RGMII_TXCLK_DELAY | \
					 M88E1118_CR_RGMII_RXCLK_DELAY)
#define M88E1118_LEDCTL		0x10		/* Reg 16 on page 3 */
  #define M88E1118_LEDCTL_LED2INT			0x200
  #define M88E1118_LEDCTL_LED2BLNK			0x400
  #define M88E1118_LEDCTL_LED0DUALMODE1	0xc
  #define M88E1118_LEDCTL_LED0DUALMODE2	0xd
  #define M88E1118_LEDCTL_LED0DUALMODE3	0xe
  #define M88E1118_LEDCTL_LED0DUALMODE4	0xf
  #define M88E1118_LEDCTL_DEFAULT	(M88E1118_LEDCTL_LED2BLNK | \
					 M88E1118_LEDCTL_LED0DUALMODE4)

#define M88E1118_LEDMIX		0x11		/* Reg 17 on page 3 */
  #define M88E1118_LEDMIX_LED050				0x4
  #define M88E1118_LEDMIX_LED150				0x8

#define M88E1118_PAGE_SEL	0x16		/* Reg page select */

/* CICADA CIS8201 Gigabit PHY Register Definition */
#define CIS_IMR		0x0019
  #define CIS_INT_ENABLE	0x8000
  #define CIS_LINK_CHANGE_INT	0x2000
#define CIS_ISR		0x001A
  #define CIS_INT_PENDING	0x8000
  #define CIS_LINK_CHANGE_STATUS	0x2000
#define CIS_AUX_CTRL_STATUS	0x001C
  #define CIS_AUTONEG_COMPLETE	0x8000
  #define CIS_SPEED_MASK	0x0018
  #define CIS_SPEED_1000		0x0010
  #define CIS_SPEED_100		0x0008
  #define CIS_SPEED_10		0x0000
  #define CIS_DUPLEX		0x0020

  #define CIS_MEDIA_MASK	(CIS_SPEED_MASK | CIS_DUPLEX)
  #define CIS_1000FULL		(CIS_SPEED_1000 | CIS_DUPLEX)
  #define CIS_1000HALF		CIS_SPEED_1000
  #define CIS_100FULL		(CIS_SPEED_100 | CIS_DUPLEX)
  #define CIS_100HALF		CIS_SPEED_100
  #define CIS_10FULL		(CIS_SPEED_10 | CIS_DUPLEX)
  #define CIS_10HALF		CIS_SPEED_10
  #define CIS_SMI_PRIORITY	0x0004

static inline unsigned short INW (struct eth_device *dev, unsigned long addr)
{
	return le16_to_cpu(readw(addr + (void *)dev->iobase));
}

/*
 Access RXBUFFER_START/TXBUFFER_START to read RX buffer/write TX buffer
*/
#if defined (CONFIG_DRIVER_AX88180_16BIT)
static inline void OUTW (struct eth_device *dev, unsigned short command, unsigned long addr)
{
	writew(cpu_to_le16(command), addr + (void *)dev->iobase);
}

static inline unsigned short READ_RXBUF (struct eth_device *dev)
{
	return le16_to_cpu(readw(RXBUFFER_START + (void *)dev->iobase));
}

static inline void WRITE_TXBUF (struct eth_device *dev, unsigned short data)
{
	writew(cpu_to_le16(data), TXBUFFER_START + (void *)dev->iobase);
}
#else
static inline void OUTW (struct eth_device *dev, unsigned short command, unsigned long addr)
{
	writel(cpu_to_le32(command), addr + (void *)dev->iobase);
}

static inline unsigned long READ_RXBUF (struct eth_device *dev)
{
	return le32_to_cpu(readl(RXBUFFER_START + (void *)dev->iobase));
}

static inline void WRITE_TXBUF (struct eth_device *dev, unsigned long data)
{
	writel(cpu_to_le32(data), TXBUFFER_START + (void *)dev->iobase);
}
#endif

#endif /* _AX88180_H_ */
