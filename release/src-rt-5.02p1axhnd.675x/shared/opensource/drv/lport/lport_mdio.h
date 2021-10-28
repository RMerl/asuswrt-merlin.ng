/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
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
