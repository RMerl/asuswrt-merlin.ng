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

#define _BCMENET_LOCAL_
#include "board.h"
#include <bcm_map_part.h>
#include "boardparms.h"
#include "bcmenet.h"
#include "bcm_eth.h"

/* WireSpeed feature: Uncomment to enable it
 * #define ENABLE_WIRESPEED
 */

#ifdef NO_CFE
void ethsw_reset(void)
{
    // Enable ENET1 clock
    PERF->blkEnables |= GPHY_ENET_CLK_EN;
    msleep(1);
}
#endif

static void eth_autoneg_waiting(BcmEnet_devctrl *pDevCtrl, int start_wait);
static void eth_refresh(BcmEnet_devctrl *pDevCtrl);
static void eth_get_link_speed(BcmEnet_devctrl *pDevCtrl);
static void eth_set_leds(BcmEnet_devctrl *pDevCtrl, t_eth_ctrl_led mode,
                                                   t_eth_gpio phy_gpio);
static void eth_led_multicolor_select(BcmEnet_devctrl *pDevCtrl,
                         t_eth_mcmode mode, t_eth_gpio phy_gpio);
static void eth_gigabit_enable(BcmEnet_devctrl *pDevCtrl);
//static void eth_gigabit_disable(BcmEnet_devctrl *pDevCtrl);

static int eth_core_is_mac(BcmEnet_devctrl *pDevCtrl)
{
    int ret = 0;

    if (pDevCtrl->sw_port_id == ENET0)
    {
        if (pDevCtrl->fmode == MII_MAC || pDevCtrl->fmode == RGMII_MAC)
        {
            ret = 1;
        }
    }
    else if (pDevCtrl->sw_port_id == ENET1)
    {
        if (pDevCtrl->fmode == RGMII_MAC || pDevCtrl->fmode == GMII)
        {
            ret = 1;
        }
    }

    return ret;
}

static void eth_mdio_init(void)
{
    volatile EnetCoreMdio *mdio = ENET_CORE0_MDIO;

    /* Enable preamble and set clk rate to 6.25 MHz */
    unsigned char val = MDIO_CNTRL_ENABLE_MDIO_PREAMBLE_MASK
                        | MDIO_CLK_RATE;
    mdio->mdioControl = val;
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
 * Send "write" command through MDIO interface
 *
 * PARAMETERS:
 * addr:     MDIO register to write
 * data:     Data to write to the register
 * enetcore:  Target Ethernet core
 */
static int eth_mdio_write(int addr, int data, int ethcore)
{
    int val = 0;
    int cl22_pid;
    volatile EnetCoreMdio *mdio = ENET_CORE0_MDIO;

    /* Make sure MDIO is not busy */
    eth_wait_mdio_busy();

    if (ethcore == ENET0)
    {
        cl22_pid = CL22_PID_EXT_PHY;
    }
    else if (ethcore == ENET1)
    {
        cl22_pid = CL22_PID_GPHY;
    }
    else
    {
        printk("eth_mdio_write: ERROR. Invalid ethcore number %d\n", ethcore);
        return -EINVAL;
    }

    val = (CL22_ST << MDIO_CNTRL_MDIO_START_BITS_SHIFT)
          | (CL22_OP_WRITE << MDIO_CNTRL_OPCODE_SHIFT)
          | (cl22_pid << MDIO_CNTRL_PMD_SHIFT)
          | ((unsigned short)addr << MDIO_CNTRL_REGISTER_ADDR_SHIFT)
          | (CL22_TA << MDIO_CNTRL_TURN_AROUND_SHIFT)
          | (unsigned short)data;
    mdio->mdioWrite = val;
    eth_wait_mdio_busy();

    return 0;
}

/*
 * eth_mdio_read
 *
 * DESCRIPTION:
 * Send "read" command through MDIO interface
 *
 * PARAMETERS:
 * addr:     MDIO register to read
 * ethcore:  Target Ethernet core
 *
 * RETURNS:
 * Read value if success
 * -EINVAL if invalid ethcore number
 */
static int eth_mdio_read(int addr, int ethcore)
{
    int val = 0;
    int cl22_pid;
    volatile EnetCoreMdio *mdio = ENET_CORE0_MDIO;

    /* Make sure MDIO is not busy */
    eth_wait_mdio_busy();

    if (ethcore == ENET0)
    {
        cl22_pid = CL22_PID_EXT_PHY;
    }
    else if (ethcore == ENET1)
    {
        cl22_pid = CL22_PID_GPHY;
    }
    else
    {
        printk("eth_mdio_write: ERROR. Invalid ethcore number %d\n", ethcore);
        return -EINVAL;
    }

    val = (CL22_ST << MDIO_CNTRL_MDIO_START_BITS_SHIFT)
          | (CL22_OP_READ << MDIO_CNTRL_OPCODE_SHIFT)
          | (cl22_pid << MDIO_CNTRL_PMD_SHIFT)
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
 * Starts advertising 1000BASE-T and forces a new autonegotiation.
 *
 * PARAMETERS:
 * pDevCtrl:  Private data of the target net_device
 */
static void eth_gigabit_enable(BcmEnet_devctrl *pDevCtrl)
{
    int val = 0;

    if (pDevCtrl->fmode == GMII || pDevCtrl->fmode == RGMII_MAC)
    {
        /* Start advertising 1000BASE-T */
        /* Favor clock master for better compatibility when in EEE */
        val = eth_mdio_read(MDIO_ADDRESS_1000BASET_CTRL, pDevCtrl->sw_port_id);
        val |= (MDIO_1000BASET_REPEATER_ADVERTISE_MASK |
                MDIO_1000BASET_FDX_ADVERTISE_MASK |
                MDIO_1000BASET_HDX_ADVERTISE_MASK);
        eth_mdio_write(MDIO_ADDRESS_1000BASET_CTRL, val, pDevCtrl->sw_port_id);
        /* Restart autonegotiation */
        val = eth_mdio_read(MDIO_ADDRESS_CONTROL, pDevCtrl->sw_port_id);
        val |= RESTART_AUTONEG_MASK;
        eth_mdio_write(MDIO_ADDRESS_CONTROL, val, pDevCtrl->sw_port_id);
    }
}

/*
 * eth_gigabit_disable
 * --CURRENTLY UNUSED--
 *
 * DESCRIPTION:
 * Stops advertising 1000BASE-T and forces a new autonegotiation.
 *
 * PARAMETERS:
 * pDevCtrl:  Private data of the target net_device
 */
#if 0
static void eth_gigabit_disable(BcmEnet_devctrl *pDevCtrl)
{
    int val = 0;

    if (pDevCtrl->fmode == GMII || pDevCtrl->fmode == RGMII_MAC)
    {
        /* Stop advertising 1000BASE-T */
        val = eth_mdio_read(MDIO_ADDRESS_1000BASET_CTRL, pDevCtrl->sw_port_id);
        val &= ~(MDIO_1000BASET_FDX_ADVERTISE_MASK | MDIO_1000BASET_HDX_ADVERTISE_MASK);
        eth_mdio_write(MDIO_ADDRESS_1000BASET_CTRL, val, pDevCtrl->sw_port_id);
        /* Restart autonegotiation */
        val = eth_mdio_read(MDIO_ADDRESS_CONTROL, pDevCtrl->sw_port_id);
        val |= RESTART_AUTONEG_MASK;
        eth_mdio_write(MDIO_ADDRESS_CONTROL, val, pDevCtrl->sw_port_id);
    }
}
#endif


/*
 * eth_phy_soft_reset
 *
 * DESCRIPTION:
 * Performs a soft reset on the PHY of the specified Enetcore
 *
 * PARAMETERS:
 * pDevCtrl:  Private data of the target net_device
 */
static void eth_phy_soft_reset(BcmEnet_devctrl *pDevCtrl)
{
    unsigned short val;
    int i = 0;

    if (!eth_core_is_mac(pDevCtrl))
    {
      return;
    }

    /* Reset phy */
    val = eth_mdio_read(MDIO_ADDRESS_CONTROL, pDevCtrl->sw_port_id);
    val |= MDIO_DATA_RESET;
    eth_mdio_write(MDIO_ADDRESS_CONTROL, val, pDevCtrl->sw_port_id);

    while ((val = eth_mdio_read(MDIO_ADDRESS_CONTROL, pDevCtrl->sw_port_id)) & MDIO_DATA_RESET)
    {
        i++;
        if (i > MDIO_TIMEOUT)
        {
            printk("External PHY MDIO failed");
            break;
        }
    }
}

/*
 * gphy_reset
 *
 * DESCRIPTION:
 * Resets GPHY to exit standby Power-down mode
 *
 * PARAMETERS:
 * ethcore: Ethernet core identifier.
 */
static void gphy_reset(int ethcore)
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

    eth_mdio_write(MDIO_ADDRESS_CONTROL, MDIO_DATA_RESET, ethcore);
    /* Upon exiting standby Standby Power-down mode, the EGPHY remains in an
     * internal reset state for 40 us, and then resumes normal operation */
    udelay(40);
}

static int eth_core_update_unimac(BcmEnet_devctrl *pDevCtrl)
{
    int auxval = 0;
    int xmii_mode = 0;

    /* Disable both tx and rx */
    pDevCtrl->unimac->cmd &= ~UNIMAC_CTRL_TX_ENA;
    pDevCtrl->unimac->cmd &= ~UNIMAC_CTRL_RX_ENA;

    if (pDevCtrl->sw_port_id == ENET1)
    {
        int interface_mode_changed = 0;
        //check if the xMII_Interface_mode needs to be changed
        xmii_mode = (pDevCtrl->unimac_if->control & IF_CNTRL_XMII_IF_MODE)
                      >> IF_CNTRL_XMII_IF_MODE_SHIFT;
        if ((pDevCtrl->MibInfo.ulIfSpeed == SPEED_1000MBIT) &&
            (xmii_mode != IF_CONTROL_RGMII))
        {
            xmii_mode = IF_CONTROL_RGMII;
            interface_mode_changed = 1;
        }
        else if ((pDevCtrl->MibInfo.ulIfSpeed == SPEED_100MBIT || pDevCtrl->MibInfo.ulIfSpeed == SPEED_10MBIT) && xmii_mode != IF_CONTROL_MII)
        {
            xmii_mode = IF_CONTROL_MII;
            interface_mode_changed = 1;
        }

        /* change interface mode if needed */
        if (interface_mode_changed)
        {
            //change to not configured
            msleep(5); /* need to wait for atleast 2 ms for successive writes */
            auxval = pDevCtrl->unimac_if->control;
            auxval &= ~IF_CNTRL_XMII_IF_MODE;
            auxval |= (IF_CONTROL_NOT_CONFIGURED << IF_CNTRL_XMII_IF_MODE_SHIFT);
            pDevCtrl->unimac_if->control = auxval;
            msleep(5); /* need to wait for atleast 2 ms for successive writes */
            // write new mode
            auxval = pDevCtrl->unimac_if->control;
            auxval &= ~IF_CNTRL_XMII_IF_MODE;
            auxval |= (xmii_mode << IF_CNTRL_XMII_IF_MODE_SHIFT);
            pDevCtrl->unimac_if->control = auxval;
        }
    }

    /* Put MAC in software reset */
    pDevCtrl->unimac->cmd |= UNIMAC_CTRL_SW_RESET;

    /* Wait 5 cycles ~= 2 us */
    msleep(20);

    /* Set ethernet speed */
    pDevCtrl->unimac->cmd &= ~(UNIMAC_CTRL_ETH_SPEED);
    switch (pDevCtrl->MibInfo.ulIfSpeed)
    {
        case SPEED_10MBIT:
            pDevCtrl->unimac->cmd |= (UNIMAC_CMD_10MBPS << UNIMAC_CTRL_ETH_SPEED_SHIFT);
            pDevCtrl->unimac->pauseCntrl &= ~(UNIMAC_CTRL_PAUSE_TIMER);
            pDevCtrl->unimac->pauseCntrl |= PAUSE_TIMER_10;
            pDevCtrl->unimac->pauseQuant &= ~(UNIMAC_CTRL_PAUSE_QUANT);
            pDevCtrl->unimac->pauseQuant |= TXOFF_PAUSE_QUANTA_10;
            break;
        case SPEED_100MBIT:
            pDevCtrl->unimac->cmd |= (UNIMAC_CMD_100MBPS << UNIMAC_CTRL_ETH_SPEED_SHIFT);
            pDevCtrl->unimac->pauseCntrl &= ~(UNIMAC_CTRL_PAUSE_TIMER);
            pDevCtrl->unimac->pauseCntrl |= PAUSE_TIMER_100;
            pDevCtrl->unimac->pauseQuant &= ~(UNIMAC_CTRL_PAUSE_QUANT);
            pDevCtrl->unimac->pauseQuant |= TXOFF_PAUSE_QUANTA_100;
            break;
        case SPEED_1000MBIT:
            pDevCtrl->unimac->cmd |= (UNIMAC_CMD_1GBPS << UNIMAC_CTRL_ETH_SPEED_SHIFT);
            pDevCtrl->unimac->pauseCntrl &= ~(UNIMAC_CTRL_PAUSE_TIMER);
            pDevCtrl->unimac->pauseCntrl |= PAUSE_TIMER_1000;
            pDevCtrl->unimac->pauseQuant &= ~(UNIMAC_CTRL_PAUSE_QUANT);
            pDevCtrl->unimac->pauseQuant |= TXOFF_PAUSE_QUANTA_1000;
            break;
        default:
            //Do nothing (not supported)
            break;
    }

    /* Enable half duplex */
    if (pDevCtrl->MibInfo.ulIfDuplex == BCMNET_DUPLEX_HALF)
    {
        pDevCtrl->unimac->cmd |= UNIMAC_CTRL_HD_ENA;
    }

    /* crc_fwd = 0. CRC is stripped by UNIMAC */
    pDevCtrl->unimac->cmd &= ~UNIMAC_CTRL_CRC_FWD;

    /* Enable overflow logic = 1 */
    pDevCtrl->unimac->cmd |= UNIMAC_CTRL_OVERFLOW_EN;

    /* Bring mac out of software reset */
    pDevCtrl->unimac->cmd &= ~UNIMAC_CTRL_SW_RESET;

    /* wait after getting out of reset */
    msleep(20);

    /* Enable TX and RX */
    pDevCtrl->unimac->cmd |= UNIMAC_CTRL_TX_ENA;
    pDevCtrl->unimac->cmd |= UNIMAC_CTRL_RX_ENA;

    return 1;
}

int eth_update_mac_mode(BcmEnet_devctrl *pDevCtrl)
{
    int link_change = 0;

    /* Wait until auto-negotiation is finished */
    eth_autoneg_waiting(pDevCtrl, 0);
    eth_refresh(pDevCtrl);
    if (((pDevCtrl->MibInfo.ulIfLastChange & SPEED_MASK) !=
                           pDevCtrl->MibInfo.ulIfSpeed) ||
        ((pDevCtrl->MibInfo.ulIfLastChange & DUPLEX_MASK) !=
                           pDevCtrl->MibInfo.ulIfDuplex))
    {
        /* if link speed changed or no link */
        link_change = 1;
        eth_core_update_unimac(pDevCtrl);
    }
    pDevCtrl->MibInfo.ulIfLastChange = pDevCtrl->MibInfo.ulIfSpeed
                                     | pDevCtrl->MibInfo.ulIfDuplex;

    return link_change;
}

void eth_mac_enable_txrx(BcmEnet_devctrl *pDevCtrl, int enable)
{
    if (enable)
    {
        pDevCtrl->unimac->cmd |= UNIMAC_CTRL_TX_ENA;
        pDevCtrl->unimac->cmd |= UNIMAC_CTRL_RX_ENA;
    }
    else
    {
        pDevCtrl->unimac->cmd &= ~(UNIMAC_CTRL_TX_ENA);
        pDevCtrl->unimac->cmd &= ~(UNIMAC_CTRL_RX_ENA);
    }
}

/*
 * ethcore_init
 *
 * DESCRIPTION:
 * Initializes an Ethernet core: UNIMAC and UNIMAC_IF
 *
 * PARAMETERS:
 * pDevCtrl: Pointer to a BcmEnet_devctrl structure (private data of the
 *           target net_device)
 *
 * RETURNS:
 * 0 if success
 * -EINVAL if invalid link_cfg parameter
 */
static int ethcore_init(BcmEnet_devctrl *pDevCtrl)
{
    /* Read misc straps */
    volatile StrapControl *straps = STRAP;

    if (pDevCtrl->sw_port_id == ENET0)
    {
        /* Disable TX and RX */
        pDevCtrl->unimac->cmd &= ~(UNIMAC_CTRL_TX_ENA);
        pDevCtrl->unimac->cmd &= ~(UNIMAC_CTRL_RX_ENA);

        /* Put MAC in software reset and wait for some time*/
        pDevCtrl->unimac->cmd |= UNIMAC_CTRL_SW_RESET;
        msleep(20);

        /* Set Ethernet speed */
        if (straps->strapOverrideBus & STRAP_BUS_ENET0_MODE_MASK)
        {
            /* RGMII mode */
            pDevCtrl->unimac->cmd &= ~(UNIMAC_CTRL_ETH_SPEED);
            if (pDevCtrl->link_cfg == 0 ||
                pDevCtrl->link_cfg == FORCE_LINK_1000FD)
            {
                pDevCtrl->unimac->cmd |= (UNIMAC_CMD_1GBPS << UNIMAC_CTRL_ETH_SPEED_SHIFT);
                pDevCtrl->unimac->pauseCntrl &= ~(UNIMAC_CTRL_PAUSE_TIMER);
                pDevCtrl->unimac->pauseCntrl |= PAUSE_TIMER_1000;
                pDevCtrl->unimac->pauseQuant &= ~(UNIMAC_CTRL_PAUSE_QUANT);
                pDevCtrl->unimac->pauseQuant |= TXOFF_PAUSE_QUANTA_1000;
            }
            else if (pDevCtrl->link_cfg == FORCE_LINK_100FD ||
                     pDevCtrl->link_cfg == FORCE_LINK_100HD)
            {
                pDevCtrl->unimac->cmd |= (UNIMAC_CMD_100MBPS << UNIMAC_CTRL_ETH_SPEED_SHIFT);
                pDevCtrl->unimac->pauseCntrl &= ~(UNIMAC_CTRL_PAUSE_TIMER);
                pDevCtrl->unimac->pauseCntrl |= PAUSE_TIMER_100;
                pDevCtrl->unimac->pauseQuant &= ~(UNIMAC_CTRL_PAUSE_QUANT);
                pDevCtrl->unimac->pauseQuant |= TXOFF_PAUSE_QUANTA_100;
            }
            else if (pDevCtrl->link_cfg == FORCE_LINK_10FD ||
                     pDevCtrl->link_cfg == FORCE_LINK_10HD)
            {
                pDevCtrl->unimac->cmd |= (UNIMAC_CMD_10MBPS << UNIMAC_CTRL_ETH_SPEED_SHIFT);
                pDevCtrl->unimac->pauseCntrl &= ~(UNIMAC_CTRL_PAUSE_TIMER);
                pDevCtrl->unimac->pauseCntrl |= PAUSE_TIMER_10;
                pDevCtrl->unimac->pauseQuant &= ~(UNIMAC_CTRL_PAUSE_QUANT);
                pDevCtrl->unimac->pauseQuant |= TXOFF_PAUSE_QUANTA_10;
            }
            else
            {
                printk("ethcore_init: ERROR. Invalid link configuration specified\n");
                return -EINVAL;
            }

        } else
        {
            /* MII mode */
            pDevCtrl->unimac->cmd &= ~(UNIMAC_CTRL_ETH_SPEED);
            if (pDevCtrl->link_cfg == FORCE_LINK_1000FD)
            {
                printk("ethcore_init: ERROR. ENET_CORE0 forced to 1000BASE-T "
                       "mode in boardparms but straps configure it to MII mode\n");
                return -EINVAL;
            }
            else if (pDevCtrl->link_cfg == 0 ||
                     pDevCtrl->link_cfg == FORCE_LINK_100FD ||
                     pDevCtrl->link_cfg == FORCE_LINK_100HD)
            {
                pDevCtrl->unimac->cmd |= (UNIMAC_CMD_100MBPS << UNIMAC_CTRL_ETH_SPEED_SHIFT);
                pDevCtrl->unimac->pauseCntrl &= ~(UNIMAC_CTRL_PAUSE_TIMER);
                pDevCtrl->unimac->pauseCntrl |= PAUSE_TIMER_100;
                pDevCtrl->unimac->pauseQuant &= ~(UNIMAC_CTRL_PAUSE_QUANT);
                pDevCtrl->unimac->pauseQuant |= TXOFF_PAUSE_QUANTA_100;
            }
            else if (pDevCtrl->link_cfg == FORCE_LINK_10FD ||
                     pDevCtrl->link_cfg == FORCE_LINK_10HD)
            {
                pDevCtrl->unimac->cmd |= (UNIMAC_CMD_10MBPS << UNIMAC_CTRL_ETH_SPEED_SHIFT);
                pDevCtrl->unimac->pauseCntrl &= ~(UNIMAC_CTRL_PAUSE_TIMER);
                pDevCtrl->unimac->pauseCntrl |= PAUSE_TIMER_10;
                pDevCtrl->unimac->pauseQuant &= ~(UNIMAC_CTRL_PAUSE_QUANT);
                pDevCtrl->unimac->pauseQuant |= TXOFF_PAUSE_QUANTA_10;
            }
            else
            {
                printk("ethcore_init: ERROR. Invalid link configuration specified\n");
                return -EINVAL;
            }
        }

        /* crc_fwd = 0. CRC is stripped by UNIMAC */
        pDevCtrl->unimac->cmd &= ~UNIMAC_CTRL_CRC_FWD;

        /* Enable overflow logic = 1 */
        pDevCtrl->unimac->cmd |= UNIMAC_CTRL_OVERFLOW_EN;

        /* pause_fwd = 1 */
        pDevCtrl->unimac->cmd |= UNIMAC_CTRL_PAUSE_FWD;

        /* Set frame length */
        //pDevCtrl->unimac->frmLen = 0x3FFF;

        if (straps->strapOverrideBus & STRAP_BUS_ENET0_MODE_MASK)
        {
            /* RGMII */
            /* Do not need to depend on enet0_onoff_sel (MAC/PHY)
             * since RGMII interface is symmetric. */

            /* xMII interface mode = RGMII on pad interface (0) */
            pDevCtrl->unimac_if->control &= ~(IF_CNTRL_XMII_IF_MODE);
            pDevCtrl->unimac_if->control |= (IF_CONTROL_RGMII
                                        << IF_CNTRL_XMII_IF_MODE_SHIFT);
            if (((straps->strapOverrideBus & STRAP_BUS_ENET0_ONOFF_MASK)
                  >> STRAP_BUS_ENET0_ONOFF_SHIFT) == ENET0_PHY)
            {
                /* 1. Enable rgmii tx_id. ENET_CORE0_IF_CONTROL::RGMII_tx_delay */
                pDevCtrl->unimac_if->control |= IF_CNTRL_RGMII_TX_DELAY;

                /* 2. Set rgmii rx_id. ENET_CORE0_IF_RX_RGMII_ID_KEY */
                printk("Set rxRgmiiIdKey = 0x%x\n", RGMII_RXID);
                pDevCtrl->unimac_if->rxRgmiiIdKey = RGMII_RXID;
            }
        } else
        {
            /* MII */
            if (((straps->strapOverrideBus & STRAP_BUS_ENET0_ONOFF_MASK)
                  >> STRAP_BUS_ENET0_ONOFF_SHIFT) == ENET0_PHY)
            {
                /* xMII interface mode = RvMII on pad interface (2) */
                pDevCtrl->unimac_if->control &= ~(IF_CNTRL_XMII_IF_MODE);
                pDevCtrl->unimac_if->control |= (IF_CONTROL_RVMII
                                            << IF_CNTRL_XMII_IF_MODE_SHIFT);
                /* 1. Enable rgmii tx_id. ENET_CORE0_IF_CONTROL::RGMII_tx_delay */
                pDevCtrl->unimac_if->control |= IF_CNTRL_RGMII_TX_DELAY;

                /* 2. Set rgmii rx_id. ENET_CORE0_IF_RX_RGMII_ID_KEY */
                printk("Set rxRgmiiIdKey = 0x%x\n", RGMII_RXID);
                pDevCtrl->unimac_if->rxRgmiiIdKey = RGMII_RXID;
            } else
            {
                /* Set to default MAC */
                /* xMII interface mode = MII on pad interface (1) */
                pDevCtrl->unimac_if->control &= ~(IF_CNTRL_XMII_IF_MODE);
                pDevCtrl->unimac_if->control |= (IF_CONTROL_RVMII
                                            << IF_CNTRL_XMII_IF_MODE_SHIFT);
            }
        }
        /* Bring MAC out of software reset */
        pDevCtrl->unimac->cmd &= ~UNIMAC_CTRL_SW_RESET;
        msleep(20);
    }
    else /* ENET1 */
    {
        /* Disable TX and RX */
        pDevCtrl->unimac->cmd &= ~(UNIMAC_CTRL_TX_ENA);
        pDevCtrl->unimac->cmd &= ~(UNIMAC_CTRL_RX_ENA);

        /* Proceed only if gphy is enabled */
        if (straps->strapOverrideBus & STRAP_BUS_GPHY_ONOFF_MASK)
        {
            if (pDevCtrl->link_cfg == 0 ||
                pDevCtrl->link_cfg == FORCE_LINK_1000FD)
            {
                /* xMII_Interface_mode = GMII on pad (0) */
                pDevCtrl->unimac_if->control &= ~(IF_CNTRL_XMII_IF_MODE);
                pDevCtrl->unimac_if->control |= (IF_CONTROL_RGMII
                                            << IF_CNTRL_XMII_IF_MODE_SHIFT);
            }
            else
            {
                /* xMII_Interface_mode = MII on pad interface (1) */
                pDevCtrl->unimac_if->control &= ~(IF_CNTRL_XMII_IF_MODE);
                pDevCtrl->unimac_if->control |= (IF_CONTROL_MII
                                            << IF_CNTRL_XMII_IF_MODE_SHIFT);
            }

            /* Since Eth1 is always connected to GPHY, UNIMAC is always in
             * MAC mode, so we don't need to bother about rgmii_tx_delay
             * and rx_id */

            /* Put MAC in software reset and wait for some time*/
            pDevCtrl->unimac->cmd |= UNIMAC_CTRL_SW_RESET;
            msleep(20);

            /* Set Ethernet link configuration */
            pDevCtrl->unimac->cmd &= ~(UNIMAC_CTRL_ETH_SPEED);
            if (pDevCtrl->link_cfg == 0 ||
                pDevCtrl->link_cfg == FORCE_LINK_1000FD)
            {
                pDevCtrl->unimac->cmd |= (UNIMAC_CMD_1GBPS << UNIMAC_CTRL_ETH_SPEED_SHIFT);
                pDevCtrl->unimac->pauseCntrl &= ~(UNIMAC_CTRL_PAUSE_TIMER);
                pDevCtrl->unimac->pauseCntrl |= PAUSE_TIMER_1000;
                pDevCtrl->unimac->pauseQuant &= ~(UNIMAC_CTRL_PAUSE_QUANT);
                pDevCtrl->unimac->pauseQuant |= TXOFF_PAUSE_QUANTA_1000;
            }
            else if (pDevCtrl->link_cfg == FORCE_LINK_100FD ||
                     pDevCtrl->link_cfg == FORCE_LINK_100HD)
            {
                pDevCtrl->unimac->cmd |= (UNIMAC_CMD_100MBPS << UNIMAC_CTRL_ETH_SPEED_SHIFT);
                pDevCtrl->unimac->pauseCntrl &= ~(UNIMAC_CTRL_PAUSE_TIMER);
                pDevCtrl->unimac->pauseCntrl |= PAUSE_TIMER_100;
                pDevCtrl->unimac->pauseQuant &= ~(UNIMAC_CTRL_PAUSE_QUANT);
                pDevCtrl->unimac->pauseQuant |= TXOFF_PAUSE_QUANTA_100;
            }
            else if (pDevCtrl->link_cfg == FORCE_LINK_10FD ||
                     pDevCtrl->link_cfg == FORCE_LINK_10HD)
            {
                pDevCtrl->unimac->cmd |= (UNIMAC_CMD_10MBPS << UNIMAC_CTRL_ETH_SPEED_SHIFT);
                pDevCtrl->unimac->pauseCntrl &= ~(UNIMAC_CTRL_PAUSE_TIMER);
                pDevCtrl->unimac->pauseCntrl |= PAUSE_TIMER_10;
                pDevCtrl->unimac->pauseQuant &= ~(UNIMAC_CTRL_PAUSE_QUANT);
                pDevCtrl->unimac->pauseQuant |= TXOFF_PAUSE_QUANTA_10;
            }
            else
            {
                printk("ethcore_init: ERROR. Invalid link configuration 0x%0x\n", pDevCtrl->link_cfg);
                return -EINVAL;
            }

            /* crc_fwd = 0. CRC is stripped by UNIMAC */
            pDevCtrl->unimac->cmd &= ~UNIMAC_CTRL_CRC_FWD;

            /* Enable overflow logic = 1 */
            pDevCtrl->unimac->cmd |= UNIMAC_CTRL_OVERFLOW_EN;

            /* pause_fwd = 1 */
            pDevCtrl->unimac->cmd |= UNIMAC_CTRL_PAUSE_FWD;

            /* Set frame length */
            //pDevCtrl->unimac->frmLen = 0x3FFF;

            /* Bring MAC out of software reset */
            pDevCtrl->unimac->cmd &= ~UNIMAC_CTRL_SW_RESET;
            msleep(20);
        }
        else
        {
            printk("ethcore_init: GPHY disabled in straps. "
                    "No setup done for ENET1\n");
        }
    }

    /* Set the TX buffer fill threshold to the maximum value to avoid UNIMAC
     * underruns */
    pDevCtrl->unimac_if->txBufFillThreshold = MAX_TX_BUFFER_FILL_THRESHOLD;

    return 0;
}

/*
 * eth_autoneg_waiting
 *
 * DESCRIPTION:
 * Waits until autonegotiation is started or completed
 *
 * PARAMETERS:
 * pDevCtrl:   Private data of the target net_device
 * start_wait: If 1, wait until the negotiation get started.
 *             If 0, wait until the negotiation is complete.
 */
static void eth_autoneg_waiting(BcmEnet_devctrl *pDevCtrl, int start_wait)
{
    int val;
    int i;
    int timeout;

    if (!eth_core_is_mac(pDevCtrl))
    {
        return;
    }

    /* if start_wait=TRUE, we wait for the autoneg to get started */
    if (start_wait)
    {
        /* Autoneg starting timeout is half the autoneg finishing timeout */
        timeout = (AUTONEG_TIMEOUT) >> 1;

        /* Wait until auto-negotiation is started */
        val = eth_mdio_read(MDIO_ADDRESS_CONTROL, pDevCtrl->sw_port_id);

        if (val & MDIO_DATA_AUTONEG_ENABLED) /* If auto-negotiation enabled */
        {
            i = 0;
            while (i < timeout)
            {
                val = eth_mdio_read(MDIO_ADDRESS_STATUS, pDevCtrl->sw_port_id);
                if (!(val & MDIO_DATA_AUTONEG_COMPLETE) ||
                    !(val & MDIO_DATA_AUTONEG_ABILITY))
                {
                    break;
                }
                msleep(AUTONEG_TIMEOUT_PERIOD);
                i += AUTONEG_TIMEOUT_PERIOD;
            }
        }
    }
    /* Otherwise, wait for the autoneg to get finished */
    else
    {
        timeout = AUTONEG_TIMEOUT;

        /* Wait until auto-negotiation is finished */
        val = eth_mdio_read(MDIO_ADDRESS_CONTROL, pDevCtrl->sw_port_id);
        if (val & MDIO_DATA_AUTONEG_ENABLED)
        {
            i = 0;
            while (i < timeout)
            {
                val = eth_mdio_read(MDIO_ADDRESS_STATUS, pDevCtrl->sw_port_id);
                if ((val & MDIO_DATA_AUTONEG_COMPLETE) ||
                    !(val & MDIO_DATA_AUTONEG_ABILITY))
                {
                    break;
                }
                msleep(AUTONEG_TIMEOUT_PERIOD);
                i += AUTONEG_TIMEOUT_PERIOD;
            }
            if (i >= timeout)
            {
                //printk("Enetcore %d - Auto-negotiation failed\n", pDevCtrl->sw_port_id);
            }
        }
    }
}

static void eth_refresh(BcmEnet_devctrl *pDevCtrl)
{
    int tmp;

    /* Read status word */
    int device_status = eth_mdio_read(MDIO_ADDRESS_STATUS, pDevCtrl->sw_port_id);
    //printk("Enet%d: device_status = 0x%x\n", pDevCtrl->sw_port_id, pDevCtrl->device_status);
    /* read Extended Status word, if available*/
    if (device_status & MDIO_STATUS_EXTENDED)
    {
        tmp = eth_mdio_read(MDIO_ADDRESS_EXT_STATUS, pDevCtrl->sw_port_id);
        device_status |= (tmp << ETH_EXTENDED_OFFSET);
    }
    eth_get_link_speed(pDevCtrl);
}

static void eth_get_link_speed(BcmEnet_devctrl *pDevCtrl)
{
    int phy_status_reg = 0;
    int phy_aux_status_reg = 0;
    t_AUTONEG_HCD_LinkSpeed current_link_speed = 0;

    /* Read status word */
    phy_status_reg = eth_mdio_read(MDIO_ADDRESS_STATUS, pDevCtrl->sw_port_id);

    /* get link status */
    if (((phy_status_reg & MDIO_DATA_LINK_STATUS_MASK) == 0)
            || (phy_status_reg == 0xFFFF))
    {
        pDevCtrl->MibInfo.ulIfSpeed = SPEED_DOWN;
        //printk("Enet%d: link down\n", pDevCtrl->sw_port_id);
    }
    else //link up, calculate current link speed & duplex mode
    {
        /* Read MII_AUXSTAT which has AUTONEG_HCD */
        phy_aux_status_reg = eth_mdio_read(GPHY_MII_AUXSTAT,
                                            pDevCtrl->sw_port_id);
        current_link_speed = (phy_aux_status_reg & CORE_BASE19_AUTONEG_HCD_MASK)
                                >> CORE_BASE19_AUTONEG_HCD_SHIFT;

        /* eth link speed for Unimac */
        switch (current_link_speed)
        {
            case AUTONEG_HCD_1000BaseT_FD:
            case AUTONEG_HCD_1000BaseT_HD:
                pDevCtrl->MibInfo.ulIfSpeed = SPEED_1000MBIT;
                break;
            case AUTONEG_HCD_100BaseTX_FD:
            case AUTONEG_HCD_100BaseT4:
            case AUTONEG_HCD_100BaseTX_HD:
                pDevCtrl->MibInfo.ulIfSpeed = SPEED_100MBIT;
                break;
            case AUTONEG_HCD_10BaseT_FD:
            case AUTONEG_HCD_10BaseT_HD:
                pDevCtrl->MibInfo.ulIfSpeed = SPEED_10MBIT;
                break;
            default :
                /* default for GPHY */
                pDevCtrl->MibInfo.ulIfSpeed = SPEED_1000MBIT;
                break;
        }
        /* Set Unimac Half Duplex mode*/
        switch (current_link_speed)
        {
            case AUTONEG_HCD_1000BaseT_FD:
            case AUTONEG_HCD_100BaseTX_FD:
            case AUTONEG_HCD_10BaseT_FD:
                pDevCtrl->MibInfo.ulIfDuplex = BCMNET_DUPLEX_FULL;
                break;
            case AUTONEG_HCD_1000BaseT_HD:
            case AUTONEG_HCD_100BaseT4:   /** \TODO - Confirm if it is Half Duplex */
            case AUTONEG_HCD_100BaseTX_HD:
            case AUTONEG_HCD_10BaseT_HD:
                pDevCtrl->MibInfo.ulIfDuplex = BCMNET_DUPLEX_HALF;
                break;
            default :
                pDevCtrl->MibInfo.ulIfDuplex = BCMNET_DUPLEX_FULL;
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
 * PARAMETERS:
 * pDevCtrl: Private data of the target net_device
 */
void eth_configure_phy(BcmEnet_devctrl *pDevCtrl)
{
    int val;
    int auto_pwr_down;

    /* Auto-negotiation / Speed selection */
    val = eth_mdio_read(MDIO_ADDRESS_CONTROL, pDevCtrl->sw_port_id);
    if ((pDevCtrl->link_cfg == 0) || (pDevCtrl->link_cfg == FORCE_LINK_1000FD))
    {
        /* Unspecified link configuration or 1000BASE-T:
         * Enable auto-negotiation */
        val |= ENABLE_AUTONEG_MASK;
    }
    else
    {
        /* Disable auto-negotiation and select speed */
        val &= ~ENABLE_AUTONEG_MASK;
        switch (pDevCtrl->link_cfg)
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
                printk("eth_configure_phy: Error. invalid link configuration specified\n");
                break;
        }
    }
    eth_mdio_write(MDIO_ADDRESS_CONTROL, val, pDevCtrl->sw_port_id);

    /* Auto-negotiation configuration */
    val = eth_mdio_read(MDIO_ADDRESS_AUTONEG_ADV, pDevCtrl->sw_port_id);
    if (pDevCtrl->link_cfg == 0)
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
        switch (pDevCtrl->link_cfg)
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
    eth_mdio_write(MDIO_ADDRESS_AUTONEG_ADV, val, pDevCtrl->sw_port_id);

    /* Enable Link Down Auto Powersaving Mode.
     * This register could be accessed using Shadow method on
     * Shadow 0x1C registers.
     * Read the auto power down register and set
     * Auto Power Down Enable field.
     */
    val = CORE_SHD1C_0A_SHD1C_SEL; /* Auto power down register */
    val &= ~(CORE_SHD1C_OP_MASK);   /* 0: read */
    eth_mdio_write(CORE_SHD1C_ADDRESS, val, pDevCtrl->sw_port_id);
    auto_pwr_down = eth_mdio_read(CORE_SHD1C_ADDRESS, pDevCtrl->sw_port_id);

    auto_pwr_down |= CORE_SHD1C_0A_AUTO_PWRDN_EN_MASK;
    val = (val & CORE_SHD1C_WRITE_VALUE_MASK)
          | (auto_pwr_down & CORE_SHD1C_WRITE_VALUE_MASK);
    val |= CORE_SHD1C_0A_SHD1C_SEL; /* Auto power down register */
    val |= CORE_SHD1C_OP_MASK;      /* 1: write */
    eth_mdio_write(CORE_SHD1C_ADDRESS, val, pDevCtrl->sw_port_id);

    /* Enable DLL Link Down Auto Powersaving */
    val = CORE_SHD1C_05_SHD1C_SEL;
    val &= ~(CORE_SHD1C_OP_MASK);   /* 0: read */
    eth_mdio_write(CORE_SHD1C_ADDRESS, val, pDevCtrl->sw_port_id);
    auto_pwr_down = eth_mdio_read(CORE_SHD1C_ADDRESS, pDevCtrl->sw_port_id);

    auto_pwr_down &= ~CORE_SHD1C_05_AUTO_PWRDN_DLL_DIS;
    auto_pwr_down |= CORE_SHD1C_05_CLK125_OUTPUT_EN;
    val = (val & CORE_SHD1C_WRITE_VALUE_MASK)
          | (auto_pwr_down & CORE_SHD1C_WRITE_VALUE_MASK);
    val |= CORE_SHD1C_05_SHD1C_SEL;
    val |= CORE_SHD1C_OP_MASK;      /* 1: write */
    eth_mdio_write(CORE_SHD1C_ADDRESS, val, pDevCtrl->sw_port_id);


    if ((pDevCtrl->link_cfg == 0) ||
        (pDevCtrl->link_cfg == FORCE_LINK_1000FD))
    {
        /* Start advertising 1000BASE-T */
        eth_gigabit_enable(pDevCtrl);

        /* Wait for autoneg to start after giving restart autoneg
         * as part of eth_gigabit_enable/disable
         */
        eth_autoneg_waiting(pDevCtrl, 1);
    }

#ifdef ENABLE_WIRESPEED
    /* Enables the Wirespeed feature on BCM PHYs: Fallback to Fast
     * Ethernet mode if a 2-pair cable is detected. This is disabled by
     * default on reset. */
    val = CORE_SHD18_MISC_CTRL_REG << CORE_SHD18_SEL_SHIFT;
    val |= (CORE_SHD18_MISC_CTRL_REG << CORE_SHD18_SHWD_SEL_SHIFT);
    val &= ~(CORE_SHD18_OP_MASK);   /* 0: read */
    eth_mdio_write(CORE_SHD18_ADDRESS, val, pDevCtrl->sw_port_id);
    val = eth_mdio_read(CORE_SHD18_ADDRESS, pDevCtrl->sw_port_id);

    val = (val & CORE_SHD18_WRITE_MASK)
          | (1 << CORE_SHD18_MISC_CTRL_WIRESPEED_EN_SHIFT);
    val |= (CORE_SHD18_MISC_CTRL_REG << CORE_SHD18_SEL_SHIFT);
    val |= (CORE_SHD18_MISC_CTRL_REG << CORE_SHD18_SHWD_SEL_SHIFT);
    val |= CORE_SHD18_OP_MASK;      /* 1: write */
    eth_mdio_write(CORE_SHD18_ADDRESS, val, pDevCtrl->sw_port_id);
#endif

    /* Force Auto MDI-X */
    val = CORE_SHD18_MISC_CTRL_REG << CORE_SHD18_SEL_SHIFT;
    val |= (CORE_SHD18_MISC_CTRL_REG << CORE_SHD18_SHWD_SEL_SHIFT);
    val &= ~(CORE_SHD18_OP_MASK);   /* 0: read */
    eth_mdio_write(CORE_SHD18_ADDRESS, val, pDevCtrl->sw_port_id);
    val = eth_mdio_read(CORE_SHD18_ADDRESS, pDevCtrl->sw_port_id);

    val = (val & CORE_SHD18_WRITE_MASK)
          | (1 << CORE_SHD18_MISC_CTRL_FORCE_AUTO_MDIX_SHIFT);
    val |= (CORE_SHD18_MISC_CTRL_REG << CORE_SHD18_SEL_SHIFT);
    val |= (CORE_SHD18_MISC_CTRL_REG << CORE_SHD18_SHWD_SEL_SHIFT);
    val |= CORE_SHD18_OP_MASK;      /* 1: write */
    eth_mdio_write(CORE_SHD18_ADDRESS, val, pDevCtrl->sw_port_id);
}

/*
 * eth_init
 *
 * DESCRIPTION:
 * Initializes and configures the Ethernet subsystem.
 * Calls gphy_reset, ethcore_init, eth_phy_soft_reset,
 *       eth_configure_phy
 *
 * PARAMETERS:
 * pDevCtrl: Pointer to a BcmEnet_devctrl structure (private data of the
 *           target net_device)
 *
 * RETURNS:
 * 0 if success
 * -EINVAL if invalid ethcore number
 */
int eth_init(BcmEnet_devctrl *pDevCtrl)
{
    volatile StrapControl *straps = STRAP;

    if (pDevCtrl->sw_port_id == ENET0)
    {
        pDevCtrl->unimac = ENET_CORE0_UNIMAC;
        pDevCtrl->unimac_if = ENET_CORE0_IF;
        if (straps->strapOverrideBus & STRAP_BUS_ENET0_MODE_MASK)
        {
            /* RGMII */
            if (((straps->strapOverrideBus & STRAP_BUS_ENET0_ONOFF_MASK)
                  >> STRAP_BUS_ENET0_ONOFF_SHIFT) == ENET0_PHY)
            {
                pDevCtrl->fmode = RGMII_PHY;
            }
            else if (((straps->strapOverrideBus & STRAP_BUS_ENET0_ONOFF_MASK)
                  >> STRAP_BUS_ENET0_ONOFF_SHIFT) == ENET0_MAC)
            {
                pDevCtrl->fmode = RGMII_MAC;
            }
        }
        else
        {
            /* MII */
            if (((straps->strapOverrideBus & STRAP_BUS_ENET0_ONOFF_MASK)
                  >> STRAP_BUS_ENET0_ONOFF_SHIFT) == ENET0_PHY)
            {
                pDevCtrl->fmode = MII_PHY;
            }
            else if (((straps->strapOverrideBus & STRAP_BUS_ENET0_ONOFF_MASK)
                  >> STRAP_BUS_ENET0_ONOFF_SHIFT) == ENET0_MAC)
            {
                pDevCtrl->fmode = MII_MAC;
            }
        }
    }
    else if (pDevCtrl->sw_port_id == ENET1)
    {
        pDevCtrl->unimac = ENET_CORE1_UNIMAC;
        pDevCtrl->unimac_if = ENET_CORE1_IF;

        if (straps->strapOverrideBus & STRAP_BUS_GPHY_ONOFF_MASK)
        {
            /* Set to default GMII. Query internal PHY and change this later */
            pDevCtrl->fmode = GMII;
        }
    }
    else
    {
        return -EINVAL;
    }

    /* clear link speed variables, device status and LP Pause capability */
    pDevCtrl->MibInfo.ulIfSpeed = SPEED_DOWN;

    eth_mdio_init();
    gphy_reset(pDevCtrl->sw_port_id);
    ethcore_init(pDevCtrl);

    eth_phy_soft_reset(pDevCtrl);
    eth_configure_phy(pDevCtrl);

    /*
     * Default GPHY LED configuration:
     * LED 2: When link -> ON. When activity -> blink.
     * LED 1: When activity -> blink.
     */
    eth_set_leds(pDevCtrl, ETH_LEDS_LINK_PLUS_ACTIVITY, ETH_GPIO_2);
    eth_set_leds(pDevCtrl, ETH_LEDS_ACTIVITY, ETH_GPIO_1);

    pDevCtrl->MibInfo.ulIfLastChange = 0;

    return 0;
}

/*
 * eth_set_leds
 *
 * DESCRIPTION:
 * Configures a led mode for a selectable PHY GPIO
 *
 * PARAMETERS:
 * pDevCtrl : Pointer to a BcmEnet_devctrl structure (private data of the
 *            target net_device).
 * mode     : The requested LED mode.
 * phy_gpio : PHY GPIO to output the selected LED mode
 *
 * RETURNS:
 * void
 */
static void eth_set_leds(BcmEnet_devctrl *pDevCtrl, t_eth_ctrl_led mode,
                                                    t_eth_gpio phy_gpio)
{
    unsigned short gpio_selector;
    unsigned short shd1c_reg = 0;
    unsigned short shd1c_mask = 0;
    unsigned short shd1c_shift = 0;
    unsigned short tmp;
    t_eth_led_mode led_mode = 0;

    switch (phy_gpio)
    {
    case ETH_GPIO_1:
        shd1c_reg = CORE_SHD1C_0D_SHD1C_SEL;
        shd1c_mask = CORE_SHD1C_0D_LED1_SEL_MASK;
        shd1c_shift = CORE_SHD1C_0D_LED1_SEL_SHIFT;
        break;
    case ETH_GPIO_2:
        shd1c_reg = CORE_SHD1C_0D_SHD1C_SEL;
        shd1c_mask = CORE_SHD1C_0D_LED2_SEL_MASK;
        shd1c_shift = CORE_SHD1C_0D_LED2_SEL_SHIFT;
        break;
    case ETH_GPIO_3:
        shd1c_reg = CORE_SHD1C_0E_SHD1C_SEL;
        shd1c_mask = CORE_SHD1C_0E_LED3_SEL_MASK;
        shd1c_shift = CORE_SHD1C_0E_LED3_SEL_SHIFT;
        break;
    case ETH_GPIO_4:
        shd1c_reg = CORE_SHD1C_0E_SHD1C_SEL;
        shd1c_mask = CORE_SHD1C_0E_LED4_SEL_MASK;
        shd1c_shift = CORE_SHD1C_0E_LED4_SEL_SHIFT;
        break;
    default:
        break;
    }

    switch (mode)
    {
    case ETH_LEDS_LINK:
        /* LED ON when link detected. This is implemented by setting the "link"
         * multicolor mode on the appropriate pair of phy gpio outputs */

        /* Access expansion registers */
        eth_led_multicolor_select(pDevCtrl, ETH_MC_LINK_LED, phy_gpio);
        led_mode = ETH_LED_MODE_MULTICOLOR;
        break;
    case ETH_LEDS_RECV:
        /* LED blinks when receiving */
        led_mode = ETH_LED_MODE_RCVLED;
        break;
    case ETH_LEDS_ACTIVITY:
        /* LED blinks when there's any activity */
        led_mode = ETH_LED_MODE_ACTIVITY;
        break;
    case ETH_LEDS_LINK_PLUS_ACTIVITY:
        /* LED ON when link detected and blinking on activity detection.
         * This is implemented by setting the "Encoded link activity"
         * multicolor mode on the appropriate pair of phy gpio outputs */

        /* Access expansion registers */
        eth_led_multicolor_select(pDevCtrl, ETH_MC_ENCODED_LINK_ACTIVITY,
                                                               phy_gpio);
        led_mode = ETH_LED_MODE_MULTICOLOR;
        break;
    default:
        break;
    }

    /* Get gpio selector register */
    tmp = 0;
    tmp |= ((shd1c_reg << CORE_SHD1C_SHD1C_SEL_SHIFT)
                        & CORE_SHD1C_SHD1C_SEL_MASK);
    /* (Read OP) */
    eth_mdio_write(CORE_SHD1C_ADDRESS, tmp & ~(1 << CORE_SHD1C_OP_SHIFT),
                                                   pDevCtrl->sw_port_id);
    gpio_selector = eth_mdio_read(CORE_SHD1C_ADDRESS, pDevCtrl->sw_port_id);

    /* Update gpio selector register */
    gpio_selector &= ~(shd1c_mask);
    gpio_selector |= (led_mode << shd1c_shift);

    /* Write back the updated gpio selector register */
    tmp = 0;
    tmp |= (gpio_selector & CORE_SHD1C_WRITE_VALUE_MASK);
    tmp |= ((shd1c_reg << CORE_SHD1C_SHD1C_SEL_SHIFT)
                        & CORE_SHD1C_SHD1C_SEL_MASK);
    /* (Write OP) */
    eth_mdio_write(CORE_SHD1C_ADDRESS, tmp | (1 << CORE_SHD1C_OP_SHIFT),
                                                  pDevCtrl->sw_port_id);
}

/*
 * eth_led_multicolor_select
 *
 * DESCRIPTION:
 * Accesses the appropriate expansion registers to configure multicolor
 * modes.
 *
 * PARAMETERS:
 * pDevCtrl : Pointer to a BcmEnet_devctrl structure (private data of the
 *            target net_device).
 * mode     : The requested multicolor mode.
 * phy_gpio : PHY GPIO to output the selected LED mode
 *
 * RETURNS:
 * void
 */
static void eth_led_multicolor_select(BcmEnet_devctrl *pDevCtrl,
                         t_eth_mcmode mode, t_eth_gpio phy_gpio)
{
    unsigned short mc_sel = 0;

    /* Access expansion register 04h (Multicolor LED selector) */
    eth_mdio_write(CORE_SHD17_ADDRESS,
                          CORE_EXPANSION_REGISTER_SEL | CORE_SHD17_04_SEL,
                          pDevCtrl->sw_port_id);
    if ((phy_gpio == ETH_GPIO_1) || (phy_gpio == ETH_GPIO_2))
    {
        mc_sel = CORE_SHD17_04_MULTICOLOR1_LED_SEL_MASK & mode;
    }
    else
    {
        mc_sel = CORE_SHD17_04_MULTICOLOR2_LED_SEL_MASK & mode;
    }
    mc_sel |= (CORE_SHD17_04_IN_PHASE_BITS << CORE_SHD17_04_IN_PHASE_SHIFT);
    eth_mdio_write(CORE_SHD15_ADDRESS, mc_sel, pDevCtrl->sw_port_id);
}

/*
 * eth_phy_powerdown
 *
 * DESCRIPTION:
 * Powers down a GPHY.
 *
 * PARAMETERS:
 * pDevCtrl : Pointer to a BcmEnet_devctrl structure (private data of the
 *            target net_device).
 *
 * RETURNS:
 * void
 */
void eth_phy_powerdown(BcmEnet_devctrl *pDevCtrl)
{
    int auto_pwr_down;
    int phy_status_reg;
  
    unsigned short val;
   
    /* This is called just before going to standby due to link-down, but we can
     * also reach this point with the cable connected (Prod Test). in that case, we need 
     * to avoid Auto-power-down*/
    phy_status_reg = eth_mdio_read(MDIO_ADDRESS_STATUS, pDevCtrl->sw_port_id);

    if (((phy_status_reg & MDIO_DATA_LINK_STATUS_MASK) != 0) && (phy_status_reg != 0xFFFF))
    {
        /* Disable Link Down Auto Powersaving Mode.
         * This register could be accessed using Shadow method on
         * Shadow 0x1C registers.
         * Read the auto power down register and set
         * Auto Power Down Enable field.
         */
        val = CORE_SHD1C_0A_SHD1C_SEL; /* Auto power down register */
        val &= ~(CORE_SHD1C_OP_MASK);   /* 0: read */
        eth_mdio_write(CORE_SHD1C_ADDRESS, val, pDevCtrl->sw_port_id);
        auto_pwr_down = eth_mdio_read(CORE_SHD1C_ADDRESS, pDevCtrl->sw_port_id);


        auto_pwr_down &= ~CORE_SHD1C_0A_AUTO_PWRDN_EN_MASK;
        val = (val & CORE_SHD1C_WRITE_VALUE_MASK)
             | (auto_pwr_down & CORE_SHD1C_WRITE_VALUE_MASK);

        val |= CORE_SHD1C_0A_SHD1C_SEL; /* Auto power down register */
        val |= CORE_SHD1C_OP_MASK;      /* 1: write */
        eth_mdio_write(CORE_SHD1C_ADDRESS, val, pDevCtrl->sw_port_id);


        /* Disable DLL Link Down Auto Powersaving */
        val = CORE_SHD1C_05_SHD1C_SEL;
        val &= ~(CORE_SHD1C_OP_MASK);   /* 0: read */
        eth_mdio_write(CORE_SHD1C_ADDRESS, val, pDevCtrl->sw_port_id);
        auto_pwr_down = eth_mdio_read(CORE_SHD1C_ADDRESS, pDevCtrl->sw_port_id);

        auto_pwr_down |= CORE_SHD1C_05_AUTO_PWRDN_DLL_DIS;
        auto_pwr_down &= ~CORE_SHD1C_05_CLK125_OUTPUT_EN;
        val = (val & CORE_SHD1C_WRITE_VALUE_MASK)
            | (auto_pwr_down & CORE_SHD1C_WRITE_VALUE_MASK);
        val |= CORE_SHD1C_05_SHD1C_SEL;
        val |= CORE_SHD1C_OP_MASK;      /* 1: write */
        eth_mdio_write(CORE_SHD1C_ADDRESS, val, pDevCtrl->sw_port_id);
    }
    

    // GPHY down MDIO command
    val = eth_mdio_read(MDIO_ADDRESS_CONTROL, pDevCtrl->sw_port_id);
    val |= MDIO_DATA_POWER_DOWN;
    eth_mdio_write(MDIO_ADDRESS_CONTROL, val, pDevCtrl->sw_port_id);
}
