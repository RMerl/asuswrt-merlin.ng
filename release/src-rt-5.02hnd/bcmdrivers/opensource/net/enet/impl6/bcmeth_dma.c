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

#include "bcm_OS_Deps.h"
#include "board.h"
#include "spidevices.h"
#include <bcm_map_part.h>
#include "bcm_intr.h"
#include "bcmmii.h"

/* Exports for other drivers */
#include "bcmsw_api.h"
#include "bcmswshared.h"
#include "bcmPktDma_defines.h"
#include "boardparms.h"
#include "bcmenet.h"
#include "bcmPktDma.h"

/* Duna has 2 Ethernet cores. Collect statistics from them only */
#define NUM_ETHCORES 2

/* Stats API */
#define MAXNUMCNTRS 62

unsigned int bcm_stat_reg_offset[MAXNUMCNTRS] = {
    0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c,
    0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c,
    0x40, 0x44, 0x48, 0x4c, 0x50, 0x54, 0x58, 0x5c,
    0x60, 0x64, 0x68, 0x6c, 0x70, 0x80, 0x84, 0x88,
    0x8c, 0x90, 0x94, 0x98, 0x9c, 0xa0, 0xa4, 0xa8,
    0xac, 0xb0, 0xb4, 0xb8, 0xbc, 0xc0, 0xc4, 0xc8,
    0xcc, 0xd0, 0xd4, 0xd8, 0xdc, 0xe0, 0xe4, 0xe8,
    0xec, 0xf0, 0x100, 0x104, 0x108, 0x10c
};

static uint64 counter_sw_val[NUM_ETHCORES][MAXNUMCNTRS];
static uint64 counter_hw_val[NUM_ETHCORES][MAXNUMCNTRS];
static uint64 counter_delta[NUM_ETHCORES][MAXNUMCNTRS];

/*
* The two functions below allow gathering SW tx/rx counters.
* Is used by wlan driver to stop calibration
* when heavy traffic runs between Eth Port, to avoid Eth pkt loss
*
* For Duna, Eth traffic will be the sum of all Eth interfaces.
*/
unsigned int eth_tx;
unsigned int eth_rx;

/*
* This function gathers the statistics every 1 second from the ethernet
* poll function. Do not create your own polling function that would call it,
* as this would interfere with power management and reduce power savings.
*/
void __ethsw_get_txrx_imp_port_pkts(void)
{
    volatile EnetCoreMib *mib0 = ENET_CORE0_MIB;
    volatile EnetCoreMib *mib1 = ENET_CORE1_MIB;

    eth_tx = mib0->gtpkt + mib1->gtpkt;
    eth_rx = mib0->grpkt + mib1->grpkt;

    return;
}

void ethsw_get_txrx_imp_port_pkts(unsigned int *tx, unsigned int *rx)
{
    *tx = eth_tx;
    *rx = eth_rx;

    return;
}
EXPORT_SYMBOL(ethsw_get_txrx_imp_port_pkts);

/*
 * Function:
 *      ethsw_counter_collect
 * Purpose:
 *      Collects and accumulates the stats
 * Parameters:
 *      discard - If true, the software counters are not updated; this
 *              results in only synchronizing the previous hardware
 *              count buffer.
 * Returns:
 *      BCM_E_XXX
 */
int ethsw_counter_collect(int discard)
{
    volatile EnetCoreMib *mib[NUM_ETHCORES] = {ENET_CORE0_MIB, ENET_CORE1_MIB};
    unsigned int ctr_new;
    unsigned int ctr_prev;
    uint64 ctr_diff;
    int i; /* Enetcore number */
    int j; /* MIB Register number */

    for (i = 0; i < NUM_ETHCORES; i++)
    {
        for (j = 0; j < MAXNUMCNTRS; j++)
        {
            ctr_prev = counter_hw_val[i][j];

            /* Read the counter value from hardware */
            //down(&bcm_ethlock_switch_config);
            ctr_new = *((char *)mib[i] + bcm_stat_reg_offset[j]);
            //up(&bcm_ethlock_switch_config);

            if (ctr_new == ctr_prev)
            {
                counter_delta[i][j] = 0;
                continue;
            }

            if (discard)
            {
                /* Update the previous value buffer */
                counter_hw_val[i][j] = ctr_new;
                counter_delta[i][j] = 0;
                continue;
            }

            ctr_diff = ctr_new;
            if (ctr_diff < ctr_prev)
            {
                /* Counter must have wrapped around.
                 * Add the proper wrap-around amount
                 */
                ctr_diff += ((uint64)1 << 32);
            }
            ctr_diff -= ctr_prev;

            counter_sw_val[i][j] += ctr_diff;
            counter_delta[i][j] = ctr_diff;
            counter_hw_val[i][j] = ctr_new;
        }
    }

    return BCM_E_NONE;
}
