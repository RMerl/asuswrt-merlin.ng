#!/bin/sh

MODEL=XT12
SDK=arm_4912hnd

# Model Dirs
mkdir -p bwdpi_source/asus/prebuild/$MODEL
mkdir -p bwdpi_source/asus_sql/prebuild/$MODEL
mkdir -p bwdpi_source/prebuild/$MODEL
mkdir -p cfg_mnt/prebuild/$MODEL
mkdir -p httpd/prebuild/$MODEL
mkdir -p rc/prebuild/$MODEL
mkdir -p shared/prebuild/$MODEL
mkdir -p sw-hw-auth/prebuild/$MODEL
mkdir -p www/sysdep/$MODEL
mkdir -p ahs/prebuild/$MODEL
mkdir -p amas-utils/prebuild/$MODEL
mkdir -p asd/prebuild/$MODEL
mkdir -p libasc/prebuild/$MODEL

# SDK dirs
mkdir -p aaews/prebuild/$SDK
mkdir -p asusnatnl/natnl/prebuild/$SDK
mkdir -p asuswebstorage/prebuild/$SDK
mkdir -p bsd/prebuilt/$SDK
mkdir -p dhd/prebuilt/$SDK
mkdir -p dropbox_client/prebuild/$SDK
mkdir -p eventd/prebuilt/$SDK
mkdir -p ftpclient/prebuild/$SDK
mkdir -p infosvr/prebuild/$SDK
mkdir -p inotify/prebuild/$SDK
mkdir -p libbcm/prebuilt/$SDK
mkdir -p libletsencrypt/prebuild/$SDK
mkdir -p lighttpd-1.4.39/prebuild/$SDK
mkdir -p networkmap/prebuild/$SDK
mkdir -p nt_center/actMail/prebuild/$SDK
mkdir -p nt_center/lib/prebuild/$SDK
mkdir -p nt_center/prebuild/$SDK
mkdir -p protect_srv/lib/prebuild/$SDK
mkdir -p protect_srv/prebuild/$SDK
mkdir -p sambaclient/prebuild/$SDK
mkdir -p sysstate/commands/prebuild/$SDK
mkdir -p sysstate/log_daemon/prebuild/$SDK
mkdir -p u2ec/prebuild/$SDK
mkdir -p usbclient/prebuild/$SDK
mkdir -p wb/prebuild/$SDK
mkdir -p webdav_client/prebuild/$SDK
mkdir -p wlc_nt/prebuild/$SDK
mkdir -p dblog/commands/prebuild/$SDK
mkdir -p dblog/daemon/prebuild/$SDK
mkdir -p libasuslog/prebuild/$SDK

# Links to SDK dirs
ln -s $SDK aaews/prebuild/$MODEL
ln -s $SDK asusnatnl/natnl/prebuild/$MODEL
ln -s $SDK asuswebstorage/prebuild/$MODEL
ln -s $SDK bsd/prebuilt/$MODEL
ln -s $SDK dhd/prebuilt/$MODEL
ln -s $SDK dropbox_client/prebuild/$MODEL
ln -s $SDK eventd/prebuilt/$MODEL
ln -s $SDK ftpclient/prebuild/$MODEL
ln -s $SDK infosvr/prebuild/$MODEL
ln -s $SDK inotify/prebuild/$MODEL
ln -s $SDK libbcm/prebuilt/$MODEL
ln -s $SDK libletsencrypt/prebuild/$MODEL
ln -s $SDK lighttpd-1.4.39/prebuild/$MODEL
ln -s $SDK networkmap/prebuild/$MODEL
ln -s $SDK nt_center/actMail/prebuild/$MODEL
ln -s $SDK nt_center/lib/prebuild/$MODEL
ln -s $SDK nt_center/prebuild/$MODEL
ln -s $SDK protect_srv/lib/prebuild/$MODEL
ln -s $SDK protect_srv/prebuild/$MODEL
ln -s $SDK sambaclient/prebuild/$MODEL
ln -s $SDK sysstate/commands/prebuild/$MODEL
ln -s $SDK sysstate/log_daemon/prebuild/$MODEL
ln -s $SDK u2ec/prebuild/$MODEL
ln -s $SDK usbclient/prebuild/$MODEL
ln -s $SDK wb/prebuild/$MODEL
ln -s $SDK webdav_client/prebuild/$MODEL
ln -s $SDK wlc_nt/prebuild/$MODEL
ln -s $SDK dblog/commands/prebuild/$MODEL
ln -s $SDK dblog/daemon/prebuild/$MODEL
ln -s $SDK libasuslog/prebuild/$MODEL


echo "Don't forget to manually create release/SDK/hostTools/ and release/SDK/router-sysdep.MODEL"
