#!/bin/bash
#
# Arg: 
#      $1 number of impl e.g 12
#	   $2 broad profile	 e.g 96362GW
#      $3 brand e.g media 
#
#############################################################################
# Find the impl from user input.

if [ -z $1 ]; then
  echo "Which impl to build?  Example:" $0 "7"
  exit
else
  imp="impl"$1
fi

if ! [ -d $impl ]; then
  exit
fi

echo Tar release on $imp

imp_org=$imp

if [ ${imp:4} -gt 20 ]
then 
imp=$imp/main/src
fi
#############################################################################
# Get epi version and router version

./make_version.sh $imp ""
. $imp/epivers
ver=$EPI_VERSION_STR

. $imp/rver
rver=$ROUTER_VERSION_STR

ver_orig=$ver
rver_orig=$rver

prefix_config_brand=wlconfig_lx_router
default_config_brand=${prefix_config_brand}_ap

if [ "$3" != "" ]; then
  dr_wl_brands=($3)
else
  . wlbrands
fi

if [ "$dr_wl_brands" != "" ]; then
   mv $imp_org ${imp}-bak

   for i in $(seq 0 $((${#dr_wl_brands[*]}-1))) ; do
       brand=${dr_wl_brands[${i}]}	

       ver=$ver_orig-$brand
       rver=$rver_orig-$brand

       # Current tree is bcmdrivers/broadcom/net/wl
       worktop=../../../../../

       cd ${worktop}linux
       make PROFILE=$2 WL=IMPL$1 wlan_clean

       cd bcmdrivers/broadcom/net/wl 
       rm -rf $imp_org
       tar zxvf ${worktop}linuxout/wlan-router.$rver/$ver/bin/wlan-bin-all-$ver.tgz
	   rm -rf wlan
       cd $imp/wl/config
       cp -f ${prefix_config_brand}_${brand} $default_config_brand
       # Come back net/wl 
       cd -
       cd ${worktop}linux

       make PROFILE=$2 WL=IMPL$1

       mkdir -p ../linuxout/wlan-router.$rver/$ver/bin/image
       cp -f targets/$2/bcm* ../linuxout/wlan-router.$rver/$ver/bin/image/. 
       rm -f ../linuxout/wlan-router.$rver/$ver/bin/image/*.w
       cd bcmdrivers/broadcom/net/wl 
   done

   cd ${worktop}linux
   make clean
   cd bcmdrivers/broadcom/net/wl
   rm -rf  $imp_org
   mv ${imp_org}-bak $imp_org
fi

# End of file
