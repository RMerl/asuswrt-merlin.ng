/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

#ifndef _BCM_ETH_H_
#define _BCM_ETH_H_

#include "bcmenet_common.h"

#define ENET0                0
#define ENET1                1

#define AON_REGISTERS_GPHY_CTRL0                        0x308
#define BSTI_START_OP 0x01
#define BSTI_READ_OP  0x02
#define BSTI_WRITE_OP 0x01

#define MDIO_STATUS_EXTENDED 0x00000100 /**< MDIO: Extended status flag. */

#define ETH_EXTENDED_OFFSET  16

/* PHY management registers addresses */
#define MDIO_ADDRESS_CONTROL            0
#define MDIO_ADDRESS_STATUS             1
#define MDIO_ADDRESS_PHY_IDENTIFIER     2
#define MDIO_ADDRESS_PHY_IDENTIFIER_3   3
#define MDIO_ADDRESS_AUTONEG_ADV        4
#define MDIO_ADDRESS_LINK_PARTNER       5
#define MDIO_ADDRESS_1000BASET_CTRL     9
/* Registers 10 to 14 are reserved */
#define MDIO_ADDRESS_EXT_STATUS         15

/* Masks for PHY management registers */
#define MDIO_DATA_RESET                     0x8000
#define MDIO_DATA_SPEEDSL_LSB               0x2000  /* 11=Rsv, 10=1000Mbps, 01=100Mbps, 00=10Mbps */
#define MDIO_DATA_SPEEDSL_MSB               0x0040  /* 11=Rsv, 10=1000Mbps, 01=100Mbps, 00=10Mbps */
#define MDIO_DATA_AUTONEG_ENABLED           0x1000
#define MDIO_DATA_POWER_DOWN                0x0800  /* 1=Low power, 0=Normal */
#define MDIO_DATA_DUPLEX                    0x0100
#define MDIO_DATA_AUTONEG_COMPLETE          0x0020
#define MDIO_DATA_AUTONEG_ABILITY           0x0008
#define MDIO_DATA_GIGA_MASK                 0xF000
#define MDIO_DATA_GIGA_NEGOTIATED           0x0010
#define MDIO_DATA_GIGA_NEGOTIATED_MASK      0x0018
#define MDIO_DATA_LOOPBACK_MASK             0x4000
#define MDIO_DATA_LINK_STATUS_MASK          0x0004
#define MDIO_LINK_PARTNER_FLOW_CONTROL_MASK 0x0400

/* Time-out for MDIO communication. */
#define MDIO_TIMEOUT                        10000
/* Time-out for autonegotiation. */
#define AUTONEG_TIMEOUT              1000  /* in ms. Multiple of 2! */
/* Autonegotiation timeout period. */
#define AUTONEG_TIMEOUT_PERIOD       100   /* in ms */

#define MDIO_AUTONEG_ADV_ASYMPAUSE_MASK     0x0800
#define MDIO_AUTONEG_ADV_PAUSABLE_MASK      0x0400
#define MDIO_AUTONEG_ADV_100BaseT4_MASK	    0x0200
#define MDIO_AUTONEG_ADV_100BaseTX_FD_MASK  0x0100
#define MDIO_AUTONEG_ADV_100BaseTX_HD_MASK  0x0080
#define MDIO_AUTONEG_ADV_10BaseT_FD_MASK    0x0040
#define MDIO_AUTONEG_ADV_10BaseT_HD_MASK    0x0020

#define MDIO_1000BASET_MANUAL_MASTER_SLAVE_MASK  0x1000
#define MDIO_1000BASET_MASTER_SLAVE_CFG_MASK     0x0800
#define MDIO_1000BASET_REPEATER_ADVERTISE_MASK   0x0400
#define MDIO_1000BASET_FDX_ADVERTISE_MASK        0x0200
#define MDIO_1000BASET_HDX_ADVERTISE_MASK        0x0100
#define RESTART_AUTONEG_MASK                     0x0200
#define ENABLE_AUTONEG_MASK                      0x1000

#define MDIO_CLK_RATE 19 /* 6.25 MHz */

/* MDIO Clause 22 defines */
#define CL22_ST                 (0x1)
#define CL22_OP_READ            (0x2)
#define CL22_OP_WRITE           (0x1)
#define CL22_TA                 (0x2)
#define CL22_PID_GPHY           (0x0)
#define CL22_PID_EXT_PHY        (1)

/* defines related to internal and external GPHYs
 * MII Aux status accesses the CORE_BASE19 which has autonegotiated speed
 */
#define GPHY_MII_AUXSTAT   0x19
#define CORE_BASE19_AUTONEG_HCD_MASK 0x0700
#define CORE_BASE19_AUTONEG_HCD_SHIFT 8

/* Defines for the Read/Write of GPHY register using
 * 0x1C Shadow Register Method.
 * These defines are general to SHD1C register access method and not
 * specific to any one Shadow Register.
 * For details Refer the Shadow Register Access Method in start of GPHY
 * Register Spec. Later part of document says bit 15 as reserved
 */
#define CORE_SHD1C_ADDRESS	0x1C
#define CORE_SHD1C_OP_MASK	0x8000
#define CORE_SHD1C_OP_ALIGN      0
#define CORE_SHD1C_OP_BITS       1
#define CORE_SHD1C_OP_SHIFT     15

#define CORE_SHD1C_SHD1C_SEL_MASK  0x7C00
#define CORE_SHD1C_SHD1C_SEL_ALIGN      0
#define CORE_SHD1C_SHD1C_SEL_BITS       5
#define CORE_SHD1C_SHD1C_SEL_SHIFT     10

#define CORE_SHD1C_WRITE_VALUE_MASK  0x03FF
#define CORE_SHD1C_WRITE_VALUE_ALIGN      0
#define CORE_SHD1C_WRITE_VALUE_BITS      10
#define CORE_SHD1C_WRITE_VALUE_SHIFT      0

/* defines related to CORE_SHD1C APD */
#define CORE_SHD1C_0A_SHD1C_SEL						0xA<<10
#define CORE_SHD1C_0A_AUTO_PWRDN_EN_MASK	0x0120
#define CORE_SHD1C_05_SHD1C_SEL						0x5<<10
#define CORE_SHD1C_05_AUTO_PWRDN_DLL_DIS	0x0002
#define CORE_SHD1C_05_CLK125_OUTPUT_EN		0x0001


/* defines related to CORE_SHD1C_0D */
#define CORE_SHD1C_0D_SHD1C_SEL             0xD
#define CORE_SHD1C_0D_LED1_SEL_MASK      0x000F
#define CORE_SHD1C_0D_LED1_SEL_ALIGN          0
#define CORE_SHD1C_0D_LED1_SEL_BITS           4
#define CORE_SHD1C_0D_LED1_SEL_SHIFT          0
#define CORE_SHD1C_0D_LED2_SEL_MASK      0x00F0
#define CORE_SHD1C_0D_LED2_SEL_ALIGN          0
#define CORE_SHD1C_0D_LED2_SEL_BITS           4
#define CORE_SHD1C_0D_LED2_SEL_SHIFT          4

/* defines related to CORE_SHD1C_0E */
#define CORE_SHD1C_0E_SHD1C_SEL             0xE
#define CORE_SHD1C_0E_LED3_SEL_MASK      0x000F
#define CORE_SHD1C_0E_LED3_SEL_ALIGN          0
#define CORE_SHD1C_0E_LED3_SEL_BITS           4
#define CORE_SHD1C_0E_LED3_SEL_SHIFT          0
#define CORE_SHD1C_0E_LED4_SEL_MASK      0x00F0
#define CORE_SHD1C_0E_LED4_SEL_ALIGN          0
#define CORE_SHD1C_0E_LED4_SEL_BITS           4
#define CORE_SHD1C_0E_LED4_SEL_SHIFT          4

#define CORE_SHD15_ADDRESS   0x15
#define CORE_SHD16_ADDRESS   0x16

/* Expansion registers (EXP_Register). */
#define CORE_SHD17_ADDRESS   0x17

#define CORE_EXPANSION_REGISTER_SEL 0x0F00

/* defines related to expansion register 04h: Multicolor LED Selector */
#define CORE_SHD17_04_SEL    0x04

#define CORE_SHD17_04_RESERVED_MASK             0xFC00
#define CORE_SHD17_04_RESERVED_ALIGN                 0
#define CORE_SHD17_04_RESERVED_BITS                  6
#define CORE_SHD17_04_RESERVED_SHIFT                10

#define CORE_SHD17_04_FLASH_NOW_MASK            0x0200
#define CORE_SHD17_04_FLASH_NOW_ALIGN                0
#define CORE_SHD17_04_FLASH_NOW_BITS                 1
#define CORE_SHD17_04_FLASH_NOW_SHIFT                9

#define CORE_SHD17_04_IN_PHASE_MASK             0x0100
#define CORE_SHD17_04_IN_PHASE_ALIGN                 0
#define CORE_SHD17_04_IN_PHASE_BITS                  1
#define CORE_SHD17_04_IN_PHASE_SHIFT                 8

#define CORE_SHD17_04_MULTICOLOR2_LED_SEL_MASK  0x00F0
#define CORE_SHD17_04_MULTICOLOR2_LED_SEL_ALIGN      0
#define CORE_SHD17_04_MULTICOLOR2_LED_SEL_BITS       4
#define CORE_SHD17_04_MULTICOLOR2_LED_SEL_SHIFT      4

#define CORE_SHD17_04_MULTICOLOR1_LED_SEL_MASK  0x000F
#define CORE_SHD17_04_MULTICOLOR1_LED_SEL_ALIGN      0
#define CORE_SHD17_04_MULTICOLOR1_LED_SEL_BITS       4
#define CORE_SHD17_04_MULTICOLOR1_LED_SEL_SHIFT      0


#define CORE_SHD18_ADDRESS         0x18

#define CORE_SHD18_OP_MASK       0x8000
#define CORE_SHD18_OP_ALIGN           0
#define CORE_SHD18_OP_BITS            1
#define CORE_SHD18_OP_SHIFT          15

#define CORE_SHD18_SEL_MASK      0x7000
#define CORE_SHD18_SEL_ALIGN          0
#define CORE_SHD18_SEL_BITS           3
#define CORE_SHD18_SEL_SHIFT         12

#define CORE_SHD18_WRITE_MASK    0xFFF8
#define CORE_SHD18_WRITE_ALIGN        0
#define CORE_SHD18_WRITE_BITS        13
#define CORE_SHD18_WRITE_SHIFT        3

#define CORE_SHD18_SHWD_SEL_MASK 0x0007
#define CORE_SHD18_SHWD_SEL_ALIGN     0
#define CORE_SHD18_SHWD_SEL_BITS      3
#define CORE_SHD18_SHWD_SEL_SHIFT     0

/* defines related to CORE_SHD18 registers*/
#define CORE_SHD18_CTRL_REG              0
#define CORE_SHD18_BASE_T_REG            1
#define CORE_SHD18_POWER_MII_CONTROL_REG 2
#define CORE_SHD18_MISC_TEST_REG         4
#define CORE_SHD18_MISC_CTRL_REG         7

#define CORE_SHD18_MISC_CTRL_FORCE_AUTO_MDIX_MASK  0x0200
#define CORE_SHD18_MISC_CTRL_FORCE_AUTO_MDIX_ALIGN      0
#define CORE_SHD18_MISC_CTRL_FORCE_AUTO_MDIX_BITS       1
#define CORE_SHD18_MISC_CTRL_FORCE_AUTO_MDIX_SHIFT      9

#define CORE_SHD18_MISC_CTRL_WIRESPEED_EN_MASK     0x0010
#define CORE_SHD18_MISC_CTRL_WIRESPEED_EN_ALIGN         0
#define CORE_SHD18_MISC_CTRL_WIRESPEED_EN_BITS          1
#define CORE_SHD18_MISC_CTRL_WIRESPEED_EN_SHIFT         4

#define PAUSE_LEN_USEC           (600)  /* 600 us */

/* Time in 512 bit times between two XOFF packets */
#define PAUSE_TIMER_10       ((PAUSE_LEN_USEC * 10) / 512)
#define PAUSE_TIMER_100      ((PAUSE_LEN_USEC * 100) / 512)
#define PAUSE_TIMER_1000     ((PAUSE_LEN_USEC * 1000) / 512)

/* Pause quanta to put inside pause packets */
#define BRIDGEFWD_PAUSE_CONTENTS_25MHZ  (16384)  /* 655.36 us (0x500 quanta at 1Gbit) */
/* Pause quanta in 512 bit times for 10, 100 and 1000Mbps */
#define TXOFF_PAUSE_QUANTA_10    ((10 * BRIDGEFWD_PAUSE_CONTENTS_25MHZ / 25) / 512)
#define TXOFF_PAUSE_QUANTA_100   ((100 * BRIDGEFWD_PAUSE_CONTENTS_25MHZ / 25) / 512)
#define TXOFF_PAUSE_QUANTA_1000  ((1000 * BRIDGEFWD_PAUSE_CONTENTS_25MHZ / 25) / 512)

/* Strap value for Enet0 mode */
#define ENET0_MAC   2
#define ENET0_PHY   1

/* SPEED_* and BCMNET_DUPLEX_* defines are in bcmnet.h */
#define SPEED_MASK  0xFFFFFFFE
#define DUPLEX_MASK 0x1

/* Set delay corresponding to selecting code 6 for all 4 cktaps. This results in
 * a 1.76ns delay. TODO: This number is not clear */
#define RGMII_RXID 0x0000

/* Maximum buffer fill threshold: We need the max value to avoid UNIMAC TX
 * underrun errors when the Bridge is overloading the system bus */
#define MAX_TX_BUFFER_FILL_THRESHOLD 0xEF

#define GPHY_MII_XCTL	0x10
#define CORE_BASE10_FORCE_LEDS_ON_MASK	0x0010
#define CORE_BASE10_FORCE_LEDS_ON_ALIGN	0
#define CORE_BASE10_FORCE_LEDS_ON_BITS	1
#define CORE_BASE10_FORCE_LEDS_ON_SHIFT	4

#define CORE_BASE10_FORCE_LEDS_OFF_MASK		0x0008
#define CORE_BASE10_FORCE_LEDS_OFF_ALIGN	0
#define CORE_BASE10_FORCE_LEDS_OFF_BITS		1
#define CORE_BASE10_FORCE_LEDS_OFF_SHIFT	3

/* Auto negotiated highest common denominator for Internal GPHY */
typedef enum
{
  AUTONEG_HCD_1000BaseT_FD = 7,
  AUTONEG_HCD_1000BaseT_HD = 6,
  AUTONEG_HCD_100BaseTX_FD = 5,
  AUTONEG_HCD_100BaseT4    = 4,
  AUTONEG_HCD_100BaseTX_HD = 3,
  AUTONEG_HCD_10BaseT_FD   = 2,
  AUTONEG_HCD_10BaseT_HD   = 1,
  AUTONEG_HCD_INVALID      = 0
} t_AUTONEG_HCD_LinkSpeed;

/* Unimac Interface xMIIInterfaceMode */
typedef enum
{
  IF_CONTROL_RGMII          = 0,
  IF_CONTROL_MII            = 1,
  IF_CONTROL_RVMII          = 2,
  IF_CONTROL_NOT_CONFIGURED = 3
} eEnetIFControl_ethernet_speed;

/* Unimac CMD eth speed. 00: 10Mbps, 01: 100Mbps, 10: 1000Mbps, 11: 2500Mbps */
typedef enum
{
  UNIMAC_CMD_10MBPS  = 0,
  UNIMAC_CMD_100MBPS = 1,
  UNIMAC_CMD_1GBPS   = 2,
  UNIMAC_CMD_2_5GBPS = 3
} eUnimac_cmd_ethernet_speed;

typedef enum
{
  TEN_MBPS     = 10000000,
  HUNDRED_MBPS = 100000000,
  ONE_GBPS     = 1000000000
} eEthernetSpeed;

/* Possible configurations of a MII device. */
typedef enum
{
  INACTIVE,    /* Interface is not active */
  MII_MAC,     /* Interface is an MII MAC */
  MII_PHY,     /* Interface is an MII PHY */
  RGMII_MAC,   /* Interface is RGMII MAC */
  RGMII_PHY,   /* Interface is RGMII PHY */
  GMII         /* Interface is GMII (MAC always on GPHY) */
} t_mii_devconf;

typedef enum
{
  LP_PAUSE_DISABLED = 0,
  LP_PAUSE_ENABLED = 1,
  LP_PAUSE_UNKNOWN = 0xFF,
} t_lp_adver_pause;

typedef enum
{
  ETH_LEDS_LINK = 1,
  ETH_LEDS_RECV = 2,
  ETH_LEDS_ACTIVITY = 3,
  ETH_LEDS_INTERRUPT = 4,
  ETH_LEDS_LINK_PLUS_ACTIVITY = 5,
} t_eth_ctrl_led;

typedef enum
{
  ETH_LED_MODE_LINKSPD0 = 0,
  ETH_LED_MODE_LINKSPD1,
  ETH_LED_MODE_XMTLED,
  ETH_LED_MODE_ACTIVITY,
  ETH_LED_MODE_FDXLED,
  ETH_LED_MODE_SLAVE,
  ETH_LED_MODE_INTERRUPT,
  ETH_LED_MODE_QUALITY,
  ETH_LED_MODE_RCVLED,
  ETH_LED_MODE_WIRESPEED,
  ETH_LED_MODE_MULTICOLOR,
  ETH_LED_MODE_WIRE_DIAGNOSTIC,
  ETH_LED_MODE_ENERGY_LINK_CISCO,
  ETH_LED_MODE_SGMMI_RX_CRS,
  ETH_LED_MODE_OFF,
  ETH_LED_MODE_ON
} t_eth_led_mode;

typedef enum
{
  ETH_MC_ENCODED_LINK_ACTIVITY = 0,
  ETH_MC_ENCODED_SPEED,
  ETH_MC_ACTIVITY_FLASH_LED,
  ETH_MC_FULL_DUPLEX_LED,
  ETH_MC_FORCE_OFF,
  ETH_MC_FORCE_ON,
  ETH_MC_LED_TOGGLING_TO_50_AUTO,
  ETH_MC_FLASHING_AT_80MS,
  ETH_MC_LINK_LED,
  ETH_MC_ACTIVITY_LED,
  ETH_MC_PROGRAMMABLE_BLINK_LED
} t_eth_mcmode;

typedef enum
{
  ETH_GPIO_1 = 1,
  ETH_GPIO_2 = 2,
  ETH_GPIO_3 = 3,
  ETH_GPIO_4 = 4
} t_eth_gpio;


int eth_init(BcmEnet_devctrl *pDevCtrl);
int eth_update_mac_mode(BcmEnet_devctrl *pDevCtrl);
void eth_mac_enable_txrx(BcmEnet_devctrl *pDevCtrl, int enable);
void eth_phy_powerdown(BcmEnet_devctrl *pDevCtrl);

#endif /* _BCM_ETH_H_ */
