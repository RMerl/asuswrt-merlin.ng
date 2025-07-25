export GENERAL_BASE := MTLANCFG_MWL=y

#
# Broadcom HND ARM platform
#
export RT-AX88U := $(HND-94908_BASE)
export RT-AX88U += BUILD_NAME="RT-AX88U" SWITCH2="BCM53134" NVSIZE="128" DHDAP=y DPSTA=y LACP=y WTFAST=y REPEATER=y \
		IPV6SUPP=y HTTPS=y ARM=y AUTODICT=y BBEXTRAS=y USBEXTRAS=y EBTABLES=y FRS_FEEDBACK=n ACL96=y ASUSCTRL=y \
		MEDIASRV=y MODEM=y PARENTAL2=y ACCEL_PPTPD=y PRINTER=y WEBDAV=y SMARTSYNCBASE=y USB="USB" APP="network" \
		PROXYSTA=y DNSMQ=y SHP=n BCMWL6=y BCMWL6A=y DISK_MONITOR=y BTN_WIFITOG=y OPTIMIZE_XBOX=y FRS_LIVE_UPDATE=n \
		ODMPID=y LED_BTN=y BCMSMP=y XHCI=y DUALWAN=y  NEW_USER_LOW_RSSI=y OPENVPN=y TIMEMACHINE=y MDNS=y EXTEND_LIMIT=y \
		VPNC=y BRCM_NAND_JFFS2=y JFFS2LOG=y BWDPI=y LINUX_MTD="64" DEBUGFS=y TEMPROOTFS=n SSH=y EMAIL=y GOOGLE_ASST=y \
		SYSSTATE=y ROG=n STAINFO=y CLOUDCHECK=n NATNL_AICLOUD=y REBOOT_SCHEDULE=y MULTICASTIPTV=y QUAGGA=y CALC_NVRAM=y \
		WLCLMLOAD=n BCM_MUMIMO=y LAN50="all" ATCOVER=y GETREALIP=y CFEZ=y ETLAN_LED=y TFAT=y NTFS="tuxera" DNSQUERY_INTERCEPT=n USB_WAN_BACKUP=y \
		HFS="tuxera" NEWSSID_REV2=y NEW_APP_ARM=y BONDING=y NETOOL=y TRACEROUTE=y INFO_EXAP=y WTFAST_V2=y \
		FORCE_AUTO_UPGRADE=n ALEXA=y IFTTT=y SW_HW_AUTH=y HD_SPINDOWN=y ASPMD=n BCM_MEVENT=n BCMEVENTD=n WIREGUARD=y \
		BCM_APPEVENTD=n LETSENCRYPT=y VPN_FUSION=n JFFS_NVRAM=y NVRAM_ENCRYPT=y IPSEC=STRONGSWAN DUMP_OOPS_MSG=n \
		IPSEC_SRVCLI_ONLY=SRV NATNL_AIHOME=y BCM_CEVENTD=y UTF8_SSID=y AMAS=y DBLOG=y ETHOBD=y CONNDIAG=y ASD=y AHS=n \
		AMASDB=n CRASHLOG=y WATCH_REINIT=n AVBLCHAN=y BW160M=y DWB=y BONDING_WAN=y BRCM_HOSTAPD=y UUPLUGIN=y IPERF3=y COMFW=y \
		HSPOT=y INTERNETCTRL=y AMAZON_WSS=y OPEN_NAT=y CAPTCHA=y SW_CTRL_ALLLED=y GAME_MODE=y CFGSYNC_LOCSYNC=y INSTANT_GUARD=y \
		NFS=y SNMPD=n TOR=y UPNPIGD2=y NANO=y NTPD=y

export GT-AX11000 := $(HND-94908_BASE)
export GT-AX11000 += BUILD_NAME="GT-AX11000" EXT_PHY="BCM84880" SWITCH2="" NVSIZE="128" DHDAP=y DPSTA=y LACP=y WTFAST=y REPEATER=y \
		IPV6SUPP=y HTTPS=y ARM=y AUTODICT=y BBEXTRAS=y USBEXTRAS=y EBTABLES=y MEDIASRV=y MODEM=y PARENTAL2=y ACCEL_PPTPD=y \
		PRINTER=y WEBDAV=y SMARTSYNCBASE=y USB="USB" APP="network" PROXYSTA=y DNSMQ=y SHP=n BCMWL6=y BCMWL6A=y \
		DISK_MONITOR=y BTN_WIFITOG=y OPTIMIZE_XBOX=y ODMPID=y LED_BTN=n BCMSMP=y XHCI=y DUALWAN=y NEW_USER_LOW_RSSI=y \
		OPENVPN=y TIMEMACHINE=y MDNS=y VPNC=y BRCM_NAND_JFFS2=y JFFS2LOG=y BWDPI=y DUMP_OOPS_MSG=n LINUX_MTD="64" DEBUGFS=y \
		TEMPROOTFS=n SSH=y EMAIL=y FRS_FEEDBACK=n SYSSTATE=y ROG=y STAINFO=y CLOUDCHECK=n NATNL_AICLOUD=y REBOOT_SCHEDULE=y \
		MULTICASTIPTV=y QUAGGA=y WLCLMLOAD=n BCM_MUMIMO=y LAN50="all" ATCOVER=y GETREALIP=y CFEZ=y ETLAN_LED=y TFAT=y \
		NTFS="tuxera" HFS="tuxera" NEWSSID_REV2=y NEW_APP_ARM=y BONDING=y BONDING_WAN=y NETOOL=y \
		TRACEROUTE=y FORCE_AUTO_UPGRADE=n ALEXA=y IFTTT=y SW_HW_AUTH=y HD_SPINDOWN=y ASPMD=n BCM_MEVENT=n BCMEVENTD=n \
		BCM_APPEVENTD=n LETSENCRYPT=y VPN_FUSION=n JFFS_NVRAM=y NVRAM_ENCRYPT=y NATNL_AIHOME=y BTN_TURBO=y LOGO_LED=y \
		BCM_CEVENTD=y UTF8_SSID=y RGBLED=y AURASYNC=y I2CTOOLS=y AMAS=y DBLOG=y ETHOBD=y CONNDIAG=y WIREGUARD=y  WTFAST_V2=y \
		IPSEC=STRONGSWAN IPSEC_SRVCLI_ONLY=SRV DWB=y GAME_PROFILE=y ROG_UI=n AMASDB=n CRASHLOG=y WATCH_REINIT=n BW160M=y \
		BRCM_HOSTAPD=y UUPLUGIN=y IPERF3=y INFO_EXAP=y OPEN_NAT=y AHS=n ASD=y FRS_LIVE_UPDATE=n CAPTCHA=y SW_CTRL_ALLLED=y \
		GAME_MODE=y CFGSYNC_LOCSYNC=y INSTANT_GUARD=y AMAS_ETHDETECT=y ACL96=y GOOGLE_ASST=y EXTEND_LIMIT=y ASUSCTRL=y \
		IPV6S46=y COMFW=y CALC_NVRAM=y DNSQUERY_INTERCEPT=n USB_WAN_BACKUP=y OCNVC=y MAX_VPNF=4 DSLITE=n OCNVC=n V6OPTION=n \
		NFS=y SNMPD=n TOR=y UPNPIGD2=y NANO=y NTPD=y


export HND-963178_BASE := HND_ROUTER=y PROFILE="96750GW" SAMBA3="3.6.x" OOKLA=y WL_SCHED_V3=y HND_ROUTER_AX_675X=y CABLEDIAG=y
export RT-AX58U := $(HND-963178_BASE)
export RT-AX58U += BUILD_NAME="RT-AX58U" NVSIZE="128" DHDAP=y HND_WL=y DPSTA=y LACP=n WTFAST=n REPEATER=y SW_CTRL_ALLLED=y \
		IPV6SUPP=y HTTPS=y ARM=y AUTODICT=y BBEXTRAS=y USBEXTRAS=y EBTABLES=y MEDIASRV=y MODEM=y PARENTAL2=y ACCEL_PPTPD=y \
		PRINTER=y WEBDAV=y SMARTSYNCBASE=y USB="USB" APP="network" PROXYSTA=y DNSMQ=y SHP=n BCMWL6=y BCMWL6A=y GAME_MODE=n \
		DISK_MONITOR=y BTN_WIFITOG=n OPTIMIZE_XBOX=y ODMPID=y LED_BTN=n BCMSMP=y XHCI=y DUALWAN=y NEW_USER_LOW_RSSI=y \
		OPENVPN=y TIMEMACHINE=y MDNS=y VPNC=y BRCM_NAND_JFFS2=y JFFS2LOG=y NOTIFICATION_CENTER=y BWDPI=y DUMP_OOPS_MSG=n \
		LINUX_MTD="64" DEBUGFS=y TEMPROOTFS=n SSH=y EMAIL=y FRS_FEEDBACK=n SYSSTATE=y ROG=n STAINFO=y CLOUDCHECK=n \
		NATNL_AICLOUD=y REBOOT_SCHEDULE=y MULTICASTIPTV=y QUAGGA=y WLCLMLOAD=n BCM_MUMIMO=y LAN50="all" ATCOVER=y AHS=n ASD=y \
		GETREALIP=y CFEZ=y TFAT=y NTFS="tuxera" HFS="tuxera" NEWSSID_REV2=y NEWSSID_REV4=y LAN4WAN_LED=y NEW_APP_ARM=y \
		BONDING=n BONDING_WAN=n NETOOL=y TRACEROUTE=y FORCE_AUTO_UPGRADE=n ALEXA=y IFTTT=y SW_HW_AUTH=y \
		HD_SPINDOWN=y ASPMD=n BCM_MEVENT=n BCMEVENTD=y BCM_APPEVENTD=n LETSENCRYPT=y VPN_FUSION=n JFFS_NVRAM=y INSTANT_GUARD=y \
		NVRAM_ENCRYPT=y IPSEC=STRONGSWAN IPSEC_SRVCLI_ONLY=SRV NATNL_AIHOME=y BCM_CEVENTD=y UTF8_SSID=y AMAS=y DWB=y \
		DBLOG=y ETHOBD=y CFGSYNC_LOCSYNC=y CONNDIAG=y CRASHLOG=y WATCH_REINIT=n BW160M=y BRCM_HOSTAPD=y UUPLUGIN=n \
		IPERF3=y FRS_LIVE_UPDATE=n ASUSCTRL=y INFO_EXAP=y AVBLCHAN=y INTERNETCTRL=y BCN_RPT=y BTM_11V=y \
		ACL96=y GOOGLE_ASST=y COMFW=n WIREGUARD=y IPV6S46=y HND_ROUTER_AX_675X=y CABLEDIAG=y CALC_NVRAM=y USB_WAN_BACKUP=y OCNVC=y \
		NFS=y SNMPD=n TOR=y UPNPIGD2=y NANO=y NTPD=y

export HND-947622_BASE := HND_ROUTER=y PROFILE="947622GW" SAMBA3="3.6.x" OOKLA=y WL_SCHED_V3=y HND_ROUTER_AX_675X=y CABLEDIAG=y
export RT-AX56U := $(HND-947622_BASE)
export RT-AX56U += BUILD_NAME="RT-AX56U" SWITCH2="BCM53134" NVSIZE="128" DHDAP=y HND_WL=y DPSTA=y LACP=n WTFAST=n REPEATER=y SW_CTRL_ALLLED=y \
		IPV6SUPP=y HTTPS=y ARM=y AUTODICT=y BBEXTRAS=y USBEXTRAS=y EBTABLES=y MEDIASRV=y MODEM=y BCN_RPT=y BTM_11V=y CONNDIAG=y \
		PARENTAL2=y ACCEL_PPTPD=y PRINTER=y WEBDAV=y SMARTSYNCBASE=y USB="USB" APP="network" PROXYSTA=y DNSMQ=y SHP=n BCMWL6=y \
		BCMWL6A=y DISK_MONITOR=y BTN_WIFITOG=n OPTIMIZE_XBOX=y ODMPID=y LED_BTN=n BCMSMP=y XHCI=y DUALWAN=y  NEW_USER_LOW_RSSI=y \
		OPENVPN=y TIMEMACHINE=y MDNS=y VPNC=y BRCM_NAND_JFFS2=y JFFS2LOG=y BWDPI=y DUMP_OOPS_MSG=n LINUX_MTD="64" DEBUGFS=y \
		TEMPROOTFS=n SSH=y EMAIL=y FRS_FEEDBACK=n SYSSTATE=y ROG=n STAINFO=y CLOUDCHECK=n NATNL_AICLOUD=y REBOOT_SCHEDULE=y \
		MULTICASTIPTV=y QUAGGA=y WLCLMLOAD=n BCM_MUMIMO=y LAN50="all" ATCOVER=y GETREALIP=y CFEZ=y ETLAN_LED=y TFAT=y NTFS="tuxera" \
		HFS="tuxera" NEWSSID_REV2=y NEWSSID_REV4=y NEW_APP_ARM=y BONDING=n BONDING_WAN=n NETOOL=y TRACEROUTE=y \
		FORCE_AUTO_UPGRADE=n ALEXA=y IFTTT=y SW_HW_AUTH=y HD_SPINDOWN=y ASPMD=n BCM_MEVENT=n BCMEVENTD=n BCM_APPEVENTD=n \
		LETSENCRYPT=y VPN_FUSION=n JFFS_NVRAM=y NVRAM_ENCRYPT=y IPSEC=STRONGSWAN IPSEC_SRVCLI_ONLY=SRV NATNL_AIHOME=y \
		BCM_CEVENTD=y UTF8_SSID=y AMAS=y DWB=y DBLOG=y ETHOBD=y CRASHLOG=y WATCH_REINIT=n BW160M=n USB_WAN_BACKUP=y \
		BRCM_HOSTAPD=y UUPLUGIN=n IPERF3=y INFO_EXAP=y FRS_LIVE_UPDATE=n AVBLCHAN=y INSTANT_GUARD=y AHS=n ASD=y \
		ACL96=y GOOGLE_ASST=y ASUSCTRL=y WIREGUARD=y IPV6S46=y HND_ROUTER_AX_675X=y CABLEDIAG=y CALC_NVRAM=y  COMFW=y OCNVC=y \
		NFS=y SNMPD=n TOR=y UPNPIGD2=y NANO=y NTPD=y

export HND-962118_BASE := HND_ROUTER=y PROFILE="962118GW" SAMBA3="3.6.x" TFAT=y NTFS="tuxera" CABLEDIAG=y \
			HFS="tuxera" LACP=y BONDING=y BONDING_WAN=y OOKLA=y WL_SCHED_V3=y HND_ROUTER_AX_6710=y
export RT-AX86U := $(HND-962118_BASE) ETLAN_LED=y LAN4WAN_LED=y LED_BTN=y I2CTOOLS=y GEFORCENOW=y \
		UUPLUGIN=y TCPLUGIN=y GAME_MODE=y IPV6S46=y

export RT-AX86U += BUILD_NAME="RT-AX86U" EXT_PHY="BCM84880" NVSIZE="128" DHDAP=y HND_WL=y \
		DPSTA=y REPEATER=y IPV6SUPP=y HTTPS=y ARM=y AUTODICT=y AMAS_ETHDETECT=y \
		BBEXTRAS=y USBEXTRAS=y EBTABLES=y MEDIASRV=y MODEM=y PARENTAL2=y AHS=n ASD=y \
		ACCEL_PPTPD=y PRINTER=y WEBDAV=y SMARTSYNCBASE=y USB="USB" APP="network" \
		PROXYSTA=y DNSMQ=y BCMWL6=y BCMWL6A=y DISK_MONITOR=y OPTIMIZE_XBOX=y ODMPID=y \
		BCMSMP=y XHCI=y DUALWAN=y NEW_USER_LOW_RSSI=y OPENVPN=y TIMEMACHINE=y MDNS=y \
		VPNC=y BRCM_NAND_JFFS2=y JFFS2LOG=y NOTIFICATION_CENTER=y BWDPI=y COMFW=y \
		DUMP_OOPS_MSG=n LINUX_MTD="64" DEBUGFS=y SSH=y EMAIL=y FRS_FEEDBACK=n \
		SYSSTATE=y STAINFO=y CLOUDCHECK=n NATNL_AICLOUD=y REBOOT_SCHEDULE=y \
		MULTICASTIPTV=y QUAGGA=y BCM_MUMIMO=y LAN50="all" ATCOVER=y GETREALIP=y CFEZ=y \
		NEWSSID_REV2=y NEWSSID_REV4=y NEW_APP_ARM=y NETOOL=y TRACEROUTE=y OCNVC=y \
		FORCE_AUTO_UPGRADE=n ALEXA=y IFTTT=n SW_HW_AUTH=y HD_SPINDOWN=y BCMEVENTD=y \
		LETSENCRYPT=y JFFS_NVRAM=y NVRAM_ENCRYPT=y IPSEC=STRONGSWAN \
		IPSEC_SRVCLI_ONLY=SRV NATNL_AIHOME=y BCM_CEVENTD=y UTF8_SSID=y AMAS=y DWB=y \
		DBLOG=y ETHOBD=y CFGSYNC_LOCSYNC=y CONNDIAG=y CRASHLOG=y BW160M=y USB_WAN_BACKUP=y \
		BRCM_HOSTAPD=y IPERF3=y FRS_LIVE_UPDATE=n BCN_RPT=y BTM_11V=y INFO_EXAP=y \
		AVBLCHAN=y OPEN_NAT=y SW_CTRL_ALLLED=y INSTANT_GUARD=y HND_ROUTER_AX_6710=y \
		CABLEDIAG=y CALC_NVRAM=y ACL96=y GOOGLE_ASST=y EXTEND_LIMIT=y ASUSCTRL=y WIREGUARD=y \
		NFS=y SNMPD=y TOR=y UPNPIGD2=y NANO=y NTPD=y

export RT-AX68U := $(HND-962118_BASE) ETLAN_LED=y UUPLUGIN=y TCPLUGIN=y
export RT-AX68U += BUILD_NAME="RT-AX68U" NVSIZE="128" DHDAP=n HND_WL=y DPSTA=y REPEATER=y IPV6SUPP=y HTTPS=y ARM=y \
		AUTODICT=y BBEXTRAS=y USBEXTRAS=y EBTABLES=y MEDIASRV=y MODEM=y PARENTAL2=y ACCEL_PPTPD=y PRINTER=y \
		WEBDAV=y SMARTSYNCBASE=y USB="USB" APP="network" PROXYSTA=y DNSMQ=y BCMWL6=y BCMWL6A=y DISK_MONITOR=y \
		OPTIMIZE_XBOX=y ODMPID=y BCMSMP=y XHCI=y DUALWAN=y NEW_USER_LOW_RSSI=y OPENVPN=y TIMEMACHINE=y MDNS=y \
		VPNC=y BRCM_NAND_JFFS2=y JFFS2LOG=y NOTIFICATION_CENTER=y BWDPI=y DUMP_OOPS_MSG=n LINUX_MTD="64" \
		DEBUGFS=y SSH=y EMAIL=y FRS_FEEDBACK=n SYSSTATE=y STAINFO=y CLOUDCHECK=y NATNL_AICLOUD=y ASD=y \
		REBOOT_SCHEDULE=y MULTICASTIPTV=y QUAGGA=y BCM_MUMIMO=y LAN50="all" ATCOVER=y GETREALIP=y CFEZ=y \
		NEWSSID_REV2=y NEWSSID_REV4=y NEW_APP_ARM=y NETOOL=y TRACEROUTE=y FORCE_AUTO_UPGRADE=n ALEXA=y IFTTT=n \
		SW_HW_AUTH=y HD_SPINDOWN=y BCMEVENTD=y LETSENCRYPT=y JFFS_NVRAM=y NVRAM_ENCRYPT=y IPSEC=STRONGSWAN \
		IPSEC_SRVCLI_ONLY=SRV NATNL_AIHOME=y BCM_CEVENTD=y UTF8_SSID=y AMAS=y DWB=y DBLOG=y ETHOBD=y \
		CFGSYNC_LOCSYNC=y CONNDIAG=y CRASHLOG=y BRCM_HOSTAPD=y IPERF3=y FRS_LIVE_UPDATE=n BCN_RPT=y \
		BTM_11V=y INFO_EXAP=y AVBLCHAN=y SW_CTRL_ALLLED=y INSTANT_GUARD=y PC_SCHED_V3=y \
		ACL96=y GOOGLE_ASST=y WIREGUARD=y HND_ROUTER_AX_6710=y CABLEDIAG=y CALC_NVRAM=y COMFW=y USB_WAN_BACKUP=y \
		NFS=y SNMPD=y TOR=y UPNPIGD2=y NANO=y NTPD=y

export RT-AC68U_V4 := HND_ROUTER=y PROFILE="962118GW" SAMBA3="3.6.x" TFAT=y NTFS="tuxera" HFS="tuxera" OOKLA=y WL_SCHED_V3=y
export RT-AC68U_V4 += BUILD_NAME="RT-AC68U_V4" NVSIZE="128" DHDAP=n HND_WL=y DPSTA=y REPEATER=y IPV6SUPP=y HTTPS=y ARM=y \
                      AUTODICT=y BBEXTRAS=y USBEXTRAS=y EBTABLES=y MEDIASRV=y MODEM=y PARENTAL2=y ACCEL_PPTPD=y PRINTER=y \
                      WEBDAV=y SMARTSYNCBASE=y USB="USB" APP="network" PROXYSTA=y DNSMQ=y BCMWL6=y BCMWL6A=y \
                      DISK_MONITOR=y BTN_WIFITOG=y OPTIMIZE_XBOX=y ODMPID=y LED_BTN=y BCMSMP=y XHCI=y DUALWAN=y \
                      NEW_USER_LOW_RSSI=y OPENVPN=y TIMEMACHINE=y MDNS=y VPNC=y BRCM_NAND_JFFS2=y JFFS2LOG=y \
                      NOTIFICATION_CENTER=y BWDPI=y DUMP_OOPS_MSG=n LINUX_MTD="64" DEBUGFS=y SSH=y EMAIL=y FILEFLEX=y \
                      FRS_FEEDBACK=n SYSSTATE=y STAINFO=y CLOUDCHECK=y NATNL_AICLOUD=y REBOOT_SCHEDULE=y MULTICASTIPTV=y \
                      QUAGGA=y LAN50="all" ATCOVER=y GETREALIP=y CFEZ=y NEWSSID_REV2=y NEWSSID_REV4=y LAN4WAN_LED=y \
                      NEW_APP_ARM=y NETOOL=y TRACEROUTE=y FORCE_AUTO_UPGRADE=n ALEXA=n IFTTT=n SW_HW_AUTH=y HD_SPINDOWN=y \
                      BCMEVENTD=y LETSENCRYPT=y JFFS_NVRAM=y NVRAM_ENCRYPT=y NATNL_AIHOME=y BCM_CEVENTD=y UTF8_SSID=y \
                      AMAS=y DWB=y DBLOG=y ETHOBD=y CFGSYNC_LOCSYNC=y CONNDIAG=y NFCM=n CRASHLOG=y BRCM_HOSTAPD=y \
                      UUPLUGIN=y IPERF3=y FRS_LIVE_UPDATE=n BCN_RPT=y BTM_11V=y INFO_EXAP=y AVBLCHAN=y SW_CTRL_ALLLED=y \
                      INSTANT_GUARD=y PC_SCHED_V3=y GAME_MODE=y AHS=n ASD=y OPEN_NAT=y KERN_SIZE_OPT=y ASUSCTRL=y \
                      HND_ROUTER_AX_6710=y COMFW=n \
                      NFS=y SNMPD=n TOR=y UPNPIGD2=n NANO=y NTPD=y

export GT-AXE11000 := $(HND-962118_BASE) ETLAN_LED=y I2CTOOLS=y GEFORCENOW=y APFS="tuxera"
export GT-AXE11000 += BUILD_NAME="GT-AXE11000" EXT_PHY="BCM84880" NVSIZE="128" DHDAP=y DPSTA=y REPEATER=y IPV6SUPP=y \
                      HTTPS=y ARM=y AUTODICT=y BBEXTRAS=y USBEXTRAS=y EBTABLES=y MEDIASRV=y MODEM=y PARENTAL2=y ACCEL_PPTPD=y \
                      PRINTER=y WEBDAV=y SMARTSYNCBASE=y USB="USB" APP="network" PROXYSTA=y DNSMQ=y SHP=n BCMWL6=y \
                      BCMWL6A=y DISK_MONITOR=y BTN_WIFITOG=y OPTIMIZE_XBOX=y ODMPID=y LED_BTN=n BCMSMP=y XHCI=y DUALWAN=y \
                      NEW_USER_LOW_RSSI=y OPENVPN=y TIMEMACHINE=y MDNS=y VPNC=y BRCM_NAND_JFFS2=y JFFS2LOG=y BWDPI=y \
                      DUMP_OOPS_MSG=n LINUX_MTD="64" DEBUGFS=y TEMPROOTFS=n SSH=y EMAIL=y FRS_FEEDBACK=n SYSSTATE=y ROG=y \
                      STAINFO=y CLOUDCHECK=n NATNL_AICLOUD=y REBOOT_SCHEDULE=y MULTICASTIPTV=y QUAGGA=y WLCLMLOAD=n IPV6S46=y \
                      BCM_MUMIMO=y LAN50="all" ATCOVER=y GETREALIP=y CFEZ=y ETLAN_LED=y NEWSSID_REV4=y NEW_APP_ARM=y \
                      NETOOL=y TRACEROUTE=y FORCE_AUTO_UPGRADE=n ALEXA=y IFTTT=y SW_HW_AUTH=y HD_SPINDOWN=y \
                      ASPMD=n BCM_MEVENT=n BCMEVENTD=y BCM_APPEVENTD=n LETSENCRYPT=y VPN_FUSION=n JFFS_NVRAM=y NVRAM_ENCRYPT=y \
                      NATNL_AIHOME=y BTN_TURBO=y LOGO_LED=y BCM_CEVENTD=y UTF8_SSID=y RGBLED=y AURASYNC=y I2CTOOLS=y AMAS=y \
                      DBLOG=y ETHOBD=y CFGSYNC_LOCSYNC=y CONNDIAG=y IPSEC=STRONGSWAN IPSEC_SRVCLI_ONLY=SRV DWB=y OPEN_NAT=y \
                      ROG_UI=n AMASDB=n CRASHLOG=y WATCH_REINIT=n BW160M=y BRCM_HOSTAPD=y UUPLUGIN=n IPERF3=y INFO_EXAP=y \
                      BCN_RPT=y BTM_11V=y INTERNETCTRL=y ASD=y AHS=n FRS_LIVE_UPDATE=n HAS_5G_2=y CAPTCHA=y WIFI6E=y MAX_VPNF=4 \
                      GAME_MODE=y AMAS_ETHDETECT=y INSTANT_GUARD=y AVBLCHAN=y FRONTHAUL_DWB=y OWE_TRANS=y HSPOT=y ACL96=y WTFAST_V2=y \
                      GOOGLE_ASST=y EXTEND_LIMIT=y WIREGUARD=y HND_ROUTER_AX_6710=y CABLEDIAG=y COMFW=y USB_WAN_BACKUP=y OCNVC=y \
                      NFS=y SNMPD=n TOR=y UPNPIGD2=y NANO=y NTPD=y

#
# Broadcom SDK 7.14 ARM platform
#
export RT-AC88U_BASE := IPV6SUPP=y HTTPS=y ARM=y AUTODICT=y BBEXTRAS=y USBEXTRAS=y EBTABLES=y SAMBA3=3.6.x \
		MEDIASRV=y MODEM=y PARENTAL2=y ACCEL_PPTPD=y PRINTER=y WEBDAV=y FRS_FEEDBACK=n \
		NETOOL=y SW_HW_AUTH=y IFTTT=y ALEXA=y LETSENCRYPT=y IPSEC=STRONGSWAN IPSEC_SRVCLI_ONLY=SRV \
		USB="USB" GRO=y APP="network" DNSMQ=y SHP=y BCMWL6=y BCMWL6A=y BCM5301X=y EXT4FS=y \
		DISK_MONITOR=y BTN_WIFITOG=y OPTIMIZE_XBOX=y LED_BTN=y ODMPID=y BCMSMP=y XHCI=y  \
		DUALWAN=y REPEATER=y NEW_USER_LOW_RSSI=y OPENVPN=y TIMEMACHINE=y MDNS=y VPNC=y BRCM_NAND_JFFS2=y \
		REBOOT_SCHEDULE=y FORCE_AUTO_UPGRADE=n FAKEID=n MULTICASTIPTV=y QUAGGA=y HD_SPINDOWN=y \
		JFFS2LOG=y BWDPI=y DUMP_OOPS_MSG=y LINUX_MTD="64" DEBUGFS=y TEMPROOTFS=y SSH=y WIFILOGO=n \
		EMAIL=y ROG=y SSD=n STAINFO=y BCM5301X_TRAFFIC_MONITOR=y PROXYSTA=y AMAS=y AVBLCHAN=y \
		BCM_RECVFILE=y TFAT=y NTFS="tuxera" HFS="tuxera" CFEZ=y ERPTEST=n CLOUDCHECK=n \
		ASPMD=n BCMEVENTD=n WLCLMLOAD=n BCM_MUMIMO=y LAN50=y ATCOVER=y GETREALIP=y \
		NOTIFICATION_CENTER=y NATNL_AICLOUD=y SYSSTATE=y SMARTSYNCBASE=y NATNL_AIHOME=y \
		CONNTRACK=n AMASDB=y WATCH_REINIT=n MFGFW=n CTFNAT=y CONNDIAG=y UTF8_SSID=y \
		UUPLUGIN=y SW_CTRL_ALLLED=y OOKLA=y WL_SCHED_V3=y \
		NFS=y SNMPD=y TOR=y UPNPIGD2=n NANO=y NTPD=y \
		TRACEROUTE=n FAKEHDR=y FORCE_SN=386 FORCE_EN=40000 NVRAM_ENCRYPT=y

export RT-AC88U := $(RT-AC88U_BASE)
export RT-AC88U += BUILD_NAME="RT-AC88U" RGMII_BRCM5301X=y SWITCH2="RTL8365MB" BCM_MMC=n BCM_7114=y \
		ETLAN_LED=y ROMCFE=n NVSIZE="128" DHDAP=y DPSTA=y USB_DEBUG=n BCMFA=n GMAC3=y \
		LACP=y RESET_SWITCH=n WTFAST=y AHS=n ASD=y BW160M=y INSTANT_GUARD=y GOOGLE_ASST=y \
		NEWSSID_REV2=y ASUSCTRL=y FRS_LIVE_UPDATE=n OPEN_NAT=y GAME_MODE=y JFFS_NVRAM=y

export RT-AC3100 := $(RT-AC88U_BASE)
export RT-AC3100 += BUILD_NAME="RT-AC3100" SWITCH2="" BCM_MMC=n BCM_7114=y ETLAN_LED=y ROMCFE=n \
		NVSIZE="128" DHDAP=y DPSTA=y BCMFA=n GMAC3=y LACP=y WTFAST=y AHS=n ASD=y \
		NEWSSID_REV2=y ASUSCTRL=y FRS_LIVE_UPDATE=n BW160M=y IPSEC=STRONGSWAN \
		IPSEC_SRVCLI_ONLY=SRV INSTANT_GUARD=y GOOGLE_ASST=y

export RT-AC5300 := $(RT-AC88U_BASE)
export RT-AC5300 += BUILD_NAME="RT-AC5300" RGMII_BRCM5301X=y SWITCH2="" BCM_MMC=n BCM_7114=y ETLAN_LED=y \
		NVSIZE="128" DHDAP=y DPSTA=y BCMFA=n GMAC3=y LACP=y WTFAST=y AHS=n ASD=y \
		BW160M=y INSTANT_GUARD=y JFFS_NVRAM=y UUPLUGIN=y HAS_5G_2=y FRS_LIVE_UPDATE=n \
		GAME_MODE=y GOOGLE_ASST=y


#
# Broadcom HND 5.04
#

export HND-94912_BASE := HND_ROUTER=y PROFILE="94912GW" SW_HW_AUTH=y AMAS=y UTF8_SSID=y ETHOBD=y DWB=y \
			CONNDIAG=y SAMBA3="3.6.x" OOKLA=y WL_SCHED_V3=y UBI=y UBIFS=y PSISTLOG=y CABLEDIAG=y

export GT-AX6000 := $(HND-94912_BASE)
export GT-AX6000 += BUILD_NAME="GT-AX6000" EXT_PHY="BCM84880" NVSIZE="128" DHDAP=y DPSTA=n REPEATER=y IPV6SUPP=y \
		HTTPS=y ARM=y AUTODICT=y BBEXTRAS=y USBEXTRAS=y EBTABLES=y MEDIASRV=y MODEM=y PARENTAL2=y \
		ACCEL_PPTPD=y PRINTER=y WEBDAV=y SMARTSYNCBASE=y USB="USB" APP="network" PROXYSTA=y \
		DNSMQ=y SHP=n BCMWL6=y BCMWL6A=y DISK_MONITOR=y BTN_WIFITOG=n OPTIMIZE_XBOX=y ODMPID=y \
		LED_BTN=y BCMSMP=y XHCI=y DUALWAN=y NEW_USER_LOW_RSSI=y OPENVPN=y TIMEMACHINE=y MDNS=y \
		VPNC=y JFFS2LOG=y BWDPI=y DUMP_OOPS_MSG=n LINUX_MTD="64" DEBUGFS=y TEMPROOTFS=n SSH=y EMAIL=y \
		FRS_FEEDBACK=n SYSSTATE=y ROG=y STAINFO=y CLOUDCHECK=n NATNL_AICLOUD=y REBOOT_SCHEDULE=y \
		MULTICASTIPTV=y QUAGGA=y WLCLMLOAD=n BCM_MUMIMO=y LAN50="all" ATCOVER=y GETREALIP=y CFEZ=y \
		ETLAN_LED=y TFAT=y NTFS="tuxera" HFS="tuxera" NEWSSID_REV4=y NEW_APP_ARM=y \
		NETOOL=y TRACEROUTE=y FORCE_AUTO_UPGRADE=n ALEXA=y SW_HW_AUTH=y HD_SPINDOWN=y ASPMD=n \
		BCM_MEVENT=n BCMEVENTD=y BCM_APPEVENTD=n LETSENCRYPT=y VPN_FUSION=y JFFS_NVRAM=y \
		CABLEDIAG=y NVRAM_ENCRYPT=y NATNL_AIHOME=y LOGO_LED=n BCM_CEVENTD=y UTF8_SSID=y \
		RGBLED=n I2CTOOLS=n AMAS=y DBLOG=y ETHOBD=y CFGSYNC_LOCSYNC=y CONNDIAG=y \
		IPSEC=STRONGSWAN IPSEC_SRVCLI_ONLY=SRV DWB=y OPEN_NAT=y ROG_UI=n AMASDB=n CRASHLOG=y \
		WATCH_REINIT=n BW160M=y BRCM_HOSTAPD=y UUPLUGIN=y IPERF3=y INFO_EXAP=y BCN_RPT=y BTM_11V=y \
		INTERNETCTRL=y ASD=y AHS=n FRS_LIVE_UPDATE=n ASUSCTRL=y HAS_5G_2=n CAPTCHA=y \
		GAME_MODE=y AMAS_ETHDETECT=y INSTANT_GUARD=y AVBLCHAN=y FRONTHAUL_DWB=n BCM_CLED=n \
		SW_CTRL_ALLLED=y BONDING=y BONDING_WAN=y ACL96=y LACP=y DHD_6715B0=y GOOGLE_ASST=y \
		WIREGUARD=y COMFW=y USB_WAN_BACKUP=y IG_SITE2SITE=n MAX_VPNF=4 WTFAST_V2=y ADGUARDDNS=y \
		CAPTIVE_PORTAL=y CP_FREEWIFI=y MTLANCFG=y MTPPP=y MULTISERVICE_WAN=y WPA3_ENTERPRISE=y \
		NFS=y SNMPD=y TOR=y UPNPIGD2=y NANO=y NTPD=y IPSET=y VPN_FUSION_MERLIN=y

export XT12 := $(HND-94912_BASE)
export XT12 += BUILD_NAME="XT12" EXT_PHY="BCM84880" NVSIZE="128" DHDAP=y REPEATER=y IPV6SUPP=y HTTPS=y ARM=y AUTODICT=y \
		BBEXTRAS=y USBEXTRAS=y EBTABLES=y XHCI=y PARENTAL2=y ACCEL_PPTPD=y USB="USB" APP="network" PROXYSTA=y \
		DNSMQ=y SHP=n BCMWL6=y BCMWL6A=y BTN_WIFITOG=n OPTIMIZE_XBOX=y ODMPID=y BCMSMP=y DUALWAN=y \
		NEW_USER_LOW_RSSI=y OPENVPN=y MDNS=y VPNC=y JFFS2LOG=y BWDPI=y LINUX_MTD="64" DEBUGFS=y SSH=y EMAIL=y \
		FRS_FEEDBACK=n SYSSTATE=y STAINFO=y REBOOT_SCHEDULE=y MULTICASTIPTV=y QUAGGA=y BCM_MUMIMO=y \
		LAN50="all" ATCOVER=y GETREALIP=y CFEZ=y NEW_APP_ARM=y NETOOL=y TRACEROUTE=y FORCE_AUTO_UPGRADE=n \
		ALEXA=y SW_HW_AUTH=y ASPMD=n BCM_MEVENT=n BCMEVENTD=y LETSENCRYPT=y JFFS_NVRAM=y COMFW=y MTLANCFG=y \
		NVRAM_ENCRYPT=y BCM_CEVENTD=y UTF8_SSID=y I2CTOOLS=y AMAS=y DBLOG=y ETHOBD=y CFGSYNC_LOCSYNC=y LACP=y \
		CONNDIAG=y IPSEC=STRONGSWAN IPSEC_SRVCLI_ONLY=SRV DWB=y AMASDB=n CRASHLOG=y BW160M=y AMAS_DIS_HASH_KEY_GEN_CHECK=y \
		BRCM_HOSTAPD=y IPERF3=y INFO_EXAP=y BCN_RPT=y BTM_11V=y INTERNETCTRL=y ASD=y AHS=n FRS_LIVE_UPDATE=n \
		ASUSCTRL=y HAS_5G_2=y CAPTCHA=y AMAS_ETHDETECT=y INSTANT_GUARD=y AVBLCHAN=y FRONTHAUL_DWB=y \
		BCM_CLED=y NEWSSID_REV4=y NEWSSID_REV5=y SW_CTRL_ALLLED=y BT_CONN=y SINGLE_LED=y ACL96=y DHD_6715B0=y \
		MSSID_PRELINK=y EXTEND_LIMIT=y GOOGLE_ASST=y VPN_FUSION=y AMAS_ADTBW=y BONDING=y BONDING_WAN=y ADGUARDDNS=y \
		WIREGUARD=y CABLEDIAG=y UUPLUGIN=y DNSQUERY_INTERCEPT=n NFCM=n IG_SITE2SITE=n \
		AMAS_CENTRAL_OPTMZ=y AMAS_SS2=y MTPPP=y MULTISERVICE_WAN=y SMART_HOME_MASTER_UI=y WPA3_ENTERPRISE=y \
		NFS=n SNMPD=y TOR=y UPNPIGD2=y NANO=y NTPD=y IPSET=y VPN_FUSION_MERLIN=y
export XT12_CFG := $(XT12)

export GT-AXE16000 := $(HND-94912_BASE)
export GT-AXE16000 += BUILD_NAME="GT-AXE16000" EXT_PHY="BCM84880" NVSIZE="192" DHDAP=y DPSTA=n REPEATER=y IPV6SUPP=y HTTPS=y \
		ARM=y AUTODICT=y BBEXTRAS=y USBEXTRAS=y EBTABLES=y MEDIASRV=y MODEM=y PARENTAL2=y ACCEL_PPTPD=y PRINTER=y \
		WEBDAV=y SMARTSYNCBASE=y USB="USB" APP="network" PROXYSTA=y DNSMQ=y SHP=n BCMWL6=y BCMWL6A=y \
		DISK_MONITOR=y BTN_WIFITOG=y OPTIMIZE_XBOX=y ODMPID=y BCMSMP=y XHCI=y DUALWAN=y NEW_USER_LOW_RSSI=y \
		OPENVPN=y TIMEMACHINE=y MDNS=y VPNC=y JFFS2LOG=y BWDPI=y DUMP_OOPS_MSG=n LINUX_MTD="64" DEBUGFS=y TEMPROOTFS=n \
		SSH=y EMAIL=y FRS_FEEDBACK=n SYSSTATE=y ROG=y STAINFO=y CLOUDCHECK=n NATNL_AICLOUD=y REBOOT_SCHEDULE=y \
		MULTICASTIPTV=y QUAGGA=y WLCLMLOAD=n BCM_MUMIMO=y LAN50="all" ATCOVER=y GETREALIP=y CFEZ=y ETLAN_LED=y TFAT=y \
		NTFS="tuxera" HFS="tuxera" NEWSSID_REV4=y NEW_APP_ARM=y NETOOL=y TRACEROUTE=y FORCE_AUTO_UPGRADE=n ALEXA=y \
		SW_HW_AUTH=y HD_SPINDOWN=y ASPMD=n BCM_MEVENT=n BCMEVENTD=y BCM_APPEVENTD=n LETSENCRYPT=y VPN_FUSION=y \
		JFFS_NVRAM=y NVRAM_ENCRYPT=y NATNL_AIHOME=y BCM_CEVENTD=y UTF8_SSID=y I2CTOOLS=y AMAS=y DBLOG=y ETHOBD=y \
		CFGSYNC_LOCSYNC=y CONNDIAG=y IPSEC=STRONGSWAN IPSEC_SRVCLI_ONLY=SRV DWB=y OPEN_NAT=y ROG_UI=n AMASDB=n \
		CRASHLOG=y WATCH_REINIT=n BW160M=y BRCM_HOSTAPD=y UUPLUGIN=y IPERF3=y INFO_EXAP=y BCN_RPT=y BTM_11V=y \
		INTERNETCTRL=y ASD=y AHS=n FRS_LIVE_UPDATE=n ASUSCTRL=y HAS_5G_2=y CAPTCHA=y OUTFOX=y GAME_MODE=y \
		AMAS_ETHDETECT=y INSTANT_GUARD=y AVBLCHAN=y FRONTHAUL_DWB=y BCM_CLED=y BONDING=y BONDING_WAN=y ACL96=y \
		WIFI6E=y LACP=y OWE_TRANS=y DHD_6715B0=y EXTEND_LIMIT=y GOOGLE_ASST=y QUADBAND=y  BTN_TURBO=y \
		BCMBSD_V2=y SW_CTRL_ALLLED=y COMFW=y CABLEDIAG=y MAX_VPNF=4 WTFAST_V2=y \
		WIREGUARD=y DNSQUERY_INTERCEPT=n NFCM=n USB_WAN_BACKUP=y IG_SITE2SITE=n ADGUARDDNS=y \
		CAPTIVE_PORTAL=y CP_FREEWIFI=y MTLANCFG=y MTPPP=y MULTISERVICE_WAN=y WPA3_ENTERPRISE=y \
		NFS=y SNMPD=y TOR=y UPNPIGD2=y NANO=y NTPD=y IPSET=y VPN_FUSION_MERLIN=y

export GT-AX11000_PRO := $(HND-94912_BASE)
export GT-AX11000_PRO += BUILD_NAME="GT-AX11000_PRO" EXT_PHY="BCM84880" NVSIZE="128" DHDAP=y DPSTA=n REPEATER=y IPV6SUPP=y \
		HTTPS=y ARM=y AUTODICT=y BBEXTRAS=y USBEXTRAS=y EBTABLES=y MEDIASRV=y MODEM=y PARENTAL2=y ACCEL_PPTPD=y PRINTER=y \
		WEBDAV=y SMARTSYNCBASE=y USB="USB" APP="network" PROXYSTA=y DNSMQ=y SHP=n BCMWL6=y BCMWL6A=y DISK_MONITOR=y \
		BTN_WIFITOG=y OPTIMIZE_XBOX=y ODMPID=y BCMSMP=y XHCI=y DUALWAN=y NEW_USER_LOW_RSSI=y OPENVPN=y TIMEMACHINE=y \
		MDNS=y VPNC=y JFFS2LOG=y BWDPI=y DUMP_OOPS_MSG=n LINUX_MTD="64" DEBUGFS=y TEMPROOTFS=n SSH=y EMAIL=y \
		FRS_FEEDBACK=n SYSSTATE=y ROG=y STAINFO=y CLOUDCHECK=n NATNL_AICLOUD=y REBOOT_SCHEDULE=y MULTICASTIPTV=y \
		QUAGGA=y WLCLMLOAD=n BCM_MUMIMO=y LAN50="all" ATCOVER=y GETREALIP=y CFEZ=y ETLAN_LED=y TFAT=y NTFS="tuxera" \
		HFS="tuxera" NEWSSID_REV4=y NEW_APP_ARM=y NETOOL=y TRACEROUTE=y FORCE_AUTO_UPGRADE=n ALEXA=y SW_HW_AUTH=y \
		HD_SPINDOWN=y ASPMD=n BCM_MEVENT=n BCMEVENTD=y BCM_APPEVENTD=n LETSENCRYPT=y VPN_FUSION=y JFFS_NVRAM=y \
		NVRAM_ENCRYPT=y NATNL_AIHOME=y BCM_CEVENTD=y UTF8_SSID=y I2CTOOLS=y AMAS=y DBLOG=y ETHOBD=y CFGSYNC_LOCSYNC=y \
		CONNDIAG=y IPSEC=STRONGSWAN IPSEC_SRVCLI_ONLY=SRV DWB=y OPEN_NAT=y ROG_UI=n AMASDB=n CRASHLOG=y WATCH_REINIT=n \
		BW160M=y BRCM_HOSTAPD=y UUPLUGIN=y IPERF3=y INFO_EXAP=y BCN_RPT=y BTM_11V=y INTERNETCTRL=y ASD=y AHS=n \
		FRS_LIVE_UPDATE=n ASUSCTRL=y HAS_5G_2=y CAPTCHA=y OUTFOX=y GAME_MODE=y AMAS_ETHDETECT=y INSTANT_GUARD=y AVBLCHAN=y \
		FRONTHAUL_DWB=y BCM_CLED=y BONDING=y BONDING_WAN=y ACL96=y LACP=y DHD_6715B0=y EXTEND_LIMIT=y BTN_TURBO=y \
		SW_CTRL_ALLLED=y COMFW=y GOOGLE_ASST=y CABLEDIAG=y IG_SITE2SITE=n WIREGUARD=y USB_WAN_BACKUP=y MAX_VPNF=4 WTFAST_V2=y \
		ADGUARDDNS=y CAPTIVE_PORTAL=y CP_FREEWIFI=y MTLANCFG=y MTPPP=y MULTISERVICE_WAN=y WPA3_ENTERPRISE=y \
		NFS=y SNMPD=y TOR=y UPNPIGD2=y NANO=y NTPD=y IPSET=y VPN_FUSION_MERLIN=y

export RT-AX86U_PRO := $(HND-94912_BASE) EXT_PHY="BCM84880" LAN4WAN_LED=y LED_BTN=y
export RT-AX86U_PRO += BUILD_NAME="RT-AX86U_PRO" NVSIZE="128" DHDAP=y DPSTA=n REPEATER=y IPV6SUPP=y HTTPS=y ARM=y AUTODICT=y \
		BBEXTRAS=y USBEXTRAS=y EBTABLES=y MEDIASRV=y MODEM=y PARENTAL2=y ACCEL_PPTPD=y PRINTER=y WEBDAV=y SMARTSYNCBASE=y \
		USB="USB" APP="network" PROXYSTA=y DNSMQ=y SHP=n BCMWL6=y BCMWL6A=y DISK_MONITOR=y BTN_WIFITOG=n OPTIMIZE_XBOX=y \
		ODMPID=y BCMSMP=y XHCI=y DUALWAN=y NEW_USER_LOW_RSSI=y OPENVPN=y TIMEMACHINE=y MDNS=y VPNC=y JFFS2LOG=y BWDPI=y \
		DUMP_OOPS_MSG=n LINUX_MTD="64" DEBUGFS=y TEMPROOTFS=n SSH=y EMAIL=y FRS_FEEDBACK=n SYSSTATE=y STAINFO=y \
		CLOUDCHECK=n NATNL_AICLOUD=y REBOOT_SCHEDULE=y MULTICASTIPTV=y QUAGGA=y WLCLMLOAD=n BCM_MUMIMO=y LAN50="all" \
		ATCOVER=y GETREALIP=y CFEZ=y TFAT=y NTFS="tuxera" HFS="tuxera" NEWSSID_REV4=y NEW_APP_ARM=y NETOOL=y \
		TRACEROUTE=y FORCE_AUTO_UPGRADE=n ALEXA=y SW_HW_AUTH=y HD_SPINDOWN=y ASPMD=n BCM_MEVENT=n BCMEVENTD=y BCM_APPEVENTD=n \
		LETSENCRYPT=y VPN_FUSION=y JFFS_NVRAM=y NVRAM_ENCRYPT=y NATNL_AIHOME=y LOGO_LED=n BCM_CEVENTD=y UTF8_SSID=y RGBLED=n \
		I2CTOOLS=n AMAS=y DBLOG=y ETHOBD=y CFGSYNC_LOCSYNC=y CONNDIAG=y IPSEC=STRONGSWAN IPSEC_SRVCLI_ONLY=SRV \
		DWB=y OPEN_NAT=y AMASDB=n CRASHLOG=y WATCH_REINIT=n BW160M=y BRCM_HOSTAPD=y UUPLUGIN=y IPERF3=y INFO_EXAP=y BCN_RPT=y \
		BTM_11V=y INTERNETCTRL=y ASD=y AHS=n FRS_LIVE_UPDATE=n ASUSCTRL=y HAS_5G_2=n CAPTCHA=y GAME_MODE=y AMAS_ETHDETECT=y \
		INSTANT_GUARD=y AVBLCHAN=y FRONTHAUL_DWB=n BCM_CLED=n SW_CTRL_ALLLED=y BONDING=y BONDING_WAN=y ACL96=y LACP=y \
		ADGUARDDNS=y CAPTIVE_PORTAL=y CP_FREEWIFI=y GEARUPPLUGIN=y MTLANCFG=y MTPPP=y MULTISERVICE_WAN=y WPA3_ENTERPRISE=y \
		DHD_6715B0=y GOOGLE_ASST=y COMFW=y WIREGUARD=y CALC_NVRAM=y INTEL_EVO=y USB_WAN_BACKUP=y IG_SITE2SITE=n \
		NFS=y SNMPD=y TOR=y UPNPIGD2=y NANO=y NTPD=y IPSET=y VPN_FUSION_MERLIN=y

export RT-AX88U_PRO := $(HND-94912_BASE)
export RT-AX88U_PRO += BUILD_NAME="RT-AX88U_PRO" EXT_PHY="BCM84880" NVSIZE="128" DHDAP=y REPEATER=y IPV6SUPP=y HTTPS=y ARM=y \
		AUTODICT=y BBEXTRAS=y USBEXTRAS=y EBTABLES=y MEDIASRV=y MODEM=y USB_WAN_BACKUP=y PARENTAL2=y ACCEL_PPTPD=y PRINTER=y \
		WEBDAV=y SMARTSYNCBASE=y USB="USB" APP="installed" PROXYSTA=y DNSMQ=y BCMWL6=y BCMWL6A=y DISK_MONITOR=y OPTIMIZE_XBOX=y \
		ODMPID=y BCMSMP=y XHCI=y DUALWAN=y NEW_USER_LOW_RSSI=y OPENVPN=y TIMEMACHINE=y MDNS=y VPNC=y JFFS2LOG=y BWDPI=y \
		LINUX_MTD="64" DEBUGFS=y SSH=y EMAIL=y FRS_FEEDBACK=n SYSSTATE=y ROG=y STAINFO=y CLOUDCHECK=n NATNL_AICLOUD=y \
		REBOOT_SCHEDULE=y MULTICASTIPTV=y QUAGGA=y BCM_MUMIMO=y LAN50="all" ATCOVER=y GETREALIP=y CFEZ=y ETLAN_LED=y TFAT=y \
		NTFS="tuxera" HFS="tuxera" NEWSSID_REV4=y NEW_APP_ARM=y NETOOL=y TRACEROUTE=y FORCE_AUTO_UPGRADE=n ALEXA=y SW_HW_AUTH=y \
		HD_SPINDOWN=y BCMEVENTD=y LETSENCRYPT=y VPN_FUSION=y JFFS_NVRAM=y NVRAM_ENCRYPT=y NATNL_AIHOME=y BCM_CEVENTD=y \
		UTF8_SSID=y AMAS=y DBLOG=y ETHOBD=y CFGSYNC_LOCSYNC=y CONNDIAG=y IPSEC=STRONGSWAN IPSEC_SRVCLI_ONLY=SRV DWB=y \
		OPEN_NAT=y AMASDB=n CRASHLOG=y BW160M=y BRCM_HOSTAPD=y UUPLUGIN=y IPERF3=y INFO_EXAP=y BCN_RPT=y BTM_11V=y \
		INTERNETCTRL=y ASD=y AHS=n FRS_LIVE_UPDATE=n ASUSCTRL=y CAPTCHA=y GAME_MODE=y AMAS_ETHDETECT=y INSTANT_GUARD=y \
		AVBLCHAN=y BCM_CLED=n SW_CTRL_ALLLED=y BONDING=y BONDING_WAN=y ACL96=y LACP=y DHD_6715B0=y GOOGLE_ASST=y \
		COMFW=y BTN_WPS_ALLLED=y WIREGUARD=y REVERTFW=y INTEL_EVO=y IG_SITE2SITE=n WTFAST_V2=y \
		CAPTIVE_PORTAL=y CP_FREEWIFI=y MTLANCFG=y MTPPP=y MULTISERVICE_WAN=y WPA3_ENTERPRISE=y ADGUARDDNS=y \
		NFS=y SNMPD=y TOR=y UPNPIGD2=y NANO=y NTPD=y IPSET=y VPN_FUSION_MERLIN=y


#
# Broadcom Wifi 7
#

export HND-96813_BASE := HND_ROUTER=y HND_ROUTER_BE_4916=y PROFILE="96813GW" SW_HW_AUTH=y AMAS=y UTF8_SSID=y ETHOBD=y DWB=y \
		CONNDIAG=y SAMBA3="3.6.x" OOKLA=y WL_SCHED_V3=y UBI=y UBIFS=y PSISTLOG=y

export RT-BE96U := $(HND-96813_BASE)
export RT-BE96U += BUILD_NAME="RT-BE96U" EXT_PHY="BCM84880" NVSIZE="192" DHDAP=y DPSTA=n REPEATER=y IPV6SUPP=y HTTPS=y ARM=y \
		AUTODICT=y BBEXTRAS=y USBEXTRAS=y EBTABLES=y MEDIASRV=y MODEM=y PARENTAL2=y ACCEL_PPTPD=y PRINTER=y WEBDAV=y \
		SMARTSYNCBASE=y USB="USB" APP="network" PROXYSTA=y DNSMQ=y SHP=n BCMWL6=y BCMWL6A=y DISK_MONITOR=y \
		OPTIMIZE_XBOX=y ODMPID=y BCMSMP=y XHCI=y DUALWAN=y NEW_USER_LOW_RSSI=y OPENVPN=y TIMEMACHINE=y MDNS=y VPNC=y \
		JFFS2LOG=y BWDPI=y DUMP_OOPS_MSG=n LINUX_MTD="64" DEBUGFS=y TEMPROOTFS=n SSH=y EMAIL=y FRS_FEEDBACK=n SYSSTATE=y \
		STAINFO=y CLOUDCHECK=n NATNL_AICLOUD=y REBOOT_SCHEDULE=y MULTICASTIPTV=y QUAGGA=y WLCLMLOAD=n BCM_MUMIMO=y LAN50="all" \
		ATCOVER=y GETREALIP=y CFEZ=y ETLAN_LED=y TFAT=y NTFS="tuxera" HFS="tuxera" NEWSSID_REV4=y NEW_APP_ARM=y NETOOL=y \
		TRACEROUTE=y FORCE_AUTO_UPGRADE=n ALEXA=y SW_HW_AUTH=y HD_SPINDOWN=y ASPMD=n BCM_MEVENT=n BCMEVENTD=y BCM_APPEVENTD=n \
		LETSENCRYPT=y VPN_FUSION=y JFFS_NVRAM=y NVRAM_ENCRYPT=y NATNL_AIHOME=y BCM_CEVENTD=y UTF8_SSID=y I2CTOOLS=y AMAS=y \
		DBLOG=y ETHOBD=y CFGSYNC_LOCSYNC=y CONNDIAG=y NFCM=n DNSQUERY_INTERCEPT=y IPSEC=STRONGSWAN IPSEC_SRVCLI_ONLY=SRV \
		DWB=y OPEN_NAT=y AMASDB=n CRASHLOG=y WATCH_REINIT=n BW160M=y BRCM_HOSTAPD=y UUPLUGIN=y IPERF3=y INFO_EXAP=y BCN_RPT=y \
		BTM_11V=y INTERNETCTRL=y ASD=y AHS=n FRS_LIVE_UPDATE=n ASUSCTRL=y HAS_6G=y CAPTCHA=y OUTFOX=y GAME_MODE=y \
		AMAS_ETHDETECT=y INSTANT_GUARD=y AVBLCHAN=y FRONTHAUL_DWB=y BCM_CLED=y BONDING=y BONDING_WAN=y ACL96=y LACP=y \
		OWE_TRANS=y EXTEND_LIMIT=y GOOGLE_ASST=y LED_BTN=y WIREGUARD=y BCMBSD_V2=y \
		SW_CTRL_ALLLED=y COMFW=y WIFI7=y BW320M=y PC_SCHED_V3=y MULTISERVICE_WAN=y MTLANCFG=y CAPTIVE_PORTAL=y CP_FREEWIFI=y \
		WPA3_ENTERPRISE=y USB_WAN_BACKUP=y DOWNGRADE_CHECK=y \
		EXTRA_KERNEL_CONFIGS="BCM_JUMBO_FRAME=y BCM_MAX_MTU_SIZE=10240 CONFIG_BCM_IGNORE_BRIDGE_MTU=y" \
		AMAS_OB_DISABLE_DSAP=y AUTO_WANPORT=y \
		NFS=n SNMPD=y TOR=y UPNPIGD2=y NANO=y NTPD=y IPSET=y VPN_FUSION_MERLIN=y

export GT-BE98_PRO := $(HND-96813_BASE)
export GT-BE98_PRO += BUILD_NAME="GT-BE98_PRO" EXT_PHY="BCM84880" NVSIZE="192" DHDAP=y DPSTA=n REPEATER=y IPV6SUPP=y HTTPS=y ARM=y \
		AUTODICT=y BBEXTRAS=y USBEXTRAS=y EBTABLES=y MEDIASRV=y MODEM=y PARENTAL2=y ACCEL_PPTPD=y PRINTER=y WEBDAV=y \
		SMARTSYNCBASE=y USB="USB" APP="network" PROXYSTA=y DNSMQ=y SHP=n BCMWL6=y BCMWL6A=y DISK_MONITOR=y BTN_WIFITOG=y \
		OPTIMIZE_XBOX=y ODMPID=y BCMSMP=y XHCI=y DUALWAN=y NEW_USER_LOW_RSSI=y OPENVPN=y TIMEMACHINE=y MDNS=y VPNC=y \
		JFFS2LOG=y BWDPI=y DUMP_OOPS_MSG=n LINUX_MTD="64" DEBUGFS=y TEMPROOTFS=n SSH=y EMAIL=y FRS_FEEDBACK=n SYSSTATE=y \
		ROG=y STAINFO=y CLOUDCHECK=n NATNL_AICLOUD=y REBOOT_SCHEDULE=y MULTICASTIPTV=y QUAGGA=y WLCLMLOAD=n BCM_MUMIMO=y \
		LAN50="all" ATCOVER=y GETREALIP=y CFEZ=y ETLAN_LED=y TFAT=y NTFS="tuxera" HFS="tuxera" NEWSSID_REV4=y NEW_APP_ARM=y \
		NETOOL=y TRACEROUTE=y FORCE_AUTO_UPGRADE=n ALEXA=y SW_HW_AUTH=y HD_SPINDOWN=y ASPMD=n BCM_MEVENT=n BCMEVENTD=y \
		BCM_APPEVENTD=n LETSENCRYPT=y VPN_FUSION=y JFFS_NVRAM=y NVRAM_ENCRYPT=y NATNL_AIHOME=y BCM_CEVENTD=y UTF8_SSID=y \
		I2CTOOLS=y AMAS=y DBLOG=y ETHOBD=y CFGSYNC_LOCSYNC=y CONNDIAG=y IPSEC=STRONGSWAN IPSEC_SRVCLI_ONLY=SRV DWB=y OPEN_NAT=y \
		ROG_UI=n AMASDB=n CRASHLOG=y WATCH_REINIT=n BW160M=y BRCM_HOSTAPD=y UUPLUGIN=y IPERF3=y INFO_EXAP=y BCN_RPT=y \
		BTM_11V=y INTERNETCTRL=y ASD=y AHS=n FRS_LIVE_UPDATE=n ASUSCTRL=y HAS_6G=y HAS_6G_2=y CAPTCHA=y OUTFOX=y \
		GAME_MODE=y AMAS_ETHDETECT=y INSTANT_GUARD=y AVBLCHAN=y FRONTHAUL_DWB=y BCM_CLED=y BONDING=y BONDING_WAN=n \
		ACL96=y LACP=y OWE_TRANS=y DHD_6715B0=y EXTEND_LIMIT=y GOOGLE_ASST=y QUADBAND=y FW_CHK_BOARDID=y \
		BTN_TURBO=y WIREGUARD=y BCMBSD_V2=y SW_CTRL_ALLLED=y COMFW=y NFCM=n DNSQUERY_INTERCEPT=y WIFI7=y BW320M=y \
		MTLANCFG=y CAPTIVE_PORTAL=y CP_FREEWIFI=y WPA3_ENTERPRISE=y DOWNGRADE_CHECK=y \
		EXTRA_KERNEL_CONFIGS="BCM_JUMBO_FRAME=y BCM_MAX_MTU_SIZE=10240 CONFIG_BCM_IGNORE_BRIDGE_MTU=y" MAX_VPNF=4 \
		WTFAST_V2=y AMAS_OB_DISABLE_DSAP=y USB_WAN_BACKUP=y AUTO_WANPORT=y \
		NFS=n SNMPD=y TOR=y UPNPIGD2=y NANO=y NTPD=y IPSET=y VPN_FUSION_MERLIN=y

export RT-BE88U := $(HND-96813_BASE)
export RT-BE88U += BUILD_NAME="RT-BE88U" EXT_PHY="BCM84880" NVSIZE="192" DHDAP=y REPEATER=y IPV6SUPP=y HTTPS=y ARM=y AUTODICT=y \
		BBEXTRAS=y USBEXTRAS=y EBTABLES=y MEDIASRV=y MODEM=y PARENTAL2=y ACCEL_PPTPD=y PRINTER=y WEBDAV=y SMARTSYNCBASE=y \
		USB="USB" APP="network" PROXYSTA=y DNSMQ=y BCMWL6=y BCMWL6A=y DISK_MONITOR=y OPTIMIZE_XBOX=y ODMPID=y BCMSMP=y \
		XHCI=y DUALWAN=y NEW_USER_LOW_RSSI=y OPENVPN=y TIMEMACHINE=y MDNS=y VPNC=y JFFS2LOG=y BWDPI=y LINUX_MTD="64" \
		DEBUGFS=y SSH=y EMAIL=y FRS_FEEDBACK=n SYSSTATE=y STAINFO=y CLOUDCHECK=n NATNL_AICLOUD=y REBOOT_SCHEDULE=y \
		MULTICASTIPTV=y QUAGGA=y BCM_MUMIMO=y LAN50="all" ATCOVER=y GETREALIP=y CFEZ=y ETLAN_LED=y TFAT=y NTFS="tuxera" \
		HFS="tuxera" NEWSSID_REV4=y NEW_APP_ARM=y NETOOL=y TRACEROUTE=y FORCE_AUTO_UPGRADE=n ALEXA=y SW_HW_AUTH=y \
		HD_SPINDOWN=y BCMEVENTD=y LETSENCRYPT=y VPN_FUSION=y JFFS_NVRAM=y NVRAM_ENCRYPT=y NATNL_AIHOME=y BCM_CEVENTD=y \
		UTF8_SSID=y AMAS=y DBLOG=y ETHOBD=y CFGSYNC_LOCSYNC=y CONNDIAG=y NFCM=n DNSQUERY_INTERCEPT=y IPSEC=STRONGSWAN \
		IPSEC_SRVCLI_ONLY=SRV DWB=y OPEN_NAT=y AMASDB=n CRASHLOG=y BW160M=y BRCM_HOSTAPD=y UUPLUGIN=y IPERF3=y INFO_EXAP=y \
		BCN_RPT=y BTM_11V=y INTERNETCTRL=y ASD=y AHS=n FRS_LIVE_UPDATE=n ASUSCTRL=y CAPTCHA=y OUTFOX=y GAME_MODE=y \
		AMAS_ETHDETECT=y INSTANT_GUARD=y AVBLCHAN=y FRONTHAUL_DWB=y BCM_CLED=y BONDING=y BONDING_WAN=y ACL96=y LACP=y \
		OWE_TRANS=y EXTEND_LIMIT=y GOOGLE_ASST=y BTN_WPS_ALLLED=y LED_BTN=y WIREGUARD=y BCMBSD_V2=y \
		SW_CTRL_ALLLED=y COMFW=y WIFI7=y PC_SCHED_V3=y MULTISERVICE_WAN=y MTLANCFG=y CAPTIVE_PORTAL=y CP_FREEWIFI=y \
		WPA3_ENTERPRISE=y USB_WAN_BACKUP=y SFPP=y SFPP_LED=y AMAS_OB_DISABLE_DSAP=y WTFAST_V2=n REVERTFW=y MLO=y \
		DOWNGRADE_CHECK=y AUTO_WANPORT=y BCM_MAX_MTU_SIZE=10240 CONFIG_BCM_IGNORE_BRIDGE_MTU=y \
		EXTRA_KERNEL_CONFIGS=BCM_JUMBO_FRAME=y GEARUPPLUGIN=y SECURE_BY_DEFAULT=y \
		NFS=n SNMPD=y TOR=y UPNPIGD2=y NANO=y NTPD=y IPSET=y VPN_FUSION_MERLIN=y

export RT-BE86U := $(HND-96813_BASE)
export RT-BE86U += BUILD_NAME="RT-BE86U" EXT_PHY="BCM84880" NVSIZE="192" DHDAP=y HND_WL=y REPEATER=y IPV6SUPP=y HTTPS=y ARM=y \
		AUTODICT=y BBEXTRAS=y USBEXTRAS=y EBTABLES=y MEDIASRV=y MODEM=y PARENTAL2=y ACCEL_PPTPD=y PRINTER=y WEBDAV=y \
		SMARTSYNCBASE=y USB="USB" APP="network" PROXYSTA=y DNSMQ=y BCMWL6=y BCMWL6A=y DISK_MONITOR=y OPTIMIZE_XBOX=y ODMPID=y \
		BCMSMP=y XHCI=y DUALWAN=y NEW_USER_LOW_RSSI=y OPENVPN=y TIMEMACHINE=y MDNS=y VPNC=y JFFS2LOG=y BWDPI=y LINUX_MTD="64" \
		DEBUGFS=y SSH=y EMAIL=y FRS_FEEDBACK=n SYSSTATE=y STAINFO=y CLOUDCHECK=n NATNL_AICLOUD=y REBOOT_SCHEDULE=y \
		MULTICASTIPTV=y QUAGGA=y BCM_MUMIMO=y LAN50="all" ATCOVER=y GETREALIP=y CFEZ=y TFAT=y NTFS="tuxera" HFS="tuxera" \
		NEWSSID_REV4=y LAN4WAN_LED=y NEW_APP_ARM=y NETOOL=y TRACEROUTE=y FORCE_AUTO_UPGRADE=n ALEXA=y SW_HW_AUTH=y HD_SPINDOWN=y \
		BCMEVENTD=y LETSENCRYPT=y VPN_FUSION=y JFFS_NVRAM=y NVRAM_ENCRYPT=y NATNL_AIHOME=y BCM_CEVENTD=y UTF8_SSID=y AMAS=y \
		DBLOG=y ETHOBD=y CFGSYNC_LOCSYNC=y CONNDIAG=y NFCM=n DNSQUERY_INTERCEPT=y IPSEC=STRONGSWAN IPSEC_SRVCLI_ONLY=SRV DWB=y \
		FRONTHAUL_DWB=y OPEN_NAT=y AMASDB=n CRASHLOG=y BW160M=y BRCM_HOSTAPD=y UUPLUGIN=y IPERF3=y INFO_EXAP=y BCN_RPT=y \
		BTM_11V=y INTERNETCTRL=y ASD=y AHS=n FRS_LIVE_UPDATE=n ASUSCTRL=y CAPTCHA=y OUTFOX=y GAME_MODE=y AMAS_ETHDETECT=y \
		INSTANT_GUARD=y AVBLCHAN=y BONDING=y BONDING_WAN=y ACL96=y LACP=y OWE_TRANS=y EXTEND_LIMIT=y GOOGLE_ASST=y \
		LED_BTN=y WIREGUARD=y BCMBSD_V2=y SW_CTRL_ALLLED=y COMFW=y WIFI7=y PC_SCHED_V3=y \
		MULTISERVICE_WAN=y MTLANCFG=y CAPTIVE_PORTAL=y CP_FREEWIFI=y WPA3_ENTERPRISE=y USB_WAN_BACKUP=y GEARUPPLUGIN=y \
		AMAS_OB_DISABLE_DSAP=y AUTO_WANPORT=y \
		NFS=n SNMPD=y TOR=y UPNPIGD2=y NANO=y NTPD=y IPSET=y VPN_FUSION_MERLIN=y

export HND-96765_BASE := HND_ROUTER=y HND_ROUTER_BE_4916=y PROFILE="96765GW" SW_HW_AUTH=y AMAS=y UTF8_SSID=y ETHOBD=y DWB=y \
		CONNDIAG=y SAMBA3="3.6.x" OOKLA=y WL_SCHED_V3=y UBI=y UBIFS=y PSISTLOG=y CABLEDIAG=y

export RT-BE92U := $(HND-96765_BASE)
export RT-BE92U += BUILD_NAME="RT-BE92U" EXT_PHY="BCM84880" NVSIZE="192" DHDAP=n HND_WL=y REPEATER=y IPV6SUPP=y HTTPS=y ARM=y AUTODICT=y \
		BBEXTRAS=y USBEXTRAS=y EBTABLES=y MEDIASRV=y MODEM=y PARENTAL2=y ACCEL_PPTPD=y PRINTER=y WEBDAV=y SMARTSYNCBASE=y \
		USB="USB" APP="network" PROXYSTA=y DNSMQ=y BCMWL6=y BCMWL6A=y DISK_MONITOR=y OPTIMIZE_XBOX=y ODMPID=y BCMSMP=y XHCI=y \
		DUALWAN=y NEW_USER_LOW_RSSI=y OPENVPN=y TIMEMACHINE=y MDNS=y VPNC=y JFFS2LOG=y BWDPI=y LINUX_MTD="64" DEBUGFS=y SSH=y \
		EMAIL=y FRS_FEEDBACK=n SYSSTATE=y STAINFO=y CLOUDCHECK=n NATNL_AICLOUD=y REBOOT_SCHEDULE=y MULTICASTIPTV=y QUAGGA=y \
		BCM_MUMIMO=y LAN50="all" ATCOVER=y GETREALIP=y CFEZ=y ETLAN_LED=y TFAT=y NTFS="tuxera" HFS="tuxera" NEWSSID_REV4=y \
		NEW_APP_ARM=y NETOOL=y TRACEROUTE=y FORCE_AUTO_UPGRADE=n ALEXA=y SW_HW_AUTH=y HD_SPINDOWN=y BCMEVENTD=y LETSENCRYPT=y \
		VPN_FUSION=y JFFS_NVRAM=y NVRAM_ENCRYPT=y NATNL_AIHOME=y BCM_CEVENTD=y UTF8_SSID=y AMAS=y DBLOG=y ETHOBD=y \
		CFGSYNC_LOCSYNC=y CONNDIAG=y NFCM=n DNSQUERY_INTERCEPT=y IPSEC=STRONGSWAN IPSEC_SRVCLI_ONLY=SRV DWB=y FRONTHAUL_DWB=y \
		AMASDB=n CRASHLOG=y BW160M=y BRCM_HOSTAPD=y UUPLUGIN=y IPERF3=y INFO_EXAP=y BCN_RPT=y BTM_11V=y INTERNETCTRL=y ASD=y AHS=n \
		FRS_LIVE_UPDATE=n ASUSCTRL=y HAS_6G=y CAPTCHA=y OUTFOX=y AMAS_ETHDETECT=y INSTANT_GUARD=y AVBLCHAN=y ACL96=y OWE_TRANS=y \
		EXTEND_LIMIT=y GOOGLE_ASST=y WIREGUARD=y BCMBSD_V2=y SW_CTRL_ALLLED=y COMFW=y WIFI7=y BW320M=y PC_SCHED_V3=y GEARUPPLUGIN=y \
		MTLANCFG=y WPA3_ENTERPRISE=y SMART_HOME_MASTER_UI=y USB_WAN_BACKUP=y AMAS_OB_DISABLE_DSAP=y HND_GPIO=y AUTO_WANPORT=y \
		NFS=n SNMPD=y TOR=y UPNPIGD2=y NANO=y NTPD=y IPSET=y VPN_FUSION_MERLIN=y
