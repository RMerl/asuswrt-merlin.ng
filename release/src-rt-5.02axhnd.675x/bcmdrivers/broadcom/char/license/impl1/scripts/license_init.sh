#!/bin/sh

echo Loading Licenses
for lic in /etc/licenses/*; do [ -e "$lic" ] || continue; cat $lic > /proc/driver/license; done
for lic in /data/licenses/*; do [ -e "$lic" ] || continue; cat $lic > /proc/driver/license; done
