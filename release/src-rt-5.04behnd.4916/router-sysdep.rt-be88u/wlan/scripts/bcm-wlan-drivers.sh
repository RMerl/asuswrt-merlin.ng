#!/bin/sh
# This script loads/unloads the loadable kernel modules for Wireless feature
# usage:
# bcm-wlan-drivers.sh operation [module]
#
# operation:  start, stop, restart
# [module]:   WLAN kernel module/s
#
# Script uses below order to find the WLAN modules to operate on
# argument $2, nvram (kernel_modules), default list based on image
# check existance of driver in the system before loading

trap "" 3
#The following will be populated by buildFS during the make process:
KERNELVER=_set_by_buildFS_
WLAN_BTEST=_set_by_buildFS_
PROD_FW_PATH=_set_by_buildFS_
CONFIG_WLDPDCTL=_set_by_buildFS_
BRCM_CHIP=_set_by_buildFS_
WLAN_REMOVE_INTERNAL_DEBUG=_set_by_buildFS_
if [ ! -z $WLAN_REMOVE_INTERNAL_DEBUG ]; then
    dhd_console_ms=0
else
    dhd_console_ms=250
fi
SYSTEM_INFO_INDICATOR=$(cat /proc/nvram/wl_nand_manufacturer)
is_mfg=$(($SYSTEM_INFO_INDICATOR & 2))
SYSTEM_UBOOT=$(($SYSTEM_INFO_INDICATOR & 8))
if [ ! -z $PROD_FW_PATH ]; then
    MFG_FW_PATH=$PROD_FW_PATH"/mfg"
else
    MFG_FW_PATH="/rom/etc/wlan/dhd/mfg"
fi

# Allow wifi.sh script to correct dpd settings first
/rom/etc/init.d/wifi.sh dpdmode 1
DPDMODE=`sh /rom/etc/init.d/wifi.sh dpdmode |  cut -d '|' -f1`
if [ -z $DPDMODE ]; then
  DPDMODE=0
fi
echo "DPDMODE:$DPDMODE"

#wl_unitlist -knvram or environment varible to define radio interface
#name index order. Ex. 
#    1 0 2: one dhd(wl1) and one NIC(wl0)
#    1 2 0: two dhd(wl1/2) and one NIC(wl0)
if [ $SYSTEM_UBOOT -gt 0 ]; then
    WIFI_DRV_LOAD_ORDER=/proc/environment/wl_unitlist
else
    WIFI_DRV_LOAD_ORDER=/proc/brcm/blxparms/wl_unitlist
fi

if  [ -e $WIFI_DRV_LOAD_ORDER ]; then
    UNITLIST="$(cat $WIFI_DRV_LOAD_ORDER)"
else
    UNITLIST=`nvram kget wl_unitlist`
    if [ -z "$UNITLIST" ]; then
        UNITLIST="0 1 2 3"
    fi
fi
echo "UNITLIST:$UNITLIST"

# PCIe Wireless Card Status - bad card detection
pwlcs_preload()
{
  # Check if PWLCS is enabled or not
  # 0/not present - Disabled
  # N             - Enabled for N cards (N is generally 2 or 3)
  # Also need to compile DHD with DSLCPE_PWLCS flag
  MAXPWLCS=`nvram kget pwlcsmaxcrd`
  if [ "$MAXPWLCS" == "" ];  then
    MAXPWLCS=0
  fi

  if [ $MAXPWLCS != 0 ];  then
    echo "PWLCS:: DHD pre-load processing ..."
    needcommit=0
    card=0
    panic_on_oops=`cat /proc/sys/kernel/panic_on_oops`

    #disable panic_on_oops to stop auto rebooting on dhd fail
    echo 0 > /proc/sys/kernel/panic_on_oops

    # Initialize card status to GOOD, if not present
    while [ $card -lt $MAXPWLCS ]
    do
      status=`nvram kget pwlcspcie$card`
      if [ "$status" == "" ];  then
        echo "PWLCS:: PCIe[$card] status intializing to GOOD"
        nvram kset pwlcspcie$card="GOOD"
        needcommit=1
      fi
      echo "PWLCS:: PCIe[$card] pre-load status = $status"
      card=$(($card+1))
    done

    # Commit changes if needed
    if [ $needcommit == 1 ];  then
      nvram kcommit
      sync
    fi
  fi
}

pwlcs_postload()
{
  if [ $MAXPWLCS != 0 ];  then
    echo "PWLCS:: DHD post-load processing ..."
    needcommit=0
    card=0

    # Get the updated card status changed by DHD
    while [ $card -lt $MAXPWLCS ]
    do
      status=`nvram kget pwlcspcie$card`
      echo "PWLCS:: PCIe[$card] post-load status = $status"

      # Save and commit driver changes to the nvram
      nvram kset pwlcspcie$card="$status"
      nvram kcommit
      sync

      #restore the panic_on_oops for the rest of the system
      echo $panic_on_oops > /proc/sys/kernel/panic_on_oops

      # Check if card is a bad card
      case "$status" in
        *BOOT*)
          ifexists=`cat /proc/net/dev | grep -e '^[[:space:]]*wl[0-9]' |grep wl$card`
          set -- $ifexists
          if [ "$1" == "" ];  then
            echo "==============================================================================="
            echo "Detected bad WLAN card on PCIe$card, rebooting system"
            echo "==============================================================================="
            # Trigger magic Sys Request reboot for fast reboot
            echo b > /proc/sysrq-trigger
          fi
        ;;
      esac
      card=$(($card+1))
    done
  fi
}

# to get instance base mixed with dhd and wl
get_unit()
{
#   selection method - first available unit

    for unit in $UNITLIST; do
        wlx=`cat /proc/net/dev | grep -e '^[[:space:]]*wl[0-9]' |grep wl$unit`
        if [ -z "$wlx" ]; then
            break
        fi
    done

     echo "$unit"
}

# create dummy network interfaces for power down units
wldpdctl_init()
{
  # Create dummy interfaces in DPD mode only (not in EDPD mode) during boot
  if [ $DPDMODE = 1 ]; then
    for unit in $UNITLIST; do
      pwrdn=`nvram kget wl${unit}_dpd`
      if [[ -z "$pwrdn" ]]; then
        pwrdn="0"
      fi
      if [ "$pwrdn" = "1" ]; then
        ip link add wl$unit type dummy
        echo "Created dummy network interface wl$unit"
      fi
    done
  fi
}

# remove dummy network interfaces for power down units
wldpdctl_deinit()
{
  # Clear any dummy interfaces created by (E)DDPD
  if [ $DPDMODE -ge 1 ]; then
    intfl=`cat /proc/net/dev | grep -e '^[[:space:]]*wl[0-9]' | grep " wl" | cut -d ":" -f1`
    for intf in $intfl; do
      noarp=`ifconfig $intf | grep NOARP`
      if [[ -z "$noarp" ]]; then
        dummy="0"
      else
        dummy="1"
      fi
      if [ "$dummy" = "1" ]; then
        ip link delete $intf
        echo "Deleted dummy network interface $intf"
      fi
    done
  fi
}

# Load given WLAN drivers
# modules_list contains the list of drivers to be loaded
load_modules()
{

        # set the default module list and parameters
        # check whether the module exists or not before adding
        dhd_module_params="iface_name=wl dhd_console_ms=$dhd_console_ms"
        wl_module_params="intf_name=wl%d"
        echo " ####  mfg mode:$is_mfg ########"
        if [ $is_mfg -gt 0 ]; then
            dhd_module_params=$dhd_module_params" firmware_path=$MFG_FW_PATH"
        fi

        wldpdctl_init

        echo "loading WLAN kernel modules ... $modules_list"

        for module in $modules_list
        do
            case "$module" in
                firmware_class)
                    insmod /lib/modules/$KERNELVER/kernel/drivers/base/firmware_loader/firmware_class.ko
                    ;;
                cfg80211)
                    echo "insmod /lib/modules/$KERNELVER/kernel/net/wireless/$module.ko"
                    ;;
                aux)
		    if [ -f /etc/wlan/nvram/aux/nvram.txt ]; then
			module_params=extnvm_path=/etc/wlan/nvram/aux/nvram.txt
		    fi
                    ;;
                wlemf|emf|igs)
                    #no module parameters
                    module_params=""
                    ;;
                hnd)
                    module_params=""
                    if  [ $is_mfg -gt 0 ] && [ -f /lib/modules/$KERNELVER/extra/hnd_mfgtest.ko ]; then
                        module="hnd_mfgtest"
                    fi
                    ;;
                wl)
                    module_params=$wl_module_params
                    if  [ $is_mfg -gt 0 ] && [ -f /lib/modules/$KERNELVER/extra/wl_mfgtest.ko ]; then
                        module="wl_mfgtest"
                    fi
                    ;;
                dhd)
                    module_params=$dhd_module_params
                    ;;
                wlshared)
                    module_params=""
                    ;;
                *)
                    echo "wlan-drivers: unrecognized module [$module] in the load module list"
                    module_params=""
                    ;;
            esac

            if [ "$module" == "dhd" ] || [ "$module" == "wl" ] || [ "$module" == "wl_mfgtest" ]; then
                idx=`get_unit`
                instance_base="instance_base=$idx"
                module_params=$module_params" "$instance_base
            fi

            if [ -e /lib/modules/$KERNELVER/extra/$module.ko ]; then
                if [ "$module" == "dhd" ]; then
                    for unit in $UNITLIST; do
                        mkdir -p /tmp/wl$unit
                        mkdir -p /tmp/hc/if$unit
                    done
                    pwlcs_preload
                    echo "insmod /lib/modules/$KERNELVER/extra/$module.ko $module_params"
                    pwlcs_postload
                else
                    nic_nvram=`nvram kget wl_path`
                    if [ "$module" == "wl" ] && [ -f "$nic_nvram" ] ; then
                        echo "*** wl$idx Using alternate nic driver from $nic_nvram"
                        echo "insmod $nic_nvram $module_params"
                    else
                        echo "insmod /lib/modules/$KERNELVER/extra/$module.ko $module_params"
                    fi
                    if [ "$module" == "wl" ]; then
                        if echo $(/bin/pspctl dump wlSkbFreeOfDis) | grep -q '1'
                        then
                            echo "Disable SKB Free offload feature for wl$idx"
                            wl -i wl$idx skb_free_offload 0
                        fi
                    fi
                fi
            fi
        done

        # Configuing wlan related threads
        wlaffinity auto
}

# Unload given WLAN drivers
# modules_list contains the list of drivers to be loaded
unload_modules()
{
    echo "unloading WLAN kernel modules ... $unload_modules_list"

    for module in $unload_modules_list
    do
        echo unload $module
        echo "rmmod $module.ko"
    done

    wldpdctl_deinit
}


# Check for arguments.
# User should give atleast one argument to the program.
if [ $# -lt 1 ]; then
    echo "Usage: $0 operation [module] ..."
    echo "       operation:  start, stop, restart"
    echo "       [module]:   WLAN kernel module"
    exit 64
fi

# Do nothing for WLAN BTEST builds
if [ $SYSTEM_UBOOT -gt 0 ]; then
WLAN_BTEST_FILEPATH=/proc/environment/wlan_btest
else
WLAN_BTEST_FILEPATH=/proc/brcm/blxparms/wlan_btest
fi
if  [ -e $WLAN_BTEST_FILEPATH ]; then
    btest_mode=$(cat $WLAN_BTEST_FILEPATH)
    #nvram kset btest_mode=$btest_mode
    #nvram kcommit
else
    btest_mode=`nvram kget wlan_btest`
fi
if [ "$btest_mode" == "1" ];  then
    echo "Skipping wlan-drivers.sh since the kernel nvram wlan_btest=1"
    exit 0
fi
if [ "$WLAN_BTEST" == "y" ]; then
    if [ -f /usr/sbin/utelnetd ] ; then
        /usr/sbin/utelnetd -d
    fi
    echo "Skipping wlan-drivers.sh for BTEST images ..."
    exit 0
fi

# Sanity check for drivers directory
if [ ! -d /lib/modules/$KERNELVER/extra ]; then
    echo "ERROR: wlan-drivers.sh: /lib/modules/$KERNELVER/extra does not exist" 1>&2
    exit 1
fi

# Get the modules list
# Check argument2 first, then nvram, then default based on image
if [ ! -z "$2" ]; then
    modules_list=$2
    unload_modules_list=$modules_list
else
    all_wlan_modules="hnd emf igs wl"
    if [ -f /lib/modules/$KERNELVER/kernel/net/wireless/cfg80211.ko ]; then
        all_wlan_modules="cfg80211 $all_wlan_modules"
    fi
    if [ -f /lib/modules/$KERNELVER/extra/aux.ko ]; then
        all_wlan_modules="$all_wlan_modules aux"
    fi
    if [ -f /lib/modules/$KERNELVER/kernel/drivers/base/firmware_loader/firmware_class.ko ]; then
        all_wlan_modules="firmware_class $all_wlan_modules"
    fi

    # add wlshared module
    if [ -f /lib/modules/$KERNELVER/extra/wlshared.ko ]; then
        all_wlan_modules="wlshared $all_wlan_modules"
    fi

    # Update the wlan module list from nvram if exists
    is_nvmodules_list=`nvram show 2>&1 |grep kernel_mods | grep -c '^'`
    if [ $is_nvmodules_list -eq '0' ]; then
        echo "no modules list in nvram, using defaults..."
        modules_list=$all_wlan_modules
    else
        modules_list=`nvram kget kernel_mods`
    fi

    for module in $modules_list
    do
        unload_modules_list="$module $unload_modules_list";
    done
fi

# Parse the first argument
case "$1" in
    start)
        echo "skip load_modules in bcm-wlan-drivers.sh"
        #indication of bcacpe wifi platform and driver loading done
        touch /tmp/.brcm_bcacpe_wifi
        exit 0
        ;;

    stop)
        echo "skip unload_modules in bcm-wlan-drivers.sh"
        exit 0
        ;;

    restart)
        unload_modules
        sleep 3
        load_modules
        exit 0
        ;;

    *)
        echo "wlan-drivers: unrecognized option [$1]"
        exit 1
        ;;
esac
