#!/bin/bash
CONFIG_WLDPDCTL=_set_by_buildFS_
PROC_DT_DIR=/proc/device-tree
SYS_PCIE_DRV_DIR=/sys/bus/platform/drivers/bcm-pcie
PCIE_CORE_LIST="0 1 2 3 4 5 6 7"
WLUNIT_LIST="0 1 2 3"
DOMAIN_LIST=$WLUNIT_LIST
INV_UNIT=15
INV_DOMAIN=$INV_UNIT
IS_RDK_BUILD=RDK_BUILD_HOLDER
RDK_EDPD_SUPPORT=RDK_EDPD_SUPPORT_HOLDER

mod='wfd bcm_pcie_hcd'

#wlan interface power control
# DPD operating modes
readonly DPD_MODE_OFF=0
readonly DPD_MODE_DPD=1
readonly DPD_MODE_EDPD=2
readonly DPD_EDPD_FILE="/usr/sbin/wifi_dsps.sh"

#wlan interface power control
wledpdmap=0
wledpdappupd=1
wldpdmode=$DPD_MODE_OFF

# Enable edpd if WLAN supports bind/unbind
# 0: Disable
# 1: Enable without PCIe persistency
if [[ ! -z $DPD_EDPD_FILE ]] && [[ -f $DPD_EDPD_FILE ]]; then
  defedpdctl=1
else
  defedpdctl=0
fi


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
rdkb_agent_ready()
{
    status=$(systemctl status ccspwifiagent.service|grep Active|awk '{print $2}')
    if [[ $status == "active" ]]; then echo "1"; else echo "0"; fi
}

# rdkb control interface up/down
# input: ifname true/false
rdkb_radio_control()
{
    local ifname=$1
    if [[ ! -z $IS_RDK_BUILD ]] && [[ $RDK_EDPD_SUPPORT -ne 0 ]]; then

        touch /tmp/dpd_in_progress
        dm_ifindex=$(echo $ifname | cut -d 'l' -f2)
        if [[ $(rdkb_agent_ready) -eq 0 ]]; then
            echo "[wifi.sh] rdkb_radio_control returns without action: ccspwifiagent.service is not ready yet"
            return 0
        fi
        doapply=0
        dm_ifindex=$(($dm_ifindex+1))
        if [[ $2 == "up" ]]; then
            dm_radio_enabled=$(dmcli eRT getv Device.WiFi.Radio.${dm_ifindex}.Enable|grep "value: true")
            if [[ -z $dm_radio_enabled ]]; then
                nvram set ${ifname}_edpd_radio=1
                dmcli eRT setv Device.WiFi.Radio.${dm_ifindex}.Enable bool true
                doapply=1
            fi
        else
            dm_radio_enabled=$(dmcli eRT getv Device.WiFi.Radio.${dm_ifindex}.Enable|grep "value: true")
            if [[ -n $dm_radio_enabled ]] ; then
                nvram set ${ifname}_edpd_radio=1
                dmcli eRT setv Device.WiFi.Radio.${dm_ifindex}.Enable bool false
                doapply=1
            fi
        fi
        [[ $doapply -eq 1 ]] && dmcli eRT setv Device.WiFi.Radio.${dm_ifindex}.X_CISCO_COM_ApplySetting bool true
        return 0
    fi
    return 0
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

# Get PCIe domain number for the given interface from EDPD map
#
# Input:
#   $1 interface name  (wlX)
#
# Return:
#   Domain Number
#
wledpdctl_get_map()
{
  local ifname=$1
  local domain
  local wlunit
  local unit

  wlunit=$(echo $ifname | cut -d 'l' -f2)

  # Search for a matching unit number in the edpdmap
  for domain in $DOMAIN_LIST; do
    unit=$(( ($wledpdmap >> ($domain*4)) & $INV_UNIT ))
    if [ $unit = $wlunit ]; then
     break
    else
      domain=$INV_DOMAIN
    fi
  done

  echo $domain
}

# Set interface number for the given domain number in the EDPD map
#
# Input:
#   $1 domain number
#   $2 interface name (wlX)
#
# Return:
#   None
#
wledpdctl_set_map()
{
  local domain=$1
  local ifname=$2
  local wlunit

  wlunit=$(echo ${ifname} | cut -d 'l' -f2)

  #first clear the map for the given domain
  wledpdmap=$(( wledpdmap - $(( $INV_UNIT << ($domain*4) )) ))

  # set the unit number
  wledpdmap=$(( wledpdmap + $(( $wlunit << ($domain*4) )) ))

  nvram kset wl_radio_to_ifid_map=$wledpdmap

  # nvram commit will be done later
}

# Initialize EDPD map with invalid unit number for all domains
#
# Input:
#   None
#
# Return:
#   $wledpdmap
#
wledpdctl_init_map()
{
  local domain

  wledpdmap=0

  for domain in $DOMAIN_LIST; do
    wledpdmap=$(( wledpdmap + $(( $INV_UNIT << ($domain*4) )) ))
  done
}

# Get the PCIe core directory entry in from Device Tree for the given PCI domain
#
# Input:
#   $1: pci domain number
#
# Handles the following type of DT entries and corresponding platform directory
#  /pcie@[N]                <reg_base[N]>.pcie
#  /pcie[N]                 <reg_base[N]>.pcie[N]
#  /vpcie@[N]               <reg_base[N]>.vpcie
#  /vpcie[N]                <reg_base[N]>.vpcie[N]
#  /vpcie@[N]/WIFI0@0       <reg_base[N]>.vpcie
#
# Return:
#   pcie_base
#
wledpdctl_get_pcie_base()
{
  local wl_domain=$1
  local pcie_base=
  local domain
  local dt_domain
  local drv
  local core
  local reg
  local status

  # walk through all possible domains
  for domain in $WLUNIT_LIST
  do
    # Get the driver (pcie/vpcie) details
    for drv in 'pcie@' 'pcie' 'vpcie@' 'vpcie'; do
      if [ -d $PROC_DT_DIR/${drv}${domain} ]; then
        core=''
        if [ -d $PROC_DT_DIR/${drv}${domain}/WIFI0@0 ]; then
          core='/WIFI0@0'
        fi

        # Check if any DT entry has the same domain as WiFi interface
        if [ ! -z $drv ]; then
          # Get the DT domain if enabled
          status=`cat $PROC_DT_DIR/${drv}${domain}${core}/status`
          if [ $status = 'okay' ]; then
            dt_domain=`xxd -p -s 2 $PROC_DT_DIR/${drv}${domain}/linux,pci-domain`
            dt_domain=$((dt_domain))
            if [ $dt_domain = $wl_domain ]; then
              # domain matched, get the pcie register base withd river
              reg=`xxd $PROC_DT_DIR/${drv}${domain}/reg | cut -d ' ' -f2`
              if [ $reg = '0000' ]; then
                #64bit address
                reg=`xxd $PROC_DT_DIR/${drv}${domain}/reg | cut -d ' ' -f4`
              fi
              if [[ "$drv" == *"@" ]]; then
                drv=`echo $drv | cut -d '@' -f1`
                pcie_base=${reg}0000.$drv
              else
                pcie_base=${reg}0000.${drv}${domain}
              fi
              break
            fi
          fi
        fi
      fi
    done

    if [ ! -z $pcie_base ]; then
      break
    fi
  done

  echo $pcie_base

}

# Get the PCIe domain for the given core
#
# Input:
#   $1: pci core number
#
# Return:
#   pci domain
#
wledpdctl_get_pcie_dt_core_domain()
{
  local core=$1
  local dt_domain
  local extn
  local drv
  local pdrv
  local vdrv

  # Get the driver (pcie/vpcie) details
  if [ $core -le 3 ]; then
    for pdrv in 'pcie@' 'pcie'; do
      if [ -d $PROC_DT_DIR/${pdrv}${core} ]; then
        extn=''
        drv=$pdrv
      fi
    done
  else
    core=$((core-4))
    for vdrv in 'vpcie@' 'vpcie'; do
      if [ -d $PROC_DT_DIR/${vdrv}${core} ]; then
        drv=$vdrv
        if [ -d $PROC_DT_DIR/${vdrv}${core}/WIFI0@0 ]; then
          extn='/WIFI0@0'
        fi
      fi
    done
  fi

  # Check if any DT entry has the same domain as WiFi interface
  if [ ! -z $drv ]; then
    # Get the DT domain if enabled
    status=`cat $PROC_DT_DIR/${drv}${core}${extn}/status`
    if [ $status = 'okay' ]; then
      dt_domain=`xxd -p -s 2 $PROC_DT_DIR/${drv}${core}/linux,pci-domain`
      dt_domain=$((dt_domain))
    fi
  fi

  echo $dt_domain

}

# Add/Remove Dummy network interfaces
#
# Input:
#   $1: interface name
#   $2: Operation (add/del)
#
# Return:
#   None
#
wledpdctl_dummy_if()
{
  local ifname=$1
  local action=$2

  if [ $action = "del" ]; then
    #remove any dummy interfaces created
    noarp=`ifconfig $ifname | grep NOARP`
    if [ ! -z "$noarp" ];then
      ip link delete $ifname
    fi
  elif [ $action = "add" ]; then
    #Add a dummy interfaces to make applications happy
    ip link add $ifname type dummy
  fi
}

# Power down WiFi radio using unbind
# remove wlan interface and corresponding PCIe instance
# Input:
#   $1: interface name
#
# Return:
#   None
#
wledpdctl_unbind()
{
  local ifname=$1
  local sts
  local domain
  local pcie_base=

  [[ ! -z $IS_RDK_BUILD ]]  && [[ $RDK_EDPD_SUPPORT -eq 0 ]] && return 0
  if [ $wldpdmode = $DPD_MODE_EDPD ]; then
      domain=$(wledpdctl_get_map $ifname)
    if [ $domain != $INV_DOMAIN ]; then

      pcie_base=$(wledpdctl_get_pcie_base $domain)

      if [ ! -z $pcie_base ]; then
        # Check if the interface already up and running
        sts=`cat /proc/interrupts | grep $ifname | cut -d ':' -f1`
        if [[ -d $SYS_PCIE_DRV_DIR/$pcie_base ]] && [[ ! -z $sts ]]; then
          # Turn off all the interfaces of this radio
          ifid=$(echo $ifname | cut -d 'l' -f2)
          iflist=$(cat /proc/net/dev |grep -e $ifname -e wds$ifid | cut -d ':' -f1)
          for iface in $iflist; do
            wl -i $iface hwa_rxpost_reclaim_en 1
            ifconfig $iface down
            wl -i $iface down
          done
          [[ -z $IS_RDK_BUILD ]] && [[ $wledpdappupd != 0 ]] && \
              ACTION=pwrdn /etc/init.d/mdev_wl.sh $ifname


          # Unbind the PCIe and wlX interface
          echo $pcie_base > ${SYS_PCIE_DRV_DIR}/unbind

          # Create dummy interface for DPD
          wledpdctl_dummy_if $ifname add
        fi
      fi
    fi
  fi
}

# Power up WiFi radio using bind
# add wlan interface and corresponding PCIe instance
# Input:
#   $1: interface name
#
# Return:
#   None
#
wledpdctl_bind()
{
  local ifname=$1
  local unit
  local sts
  local domain
  local pcie_base=

  [[ ! -z $IS_RDK_BUILD ]]  && [[ $RDK_EDPD_SUPPORT -eq 0 ]] && return 0
  if [ $wldpdmode = $DPD_MODE_EDPD ]; then
    domain=$(wledpdctl_get_map $ifname)
    if [ $domain != $INV_DOMAIN ]; then
      pcie_base=$(wledpdctl_get_pcie_base $domain)

      if [ ! -z $pcie_base ]; then
        sts=`cat /proc/interrupts | grep $ifname | cut -d ':' -f1`
        if [[ -z $sts ]] && [[ -d $SYS_PCIE_DRV_DIR/$pcie_base ]]; then
          # powered down during boot itself, unbind PCIe, remove dummy
          # Check if it is dummy interface ?
          echo $pcie_base > $SYS_PCIE_DRV_DIR/unbind
          sleep 2
        fi

        if [ ! -d $SYS_PCIE_DRV_DIR/$pcie_base ]; then
          # Remove DPD dummy interface if exists
          wledpdctl_dummy_if $ifname del

          # Bind the PCIe and wlX interface
          echo $pcie_base > $SYS_PCIE_DRV_DIR/bind
          sleep 2
          [[ -n $IS_RDK_BUILD  ]] && nvram unset ${ifname}_edpd_radio
          [[ -z $IS_RDK_BUILD ]] && [[ $wledpdappupd != 0 ]] && \
              ACTION=pwrup /etc/init.d/mdev_wl.sh $ifname
        fi
      fi
    fi
  fi
}

# Restart EDPD after changing DPD settings
# Go through all radio's and do bind/unbind based on new settings
# This is similar to wldpdctl_restart for EDPD
#
# Input:
#   None
#
# Return:
#   None
#
wledpdctl_restart()
{
  local ifname
  local dpd
  local domain
  local sts
  local pci_base=

  for ifname in $A_WLIFL; do
    dpd=$(nvram kget ${ifname}_dpd)
    if [ ! -z $dpd ]; then
      if [ $dpd = 1 ]; then
        rdkb_radio_control $ifname "down"
        wledpdctl_unbind $ifname
      else
        domain=$(wledpdctl_get_map $ifname)
        if [ $domain != $INV_DOMAIN ]; then
          pcie_base=$(wledpdctl_get_pcie_base $domain)
          if [ ! -z $pcie_base ]; then
            sts=`cat /proc/interrupts | grep $ifname | cut -d ':' -f1`

            #Check if this is first time powering up after boot
            if [[ -z $sts ]] && [[ -d $SYS_PCIE_DRV_DIR/$pcie_base ]]; then
              # pcie base present but not the driver
              # powered down during boot itself, unbind PCIe
              echo $pcie_base > $SYS_PCIE_DRV_DIR/unbind
              sleep 1
            fi
          fi
          wledpdctl_bind $ifname
          rdkb_radio_control $ifname "up"
        fi
      fi
    fi
  done
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
    if [ $wldpdmode = $DPD_MODE_EDPD ]; then
      wledpdctl_restart
    else
      wl_suspend
      sleep 1
      wl_resume
      # let wlssk restart before the script exist
      sleep 5
    fi
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

  if [ $wldpdmode != $DPD_MODE_EDPD ]; then
    nvram kset pcie_apon=0x$hex_v
  fi
}

#WLAN Interface power control init
# - check if enabled
# - initialize wlan interface to pcie apon setting (if not done)
wldpdctl_init()
{
  wldpdmode=$(wldpdctl_mode)

  if [ $wldpdmode != $DPD_MODE_OFF ]; then
    wlpciecore=$(nvram get wl0_dpd_apon)
    if [ -z $wlpciecore ]; then
      echo "Setting wl interface to pcie core mapping ....."
      #Virtual PCIe or without PCIe Error interrupt
      A_WLIFL=$(cat /proc/interrupts | grep wl | grep -v wlan | cut -d ',' -f3; cat /proc/interrupts | grep wl | grep -v wlan | cut -d ',' -f2 | grep -v ":")
      wledpdctl_init_map

      for intf in $A_WLIFL; do
        domain=$(cat /proc/interrupts | grep $intf | grep -v msi_pcie | cut -d ':' -f3; cat /proc/interrupts | grep $intf | grep msi_pcie | cut -d ':' -f4)
        core=$(cat /proc/pcie_hcd/coreinfo | grep $domain | cut -d ':' -f1)
        wledpdctl_set_map $domain $intf
        # ignore device information as it will be on a separate domain
        # for both virtual and physical pcie devices
        nvram set ${intf}_dpd_apon=$(( 3 << ($core*4) ))
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
    if [ $wldpdmode = $DPD_MODE_EDPD ]; then
      wledpdmap=$(nvram kget wl_radio_to_ifid_map)
      if [ -z $wledpdmap ]; then
        echo "Auto switching to EDPD mode"
        wledpdctl_init_map
        for intf in $A_WLIFL; do
          dpd_apon=$(nvram get ${intf}_dpd_apon)
          for core in $PCIE_CORE_LIST; do
            #first check if this is second virtual core
            apon=$(( 12 << ($core*4) ))
            if [ $dpd_apon = $apon ]; then
              domain=$(wledpdctl_get_pcie_dt_core_domain $((core+1)))
              wledpdctl_set_map $domain $intf
            fi
            apon=$(( 3 << ($core*4) ))
            if [ $dpd_apon = $apon ]; then
              domain=$(wledpdctl_get_pcie_dt_core_domain $core)
              echo Set ifid map $intf:$domain
              wledpdctl_set_map $domain $intf
              break
            fi
          done
        done
        nvram kcommit
      fi
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
  local en=$1
  local edpd=$2

  if [ $en = 1 ]; then
    if [ -z $edpd ]; then
      edpd=$defedpdctl
    fi
  else
    edpd=0
  fi

  change=0;
  if [[ $en = 0 ]] && [[ $wldpdmode != $DPD_MODE_OFF ]]; then
    change=1;
  fi
  if [[ $en = 1 ]] && [[ $wldpdmode = $DPD_MODE_OFF ]]; then
    change=1;
  fi

  if [ $change = 1 ]; then
    if [ $wldpdmode != $DPD_MODE_OFF ]; then
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
    for unit in $WLUNIT_LIST; do
      nvram kunset wl${unit}_dpd
      nvram unset wl${unit}_dpd_apon
      nvram unset wl${unit}_dpd_mode
    done

    nvram kunset wl_radio_to_ifid_map

    nvram kcommit
    nvram commit

    wldpdctl_init

    echo "Cleared all WLAN Deep Power Down settings"
    if [ $en = 1 ]; then
      echo "Enabled WLAN Deep Power Down"
    fi
  else
    echo "No Change, dpd mode=$wldpdmode"
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

  if [ $wldpdmode != $DPD_MODE_OFF ]; then
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
  local nv=$1

  echo
  echo "***********************"
  echo "| Mode  |     $(wldpdctl_modestr $wldpdmode)    |"
  echo "***********************"
  echo "| intf  | cfg | pwrdn |"
  echo "***********************"
  for intf in $A_WLIFL; do
    cfg=$(nvram kget ${intf}_dpd)
    if [ -z $cfg ]; then
      cfg=0
    fi
    sts=`cat /proc/interrupts | grep $intf | cut -d ':' -f1`
    if [ -z $sts ]; then
      sts=1
    else
      sts=0
    fi
    echo "|  $intf  |  $cfg  |  $sts    |"
  done
  echo "***********************"

  if [ -z $nv ]; then
    nv=0
  fi
  if [ $nv != 0 ]; then
    echo ""
    nvram show |grep -e dpd -e radio_to_ifid_map -e pcie_apon
  fi
}

# Get DPD operating Mode
#
# $1: Correct mode (with possible reboot)
wldpdctl_mode()
{
  local mode
  local edpd
  local apon
  local map
  local correct
  local restart

  restart=0
  if [ ! -z $1 ]; then
    correct=$1
  else
    correct=0
  fi

  mode=$DPD_MODE_OFF
  if [ ! -z $CONFIG_WLDPDCTL ]; then
    mode=$(nvram kget wl_dpdctl_enable)

    if [ ! -z $mode ] && [ $mode != $DPD_MODE_OFF ]; then
      mode=$DPD_MODE_DPD
    else
        mode=$DPD_MODE_OFF
    fi
  fi

  if [ $mode = $DPD_MODE_DPD ]; then
    apon=$(nvram kget pcie_apon)
    map=$(nvram kget wl_radio_to_ifid_map)
    if [ $defedpdctl != 0 ]; then
      edpd=$(nvram kget wl_edpdctl_enable)
      if [ -z $edpd ]; then
        if [ ! -z $map ]; then
          if [ -z $apon ] || [ $apon = "0x0" ]; then
            # EDPD1 mode (no edpdctl, but ifid_map)
            mode=$DPD_MODE_EDPD
          elif [ $correct = 1 ]; then
            echo "=============================================="
            echo "        DPD ERROR - Settings mismatch         "
            echo "         pcie_apon set in EDPD mode           "
            echo "=============================================="
            nvram kunset pcie_apon
            restart=1
            mode=$DPD_MODE_EDPD
          fi
        fi
      elif [ $edpd != 0 ]; then
        mode=$DPD_MODE_EDPD
      fi
    elif [ -z $apon ] && [ $correct = 1 ]; then
      if [ ! -z $map ]; then
        apon=0
        for domain in $DOMAIN_LIST; do
          wlunit=$(( $map & 15 ))
          map=$(( $map >> 4 ))
          dpd=$(nvram kget wl${wlunit}_dpd)
          if [ ! -z $dpd ] && [ $dpd = 1 ]; then
            core=$(cat /proc/pcie_hcd/coreinfo | grep 000${domain} | cut -d ':' -f1)
            if [ ! -z $core ]; then
              apon=$(( apon | ( 3 << ($core*4) ) ))
            fi
          fi
        done
        if [ $apon != 0 ]; then
          echo "=============================================="
          echo "        DPD ERROR : Settings mismatch         "
          echo "        pcie_apon not set in DPD mode         "
          echo "=============================================="
          wldpdctl_set_apon $apon
          restart=1
        fi
      else
        # Can not get pcie_dpd_apon (user space settings) to construct pcie_apon
        echo "=============================================="
        echo "        DPD ERROR : Settings mismatch         "
        echo "        pcie_apon not set in DPD mode         "
        echo "=============================================="
        nvram kunset pcie_apon
        nvram kunset wl_dpdctl_enable
        restart=1
        mode=$DPD_MODE_OFF
      fi
    fi
  fi

  if [ $restart = 1 ]; then
    nvram kcommit
    echo "=============================================="
    echo "    Restarting PCIe with correct settings     "
    echo "=============================================="
    rmmod -w bcm_pcie_hcd
    grep -e bcm_pcie_hcd /etc/init.d/bcm-base-drivers.sh | sh
  fi

  echo $mode
}

wldpdctl_modestr()
{
  mode=$1

  case $mode in

    $DPD_MODE_OFF)
      modestr="OFF "
      ;;

    $DPD_MODE_DPD)
      modestr="DPD "
      ;;

    $DPD_MODE_EDPD)
      modestr="EDPD"
      ;;

    *)
      modestr="INV "
      ;;
  esac

  echo "$modestr"
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
    echo "         dpdsts     [nv dump]"
    echo "         dpdmode    [correct]"
    echo "         dpden      <en>"
    echo "                     0: Disable, clear all settings"
    echo "                     1: Enable,  reset all settings"
    echo "         edpdup     <wlan interface>  [apps update]"
    echo "         edpddn     <wlan interface>  [apps update]"
    echo "         edpdbind   <wlan interface>"
    echo "         edpdunbind <wlan interface>"
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
    if [ $wldpdmode != $DPD_MODE_OFF ]; then
      wldpdctl_set 0 "$2"
      wldpdctl_restart
    fi
    ;;

  dpddn)
    wldpdctl_init
    if [ $wldpdmode != $DPD_MODE_OFF ]; then
      wldpdctl_set 1 "$2"
      wldpdctl_restart
    fi
    ;;

  *dpdsts)
    wldpdctl_init
    if [ ! -z $CONFIG_WLDPDCTL ]; then
      vb=0
      if [ ! -z $2 ]; then
        vb=1
      fi
      wldpdctl_status $vb
    fi
    ;;

  *dpden)
    wldpdctl_init
    if [ ! -z $CONFIG_WLDPDCTL ]; then
      if [ -z $2 ]; then
        echo "$wldpdmode - $(wldpdctl_modestr $wldpdmode)"
      else
        wldpdctl_enable $2 $3
      fi
    fi
    ;;

  edpdup)
    upintf=$2
    if [ ! -z $3 ]; then
      wledpdappupd=$3
    fi
    wldpdctl_init
    if [ -z $upintf ]; then
      wl_usage
    elif [ $wldpdmode != $DPD_MODE_OFF ]; then
      intfl=""
      for intf in $A_WLIFL; do
        if [ "$upintf" = "$intf" ]; then
          dpd=0
        else
          dpd=$(wldpdctl_status | grep $intf | cut -d '|' -f3)
        fi
        if [ $dpd = 0 ]; then
          intfl=$(echo $intfl $intf)
        fi
      done
      wldpdctl_set 0 "$intfl"
      wldpdctl_restart
    fi
    ;;

  edpddn)
    dnintf=$2
    if [ ! -z $3 ]; then
      wledpdappupd=$3
    fi
    wldpdctl_init
    if [ -z $dnintf ]; then
      wl_usage
    elif [ $wldpdmode != $DPD_MODE_OFF ]; then
      intfl=""
      for intf in $A_WLIFL; do
        if [ "$dnintf" = "$intf" ]; then
          dpd=1
        else
          dpd=$(wldpdctl_status | grep $intf | cut -d '|' -f3)
        fi
        if [ $dpd = 1 ]; then
          intfl=$(echo $intfl $intf)
        fi
      done
      wldpdctl_set 1 "$intfl"
      wldpdctl_restart
    fi
    ;;

  edpdbind)
    upintf=$2
    wldpdctl_init
    if [ -z $upintf ]; then
      wl_usage
    elif [ $wldpdmode = $DPD_MODE_EDPD ]; then
      wledpdappupd=0
      wledpdctl_bind $upintf
    else
      echo "Not permitted in DPD Mode - $(wldpdctl_modestr $wldpdmode)"
    fi
    ;;

  edpdunbind)
    dnintf=$2
    wldpdctl_init
    if [ -z $dnintf ]; then
      wl_usage
    elif [ $wldpdmode = $DPD_MODE_EDPD ]; then
      wledpdappupd=0
      wledpdctl_unbind $dnintf
    else
      echo "Not permitted in DPD Mode - $(wldpdctl_modestr $wldpdmode)"
    fi
    ;;

  dpdmode)
    correct=$2
    wldpdmode=$(wldpdctl_mode $correct)

    echo "$wldpdmode | $(wldpdctl_modestr $wldpdmode)"
    ;;

  *)
    wl_usage
    ;;
esac
