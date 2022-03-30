/*
<:copyright-BRCM:2017:DUAL/GPL:standard

   Copyright (c) 2017 Broadcom 
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
 
#include <linux/string.h>
#include <bcmsfp.h>

void ltw2601_activation(struct device *dev)
{
    uint8_t trx_act_seq[4] = {0x12, 0x34, 0x56, 0x78};

    sfp_mon_write_buf(dev, bcmsfp_mon_xfp_password, 0, trx_act_seq, sizeof(trx_act_seq));
}

