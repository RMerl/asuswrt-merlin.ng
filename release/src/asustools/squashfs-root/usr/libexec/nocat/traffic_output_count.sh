#!/bin/sh
/usr/sbin/iptables -vnx -t filter -L NoCat_Upload | /bin/grep $1 |  /bin/sed 's/[ ]\+/ /g' | /bin/cut -d ' ' -f3
