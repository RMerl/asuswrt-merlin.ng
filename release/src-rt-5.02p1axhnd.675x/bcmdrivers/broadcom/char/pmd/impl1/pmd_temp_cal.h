/*
   Copyright (c) 2013 Broadcom Corporation
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
