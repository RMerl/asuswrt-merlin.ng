// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2019 Broadcom Corporation
   All Rights Reserved

    
*/

#ifndef _BRCM_RGMII_H_
#define _BRCM_RGMII_H_

typedef struct
{
    int instance;
    int delay_rx, delay_tx;
    int is_1p8v;
    int is_3p3v;
    int is_disabled;
    int num_pins;
    int *pins;
} rgmii_params;

#ifndef BRCM_RGMII
static inline
#endif
int rgmii_attach(rgmii_params *params)
#ifndef BRCM_RGMII
{ return 0; }
#endif
;

#ifndef BRCM_RGMII
static inline
#endif
int rgmii_ib_status_override(int instance, int speed, int duplex)
#ifndef BRCM_RGMII
{ return 0; }
#endif
;

#endif

