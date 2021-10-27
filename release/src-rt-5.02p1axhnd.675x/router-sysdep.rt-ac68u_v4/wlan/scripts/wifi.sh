#!/bin/bash
CPEROUTER=_set_by_buildFS_
CONFIG_WLDPDCTL=_set_by_buildFS_

#disable DPDCTL if not CPEROUTER platform
if [ -z $CPEROUTER ]; then
  CONFIG_WLDPDCTL=
fi

mod='wfd bcm_pcie_hcd'

#wlan interface power control
wldpdctl=0

#available wlan interface list
A_WLIFL=""

#down all wlan mac
wl_down()
{
  ifstr="$(ifconfig -a | grep wl)"
  for wlx in $ifstr
  do
    if case $wlx in wl*) true;; *) false;; esac; then
      wl -i $wlx down
    fi
  done
}

#suspend WLAN
# - kill all wlan applications
# - remove wlan/pcie kernel modules
wl_suspend()
{
  wl_down
  killall -q dhd_monitor
  killall -q debug_monitor
  killall -q acsd acsd2
  acs_cli -i wl0 acs_suspend 1
  nvram unset acs_ifnames
  /etc/init.d/bcm-wlan-drivers.sh stop
  for m in ${mod//-/_}; do rmmod -w $m; done
}

#resume WLAN
# - re-insert wlan/pcie kernel modules
# - restart wlan configuration manager
wl_resume()
{
  grep -e${mod// /.ko -e}.ko /etc/init.d/bcm-base-drivers.sh | sh
  /etc/init.d/bcm-wlan-drivers.sh start
  nvram restart
}

# restart WLAN sub-system after applying the settings
# check nvram control to do a system reboot or not
wldpdctl_restart()
{
# Check if we need a system reboot (default no)
  reboot=$(nvram get wl_dpdctl_enable_reboot)
  if [ -z $reboot ]; then
    reboot=0
  fi

#  apply nvram settings
  if [ $reboot = 1 ]; then
    nvram kcommit
    nvram commit
    sleep 5
    reboot
  else
    nvram kcommit
    nvram commit
    wl_suspend
    sleep 1
    wl_resume
# let wlmngr restart before the script exist
    sleep 5
  fi
}

#set pcie_apon nvram variable
# - convert decimal to hex value
# - update nvram
wldpdctl_set_apon()
{
  HEX_DIGITS="0123456789ABCDEF"
  dec_v=$1

  if [ $dec_v = 0 ]; then
    hex_v=0
  else
    hex_v=
    until [ $dec_v == 0 ]; do
      rem_v=$((dec_v % 16))
      dec_v=$((dec_v / 16))
      hex_d=${HEX_DIGITS:$rem_v:1}
      hex_v="${hex_d}${hex_v}"
    done
  fi

  nvram kset pcie_apon=0x$hex_v
}

#WLAN Interface power control init
# - check if enabled
# - initialize wlan interface to pcie apon setting (if not done)
wldpdctl_init()
{
  if [ ! -z $CONFIG_WLDPDCTL ]; then
    wldpdctl=$(nvram kget wl_dpdctl_enable)
  fi

  if [ -z $wldpdctl ]; then
    wldpdctl=0
  fi

  if [ $wldpdctl = 1 ]; then
    wlpciecore=$(nvram get wl0_dpd_apon)
    if [ -z $wlpciecore ]; then
      echo "Setting wl interface to pcie core mapping ....."
      #Virtual PCIe or without PCIe Error interrupt
      A_WLIFL=$(cat /proc/interrupts | grep wl | cut -d ',' -f3; cat /proc/interrupts | grep wl | cut -d ',' -f2 | grep -v ":")

      for intf in $A_WLIFL; do
        domain=$(cat /proc/interrupts | grep $intf | cut -d ':' -f3)
        device=$(cat /proc/interrupts | grep $intf | cut -d ':' -f5 | cut -d '.' -f1)
        core=$(cat /proc/pcie_hcd/coreinfo | grep $domain | cut -d ':' -f1)
        nvram set ${intf}_dpd_apon=$(( 3 << ($core*4 + $device*2) ))
        nvram kset ${intf}_dpd=0
      done
      nvram commit
      nvram kcommit
      echo "Available interfaces" $A_WLIFL
    fi
    if [ -z `nvram get wl0_dpd_apon` ]; then
      A_WLIFL=""
    elif [ -z `nvram get wl1_dpd_apon` ]; then
      A_WLIFL="wl0"
    elif [ -z `nvram get wl2_dpd_apon` ]; then
      A_WLIFL="wl0 wl1"
    elif [ -z `nvram get wl3_dpd_apon` ]; then
      A_WLIFL="wl0 wl1 wl2"
    else
      A_WLIFL="wl0 wl1 wl2 wl3"
    fi
  else
    if [ -z $CONFIG_WLDPDCTL ]; then
      echo "wlan deep power down control - feature missing"
    else
      if [ -z "$(nvram get lan_ifnames | grep wl0)" ]; then
        A_WLIFL=""
      elif [ -z "$(nvram get lan_ifnames | grep wl1)" ]; then
        A_WLIFL="wl0"
      elif [ -z "$(nvram get lan_ifnames | grep wl2)" ]; then
        A_WLIFL="wl0 wl1"
      elif [ -z "$(nvram get lan_ifnames | grep wl3)" ]; then
        A_WLIFL="wl0 wl1 wl2"
      else
        A_WLIFL="wl0 wl1 wl2 wl3"
      fi
      echo "wlan deep power down control - disabled"
    fi
  fi
}

#WLAN Interface power control reset all nvram settings
#enable or disable the feature based on the input
wldpdctl_enable()
{
  en=$1

  change=0;
  if [[ $en = 0 ]] && [[ $wldpdctl = 1 ]]; then
    change=1;
  fi
  if [[ $en = 1 ]] && [[ $wldpdctl = 0 ]]; then
    change=1;
  fi

  if [ $change = 1 ]; then
    if [ $wldpdctl = 1 ]; then
      #disable, First power up all interfaces
      change=0
      #Check if any interface powered down
      for intf in $A_WLIFL; do
        dpd=$(nvram kget ${intf}_dpd)
        if [ ! -z $dpd ]; then
          if [ $dpd = 1 ]; then
            change=1
          fi
        fi
      done

      if [ $change = 1 ]; then
        #Power up all interfaces
        wldpdctl_set 1 ""
        wldpdctl_restart
        sleep 5
      fi
    fi
    nvram kset wl_dpdctl_enable=$en

    nvram kunset pcie_apon
    nvram kunset wl0_dpd
    nvram kunset wl1_dpd
    nvram kunset wl2_dpd
  
    nvram unset wl0_dpd_apon
    nvram unset wl1_dpd_apon
    nvram unset wl2_dpd_apon

    nvram kcommit
    nvram commit

    wldpdctl_init

    echo "Cleared all WLAN Deep Power Down settings"
    if [ $en = 1 ]; then
      echo "Enabled WLAN Deep Power Down"
    fi
  else
    echo "No Change, dpdctl_enable=$wldpdctl"
  fi
}

# WLAN Interface power control (up/down)
# - initialize
# - parse input wlan interface list
# - Apply power down/up settings
# - restart wlan (?)
wldpdctl_set()
{
  pwrdn=$1
  intfl=$2

  for intf in $intfl; do
    match=0
    for aintf in $A_WLIFL; do
      if [ $intf = $aintf ]; then
        match=1
        break;
      fi
    done
    if [ $match = 0 ]; then
      echo "Discarding unknown interface [$intf]"
    fi
  done

  if [ $wldpdctl = 1 ]; then
    pcie_apon=0
    for aintf in $A_WLIFL; do
      match=0
      for intf in $intfl; do
        if [ $intf = $aintf ]; then
          match=1
          break
        fi
      done
      if [ $match = $pwrdn ]; then
        nvram kset ${aintf}_dpd=1
        pcie_apon=$(( pcie_apon | `nvram get ${aintf}_dpd_apon` ))
      else
        nvram kset ${aintf}_dpd=0
      fi
    done
    wldpdctl_set_apon $pcie_apon
  fi
}

# print power down status of wlan interfaces
wldpdctl_status()
{
  echo "intf  - pwrdn"
  echo "============"
  for intf in $A_WLIFL; do
    sts=$(nvram kget ${intf}_dpd)
    if [ -z $sts ]; then
      sts=0
    fi
    echo " $intf  - $sts"
  done
  echo " pcie - $(nvram kget pcie_apon)"
}

# print usage
wl_usage()
{
    echo "Usage:"
    echo " wifi.sh <cmd>      <cmd args>"
    echo "         suspend"
    echo "         resume"
    echo "         modules"
    echo "         dpdup      <wlan interface list>"
    echo "         dpddn      <wlan interface list>"
    echo "         dpdsts"
    echo "         dpden      <en>"
    echo "                     0: Disable, clear all settings"
    echo "                     1: Enable,  reset all settings"
}

case $1 in

  suspend)
    wl_suspend
    ;;

  resume)
    wl_resume
    ;;

  modules)
    for m in ${mod//-/_}; do grep -e"^\<$m\>" /proc/modules; done
    ;;

  dpdup)
    wldpdctl_init
    if [ $wldpdctl = 1 ]; then
      wldpdctl_set 0 "$2"
      wldpdctl_restart
      wldpdctl_status
    fi
    ;;

  dpddn)
    wldpdctl_init
    if [ $wldpdctl = 1 ]; then
      wldpdctl_set 1 "$2"
      wldpdctl_restart
      wldpdctl_status
    fi
    ;;

  dpdsts)
    wldpdctl_init
    if [ ! -z $CONFIG_WLDPDCTL ]; then
      wldpdctl_status
    fi
    ;;

  dpden)
    wldpdctl_init
    if [ ! -z $CONFIG_WLDPDCTL ]; then
      if [ -z $2 ]; then
        echo wldpdctl=$wldpdctl
      else
        wldpdctl_enable $2
      fi
    fi
    ;;

  *)
    wl_usage
    ;;
esac
