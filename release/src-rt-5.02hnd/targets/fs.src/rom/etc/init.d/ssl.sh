#!/bin/sh

if openssl -v 2> /dev/null
then
  if [ -r /data/newkey ]
  then
    cat /data/newkey > /var/tmp/newkey
    cat /data/newkey.cert > /var/tmp/newkey.cert
  elif (pspctl list | grep -qw sslkey ) && (pspctl list | grep -qw sslcert)
  then
    pspctl adump sslkey > /var/tmp/newkey
    pspctl adump sslcert > /var/tmp/newkey.cert
  else
    (
    cat /proc/nvram/BaseMacAddr > /dev/random
    export OPENSSL_CONF=/rom/etc/openssl.cnf
    cd /var/tmp
    openssl req -new -newkey rsa:2048 -days 36500 -nodes -x509 -subj "/C=US/ST=Denial/L=Irvine/O=Dis/CN=example.com" -keyout newkey -out newkey.cert
    openssl rsa -noout -in newkey -modulus
    if echo > /data/newkey.cert 2>/dev/null
    then
      cp /var/tmp/newkey /data
      cp /var/tmp/newkey.cert /data
      chmod 600 /data/newkey
      chmod 600 /data/newkey.cert
      sync
    else
      pspctl set sslkey "`cat /var/tmp/newkey`"
      pspctl set sslcert "`cat /var/tmp/newkey.cert`"
    fi
    )
  fi
  (
  cd /var/tmp
  openssl rsa -in newkey -out rsak
  dropbearconvert openssh dropbear rsak dropbear_rsa_host_key 2> /dev/null
  rm rsak
  )
fi

