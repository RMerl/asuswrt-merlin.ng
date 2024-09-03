/*
   Copyright (c) 2013 Broadcom Corporation
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
#ifndef _PMD_TEMP_CAL_H_
#define _PMD_TEMP_CAL_H_


/* table lowest temperature absolute value */
#define TEMP_TABLE_LOWEST_TEMP 40
#define APD_TEMP_TABLE_LOWEST_TEMP -40

#define TEMP_MAX_TABLE_LEN 200
#define TEMP_TABLE_ADDR 0x7000

#define APD_TEMP_TABLE_ADDR 0x7500

extern const uint32_t default_pmd_res_temp_conv[TEMP_TABLE_SIZE];
extern const uint16_t default_pmd_temp_apd_conv[APD_TEMP_TABLE_SIZE];

#endif /* _PMD_TEMP_CAL_H_ */
