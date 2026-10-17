#!/bin/sh
#
# bcm_wifireconfig.sh
#
# Copyright (C) 2025, Broadcom. All Rights Reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#
# <<Broadcom-WL-IPTag/Dual>>
#
# $Id: bcm_wifireconfig.sh 851587 2025-02-07 05:43:27Z $

######################################################
#
# Functions
#
######################################################

usage() {
    exe=`basename $0`
    echo
    echo "Broadcom utility to change Wi-Fi configuration at run time"
    echo "Usage $exe [OPTIONS]"
    echo
    echo "Supported OPTIONS are"
    echo "  -h, --help                     Show this help"
    echo "  -i, --interface <ifname>       Interface where the settings have to be applied. Multiple -i options"
    echo "                                 can be provided in one command. Ex : -i wl0.1 -i wl1.1. If the"
    echo "                                 interface belongs to an MLD, then -m option must be used insted of -i"
    echo "  -m, --mldid <mldid>            The MLDID to be used (0 1 2 etc). The interface names will be picked up"
    echo "                                 from the nvram mldX_ifnames, where X is the mldid passed. This option"
    echo "                                 can not be used with -i option. Only one MLDID can be given"
    echo "  -n, --noreload                 Do not reload hostapd after setting ssid or psk. By default hostapd"
    echo "                                 reload will be called for all interfaces after setting ssid or psk."
    echo "                                 If you are running this command multiple times, you can do hostapd reload"
    echo "                                 as part of the last run"
    echo "  -p, --psk <psk>                PSK to be set to the interfaces. Only one PSK can be given"
    echo "  -s, --ssid <ssid>              SSID to be set to the given interfaces. Only one SSID can be given"
    exit 1
}

# Wrapper function to print the command and execute it
exec_cmd() {
    echo "#" $@
    "$@"
    ret=$?
    if [ $ret -ne 0 ]
    then
        echo "Command Failed : $*"
        exit $ret
    fi
}

function hostapd_reload() {
    ifname=$1
    exec_cmd hostapd_cli -i $ifname reload
}

function change_ssid() {
    ifname=$1
    ssid="$2"
    exec_cmd hostapd_cli -i $ifname set ssid "$ssid"
    exec_cmd nvram set ${ifname}_ssid=$ssid
    exec_cmd nvram commit
}

function change_psk() {
    ifname=$1
    psk="$2"
    exec_cmd hostapd_cli -i $ifname set wpa 2
    exec_cmd hostapd_cli -i $ifname set wpa_key_mgmt "SAE SAE-EXT-KEY"
    exec_cmd hostapd_cli -i $ifname set wpa_passphrase "$psk"
    exec_cmd hostapd_cli -i $ifname set sae_password "$psk"
    exec_cmd hostapd_cli -i $ifname set wpa_pairwise "CCMP GCMP-256"
    exec_cmd hostapd_cli -i $ifname set rsn_pairwise "CCMP GCMP-256"
    exec_cmd nvram set ${ifname}_wpa_psk=$psk
    exec_cmd nvram commit
}

function validate_ifnames() {
    ifnames=$*
    if [ -z "$ifnames" ]
    then
        echo "Error : Missing interface name"
        usage
    fi

    for ifname in $ifnames
    do
        # Check if interface is Wi-Fi
        wl -i $ifname msglevel > /dev/null
        if [ $? -ne 0 ]
        then
            echo "Error : Not a Wi-Fi interface $ifname"
            exit 1
        fi
    done
}

function store_ifnames() {
    if [ -z "$1" ]
    then
        echo "Error : Missing interface name"
        usage
    fi
    if [ ! -z "$mifnames" ]
    then
        echo "Error : -i and -m options can not be used together"
        exit 1
    fi

    # If this is part of an MLD, force to use the -m option
    unit=`nvram get ${1}_mld_unit`
    if [ ! -z $unit ]
    then
        echo "Error : $1 is part of MLD${unit}. Use -m option to change MLD settings"
        exit 1
    fi
    ifnames="$ifnames $1"
}

function store_mldid() {
    if [ -z "$1" ]
    then
        echo "Error : Missing MLDID"
        usage
    fi
    # Allow only 1 MLDID per command
    if [ ! -z "${mifnames}" ]
    then
        echo "Error : Multiple MLDIDs not supported"
        exit 1
    fi
    if [ ! -z "$ifnames" ]
    then
        echo "Error : -i and -m options can not be used together"
        exit 1
    fi
    mifnames=`nvram get mld${1}_ifnames`
    if [ -z "$mifnames" ]
    then
        echo "Error : mld${1}_ifnames is not set"
        exit 1
    fi
    ifnames=$mifnames
}

function store_psk() {
    if [ -z "$1" ]
    then
        echo "Error : Missing psk"
        usage
    fi

    # Allow only 1 psk to be changed
    if [ ! -z "${psk}" ]
    then
        echo "Error : Multiple psks not supported"
        usage
    fi
    psk=$1
    set_psk=1
}

function store_ssid() {
    if [ -z "$1" ]
    then
        echo "Error : Missing ssid"
        usage
    fi

    # Allow only 1 ssid to be changed
    if [ ! -z "${ssid}" ]
    then
        echo "Error : Multiple ssid not supported"
        usage
    fi
    ssid=$1
    set_ssid=1
}

# Script starts

# echo the full command. Helpful for debug
echo $0 $*

# getopts "hi:m:np:s:"
while [[ $# -ge 1 ]]; do
    o=$1
    case "${o}" in
        -h|--help)
            usage
        ;;
        -i|--interface)
            store_ifnames $2
            shift 2
        ;;
        -p|--psk)
            store_psk $2
            shift 2
        ;;
        -m|--mldid)
            store_mldid $2
            shift 2
        ;;
        -n|--noreload)
            hapd_reload=0
            shift
        ;;
        -s|--ssid)
            store_ssid $2
            shift 2
        ;;
        *)
            echo "Error : Unknown option $o"
            usage
        ;;
    esac
done

validate_ifnames $ifnames

if [ "$set_ssid" == "1" ]
then
    for ifname in $ifnames
    do
        change_ssid $ifname $ssid
    done
fi

if [ "$set_psk" == "1" ]
then
    for ifname in $ifnames
    do
        change_psk $ifname $psk
    done
fi

if [ "$hapd_reload" != "0" ]
then
    for ifname in $ifnames
    do
        hostapd_reload $ifname
    done
    sleep 3
    exec_cmd wl --all bss up
fi

# This comment block must stay at the bottom of the file.
# vim: ts=4 et sw=4
