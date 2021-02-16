#!/bin/sh

# detect basic_center and remove it
if [ -d "/jffs/.basic_center" ];then
	rm -rf /jffs/.basic_center
	[ -d "/jffs/db" ] && rm -rf /jffs/db
	[ -d "/jffs/asdb" ] && rm -rf /jffs/asdb
	[ -L "/jffs/etc/profile" ] && rm -rf /jffs/etc
	[ -L "/jffs/configs/profile.add" ] && rm -rf /jffs/configs
	sync
fi

# http web should work under port 80
if [ "$(nvram get http_lanport)" != "80" -o -n "$(ps|grep -w httpd|grep -v grep|grep 81)" ];then
	nvram set http_lanport=80
	nvram commit
	service restart_httpd >/dev/null 2>&1
fi

# remove files after router started, incase dnsmasq won't start
[ -d "/jffs/configs/dnsmasq.d" ] && rm -rf /jffs/configs/dnsmasq.d/*

# make some folders
mkdir -p /jffs/scripts
mkdir -p /jffs/configs/dnsmasq.d
mkdir -p /jffs/etc
mkdir -p /tmp/upload

# install all
if [ -f "/koolshare/.soft_ver" ];then
	CUR_VERSION=$(cat /koolshare/.soft_ver)
else
	CUR_VERSION="0"
fi
ROM_VERSION=$(cat /rom/etc/koolshare/.soft_ver)
COMP=$(/rom/etc/koolshare/bin/versioncmp $CUR_VERSION $ROM_VERSION)

if [ ! -d "/jffs/.koolshare" -o "$COMP" == "1" ]; then
	#cp -a /rom/etc/koolshare/ /jffs/.koolshare
	mkdir -p /jffs/.koolshare
	cp -rf /rom/etc/koolshare/* /jffs/.koolshare/
	cp -rf /rom/etc/koolshare/.soft_ver /jffs/.koolshare/
	mkdir -p /jffs/.koolshare/configs/
	chmod 755 /koolshare/bin/*
	chmod 755 /koolshare/init.d/*
	chmod 755 /koolshare/perp/*
	chmod 755 /koolshare/perp/.boot/*
	chmod 755 /koolshare/perp/.control/*
	chmod 755 /koolshare/perp/httpdb/*
	chmod 755 /koolshare/scripts/*
	# make some link
	[ ! -L "/koolshare/bin/base64_decode" ] && ln -sf /koolshare/bin/base64_encode /koolshare/bin/base64_decode
	[ ! -L "/koolshare/scripts/ks_app_remove.sh" ] && ln -sf /koolshare/scripts/ks_app_install.sh /koolshare/scripts/ks_app_remove.sh
	[ ! -L "/jffs/.asusrouter" ] && ln -sf /koolshare/bin/kscore.sh /jffs/.asusrouter
	if [ -n "$(nvram get extendno | grep koolshare)" ];then
		# for offcial mod, RT-AC86U, GT-AC5300, TUF-AX3000, RT-AX86U, etc
		[ ! -L "/jffs/etc/profile" ] && ln -sf /koolshare/scripts/base.sh /jffs/etc/profile
	else
		# for Merlin mod, RT-AX88U, RT-AC86U, etc
		[ ! -L "/jffs/configs/profile.add" ] && ln -sf /koolshare/scripts/base.sh /jffs/configs/profile.add
	fi
	#service restart_skipd
fi

# check start up scripts
if [ ! -f "/jffs/scripts/wan-start" ];then
	cat > /jffs/scripts/wan-start <<-EOF
	#!/bin/sh
	/koolshare/bin/ks-wan-start.sh start
	EOF
	chmod +x /jffs/scripts/wan-start
else
	STARTCOMAND1=$(cat /jffs/scripts/wan-start | grep -c "/koolshare/bin/ks-wan-start.sh start")
	[ "$STARTCOMAND1" -gt "1" ] && sed -i '/ks-wan-start.sh/d' /jffs/scripts/wan-start && sed -i '1a /koolshare/bin/ks-wan-start.sh start' /jffs/scripts/wan-start
	[ "$STARTCOMAND1" == "0" ] && sed -i '1a /koolshare/bin/ks-wan-start.sh start' /jffs/scripts/wan-start
fi

if [ ! -f "/jffs/scripts/nat-start" ];then
	cat > /jffs/scripts/nat-start <<-EOF
	#!/bin/sh
	/koolshare/bin/ks-nat-start.sh start_nat
	EOF
	chmod +x /jffs/scripts/nat-start
else
	STARTCOMAND2=$(cat /jffs/scripts/nat-start | grep -c "/koolshare/bin/ks-nat-start.sh start_nat")
	[ "$STARTCOMAND2" -gt "1" ] && sed -i '/ks-nat-start.sh/d' /jffs/scripts/nat-start && sed -i '1a /koolshare/bin/ks-nat-start.sh start_nat' /jffs/scripts/nat-start
	[ "$STARTCOMAND2" == "0" ] && sed -i '1a /koolshare/bin/ks-nat-start.sh start_nat' /jffs/scripts/nat-start
fi

if [ ! -f "/jffs/scripts/post-mount" ];then
	cat > /jffs/scripts/post-mount <<-EOF
	#!/bin/sh
	/koolshare/bin/ks-mount-start.sh start
	EOF
	chmod +x /jffs/scripts/post-mount
else
	STARTCOMAND3=$(cat /jffs/scripts/post-mount | grep -c "/koolshare/bin/ks-mount-start.sh start")
	[ "$STARTCOMAND3" -gt "1" ] && sed -i '/ks-mount-start.sh/d' /jffs/scripts/post-mount && sed -i '1a /koolshare/bin/ks-mount-start.sh start' /jffs/scripts/post-mount
	[ "$STARTCOMAND3" == "0" ] && sed -i '1a /koolshare/bin/ks-mount-start.sh start' /jffs/scripts/post-mount
fi

if [ ! -f "/jffs/scripts/services-start" ];then
	cat > /jffs/scripts/services-start <<-EOF
	#!/bin/sh
	/koolshare/bin/ks-services-start.sh start
	EOF
	chmod +x /jffs/scripts/services-start
else
	STARTCOMAND4=$(cat /jffs/scripts/services-start | grep -c "/koolshare/bin/ks-services-start.sh start")
	[ "$STARTCOMAND4" -gt "1" ] && sed -i '/ks-services-start.sh/d' /jffs/scripts/services-start && sed -i '1a /koolshare/bin/ks-services-start.sh start' /jffs/scripts/services-start
	[ "$STARTCOMAND4" == "0" ] && sed -i '1a /koolshare/bin/ks-services-start.sh start' /jffs/scripts/services-start
fi

sync
