#!/bin/sh
# This script loads/unloads the loadable kernel modules for Wireless feature for non-HND images
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
HNDROUTER=_set_by_buildFS_
CPEROUTER=_set_by_buildFS_
PROD_FW_PATH=_set_by_buildFS_
if [ ! -z $PROD_FW_PATH ]; then
    MFG_FW_PATH=$PROD_FW_PATH"/mfg"
else
    MFG_FW_PATH="/etc/wlan/dhd/mfg"
fi

# to get instance base mixed with dhd and wl
get_instance_base()
{
     unit=0
     ifstr="$(ifconfig -a | grep wl)"
     for wlx in $ifstr
     do
         case "$wlx" in
             wl0)
                 unit=1
                 ;;
             wl1)
                 unit=2
                 ;;
             wl2)
                 unit=3
                 ;;
         esac
     done
     echo "instance_base=$unit"
}

# Load given WLAN drivers
# modules_list contains the list of drivers to be loaded
load_modules()
{

        # set the default module list and parameters
        # check whether the module exists or not before adding
        # set the module parameters based on hnd (cperouter) or wlemf (legacy)
        if [ ! -z $CPEROUTER ]; then
            #CPE Router
            dhd_module_params="iface_name=wl dhd_console_ms=0"
            wl_module_params="intf_name=wl%d"
            wl_mfgtest_module="wl_mfgtest"
            hnd_mfgtest_module="hnd_mfgtest"
            if [ -f /proc/nvram/wl_nand_manufacturer ]; then
                is_mfg=`cat /proc/nvram/wl_nand_manufacturer`
                case $is_mfg in
                    2|3|6|7)
                        dhd_module_params="iface_name=wl dhd_console_ms=0 firmware_path=$MFG_FW_PATH"
                        if [ -f /lib/modules/$KERNELVER/extra/$wl_mfgtest_module.ko ]; then
                            modules_list=${modules_list/wl/$wl_mfgtest_module}
                        fi
                        if [ -f /lib/modules/$KERNELVER/extra/$hnd_mfgtest_module.ko ]; then
                            modules_list=${modules_list/hnd/$hnd_mfgtest_module}
                        fi
                        ;;
                    *)
                        ;;
                esac
            fi
        else
            #Legacy
            dhd_module_params="iface_name=wl dhd_console_ms=0 firmware_path=/etc/wlan/dhd mfg_firmware_path=/etc/wlan/dhd/mfg  dhd_dpc_prio=5"
            wl_module_params="intf_name=wl%d"
        fi

        echo "loading WLAN kernel modules ... $modules_list"

        for module in $modules_list
        do
            case "$module" in
                wlemf|hnd|hnd_mfgtest|emf|igs)
                    #no module parameters
                    module_params=""
                    ;;
                wl|wl_mfgtest)
                    module_params=$wl_module_params
                    ;;
                dhd)
                    module_params=$dhd_module_params
                    ;;
                *)
                    echo "wlan-drivers: unrecognized module [$module] in the load module list"
                    module_params=""
                    ;;
            esac

            if [ "$module" == "dhd" ] || [ "$module" == "wl" ]; then
                instance_base=`get_instance_base`
                module_params=$module_params" "$instance_base
            fi

            if [ -e /lib/modules/$KERNELVER/extra/$module.ko ]; then
                insmod /lib/modules/$KERNELVER/extra/$module.ko $module_params
            fi
        done
}

# Unload given WLAN drivers
# modules_list contains the list of drivers to be loaded
unload_modules()
{
    echo "unloading WLAN kernel modules ... $unload_modules_list"

    for module in $unload_modules_list
    do
	echo unload $module
        rmmod $module.ko
    done
}


# Check for arguments.
# User should give atleast one argument to the program.
if [ $# -lt 1 ]; then
    echo "Usage: $0 operation [module] ..."
    echo "       operation:  start, stop, restart"
    echo "       [module]:   WLAN kernel module"
    exit 64
fi

# Do nothing for hnd only builds (HNDROUTER=y and CPEROUTER=)
if [ ! -z $HNDROUTER ]; then
    if [ -z $CPEROUTER ]; then
        echo "Skipping wlan-drivers.sh for HND images ..."
        exit 0
    fi
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
    if [ ! -z $CPEROUTER ]; then
        all_wlan_modules="hnd emf igs dhd wl"
    else
        all_wlan_modules="wlemf dhd wl"
    fi

    # Update the wlan module list from nvram if exists
    is_nvmodules_list=`nvram show 2>&1 |grep kernel_mods | grep -c '^'`
    if [ $is_nvmodules_list -eq '0' ]; then
        echo "no modules list in nvram, using defaults..."
        modules_list=$all_wlan_modules
    else
        modules_list=`nvram get kernel_mods`
    fi
    for module in $modules_list
    do
        unload_modules_list="$module $unload_modules_list";
    done
fi

# Parse the first argument
case "$1" in
    start)
        load_modules
        exit 0
        ;;

    stop)
        unload_modules
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
