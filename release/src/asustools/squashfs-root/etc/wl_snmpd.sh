#!/bin/sh

place=".1.3.6.1.4.1.2021.255"

refresh() {

  id=1
  lastid=0
  
  for mac in $(wl_rt2880 assoclist | cut -d" " -f2)
  do
    if test $lastid -eq 0
    then
      getnext_1361412021255="$place.3.54.1.3.32.1.1.1"
      getnext_1361412021255354133211="$place.3.54.1.3.32.1.1.1"
      getnext_1361412021255354133214="$place.3.54.1.3.32.1.4.1"
      getnext_13614120212553541332113="$place.3.54.1.3.32.1.13.1"
      getnext_13614120212553541332126="$place.3.54.1.3.32.1.26.1"
    else
      eval getnext_1361412021255354133211${lastid}="$place.3.54.1.3.32.1.1.$id"
      eval getnext_1361412021255354133214${lastid}="$place.3.54.1.3.32.1.4.$id"
      eval getnext_13614120212553541332113${lastid}="$place.3.54.1.3.32.1.13.$id"
      eval getnext_13614120212553541332126${lastid}="$place.3.54.1.3.32.1.26.$id"
    fi
  
    rssi=$(wl_rt2880 rssi $mac | cut -d" " -f3)
    noise_reference=$(wl_rt2880 noise $mac | cut -d" " -f3)
    if test $rssi -eq 0
    then
      snr=0
    else
      let snr=-1*$noise_reference+$rssi
    fi
    mac=$(echo $mac | tr : ' ')
  
    eval value_1361412021255354133211${id}=$id;
    eval type_1361412021255354133211${id}='integer';
    eval value_1361412021255354133214${id}='$mac';
    eval type_1361412021255354133214${id}='octet';
    eval value_13614120212553541332113${id}=$noise_reference;
    eval type_13614120212553541332113${id}='integer';
    eval value_13614120212553541332126${id}=$snr;
    eval type_13614120212553541332126${id}='integer';

    lastid=$id
    let id=$id+1
  
  done

  if test $lastid -ne 0
  then
    eval getnext_1361412021255354133211${lastid}="$place.3.54.1.3.32.1.4.1"
    eval getnext_1361412021255354133214${lastid}="$place.3.54.1.3.32.1.13.1"
    eval getnext_13614120212553541332113${lastid}="$place.3.54.1.3.32.1.26.1"
    eval getnext_13614120212553541332126${lastid}="NONE"
  fi
} 

LASTREFRESH=0

while read CMD
do
  case "$CMD" in
    PING)
      echo PONG
      continue 
      ;;
    getnext)
      read REQ
      let REFRESH=$(date +%s)-$LASTREFRESH
      if test $REFRESH -gt 30
      then
        LASTREFRESH=$(date +%s)
        refresh
      fi
      
      oid=$(echo $REQ | tr -d .) 
      eval ret=\$getnext_${oid}
      if test "x$ret" = "xNONE"
      then
        echo NONE
        continue 
      fi 
      ;;
    *)
      read REQ
      if test "x$REQ" = "x$place"
      then
        echo NONE
        continue 
      else
        ret=$REQ
      fi
      ;;
  esac

  oid=$(echo $ret | tr -d .) 
  if eval test "x\$type_${oid}" != "x"
  then
    echo $ret
    eval echo "\$type_${oid}"
    eval echo "\$value_${oid}"
  else
    echo NONE
  fi

done
