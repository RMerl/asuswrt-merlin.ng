/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  MII register definitions			File: mii.h
    *  
    *  Register and bit definitions for the standard MII management
    *  interface.
    *  
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *  
    *********************************************************************  
    *
    *  Copyright 2000,2001,2002,2003
    *  Broadcom Corporation. All rights reserved.
    *  
    *  This software is furnished under license and may be used and 
    *  copied only in accordance with the following terms and 
    *  conditions.  Subject to these conditions, you may download, 
    *  copy, install, use, modify and distribute modified or unmodified 
    *  copies of this software in source and/or binary form.  No title 
    *  or ownership is transferred hereby.
    *  
    *  1) Any source code used, modified or distributed must reproduce 
    *     and retain this copyright notice and list of conditions 
    *     as they appear in the source file.
    *  
    *  2) No right is granted to use any trade name, trademark, or 
    *     logo of Broadcom Corporation.  The "Broadcom Corporation" 
    *     name may not be used to endorse or promote products derived 
    *     from this software without the prior written permission of 
    *     Broadcom Corporation.
    *  
    *  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
    *     IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
    *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
    *     PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
    *     SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
    *     PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
    *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
    *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    *     GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    *     BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
    *     OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
    *     TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
    *     THE POSSIBILITY OF SUCH DAMAGE.
    ********************************************************************* */

#ifndef _MII_H_
#define _MII_H_

/* Access/command codes */

#define	MII_COMMAND_START	0x01
#define	MII_COMMAND_READ	0x02
#define	MII_COMMAND_WRITE	0x01
#define	MII_COMMAND_ACK		0x02


/* Registers */

#define	MII_BMCR	0x00 	/* Basic Mode Control (rw) */
#define	MII_BMSR	0x01	/* Basic Mode Status (ro) */
#define MII_PHYIDR1	0x02
#define MII_PHYIDR2	0x03
#define MII_ANAR	0x04	/* Autonegotiation Advertisement */
#define	MII_ANLPAR	0x05	/* Autonegotiation Link Partner Ability (rw) */
#define MII_ANER	0x06	/* Autonegotiation Expansion */
#define MII_K1CTL	0x09	/* 1000baseT control */
#define MII_K1STSR	0x0A	/* 1K Status Register (ro) */
#define MII_AUXCTL	0x18	/* aux control register */
#define MII_AUXSTA  0x19    /* aux status register */


/* Basic Mode Control register (RW) */

#define BMCR_RESET		0x8000
#define BMCR_LOOPBACK		0x4000
#define BMCR_SPEED0		0x2000
#define BMCR_ANENABLE		0x1000
#define BMCR_POWERDOWN		0x0800
#define BMCR_ISOLATE		0x0400
#define BMCR_RESTARTAN		0x0200
#define BMCR_DUPLEX		0x0100
#define BMCR_COLTEST		0x0080
#define BMCR_SPEED1		0x0040
#define BMCR_SPEED1000		(BMCR_SPEED1)
#define BMCR_SPEED100		(BMCR_SPEED0)
#define BMCR_SPEED10		0


/* Basic Mode Status register (RO) */

#define BMSR_100BT4		0x8000
#define BMSR_100BT_FDX		0x4000
#define BMSR_100BT_HDX  	0x2000
#define BMSR_10BT_FDX   	0x1000
#define BMSR_10BT_HDX   	0x0800
#define BMSR_100BT2_FDX 	0x0400
#define BMSR_100BT2_HDX 	0x0200
#define BMSR_1000BT_XSR		0x0100
#define BMSR_PRESUP		0x0040
#define BMSR_ANCOMPLETE		0x0020
#define BMSR_REMFAULT		0x0010
#define BMSR_AUTONEG		0x0008
#define BMSR_LINKSTAT		0x0004
#define BMSR_JABDETECT		0x0002
#define BMSR_EXTCAPAB		0x0001


/* PHY Identifer registers (RO) */

#define PHYIDR1 		0x2000
#define PHYIDR2			0x5C60


/* Autonegotiation Advertisement register (RW) */

#define ANAR_NP			0x8000
#define ANAR_RF			0x2000
#define ANAR_ASYPAUSE		0x0800
#define ANAR_PAUSE		0x0400
#define ANAR_T4			0x0200
#define ANAR_TXFD		0x0100
#define ANAR_TXHD		0x0080
#define ANAR_10FD		0x0040
#define ANAR_10HD		0x0020
#define ANAR_PSB		0x001F

#define PSB_802_3		0x0001	/* 802.3 */

/* Autonegotiation Link Partner Abilities register (RW) */

#define ANLPAR_NP		0x8000
#define ANLPAR_ACK		0x4000
#define ANLPAR_RF		0x2000
#define ANLPAR_ASYPAUSE		0x0800
#define ANLPAR_PAUSE		0x0400
#define ANLPAR_T4		0x0200
#define ANLPAR_TXFD		0x0100
#define ANLPAR_TXHD		0x0080
#define ANLPAR_10FD		0x0040
#define ANLPAR_10HD		0x0020
#define ANLPAR_PSB		0x001F


/* Autonegotiation Expansion register (RO) */

#define ANER_PDF		0x0010
#define ANER_LPNPABLE		0x0008
#define ANER_NPABLE		0x0004
#define ANER_PAGERX		0x0002
#define ANER_LPANABLE		0x0001


#define ANNPTR_NP		0x8000
#define ANNPTR_MP		0x2000
#define ANNPTR_ACK2		0x1000
#define ANNPTR_TOGTX		0x0800
#define ANNPTR_CODE		0x0008

#define ANNPRR_NP		0x8000
#define ANNPRR_MP		0x2000
#define ANNPRR_ACK3		0x1000
#define ANNPRR_TOGTX		0x0800
#define ANNPRR_CODE		0x0008


#define K1TCR_TESTMODE		0x0000
#define K1TCR_MSMCE		0x1000
#define K1TCR_MSCV		0x0800
#define K1TCR_RPTR		0x0400
#define K1TCR_1000BT_FDX 	0x200
#define K1TCR_1000BT_HDX 	0x100

#define K1STSR_MSMCFLT		0x8000
#define K1STSR_MSCFGRES		0x4000
#define K1STSR_LRSTAT		0x2000
#define K1STSR_RRSTAT		0x1000
#define K1STSR_LP1KFD		0x0800
#define K1STSR_LP1KHD   	0x0400
#define K1STSR_LPASMDIR		0x0200

#define K1SCR_1KX_FDX		0x8000
#define K1SCR_1KX_HDX		0x4000
#define K1SCR_1KT_FDX		0x2000
#define K1SCR_1KT_HDX		0x1000

/*---------------------------------------------------------------------*/
/* Broadcom PHY MII register address                                   */
/* use when PhyType is BP_ENET_INTERNAL_PHY                            */
/*---------------------------------------------------------------------*/
#define MII_ASR                             0x19
#define MII_INTERRUPT                       0x1A
#define MII_RESERVED_1B                     0x1B
#define MII_BRCM_TEST                       0x1F

/* MII ASR register. */
#define MII_ASR_DONE(r) ((r & 0x8000) != 0)
#define MII_ASR_LINK(r) ((r & 0x0004) != 0)
#define MII_ASR_FDX(r)  (((r & 0x0700) == 0x0700) || ((r & 0x0700) == 0x0500) || ((r & 0x0700) == 0x0200))
#define MII_ASR_1000(r) (((r & 0x0700) == 0x0700) || ((r & 0x0700) == 0x0600))
#define MII_ASR_100(r)  (((r & 0x0700) == 0x0500) || ((r & 0x0700) == 0x0300))
#define MII_ASR_10(r)   (((r & 0x0700) == 0x0200) || ((r & 0x0700) == 0x0100))

/* Reserved 0x1B register */
#define MII_RESERVED_1B_ACT_LED             0x0004

/* Broadcom Test register. */
#define MII_BRCM_TEST_SHADOW2_ENABLE        0x0004

/* MII Interrupt register. */
#define MII_INTR_ENABLE                     0x4000

#define BCM50612_PHYID2  0x5E60
#define BCM54610_PHYID2  0xBD63
#define BCM_PHYID_M      0xFFF0

#define MII_REGISTER_1C                     0x1c
    #define MII_1C_WRITE_ENABLE             (1 << 15)
	#define MII_1C_SHADOW_REG_SEL_S         10
	#define MII_1C_SHADOW_REG_SEL_M         0x1F
#define MII_1C_SHADOW_CLK_ALIGN_CTRL        0x3
    #define GTXCLK_DELAY_BYPASS_DISABLE     (1 << 9)
#define MII_1C_SHADOW_LED_CONTROL           0x9
    #define ACT_LINK_LED_ENABLE             (1 << 4)
#define MII_1C_EXTERNAL_CONTROL_1           0xB
    #define LOM_LED_MODE                    (1 << 2)
    #define RGMII_MODE_SEL_M                0x3
    #define RGMII_MODE_SEL_S                3
    #define RGMII_MODE_3P3V                 0x0
    #define RGMII_MODE_2P5V                 0x1
    #define RGMII_MODE_1P8V                 0x2

#define PAGE_CONTROL                        0x00
#define PORT_CTRL_PORT                      0
    #define PORT_CTRL_PORT_STATUS_S         5
    #define PORT_CTRL_PORT_STATUS_M         (7<<PORT_CTRL_PORT_STATUS_S)
    #define PORT_CTRL_NO_STP                (0<<PORT_CTRL_PORT_STATUS_S)
    #define PORT_CTRL_PORT_FORWARDING       (5<<PORT_CTRL_PORT_STATUS_S)
    #define PORT_CTRL_SWITCH_RESERVE        (7<<2)
    #define PORT_CTRL_TX_DISABLE            0x2
    #define PORT_CTRL_RX_DISABLE            0x1
    #define PORT_CTRL_RXTX_DISABLE          (PORT_CTRL_TX_DISABLE|PORT_CTRL_RX_DISABLE)
#define SOFTWARE_RESET_CTRL                 0x79
    #define SOFTWARE_RESET                  (1<<7)
    #define EN_SW_RST                       (1<<4)

#define PAGE_SELECT                         0xff

/* Control page registers */
#define REG_MII_PORT_CONTROL                0x08
#define REG_SWITCH_MODE                     0x0b
#define REG_CONTROL_MII1_PORT_STATE_OVERRIDE 0x0e
#define REG_POWER_DOWN_MODE                 0x0f

#define REG_CONTROL_MPSO_MII_SW_OVERRIDE          0x80
#define REG_CONTROL_MPSO_FLOW_CONTROL             0x30
/* Below two bits : bit03:02 for speed */
#define REG_CONTROL_MPSO_SPEED100                 0x04
#define REG_CONTROL_MPSO_SPEED1000                0x08
#define REG_CONTROL_MPSO_FDX                      0x02
#define REG_CONTROL_MPSO_LINKPASS                 0x01

/* MII Port Control Register, Page 0x00 Address 0x08 */
#define REG_MII_PORT_CONTROL_RX_UCST_EN           0x10
#define REG_MII_PORT_CONTROL_RX_MCST_EN           0x08
#define REG_MII_PORT_CONTROL_RX_BCST_EN           0x04  

/* Switch mode register, Page 0x00 Address 0x0b */
#define REG_SWITCH_MODE_FRAME_MANAGE_MODE   0x01
#define REG_SWITCH_MODE_SW_FWDG_EN          0x02
#define REG_SWITCH_MODE_RETRY_LIMIT_DIS     0x04

/* MII1 Port State Override Register Page 0x00 Address 0x0e */
#define REG_CONTROL_MPSO_MII_SW_OVERRIDE    0x80
#define REG_CONTROL_MPSO_REVERSE_MII        0x10
#define REG_CONTROL_MPSO_LP_FLOW_CONTROL    0x08
#define REG_CONTROL_MPSO_TX_FLOW_CONTROL    0x20
#define REG_CONTROL_MPSO_RX_FLOW_CONTROL    0x10
#define REG_CONTROL_MPSO_SPEED100           0x04
#define REG_CONTROL_MPSO_SPEED1000          0x08
#define REG_CONTROL_MPSO_FDX                0x02
#define REG_CONTROL_MPSO_LINKPASS           0x01

/* Power down mode register Page 0x00 Address 0x0f */
#define REG_POWER_DOWN_MODE_PORT1_PHY_DISABLE     0x01
#define REG_POWER_DOWN_MODE_PORT2_PHY_DISABLE     0x02
#define REG_POWER_DOWN_MODE_PORT3_PHY_DISABLE     0x04
#define REG_POWER_DOWN_MODE_PORT4_PHY_DISABLE     0x08
#define REG_POWER_DOWN_MODE_PORT5_PHY_DISABLE     0x10

/* Switch control register page 0x0 */
#define REG_SWITCH_CONTROL                  0x22
#define REG_SWITCH_CONTROL_MII_DUMP_FWD_EN  0x40

#define PAGE_STATUS                         0x01
#define REG_STRAP_VAL                               0x70
#define REG_STRAP_P8_SEL_SGMII                      1<<9

#define PAGE_MANAGEMENT                     0x02
/* Device ID register page 0x02 */
#define REG_DEVICE_ID                       0x30
#define REG_GLOBAL_CONFIG                   0x00
#define REG_BRCM_HDR_CTRL                   0x03

/* Global Configuration Regiater Page 0x02 Address 0x00 */
#define ENABLE_MII_PORT                     0x80

/* Broadcom Header Control Register Page 0x02 Address 0x03*/
#define REG_BRCM_HDR_ENABLE                 0x01

#define PAGE_PORT_BASED_VLAN                0x31
#define REG_VLAN_CTRL_P0                    0x00


/*---------------------------------------------------------------------*/
/* 5325 Switch SPI Interface                                           */
/* use when configuration type is BP_ENET_CONFIG_SPI_SSB_x             */
/*---------------------------------------------------------------------*/
#define BCM5325_SPI_CMD_LEN                 1
#define BCM5325_SPI_ADDR_LEN                1
#define BCM5325_SPI_PREPENDCNT              (BCM5325_SPI_CMD_LEN+BCM5325_SPI_ADDR_LEN)

/* 5325 SPI Status Register */
#define BCM5325_SPI_STS                     0xfe

/* 5325 SPI Status Register definition */
#define BCM5325_SPI_CMD_SPIF                0x80
#define BCM5325_SPI_CMD_RACK                0x20

/* 5325 Command Byte definition */
#define BCM5325_SPI_CMD_READ                0x00    /* bit 0 - Read/Write */
#define BCM5325_SPI_CMD_WRITE               0x01    /* bit 0 - Read/Write */
#define BCM5325_SPI_CHIPID_MASK             0x7     /* bit 3:1 - Chip ID */
#define BCM5325_SPI_CHIPID_SHIFT            1
#define BCM5325_SPI_CMD_NORMAL              0x60    /* bit 7:4 - Mode */
#define BCM5325_SPI_CMD_FAST                0x10    /* bit 4 - Mode */

/*---------------------------------------------------------------------*/
/* 5325 Switch Pseudo PHY MII Register                                 */
/* use when configuration type is BP_ENET_CONFIG_MDIO_PSEUDO_PHY       */
/*---------------------------------------------------------------------*/
#define PSEUDO_PHY_ADDR             0x1e    /* Pseduo PHY address */

/* Pseudo PHY MII registers */
#define REG_PSEUDO_PHY_MII_REG16    0x10    /* register 16 - Switch Register Set Access Control Register */
#define REG_PSEUDO_PHY_MII_REG17    0x11    /* register 17 - Switch Register Set Read/Write Control Register */
#define REG_PSEUDO_PHY_MII_REG24    0x18    /* register 24 - Switch Accesss Register bit 15:0 */
#define REG_PSEUDO_PHY_MII_REG25    0x19    /* register 25 - Switch Accesss Register bit 31:16 */
#define REG_PSEUDO_PHY_MII_REG26    0x20    /* register 26 - Switch Accesss Register bit 47:32 */
#define REG_PSEUDO_PHY_MII_REG27    0x21    /* register 27 - Switch Accesss Register bit 63:48 */

/*Pseudo PHY MII register 16 Switch Register Set Access Control Register */
#define REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT  8       /* bit 8..15 - switch page number */
#define REG_PPM_REG16_MDIO_ENABLE               0x01    /* bit 0 - set MDC/MDIO access enable */

/*Pseudo PHY MII register 17 Switch Register Set Read/Write Control Register */
#define REG_PPM_REG17_REG_NUMBER_SHIFT          8       /* bit 8..15 - switch register number */
#define REG_PPM_REG17_OP_DONE       0x00    /* bit 0..1 - no operation */
#define REG_PPM_REG17_OP_WRITE      0x01    /* bit 0..1 - write operation */
#define REG_PPM_REG17_OP_READ       0x02    /* bit 0..1 - read operation */

/****************************************************************************
    Broadcom Extended PHY registers
****************************************************************************/
#define BRCM_MIIEXT_BANK            0x1f
    #define BRCM_MIIEXT_BANK_MASK       0xfff0
    #define BRCM_MIIEXT_ADDR_RANGE      0xffe0
    #define BRCM_MIIEXT_DEF_BANK        0x8000
#define BRCM_MIIEXT_OFFSET          0x10
    #define BRCM_MIIEXT_OFF_MASK    0xf

uint32 mii_read(uint32 uPhyAddr, uint32 uRegAddr);
void mii_write(uint32 uPhyAddr, uint32 uRegAddr, uint32 data);

#define K1CTL_REPEATER_DTE	0x400
#define K1CTL_1000BT_FDX 	0x200
#define K1CTL_1000BT_HDX 	0x100

#endif /* _MII_H_ */
