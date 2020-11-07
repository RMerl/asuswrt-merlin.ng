# Linux Makefile
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
#test merge

#SRCBASE = ../..
ifneq ($(wildcard $(PASSPOINT_AP_DIR)/hspotap.c),)
WLAN_ComponentsInUse := bcmwifi ppr olpc

ifeq (2_6,$(LINUX_VERSION))
SRCBASE_MAIN := $(BASEDIR)/src
SRCBASE_SYS := $(BASEDIR)/src
SRCBASE := $(BASEDIR)/src
endif
include $(SRCBASE_SYS)/makefiles/WLAN_Common.mk

include $(SRCBASE_ROUTER)/.config

INCLUDE += -I$(PASSPOINT_DIR)/include -I$(PASSPOINT_DIR)/dsp -I$(PASSPOINT_DIR)/wlan -I$(PASSPOINT_DIR)/pkt
INCLUDE += -I$(PASSPOINT_DIR)/tcp_srv -I$(PASSPOINT_DIR)/pro -I$(PASSPOINT_ROUTER)/shared -I$(PASSPOINT_DIR)/tmr

INCLUDE += -I$(SRCBASE_SYS)/include -I$(SRCBASE_SYS)/../components/shared/
INCLUDE += -I$(SRCBASE_SYS)/../components/wlioctl/include
INCLUDE += -I$(SRCBASE_SYS)/../components/awd/include
INCLUDE += -I$(SRCBASE_SYS)/../components/math/include
INCLUDE += -I$(SRCBASE_SYS)/wl/exe -I$(SRCBASE_SYS)/security/secfrw/include

INCLUDE += -I$(SRCBASE_ROUTER)/shared
INCLUDE += $(WLAN_ComponentIncPathR)

CFLAGS += $(INCLUDE)
CFLAGS += -DWIFI_ACT_FRAME -DHOTSPOT_AP -DWNM_BSSTRANS_SUPPORT -DWLTDLS -Wall

CFLAGS  += -s

#ifdef WLTXPWR_PER_CORE
CFLAGS += -DWLTXPWR_PER_CORE
#endif // endif

CFLAGS += -DPPR_API

#CFLAGS += -c -g -m32 -Wextra -Werror
#LDFLAGS += -m32 --cref -lpthread -lrt
#LDFLAGS += -Wl,-Map,hspotap.map

LDFLAGS += -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared -lpthread
LDFLAGS += $(EXTRA_LDFLAGS)

vpath %.c $(PASSPOINT_ROUTER)/shared/
vpath %.c $(PASSPOINT_AP_DIR)
vpath %.c $(PASSPOINT_DIR)/util
vpath %.c $(PASSPOINT_DIR)/pkt
vpath %.c $(PASSPOINT_DIR)/dsp
vpath %.c $(PASSPOINT_DIR)/tmr
vpath %.c $(PASSPOINT_DIR)/wlan
vpath %.c $(PASSPOINT_DIR)/pro
vpath %.c $(PASSPOINT_DIR)/tcp_srv
vpath %.c $(SRCBASE_SYS)/shared/bcmwifi/src
vpath %.c $(SRCBASE_SYS)/wl/exe
vpath %.c $(SRCBASE_SYS)/wl/ppr/src
vpath %.c $(SRCBASE_SYS)/shared
vpath %.c $(WLAN_ComponentSrcDirsR)

vpath %.o $(SRCBASE_ROUTER)/shared

OBJS = trace.o
OBJS += bcm_encode.o bcm_decode.o
OBJS += bcm_encode_ie.o bcm_decode_ie.o bcm_decode_p2p.o
OBJS += bcm_encode_gas.o bcm_decode_gas.o
OBJS += bcm_encode_anqp.o bcm_decode_anqp.o
OBJS += bcm_encode_hspot_anqp.o bcm_decode_hspot_anqp.o
OBJS += bcm_encode_wnm.o bcm_decode_wnm.o
OBJS += bcm_encode_qos.o bcm_decode_qos.o
OBJS += dsp_ap.o tmr.o
OBJS += wlu_api.o wlu_linux_api.o
OBJS += wlu.o wlu_common.o wlu_cmd.o wlu_iov.o miniopt.o bcmutils.o wlu_rates_matrix.o wlc_ppr.o
OBJS += wlu_subcounters.o
OBJS += bcmxtlv.o bcm_app_utils.o bcmseclib_linux_timer.o bcmseclib_timer.o bcm_llist.o
OBJS += bcm_gas.o
OBJS += bcmevent.o
OBJS += hspotap.o
OBJS += tcp_srv.o
OBJS += common_utils.o passpoint_nvparse.o

ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_phy.c),)
OBJS += wluc_phy.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_wnm.c),)
OBJS += wluc_wnm.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_cac.c),)
OBJS += wluc_cac.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_relmcast.c),)
OBJS += wluc_relmcast.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_rrm.c),)
OBJS += wluc_rrm.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_mbo.c),)
OBJS += wluc_mbo.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_fbt.c),)
OBJS += wluc_fbt.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_wowl.c),)
OBJS += wluc_wowl.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_pkt_filter.c),)
OBJS += wluc_pkt_filter.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_mfp.c),)
OBJS += wluc_mfp.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_ota_test.c),)
OBJS += wluc_ota_test.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_bssload.c),)
OBJS += wluc_bssload.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_stf.c),)
OBJS += wluc_stf.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_offloads.c),)
OBJS += wluc_offloads.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_tpc.c),)
OBJS += wluc_tpc.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_toe.c),)
OBJS += wluc_toe.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_arpoe.c),)
OBJS += wluc_arpoe.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_keep_alive.c),)
OBJS += wluc_keep_alive.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_ap.c),)
OBJS += wluc_ap.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_ampdu.c),)
OBJS += wluc_ampdu.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_ampdu_cmn.c),)
OBJS += wluc_ampdu_cmn.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_bmac.c),)
OBJS += wluc_bmac.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_ht.c),)
OBJS += wluc_ht.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_wds.c),)
OBJS += wluc_wds.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_keymgmt.c),)
OBJS += wluc_keymgmt.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_omn.c),)
OBJS += wluc_omn.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_scan.c),)
OBJS += wluc_scan.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_obss.c),)
OBJS += wluc_obss.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_prot_obss.c),)
OBJS += wluc_prot_obss.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_lq.c),)
OBJS += wluc_lq.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_seq_cmds.c),)
OBJS += wluc_seq_cmds.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_btcx.c),)
OBJS += wluc_btcx.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_led.c),)
OBJS += wluc_led.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_interfere.c),)
OBJS += wluc_interfere.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_ltecx.c),)
OBJS += wluc_ltecx.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_he.c),)
OBJS += wluc_he.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_twt.c),)
OBJS += wluc_twt.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_otp.c),)
OBJS += wluc_otp.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_heb.c),)
OBJS += wluc_heb.o
endif
ifneq ($(wildcard $(SRCBASE_SYS)/wl/exe/wluc_perf_utils.c),)
OBJS += wluc_perf_utils.o
endif
ifneq ($(findstring WLTDLS, $(CFLAGS)),)
OBJS += wluc_tdls.o
endif

endif

all: hspotap

ifneq ($(wildcard $(PASSPOINT_AP_DIR)/hspotap.c),)
hspotap: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
else
ifneq ($(wildcard ./prebuilt),)
hspotap:
	cp prebuilt/hspotap .
endif
endif

install: all
	install -d $(INSTALLDIR)/bin
	install -d $(INSTALLDIR)$(ICONPATH)
	install hspotap $(INSTALLDIR)/bin
	install WiFi-alliancelogo_3D.png $(INSTALLDIR)$(ICONPATH)
	install wifi-abgn-logo_270x73.png $(INSTALLDIR)$(ICONPATH)
	install icon_red_eng.png $(INSTALLDIR)$(ICONPATH)
	install icon_blue_eng.png $(INSTALLDIR)$(ICONPATH)
	install icon_green_eng.png $(INSTALLDIR)$(ICONPATH)
	install icon_orange_eng.png $(INSTALLDIR)$(ICONPATH)
	install icon_red_zxx.png $(INSTALLDIR)$(ICONPATH)
	install icon_blue_zxx.png $(INSTALLDIR)$(ICONPATH)
	install icon_green_zxx.png $(INSTALLDIR)$(ICONPATH)
	install icon_orange_zxx.png $(INSTALLDIR)$(ICONPATH)
	$(STRIP) $(INSTALLDIR)/bin/hspotap
	if [ -d ./prebuilt ]; then \
		cp -f hspotap ./prebuilt; \
	fi

clean:
	rm -f *.o *.map hspotap
