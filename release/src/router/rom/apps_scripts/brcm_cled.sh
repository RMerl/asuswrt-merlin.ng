#!/bin/sh

CONFIGADDRESS="\
0xFF803020 0xFF803024 0xFF803028 0xFF80302C \
0xFF803030 0xFF803034 0xFF803038 0xFF80303C \
0xFF803040 0xFF803044 0xFF803048 0xFF80304C \
0xFF803050 0xFF803054 0xFF803058 0xFF80305C \
0xFF803060 0xFF803064 0xFF803068 0xFF80306C \
0xFF803070 0xFF803074 0xFF803078 0xFF80307C \
0xFF803080 0xFF803084 0xFF803088 0xFF80308C \
0xFF803090 0xFF803094 0xFF803098 0xFF80309C \
0xFF8030A0 0xFF8030A4 0xFF8030A8 0xFF8030AC \
0xFF8030B0 0xFF8030B4 0xFF8030B8 0xFF8030BC \
0xFF8030C0 0xFF8030C4 0xFF8030C8 0xFF8030CC \
0xFF8030D0 0xFF8030D4 0xFF8030D8 0xFF8030DC \
0xFF8030E0 0xFF8030E4 0xFF8030E8 0xFF8030EC \
0xFF8030F0 0xFF8030F4 0xFF8030F8 0xFF8030FC \
0xFF803100 0xFF803104 0xFF803108 0xFF80310C \
0xFF803110 0xFF803114 0xFF803118 0xFF80311C \
0xFF803120 0xFF803124 0xFF803128 0xFF80312C \
0xFF803130 0xFF803134 0xFF803138 0xFF80313C \
0xFF803140 0xFF803144 0xFF803148 0xFF80314C \
0xFF803150 0xFF803154 0xFF803158 0xFF80315C \
0xFF803160 0xFF803164 0xFF803168 0xFF80316C \
0xFF803170 0xFF803174 0xFF803178 0xFF80317C \
0xFF803180 0xFF803184 0xFF803188 0xFF80318C \
0xFF803190 0xFF803194 0xFF803198 0xFF80319C \
0xFF8031A0 0xFF8031A4 0xFF8031A8 0xFF8031AC \
0xFF8031B0 0xFF8031B4 0xFF8031B8 0xFF8031BC \
0xFF8031C0 0xFF8031C4 0xFF8031C8 0xFF8031CC \
0xFF8031D0 0xFF8031D4 0xFF8031D8 0xFF8031DC \
0xFF8031E0 0xFF8031E4 0xFF8031E8 0xFF8031EC \
0xFF8031F0 0xFF8031F4 0xFF8031F8 0xFF8031FC \
0xFF803200 0xFF803204 0xFF803208 0xFF80320C \
0xFF803210 0xFF803214 0xFF803218 0xFF80321C"


# Set CLED 0~31 config 0~3 
SetConfig(){
	LED=$((${MODE}))
	# CONFIGBANK=$((${ACTION}+1))
	CONFIGBANK=$((${ACTION}))
	CONFIGVALUE=$VALUE
	TMPINDEX=0
	ADDR=""
	
	# Show Get or set CLDE number and config bank
	if [ "x$CONFIGVALUE" == "x" ]; then
		echo "Get LED: ${LED} Config Bank: ${CONFIGBANK}"
	else
		echo "Set LED: ${LED} Config Bank: ${CONFIGBANK} Value: ${CONFIGVALUE}"
	fi
	
	# Get the index
	INDEX=$((${LED}*4))
	INDEX=$((${INDEX}+${CONFIGBANK}))
	
	# echo "index: ${INDEX}"
	
	# Shell not support array index so check the each element
	for var in $CONFIGADDRESS
	do
		if [ "$INDEX" -eq "$TMPINDEX" ]; then
			# echo $var
			ADDR=$var
			break
		# else
			# echo "${TMPINDEX}"
		fi
		TMPINDEX=`expr $TMPINDEX + 1`
	done

	# Split get register value or set register
	if [ "x$CONFIGVALUE" == "x" ]; then
		echo "dw ${ADDR}"
		dw ${ADDR}
	else
		echo "sw ${ADDR} ${CONFIGVALUE}"
		sw ${ADDR} ${CONFIGVALUE}
	fi
}

HardwareLed(){
	if [ "x$ACTIVE_VALUE" == "x" ]; then
		echo "dw 0xFF803004"
		dw 0xFF803004
	else
		echo "sw 0xFF803004 ${ACTIVE_VALUE}"
		sw 0xFF803004 $ACTIVE_VALUE
	fi
}
SoftLEDSetLed(){
	if [ "x$ACTIVE_VALUE" == "x" ]; then
		echo "dw 0xFF803010"
		dw 0xFF803010
	else
		echo "sw 0xFF803010 ${ACTIVE_VALUE}"
		sw 0xFF803010 $ACTIVE_VALUE
	fi
}
InputPolarityLed(){
	if [ "x$ACTIVE_VALUE" == "x" ]; then
		echo "dw 0xFF803014"
		dw 0xFF803014
	else
		echo "sw 0xFF803014 ${ACTIVE_VALUE}"
		sw 0xFF803014 $ACTIVE_VALUE
	fi
}
ActiveLed(){
	if [ "x$ACTIVE_VALUE" == "x" ]; then
		echo "dw 0xFF80301c"
		dw 0xFF80301c
	else
		echo "sw 0xFF80301c ${ACTIVE_VALUE}"
		sw 0xFF80301c $ACTIVE_VALUE
	fi
}


ErrorPrint(){
	echo Please try:
	echo "${0} [Action] [Value]"
	echo "${0} [CLED number(0-31)] [Config Bank(0-3)] [Value]"

	echo "Action:"
	echo "	hw"
	echo "	sw"
	echo "	ppol"
	echo "	activate"
	
	echo "Example:"
	echo "${0} hw 0x00000000"
	echo "${0} activate 0x07c00688"
	echo "${0} 0 0 0x0002c002  --> CLED0 config0 set as 0x0002c002"
	echo "${0} 1 3 0x00320064  --> CLED1 config3 set as 0x00320064"
	
}

MODE=$1


if [ "x${MODE}" == "xactivate" ]; then
	ACTIVE_VALUE=$2
	ActiveLed
elif [ "x${MODE}" == "xppol" ]; then
	ACTIVE_VALUE=$2
	InputPolarityLed
elif [ "x${MODE}" == "xhw" ]; then
	ACTIVE_VALUE=$2
	HardwareLed
elif [ "x${MODE}" == "xsw" ]; then
	ACTIVE_VALUE=$2
	SoftLEDSetLed
elif [ "x${MODE}" == "x" ]; then
	ErrorPrint	
elif [ "$MODE" -ge  0 ] && [ "$MODE" -lt  32 ]; then
	ACTION=$2
	VALUE=$3
	if [ "$ACTION" -ge  0 ] && [ "$ACTION" -lt  4 ]; then
		SetConfig
	else
		ErrorPrint
	fi
else
	ErrorPrint

fi
