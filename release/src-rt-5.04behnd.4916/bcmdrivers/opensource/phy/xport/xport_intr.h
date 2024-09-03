/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard
    
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
 *  xport_intr.h
 *
 *  XPORT interrupt layer
 * 
 *  Created on: Dec 2022
 *  Author: Marc Jalfon mj889415
 */

#ifndef SHARED_OPENSOURCE_DRV_XPORT_XPORT_INT_H_
#define SHARED_OPENSOURCE_DRV_XPORT_XPORT_INT_H_

typedef enum 
{
    XPORT_TIMESYNC_FIFO,
    XPORT_INTR_LAST
} XPORT_INTR_ID;

typedef struct 
{
    int intr_id;

    /* Entity Id:
        PORT(0-7), SERDES(0-3), QGPHY(0-3) or XMLAC(0-1), 
        depends on intr_id that was requested
        68880 PTP uses only XPORT_TIMESYNC_FIFO which has only one entity, 4 bits in each of 3 regs
    */
    uint32_t entity_id; 

    /* Status: 
        LPORT_PORT_LINK: 0 - link off, 1 - link on
        LPORT_SERDER_SIGNAL: 0 - signal off, 1 - signal on
        LPORT_QGPHY_ENERGY: 0 - no energy detected, 1 - energy detected
        LPORT_XLMAC: event as described in XLMAC_INTR_STATUS in XLMAC spec
        LPORT_UBUS_ERROR, LPORT_MDIO_DONE, LPORT_MDIO_READ_ERROR: ignore, always 0
    */
    int status;
} xport_intr_info_s;

typedef void(*xport_intr_cb_t)(const xport_intr_info_s *info, void *priv);

int xport_intr_init(int irq0, int irq1, int irq2);

/* Note: interrupt must be enabled using xport_intr_enable in order to start getting interrupts */
int xport_intr_register(XPORT_INTR_ID intr_id, uint32_t entity_id, xport_intr_cb_t cb, void *priv);
void xport_intr_unregister(XPORT_INTR_ID intr_id, uint32_t entity_id, uint32_t register_num);
void xport_intr_enable(XPORT_INTR_ID intr_id, uint32_t entity_id, int enable, uint32_t register_num);
void xport_intr_clear(XPORT_INTR_ID intr_id, uint32_t entity_id, uint32_t register_num);

#define lport_mdio_intr_register(intr_id, cb, cb_priv) lport_intr_register(intr_id, 0, cb, cb_priv)
#define lport_mdio_intr_unregister(intr_id) lport_intr_unregister(intr_id, 0)
#define lport_mdio_intr_enable(intr_id,enable) lport_intr_enable(intr_id, 0, enable)
#define lport_mdio_intr_clear(intr_id) lport_intr_clear(intr_id, 0)

#endif
