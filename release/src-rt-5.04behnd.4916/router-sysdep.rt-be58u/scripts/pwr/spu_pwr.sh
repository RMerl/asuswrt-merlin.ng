#!/bin/sh

##########################################################################
# @file:   spu_pwr.sh                                                    #
# @author: Yoel Tulatov (yoel.tulatov@broadcom.com)                      #
# @brief:  commands powering up/down SPU HW accelerator                  #
# @date:   2022-12-11                                                    #
##########################################################################

# @todo: Put all af these common definitions in some common .sh file to be shard among all init.d scripts
MODULE_FOLDER="/lib/modules/$(uname -r)/extra"
FLEXRM_MODULE="bcmflex"
SPU_MODULE="bcmspu"
#As defined in sysexits.h: EX_TEMPFAIL     75      /* temp failure; user is invited to retry */
EX_TEMPFAIL=75

# @brief prints usage of this script
function usage ()
{
    echo "brief:    Commands powering up/down SPU HW accelerator"
    echo "usage:    $0 [OPTIONS]"
    echo "          resume  [0-9]+     resume SPU, wait until all active streams"
    echo "          suspend [0-9]+     suspend SPU, wait until all active streams"
}

# @brief checks if ipsec is not active then starts it and *synchronizes on the completion of the latter
# @param $1 time to wait (in seconds) before ip xfrm state returs stream stats
# @return 0 if ip xfrm state contains ANY information, error code otherwise
function gracefully_start_ipsec()
{
    ret=0    
    if [ -z "$(ip xfrm state)" ]; then
        ipsec start
        ret=$?
        if [ "$ret" -eq 0 ]; then
            #Syncronization sleep before ip xfrm state starts providing data
            if [ "$1" != "" ] && [[ "$1" =~ ^[0-9]+$ ]]
                then
                # *NOTE:This workaround is problematic: 
                # It takes 1 sec to sync on B0, what will happen on other boards is a question
                sleep $1
            fi    
            if [ -z "$(ip xfrm state)" ]; then
                echo "ipsec stream is not active"
            fi
        fi
    fi
    return $ret
}

# @brief checks if ipsec is active then stops it
# @return ret code 75 if ipsec is streaming, 0 otherwise
function gracefully_stop_ipsec()
{
    ret=0
    if [ ! -z "$(ip xfrm state)" ]; then
        ipsec stop
        ret=$?
        if [ "$ret" -eq 0 ]; then
            if [ ! -z "$(ip xfrm state)" ]; then
                echo "ipsec stream is still active"
                ret=$EX_TEMPFAIL
            fi
        fi
    fi
    return $ret
}

########################################
#                MAIN                  #
########################################
rc=0
ipxfrmstate_sync_time=1

case $1 in
suspend)
    echo "Susspending SPU ..."
    gracefully_stop_ipsec
    rc=$?
    if [ "$rc" -eq 0 ]; then
        rmmod $SPU_MODULE
        rc=$?
        if [ "$rc" -eq 0 ]; then
            rmmod $FLEXRM_MODULE
            rc=$?
            if [ "$rc" -eq 0 ]; then 
                gracefully_start_ipsec $ipxfrmstate_sync_time
                rc=$?
            fi
        fi
    fi
    ;;
resume)
    echo "Resuming SPU ..."
    gracefully_stop_ipsec
    rc=$?
    if [ "$rc" -eq 0 ]; then
        insmod $MODULE_FOLDER/$FLEXRM_MODULE.ko
        rc=$?
        if [ "$rc" -eq 0 ]; then
            insmod $MODULE_FOLDER/$SPU_MODULE.ko
            rc=$?
            if [ "$rc" -eq 0 ]; then
                gracefully_start_ipsec $ipxfrmstate_sync_time
                rc=$?
            fi
        fi
    fi
    ;;
*)
    usage
    exit 1
esac

if [ $rc -ne 0 ]; then
    echo "$0 ${@} failed. Exit code: $rc"
fi
exit $rc
