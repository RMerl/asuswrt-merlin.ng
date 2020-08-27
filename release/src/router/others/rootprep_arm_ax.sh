#!/bin/bash

ROOTDIR=$PWD

#mkdir -p -m 0755 dev
mkdir -p -m 0755 proc
mkdir -p -m 0755 sys
mkdir -p -m 0755 jffs
mkdir -p -m 0755 cifs1
mkdir -p -m 0755 cifs2
mkdir -p -m 0755 sysroot
mkdir -p -m 0755 var
ln -sf tmp/opt opt

# tmp
mkdir -p -m 0755 tmp
rm -rf media
#ln -sf tmp/var var
ln -sf tmp/media media
(cd $ROOTDIR/usr && ln -sf ../tmp)

# etc
rm -rf tmp/etc/*
#(cd etc && tar -cpf - .) | (cd tmp/etc && tar -xpf - )
rm -rf etc && ln -sf tmp/etc etc
echo "/lib/aarch64" > rom/etc/ld.so.conf
echo "/lib" >> rom/etc/ld.so.conf
echo "/usr/lib" >> rom/etc/ld.so.conf
#/sbin/ldconfig -r $ROOTDIR
mv tmp/etc/* rom/etc
rm -f etc/ld.so.conf etc/ld.so.cache
ln -sf /rom/etc/ld.so.conf etc/ld.so.conf
#ln -sf /rom/etc/ld.so.cache etc/ld.so.cache

# !!TB
mkdir -p -m 0755 mmc
mkdir -p -m 0755 usr/local
rm -rf usr/share usr/local/share
ln -sf /tmp/share usr/share
ln -sf /tmp/share usr/local/share

rm -rf mnt
ln -sf tmp/mnt mnt
rm -rf home root
ln -sf tmp/home home
ln -sf tmp/home/root root
(cd usr && ln -sf ../tmp)

# !!TB
rm -rf www/ext www/user www/proxy.pac www/wpad.dat
ln -sf /tmp/var/wwwext www/ext
ln -sf /tmp/var/wwwext www/user
ln -sf /www/ext/proxy.pac www/proxy.pac
ln -sf /www/ext/proxy.pac www/wpad.dat

chmod 775 sbin/rc bin/rstats
