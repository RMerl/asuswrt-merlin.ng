#
# Helper makefile for building PHY included in wl.mk
#
# Copyright (C) 2017, Broadcom. All Rights Reserved.
# 
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
# 
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# $Id$

#
# New PHY Architecture
#

PHY_TOP_DIR = src/wl/phymods

NPHY ?= 1
ifneq ($(MINIAP),1)
	LCNPHY ?= 1
	LCN40PHY ?= 1
	HTPHY ?= 1
	ACPHY ?= 1
endif

# common
#PHY_FLAGS += -DWLC_SW_DIVERSITY
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/core/phy.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/core/phy_cmn.c
PHY_SRC_HI += $(PHY_TOP_DIR)/cmn/core/phy_high.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/core/phy_type_disp.c
PHY_SRC_HI += $(PHY_TOP_DIR)/cmn/core/phy_type_disp_high.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/core/phy_iovt.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/core/phy_ioct.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/dbg/phy_dbg.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/misc/phy_rstr.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/utils/phy_utils_misc.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/utils/phy_utils_reg.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/utils/phy_utils_var.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/utils/phy_utils_status.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/utils/phy_utils_math.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/utils/phy_utils_channel.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/utils/phy_utils_radio.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/init/phy_init.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/tbl/phy_tbl.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/tbl/phy_tbl_iov.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/rssi/phy_rssi.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/radar/phy_radar.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/radar/phy_radar_utils.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/radar/phy_radar_iov.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/tpc/phy_tpc.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/antdiv/phy_antdiv.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/ana/phy_ana.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/radio/phy_radio.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/temp/phy_temp.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/temp/phy_temp_iov.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/chanmgr/phy_chanmgr.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/chanmgr/phy_chanmgr_notif.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/wd/phy_wd.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/btcx/phy_btcx.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/noise/phy_noise.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/calmgr/phy_calmgr.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/rxiqcal/phy_rxiqcal.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/txiqlocal/phy_txiqlocal.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/papdcal/phy_papdcal.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/vcocal/phy_vcocal.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/cache/phy_cache.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/misc/phy_misc.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/lpc/phy_lpc.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/tssical/phy_tssical.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/rxspur/phy_rxspur.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/fcbs/phy_fcbs.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/rxgcrs/phy_rxgcrs.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/samp/phy_samp.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/mu/phy_mu.c


# NPHY
ifeq ($(NPHY),1)
PHY_SRC_LO += $(PHY_TOP_DIR)/n/core/phy_n.c
PHY_SRC += $(PHY_TOP_DIR)/n/core/phy_n_iovt.c
PHY_SRC_HI += $(PHY_TOP_DIR)/n/core/phy_n_iovt_high.c
PHY_SRC += $(PHY_TOP_DIR)/n/core/phy_n_ioct.c
PHY_SRC_HI += $(PHY_TOP_DIR)/n/core/phy_n_ioct_high.c
PHY_SRC_LO += $(PHY_TOP_DIR)/n/tbl/phy_n_tbl.c
PHY_SRC += $(PHY_TOP_DIR)/n/tbl/phy_n_tbl_iov.c
PHY_SRC_LO += $(PHY_TOP_DIR)/n/rssi/phy_n_rssi.c
PHY_SRC_LO += $(PHY_TOP_DIR)/n/radar/phy_n_radar.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/radar/phy_radar_shared.c
PHY_SRC_LO += $(PHY_TOP_DIR)/n/noise/phy_n_noise.c
PHY_SRC_LO += $(PHY_TOP_DIR)/n/antdiv/phy_n_antdiv.c
PHY_SRC_LO += $(PHY_TOP_DIR)/n/tpc/phy_n_tpc.c
PHY_SRC_LO += $(PHY_TOP_DIR)/n/ana/phy_n_ana.c
PHY_SRC_LO += $(PHY_TOP_DIR)/n/radio/phy_n_radio.c
PHY_SRC_LO += $(PHY_TOP_DIR)/n/temp/phy_n_temp.c
PHY_SRC_LO += $(PHY_TOP_DIR)/n/rxiqcal/phy_n_rxiqcal.c
PHY_SRC_LO += $(PHY_TOP_DIR)/n/txiqlocal/phy_n_txiqlocal.c
PHY_SRC_LO += $(PHY_TOP_DIR)/n/papdcal/phy_n_papdcal.c
PHY_SRC_LO += $(PHY_TOP_DIR)/n/vcocal/phy_n_vcocal.c
PHY_SRC_LO += $(PHY_TOP_DIR)/n/calmgr/phy_n_calmgr.c
PHY_SRC_LO += $(PHY_TOP_DIR)/n/samp/phy_n_samp.c
endif

# LCNPHY
ifeq ($(LCNPHY),1)
PHY_SRC_LO += $(PHY_TOP_DIR)/lcn/core/phy_lcn.c
PHY_SRC += $(PHY_TOP_DIR)/lcn/core/phy_lcn_iovt.c
PHY_SRC_HI += $(PHY_TOP_DIR)/lcn/core/phy_lcn_iovt_high.c
PHY_SRC += $(PHY_TOP_DIR)/lcn/core/phy_lcn_ioct.c
PHY_SRC_HI += $(PHY_TOP_DIR)/lcn/core/phy_lcn_ioct_high.c
PHY_SRC_LO += $(PHY_TOP_DIR)/lcn/tbl/phy_lcn_tbl.c
PHY_SRC += $(PHY_TOP_DIR)/lcn/tbl/phy_lcn_tbl_iov.c
PHY_SRC_LO += $(PHY_TOP_DIR)/lcn/rssi/phy_lcn_rssi.c
PHY_SRC_LO += $(PHY_TOP_DIR)/lcn/tpc/phy_lcn_tpc.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/tpc/phy_tpc_shared.c
PHY_SRC_LO += $(PHY_TOP_DIR)/lcn/antdiv/phy_lcn_antdiv.c
PHY_SRC_LO += $(PHY_TOP_DIR)/lcn/ana/phy_lcn_ana.c
PHY_SRC_LO += $(PHY_TOP_DIR)/lcn/radio/phy_lcn_radio.c
PHY_SRC_LO += $(PHY_TOP_DIR)/lcn/noise/phy_lcn_noise.c
endif

# LCN40PHY
ifeq ($(LCN40PHY),1)
PHY_SRC_LO += $(PHY_TOP_DIR)/lcn40/core/phy_lcn40.c
PHY_SRC += $(PHY_TOP_DIR)/lcn40/core/phy_lcn40_iovt.c
PHY_SRC_HI += $(PHY_TOP_DIR)/lcn40/core/phy_lcn40_iovt_high.c
PHY_SRC += $(PHY_TOP_DIR)/lcn40/core/phy_lcn40_ioct.c
PHY_SRC_HI += $(PHY_TOP_DIR)/lcn40/core/phy_lcn40_ioct_high.c
PHY_SRC_LO += $(PHY_TOP_DIR)/lcn40/misc/phy_lcn40_rstr.c
PHY_SRC_LO += $(PHY_TOP_DIR)/lcn40/tbl/phy_lcn40_tbl.c
PHY_SRC += $(PHY_TOP_DIR)/lcn40/tbl/phy_lcn40_tbl_iov.c
PHY_SRC_LO += $(PHY_TOP_DIR)/lcn40/rssi/phy_lcn40_rssi.c
PHY_SRC_LO += $(PHY_TOP_DIR)/lcn40/noise/phy_lcn40_noise.c
PHY_SRC_LO += $(PHY_TOP_DIR)/lcn40/tpc/phy_lcn40_tpc.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/tpc/phy_tpc_shared.c
PHY_SRC_LO += $(PHY_TOP_DIR)/lcn40/antdiv/phy_lcn40_antdiv.c
PHY_SRC_LO += $(PHY_TOP_DIR)/lcn40/ana/phy_lcn40_ana.c
PHY_SRC_LO += $(PHY_TOP_DIR)/lcn40/radio/phy_lcn40_radio.c
PHY_SRC_LO += $(PHY_TOP_DIR)/lcn40/samp/phy_lcn40_samp.c
endif

# HTPHY
ifeq ($(HTPHY),1)
PHY_SRC_LO += $(PHY_TOP_DIR)/ht/core/phy_ht.c
PHY_SRC += $(PHY_TOP_DIR)/ht/core/phy_ht_iovt.c
PHY_SRC_HI += $(PHY_TOP_DIR)/ht/core/phy_ht_iovt_high.c
PHY_SRC += $(PHY_TOP_DIR)/ht/core/phy_ht_ioct.c
PHY_SRC_HI += $(PHY_TOP_DIR)/ht/core/phy_ht_ioct_high.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ht/tbl/phy_ht_tbl.c
PHY_SRC += $(PHY_TOP_DIR)/ht/tbl/phy_ht_tbl_iov.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ht/rssi/phy_ht_rssi.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ht/radar/phy_ht_radar.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/radar/phy_radar_shared.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ht/noise/phy_ht_noise.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ht/tpc/phy_ht_tpc.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ht/ana/phy_ht_ana.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ht/radio/phy_ht_radio.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ht/temp/phy_ht_temp.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ht/rxiqcal/phy_ht_rxiqcal.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ht/txiqlocal/phy_ht_txiqlocal.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ht/papdcal/phy_ht_papdcal.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ht/vcocal/phy_ht_vcocal.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ht/calmgr/phy_ht_calmgr.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ht/samp/phy_ht_samp.c
endif

# ACPHY
ifeq ($(ACPHY),1)
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/core/phy_ac.c
PHY_SRC += $(PHY_TOP_DIR)/ac/core/phy_ac_iovt.c
PHY_SRC_HI += $(PHY_TOP_DIR)/ac/core/phy_ac_iovt_high.c
PHY_SRC += $(PHY_TOP_DIR)/ac/core/phy_ac_ioct.c
PHY_SRC_HI += $(PHY_TOP_DIR)/ac/core/phy_ac_ioct_high.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/tbl/phy_ac_tbl.c
PHY_SRC += $(PHY_TOP_DIR)/ac/tbl/phy_ac_tbl_iov.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/rssi/phy_ac_rssi.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/radar/phy_ac_radar.c
PHY_SRC_LO += $(PHY_TOP_DIR)/cmn/radar/phy_radar_shared.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/noise/phy_ac_noise.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/tpc/phy_ac_tpc.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/ana/phy_ac_ana.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/radio/phy_ac_radio.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/temp/phy_ac_temp.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/rxiqcal/phy_ac_rxiqcal.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/txiqlocal/phy_ac_txiqlocal.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/papdcal/phy_ac_papdcal.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/vcocal/phy_ac_vcocal.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/antdiv/phy_ac_antdiv.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/btcx/phy_ac_btcx.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/cache/phy_ac_cache.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/calmgr/phy_ac_calmgr.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/chanmgr/phy_ac_chanmgr.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/fcbs/phy_ac_fcbs.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/lpc/phy_ac_lpc.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/misc/phy_ac_misc.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/rxgcrs/phy_ac_rxgcrs.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/rxspur/phy_ac_rxspur.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/tssical/phy_ac_tssical.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/samp/phy_ac_samp.c
PHY_SRC_LO += $(PHY_TOP_DIR)/ac/mu/phy_ac_mu.c
endif

#
# Obsoleting PHY Architecture
#

PHY_SRC += src/wl/phy/wlc_phy_iovar.c
PHY_SRC += src/wl/phy/wlc_phy_ioctl.c

PHY_SRC_LO += src/wl/phy/wlc_phy_cmn.c
PHY_SRC_LO += src/wl/phy/wlc_phy_n.c
PHY_SRC_LO += src/wl/phy/wlc_phy_radio_n.c
PHY_SRC_LO += src/wl/phy/wlc_phy_extended_n.c
PHY_SRC_LO += src/wl/phy/wlc_phytbl_n.c

ifneq ($(MINIAP),1)
	PHY_SRC_LO += src/wl/phy/wlc_phytbl_ac.c
	PHY_SRC_LO += src/wl/phy/wlc_phytbl_20691.c
	PHY_SRC_LO += src/wl/phy/wlc_phytbl_ac_gains.c
	PHY_SRC_LO += src/wl/phy/wlc_phy_ac.c
	PHY_SRC_LO += src/wl/phy/wlc_phy_ac_gains.c
	PHY_SRC_LO += src/wl/phy/wlc_phy_ht.c
	PHY_SRC_LO += src/wl/phy/wlc_phytbl_ht.c
	PHY_SRC_LO += src/wl/phy/wlc_phy_lcn40.c
	PHY_SRC_LO += src/wl/phy/wlc_phytbl_lcn40.c
	PHY_SRC_LO += src/wl/phy/wlc_phy_lcn.c
	PHY_SRC_LO += src/wl/phy/wlc_phytbl_lcn.c
endif
