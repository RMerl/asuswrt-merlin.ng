// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved
*/

/*
 *  Created on: Jun 2021
 *      Author: yuval.raviv@broadcom.com
 */

#ifndef __WOL_H__
#define __WOL_H__

typedef struct {
    int en_mpd;
    int en_ard;
    int en_lnk;
    int en_psw;
    uint8_t mac_addr[6];
    uint8_t password[6];
    int repetitions;
} wol_params_t;

void wol_enable(char *dev_name, wol_params_t *wol_params);
#endif
