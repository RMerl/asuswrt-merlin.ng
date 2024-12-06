/*
   Copyright (c) 2013 Broadcom
   All Rights Reserved

    <:label-BRCM:2013:DUAL/GPL:standard

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
#ifndef _PMD_CAL_H_
#define _PMD_CAL_H_


#include "pmd.h"
#include "pmd_temp_cal.h"


int pmd_cal_param_set(pmd_calibration_parameters_index cal_param, int32_t val, uint16_t set_index);
int pmd_cal_param_get(pmd_calibration_parameters_index cal_param, int32_t *val, uint16_t get_index);
void pmd_cal_param_init(pmd_calibration_parameters *calibration_parameters_from_json);
int pmd_cal_is_config_valid(pmd_calibration_parameters_index *pmd_cal_param_valid, uint8_t size);


#endif /* _PMD_CAL_H_ */
