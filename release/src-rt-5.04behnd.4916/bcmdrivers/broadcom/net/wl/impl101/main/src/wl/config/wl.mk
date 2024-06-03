## Helper makefile for building Broadcom wl device driver
# This file maps wl driver feature flags (import) to WLFLAGS and WLFILES_SRC (export).
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
# $Id: wl.mk 831796 2023-10-26 03:56:40Z $

WLFLAGS += -DBPHY_DESENSE
WLFLAGS += -DWL_EXPORT_CURPOWER

# 6GHz CLM support - CLM API code has 6GHz support under this flag.
# As we always want it enabled define it unconditionally here.
WLFLAGS += -DWL_BAND6G

ifdef UCODE_RAM
UCODE_RAM_DIR = components/ucode/dot11_releases/trunk/$(UCODE_RAM)/ram
else
UCODE_RAM_DIR = src/wl/sys
endif

ifeq ($(WLTEST),1)
# all MFGTEST images, as a minimum, should support error messages
DBG_ERROR:=1
endif

# all images, as a minimum, should support 'assoc light' messages, which provides basic association
# messages with './wl msglevel +assoc'
WLFLAGS += -DWLMSG_ASSOC_LT

ifeq ($(NO_BCMDBG_ASSERT), 1)
	WLFLAGS += -DNO_BCMDBG_ASSERT
endif

# debug/internal
ifeq ($(DEBUG),1)
	ifeq ($(CMWIFI),)
		WLFLAGS += -DBCMDBG -DRWL_WIFI -DWLRWL -DWL_EXPORT_CURPOWER
		WLRWL = 1
		ifeq ($(STBLINUX),1)
			ifeq ($(WLTEST),1)
				WLFLAGS += -DWLTEST
			endif
		else
			WLFLAGS += -DWLTEST
		endif
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
	endif
#endif
endif

#ifdef BCMDBG_TXSTALL
ifeq ($(BCMDBG_TXSTALL),1)
	WLFLAGS += -DBCMDBG_TXSTALL
endif
#endif

#ifdef BCMDBG
ifeq ($(BCMDBG),1)
	WLFLAGS += -DWL_EXPORT_CURPOWER
endif
#endif

ifeq ($(DBG_ERROR),1)
	WLFLAGS += -DBCMDBG_ERR
endif

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
	WLOSEN = 1
endif

WLFLAGS += -DPPR_API

ifdef BCMPHYCORENUM
	WLFLAGS += -DBCMPHYCORENUM=$(BCMPHYCORENUM)
endif

ifeq ($(WL_AP_CHAN_CHANGE_EVENT),1)
	WLFLAGS += -DWL_AP_CHAN_CHANGE_EVENT
endif

ifeq ($(BCM_CEVENT),1)
	WLFLAGS += -DBCM_CEVENT
endif

ifeq ($(BCM_ARP_CEVENT),1)
	WLFLAGS += -DBCM_ARP_CEVENT
endif

ifeq ($(WL_SPLIT_ASSOC),1)
	WLFLAGS += -DSPLIT_ASSOC
endif

#ifdef BCM_DMA_CT
ifeq ($(BCM_DMA_CT),1)
	WLFLAGS += -DBCM_DMA_CT
endif
#endif

#ifdef BCM_DMA_INDIRECT
ifeq ($(BCM_DMA_INDIRECT),1)
	WLFLAGS += -DBCM_DMA_INDIRECT
endif
#endif

ifeq ($(DBG_HEAPCHECK),1)
	WLFLAGS += -DBCMDBG_HEAPCHECK
endif

ifeq ($(DBG_HEAP_SEC_TEST),1)
	WLFLAGS += -DDBG_HEAP_SEC_TEST
endif

ifeq ($(WLATM_PERC),1)
	WLFLAGS += -DWLATM_PERC
endif

#ifdef DONGLEBUILD
ifeq ($(FULLDNGLBLD),1)
	WLFILES_SRC	+= src/wl/sys/wlc_pktfetch.c
	ifeq ($(BME_PKTFETCH),1)
		WLFLAGS += -DBME_PKTFETCH
	endif
#ifdef BCM_HEALTH_CHECK
	WLFILES_SRC     += src/wl/sys/wlc_health_check.c
	WLFILES_SRC     += src/wl/sys/wlc_hc_datastall.c
	WLFILES_SRC     += src/wl/sys/wlc_hc_sounding.c
#endif
endif
#endif

#if 1
ifeq ($(NO_BCMINTERNAL),1)
#ifndef DONGLEBUILD
ifeq ($(FULLDNGLBLD),)
WLFLAGS += -DUL_RU_STATS_DUMP -DDL_RU_STATS_DUMP
endif
endif

#ifdef CHAN_SWITCH_HIST
ifeq ($(CHAN_SWITCH_HIST),1)
	WLFLAGS += -DCHAN_SWITCH_HIST
endif
#endif

#ifdef PKTQ_LOG
ifeq ($(PKTQ_LOG),1)
	WLFLAGS += -DPKTQ_LOG
	WLFILES_SRC += src/wl/sys/wlc_rx_report.c
	ifeq ($(SCB_BS_DATA),1)
		WLFILES_SRC += src/wl/sys/wlc_bs_data.c
		WLFLAGS += -DSCB_BS_DATA
	endif
	ifeq ($(TXRX_SUMMARY),1)
		WLFLAGS += -DTXRX_SUMMARY
	endif
endif
#endif

#ifdef PKTQ_STATUS
ifeq ($(PKTQ_STATUS),1)
	WLFLAGS += -DPKTQ_STATUS
endif
#endif

ifeq ($(PSPRETEND),1)
	WLFLAGS += -DPSPRETEND
	WLFLAGS += -DWL_CS_PKTRETRY
	WLFILES_SRC += src/wl/sys/wlc_pspretend.c
endif

#ifdef BCMDBG_TRAP
# CPU debug traps (lomem access, divide by 0, etc) are enabled except when mogrified out for
# external releases.
WLFLAGS += -DBCMDBG_TRAP
#endif

## wl driver common

ifeq ($(GTK_RESET),1)
	WLFLAGS += -DGTK_RESET
endif

#ifdef BCMQT
ifndef BCMQT
	BCMQT := 0
endif
ifeq ($(BCMQT),1)
	WLFLAGS += -DBCMQT=1 # full chip veloce emulation
else ifeq ($(BCMQT),2)
	WLFLAGS += -DBCMQT=2 # WLAN core only veloce emulation
endif
ifneq ($(BCMQT),0)
	WLFLAGS += -DBCMSLTGT
	ifeq ($(WLRTE),1)
		# Use of RTE implies embedded (CPU emulated)
		WLFLAGS += -DBCMQT_CPU
	endif
	# Disable Radar function in emulated platform
	WLC_DISABLE_DFS_RADAR_SUPPORT = 1
endif
#endif

#ifdef BCMQT_PHYLESS
ifeq ($(BCMQT_PHYLESS),1)
	WLFLAGS += -DBCMQT_PHYLESS
endif
#endif

#ifdef BCMQT_COMBO
ifeq ($(BCMQT_COMBO),1)
	WLFLAGS += -DBCMQT_COMBO
endif
#endif

# 6G operation requires Spectrum Management and global operating classes
ifeq ($(BAND6G),1)
	WLFLAGS += -DWL_GLOBAL_RCLASS
	WL11H = 1
endif

ifeq ($(WL_EAP_STATS),1)
	WLFLAGS += -DWL_EAP_STATS -DWL_BSS_STATS -DBCMDBG_AMPDU
endif

#ifdef WL
ifeq ($(WL),1)
	WLFILES_SRC += src/shared/bcmwifi/src/bcmwifi_rclass.c
	WLFILES_SRC += src/shared/bcmevent.c
	WLFILES_SRC += src/shared/bcm_notif.c
	WLFILES_SRC += src/shared/bcmiov.c
	WLFILES_SRC += src/shared/bcmstdlib_s.c
	WLFILES_SRC += src/wl/sys/wlc_alloc.c
	WLFILES_SRC += src/wl/sys/wlc_intr.c
	WLFILES_SRC += src/wl/sys/mac_intr.c
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
	WLFILES_SRC += src/wl/sys/wlc_rate_def.c
	WLFILES_SRC += src/wl/sys/wlc_vasip.c
	WLFILES_SRC += src/wl/sys/wlc_smc.c
	WLFILES_SRC += src/wl/sys/d11smc_code.c
	WLFILES_SRC += src/wl/sys/wlc_wareng.c
	WLFILES_SRC += src/wl/sys/d11wareng_code.c
	WLFILES_SRC += src/wl/sys/d11lmac_code.c
	WLFILES_SRC += src/wl/sys/wlc_phydsp.c
	WLFILES_SRC += src/wl/sys/d11phydsp_code.c
	WLFILES_SRC += src/wl/sys/wlc_ratelinkmem.c
	WLFILES_SRC += src/wl/sys/wlc_ampdu.c
	WLFILES_SRC += src/wl/sys/wlc_ampdu_rx.c
	WLFILES_SRC += src/wl/sys/wlc_ampdu_cmn.c
	ifeq ($(BCMDBG_AMPDU),1)
		# Debug for: wl dump ampdu; wl ampdu_clear_dump
		WLFLAGS += -DBCMDBG_AMPDU
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
#endif

	ifeq ($(WL_EAP_UCODE),1)
		WLFILES_SRC += $(UCODE_RAM_DIR)/d11ucode_ge40_eap.c
	else
		WLFILES_SRC += $(UCODE_RAM_DIR)/d11ucode_ge40.c
	endif
	WLFILES_SRC += src/wl/sys/d11vasip_code.c
	ifeq ($(WLDIAG),1)
		WLFILES_SRC += $(UCODE_RAM_DIR)/d11ucode_diag.c
	endif
	ifeq ($(WLCX_ATLAS),1)
		WLFILES_SRC += src/wl/sys/d11ucode_wlcx.c
		WLFLAGS += -DWLCX_ATLAS
	endif
	ifeq ($(WL_EAP_UCODE),1)
		WLFILES_SRC += $(UCODE_RAM_DIR)/d11ucode_btcx_eap.c
		WLFILES_SRC += $(UCODE_RAM_DIR)/d11ucode_btcx_ecieap.c
	else
		WLFILES_SRC += src/wl/sys/d11ucode_btcxmu.c
	endif
	WLFILES_SRC += src/wl/sys/d11ucode_btcxeci.c
	WLFILES_SRC += src/wl/sys/wlc.c
	WLFILES_SRC += src/wl/sys/wlc_tx.c
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
	WLFILES_SRC += src/wl/sys/wlc_nar.c
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
		HNDM2M = 1
		WL_MODESW = 1
		WLFLAGS += -DWL_AIR_IQ
		WLFLAGS += -DWL_EAP_VASIP
		WLFILES_SRC += src/wl/airiq/src/wlc_airiq.c
		WLFILES_SRC += src/wl/airiq/src/wlc_airiq_3p1.c
		WLFILES_SRC += src/wl/airiq/src/wlc_airiq_capture.c
		WLFILES_SRC += src/wl/airiq/src/wlc_airiq_iov.c
		WLFILES_SRC += src/wl/airiq/src/wlc_airiq_phy.c
		WLFILES_SRC += src/wl/airiq/src/wlc_airiq_scan.c
		WLFILES_SRC += src/shared/phy_intr.c
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
ifeq ($(WL_RATESEL_SHLADDER),1)
	WLFLAGS += -DWL_RATESEL_SHLADDER
endif
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
		ifeq ($(WL_PROXD_PHYTS),1)
			WLFLAGS += -DWL_SAMPLE_COLLECT_BMC_ALLOC
			WLFLAGS += -DWL_PROXD_PHYTS
			#WLFLAGS += -DWL_PROXD_PHYTS_DEBUG
			WLFLAGS += -DWL_FTM_CORE_ROTATE
			ifeq ($(WL_PROXD_PHYTS_DISABLED),1)
				WLFLAGS += -DWL_PROXD_PHYTS_DISABLED
			endif
		endif

		ifeq ($(FTM), 0) ### src/wl/proxd/ ###
		ifeq ($(WL_FTM), 1)
		        WLFILES_PROXD = wlc_pdsvc.c wlc_tof.c wlc_fft.c
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
		### end src/wl/proxd/ ###
		else
                ######## components/ftm/ ######

		ifeq ($(WL_FTM), 1)
		  WLFLAGS += -DWL_FTM -DWL_FTM_MSCH -DWL_FTM_11K

		  WLFILES_FTM_BCMCRYPTO = sha2.c sha2x.c
		  # ranging with ftm component
		  ifeq ($(FTM),1)
		    # TODO: ftm component still has dependency on WL_PROXD* flags

		    # FTM default flags
		    WLFLAGS += \
		      -DFTM \
		      -DFTM_DONGLE_DEBUG \
		      -DWL_FTM_TSF_SYNC

		    # FTM default sources (alphabetical order)
		    WLFILES_FTM += \
		      ftm.c \
		      ftm_dbg.c \
		      ftm_evt.c \
		      ftm_fft.c \
		      ftm_iov.c \
		      ftm_mc_burst.c \
		      ftm_mc.c \
		      ftm_msch.c \
		      ftm_proto.c \
		      ftm_range.c \
		      ftm_sched_utils.c \
		      ftm_sn.c \
		      ftm_tofutil.c \
		      ftm_util.c \
		      ftm_vs.c \
		      wlc_ftm_svc.c

		      WLFILES_FTM += ftm_fsm.c
		      WLFILES_SRC += src/shared/bcm_fsm.c
		      WLFLAGS += -DBCM_FSM_USE_OSL
		      WLFLAGS += -DFTM_FSM_DEBUG -DBCM_FSM_DEBUG

		    # 11az TB/NTB ranging (requires CSI interface)
		    ifeq ($(FTM_11AZ),1)
		      WLFLAGS += -DFTM_11AZ

		      WLFLAGS += -DWL_FTM_SECURITY
		      WLFILES_FTM += ftm_sec.c

		      # for WFA Location R2 CTT target
		      ifeq ($(LOCR2_CTT),1)
		        WLFLAGS += -DWFA_LOCATION_R2_CE # to be removed
		        WLFLAGS += -DLOCR2_CTT
		        WLFLAGS += -DFTM_TEST_MODE
		        WLFLAGS += -DPASN_WFA_CTT
		        WLFLAGS += -DFTM_HW_DUMP_SHM_DEBUG
		        WLFLAGS += -DBCMDBG_SHM
		      endif # LOCR2_CTT

		      # 11az ISTA test mode starts measurement without IFTM negotiation
		      # for testing with the signal generator unable to support negotiation
		      ifeq ($(FTM_11AZ_BYPASS_FTM),1)
		        WLFLAGS += -DFTM_11AZ_BYPASS_FTM
		      endif # FTM_11AZ_BYPASS_FTM

		      WLFILES_FTM += ftm_ntb.c
		      WLFILES_FTM += ftm_tb.c
		      WLFILES_FTM += ftm_tb_ntb.c
		      WLFILES_SRC += src/wl/sys/wlc_ftm_hw.c
		    endif # FTM_11AZ

		    # MAC/HW CSI based ranging interface
		    ifeq ($(FTM_CSI),1)
		      WLFLAGS += -DFTM_CSI

		      ifeq ($(FTM_CSI_DUMP_EVENT),1)
		        WLFLAGS += -DFTM_CSI_DUMP_EVENT
		      endif # FTM_CSI_DUMP_EVENT

		      WLFILES_FTM += ftm_hal.c
		    endif # FTM_CSI
		  endif
		endif
		WLFILES_SRC += $(addprefix components/ftm/src/, $(WLFILES_FTM))
		WLFILES_SRC += $(addprefix \
		  components/bcmcrypto/src/,$(WLFILES_FTM_BCMCRYPTO))

		endif ######## end components/ftm/ ######
	endif

	WLFILES_SRC += src/wl/sys/wlc_probresp.c
ifeq ($(WLPROBRESP_MAC_FILTER),1)
	WLFLAGS += -DWLPROBRESP_MAC_FILTER
	WLFILES_SRC += src/wl/sys/wlc_probresp_mac_filter.c
endif

ifeq ($(WL_RANDMAC),1)
	WLFLAGS += -DWL_RANDMAC
	WLFILES_SRC += src/wl/randmac/src/wlc_randmac.c
endif

ifeq ($(SUPPORT_RANDOM_MAC),1)
	WLFLAGS += -DSUPPORT_RANDOM_MAC
endif

ifeq ($(WL_QOSMGMT),1)
	WLFLAGS += -DWL_QOSMGMT
	WLFILES_SRC += src/wl/sys/wlc_qosmgmt.c

	WLFLAGS += -DBCM_QOSMGMT_R1
	ifeq ($(BCM_QOSMGMT_REL), 2)
		WLFLAGS += -DBCM_QOSMGMT_R2
	endif
	ifeq ($(BCM_QOSMGMT_R3), 1)
		WLFLAGS += -DBCM_QOSMGMT_R3
	endif
endif

	ifeq ($(WL_RELMCAST),1)
		WLFLAGS += -DWL_RELMCAST -DIBSS_RMC
		WLFILES_SRC += src/wl/rel_mcast/src/wlc_relmcast.c
	endif
	WLFILES_SRC += src/wl/sys/wlc_bsscfg.c
	WLFILES_SRC += src/wl/sys/wlc_bsscfg_viel.c
	WLFILES_SRC += src/wl/sys/wlc_vndr_ie_list.c
	WLFILES_SRC += src/wl/sys/wlc_scan.c
	WLFILES_SRC += src/wl/sys/wlc_scan_utils.c
	WLFILES_SRC += components/msch/src/wlc_msch.c
	WLFILES_SRC += components/msch/src/wlc_mschutil.c
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
		WL_MODESW ?= 1
		ifeq ($(WLTXBF),1)
			WLFLAGS += -DWL_BEAMFORMING
			WLFILES_SRC += src/wl/sys/wlc_txbf.c
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
		WL_MUSCHEDULER ?= 1
		WL_ULMU ?= 1
		WLFILES_SRC += src/wl/sys/wlc_he.c
		WLFLAGS += -DWLPKTENG
		ifeq ($(AP),1)
			WL_ULRT_DRVR ?= 1
		endif
	endif
	ifeq ($(WLTWT),1)
		WLFLAGS += -DWLTWT
		WLFILES_SRC += src/wl/sys/wlc_twt.c
	endif
	ifeq ($(TESTBED_AP_11AX),1)
		WLFLAGS += -DTESTBED_AP_11AX
		WLFILES_SRC += src/wl/sys/d11ucode_wfa.c
	endif
	ifeq ($(TESTBED_AP_11BE),1)
		WLFLAGS += -DTESTBED_AP_11BE
	endif
	ifeq ($(WL_MODESW),1)
		WLFLAGS += -DWL_MODESW
		WLFILES_SRC += src/wl/sys/wlc_modesw.c
		WLFLAGS += -DPHYCAL_CACHING
		WLFLAGS += -DRADIO_HEALTH_CHECK_P1
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
		ifeq ($(WL_ULRT_DRVR),1)
			WLFLAGS += -DWL_ULRT_DRVR
		endif
		WLFILES_SRC += src/wl/sys/wlc_ulmu.c
	endif
	ifeq ($(WL_PSMX),1)
		WLFLAGS += -DWL_PSMX
	endif
	ifeq ($(WL_PSMR1),1)
		WLFLAGS += -DWL_PSMR1
	endif
	ifeq ($(WL_SMAC),1)
		WLFLAGS += -DWL_SMAC
	endif
	ifeq ($(CPUDBG_XFER),1)
		HNDM2M=1
		WLFLAGS += -DCPUDBG_XFER
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
		WLFLAGS += -DPMMT_ENABLE_MUTE
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
#endif
		ifeq ($(WL_CHANSPEC_TXPWR_MAX),1)
			WLFLAGS += -DWL_CHANSPEC_TXPWR_MAX
		endif
-include ../../../../../../../../../.config
ifeq ($(BUILD_NAME), $(filter $(BUILD_NAME), RT-AX58U_V2 GT-AX6000 TUF-AX3000_V2 RT-AXE7800 GT10 RT-AX3000N BR63 XT8PRO BT12 BT10 BQ16 BQ16_PRO BM68 RT-AX82U_V2 RT-AX86U_PRO TUF-AX5400_V2 XD6_V2 RT-AX5400 RT-AX88U_PRO GT-AX11000_PRO RT-BE96U GT-BE98 GTBE98_PRO GT-BE96 RT-BE88U RT-BE86U RT-BE58U TUF-BE3600 GT-BE19000 RT-BE92U RT-BE95U))
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
#endif

	# If wlc_clm_data.xml is not there and we have generated wlc_clm_data_xxxx.c
	# files, use the generated file for compilation.
	WLCLM_SRC   = components/clm-api/src/wlc_clm_data.c
ifeq (_nic, $(CLM_FILE_SUFFIX))
	WLCLM_SRC   = components/clm-api/src/wlc_clm_data$(CLM_FILE_SUFFIX).c
endif
ifeq (,$(wildcard $(SRCBASE)/../components/clm-private/wlc_clm_data.xml))
ifneq (,$(wildcard $(SRCBASE)/../components/clm-api/src/wlc_clm_data$(CLM_FILE_SUFFIX).c))
	WLCLM_SRC   = components/clm-api/src/wlc_clm_data$(CLM_FILE_SUFFIX).c
endif
endif

	WLFILES_SRC += components/clm-api/src/wlc_clm.c
	WLFILES_SRC += $(WLCLM_SRC)
#ifdef WL_ECOUNTERS
ifeq ($(WL_ECOUNTERS),1)
		WLFLAGS += -DWL_ECOUNTERS
		WLFILES_SRC += src/wl/sys/wlc_ecounters.c
		WLFILES_SRC += src/shared/bcm_xtlv_cbuf.c
endif
#endif
#ifdef WLC_TXCAL
	ifeq ($(WLC_TXCAL),1)
		WLFLAGS += -DWLC_TXCAL
		WLFILES_SRC += src/wl/sys/wlc_calload.c
	endif
#endif
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
#endif
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
ifeq ($(WL_STF_ARBITRATOR),1)
	 WLFILES_SRC += src/wl/sys/wlc_stf_arb.c
endif
ifeq ($(WL_MIMOPS)$(WL_STF_ARBITRATOR),11)
	 WLFILES_SRC += src/wl/sys/wlc_stf_mimops.c
endif

ifeq ($(BCMMLC),1)
	WLFLAGS += -DBCMMLC
	ifeq ($(BCMMLCDBG),1)
		WLFLAGS += -DBCMMLCDBG
	endif

        ifeq ($(MLC_REBIND),1)
                WLFLAGS += -DMLC_REBIND
        endif

	WLFILES_SRC += src/shared/mlc_lib.c
	WLFILES_SRC += src/shared/mlc_s2s.c
	WLFILES_SRC += src/shared/mlc_h2s.c
	WLFILES_SRC += src/shared/mlc_services.c
	WLFILES_SRC += src/shared/mlc_intr.c
	WLFILES_SRC += src/wl/sys/wlc_icc.c
	WLFILES_SRC += src/shared/mlc_ipc.c
endif # BCMMLC

ifeq ($(WL_MLO),1)
	WLFLAGS += -DWL_MLO
	ifeq ($(WL_MLO_FWD_MGMT),1)
		WLFLAGS += -DWL_MLO_FWD_MGMT
	endif
	WLFLAGS += -DWL_MLO_TXRX_EVNTCMD_S2S
	WLFILES_SRC += src/wl/sys/wlc_mlo.c
	WLFILES_SRC += src/wl/sys/wlc_mlc.c
	WLFILES_SRC += src/wl/sys/wlc_icc.c
        # add support PTM protocol
	WLFILES_SRC += src/wl/sys/wlc_ptm.c
	# Enable HWALite in NIC mode
	ifneq ($(FULLDNGLBLD),1)
		WLFILES_SRC += src/shared/mlc_hwalite.c
		WLFLAGS += -DBCM_HWALITE
                # HWA2a FHR definition
		WLFLAGS += -DBCMHWA2A
	endif

	# MLO Graceful Recovery
	ifneq ($(BCMMLO_GR),)
		WLFLAGS += -DBCMMLO_GR
	endif
	# MLO Power Save
	ifeq ($(WL_MLO_PS),1)
		WLFLAGS += -DWL_MLO_PS
	endif
        # MLO STA
	ifeq ($(WLMLO_STA),1)
		WLFLAGS += -DWLMLO_STA
	endif
        # MLO Tx Status ncons Reordering using BMP service
	ifeq ($(BMPTSR),1)
                WLFLAGS += -DWL_TXS_MN_BMPTSR
	endif # BMPTSR
endif # WL_MLO

ifeq ($(WL11BE),1)
	WLFLAGS += -DWL11BE
	WLFILES_SRC += src/wl/sys/wlc_eht.c
endif

ifeq ($(WL_MLOIPC),1)
	WLFLAGS += -DMLO_IPC
	WLFILES_SRC += src/wl/sys/wl_mlo_ipc.c
endif

WLFLAGS += -DWL_BSS_INFO_TYPEDEF_HAS_ALIAS
WLFLAGS += -DRATESET_VERSION_ENABLED
endif
#endif /* WL */

# System services for Linux/ThreadX
ifneq ($(filter 1,$(WLRTE)$(WLLX)),)
	WLFILES_SRC += src/shared/bcm_globals.c
	WLFILES_SRC += src/shared/bcm_context.c
	WLFILES_SRC += src/shared/bcm_cmd.c
	WLFILES_SRC += src/shared/bcm_intr_mgmt.c
	WLFILES_SRC += src/shared/bcm_intr_common.c
	WLFILES_SRC += src/shared/bcm_intstats.c
	WLFILES_SRC += src/shared/bcm_scheduler_s3.c
	WLFILES_SRC += src/shared/bcm_m2m.c
	WLFILES_SRC += src/shared/m2m_intr.c
	ifeq ($(WLRTE),1)
		WLFILES_SRC += src/rte/src/bcm_timers_threadx.c
	else
		WLFILES_SRC += src/shared/bcm_timers_linux.c
	endif

	# ThreadX context switch notifications
	WLFLAGS += -DBCM_ENABLE_THREAD_CHANGE_NOTIFY

	ifneq ($(filter 1,$(DEBUG)$(BCMDBG)),)
		# Enable command handlers by default for internal builds
		WLFLAGS += -DBCM_SCHEDULER_CMDS
		WLFLAGS += -DBCM_TIMER_CMDS
		WLFLAGS += -DBCM_INTR_CMDS

		# Enable statistics and tracing by default for internal builds
		#BCM_INTSTATS        := 1
		BCM_INTR_STATS      := 1
		BCM_TIMERS_STATS    := 1
		BCM_SCHEDULER_STATS := 1
		BCM_BTRACE          := 1
	endif

	# Integrated statistics support
	BCM_INTSTATS ?= 0
	ifeq ($(BCM_INTSTATS),1)
		WLFLAGS += -DBCM_INTSTATS
		WLFLAGS += -DBCM_OBJECT_ID

		# Enable statistics at driver load
		#WLFLAGS += -DBCM_INTSTATS_ENABLE
	endif

	# Interrupt management module statistics support
	BCM_INTR_STATS ?= 0
	ifeq ($(BCM_INTR_STATS),1)
		WLFLAGS += -DBCM_INTR_STATS
		WLFLAGS += -DBCM_INTR_CMDS
		WLFLAGS += -DBCM_OBJECT_ID
	endif

	# Interrupt management module tracing support
	BCM_INTR_TRACING ?= 0
	ifeq ($(BCM_INTR_TRACING),1)
		WLFLAGS += -DBCM_INTR_TRACING
		WLFLAGS += -DBCM_INTR_CMDS
		WLFLAGS += -DBCM_OBJECT_ID

		# Automatically enable tracing for specified objects
		#WLFLAGS += -DBCM_INTR_TRACING_MODULE
		#WLFLAGS += -DBCM_INTR_TRACING_CORES
		#WLFLAGS += -DBCM_INTR_TRACING_GROUPS
	endif

	# Scheduler statistics support
	BCM_SCHEDULER_STATS ?= 0
	ifeq ($(BCM_SCHEDULER_STATS),1)
		WLFLAGS += -DBCM_SCHEDULER_STATS
		WLFLAGS += -DBCM_SCHEDULER_CMDS
		WLFLAGS += -DBCM_OBJECT_ID
	endif

	# Scheduler tracing support
	BCM_SCHEDULER_TRACING ?= 0
	ifeq ($(BCM_SCHEDULER_TRACING),1)
		WLFLAGS += -DBCM_SCHEDULER_TRACING
		WLFLAGS += -DBCM_SCHEDULER_CMDS
		WLFLAGS += -DBCM_OBJECT_ID
	endif

	# Timer statistics support
	BCM_TIMERS_STATS ?= 0
	ifeq ($(BCM_TIMERS_STATS),1)
		WLFLAGS += -DBCM_TIMERS_STATS
		WLFLAGS += -DBCM_TIMERS_CMDS
		WLFLAGS += -DBCM_OBJECT_ID
	endif

	# Tracing support
	BCM_BTRACE ?= 0
	ifeq ($(BCM_BTRACE),1)
		WLFILES_SRC += src/shared/bcm_btrace.c
		WLFLAGS += -DBCM_BTRACE
		WLFLAGS += -DBCM_OBJECT_ID

		# Also enable interrupt command support to enable 'cmd intr tracedata' command
		WLFLAGS += -DBCM_INTR_CMDS

		# Enable tracing at driver load
		# Trace mode and trace categories can be configured in bcm_btrace.c:btrace_init()
		#WLFLAGS += -DBCM_BTRACE_ENABLE
	endif
endif

## wl OSL

#ifdef WLLX
ifeq ($(WLLX),1)
	ifneq ($(OSLLX),1)
		WLFLAGS += -DPKTFREE_NEW_API_AVAIL
	endif
	WLFILES_SRC += src/wl/sys/wl_linux.c
	WLFILES_SRC += src/wl/sys/wl_core.c
	WLFILES_SRC += src/wl/sys/wldev_common.c

	# Logging to mem
	WLFLAGS += -DBCM_IOCV_MEM_LOG

#ifdef PCIEG3_EP
	# pcie gen3 support.
	ifneq ($(BCMPCI), 0)
	  # TODO: Revisit once mogrification is implemented.
	  _topdir := $(or $(WL_MOG),$(WLAN_TreeBaseA)/)
	  ifneq ($(wildcard $(_topdir)src/include/pcieg3_ep.h),)
	    WLFILES_SRC += src/shared/pcieg3_ep.c
	    WLFLAGS += -DPCIEG3_EP
	  endif
	endif
#endif /* PCIEG3_EP */
endif
#endif

#ifdef WLLXIW
ifeq ($(WLLXIW),1)
	WLFILES_SRC += src/wl/sys/wl_iw.c
	WLFLAGS += -DWLLXIW
endif
#endif

ifeq ($(WL_AP_CFG80211),1)
	WL_CFG80211 = 1
endif
ifeq ($(WL_STA_CFG80211),1)
	WL_CFG80211 = 1
endif
ifeq ($(WL_CFG80211),1)
	WLFILES_SRC += src/wl/sys/wl_cfg80211.c
	WLFILES_SRC += src/wl/sys/wl_cfgp2p.c
	WLFLAGS += -DWL_CFG80211
	WLFLAGS += -DUSE_CFG80211
	WLFLAGS += -DWL_CFG80211_NIC
	WLFLAGS += -DSUPPORT_SOFTAP_WPAWPA2_MIXED
	ifeq ($(WL_STA_CFG80211),1)
		WLFLAGS += -DMEDIA_CFG
	endif
	ifeq ($(WL_AP_CFG80211),1)
		WLFLAGS += -DROUTER_CFG
	endif
	WLFLAGS += -DWL_DRV_AVOID_SCANCACHE
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
	ifeq ($(KERNEL_SUPPORTS_6GHZ),1)
	   WLFLAGS += -DKERNEL_SUPPORTS_6GHZ
	endif
else ifeq ($(WL_SAE),1)
	MFP := 1
	WLFLAGS += -DWL_SAE
endif

ifeq ($(WL_DPP),1)
	WLFLAGS += -DWL_DPP
endif

ifeq ($(HEALTH_CHECK),1)
ifneq ($(STBLINUX),1)
	WLFLAGS += -DHEALTH_CHECK
	WLFILES_SRC += src/wl/sys/wlc_health_check.c
endif
endif

ifeq ($(WL_BCN_PROT),1)
	WLFLAGS += -DWL_BCN_PROT
	WLFILES_SRC += src/wl/sys/wlc_bcn_prot.c
endif

#ifdef WLCFE
ifeq ($(WLCFE),1)
	WLFILES_SRC += src/wl/sys/wl_cfe.c
endif
#endif

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
#endif

ifeq ($(BCM_DHDHDR),1)
	EXTRA_DFLAGS += -DBCM_DHDHDR
	EXTRA_DFLAGS += -DD3_BUFFER_LEN=$(FRAG_D3_BUFFER_LEN)
	EXTRA_DFLAGS += -DD11_BUFFER_LEN=$(FRAG_D11_BUFFER_LEN)
	ifeq ($(PROP_TXSTATUS),1)
		EXTRA_DFLAGS += -DWL_REUSE_KEY_SEQ
	endif
endif

ifeq ($(BCMECICOEX),1)
	WLFLAGS += -DBCMECICOEX
endif

ifeq ($(BCMLTECOEX),1)
	WLFILES_SRC += src/wl/sys/wlc_ltecx.c
	WLFLAGS += -DBCMLTECOEX
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

		WLFILES_SRC += src/shared/bcmevent.c
		WLFILES_SRC += src/dhd/sys/dhd_cdc.c
		WLFILES_SRC += src/dhd/sys/dhd_wlfc.c
		WLFILES_SRC += src/dhd/sys/dhd_common.c
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
#endif

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
#endif

ifeq ($(ADV_PS_POLL),1)
	WLFLAGS += -DADV_PS_POLL
endif

ifeq ($(GTKOE),1)
	WLFLAGS += -DGTKOE
	WLFILES_SRC += src/wl/sys/wl_gtkrefresh.c
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
#endif

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
ifneq ($(ACCONF5),)
	WLFLAGS += -DACCONF5=$(ACCONF5)
endif

#ifdef SOFTAP
ifeq ($(SOFTAP),1)
	WLFLAGS += -DSOFTAP
endif
#endif

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
		WLFILES_SRC += src/wl/sys/wlc_multibssid.c
		WLFLAGS += -DWL_MBSSID
		WLFLAGS += -DWL_MF_MBSSID
		ifeq ($(WL_MBSS_BCNROTATE),1)
			WLFLAGS += -DWL_MBSS_BCNROTATE
		endif
	endif

	ifeq ($(WDS),1)
		WLFILES_SRC += src/wl/sys/wlc_wds.c
		WLFLAGS += -DWDS
	endif

	ifeq ($(DWDS),1)
		WLFLAGS += -DDWDS
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
#endif

#ifdef STA
# sta
ifeq ($(STA),1)
	WLFLAGS += -DSTA
	WLFILES_SRC += src/wl/sys/wlc_sta.c
endif
#endif

#ifdef APSTA
# apsta
ifeq ($(APSTA),1)
	WLFLAGS += -DAPSTA
endif
# apsta
#endif

#ifdef WET
# wet
ifeq ($(WET),1)
	WLFLAGS += -DWET
	WLFILES_SRC += src/wl/sys/wlc_wet.c
endif
#endif

ifeq ($(WET_DONGLE),1)
	WLFLAGS += -DWET_DONGLE
endif

#ifdef RXCHAIN_PWRSAVE
ifeq ($(RXCHAIN_PWRSAVE), 1)
	WLFLAGS += -DRXCHAIN_PWRSAVE
endif
#endif

#ifdef RADIO_PWRSAVE
ifeq ($(RADIO_PWRSAVE), 1)
	WLFLAGS += -DRADIO_PWRSAVE
endif
#endif

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
#endif

#ifdef MCAST_REGEN
ifeq ($(MCAST_REGEN), 1)
	WLFLAGS += -DMCAST_REGEN
endif
#endif

ifneq ($(STBLINUX),1)
#ifdef  ROUTER_COMA
ifeq ($(ROUTER_COMA), 1)
	WLFILES_SRC += src/shared/hndmips.c
	WLFILES_SRC += src/shared/hndchipc.c
	WLFLAGS += -DROUTER_COMA
endif
#endif
endif

#ifdef WLOVERTHRUSTER
ifeq ($(WLOVERTHRUSTER), 1)
	WLFLAGS += -DWLOVERTHRUSTER
endif
#endif

#ifdef MAC_SPOOF
# mac spoof
ifeq ($(MAC_SPOOF),1)
	WLFLAGS += -DMAC_SPOOF
endif
#endif

#ifdef PSTA
# Proxy STA
ifeq ($(PSTA),1)
	$(error PSTA is not supported anymore)
endif
#endif

#ifdef DPSTA
# Dualband Proxy STA
ifeq ($(STA),1)
	ifeq ($(DPSTA),1)
		WLFLAGS += -DDPSTA
	endif
endif
#endif

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
#endif

#ifdef WL_MONITOR
# MONITOR
ifeq ($(WL_MONITOR),1)
	WLFLAGS += -DWL_MONITOR
	WLFLAGS += -DWL_NEW_RXSTS
	ifneq ($(FULLDNGLBLD),1)
		WLFILES_SRC += src/shared/bcmwifi/src/bcmwifi_monitor.c
	endif
endif
#endif

#ifdef WL_RADIOTAP
ifeq ($(WL_RADIOTAP),1)
	WLFLAGS += -DWL_RADIOTAP
endif
#endif

#ifdef WL_STA_MONITOR
ifeq ($(WL_STA_MONITOR),1)
	WLFLAGS += -DWL_STA_MONITOR
endif
#endif

#ifdef ACKSUPR_MAC_FILTER
ifeq ($(ACKSUPR_MAC_FILTER),1)
	WLFLAGS += -DACKSUPR_MAC_FILTER
endif
#endif

#ifdef WL_PROMISC
# PROMISC
ifeq ($(PROMISC),1)
	WLFLAGS += -DWL_PROMISC
endif
#endif

ifeq ($(WL_ALL_PASSIVE),1)
	WLFLAGS += -DWL_ALL_PASSIVE
ifdef WL_ALL_PASSIVE_MODE
	WLFLAGS += -DWL_ALL_PASSIVE_MODE=$(WL_ALL_PASSIVE_MODE)
endif
endif

#ifdef WL11H
# 11H
ifeq ($(WL11H),1)
	WLFLAGS += -DWL11H
endif
#endif

#ifdef WL11D
# 11D
ifeq ($(WL11D),1)
	WLFLAGS += -DWL11D
endif
#endif

ifeq ($(WL_IAPP),1)
	WLFLAGS += -DWL_IAPP
endif

#ifdef WL11U
# 11U
ifeq ($(WL11U),1)
	L2_FILTER := 1
	WLFLAGS += -DWL11U
	WLFILES_SRC += src/wl/sys/wlc_11u.c
endif
#endif

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
#endif

#ifdef WLCNT
# WLCNT
ifeq ($(WLCNT),1)
	WLFLAGS += -DWLCNT
	WLFILES_SRC += src/shared/bcm_app_utils.c
ifndef DELTASTATS
	DELTASTATS := 1
endif
endif
#endif

#ifdef WLTAF
ifeq ($(WLTAF),1)
	WLFILES_SRC += src/wl/sys/wlc_taf.c
	WLFILES_SRC += src/wl/sys/wlc_taf_ias.c
	WLFLAGS += -DWLTAF
	ifneq ($(HNDBMP),)
		# WLFLAGS += -DWL_BMPTSC
	endif
endif
#endif

ifeq ($(WL_LSAD),1)
	WLFLAGS += -DWL_LSAD
	WLFLAGS += -DWL_PS_STATS_MIN
	WLFILES_SRC += src/wl/sys/wlc_lsa.c
endif

# DELTASTATS
ifeq ($(DELTASTATS),1)
	WLFLAGS += -DDELTASTATS
endif

#ifdef WLCHANIM
# WLCHANIM
ifeq ($(WLCHANIM),1)
	WLFLAGS += -DWLCHANIM
endif
#endif

ifeq ($(WL_SBFNIC),1)
	WLFLAGS += -DWL_SBFNIC
	WLFILES_SRC += src/wl/sys/wlc_sbf_nic.c
endif

#ifdef WLCNTSCB
# WLCNTSCB
ifeq ($(WLCNTSCB),1)
	WLFLAGS += -DWLCNTSCB
ifeq ($(WLSCB_HISTO),1)
	WLFLAGS += -DWLSCB_HISTO
endif
endif
#endif
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

#ifdef WLCOEX
# WLCOEX
ifeq ($(WLCOEX),1)
	WLFLAGS += -DWLCOEX
endif
#endif

#ifdef WLOSEN
# hotspot OSEN
ifeq ($(WLOSEN),1)
	WLFLAGS += -DWLOSEN
endif
#endif

## wl security
# external linux supplicant
#ifdef LINUX_CRYPTO
ifeq ($(LINUX_CRYPTO),1)
	WLFLAGS += -DLINUX_CRYPTO
endif
#endif

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
#endif

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
#endif

#ifdef BCMAUTH_PSK
# in-driver authenticator
ifeq ($(BCMAUTH_PSK),1)
	PSK_COMMON := 1
	WLFLAGS += -DBCMAUTH_PSK
	WLFILES_SRC += src/wl/sys/wlc_auth.c
endif
#endif

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
#endif

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
	BCMGCMP := 1
	ifeq ($(WL_OCV),1)
		WLFLAGS += -DWL_OCV
	endif
endif
#endif

#ifdef BCMCCMP
# Soft AES CCMP
ifeq ($(BCMCCMP),1)
	WLFLAGS += -DBCMCCMP
	WLFILES_SRC += components/bcmcrypto/src/aes.c
	WLFILES_SRC += components/bcmcrypto/src/rijndael-alg-fst.c
endif
#endif

#ifdef BCMGCMP
# Soft AES GCMP
ifeq ($(BCMGCMP),1)
	WLFLAGS += -DBCMGCMP
	WLFILES_SRC += components/bcmcrypto/src/aes.c
	WLFILES_SRC += components/bcmcrypto/src/rijndael-alg-fst.c
endif
#endif

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
#endif

#ifdef WLAMSDU
ifeq ($(WLAMSDU),1)
	WLFLAGS += -DWLAMSDU
	WLFILES_SRC += src/wl/sys/wlc_amsdu.c

	ifeq ($(BCMDBG_AMSDU),1)
		# Debug for: wl dump amsdu
		WLFLAGS += -DBCMDBG_AMSDU
	endif
endif
#endif

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
#endif

#ifdef WLAMSDU_SWDEAGG
ifeq ($(WLAMSDU_SWDEAGG),1)
	WLFLAGS += -DWLAMSDU_SWDEAGG
endif
#endif

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
#endif

#ifdef WL_ASSOC_RECREATE
ifeq ($(WL_ASSOC_RECREATE),1)
	WLFLAGS += -DWL_ASSOC_RECREATE
endif
#endif

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

#ifdef WLDLS
ifeq ($(WLDLS), 1)
	WLFLAGS += -DWLDLS
	WLFILES_SRC += src/wl/sys/wlc_dls.c
endif
#endif

#ifdef WLBSSLOAD
# WLBSSLOAD
ifeq ($(WLBSSLOAD),1)
	WLFLAGS += -DWLBSSLOAD
	WLFILES_SRC += src/wl/sys/wlc_bssload.c
endif
#endif

#ifdef L2_FILTER
ifeq ($(L2_FILTER),1)
	WLFLAGS += -DL2_FILTER
	ifeq ($(L2_FILTER_STA),1)
		WLFLAGS += -DL2_FILTER_STA
	endif
	WLFILES_SRC += src/wl/sys/wlc_l2_filter.c
        WLFILES_SRC += src/shared/bcm_l2_filter.c
endif
#endif

# FCC power limit control on ch12/13.
ifeq ($(FCC_PWR_LIMIT_2G),1)
	WLFLAGS += -DFCC_PWR_LIMIT_2G
endif

#ifdef WIFI_ACT_FRAME
# WIFI_ACT_FRAME
ifeq ($(WIFI_ACT_FRAME),1)
	WLFLAGS += -DWIFI_ACT_FRAME
endif
#endif

ifeq ($(SRSCAN),1)
	WLFLAGS += -DWLMSG_SRSCAN
endif

#ifdef WL_RXEARLYRC
ifeq ($(WL_RXEARLYRC),1)
	WLFLAGS += -DWL_RXEARLYRC
endif
#endif

ifneq ($(WLC_MAX_UCODE_BSS),)
	WLFLAGS += -DWLC_MAX_UCODE_BSS=$(WLC_MAX_UCODE_BSS)
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
#endif
endif

#ifdef WLRWL
ifeq ($(WLRWL),1)
	WLFLAGS += -DWLRWL
	WLFILES_SRC += src/wl/sys/wlc_rwl.c
endif
#endif

ifeq ($(D0_COALESCING),1)
	WLFLAGS += -DD0_COALESCING
	WLFILES_SRC += src/wl/sys/wl_d0_filter.c
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
#endif

#ifdef WLPKTDLYSTAT
ifeq ($(WLPKTDLYSTAT),1)
	WLFLAGS += -DWLPKTDLYSTAT
endif
#endif

#ifdef WLPKTDLYSTAT_IND
ifeq ($(WLPKTDLYSTAT_IND),1)
	WLFLAGS += -DWLPKTDLYSTAT_IND
endif
#endif

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
#endif

WLFLAGS += -DBCMCHIPTYPE=SOCI_AI

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
#endif

#ifdef HNDBME
ifeq ($(HNDBME),1)
	WLFLAGS += -DHNDBME
	WLFILES_SRC += src/shared/hndbme.c
endif
#endif

#ifdef HNDGCI
ifeq ($(HNDGCI),1)
	WLFLAGS += -DHNDGCI
	WLFILES_SRC += src/shared/hndgci.c
endif
#endif

#ifdef BCMUTILS
ifeq ($(BCMUTILS),1)
	WLFILES_SRC += src/shared/bcmutils.c
	WLFILES_SRC += src/shared/bcmxtlv.c
	WLFILES_SRC += src/shared/hnd_pktpool.c
	WLFILES_SRC += src/shared/hnd_pktq.c
endif
#endif

#ifdef HND_LINUX
ifeq ($(HND_LINUX),1)
	WLFLAGS += -DHND_LINUX
	WLFILES_SRC += src/shared/hnd_linux.c
endif
#endif /* HND_LINUX */

#ifdef MLO_IPC
ifeq ($(MLO_IPC),1)
	WLFLAGS += -DMLO_IPC
	WLFILES_SRC += src/shared/mlo_ipc.c
endif
#endif /* MLO_IPC */

ifneq ($(BCM_SANITIZE),)
	WLFILES_SRC += src/shared/bcm_asan.c
	WLFLAGS += -DBCM_SANITIZE
	ifneq ($(BCM_SANITIZE_LIST),)
		# transform a list given like BCM_SANITIZE_LIST="scb,bsscfg" to:
		# BCM_SANITIZE_LIST="\"scb\",\"bsscfg\""
		# which can be directly used to define an array of strings, like:
		# char *a[] = { BCM_SANITIZE_LIST };
		c:= ,
		WLFLAGS += -DBCM_SANITIZE_LIST="\"$(subst $c,\"$c\",$(BCM_SANITIZE_LIST))\""
	endif
endif

#ifdef CMWIFI
ifeq ($(FULLDNGLBLD),)
ifneq ($(CMWIFI),)
WLFLAGS += -DCMWIFI
WLFLAGS += -DMAX_WLAN_ADAPTER=$(MAX_WIFI_CARDS)
ifneq ($(CMWIFI_RDKB),)
	WLFLAGS += -DCMWIFI_RDKB=1
endif
ifneq ($(CMWIFI_33940),)
	WLFLAGS += -DCMWIFI_33940
	#CM33940 is arm64 arch
	WLFLAGS += -DBCMDMA64OSL
endif

ifneq ($(WL),)
	# The wl.ko
	ifneq ($(CMWIFI_EROUTER),)
		WLFLAGS += -DCMWIFI_EROUTER
		WLFLAGS += -I$(LINUXDIR)/drivers/bcm_media_gw/include
		WLFLAGS += -I$(LINUXDIR)/drivers/bcm_media_gw/dqm
		WLFLAGS += -I$(LINUXDIR)/drivers/bcm_media_gw/dqnet
		WLFLAGS += -I$(LINUXDIR)/drivers/bcm_media_gw/mdqm
		ifneq ($(PKTC),)
			WLFLAGS += -DPKTC
			ifeq ($(WLEROUTER_AS_MODULE),)
				WLFILES_SRC += ../cmwifi/mods/wlerouter/src/wl_erouter_pktc.c
			endif # WLEROUTER_AS_MODULE
		endif
		ifneq ($(CMWIFI_EROUTER_RFAP),)
			WLFLAGS += -DCMWIFI_EROUTER_RFAP -DCM3390
			WLFLAGS += -I$(EXTMODDIR)/runner/drivers/dhd/sys
			WLFLAGS += -I$(EXTMODDIR)/runner/drivers/rdpa_gpl/include
			WLFLAGS += -I$(EXTMODDIR)/runner/drivers/bdmf/framework
			WLFLAGS += -I$(EXTMODDIR)/runner/drivers/bdmf/system
			WLFLAGS += -I$(EXTMODDIR)/runner/drivers/bdmf/system/linux
			WLFLAGS += -I$(EXTMODDIR)/runner/drivers/rdp_subsystem
			WLFLAGS += -I$(EXTMODDIR)/runner/drivers/rdp_subsystem/BCM3390
			WLFLAGS += -I$(EXTMODDIR)/runner/projects/CM3390/drivers/rdd
			ifeq ($(WLEROUTER_AS_MODULE),)
				ifeq ($(CM_BUILDROOT),)
				        WLFILES_SRC += ../cmwifi/mods/wlerouter/src/wl_erouter_rfap_legacy.c
                                else
				        WLFILES_SRC += ../cmwifi/mods/wlerouter/src/wl_erouter_rfap.c
                                endif
			else
				WLFLAGS += -DWLEROUTER_AS_MODULE
			endif # WLEROUTER_AS_MODULE
		endif
		WLFLAGS += -I$(LINUXDIR)/include/linux
	endif

	WLFLAGS += -DWL_EXPORT_CURPOWER -DEAPOL_PKT_PRIO
	ifneq ($(BCMEXTNVM),)
		WLFLAGS += -DBCMEXTNVM
	endif
	ifneq ($(BUILD_IEEE1905),)
		WLFLAGS += -DCMWIFI_IEEE1905
		WLFILES_SRC += src/wl/sys/wlc_1905metric.c
	endif
else
	# The hnd.ko
	# Pick the default vars if srom/otp and provision files doesn't exist.
	WLFLAGS += -DBCMHOSTVARS
endif
endif # CMWIFI
endif # FULLDNGLBLD
#endif

#ifdef BCMSROM
ifeq ($(BCMSROM),1)
	ifeq ($(BCMSDIO),1)
		WLFILES_SRC += src/shared/bcmsrom.c
		WLFILES_SRC += src/shared/bcmotp.c
	endif
	WLFILES_SRC += src/shared/bcmsrom.c
	WLFILES_SRC += src/shared/bcmotp.c
endif
#endif

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
#endif

#ifdef SIUTILS
ifeq ($(SIUTILS),1)
	WLFILES_SRC += src/shared/siutils.c
	WLFILES_SRC += src/shared/aiutils.c
	WLFILES_SRC += src/shared/hndpmu.c
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
#endif

#ifdef SBPCI
ifeq ($(SBPCI),1)
	WLFILES_SRC += src/shared/hndpci.c
endif
#endif

#ifdef SFLASH
ifeq ($(SFLASH),1)
	WLFILES_SRC += src/shared/sflash.c
endif
#endif

#ifdef FLASHUTL
ifeq ($(FLASHUTL),1)
	WLFILES_SRC += src/shared/flashutl.c
endif
#endif

## --- shared OSL
#ifdef OSLLX
# linux osl
ifeq ($(OSLLX),1)
	WLFLAGS += -DPKTFREE_NEW_API_AVAIL
	WLFILES_SRC += src/shared/linux_osl.c
	WLFILES_SRC += src/shared/linux_pkt.c
endif
#endif

#ifdef OSLCFE
ifeq ($(OSLCFE),1)
	WLFILES_SRC += src/shared/cfe_osl.c
endif
#endif

#ifdef OSLRTE
ifeq ($(OSLRTE),1)
	WLFILES_SRC += src/shared/rte_osl.c
endif
#endif

#ifdef OSLNDIS
ifeq ($(OSLNDIS),1)
	WLFILES_SRC += components/ndis/src/ndshared.c
	WLFILES_SRC += src/shared/ndis_osl.c
endif
#endif

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
ifeq ($(BCMIPXOTP),1)
	WLFLAGS += -DBCMIPXOTP
endif

#ifdef WLDIAG
ifeq ($(WLDIAG),1)
	WLFLAGS += -DWLDIAG
	WLFILES_SRC += src/wl/sys/wlc_diag.c
endif
#endif

#ifdef BCMDBG
ifneq ($(BCMDBG),1)
	ifeq ($(WLTINYDUMP),1)
		WLFLAGS += -DWLTINYDUMP
	endif
endif
#endif

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
#endif

# Excess PMwake
#ifdef WL_EXCESS_PMWAKE
ifeq ($(WL_EXCESS_PMWAKE),1)
	WLFLAGS += -DWL_EXCESS_PMWAKE
endif
#endif

#ifdef TOE
ifeq ($(TOE),1)
	WLFLAGS += -DTOE
	WLFILES_SRC += src/wl/sys/wl_toe.c
endif
#endif

#ifdef ARPOE
ifeq ($(ARPOE),1)
	WLFLAGS += -DARPOE
	WLFILES_SRC += src/wl/sys/wl_arpoe.c
endif
#endif

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
#endif

#ifdef TKO
ifeq ($(TKO),1)
	WLFLAGS += -DTKO
	WLFILES_SRC += src/wl/sys/wl_tko.c
endif
#endif

#ifdef ICMP
ifeq ($(ICMP),1)
	WLFLAGS += -DICMP
	WLFILES_SRC += src/wl/sys/wl_icmp.c
endif
#endif

#ifdef WLNDOE
ifeq ($(WLNDOE),1)
	WLFLAGS += -DWLNDOE -DWLNDOE_RA
	WLFILES_SRC += src/wl/sys/wl_ndoe.c
endif
#endif

#ifdef WL_BWTE
ifeq ($(WL_BWTE),1)
	WLFLAGS += -DWL_BWTE
	WLFILES_SRC += src/wl/sys/wlc_bwte.c
endif
#endif

#msch
ifeq ($(MSCH_PROFILER),1)
	WLFLAGS += -DMSCH_PROFILER
endif
ifeq ($(MSCH_TESTING),1)
	WLFLAGS += -DMSCH_TESTING
endif

#ifdef PCOEM_LINUXSTA
ifeq ($(PCOEM_LINUXSTA),1)
	WLFLAGS += -DPCOEM_LINUXSTA
endif
#endif

#ifdef LINUXSTA_PS
ifeq ($(LINUXSTA_PS),1)
	WLFLAGS += -DLINUXSTA_PS
endif
#endif

#ifdef OPENSRC_IOV_IOCTL
ifeq ($(OPENSRC_IOV_IOCTL),1)
	WLFLAGS += -DOPENSRC_IOV_IOCTL
endif
#endif

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
	WLFILES_SRC += src/wl/sys/wlc_wnm.c
        WLFILES_SRC += src/shared/bcm_l2_filter.c
endif

ifeq ($(FCBS),1)
	WLFLAGS += -DBCMFCBS
	WLFILES_SRC += src/shared/fcbs.c
	WLFILES_SRC += src/shared/fcbsutils.c
	WLFILES_SRC += src/shared/hndd11.c
endif

ifeq ($(HNDLIB),1)
	WLFILES_SRC += src/shared/bcmwifi/src/bcmwifi_channels.c
	WLFILES_SRC += src/shared/bcmwifi/src/bcmwifi_radiotap.c
	WLFILES_SRC += src/shared/bcmwifi/src/bcmwifi_rspec.c
	WLFILES_SRC += src/shared/bcmwifi/src/bcmwifi_rates.c
	ifneq ($(FULLDNGLBLD),1)
		WLFILES_SRC += src/shared/dld_linux.c
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
		WLFLAGS += -DMBO_STA
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
	ifeq ($(WL_OCE_STA),1)
		WLFLAGS += -DWL_OCE_STA
	endif
endif

ifeq ($(WL_FILS),1)
	WLFLAGS += -DWL_FILS
	WLFILES_SRC += src/wl/sys/wlc_fils.c
	ifeq ($(STA),1)
		WLFLAGS += -DWL_FILS_STA
	endif
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
#endif

ifeq ($(WL_USE_STB_THREAD),1)
	WLFLAGS += -DWL_USE_STB_THREAD
endif

ifeq ($(WL_NVRAM_FILE),1)
	WLFLAGS += -DWL_NVRAM_FILE
endif

ifeq ($(WL_LTR),1)
	WLFLAGS += -DWL_LTR
	WLFILES_SRC += src/wl/sys/wlc_ltr.c
endif

ifeq ($(BCM_BACKPLANE_TIMEOUT),1)
	WLFLAGS += -DBCM_BACKPLANE_TIMEOUT
endif

#ifdef BCM_REQUEST_FW
ifeq ($(BCM_REQUEST_FW), 1)
	WLFLAGS += -DBCM_REQUEST_FW
endif
#endif

#ifdef BCM_UCODE_FILES
ifeq ($(BCM_UCODE_FILES), 1)
	WLFLAGS += -DBCM_UCODE_FILES
endif
#endif

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
#endif

#ifdef WLDURATION
ifeq ($(WLDURATION),1)
	WLFLAGS += -DWLDURATION
	WLFILES_SRC += src/wl/sys/wlc_duration.c
endif
#endif

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
#endif

#ifdef WLSCAN_PS
ifeq ($(WLSCAN_PS),1)
	WLFLAGS += -DWLSCAN_PS
endif
#endif

#Disable aggregation for Voice traffic
#ifdef WL_DISABLE_VO_AGG
ifeq ($(WL_DISABLE_VO_AGG),1)
	WLFLAGS += -DWL_DISABLE_VO_AGG
endif
#endif

#ifdef TINY_PKTJOIN
ifeq ($(TINY_PKTJOIN),1)
	WLFLAGS += -DTINY_PKTJOIN
endif
#endif

#ifdef WL_RXEARLYRC
ifeq ($(WL_RXEARLYRC),1)
	WLFLAGS += -DWL_RXEARLYRC
endif
#endif

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
#endif

#ifdef BCM_EVENT_LOG
ifeq ($(BCM_EVENT_LOG),1)
	WLFLAGS += -DEVENT_LOG_COMPILE
	WLFLAGS += -DEVENT_LOG_ACCESS
	WLFILES_SRC += src/shared/event_log.c

	ifeq ($(BCM_PHY_PERI_LOG),1)
	WLFLAGS += -DPHY_PERI_LOG
	endif

	ifeq ($(BCM_EVENT_LOG_NIC),1)
		WLFLAGS += -DEVENT_LOG_NIC
		WLFLAGS += -DSHOW_LOGTRACE
		WLFILES_SRC += src/shared/linux_dbg.c
		WLFILES_SRC += src/shared/eldbg_ring.c
		WLFILES_SRC += src/shared/linux_exportfs.c
		WLFILES_SRC += src/shared/event_log.c
		# ERR_USE_EVENT_LOG is only for verification. will be removed after review and verification.
		WLFLAGS += -DERR_USE_EVENT_LOG
	endif
    ifeq ($(FULLDNGLBLD),1)
		WLFLAGS += -DLOGTRACE
		WLFILES_SRC += src/dongle/src/dngl_logtrace.c
	endif
endif
#endif /* BCM_EVENT_LOG */

# if dbg error is defined and bcm_event_log is defined
# and if errors need to use event log, define ERR_USE_EVENT_LOG
ifeq ($(DBG_ERROR),1)
	ifeq ($(BCM_EVENT_LOG),1)
		# for normal event logs
		ifeq ($(BCM_ERR_USE_EVENT_LOG),1)
			WLFLAGS    += -DERR_USE_EVENT_LOG
		endif
		# For event logs with return address only
		ifeq ($(BCM_ERR_USE_EVENT_LOG_RA),1)
			WLFLAGS    += -DERR_USE_EVENT_LOG_RA
		endif
	endif
endif

ifeq ($(WL_EAP_AP),1)
	WLFLAGS += -DWL_EAP_AP
	WLFLAGS += -DWL_MACDBG
endif

ifeq ($(WL_PER_VAP_CONFIG),1)
	WLFLAGS += -DWL_PER_VAP_CONFIG
endif

#ifeq ($(WL_EAP_PER_VAP_CONFIG),1)
#	WLFLAGS += -DWL_EAP_PER_VAP_CONFIG
#endif

ifeq ($(WL_EAP_80211RAW),1)
	WLFLAGS += -DWL_EAP_80211RAW
endif

ifeq ($(WL_EAP_80211RAW_TEST),1)
	WLFLAGS += -DWL_EAP_80211RAW_TEST
endif

ifeq ($(WL_PER_VAP_DTIM),1)
	WLFLAGS += -DWL_PER_VAP_DTIM
endif

ifeq ($(WL_RXTX_UTILIZATION),1)
	WLFLAGS += -DWL_RXTX_UTILIZATION
endif

ifeq ($(WL_EAP_SAMPLE_COLLECT),1)
	WLFLAGS += -DWL_EAP_SAMPLE_COLLECT
endif

ifeq ($(WL_BSS_STATS),1)
	WLFLAGS += -DWL_BSS_STATS
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

ifeq ($(WL_MU_GROUP_BSS),1)
	WLFLAGS += -DWL_MU_GROUP_BSS
endif

ifeq ($(WL_EAP_MCLX_MODE),1)
	WLFLAGS += -DWL_EAP_MCLX_MODE
endif

ifeq ($(WLBSSLOAD_REPORT),1)
	WLFLAGS += -DWLBSSLOAD_REPORT
	WLFILES_SRC += src/wl/sys/wlc_bssload.c
endif

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
	WLFLAGS += -DBRCMAPIVTW -DWLAMPDU_HOSTREORDER
	ifeq ($(BMPARQ),1)
		WLFLAGS += -DWLAMPDU_BMPARQ
	endif # BMPARQ
else
	ifeq ($(BRCMAPIVTW), 1)
		WLFLAGS += -DBRCMAPIVTW
	endif
endif

# code optimisation for non dual phy chips
ifeq ($(DUAL_PHY_CHIP), 1)
	WLFLAGS += -DDUAL_PHY
endif

ifndef WLCFGDIR
	ifdef WLCFG_DIR
		WLCFGDIR=$(WLCFG_DIR)
	endif
endif
ifndef WLCFGDIR
$(error WLCFGDIR is not defined)
endif

PHY_TOP_DIR = components/phy

# add keymgmt
ifeq ($(WL),1)
include $(WLCFGDIR)/keymgmt.mk
WLFILES_SRC += $(KEYMGMT_SRC_FILES)
endif

ifeq ($(WLROAMPROF),1)
	WLFLAGS += -DWLROAMPROF
endif

#endif

# PHY modules
ifeq ($(WL),1)
include $(WLCFGDIR)/../../../$(PHY_TOP_DIR)/phy.mk
ifeq ($(WL_PHYLIB),)
WLFILES_SRC += $(PHY_SRC)
endif
WLFLAGS += $(PHY_FLAGS)
endif

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

ifeq ($(PHY_EPAPD_NVRAMCTL),1)
	WLFLAGS += -DEPAPD_SUPPORT_NVRAMCTL=1
endif

# Dynamic bt coex(auto mode sw,desese, pwr ctrl. etc) -
ifeq ($(WL_BTCDYN),1)
	WLFLAGS += -DWL_BTCDYN
endif

ifeq ($(WLC_MACDBG_FRAMEID_TRACE),1)
	WLFLAGS += -DWLC_MACDBG_FRAMEID_TRACE
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

# Enable WAR-engine
ifeq ($(WLWARENG), 1)
	WLFLAGS += -DWLWARENG
endif

# Enable PHYDSP
ifeq ($(WLPHYDSP), 1)
	WLFLAGS += -DWLPHYDSP
endif

ifeq ($(WLOCL),1)
	EXTRA_DFLAGS += -DOCL
endif

#Arbitrator
ifeq ($(WL_STF_ARBITRATOR),1)
	WLFLAGS += -DWL_STF_ARBITRATOR
endif

# MIMOPS
ifeq ($(WL_MIMOPS),1)
	WLFLAGS += -DWL_MIMOPS
	WLFLAGS += -DWL_MIMOPS_CFG
endif

# cached flow processing
ifeq ($(WLCFP),1)
	WLFLAGS += -DWLCFP
	# Enable CFP_DEBUG manually as per use case
	# See wlc_cfp.h to modify debug level
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

# Tx Status Transfer using M2M DMA feature (rev >= 130)
ifeq ($(STS_XFER_TXS),1)
	# 80 Byte Tx Status
	WLFLAGS += -DTX_STATUS_MACTXS_BYTES=80
	WLFLAGS += -DSTS_XFER_TXS

	# TX Status Transfer using Mailbox Pager service and PKTPGR
	ifeq ($(HWAPP),1)
		# SW Mailbox pager (hndmbx)
		ifeq ($(MBX),1)
			WLFLAGS += -DSTS_XFER_TXS_MBX_PP
		endif
		# HW Mailbox pager (M2MDMA Ch#2)
		ifeq ($(STS_XFER_HWMBX),1)
			WLFLAGS += -DSTS_XFER_TXS_MBX_PP
		endif
	endif

	# Enable M2M core interrupts
	STS_XFER_M2M_INTR := 1
	STS_XFER := 1
else
	# 32 Byte Tx Status
	WLFLAGS += -DTX_STATUS_MACTXS_BYTES=32
endif

# PhyRx Status Transfer over Rx FIFO-3 feature (rev129)
ifeq ($(STS_XFER_PHYRXS_MODE_FIFO),1)
	WLFLAGS += -DSTS_XFER_PHYRXS_MODE_FIFO
endif

# PhyRx Status Transfer using M2M DMA feature (rev >= 130)
ifeq ($(STS_XFER_PHYRXS_MODE_M2M),1)
	WLFLAGS += -DSTS_XFER_PHYRXS_MODE_M2M
	# Enable M2M core interrupts
	STS_XFER_M2M_INTR := 1
endif

# PhyRx Status Transfer with 12 bit seq number
ifeq ($(STS_XFER_PHYRXS_SEQ),1)
	WLFLAGS += -DSTS_XFER_PHYRXS_SEQ
	# SW Mailbox Pager (hndmbx)
	ifeq ($(MBX),1)
		WLFLAGS += -DSTS_XFER_PHYRXS_MBX
	endif

	STS_XFER_PHYRXS := 1
	# PhyRx Status unit size
	PHYRX_STATUS_BYTES := 160
endif

# PhyRx Status Transfer transferred as standalone pkt
ifeq ($(STS_XFER_PHYRXS_PKT),1)
	WLFLAGS += -DSTS_XFER_PHYRXS_PKT
	STS_XFER_PHYRXS := 1
	# HW Mailbox pager (M2MDMA Ch#2)
	ifeq ($(STS_XFER_HWMBX),1)
		ifeq ($(HME),1)
			WLFLAGS += -DSTS_XFER_PHYRXS_MBX
		endif
	endif

	# PhyRx Packet size
	PHYRX_STATUS_BYTES := 240
endif

# PhyRx Status Transfer
ifeq ($(STS_XFER_PHYRXS),1)
	WLFLAGS += -DSTS_XFER_PHYRXS
	# PHYRX_STATUS_BYTES should be used only to reserve memory for PhyRx Ring (MBX, SECDMA).
	WLFLAGS += -DPHYRX_STATUS_BYTES=$(PHYRX_STATUS_BYTES)

	# Trigger WLAN Rx processing on arrival of PhyRx Status
	ifeq ($(STS_XFER_PHYRXS_RECV_PROC),1)
		WLFLAGS += -DSTS_XFER_PHYRXS_RECV_PROC
	endif

	STS_XFER := 1
endif

# TxStatus, PhyRxStatus or PhyRxPkt transfer
ifeq ($(STS_XFER),1)
	ifeq ($(STS_XFER_M2M_INTR),1)
		WLFLAGS += -DSTS_XFER_M2M_INTR
	endif

	# Status Tranfer using HW Mailbox pager (M2MDMA Ch#2)
	ifeq ($(STS_XFER_HWMBX),1)
		WLFLAGS += -DSTS_XFER_HWMBX
	endif

	WLFLAGS += -DSTS_XFER
	WLFILES_SRC += src/wl/sys/wlc_sts_xfer.c
endif

# Single stage Queuing and Scheduling
ifeq ($(WLSQS),1)
	WLFLAGS += -DWLSQS
	WLFILES_SRC += src/wl/sys/wlc_sqs.c
endif

# Sub PPDU Processing
ifeq ($(WLSPP),1)
	WLFLAGS += -DWLSPP
	WLFILES_SRC += src/wl/sys/wlc_spp.c
endif

ifeq ($(BCM_PCAP),1)
       EXTRA_DFLAGS += -DBCM_PCAP
       WLFILES_SRC += src/wl/sys/wlc_pcap.c
endif

# Per User Queue
ifeq ($(WL_PUQ),1)
	WLFLAGS += -DWL_PUQ
	WLFLAGS += -DWL_PUQMGR
        ifeq ($(WL_MLO),1)
	        WLFLAGS += -DWL_PUQ_SYNC_EMPTY
	        WLFLAGS += -DMLO_PUQMGR_1POOL
	        #WLFLAGS += -DMLO_PUQMGR_TESTBUILD
        endif
	WLFILES_SRC += src/shared/puqmgr.c
	WLFILES_SRC += src/wl/sys/wlc_puqmgr.c
endif

ifeq ($(WL_MCAST_FILTER_NOSTA),1)
	WLFLAGS += -DWL_MCAST_FILTER_NOSTA
endif

# trunk uses bcmcrypto component
WLFLAGS += -DBCMCRYPTO_COMPONENT

ifeq ($(BCM_CSIMON),1)
	HNDM2M = 1
	WLFLAGS += -DBCM_CSIMON
	ifeq ($(BCM_CSIMON_AP),1)
		WLFLAGS += -DBCM_CSIMON_AP
	endif
	WLFILES_SRC += src/wl/sys/wlc_csimon.c
endif

ifeq ($(BCM_SBFTBL),1)
	WLFLAGS += -DBCM_SBFTBL
	WLFILES_SRC += src/wl/sys/wlc_sbf.c
endif

# randomize probe req seq
ifeq ($(WL_PRQ_RAND_SEQ),1)
	WLFLAGS += -DWL_PRQ_RAND_SEQ
endif

ifeq ($(WL_SCAN_TX),1)
	WLFLAGS += -DWL_SCAN_TX
endif

ifeq ($(WL_SCAN_TX_MAX_POWER),1)
	WLFLAGS += -DWL_SCAN_TX_MAX_POWER
endif

ifeq ($(WL_STA_SCB_TIMEOUT),1)
	WLFLAGS += -DWL_STA_SCB_TIMEOUT
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

ifeq ($(WL_EVENT_SERVICE),1)
	WLFLAGS += -DWL_EVENT_SERVICE
endif

ifeq ($(WL_CUSTOM_SCAN),1)
	WLFLAGS += -DWL_CUSTOM_SCAN
	WLFILES_SRC += src/wl/sys/wlc_custom_scan.c
endif

ifeq ($(WL_SCAN_MEASUREMENT),1)
	WLFLAGS += -DWL_SCAN_MEASUREMENT
        ifeq ($(HME),1)
                ifneq ($(BCM_HMO_SCMEAS),)
	        EXTRA_DFLAGS	+= -DBCM_HMO_SCMEAS
        endif
endif

endif

ifeq ($(WL_SCAN_PROTECT),1)
	WLFLAGS += -DWL_SCAN_PROTECT
endif

ifeq ($(WL_SCAN_BEACON_DELAY),1)
	WLFLAGS += -DWL_SCAN_BEACON_DELAY
endif

ifeq ($(WL_SCAN_DATA_SNOOP),1)
	WLFLAGS += -DWL_SCAN_DATA_SNOOP
endif

ifeq ($(WL_EAP_RUNT_ON_BADFCS_FRM),1)
	WLFLAGS += -D WL_EAP_RUNT_ON_BADFCS_FRM
endif

ifeq ($(WL_EAP_NOISE_MEASUREMENTS),1)
	WLFLAGS += -DWL_EAP_NOISE_MEASUREMENTS
endif

ifeq ($(WL_FAST_NOISE_MEASURE),1)
	WLFLAGS += -DWL_FAST_NOISE_MEASURE
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

ifeq ($(WL_PER_VAP_CONFIG_RATESET),1)
	WLFLAGS += -DWL_PER_VAP_CONFIG_RATESET
endif

ifeq ($(WL_EAP_FIPS_LOOPBACK),1)
	WLFLAGS += -DWL_EAP_FIPS_LOOPBACK
endif

ifeq ($(WL_FIPS_CMVP_TEST),1)
	WLFLAGS += -DWL_FIPS_CMVP_TEST
endif

ifeq ($(WL_EAP_DROP_RX_MGMT_RSSI),1)
	WLFLAGS += -DWL_EAP_DROP_RX_MGMT_RSSI
	WLFILES_SRC += src/wl/sys/wlc_filter_rx_mgmt_ctl_rssi.c
endif

# Support for run-time refreshing of Phy RF analog calibration data
# NOTE: Phy Refresh deprecates WL_EAP_BOARD_RF_5G_FILTER
ifeq ($(WL_PHY_REFRESH),1)
       WLFLAGS += -DWL_PHY_REFRESH
else
# Support for board-specific hardware RF analog band-pass filters for
# 5G channel isolation (usurped by Phy Refresh)
ifeq ($(WL_EAP_BOARD_RF_5G_FILTER),1)
        WLFLAGS += -DWL_EAP_BOARD_RF_5G_FILTER
endif
endif

ifeq ($(WL_KEY_CACHE),1)
	WLFLAGS += -DWL_KEY_CACHE
endif

ifeq ($(WL_CUST_EVENT_HNDLR),1)
	WLFLAGS += -DWL_CUST_EVENT_HNDLR
	WLFILES_SRC += src/wl/sys/wlc_customer_event_handler.c
endif

ifeq ($(WL_OUTDOOR_AP),1)
	WLFLAGS += -DWL_OUTDOOR_AP
endif

ifeq ($(WL_EAP_AP1),1)
	WLFLAGS += -DWL_EAP_AP1
	WLFLAGS += -DBCMDBG_AMPDU
	WLFLAGS += -DBCMDBG_DUMP_RATELINKMEM
	WLFLAGS += -DWL_PSMX
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
	WLFILES_SRC += src/shared/hwa_intr.c
	WLFLAGS += -DBCMHWA=$(BCMHWA)
ifeq ($(FULLDNGLBLD),1)
	WLFILES_SRC += src/shared/hwa_pcie.c
endif
ifneq ($(BCMPCI), 0)
	# flags unique to NIC build
	WLFILES_SRC += src/shared/hwa_linux.c
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
	ifneq ($(STS_XFER_TXS),1)
		WLFLAGS += -DBCMHWA4A
	endif
endif
ifeq ($(HWA4B),1)
	WLFLAGS += -DBCMHWA4B
endif
ifeq ($(HWAPP),1)
	WLFILES_SRC += src/shared/hwa_pktpgr.c
	WLFLAGS += -DBCMHWAPP
endif
ifeq ($(HWADBG),1)
	# hwa_pktpgr.c is for firmware builds only. See Makeconf
	WLFLAGS += -DBCMHWADBG
endif

ifeq ($(HWAPP),1)
	ifeq ($(HNDRCH),1)
		WLFLAGS += -DHNDRCH
	endif
	ifeq ($(HNDRCH_SW),1)
		WLFLAGS += -DHNDRCH
		WLFLAGS += -DHNDRCH_SW
	endif
endif
endif

ifeq ($(HNDM2M),1)
	WLFLAGS += -DHNDM2M
	ifneq ($(HNDM2M_VER),)
		WLFLAGS += -DHNDM2M_VER=$(HNDM2M_VER)
	endif
	WLFILES_SRC += src/shared/hndm2m.c
endif

# Enable phytx error logging
ifeq ($(PHYTXERR_DUMP),1)
	WLFLAGS += -DPHYTXERR_DUMP
endif

ifeq ($(TRAFFIC_THRESH),1)
	WLFLAGS += -DWL_TRAFFIC_THRESH
endif

ifeq ($(strip $(USE_WLAN_SHARED)), 1)
ifneq ($(strip $(WLAN_SHARED_DIR)),)
-include $(WLAN_SHARED_DIR)/wifi_cfg_common.mk
ifneq ($(strip $(CONFIG_BCM_KF_EXTSTATS)),)
	WLFLAGS += -DBCM_CPEROUTER_EXTSTATS
endif
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

ifeq ($(WL_PKTCNTR),1)
	WLFLAGS += -DWL_PKTCNTR
endif

ifeq ($(WL_POLL_IDLE_STA),1)
	WLFLAGS += -DWL_POLL_IDLE_STA
endif

ifneq ($(strip $(CONFIG_BCM_PKTRUNNER)),)
    EXTRA_CFLAGS    += -DPLATFORM_WITH_RUNNER
endif

ifeq ($(WL_PS_STATS),1)
	WLFLAGS += -DWL_PS_STATS
endif

WLFLAGS += -DWL_REG_SIZECHECK

ifeq ($(WL_PKTDROP_STATS),1)
	WLFLAGS += -DWL_PKTDROP_STATS
endif

ifneq ($(findstring BCMDBG, $(WLFLAGS)),)
ifneq ($(filter 1,$(WLAUTHRESP_MAC_FILTER) $(WLPROBRESP_MAC_FILTER)),)
	WLFLAGS += -DWLC_MACFLTR_STATS
endif
endif
ifeq ($(WLC_MACFLTR_STATS),1)
ifneq ($(filter 1,$(WLAUTHRESP_MAC_FILTER) $(WLPROBRESP_MAC_FILTER)),)
	WLFLAGS += -DWLC_MACFLTR_STATS
endif
endif

ifeq ($(WL_TXPKTPEND_SYNC),1)
	WLFLAGS += -DWL_TXPKTPEND_SYNC
endif

ifeq ($(WL_WME_DYN),1)
	WLFLAGS += -DWL_WME_DYN
endif

ifeq ($(SPP_AMSDU),1)
	WLFLAGS += -DWL_SPP_AMSDU
endif

ifeq ($(WL_BA256),1)
WLFLAGS   += -DWL_BA256
endif

ifeq ($(WL_DP_DBG),1)
WLFLAGS   += -DWL_DP_DBG
endif

ifeq ($(WLDEAUTH_INTRANSIT_FILTER),1)
WLFLAGS   += -DWLDEAUTH_INTRANSIT_FILTER
endif

ifeq ($(WLPROBRESP_INTRANSIT_FILTER),1)
WLFLAGS   += -DWLPROBRESP_INTRANSIT_FILTER
endif

ifneq ($(strip $(WL_DIAG_CAPTURE_TXRX_FRAMES)),)
WLFLAGS += -DWL_DIAG_CAPTURE_TXRX_FRAMES
endif

ifneq ($(CMWIFI_WMF_IPV6), )
WLFLAGS += -DCMWIFI_WMF_IPV6
endif

ifeq ($(WL_EAP_REKEY_WAR),1)
	WLFLAGS += -DWL_EAP_REKEY_WAR
endif

ifneq ($(HMOSCB_MAX_ITEMS),)
	EXTRA_DFLAGS	+= -DHMOSCB_MAX_ITEMS=$(HMOSCB_MAX_ITEMS)
endif

ifeq ($(HME),1)
ifneq ($(BCM_HMO_MACDBG),)
	EXTRA_DFLAGS	+= -DBCM_HMO_MACDBG
endif
endif

ifeq ($(HME),1)
ifneq ($(BCM_HMO_RU_CFG),)
	EXTRA_DFLAGS	+= -DBCM_HMO_RU_CFG
endif
endif

# Packet Classification for NIC(PCN) Mode
ifeq ($(BCMRX_PCN),1)
	WLFLAGS	+= -DBCMRX_PCN
# Preinit PKTPOOL - Packet Classification for NIC(PCN) Mode
ifeq ($(BCMRX_PCN_PKTPOOL),1)
	WLFLAGS	+= -DBCMRX_PCN_PKTPOOL
endif
endif

# allow application construct beacon/probe response frame
ifeq ($(USR_BCN_PROBRESP),1)
	WLFLAGS += -DWL_USR_BCN_PROBRESP
endif

# Ranging CSI for Access 11az solution
ifeq ($(ACCESS_CSI),1)
	WLFLAGS += -DACCESS_CSI
endif

# Sorting has two benefits: it uniqifies the list, which may have
# gotten some double entries above, and it makes for prettier and
# more predictable log output.
WLFILES_SRC := $(sort $(WLFILES_SRC))
# Legacy WLFILES pathless definition, please use new src relative path
# in make files.
#
# WLFILES expands immediately at this point, which means anything appended
# to WLFILES_SRC **after** the assignment below will not change WLFILES.
# We still use this variable in some builds (even though it is supposedly
# deprecated). Please keep this assignment as the last non-comment line of this
# file so that WLFILES reflects WLFILES_SRC properly.
WLFILES := $(sort $(notdir $(WLFILES_SRC)))

#################### !!! NO CODE BELOW THIS LINE !!! ####################

# This comment block must stay at the bottom of the file.
# Local Variables:
# mode: GNUmakefile
# fill-column: 80
# End:
#
# vim: filetype=make sw=2 tw=80 cc=+1 et
