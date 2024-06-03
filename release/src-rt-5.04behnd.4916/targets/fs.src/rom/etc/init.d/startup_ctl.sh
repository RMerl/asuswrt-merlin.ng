
###### The process startup/stop control common definitions ######

###################### BDK start sequence #######################
#             mcpd, ssl, dbus or ubus, save-dmesg  (in parallel)
#                          |
#                    sys_directory
#                          |
# devinfo_md, diag_md, epon_md, gpon_md, dsl_md, wifi_md, tr69_md, voice_md (in parallel)
#                          |
#                      sysmgmt_md

bootflag_prefix_start=/tmp/bcm_bootflag_start

# param: process1 process2 process3 ... 
is_start_done()
{
  while [ $# != 0 ]; do
    proc=$1
    shift
    bootflag_in_starting=${bootflag_prefix_start}_${proc}
    k=0
    while true; do
      if [ ! -f ${bootflag_in_starting} ]; then
        break
      elif [ $k -gt 1500 ]; then
        echo "ERR: Waiting for ${proc} ready, timeout!"
        exit 1
      else
        usleep 200000
      fi
      let k+=1
    done
  done
}

# Note: this must be called before is_start_done
start_up()
{
  process=$1
  bootflag_start=${bootflag_prefix_start}_${process}
  echo "Starting ${process} ..."
  echo "" > ${bootflag_start}
}

start_done()
{
  echo "Starting ${process} DONE"
  rm -f ${bootflag_start}
}

trap_err()
{
  # busybox does not support "trap ERR", so use "set -e" + "trap EXIT" instead
  set -e
  trap '[ $? == 0 ] && exit 0 || echo ERR: Start ${process} failed!; exit 1' EXIT
}
