/*
   Copyright (c) 2013 Broadcom
   All Rights Reserved

    <:label-BRCM:2013:DUAL/GPL:standard
    
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
#ifndef _PMD_CAL_H_
#define _PMD_CAL_H_


#include "pmd.h"
#include "pmd_temp_cal.h"


int pmd_cal_param_set(pmd_calibration_parameters_index cal_param, int32_t val, uint16_t set_index);
int pmd_cal_param_get(pmd_calibration_parameters_index cal_param, int32_t *val, uint16_t get_index);
void pmd_cal_param_init(pmd_calibration_parameters *calibration_parameters_from_json);
int pmd_cal_is_config_valid(pmd_calibration_parameters_index *pmd_cal_param_valid, uint8_t size);


#endif /* _PMD_CAL_H_ */
