#
# Helper makefile for building PHY included in wl.mk
#
# Copyright (C) 2023, Broadcom. All Rights Reserved.
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
ifeq ($(FTM),1)
	EXTRA_DFLAGS += -DWL_AZ
	PHY_SRC += $(PHY_TOP_DIR)/cmn/az/phy_az.c
	PHY_SRC += $(PHY_TOP_DIR)/cmn/az/phy_az_iov.c
endif
endif
PHY_SRC += $(PHY_TOP_DIR)/cmn/prephy/phy_prephy.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/vasip/phy_vasip.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/smc/phy_smc.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/wareng/phy_wareng.c
ifeq ($(TXSHAPER),1)
PHY_SRC += $(PHY_TOP_DIR)/cmn/txss/phy_txss.c
endif
ifeq ($(RFEM),1)
PHY_SRC += $(PHY_TOP_DIR)/cmn/rfem/phy_rfem.c
endif
PHY_SRC += $(PHY_TOP_DIR)/cmn/padroop/phy_padroop.c
ifeq ($(WL_PHY_REFRESH),1)
PHY_SRC += $(PHY_TOP_DIR)/cmn/refresh/phy_refresh.c
endif

# CMN IOVARS
PHY_SRC += $(PHY_TOP_DIR)/cmn/core/phy_iovt.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/core/phy_ioct.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/btcx/phy_btcx_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/chanmgr/phy_chanmgr_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/calmgr/phy_calmgr_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/dbg/phy_dbg_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/hirssi/phy_hirssi_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/tbl/phy_tbl_iov.c
PHY_SRC += $(PHY_TOP_DIR)/cmn/temp/phy_temp_iov.c
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
ifeq ($(TXSHAPER),1)
PHY_SRC += $(PHY_TOP_DIR)/cmn/txss/phy_txss_iov.c
endif
ifeq ($(WL_PHY_REFRESH),1)
PHY_SRC += $(PHY_TOP_DIR)/cmn/refresh/phy_refresh_iov.c
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
PHY_SRC += $(PHY_TOP_DIR)/ac/radio/phy_ac_pllconfig_20708.c
PHY_SRC += $(PHY_TOP_DIR)/ac/radio/phy_ac_pllconfig_20710.c
PHY_SRC += $(PHY_TOP_DIR)/ac/radio/phy_ac_pllconfig_20711.c
PHY_SRC += $(PHY_TOP_DIR)/ac/radio/phy_ac_pllconfig_20712.c
PHY_SRC += $(PHY_TOP_DIR)/ac/radio/phy_ac_pllconfig_20713.c
PHY_SRC += $(PHY_TOP_DIR)/ac/radio/phy_ac_pllconfig_20714.c
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
PHY_SRC += $(PHY_TOP_DIR)/ac/stf/phy_ac_stf.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rxiqcal/phy_ac_rxiqcal_iov.c
PHY_SRC += $(PHY_TOP_DIR)/ac/dccal/phy_ac_dccal_iov.c
ifeq ($(WL_PROXDETECT),1)
PHY_SRC += $(PHY_TOP_DIR)/ac/tof/phy_ac_tof.c
ifeq ($(FTM),1)
PHY_SRC += $(PHY_TOP_DIR)/ac/az/phy_ac_az.c
endif
endif
PHY_SRC += $(PHY_TOP_DIR)/ac/prephy/phy_ac_prephy.c
PHY_SRC += $(PHY_TOP_DIR)/ac/hc/phy_ac_hc.c
PHY_SRC += $(PHY_TOP_DIR)/ac/vasip/phy_ac_vasip.c
PHY_SRC += $(PHY_TOP_DIR)/ac/smc/phy_ac_smc.c
PHY_SRC += $(PHY_TOP_DIR)/ac/wareng/phy_ac_wareng.c
ifeq ($(TXSHAPER),1)
PHY_SRC += $(PHY_TOP_DIR)/ac/txss/phy_ac_txss.c
endif
ifeq ($(RFEM),1)
PHY_SRC += $(PHY_TOP_DIR)/ac/rfem/phy_ac_rfem.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rfem/phy_ac_rfem_iov.c
endif
PHY_SRC += $(PHY_TOP_DIR)/ac/padroop/phy_ac_padroop.c

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
PHY_SRC += $(PHY_TOP_DIR)/ac/noise/phy_ac_noise_iov.c
PHY_SRC += $(PHY_TOP_DIR)/ac/txss/phy_ac_txss_iov.c
PHY_SRC += $(PHY_TOP_DIR)/ac/dccal/phy_ac_dccal_iov.c
endif

# ---------------------------------------------------------------------------------------------- #
# Phy Features
#

ifeq ($(WLOCL),1)
	EXTRA_DFLAGS += -DOCL
	PHY_SRC += $(PHY_TOP_DIR)/ac/ocl/phy_ac_ocl.c
	PHY_SRC += $(PHY_TOP_DIR)/cmn/ocl/phy_ocl.c
endif

EXTRA_DFLAGS += -DWL_DSI_DISABLED

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
	WLFLAGS += -DWL_WBPAPD
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

ifeq ($(TXSHAPER),1)
	EXTRA_DFLAGS += -DWLC_TXSHAPER
	WLFLAGS += -DWLC_TXSHAPER
	ifeq ($(TXSHAPER_DISABLED), 1)
		EXTRA_DFLAGS += -DWLC_TXSHAPER_DISABLED
	endif
else
	EXTRA_DFLAGS += -DWLC_TXSHAPER_DISABLED
endif

ifeq ($(RFEM),1)
	EXTRA_DFLAGS += -DPHY_RFEM
	WLFLAGS += -DPHY_RFEM
	ifeq ($(RFEM_DISABLED), 1)
		EXTRA_DFLAGS += -DPHY_RFEM_DISABLED
	endif
else
	EXTRA_DFLAGS += -DPHY_RFEM_DISABLED
endif

ifeq ($(TXFDIQ),1)
	EXTRA_DFLAGS += -DWLC_TXFDIQ
	WLFLAGS += -DWLC_TXFDIQ
endif
# ---------------------------------------------------------------------------------------------- #
# Obsoleting PHY Architecture
#

PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phy_iovar.c
PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phy_ioctl.c

ifneq ($(MINIAP),1)
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_ac.c
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_20698.c
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_20704.c
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_20707.c
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_20708.c
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_20710.c
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_20711.c
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_20712.c
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_20713.c
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_20714.c
	PHY_SRC += $(PHY_TOP_DIR)/old/wlc_phytbl_ac_gains.c
endif

ifeq ($(RFEM),1)
PHY_SRC += $(PHY_TOP_DIR)/ac/rfem/data/src/radio_10700_rev1_regs.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rfem/data/src/radio_10700_rev1_lut.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rfem/data/src/radio_10700_rev1_tuning.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rfem/data/src/radio_10700_rev1_tuning_lut.c

PHY_SRC += $(PHY_TOP_DIR)/ac/rfem/data/src/radio_10700_rev2_regs.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rfem/data/src/radio_10700_rev2_lut.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rfem/data/src/radio_10700_rev2_tuning.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rfem/data/src/radio_10700_rev2_tuning_lut.c

PHY_SRC += $(PHY_TOP_DIR)/ac/rfem/data/src/radio_10700_rev3_regs.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rfem/data/src/radio_10700_rev3_lut.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rfem/data/src/radio_10700_rev3_tuning.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rfem/data/src/radio_10700_rev3_tuning_lut.c

PHY_SRC += $(PHY_TOP_DIR)/ac/rfem/data/src/radio_10700_rev4_regs.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rfem/data/src/radio_10700_rev4_lut.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rfem/data/src/radio_10700_rev4_tuning.c
PHY_SRC += $(PHY_TOP_DIR)/ac/rfem/data/src/radio_10700_rev4_tuning_lut.c
endif

PHY_SRC += $(PHY_TOP_DIR)/ac/radio/phy_ac_phy_ac_cg_mem_ctrl.c

ifeq ($(WL11BE),1)
  PHY_SRC += $(PHY_TOP_DIR)/cmn/ehtcap/phy_ehtcap.c
  PHY_SRC += $(PHY_TOP_DIR)/ac/ehtcap/phy_ac_ehtcap.c
endif
