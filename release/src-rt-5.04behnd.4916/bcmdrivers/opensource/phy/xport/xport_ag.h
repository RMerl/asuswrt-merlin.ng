/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2017:DUAL/GPL:standard

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
