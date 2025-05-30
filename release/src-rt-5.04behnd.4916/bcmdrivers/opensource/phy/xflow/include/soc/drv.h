/*
 *  Copyright: (c) 2020 Broadcom.
 *  All rights reserved.
 *
 * This file contains structure and routine declarations for the
 * Switch-on-a-Chip Driver.
 *
 * This file also includes the more common include files so the
 * individual driver files don't have to include as much.
 *
 * <:label-BRCM:2022:DUAL/GPL:standard
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
 *
 */

#ifndef _SOC_DRV_H
#define _SOC_DRV_H

#include "soc/mem.h"
#include "soc/memory.h"
#include "soc/mcm/enum_types.h"


/*
 * Typedef: soc_driver_t
 * Purpose: Chip driver.  All info about a particular device type.
 * Notes: These structures are automatically generated by mcm.
 *        Used in soc/mcm/bcm*.c files.
 */
typedef struct soc_driver_s {
    soc_mem_info_t          **mem_info;          /* memory array */
} soc_driver_t;

/*
 * Typedef: soc_control_t
 * Purpose: SOC Control Structure.  All info about a device instance.
 */

typedef struct soc_control_s {
    soc_driver_t *chip_driver;
} soc_control_t;

extern int soc_attach(int unit);
extern soc_control_t *soc_control[SOC_MAX_NUM_DEVICES];
extern int soc_macsec_hw_init(int unit, int macsec_ing_byp, int macsec_egr_byp);

#endif  /* !_SOC_DRV_H */
