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
/****************************************************************************
 * vFlash rpbm  driver
 *
 * Author: Dima Mamut <dima.mamut@broadcom.com>
*****************************************************************************/
#ifndef _VFBIO_RPMB_H_
#define _VFBIO_RPMB_H_

#include <linux/mmc/ioctl.h>
#include "vfbio_lvm.h"

int vfbio_rpmb_ioctl_cmd(struct mmc_ioc_cmd *ic_ptr);
int vfbio_rpmb_ioctl_multi_cmd(struct mmc_ioc_multi_cmd *user);

#endif /* _VFBIO_RPMB_H_ */
