#!/bin/sh
# Usage:
#	./dump_mib.sh <mxl_nport_id> <sf2_unit> <sf2_port>
#
# Example:: dump LAN2 (in gs-be18000):
#	./dump_mib.sh 6 0 5 
#

MXL_PORT=$1      # MXL switch port number
SF2_UNIT=$2      # BRCM SF2 unit number
SF2_PORT=$3      # BRCM SF2 port number

if [ -z "$MXL_PORT" ] || [ -z "$SF2_UNIT" ] || [ -z "$SF2_PORT" ]; then
    echo "Usage: $0 <mxl_nport_id> <sf2_unit> <sf2_port>"
    exit 1
fi

echo "=========================================="
echo "Dumping MIB Counters for MXL Switch (Port $MXL_PORT)"
echo "=========================================="
fapi-GSW-RMON-PortGet nPortId=$MXL_PORT

sleep 1

echo
echo
echo
echo "=========================================="
echo "Dumping MIB Counters for BRCM SF2 Switch (Unit $SF2_UNIT, Port $SF2_PORT)"
echo "=========================================="
ethswctl -c mibdump -n $SF2_UNIT -p $SF2_PORT -a
if [ $? -ne 0 ]; then
    echo "[ERROR] Failed to get BRCM SF2 switch MIB counters for unit $SF2_UNIT port $SF2_PORT"
fi

