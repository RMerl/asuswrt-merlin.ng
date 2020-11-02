#
# Helper makefile for building PHY included in wl.mk
#
# Copyright (C) 2020, Broadcom. All Rights Reserved.
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
#
# <<Broadcom-WL-IPTag/Open:>>
#
# $Id$

#
# New PHY Architecture
#

PHY_TOP_DIR = components/phy

NPHY ?= 1
ifneq ($(MINIAP),1)
	ACPHY ?= 1
endif

# ---------------------------------------------------------------------------------------------- #
# PHYMODS source
#

# CMN
PHY_SRC += $(PHY_TOP_DIR)/cmn/core/phy.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/core/phy_cmn.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/core/phy_type_disp.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/dbg/phy_dbg.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/misc/phy_rstr.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/utils/phy_utils_misc.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/utils/phy_utils_reg.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/utils/phy_utils_var.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/utils/phy_utils_status.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/utils/phy_utils_math.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/utils/phy_utils_channel.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/utils/phy_utils_radio.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/utils/phy_utils_pmu.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/init/phy_init.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/hirssi/phy_hirssi.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/tbl/phy_tbl.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/rssi/phy_rssi.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/radar/phy_radar.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/radar/phy_radar_utils.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/tpc/phy_tpc.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/txpwrcap/phy_txpwrcap.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/antdiv/phy_antdiv.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/ana/phy_ana.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/et/phy_et.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/radio/phy_radio.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/temp/phy_temp.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/chanmgr/phy_chanmgr.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/chanmgr/phy_chanmgr_notif.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/wd/phy_wd.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/btcx/phy_btcx.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/noise/phy_noise.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/calmgr/phy_calmgr.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/rxiqcal/phy_rxiqcal.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/txiqlocal/phy_txiqlocal.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/papdcal/phy_papdcal.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/vcocal/phy_vcocal.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/cache/phy_cache.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/misc/phy_misc.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/lpc/phy_lpc.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/tssical/phy_tssical.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/rxspur/phy_rxspur.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/fcbs/phy_fcbs.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/rxgcrs/phy_rxgcrs.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/samp/phy_samp.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/mu/phy_mu.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/dccal/phy_dccal.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/ocl/phy_ocl.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/hecap/phy_hecap.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/hc/phy_hc.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/stf/phy_stf.c
ifeq ($(WL_PROXDETECT),1)
PHY_SRC += $(PHY_TOP_DIR)/cmn/tof/phy_tof.c
endif
PHY_SRC += $(PHY_TOP_DIR)/cmn/prephy/phy_prephy.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/vasip/phy_vasip.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/smc/phy_smc.c

# CMN IOVARS
PHY_SRC += $(PHY_TOP_DIR)/cmn/core/phy_high.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/core/phy_type_disp_high.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/core/phy_iovt.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/core/phy_ioct.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/btcx/phy_btcx_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/chanmgr/phy_chanmgr_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/calmgr/phy_calmgr_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/dbg/phy_dbg_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/hirssi/phy_hirssi_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/tbl/phy_tbl_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/temp/phy_temp_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/samp/phy_samp_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/radar/phy_radar_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/misc/phy_misc_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/tpc/phy_tpc_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/txpwrcap/phy_txpwrcap_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/rxgcrs/phy_rxgcrs_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/antdiv/phy_antdiv_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/papdcal/phy_papdcal_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/rssi/phy_rssi_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/rxspur/phy_rxspur_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/vcocal/phy_vcocal_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/tssical/phy_tssical_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/fcbs/phy_fcbs_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/txiqlocal/phy_txiqlocal_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/noise/phy_noise_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/hc/phy_hc_iov.c

# NPHY
ifeq ($(NPHY),1)
PHY_SRC += $(PHY_TOP_DIR)/n/core/phy_n.c
PHY_SRC += $(PHY_TOP_DIR)/n/btcx/phy_n_btcx.c
PHY_SRC += $(PHY_TOP_DIR)/n/tbl/phy_n_tbl.c
PHY_SRC += $(PHY_TOP_DIR)/n/rssi/phy_n_rssi.c
PHY_SRC += $(PHY_TOP_DIR)/n/radar/phy_n_radar.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/radar/phy_radar_shared.c
PHY_SRC += $(PHY_TOP_DIR)/n/noise/phy_n_noise.c
PHY_SRC += $(PHY_TOP_DIR)/n/antdiv/phy_n_antdiv.c
PHY_SRC += $(PHY_TOP_DIR)/n/tpc/phy_n_tpc.c
PHY_SRC += $(PHY_TOP_DIR)/n/ana/phy_n_ana.c
PHY_SRC += $(PHY_TOP_DIR)/n/radio/phy_n_radio.c
PHY_SRC += $(PHY_TOP_DIR)/n/temp/phy_n_temp.c
PHY_SRC += $(PHY_TOP_DIR)/n/rxiqcal/phy_n_rxiqcal.c
PHY_SRC += $(PHY_TOP_DIR)/n/txiqlocal/phy_n_txiqlocal.c
PHY_SRC += $(PHY_TOP_DIR)/n/papdcal/phy_n_papdcal.c
PHY_SRC += $(PHY_TOP_DIR)/n/vcocal/phy_n_vcocal.c
PHY_SRC += $(PHY_TOP_DIR)/n/calmgr/phy_n_calmgr.c
PHY_SRC += $(PHY_TOP_DIR)/n/chanmgr/phy_n_chanmgr.c
PHY_SRC += $(PHY_TOP_DIR)/n/samp/phy_n_samp.c
PHY_SRC += $(PHY_TOP_DIR)/n/cache/phy_n_cache.c
PHY_SRC += $(PHY_TOP_DIR)/n/misc/phy_n_misc.c
PHY_SRC += $(PHY_TOP_DIR)/n/rxgcrs/phy_n_rxgcrs.c
PHY_SRC += $(PHY_TOP_DIR)/n/rxspur/phy_n_rxspur.c
PHY_SRC += $(PHY_TOP_DIR)/n/lpc/phy_n_lpc.c
PHY_SRC += $(PHY_TOP_DIR)/n/dbg/phy_n_dbg.c
PHY_SRC += $(PHY_TOP_DIR)/n/stf/phy_n_stf.c

# NPHY IOVARS
PHY_SRC += $(PHY_TOP_DIR)/n/core/phy_n_iovt_high.c
PHY_SRC += $(PHY_TOP_DIR)/n/core/phy_n_ioct_high.c
PHY_SRC += $(PHY_TOP_DIR)/n/core/phy_n_iovt.c
PHY_SRC += $(PHY_TOP_DIR)/n/core/phy_n_ioct.c
PHY_SRC += $(PHY_TOP_DIR)/n/tbl/phy_n_tbl_iov.c
endif

# ACPHY
ifeq ($(ACPHY),1)
PHY_SRC += $(PHY_TOP_DIR)/ac/core/phy_ac.c
PHY_SRC += $(PHY_TOP_DIR)/ac/et/phy_ac_et.c
PHY_SRC += $(PHY_TOP_DIR)/ac/et/phy_ac_et_data.c
PHY_SRC += $(PHY_TOP_DIR)/ac/hirssi/phy_ac_hirssi.c
PHY_SRC += $(PHY_TOP_DIR)/ac/tbl/phy_ac_tbl.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rssi/phy_ac_rssi.c
PHY_SRC += $(PHY_TOP_DIR)/ac/radar/phy_ac_radar.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/radar/phy_radar_shared.c
PHY_SRC += $(PHY_TOP_DIR)/ac/noise/phy_ac_noise.c
PHY_SRC += $(PHY_TOP_DIR)/ac/tpc/phy_ac_tpc.c
PHY_SRC += $(PHY_TOP_DIR)/ac/txpwrcap/phy_ac_txpwrcap.c
PHY_SRC += $(PHY_TOP_DIR)/ac/ana/phy_ac_ana.c
PHY_SRC += $(PHY_TOP_DIR)/ac/radio/phy_ac_radio.c
PHY_SRC += $(PHY_TOP_DIR)/ac/radio/phy_ac_pllconfig_20698.c
PHY_SRC += $(PHY_TOP_DIR)/ac/radio/phy_ac_pllconfig_20704.c
PHY_SRC += $(PHY_TOP_DIR)/ac/radio/phy_ac_pllconfig_20707.c
PHY_SRC += $(PHY_TOP_DIR)/ac/radio/phy_ac_pllconfig_20709.c
PHY_SRC += $(PHY_TOP_DIR)/ac/temp/phy_ac_temp.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rxiqcal/phy_ac_rxiqcal.c
PHY_SRC += $(PHY_TOP_DIR)/ac/txiqlocal/phy_ac_txiqlocal.c
PHY_SRC += $(PHY_TOP_DIR)/ac/papdcal/phy_ac_papdcal.c
PHY_SRC += $(PHY_TOP_DIR)/ac/papdcal/phy_ac_papdcal_data.c
PHY_SRC += $(PHY_TOP_DIR)/ac/vcocal/phy_ac_vcocal.c
PHY_SRC += $(PHY_TOP_DIR)/ac/antdiv/phy_ac_antdiv.c
PHY_SRC += $(PHY_TOP_DIR)/ac/btcx/phy_ac_btcx.c
PHY_SRC += $(PHY_TOP_DIR)/ac/cache/phy_ac_cache.c
PHY_SRC += $(PHY_TOP_DIR)/ac/calmgr/phy_ac_calmgr.c
PHY_SRC += $(PHY_TOP_DIR)/ac/chanmgr/phy_ac_chanmgr.c
PHY_SRC += $(PHY_TOP_DIR)/ac/fcbs/phy_ac_fcbs.c
PHY_SRC += $(PHY_TOP_DIR)/ac/lpc/phy_ac_lpc.c
PHY_SRC += $(PHY_TOP_DIR)/ac/misc/phy_ac_misc.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rxgcrs/phy_ac_rxgcrs.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rxgcrs/phy_ac_rxgcrs_iov.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rxspur/phy_ac_rxspur.c
PHY_SRC += $(PHY_TOP_DIR)/ac/tssical/phy_ac_tssical.c
PHY_SRC += $(PHY_TOP_DIR)/ac/samp/phy_ac_samp.c
PHY_SRC += $(PHY_TOP_DIR)/ac/mu/phy_ac_mu.c
PHY_SRC += $(PHY_TOP_DIR)/ac/dbg/phy_ac_dbg.c
PHY_SRC += $(PHY_TOP_DIR)/ac/dccal/phy_ac_dccal.c
PHY_SRC += $(PHY_TOP_DIR)/ac/ocl/phy_ac_ocl.c
PHY_SRC += $(PHY_TOP_DIR)/ac/hecap/phy_ac_hecap.c
PHY_SRC += $(PHY_TOP_DIR)/ac/samp/phy_ac_samp_data.c
PHY_SRC += $(PHY_TOP_DIR)/ac/stf/phy_ac_stf.c
ifeq ($(WL_PROXDETECT),1)
PHY_SRC += $(PHY_TOP_DIR)/ac/tof/phy_ac_tof.c
endif
PHY_SRC += $(PHY_TOP_DIR)/ac/prephy/phy_ac_prephy.c
PHY_SRC += $(PHY_TOP_DIR)/ac/hc/phy_ac_hc.c
PHY_SRC += $(PHY_TOP_DIR)/ac/vasip/phy_ac_vasip.c
PHY_SRC += $(PHY_TOP_DIR)/ac/smc/phy_ac_smc.c

# ACPHY IOVARS
PHY_SRC += $(PHY_TOP_DIR)/ac/core/phy_ac_iovt_high.c
PHY_SRC += $(PHY_TOP_DIR)/ac/core/phy_ac_ioct_high.c
PHY_SRC += $(PHY_TOP_DIR)/ac/core/phy_ac_iovt.c
PHY_SRC += $(PHY_TOP_DIR)/ac/core/phy_ac_ioct.c
PHY_SRC += $(PHY_TOP_DIR)/ac/chanmgr/phy_ac_chanmgr_iov.c
PHY_SRC += $(PHY_TOP_DIR)/ac/misc/phy_ac_misc_iov.c
PHY_SRC += $(PHY_TOP_DIR)/ac/radio/phy_ac_radio_iov.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rssi/phy_ac_rssi_iov.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rxgcrs/phy_ac_rxgcrs_iov.c
PHY_SRC += $(PHY_TOP_DIR)/ac/tbl/phy_ac_tbl_iov.c
PHY_SRC += $(PHY_TOP_DIR)/ac/tpc/phy_ac_tpc_iov.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rxspur/phy_ac_rxspur_iov.c
endif

# ---------------------------------------------------------------------------------------------- #
# Phy Features
#
ifeq ($(WLOCL),1)
	EXTRA_DFLAGS += -DOCL
	PHY_SRC += $(PHY_TOP_DIR)/ac/ocl/phy_ac_ocl.c
	PHY_SRC += $(PHY_TOP_DIR)/cmn/ocl/phy_ocl.c
endif

ifeq ($(DSI),1)
	EXTRA_DFLAGS += -DWL_DSI

	PHY_SRC += $(PHY_TOP_DIR)/cmn/dsi/phy_dsi.c
	PHY_SRC += $(PHY_TOP_DIR)/cmn/dsi/phy_dsi_iov.c

	ifeq ($(ACPHY),1)
		PHY_SRC += $(PHY_TOP_DIR)/ac/dsi/phy_ac_dsi.c
		PHY_SRC += $(PHY_TOP_DIR)/ac/dsi/phy_ac_dsi_data.c
		PHY_SRC += $(PHY_TOP_DIR)/ac/dsi/phy_ac_dsi_prototype.c
	endif

	ifeq ($(DSI_DISABLED), 1)
		EXTRA_DFLAGS += -DWL_DSI_DISABLED
	endif
else
	EXTRA_DFLAGS += -DWL_DSI_DISABLED
endif

ifeq ($(APAPD),1)
	EXTRA_DFLAGS += -DWL_APAPD
	ifeq ($(APAPD_DISABLED), 1)
		EXTRA_DFLAGS += -DWL_APAPD_DISABLED
	endif
else
	EXTRA_DFLAGS += -DWL_APAPD_DISABLED
endif

ifeq ($(WBPAPD),1)
	EXTRA_DFLAGS += -DWL_WBPAPD
	ifeq ($(WBPAPD_DISABLED), 1)
		EXTRA_DFLAGS += -DWL_WBPAPD_DISABLED
	endif
else
	EXTRA_DFLAGS += -DWL_WBPAPD_DISABLED
endif

ifeq ($(ET),1)
	EXTRA_DFLAGS += -DWL_ETMODE
	ifeq ($(ACPHY), 1)
		PHY_SRC += $(PHY_TOP_DIR)/ac/et/phy_ac_et.c
		PHY_SRC += $(PHY_TOP_DIR)/ac/et/phy_ac_et_data.c
	endif
else
	EXTRA_DFLAGS += -DWL_ETMODE_DISABLED
endif

ifeq ($(PHYNAP),1)
	EXTRA_DFLAGS += -DWL_NAP
	PHY_SRC += $(PHY_TOP_DIR)/cmn/nap/phy_nap.c
	PHY_SRC += $(PHY_TOP_DIR)/cmn/nap/phy_nap_iov.c
	ifeq ($(ACPHY), 1)
		PHY_SRC += $(PHY_TOP_DIR)/ac/nap/phy_ac_nap.c
	endif
	ifeq ($(PHYNAP_DISABLED), 1)
		EXTRA_DFLAGS += -DWL_NAP_DISABLED
	endif
else
	EXTRA_DFLAGS += -DWL_NAP_DISABLED
endif

# ---------------------------------------------------------------------------------------------- #
# Obsoleting PHY Architecture
#

PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phy_iovar.c
PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phy_ioctl.c

ifeq ($(NPHY),1)
PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phy_n.c
PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phy_radio_n.c
PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phy_extended_n.c
PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_n.c
endif

ifneq ($(MINIAP),1)
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_ac.c
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_20691.c
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_20693.c
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_20694.c
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_20695.c
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_20697.c
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_20698.c
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_20704.c
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_20707.c
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_20709.c
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_ac_gains.c
endif
