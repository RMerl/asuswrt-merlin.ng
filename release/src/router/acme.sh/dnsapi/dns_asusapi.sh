#!/bin/sh



########  Public functions #####################

#Usage: dns_asusapi_add   _acme-challenge.domain.asuscomm.com   "XKrxpRBosdIKFzxW_CT3KLZNf6q0HG9i01zxXp5CPBs"
dns_asusapi_add() {
  fulldomain=$1
  txtvalue=$2
  echo -n "$txtvalue" > /tmp/acme.txt

  if ! add_record ; then
     _err "Add DNS record fail"
     rm /tmp/acme.txt
     return 1
  else
     rm /tmp/acme.txt
     return 0
  fi
}





####################  Private functions bellow ##################################
add_record() {
  _info "Adding record"

  rm -f /var/cache/inadyn/*.cache
  service start_ddns

  cnt="20"
  until [ "$ddns_status" = "1" ] || [ "$cnt" = "0" ]; do
    _info "Wait DDNS service ...$cnt"
    sleep 3
    ddns_status="`nvram get ddns_status`"
    cnt="`expr $cnt - 1`"
  done

  if [ "$cnt" = "0" ]; then
    _debug "DDNS return code `nvram get ddns_return_code`"
    return 1
  else
    _debug "Add DNS record finished"
    return 0
  fi
}

_info() {
  if [ -z "$2" ] ; then
    echo "[$(date)] $1"
  else
    echo "[$(date)] $1='$2'"
  fi
}

_err() {
  _info "$@" >&2
  return 1
}

_debug() {
  if [ -z "$DEBUG" ] ; then
    return
  fi
  _err "$@"
  return 0
}

_debug2() {
  if [ "$DEBUG" ] && [ "$DEBUG" -ge "2" ] ; then
    _debug "$@"
  fi
  return
}
