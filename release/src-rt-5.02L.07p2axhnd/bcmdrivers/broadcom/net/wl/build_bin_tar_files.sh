#!/bin/bash

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

use_shared="n"
if [ ${imp:4} -ge 24 ]; then 
use_shared="y"
fi

if [ ${imp:4} -gt 20 ]
then 
imp=$imp/main/src
fi

#############################################################################
# Get epi version and router version

./make_version.sh $imp $2
. $imp/epivers
ver=$EPI_VERSION_STR

. $imp/rver
rver=$ROUTER_VERSION_STR

# Get wl rel brand
brand=$3
if [ "$brand" != "" ]; then
  ver=$ver-$brand
  rver=$rver-$brand
fi

#############################################################################
# These files are to be excluded.

exclude=" \
  --wildcards \
  --wildcards-match-slash \
  --exclude=.*.db \
  --exclude=*.cmd \
  --exclude=*.contrib* \
  --exclude=*.db \
  --exclude=*.keep* \
  --exclude=*.merge \
  --exclude=*.tmp \
  --exclude=*build \
  --exclude=tags \
  --exclude=$imp/shared/cfe \
  --exclude=$imp/shared/linux \
  --exclude=$imp/shared/nvram \
  --exclude=$imp/shared/zlib \
  --exclude=$imp/router/bshared/src \
  --exclude=$imp/usbdev \
  --exclude=$imp/tools \
  --exclude=$imp/router_rules*.mk \
  --exclude=$imp/router-filelist*.txt \
  --exclude=$imp_org/main/components/apps/airiq/src/internal \
"

bin_found=no

for x in `find $imp -maxdepth 1 -name *.o_save`; do
  bin_found=yes
done

if [ $bin_found != yes ]; then
  exit
fi

echo "Tarball==========" wlan-bin-all-$ver.tgz

# When cus=1 (manually change this file), 
# there is customized release request: 
# To add directory images and userspace/private/apps/wlan and to the tarball,
# and put directory images to directory bin.
cus=1
comengine=../../../../
worktop=${comengine}../
if  [ $cus == "1" ]; then
  dver=`grep '^BRCM_VERSION=' ${comengine}version.make|cut -d'=' -f2`
  drel=`grep '^BRCM_RELEASE=' ${comengine}version.make|cut -d'=' -f2`
  dext=`grep '^BRCM_EXTRAVERSION=' ${comengine}version.make|cut -d'=' -f2`
  drver="$dver"."$drel"L."$dext"
  dimage=${worktop}linuxout/${drver}
  cp -R ${dimage}/images .
  tar -czf - ${comengine}bcmdrivers/broadcom/net/wl/$imp_org | tar zxf -
  tar -czf - ${comengine}bcmdrivers/broadcom/net/wl/makefile.* | tar zxf -
  tar -czf - ${comengine}userspace/private/apps/wlan | tar zxf -

  if  [ -e  wlan-router.$rver/$ver/src/wlan-all-src-$ver.tgz ]; then
     echo "Add AIRIQ executable files to wlan-all-src-$ver.tgz"
     gunzip wlan-router.$rver/$ver/src/wlan-all-src-$ver.tgz

     # Append envram binary files to wlan-all-src
     if [ -d bcmdrivers/broadcom/net/wl/$imp_org/main/src/router/envram ]; then
       echo "Add ENVRAM executable files to wlan-all-src-$ver.tgz"
       tar --append --ignore-failed-read \
           --file=wlan-router.$rver/$ver/src/wlan-all-src-$ver.tar \
             bcmdrivers/broadcom/net/wl/$imp_org/main/src/router/envram/envram-* \
             bcmdrivers/broadcom/net/wl/$imp_org/main/src/router/envram/envrams-*
     fi

     # must do for building customer/release builds
     echo "Remove bshared/src from original src base for tarballs" 
     rm -rf ${comengine}bcmdrivers/broadcom/net/wl/$imp_org/main/src/router/bshared/src
     tar --append \
         --file=wlan-router.$rver/$ver/src/wlan-all-src-$ver.tar \
             bcmdrivers/broadcom/net/wl/$imp_org/main/components/apps/airiq/prebuilt
     gzip wlan-router.$rver/$ver/src/wlan-all-src-$ver.tar
     mv wlan-router.$rver/$ver/src/wlan-all-src-$ver.tar.gz wlan-router.$rver/$ver/src/wlan-all-src-$ver.tgz
  fi

  # Not keep image when using BRAND profiles
  if [ "$brand" != "" ]; then
    tar -zcvf wlan-bin-all-$ver.tgz $exclude bcmdrivers userspace
  else
    tar -zcvf wlan-bin-all-$ver.tgz $exclude bcmdrivers userspace images
  fi
  rm -rf images
  rm -rf bcmdrivers userspace
else
  tar -zcvf wlan-bin-all-$ver.tgz $exclude makefile.wlan.src makefile.dhd.src $imp_org
fi

if  [ $use_shared == "y" ]; then
  echo Add/Del directories to wlan-bin-all-$ver.tgz
  gunzip wlan-bin-all-$ver.tgz
  tar --append \
      --file=wlan-bin-all-$ver.tar \
         ${comengine}bcmdrivers/broadcom/net/wl/shared
  gzip wlan-bin-all-$ver.tar
  mv wlan-bin-all-$ver.tar.gz wlan-bin-all-$ver.tgz
fi


#############################################################################
# Put all files in router version directory

if [ ! -d wlan-router.$rver/$ver/bin ]; then
  mkdir -p wlan-router.$rver/$ver/bin
fi

mv *tgz wlan-router.$rver/$ver/bin

if  [ $cus == "1" ]; then
  # Not keep image when using BRAND profiles 
  if [ "$brand" == "" ]; then  
    cp -r ${dimage}/images wlan-router.$rver/$ver/bin/.
  fi
fi
# End of file
