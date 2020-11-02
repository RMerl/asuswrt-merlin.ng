/*
 * Key Management Module Implementation - WOWL support
 * Copyright (c) 2013 Broadcom Corporation, All rights reserved.
 * $Id: km_wowl_hw.h 401822 2013-05-13 20:48:06Z $
 */

#ifndef _km_wowl_hw_h_
#define _km_wowl_hw_h_

#include "km.h"
#include "km_key.h"
#include "km_hw.h"

km_hw_t *km_wowl_hw_attach(wlc_info_t *wlc, wlc_keymgmt_t *km);
void km_wowl_hw_detach(km_hw_t **hw);
void km_wowl_hw_set_mode(km_hw_t *hw, scb_t *scb, bool enable);

#endif /* _km_wowl_hw_h_ */
