#!/bin/bash
# This scripts is to build the DESKTOP_LINUX/DESKTOP_BEEP/DESKTOP_BDK
# ./dt.sh [beep] $PROFILE_NAME.
# e.g. ./dt.sh 963138GW           // build DESKTOP_LINUX with 963138GW profile
#      ./dt.sh beep 963138GW_BEEP // build DESKTOP_BEEP with 963138GW_BEEP profile
#      ./dt.sh noregen            // use the .last_profile as the $PROFILE_NAME
#      ./dt.sh result             // Don't rebuild only shows the result.
#      ./dt.sh                    // use 963138GW as the $PROFILE_NAME
# Last update: 2020.03.12
# Author: Samuel Hsu (samuel.hsu@broadcom.com)

# Default configuration.
beep_build="0"
noregen_build="0"
checkresult_build="0"
beep_enabled="0"
bdk_enabled="0"
bexee_enabled="0"
cms_enabled="0"

# Parse the arguments
if [ $# -eq 2 ]; then
  pr=$2
  if [ "$1" = "beep" ]; then
    # Want to build beep but not enable with the profile.
    if ! $(grep -q "BUILD_MODSW_EE=y"  ./targets/$pr/$pr) ; then
       echo "${pr} doesn't enable beep!!"
       exit
    fi
    beep_build="1"
  fi
else
  if [ $# -eq 1 ]; then
    if [ "$1" == "noregen" ]; then
       noregen_build="1"
    else
       if [ "$1" == "result" ]; then
          checkresult_build="1"
          noregen_build="1"
       else
          pr=$1
       fi
    fi
  else
    # Use 963138GW as the default profile if not specified.
    pr=963138GW
  fi
fi

if [ "$noregen_build" == "0" ]; then
  # check if the profile is existing
  if [ ! -f targets/$pr/$pr ]; then
    echo "${pr} doesn't exist!!"
    exit
  fi
  if [ "$beep_build" != "1" ]; then
    # Generate the DESKTOP profile on top of the given $PROFILE_NAME
    release/maketargets ${pr}_DESKTOP
    echo "${pr}_DESKTOP" > .last_profile
  else
    # Generate the DESKTOP_BEEP profile on top of the given $PROFILE_NAME
    release/maketargets ${pr}_DESKBEEP
    echo "${pr}_DESKBEEP" > .last_profile
  fi
else
  if [ ! -f .last_profile ]; then
    echo ".last_profile doesn't exist!!"
    exit
  fi
  # To avoid that we modify the profile and timestamp is newer than .last_profile.
  touch .last_profile
fi

pr=`cat .last_profile`

if $(grep -q "BUILD_BRCM_BDK=y"  ./targets/$pr/$pr) ; then
  bdk_enabled="1"
fi
if $(grep -q "BUILD_MODSW_EE=y"  ./targets/$pr/$pr) ; then
  beep_enabled="1"
fi
if $(grep -q "BUILD_MODSW_EXAMPLEEE=y"  ./targets/$pr/$pr) ; then
  bexee_enabled="1"
fi
if $(grep -q "BUILD_BRCM_CMS=y"  ./targets/$pr/$pr) ; then
  cms_enabled="1"
fi

# For busybox to include the adduser/addgroup commands.
if [ "$beep_enabled" == "1" ]; then
  export BUILD_LXC='y'
fi

if [ "$checkresult_build" != "1" ]; then
  # prepare the kernel build, for busybox build
  make pre_kernelbuild

  # this should probably be pulled into the prepare_userspace rule
  make prepare_userspace

  # to avoid -ltmctl error. (some hw dependence)
  #cd userspace/private/libs/tmctl/; make; cd ../../../..

  # build the targets
  make -C userspace gpl/apps/busybox
  make -C userspace gpl/apps/e2fsprogs
  make -C userspace gpl/apps/dhcpclient
  make -C userspace gpl/apps/iproute2
  if [ "$cms_enabled" == "1" ]; then
    make -C userspace private/apps/httpd
    make -C userspace private/apps/smd
    make -C userspace private/apps/ssk
    make -C userspace private/apps/consoled
    make -C userspace private/apps/tr69c
    make -C userspace private/apps/telnetd
  fi

  # for beep applications
  if [ "$beep_enabled" == "1" ]; then
    if [ "$bdk_enabled" == "0" ]; then
       make -C userspace private/apps/spd
    fi
    make -C userspace gpl/apps/lxc
  fi
  # for broadcom exampleEE applications
  if [ "$bexee_enabled" == "1" ]; then
    make -C userspace private/apps/exampleEE
    # To build the exampleEE package
    make beep beepPkg=exampleEE
  fi

  # extend to cover more applications.
  if [ "$cms_enabled" == "1" ]; then
    # for bee applications
    if [ "$beep_enabled" == "1" ]; then
      make -C userspace private/apps/dmad
      make -C userspace private/apps/bbcd
      make -C userspace private/apps/pmd
      make -C userspace private/apps/cwmpd
      make -C userspace private/apps/cwmpctl
      make -C userspace private/apps/dad
      make -C userspace private/apps/firewalld
      make -C userspace private/apps/spTestSuite
      make -C userspace public/apps/iperf
      make -C userspace gpl/apps/samba
      # To build the BEE package
      make beep beepPkg=bee
    fi
    make -C userspace private/apps/sntp
    make -C userspace private/apps/ddnsd
    make -C userspace private/apps/ippd
    make -C userspace private/apps/upnp
    make -C userspace private/apps/snmp
    make -C userspace private/apps/rastatus6
    make -C userspace private/apps/dsldiagd
  fi
  make -C userspace private/apps/x_dms
  make -C userspace private/apps/stress
  make -C userspace private/apps/xdslctl
  make -C userspace private/apps/hotplug
  make -C userspace gpl/apps/udhcp
  make -C userspace gpl/apps/dproxy-nexgen
  make -C userspace gpl/apps/ntfs-3g
  make -C userspace gpl/apps/ftpd
  make -C userspace public/apps/dropbear
  #make -C userspace private/apps/mcpd
  #make -C userspace public/apps/radvd
fi

# Check if the applications are built successfully.
echo "========================RESULT==================================="
cat .last_profile

# Check CMS related build
if [ "$cms_enabled" == "1" -o "$bdk_enabled" == "1" ]; then
  cms_dirs="smd httpd ssk tr69c"
  for target_dir in $cms_dirs
  do
    if [ -f userspace/private/apps/$target_dir/$target_dir ]; then
    	echo "$target_dir is built successfully!!"
    else
    	echo "$target_dir is NOK!!"
    fi
  done
fi

# Check BEEP related build
if [ "$beep_enabled" == "1" ]; then
  if [ "$bdk_enabled" == "0" ]; then
    beep_dirs="spd"
    for target_dir in $beep_dirs
    do
      if [ -f userspace/private/apps/$target_dir/$target_dir ]; then
      	echo "$target_dir is built successfully!!"
      else
      	echo "$target_dir is NOK!!"
      fi
    done
  fi
  if [ -f userspace/gpl/apps/lxc/lxc-3.1.0/src/lxc/lxc-attach ]; then
  	echo "lxc-attach is built successfully!!"
  else
  	echo "lxc-attach is NOK!!"
  fi
fi
# Check Broadcom ExampleEE related build
if [ "$bexee_enabled" == "1" ]; then
  if [ -f userspace/private/apps/exampleEE/exampleEE ]; then
  	echo "exampleEE is built successfully!!"
  else
  	echo "exampleEE is NOK!!"
  fi
fi

# Check BDK related build
if [ "$bdk_enabled" == "1" ]; then
  bdk_dirs_in_objs="sysdir_notifier sysdirctl sys_directory sysmgmt_md bcm_msgd devinfo_md openplat_md"
  for target_dir in $bdk_dirs_in_objs
  do
    if [ -f userspace/private/apps/$target_dir/objs/$target_dir ]; then
    	echo "$target_dir is built successfully!!"
    else
    	echo "$target_dir is NOK!!"
    fi
  done

  if $(grep -q "BUILD_DSL=y"  ./targets/$pr/$pr) ; then
    bdk_dsl_dirs_in_objs="dsl_md dsl_ssk"
    for target_dir in $bdk_dsl_dirs_in_objs
    do
      if [ -f userspace/private/apps/$target_dir/objs/$target_dir ]; then
    	echo "$target_dir is built successfully!!"
      else
    	echo "$target_dir is NOK!!"
      fi
    done
  fi
fi

# Check BEE related build
if [ "$cms_enabled" == "1" -o "$bdk_enabled" == "1" ]; then
  if [ "$beep_enabled" == "1" ]; then
    if [ "$cms_enabled" == "1" ]; then
      bee_dirs="bbcd"
    fi
    for target_dir in $bee_dirs
    do
      if [ -f userspace/private/apps/$target_dir/$target_dir ]; then
      	echo "$target_dir is built successfully!!"
      else
      	echo "$target_dir is NOK!!"
      fi
    done
    bee_dirs_in_objs="pmd dmad firewalld"
    if [ "$cms_enabled" == "1" ]; then
      bee_dirs_in_objs+=" dad cwmpd cwmpctl"
    else
      bee_dirs_in_objs+=" bbcd2"
    fi
    for target_dir in $bee_dirs_in_objs
    do
      if [ -f userspace/private/apps/$target_dir/objs/$target_dir ]; then
      	echo "$target_dir is built successfully!!"
      else
      	echo "$target_dir is NOK!!"
      fi
    done
    if [ -f userspace/private/apps/spTestSuite/objs/spMaster ] && [ -f userspace/private/apps/spTestSuite/objs/spRobot ] && [ -f userspace/private/apps/spTestSuite/objs/spSuperRobot ] ; then
    	echo "spTestSuite is built successfully!!"
    else
    	echo "spTestSuite is NOK!!"
    fi
    if $(grep -q "BUILD_BEEP_DSLDIAGD=y"  ./targets/$pr/$pr) ; then
      if [ -f userspace/private/apps/dsldiagd/dsldiagd ] ; then
      	echo "dsldiagd is built successfully!!"
      else
      	echo "dsldiagd is NOK!!"
      fi
    fi
    if $(grep -q "BUILD_BEEP_IPERFV2=y"  ./targets/$pr/$pr) ; then
      if [ -f userspace/public/apps/iperf/iperf-2.0.9_beep/src/iperf ]; then
      	echo "beep_iperfv2 is generated successfully!!"
      else
      	echo "beep_iperfv2 binary is NOK!!"
      fi
    fi
    if $(grep -q "BUILD_BEEP_SAMBA=y"  ./targets/$pr/$pr) ; then
      if [ -f userspace/gpl/apps/samba/samba_beep/sambaservice ]; then
      	echo "sambaservice is generated successfully!!"
      else
      	echo "sambaservice is NOK!!"
      fi
    fi
  fi
fi


# Check some userspace applications build
if [ "$cms_enabled" == "1" -o "bdk_enabled" ==  "1" ]; then
  if $(grep -q "BUILD_TELNETD=y"  ./targets/$pr/$pr) ; then
    if [ -f userspace/private/apps/telnetd/telnetd ]; then
    	echo "telnetd is built successfully!!"
    else
  	  echo "telnetd is NOK!!"
    fi
  fi
  if $(grep -q "BUILD_CONSOLED=y"  ./targets/$pr/$pr) ; then
    if [ -f userspace/private/apps/consoled/consoled ]; then
  	  echo "consoled is built successfully!!"
    else
  	  echo "consoled is NOK!!"
    fi
  fi
  if $(grep -q "BUILD_DDNSD=dynamic"  ./targets/$pr/$pr) ; then
    if [ -f userspace/private/apps/ddnsd/ddnsd ]; then
   	  echo "ddnsd is built successfully!!"
    else
  	  echo "ddnsd is NOK!!"
    fi
  fi
  if $(grep -q "BUILD_IPPD=dynamic"  ./targets/$pr/$pr) ; then
    if [ -f userspace/private/apps/ippd/objs/ippd ]; then
  	  echo "ippd is built successfully!!"
    else
  	  echo "ippd is NOK!!"
    fi
  fi
  if $(grep -q "BUILD_UPNP=dynamic"  ./targets/$pr/$pr) ; then
    if [ -f userspace/private/apps/upnp/upnp ]; then
  	  echo "upnp is built successfully!!"
    else
  	  echo "upnp is NOK!!"
    fi
  fi
  if $(grep -q "BUILD_DPROXY=y"  ./targets/$pr/$pr) ; then
    if [ -f userspace/gpl/apps/dproxy-nexgen/dproxy-nexgen/dnsproxy ]; then
    	echo "dnsproxy is built successfully!!"
    else
    	echo "dnsproxy is NOK!!"
    fi
  fi
  if $(grep -q "BUILD_IPV6=y"  ./targets/$pr/$pr) ; then
    if [ -f userspace/private/apps/rastatus6/rastatus6 ]; then
    	echo "rastatus6 is built successfully!!"
    else
    	echo "rastatus6 is NOK!!"
    fi
  fi
fi

if $(grep -q "BUILD_DLNA=y"  ./targets/$pr/$pr) ; then
  if [ -f userspace/private/apps/x_dms/x_dms/bcmmserver/bcmmserver ] || [ -f userspace/private/apps/x_dms/x_dms/bcmmserver ]; then
  	echo "DLNA_DMS is built successfully!!"
  else
  	echo "DLNA_DMS is NOK!!"
  fi
fi
if $(grep -q "BUILD_STRESS=y"  ./targets/$pr/$pr) ; then
  if [ -f userspace/private/apps/stress/stress ]; then
  	echo "stress is built successfully!!"
  else
  	echo "stress is NOK!!"
  fi
fi
if $(grep -q "BUILD_SSHD=y"  ./targets/$pr/$pr) ; then
  if [ -f userspace/public/apps/dropbear/dropbear/dropbear ]; then
  	echo "dropbear is built successfully!!"
  else
  	echo "dropbear is NOK!!"
  fi
fi
#if $(grep -q "BUILD_IPV6=y"  ./targets/$pr/$pr) ; then
#  if [ -f userspace/private/apps/mcpd/mcpd ]; then
#  	echo "mcpd is built successfully!!"
#  else
#  	echo "mcpd is NOK!!"
#  fi
#  if [ -f userspace/public/apps/radvd/radvd/radvd ]; then
#  	echo "radvd is built successfully!!"
#  else
#  	echo "radvd is NOK!!"
#  fi
#fi
