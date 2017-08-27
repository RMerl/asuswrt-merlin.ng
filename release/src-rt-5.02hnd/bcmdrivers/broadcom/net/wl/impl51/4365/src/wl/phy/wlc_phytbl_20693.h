/*
 * Declarations for Broadcom PHY core tables,
 * Networking Adapter Device Driver.
 *
 * THE CONTENTS OF THIS FILE IS TEMPORARY.
 * Eventually it'll be auto-generated.
 *
 * Copyright(c) 2012 Broadcom Corp.
 * All Rights Reserved.
 *
 * $Id$
 */

#ifndef _WLC_PHYTBL_20693_H_
#define _WLC_PHYTBL_20693_H_

#include <wlc_cfg.h>
#include <typedefs.h>

#include "wlc_phy_int.h"

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
#if defined(DBG_PHY_IOV)
extern radio_20xx_dumpregs_t dumpregs_20693_rev5[];

#endif	
#endif	/* BCMDBG || BCMDBG_DUMP */

/* Radio referred values tables */
extern radio_20xx_prefregs_t prefregs_20693_rev5[];
extern radio_20xx_prefregs_t prefregs_20693_rev6[];
extern radio_20xx_prefregs_t prefregs_20693_rev10[];
extern radio_20xx_prefregs_t prefregs_20693_rev13[];
extern radio_20xx_prefregs_t prefregs_20693_rev32[];

/* For 2g ipa only, to be removed after code addition */
extern uint16 acphy_radiogainqdb_20693_majrev3[128];

#endif	/* _WLC_PHYTBL_20693_H_ */
