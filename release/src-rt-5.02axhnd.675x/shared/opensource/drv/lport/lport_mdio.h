/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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
/*
 * lport_mdio.h
 *
 *  Created on: 24 בספט׳ 2015
 *      Author: yonatani
 */

#ifndef SHARED_OPENSOURCE_DRV_LPORT_LPORT_MDIO_H_
#define SHARED_OPENSOURCE_DRV_LPORT_LPORT_MDIO_H_

#define LPORT_MDIO_MODE_INTERRUPT 1
#define LPORT_MDIO_MODE_POLLING   2
#define LPORT_MDIO_TRANS_LOCK     1
#define LPORT_MDIO_TRANS_NOLOCK   2


#ifdef _CFE_
#define LPORT_MDIO_MODE LPORT_MDIO_MODE_POLLING
#define LPORT_MDIO_TRANSACTION LPORT_MDIO_TRANS_NOLOCK
#else
#define LPORT_MDIO_MODE LPORT_MDIO_MODE_INTERRUPT
#define LPORT_MDIO_TRANSACTION LPORT_MDIO_TRANS_LOCK
#endif

#define LPORT_MDIO_TRANS_LOCKED (LPORT_MDIO_TRANSACTION == LPORT_MDIO_TRANS_LOCK)
#define LPORT_MDIO_TRANS_INT     (LPORT_MDIO_MODE == LPORT_MDIO_MODE_INTERRUPT)

int lport_mdio_bus_init(void);
int lport_mdio22_wr(uint16_t phyid,uint16_t addr, uint16_t data);
int lport_mdio22_rd(uint16_t phyid,uint16_t addr, uint16_t *data);
int lport_mdio45_rd(uint16_t phyid, uint16_t devid, uint16_t addr,
                                uint16_t *data);
int lport_mdio45_wr(uint16_t phyid, uint16_t devid, uint16_t addr,
                                uint16_t data);
#endif /* SHARED_OPENSOURCE_DRV_LPORT_LPORT_MDIO_H_ */
