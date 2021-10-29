/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2017:DUAL/GPL:standard
   
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
/*
 * xport_ag.h
 *
 */

//includes
#if defined(CONFIG_BCM963158)
#include "bcm63158_drivers_xport_ag.h"
#include "bcm63158_xport_intr_ag.h"
#include "bcm63158_xport_mab_ag.h"
#include "bcm63158_xport_mib_core_ag.h"
#include "bcm63158_xport_mib_reg_ag.h"
#include "bcm63158_xport_reg_ag.h"
#include "bcm63158_xport_xlmac_core_ag.h"
#include "bcm63158_xport_xlmac_reg_ag.h"
#else
#endif
