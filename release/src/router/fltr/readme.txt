
the fltr directory is soft-link in $(SRCBASE)/net/netfilter/fltr
net/netfilter/fltr -> ../../../../router/fltr/

=============================================
don't want to implement NOW
=============================================
[src-rt-6.x]
rt-n66u ;{"fwsize":"33161216"}
rt-ac66u ;{"fwsize":"33554432"}

[src-rt-7.x.main/src]
rt-ac3200
=============================================

fltr.ko porting done
=============================================
have no broadcom fc
---------------------------------------------
[src-rt-9.x/src]
rt-ac1200g
rt-ac1200g+

[src-rt-6.x.4708]
rt-ac68u rt-ac56s rt-ac56u rt-ac68a 4g-ac68u

[src-rt-7.14.114.x/src]
rt-ac88u rt-ac5300 rt-ac3100

---------------------------------------------
have broadcom fc
---------------------------------------------
[src-rt-5.02axhnd]
rt-ax88u rt-ax92u gt-ax11000

[src-rt-5.02axhnd.675x]
rt-ax95q
rt-ax82u
rt-ax58u
rt-ax56u
tuf-ax3000

[src-rt-5.02hnd]
gt-ac5300
gt-ac2900
rt-ac86u

=============================================
need other mechanism to support
=============================================
[src-qca-ipq806x]
brt-ac828 ;{"fwsize":"67170304"}
rt-ad7200 ;{"fwsize":"67170304"}
gt-axy16000 ;{"fwsize":"104882176"}
gt-ax6000n ;{"fwsize":"104882176"}
rt-ax89u ;{"fwsize":"104882176"}

=============================================
in file $(KERNEL_SRC_DIR)/config_base.6a

# support ASUS filter
CONFIG_NETFILTER_ASUS_FILTER=m

#
# Xtables combined modules
#
CONFIG_NETFILTER_XT_MARK=y

----------------------------------------------
in folder net/netfilter/
ln -s ../../../../router/fltr fltr

----------------------------------------------
in file net/netfilter/Makefile
# ASUS network filter -- Andrew
obj-$(CONFIG_NETFILTER_ASUS_FILTER) += fltr/

----------------------------------------------
in file net/netfilter/Kconfig
source "net/netfilter/fltr/Kconfig"

=============================================
targets/fs.src/rom/etc/init.d/bcm-base-drivers.list

bwdpi wrs_url

//start TM tdts
rc rc_service start_wrs

//stop TM tdts
rc rc_service stop_wrs_force

//get the shn information
shn_ctrl -a get_user_detail

