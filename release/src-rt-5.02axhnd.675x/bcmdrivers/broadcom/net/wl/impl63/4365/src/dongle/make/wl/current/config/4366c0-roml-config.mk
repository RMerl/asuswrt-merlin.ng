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
TARGET_OPTIONS_config_pcie_base	:= pcie-ag-splitrx-fdap-mbss-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-hostpmac-murx-splitassoc-hostmemucode-bgdfs-assoc_lt-ccamesh
TARGET_OPTIONS_config_pcie_base_stb	:= pcie-ag-splitrx-fdap-mbss-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-hostpmac-murx-splitassoc-bgdfs-assoc_lt-ccamesh
TARGET_OPTIONS_config_ext_and_int := mfp-wnm-osen-wl11k-wl11u-proptxstatus-obss-dbwsw-ringer-dmaindex16-stamon-fbt-mbo-map-sae

TARGET_OPTIONS_config_pcie_fdap_mfgtest := $(TARGET_OPTIONS_config_pcie_base)-mfgtest-seqcmds-phydbg-phydump-proptxstatus-dbgam-dbgams-ringer-dmaindex16-murx-fbt-mbo
TARGET_OPTIONS_config_pcie_fdap_mfgtest_stb := $(TARGET_OPTIONS_config_pcie_base_stb)-mfgtest-seqcmds-phydbg-phydump-proptxstatus-dbgam-dbgams-ringer-dmaindex16-murx-fbt-mbo
TARGET_OPTIONS_config_pcie_fdap_internal := $(TARGET_OPTIONS_config_pcie_base)-$(TARGET_OPTIONS_config_ext_and_int)-txpwr-err-assert-dbgam-dbgams-mfgtest-dump-acksupr-authrmf-dbgmu-dhdhdr-htxhdr-amsdufrag-mbo
TARGET_OPTIONS_config_pcie_fdap_internal_stb := $(TARGET_OPTIONS_config_pcie_base_stb)-$(TARGET_OPTIONS_config_ext_and_int)-txpwr-err-assert-dbgam-dbgams-mfgtest-dump-acksupr-authrmf-dbgmu-mbo-stb-hdmaaddr64
TARGET_OPTIONS_config_pcie_fdap_release := $(TARGET_OPTIONS_config_pcie_base)-$(TARGET_OPTIONS_config_ext_and_int)-dhdhdr-htxhdr-amsdufrag
TARGET_OPTIONS_config_pcie_fdap_release_stb := $(TARGET_OPTIONS_config_pcie_base_stb)-$(TARGET_OPTIONS_config_ext_and_int)-stb-hdmaaddr64

TARGET_OPTIONS_config_pcie_internal := $(TARGET_OPTIONS_config_pcie_base)-$(TARGET_OPTIONS_config_ext_and_int)-txpwr-err-assert-dbgam-dbgams-mfgtest-dump-acksupr-authrmf-dbgmu-dhdhdr-htxhdr-amsdufrag-mbo
TARGET_OPTIONS_config_pcie_internal_stb := $(TARGET_OPTIONS_config_pcie_base_stb)-$(TARGET_OPTIONS_config_ext_and_int)-txpwr-err-assert-dbgam-dbgams-mfgtest-dump-acksupr-authrmf-dbgmu-mbo-stb-hdmaaddr64
