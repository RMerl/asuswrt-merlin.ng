// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

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
#elif defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
#include "bcm4912_drivers_xport_ag.h"
#include "bcm4912_xport_intr_ag.h"
#include "bcm4912_xport_mab_ag.h"
#include "bcm4912_xport_mib_core_ag.h"
#include "bcm4912_xport_mib_reg_ag.h"
#include "bcm4912_xport_portreset_ag.h"
#include "bcm4912_xport_top_ag.h"
#include "bcm4912_xport_xlmac_core_ag.h"
#include "bcm4912_xport_xlmac_reg_ag.h"
#else
#endif
