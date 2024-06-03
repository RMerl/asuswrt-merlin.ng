include ../../common.mak

ifndef CC
CC=gcc
endif

#support for local compilation validation
#make ARCH=mips BUILD=local
ifeq ($(BUILD), local)
ifeq ($(ARCH),arm)
CC=/proj/mtk15451/toolchain-7622/bin/aarch64-openwrt-linux-gnu-gcc
endif

ifeq ($(ARCH),mips)
CC= /opt/buildroot-gcc463/usr/bin/mipsel-linux-gcc
endif
endif

ifndef CFLAGS
CFLAGS = -MMD -O2 -Wall -g
endif
#CFLAGS += -Werror -Wall -g


ifndef LDO
	LDO=$(CC)
endif

Q=@
E=echo
ifeq ($(V), 1)
	Q=
	E=true
endif
ifeq ($(QUIET), 1)
	Q=@
	E=true
endif

-include mapd_config
#include $(TOPDIR)/.config

#ifeq ($(CONFIG_MAP_R2_SUPPORT),y)
CFLAGS += -DMAP_R2
CFLAGS += -DDFS_CAC_R2
#CFLAGS += -DMAP_160BW
#endif
#CFLAGS += -DACL_CTRL

CFLAGS += $(EXTRA_CFLAGS)
CFLAGS += -I$(abspath ./src)
CFLAGS += -I$(abspath ./src/utils)
CFLAGS += -I$(abspath ./src/dot11_if)
CFLAGS += -I$(abspath ./src/clientDB)
CFLAGS += -I$(abspath ./src/channelMon)
CFLAGS += -I$(abspath ./src/clientMon)
CFLAGS += -I$(abspath ./src/steerAction)
CFLAGS += -I$(abspath ./src/apEstimator)
CFLAGS += -I$(abspath ./src/ApRoamAlgo)
CFLAGS += -I$(abspath ./src/ApCentStr)
CFLAGS += -I$(abspath ./src/topologySrv)
CFLAGS += -I$(abspath ./src/apSelection)
CFLAGS += -I$(abspath ./src/1905_if)
CFLAGS += -I$(abspath ./)
CFLAGS += -I$(abspath ./mapd_interface/)
CFLAGS += -I$(abspath ../wappd/wapp_usr_intf/)
CFLAGS += -I$(abspath ../wappd/include/)
CFLAGS += -I$(abspath ./src/ch_planning/)
CFLAGS += -I$(abspath ./src/networkoptimization/)
CFLAGS += -I$(abspath ./src/mapfilter_if/)

ifeq ($(CONFIG_1905_SUPPPORT),y)
FLAGS_1905 += -I$(abspath ../1905daemon/lib_export/)
else
CFLAGS += -I$(abspath ./1905_local_lib/)
endif

ifdef CONFIG_RELEASE_EXCLUDE
CFLAGS += -DRELEASE_EXCLUDE
endif

# VENDOR Flags begin
#CFLAGS += -DVENDOR1_FEATURE_EXTEND
# VENDOR Flags end

OBJS += ./src/utils/common.o
OBJS += ./src/utils/mapd_debug.o
OBJS += ./src/utils/os_unix.o
OBJS += ./src/utils/eloop.o
OBJS += ./src/steer_fsm.o
OBJS += ./src/clientDB/client_db.o
ifdef CONFIG_CLIENT_DB
CFLAGS += -DCONFIG_CLIENT_DB
ifeq ($(CONFIG_CLIENT_DB), file)
CFLAGS += -DCONFIG_CLIENT_DB_FILE
else ifeq ($(CONFIG_CLIENT_DB), nvram)
CFLAGS += -DCONFIG_CLIENT_DB_NVRAM
CFLAGS += -I$(ROOTDIR)/lib/libnvram
CFLAGS += -I$(abspath ../../../lib/libnvram/)
CFLAGS += -I$(abspath ../../../kernel_headers/include/)
LIBS += ../../../lib/libnvram/libnvram.a
LIBS_bs += ../../../lib/libnvram/libnvram.a
#LDFLAGS += -lnvram
endif
OBJS += ./src/clientDB/db_$(CONFIG_CLIENT_DB).o
endif

OBJS += ./src/channelMon/chan_mon.o
OBJS += ./src/clientMon/client_mon.o
OBJS += ./src/apEstimator/ap_est.o
OBJS += ./src/steerAction/steer_action.o
ifeq ($(CONFIG_MULTIAP),y)
OBJS += ./src/topologySrv/tlv_parsor.o
OBJS += ./src/topologySrv/topologySrv.o
OBJS += ./src/topologySrv/wappEvent.o
OBJS += ./src/topologySrv/metrics.o
OBJS += ./src/1905_if/1905_if.o
OBJS += ./src/apSelection/apSelection.o
OBJS += ./src/ch_planning/ch_planning.o
OBJS += ./src/ch_planning/ch_planning_r2.o
OBJS += ./src/ApCentStr/ap_cent_str.o
OBJS += ./src/mapfilter_if/mapfilter_if.o
OBJS += ./src/networkoptimization/network_optimization.o
endif
OBJS += main.o
OBJS += mapd.o
OBJS += ./mapd_interface/ctrl_iface.o ./mapd_interface/ctrl_iface_unix.o
OBJS += ./src/dot11_if/wapp_if.o

LIBS += ./lib/libwapp_usr_intf_client.a
LIBS += libmapd_core.a
LIBS += libmapd_steer_core.a
ifeq ($(CONFIG_1905_SUPPPORT),y)
LIBS += ./lib/lib1905_interface.a
endif
LIBS_bs += ./lib/libwapp_usr_intf_client.a
LIBS_bs += libmapd_core.a
LIBS_bs += libmapd_steer_core.a
ifdef CONFIG_SUPPORT_OPENWRT
LIBS_c += -L./lib -lmapd_interface_client
else

LIBS += -L ./lib -lmapd_interface_client

endif #CONFIG_SUPPORT_OPENWRT

OBJS_c += mapd_ctrl/mapd_cli.o ./src/utils/common.o
LIBS_c +=  ./lib/libwapp_usr_intf_client.a

MAPD_LIB = libmapd_core.a
ifeq ($(CONFIG_MULTIAP_MTK_LIB),y)
OBJS_MAPD_LIB += mapd_lib/lib_score/lib_score.o
OBJS_MAPD_LIB += mapd_lib/networkoptimization/network_optimization_lib.o
OBJS_MAPD_LIB += ./mapd_lib/topologySrv/topologySrv_lib.o
OBJS_MAPD_LIB += ./mapd_lib/ch_planning_r2/ch_planning_r2_lib.o
else
OBJS_MAPD_LIB += ./mapd_lib/dummy_lib.o
endif

MAPD_STEER_LIB = libmapd_steer_core.a
ifeq ($(CONFIG_MULTIAP_MTK_STEER_LIB),y)
ifeq ($(CONFIG_MULTIAP_MTK_LIB),n)
OBJS_MAPD_STEER_LIB += mapd_lib/lib_score/lib_score.o
endif
OBJS_MAPD_STEER_LIB += ./mapd_lib/ApRoamAlgo/ap_roam_algo.o
OBJS_MAPD_STEER_LIB += ./mapd_lib/steerAction/steer_action_lib.o
OBJS_MAPD_STEER_LIB += ./mapd_lib/apEstimator/ap_est_lib.o
else
OBJS_MAPD_STEER_LIB += ./mapd_lib/dummy_steer_lib.o
endif

ifeq ($(CONFIG_MULTIAP),y)
all: clean version mapd mapd_cli standalone_bs mapd_user_iface
else
all: clean version standalone_bs bs_cli
endif
CFLAGS += -DCONFIG_SUPPORT_OPENWRT
ifeq ($(CONFIG_MULTIAP),y)
CFLAGS += -DSUPPORT_MULTI_AP
ifeq ($(CONFIG_1905_SUPPPORT),y)
FLAGS_1905 += -DSUPPORT_1905
endif
ifeq ($(CONFIG_MULTIAP),y)
ifeq ($(CONFIG_AP_CENT_STR),y)
CFLAGS += -DCENT_STR
endif
endif
endif
ifeq ($(CONFIG_MULTIAP_MTK_STEER_LIB),n)
ifeq ($(CONFIG_MULTIAP_MTK_LIB),n)
CFLAGS += -DNO_SUPPORT_MULTIAP_MTK_LIB
endif
endif
ifdef CONFIG_CORE_DUMP
CFLAGS += -DCORE_DUMP_ENABLED
endif
#validated only for arm64, core dump should be used for normal execution
ifeq ($(CONFIG_BACKTRACK_SUPPORT),y)
CFLAGS += -funwind-tables -fasynchronous-unwind-tables -DBACKTRACK_SUPPORT
LDFLAGS = -rdynamic
endif
ifeq ($(CONFIG_DEBUG_SYSLOG),y)
CFLAGS += -DCONFIG_DEBUG_SYSLOG
CFLAGS += -DCONFIG_DEBUG_FILE
endif


LDFLAGS += -L./lib 
LDFLAGS_CLI += -L./lib -lmapd_interface_client

mapd_interface/mapd_interface_ctrl.o : mapd_interface/mapd_interface_ctrl.c 
	$(Q)$(LDO) -I src/utils -c -fPIC $^ -o $@
	
ifdef CONFIG_SUPPORT_OPENWRT
#src/utils/os_unix_lib.o : src/utils/os_unix.c
#	$(Q)$(LDO) -I src/utils -c -fPIC $^ -o $@
	
#libmapd_interface_client.so: mapd_interface/mapd_interface_ctrl.o src/utils/os_unix_lib.o
#	$(Q)$(LDO) -shared -o $@ $^
endif

version:
	@echo "#define current_version \"$$(date)\"" > ./version.h
	@$(E) "  LD " $@
ifeq ($(CONFIG_MULTIAP),y)
mapd: CFLAGS += $(FLAGS_1905)
ifneq ("$(wildcard ./mapd_lib/ApRoamAlgo/ap_roam_algo.c)","")
mapd: $(OBJS_MAPD_LIB) $(OBJS_MAPD_STEER_LIB) $(OBJS)
	ar r $(MAPD_LIB) $(OBJS_MAPD_LIB)
	ar r $(MAPD_STEER_LIB) $(OBJS_MAPD_STEER_LIB)
else
mapd: $(OBJS)
endif
	$(Q)$(LDO) $(LDFLAGS) -o mapd $(OBJS) $(LIBS) $(EXTRALIBS)
	@$(E) "  LD " $@
standalone_bs:
	mv mapd mapd_bk
	mv mapd_cli mapd_cli_bk
	make clean
	make bs20
	mv mapd_cli_bk mapd_cli
	mv mapd_bk mapd
else
standalone_bs:
	make clean
	make bs20
endif
bs20: CFLAGS += -I$(abspath ./1905_local_lib/)
ifneq ("$(wildcard ./mapd_lib/ApRoamAlgo/ap_roam_algo.c)","")
bs20: $(OBJS_MAPD_LIB) $(OBJS_MAPD_STEER_LIB) $(OBJS)
	ar r $(MAPD_LIB) $(OBJS_MAPD_LIB)
	ar r $(MAPD_STEER_LIB) $(OBJS_MAPD_STEER_LIB)
else
bs20: $(OBJS)
endif
	$(Q)$(LDO) $(LDFLAGS) -o bs20 $(OBJS) $(LIBS_bs) $(EXTRALIBS)
	@$(E) "  LD " $@

mapd_lib_test: CFLAGS += -I$(abspath ./1905_local_lib/)
mapd_lib_test: $(OBJS_MAPD_LIB) $(OBJS_MAPD_STEER_LIB)
	ar r $(MAPD_LIB) $(OBJS_MAPD_LIB)
	ar r $(MAPD_STEER_LIB) $(OBJS_MAPD_STEER_LIB)
#ifdef CONFIG_SUPPORT_OPENWRT
#mapd_cli: $(OBJS_c) libmapd_interface_client.so
#else
ifeq ($(CONFIG_MULTIAP),y)
mapd_cli: $(OBJS_c) 
#endif
	$(Q)$(LDO) $(LDFLAGS_CLI) -o mapd_cli $(OBJS_c) $(LIBS_c) $(EXTRALIBS_c) $(LIBS)
	@$(E) "  LD " $@
#ifeq ($(CONFIG_MULTIAP),y)
OBJS_u += user_iface_sample.o
mapd_user_iface: $(OBJS_u) 
	$(Q)$(LDO) $(LDFLAGS_CLI) -o mapd_user_iface $(OBJS_u) -lmapd_interface_client
else
bs_cli: $(OBJS_c)
#endif
	$(Q)$(LDO) $(LDFLAGS) -o bs_cli $(OBJS_c) $(LIBS_c) $(EXTRALIBS_c) $(LIBS)
	@$(E) "  LD " $@
endif
romfs:
ifeq ($(CONFIG_MULTIAP),y)
	$(ROMFSINST) /sbin/mapd
	$(ROMFSINST) /sbin/mapd_cli
else
	$(ROMFSINST) /sbin/bs_cli
endif
	$(ROMFSINST) /sbin/bs20
	$(ROMFSINST) /sbin/EasyMesh.sh
ifeq ($(CONFIG_MULTIAP),y)
	$(ROMFSINST) /sbin/map_cert_script.sh
endif
#ifdef CONFIG_SUPPORT_OPENWRT
#	$(ROMFSINST) /lib/libmapd_interface_client.so
#endif
	$(ROMFSINST) /etc_ro/wts_bss_info_config
clean:
	$(MAKE) -C ./src clean
	$(MAKE) -C ./mapd_ctrl clean
	$(MAKE) -C ./mapd_interface clean
ifdef CONFIG_SUPPORT_OPENWRT
#	rm -f mapd_cli mapd libmapd_interface_client.so
endif
ifeq ($(CONFIG_MULTIAP),y)
	rm -f mapd_cli mapd bs20
else
	rm -f bs_cli bs20
endif
ifneq ("$(wildcard ./mapd_lib/ApRoamAlgo/ap_roam_algo.c)","")
	rm -f core *~ *.o *.d *.gcno *.gcda *.gcov *.a
	rm -f $(OBJS_MAPD_LIB)
	rm -f $(OBJS_MAPD_STEER_LIB)
	rm -f $(ALL)
else
	rm -f mapd.o user_iface_sample.o main.o
	rm -f core *~ *.d *.gcno *.gcda *.gcov
endif

install:all
	install -D bs20 $(INSTALLDIR)/bin/bs20
	$(STRIP) $(INSTALLDIR)/bin/bs20
	chmod 0500 $(INSTALLDIR)/bin/bs20
	install -D mapd_cli $(INSTALLDIR)/bin/mapd_cli
	$(STRIP) $(INSTALLDIR)/bin/mapd_cli
	chmod 0500 $(INSTALLDIR)/bin/mapd_cli
	install -D -m 755 lib/libmapd_interface_client.so $(INSTALLDIR)/usr/lib/libmapd_interface_client.so
	$(STRIP) $(INSTALLDIR)/usr/lib/libmapd_interface_client.so
#	install -D conf/1905d.cfg $(INSTALLDIR)/usr/etc/map/1905d.cfg
#	install -D conf/mapd_default.cfg $(INSTALLDIR)/usr/etc/map/mapd_default.cfg
#	install -D conf/mapd_user.cfg $(INSTALLDIR)/usr/etc/map/mapd_user.cfg
#	install -D conf/wts_bss_info_config $(INSTALLDIR)/usr/etc/map/wts_bss_info_config
#	install -D conf/mapd_cfg $(INSTALLDIR)/usr/etc/map/mapd_cfg
#	install -D conf/mapd_strng.conf $(INSTALLDIR)/usr/etc/mapd_strng.conf
#	install -D conf/wapp_ap_ra0.conf $(INSTALLDIR)/usr/etc/wapp_ap_ra0.conf
#	install -D conf/wapp_ap_rai0.conf $(INSTALLDIR)/usr/etc/wapp_ap_rai0.conf
#	install -D conf/wapp_ap.conf $(INSTALLDIR)/usr/etc/wapp_ap.conf


