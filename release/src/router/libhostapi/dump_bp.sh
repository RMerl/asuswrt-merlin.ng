#!/bin/sh

id=1
while [ "$id" -le 25 ]; do
    echo "=== Dumping config for nBridgePortId=$id ==="
    fapi-GSW-BridgePortConfigGet nBridgePortId=$id
    echo ""
    echo "----------------------------------------"
    echo ""
    echo ""
    id=`expr $id + 1`
done
