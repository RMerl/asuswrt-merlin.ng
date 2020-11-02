# Makefile fragment for phy-specific configuration.
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
# $Id: cfg-phy.mk 595514 2015-10-27 23:31:45Z $
#

PHY_CMN_DIR_LIST := utils wd init
PHY_TYPE_LIST := ac n lcn20
PHY_MOD_LIST := ana antdiv btcx cache calmgr chanmgr core dccal dbg dsi et fcbs hirssi \
                lpc misc mu nap noise papdcal radar radio rssi rxgcrs rxiqcal rxspur \
                samp tbl temp tof tpc txpwrcap tssical txiqlocal vcocal ocl hecap prephy hc vasip stf smc
PHY_MOD_SRC_DIRS := $(strip \
  $(foreach _phy_cmn_dir,$(PHY_CMN_DIR_LIST),cmn/$(_phy_cmn_dir)) \
  $(foreach _phy_type,cmn $(PHY_TYPE_LIST),\
    $(foreach _phy_mod,$(PHY_MOD_LIST),\
      $(if $(wildcard $(WLAN_TreeBaseA)/components/phy/$(_phy_type)/$(_phy_mod)),\
	$(_phy_type)/$(_phy_mod)))))
PHY_MOD_INC_DIRS := cmn/hal
PHY_MOD_INC_DIRS += $(foreach _phy_type,cmn $(PHY_TYPE_LIST),$(_phy_type)/include)
PHY_SRC_INC_DIRS := $(foreach _phy_cmn_dir,$(PHY_CMN_DIR_LIST) $(PHY_MOD_LIST),cmn/$(_phy_cmn_dir))
PHY_SRC_INC_DIRS += $(foreach _phy_type,$(PHY_TYPE_LIST),$(_phy_type)/core)

# required for modules which have separate data files & interface headers co-located in same DIR
PHY_SRC_INC_DIRS += ac/dsi
PHY_SRC_INC_DIRS += ac/papdcal
PHY_SRC_INC_DIRS += ac/et
PHY_SRC_INC_DIRS += ac/samp
PHY_SRC_INC_DIRS += ac/radio

phy_IncDirs := old $(PHY_MOD_INC_DIRS) $(PHY_SRC_INC_DIRS)
phy_SrcDirs := old $(PHY_MOD_SRC_DIRS) $(PHY_SRC_INC_DIRS)
