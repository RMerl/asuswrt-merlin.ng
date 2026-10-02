#!/bin/sh

function what_type_phy() # 0:No PHY; 1:GPHY; 2:CL45 PHY
{
    local addr=$1;
    local ret=0;

    ethctl phy ext $addr 0x7ffe0 2>/dev/null 1>/dev/null
    if [ $? == 0 ]
        then ret=2; 
    else
        ethctl phy ext $addr 0 2>/dev/null 1>/dev/null
        if [ $? == 0 ]; then ret=1; fi
    fi
    echo $ret;
}

function get_reg_on_phy_type()
{
    local addr=$1
    local reg=$2
    local what_phy=$(what_type_phy $addr)

    case $what_phy in
        "1") ;;
        "2") reg=$((reg+0x7ffe0));;
        *) echo "No PHY at address $addr" 1>&2; return 1;;
    esac
    echo $reg
    return 0
}

function phy_shadow18_read() # phy_shadow18_read <phy addr> <shadow>
{
    if [ $# -lt 2 ]
        then echo "phy_shadow18_read <phy addr> <shadow>"; return 1; 
    fi

    local addr=$1
    local shadow=$2
    local reg18

    reg18=$(get_reg_on_phy_type $addr 0x18)
    if [ $? != 0 ]; then return; fi
    ethctl phy ext $addr $reg18 $(((shadow<<12)|7)) > /dev/null
    ethctl phy ext $addr $reg18 |awk -v s=$shadow 'NF {printf "Shadow18 at shadow %s has value 0x%04x\n", s, $5}'
}

function phy_shadow18_write() # phy_shadow18_write <phy addr> <shadow> <val with b15_to_b3, b2-b0 ignored>"
{
    if [ $# -lt 3 ]
        then echo "phy_shadow18_write <phy addr> <shadow> <val with b15_to_b3, b2-b0 ignored>"; return 1
    fi

    local addr=$1
    local shadow=$2
    local val=$3
    local reg18

    reg18=$(get_reg_on_phy_type $addr 0x18)
    if [ $? != 0 ]; then return; fi

    if [ $shadow == 7 ]; then val=$((0x8000|val)); fi
    val=$((val|shadow))
    phy_shadow18_read $addr $shadow
    ethctl phy ext $addr $reg18 $val > /dev/null
    echo Write Done! Read back:
    phy_shadow18_read $addr $shadow
}

function phy_shadow18() # phy_shadow18 <phy addr> <shadow> [<value>]
{
    if [ $# -lt 2 ]
        then echo "phy_shadow18 <phy addr> <shadow> [<value>]"; return 1
    fi

    if [ $# -lt 3 ]
        then phy_shadow18_read $1 $2
    else
        phy_shadow18_write $1 $2 $3
    fi
}

function phy_cmdhandler_read() # phy_cmdhandler_read <phy addr> <command>"
{
    if [ $# -lt 2 ]
        then echo "phy_cmdhandler_read <phy addr> <command>"; return 1
    fi
    local addr=$1
    local cmd=$2
    ethctl phy ext $addr 0x1e4005 $cmd > /dev/null
    echo $cmd|awk '{printf "command = 0x%04x\n", $1}'
    ethctl phy ext $addr 0x1e4038|awk 'NF {printf "data1 = 0x%04x\n", $5}'
    ethctl phy ext $addr 0x1e4039|awk 'NF {printf "data2 = 0x%04x\n", $5}'
    ethctl phy ext $addr 0x1e403a|awk 'NF {printf "data3 = 0x%04x\n", $5}'
    ethctl phy ext $addr 0x1e403b|awk 'NF {printf "data4 = 0x%04x\n", $5}'
    ethctl phy ext $addr 0x1e403c|awk 'NF {printf "data5 = 0x%04x\n", $5}'
    echo 
}

function phy_cmdhandler_write() # phy_cmdhandler_read <phy addr> <command> <data1> <data2> ..."
{
    if [ $# -lt 3 ]
    then echo "phy_cmdhandler_read <phy addr> <command> <data1> <data2> ..."; return 1
    fi
    local addr=$1
    local cmd=$2
    local data1=$3
    local data2=0
    local data3=0
    local data4=0
    local data5=0
    if [ -n $4 ]; then data2=$4; fi
    if [ -n $5 ]; then data3=$5; fi
    if [ -n $6 ]; then data4=$6; fi
    if [ -n $7 ]; then data5=$7; fi
    ethctl phy ext $addr 0x1e4038 $data1 > /dev/null
    if [ -n $4 ]; then ethctl phy ext $addr 0x1e4039 $data2 > /dev/null; fi
    if [ -n $5 ]; then ethctl phy ext $addr 0x1e403a $data3 > /dev/null; fi
    if [ -n $6 ]; then ethctl phy ext $addr 0x1e403b $data4 > /dev/null; fi
    if [ -n $7 ]; then ethctl phy ext $addr 0x1e403c $data5 > /dev/null; fi
    ethctl phy ext $addr 0x1e4005 $cmd > /dev/null
    echo "Done"
}

function phy_cmdhandler() # phy_cmdhandler <phy addr> <command> [<data1> <data2> ...]"
{
    if [ $# -lt 2 ]
        then echo "phy_cmdhandler_read <phy addr> <command> [<data1> <data2> ...]"; return 1
    elif [ $# -lt 3 ]
        then phy_cmdhandler_read $1 $2
    else
        phy_cmdhandler_write $1 $2 $3 $4 $5 $6 $7
    fi
}

function phy_shadow1c_read() # phy_shadow1c_read <addr> <shadow>
{ 
    if [ $# -lt 2 ]
        then echo "phy_shadow1c_read <addr> <shadow>"; return 1
    fi
    local addr=$1
    local shadow=$2
    local reg1c

    reg1c=$(get_reg_on_phy_type $addr 0x1c)
    if [ $? != 0 ]; then return; fi;

    ethctl phy ext $addr $reg1c $((shadow<<10)) > /dev/null
    ethctl phy ext $addr $reg1c | awk -v s=$shadow 'NF {printf "Shadow1c at shadow %s has value 0x%04x\n", s, $5}'
}
 
function phy_shadow1c_write() # phy_shadow1c_write <addr> <shadow> <val_b9_to_b0>
{ 
    if [ $# -lt 3 ]
    then echo "phy_shadow1c_write <addr> <shadow> <val_b9_to_b0>"; return 1
    fi
    local addr=$1
    local shadow=$2
    local val=$3
    local reg1c

    reg1c=$(get_reg_on_phy_type $addr 0x1c)
    if [ $? != 0 ]; then return; fi

    echo Read First:
    phy_shadow1c_read $addr $shadow
    val=$((val|(shadow<<10)|0x8000))
    ethctl phy ext $addr $reg1c $val > /dev/null
    echo Write Done! Read back:
    phy_shadow1c_read $addr $shadow
}

function phy_shadow1c() # phy_shadow1c <addr> <shadow> [<val_b9_to_b0>]
{
    if [ $# -lt 2 ]
        then echo "phy_shadow1c <addr> <shadow> [<val_b9_to_b0>]"
    fi
    if [ $# -lt 3 ]
        then phy_shadow1c_read $1 $2
    else
        phy_shadow1c_write $1 $2 $3
    fi
}

function phy_xfi() # phy_xfi <addr> <reg> <val|reg2> <-d>
{
    local addr=$1
    local reg=$2
    local val=$3
    local reg2=$3
    local dump=$4
    local rg
    local vl
    local i

    ethctl phy ext $addr 0x1e4110 0x2004 > /dev/null
    ethctl phy ext $addr 0x1e4111 0x2004 > /dev/null
    ethctl phy ext $addr 0x1e4113 0x2004 > /dev/null

    val=$((val+0))
    reg=$((reg+0))
    reg2=$((reg2+0))
    if [ "$dump" == "-d" ]; then
        # for (( rg=${reg}; rg<${reg2}; rg++ ))
        rg=$reg
        i=0
        while true
        do
            vl=$(ethctl phy ext $addr $rg|cut -d ' ' -f 5)
            if [ $((i%8)) -eq 0 ]; then echo ""; echo $rg|awk '{printf "%05x: ", $0}'; fi
            echo -n " $vl"
            rg=$((rg+1))
            if [ "$rg" -ge "$reg2" ]; then break; fi
            i=$((i+1))
        done
    elif [ $# -eq 3 ]; then
        ethctl phy ext $addr $reg $val
    else
        ethctl phy ext $addr $reg|awk 'NF {printf "val = 0x%04x\n", $5}'
    fi
    echo ""

    ethctl phy ext $addr 0x1e4110 0x0001 > /dev/null
    ethctl phy ext $addr 0x1e4111 0x0001 > /dev/null
    ethctl phy ext $addr 0x1e4113 0x1002 > /dev/null
}

function phy_local_loopback() # phy_local_loopback <addr> <ethX> <xfi_mode> [-r for restore]
{
    local addr=$1
    local ethx=$2
    local xfi=$3
    local restore=$3

    if [ $restore == "-r" ]; then
        ethctl phy ext $addr 0x1d0c1 0x8
        ethctl phy ext $addr 0x1d0d2 0xe
        ethctl phy ext $addr 0x39109 0x0
        ethctl phy serdespower $addr 1
        ethctl $ethx media-type auto
        return 0
    fi

    ethctl phy serdespower $addr 0
    ethctl $ethx media-type $xfi
    ethctl phy ext $addr 0x1d0c1 0x188
    if [[ $3 == "10G"* ]]; then
        ethctl phy ext $addr 0x1d0d2 0xf
    else
        ethctl phy ext $addr 0x39109 0x10
    fi
}

function phy_remote_loopback() # phy_remote_loopback <addr> <ethX> <xfi_mode> [-r for restore]
{
    local addr=$1
    local ethx=$2
    local xfi=$3
    local restore=$3

    if [ $restore == "-r" ]; then
        ethctl phy ext $addr 0x1d075 4
        ethctl phy ext $addr 0x1d070 0
        ethctl phy ext $addr 0x1d0e2 2
        ethctl phy serdespower $addr 1
        ethctl $ethx media-type auto
        return 0
    fi

    ethctl phy serdespower $addr 0
    ethctl $ethx media-type $xfi
    ethctl phy ext $addr 0x1d075 0
    ethctl phy ext $addr 0x1d070 1
    ethctl phy ext $addr 0x1d070 3
    ethctl phy ext $addr 0x1d070 7
    ethctl phy ext $addr 0x1d0e2 3
}

function phy_scripts_help()
{
    echo Available Functions:
    echo "phy_shadow18() # phy_shadow18 <phy addr> <shadow> [<val with b15_to_b3, b2-b0 ignored>]"
    echo "phy_shadow1c() # phy_shadow1c <addr> <shadow> [<val_b9_to_b0>]"
    echo "phy_cmdhandler() # phy_cmdhandler <phy addr> <command> [<data1> <data2> ...]"
    echo "phy_xfi() # phy_xfi <addr> <reg> <val|reg2> <-d>"
    echo "phy_local_loopback() # phy_local_loopback <addr> <ethX> <xfi_mode> [-r for restore]"
    echo "phy_local_loopback() # phy_local_loopback <addr> <ethX> -r(for restore)"
    echo "phy_remote_loopback() # phy_remote_loopback <addr> <ethX> <xfi_mode>"
    echo "phy_remote_loopback() # phy_remote_loopback <addr> <ethX> -r(for restore)"
    echo "                        <xfi_mode> can be 1000Base-X, 2500Base-X, 10GBase-R etc."
}

phy_scripts_help


