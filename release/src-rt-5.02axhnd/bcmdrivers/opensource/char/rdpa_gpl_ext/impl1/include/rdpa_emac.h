
/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :> 
*/


#ifndef _RDPA_EMAC_H_
#define _RDPA_EMAC_H_

/** \defgroup emac Ethernet MAC Management
 *
 * This header file mostly deals with Ethernet MAC h/w
 * @{
 */

#define RDPA_NUM_EMACS    10 /**< Max number of EMACs */

typedef struct
{
	rdpa_emac_cfg_t emac_param; /**< EMAC internal parameters */
	rdpa_emac_mode mode; /**< EMAC mode */
	bdmf_boolean enable; /**< true=enable */

}rdpa_port_emac_cfg_t;


typedef struct
{
    rdpa_emac_mode mode;    /**< EMAC mode */
    bdmf_boolean enable;    /**< true=enable */
    rdpa_emac_cfg_t cfg;    /**< configuration */
    rdpa_emac_stat_t stat;  /**< EMAC statistics */
}rdpa_hw_emac_cfg_t;


/* Emac arr functions */
int emac_get_mode(rdpa_emac emac, rdpa_emac_mode *mode);
int emac_set_hw_cfg(rdpa_emac emac, rdpa_emac_cfg_t *cfg);
int emac_get_hw_cfg(rdpa_emac emac, rdpa_emac_cfg_t *cfg);
int emac_set_hw_enable(rdpa_emac emac, bdmf_boolean enable);
int emac_get_hw_enable(rdpa_emac emac, bdmf_boolean *enable);
int emac_get_hw_rx_stat(rdpa_emac emac, rdpa_emac_rx_stat_t *stat);
int emac_get_hw_tx_stat(rdpa_emac emac, rdpa_emac_tx_stat_t *stat);


/*
 * The following declarations are available only to
 * RDPA drivers, but not to applications.
 */

#ifdef BDMF_DRIVER
extern const bdmf_attr_enum_table_t rdpa_emac_mode_enum_table;
#endif /* #ifdef RDPA_DRIVER */

/** @} end of emac doxygen group */

#endif /* _RDPA_EMAC_H_ */
