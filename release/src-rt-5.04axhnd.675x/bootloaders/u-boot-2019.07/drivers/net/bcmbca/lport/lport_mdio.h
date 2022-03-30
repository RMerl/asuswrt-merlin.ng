// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    
*/
/*
 * lport_mdio.h
 *
 *  Created on: April 2015
 *      Author: yonatani
 */

#ifndef SHARED_OPENSOURCE_DRV_LPORT_LPORT_MDIO_H_
#define SHARED_OPENSOURCE_DRV_LPORT_LPORT_MDIO_H_

#define LPORT_MDIO_MODE_INTERRUPT 1
#define LPORT_MDIO_MODE_POLLING   2
#define LPORT_MDIO_TRANS_LOCK     1
#define LPORT_MDIO_TRANS_NOLOCK   2


#ifdef LPORT_INTERRUPTS
#define LPORT_MDIO_MODE LPORT_MDIO_MODE_INTERRUPT
#define LPORT_MDIO_TRANSACTION LPORT_MDIO_TRANS_LOCK
#else
#define LPORT_MDIO_MODE LPORT_MDIO_MODE_POLLING
#define LPORT_MDIO_TRANSACTION LPORT_MDIO_TRANS_NOLOCK
#endif

#define LPORT_MDIO_TRANS_LOCKED (LPORT_MDIO_TRANSACTION == LPORT_MDIO_TRANS_LOCK)
#define LPORT_MDIO_TRANS_INT     (LPORT_MDIO_MODE == LPORT_MDIO_MODE_INTERRUPT)

int lport_mdio_bus_init(void);
int lport_mdio22_wr(uint16_t phyid,uint16_t addr, uint16_t data);
int lport_mdio22_rd(uint16_t phyid,uint16_t addr, uint16_t *data);
int lport_mdio45_rd(uint16_t phyid, uint16_t devid, uint16_t addr, uint16_t *data);
int lport_mdio45_wr(uint16_t phyid, uint16_t devid, uint16_t addr, uint16_t data);

#endif /* SHARED_OPENSOURCE_DRV_LPORT_LPORT_MDIO_H_ */
