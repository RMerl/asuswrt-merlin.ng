/***********************************************************************
 *
 * Copyright (c) 2021  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2021:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 *
 ************************************************************************/

#ifndef _MACSEC_COMMON_H_
#define _MACSEC_COMMON_H_

#include <linux/printk.h>

#define PR_ERR(fmt, args...) pr_err("ERROR: %s:%d " fmt, __func__, __LINE__, ## args)
#define PR_DBG(fmt, args...) printk("DBG: %s:%d " fmt "\n", __func__, __LINE__, ## args)

#define FL_UNIT 0

#endif
