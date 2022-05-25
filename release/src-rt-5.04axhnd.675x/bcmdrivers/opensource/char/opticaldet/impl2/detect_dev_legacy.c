/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
   All Rights Reserved

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

#include "trx_descr.h"
#include "bcmsfp.h"
#include <linux/kernel.h>
#include <linux/string.h>
extern struct device *bcm_i2c_legacy_sfp_get_dev(int bus);
extern void *bcm_i2c_legacy_opticaldet_desc_get_dev(int bus);

#define TRX_DESC(p) (p)
#define TRX_DEFINE \
    TRX_DESCRIPTOR *trx = NULL;\
    int trx_ret;
#define TRX_GET_RET\
    if ((trx_ret = get_bus(bus, &trx)))\
        return trx_ret;

static int get_bus(int bus, TRX_DESCRIPTOR **trxp)
{
    TRX_DESCRIPTOR *trx = bcm_i2c_legacy_opticaldet_desc_get_dev(bus);

    if (!TRX_DESC(trx))
        return OPTICALDET_NOBUS;

    *trxp = trx;
    return 0;
}

int _trx_get_tx_sd_polarity(TRX_DESCRIPTOR *desc, TRX_SIG_ACTIVE_POLARITY *tx_sd_polarity_p)
{ 
    if (!desc)
        return -1;

    *tx_sd_polarity_p = desc->tx_sd_polarity;
    return 0;
}

int _trx_get_supported_wan_type_bm(TRX_DESCRIPTOR *desc, SUPPORTED_WAN_TYPES_BITMAP *wan_type_bm)
{
    *wan_type_bm = SUPPORTED_WAN_TYPES_AUTO_SENSE_UNAVAILABLE;
    if (!desc)
        return -1;

    *wan_type_bm = desc->wan_types_bitmap;
    return 0;
}

int _trx_get_tx_sd_supported(TRX_DESCRIPTOR *desc, TRX_SIG_PRESENCE *signal_supported_p)
{
    if (!desc)
        return -1;

    *signal_supported_p = desc->tx_sd_supported;
    return 0;
}

int _trx_get_lbe_polarity(TRX_DESCRIPTOR *desc, TRX_SIG_ACTIVE_POLARITY *lbe_polarity_p)
{
    if (!desc)
        return -1;

    *lbe_polarity_p = desc->lbe_polarity;
    return 0;
}
EXPORT_SYMBOL(_trx_get_lbe_polarity);

int _trx_get_type(TRX_DESCRIPTOR *desc, TRX_TYPE *trx_type)
{
    if (!desc)
        return -1;

    *trx_type = desc->type;
    return 0;
}

int trx_get_tx_sd_polarity(int bus, TRX_SIG_ACTIVE_POLARITY *tx_sd_polarity_p)
{ 
    TRX_DEFINE;
    TRX_GET_RET;

    _trx_get_tx_sd_polarity(TRX_DESC(trx), tx_sd_polarity_p);
    return OPTICALDET_SUCCESS;
}
EXPORT_SYMBOL(trx_get_tx_sd_polarity);

int trx_get_supported_wan_type_bm(int bus, SUPPORTED_WAN_TYPES_BITMAP *wan_type_bm)
{
    TRX_DEFINE;

    *wan_type_bm = SUPPORTED_WAN_TYPES_AUTO_SENSE_UNAVAILABLE;
    TRX_GET_RET;

    _trx_get_supported_wan_type_bm(TRX_DESC(trx), wan_type_bm);
    return OPTICALDET_SUCCESS;
}
EXPORT_SYMBOL(trx_get_supported_wan_type_bm);

int trx_get_tx_sd_supported(int bus, TRX_SIG_PRESENCE *signal_supported_p)
{
    TRX_DEFINE;
    TRX_GET_RET;

    _trx_get_tx_sd_supported(TRX_DESC(trx), signal_supported_p);
    return OPTICALDET_SUCCESS;
}
EXPORT_SYMBOL(trx_get_tx_sd_supported);

#define MIN(a,b)  ((a) <= (b)? (a):(b))

int trx_get_full_info(int bus, TRX_INFOMATION *trx_info)
{
    int len;
    char *str;
    unsigned long wavelen;
    struct device *dev;
    TRX_DEFINE;
    TRX_GET_RET;

    if (TRX_DESC(trx)->form_factor == TRX_PMD)
    {
        strncpy(trx_info->vendor_name, TRX_DESC(trx)->vendor_name, sizeof(trx_info->vendor_name));
        strncpy(trx_info->vendor_pn, TRX_DESC(trx)->vendor_pn, sizeof(trx_info->vendor_pn));
        strcpy(trx_info->vendor_sn, "");
    }
    else
    {
        if (!(dev = bcm_i2c_legacy_sfp_get_dev(bus)))
            return OPTICALDET_NOSFP;

        sfp_mon_read_buf(dev, bcmsfp_mon_id_vendor_name, 0, &str, &len);
        strncpy(trx_info->vendor_name, str, MIN(len, sizeof(trx_info->vendor_name)));

        sfp_mon_read_buf(dev, bcmsfp_mon_id_vendor_pn, 0, &str, &len);
        strncpy(trx_info->vendor_pn, str, MIN(len, sizeof(trx_info->vendor_pn)));

        sfp_mon_read_buf(dev, bcmsfp_mon_id_vendor_sn, 0, &str, &len);
        strncpy(trx_info->vendor_sn, str, MIN(len, sizeof(trx_info->vendor_sn)));

        sfp_mon_read(dev, bcmsfp_mon_id_optical_wavelength, 0, &wavelen);
        trx_info->tx_wavlen = wavelen;
    }

    trx_info->form_factor = TRX_DESC(trx)->form_factor;
    trx_info->type = TRX_DESC(trx)->type;
    trx_info->wan_types_bitmap = TRX_DESC(trx)->wan_types_bitmap;
    trx_info->power_budget = TRX_DESC(trx)->power_budget;
    trx_info->rx_wavlen = TRX_DESC(trx)->rx_wavlen;

    return OPTICALDET_SUCCESS;
}

/* XXX: remove from epon */
int trx_get_lbe_polarity(int bus, TRX_SIG_ACTIVE_POLARITY *lbe_polarity_p)
{
    TRX_DEFINE;
    TRX_GET_RET;

    _trx_get_lbe_polarity(TRX_DESC(trx), lbe_polarity_p);
    return OPTICALDET_SUCCESS;
}
EXPORT_SYMBOL(trx_get_lbe_polarity);

int trx_get_type(int bus, TRX_TYPE *trx_type)
{
    TRX_DEFINE;
    TRX_GET_RET;

    _trx_get_type(TRX_DESC(trx), trx_type);
    return OPTICALDET_SUCCESS;
}
EXPORT_SYMBOL(trx_get_type);

