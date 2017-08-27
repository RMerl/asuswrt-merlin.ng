# Config makefile that maps config based target names to features.
#
# Copyright (C) 2016, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id$

# Variables that map config names to features - 'TARGET_OPTIONS_config_[bus-type]_xxx'.
TARGET_OPTIONS_config_pcie_base	:= pcie-ag-splitrx-fdap-mbss-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-hostpmac-murx-splitassoc-hostmemucode-bgdfs-dyn160
ifeq ($(WLCLMLOAD),1)
	TARGET_OPTIONS_config_pcie_base := $(addsuffix -noclminc-clm_min,$(TARGET_OPTIONS_config_pcie_base))
endif
ifeq ($(__CONFIG_GMAC3__),1)
ifeq ($(__CONFIG_FLATLAS__),1)
	WLCX := "-wlcx"
endif
endif
TARGET_OPTIONS_config_ext_and_int := mfp-wnm-osen-wl11k-wl11u-proptxstatus-11nprop-obss-dbwsw-ringer-dmaindex16-stamon
TARGET_OPTIONS_config_pcie_external := $(TARGET_OPTIONS_config_pcie_base)-$(TARGET_OPTIONS_config_ext_and_int)$(WLCX)
TARGET_OPTIONS_config_pcie_wltest := $(TARGET_OPTIONS_config_pcie_base)-mfgtest-seqcmds-phydbg-phydump-11nprop-dbgam-dbgams-ringer-dmaindex16$(WLCX)
TARGET_OPTIONS_config_pcie_internal := $(TARGET_OPTIONS_config_pcie_base)-$(TARGET_OPTIONS_config_ext_and_int)-txpwr-err-assert-dbgam-dbgams-mfgtest-dump$(WLCX)-acksupr-authrmf-dbgmu-dhdhdr
