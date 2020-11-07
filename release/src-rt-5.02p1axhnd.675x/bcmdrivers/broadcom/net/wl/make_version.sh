#!/bin/bash

# $1: impl no; $2: append version(-wapi)
imp=$1

. ../../../../version.make
. $imp/dslcpe_wlan_minor_version

DSLCPE_WLAN_VERSION=cpe$BRCM_VERSION.$BRCM_RELEASE'L.'$BRCM_EXTRAVERSION.$DSLCPE_WLAN_MINOR_VERSION

EPI_VERSION_STR=`sed -ne "s/,[ ]/./g" -ne "s/.*EPI_VERSION[ \t][ \t]*\(.*\)/\1/p" \
  < $imp/include/epivers.h`

echo "EPI_VERSION_STR=$EPI_VERSION_STR.$DSLCPE_WLAN_VERSION"$2 > $imp/epivers
dos2unix -q $imp/epivers

ROUTER_VERSION_STR=`sed -ne "s/,[ ]/./g" -ne "s/.*ROUTER_VERSION[ \t][ \t]*\(.*\)/\1/p" \
  < $imp/router/shared/router_version.h`

echo "ROUTER_VERSION_STR=$ROUTER_VERSION_STR" > $imp/rver
dos2unix -q $imp/rver
