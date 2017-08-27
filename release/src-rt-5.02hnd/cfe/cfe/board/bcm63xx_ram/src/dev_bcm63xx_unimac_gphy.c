/*
 * <:copyright-BRCM:2014:DUAL/GPL:standard
 * 
 *    Copyright (c) 2014 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */


#include "bcm_map.h"
#include "lib_types.h"
#include "lib_malloc.h"
#include "lib_string.h"
#include "lib_printf.h"
#include "mii_shared.h"
#include "sbmips.h"
#include "cfe_iocb.h"
#include "cfe_timer.h"
#include "robosw_reg.h"
#include "dev_bcm63xx_eth.h"
#include "bcmSpiRes.h"
#include "shared_utils.h"

#define AON_REGISTERS_GPHY_CTRL0                        0x308
#define BSTI_START_OP 0x01
#define BSTI_READ_OP  0x02
#define BSTI_WRITE_OP 0x01

#define MDIO_STATUS_EXTENDED            0x00000100 /**< MDIO: Extended status flag. */

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
#define ETH_EXTENDED_OFFSET  16

/* Masks for PHY management registers */
#define MDIO_DATA_RESET                     0x8000
#define MDIO_DATA_SPEEDSL_LSB               0x2000  /* 11=Rsv, 10=1000Mbps, 01=100Mbps, 00=10Mbps */
#define MDIO_DATA_SPEEDSL_MSB               0x0040  /* 11=Rsv, 10=1000Mbps, 01=100Mbps, 00=10Mbps */
#define MDIO_DATA_AUTONEG_ENABLED           0x1000
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
#define MDIO_TIMEOUT                        1000
/* Time-out for autonegotiation. */
#define AUTONEG_TIMEOUT              100  /* in ms. Multiple of 2! */
/* Autonegotiation timeout period. */
#define AUTONEG_TIMEOUT_PERIOD       50   /* in ms */

#define MDIO_AUTONEG_ADV_ASYMPAUSE_MASK     0x0800
#define MDIO_AUTONEG_ADV_PAUSABLE_MASK      0x0400
#define MDIO_AUTONEG_ADV_100BaseT4_MASK		  0x0200
#define MDIO_AUTONEG_ADV_100BaseTX_FD_MASK  0x0100
#define MDIO_AUTONEG_ADV_100BaseTX_HD_MASK  0x0080
#define MDIO_AUTONEG_ADV_10BaseT_FD_MASK		0x0040
#define MDIO_AUTONEG_ADV_10BaseT_HD_MASK		0x0020

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

/* defines related to Internal GPHY
 * MII Aux status accesses the CORE_BASE19 which has autonegotiated speed
 */
#define GPHY_MII_AUXSTAT   0x19
#define CORE_BASE19_AUTONEG_HCD_MASK 0x0700
#define CORE_BASE19_AUTONEG_HCD_SHIFT 8
#define CORE_BASE19_AUTONEG_HCD_BITS  3
#define CORE_BASE19_AUTONEG_HCD_ALIGN 0

/* Defines for the Read/Write of GPHY register using
 * 0x1C Shadow Register Method.
 * These defines are general to SHD1C register access method and not
 * specific to any one Shadow Register.
 * For details Refer the Shadow Register Access Method in start of GPHY
 * Register Spec. Later part of document says bit 15 as reserved
 */
#define CORE_SHD1C_ADDRESS	0x1C
#define CORE_SHD1C_OP_MASK	0x8000
#define CORE_SHD1C_SHD1C_SEL_MASK		0x7C00
#define CORE_SHD1C_WRITE_VALUE_MASK		0x03FF

/* defines related to CORE_SHD1C_0A */
#define CORE_SHD1C_0A_SHD1C_SEL						0xA
#define CORE_SHD1C_0A_AUTO_PWRDN_EN_MASK	0x0020
#define CORE_SHD1C_05_SHD1C_SEL                 0x5<<10
#define CORE_SHD1C_05_AUTO_PWRDN_DLL_DIS	0x0002
#define CORE_SHD1C_05_CLK125_OUTPUT_EN		0x0001


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
  LINK_DOWN,
  LINK_AT_10,
  LINK_AT_100,
  LINK_AT_1000
} t_link_speed;

/********************************/
/* Variables local to this file */
/********************************/

volatile EnetCoreUnimac *unimac;    /* UNIMAC register base address */
volatile EnetCoreIf *unimac_if;     /* UNIMAC_IF register base address */
/* These variables save the link status */
int link_speed;
int device_status;
int lp_adver_pause;
int half_duplex;
int last_link_speed;
int last_phy_speed;
int last_half_duplex;
int fmode;
int enable_link_check;

/***********************/
/* Function prototypes */
/***********************/

static void eth_autoneg_waiting(int start_wait);
static void eth_refresh(void);
static void eth_get_link_speed(void);
static int eth_core_is_mac(void);
static void eth_gigabit_enable(void);
static void eth_mdio_init(void);
static void eth_mdio_write(int addr, int data);
static void eth_wait_mdio_busy(void);
static int eth_mdio_read(int addr);
static void gphy_reset(void);
static void eth_core_update_unimac(void);
static void eth_configure_phy(int link_cfg);
void robosw_check_ports(void);
void robosw_configure_ports(void);

static int eth_core_is_mac(void)
{
    if (fmode == RGMII_MAC || fmode == GMII)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*
 * eth_mdio_init
 *
 * DESCRIPTION:
 * Initializes MDIO
 */
static void eth_mdio_init(void)
{
    volatile EnetCoreMdio *mdio = ENET_CORE0_MDIO;

    /* Enable preamble and set clk rate to 6.25 MHz */
    mdio->mdioControl = MDIO_CNTRL_ENABLE_MDIO_PREAMBLE_MASK | MDIO_CLK_RATE;
}

/*
 * eth_wait_mdio_busy
 *
 * DESCRIPTION:
 * Waits in a loop while MDIO is busy
 */
static void eth_wait_mdio_busy(void)
{
    volatile EnetCoreMdio *mdio = ENET_CORE0_MDIO;

    while (mdio->mdioStatus & MDIO_CNTRL_BUSY) ;
}

/*
 * eth_mdio_write
 *
 * DESCRIPTION:
 * Send "write" command to internal GPHY through MDIO interface
 *
 * PARAMETERS:
 * addr:     MDIO register to write
 * data:     Data to write to the register
 */
static void eth_mdio_write(int addr, int data)
{
    int val = 0;
    volatile EnetCoreMdio *mdio = ENET_CORE0_MDIO;

    /* Make sure MDIO is not busy */
    eth_wait_mdio_busy();

    val = (CL22_ST << MDIO_CNTRL_MDIO_START_BITS_SHIFT)
          | (CL22_OP_WRITE << MDIO_CNTRL_OPCODE_SHIFT)
          | (CL22_PID_GPHY << MDIO_CNTRL_PMD_SHIFT)
          | ((unsigned short)addr << MDIO_CNTRL_REGISTER_ADDR_SHIFT)
          | (CL22_TA << MDIO_CNTRL_TURN_AROUND_SHIFT)
          | (unsigned short)data;
    mdio->mdioWrite = val;
    eth_wait_mdio_busy();
}

/*
 * eth_mdio_read
 *
 * DESCRIPTION:
 * Send "read" command to internal GPHY through MDIO interface
 *
 * PARAMETERS:
 * addr: MDIO register to read
 *
 * RETURNS:
 * Read value
 */
static int eth_mdio_read(int addr)
{
    int val = 0;
    volatile EnetCoreMdio *mdio = ENET_CORE0_MDIO;

    /* Make sure MDIO is not busy */
    eth_wait_mdio_busy();

    val = (CL22_ST << MDIO_CNTRL_MDIO_START_BITS_SHIFT)
          | (CL22_OP_READ << MDIO_CNTRL_OPCODE_SHIFT)
          | (CL22_PID_GPHY << MDIO_CNTRL_PMD_SHIFT)
          | ((unsigned short)addr << MDIO_CNTRL_REGISTER_ADDR_SHIFT)
          | (CL22_TA << MDIO_CNTRL_TURN_AROUND_SHIFT);
    mdio->mdioWrite = val;
    eth_wait_mdio_busy();

    return (mdio->mdioRead & MDIO_CNTRL_READ);
}

/*
 * eth_gigabit_enable
 *
 * DESCRIPTION:
 * Starts advertising 1000BASE-T in internal GPHY
 * and forces a new autonegotiation.
 *
 */
static void eth_gigabit_enable(void)
{
    int val = 0;

    if (fmode == GMII || fmode == RGMII_MAC)
    {
        /* Start advertising 1000BASE-T */
        val = eth_mdio_read(MDIO_ADDRESS_1000BASET_CTRL);
        val |= (MDIO_1000BASET_REPEATER_ADVERTISE_MASK |
                MDIO_1000BASET_FDX_ADVERTISE_MASK |
                MDIO_1000BASET_HDX_ADVERTISE_MASK);
        eth_mdio_write(MDIO_ADDRESS_1000BASET_CTRL, val);
        /* Restart autonegotiation */
        val = eth_mdio_read(MDIO_ADDRESS_CONTROL);
        val |= RESTART_AUTONEG_MASK;
        eth_mdio_write(MDIO_ADDRESS_CONTROL, val);
    }
}

/*
 * eth_phy_soft_reset
 *
 * DESCRIPTION:
 * Performs a soft reset on the internal GPHY
 *
 */
static void eth_phy_soft_reset(void)
{
    unsigned short val;
    int i = 0;

    if (!eth_core_is_mac())
    {
      return;
    }

    /* Reset phy */
    val = eth_mdio_read(MDIO_ADDRESS_CONTROL);
    val |= MDIO_DATA_RESET;
    eth_mdio_write(MDIO_ADDRESS_CONTROL, val);

    while ((val = eth_mdio_read(MDIO_ADDRESS_CONTROL)) & MDIO_DATA_RESET)
    {
        i++;
        if (i > MDIO_TIMEOUT)
        {
            break;
        }
    }
}

/*
 * gphy_reset
 *
 * DESCRIPTION:
 * Resets internal GPHY to exit standby Power-down mode
 *
 */
static void gphy_reset(void)
{
    volatile BSTIControl *bsti = BSTI;
    int bstictrl = 0;
    int finish = 0;

    /* Set GPHY to Normal power mode (Data field of BSTI frame = 0) */
    bstictrl |= ((AON_REGISTERS_GPHY_CTRL0 << BSTI_SER_CTRL_ADDR_SHIFT)
                    & BSTI_SER_CTRL_ADDR_MASK);
    bstictrl |= ((BSTI_WRITE_OP << BSTI_SER_CTRL_CMD_SHIFT)
                    & BSTI_SER_CTRL_CMD_MASK);
    bstictrl |= ((BSTI_START_OP << BSTI_SER_CTRL_START_SHIFT)
                    & BSTI_SER_CTRL_START_MASK);
    bsti->ser_ctrl = bstictrl;

    do
    {
        finish = (bsti->ser_ctrl & BSTI_SER_CTRL_START_MASK)
                    >> BSTI_SER_CTRL_START_SHIFT;
    } while (BSTI_START_OP == finish);

    eth_mdio_write(MDIO_ADDRESS_CONTROL, MDIO_DATA_RESET);
    /* Upon exiting standby Standby Power-down mode, the EGPHY remains in an
     * internal reset state for 40 us, and then resumes normal operation */
    cfe_usleep(40);
}

/*
 * eth_core_update_unimac
 *
 * DESCRIPTION:
 * Updates UNIMAC configuration according to the latest read status from the
 * GPHY
 *
 */
static void eth_core_update_unimac(void)
{
    int auxval = 0;
    int xmii_mode = 0;
    int interface_mode_changed = 0;

    /* Disable both tx and rx */
    unimac->cmd &= ~UNIMAC_CTRL_TX_ENA;
    unimac->cmd &= ~UNIMAC_CTRL_RX_ENA;

    //check if the xMII_Interface_mode needs to be changed
    xmii_mode = (unimac_if->control & IF_CNTRL_XMII_IF_MODE)
                  >> IF_CNTRL_XMII_IF_MODE_SHIFT;
    if ((link_speed == LINK_AT_1000) &&
        (xmii_mode != IF_CONTROL_RGMII))
    {
        xmii_mode = IF_CONTROL_RGMII;
        interface_mode_changed = 1;
    }
    else if ((link_speed == LINK_AT_100 || link_speed == LINK_AT_10) && xmii_mode != IF_CONTROL_MII)
    {
        xmii_mode = IF_CONTROL_MII;
        interface_mode_changed = 1;
    }

    /* change interface mode if needed */
    if (interface_mode_changed)
    {
        //change to not configured
        cfe_usleep(5000); /* need to wait for atleast 2 ms for successive writes */
        auxval = unimac_if->control;
        auxval &= ~IF_CNTRL_XMII_IF_MODE;
        auxval |= (IF_CONTROL_NOT_CONFIGURED << IF_CNTRL_XMII_IF_MODE_SHIFT);
        unimac_if->control = auxval;
        cfe_usleep(5000); /* need to wait for atleast 2 ms for successive writes */
        // write new mode
        auxval = unimac_if->control;
        auxval &= ~IF_CNTRL_XMII_IF_MODE;
        auxval |= (xmii_mode << IF_CNTRL_XMII_IF_MODE_SHIFT);
        unimac_if->control = auxval;
    }

    /* Put MAC in software reset */
    unimac->cmd |= UNIMAC_CTRL_SW_RESET;

    /* Wait 5 cycles ~= 2 us */
    cfe_usleep(5);

    /* Set ethernet speed */
    unimac->cmd &= ~(UNIMAC_CTRL_ETH_SPEED);
    switch (link_speed)
    {
        case LINK_AT_10:
            unimac->cmd |= (UNIMAC_CMD_10MBPS << UNIMAC_CTRL_ETH_SPEED_SHIFT);
            unimac->pauseCntrl &= ~(UNIMAC_CTRL_PAUSE_TIMER);
            unimac->pauseCntrl |= PAUSE_TIMER_10;
            unimac->pauseQuant &= ~(UNIMAC_CTRL_PAUSE_QUANT);
            unimac->pauseQuant |= TXOFF_PAUSE_QUANTA_10;
            break;
        case LINK_AT_100:
            unimac->cmd |= (UNIMAC_CMD_100MBPS << UNIMAC_CTRL_ETH_SPEED_SHIFT);
            unimac->pauseCntrl &= ~(UNIMAC_CTRL_PAUSE_TIMER);
            unimac->pauseCntrl |= PAUSE_TIMER_100;
            unimac->pauseQuant &= ~(UNIMAC_CTRL_PAUSE_QUANT);
            unimac->pauseQuant |= TXOFF_PAUSE_QUANTA_100;
            break;
        case LINK_AT_1000:
            unimac->cmd |= (UNIMAC_CMD_1GBPS << UNIMAC_CTRL_ETH_SPEED_SHIFT);
            unimac->pauseCntrl &= ~(UNIMAC_CTRL_PAUSE_TIMER);
            unimac->pauseCntrl |= PAUSE_TIMER_1000;
            unimac->pauseQuant &= ~(UNIMAC_CTRL_PAUSE_QUANT);
            unimac->pauseQuant |= TXOFF_PAUSE_QUANTA_1000;
            break;
        default:
            //Do nothing (not supported)
            break;
    }

    /* Enable half duplex */
    if (half_duplex)
    {
        unimac->cmd |= UNIMAC_CTRL_HD_ENA;
    }

    /* crc_fwd = 0. CRC is stripped by UNIMAC */
    unimac->cmd &= ~UNIMAC_CTRL_CRC_FWD;

    /* Enable overflow logic = 1 */
    unimac->cmd |= UNIMAC_CTRL_OVERFLOW_EN;

    /* Bring mac out of software reset */
    unimac->cmd &= ~UNIMAC_CTRL_SW_RESET;

    /* wait after getting out of reset */
    cfe_usleep(5);

    /* Enable TX and RX */
    unimac->cmd |= UNIMAC_CTRL_TX_ENA;
    unimac->cmd |= UNIMAC_CTRL_RX_ENA;
}

/*
 * robosw_check_ports
 *
 * DESCRIPTION:
 * Reads GPHY configuration, refresh status variables and calls
 * eth_core_update_unimac to update de UNIMAC accordingly.
 * This function is called periodically to check for link changes
 *
 */
void robosw_check_ports(void)
{
    if (enable_link_check)
    {
        /* Wait until auto-negotiation is finished */
        eth_autoneg_waiting(0);
        eth_refresh();
        if ((last_link_speed != link_speed) || (last_half_duplex != half_duplex))
        {
            /* if link speed changed or no link */
            eth_core_update_unimac();
        }
        last_link_speed = link_speed;
        last_half_duplex = half_duplex;
    }
}

/*
 * ethcore_init
 *
 * DESCRIPTION:
 * Initializes Ethernet core 1: UNIMAC and UNIMAC_IF
 *
 * PARAMETERS:
 * link_cfg: Ethernet link configuration (speed & duplex)
 *           (FORCE_LINK_* from boardparms.h).
 *
 * RETURNS:
 * 0 if success
 * -1 if invalid link_cfg parameter
 */
static int ethcore_init(int link_cfg)
{
    /* Read misc straps */
    volatile StrapControl *straps = STRAP;

    /* Disable TX and RX */
    unimac->cmd &= ~(UNIMAC_CTRL_TX_ENA);
    unimac->cmd &= ~(UNIMAC_CTRL_RX_ENA);

    /* Proceed only if gphy is enabled */
    if (straps->strapOverrideBus & STRAP_BUS_GPHY_ONOFF_MASK)
    {
        if (link_cfg == 0 ||
            link_cfg == FORCE_LINK_1000FD)
        {
            /* xMII_Interface_mode = GMII on pad (0) */
            unimac_if->control &= ~(IF_CNTRL_XMII_IF_MODE);
            unimac_if->control |= (IF_CONTROL_RGMII
                                        << IF_CNTRL_XMII_IF_MODE_SHIFT);
        }
        else
        {
            /* xMII_Interface_mode = MII on pad interface (1) */
            unimac_if->control &= ~(IF_CNTRL_XMII_IF_MODE);
            unimac_if->control |= (IF_CONTROL_MII
                                        << IF_CNTRL_XMII_IF_MODE_SHIFT);
        }

        /* Since Eth1 is always connected to GPHY, UNIMAC is always in
         * MAC mode, so we don't need to bother about rgmii_tx_delay
         * and rx_id */

        /* Put MAC in software reset and wait for some time*/
        unimac->cmd |= UNIMAC_CTRL_SW_RESET;
        cfe_usleep(5);

        /* Set Ethernet link configuration */
        unimac->cmd &= ~(UNIMAC_CTRL_ETH_SPEED);
        if (link_cfg == 0 ||
            link_cfg == FORCE_LINK_1000FD)
        {
            unimac->cmd |= (UNIMAC_CMD_1GBPS << UNIMAC_CTRL_ETH_SPEED_SHIFT);
            unimac->pauseCntrl &= ~(UNIMAC_CTRL_PAUSE_TIMER);
            unimac->pauseCntrl |= PAUSE_TIMER_1000;
            unimac->pauseQuant &= ~(UNIMAC_CTRL_PAUSE_QUANT);
            unimac->pauseQuant |= TXOFF_PAUSE_QUANTA_1000;
        }
        else if (link_cfg == FORCE_LINK_100FD ||
                 link_cfg == FORCE_LINK_100HD)
        {
            unimac->cmd |= (UNIMAC_CMD_100MBPS << UNIMAC_CTRL_ETH_SPEED_SHIFT);
            unimac->pauseCntrl &= ~(UNIMAC_CTRL_PAUSE_TIMER);
            unimac->pauseCntrl |= PAUSE_TIMER_100;
            unimac->pauseQuant &= ~(UNIMAC_CTRL_PAUSE_QUANT);
            unimac->pauseQuant |= TXOFF_PAUSE_QUANTA_100;
        }
        else if (link_cfg == FORCE_LINK_10FD ||
                 link_cfg == FORCE_LINK_10HD)
        {
            unimac->cmd |= (UNIMAC_CMD_10MBPS << UNIMAC_CTRL_ETH_SPEED_SHIFT);
            unimac->pauseCntrl &= ~(UNIMAC_CTRL_PAUSE_TIMER);
            unimac->pauseCntrl |= PAUSE_TIMER_10;
            unimac->pauseQuant &= ~(UNIMAC_CTRL_PAUSE_QUANT);
            unimac->pauseQuant |= TXOFF_PAUSE_QUANTA_10;
        }
        else
        {
            printf("%s: Error. Invalid link_cfg 0x%0x\n", __FUNCTION__, link_cfg);
            return -1;
        }

        /* crc_fwd = 0. CRC is stripped by UNIMAC */
        unimac->cmd &= ~UNIMAC_CTRL_CRC_FWD;

        /* Enable overflow logic = 1 */
        unimac->cmd |= UNIMAC_CTRL_OVERFLOW_EN;

        /* pause_fwd = 0 */
        unimac->cmd |= UNIMAC_CTRL_PAUSE_FWD;

        /* Set frame length */
        //unimac->frmLen = 0x3FFF;

        /* Bring MAC out of software reset */
        unimac->cmd &= ~UNIMAC_CTRL_SW_RESET;
        cfe_usleep(5);

        /* Enable TX and RX */
        unimac->cmd |= UNIMAC_CTRL_TX_ENA;
        unimac->cmd |= UNIMAC_CTRL_RX_ENA;
    }
    else
    {
        printf("%s: GPHY disabled in straps. "
                "No setup done for ENET1\n", __FUNCTION__);
    }

    return 0;
}

/*
 * eth_autoneg_waiting
 *
 * DESCRIPTION:
 * Waits until autonegotiation is started or completed
 *
 * PARAMETERS:
 * start_wait: If 1, wait until the negotiation get started.
 *             If 0, wait until the negotiation is complete.
 */
static void eth_autoneg_waiting(int start_wait)
{
    int val;
    int i;
    int timeout;

    if (!eth_core_is_mac())
    {
        return;
    }

    /* if start_wait=TRUE, we wait for the autoneg to get started */
    if (start_wait)
    {
        /* Autoneg starting timeout is half the autoneg finishing timeout */
        timeout = (AUTONEG_TIMEOUT) >> 1;

        /* Wait until auto-negotiation is started */
        val = eth_mdio_read(MDIO_ADDRESS_CONTROL);

        if (val & MDIO_DATA_AUTONEG_ENABLED) /* If auto-negotiation enabled */
        {
            i = 0;
            while (i < timeout)
            {
                val = eth_mdio_read(MDIO_ADDRESS_STATUS);
                if (!(val & MDIO_DATA_AUTONEG_COMPLETE) ||
                    !(val & MDIO_DATA_AUTONEG_ABILITY))
                {
                    break;
                }
                cfe_usleep(AUTONEG_TIMEOUT_PERIOD * 1000);
                i += AUTONEG_TIMEOUT_PERIOD;
            }
        }
    }
    /* Otherwise, wait for the autoneg to get finished */
    else
    {
        timeout = AUTONEG_TIMEOUT;

        /* Wait until auto-negotiation is finished */
        val = eth_mdio_read(MDIO_ADDRESS_CONTROL);
        if (val & MDIO_DATA_AUTONEG_ENABLED)
        {
            i = 0;
            while (i < timeout)
            {
                val = eth_mdio_read(MDIO_ADDRESS_STATUS);
                if ((val & MDIO_DATA_AUTONEG_COMPLETE) ||
                    !(val & MDIO_DATA_AUTONEG_ABILITY))
                {
                    break;
                }
                cfe_usleep(AUTONEG_TIMEOUT_PERIOD * 1000);
                i += AUTONEG_TIMEOUT_PERIOD;
            }
            if (i >= timeout)
            {
                //printf("%s: Auto-negotiation failed\n", __FUNCTION__);
            }
        }
    }
}

/*
 * eth_refresh
 *
 * DESCRIPTION:
 * Reads and saves GPHY status
 *
 */
static void eth_refresh(void)
{
    int tmp;

    /* Read status word */
    device_status = eth_mdio_read(MDIO_ADDRESS_STATUS);
    /* read Extended Status word, if available*/
    if (device_status & MDIO_STATUS_EXTENDED)
    {
        tmp = eth_mdio_read(MDIO_ADDRESS_EXT_STATUS);
        device_status |= (tmp << ETH_EXTENDED_OFFSET);
    }
    eth_get_link_speed();
}

/*
 * eth_get_link_speed
 *
 * DESCRIPTION:
 * Reads and saves GPHY link speed and duplex status
 *
 */
static void eth_get_link_speed(void)
{
    int phy_status_reg = 0;
    int phy_aux_status_reg = 0;
    t_AUTONEG_HCD_LinkSpeed current_link_speed = 0;

    /* Read status word */
    phy_status_reg = eth_mdio_read(MDIO_ADDRESS_STATUS);

    /* get link status */
    if (((phy_status_reg & MDIO_DATA_LINK_STATUS_MASK) == 0)
            || (phy_status_reg == 0xFFFF))
    {
        link_speed = LINK_DOWN;
    }
    else //link up, calculate current link speed & duplex mode
    {
        /* Read MII_AUXSTAT which has AUTONEG_HCD */
        phy_aux_status_reg = eth_mdio_read(GPHY_MII_AUXSTAT);
        current_link_speed = (phy_aux_status_reg &
                              CORE_BASE19_AUTONEG_HCD_MASK)
                              >> CORE_BASE19_AUTONEG_HCD_SHIFT;

        /* eth link speed for Unimac */
        switch (current_link_speed)
        {
            case AUTONEG_HCD_1000BaseT_FD:
            case AUTONEG_HCD_1000BaseT_HD:
                link_speed = LINK_AT_1000;
                break;
            case AUTONEG_HCD_100BaseTX_FD:
            case AUTONEG_HCD_100BaseT4:
            case AUTONEG_HCD_100BaseTX_HD:
                link_speed = LINK_AT_100;
                break;
            case AUTONEG_HCD_10BaseT_FD:
            case AUTONEG_HCD_10BaseT_HD:
                link_speed = LINK_AT_10;
                break;
            default :
                /* default for GPHY */
                link_speed = LINK_AT_1000;
                break;
        }
        /* Set Unimac Half Duplex mode*/
        switch (current_link_speed)
        {
            case AUTONEG_HCD_1000BaseT_FD:
            case AUTONEG_HCD_100BaseTX_FD:
            case AUTONEG_HCD_10BaseT_FD:
                half_duplex = 0;
                break;
            case AUTONEG_HCD_1000BaseT_HD:
            case AUTONEG_HCD_100BaseT4:   /** \TODO - Confirm if it is Half Duplex */
            case AUTONEG_HCD_100BaseTX_HD:
            case AUTONEG_HCD_10BaseT_HD:
                half_duplex = 1;
                break;
            default :
                half_duplex = 0;
                break;
        }
    }
}

/*
 * eth_configure_phy
 *
 * DESCRIPTION:
 * Configures PHY control registers
 *
 */
static void eth_configure_phy(int link_cfg)
{
    int val;
    int auto_pwr_down;

    /* Auto-negotiation / Speed selection */
    val = eth_mdio_read(MDIO_ADDRESS_CONTROL);
    if ((link_cfg == 0) || (link_cfg == FORCE_LINK_1000FD))
    {
        /* Unspecified link configuration or 1000BASE-T:
         * Enable auto-negotiation */
        val |= ENABLE_AUTONEG_MASK;
    }
    else
    {
        /* Disable auto-negotiation and select speed */
        val &= ~ENABLE_AUTONEG_MASK;
        switch (link_cfg)
        {
            case FORCE_LINK_100HD:
                val &= ~MDIO_DATA_SPEEDSL_MSB;
                val |= MDIO_DATA_SPEEDSL_LSB;
                val &= ~MDIO_DATA_DUPLEX;
                break;
            case FORCE_LINK_100FD:
                val &= ~MDIO_DATA_SPEEDSL_MSB;
                val |= MDIO_DATA_SPEEDSL_LSB;
                val |= MDIO_DATA_DUPLEX;
                break;
            case FORCE_LINK_10HD:
                val &= ~MDIO_DATA_SPEEDSL_MSB;
                val &= ~MDIO_DATA_SPEEDSL_LSB;
                val &= ~MDIO_DATA_DUPLEX;
                break;
            case FORCE_LINK_10FD:
                val &= ~MDIO_DATA_SPEEDSL_MSB;
                val &= ~MDIO_DATA_SPEEDSL_LSB;
                val |= MDIO_DATA_DUPLEX;
                break;
            default:
                printf("%s: Error invalid link configuration specified\n", __FUNCTION__);
                break;
        }
    }
    eth_mdio_write(MDIO_ADDRESS_CONTROL, val);

    /* Auto-negotiation configuration */
    val = eth_mdio_read(MDIO_ADDRESS_AUTONEG_ADV);
    if (link_cfg == 0)
    {
        /* 1. Enable advertisement of asymmetric pause.
         * 2. By default only full duplex modes are being advertised.
         *    Enable Half duplex as well full duplex modes for all speeds.
         *    For 1 GBPS only full duplex is to be supported.
         */
        val |= (MDIO_AUTONEG_ADV_100BaseT4_MASK
                | MDIO_AUTONEG_ADV_100BaseTX_FD_MASK
                | MDIO_AUTONEG_ADV_100BaseTX_HD_MASK
                | MDIO_AUTONEG_ADV_10BaseT_FD_MASK
                | MDIO_AUTONEG_ADV_10BaseT_HD_MASK);
    }
    else
    {
        switch (link_cfg)
        {
            case FORCE_LINK_100HD:
                val |= MDIO_AUTONEG_ADV_100BaseTX_HD_MASK;
                break;
            case FORCE_LINK_100FD:
                val |= MDIO_AUTONEG_ADV_100BaseTX_FD_MASK;
                break;
            case FORCE_LINK_10HD:
                val |= MDIO_AUTONEG_ADV_10BaseT_HD_MASK;
                break;
            case FORCE_LINK_10FD:
                val |= MDIO_AUTONEG_ADV_10BaseT_FD_MASK;
                break;
        }
    }
    eth_mdio_write(MDIO_ADDRESS_AUTONEG_ADV, val);

    /* Enable Link Down Auto Powersaving Mode.
     * This register could be accessed using Shadow method on
     * Shadow 0x1C registers.
     * Read the auto power down register and set
     * Auto Power Down Enable field.
     */
    val = CORE_SHD1C_0A_SHD1C_SEL; /* Auto power down register */
    val &= ~(CORE_SHD1C_OP_MASK);   /* 0: read */
    eth_mdio_write(CORE_SHD1C_ADDRESS, val);
    auto_pwr_down = eth_mdio_read(CORE_SHD1C_ADDRESS);

    auto_pwr_down |= CORE_SHD1C_0A_AUTO_PWRDN_EN_MASK;
    val = (val & CORE_SHD1C_WRITE_VALUE_MASK)
          | (auto_pwr_down & CORE_SHD1C_WRITE_VALUE_MASK);
    val |= CORE_SHD1C_0A_SHD1C_SEL; /* Auto power down register */
    val |= CORE_SHD1C_OP_MASK;      /* 1: write */
    eth_mdio_write(CORE_SHD1C_ADDRESS, val);

    /* Enable DLL Link Down Auto Powersaving */
    val = CORE_SHD1C_05_SHD1C_SEL;
    val &= ~(CORE_SHD1C_OP_MASK);   /* 0: read */
    eth_mdio_write(CORE_SHD1C_ADDRESS, val);
    auto_pwr_down = eth_mdio_read(CORE_SHD1C_ADDRESS);

    auto_pwr_down &= ~CORE_SHD1C_05_AUTO_PWRDN_DLL_DIS;
    auto_pwr_down |= CORE_SHD1C_05_CLK125_OUTPUT_EN;
    val = (val & CORE_SHD1C_WRITE_VALUE_MASK)
          | (auto_pwr_down & CORE_SHD1C_WRITE_VALUE_MASK);
    val |= CORE_SHD1C_05_SHD1C_SEL;
    val |= CORE_SHD1C_OP_MASK;      /* 1: write */
    eth_mdio_write(CORE_SHD1C_ADDRESS, val);

    if ((link_cfg == 0) || (link_cfg == FORCE_LINK_1000FD))
    {
        /* Start advertising 1000BASE-T */
        eth_gigabit_enable();

        /* Wait for autoneg to start after giving restart autoneg
         * as part of eth_gigabit_enable/disable
         */
        eth_autoneg_waiting(1);
    }

    /* Force Auto MDI-X */
    val = CORE_SHD18_MISC_CTRL_REG << CORE_SHD18_SEL_SHIFT;
    val |= (CORE_SHD18_MISC_CTRL_REG << CORE_SHD18_SHWD_SEL_SHIFT);
    val &= ~(CORE_SHD18_OP_MASK);   /* 0: read */
    eth_mdio_write(CORE_SHD18_ADDRESS, val);
    val = eth_mdio_read(CORE_SHD18_ADDRESS);

    val = (val & CORE_SHD18_WRITE_MASK)
          | (1 << CORE_SHD18_MISC_CTRL_FORCE_AUTO_MDIX_SHIFT);
    val |= (CORE_SHD18_MISC_CTRL_REG << CORE_SHD18_SEL_SHIFT);
    val |= (CORE_SHD18_MISC_CTRL_REG << CORE_SHD18_SHWD_SEL_SHIFT);
    val |= CORE_SHD18_OP_MASK;      /* 1: write */
    eth_mdio_write(CORE_SHD18_ADDRESS, val);

#ifdef ENABLE_WIRESPEED
    /* Enables the Wirespeed feature on BCM PHYs: Fallback to Fast
     * Ethernet mode if a 2-pair cable is detected. This is disabled by
     * default on reset. */
    val = CORE_SHD18_MISC_CTRL_REG << CORE_SHD18_SEL_SHIFT;
    val |= (CORE_SHD18_MISC_CTRL_REG << CORE_SHD18_SHWD_SEL_SHIFT);
    val &= ~(CORE_SHD18_OP_MASK);   /* 0: read */
    eth_mdio_write(CORE_SHD18_ADDRESS, val);
    val = eth_mdio_read(CORE_SHD18_ADDRESS);

    val = (val & CORE_SHD18_WRITE_MASK)
          | (1 << CORE_SHD18_MISC_CTRL_WIRESPEED_EN_SHIFT);
    val |= (CORE_SHD18_MISC_CTRL_REG << CORE_SHD18_SEL_SHIFT);
    val |= (CORE_SHD18_MISC_CTRL_REG << CORE_SHD18_SHWD_SEL_SHIFT);
    val |= CORE_SHD18_OP_MASK;      /* 1: write */
    eth_mdio_write(CORE_SHD18_ADDRESS, val);
#endif
}

/*
 * robosw_configure_ports
 *
 * DESCRIPTION:
 * Initializes and configures the Ethernet subsystem.
 * Calls gphy_reset, ethcore_init, eth_phy_soft_reset,
 *       eth_configure_phy
 * Note that this function is not related to roboswitch. The name is used for
 * code compatibility with dev_bcm63xx_eth.c
 *
 */
void robosw_configure_ports(void)
{
    volatile StrapControl *straps = STRAP;

    unimac = ENET_CORE1_UNIMAC;
    unimac_if = ENET_CORE1_IF;
    ETHERNET_MAC_INFO EnetInfo[1];
    int i;
    int link_cfg = 0;

    /* Read forced mode parameter from boardparms if available.
     * link_cfg = 0 ==> No specific mode forced, use autonegotiation */
    BpGetEthernetMacInfo(EnetInfo, 1);
    for (i = 0; i < BP_MAX_SWITCH_PORTS; i++)
    {
        if ((EnetInfo->sw.phy_id[i] & 0xFF) == 1)
        {
            link_cfg = EnetInfo->sw.phy_id[i] &
                       (PHY_LNK_CFG_M << PHY_LNK_CFG_S);
            break;
        }
    }
    if (i == BP_MAX_SWITCH_PORTS)
    {
        printf("%s: No interface definition found for ENET_CORE1 "
               "(bp_ulPhyId = 1) in board parameters\n", __FUNCTION__);
        return;
    }
    if (link_cfg == 0)
    {
        enable_link_check = 1;
    }

    if (straps->strapOverrideBus & STRAP_BUS_GPHY_ONOFF_MASK)
    {
        /* Set to default GMII. Query internal PHY and change this later */
        fmode = GMII;
    }

    /* clear link speed variables, device status and LP Pause capability */
    link_speed = LINK_DOWN;
    device_status = 0x00000000;
    lp_adver_pause = LP_PAUSE_UNKNOWN;

    eth_mdio_init();
    gphy_reset();
    ethcore_init(link_cfg);

    eth_phy_soft_reset();
    eth_configure_phy(link_cfg);

    last_link_speed = LINK_DOWN;
    last_phy_speed = 0;
    last_half_duplex = 0;
}
