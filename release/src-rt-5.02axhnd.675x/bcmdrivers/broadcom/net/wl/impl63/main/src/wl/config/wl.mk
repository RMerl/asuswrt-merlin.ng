# Helper makefile for building Broadcom wl device driver
# This file maps wl driver feature flags (import) to WLFLAGS and WLFILES_SRC (export).
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
# $Id: wl.mk 789167 2020-07-21 17:34:58Z $

WLFLAGS += -DBCM943217ROUTER_ACI_SCANMORECH
WLFLAGS += -DBPHY_DESENSE
WLFLAGS += -DWL_EXPORT_CURPOWER

ifdef UCODE_RAM
UCODE_RAM_DIR = components/ucode/dot11_releases/trunk/$(UCODE_RAM)/ram
else
UCODE_RAM_DIR = src/wl/sys
endif

ifeq ($(UCODE_IN_ROM),1)
UCODE_ROM_DIR = components/ucode/dot11_releases/trunk/$(UCODE_ROM)/rom
endif

ifeq ($(NO_BCMDBG_ASSERT), 1)
	WLFLAGS += -DNO_BCMDBG_ASSERT
endif

# debug/internal
ifeq ($(DEBUG),1)
	ifeq ($(CMWIFI),)
		WLFLAGS += -DBCMDBG -DWLTEST -DRWL_WIFI -DWLRWL -DWL_EXPORT_CURPOWER
		WLRWL = 1
	else
		WLFLAGS += -DBCMDBG -DWLTEST -DWL_EXPORT_CURPOWER
	endif
	WIFI_ACT_FRAME := 1
else ifeq ($(WLDEBUG),1)
	BCMUTILS = 1
	OSLLX = 1
	WIFI_ACT_FRAME := 1
	WLFLAGS += -DBCMDBG -DWLTEST -DWL_EXPORT_CURPOWER
else
#ifdef WLTEST
	# mfgtest builds
	ifeq ($(WLTEST),1)
		WLFLAGS += -DWLTEST -DWL_EXPORT_CURPOWER
		# Disable parallel scan for MFGTEST
		WLFLAGS += -DRSDB_PARALLEL_SCAN_DISABLED
	endif
#endif // endif
endif

#ifdef BCMDBG_TXSTALL
ifeq ($(BCMDBG_TXSTALL),1)
	WLFLAGS += -DBCMDBG_TXSTALL
endif
#endif // endif

#ifdef BCMWDF
ifeq ($(BCMWDF),1)
	WLFLAGS += -DBCMWDF
endif
#endif // endif

#ifdef BCMDBG
ifeq ($(BCMDBG),1)
	WLFLAGS += -DWL_EXPORT_CURPOWER
endif
#endif // endif

ifeq ($(WLAIRDBG),1)
	WLFLAGS += -DWLAIRDBG
	WLFILES_SRC += src/wl/sys/wlc_airdbg.c
endif

# hotspot AP
ifeq ($(HSPOT),1)
	WLBSSLOAD = 1
	L2_FILTER = 1
	WLDLS = 1
	WLWNM = 1
	WIFI_ACT_FRAME = 1
	WL11U = 1
	WLPROBRESP_SW = 1
	WLOSEN = 1
endif

WLFLAGS += -DPPR_API

ifdef BCMPHYCORENUM
	WLFLAGS += -DBCMPHYCORENUM=$(BCMPHYCORENUM)
endif

ifeq ($(WLATF),1)
	WLATF := 1
	WLFLAGS += -DWLATF
endif

ifeq ($(WLATF_DONGLE),1)
	WLATF := 1
	WLFLAGS += -DWLATF_DONGLE
endif

ifeq ($(WL_AP_CHAN_CHANGE_EVENT),1)
	WLFLAGS += -DWL_AP_CHAN_CHANGE_EVENT
endif

ifeq ($(BCM_CEVENT),1)
	WLFLAGS += -DBCM_CEVENT
endif

ifeq ($(WL_SPLIT_ASSOC),1)
	WLFLAGS += -DSPLIT_ASSOC
endif

#ifdef BCM_DMA_CT
ifeq ($(BCM_DMA_CT),1)
	WLFLAGS += -DBCM_DMA_CT
endif
#endif // endif

#ifdef BCM_DMA_INDIRECT
ifeq ($(BCM_DMA_INDIRECT),1)
	WLFLAGS += -DBCM_DMA_INDIRECT
endif
#endif // endif

ifeq ($(DBG_HEAPCHECK),1)
	WLFLAGS += -DBCMDBG_HEAPCHECK
endif

ifeq ($(WLATF_PERC),1)
	WLFLAGS += -DWLATF_PERC
endif

ifeq ($(WLWRR),1)
	WLFLAGS += -DWLWRR
endif

#ifdef BCMSPLITRX
ifeq ($(BCMSPLITRX),1)
	WLFILES_SRC	+= src/wl/sys/wlc_pktfetch.c
endif
#endif // endif

#ifdef BCMDBG_TXSTUCK
ifeq ($(BCMDBG_TXSTUCK),1)
	# Debug for: wl dump txstuck;
        WLFLAGS += -DBCMDBG_TXSTUCK
endif
#endif // endif

#ifdef CHAN_SWITCH_HIST
ifeq ($(CHAN_SWITCH_HIST),1)
	WLFLAGS += -DCHAN_SWITCH_HIST
endif
#endif // endif

#ifdef PKTQ_LOG
ifeq ($(PKTQ_LOG),1)
	WLFLAGS += -DPKTQ_LOG
	WLFILES_SRC += src/wl/sys/wlc_rx_report.c
	ifeq ($(SCB_BS_DATA),1)
		WLFILES_SRC += src/wl/sys/wlc_bs_data.c
		WLFLAGS += -DSCB_BS_DATA
	endif
endif
#endif // endif

#ifdef PKTQ_STATUS
ifeq ($(PKTQ_STATUS),1)
	WLFLAGS += -DPKTQ_STATUS
endif
#endif // endif

ifeq ($(PSPRETEND),1)
	WLFLAGS += -DPSPRETEND
	WLFLAGS += -DWL_CS_PKTRETRY
	WLFLAGS += -DWL_CS_RESTRICT_RELEASE
	WLFILES_SRC += src/wl/sys/wlc_pspretend.c
	WLFILES_SRC += src/wl/sys/wlc_csrestrict.c
endif

#ifdef BCMDBG_TRAP
# CPU debug traps (lomem access, divide by 0, etc) are enabled except when mogrified out for
# external releases.
WLFLAGS += -DBCMDBG_TRAP
#endif // endif

## wl driver common

ifeq ($(GTK_RESET),1)
	WLFLAGS += -DGTK_RESET
endif

ifeq ($(WL_NETX),1)
	WLFLAGS += -DNETX
endif

ifeq ($(PROPRIETARY_11N_RATES),1)
	WLFLAGS += -DWLPROPRIETARY_11N_RATES
endif

ifneq ($(BRCM_CHIP),47189)
# XXX RB:157423 RB:157507
WLRSDB := 0
endif

ifeq ($(WLRSDB),1)
	WLFLAGS += -DWLRSDB -DSHARED_OSL_CMN
	WL_OBJ_REGISTRY := 1
	ifeq ($(WL_RSDB_DISABLED),1)
		WLFLAGS += -DWLRSDB_DISABLED -DNUMD11CORES=1 -DRSDB_MODE_MIMO
	endif
	# RSDB Needs multiqueue for txfifo sync
	ifndef WLMULTIQUEUE
		WLMULTIQUEUE := 1
	endif
	ifeq ($(RSDB_POLICY_MGR),1)
		WLFLAGS += -DWLRSDB_POLICY_MGR
		WLFILES_SRC += src/wl/sys/wlc_rsdb_policymgr.c
	endif
	ifeq ($(RSDB_POLICY_MGR_DISABLED),1)
		WLFLAGS += -DWLRSDB_POLICY_MGR_DISABLED
	endif
	ifeq ($(RSDB_CMN_BANDSTATE),1)
		WLFLAGS += -DRSDB_CMN_BANDSTATE
	endif
	ifeq ($(RSDB_CMN_BANDSTATE_DISABLED),1)
		WLFLAGS += -DRSDB_CMN_BANDSTATE_DISABLED
	endif
	ifeq ($(BCMDBG_RSDB),1)
		WLFLAGS += -DBCMDBG_RSDB
		WLFLAGS += -DBCM_DNGDMP
	endif
	ifeq ($(RSDB_APSCAN),1)
		WLFLAGS += -DRSDB_APSCAN
	endif
	ifeq ($(RSDB_PM_MODESW),1)
		EXTRA_DFLAGS += -DRSDB_PM_MODESW
	endif
endif

ifeq ($(WL_OBJ_REGISTRY),1)
	WLFLAGS += -DWL_OBJ_REGISTRY
	ifeq ($(WL_NO_BSSCFG_SHARE),1)
		WLFLAGS += -DWL_NO_BSSCFG_SHARE
	endif
endif

#ifdef BCMQT
ifeq ($(BCMQT),1)
	# Set flag to indicate emulated chip
	WLFLAGS += -DBCMSLTGT -DBCMQT
	ifeq ($(WLRTE),1)
		# Use of RTE implies embedded (CPU emulated)
		WLFLAGS += -DBCMQT_CPU
	endif
	# Disable Radar function in emulated platform
	WLC_DISABLE_DFS_RADAR_SUPPORT = 1
endif
#endif // endif

#ifdef WL
ifeq ($(WL),1)
	WLFILES_SRC += src/shared/bcmwifi/src/bcmwifi_channels.c
	WLFILES_SRC += src/shared/bcmwifi/src/bcmwifi_rclass.c
	WLFILES_SRC += src/shared/bcmwifi/src/bcmwifi_rspec.c
	WLFILES_SRC += src/shared/bcmwifi/src/bcmwifi_rates.c
	WLFILES_SRC += src/shared/bcmevent.c
	WLFILES_SRC += src/shared/bcm_mpool.c
	WLFILES_SRC += src/shared/bcm_notif.c
	WLFILES_SRC += src/shared/bcm_objregistry.c
	WLFILES_SRC += src/shared/bcmiov.c
	WLFILES_SRC += src/wl/sys/wlc_alloc.c
	WLFILES_SRC += src/wl/sys/wlc_intr.c
	WLFILES_SRC += src/wl/sys/wlc_hw.c
	WLFILES_SRC += $(D11SHMCDIR)/d11shm.c
	WLFILES_SRC += $(D11SHMCDIR)/d11regsoffs.c
	ifeq ($(WLPFN),1)
		WLFLAGS += -DWLPFN
		WLFILES_SRC += src/wl/sys/wl_pfn.c
	endif
	WLFILES_SRC += src/shared/qmath.c
	# In router NIC mode, math functions are compiled into and
	# exported by hnd, so wl doesn't need them. Not the case in
	# dongle builds and wl_hnd (standalone NIC) builds.
	# So dongle and standalone NIC define BCMMATH explicitly.
	ifeq ($(BCMMATH), 1)
		WLFILES_SRC += components/math/src/bcm_math.c
	endif
	WLFILES_SRC += $(UCODE_RAM_DIR)/d11ucode_gt15.c
	ifeq ($(WL_EAP_UCODE),1)
		WLFILES_SRC += $(UCODE_RAM_DIR)/d11ucode_ge24_eap.c
	else
		WLFILES_SRC += $(UCODE_RAM_DIR)/d11ucode_ge24.c
	endif
	WLFILES_SRC += src/wl/sys/d11reglist.c
	WLFILES_SRC += src/wl/ppr/src/wlc_ppr.c
	WLFILES_SRC += src/wl/sys/wlc_phy_shim.c
	WLFILES_SRC += src/wl/sys/wlc_bmac.c
	WLFILES_SRC += src/wl/sys/wlc_ucinit.c
	WLFILES_SRC += src/wl/sys/wlc_rate_def.c
	WLFILES_SRC += src/wl/sys/wlc_vasip.c
	WLFILES_SRC += src/wl/sys/wlc_smc.c
	WLFILES_SRC += src/wl/sys/d11smc_code.c
	WLFILES_SRC += src/wl/sys/wlc_debug_crash.c

	ifeq ($(AWD_EXT_TRAP),1)
		WLFILES_SRC += components/awd/src/awd_ext_trap.c
	endif

#ifdef BCM_DMA_CT
	ifeq ($(BCM_DMA_CT),1)
		ifeq ($(WL_EAP_UCODE),1)
			WLFILES_SRC += $(UCODE_RAM_DIR)/d11ucode_mu_eap.c
			WLFILES_SRC += $(UCODE_RAM_DIR)/d11ucode_mu_eap_ftm.c
		else
			WLFILES_SRC += $(UCODE_RAM_DIR)/d11ucode_mu.c
		endif
	endif
#endif // endif

	ifneq ($(MINIAP),1)
		ifeq ($(WL_EAP_UCODE),1)
			WLFILES_SRC += $(UCODE_RAM_DIR)/d11ucode_ge40_eap.c
		else
			WLFILES_SRC += $(UCODE_RAM_DIR)/d11ucode_ge40.c
		endif
		WLFILES_SRC += src/wl/sys/d11vasip_code.c
	endif
	ifeq ($(WLDIAG),1)
		WLFILES_SRC += $(UCODE_RAM_DIR)/d11ucode_diag.c
	endif
	ifeq ($(WLCX_ATLAS),1)
		WLFILES_SRC += src/wl/sys/d11ucode_wlcx.c
		WLFLAGS += -DWLCX_ATLAS
	endif
	ifeq ($(WL_EAP_UCODE),1)
		WLFILES_SRC += $(UCODE_RAM_DIR)/d11ucode_btcx_eap.c
	else
		WLFILES_SRC += src/wl/sys/d11ucode_btcxmu.c
	endif
	WLFILES_SRC += src/wl/sys/wlc.c
	ifeq ($(BCM_ECOUNTERS),1)
		WLFILES_SRC += src/wl/sys/wlc_event_ecounters.c
	endif
	WLFILES_SRC += src/wl/sys/wlc_tx.c
	WLFILES_SRC += src/wl/sys/wlc_txmod.c
	WLFILES_SRC += src/wl/sys/wlc_rx.c
	WLFILES_SRC += src/wl/sys/wlc_frag.c
	WLFILES_SRC += src/wl/sys/wlc_cubby.c
	WLFILES_SRC += src/wl/sys/wlc_qoscfg.c
	WLFILES_SRC += src/wl/sys/wlc_flow_ctx.c
	ifeq ($(ATE),1)
		WLFILES_SRC += src/wl/ate/src/wlu_ate.c
		WLFILES_SRC += src/shared/miniopt.c
		WLFILES_SRC += src/wl/sys/wl_ate.c
	endif
	WLFILES_SRC += src/wl/sys/wlc_dbg.c
	WLFILES_SRC += src/wl/sys/wlc_dump.c
	WLFILES_SRC += src/wl/sys/wlc_macdbg.c
	WLFILES_SRC += src/wl/sys/wlc_macreq.c
	WLFILES_SRC += src/wl/sys/wlc_txtime.c
	WLFILES_SRC += src/wl/sys/wlc_ie_misc_hndlrs.c
	WLFILES_SRC += src/wl/sys/wlc_addrmatch.c
	WLFILES_SRC += src/wl/sys/wlc_utils.c
	WLFILES_SRC += src/wl/sys/wlc_prot.c
	WLFILES_SRC += src/wl/sys/wlc_prot_g.c
	WLFILES_SRC += src/wl/sys/wlc_prot_n.c
	WLFILES_SRC += src/wl/sys/wlc_assoc.c
	WLFILES_SRC += src/wl/sys/wlc_txc.c
	WLFILES_SRC += src/wl/sys/wlc_pcb.c
	WLFILES_SRC += src/wl/sys/wlc_rate.c
	WLFILES_SRC += src/wl/sys/wlc_rate_def.c
	WLFILES_SRC += src/wl/sys/wlc_stf.c
	WLFILES_SRC += src/wl/sys/wlc_lq.c
	WLFILES_SRC += src/wl/sys/wlc_log.c
	WLFILES_SRC += src/wl/sys/wlc_txcfg.c
	WLFILES_SRC += src/wl/sys/wlc_fifo.c
	ifeq ($(SRHWVSDB),1)
		WLFILES_SRC += src/wl/sys/wlc_srvsdb.c
	endif
	ifeq ($(WL_PROT_OBSS),1)
		WLFLAGS += -DWL_PROT_OBSS
		WLFILES_SRC += src/wl/sys/wlc_obss_util.c
		WLFILES_SRC += src/wl/sys/wlc_prot_obss.c
	endif

	ifeq ($(WL_OBSS_DYNBW),1)
		WL_MODESW = 1
		WLFLAGS += -DWL_OBSS_DYNBW
		WLFILES_SRC += src/wl/sys/wlc_obss_util.c
		WLFILES_SRC += src/wl/sys/wlc_obss_dynbw.c
	endif
	ifneq ($(WL_AIRIQ),)
		WL_MODESW = 1
		WLFLAGS += -DWL_AIR_IQ
		WLFLAGS += -DWL_EAP_VASIP
		WLFILES_SRC += src/wl/airiq/src/wlc_airiq.c
		WLFILES_SRC += src/wl/airiq/src/wlc_airiq_3p1.c
		WLFILES_SRC += src/wl/airiq/src/wlc_airiq_capture.c
		WLFILES_SRC += src/wl/airiq/src/wlc_airiq_iov.c
		WLFILES_SRC += src/wl/airiq/src/wlc_airiq_phy.c
		WLFILES_SRC += src/wl/airiq/src/wlc_airiq_scan.c
	endif
	ifeq ($(WL_BCNTRIM),1)
		WLFLAGS += -DWL_BCNTRIM
		WLFILES_SRC += src/wl/sys/wlc_bcntrim.c
	endif
	WLFILES_SRC += src/wl/sys/wlc_pm.c
	WLFILES_SRC += src/wl/sys/wlc_btcx.c
	WLFILES_SRC += src/wl/sys/wlc_stamon.c
	WLFILES_SRC += src/wl/sys/wlc_monitor.c
ifeq ($(WLNDIS),1)
	WLFILES_SRC += src/wl/sys/wlc_ndis_iovar.c
endif
	WLFILES_SRC += src/wl/sys/wlc_scb.c
	WLFILES_SRC += src/wl/sys/wlc_rate_sel.c
	WLFILES_SRC += src/wl/sys/wlc_scb_ratesel.c
	WLFILES_SRC += src/wl/sys/wlc_macfltr.c
	ifeq ($(WL_PROXDETECT),1)
		WLFLAGS += -DWL_PROXDETECT
		WLFLAGS += -DWL_PROXD_UCODE_TSYNC
		WLFLAGS += -DWL_PROXD_GDCOMP
		WLFLAGS += -DWL_PROXD_OUTLIER_FILTERING
		ifeq ($(WLDEBUG),1)
			WLFLAGS += -DTOF_DEBUG -DTOF_DEBUG_TIME
		endif
		ifeq ($(TOF_DBG),1)
			WLFLAGS += -DTOF_DBG
		endif
		WLFILES_PROXD = wlc_pdsvc.c wlc_tof.c wlc_fft.c
		ifeq ($(WL_FTM), 1)
			WLFLAGS += -DWL_FTM -DWL_FTM_MSCH
			ifeq ($(WL_FTM_11K),1)
				WLFLAGS += -DWL_FTM_11K
			endif
			ifeq ($(WL_FTM_TSF_SYNC),1)
				WLFLAGS += -DWL_FTM_TSF_SYNC
			endif
			WLFILES_FTM_BCMCRYPTO = sha2.c sha2x.c
			WLFILES_PROXD += pdftm.c pdftmevt.c pdftmiov.c pdftmproto.c \
				pdftmsn.c pdftmsched.c pdburst.c pdftmutil.c pdftmdbg.c \
				pdftmvs.c pdftmrange.c pdftmmsch.c \
				pdftmsec.c
		endif
		WLFILES_SRC += $(addprefix src/wl/proxd/src/, $(WLFILES_PROXD))
		WLFILES_SRC += $(addprefix components/bcmcrypto/src/, $(WLFILES_FTM_BCMCRYPTO))
	endif
	ifeq ($(WL_NAN),1)
		WL_NAN_AVAIL ?= 1
		WL_NAN_DATA ?= 1
		WL_NAN_DISC ?= 1
		WL_NAN_SEC ?= 1
		ifeq ($(WL_NAN_SEC),1)
			WLWSEC := 1
			MFP := 1
			BCMCCMP := 1
		endif
		# nan requires scancache
		WLSCANCACHE := 1
		WLSLOTTED_BSS := 1
		WLFLAGS += -DWL_NAN
		WLFILES_SRC += src/shared/bcmbloom.c
		WLFILES_SRC += components/nan/src/nan_cmn.c
		WLFILES_SRC += components/nan/src/nan_iov.c
		WLFILES_SRC += components/nan/src/nan_mac.c
		WLFILES_SRC += components/nan/src/nan_sched.c
		WLFILES_SRC += components/nan/src/nan_util.c
		#wfa test bed mode
		WLFLAGS += -DWL_NAN_WFA_TB_MODE -DNAN_PF
		WLFLAGS += -DWL_SLOTTED_BSS_SHARING
		WLFLAGS += -DWL_NAN_MULTI_NDI
		WLFLAGS += -DWL_NAN_RSDB_MULTI_PEER
		WLFLAGS += -DWL_NAN_BCMC_IN_DW

		ifeq ($(WL_NAN_AVAIL), 1)
			WLFILES_SRC += components/nan/src/nan_peer.c
			WLFILES_SRC += components/nan/src/nan_avail.c
			WLFILES_SRC += components/nan/src/nan_parse.c
			WLFILES_SRC += components/nan/src/nan_dam.c
			WLFLAGS += -DWL_NAN_AVAIL
			ifeq ($(WL_NAN_DISC), 1)
				WLFILES_SRC += components/nan/src/nan_disc.c
				WLFLAGS += -DWL_NAN_DISC
			endif
			ifeq ($(WL_NAN_DATA), 1)
				WLFILES_SRC += components/nan/src/nan_data.c
				WLFILES_SRC += components/nan/src/nan_ndp.c
				WLFILES_SRC += components/nan/src/nan_ndl.c
				WLFILES_SRC += components/nan/src/nan_dpe.c
				WLFILES_SRC += components/nan/src/nan_fsm.c
				WLFLAGS += -DWL_NAN_DATA
				ifeq ($(WL_NAN_SEC), 1)
					WLFILES_SRC += components/bcmcrypto/src/rijndael-alg-fst.c
					WLFILES_SRC += components/bcmcrypto/src/aeskeywrap.c
					WLFILES_SRC += components/bcmcrypto/src/aes.c
					WLFILES_SRC += components/bcmcrypto/src/md5.c
					WLFILES_SRC += components/bcmcrypto/src/sha2.c
					WLFILES_SRC += components/bcmcrypto/src/sha2x.c
					WLFILES_SRC += components/nan/src/nan_sec.c
					WLFLAGS += -DWL_NAN_SEC
				endif
			endif
			ifeq ($(WL_NAN_RANGE), 1)
				WLFILES_SRC += components/nan/src/nan_re.c
				WLFILES_SRC += components/nan/src/nan_range.c
				WLFILES_SRC += components/nan/src/nan_ftm.c
				WLFILES_SRC += components/nan/src/nan_fsm.c
				WLFLAGS += -DWL_NAN_RANGE
			endif
		endif
		ifeq ($(DBG_NAN), 1)
			WLFILES_SRC += components/nan/src/nan_dbg.c
			WLFLAGS += -DDBG_NAN
		endif
	endif
	ifneq ($(WLWSEC),0)
		WLFLAGS += -DWLWSEC
	endif
ifeq ($(WL_RANDMAC),1)
	WLFLAGS += -DWL_RANDMAC
	WLFILES_SRC += src/wl/randmac/src/wlc_randmac.c
endif
#ifdef WL_LPC
	ifeq ($(WL_LPC),1)
		WLFLAGS += -DWL_LPC
		WLFILES_SRC += src/wl/sys/wlc_power_sel.c
		WLFILES_SRC += src/wl/sys/wlc_scb_powersel.c
	else
		ifeq ($(LP_P2P_SOFTAP),1)
			WLFLAGS += -DLP_P2P_SOFTAP
		endif
	endif
#ifdef WL_LPC_DEBUG
	ifeq ($(WL_LPC_DEBUG),1)
		WLFLAGS += -DWL_LPC_DEBUG
	endif
#endif // endif
#endif // endif
	ifeq ($(WL_RELMCAST),1)
		WLFLAGS += -DWL_RELMCAST -DIBSS_RMC
		WLFILES_SRC += src/wl/rel_mcast/src/wlc_relmcast.c
	endif
	WLFILES_SRC += src/wl/sys/wlc_antsel.c
	WLFILES_SRC += src/wl/sys/wlc_bsscfg.c
	WLFILES_SRC += src/wl/sys/wlc_bsscfg_psq.c
	WLFILES_SRC += src/wl/sys/wlc_bsscfg_viel.c
	WLFILES_SRC += src/wl/sys/wlc_vndr_ie_list.c
	WLFILES_SRC += src/wl/sys/wlc_scan.c
	WLFILES_SRC += src/wl/sys/wlc_scan_utils.c
	WLFILES_SRC += components/msch/src/wlc_msch.c
	WLFILES_SRC += components/msch/src/wlc_mschutil.c
	WLFILES_SRC += components/msch/src/wlc_msch_profiler.c
	WLFILES_SRC += src/wl/sys/wlc_chanctxt.c
	WLFILES_SRC += src/wl/sys/wlc_rm.c
	WLFILES_SRC += src/wl/sys/wlc_tso.c
	WLFILES_SRC += src/wl/sys/wlc_pmkid.c
	WLFILES_SRC += src/wl/sys/wlc_pktc.c
	WLFILES_SRC += src/wl/sys/wlc_rspec.c
	WLFILES_SRC += src/wl/sys/wlc_perf_utils.c
	WLFILES_SRC += src/wl/sys/wlc_act_frame.c
	WLFILES_SRC += src/wl/sys/wlc_chsw_timecal.c
	WLFILES_SRC += src/wl/sys/wlc_pmq.c
	WLFILES_SRC += src/wl/sys/wlc_ht.c
	ifeq ($(WL11N_SINGLESTREAM),1)
		WLFLAGS += -DWL11N_SINGLESTREAM
	endif
	ifeq ($(WL11AC),1)
		WLFLAGS += -DWL11AC
		WLFILES_SRC += src/wl/sys/wlc_vht.c
		ifeq ($(WL11AC_160),1)
			WLFLAGS += -DWL11AC_160
		endif
		ifeq ($(WL11AC_80P80),1)
			WLFLAGS += -DWL11AC_80P80
		endif
		WL_MODESW ?= 1
		ifeq ($(WLTXBF),1)
			WLFLAGS += -DWL_BEAMFORMING
			WLFILES_SRC += src/wl/sys/wlc_txbf.c
			ifeq ($(TXBF_MORE_LINKS),1)
				WLFLAGS += -DTXBF_MORE_LINKS
			endif
		endif
		ifeq ($(WLOLPC),1)
			WLFLAGS += -DWLOLPC
			WLFILES_SRC += src/wl/olpc/src/wlc_olpc_engine.c
		endif
	endif
	ifeq ($(DYN160),1)
		WLFLAGS += -DDYN160
	endif
	ifeq ($(WL11AX),1)
		WLFLAGS += -DWL11AX
		WLTWT ?= 1
		WLHEB ?= 1
		WL_MUSCHEDULER ?= 1
		WL_ULMU ?= 1
		WLFILES_SRC += src/wl/sys/wlc_he.c
#		# FIXME: For testing before DL OFDMA auto indexing/grouping implement.
		WLFLAGS += -DWLPKTENG
	endif
	ifeq ($(WLTWT),1)
		WLFLAGS += -DWLTWT
		WLFILES_SRC += src/wl/sys/wlc_twt.c
	endif
	ifeq ($(TESTBED_AP_11AX),1)
		WLFLAGS += -DTESTBED_AP_11AX
	endif
	ifeq ($(WL_MODESW),1)
		WLFLAGS += -DWL_MODESW
		WLFILES_SRC += src/wl/sys/wlc_modesw.c
		WLFLAGS += -DPHYCAL_CACHING
	endif
	ifeq ($(WL_MU_TX),1)
		WLFLAGS += -DWL_MU_TX
		WLFILES_SRC += src/wl/sys/wlc_mutx.c
		WL_MUSCHEDULER ?= 1
	endif
	ifeq ($(WL_MU_RX),1)
		WLFLAGS += -DWL_MU_RX
		WLFILES_SRC += src/wl/sys/wlc_murx.c
	endif
	WLFILES_SRC += src/wl/sys/wlc_airtime.c
	ifeq ($(WL_MUSCHEDULER),1)
		WLFLAGS += -DWL_MUSCHEDULER
		WLFILES_SRC += src/wl/sys/wlc_musched.c
	endif
	ifeq ($(WL_ULMU),1)
		WLFLAGS += -DWL_ULMU
		WLFILES_SRC += src/wl/sys/wlc_ulmu.c
	endif
	ifeq ($(WL_PSMX),1)
		WLFLAGS += -DWL_PSMX
	endif
	ifeq ($(WL_PSMR1),1)
		WLFLAGS += -DWL_PSMR1
	endif
	ifeq ($(WL_AUXPMQ),1)
		WLFLAGS += -DWL_AUXPMQ
	endif

#ifdef WL11H
	ifeq ($(WL11H),1)
		WLFLAGS += -DWL11H
		WLFILES_SRC += src/wl/sys/wlc_11h.c
		WLFLAGS += -DWLCSA
		WLFILES_SRC += src/wl/sys/wlc_csa.c
		WLFLAGS += -DWLQUIET
		WLFILES_SRC += src/wl/sys/wlc_quiet.c
	endif
#endif /* WL11H */

#ifdef WL_PM_MUTE_TX
	ifeq ($(WL_PM_MUTE_TX),1)
		WLFLAGS += -DWL_PM_MUTE_TX
		WLFILES_SRC += src/wl/sys/wlc_pm_mute_tx.c
	endif
#endif /* WL_PM_MUTE_TX */

	# tpc module is shared by 11h tpc and wl tx power control */
	WLTPC ?= 1
	ifeq ($(WLTPC),1)
		WLFLAGS += -DWLTPC
		WLFILES_SRC += src/wl/sys/wlc_tpc.c
#ifdef WL_AP_TPC
		ifeq ($(WL_AP_TPC),1)
			WLFLAGS += -DWL_AP_TPC
		endif
#endif // endif
		ifeq ($(WL_CHANSPEC_TXPWR_MAX),1)
			WLFLAGS += -DWL_CHANSPEC_TXPWR_MAX
		endif
	endif
	ifeq ($(WLC_DISABLE_DFS_RADAR_SUPPORT),1)
		WLFLAGS += -DWLC_DISABLE_DFS_RADAR_SUPPORT
	else
		ifeq ($(BAND5G),1)
			WLFILES_SRC += src/wl/sys/wlc_dfs.c
			ifeq ($(BGDFS),1)
				WLFLAGS += -DBGDFS
			endif
			ifeq ($(BAND2G),1)
				ifeq ($(BGDFS_2G),1)
					WLFLAGS += -DBGDFS_2G
				endif
			endif
		endif
	endif
	ifeq ($(DFS_TEST_MODE),1)
		WLFLAGS += -DWL_DFS_TEST_MODE
	endif
	ifeq ($(WL_SCAN_DFS_HOME),1)
		WLFLAGS += -DWL_SCAN_DFS_HOME
	endif
#ifdef WL11D
	ifeq ($(WL11D),1)
		WLFLAGS += -DWL11D
		WLFILES_SRC += src/wl/sys/wlc_11d.c
	endif
#endif /* WL11D */
	# cntry module is shared by 11h/11d and wl channel */
	WLCNTRY := 1
	ifeq ($(WLCNTRY),1)
		WLFLAGS += -DWLCNTRY
		WLFILES_SRC += src/wl/sys/wlc_cntry.c
	endif
	WLFILES_SRC += src/wl/sys/wlc_event.c
	WLFILES_SRC += src/wl/sys/wlc_event_utils.c
	WLFILES_SRC += src/wl/sys/wlc_msfblob.c
	WLFILES_SRC += src/wl/sys/wlc_channel.c
#ifdef WL_SARLIMIT
	ifeq ($(WL_SARLIMIT),1)
		# Note that wlc_sar_tbl.c is dynamically generated. Its whereabouts
		# shall be assigned to WLC_SAR_TBL_DIR, its generation recipe shall
		# established by WLAN_GenSarTblRule macro, defined in WLAN_Common.mk
		WLFILES_SRC += $(WLC_SAR_TBL_DIR)/wlc_sar_tbl.c
	endif
#endif // endif

	# If wlc_clm_data.xml is not there and we have generated wlc_clm_data_xxxx.c
	# files, use the generated file for compilation.
	WLCLM_SRC   = components/clm-api/src/wlc_clm_data.c
ifeq (,$(wildcard $(SRCBASE)/../components/clm-private/wlc_clm_data.xml))
ifneq (,$(wildcard $(SRCBASE)/../components/clm-api/src/wlc_clm_data$(CLM_FILE_SUFFIX).c))
	WLCLM_SRC   = components/clm-api/src/wlc_clm_data$(CLM_FILE_SUFFIX).c
endif
endif

	WLFILES_SRC += components/clm-api/src/wlc_clm.c
	WLFILES_SRC += $(WLCLM_SRC)
#ifdef WLC_TXCAL
	ifeq ($(WLC_TXCAL),1)
		WLFLAGS += -DWLC_TXCAL
		WLFILES_SRC += src/wl/sys/wlc_calload.c
	endif
#endif // endif
	WLFILES_SRC += src/shared/bcmwpa.c
#ifndef LINUX_CRYPTO
	ifneq ($(LINUX_CRYPTO),1)
		WLFILES_SRC += components/bcmcrypto/src/rc4.c
		WLFILES_SRC += components/bcmcrypto/src/tkhash.c
		WLFILES_SRC += components/bcmcrypto/src/tkmic.c
		WLFILES_SRC += components/bcmcrypto/src/wep.c
	endif
#endif /* LINUX_CRYPTO */
#ifdef WLSCANCACHE
ifeq ($(WLSCANCACHE),1)
	WLFLAGS += -DWLSCANCACHE
	WLFILES_SRC += src/wl/sys/wlc_scandb.c
endif
#endif // endif
	WLFILES_SRC += src/wl/sys/wlc_hrt.c
	WLFILES_SRC += src/wl/sys/wlc_ie_helper.c
	WLFILES_SRC += src/wl/sys/wlc_ie_mgmt_lib.c
	WLFILES_SRC += src/wl/sys/wlc_ie_mgmt_vs.c
	WLFILES_SRC += src/wl/sys/wlc_ie_mgmt.c
	WLFILES_SRC += src/wl/sys/wlc_ie_reg.c
ifeq ($(IEM_TEST),1)
	WLFLAGS += -DIEM_TEST
	WLFILES_SRC += src/wl/sys/wlc_ie_mgmt_test.c
endif
	WLFILES_SRC += src/wl/sys/wlc_akm_ie.c
	WLFILES_SRC += src/wl/sys/wlc_obss.c
	WLFILES_SRC += src/wl/sys/wlc_hs20.c
	WLFILES_SRC += src/wl/sys/wlc_iocv.c
	WLFILES_SRC += src/wl/sys/wlc_chctx.c
	#iovar/ioctl registration
	WLFILES_SRC += src/wl/iocv/src/wlc_iocv_reg.c
	WLFILES_SRC += src/wl/iocv/src/wlc_iocv_low.c
	WLFILES_SRC += src/wl/iocv/src/wlc_iocv_high.c
	WLFILES_SRC += src/wl/iocv/src/wlc_iocv_cmd.c
	#named dump callback registration
	WLFILES_SRC += src/wl/dump/src/wlc_dump_reg.c
	#channel context registration
	WLFILES_SRC += src/wl/chctx/src/wlc_chctx_reg.c
	#BMAC iovar/ioctl
	WLFILES_SRC += src/wl/sys/wlc_bmac_iocv.c
	WLFILES_SRC += src/wl/sys/wlc_misc.c
	WLFILES_SRC += src/wl/sys/wlc_txs.c
	WLFILES_SRC += src/wl/sys/wlc_test.c
ifeq ($(WLRSDB),1)
	WLFILES_SRC += src/wl/sys/wlc_rsdb.c
	WLFILES_SRC += src/shared/si_d11rsdb_utils.c
	WLFILES_SRC += src/shared/ai_d11rsdb_utils.c
endif
ifeq ($(WL_STF_ARBITRATOR),1)
	 WLFILES_SRC += src/wl/sys/wlc_stf_arb.c
endif
ifeq ($(WL_MIMOPS)$(WL_STF_ARBITRATOR),11)
	 WLFILES_SRC += src/wl/sys/wlc_stf_mimops.c
endif
ifeq ($(WL_OPS),1)
	WLFLAGS += -DWL_OPS
	WLFILES_SRC += src/wl/sys/wlc_ops.c
endif
ifeq ($(WL_OBJ_REGISTRY),1)
	WLFILES_SRC += src/wl/sys/wlc_objregistry.c
endif
WLFLAGS += -DWL_BSS_INFO_TYPEDEF_HAS_ALIAS
WLFLAGS += -DRATESET_VERSION_ENABLED
endif
#endif /* WL */

## wl OSL

#ifdef WLLX
ifeq ($(WLLX),1)
        WLFILES_SRC += src/wl/sys/wl_linux.c
	WLFILES_SRC += src/wl/sys/wl_core.c
	WLFILES_SRC += src/wl/sys/wldev_common.c
endif
#endif // endif

#ifdef WLLXIW
ifeq ($(WLLXIW),1)
	WLFILES_SRC += src/wl/sys/wl_iw.c
	WLFLAGS += -DWLLXIW
endif
#endif // endif

ifeq ($(WL_AP_CFG80211),1)
	WL_CFG80211 = 1
endif
ifeq ($(WL_STA_CFG80211),1)
	WL_CFG80211 = 1
endif
ifeq ($(WL_CFG80211),1)
	WLFILES_SRC += src/wl/sys/wl_cfg80211.c
	WLFILES_SRC += src/wl/sys/wl_cfgp2p.c
	WLFILES_SRC += src/wl/sys/wl_feature.c
	WLFILES_SRC += src/wl/sys/wl_linux_mon.c
	WLFLAGS += -DWL_CFG80211
	WLFLAGS += -DUSE_CFG80211
	WLFLAGS += -DWL_CFG80211_NIC
	WLFLAGS += -DWL_HOST_BAND_MGMT
	WLFLAGS += -DSUPPORT_SOFTAP_WPAWPA2_MIXED
	ifeq ($(WL_STA_CFG80211),1)
		WLFLAGS += -DMEDIA_CFG
	endif
	ifeq ($(WL_AP_CFG80211),1)
		WLFLAGS += -DROUTER_CFG
	endif
endif

ifeq ($(WL_HAPD_WDS),1)
	WLFLAGS += -DWL_HAPD_WDS
endif

#Enable Sae if STB_BACKPORT exported for stb
ifeq ($(STBLINUX),1)
	ifeq ($(STB_BACKPORT),1)
	   MFP := 1
	   WLFLAGS += -DWL_SAE
	   WLFLAGS += -DWL_DFS_OFFLOAD
	endif
else ifeq ($(WL_SAE),1)
	MFP := 1
	WLFLAGS += -DWL_SAE
endif

ifeq ($(WL_DPP),1)
	WLFLAGS += -DWL_DPP
endif

#ifdef WLCFE
ifeq ($(WLCFE),1)
	WLFILES_SRC += src/wl/sys/wl_cfe.c
endif
#endif // endif

#ifdef WLRTE
ifeq ($(WLRTE),1)
	WLFILES_SRC += src/wl/sys/wl_rte.c
#	#ifdef PROP_TXSTATUS
	ifeq ($(PROP_TXSTATUS),1)
		WLFLAGS += -DPROP_TXSTATUS
		WLFILES_SRC += src/wl/sys/wlc_wlfc.c
	endif
#	#endif  ## PROP_TXSTATUS
endif
#endif // endif

ifeq ($(BCMECICOEX),1)
	WLFLAGS += -DBCMECICOEX
endif

ifeq ($(BCMLTECOEX),1)
	WLFILES_SRC += src/wl/sys/wlc_ltecx.c
	WLFLAGS += -DBCMLTECOEX
endif

ifeq ($(WLSTAPRIO),1)
	WLFLAGS += -DWL_STAPRIO
	WLFILES_SRC += src/wl/sys/wlc_staprio.c
endif

#ifdef NDIS
# anything Windows/NDIS specific for xp/vista/windows7/8.0/8.1
ifeq ($(WLNDIS),1)
		WLFILES_SRC += components/ndis/src/wl_ndconfig.c
		WLFILES_SRC += components/ndis/src/wl_ndis.c
		WLFILES_SRC += components/ndis/src/wl_oidcmn.c
		WLFILES_SRC += components/ndis/src/wl_oidext.c
		WLFILES_SRC += components/ndis/src/wl_nddbg.c
		WLFILES_SRC += components/ndis/src/wl_ndtimer.c
		WLFILES_SRC += components/ndis/src/wl_ndindicate.c
		WLFILES_SRC += components/ndis/src/wl_ndcommon.c
		WLFILES_SRC += components/ndis/src/wl_ndglobal.c
		WLFILES_SRC += components/ndis/src/wl_ndnwifi.c
# when fast roam is on for windows, the OS processes act frames
ifeq ($(WLFBT),1)
	WLFLAGS += -DWL_OSL_ACTION_FRAME_SUPPORT
endif
	WLFLAGS += -DWL_WLC_SHIM
	WLFLAGS += -DWL_WLC_SHIM_EVENTS
	# WLFLAGS += -DBISON_SHIM_PATCH
	# WLFLAGS += -DCARIBOU_SHIM_PATCH
	WLFILES_SRC += components/shim/src/wl_shim.c
	WLFILES_SRC += components/shim/src/wl_shim_nodes_arr.c
	WLFILES_SRC += components/shim/src/wl_shim_node_default.c
	WLFILES_SRC += components/shim/src/wl_shim_node_2.c
	WLFILES_SRC += components/shim/src/wl_shim_node_4.c
	ifeq ($(WLVIF),1)
		WLFILES_SRC += components/ndis/src/wl_ndvif.c
	endif

        # non-DHD files (NHD files)
	ifeq ($(WLNDIS_DHD),)
		WLFILES_SRC += components/ndis/src/nhd_ndis.c
		WLFLAGS += -DMEMORY_TAG='NWMB'
	else
		WLFILES_SRC += components/ndis/src/dhd_ndis.c
		WLFILES_SRC += src/wl/sys/ndis_threads.c
		WLFLAGS += -DMEMORY_TAG='DWMB'
	endif

	WLFILES_SRC += src/shared/bcmwifi/src/bcmwifi_channels.c
	WLFILES_SRC += src/shared/bcmwifi/src/bcmwifi_rclass.c
	WLFILES_SRC += src/shared/bcmstdlib.c
	WLFILES_SRC += src/shared/ndiserrmap.c

	# support host supplied nvram variables
	ifeq ($(WLTEST),1)
		ifeq ($(WLHOSTVARS), 1)
			WLFLAGS += -DBCMHOSTVARS
		endif
	else
		ifeq ($(BCMEXTNVM),1)
			ifeq ($(WLHOSTVARS), 1)
				WLFLAGS += -DBCMHOSTVARS
			endif
		endif
	endif

	ifneq ($(EXTSTA),)
		WLFLAGS += -DEXT_STA
		WLFLAGS += -DWL_MONITOR
		WLFLAGS += -DIBSS_PEER_GROUP_KEY
		WLFLAGS += -DIBSS_PEER_DISCOVERY_EVENT
		WLFLAGS += -DIBSS_PEER_MGMT
		WLFLAGS += -DAP
		# Disable parallel scan when EXTSTA is on for now.
		WLFLAGS += -DRSDB_PARALLEL_SCAN_DISABLED
		ifeq ($(WLPFN),1)
			WLFLAGS += -DNLO
		endif
	endif

	ifeq ($(WLVIF),1)
		WLFLAGS += -DWLVIF
	endif

	# DHD host: ?? to clean up and to support all other DHD OSes
	ifeq ($(WLNDIS_DHD),1)
		WLFLAGS += -DSHOW_EVENTS -DBCMPERFSTATS
		WLFLAGS += -DBDC -DBCMDONGLEHOST

		ifneq ($(BCMSDIO),1)
			WLFLAGS += -DBCMDHDUSB
		endif

		WLFILES_SRC += src/shared/bcmevent.c
		WLFILES_SRC += src/dhd/sys/dhd_cdc.c
		WLFILES_SRC += src/dhd/sys/dhd_wlfc.c
		WLFILES_SRC += src/dhd/sys/dhd_common.c
		WLFILES_SRC += src/dhd/sys/dhd_debug.c
		WLFILES_SRC += src/dhd/sys/dhd_mschdbg.c
		WLFILES_SRC += src/dhd/sys/dhd_macdbg.c
		WLFILES_SRC += src/dhd/sys/dhd_ip.c

		BCMPCI = 0

		WLFILES_SRC += src/dhd/sys/dhd_usb_ndis.c

		ifneq ($(EXTSTA)),)
			WLFILES_SRC += src/wl/sys/wlc_rate.c
			WLFILES_SRC += src/wl/sys/wlc_rate_def.c
			WLFILES_SRC += src/wl/sys/wlc_ap.c
			WLFILES_SRC += src/wl/sys/wlc_apps.c
		endif
	endif
endif
#endif // endif

ifdef EXT_STA_DONGLE
$(error EXT_STA_DONGLE should be replaced by NDISFW & EXTSTA)
endif

#ifdef NDISFW
ifeq ($(NDISFW),1)
	ifeq ($(EXTSTA),1)
		# DONGLE vista needs WL_MONITOR to pass RTM
		WLFLAGS += -DEXT_STA
		WLFLAGS += -DWL_MONITOR
		WLFLAGS += -DWLCNTSCB
		WLFLAGS += -DIBSS_PEER_GROUP_KEY
		WLFLAGS += -DIBSS_PEER_DISCOVERY_EVENT
		WLFLAGS += -DIBSS_PEER_MGMT
	endif
	WLFILES_SRC += src/wl/sys/wlc_ndis_iovar.c
endif
#endif // endif

ifeq ($(ADV_PS_POLL),1)
	WLFLAGS += -DADV_PS_POLL
endif

ifeq ($(GTKOE),1)
	WLFLAGS += -DGTKOE
	WLFILES_SRC += src/wl/sys/wl_gtkrefresh.c
	ifeq ($(BCMULP),1)
		WLFILES_SRC += src/wl/sys/wl_gtkrefresh_ulp.c
    endif
	WLFILES_SRC += components/bcmcrypto/src/aes.c
	WLFILES_SRC += components/bcmcrypto/src/aeskeywrap.c
	WLFILES_SRC += components/bcmcrypto/src/sha2.c
	WLFILES_SRC += components/bcmcrypto/src/sha2x.c
	WLFILES_SRC += components/bcmcrypto/src/md5.c
	WLFILES_SRC += components/bcmcrypto/src/rijndael-alg-fst.c
	WLFILES_SRC += components/bcmcrypto/src/passhash.c
endif

#ifdef BINOSL
ifeq ($(BINOSL),1)
	WLFLAGS += -DBINOSL
endif
#endif // endif

## wl features
# D11CONF, D11CONF2, D11CONF3, D11CONF4, and D11CONF5 --  bit mask of supported d11 core revs
ifneq ($(D11CONF),)
	WLFLAGS += -DD11CONF=$(D11CONF)
endif
ifneq ($(D11CONF2),)
	WLFLAGS += -DD11CONF2=$(D11CONF2)
endif
ifneq ($(D11CONF3),)
	WLFLAGS += -DD11CONF3=$(D11CONF3)
endif
ifneq ($(D11CONF4),)
	WLFLAGS += -DD11CONF4=$(D11CONF4)
endif
ifneq ($(D11CONF5),)
	WLFLAGS += -DD11CONF5=$(D11CONF5)
endif

# D11CONF_MINOR --  bit mask of supported d11 core minor revs
ifneq ($(D11CONF_MINOR),)
	WLFLAGS += -DD11CONF_MINOR=$(D11CONF_MINOR)
endif

# ACCONF and ACCONF2 -- 0 is remove from code, else bit mask of supported acphy revs
ifneq ($(ACCONF),)
	WLFLAGS += -DACCONF=$(ACCONF)
endif
ifneq ($(ACCONF2),)
	WLFLAGS += -DACCONF2=$(ACCONF2)
endif

# NCONF -- 0 is remove from code, else bit mask of supported nphy revs
ifneq ($(NCONF),)
	WLFLAGS += -DNCONF=$(NCONF)
endif

# HTCONF -- 0 is remove from code, else bit mask of supported htphy revs
ifneq ($(HTCONF),)
	WLFLAGS += -DHTCONF=$(HTCONF)
endif

# ACONF -- 0 is remove from code, else bit mask of supported aphy revs
ifneq ($(ACONF),)
	WLFLAGS += -DACONF=$(ACONF)
endif

# GCONF -- 0 is remove from code, else bit mask of supported gphy revs
ifneq ($(GCONF),)
	WLFLAGS += -DGCONF=$(GCONF)
endif

# LPCONF -- 0 is remove from code, else bit mask of supported lpphy revs
ifneq ($(LPCONF),)
	WLFLAGS += -DLPCONF=$(LPCONF)
endif

# SSLPNCONF -- 0 is remove from code, else bit mask of supported sslpnphy revs
ifneq ($(SSLPNCONF),)
	WLFLAGS += -DSSLPNCONF=$(SSLPNCONF)
endif

#ifdef SOFTAP
ifeq ($(SOFTAP),1)
	WLFLAGS += -DSOFTAP
endif
#endif // endif

#ifdef AP
# ap
ifeq ($(AP),1)
	WLFILES_SRC += src/wl/sys/wlc_ap.c
	WLFILES_SRC += src/wl/sys/wlc_apps.c
	WLFLAGS += -DAP

	ifeq ($(WL_MAP),1)
		WLFLAGS += -DMULTIAP
		WLFLAGS += -DWL_GLOBAL_RCLASS
		WLFLAGS += -DWL_AP_CHAN_CHANGE_EVENT
		DWDS := 1
	endif

	ifeq ($(MBSS),1)
		WLFILES_SRC += src/wl/sys/wlc_mbss.c
		WLFLAGS += -DMBSS
		ifeq ($(WL_EAP_MBSS_BCNROTATE),1)
			WLFLAGS += -DWL_EAP_MBSS_BCNROTATE
		endif
	endif

	ifeq ($(WDS),1)
		WLFILES_SRC += src/wl/sys/wlc_wds.c
		WLFLAGS += -DWDS
	endif

	ifeq ($(DWDS),1)
		WLFLAGS += -DDWDS
	endif

	# Channel Select
	ifeq ($(APCS),1)
		WLFILES_SRC += src/wl/sys/wlc_apcs.c
		WLFLAGS += -DAPCS
	endif

	# WME_PER_AC_TX_PARAMS
	ifeq ($(WME_PER_AC_TX_PARAMS),1)
		WLFLAGS += -DWME_PER_AC_TX_PARAMS
	endif

	# WME_PER_AC_TUNING
	ifeq ($(WME_PER_AC_TUNING),1)
		WLFLAGS += -DWME_PER_AC_TUNING
	endif

	# Dynanic Tx Poiwer control
	ifeq ($(WL_DTPC),1)
		WLFILES_SRC += src/wl/sys/wlc_dtpc.c
		WLFLAGS += -DWLC_DTPC
	endif
	ifeq ($(WL_DTPC_DBG),1)
		WLFLAGS += -DWLC_DTPC_DBG
	endif
endif
#endif // endif

#ifdef STA
# sta
ifeq ($(STA),1)
	WLFLAGS += -DSTA
	WLFILES_SRC += src/wl/sys/wlc_sta.c
endif
#endif // endif

#ifdef APSTA
# apsta
ifeq ($(APSTA),1)
	WLFLAGS += -DAPSTA
endif
# apsta
#endif // endif

#ifdef BCMFRWDPOOLREORG
ifeq ($(BCMFRWDPOOLREORG),1)
	WLFLAGS += -DBCMFRWDPOOLREORG
	WLFILES_SRC += src/shared/hnd_poolreorg.c
	WLFILES_SRC += src/wl/sys/wlc_poolreorg.c
endif
#endif // endif
#ifdef BCMPOOLRECLAIM
ifeq ($(BCMPOOLRECLAIM),1)
	WLFLAGS += -DBCMPOOLRECLAIM
endif
#endif // endif

#ifdef WET
# wet
ifeq ($(WET),1)
	WLFLAGS += -DWET
	WLFILES_SRC += src/wl/sys/wlc_wet.c
endif
#endif // endif

ifeq ($(WET_DONGLE),1)
	WLFLAGS += -DWET_DONGLE
endif

#ifdef RXCHAIN_PWRSAVE
ifeq ($(RXCHAIN_PWRSAVE), 1)
	WLFLAGS += -DRXCHAIN_PWRSAVE
endif
#endif // endif

#ifdef RADIONOA_PWRSAVE
ifeq ($(RADIONOA_PWRSAVE),1)
	WLFLAGS += -DRADIONOA_PWRSAVE
	WLFILES_SRC += src/wl/sys/wlc_rpsnoa.c
	WL_BSSCFG_TX_SUPR := 1
	WLP2P := 1
	RADIO_PWRSAVE :=1
endif
#endif /* RADIONOA_PWRSAVE */

#ifdef RADIO_PWRSAVE
ifeq ($(RADIO_PWRSAVE), 1)
	WLFLAGS += -DRADIO_PWRSAVE
endif
#endif // endif

#ifdef WMF
ifeq ($(WMF), 1)
	WLFILES_SRC += src/wl/sys/wlc_wmf.c
	WLFLAGS += -DWMF
endif
ifeq ($(IGMP_UCQUERY), 1)
	WLFLAGS += -DWL_IGMP_UCQUERY
endif
ifeq ($(UCAST_UPNP), 1)
	WLFLAGS += -DWL_UCAST_UPNP
endif
#endif // endif

#ifdef MCAST_REGEN
ifeq ($(MCAST_REGEN), 1)
	WLFLAGS += -DMCAST_REGEN
endif
#endif // endif

ifneq ($(STBLINUX),1)
#ifdef  ROUTER_COMA
ifeq ($(ROUTER_COMA), 1)
	WLFILES_SRC += src/shared/hndmips.c
	WLFILES_SRC += src/shared/hndchipc.c
	WLFLAGS += -DROUTER_COMA
endif
#endif // endif
endif

#ifdef WLOVERTHRUSTER
ifeq ($(WLOVERTHRUSTER), 1)
	WLFLAGS += -DWLOVERTHRUSTER
endif
#endif // endif

#ifdef MAC_SPOOF
# mac spoof
ifeq ($(MAC_SPOOF),1)
	WLFLAGS += -DMAC_SPOOF
endif
#endif // endif

#ifdef PSTA
# Proxy STA
ifeq ($(PSTA),1)
	WLFILES_SRC += src/shared/bcm_psta.c
	WLFILES_SRC += src/wl/sys/wlc_psta.c
	WLFLAGS += -DPSTA
endif
#endif // endif

#ifdef DPSTA
# Dualband Proxy STA
ifeq ($(STA),1)
	ifeq ($(DPSTA),1)
		WLFLAGS += -DDPSTA
	endif
endif
#endif // endif

# Router IBSS Security Support
ifeq ($(ROUTER_SECURE_IBSS),1)
         WLFLAGS += -DIBSS_PEER_GROUP_KEY
         WLFLAGS += -DIBSS_PSK
         WLFLAGS += -DIBSS_PEER_MGMT
         WLFLAGS += -DIBSS_PEER_DISCOVERY_EVENT
endif

#ifdef WLLED
# led
ifeq ($(WLLED),1)
	WLFLAGS += -DWLLED
	WLFILES_SRC += src/wl/sys/wlc_led.c
endif
#endif // endif

#ifdef WL_MONITOR
# MONITOR
ifeq ($(WL_MONITOR),1)
	WLFLAGS += -DWL_MONITOR
	WLFLAGS += -DWL_NEW_RXSTS
	ifneq ($(FULLDNGLBLD),1)
		WLFILES_SRC += src/shared/bcmwifi/src/bcmwifi_monitor.c
		WLFILES_SRC += src/shared/bcmwifi/src/bcmwifi_radiotap.c
	endif
endif
#endif // endif

#ifdef WL_RADIOTAP
ifeq ($(WL_RADIOTAP),1)
	WLFLAGS += -DWL_RADIOTAP
	WLFILES_SRC += src/shared/bcmwifi/src/bcmwifi_radiotap.c
endif
#endif // endif

#ifdef WL_STA_MONITOR
ifeq ($(WL_STA_MONITOR),1)
	WLFLAGS += -DWL_STA_MONITOR
endif
#endif // endif

#ifdef ACKSUPR_MAC_FILTER
ifeq ($(ACKSUPR_MAC_FILTER),1)
        WLFLAGS += -DACKSUPR_MAC_FILTER
endif
#endif // endif

#ifdef WL_PROMISC
# PROMISC
ifeq ($(PROMISC),1)
	WLFLAGS += -DWL_PROMISC
endif
#endif // endif

ifeq ($(WL_ALL_PASSIVE),1)
	WLFLAGS += -DWL_ALL_PASSIVE
ifdef WL_ALL_PASSIVE_MODE
	WLFLAGS += -DWL_ALL_PASSIVE_MODE=$(WL_ALL_PASSIVE_MODE)
endif
endif

#ifdef ND_ALL_PASSIVE
ifeq ($(ND_ALL_PASSIVE),1)
	WLFLAGS += -DND_ALL_PASSIVE
endif
#endif // endif

#ifdef WME
# WME
ifeq ($(WME),1)
	WLFLAGS += -DWME
endif
#endif // endif

#ifdef WL11H
# 11H
ifeq ($(WL11H),1)
	WLFLAGS += -DWL11H
endif
#endif // endif

#ifdef WL11D
# 11D
ifeq ($(WL11D),1)
	WLFLAGS += -DWL11D
endif
#endif // endif

#ifdef WL11U
# 11U
ifeq ($(WL11U),1)
	L2_FILTER := 1
	WLFLAGS += -DWL11U
	WLFILES_SRC += src/wl/sys/wlc_11u.c
endif
#endif // endif

#ifdef WLPROBRESP_SW
# WLPROBRESP_SW
ifeq ($(WLPROBRESP_SW),1)
	WLFLAGS += -DWLPROBRESP_SW
	WLFILES_SRC += src/wl/sys/wlc_probresp.c
ifeq ($(WLPROBRESP_MAC_FILTER),1)
	WLFLAGS += -DWLPROBRESP_MAC_FILTER
	WLFILES_SRC += src/wl/sys/wlc_probresp_mac_filter.c
endif
endif
#endif // endif

ifeq ($(WLAUTHRESP_MAC_FILTER),1)
	WLFLAGS += -DWLAUTHRESP_MAC_FILTER
endif

WLFLAGS += -DBAND2G=$(BAND2G)
WLFLAGS += -DBAND5G=$(BAND5G)
WLFLAGS += -DBAND6G=$(BAND6G)

#ifdef WLRM
# WLRM
ifeq ($(WLRM),1)
	WLFLAGS += -DWLRM
endif
#endif // endif

#ifdef WLCQ
# WLCQ
ifeq ($(WLCQ),1)
	WLFLAGS += -DWLCQ
endif
#endif // endif

#ifdef WLCNT
# WLCNT
ifeq ($(WLCNT),1)
	WLFLAGS += -DWLCNT
	WLFILES_SRC += src/shared/bcm_app_utils.c
ifndef DELTASTATS
	DELTASTATS := 1
endif
endif
#endif // endif

#ifdef WLTAF
ifeq ($(WLTAF),1)
    WLFILES_SRC += src/wl/sys/wlc_taf.c
    WLFILES_SRC += src/wl/sys/wlc_taf_ias.c
    WLFLAGS += -DWLTAF
endif
#endif // endif

# DELTASTATS
ifeq ($(DELTASTATS),1)
	WLFLAGS += -DDELTASTATS
endif

#ifdef WLCHANIM
# WLCHANIM
ifeq ($(WLCHANIM),1)
	WLFLAGS += -DWLCHANIM
endif
#endif // endif

#ifdef WLCNTSCB
# WLCNTSCB
ifeq ($(WLCNTSCB),1)
	WLFLAGS += -DWLCNTSCB
ifeq ($(WLSCB_HISTO),1)
	WLFLAGS += -DWLSCB_HISTO
endif
endif
#endif // endif
ifeq ($(WL_OKC),1)
	WLFLAGS += -DWL_OKC
	WLFILES_SRC += src/wl/sys/wlc_okc.c
	WLFILES_SRC += components/bcmcrypto/src/sha2.c
	WLFILES_SRC += components/bcmcrypto/src/sha2x.c
	WLFILES_SRC += components/bcmcrypto/src/md5.c
endif
ifeq ($(WLRCC),1)
	WLFLAGS += -DWLRCC
	ifneq ($(WL_OKC),1)
		WLFILES_SRC += src/wl/sys/wlc_okc.c
	endif
endif
ifeq ($(WLABT),1)
	WLFLAGS += -DWLABT
endif

#ifdef WLCOEX
# WLCOEX
ifeq ($(WLCOEX),1)
	WLFLAGS += -DWLCOEX
endif
#endif // endif

#ifdef WLOSEN
# hotspot OSEN
ifeq ($(WLOSEN),1)
	WLFLAGS += -DWLOSEN
endif
#endif // endif

## wl security
# external linux supplicant
#ifdef LINUX_CRYPTO
ifeq ($(LINUX_CRYPTO),1)
	WLFLAGS += -DLINUX_CRYPTO
endif
#endif // endif

#ifdef WLFBT
ifeq ($(WLFBT),1)
	WLFLAGS += -DWLFBT
	WLFLAGS += -DBCMINTSUP
	WLFILES_SRC += src/wl/sys/wlc_fbt.c
	WLFILES_SRC += src/wl/sys/wlc_wpapsk.c
	WLFILES_SRC += components/bcmcrypto/src/aes.c
	WLFILES_SRC += components/bcmcrypto/src/aeskeywrap.c
	WLFILES_SRC += components/bcmcrypto/src/sha2.c
	WLFILES_SRC += components/bcmcrypto/src/sha2x.c
	WLFILES_SRC += components/bcmcrypto/src/md5.c
	WLFILES_SRC += components/bcmcrypto/src/passhash.c
	MD5 := 1
	ifeq ($(STA),1)
		# external supplicant: need some crypto files for fbt
		ifneq ($(BCMSUP_PSK),1)
			WLFILES_SRC += src/wl/sys/wlc_sup.c
		endif
	endif
	WLCAC := 1
endif
#endif // endif

ifeq ($(WL_ASSOC_MGR),1)
	WLFLAGS += -DWL_ASSOC_MGR
	WLFILES_SRC += src/wl/sys/wlc_assoc_mgr.c
endif

ifeq ($(WLCHANIM_US),1)
	WLFLAGS += -DWLCHANIM_US
endif
#ifdef BCMSUP_PSK
# in-driver supplicant
ifeq ($(BCMSUP_PSK),1)
	PSK_COMMON := 1
	WLFLAGS += -DBCMSUP_PSK -DBCMINTSUP
	WLFILES_SRC += src/wl/sys/wlc_sup.c
endif
#endif // endif

#ifdef BCMAUTH_PSK
# in-driver authenticator
ifeq ($(BCMAUTH_PSK),1)
	PSK_COMMON := 1
	WLFLAGS += -DBCMAUTH_PSK
	WLFILES_SRC += src/wl/sys/wlc_auth.c
endif
#endif // endif

# common files for both idsup & authenticator
ifeq ($(PSK_COMMON),1)
	WLFILES_SRC += src/wl/sys/wlc_wpapsk.c
	WLFILES_SRC += components/bcmcrypto/src/aes.c
	WLFILES_SRC += components/bcmcrypto/src/aeskeywrap.c
	WLFILES_SRC += components/bcmcrypto/src/sha2.c
	WLFILES_SRC += components/bcmcrypto/src/sha2x.c
	WLFILES_SRC += components/bcmcrypto/src/md5.c
	WLFILES_SRC += components/bcmcrypto/src/passhash.c
	MD5 := 1
	RIJNDAEL := 1
endif

#ifdef WLCAC
ifeq ($(WLCAC),1)
	WLFLAGS += -DWLCAC
	WLFILES_SRC += src/wl/sys/wlc_cac.c
endif
#endif // endif

#ifdef WLTDLS
ifeq ($(WLTDLS), 1)
	WLFILES_SRC += components/bcmcrypto/src/sha2.c
	WLFILES_SRC += components/bcmcrypto/src/sha2x.c
	WLFILES_SRC += components/bcmcrypto/src/md5.c
endif
#endif // endif

#ifdef MFP
# Management Frame Protection
ifeq ($(MFP),1)
	WLFLAGS += -DMFP
	WLFILES_SRC += components/bcmcrypto/src/aes.c
	WLFILES_SRC += components/bcmcrypto/src/sha2.c
	WLFILES_SRC += components/bcmcrypto/src/sha2x.c
	WLFILES_SRC += components/bcmcrypto/src/md5.c
	WLFILES_SRC += src/wl/sys/wlc_mfp.c
	WLFILES_SRC += components/bcmcrypto/src/rijndael-alg-fst.c
	ifeq ($(MFP_TEST),1)
		WLFLAGS += -DMFP_TEST
		WLFILES_SRC += src/wl/sys/wlc_mfp_test.c
	endif
	BCMCCMP := 1
endif
#endif // endif

#ifdef BCMCCMP
# Soft AES CCMP
ifeq ($(BCMCCMP),1)
	WLFLAGS += -DBCMCCMP
	WLFILES_SRC += components/bcmcrypto/src/aes.c
	WLFILES_SRC += components/bcmcrypto/src/rijndael-alg-fst.c
endif
#endif // endif

# MD5 and AES
ifeq ($(MD5),1)
	WLFILES_SRC += components/bcmcrypto/src/md5.c
endif
ifeq ($(RIJNDAEL),1)
	WLFILES_SRC += components/bcmcrypto/src/rijndael-alg-fst.c
endif

ifeq ($(BCMDMA64OSL),1)
	WLFLAGS += -DBCMDMA64OSL
endif

ifeq ($(BCMDMASGLISTOSL),1)
	WLFLAGS += -DBCMDMASGLISTOSL
endif

## wl over jtag
#ifdef BCMJTAG
ifeq ($(BCMJTAG),1)
	WLFLAGS += -DBCMJTAG -DBCMSLTGT
	WLFILES_SRC += src/shared/bcmjtag.c
	WLFILES_SRC += src/shared/bcmjtag_linux.c
	WLFILES_SRC += src/shared/ejtag.c
	WLFILES_SRC += src/shared/jtagm.c
endif
#endif // endif

#ifdef WLAMSDU
ifeq ($(WLAMSDU),1)
	WLFLAGS += -DWLAMSDU
	WLFILES_SRC += src/wl/sys/wlc_amsdu.c

	ifeq ($(BCMDBG_AMSDU),1)
		# Debug for: wl dump amsdu
		WLFLAGS += -DBCMDBG_AMSDU
	endif
endif
#endif // endif

ifeq ($(BCMDBG_WLMAC),1)
	WLFLAGS += -DWL_MACDBG
endif

ifeq ($(BCMDBG_WLUTRACE),1)
	WLFLAGS += -DWL_UTRACE
endif

ifeq ($(BCMDBG_SSSR),1)
	WLFLAGS += -DWL_SSSR
endif

ifeq ($(BCMDBG_TEMPSENSE),1)
	WLFLAGS += -DBCMDBG_TEMPSENSE
endif

#ifdef WLAMSDU_TX
ifeq ($(WLAMSDU_TX),1)
	WLFLAGS += -DWLAMSDU_TX
	WLFILES_SRC += src/wl/sys/wlc_amsdu.c
endif
#endif // endif

#ifdef WLAMSDU_SWDEAGG
ifeq ($(WLAMSDU_SWDEAGG),1)
	WLFLAGS += -DWLAMSDU_SWDEAGG
endif
#endif // endif

#ifdef WLNAR
ifeq ($(WLNAR),1)
	WLFILES_SRC += src/wl/sys/wlc_nar.c
	WLFLAGS += -DWLNAR
endif
#endif // endif

ifdef WLAMPDU_HW
$(error "WLAMPDU_HW is an obsolete build option")
endif

ifeq ($(WLAMPDU),1)
	WLFLAGS += -DWLAMPDU
	WLFILES_SRC += src/wl/sys/wlc_ampdu.c
	WLFILES_SRC += src/wl/sys/wlc_ampdu_rx.c
	WLFILES_SRC += src/wl/sys/wlc_ampdu_cmn.c
	ifeq ($(WLAMPDU_UCODE),1)
		WLFLAGS += -DWLAMPDU_UCODE
	endif
	ifeq ($(WLAMPDU_UCODE_ONLY),1)
		WLFLAGS += -DWLAMPDU_UCODE_ONLY
	endif
	ifeq ($(WLAMPDU_AQM),1)
		WLFLAGS += -DWLAMPDU_AQM
	endif
	ifeq ($(WLAMPDU_PRECEDENCE),1)
		WLFLAGS += -DWLAMPDU_PRECEDENCE
	endif
	ifeq ($(WL_EXPORT_AMPDU_RETRY),1)
		WLFLAGS += -DWL_EXPORT_AMPDU_RETRY
	endif

	ifeq ($(BCMDBG_AMPDU),1)
		# Debug for: wl dump ampdu; wl ampdu_clear_dump
		WLFLAGS += -DBCMDBG_AMPDU
	endif
endif

#ifdef WOWL
ifeq ($(WOWL),1)
	WLFLAGS += -DWOWL
	ifeq ($(BCMULP),1)
		WLFILES_SRC += $(UCODE_RAM_DIR)/d11ucode_ulp.c
	else
		WLFILES_SRC += $(UCODE_RAM_DIR)/d11ucode_wowl.c
	endif
	WLFILES_SRC += $(UCODE_RAM_DIR)/d11ucode_p2p.c
	WLFILES_SRC += $(UCODE_RAM_DIR)/d11ucode_ge40.c
	WLFILES_SRC += $(UCODE_RAM_DIR)/d11ucode_ge24.c
	WLFILES_SRC += src/wl/sys/wlc_wowl.c
	WLFILES_SRC += src/wl/sys/wowlaestbls.c
endif
#endif // endif

# ucode in rom
ifeq ($(UCODE_IN_ROM),1)
	WLFLAGS += -DUCODE_IN_ROM_SUPPORT
	ifneq ($(ULP_DS1ROM_DS0RAM),1)
		WLFILES_SRC += $(UCODE_ROM_DIR)/d11ucode_p2p_upatch.c
	else # ULP_DS1ROM_DS0RAM
		WLFLAGS += -DULP_DS1ROM_DS0RAM
	endif # ULP_DS1ROM_DS0RAM
	WLFILES_SRC += $(UCODE_ROM_DIR)/d11ucode_ulp_upatch.c
endif # UCODE_IN_ROM

#ifdef WOWLPF
ifeq ($(WOWLPF),1)
	ifneq ($(PACKET_FILTER)$(PACKET_FILTER2)$(PACKET_FILTER2_DEBUG),)
		WLFLAGS += -DWOWLPF
		WLFILES_SRC += src/wl/sys/wlc_wowlpf.c
		#ifdef SECURE_WOWL
		ifeq ($(SECURE_WOWL),1)
			WLFLAGS += -DSECURE_WOWL
		endif
		#endif
		#ifdef SS_WOWL
		ifeq ($(SS_WOWL),1)
			WLFLAGS += -DSS_WOWL
		endif
		#endif
	endif
endif
#endif // endif

#ifdef WL_ASSOC_RECREATE
ifeq ($(WL_ASSOC_RECREATE),1)
	WLFLAGS += -DWL_ASSOC_RECREATE
endif
#endif // endif

ifeq ($(ATE),1)
	WLFLAGS += -DATE_BUILD=1
	WLFILES_SRC += src/wl/sys/wl_ate.c

	EXTRA_DFLAGS    += -DWLPKTENG
	EXTRA_DFLAGS    += -DSAMPLE_COLLECT
	EXTRA_DFLAGS    += -DWLTEST
	EXTRA_DFLAGS    += -DBCMDBG_PHYDUMP
	EXTRA_DFLAGS    += -DBCMNVRAMW
	EXTRA_DFLAGS    += -DBCMNVRAMR
endif

#ifdef WLTDLS
ifeq ($(TDLS_TESTBED), 1)
	WLFLAGS += -DTDLS_TESTBED
endif
ifeq ($(WLTDLS), 1)
	WLFLAGS += -DWLTDLS
	WLFLAGS += -DWLTDLS_SEND_PTI_RETRY
	WLFLAGS += -DIEEE2012_TDLSSEPC
	WLFILES_SRC += src/wl/sys/wlc_tdls.c
endif
ifeq ($(BE_TDLS),1)
	WLFLAGS += -DBE_TDLS
endif
#endif // endif

#ifdef WLDLS
ifeq ($(WLDLS), 1)
	WLFLAGS += -DWLDLS
	WLFILES_SRC += src/wl/sys/wlc_dls.c
endif
#endif // endif

#ifdef WLBSSLOAD
# WLBSSLOAD
ifeq ($(WLBSSLOAD),1)
	WLFLAGS += -DWLBSSLOAD
	WLFILES_SRC += src/wl/sys/wlc_bssload.c
endif
#endif // endif

#ifdef L2_FILTER
ifeq ($(L2_FILTER),1)
	WLFLAGS += -DL2_FILTER
	ifeq ($(L2_FILTER_STA),1)
		WLFLAGS += -DL2_FILTER_STA
	endif
	WLFILES_SRC += src/wl/sys/wlc_l2_filter.c
	WLFILES_SRC += src/shared/bcm_l2_filter.c
endif
#endif // endif

# FCC power limit control on ch12/13.
ifeq ($(FCC_PWR_LIMIT_2G),1)
	WLFLAGS += -DFCC_PWR_LIMIT_2G
endif

#ifdef WLP2P
ifeq ($(WLP2P),1)
	WLFLAGS += -DWLP2P
	WLFILES_SRC += src/wl/sys/wlc_p2p.c
	WL_BSSCFG_TX_SUPR := 1
	WIFI_ACT_FRAME := 1
	WLMCNX := 1
ifeq ($(WL_LEGACY_P2P),1)
	WLFLAGS += -DWL_LEGACY_P2P
	WLFLAGS += -DP2P_IE_OVRD
endif
ifndef WLMCHAN
	WLMCHAN := 1
endif
# WFDS support
ifeq ($(WLWFDS),1)
	WLFLAGS += -DWLWFDS
endif
endif
#endif /* WLP2P */

#ifdef BCMCOEXNOA
ifeq ($(BCMCOEXNOA),1)
	WLFLAGS += -DBCMCOEXNOA
	WLFILES_SRC += src/wl/sys/wlc_cxnoa.c
	WL_BSSCFG_TX_SUPR := 1
	WIFI_ACT_FRAME := 1
	WLMCNX := 1
endif
#endif /* BCMCOEXNOA */

#ifdef WIFI_ACT_FRAME
# WIFI_ACT_FRAME
ifeq ($(WIFI_ACT_FRAME),1)
	WLFLAGS += -DWIFI_ACT_FRAME
endif
#endif // endif

ifeq ($(WL_BSSCFG_TX_SUPR),1)
	WLFLAGS += -DWL_BSSCFG_TX_SUPR
endif

ifeq ($(SRSCAN),1)
	WLFLAGS += -DWLMSG_SRSCAN
endif

#ifdef WL_RXEARLYRC
ifeq ($(WL_RXEARLYRC),1)
	WLFLAGS += -DWL_RXEARLYRC
endif
#endif // endif

#ifdef SRHWVSDB
ifeq ($(SRHWVSDB),1)
	WLFLAGS += -DSRHWVSDB
endif
ifeq ($(PHY_WLSRVSDB),1)
	# cpp define WLSRVSDB is used in this branch by PHY code only
	WLFLAGS += -DWLSRVSDB
endif
#endif /* SRHWVSDB */

#ifdef WLMCHAN
ifeq ($(WLMCHAN),1)
	WLMCNX := 1
endif
#endif // endif

WLP2P_UCODE ?= 0
# multiple connection
ifeq ($(WLMCNX),1)
	WLP2P_UCODE := 1
	WLFLAGS += -DWLMCNX
	WLFILES_SRC += src/wl/sys/wlc_mcnx.c
	WLFILES_SRC += src/wl/sys/wlc_tbtt.c
endif

ifeq ($(WLSLOTTED_BSS),1)
	WLFLAGS += -DWLSLOTTED_BSS
	WLFILES_SRC += src/wl/sys/wlc_slotted_bss.c
endif

ifeq ($(WLP2P_UCODE_ONLY),1)
	WLFLAGS += -DWLP2P_UCODE_ONLY
#	WLP2P_UCODE := 1
endif

ifeq ($(WLP2P_UCODE),1)
	WLFLAGS += -DWLP2P_UCODE
	WLFILES_SRC += $(UCODE_RAM_DIR)/d11ucode_p2p.c
endif

#ifdef WLMCHAN
ifeq ($(WLMCHAN),1)
	WLFLAGS += -DWLMCHAN
	WLFLAGS += -DWLTXPWR_CACHE
	WLFLAGS += -DWLTXPWR_CACHE_PHY_ONLY
	WLFILES_SRC += src/wl/sys/wlc_mchan.c
ifndef WLMULTIQUEUE
	WLMULTIQUEUE := 1
endif
endif
#endif /* WLMCHAN */

ifeq ($(and $(WLMCHAN),$(WLRSDB),$(WL_MODESW),$(WLASDB)),1)
	WLFLAGS += -DWL_ASDB
	WLFILES_SRC_HI += src/wl/sys/wlc_asdb.c
endif

ifeq ($(WL_DUALMAC_RSDB), 1)
	WLFLAGS += -DWL_DUALMAC_RSDB
endif

ifneq ($(WL_NO_IOVF_RSDB_SET), 1)
	WLFLAGS += -DWL_IOVF_RSDB_SET
endif

ifneq ($(WLC_MAX_UCODE_BSS),)
	WLFLAGS += -DWLC_MAX_UCODE_BSS=$(WLC_MAX_UCODE_BSS)
endif

ifeq ($(WL_DUALNIC_RSDB),1)
	WLFLAGS += -DWL_DUALNIC_RSDB
endif

ifeq ($(RSDB_DVT), 1)
	WLFLAGS += -DWLRSDB_DVT
endif

ifeq ($(CCA_STATS),1)
	WLFLAGS += -DCCA_STATS
	WLFILES_SRC += src/wl/sys/wlc_cca.c
ifeq ($(ISID_STATS),1)
	WLFLAGS += -DISID_STATS
	WLFILES_SRC += src/wl/sys/wlc_interfere.c
endif
#ifdef WL_CCA_STATS_MESH
ifeq ($(WL_CCA_STATS_MESH),1)
        WLFLAGS += -DWL_CCA_STATS_MESH
endif
#endif // endif
endif

#ifdef WLRWL
ifeq ($(WLRWL),1)
	WLFLAGS += -DWLRWL
	WLFILES_SRC += src/wl/sys/wlc_rwl.c
endif
#endif // endif

ifeq ($(D0_COALESCING),1)
	WLFLAGS += -DD0_COALESCING
	WLFILES_SRC += src/wl/sys/wl_d0_filter.c
	WLMCHAN := 1
endif

#ifdef WLMEDIA
ifeq ($(WLMEDIA),1)
	WLFLAGS += -DWLMEDIA_EN
	WLFLAGS += -DWLMEDIA_RATESTATS
	WLFLAGS += -DWLMEDIA_MULTIQUEUE
	WLFLAGS += -DWLMEDIA_TXFIFO_FLUSH_SCB
	WLFLAGS += -DWLMEDIA_AMPDUSTATS
	WLFLAGS += -DWLMEDIA_LQSTATS
	WLFLAGS += -DWLMEDIA_CALDBG
	WLFLAGS += -DWLMEDIA_EXT
	WLFLAGS += -DWLMEDIA_TXFILTER_OVERRIDE
	WLFLAGS += -DWLMEDIA_TSF
	WLFLAGS += -DWLMEDIA_PEAKRATE
endif
#endif // endif

#ifdef WLPKTDLYSTAT
ifeq ($(WLPKTDLYSTAT),1)
	WLFLAGS += -DWLPKTDLYSTAT
endif
#endif // endif

#ifdef WLPKTDLYSTAT_IND
ifeq ($(WLPKTDLYSTAT_IND),1)
	WLFLAGS += -DWLPKTDLYSTAT_IND
endif
#endif // endif

# Clear restricted channels upon receiving bcn/prbresp frames
#ifdef WLNON_CLEARING_RESTRICTED_CHANNEL_BEHAVIOR
ifeq ($(WLNON_CLEARING_RESTRICTED_CHANNEL_BEHAVIOR),1)
	WLFLAGS += -DWLNON_CLEARING_RESTRICTED_CHANNEL_BEHAVIOR
endif
#endif /* WLNON_CLEARING_RESTRICTED_CHANNEL_BEHAVIOR */

ifeq ($(WLFMC),1)
	WLFLAGS += -DWLFMC
endif

## --- which buses

# silicon backplane

#ifdef BCMSIBUS
ifeq ($(BCMSIBUS),1)
	WLFLAGS += -DBCMBUSTYPE=SI_BUS
	BCMPCI = 0
endif
#endif // endif

ifeq ($(SOCI_SB),1)
	WLFLAGS += -DBCMCHIPTYPE=SOCI_SB
else
	ifeq ($(SOCI_AI),1)
		WLFLAGS += -DBCMCHIPTYPE=SOCI_AI
	endif
endif

# AP/ROUTER with SDSTD
ifeq ($(WLAPSDSTD),1)
	WLFILES_SRC += src/shared/nvramstubs.c
	WLFILES_SRC += src/shared/bcmsrom.c
endif

## --- basic shared files

#ifdef HNDDMA
ifeq ($(HNDDMA),1)
	WLFILES_SRC += src/shared/hnddma_tx.c
	WLFILES_SRC += src/shared/hnddma_rx.c
	WLFILES_SRC += src/shared/hnddma.c
endif
#endif // endif

#ifdef HNDBME
ifeq ($(HNDBME),1)
	WLFLAGS += -DHNDBME
	WLFILES_SRC += src/shared/hndbme.c
endif
#endif // endif

#ifdef HNDGCI
ifeq ($(HNDGCI),1)
	WLFLAGS += -DHNDGCI
	WLFILES_SRC += src/shared/hndgci.c
endif
#endif // endif

#ifdef BCMUTILS
  ifeq ($(BCMUTILS),1)
	WLFILES_SRC += src/shared/bcmutils.c
	WLFILES_SRC += src/shared/bcmxtlv.c
	WLFILES_SRC += src/shared/hnd_pktpool.c
	WLFILES_SRC += src/shared/hnd_pktq.c
 endif
#endif // endif

# Use LHL Timer
ifeq ($(USE_LHL_TIMER),1)
	WLFLAGS += -DUSE_LHL_TIMER
endif

#ifdef BCMSROM
ifeq ($(BCMSROM),1)
	ifeq ($(BCMSDIO),1)
		WLFILES_SRC += src/shared/bcmsrom.c
		WLFILES_SRC += src/shared/bcmotp.c
	endif
	WLFILES_SRC += src/shared/bcmsrom.c
	WLFILES_SRC += src/shared/bcmotp.c
endif
#endif // endif

#ifdef BCMOTP
ifeq ($(BCMOTP),1)
	ifneq ($(BCMOTP_SHARED_SYMBOLS_USE),1)
		WLFILES_SRC += src/shared/bcmotp.c
	endif
	WLFLAGS += -DBCMNVRAMR
	ifeq ($(WLTEST),1)
		WLFLAGS += -DBCMNVRAMW
	endif

	ifeq ($(BCMOTPSROM),1)
		WLFLAGS += -DBCMOTPSROM
	endif
endif
#endif // endif

#ifdef SIUTILS
ifeq ($(SIUTILS),1)
	WLFILES_SRC += src/shared/siutils.c
	WLFILES_SRC += src/shared/sbutils.c
	WLFILES_SRC += src/shared/aiutils.c
	WLFILES_SRC += src/shared/hndpmu.c
	WLFILES_SRC += src/shared/hndlhl.c
	ifneq ($(BCMPCI), 0)
		WLFILES_SRC += src/shared/nicpci.c
		WLFILES_SRC += src/shared/pcie_core.c
	endif
endif
#endif /* SIUTILS */

#ifdef SBMIPS
ifeq ($(SBMIPS),1)
	WLFLAGS += -DBCMMIPS
	WLFILES_SRC += src/shared/hndmips.c
	WLFILES_SRC += src/shared/hndchipc.c
endif
#endif // endif

#ifdef SBPCI
ifeq ($(SBPCI),1)
	WLFILES_SRC += src/shared/hndpci.c
endif
#endif // endif

#ifdef SFLASH
ifeq ($(SFLASH),1)
	WLFILES_SRC += src/shared/sflash.c
endif
#endif // endif

#ifdef FLASHUTL
ifeq ($(FLASHUTL),1)
	WLFILES_SRC += src/shared/flashutl.c
endif
#endif // endif

## --- shared OSL
#ifdef OSLLX
# linux osl
ifeq ($(OSLLX),1)
	WLFILES_SRC += src/shared/linux_osl.c
	WLFILES_SRC += src/shared/linux_pkt.c
endif
#endif // endif

#ifdef OSLCFE
ifeq ($(OSLCFE),1)
	WLFILES_SRC += src/shared/cfe_osl.c
endif
#endif // endif

#ifdef OSLRTE
ifeq ($(OSLRTE),1)
	WLFILES_SRC += src/shared/rte_osl.c
	ifeq ($(SRMEM),1)
		WLFLAGS += -DSRMEM
		WLFILES_SRC += src/shared/hndsrmem.c
	endif
endif
#endif // endif

#ifdef OSLNDIS
ifeq ($(OSLNDIS),1)
	WLFILES_SRC += components/ndis/src/ndshared.c
	WLFILES_SRC += src/shared/ndis_osl.c
endif
#endif // endif

ifeq ($(NVRAM),1)
	WLFILES_SRC += src/dongle/rte/test/nvram.c
	WLFILES_SRC += src/dongle/rte/sim/nvram.c
	WLFILES_SRC += src/shared/nvram.c
endif

#ifdef BCMNVRAMR
ifeq ($(BCMNVRAMR),1)
	WLFILES_SRC += src/shared/nvram_ro.c
	WLFILES_SRC += src/shared/sflash.c
	WLFILES_SRC += src/shared/bcmotp.c
	WLFLAGS += -DBCMNVRAMR
endif
#else /* !BCMNVRAMR */
ifneq ($(BCMNVRAMR),1)
	ifeq ($(WLLXNOMIPSEL),1)
		ifneq ($(WLUMK),1)
			WLFILES_SRC += src/shared/nvramstubs.c
		endif
	else
		ifeq ($(WLNDIS),1)
			WLFILES_SRC += src/shared/nvramstubs.c
		else
			ifeq ($(BCMNVRAMW),1)
				WLFILES_SRC += src/shared/nvram_ro.c
				WLFILES_SRC += src/shared/sflash.c
			endif
		endif
	endif
	ifeq ($(BCMNVRAMW),1)
		WLFILES_SRC += src/shared/bcmotp.c
		WLFLAGS += -DBCMNVRAMW
	endif
endif
#endif /* !BCMNVRAMR */

# Define one OTP mechanism, or none to support all dynamically
ifeq ($(BCMHNDOTP),1)
	WLFLAGS += -DBCMHNDOTP
endif
ifeq ($(BCMIPXOTP),1)
	WLFLAGS += -DBCMIPXOTP
endif

#ifdef WLDIAG
ifeq ($(WLDIAG),1)
	WLFLAGS += -DWLDIAG
	WLFILES_SRC += src/wl/sys/wlc_diag.c
endif
#endif // endif

#ifdef BCMDBG
ifneq ($(BCMDBG),1)
	ifeq ($(WLTINYDUMP),1)
		WLFLAGS += -DWLTINYDUMP
	endif
endif
#endif // endif

#ifdef WLPFN
ifeq ($(WLPFN),1)
	WLFLAGS += -DWLPFN
	WLFILES_SRC += src/wl/sys/wl_pfn.c
	ifeq ($(WLPFN_AUTO_CONNECT),1)
		WLFLAGS += -DWLPFN_AUTO_CONNECT
	endif
endif
#endif /* WLPFN */

#ifdef GSCAN
ifeq ($(GSCAN),1)
	WLFLAGS += -DGSCAN
endif
#endif /* GSCAN */

#ifdef RSSI_MONITOR
ifeq ($(RSSI_MONITOR),1)
		WLFLAGS += -DRSSI_MONITOR
endif
#endif /* RSSI_MONITOR */

#ifdef WL_PWRSTATS
ifeq ($(WL_PWRSTATS),1)
	WLFLAGS += -DWL_PWRSTATS
	WLFILES_SRC += src/wl/sys/wlc_pwrstats.c
endif
#endif // endif

#ifdef WL_LEAKY_AP_STATS
ifeq ($(WL_LEAKY_AP_STATS),1)
	WLFLAGS += -DWL_LEAKY_AP_STATS
	WLFILES_SRC += src/wl/sys/wlc_leakyapstats.c
endif
#endif // endif

#ifdef WL_TVPM
ifeq ($(WL_TVPM),1)
	WLFLAGS += -DWL_TVPM
	WLFLAGS += -DTXPWRBACKOFF
	WLFILES_SRC += src/wl/sys/wlc_tvpm.c
endif
#endif // endif

#ifdef WL_TDMTX
ifeq ($(WL_TDMTX),1)
	WLFLAGS += -DTDMTX
	WLFILES_SRC += src/wl/sys/wlc_tdm_tx.c
endif
#endif // endif

# Excess PMwake
#ifdef WL_EXCESS_PMWAKE
ifeq ($(WL_EXCESS_PMWAKE),1)
	WLFLAGS += -DWL_EXCESS_PMWAKE
endif
#endif // endif

#ifdef TOE
ifeq ($(TOE),1)
	WLFLAGS += -DTOE
	WLFILES_SRC += src/wl/sys/wl_toe.c
endif
#endif // endif

#ifdef ARPOE
ifeq ($(ARPOE),1)
	WLFLAGS += -DARPOE
	WLFILES_SRC += src/wl/sys/wl_arpoe.c
endif
#endif // endif

#ifdef MDNS
ifeq ($(MDNS),1)
	WLFLAGS += -DMDNS
	WLFILES_SRC += src/wl/sys/wl_mdns_main.c
	WLFILES_SRC += src/wl/sys/wl_mdns_common.c
endif
#endif // endif

#ifdef P2PO
ifeq ($(P2PO),1)
	WLFLAGS += -DP2PO
	WLFLAGS += -DWL_EVENTQ
	WLFLAGS += -DBCM_DECODE_NO_ANQP
	WLFLAGS += -DBCM_DECODE_NO_HOTSPOT_ANQP
	WLFLAGS += -DWLWFDS
	WLFILES_SRC += src/wl/sys/wl_p2po.c
	WLFILES_SRC += src/wl/sys/wl_p2po_disc.c
	WLFILES_SRC += src/wl/sys/wl_gas.c
	WLFILES_SRC += src/wl/sys/wl_tmr.c
	WLFILES_SRC += src/wl/sys/bcm_p2p_disc.c
	WLFILES_SRC += src/wl/sys/wl_eventq.c
	WLFILES_SRC += src/wl/gas/src/bcm_gas.c
	WLFILES_SRC += src/wl/encode/src/bcm_decode.c
	WLFILES_SRC += src/wl/encode/src/bcm_decode_gas.c
	WLFILES_SRC += src/wl/encode/src/bcm_decode_ie.c
	WLFILES_SRC += src/wl/encode/src/bcm_decode_anqp.c
	WLFILES_SRC += src/wl/encode/src/bcm_decode_hspot_anqp.c
	WLFILES_SRC += src/wl/encode/src/bcm_decode_p2p.c
	WLFILES_SRC += src/wl/encode/src/bcm_encode.c
	WLFILES_SRC += src/wl/encode/src/bcm_encode_gas.c
	WLFILES_SRC += src/wl/encode/src/bcm_encode_ie.c
	WLFILES_SRC += src/wl/encode/src/bcm_encode_anqp.c
	WLFILES_SRC += src/wl/encode/src/bcm_encode_hspot_anqp.c
endif
#endif // endif

#ifdef P2POELO
ifeq ($(P2POELO),1)
	WLFLAGS += -DP2POELO
	WLFILES_SRC += src/wl/sys/wl_p2po.c
	WLFILES_SRC += src/wl/sys/wl_p2po_disc.c
	WLFILES_SRC += src/wl/sys/wl_tmr.c
	WLFILES_SRC += src/wl/sys/bcm_p2p_disc.c
endif
#endif // endif

#ifdef ANQPO
ifeq ($(ANQPO),1)
	WLFLAGS += -DANQPO
	WLFLAGS += -DWL_EVENTQ
	WLFLAGS += -DBCM_DECODE_NO_ANQP
	WLFLAGS += -DBCM_DECODE_NO_HOTSPOT_ANQP
	WLFILES_SRC += src/wl/sys/wl_anqpo.c
	WLFILES_SRC += src/wl/sys/wl_gas.c
	WLFILES_SRC += src/wl/sys/wl_tmr.c
	WLFILES_SRC += src/wl/sys/wl_eventq.c
	WLFILES_SRC += src/wl/gas/src/bcm_gas.c
	WLFILES_SRC += src/wl/encode/src/bcm_decode.c
	WLFILES_SRC += src/wl/encode/src/bcm_decode_gas.c
	WLFILES_SRC += src/wl/encode/src/bcm_decode_ie.c
	WLFILES_SRC += src/wl/encode/src/bcm_decode_p2p.c
	WLFILES_SRC += src/wl/encode/src/bcm_encode.c
	WLFILES_SRC += src/wl/encode/src/bcm_encode_gas.c
	WLFILES_SRC += src/wl/encode/src/bcm_encode_ie.c
endif
#endif // endif

#ifdef WLC_TSYNC
ifeq ($(WLC_TSYNC), 1)
	WLFLAGS += -DWLC_TSYNC
	WLFILES_SRC += src/wl/sys/wlc_tsync.c
endif
#endif // endif

#ifdef BDO
ifeq ($(BDO),1)
	WLFLAGS += -DBDO
	WLFILES_SRC += src/wl/sys/wl_bdo.c
	WLFILES_SRC += src/wl/sys/wl_mdns_main.c
	WLFILES_SRC += src/wl/sys/wl_mdns_common.c
endif
#endif // endif

#ifdef TKO
ifeq ($(TKO),1)
	WLFLAGS += -DTKO
	WLFILES_SRC += src/wl/sys/wl_tko.c
endif
#endif // endif

#ifdef ICMP
ifeq ($(ICMP),1)
	WLFLAGS += -DICMP
	WLFILES_SRC += src/wl/sys/wl_icmp.c
endif
#endif // endif

#ifdef WLNDOE
ifeq ($(WLNDOE),1)
	WLFLAGS += -DWLNDOE -DWLNDOE_RA
	WLFILES_SRC += src/wl/sys/wl_ndoe.c
endif
#endif // endif

#ifdef WL_BWTE
ifeq ($(WL_BWTE),1)
        WLFLAGS += -DWL_BWTE
        WLFILES_SRC += src/wl/sys/wlc_bwte.c
endif
#endif // endif

#ifdef WL_TBOW
ifeq ($(WL_TBOW),1)
	WLFLAGS += -DWL_TBOW
	WLFILES_SRC += src/wl/sys/wlc_tbow.c
	WLFILES_SRC += src/wl/sys/wlc_homanager.c
endif
#endif // endif

#msch
ifeq ($(MSCH_PROFILER),1)
	WLFLAGS += -DMSCH_PROFILER
endif
ifeq ($(MSCH_EVENT_LOG),1)
	WLFLAGS += -DMSCH_EVENT_LOG
endif
ifeq ($(MSCH_TESTING),1)
	WLFLAGS += -DMSCH_TESTING
endif

#ifdef WL_SHIF
ifeq ($(WL_SHIF),1)
	WLFLAGS += -DWL_SHIF
	WLFILES_SRC += src/wl/sys/wl_shub.c
endif
#endif // endif

#ifdef PCOEM_LINUXSTA
ifeq ($(PCOEM_LINUXSTA),1)
	WLFLAGS += -DPCOEM_LINUXSTA
endif
#endif // endif

#ifdef LINUXSTA_PS
ifeq ($(LINUXSTA_PS),1)
	WLFLAGS += -DLINUXSTA_PS
endif
#endif // endif

#ifdef OPENSRC_IOV_IOCTL
ifeq ($(OPENSRC_IOV_IOCTL),1)
	WLFLAGS += -DOPENSRC_IOV_IOCTL
endif
#endif // endif

ifeq ($(PACKET_FILTER),1)
	WLFILES_SRC += src/wl/sys/wlc_pkt_filter.c
	WLFLAGS += -DPACKET_FILTER
	ifeq ($(PACKET_FILTER2),1)
		WLFLAGS += -DPACKET_FILTER2
		ifeq ($(PACKET_FILTER2_DEBUG),1)
			WLFLAGS += -DPF2_DEBUG
		endif
		ifeq ($(APF),1)
			WLFILES_SRC += components/apf/src/apf_interpreter.c
			WLFLAGS += -DAPF
			WLFLAGS += -DAPF_FRAME_HEADER_SIZE=14
		endif
		ifeq ($(PACKET_FILTER6),1)
			WLFLAGS += -DPACKET_FILTER6
		endif
	endif
endif

ifeq ($(SEQ_CMDS),1)
	WLFLAGS += -DSEQ_CMDS
	WLFILES_SRC += src/wl/sys/wlc_seq_cmds.c
endif

ifeq ($(OTA_TEST),1)
	WLFLAGS += -DWLOTA_EN
	WLFILES_SRC += src/wl/sys/wlc_ota_test.c
endif

ifeq ($(WLSWDIV),1)
	WLFLAGS += -DWLC_SW_DIVERSITY
	WLFILES_SRC += src/wl/sys/wlc_swdiv.c
endif

ifeq ($(WLC_TXPWRCAP),1)
	WLFLAGS += -DWLC_TXPWRCAP
endif

ifeq ($(RECEIVE_THROTTLE),1)
	WLFLAGS += -DWL_PM2_RCV_DUR_LIMIT
endif

ifeq ($(DEBUG),1)
	ifeq ($(ASYNC_TSTAMPED_LOGS),1)
		WLFLAGS += -DBCMTSTAMPEDLOGS
	endif
endif

ifeq ($(WL11K_AP),1)
	WLFLAGS += -DWL11K_AP
	WL11K := 1
endif
ifeq ($(WL11K),1)
	WLFLAGS += -DWL11K
	WLFILES_SRC += src/wl/sys/wlc_rrm.c
ifeq ($(WL11K_ALL_MEAS),1)
	WLFLAGS += -DWL11K_ALL_MEAS
	WL11K_BCN_MEAS := 1
	WL11K_NBR_MEAS := 1
endif
ifeq ($(WL11K_BCN_MEAS),1)
	WLFLAGS += -DWL11K_BCN_MEAS
endif
ifeq ($(WL11K_NBR_MEAS),1)
	WLFLAGS += -DWL11K_NBR_MEAS
endif
endif

ifeq ($(WLWNM_AP),1)
	WLWNM := 1
	WLFLAGS += -DWLWNM_AP
endif

ifeq ($(WLWNM_BRCM),1)
	WLWNM := 1
	WLFLAGS += -DWLWNM_BRCM
endif

ifeq ($(WLWNM),1)
	WLFLAGS += -DWLWNM
	ifeq ($(STA),1)
		ifneq ($(WLWNM_AP),1)
			KEEP_ALIVE = 1
		endif
	endif
	WLFILES_SRC += src/wl/sys/wlc_wnm.c
	WLFILES_SRC += src/shared/bcm_l2_filter.c
endif

# BCMULP
ifeq ($(BCMULP),1)
	FCBS := 1
	USE_FCBS_ROM := 1
	WLFLAGS += -DBCMULP
	WLFLAGS += -DWOWL_OS_OFFLOADS
	WLFILES_SRC += src/wl/sys/wlc_ulp.c
	WLFILES_SRC += src/shared/ulp.c
	WLFILES_SRC += src/shared/hndmem.c
endif

ifeq ($(FCBS),1)
	WLFLAGS += -DBCMFCBS
	WLFILES_SRC += src/shared/fcbs.c
	WLFILES_SRC += src/shared/fcbsutils.c
	WLFILES_SRC += src/shared/hndd11.c
	ifeq ($(BCMULP),1)
		WLFLAGS += -DULP_DUMP
		WLFILES_SRC += src/shared/fcbsdata_pmu.c
		WLFILES_SRC += src/wl/sys/wlc_fcbsdata_d11.c
	endif
	ifeq ($(USE_FCBS_ROM),1)
		WLFLAGS += -DUSE_FCBS_ROM
		WLFILES_SRC += fcbs_metadata.c
		WLFILES_SRC += fcbs_ram_data.c
	endif
endif

ifeq ($(WNM_BSSTRANS_EXT),1)
	WLFLAGS += -DWNM_BSSTRANS_EXT
endif

ifeq ($(SLAVE_RADAR),1)
	WLFLAGS += -DSLAVE_RADAR
endif

ifeq ($(WL_MBO),1)
	ifeq ($(MBO_STA),1)
		WLFLAGS += -DWL_MBO_OCE
		WLFLAGS += -DWL_MBO
		ifeq ($(WL_MBO_WFA_CERT),1)
			WLFLAGS += -DWL_MBO_WFA_CERT
		endif
		WLFILES_SRC += src/wl/mbo_oce/src/wlc_mbo_oce.c
		WLFILES_SRC += src/wl/mbo_oce/src/wlc_mbo.c
	endif
	ifeq ($(MBO_AP),1)
		WLFLAGS += -DWL_MBO
		WLFLAGS += -DMBO_AP
		WLFLAGS += -DWL_EVENTQ
		WLFLAGS += -DWL_GLOBAL_RCLASS
		WLFLAGS += -DWL_MBO_OCE
		WLFILES_SRC += src/wl/sys/wl_gas.c
		WLFILES_SRC += src/wl/sys/wl_tmr.c
		WLFILES_SRC += src/wl/sys/wl_eventq.c
		WLFILES_SRC += src/wl/gas/src/bcm_gas.c
		WLFILES_SRC += src/wl/encode/src/bcm_decode.c
		WLFILES_SRC += src/wl/encode/src/bcm_decode_gas.c
		WLFILES_SRC += src/wl/encode/src/bcm_decode_ie.c
		WLFILES_SRC += src/wl/encode/src/bcm_encode.c
		WLFILES_SRC += src/wl/encode/src/bcm_encode_gas.c
		WLFILES_SRC += src/wl/encode/src/bcm_encode_ie.c
		WLFILES_SRC += src/wl/encode/src/bcm_encode_anqp.c
		WLFILES_SRC += src/wl/encode/src/bcm_decode_anqp.c
		WLFILES_SRC += src/wl/encode/src/bcm_decode_hspot_anqp.c
		WLFILES_SRC += src/wl/mbo_oce/src/wlc_mbo_ap.c
		WLFILES_SRC += src/wl/mbo_oce/src/wlc_mbo_oce.c
	endif
endif

ifeq ($(WL_OCE),1)
	WLFLAGS += -DWL_MBO_OCE
	WLFLAGS += -DWL_OCE
	WLFILES_SRC += src/wl/mbo_oce/src/wlc_mbo_oce.c
	WLFILES_SRC += src/wl/mbo_oce/src/wlc_oce.c
	ifeq ($(AP),1)
		WLFLAGS += -DWL_OCE_AP
	else
		ifeq ($(APSTA),1)
			WLFLAGS += -DWL_OCE_AP
		endif
	endif
	# test bench features
	ifeq ($(WL_OCE_TB),1)
		WLFLAGS += -DWL_OCE_TB
	endif
endif

ifeq ($(WL_FILS),1)
	WLFLAGS += -DWL_FILS
	WLFILES_SRC += src/wl/sys/wlc_fils.c
endif

ifeq ($(WL_ESP),1)
	WLFLAGS += -DWL_ESP
	ifeq ($(AP),1)
		WLFLAGS += -DWL_ESP_AP -DWL_ESP_AP_STATIC
	endif
	WLFILES_SRC += src/wl/sys/wlc_esp.c
endif

ifeq ($(WL_MBO_WFA_CERT),1)
	WLFLAGS += -DWL_MBO_WFA_CERT
endif

# Debug feature file
ifeq ($(WL_STATS), 1)
	WLFLAGS += -DWL_STATS
	WLFILES_SRC += src/wl/sys/wlc_stats.c
endif

ifeq ($(DBG_LINKSTAT), 1)
	WLFILES_SRC += src/wl/sys/wlc_linkstats.c
endif

ifeq ($(WL_LINKSTAT), 1)
	WLFLAGS += -DWL_LINKSTAT
	WLFILES_SRC += src/wl/sys/wlc_linkstats.c
endif

ifeq ($(KEEP_ALIVE),1)
	WLFLAGS += -DKEEP_ALIVE
	WLFILES_SRC += src/wl/sys/wl_keep_alive.c
endif

# wl patch code
ifneq ($(WLPATCHFILE), )
	WLFLAGS += -DWLC_PATCH
	WLFILES_SRC += $(WLPATCHFILE)
endif

ifeq ($(WLC_PATCH_IOCTL),1)
	WLFLAGS += -DWLC_PATCH_IOCTL
endif

ifeq ($(SAMPLE_COLLECT),1)
	WLFLAGS += -DSAMPLE_COLLECT
endif

ifeq ($(SMF_STATS),1)
	WLFLAGS += -DSMF_STATS
	WLFILES_SRC += src/wl/sys/wlc_smfs.c
endif

#ifdef PHYMON
ifeq ($(PHYMON),1)
	WLFLAGS += -DPHYMON
endif
#endif // endif

#ifdef BCM_DCS
ifeq ($(BCM_DCS),1)
	WLFLAGS += -DBCM_DCS
endif
#endif // endif

ifeq ($(WLMCHAN), 1)
	WLFLAGS += -DWLTXPWR_CACHE
	WLFLAGS += -DWLTXPWR_CACHE_PHY_ONLY
endif

ifeq ($(WL_THREAD),1)
	WLFLAGS += -DWL_THREAD
endif

ifneq ($(WL_THREADNICE),)
	WLFLAGS += -DWL_THREADNICE=$(WL_THREADNICE)
endif

ifeq ($(WL_NVRAM_FILE),1)
	WLFLAGS += -DWL_NVRAM_FILE
endif

ifeq ($(WL_WOWL_MEDIA),1)
	WLFLAGS += -DWL_WOWL_MEDIA
endif

ifeq ($(WL_LTR),1)
	WLFLAGS += -DWL_LTR
	WLFILES_SRC += src/wl/sys/wlc_ltr.c
endif

ifeq ($(BCM_BACKPLANE_TIMEOUT),1)
	WLFLAGS += -DBCM_BACKPLANE_TIMEOUT
endif

ifeq ($(WL_PRE_AC_DELAY_NOISE_INT),1)
	WLFLAGS += -DWL_PRE_AC_DELAY_NOISE_INT
endif

#ifdef SAVERESTORE
ifeq ($(SAVERESTORE),1)
	WLFLAGS += -DSAVERESTORE
	SR_ESSENTIALS := 1
endif

ifeq ($(SRFAST),1)
	WLFLAGS += -DSRFAST
endif

# minimal functionality required to operate SR engine. Examples: sr binary download, sr enable.
ifeq ($(SR_ESSENTIALS),1)
	WLFLAGS += -DSR_ESSENTIALS
	WLFILES_SRC += src/shared/sr_array.c
	WLFILES_SRC += src/shared/saverestore.c
endif
#endif // endif

# Enable SR Power Control
ifeq ($(BCMSRPWR),1)
	WLFLAGS += -DBCMSRPWR
endif

#ifdef SR_DEBUG
ifeq ($(SR_DEBUG),1)
	WLFLAGS += -DSR_DEBUG
endif
#endif // endif

#ifdef BCM_REQUEST_FW
ifeq ($(BCM_REQUEST_FW), 1)
	WLFLAGS += -DBCM_REQUEST_FW
endif
#endif // endif

#ifdef BCM_UCODE_FILES
ifeq ($(BCM_UCODE_FILES), 1)
        WLFLAGS += -DBCM_UCODE_FILES
endif
#endif // endif

# HW CSO support (D11 rev40 feature)
ifeq ($(WLCSO),1)
	WLFLAGS += -DWLCSO
endif

# add a flag to indicate the split to linux kernels
WLFLAGS += -DPHY_HAL

# compile only 1x1 ACPHY related code
ifeq ($(ACPHY_1X1_ONLY),1)
WLFLAGS += -DACPHY_1X1_ONLY
endif

#ifdef WET_TUNNEL
ifeq ($(WET_TUNNEL),1)
	WLFLAGS += -DWET_TUNNEL
	WLFILES_SRC += src/wl/sys/wlc_wet_tunnel.c
endif
#endif // endif

#ifdef WLDURATION
ifeq ($(WLDURATION),1)
	WLFLAGS += -DWLDURATION
	WLFILES_SRC += src/wl/sys/wlc_duration.c
endif
#endif // endif

# Memory optimization. Use functions instead of macros for bit operations.
ifeq ($(BCMUTILS_BIT_MACROS_USE_FUNCS),1)
	WLFLAGS += -DBCMUTILS_BIT_MACROS_USE_FUNCS
endif

# Disable MCS32 in 40MHz
ifeq ($(DISABLE_MCS32_IN_40MHZ),1)
	WLFLAGS += -DDISABLE_MCS32_IN_40MHZ
endif

#ifdef WLPM_BCNRX
ifeq ($(WLPM_BCNRX),1)
	WLFLAGS += -DWLPM_BCNRX
endif
#endif // endif

#ifdef WLSCAN_PS
ifeq ($(WLSCAN_PS),1)
	WLFLAGS += -DWLSCAN_PS
endif
#endif // endif

#Disable aggregation for Voice traffic
#ifdef WL_DISABLE_VO_AGG
ifeq ($(WL_DISABLE_VO_AGG),1)
	WLFLAGS += -DWL_DISABLE_VO_AGG
endif
#endif // endif

#ifdef TINY_PKTJOIN
ifeq ($(TINY_PKTJOIN),1)
	WLFLAGS += -DTINY_PKTJOIN
endif
#endif // endif

#ifdef WL_RXEARLYRC
ifeq ($(WL_RXEARLYRC),1)
	WLFLAGS += -DWL_RXEARLYRC
endif
#endif // endif

#ifdef WLMCHAN
#ifdef PROP_TXSTATUS
ifeq ($(WLMCHAN),1)
ifeq ($(PROP_TXSTATUS),1)
	WLFLAGS += -DROBUST_DISASSOC_TX
		WLFLAGS += -DWLMCHANPRECLOSE
		WLFLAGS += -DBBPLL_PARR
	endif
endif
#endif  ## PROP_TXSTATUS
#endif  ## WLMCHAN

#ifdef WLRXOV
ifeq ($(WLRXOV),1)
	WLFLAGS += -DWLRXOV
endif
#endif // endif

ifeq ($(PKTC),1)
	WLFLAGS += -DPKTC
endif

ifeq ($(PKTC_DONGLE),1)
	WLFLAGS += -DPKTC_DONGLE
endif

#ifdef BCMPKTPOOL
# Packet Pool
ifeq ($(BCMPKTPOOL),1)
	WLFLAGS += -DBCMPKTPOOL
endif	# BCMPKTPOOL
#endif // endif

ifeq ($(WLHEB), 1)
        WLFLAGS += -DWLHEB
        WLFILES_SRC += src/wl/sys/wlc_heb.c
        WLFILES_SRC += src/wl/sys/wlc_hw_heb.c
endif

ifeq ($(WL_EAP_AP),1)
    WLFLAGS += -DWL_EAP_AP
endif

ifeq ($(WL_EAP_PER_VAP_CONFIG),1)
	WLFLAGS += -DWL_EAP_PER_VAP_CONFIG
endif

ifeq ($(WL_EAP_80211RAW),1)
	WLFLAGS += -DWL_EAP_80211RAW
endif

ifeq ($(WL_EAP_80211RAW_TEST),1)
	WLFLAGS += -DWL_EAP_80211RAW_TEST
endif

ifeq ($(WL_EAP_PER_VAP_DTIM),1)
	WLFLAGS += -DWL_EAP_PER_VAP_DTIM
endif

ifeq ($(WL_EAP_PER_VAP_AMSDU_HWDAGG_DIS),1)
	WLFLAGS += -DWL_EAP_PER_VAP_AMSDU_HWDAGG_DIS
endif

ifeq ($(WL_EAP_RXTX_UTILIZATION),1)
	WLFLAGS += -DWL_EAP_RXTX_UTILIZATION
endif

ifeq ($(WL_EAP_SAMPLE_COLLECT),1)
	WLFLAGS += -DWL_EAP_SAMPLE_COLLECT
endif

ifeq ($(WL_EAP_PRS_RTX_LIMIT),1)
        WLFLAGS += -DWL_EAP_PRS_RTX_LIMIT
endif

ifeq ($(WL_EAP_STATS),1)
        WLFLAGS += -DWL_EAP_STATS -DBCMDBG_AMPDU
endif

ifeq ($(WL_EAP_SNR),1)
        WLFLAGS += -DWL_EAP_SNR
endif

ifeq ($(WL_EAP_OBJMEM_DBG),1)
        WLFLAGS += -DWL_EAP_OBJMEM_DBG
endif

ifeq ($(WL_EAP_FFT_SAMPLE),1)
	WLFLAGS += -DWL_EAP_FFT_SAMPLE
endif

ifeq ($(WL_EAP_ACK_RSSI),1)
	WLFLAGS += -DWL_EAP_ACK_RSSI
endif

ifeq ($(WLBSSLOAD_REPORT),1)
	WLFLAGS += -DWLBSSLOAD_REPORT
	WLFILES_SRC += src/wl/sys/wlc_bssload.c
endif

# XXX this is a temporary build flag which is going to be got rid of and consolidated to
# the existing PKTC_DONGLE once this additional feature is proven to be stable
ifeq ($(PKTC_TX_DONGLE),1)
	WLFLAGS += -DPKTC_TX_DONGLE
endif

# Secure WiFi through NFC
ifeq ($(WLNFC),1)
	WLFLAGS += -DWLNFC
	WLFILES_SRC += src/wl/sys/wl_nfc.c
endif

ifeq ($(BCM_NFCIF),1)
	WLFLAGS += -DBCM_NFCIF_ENABLED
	WLFLAGS += -DDISABLE_CONSOLE_UART_OUTPUT
	WLFILES_SRC += src/shared/bcm_nfcif.c
endif

ifeq ($(AMPDU_HOSTREORDER), 1)
	WLFLAGS += -DBRCMAPIVTW=128 -DWLAMPDU_HOSTREORDER
endif

# Append bss_info_t to selected events
ifeq ($(EVDATA_BSSINFO), 1)
	WLFLAGS += -DWL_EVDATA_BSSINFO
endif

# code optimisation for non dual phy chips
ifeq ($(DUAL_PHY_CHIP), 1)
	WLFLAGS += -DDUAL_PHY
endif

# XXX dongle/rte/sim uses a different define
ifndef WLCFGDIR
	ifdef WLCFG_DIR
		WLCFGDIR=$(WLCFG_DIR)
	endif
endif
ifndef WLCFGDIR
$(error WLCFGDIR is not defined)
endif

# add keymgmt
ifeq ($(WLWSEC),1)
include $(WLCFGDIR)/keymgmt.mk
WLFILES_SRC += $(KEYMGMT_SRC_FILES)
endif

ifeq ($(WLROAMPROF),1)
	WLFLAGS += -DWLROAMPROF
endif

ifeq ($(SCAN_SUMEVT),1)
	WLFLAGS += -DWLSCAN_SUMEVT
endif
#endif // endif

# PHY modules
ifeq ($(WL),1)
include $(WLCFGDIR)/phy.mk
ifeq ($(WL_PHYLIB),)
WLFILES_SRC += $(PHY_SRC)
endif
WLFLAGS += $(PHY_FLAGS)
endif

#ifdef WL
ifeq ($(WL),1)
# HWA modules
include $(WLCFGDIR)/accel.mk
endif
#endif // endif

# Adaptive Voltage Scaling
ifeq ($(AVS),1)
include $(WLCFGDIR)/avs.mk
WLFILES_SRC += $(AVS_SRC)
WLFLAGS += $(AVS_FLAGS)
endif

# enabling secure DMA feature
ifeq ($(BCM_SECURE_DMA),1)
	WLFLAGS += -DBCM_SECURE_DMA
endif

# ARMV7L
ifeq ($(ARMV7L),1)
	ifeq ($(STBLINUX),1)
		WLFLAGS += -DSTBLINUX
		WLFLAGS += -DSTB
		ifneq ($(BCM_SECURE_DMA),1)
			WLFLAGS += -DBCM47XX
		endif
		ifeq ($(BCM_SECURE_DMA),1)
			WLFILES_SRC += src/shared/stbutils.c
		endif
	endif
endif

#Support for STBC
ifeq ($(WL11N_STBC_RX_ENABLED),1)
	WLFLAGS += -DWL11N_STBC_RX_ENABLED
endif

# ARMV8 (for 64 bit)
ifeq ($(ARMV8),1)
	ifeq ($(STBLINUX),1)
		WLFLAGS += -DSTBLINUX
		WLFLAGS += -DSTB
		ifneq ($(BCM_SECURE_DMA),1)
			WLFLAGS += -DBCM47XX
		endif
		ifeq ($(BCM_SECURE_DMA),1)
			WLFILES_SRC += src/shared/stbutils.c
		endif
	endif
endif

# STB integrated wlan
ifeq ($(STB_SOC_WIFI),1)
	WLFLAGS += -DSTB_SOC_WIFI
	WLFLAGS += -DPLATFORM_INTEGRATED_WIFI
	WLFILES_SRC += src/wl/sys/wl_stbsoc.c
endif

ifeq ($(STBLINUX),1)
	WLFLAGS += -DSTBLINUX
endif

ifeq ($(WL_AUTH_SHARED_OPEN),1)
	WLFLAGS += -DWL_AUTH_SHARED_OPEN
endif

ifeq ($(WLASSOC_NBR_REQ),1)
	EXTRA_DFLAGS += -DWLASSOC_NBR_REQ
endif

# rssi refinement
ifeq ($(WL_RSSIREFINE),1)
	EXTRA_DFLAGS += -DRSSI_REFINE
endif

ifeq ($(LPAS),1)
        EXTRA_DFLAGS += -DLPAS
endif   # LPAS

ifeq ($(PHY_EPAPD_NVRAMCTL),1)
	WLFLAGS += -DEPAPD_SUPPORT_NVRAMCTL=1
endif

# Dynamic bt coex(auto mode sw,desese, pwr ctrl. etc) -
ifeq ($(WL_BTCDYN),1)
	WLFLAGS += -DWL_BTCDYN
endif

# Unified coex manager
ifeq ($(WL_UCM),1)
	WLFLAGS += -DWL_UCM
endif

ifeq ($(WLC_MACDBG_FRAMEID_TRACE),1)
	WLFLAGS += -DWLC_MACDBG_FRAMEID_TRACE
endif

ifeq ($(WLC_MACDBG_TRIGBSR),1)
	WLFLAGS += -DWLC_MACDBG_TRIGBSR
endif

ifeq ($(WLC_RXFIFO_CNT_ENABLE),1)
	WLFLAGS += -DWLC_RXFIFO_CNT_ENABLE
endif

# Enable VASIP
ifeq ($(WLVASIP), 1)
        WLFLAGS += -DWLVASIP
endif

# Enable SMC
ifeq ($(WLSMC), 1)
        WLFLAGS += -DWLSMC
endif

# Datapath Debuggability, Health Check
ifeq ($(WL_DATAPATH_HC),1)
	EXTRA_DFLAGS += -DWL_DATAPATH_HC

        EXTRA_DFLAGS += -DWL_RX_DMA_STALL_CHECK

        EXTRA_DFLAGS += -DWL_TX_STALL -DWL_TX_CONT_FAILURE
        EXTRA_DFLAGS += -DWLC_TXSTALL_FULL_STATS

        EXTRA_DFLAGS += -DWL_TXQ_STALL

        EXTRA_DFLAGS += -DWL_RX_STALL

        EXTRA_DFLAGS += -DWL_SCAN_STALL_CHECK
endif

ifeq ($(WLOCL),1)
	EXTRA_DFLAGS += -DOCL
endif

#Arbitartor
ifeq ($(WL_STF_ARBITRATOR),1)
    EXTRA_DFLAGS += -DWL_STF_ARBITRATOR
endif

# MIMOPS
ifeq ($(WL_MIMOPS),1)
    WLFLAGS += -DWL_MIMOPS
    EXTRA_DFLAGS += -DWL_MIMOPS_CFG
endif

# MIMO SISO stats
ifeq ($(WL_MIMO_SISO_STATS),1)
	ifeq ($(WL_PWRSTATS),1)
		EXTRA_DFLAGS += -DWL_MIMO_SISO_STATS
	endif
endif

# Datapath Debuggability, option to prevent ASSERT on check
ifeq ($(WL_DATAPATH_HC_NOASSERT),1)
	EXTRA_DFLAGS += -DWL_DATAPATH_HC_NOASSERT
endif

ifeq ($(WL_DATAPATH_LOG_DUMP),1)
	WLFLAGS += -DWL_DATAPATH_LOG_DUMP
endif

# SmartAmpdu (TX DTS suppression)
ifeq ($(WL_DTS),1)
	WLFLAGS += -DWL_DTS
endif
# cached flow processing
ifeq ($(WLCFP),1)
	WLFLAGS += -DWLCFP
	# Enable manually as per use case
	#WLFLAGS += -DCFP_DEBUG
	WLFILES_SRC += src/wl/sys/wlc_cfp.c
endif

# bulk DMA processing
ifeq ($(BULKDMA),1)
	WLFLAGS += -DBULK_PKTLIST
#enable debug if needed
#	WLFLAGS += -DBULK_PKTLIST_DEBUG
endif

ifeq ($(BULKRXDMA),1)
	WLFLAGS += -DBULKRX_PKTLIST
endif

# d11 core outputting phyrxsts over rx fifo 3 (>=129 && !130)
ifeq ($(STS_FIFO_RXEN),1)
	WLFLAGS += -DSTS_FIFO_RXEN
endif

# m2m core reading TXStatus out of d11 memory and DMA'ing it (>=130)
ifeq ($(WLC_OFFLOADS_TXSTS),1)
        WLFLAGS += -DWLC_OFFLOADS_TXSTS
	WLC_OFFLOADS_STATUS := 1
endif
# m2m core reading phyrxsts out of d11 memory and DMA'ing it (>=130)
ifeq ($(WLC_OFFLOADS_RXSTS),1)
        WLFLAGS += -DWLC_OFFLOADS_RXSTS
	WLC_OFFLOADS_STATUS := 1
endif
ifeq ($(WLC_OFFLOADS_STATUS),1)
	WLFILES_SRC += src/wl/sys/wlc_offld.c
endif

# Single stage Queuing and Scheduling
ifeq ($(WLSQS),1)
	WLFLAGS += -DWLSQS
ifeq ($(WLATF),1)
	WLFLAGS += -DRAVG_SIMPLE
endif
	WLFILES_SRC += src/wl/sys/wlc_sqs.c
endif

ifeq ($(WL_MCAST_FILTER_NOSTA),1)
	WLFLAGS += -DWL_MCAST_FILTER_NOSTA
endif

# trunk uses bcmcrypto component
WLFLAGS += -DBCMCRYPTO_COMPONENT

# Rate and Link Memory support
ifeq ($(WL_RATELINKMEM),1)
	WLFLAGS += -DWL_RATELINKMEM
	WLFILES_SRC += src/wl/sys/wlc_ratelinkmem.c
endif

ifeq ($(WLC_BAM),1)
	WLFLAGS += -DWLC_BAM
	WLFILES_SRC += src/wl/sys/wlc_bad_ap_manager.c
endif

ifeq ($(WLC_ADPS),1)
	WLFLAGS += -DWLC_ADPS
	WLFILES_SRC += src/wl/sys/wlc_adps.c
endif

# 6GHz CLM support - only used in CLM API code!
ifeq ($(BAND6G),1)
	WLFLAGS += -DWL_BAND6G
endif

ifeq ($(BCM_CSIMON),1)
	WLFILES_SRC += src/wl/sys/wlc_csimon.c
endif

# randomize probe req seq
ifeq ($(WL_PRQ_RAND_SEQ),1)
    WLFLAGS += -DWL_PRQ_RAND_SEQ
endif

ifeq ($(WL_EAP_SCAN_TX),1)
	WLFLAGS += -DWL_EAP_SCAN_TX
endif

ifeq ($(WL_EAP_SCAN_TX_MAX_POWER),1)
	WLFLAGS += -DWL_EAP_SCAN_TX_MAX_POWER
endif

ifeq ($(WL_EAP_STA_SCB_TIMEOUT),1)
	WLFLAGS += -DWL_EAP_STA_SCB_TIMEOUT
endif

ifeq ($(WL_EAP_PER_VAP_CONFIG),1)
	WLFLAGS += -DWL_EAP_PER_VAP_CONFIG
endif

ifeq ($(WL_EAP_UCODE),1)
	WLFLAGS += -DWL_EAP_UCODE
endif

ifeq ($(WL_EAP_UCODE_TX_DESC),1)
	WLFLAGS += -DWL_EAP_UCODE_TX_DESC
endif

ifeq ($(WL_EAP_PER_VAP_PKTC),1)
	WLFLAGS += -DWL_EAP_PER_VAP_PKTC
endif

ifeq ($(WL_EAP_EMSGLVL),1)
	WLFLAGS += -DWL_EAP_EMSGLVL
endif

ifeq ($(WL_EAP_PKTTAG_EXT),1)
	WLFLAGS += -DWL_EAP_PKTTAG_EXT
endif

ifeq ($(WL_EAP_TPDUMP),1)
	WLFLAGS += -DWL_EAP_TPDUMP
endif

ifeq ($(WL_EAP_LAST_PKT_RSSI),1)
	WLFLAGS += -DWL_EAP_LAST_PKT_RSSI
endif

ifeq ($(WL_EAP_EVENT_SERVICE),1)
    WLFLAGS += -DWL_EAP_EVENT_SERVICE
endif

ifeq ($(WL_EAP_CUSTOM_SCAN),1)
    WLFLAGS += -DWL_EAP_CUSTOM_SCAN
    WLFILES_SRC += src/wl/sys/wlc_custom_scan.c
endif

ifeq ($(WL_EAP_SCAN_MEASUREMENT),1)
	WLFLAGS += -DWL_EAP_SCAN_MEASUREMENT
endif

ifeq ($(WL_EAP_SCAN_TEST),1)
	WLFLAGS += -DWL_EAP_SCAN_TEST
endif

ifeq ($(WL_EAP_SCAN_PROTECT),1)
	WLFLAGS += -DWL_EAP_SCAN_PROTECT
endif

ifeq ($(WL_EAP_SCAN_BEACON_DELAY),1)
	WLFLAGS += -DWL_EAP_SCAN_BEACON_DELAY
endif

ifeq ($(WL_EAP_DATA_SNOOP),1)
	WLFLAGS += -DWL_EAP_DATA_SNOOP
endif

ifeq ($(WL_EAP_RUNT_ON_BADFCS_FRM),1)
	WLFLAGS += -D WL_EAP_RUNT_ON_BADFCS_FRM
endif

ifeq ($(WL_EAP_NOISE_MEASUREMENTS),1)
	WLFLAGS += -DWL_EAP_NOISE_MEASUREMENTS
endif

ifeq ($(WL_EAP_FAST_NOISE_MEASURE),1)
	WLFLAGS += -DWL_EAP_FAST_NOISE_MEASURE
endif

ifeq ($(WL_EAP_OLPC),1)
	WLFLAGS += -DWL_EAP_OLPC
endif

ifeq ($(WL_EAP_MONITOR),1)
	WLFLAGS += -DWL_EAP_MONITOR
endif

ifeq ($(WL_EAP_ALLOW_MESH_FRM),1)
	WLFLAGS += -DWL_EAP_ALLOW_MESH_FRM
endif

ifeq ($(WL_EAP_ALLOW_MGMT_FRM),1)
	WLFLAGS += -DWL_EAP_ALLOW_MGMT_FRM
endif

ifeq ($(WL_EAP_ALLOW_SOUND_FB),1)
	WLFLAGS += -DWL_EAP_ALLOW_SOUND_FB
endif

ifeq ($(WL_EAP_PER_VAP_CONFIG_RATESET),1)
	WLFLAGS += -DWL_EAP_PER_VAP_CONFIG_RATESET
endif

ifeq ($(WL_EAP_FIPS_LOOPBACK),1)
	WLFLAGS += -DWL_EAP_FIPS_LOOPBACK
endif

ifeq ($(WL_EAP_DROP_RX_MGMT_RSSI),1)
    WLFLAGS += -DWL_EAP_DROP_RX_MGMT_RSSI
    WLFILES_SRC += src/wl/sys/wlc_filter_rx_mgmt_ctl_rssi.c
endif

# Support for board-specific hardware RF analog band-pass filters for
# 5G channel isolation
ifeq ($(WL_EAP_BOARD_RF_5G_FILTER),1)
	WLFLAGS += -DWL_EAP_BOARD_RF_5G_FILTER
endif

ifeq ($(WL_EAP_KEY_CACHE),1)
	WLFLAGS += -DWL_EAP_KEY_CACHE
endif

ifeq ($(WL_EAP_CUST_EVENT_HNDLR),1)
	WLFLAGS += -DWL_EAP_CUST_EVENT_HNDLR
	WLFILES_SRC += src/wl/sys/wlc_customer_event_handler.c
endif

ifeq ($(WL_EAP_OUTDOOR_AP),1)
	WLFLAGS += -DWL_EAP_OUTDOOR_AP
endif

ifeq ($(WL_EAP_AP1),1)
	WLFLAGS += -DWL_EAP_AP1
endif

ifeq ($(WL_EAP_BCM43570),1)
	WLFLAGS += -DWL_EAP_BCM43570
endif

ifeq ($(WL_EAP_VHT_PROPRIETARY_RATES_DIS),1)
	WLFLAGS += -DWL_EAP_VHT_PROPRIETARY_RATES_DIS
endif

# Instead of disabling frameburst completly in dynamic frame burst logic,
# we enable RTS/CTS in frameburst.
ifeq ($(FRAMEBURST_RTSCTS_PER_AMPDU),1)
	WLFLAGS += -DFRAMEBURST_RTSCTS_PER_AMPDU
endif

# To tune frameburst override thresholds
ifeq ($(TUNE_FBOVERRIDE),1)
	WLFLAGS += -DTUNE_FBOVERRIDE
endif

# HWA support
ifeq ($(HWA),1)
	# files used both in NIC and firmware builds
	WLFILES_SRC += src/shared/hwa_mac.c
	WLFILES_SRC += src/shared/hwa_lib.c
	WLFLAGS += -DBCMHWA=$(BCMHWA)
ifneq ($(BCMPCI), 0)
	# flags unique to NIC build
    WLFLAGS += -D__WORDSIZE=64
    WLFLAGS += -DHWA_MODE=HWA_NIC_MODE
endif
ifeq ($(HWA1AB),1)
	WLFLAGS += -DBCMHWA1AB
endif
ifeq ($(HWA2A),1)
	WLFLAGS += -DBCMHWA2A
endif
ifeq ($(HWA2B),1)
	WLFLAGS += -DBCMHWA2B
endif
ifeq ($(HWA3AB),1)
	WLFLAGS += -DBCMHWA3AB
endif
ifeq ($(HWA3A),1)
	WLFLAGS += -DBCMHWA3A
endif
ifeq ($(HWA4A),1)
	WLFLAGS += -DBCMHWA4A
endif
ifeq ($(HWA4B),1)
	WLFLAGS += -DBCMHWA4B
endif
ifeq ($(HWAPP),1)
	WLFLAGS += -DBCMHWAPP
endif
ifeq ($(HWADBG),1)
	# hwa_pktpgr.c is for firmware builds only. See Makeconf
	WLFLAGS += -DBCMHWADBG
endif
endif

ifneq ($(HNDM2M),)
	WLFLAGS += -DHNDM2M=$(HNDM2M)
	WLFILES_SRC += src/shared/hndm2m.c
endif

# Enable phytx error logging
ifeq ($(PHYTXERR_DUMP),1)
	WLFLAGS += -DPHYTXERR_DUMP
endif

ifeq ($(TRAFFIC_THRESH),1)
	WLFLAGS += -DWL_TRAFFIC_THRESH
endif

# Sorting has two benefits: it uniqifies the list, which may have
# gotten some double entries above, and it makes for prettier and
# more predictable log output.
WLFILES_SRC := $(sort $(WLFILES_SRC))
# Legacy WLFILES pathless definition, please use new src relative path
# in make files.
WLFILES := $(sort $(notdir $(WLFILES_SRC)))

ifeq ($(strip $(USE_WLAN_SHARED)), 1)
ifneq ($(strip $(WLAN_SHARED_DIR)),)
-include $(WLAN_SHARED_DIR)/wifi_cfg_common.mk
endif
endif

ifeq ($(WL_EAP_DFS_ALTMODE),1)
    WLFLAGS += -DWL_EAP_DFS_ALTMODE
endif

ifeq ($(WL_EAP_WLAN_ONLY_UL_PKTC),1)
    WLFLAGS += -DWL_EAP_WLAN_ONLY_UL_PKTC
endif

ifeq ($(BCM_SKB_FREE_OFFLOAD),1)
    WLFLAGS += -DBCM_SKB_FREE_OFFLOAD
endif

ifeq ($(WL_EAP_PKTCNTR),1)
    WLFLAGS += -DWL_EAP_PKTCNTR
endif

ifeq ($(WL_EAP_POLL_IDLE_STA),1)
    WLFLAGS += -DWL_EAP_POLL_IDLE_STA
endif

ifneq ($(strip $(CONFIG_BCM_PKTRUNNER)),)
    EXTRA_CFLAGS    += -DPLATFORM_WITH_RUNNER
endif

ifeq ($(WL_USE_SUPR_PSQ),1)
    WLFLAGS += -DWL_USE_SUPR_PSQ
endif

ifeq ($(WL_PS_STATS),1)
	WLFLAGS += -DWL_PS_STATS
endif

WLFLAGS += -DWL_REG_SIZECHECK

ifeq ($(WL_VASIP_MU_INFO),1)
	WLFLAGS += -DWL_VASIP_MU_INFO
endif

ifeq ($(WL_TXPKTPEND_SYNC),1)
    WLFLAGS += -DWL_TXPKTPEND_SYNC
endif
