include ../common.mak

ASUSWRTTOOLSDIR = $(SRCBASE)/router/tools/Lnx_AsusWrtDictPrep
ASUSWRTTOOLSDIR2 = $(SRCBASE)/router/tools/Lnx_ToolHelp

MODELS_HAVE_USB3 = GT-AC2900 \
 	BRT-AC828 RT-AD7200 \
	GT-AXY16000 \
	DSL-AC68U DSL-AX82U \
	RT-AC5300 GT-AC5300 GT-AC9600 RT-AC95U RT-AC3100 RT-AC3200 GT-AX11000\
	RT-AC55U RT-AC55UHP RT-AC58U 4G-AC53U RT-AC82U RT-AC88Q RT-AX89U \
	RT-AC86U RT-AC88U RT-AX88U RT-AC87U RT-AC85U RT-AC68U RT-AC68A 4G-AC68U RT-AC65U RT-AC56S RT-AC56U RT-AX92U RT-AX95Q RT-AX56_XD4 CT-AX56_XD4 RT-AX58U TUF-AX3000 RT-AX82U RT-AX56U GS-AX3000 GS-AX5400 \
	RT-AX86U RT-AX5700 RT-AX68U \
	RT-N65U \
	RT-N18U \
	RP-AC68U \
	GT-AXE11000

MODELS_HAVE_TRAFFIC_ANALYZER = RT-AC3200 RT-AC87U RT-AC88U RT-AX88U RT-AC86U GT-AC2900 RT-AC3100 RT-AC5300 GT-AC5300 GT-AXY16000 RT-AX89U GT-AC9600 DSL-AC68U \
	RT-AC68U 4G-AC68U RT-AC68R RT-AC68W RT-AC68RW GT-AX11000 RT-AC1900P RT-AC67U RT-AC56U RT-AC56R RT-AC85U RT-AC85P RT-AC65U BRT-AC828 BLUECAVE RT-AX92U RT-AX95Q RT-AX56_XD4 CT-AX56_XD4 RT-AX58U TUF-AX3000 RT-AX82U GS-AX3000 GS-AX5400 RT-AX56U MAP-AC1300 \
	RT-AX86U RT-AX5700 RT-AX68U DSL-AX82U \
	MAP-AC2200 MAP-AC2200V RT-AC95U \
	GT-AXE11000

MODELS_QIS_SLIDER = GT-AX11000 GT-AXY16000 RT-AX89U GT-AXE11000

all:
ifeq ($(and $(HND_ROUTER),$(RTCONFIG_VISUALIZATION)),y)
VISWWW = $(SRCBASE)/../components/apps/visualization/www
JQUERYLIB = $(SRCBASE)/../components/opensource/jquerylib
endif

ifeq ($(and $(HND_ROUTER),$(RTCONFIG_VISUALIZATION)),y)
VISPAGES += $(VISWWW)/visindex.asp $(VISWWW)/channelcapacity.asp $(VISWWW)/channels.asp $(VISWWW)/configure.asp $(VISWWW)/metrics.asp
VISPAGES += $(VISWWW)/channelcapacity.js $(VISWWW)/channels.js $(VISWWW)/common.js $(VISWWW)/configure.js $(VISWWW)/main.js $(VISWWW)/matrics.js
VISPAGES += $(VISWWW)/tabheading.jpg $(VISWWW)/down-arrow.gif $(VISWWW)/up-arrow.gif $(VISWWW)/ap-image.jpg $(VISWWW)/download.jpg
VISPAGES += $(VISWWW)/channelCapacity.css $(VISWWW)/configure.css $(VISWWW)/matrics.css $(VISWWW)/visstyle.css $(VISWWW)/tablestyle.css
VISPAGES += $(VISWWW)/visualization.gif
ifneq ($(NO_BCMINTERNAL), 1)
VISPAGES += $(VISWWW)/zerowaitdfs.asp $(VISWWW)/zerowaitdfs.css $(VISWWW)/zerowaitdfs.js
endif
VISPAGES += $(JQUERYLIB)/curvedLines.js $(JQUERYLIB)/jquery.flot.axislabels.js $(JQUERYLIB)/jquery.flot.navigate.min.js
VISPAGES += $(JQUERYLIB)/jquery.flot.stack.js $(JQUERYLIB)/highlightseries.js $(JQUERYLIB)/jquery.flot.js $(JQUERYLIB)/jquery.flot.orderBars.js
VISPAGES += $(JQUERYLIB)/jquery.flot.time.min.js $(JQUERYLIB)/jquery.tablesorter.min.js $(JQUERYLIB)/jquery-2.0.3.min.js
VISPAGES += $(JQUERYLIB)/jquery.cookie.js $(JQUERYLIB)/jquery.flot.min.js $(JQUERYLIB)/jquery.flot.pie.js
VISPAGES += $(JQUERYLIB)/jquery-ui.js $(JQUERYLIB)/jquery-ui.min.css
endif # HND_ROUTER && RTCONFIG_VISUALIZATION

clean:

install:
	echo $(VISWWW)
	rm -rf $(INSTALLDIR)/www

	# INSTALLDIR is $(INSTALLDIR)/www here
	mkdir -p $(INSTALLDIR)
	cp ../www $(INSTALLDIR)/. -rf
ifeq ($(and $(HND_ROUTER),$(RTCONFIG_VISUALIZATION)),y)
	install -m 644 $(VISPAGES) $(INSTALLDIR)/www
endif # CONFIG_VISUALIZATION
	# INSTALL MODEL DEPENDENT files
ifeq ($(and $(RTCONFIG_AMAS),$(RTCONFIG_WIFI_SON)),y)
	mkdir -p $(INSTALLDIR)/www/www-sys
	cp ../www/sysdep/$(BUILD_NAME)/www/* $(INSTALLDIR)/www/www-sys -rfL || true
	rm -rf $(INSTALLDIR)/www/www-sys/images
	cp ../www/sysdep/$(BUILD_NAME)/www/images $(INSTALLDIR)/www/. -rfL || true
endif
	cp ../www/sysdep/$(BUILD_NAME)/www $(INSTALLDIR)/. -rfL || true
	rm -rf $(INSTALLDIR)/www/Makefile
	rm -rf $(INSTALLDIR)/www/sysdep
	rm -rf $(INSTALLDIR)/www/gendict.pl
	rm -rf $(INSTALLDIR)/www/dict.tmp
	rm -rf $(INSTALLDIR)/www/encoding_check.sh
	rm -rf $(INSTALLDIR)/www/en_result.txt
	rm -rf $(INSTALLDIR)/www/temp.dict
	rm -rf $(INSTALLDIR)/www/model_patch
	rm -rf $(INSTALLDIR)/www/.gitignore
	rm -rf `find $(INSTALLDIR) -name "*.bak"`
	rm -rf $(INSTALLDIR)/www/device-map/compatibility.asp
	cp -rf $(INSTALLDIR)/www/images/new/* $(INSTALLDIR)/www/images/
	rm -rf $(INSTALLDIR)/www/images/new

ifeq ($(BUILD_NAME), GT-AC2900)
ifeq ($(RTCONFIG_ROG_UI), y)
	cp -r $(TOP)/www/sysdep/FUNCTION/ROG_UI/* $(INSTALLDIR)/www/
endif
endif

ifeq ($(RTCONFIG_AMAS),y)
ifneq ($(RTCONFIG_DSL),y)
	cp $(TOP)/www/sysdep/BLUECAVE/www/Advanced_Wireless_Survey.asp $(INSTALLDIR)/www/Advanced_Wireless_Survey.asp
	cp $(TOP)/www/sysdep/BLUECAVE/www/QIS_wizard.htm $(INSTALLDIR)/www/QIS_wizard.htm
	cp $(TOP)/www/sysdep/BLUECAVE/www/mobile $(INSTALLDIR)/www/mobile -rf
	cp $(TOP)/www/sysdep/BLUECAVE/www/require/modules/modem.js $(INSTALLDIR)/www/require/modules/ -rf
	rm -rf $(INSTALLDIR)/www/images/New_ui/smart_connect_concept.gif
ifneq ($(RTCONFIG_WIFI_SON),y)
	rm -rf $(INSTALLDIR)/www/images/qis
endif
	rm -rf $(INSTALLDIR)/www/images/rog_feature*
	rm -rf $(INSTALLDIR)/www/qis
	rm -rf $(INSTALLDIR)/www/qis_m
	rm -rf $(INSTALLDIR)/www/QIS_wizard_m.htm
endif
endif

ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME), $(MODELS_QIS_SLIDER)))
	cp $(TOP)/www/sysdep/BLUECAVE/www/QIS_wizard.htm $(INSTALLDIR)/www/QIS_wizard.htm
	cp $(TOP)/www/sysdep/BLUECAVE/www/mobile $(INSTALLDIR)/www/mobile -rf
	cp $(TOP)/www/sysdep/BLUECAVE/www/require/modules/modem.js $(INSTALLDIR)/www/require/modules/ -rf
	rm -rf $(INSTALLDIR)/www/images/qis
	rm -rf $(INSTALLDIR)/www/images/rog_feature*
	rm -rf $(INSTALLDIR)/www/qis
	rm -rf $(INSTALLDIR)/www/qis_m
	rm -rf $(INSTALLDIR)/www/QIS_wizard_m.htm
endif

ifeq ($(RTCONFIG_WL_SCHED_V2), y)
	cp $(TOP)/www/sysdep/FUNCTION/WL_SCHED_V2/Advanced_WAdvanced_Content.asp $(INSTALLDIR)/www/Advanced_WAdvanced_Content.asp
endif
ifeq ($(RTCONFIG_PC_SCHED_V3), y)
	cp $(TOP)/www/sysdep/FUNCTION/PC_SCHED_V3/ParentalControl.asp $(INSTALLDIR)/www/ParentalControl.asp
endif
ifneq ($(or $(RTCONFIG_WL_SCHED_V2),$(RTCONFIG_PC_SCHED_V3)),y)
	rm -rf $(INSTALLDIR)/www/js/weekSchedule
	rm -rf $(INSTALLDIR)/www/images/New_ui/weekSchedule
endif

	chmod -x `find $(INSTALLDIR)/www/ -name "*.asp"`
	chmod -x `find $(INSTALLDIR)/www/ -name "*.htm"`
	chmod -x `find $(INSTALLDIR)/www/ -name "*.css"`
	chmod -x `find $(INSTALLDIR)/www/ -name "*.js"`
	chmod -x `find $(INSTALLDIR)/www/ -name "*.svg"`
	chmod -x `find $(INSTALLDIR)/www/ -name "*.htc"`
	chmod -x `find $(INSTALLDIR)/www/ -name "*.dict"`

ifeq ($(RTCONFIG_AUTODICT), y)
	# now dict prep will generate necessary dict files to dest dir
	rm -f $(INSTALLDIR)/www/*.dict
	# Tool dir , dictctrl.txt , modem name , header mapping , src , dst , output msg file
	$(ASUSWRTTOOLSDIR)/LnxDictPrep $(ASUSWRTTOOLSDIR) dictctrl.txt $(BUILD_NAME) HdrMapping.txt . $(INSTALLDIR)/www $(SRCBASE)/image/dictenum.txt $(SRCBASE)/image/dict_prep.txt
	# generate sh file for clear up html and dict auto enum
	# this is for compression
	$(ASUSWRTTOOLSDIR2)/genshfile.pl $(ASUSWRTTOOLSDIR2) $(INSTALLDIR)/www $(SRCBASE)/image/compress.sh $(SRCBASE)/image/dictenum.txt $(SRCBASE)/image/output2.txt $(BUILD_NAME) $(SRCBASE)/image/output3.txt
	chmod 777 $(SRCBASE)/image/compress.sh
	sh $(SRCBASE)/image/compress.sh
else
	for F in $(wildcard *.dict); do \
		./gendict.pl $(BUILD_NAME) $$F $(INSTALLDIR)/www/$$F; \
	done
endif
	echo " WWW FOR $(BUILD_NAME)"

	rm -rf $(INSTALLDIR)/www/images/New_ui/asustitle_ie6.png
#ifneq ($(RTCONFIG_FANCTRL),y)
#	rm -rf $(INSTALLDIR)/www/Advanced_PerformanceTuning_Content.asp
#	rm -rf $(INSTALLDIR)/www/fan.svg
#	rm -rf $(INSTALLDIR)/www/images/btnPressed.png
#	rm -rf $(INSTALLDIR)/www/images/btnReleased.png
#	rm -rf $(INSTALLDIR)/www/images/position.png
#endif
ifeq ($(RTCONFIG_SFP),y)
	rm -rf $(INSTALLDIR)/www/images/PrinterServer
	rm -rf $(INSTALLDIR)/www/images/appinstall
endif
ifeq ($(RTCONFIG_8M_SFP),y)
	rm -rf $(INSTALLDIR)/www/images/cloudsync/invite_model.jpg
	rm -rf $(INSTALLDIR)/www/images/New_ui/network_config.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/TimeLimits.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/QoS.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/parental-control.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/rt.jpg
	rm -rf $(INSTALLDIR)/www/images/New_ui/re.jpg
	rm -rf $(INSTALLDIR)/www/images/New_ui/ap.jpg
	rm -rf $(INSTALLDIR)/www/images/New_ui/mb.jpg
	rm -rf $(INSTALLDIR)/www/images/New_ui/login_bg.png
	rm -rf $(INSTALLDIR)/www/images/backtohome.png
	rm -rf $(INSTALLDIR)/www/images/backtohomeclick.png
	rm -rf $(INSTALLDIR)/www/images/wifi_hw_sw.png
	rm -rf $(INSTALLDIR)/www/images/qis/security_key.png
	rm -rf $(INSTALLDIR)/www/images/qis/select_wireless.jpg
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/vender-listover.png
	cd $(INSTALLDIR)/www/images/New_ui/networkmap && ln -sf vender-list.png vender-listover.png && cd $(TOP)/www
endif
ifeq ($(RTCONFIG_4M_SFP),y)
	rm -rf $(INSTALLDIR)/www/images/cloudsync/invite_model.jpg
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/bg.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/client-list.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/vender-list.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/vender-listover.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/client.svg
	rm -rf $(INSTALLDIR)/www/Main_ConnStatus_Content.asp
	rm -rf $(INSTALLDIR)/www/svghtc
	rm -rf $(INSTALLDIR)/www/Main_TrafficMonitor_daily.asp
	rm -rf $(INSTALLDIR)/www/Main_TrafficMonitor_last24.asp
	rm -rf $(INSTALLDIR)/www/Main_TrafficMonitor_realtime.asp
	rm -rf $(INSTALLDIR)/www/tm.svg
	rm -rf $(INSTALLDIR)/www/tmcal.js
	rm -rf $(INSTALLDIR)/www/tmhist.js
	rm -rf $(INSTALLDIR)/www/tmmenu.css
	rm -rf $(INSTALLDIR)/www/tmmenu.js
#	rm -rf $(INSTALLDIR)/www/images/qis/router_login.jpg
	rm -rf $(INSTALLDIR)/www/images/qis/security_key.png
	rm -rf $(INSTALLDIR)/www/images/qis/select_wireless.jpg
#	rm -rf $(INSTALLDIR)/www/Advanced_WANPort_Content.asp
#	rm -rf $(INSTALLDIR)/www/mediaserver.asp
	rm -rf $(INSTALLDIR)/www/ajax_dms_status.asp
	rm -rf $(INSTALLDIR)/www/Advanced_PerformanceTuning_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_IPv6_Content.asp
	rm -rf $(INSTALLDIR)/www/Advacned_SwitchCtrl_Content.asp
	rm -rf $(INSTALLDIR)/www/ure*
	rm -rf $(INSTALLDIR)/www/disk*
	rm -rf $(INSTALLDIR)/www/fan.svg
#	rm -rf $(INSTALLDIR)/www/detect_firmware.asp
	rm -rf $(INSTALLDIR)/www/ajax_coretmp.asp
	rm -rf $(INSTALLDIR)/www/qis/QIS_internet_ruisp.htm
	rm -rf $(INSTALLDIR)/www/qis/ruisp.js
	rm -rf $(INSTALLDIR)/www/images/New_ui/network_config.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/TimeLimits.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/QoS.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/parental-control.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/rt.jpg
	rm -rf $(INSTALLDIR)/www/images/New_ui/re.jpg
	rm -rf $(INSTALLDIR)/www/images/New_ui/ap.jpg
	rm -rf $(INSTALLDIR)/www/images/New_ui/mb.jpg
	rm -rf $(INSTALLDIR)/www/images/New_ui/login_bg.png
	rm -rf $(INSTALLDIR)/www/images/backtohome.png
	rm -rf $(INSTALLDIR)/www/images/backtohomeclick.png
	rm -rf $(INSTALLDIR)/www/images/wifi_hw_sw.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/vender-listover.png
	cd $(INSTALLDIR)/www/images/New_ui/networkmap && ln -sf vender-list.png vender-listover.png && cd $(TOP)/www
endif
ifneq ($(RTCONFIG_PROXYSTA),y)
	rm -rf $(INSTALLDIR)/www/images/New_ui/mb.jpg
endif
	rm -rf $(INSTALLDIR)/www/MobileQIS_Login.asp
	rm -rf $(INSTALLDIR)/www/AjaxLogin.asp
	rm -rf $(INSTALLDIR)/www/Passpoint_hidden.asp
	rm -rf $(INSTALLDIR)/www/Advanced_WAdvPassPoint_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_WPasspoint_Content.asp
	rm -rf $(INSTALLDIR)/www/SMTP_Client.asp
	rm -rf $(INSTALLDIR)/www/images/tmo_modem_plug.png
	rm -rf $(INSTALLDIR)/www/images/tmo_modem_unplug.png
	rm -rf $(INSTALLDIR)/www/iui/cellspot_logo.png
	rm -rf $(INSTALLDIR)/www/iui/default.css
	rm -rf $(INSTALLDIR)/www/iui/registerEvent.js
	rm -rf $(INSTALLDIR)/www/iui/tmo-logo.png
ifneq ($(RTCONFIG_SNMPD),y)
	rm -rf $(INSTALLDIR)/www/Advanced_SNMP_Content.asp
endif
ifneq ($(RTCONFIG_NETOOL),y)
	rm -rf $(INSTALLDIR)/www/Main_GameServer_Content.asp
	rm -rf $(INSTALLDIR)/www/ajax/gameList.json
	rm -rf $(INSTALLDIR)/www/cards/pingMap*
	rm -rf $(INSTALLDIR)/www/require/modules/mapApi.js
	rm -rf $(INSTALLDIR)/www/js/table.js
endif
ifeq ($(RTCONFIG_DSL),y)
	rm -rf $(INSTALLDIR)/www/qis/QIS_welcome.htm
	rm -rf $(INSTALLDIR)/www/qis/QIS_manual.htm
	rm -rf $(INSTALLDIR)/www/images/New_ui/rt.jpg
	rm -rf $(INSTALLDIR)/www/images/New_ui/ap.jpg
	rm -rf $(INSTALLDIR)/www/images/New_ui/re.jpg
	rm -rf $(INSTALLDIR)/www/Advanced_OperationMode_Content.asp
else
	rm -rf $(INSTALLDIR)/www/Main_Spectrum_Content.asp
	rm -rf $(INSTALLDIR)/www/SNR.svg
	rm -rf $(INSTALLDIR)/www/BPC.svg
	rm -rf $(INSTALLDIR)/www/ajax_signals.asp
endif

ifeq ($(BUILD_NAME),DSL-N55U)
ifeq ($(BUILD_NAME),DSL-N55U-B)
	rm -rf $(INSTALLDIR)/www/images/Model_product.png
endif
endif

ifeq ($(HND_ROUTER),y)
	rm -rf $(INSTALLDIR)/www/Main_TrafficMonitor_devdaily.asp
	rm -rf $(INSTALLDIR)/www/Main_TrafficMonitor_devmonthly.asp
	rm -rf $(INSTALLDIR)/www/Main_TrafficMonitor_devrealtime.asp
endif

ifneq ($(BUILD_NAME),DSL-N55U)
ifneq ($(BUILD_NAME),DSL-N55U-B)
ifneq ($(BUILD_NAME),RT-N56U)
ifneq ($(BUILD_NAME),RT-AC65U)
ifneq ($(RTCONFIG_CLOUDSYNC),y)
	rm -rf $(INSTALLDIR)/www/images/cloudsync
	rm -rf $(INSTALLDIR)/www/images/aicloud_logo.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/USBExt/*cloud*.png
	rm -rf $(INSTALLDIR)/www/update_cloudstatus.asp
	rm -rf $(INSTALLDIR)/www/*cloud*.asp
endif
endif
endif
endif
endif

ifneq ($(RTCONFIG_IPV6),y)
	rm -rf $(INSTALLDIR)/www/Main_IPV6Status_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_IPv6_Content.asp
	rm -rf $(INSTALLDIR)/www/update_IPv6state.asp
endif

ifeq ($(RTCONFIG_NO_USBSTORAGE),y)
	rm -rf $(INSTALLDIR)/www/images/New_ui/USBExt
	rm -rf $(INSTALLDIR)/www/images/New_ui/aidisk
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/map-iconPrinter.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/map-iconmodem.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/map-iconUSBdisk.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/advancesetting
	rm -rf $(INSTALLDIR)/www/aidisk
	rm -rf $(INSTALLDIR)/www/getfoldertree.asp
	rm -rf $(INSTALLDIR)/www/gettree.asp
	rm -rf $(INSTALLDIR)/www/images/Tree
	# disk utility
	rm -rf $(INSTALLDIR)/www/device-map/disk_utility.asp
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/backgroud_move_8P_2.0.gif
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/white_04.gif
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/normal.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/blue.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/red.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/USB_2.png
	# disk format
	rm -rf $(INSTALLDIR)/www/device-map/disk_format.asp
else
ifneq ($(RTCONFIG_USB),y)
	rm -rf $(INSTALLDIR)/www/images/New_ui/USBExt
	rm -rf $(INSTALLDIR)/www/images/New_ui/aidisk
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/map-iconPrinter.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/map-iconmodem.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/map-iconUSBdisk.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/advancesetting
	rm -rf $(INSTALLDIR)/www/images/WANunplug_usb.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/usbstatus.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/printstatus.png
	rm -rf $(INSTALLDIR)/www/aidisk
#	rm -rf $(INSTALLDIR)/www/mediaserver.asp
	rm -rf $(INSTALLDIR)/www/ajax_dms_status.asp
	rm -rf $(INSTALLDIR)/www/Advanced_AiDisk_ftp.asp
	rm -rf $(INSTALLDIR)/www/Advanced_AiDisk_samba.asp
	rm -rf $(INSTALLDIR)/www/Advanced_AiDisk_others.asp
	rm -rf $(INSTALLDIR)/www/Advanced_AiDisk_webdav.asp
	rm -rf $(INSTALLDIR)/www/Advanced_APPList_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_Modem_Content.asp
	rm -rf $(INSTALLDIR)/www/aidisk.asp
	rm -rf $(INSTALLDIR)/www/PrinterServer.asp
	rm -rf $(INSTALLDIR)/www/APP_Installation.asp
	rm -rf $(INSTALLDIR)/www/cdma2000_list.js
	rm -rf $(INSTALLDIR)/www/wcdma_list.js
	rm -rf $(INSTALLDIR)/www/wimax_list.js
	rm -rf $(INSTALLDIR)/www/td-scdma_list.js
	rm -rf $(INSTALLDIR)/www/device-map/printer.asp
	rm -rf $(INSTALLDIR)/www/device-map/modem.asp
	rm -rf $(INSTALLDIR)/www/device-map/disk.asp
	rm -rf $(INSTALLDIR)/www/getfoldertree.asp
	rm -rf $(INSTALLDIR)/www/gettree.asp
	rm -rf $(INSTALLDIR)/www/update_appstate.asp
	rm -rf $(INSTALLDIR)/www/images/Tree
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/devicebt_default.png
	rm -rf $(INSTALLDIR)/www/switch_AiDisk_app.asp
	rm -rf $(INSTALLDIR)/www/update_diskinfo.asp
	rm -rf $(INSTALLDIR)/www/device-map/safely_remove_disk.asp
	rm -rf $(INSTALLDIR)/www/device-map/disk.asp
	# disk utility
	rm -rf $(INSTALLDIR)/www/device-map/disk_utility.asp
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/backgroud_move_8P_2.0.gif
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/white_04.gif
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/normal.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/blue.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/red.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/normal.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/USB_2.png
	rm -rf $(INSTALLDIR)/www/disk_msg.js
	rm -rf $(INSTALLDIR)/www/disk_fsck.xml
	rm -rf $(INSTALLDIR)/www/disk_scan.asp
	rm -rf $(INSTALLDIR)/www/diskmon_test.asp
	rm -rf $(INSTALLDIR)/www/disk_functions.js
	# disk format
	rm -rf $(INSTALLDIR)/www/device-map/disk_format.asp
endif
ifneq ($(MODEM),y)
	-rm -f $(INSTALLDIR)/www/qis/QIS_modem.htm
endif
endif

ifeq ($(BUILD_NAME), RT-N12)
	cd $(INSTALLDIR)/www && find . -name "*.dict"|xargs -n 1 -I @ sed -i "s/RT-N12/ZVDYNMODELVZ/g" @
endif

ifeq ($(RTCONFIG_OPENVPN), y)
	cd $(INSTALLDIR)/www && ln -sf ../etc/openvpn/server1/client.ovpn client1.ovpn && cd $(TOP)/www
	cd $(INSTALLDIR)/www && ln -sf ../etc/openvpn/server2/client.ovpn client2.ovpn && cd $(TOP)/www
else
	rm -rf $(INSTALLDIR)/www/ajax_openvpn_server.asp
ifneq ($(RTCONFIG_ACCEL_PPTPD), y)
	rm -rf $(INSTALLDIR)/www/images/New_ui/vpnstatus.png
	rm -rf $(INSTALLDIR)/www/Advanced_VPN_PPTP.asp
	rm -rf $(INSTALLDIR)/www/ajax_vpnserver_client_status.xml
endif
endif
ifneq ($(RTCONFIG_VPNC), y)
	rm -rf $(INSTALLDIR)/www/Advanced_VPNClient_Content.asp
endif
ifneq ($(RTCONFIG_TIMEMACHINE),y)
	rm -rf $(INSTALLDIR)/www/Advanced_TimeMachine.asp
	rm -rf $(INSTALLDIR)/www/images/New_ui/USBExt/time_machine_banner.png
endif
ifneq ($(RTCONFIG_PROXYSTA),y)
ifneq ($(RTCONFIG_WIRELESSREPEATER),y)
	rm -rf $(INSTALLDIR)/www/images/New_ui/re.jpg
	rm -rf $(INSTALLDIR)/www/qis/QIS_sitesurvey.htm
endif
endif

ifneq ($(RTCONFIG_YANDEXDNS),y)
	rm -rf $(INSTALLDIR)/www/YandexDNS.asp
	rm -rf $(INSTALLDIR)/www/images/New_ui/yandex*.png
endif
ifneq ($(RTCONFIG_KYIVSTAR),y)
	rm -rf $(INSTALLDIR)/www/userRpm/SoftwareUpgradeRpm.htm
endif

ifneq ($(RTCONFIG_BWDPI)$(RTCONFIG_BWDPI_DEP),y)
	rm -rf $(INSTALLDIR)/www/AdaptiveQoS_Bandwidth_Monitor.asp
	rm -rf $(INSTALLDIR)/www/AdaptiveQoS_WebHistory.asp
	rm -rf $(INSTALLDIR)/www/AiProtection_HomeSecurity.asp
	rm -rf $(INSTALLDIR)/www/AiProtection_Group.asp
	rm -rf $(INSTALLDIR)/www/AiProtection_HomeProtection.asp
	rm -rf $(INSTALLDIR)/www/AiProtection_WebProtector.asp
	rm -rf $(INSTALLDIR)/www/AiProtection_InfectedDevicePreventBlock.asp
	rm -rf $(INSTALLDIR)/www/AiProtection_InfectedDevicePreventBlock_m.asp
	rm -rf $(INSTALLDIR)/www/AiProtection_IntrusionPreventionSystem.asp
	rm -rf $(INSTALLDIR)/www/AiProtection_IntrusionPreventionSystem_m.asp
	rm -rf $(INSTALLDIR)/www/AiProtection_MaliciousSitesBlocking.asp
	rm -rf $(INSTALLDIR)/www/AiProtection_MaliciousSitesBlocking_m.asp
	rm -rf $(INSTALLDIR)/www/css/adaptive_mobile.css
	rm -rf $(INSTALLDIR)/www/images/New_ui/QoS_quick
	rm -rf $(INSTALLDIR)/www/images/New_ui/infected.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/IPS.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/mals.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/recount.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/recount_hover.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/add.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/add_hover.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/disable.svg
	rm -rf $(INSTALLDIR)/www/images/save.svg
	rm -rf $(INSTALLDIR)/www/images/save_hover.svg
	rm -rf $(INSTALLDIR)/www/images/save_active.svg
	rm -rf $(INSTALLDIR)/www/images/delete.svg
	rm -rf $(INSTALLDIR)/www/images/delete_hover.svg
	rm -rf $(INSTALLDIR)/www/images/delete_active.svg
	rm -rf $(INSTALLDIR)/www/images/edit.svg
	rm -rf $(INSTALLDIR)/www/images/edit_hover.svg
	rm -rf $(INSTALLDIR)/www/images/edit_active.svg
ifneq ($(RTCONFIG_PARENTALCTRL),y)
	rm -rf $(INSTALLDIR)/www/blocking.asp
endif
	rm -rf $(INSTALLDIR)/www/getTraffic.asp
	rm -rf $(INSTALLDIR)/www/getWebHistory.asp
	rm -rf $(INSTALLDIR)/www/getDBList.asp
	rm -rf $(INSTALLDIR)/www/getAppTraffic.asp
	rm -rf $(INSTALLDIR)/www/getWanTraffic.asp
	rm -rf $(INSTALLDIR)/www/getAiProtectionEvent.asp
	rm -rf $(INSTALLDIR)/www/getIPSChart.asp
	rm -rf $(INSTALLDIR)/www/getIPSDetailEvent.asp
	rm -rf $(INSTALLDIR)/www/getIPSEvent.asp
	rm -rf $(INSTALLDIR)/www/getNonIPSChart.asp
	rm -rf $(INSTALLDIR)/www/tm_eula.htm
	rm -rf $(INSTALLDIR)/www/js/Chart.js
	rm -rf $(INSTALLDIR)/www/images/New_ui/HomeProtection.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/TimeLimits.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/Web_Apps_Restriction.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/AiProtection_01.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/AiProtection_02.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/AiProtection_03.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/Home_Protection_Scenario.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/tm_logo.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/TrendMirco_logo.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/tm_logo_1.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/app_icons
ifneq ($(RTCONFIG_ROG_UI),y)
	rm -rf $(INSTALLDIR)/www/images/New_ui/indicator.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/speedmeter.png
endif
endif
ifneq ($(RTCONFIG_ADBLOCK),y)
	rm -rf $(INSTALLDIR)/www/AiProtection_AdBlock.asp
endif

ifneq ($(RTCONFIG_ROG_UI),y)
	rm -rf $(INSTALLDIR)/www/images/ROG_Logo.png
	rm -rf $(INSTALLDIR)/www/rog.css
	rm -rf $(INSTALLDIR)/www/AdaptiveQoS_ROG.asp
	rm -rf $(INSTALLDIR)/www/api.asp
endif

ifneq ($(RTCONFIG_DROPBOXCLIENT),y)
	rm -rf $(INSTALLDIR)/www/images/cloudsync/dropbox.png
endif

ifneq ($(RTCONFIG_FTPCLIENT),y)
ifneq ($(RTCONFIG_SAMBACLIENT),y)
	rm -rf $(INSTALLDIR)/www/images/cloudsync/ftp_server.png
endif
endif

ifneq ($(RTCONFIG_SNMPD), y)
	rm -rf $(INSTALLDIR)/www/Advanced_SNMP_Content.asp
endif

ifneq ($(BUILD_NAME),$(filter $(BUILD_NAME), RT-AC3200 RT-AC5300 GT-AC5300 GT-AC9600 RT-AC86U GT-AC2900 RT-AC88U RT-AX88U RT-AC3100 GT-AX11000 RT-AX92U RT-AX95Q RT-AX56_XD4 CT-AX56_XD4 RT-AX58U TUF-AX3000 DSL-AX82U RT-AX82U RT-AX56U RP-AX56 RT-AX86U RT-AX5700 RT-AX68U GT-AXE11000 RT-AX55 GS-AX3000 GS-AX5400))
	rm -rf $(INSTALLDIR)/www/Advanced_Smart_Connect.asp
	rm -rf $(INSTALLDIR)/www/images/New_ui/smart_connect.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/smart_connect_concept.gif
endif

ifneq ($(RTCONFIG_FINDASUS), y)
	rm -rf $(INSTALLDIR)/www/find_device.asp
	rm -rf $(INSTALLDIR)/www/images/New_ui/wifi-logo.png
endif

ifneq ($(RTCONFIG_JFFS2USERICON), y)
	rm -rf $(INSTALLDIR)/www/ajax_uploadicon.asp
endif

ifneq ($(RTCONFIG_TR069), y)
	rm -rf $(INSTALLDIR)/www/Advanced_TR069_Content.asp
endif

ifneq ($(RTCONFIG_TOR), y)
	rm -rf $(INSTALLDIR)/www/Advanced_TOR_Content.asp
endif

ifneq ($(BUILD_NAME),$(filter $(BUILD_NAME), $(MODELS_HAVE_TRAFFIC_ANALYZER)))
	rm -rf $(INSTALLDIR)/www/TrafficAnalyzer_Statistic.asp
	rm -rf $(INSTALLDIR)/www/images/New_ui/cancel.svg
	rm -rf $(INSTALLDIR)/www/chart.js
	rm -rf $(INSTALLDIR)/www/datepicker.css
endif

ifneq ($(RTCONFIG_KEY_GUARD), y)
	rm -rf $(INSTALLDIR)/www/AiProtection_Key_Guard.asp
	rm -rf $(INSTALLDIR)/www/images/New_ui/Key-Guard.png
endif

ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME), 4G-AC53U 4G-AC55U 4G-AC68U))
	rm -rf $(INSTALLDIR)/www/Advanced_Modem_Content.asp
endif

ifneq ($(BUILD_NAME),$(filter $(BUILD_NAME), RT-AC88U RT-AX88U RT-AC3100 RT-AC5300 GT-AC5300 GT-AC9600 RT-AC86U GT-AC2900 RT-AC3200 GT-AX11000 RT-AX92U RT-AX95Q RT-AX56_XD4 CT-AX56_XD4 RT-AX58U TUF-AX3000 DSL-AX82U RT-AX82U RT-AX56U RT-AX86U RT-AX5700 RT-AX68U GT-AXE11000 GS-AX3000 GS-AX5400))
	rm -rf $(INSTALLDIR)/www/QIS_wizard_m.htm
	rm -rf $(INSTALLDIR)/www/css/effect.css
	rm -rf $(INSTALLDIR)/www/qis_m
	rm -rf $(INSTALLDIR)/www/images/qis/icon_MAC_clone.svg
	rm -rf $(INSTALLDIR)/www/images/qis/logo.png
ifneq ($(BUILD_NAME),$(filter $(BUILD_NAME), RP-AC66 RP-AC68U RP-AC51 MAP-AC1300 MAP-AC2200 VZW-AC1300 MAP-AC1750))
	rm -rf $(INSTALLDIR)/www/css/icon-qis.css
	rm -rf $(INSTALLDIR)/www/images/qis/loading.gif
	rm -rf $(INSTALLDIR)/www/images/qis/mainimage_img4.png
	rm -rf $(INSTALLDIR)/www/images/qis/pattern3-3_10_A15.png
	rm -rf $(INSTALLDIR)/www/images/qis/pattern3_05_4.png
endif
endif

ifneq ($(BUILD_NAME),$(filter $(BUILD_NAME), MAP-AC1300 MAP-AC2200 VZW-AC1300 MAP-AC1750))
	rm -rf $(INSTALLDIR)/www/images/New_ui/asus_hive_block_internet_web.png
endif
ifneq ($(BUILD_NAME),$(filter $(BUILD_NAME), MAP-AC1300 MAP-AC2200 MAP-AC1750))
	rm -rf $(INSTALLDIR)/www/images/qis/trio_io.png
	rm -rf $(INSTALLDIR)/www/images/qis/lyra_io.png
endif

ifneq ($(RTCONFIG_DISK_MONITOR), y)
	rm -rf $(INSTALLDIR)/www/device-map/disk_utility.asp
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/backgroud_move_8P_2.0.gif
endif

ifneq ($(RTCONFIG_WTFAST), y)
	rm -rf $(INSTALLDIR)/www/Advanced_WTFast_Content.asp
	rm -rf $(INSTALLDIR)/www/js/fire.js
	rm -rf $(INSTALLDIR)/www/images/fire.jpg
	rm -rf $(INSTALLDIR)/www/images/wtfast_bg.png
	rm -rf $(INSTALLDIR)/www/images/wtfast_logo.png
	rm -rf $(INSTALLDIR)/www/images/wtfast_device.svg
	rm -rf $(INSTALLDIR)/www/images/wtfast_back.svg
	rm -rf $(INSTALLDIR)/www/images/wtfast_back_hover.svg
	rm -rf $(INSTALLDIR)/www/images/wtfast_device.svg
	rm -rf $(INSTALLDIR)/www/images/wtfast_point.gif
ifneq ($(RTCONFIG_BWDPI)$(RTCONFIG_BWDPI_DEP),y)
	rm -rf $(INSTALLDIR)/www/images/New_ui/delete.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/delete_hover.svg
endif
	rm -rf $(INSTALLDIR)/www/images/New_ui/disable_hover.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/enable.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/enable_hover.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/logout.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/logout_hover.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/upgrade.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/upgrade_hover.svg
endif
	rm -rf $(INSTALLDIR)/www/js/fire.js
ifneq ($(RTCONFIG_WTF_REDEEM)$(RTCONFIG_WTFAST), y)
	rm -rf $(INSTALLDIR)/www/images/New_ui/GameBoost_AiProtection.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/GameBoost_QoS.png
endif

ifneq ($(RTCONFIG_TCPLUGIN), y)
	rm -rf $(INSTALLDIR)/www/images/tencent
	rm -f $(INSTALLDIR)/www/GameBoost_Tencent.asp
	rm -f $(INSTALLDIR)/www/game_accelerator_tencent.html
endif

ifneq ($(RTCONFIG_IPSEC), y)
	rm -rf $(INSTALLDIR)/www/Advanced_VPN_IPSec.asp
	rm -rf $(INSTALLDIR)/www/ajax_ipsec.asp
	rm -rf $(INSTALLDIR)/www/ajax_ipsec_log.xml
	rm -rf $(INSTALLDIR)/www/Advanced_Instant_Guard.asp
endif

ifneq ($(RTCONFIG_CAPTIVE_PORTAL), y)
	rm -rf $(INSTALLDIR)/www/images/New_ui/captive_portal
	rm -rf $(INSTALLDIR)/www/ajax_captive_portal.asp

	rm -rf $(INSTALLDIR)/www/Captive_Portal.asp
	rm -rf $(INSTALLDIR)/www/Captive_Portal.css
	rm -rf $(INSTALLDIR)/www/Captive_Portal_Preview.asp
	rm -rf $(INSTALLDIR)/www/Captive_Portal_template.js

	rm -rf $(INSTALLDIR)/www/Captive_Portal_Advanced.asp
	rm -rf $(INSTALLDIR)/www/Captive_Portal_Advanced.css
	rm -rf $(INSTALLDIR)/www/Captive_Portal_Advanced_Preview.asp
	rm -rf $(INSTALLDIR)/www/Captive_Portal_Advanced_template.js
	rm -rf $(INSTALLDIR)/www/ajax_captive_portal_log.asp
	rm -rf $(INSTALLDIR)/www/jscolor
endif
ifeq ($(RTCONFIG_NOIPTV), y)
	rm -rf $(INSTALLDIR)/www/Advanced_IPTV_Content.asp
endif

ifneq ($(RTCONFIG_TRAFFIC_LIMITER), y)
	rm -rf $(INSTALLDIR)/www/AdaptiveQoS_TrafficLimiter.asp
	rm -rf $(INSTALLDIR)/www/ajax/getTrafficLimiter.asp
endif

ifneq ($(RTCONFIG_IFTTT), y)
	rm -rf $(INSTALLDIR)/www/Advanced_Smart_Home_IFTTT.asp
	rm -rf $(INSTALLDIR)/www/images/New_ui/smh_ifttt.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/smh_ifttt_asus_channel.png
endif
ifneq ($(RTCONFIG_ALEXA), y)
	rm -rf $(INSTALLDIR)/www/Advanced_Smart_Home_Alexa.asp
endif
ifneq ($(or $(RTCONFIG_IFTTT),$(RTCONFIG_ALEXA)),y)
	rm -rf $(INSTALLDIR)/www/images/New_ui/smh_step_1.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/smh_step_2.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/smh_step_2_text.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/smh_step_3.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/smh_step_3_acclink.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/smh_step_4_flow.svg
	rm -rf $(INSTALLDIR)/www/ifttt_style.css
endif

ifneq ($(RTCONFIG_PORT_BASED_VLAN), y)
	rm -rf $(INSTALLDIR)/www/Advanced_VLAN_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_VLAN_DHCP_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_VLAN_Group_Content.asp
endif
ifneq ($(RTCONFIG_TAGGED_BASED_VLAN), y)
	rm -rf $(INSTALLDIR)/www/Advanced_TagBasedVLAN_Content.asp
	rm -rf $(INSTALLDIR)/www/js/subnet_rule.js
	rm -rf $(INSTALLDIR)/www/images/New_ui/pvid.png
endif
ifneq ($(RTCONFIG_FBWIFI), y)
	rm -rf $(INSTALLDIR)/www/Guest_network_fbwifi.asp
	rm -rf $(INSTALLDIR)/www/ajax_fbwifi.asp
	rm -rf $(INSTALLDIR)/www/fbwifi
endif
ifeq ($(RTCONFIG_NOTIFICATION_CENTER), y)
ifneq ($(BUILD_NAME),$(filter $(BUILD_NAME), ))
	rm -rf $(INSTALLDIR)/www/images/New_ui/nt_center
	rm -rf $(INSTALLDIR)/www/notification_center
	rm -rf $(INSTALLDIR)/www/nt_content.json
else
	cp $(INSTALLDIR)/www/notification_center/. $(INSTALLDIR)/www/. -rfL || true
	rm -rf $(INSTALLDIR)/www/notification_center
endif
else
	rm -rf $(INSTALLDIR)/www/images/New_ui/nt_center
	rm -rf $(INSTALLDIR)/www/notification_center
	rm -rf $(INSTALLDIR)/www/nt_content.json
endif

ifeq ($(BUILD_NAME),RT-AC68A)
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_RTAC68A.js modules/menuTree.js
else ifeq ($(BUILD_NAME),RT-AC85U)
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_RT-AC85U.js modules/menuTree.js
else ifeq ($(BUILD_NAME),RT-ACRH26)
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_RT-ACRH26.js modules/menuTree.js
else ifeq ($(BUILD_NAME),RT-AC65U)
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_RT-AC65U.js modules/menuTree.js
else ifeq ($(BUILD_NAME),GT-AXY16000)
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_ROG_AXY16000.js modules/menuTree.js
else ifeq ($(RTCONFIG_ROG_UI),y)
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_ROG.js modules/menuTree.js
else ifeq ($(BUILD_NAME),BRT-AC828)
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_BRTAC828.js modules/menuTree.js
else ifeq ($(BUILD_NAME),BLUECAVE)
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_BLUECAVE.js modules/menuTree.js
else ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME), MAP-AC1300 MAP-AC2200 MAP-AC2200V))
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_LYRA.js modules/menuTree.js
else ifeq ($(BUILD_NAME),MAP-AC1750)
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_TRIO.js modules/menuTree.js
else ifeq ($(BUILD_NAME),VZW-AC1300)
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_VZW-AC1300.js modules/menuTree.js
else ifeq ($(BUILD_NAME),TUF-AX3000)
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_TUF.js modules/menuTree.js	
else ifeq ($(RTCONFIG_BWDPI),y)
ifeq ($(findstring $(BUILD_NAME),$(MODELS_HAVE_TRAFFIC_ANALYZER)),)
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_bwdpi_no_traffic_analyzer.js modules/menuTree.js
else
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_bwdpi_traffic_analyzer.js modules/menuTree.js
endif
else
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_no_bwdpi.js modules/menuTree.js
endif
	rm -rf $(INSTALLDIR)/www/require/menuTrees
ifneq ($(BUILD_NAME),$(filter $(BUILD_NAME), BRT-AC828))
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/M2.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/M2_default.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/icon_Guset_bg.png
endif

ifeq ($(BUILD_NAME),BLUECAVE)
	rm -rf $(INSTALLDIR)/www/images/qis
	rm -rf $(INSTALLDIR)/www/qis
	rm -rf $(INSTALLDIR)/www/qis_m
	rm -rf $(INSTALLDIR)/www/QIS_wizard_m.htm
endif

ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME), GT-AC5300 GT-AC9600 GT-AXY16000 GT-AX11000 TUF-AX3000 GT-AXE11000))
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/white_04.gif
endif

ifneq ($(RTCONFIG_VPN_FUSION), y)
	rm -rf $(INSTALLDIR)/www/images/vpn_illustration.png
	rm -rf $(INSTALLDIR)/www/ajax_vpn_fusion.asp
	rm -rf $(INSTALLDIR)/www/Advanced_VPNClientFusion_Content.asp
else
	cd $(INSTALLDIR)/www/ && mv Advanced_VPNClientFusion_Content.asp Advanced_VPNClient_Content.asp
endif

	ln -sf /tmp/fb_data.tgz.gz $(INSTALLDIR)/www/fb_data.tgz.gz
	ln -sf /tmp/fb_data.tgz.gz.part.a $(INSTALLDIR)/www/fb_data.tgz.gz.part.a
	ln -sf /tmp/fb_data.tgz.gz.part.b $(INSTALLDIR)/www/fb_data.tgz.gz.part.b
	ln -sf /tmp/fb_data.tgz.gz.part.c $(INSTALLDIR)/www/fb_data.tgz.gz.part.c
	ln -sf /tmp/fb_data.tgz.gz.part.d $(INSTALLDIR)/www/fb_data.tgz.gz.part.d

ifneq ($(RTCONFIG_AMAS), y)
	rm -rf $(INSTALLDIR)/www/device-map/amesh.asp
	rm -rf $(INSTALLDIR)/www/device-map/amesh.css
	rm -rf $(INSTALLDIR)/www/images/New_ui/amesh
	rm -rf $(INSTALLDIR)/www/require/modules/amesh.js
	rm -rf $(INSTALLDIR)/www/ajax_onboarding.asp
	rm -rf $(INSTALLDIR)/www/get_release_note_amas.asp
	rm -rf $(INSTALLDIR)/www/chcap.json
	rm -rf $(INSTALLDIR)/www/message.htm
	rm -rf $(INSTALLDIR)/www/AiMesh_Node_FirmwareUpgrade.asp
	rm -rf $(INSTALLDIR)/www/Advanced_Roaming_Block_Content.asp
	rm -rf $(INSTALLDIR)/www/js/jstree/jstree.js
	rm -rf $(INSTALLDIR)/www/AiMesh.asp
	rm -rf $(INSTALLDIR)/www/aimesh
endif
	rm -rf $(INSTALLDIR)/www/Main_ChkSta_Content.asp
	rm -rf $(INSTALLDIR)/www/ajax_chk_sta.asp
ifeq ($(RTCONFIG_TCPDUMP),y)
ifeq ($(RTCONFIG_SOC_IPQ40XX)$(RTCONFIG_PSISTLOG),yy)
	ln -fs /jffs/pppoe.pcap $(INSTALLDIR)/www/pppoe.pcap
	ln -fs /jffs/discover.pcap $(INSTALLDIR)/www/discover.pcap
endif
endif
ifeq ($(RTCONFIG_DUMP4000),y)
	ln -fs /tmp/wan1.pcap $(INSTALLDIR)/www/wan1.pcap
	ln -fs /tmp/wan2.pcap $(INSTALLDIR)/www/wan2.pcap
endif

ifneq ($(RTCONFIG_DBLOG),y)
	rm -rf $(INSTALLDIR)/www/images/New_ui/dblog_diag.gif
endif

ifeq ($(and $(RTCONFIG_AMAS),$(RTCONFIG_WIFI_SON)),y)
	rm -rf $(INSTALLDIR)/www-sys
	mv $(INSTALLDIR)/www/www-sys $(INSTALLDIR)/www-sys
endif
	mkdir -p $(INSTALLDIR)/www

ifneq ($(BUILD_NAME),$(filter $(BUILD_NAME), GT-AC9600 GT-AXY16000 GT-AX11000 GT-AXE11000))
	rm -rf $(INSTALLDIR)/www/qis/QIS_tge_setting.htm
endif

ifneq ($(BUILD_NAME),$(filter $(BUILD_NAME),  GT-AX11000 GT-AC2900 GT-AXY16000 GT-AXE11000))
	rm -rf $(INSTALLDIR)/www/images/New_ui/img-aurasync-logo.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/img-aurasync-rog-logo-mask.png
	rm -rf $(INSTALLDIR)/www/images/aura
endif

ifneq ($(RTCONFIG_UUPLUGIN), y)
	rm -rf $(INSTALLDIR)/www/images/uu_accelerator.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/icon_UU.png
endif

ifneq ($(RTCONFIG_ROG_UI),y)   
	rm -rf $(INSTALLDIR)/www/fonts/ROG_Fonts-Regular.otf
	rm -rf $(INSTALLDIR)/www/fonts/ROG_Fonts-Regular.woff
	rm -rf $(INSTALLDIR)/www/fonts/xolonium.regular.otf
	rm -rf $(INSTALLDIR)/www/fonts/xolonium.regular.woff
	rm -rf $(INSTALLDIR)/www/images/triLvGameAcceleration
	rm -rf $(INSTALLDIR)/www/css/triLvGameAcceleration.css
	rm -rf $(INSTALLDIR)/www/ajax_wanlink.asp
	rm -rf $(INSTALLDIR)/www/GearAccelerator.asp
endif

ifneq ($(RTCONFIG_APP_FILEFLEX), y)
	rm -rf $(INSTALLDIR)/www/fileflex.asp
endif
	ln -sf user/user1.asp $(INSTALLDIR)/www/user1.asp
	ln -sf user/user2.asp $(INSTALLDIR)/www/user2.asp
	ln -sf user/user3.asp $(INSTALLDIR)/www/user3.asp
	ln -sf user/user4.asp $(INSTALLDIR)/www/user4.asp
	ln -sf user/user5.asp $(INSTALLDIR)/www/user5.asp
	ln -sf user/user6.asp $(INSTALLDIR)/www/user6.asp
	ln -sf user/user7.asp $(INSTALLDIR)/www/user7.asp
	ln -sf user/user8.asp $(INSTALLDIR)/www/user8.asp
	ln -sf user/user9.asp $(INSTALLDIR)/www/user9.asp
	ln -sf user/user10.asp $(INSTALLDIR)/www/user10.asp
	ln -sf user/user11.asp $(INSTALLDIR)/www/user11.asp
	ln -sf user/user12.asp $(INSTALLDIR)/www/user12.asp
	ln -sf user/user13.asp $(INSTALLDIR)/www/user13.asp
	ln -sf user/user14.asp $(INSTALLDIR)/www/user14.asp
	ln -sf user/user15.asp $(INSTALLDIR)/www/user15.asp
	ln -sf user/user16.asp $(INSTALLDIR)/www/user16.asp
	ln -sf user/user17.asp $(INSTALLDIR)/www/user17.asp
	ln -sf user/user18.asp $(INSTALLDIR)/www/user18.asp
	ln -sf user/user19.asp $(INSTALLDIR)/www/user19.asp
	ln -sf user/user20.asp $(INSTALLDIR)/www/user20.asp
	rm -rf $(INSTALLDIR)/www/Advanced_Feedback.asp

ifeq ($(REMOVE_ROUTER_UI), y)
	# General
	rm -rf $(INSTALLDIR)/www/AiMesh.asp
	rm -rf $(INSTALLDIR)/www/aimesh/aimesh_topology.css
ifeq ($(REMOVE_AP_UI), y)
	rm -rf $(INSTALLDIR)/www/Guest_network.asp
	rm -rf $(INSTALLDIR)/www/Guest_network.css
endif
	rm -rf $(INSTALLDIR)/www/QoS_EZQoS.asp
	rm -rf $(INSTALLDIR)/www/Advanced_QOSUserRules_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_QOSUserPrio_Content.asp
	rm -rf $(INSTALLDIR)/www/Main_TrafficMonitor_daily.asp
	rm -rf $(INSTALLDIR)/www/Main_TrafficMonitor_last24.asp
	rm -rf $(INSTALLDIR)/www/Main_TrafficMonitor_realtime.asp
	rm -rf $(INSTALLDIR)/www/ParentalControl.asp
	rm -rf $(INSTALLDIR)/www/ParentalControl.css
	# Advanced Settings
ifeq ($(REMOVE_AP_UI), y)
	# Wireless
	rm -rf $(INSTALLDIR)/www/Advanced_Wireless_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_WWPS_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_WMode_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_ACL_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_WSecurity_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_WAdvanced_Content.asp
endif
	# LAN
	rm -rf $(INSTALLDIR)/www/Advanced_DHCP_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_GWStaticRoute_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_IPTV_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_SwitchCtrl_Content.asp
	# WAN
	rm -rf $(INSTALLDIR)/www/Advanced_WAN_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_WANPort_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_PortTrigger_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_VirtualServer_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_Exposed_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_ASUSDDNS_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_NATPassThrough_Content.asp
	# Firewall
	rm -rf $(INSTALLDIR)/www/Advanced_BasicFirewall_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_URLFilter_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_KeywordFilter_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_Firewall_Content.asp
	# System Log
	rm -rf $(INSTALLDIR)/www/Main_DHCPStatus_Content.asp
	rm -rf $(INSTALLDIR)/www/Main_RouteStatus_Content.asp
	rm -rf $(INSTALLDIR)/www/Main_IPTStatus_Content.asp
	rm -rf $(INSTALLDIR)/www/Main_ConnStatus_Content.asp
	# images
	rm -rf $(INSTALLDIR)/www/images/checked_parentctrl.png
	rm -rf $(INSTALLDIR)/www/images/modem_plug.png
	rm -rf $(INSTALLDIR)/www/images/modem_unplug.png
	rm -rf $(INSTALLDIR)/www/images/setup.jpg
	rm -rf $(INSTALLDIR)/www/images/WANunplug_eth.png
	rm -rf $(INSTALLDIR)/www/images/WANunplug.png
	rm -rf $(INSTALLDIR)/www/images/mobile
ifeq ($(REMOVE_AP_UI), y)
	rm -rf $(INSTALLDIR)/www/images/New_ui/ap.jpg
	rm -rf $(INSTALLDIR)/www/images/New_ui/icon_close.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/icon_CopySetting_hover.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/icon_CopySetting.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/icon_GuestNetwork_New_hover.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/icon_GuestNetwork_New.svg
endif
	rm -rf $(INSTALLDIR)/www/images/New_ui/game.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/parental-control.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/QoS.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/rt.jpg
	rm -rf $(INSTALLDIR)/www/images/New_ui/TimeLimitsSmall.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/amesh/AiMesh-introduction.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/amesh/onboardinglist.jpg
	rm -rf $(INSTALLDIR)/www/images/New_ui/amesh/onboardinglist_search.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/amesh/location
	rm -rf $(INSTALLDIR)/www/images/New_ui/new_qis
endif

ifneq ($(BUILD_NAME),$(filter $(BUILD_NAME), GT-AC2900 GT-AC5300 GT-AX11000 GT-AXE11000 GT-AXY16000 RT-AX82U DSL-AX82U GS-AX3000 GS-AX5400))
ifneq ($(RTCONFIG_AMAS),y)
	rm -rf $(INSTALLDIR)/www/fonts
endif
endif

ifneq ($(BUILD_NAME),$(filter $(BUILD_NAME), RT-AX82U DSL-AX82U GT-AXE11000 GT-AC2900 GT-AX11000 GS-AX3000 GS-AX5400))
	rm -rf $(INSTALLDIR)/www/light_effect
	rm -rf $(INSTALLDIR)/www/images/New_ui/light_effect
	rm -rf $(INSTALLDIR)/www/device-map/router_light_effect.asp
endif

ifneq ($(BUILD_NAME),$(filter $(BUILD_NAME), RT-AX82U RT-AX86U))
	rm -rf $(INSTALLDIR)/www/css/gundam.css
endif

ifeq ($(RTCONFIG_ROG_UI),y)
	cp -rf $(INSTALLDIR)/www/css/ROG/* $(INSTALLDIR)/www/css/
	rm -rf $(INSTALLDIR)/www/css/ROG
endif

ifneq ($(RTCONFIG_BCM_OAM), y)
	rm -rf $(INSTALLDIR)/www/Advanced_OAM_Content.asp
endif

ifneq ($(RTCONFIG_OUTFOX), y)
	rm -rf $(INSTALLDIR)/www/images/outfox_dark.png
endif

ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME), TUF-AX3000))
	cp -rf $(INSTALLDIR)/www/css/TUF/* $(INSTALLDIR)/www/css/
	rm -rf $(INSTALLDIR)/www/css/TUF
endif
