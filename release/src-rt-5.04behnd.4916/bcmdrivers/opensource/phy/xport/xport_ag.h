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
#elif defined(CONFIG_BCM94912)
#include "bcm4912_drivers_xport_ag.h"
#include "bcm4912_xport_intr_ag.h"
#include "bcm4912_xport_mab_ag.h"
#include "bcm4912_xport_mib_core_ag.h"
#include "bcm4912_xport_mib_reg_ag.h"
#include "bcm4912_xport_portreset_ag.h"
#include "bcm4912_xport_top_ag.h"
#include "bcm4912_xport_xlmac_core_ag.h"
#include "bcm4912_xport_xlmac_reg_ag.h"
#elif defined(CONFIG_BCM96813)
#include "bcm6813_drivers_xport_ag.h"
#include "bcm6813_xport_intr_ag.h"
#include "bcm6813_xport_mab_ag.h"
#include "bcm6813_xport_mib_core_ag.h"
#include "bcm6813_xport_mib_reg_ag.h"
#include "bcm6813_xport_portreset_ag.h"
#include "bcm6813_xport_top_ag.h"
#include "bcm6813_xport_xlmac_core_ag.h"
#include "bcm6813_xport_xlmac_reg_ag.h"
#include "bcm6813_xport_wol_ard_ag.h"
#include "bcm6813_xport_wol_mpd_ag.h"
#elif defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880)
#include "bcm6888_drivers_xport_ag.h"
#include "bcm6888_xport_intr_ag.h"
#include "bcm6888_xport_mab_ag.h"
#include "bcm6888_xport_mib_core_ag.h"
#include "bcm6888_xport_mib_reg_ag.h"
#include "bcm6888_xport_portreset_ag.h"
#include "bcm6888_xport_top_ag.h"
#include "bcm6888_xport_xlmac_core_ag.h"
#include "bcm6888_xport_xlmac_reg_ag.h"
#include "bcm6888_xport_mpd_ag.h"
#elif defined(CONFIG_BCM96837)
#include "bcm6837_drivers_xport_ag.h"
#include "bcm6837_xport_intr_ag.h"
#include "bcm6837_xport_mab_ag.h"
#include "bcm6837_xport_mib_core_ag.h"
#include "bcm6837_xport_mib_reg_ag.h"
#include "bcm6837_xport_portreset_ag.h"
#include "bcm6837_xport_top_ag.h"
#include "bcm6837_xport_xlmac_core_ag.h"
#include "bcm6837_xport_xlmac_reg_ag.h"
#include "bcm6837_xport_wol_ard_ag.h"
#include "bcm6837_xport_wol_mpd_ag.h"
#else
#error undefined chip
#endif
