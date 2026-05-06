#!/bin/sh

get_id() {
    echo "$1" | grep "Allocated" | awk -F'=' '{print $NF}' | tr -d ' \r\n'
}

echo "Alloc Br/Bp"

BR_L1=$(get_id "$(fapi-GSW-BridgeAlloc)"); 
BR_L2=$(get_id "$(fapi-GSW-BridgeAlloc)"); 
BR_L3=$(get_id "$(fapi-GSW-BridgeAlloc)"); 
BR_L4=$(get_id "$(fapi-GSW-BridgeAlloc)"); 
BP_L1=$(get_id "$(fapi-GSW-BridgePortAlloc)"); 
BP_L2=$(get_id "$(fapi-GSW-BridgePortAlloc)");
BP_L3=$(get_id "$(fapi-GSW-BridgePortAlloc)"); 
BP_L4=$(get_id "$(fapi-GSW-BridgePortAlloc)");
EXV_L1=$(get_id "$(fapi-GSW-ExtendedVlanAlloc nNumberOfEntries=3)"); 
EXV_L2=$(get_id "$(fapi-GSW-ExtendedVlanAlloc nNumberOfEntries=3)");
EXV_L3=$(get_id "$(fapi-GSW-ExtendedVlanAlloc nNumberOfEntries=3)");
EXV_L4=$(get_id "$(fapi-GSW-ExtendedVlanAlloc nNumberOfEntries=3)");

if ! [ "$BP_L4" -eq "$BP_L4" ] 2>/dev/null; then
    echo "Wrong ! cannot get bp id: BP_L4='$BP_L4'"
    exit 1
fi

echo "Alloc results: LAN1(BR:$BR_L1, BP:$BP_L1, EXV:$EXV_L1), LAN2(BR:$BR_L2, BP:$BP_L2, EXV:$EXV_L2)"


echo "CPU Ingress (cpu->switch rules)"
fapi-GSW-CtpPortConfigSet nLogicalPortId=13 bIngressExtendedVlanEnable=1 nIngressExtendedVlanBlockId=0x60;

# idx_3,5,7,9 rules are for inner-untagged packets
fapi-GSW-ExtendedVlanSet nExtendedVlanBlockId=0x60 nEntryIndex=3 eOuterVlanFilterVlanType=0 bOuterVlanFilterVidEnable=1 nOuterVlanFilterVidVal=1001 eOuterVlanFilterTpid=1 eInnerVlanFilterVlanType=3 eRemoveTagAction=1 bReassignBridgePortEnable=1 nNewBridgePortId=$BP_L1;
fapi-GSW-ExtendedVlanSet nExtendedVlanBlockId=0x60 nEntryIndex=5 eOuterVlanFilterVlanType=0 bOuterVlanFilterVidEnable=1 nOuterVlanFilterVidVal=1002 eOuterVlanFilterTpid=1 eInnerVlanFilterVlanType=3 eRemoveTagAction=1 bReassignBridgePortEnable=1 nNewBridgePortId=$BP_L2;
fapi-GSW-ExtendedVlanSet nExtendedVlanBlockId=0x60 nEntryIndex=7 eOuterVlanFilterVlanType=0 bOuterVlanFilterVidEnable=1 nOuterVlanFilterVidVal=1003 eOuterVlanFilterTpid=1 eInnerVlanFilterVlanType=3 eRemoveTagAction=1 bReassignBridgePortEnable=1 nNewBridgePortId=$BP_L3; 
fapi-GSW-ExtendedVlanSet nExtendedVlanBlockId=0x60 nEntryIndex=9 eOuterVlanFilterVlanType=0 bOuterVlanFilterVidEnable=1 nOuterVlanFilterVidVal=1004 eOuterVlanFilterTpid=1 eInnerVlanFilterVlanType=3 eRemoveTagAction=1 bReassignBridgePortEnable=1 nNewBridgePortId=$BP_L4; 

# idx_11~14 rules are for inner-tagged packets
fapi-GSW-ExtendedVlanSet nExtendedVlanBlockId=0x60 nEntryIndex=11 eOuterVlanFilterVlanType=0 bOuterVlanFilterVidEnable=1 nOuterVlanFilterVidVal=1001 eOuterVlanFilterTpid=1 eInnerVlanFilterVlanType=0 eRemoveTagAction=1 bReassignBridgePortEnable=1 nNewBridgePortId=$BP_L1; 
fapi-GSW-ExtendedVlanSet nExtendedVlanBlockId=0x60 nEntryIndex=12 eOuterVlanFilterVlanType=0 bOuterVlanFilterVidEnable=1 nOuterVlanFilterVidVal=1002 eOuterVlanFilterTpid=1 eInnerVlanFilterVlanType=0 eRemoveTagAction=1 bReassignBridgePortEnable=1 nNewBridgePortId=$BP_L2; 
fapi-GSW-ExtendedVlanSet nExtendedVlanBlockId=0x60 nEntryIndex=13 eOuterVlanFilterVlanType=0 bOuterVlanFilterVidEnable=1 nOuterVlanFilterVidVal=1003 eOuterVlanFilterTpid=1 eInnerVlanFilterVlanType=0 eRemoveTagAction=1 bReassignBridgePortEnable=1 nNewBridgePortId=$BP_L3; 
fapi-GSW-ExtendedVlanSet nExtendedVlanBlockId=0x60 nEntryIndex=14 eOuterVlanFilterVlanType=0 bOuterVlanFilterVidEnable=1 nOuterVlanFilterVidVal=1004 eOuterVlanFilterTpid=1 eInnerVlanFilterVlanType=0 eRemoveTagAction=1 bReassignBridgePortEnable=1 nNewBridgePortId=$BP_L4; 


echo "Add tag rules(switch->cpu)"

# idx_0 are for outer-untagged
fapi-GSW-ExtendedVlanSet nExtendedVlanBlockId=$EXV_L1 nEntryIndex=0 eOuterVlanFilterVlanType=3 eInnerVlanFilterVlanType=3 bOuterVlanActionEnable=1 eOuterVlanActionTpid=3 eOuterVlanActionVidMode=0 eOuterVlanActionVidVal=1001 eOuterVlanActionPriorityMode=2 eOuterVlanActioneDei=2; 
fapi-GSW-ExtendedVlanSet nExtendedVlanBlockId=$EXV_L2 nEntryIndex=0 eOuterVlanFilterVlanType=3 eInnerVlanFilterVlanType=3 bOuterVlanActionEnable=1 eOuterVlanActionTpid=3 eOuterVlanActionVidMode=0 eOuterVlanActionVidVal=1002 eOuterVlanActionPriorityMode=2 eOuterVlanActioneDei=2; 
fapi-GSW-ExtendedVlanSet nExtendedVlanBlockId=$EXV_L3 nEntryIndex=0 eOuterVlanFilterVlanType=3 eInnerVlanFilterVlanType=3 bOuterVlanActionEnable=1 eOuterVlanActionTpid=3 eOuterVlanActionVidMode=0 eOuterVlanActionVidVal=1003 eOuterVlanActionPriorityMode=2 eOuterVlanActioneDei=2; 
fapi-GSW-ExtendedVlanSet nExtendedVlanBlockId=$EXV_L4 nEntryIndex=0 eOuterVlanFilterVlanType=3 eInnerVlanFilterVlanType=3 bOuterVlanActionEnable=1 eOuterVlanActionTpid=3 eOuterVlanActionVidMode=0 eOuterVlanActionVidVal=1004 eOuterVlanActionPriorityMode=2 eOuterVlanActioneDei=2; 

# idx_1 are for outer-tagged
fapi-GSW-ExtendedVlanSet nExtendedVlanBlockId=$EXV_L1 nEntryIndex=1 eOuterVlanFilterVlanType=0 eInnerVlanFilterVlanType=3 bOuterVlanActionEnable=1 eOuterVlanActionTpid=3 eOuterVlanActionVidMode=0 eOuterVlanActionVidVal=1001 eOuterVlanActionPriorityMode=2 eOuterVlanActioneDei=2; 
fapi-GSW-ExtendedVlanSet nExtendedVlanBlockId=$EXV_L2 nEntryIndex=1 eOuterVlanFilterVlanType=0 eInnerVlanFilterVlanType=3 bOuterVlanActionEnable=1 eOuterVlanActionTpid=3 eOuterVlanActionVidMode=0 eOuterVlanActionVidVal=1002 eOuterVlanActionPriorityMode=2 eOuterVlanActioneDei=2; 
fapi-GSW-ExtendedVlanSet nExtendedVlanBlockId=$EXV_L3 nEntryIndex=1 eOuterVlanFilterVlanType=0 eInnerVlanFilterVlanType=3 bOuterVlanActionEnable=1 eOuterVlanActionTpid=3 eOuterVlanActionVidMode=0 eOuterVlanActionVidVal=1003 eOuterVlanActionPriorityMode=2 eOuterVlanActioneDei=2; 
fapi-GSW-ExtendedVlanSet nExtendedVlanBlockId=$EXV_L4 nEntryIndex=1 eOuterVlanFilterVlanType=0 eInnerVlanFilterVlanType=3 bOuterVlanActionEnable=1 eOuterVlanActionTpid=3 eOuterVlanActionVidMode=0 eOuterVlanActionVidVal=1004 eOuterVlanActionPriorityMode=2 eOuterVlanActioneDei=2; 


echo "Bind port-fwd map"

IDX1_L1=$(printf "0x%x" $((1 << ($BP_L1 - 16))))
IDX1_L2=$(printf "0x%x" $((1 << ($BP_L2 - 16))))
IDX1_L3=$(printf "0x%x" $((1 << ($BP_L3 - 16))))
IDX1_L4=$(printf "0x%x" $((1 << ($BP_L4 - 16))))

# LAN1
fapi-GSW-BridgePortConfigSet nBridgePortId=$BP_L1 nBridgeId=$BR_L1 nDestLogicalPortId=13 bBridgePortMapEnable=1 nBridgePortMapIndex[0]=0x4 nBridgePortMapIndex[1]=0x0 bEgressExtendedVlanEnable=1 nEgressExtendedVlanBlockId=$EXV_L1; 
fapi-GSW-BridgePortConfigSet nBridgePortId=2 nBridgeId=$BR_L1 bBridgePortMapEnable=1 nBridgePortMapIndex[0]=0x0 nBridgePortMapIndex[1]=$IDX1_L1; 

# LAN2
fapi-GSW-BridgePortConfigSet nBridgePortId=$BP_L2 nBridgeId=$BR_L2 nDestLogicalPortId=13 bBridgePortMapEnable=1 nBridgePortMapIndex[0]=0x8 nBridgePortMapIndex[1]=0x0 bEgressExtendedVlanEnable=1 nEgressExtendedVlanBlockId=$EXV_L2; 
fapi-GSW-BridgePortConfigSet nBridgePortId=3 nBridgeId=$BR_L2 bBridgePortMapEnable=1 nBridgePortMapIndex[0]=0x0 nBridgePortMapIndex[1]=$IDX1_L2; 

# LAN3
fapi-GSW-BridgePortConfigSet nBridgePortId=$BP_L3 nBridgeId=$BR_L3 nDestLogicalPortId=13 bBridgePortMapEnable=1 nBridgePortMapIndex[0]=0x10 nBridgePortMapIndex[1]=0x0 bEgressExtendedVlanEnable=1 nEgressExtendedVlanBlockId=$EXV_L3; 
fapi-GSW-BridgePortConfigSet nBridgePortId=4 nBridgeId=$BR_L3 bBridgePortMapEnable=1 nBridgePortMapIndex[0]=0x0 nBridgePortMapIndex[1]=$IDX1_L3; 

# LAN4
fapi-GSW-BridgePortConfigSet nBridgePortId=$BP_L4 nBridgeId=$BR_L4 nDestLogicalPortId=13 bBridgePortMapEnable=1 nBridgePortMapIndex[0]=0x20 nBridgePortMapIndex[1]=0x0 bEgressExtendedVlanEnable=1 nEgressExtendedVlanBlockId=$EXV_L4; 
fapi-GSW-BridgePortConfigSet nBridgePortId=5 nBridgeId=$BR_L4 bBridgePortMapEnable=1 nBridgePortMapIndex[0]=0x0 nBridgePortMapIndex[1]=$IDX1_L4; 

echo "Mxl ext-vlan config done！"
