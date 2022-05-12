#!/bin/sh
## MODULE
#  WME: 0xa
#  WPA: 0xc
#  dump radius pkt: 0xe
#  802.1x radius client: 0xf
#  802.1x state machine: 0x10
#  802.1x authenticator: 0x11
#  state: 0x13
#  auth: 0x16
#  assoc: 0x17
#  node: 0x18
#  mlme: 0x1f
#  DP: 0x45
#  NSS: 0x58
## LEVEL
# 0: none,   1: fatal,  2: error,  3: warn,  4: info
# 5: info_h, 6: info_m, 7: info_l, 8: debug, 9: trace
# 10: all
## Command line Parameters
# $1: Module hex ID
# $2: Max. dbg lvl
# $3: Module name
function usage() {
echo "Usage: $0 VAP MODULE_ID MAX_DBG_LVL"
exit
}

[ -z "$1" -o -z "$2" -o -z "$3" ] && usage()
echo "VAP: $1, ID: $2, LVL: $3"

for l in `seq 0 $3` ; do
cfg80211tool $1 qdf_cv_lvl `printf "0x%x" $2``printf "%04x" ${l}`
done
cfg80211tool $1 g_qdf_cv_lvl

