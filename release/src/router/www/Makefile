include ../common.mak

ASUSWRTTOOLSDIR = $(SRCBASE)/router/tools/Lnx_AsusWrtDictPrep
ASUSWRTTOOLSDIR2 = $(SRCBASE)/router/tools/Lnx_ToolHelp

MODELS_HAVE_USB3 = AC2900 \
	BRT-AC828 RT-AD7200 \
	DSL-AC68U \
	RT-AC5300 GT-AC5300 GT-AC9600 RT-AC95U RT-AC3100 RT-AC3200 \
	RT-AC55U RT-AC55UHP RT-AC58U 4G-AC53U RT-AC82U RT-AC88Q \
	RT-AC86U RT-AC88U RT-AC87U RT-AC85U RT-AC68U RT-AC68A 4G-AC68U RT-AC65U RT-AC56S RT-AC56U \
	RT-N65U \
	RT-N18U \
	RP-AC68U

MODELS_HAVE_TRAFFIC_ANALYZER = RT-AC3200 RT-AC87U RT-AC88U RT-AC86U AC2900 RT-AC3100 RT-AC5300 GT-AC5300 GT-AC9600 DSL-AC68U RT-AC68U \
	4G-AC68U RT-AC68R RT-AC68W RT-AC68RW RT-AC1900P RT-AC67U RT-AC56U RT-AC56R RT-AC85U RT-AC65U BRT-AC828 BLUECAVE

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

ifeq ($(RTCONFIG_AMAS),y)
	cp $(TOP)/www/sysdep/BLUECAVE/www/Advanced_Wireless_Survey.asp $(INSTALLDIR)/www/Advanced_Wireless_Survey.asp
	cp $(TOP)/www/sysdep/BLUECAVE/www/QIS_wizard.htm $(INSTALLDIR)/www/QIS_wizard.htm
	cp $(TOP)/www/sysdep/BLUECAVE/www/mobile $(INSTALLDIR)/www/mobile -rf
	cp $(TOP)/www/sysdep/BLUECAVE/www/require/modules/modem.js $(INSTALLDIR)/www/require/modules/ -rf
	cp $(TOP)/www/sysdep/downsizeImages/mobile.customize.black $(INSTALLDIR)/www/mobile.customize -rf
	rm -rf $(INSTALLDIR)/www/images/qis
	rm -rf $(INSTALLDIR)/www/qis
	rm -rf $(INSTALLDIR)/www/qis_m
	rm -rf $(INSTALLDIR)/www/QIS_wizard_m.htm
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
	rm -rf $(INSTALLDIR)/www/images/New_ui/Bandwidth_Limiter.png
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
	rm -rf $(INSTALLDIR)/www/images/qis/android*
	rm -rf $(INSTALLDIR)/www/images/qis/iOS*
	rm -rf $(INSTALLDIR)/www/images/qis/mac.jpg
	rm -rf $(INSTALLDIR)/www/images/qis/osx_network_icon.jpg
#	rm -rf $(INSTALLDIR)/www/images/qis/router_login.jpg
	rm -rf $(INSTALLDIR)/www/images/qis/security_key.png
	rm -rf $(INSTALLDIR)/www/images/qis/select_wireless.jpg
	rm -rf $(INSTALLDIR)/www/images/qis/win7.jpg
	rm -rf $(INSTALLDIR)/www/images/qis/win7_network_icon1.jpg
	rm -rf $(INSTALLDIR)/www/images/qis/win7_network_icon2.jpg
	rm -rf $(INSTALLDIR)/www/images/qis/winxp.jpg
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
	rm -rf $(INSTALLDIR)/www/images/qis/security_key.png
	rm -rf $(INSTALLDIR)/www/images/qis/select_wireless.jpg
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/vender-listover.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/Bandwidth_Limiter.png
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
	rm -rf $(INSTALLDIR)/www/images/map.png
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
	rm -rf $(INSTALLDIR)/www/images/New_ui/export/line_export_notif.png
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
	rm -rf $(INSTALLDIR)/www/get_webdavInfo.asp
	rm -rf $(INSTALLDIR)/www/*cloud*.asp
endif
endif
endif
endif
endif

ifneq ($(or $(RTCONFIG_BWDPI),$(RTCONFIG_BWDPI_DEP)),y)
ifneq ($(RTCONFIG_CLOUDSYNC),y)
	rm -rf $(INSTALLDIR)/www/images/line.png
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
	rm -rf $(INSTALLDIR)/www/app_installation.css
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
endif
endif
ifneq ($(RTCONFIG_VPNC), y)	
	rm -rf $(INSTALLDIR)/www/Advanced_VPNClient_Content.asp
endif	
ifneq ($(RTCONFIG_TIMEMACHINE),y)
	rm -rf $(INSTALLDIR)/www/Advanced_TimeMachine.asp
	rm -rf $(INSTALLDIR)/www/images/New_ui/USBExt/time_machine_banner.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/USBExt/TimeMachine.png
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
	rm -rf $(INSTALLDIR)/www/AiProtection_IntrusionPreventionSystem.asp
	rm -rf $(INSTALLDIR)/www/AiProtection_MaliciousSitesBlocking.asp
	rm -rf $(INSTALLDIR)/www/images/New_ui/QoS_quick
	rm -rf $(INSTALLDIR)/www/images/New_ui/infected.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/IPS.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/mals.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/recount.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/recount_hover.svg
ifneq ($(RTCONFIG_PARENTALCTRL),y)
	rm -rf $(INSTALLDIR)/www/blocking.asp
endif
	rm -rf $(INSTALLDIR)/www/getTraffic.asp
	rm -rf $(INSTALLDIR)/www/getWebHistory.asp
	rm -rf $(INSTALLDIR)/www/getAppTraffic.asp
	rm -rf $(INSTALLDIR)/www/getWanTraffic.asp
	rm -rf $(INSTALLDIR)/www/getAiProtectionEvent.asp
	rm -rf $(INSTALLDIR)/www/getIPSChart.asp
	rm -rf $(INSTALLDIR)/www/getIPSDetailEvent.asp
	rm -rf $(INSTALLDIR)/www/getIPSEvent.asp
	rm -rf $(INSTALLDIR)/www/getNonIPSChart.asp
	rm -rf $(INSTALLDIR)/www/tm_eula.htm
	rm -rf $(INSTALLDIR)/www/JP_tm_eula.htm
	rm -rf $(INSTALLDIR)/www/tm_eula_TC.htm
	rm -rf $(INSTALLDIR)/www/tm_eula_SC.htm
	rm -rf $(INSTALLDIR)/www/tm_eula_FR.htm
	rm -rf $(INSTALLDIR)/www/tm_eula_RU.htm
	rm -rf $(INSTALLDIR)/www/js/Chart.js
	rm -rf $(INSTALLDIR)/www/images/New_ui/HomeProtection.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/TimeLimits.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/Web_Apps_Restriction.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/AiProtection_01.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/AiProtection_02.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/AiProtection_03.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/Home_Protection_Scenario.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/tm_logo.png	
	rm -rf $(INSTALLDIR)/www/images/New_ui/tm_logo_power.png	
	rm -rf $(INSTALLDIR)/www/images/New_ui/tm_logo_1.png	
	rm -rf $(INSTALLDIR)/www/images/New_ui/sample_35.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/app_icons
ifneq ($(RTCONFIG_ROG),y)
	rm -rf $(INSTALLDIR)/www/images/New_ui/indicator.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/speedmeter.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/speedmeter_2.png
endif
endif
ifneq ($(RTCONFIG_ADBLOCK),y)
	rm -rf $(INSTALLDIR)/www/AiProtection_AdBlock.asp
endif

ifneq ($(RTCONFIG_ROG),y)
	rm -rf $(INSTALLDIR)/www/images/ROG_Logo.png
	rm -rf $(INSTALLDIR)/www/rog.css
	rm -rf $(INSTALLDIR)/www/AdaptiveQoS_ROG.asp
	rm -rf $(INSTALLDIR)/www/api.asp
endif

ifneq ($(RTCONFIG_DROPBOXCLIENT),y)
	rm -rf $(INSTALLDIR)/www/images/cloudsync/dropbox.png
	rm -rf $(INSTALLDIR)/www/dropbox_callback.htm
endif

ifneq ($(RTCONFIG_FTPCLIENT),y)
ifneq ($(RTCONFIG_SAMBACLIENT),y)
	rm -rf $(INSTALLDIR)/www/images/cloudsync/ftp_server.png
endif
endif

ifneq ($(RTCONFIG_SNMPD), y)	
	rm -rf $(INSTALLDIR)/www/Advanced_SNMP_Content.asp
endif

ifneq ($(BUILD_NAME),$(filter $(BUILD_NAME), RT-AC3200 RT-AC5300 GT-AC5300 GT-AC9600 RT-AC86U AC2900 RT-AC88U RT-AC3100))
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
	rm -rf $(INSTALLDIR)/www/images/New_ui/TrafficAnalyzer.gif
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

ifneq ($(BUILD_NAME),$(filter $(BUILD_NAME), RT-AC88U RT-AC3100 RT-AC5300 GT-AC5300 GT-AC9600 RT-AC86U AC2900 RT-AC3200))
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


ifneq ($(RTCONFIG_DISK_MONITOR), y)
	rm -rf $(INSTALLDIR)/www/device-map/disk_utility.asp
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/backgroud_move_8P_2.0.gif
endif

ifneq ($(RTCONFIG_WTFAST), y)
	rm -rf $(INSTALLDIR)/www/Advanced_WTFast_Content.asp
	rm -rf $(INSTALLDIR)/www/GameBoost.asp
	rm -rf $(INSTALLDIR)/www/js/fire.js
	rm -rf $(INSTALLDIR)/www/images/fire.jpg
	rm -rf $(INSTALLDIR)/www/images/wtfast_bg.png
	rm -rf $(INSTALLDIR)/www/images/wtfast_logo.png
	rm -rf $(INSTALLDIR)/www/images/wtfast_device.svg
	rm -rf $(INSTALLDIR)/www/images/wtfast_back.svg
	rm -rf $(INSTALLDIR)/www/images/wtfast_back_hover.svg
	rm -rf $(INSTALLDIR)/www/images/wtfast_device.svg
	rm -rf $(INSTALLDIR)/www/images/wtfast_point.gif
	rm -rf $(INSTALLDIR)/www/images/GameBoost_AiProtection.png
	rm -rf $(INSTALLDIR)/www/images/GameBoost_QoS.png
	rm -rf $(INSTALLDIR)/www/images/GameBoost_WTFast.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/game.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/mainimage_img_Game.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/pattern3_05.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/pattern3_10.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/add.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/add_hover.svg
ifneq ($(RTCONFIG_BWDPI)$(RTCONFIG_BWDPI_DEP),y)
	rm -rf $(INSTALLDIR)/www/images/New_ui/delete.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/delete_hover.svg
endif
	rm -rf $(INSTALLDIR)/www/images/New_ui/disable.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/enable.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/logout.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/logout_hover.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/upgrade.svg
	rm -rf $(INSTALLDIR)/www/images/New_ui/upgrade_hover.svg	
endif
	rm -rf $(INSTALLDIR)/www/js/fire.js

ifneq ($(RTCONFIG_IPSEC), y)
	rm -rf $(INSTALLDIR)/www/Advanced_VPN_IPSec.asp
	rm -rf $(INSTALLDIR)/www/ajax_ipsec.asp
	rm -rf $(INSTALLDIR)/www/ajax_ipsec_log.xml
endif

ifneq ($(RTCONFIG_IPSEC), y)
	rm -rf $(INSTALLDIR)/www/Advanced_VPN_IPSec.asp
	rm -rf $(INSTALLDIR)/www/ajax_ipsec.asp
	rm -rf $(INSTALLDIR)/www/ajax_ipsec_log.xml
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
	rm -rf $(INSTALLDIR)/www/images/New_ui/smh_asus_router.png
endif
ifneq ($(or $(RTCONFIG_IFTTT),$(RTCONFIG_ALEXA)),y)
	rm -rf $(INSTALLDIR)/www/images/New_ui/smh_step_1.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/smh_step_2.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/smh_step_2_text.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/smh_step_3.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/smh_step_3_acclink.png
	rm -rf $(INSTALLDIR)/www/images/New_ui/smh_step_4_flow.png
	rm -rf $(INSTALLDIR)/www/ifttt_style.css
endif

ifneq ($(RTCONFIG_PORT_BASED_VLAN), y)
	rm -rf $(INSTALLDIR)/www/Advanced_VLAN_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_VLAN_DHCP_Content.asp
	rm -rf $(INSTALLDIR)/www/Advanced_VLAN_Group_Content.asp
endif
ifneq ($(RTCONFIG_TAGGED_BASED_VLAN), y)
	rm -rf $(INSTALLDIR)/www/Advanced_TagBasedVLAN_Content.asp
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
else ifeq ($(BUILD_NAME),AC2900)
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_AC2900.js modules/menuTree.js
else ifeq ($(BUILD_NAME),RT-AC85U)
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_RT-AC85U.js modules/menuTree.js
else ifeq ($(BUILD_NAME),RT-AC65U)
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_RT-AC65U.js modules/menuTree.js
else ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME), GT-AC5300 GT-AC9600))
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_ROG.js modules/menuTree.js
else ifeq ($(BUILD_NAME),BRT-AC828)
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_BRTAC828.js modules/menuTree.js
else ifeq ($(BUILD_NAME),BLUECAVE)
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_BLUECAVE.js modules/menuTree.js
else ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME), MAP-AC1300 MAP-AC2200 VZW-AC1300 MAP-AC1750))
	cd $(INSTALLDIR)/www/require && mv menuTrees/menuTree_LYRA.js modules/menuTree.js
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
endif

ifeq ($(BUILD_NAME),BLUECAVE)
	rm -rf $(INSTALLDIR)/www/images/qis
	rm -rf $(INSTALLDIR)/www/qis
	rm -rf $(INSTALLDIR)/www/qis_m
	rm -rf $(INSTALLDIR)/www/QIS_wizard_m.htm
endif

ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME), GT-AC5300 GT-AC9600))
	rm -rf $(INSTALLDIR)/www/images/New_ui/networkmap/white_04.gif
endif

ifneq ($(RTCONFIG_VPN_FUSION), y)
	rm -rf $(INSTALLDIR)/www/images/New_ui/vpn_illustration.png
	rm -rf $(INSTALLDIR)/www/ajax_vpn_fusion.asp
	rm -rf $(INSTALLDIR)/www/Advanced_VPNClientFusion_Content.asp
else
	cd $(INSTALLDIR)/www/ && mv Advanced_VPNClientFusion_Content.asp Advanced_VPNClient_Content.asp
endif

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
endif

ifeq ($(RTCONFIG_TCPDUMP),y)
ifeq ($(RTCONFIG_SOC_IPQ40XX)$(RTCONFIG_PSISTLOG),yy)
	ln -fs /jffs/pppoe.pcap $(INSTALLDIR)/www/pppoe.pcap
	ln -fs /jffs/discover.pcap $(INSTALLDIR)/www/discover.pcap
endif
endif

ifneq ($(RTCONFIG_DBLOG),y)
	rm -rf $(INSTALLDIR)/www/images/New_ui/dblog_diag.gif
endif
