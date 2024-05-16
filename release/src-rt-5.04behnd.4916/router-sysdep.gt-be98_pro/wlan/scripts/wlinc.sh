#!/bin/bash
# This script configures interrupt & thread affinity regarding to WLAN.
#

MAX_WLAN_ADAPTER=4
NUM_PROCESSOR=`cat /proc/cpuinfo | grep -c "processor"`
NUM_WL_INTF=`nvram get wl_ifnames | wc -w`

WLDRV_PROC_LIST=" dhd wl wl-scheduler wl-avs "
WLDRV_INTR_LIST=" d11 "
PROC_LIST_COMMON=" bcmsw_rx skb_free_task "

CROSSBOW=`gunzip < /proc/config.gz |grep "CONFIG_BCM_CROSSBOW=y"`
ARCHER=`cat /proc/net/dev |grep archer |cut -d ':' -f1`
CROSSBOWFOL=`gunzip < /proc/config.gz |grep "CONFIG_BCM_CROSSBOW_FULL_OFFLOAD=y"`
if [ ! -z $CROSSBOW ] && [ -z $CROSSBOWFOL ]; then
    # Crossbow platform without hardware offload (6764L),
    # make it Archer data path with crossbow WiFi
    PLTDRV_PROC_LIST=" awl_xfer archer_dhd "
    WLDRV_INTR_LIST=" ${WLDRV_INTR_LIST} m2m0 m2m1 mlc phy "
    PROC_LIST_COMMON=" ${PROC_LIST_COMMON} archer_wlan recycle "
    ARCHER="archer"
elif [ ! -z $ARCHER ]; then
    # Archer platforms
    PLTDRV_PROC_LIST=" awl_xfer archer_dhd "
    WLDRV_INTR_LIST=" ${WLDRV_INTR_LIST} m2m "
    PROC_LIST_COMMON=" ${PROC_LIST_COMMON} archer_wlan recycle "

    # wl_allow_cpu0_affinity
    # Used during auto configuration using defaults, to allow cpu0 for WLAN process affinity
    # 0: defaulut value (don't allow cpu0 in affinity and keep it free)
    ALLOW_CPU0_AFF=`nvram kget wl_allow_cpu0_affinity`
elif [ ! -z $CROSSBOW ]; then
    # CrossBow platforms
    PLTDRV_PROC_CPU_LIST=" xbow_fwq archer_dhd "
    PLTDRV_PROC_COMMON_LIST=" xbow_stq "
    PLTDRV_PROC_LIST=${PLTDRV_PROC_CPU_LIST}${PLTDRV_PROC_COMMON_LIST}
    WLDRV_INTR_LIST=" ${WLDRV_INTR_LIST} m2m0 m2m1 mlc phy "
    PLTDRV_INTR_LIST=" xbow_socket "
    PROC_LIST_COMMON=" ${PROC_LIST_COMMON} bcm_archer_us recycle "
else
    #non-Archer platforms
    WLDRV_PROC_LIST=" wfd ${WLDRV_PROC_LIST} "
fi

PROC_LIST_PER_RADIO=${WLDRV_PROC_LIST}${PLTDRV_PROC_LIST}
INTR_LIST_PER_RADIO=${WLDRV_INTR_LIST}${PLTDRV_INTR_LIST}

if [ -z $ALLOW_CPU0_AFF ]; then
    ALLOW_CPU0_AFF=1
fi

# is radio interface DHD ?
# $1 radio interface number
isDHDRadio()
{
    local dhd_intr is_dhd

    dhd_intr=`cat /proc/interrupts |grep wl$1 |grep dhdpcie | cut -d ':' -f1`

    if [ ! -z "$dhd_intr" ]; then
        is_dhd=1
    else
        is_dhd=0
    fi

    echo $is_dhd
}

#$(getStrByPos "12 33 44" " " 2) will return 33
getStrByPos() {
        local num=$3;
        IFS="$2"
        set -- $1
        eval result='$'$num;
        echo $result;
}

# $1 item
# $2 alignment
# $3 maxlen
align_string()
{
    local item=$1
    local len=${#item}
    local padding="                                                                                                                        "
    local maxlen=20

    if [ -z $2 ]; then
        justify=0
    else
        justify=$2
    fi

    if [ -z $3 ]; then
        maxlen=20
    else
        maxlen=$3
    fi

    if [ $justify = 0 ]; then
        # Center Justified
        item=`printf '%*.*s%s%*.*s' 0 $(((maxlen-2-len)/2)) "$padding" "$item"  0 $(((maxlen-1-len)/2)) "$padding"`
    elif [ $justify -lt 0 ]; then
        # Left Justified
        item=`printf ' %s%*.*s' "$item"  0 $((maxlen-2-len)) "$padding"`
    else
        # Right Justified
        item=`printf '%*.*s%s ' 0 $((maxlen-2-len)) "$padding" "$item"`
    fi

    echo "${item}"
}

getPidByName() 
{
    local pid=$(getStrByPos "`ps | grep -v grep | grep $1`" " " 1);
    if [ -z $pid ]; then 
        echo 0;
    else
        echo $pid;
    fi
}

#trim leading and trailing space
trim() {
    if [ ! -z $* ]; then
        local var="$*"
        var="${var#"${var%%[![:space:]]*}"}"
        var="${var%"${var##*[![:space:]]}"}"   
        echo -n "$var"
    fi
}

#change bitmask number to string mark of bit postion string
getBitmaskStr() {
    local affinity=$((0x$1));
    local cpu cpu_mask cpus="";
    local cpu=0;
    if [ ! -z $affinity ]; then
        while [ ${cpu} -lt ${NUM_PROCESSOR} ];
        do
            cpu_mask=$((1 << $cpu)) 
            if [ $((affinity&cpu_mask)) -gt 0 ]; then
                if [ "$cpus" = "" ]; then
                    cpus=$cpu
                else
                    cpus=$cpus"-$cpu"
                fi
            fi
            cpu=$((cpu+1));
        done
    fi
    echo $cpus;
}

#
# Get WLAN Interrupt nvram setting name for the given interrupt type and radio number
# $1: interupt type
# $2: radio
#
get_interrupt_nv_name()
{
    local radio=$2;

    case "$1" in 
        d11)
            echo "wl${radio}_interrupt_affinity"
            ;;
        xbow_socket)
            echo "xbow${radio}_interrupt_affinity"
            ;;
        *)
            echo "wl${radio}_${1}_interrupt_affinity"
            ;;
    esac
}

#
# Get WLAN Interrupt name for the given interrupt type and radio number
# $1: interupt type
# $2: radio
#
get_interrupt_name()
{
    local radio=$2;

    case "$1" in 
        d11)
            echo "wl${radio}"
            ;;
        m2m)
            echo "wlan_${radio}_m2m"
            ;;
        m2m0)
            echo "wlan_${radio}_m2m0"
            ;;
        m2m1)
            echo "wlan_${radio}_m2m1"
            ;;
        mlc)
            echo "wlan_${radio}_mlc"
            ;;
        phy)
            echo "wlan_${radio}_phy"
            ;;
        xbow_socket)
            echo "crossbow_socket[${radio}]"
            ;;
    esac
}

get_process_name() 
{
    local radio=$2;
    case "$1" in 
        wl)
            echo "wl${radio}-kthrd"
            ;;
        wl-scheduler)
            echo "wl${radio}-scheduler"
            ;;
        wl-avs)
            echo "wl${radio}-avs"
            ;;
        dhd)
            echo "dhd${radio}_dpc";
            ;;
        wfd)
            echo "wfd${radio}-thrd";
            ;;
        awl_xfer)
            echo "bcm_awl_xfer_${radio}";
            ;;
        archer_dhd)
            echo "archer_dhd_${radio}";
            ;;
        archer_wlan)
            echo "bcm_archer_wlan";
            ;;
        recycle)
            echo "bcmsw_recycle";
            ;;
        events)
            echo "events/${radio}";
            ;;
        bcmsw_rx)
            echo "bcmsw_rx";
            ;;
        skb_free_task)
            echo "skb_free_task";
            ;;
        xbow_fwq)
            echo "xbow_wlan_${radio}_fwq";
            ;;
        xbow_stq)
            echo "xbow_wlan_${radio}_stq";
            ;;
        bcm_archer_us)
            echo "bcm_archer_us";
            ;;
    esac
}

#
# Get WLAN interrupt number for the given interrupt name
#  $1 - WLAN Interrupt Name
#  $2 - Handler name:  dhdpcie/wlpcie (Optional)
#
getIntrNum()
{
  local int_name=$1
  local hndler

  if [ -z "$2" ]; then
    hndler="wl"
  else
    hndler=$2
  fi

  if [[ "$int_name" == "crossbow"* ]]; then
      local interrupt_line=`cat /proc/interrupts |grep -F "$int_name"`
  else
      local interrupt_line=`cat /proc/interrupts |grep $int_name |grep $hndler`
  fi

  # Check if interrupt is PCIe MSI interrupt
  # contains "msi_pcie:"X string, where [X] is msi core
  local msi_interrupt_line=`echo $interrupt_line | grep "msi_pcie:" | sed -r -e 's/[ ]+/ /g'`
  if [ ! -z "$msi_interrupt_line" ]; then
    # get the msi core
    msi_core=$(getStrByPos "$(getStrByPos "${msi_interrupt_line}" ":" 3)" " " 1)
    # get the parent msi core interrupt
    local interrupt_line=`cat /proc/interrupts |grep "msi_pcie:$msi_core"`
  fi

  if [ ! -z "$interrupt_line" ]; then
      interrupt=$(getStrByPos "${interrupt_line}" ":" 1)
      echo $(trim $interrupt);
  fi
}

#
# Get interrupt affinity for the given WLAN interrupt
#
# $1 - WLAN Interrupt name
# $2 - handler name:  dhdpcie/wlpcie (Optional)
#
getIntrAffinity()
{
    local interrupt=$(getIntrNum $1 $2);

    if [ ! -z $interrupt ]; then
        local affinity="/proc/irq/$interrupt/smp_affinity";
        echo `cat $affinity 2>/dev/NULL`
    fi
}

#
# Set WLAN interrupt affinity for the given interrupt name
#  $1 - WLAN Interrupt Name
#  $2 - Interrupt Affinity
#
setIntrAffinity()
{
    local int_name=$1;
    local affinity=$2;
    local irq=$(getIntrNum $int_name);

    if [ ! -z $affinity ] && [ ! -z $irq ]; then
        echo $affinity >/proc/irq/$irq/smp_affinity 2>/dev/NULL
        echo set $int_name irq:$irq aff:$affinity
        echo ""
        #nvram kset wlan_${radio}_${int_name}_interrupt_affinity=${affinity}
    fi
}

getPidAffinityNum() {
    echo $(getStrByPos "`taskset -p $1`" " " 6);
}

getPidAffinity() {
    local pid=$1;
    echo $(getBitmaskStr $(getPidAffinityNum $pid));
}

getPidPriority() {
    echo $(getStrByPos "`chrt -p $1 | grep priority`" " " 6)
}

setProcPriority() {
    local nvram_name=$1
    local proc_name=$2
    local pid=$3
    local prio=$4
    if [ -f "/bin/rtpolicy" ]; then
        echo "set $proc_name and pid:$pid to prio:$prio"
        echo ""
        chrt -p $prio $pid> /dev/null 2>&1
        if [ "$?" != "0" ]; then
            echo "Failed to set $proc_name to realtime priority $prio"
        fi
        #nvram kset $nvram_name=$prio;
    else
        echo "no rtpolicy to set $proc_name and pid:$pid to prio:$prio"
    fi
}

setProcAffinity() {
    local nvram_name=$1
    local proc_name=$2
    local pid=$3
    local affinity=$4
    echo "set $proc_name $pid to aff:$affinity"
    echo ""
    if [ ! -f "/bin/rtpolicy" ]; then
       echo "no affinity when no RT policy set"
       return;
    fi
    taskset -p $affinity $pid> /dev/null 2>&1
    if [ "$?" != "0" ]; then
        echo "pin $proc_name failed, rc=$?, pid_wl0=$pid"
    fi
    #nvram kset $nvram_name=$affinity
}

# show separator line
list_seperator()
{
    local title="----------------";
    local radio=0;

    while [ $radio -lt $NUM_WL_INTF ];
    do
        title="$title---------------------"
        radio=$((radio+1))
    done

    echo "${title}"
}

list_process() 
{
    local radio=0;
    local title="$1 |";
    local affinity="";
    local priority=0;

    title="$(align_string "$1" -1 15) |"
    while [ $radio -lt $NUM_WL_INTF ];
    do
        if [ $1 == "Process" ];  then
            title="$title $(align_string "wl$radio(PID:CPUS:PRIO)") |"
        else
            proc_name=$(get_process_name $1 $radio)
            pid_wfd=$(getPidByName ${proc_name})
            if [ $pid_wfd -gt 0 ]; then
                affinity=$(getPidAffinity $pid_wfd)
                priority=$(getPidPriority $pid_wfd)
                title="$title $(align_string "$pid_wfd:$affinity:$priority") |"
            else
                title="$title $(align_string "-") |"
            fi
        fi
        radio=`expr $radio + 1`
    done
    echo "${title}"
}

list_interrupt() 
{
    local radio=0;
    local intr=$1
    local title
    local affinity="";
    local priority=0;

    title="$(align_string "$intr" -1 15) |"
    while [ $radio -lt $NUM_WL_INTF ];
    do
        if [ $1 == "Interrupts" ];  then
            title="$title $(align_string "wl$radio(IRQ:CPUS)") |"
        else
            intr_name=$(get_interrupt_name $intr $radio)
            interrupt=$(getIntrNum "$intr_name")
            if [ ! -z $interrupt ]; then
                interrpt=""$interrupt
                affinity="cpu:"$(getBitmaskStr $(getIntrAffinity $intr_name))
                title="$title $(align_string "$interrpt($affinity)") |"
            else
                 title="$title $(align_string "-") |"
            fi
        fi
        radio=`expr $radio + 1`
    done

    echo "${title}"
}

list_nvram_settings() {

    local affinity radio proc_name proc_pid prio name;
    for ctl_item in $PROC_LIST_PER_RADIO
    do
        radio=0;
        while [ $radio -lt $NUM_WL_INTF ];
        do
            proc_name=$(get_process_name $ctl_item $radio);
            proc_pid=$(getPidByName $proc_name);
            if [ $proc_pid -gt 0 ]; then
                name="wl${radio}_${ctl_item}_affinity"
                affinity=`nvram kget ${name}`;
                if [ ! -z ${affinity} ]; then
                    echo $name=$affinity;
                else
                    echo "$name is not defined, use default";
                fi
                name="wl${radio}_${ctl_item}_prio"
                prio=`nvram kget ${name}`;
                if [ ! -z ${prio} ]; then
                    echo $name=$prio;
                else
                    echo "$name is not defined, use default";
                fi
            fi
            radio=$((radio+1))
        done
    done

    #Common WiFi process affinity and priority
    for ctl_item in $PROC_LIST_COMMON
    do
        proc_name=$(get_process_name $ctl_item);
        proc_pid=$(getPidByName $proc_name);
        if [ $proc_pid -gt 0 ]; then
            name="wl_${ctl_item}_affinity"
            affinity=`nvram kget ${name}`;
            if [ ! -z ${affinity} ]; then
                echo $name=$affinity;
            else
                echo "$name is not defined, use default";
            fi
            name="wl_${ctl_item}_prio"
            prio=`nvram kget ${name}`;
            if [ ! -z ${prio} ]; then
                echo $name=$prio;
            else
                echo "$name is not defined, use default";
            fi
        fi
    done

    if [ ! -z $ARCHER ]; then
            name="wl_allow_cpu0_affinity"
            affinity=`nvram kget ${name}`;
            if [ ! -z ${affinity} ]; then
                echo $name=$affinity;
            else
                echo "$name is not defined, use default";
            fi
    fi

    # List WLAN interrupts NVRAM settings
    radio=0;
    while [ $radio -lt $NUM_WL_INTF ];
    do
        for ctl_item in $INTR_LIST_PER_RADIO
        do
            nv_name=$(get_interrupt_nv_name $ctl_item $radio);
            affinity=`nvram kget ${nv_name}`;
            if [ ! -z $affinity ]; then 
                echo "$nv_name=$affinity";
            else
                echo "$nv_name is not defined,use default";
            fi
        done
        radio=$((radio+1))
    done
}

list_wifi_process()
{
    echo ""
    echo "	      WiFi Processes on System (Total CPU:${NUM_PROCESSOR})	";
    list_seperator
    list_process "Process";

    list_seperator
    for ctl_item in $PROC_LIST_PER_RADIO
    do
        list_process $ctl_item
    done

    if [ ! -z "$PROC_LIST_COMMON" ]; then
        list_seperator
        for ctl_item in $PROC_LIST_COMMON
        do
            list_process $ctl_item
        done
    fi

    list_seperator
    echo ""
    echo "	      WiFi Interrupts on System (Total CPU:${NUM_PROCESSOR})	";
    list_seperator
    for ctl_item in $INTR_LIST_PER_RADIO
    do
        list_interrupt $ctl_item
    done

    list_seperator

    echo ""
}

### command set #####

pin_wifi_proc_advanced()
{
    local affinity radio proc_name proc_pid prio name;
    local max title hdlr_name setting

    radio=0;
    max=$((1<<${NUM_PROCESSOR}))
    max=$((max-1))

    echo 
    echo "----------------------------------------------------------------"
    echo "This menu allows you to set directly the required nvram settings"
    echo
    echo "affinity setting is a bitmap with each bit corresponding to a CPU"
    echo "----------------------------------------------------------------"
    echo 
    echo "-----------INFO-----------"
    echo "Num CPUs         :${NUM_PROCESSOR}" 
    echo "Affinity mask    :(1-$max)" 
    title="${title}Num Radios       :${NUM_WL_INTF} ("
    hdlr_name=dhdpcie
    while [ $radio -lt $NUM_WL_INTF ];
    do
        affinity=$(getIntrAffinity wl$radio $hdlr_name);
        if [ ! -z $affinity ]; then 
            title="${title}DHD/"
        else
            title="${title}NIC/"
        fi
        radio=$((radio+1))
    done
    title="${title})"
    echo "$title"
    echo "--------------------------"
    echo 

    radio=0;
    while [ $radio -lt $NUM_WL_INTF ];
    do
        echo "---------------------------------------------"
        echo "WLAN Radio#$radio (wl$radio) Process Settings"
        echo "---------------------------------------------"
        echo 
        for ctl_item in $PROC_LIST_PER_RADIO
        do
            proc_name=$(get_process_name $ctl_item $radio);
            proc_pid=$(getPidByName $proc_name);
            if [ $proc_pid -gt 0 ]; then
                name="wl${radio}_${ctl_item}_affinity"
                setting=$(getPidAffinityNum $proc_pid)
                read -p "\"$name\" mask setting to (1-$max)[$setting]: " affinity
                if [ -z $affinity ]; then
                    affinity=$setting
                fi
                setProcAffinity $name $proc_name $proc_pid $affinity

                name="wl${radio}_${ctl_item}_prio"
                setting=$(getPidPriority $proc_pid)
                read -p "\"$name\" setting to (1-99)[$setting]: " prio
                if [ -z $prio ]; then
                    prio=$setting
                fi
                setProcPriority $name $proc_name $proc_pid $prio
            fi
        done

        # WLAN interrupts NVRAM settings
        for ctl_item in $INTR_LIST_PER_RADIO
        do
            nv_name=$(get_interrupt_nv_name $ctl_item $radio)
            int_name=$(get_interrupt_name $ctl_item $radio)
            setting=$(getIntrAffinity $int_name)
            if [ ! -z $setting ]; then
                read -p "\"$nv_name\" mask setting to (1-$max)[$setting]: " affinity
                if [ -z $affinity ]; then
                    affinity=$setting
                fi
            fi
            setIntrAffinity $int_name $affinity;
        done

        radio=$((radio+1))
    done

    #Common WiFi process affinity and priority
    echo "---------------------------------------------"
    echo "WLAN Common Process Settings"
    echo "---------------------------------------------"
    echo 
    for ctl_item in $PROC_LIST_COMMON
    do
        proc_name=$(get_process_name $ctl_item);
        proc_pid=$(getPidByName $proc_name);
        if [ $proc_pid -gt 0 ]; then
            name="wl_${ctl_item}_affinity"
            setting=$(getPidAffinityNum $proc_pid)
            read -p "\"$name\" mask setting to (1-$max)[$setting]: " affinity
            if [ -z $affinity ]; then
                affinity=$setting
            fi
            setProcAffinity $name $proc_name $proc_pid $affinity

            name="wl_${ctl_item}_prio"
            setting=$(getPidPriority $proc_pid)
            read -p "\"$name\" setting to (1-99)[$setting]: " prio
            if [ -z $prio ]; then
                prio=$setting
            fi
            setProcPriority $name $proc_name $proc_pid $prio
        fi
    done
}

pin_wifi_processes() 
{
    local radio=0;
    local num_wl_toconf=$NUM_WL_INTF;
    local prompt proc_name prio_nvram_name affi_nvram_name;

    if [ ! -z $1 ]; then
        radio=$1;
        num_wl_toconf=$((radio+1))
        prompt="Config WiFi Radio $radio's Threads onto CPUs";
    else
        prompt="Config WiFi Radios Threads onto CPUs";

    fi
    echo ""
    echo "${prompt}(Total CPU:${NUM_PROCESSOR})" 
    echo "-------------------------------------------------------------------------";
    while [ $radio -lt $num_wl_toconf ];
    do
       for ctl_item in $PROC_LIST_PER_RADIO;
       do
           proc_name=$(get_process_name $ctl_item $radio);
           pid_wfd=$(getPidByName ${proc_name});
           if [ $pid_wfd -gt 0 ]; then
               local affinity=0
               local cpu=0;
               while [ ${cpu} -lt ${NUM_PROCESSOR} ];
               do
                   echo "Pin $ctl_item on radio $radio ($proc_name) onto CPU${cpu}(y/n):"
                   read answer
                   if [ $answer == "y" ]; then
                       affinity=$((affinity|(1<<cpu)))
                   fi
                   cpu=$((cpu+1))
               done

               echo "Assign $ctl_item on radio $radio real time priority[1-99]:"
               read priority;
               echo "Sure to change it?(y/n)"
               read answer;
               if [ $answer == "y" ]; then
                   prio_nvram_name="wl${radio}_${ctl_item}_prio";
                   setProcPriority $prio_nvram_name $proc_name $pid_wfd $priority
                   affi_nvram_name="wl${radio}_${ctl_item}_affinity"
                   setProcAffinity $affi_nvram_name $proc_name $pid_wfd $affinity
               fi
           fi
       done
       radio=$((radio+1))
    done

    #Common WiFi process affinity and priority
    for ctl_item in $PROC_LIST_COMMON;
    do
        proc_name=$(get_process_name $ctl_item);
        pid_wfd=$(getPidByName ${proc_name});
        if [ $pid_wfd -gt 0 ]; then
            local affinity=0
            local cpu=0;
            while [ ${cpu} -lt ${NUM_PROCESSOR} ];
            do
                echo "Pin $ctl_item ($proc_name) onto CPU${cpu}(y/n):"
                read answer
                if [ $answer == "y" ]; then
                    affinity=$((affinity|(1<<cpu)))
                fi
                cpu=$((cpu+1))
            done

            echo "Assign $ctl_item real time priority[1-99]:"
            read priority;
            echo "Sure to change it?(y/n)"
            read answer;
            if [ $answer == "y" ]; then
                prio_nvram_name="wl_${ctl_item}_prio";
                setProcPriority $prio_nvram_name $proc_name $pid_wfd $priority
                affi_nvram_name="wl_${ctl_item}_affinity"
                setProcAffinity $affi_nvram_name $proc_name $pid_wfd $affinity
            fi
        fi
    done
}

pin_wifi_interrupt()
{
    local radio=0;
    local m2m_radio=0;
    local num_wl_toconf=$NUM_WL_INTF;
    local prompt cpu affinity;
    if [ ! -z $1 ]; then
        radio=$1;
        m2m_radio=$1;
        num_wl_toconf=$((radio+1))
        prompt="Config WiFi Radio $radio's Interrupt onto CPUs";
    else
        prompt="Config WiFi Radios Interrupt onto CPUs";

    fi

    echo ""
    echo "${prompt}(Total CPU:${NUM_PROCESSOR})"
    echo "-------------------------------------------------------------------------";

    # WLAN interrupts configuration
    while [ $radio -lt $num_wl_toconf ];
    do
        for ctl_item in $INTR_LIST_PER_RADIO
        do
            affinity=0
            cpu=0;
            int_name=$(get_interrupt_name $ctl_item $radio);
            while [ ${cpu} -lt ${NUM_PROCESSOR} ];
            do
                echo "Assign ${int_name}'s interrupt(tasklet) onto CPU${cpu}(y/n):"
                read answer
                if [ $answer == "y" ]; then
                    affinity=$((affinity|(1<<cpu)))
                fi
                cpu=$((cpu+1))
            done

            echo "Sure to change it?(y/n)"
            read answer;
            if [ $answer == "y" ]; then
                setIntrAffinity $int_name $affinity;
            fi
        done
        radio=$((radio+1))
    done
}

save_wifi_pin_cfg() {
    local to_save=0;
    local affinity radio proc_name proc_pid prio name;

    radio=0;
    while [ $radio -lt $NUM_WL_INTF ];
    do
        for ctl_item in $PROC_LIST_PER_RADIO
        do
            proc_name=$(get_process_name $ctl_item $radio);
            proc_pid=$(getPidByName $proc_name);
            if [ $proc_pid -gt 0 ]; then
                to_save=1;
                affinity=$(getPidAffinityNum $proc_pid);
                prio=$(getPidPriority $proc_pid);
                name="wl${radio}_${ctl_item}_affinity"
                nvram kset $name=$affinity
                name="wl${radio}_${ctl_item}_prio"
                nvram kset $name=$prio
            fi
        done

        for ctl_item in $INTR_LIST_PER_RADIO
        do
            int_name=$(get_interrupt_name $ctl_item $radio);
            affinity=$(getIntrAffinity $int_name);
            if [ ! -z $affinity ]; then 
                to_save=1;
                nvram kset $int_name=$affinity;
            fi
        done
        radio=$((radio+1))
    done

    #Common WiFi process affinity and priority
    for ctl_item in $PROC_LIST_COMMON
    do
        proc_name=$(get_process_name $ctl_item);
        proc_pid=$(getPidByName $proc_name);
        if [ $proc_pid -gt 0 ]; then
            to_save=1;
            affinity=$(getPidAffinityNum $proc_pid);
            prio=$(getPidPriority $proc_pid);
            name="wl_${ctl_item}_affinity"
            nvram kset $name=$affinity
            name="wl_${ctl_item}_prio"
            nvram kset $name=$prio
        fi
    done

    if [ $to_save -gt 0 ]; then
        nvram kcommit
    else
        echo "!There is nothing to save actually!"
    fi
}

unset_wifi_pin_cfg() {
    local radio name ctl_item;

    radio=0;
    while [ $radio -lt $NUM_WL_INTF ];
    do
        for ctl_item in $PROC_LIST_PER_RADIO
        do
            name="wl${radio}_${ctl_item}_affinity"
            nvram kunset $name
            name="wl${radio}_${ctl_item}_prio"
            nvram kunset $name
        done

        for ctl_item in $INTR_LIST_PER_RADIO
        do
            int_name=$(get_interrupt_name $ctl_item $radio);
            nvram kunset $int_name
        done
        radio=$((radio+1))
    done

    #Common WiFi process affinity and priority
    for ctl_item in $PROC_LIST_COMMON
    do
        name="wl_${ctl_item}_affinity"
        nvram kunset $name
        name="wl_${ctl_item}_prio"
        nvram kunset $name
    done

    nvram kunset wlthread_pinned
    nvram kcommit
}

save_config_default() {

    local radio=0;
    local irq name prio affinity proc_name proc_pid;
    local KERNEL_NVRAM_FILE="/etc/wlan/kernel_nvram.setting";
    mount -t ubifs ubi:rootfs_ubifs / -o remount,rw

    radio=0;
    while [ $radio -lt $NUM_WL_INTF ];
    do
        for ctl_item in $PROC_LIST_PER_RADIO
        do
            proc_name=$(get_process_name $ctl_item $radio);
            proc_pid=$(getPidByName $proc_name);
            if [ $proc_pid -gt 0 ]; then
                name="wl${radio}_${ctl_item}_affinity"
                affinity=`nvram kget ${name}`
                if [ ! -z $affinity ]; then 
                    echo $name=$affinity >> ${KERNEL_NVRAM_FILE}
                fi
                name="wl${radio}_${ctl_item}_prio"
                prio=`nvram kget ${name}`
                if [ ! -z $affinity ]; then 
                    echo $name=$prio >>${KERNEL_NVRAM_FILE}
                fi
            fi
        done

        for ctl_item in $INTR_LIST_PER_RADIO
        do
            nv_name=$(get_interrupt_nv_name $ctl_item $radio)
            affinity=`nvram kget ${nv_name}`
            if [ ! -z $affinity ]; then 
                echo $nv_name=$affinity >> ${KERNEL_NVRAM_FILE}
            fi
        done
        radio=$((radio+1))
    done

    #Common WiFi process affinity and priority
    for ctl_item in $PROC_LIST_COMMON
    do
        proc_name=$(get_process_name $ctl_item);
        proc_pid=$(getPidByName $proc_name);
        if [ $proc_pid -gt 0 ]; then
            name="wl_${ctl_item}_affinity"
            affinity=`nvram kget ${name}`
            if [ ! -z $affinity ]; then 
                echo $name=$affinity >> ${KERNEL_NVRAM_FILE}
            fi
            name="wl_${ctl_item}_prio"
            prio=`nvram kget ${name}`
            if [ ! -z $affinity ]; then 
                echo $name=$prio >>${KERNEL_NVRAM_FILE}
            fi
        fi
    done

    mount -t ubifs ubi:rootfs_ubifs / -o remount,ro
}

apply_wifi_pin_cfg() {

    local applied=0 radio=0;
    local irq name prio affinity proc_name proc_pid;

    radio=0;
    while [ $radio -lt $NUM_WL_INTF ];
    do
        for ctl_item in $PROC_LIST_PER_RADIO
        do
            proc_name=$(get_process_name $ctl_item $radio);
            proc_pid=$(getPidByName $proc_name);
            if [ $proc_pid -gt 0 ]; then
                name="wl${radio}_${ctl_item}_affinity"
                affinity=`nvram kget ${name}`
                if [ ! -z $affinity ]; then 
                    applied=1;
                    setProcAffinity  $name $proc_name $proc_pid $affinity
                fi
                name="wl${radio}_${ctl_item}_prio"
                prio=`nvram kget ${name}`
                if [ ! -z $prio ]; then 
                    applied=1;
                    setProcPriority $name $proc_name $proc_pid $prio
                fi
            fi
        done

        for ctl_item in $INTR_LIST_PER_RADIO
        do
            nv_name=$(get_interrupt_nv_name $ctl_item $radio)
            affinity=`nvram kget ${nv_name}`
            if [ ! -z $affinity ]; then 
                applied=1;
                int_name=$(get_interrupt_name $ctl_item $radio)
                setIntrAffinity $int_name $affinity
            fi
        done

        radio=$((radio+1))
    done

    #Common WiFi process affinity and priority
    for ctl_item in $PROC_LIST_COMMON
    do
        proc_name=$(get_process_name $ctl_item);
        proc_pid=$(getPidByName $proc_name);
        if [ $proc_pid -gt 0 ]; then
            name="wl_${ctl_item}_affinity"
            affinity=`nvram kget ${name}`
            if [ ! -z $affinity ]; then 
                applied=1;
                setProcAffinity  $name $proc_name $proc_pid $affinity
            fi
            name="wl_${ctl_item}_prio"
            prio=`nvram kget ${name}`
            if [ ! -z $prio ]; then 
                applied=1;
                setProcPriority $name $proc_name $proc_pid $prio
            fi
        fi
    done

    #echo $applied;
    if [ ! -z "$1" ]; then
        eval $1=$applied
    fi
}

set_default_affinity_plat3()
{
    local ctl_item radio affinity

    radio=0
    while [ $radio -lt $NUM_WL_INTF ];
    do
        # /* bind to CPU{x} */
        affinity=$((1 << (radio%NUM_PROCESSOR)))
        # echo "affinity=$affinity"
        for ctl_item in $PROC_LIST_PER_RADIO
        do
            proc_name=$(get_process_name $ctl_item $radio);
            proc_pid=$(getPidByName $proc_name);
            if [ $proc_pid -gt 0 ]; then
                setProcAffinity "" $proc_name $proc_pid $affinity
            fi
        done
        radio=$((radio+1))
    done
}

# When CPU0 to be free affine all radio processes to CPU#1
# Otherwise Affine light radio (2G) processes to CPU#0 to moderately keep it free
set_default_affinity_Archer_NRCPUS3()
{
    local radio affinity dhd_radio

    dhd_radio=`cat /proc/interrupts |grep wl0 |grep dhdpcie`
    radio=0
    while [ $radio -lt $NUM_WL_INTF ];
    do
        affinity=$((1 << 1))
        if [ $NUM_WL_INTF -eq 2 ]; then
            if [ $ALLOW_CPU0_AFF -gt 0 ]; then
                # Two Radios - External Radio (DHD/NIC) and Internal Radio (NIC)
                # Always affine Internal Radio to cpu#0 (light load)
                # External DHD -  wl0/DHD/cpu#1,     wl1/INT_NIC/cpu#0
                # External NIC -  wl0/INT_NIC/cpu#0, wl1/EXT_NIC/cpu#1
                if [ $radio -eq 0 ]; then
                    if [ -z "$dhd_radio" ]; then
                        #dhd not present, first radio is Internal NIC - affine to cpu0
                        affinity=$((1 << 0))
                    fi
                else
                    if [ ! -z "$dhd_radio" ]; then
                        #dhd is present, second radio is Internal NIC - affine to cpu0
                        affinity=$((1 << 0))
                    fi
               fi
            fi
        fi

        #echo "affinity=$affinity"
        for ctl_item in $PROC_LIST_PER_RADIO
        do
            proc_name=$(get_process_name $ctl_item $radio);
            proc_pid=$(getPidByName $proc_name);
            if [ $proc_pid -gt 0 ]; then
                setProcAffinity "" $proc_name $proc_pid $affinity
            fi
        done

        #Set WLAN D11 and M2M interrupt affinity
        for ctl_item in $INTR_LIST_PER_RADIO
        do
            int_name=$(get_interrupt_name $ctl_item $radio);
            setIntrAffinity $int_name $affinity;
        done

        radio=$((radio+1))
    done

    # /* The bcm_archer_us thread affinity is defined in ARCHER_CPU_AFFINITY */
    for ctl_item in $PROC_LIST_COMMON
    do
        # /* bind common process to cpu#1. to make cpu#0 free */
        affinity=$((1 << 1))

        proc_name=$(get_process_name $ctl_item $radio);
        proc_pid=$(getPidByName $proc_name);
        if [ $proc_pid -gt 0 ]; then
            setProcAffinity "" $proc_name $proc_pid $affinity
            setProcPriority "" $proc_name $proc_pid 75
        fi
    done
}

set_default_affinity_Archer_NRCPUS4()
{
    local radio affinity

    # /* The bcm_archer_us thread affinity is defined in ARCHER_CPU_AFFINITY */

    # /* bind bcmsw_recycle to cpu#1 or cpu#2 */
    affinity=6
    proc_name=bcmsw_recycle
    proc_pid=$(getPidByName $proc_name);
    if [ $proc_pid -gt 0 ]; then
        setProcAffinity "" $proc_name $proc_pid $affinity
    fi

    radio=0
    while [ $radio -lt $NUM_WL_INTF ];
    do
        # /* bind to CPU{x} */
        affinity=$((1 << (radio%NUM_PROCESSOR)))
        #echo "affinity=$affinity"
        for ctl_item in $WLDRV_PROC_LIST
        do
            proc_name=$(get_process_name $ctl_item $radio);
            proc_pid=$(getPidByName $proc_name);
            if [ $proc_pid -gt 0 ]; then
                setProcAffinity "" $proc_name $proc_pid $affinity
            fi
        done

        #Set WLAN D11 and M2M interrupt affinity
        for ctl_item in $INTR_LIST_PER_RADIO
        do
            int_name=$(get_interrupt_name $ctl_item $radio);
            setIntrAffinity $int_name $affinity;
        done

        # /* bind bcm_awl_xfer_xx to cpu#1 or cpu#2 */
        affinity=6
        proc_name=bcm_awl_xfer_$radio
        proc_pid=$(getPidByName $proc_name);
        if [ $proc_pid -gt 0 ]; then
            setProcAffinity "" $proc_name $proc_pid $affinity
            setProcPriority "" $proc_name $proc_pid 75
        fi

        # /* bind archer_dhd_xx to CPU{x} */
        affinity=$((1 << (radio%NUM_PROCESSOR)))
        proc_name=archer_dhd_$radio
        proc_pid=$(getPidByName $proc_name);
        if [ $proc_pid -gt 0 ]; then
            setProcAffinity "" $proc_name $proc_pid $affinity
            setProcPriority "" $proc_name $proc_pid 75
        fi

        radio=$((radio+1))
    done

    # /* bind bcm_archer_wlan to cpu#dhd if exists and offload is disabled */
    local doa=`ps |grep archer_dhd |grep -v grep | cut -d '[' -f2`
    proc_name=bcm_archer_wlan
    proc_pid=$(getPidByName $proc_name);
    if [ $proc_pid -gt 0 ]; then
        affinity=6
        if [ -z $doa ]; then
            affinity=$(getIntrAffinity wl0 dhdpcie);
            if [ -z $affinity ]; then
                affinity=6
            #else same as DHD priority
            fi
        fi
        setProcAffinity "" $proc_name $proc_pid $affinity
    fi
}

set_xBow_socket_affinity()
{
    echo "wlaffinity: delayed affinity operation"
    # wait 60 seconds for bcm_enet to load and run a worklet that makes
    # archer register the crossbow_socket irq with the kernel
    intr_name=$(get_interrupt_name xbow_socket 0)
    for i in $(seq 1 60);
    do
        if grep -qF "$intr_name" /proc/interrupts; then
            break;
        fi
        sleep 1
    done

    radio=0
    while [ $radio -lt $NUM_WL_INTF ];
    do
        cpu=$((radio%NUM_PROCESSOR + 1))
        affinity=$((1 << cpu))

        for ctl_item in $PLTDRV_INTR_LIST
        do
            int_name=$(get_interrupt_name $ctl_item $radio);
            setIntrAffinity $int_name $affinity;
        done

        radio=$((radio+1))
    done
}
# All SDK housekeeping processes:                      CPU0
# All processes/interrupts related to Internal WiFi#0: CPU1
# All processes/interrupts related to Internal WiFi#1: CPU2
# All processes/interrupts related to WiFi#2:          CPU3
#
# wl{x} arragement:
# Tri-band : DHD(wl0/cpu1), internal radio (wl1/cpu2, wl2/cpu3)
# Tri-band : internal radio (wl0/cpu1, wl1/cpu2), NIC (wl2/cpu3)
# Dual-band: internal radio (wl0/cpu1, wl1/cpu2)
#
set_default_affinity_priority_xBow()
{
    local radio affinity cpu
    local ctl_item int_name proc_name proc_pid
    local cmn_proc_cpu dhd_radio

    echo "Default Affinity for xBow"

    cmn_proc_cpu=0
    cmn_affinity=$((1 << cmn_proc_cpu))

    echo "wl{x}=>CPU{x+1}, priority 5"

    # Affine WiFi Radio interface specific processes and interrupts
    radio=0
    while [ $radio -lt $NUM_WL_INTF ];
    do
        cpu=$((radio%NUM_PROCESSOR + 1))

        # /* bind WLAN processes to CPU{x} */
        affinity=$((1 << cpu))
        priority=5
        for ctl_item in $WLDRV_PROC_LIST
        do
            proc_name=$(get_process_name $ctl_item $radio);
            proc_pid=$(getPidByName $proc_name);
            if [ $proc_pid -gt 0 ]; then
                setProcAffinity "" $proc_name $proc_pid $affinity
                setProcPriority "" $proc_name $proc_pid $priority
            fi
        done

        priority=1
        for ctl_item in $PLTDRV_PROC_CPU_LIST
        do
            proc_name=$(get_process_name $ctl_item $radio);
            proc_pid=$(getPidByName $proc_name);
            if [ $proc_pid -gt 0 ]; then
                setProcAffinity "" $proc_name $proc_pid $affinity
                setProcPriority "" $proc_name $proc_pid $priority
            fi
        done

	# per-radio threads that need to be bound to the common cpu (for example
	# stq):
        for ctl_item in $PLTDRV_PROC_COMMON_LIST
        do
            proc_name=$(get_process_name $ctl_item $radio);
            proc_pid=$(getPidByName $proc_name);
            if [ $proc_pid -gt 0 ]; then
                setProcAffinity "" $proc_name $proc_pid $cmn_affinity
                setProcPriority "" $proc_name $proc_pid $priority
            fi
        done
        # bind WLAN interrupts to CPU{x+1}
        for ctl_item in $WLDRV_INTR_LIST
        do
            int_name=$(get_interrupt_name $ctl_item $radio);
            setIntrAffinity $int_name $affinity;
        done

        radio=$((radio+1))
    done

    for ctl_item in $PROC_LIST_COMMON
    do
        if [ "$ctl_item" == "bcm_archer_us" ]; then
            # exclude bcm_archer_us from default affinity setting
            continue
        fi
        proc_name=$(get_process_name $ctl_item);
        proc_pid=$(getPidByName $proc_name);
        if [ $proc_pid -gt 0 ]; then
            setProcAffinity "" $proc_name $proc_pid $cmn_affinity
        fi
    done

    # Set xbow_socket affinities but can't do it here, because these are
    # registered with the kernel only after bcm_enet.ko is loaded and calls
    # archer_driver_host_config(). So launch a background task to check for
    # bcm_enet.ko to be loaded and then run the affinity loop.
    set_xBow_socket_affinity &
}

set_default_priority()
{
    local ctl_item ctl_item1 radio priority

    radio=0
    while [ $radio -lt $NUM_WL_INTF ];
    do
        for ctl_item in $PROC_LIST_PER_RADIO
        do
            priority=75
            for ctl_item1 in $WLDRV_PROC_LIST
            do
                if [ "$ctl_item" == "$ctl_item1" ]; then
                    #wlan threads default priority=5
                    priority=5
                    break
                fi
            done
            proc_name=$(get_process_name $ctl_item $radio);
            proc_pid=$(getPidByName $proc_name);
            if [ $proc_pid -gt 0 ]; then
                setProcPriority "" $proc_name $proc_pid $priority
            fi
        done
        radio=$((radio+1))
    done

    #$PROC_LIST_COMMON priority is left as default

}

# configuring with default policy
pin_wifi_processes_default() {
   echo "Wlan related processes are pinned by default"
   if [ $NUM_WL_INTF -gt 0 ]; then
       # Archer based platforms
       if [ ! -z $ARCHER ]; then
           if [ ${NUM_PROCESSOR} -lt 4 ]; then
               # Archer based platforms with 3 cores
               # BCM963178....
               set_default_affinity_Archer_NRCPUS3
           else
               # Archer based platforms with 4 cores
               # BCM947622, BCM96756 ....
               set_default_affinity_Archer_NRCPUS4
           fi
       elif [ ! -z $CROSSBOW ]; then
           # CrossBow platforms
           # BCM96765, BCM947722, BCM6766
           set_default_affinity_priority_xBow
           return
       else  # High-end wfd based platforms
           set_default_affinity_plat3
       fi
       # set priority
       set_default_priority
   fi
}

auto_config() {
    local apply_configed=0
    echo "Configuring wlan related threads..."

    apply_wifi_pin_cfg apply_configed

    if [ $apply_configed -eq 0 ]; then
        echo "no thread pin applied"
        pin_wifi_processes_default
    else
        echo "Wlan related processes are pinned by configuration";
    fi
}
