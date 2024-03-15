# wsdd2 (1.8.x)
WSD/LLMNR Discovery/Name Service Daemon

With Microsoft turning off SMB1 feature completely on Windows 10, any Samba shares on the
local network become invisible to Windows 10 computers. That's due to the fact that SMB1 is
required for Computer Browser service to function.

Newer Windows systems can use WSD (Web Services for Devices) to discover shares hosted on
other Windows computers while Linux systems normally utilize mNDS/DNS-SD for service discovery.
Microsoft is moving to support mDNS/DNS-SD, but not yet there.

The primary purpose of this project is to enable WSD on Samba servers so that network shares
hosted on a Unix box can appear in Windows File Explorer / Network.

NOTE: Make sure there is no firewall rule blocking WSD multicast address
239.255.255.250 and ff02::c, protocol UDP port 3702. Unicast SOAP HTTP
WS-Discovery responder listens on TCP port 3702.

LLMNR responder listens on multicast 224.0.0.252 / ff02::1:3 UDP port 5355
and unicast TCP port 5355.

The original source code was taken from Netgear ReadyNAS OS v6.9.3 published at
https://kb.netgear.com/2649/NETGEAR-Open-Source-Code-for-Programmers-GPL
https://www.downloads.netgear.com/files/GPL/ReadyNASOS_V6.9.3_WW_src.zip

Consumed by this openwrt package:
https://github.com/openwrt/packages/tree/master/net/wsdd2

Consumed by this archlinux user repository package:
https://aur.archlinux.org/packages/wsdd2/
