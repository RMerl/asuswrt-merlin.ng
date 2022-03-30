/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
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

#include <linux/module.h>
#include "bcmsfp.h"

struct device *bcm_i2c_legacy_sfp_get_dev(int bus);

int bcm_i2c_legacy_optics_tx_control(int bus, int enable)
{
    struct device *dev = bcm_i2c_legacy_sfp_get_dev(bus);

    if (dev)
        sfp_mon_write(dev, bcmsfp_mon_tx_enable, 0, enable);

    return 0;
}
EXPORT_SYMBOL(bcm_i2c_legacy_optics_tx_control);

int bcm_i2c_legacy_pon_optics_sd_get(int bus, unsigned char * sig_det)
{
    long v = 1;
    struct device *dev = bcm_i2c_legacy_sfp_get_dev(bus);

    if (dev)
        sfp_mon_read(dev, bcmsfp_mon_los, 0, &v);

    *sig_det = !v;

    return 0;
}
EXPORT_SYMBOL(bcm_i2c_legacy_pon_optics_sd_get);

MODULE_LICENSE("GPL");

