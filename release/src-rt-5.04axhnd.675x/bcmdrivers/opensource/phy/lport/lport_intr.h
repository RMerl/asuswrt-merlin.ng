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
 *  lport_intr.h
 *
 *  LPORT interrupt layer
 * 
 *  Created on: Dec 2015
 *  Author: Kosta Sopov kosta@broadcom.com
 */

#ifndef SHARED_OPENSOURCE_DRV_LPORT_LPORT_INT_H_
#define SHARED_OPENSOURCE_DRV_LPORT_LPORT_INT_H_

typedef enum 
{
    LPORT_MDIO_DONE,
    LPORT_MDIO_READ_ERROR,
    LPORT_PORT_LINK,
    LPORT_REMOTE_FAULT,
    LPORT_TIMESYNC_FIFO,
    LPORT_INTR_LAST
} LPORT_INTR_ID;

typedef struct 
{
    int intr_id;

    /* Entity Id:
        PORT(0-7), SERDES(0-3), QGPHY(0-3) or XMLAC(0-1), 
        depends on intr_id that was requested
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
} lport_intr_info_s;

typedef void(*lport_intr_cb_t)(const lport_intr_info_s *info, void *priv);

int lport_intr_init(void);

/* Note: interrupt must be enabled using lport_intr_enable in order to start getting interrupts */
int lport_intr_register(LPORT_INTR_ID intr_id, uint32_t entity_id, lport_intr_cb_t cb, void *priv);

void lport_intr_unregister(LPORT_INTR_ID intr_id, uint32_t entity_id);
void lport_intr_enable(LPORT_INTR_ID intr_id, uint32_t entity_id, int enable);
void lport_intr_clear(LPORT_INTR_ID intr_id, uint32_t entity_id);

#define lport_mdio_intr_register(intr_id, cb, cb_priv) lport_intr_register(intr_id, 0, cb, cb_priv)
#define lport_mdio_intr_unregister(intr_id) lport_intr_unregister(intr_id, 0)
#define lport_mdio_intr_enable(intr_id,enable) lport_intr_enable(intr_id, 0, enable)
#define lport_mdio_intr_clear(intr_id) lport_intr_clear(intr_id, 0)

#endif
