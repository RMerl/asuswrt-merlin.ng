/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
   All Rights Reserved

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

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/bcm_log.h>
#include "trx_descr.h"
#include "trx_descr_gen.h"
#include "trx_descr_usr.h"
#include "trxbus.h"

static TRX_DESCRIPTOR default_pluggable_trx =
{
    .form_factor           = TRX_SFP,
    .type                  = TRX_TYPE_UNKNOWN,
    .vendor_name           = "Default",
    .vendor_pn             = "Default",
    .lbe_polarity          = TRX_ACTIVE_LOW,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false
}, default_on_board_trx =
{
    .form_factor           = TRX_SFF,
    .type                  = TRX_TYPE_XPON,
    .vendor_name           = "Default",
    .vendor_pn             = "Default",
    .lbe_polarity          = TRX_ACTIVE_HIGH,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false
}, default_pmd_trx =
{
    .form_factor           = TRX_PMD,
    .type                  = TRX_TYPE_XPON,
    .vendor_name           = "Broadcom",
    .vendor_pn             = "689xx",
    .lbe_polarity          = TRX_ACTIVE_HIGH,
    .tx_sd_polarity        = TRX_ACTIVE_HIGH,
    .tx_pwr_down_polarity  = TRX_ACTIVE_LOW,
    .tx_pwr_down_cfg_req   = false,
    .wan_types_bitmap      = SUPPORTED_WAN_TYPES_BIT_GPON | SUPPORTED_WAN_TYPES_BIT_EPON_1_1 | SUPPORTED_WAN_TYPES_BIT_TURBO_EPON_2_1 \
        | SUPPORTED_WAN_TYPES_BIT_EPON_10_1 | SUPPORTED_WAN_TYPES_BIT_XGPON
};

static char *trx_module_ff_table[] =
{
   "UNKNOWN",   /* 0x0 */
   "GBIC",      /* 0x1 */
   "SFF",       /* 0x2 - SFF */
   "SFP/SFP+",  /* 0x3 - SFP or SFP+ */
   "XBI",       /* 0x4 */
   "XENPAK",    /* 0x5 */
   "XFP",       /* 0x6 */
   "XFF",       /* 0x7 */
   "XFP-E",     /* 0x8 */
   "XPAK",      /* 0x9 */
   "X2"         /* 0xA */
};

static char *trx_module_type_table[] =
{
   "xPON",
   "ETHERNET",
   "UNKNOWN",
};

static void trx_print_descriptor(int trx_ff, char *v_name, int v_name_len, char *v_pn, int v_pn_len, char *v_rev,
    int v_rev_len, int match, int type)
{
    char *str_ff; 

    if (trx_ff == TRX_PMD)
        str_ff = "PMD";
    else
        str_ff = trx_module_ff_table[trx_ff];

    printk("Opticaldet %s Transceiver\n", match ? "Known" : "Unknown");
    printk("Module Form Factor: %s\n", str_ff);
    printk("Module Type       : %s\n", trx_module_type_table[type]);
    printk("Vendor Name       : %.*s\n", v_name_len, v_name);
    printk("Part Number       : %.*s\n", v_pn_len, v_pn);
    printk("Part REV          : %.*s\n", v_rev_len, v_rev);

    if (!match)
    {
        printk(KERN_ALERT "************************************************************************\n");
        printk(KERN_ALERT "* Opticaldet: Unknown optical module - using default configuration     *\n");
        printk(KERN_ALERT "* Please make sure the optical module is correct for your connection   *\n");
        printk(KERN_ALERT "************************************************************************\n");
    }
}

static TRX_DESCRIPTOR *match_descriptor(TRX_DESCRIPTOR trx_db[], int size, char *vname, char *vpn)
{
    int i;

    for (i = 0; i < size; i++)
    {
        if ((!strncmp(vname, trx_db[i].vendor_name, strlen(trx_db[i].vendor_name))) &&
            (!strncmp(vpn, trx_db[i].vendor_pn, strlen(trx_db[i].vendor_pn))))
        {
            return &trx_db[i];
        }
    }

    return NULL;
}

static void *detect(int bus)
{
    TRX_DESCRIPTOR *trx = NULL;
    unsigned long trx_ff;
    int ret, match = 1, v_name_len, v_pn_len, v_rev_len;
    char *v_name, *v_pn, *v_rev;
    int is_pmd = trxbus_is_pmd(bus);
    
    if (is_pmd)
    {
        printk("Opticaldet: PMD found\n");
        trx = &default_pmd_trx;
        trx_print_descriptor(TRX_PMD, trx->vendor_name, sizeof(trx->vendor_name),
            trx->vendor_pn, sizeof(trx->vendor_pn), NULL, 0, 1, trx->type);

        return trx;
    }

    ret = trxbus_mon_read(bus, bcmsfp_mon_id_phys_id, 0, &trx_ff);
    switch (trx_ff)
    {
        case TRX_SFF:
        case TRX_SFP:
        case TRX_XFP:
            trxbus_mon_read_buf(bus, bcmsfp_mon_id_vendor_name, 0, &v_name, &v_name_len);
            trxbus_mon_read_buf(bus, bcmsfp_mon_id_vendor_pn, 0, &v_pn, &v_pn_len);
            trxbus_mon_read_buf(bus, bcmsfp_mon_id_vendor_rev, 0, &v_rev, &v_rev_len);

            trx = match_descriptor(trx_usr, sizeof(trx_usr) / sizeof(TRX_DESCRIPTOR), v_name, v_pn);
            if (!trx)
                trx = match_descriptor(trx_lst, sizeof(trx_lst) / sizeof(TRX_DESCRIPTOR), v_name, v_pn);

            if (!trx)
            {
                match = 0;
                /* Unidentified TRX configuration */
                if (trx_ff == TRX_SFF)
                    trx = &default_on_board_trx;
                else
                    trx = &default_pluggable_trx;
            }

            trx_print_descriptor(trx_ff, v_name, v_name_len, v_pn, v_pn_len, v_rev, v_rev_len, match, trx->type);
            break;
        default:
            BCM_LOG_ERROR(BCM_LOG_ID_OPTICALDET, "Opticaldet: Illegal TRX type %d\n", trx_ff);
    }

    return trx;
}

static void trx_activate(TRX_DESCRIPTOR *desc, int bus)
{
    if (desc && desc->activation_func)
        desc->activation_func(bus);
}

static void trx_fixup(TRX_DESCRIPTOR *desc, int bus)
{
    if (desc->tx_pwr_down_cfg_req)
    {
        trxbus_mon_write(bus, bcmsfp_mon_tx_power_down, 0, TRX_ACTIVE_HIGH == desc->tx_pwr_down_polarity);
    }
}

void *opticaldet_trx_register(int bus) 
{
    TRX_DESCRIPTOR *desc;

    if (!(desc = detect(bus)))
        return NULL;

    trx_fixup(desc, bus);
    trx_activate(desc, bus);
    return desc;
}
