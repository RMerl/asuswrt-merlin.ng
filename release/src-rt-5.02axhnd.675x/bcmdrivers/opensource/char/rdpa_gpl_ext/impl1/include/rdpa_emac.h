
/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
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
