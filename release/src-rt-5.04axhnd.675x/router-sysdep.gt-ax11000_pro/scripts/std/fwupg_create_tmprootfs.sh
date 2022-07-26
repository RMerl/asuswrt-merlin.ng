#!/bin/sh
# Copyright (C) 2021, Broadcom. All Rights Reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#
# <<Broadcom-WL-IPTag/Proprietary,Open:.*>>
#



#ubivol number to dynamically create temporarily rootfs. default volume 12
#7MB about 94% ussed, reserve 10MB
N_TMP_UBIVOL=12
SZ_TMP_UBIVOL=10000000

ubinfo -a
bcm_bootstate

wl -i wl0 down
wl -i wl1 down
wl -i wl2 down

#kill apps who is going to bring up others
wdtctl -d stop
killall debug_monitor

ubirmvol /dev/ubi0 -n $N_TMP_UBIVOL 
ubimkvol /dev/ubi0 -s $SZ_TMP_UBIVOL -n $N_TMP_UBIVOL -N tmprootfs --type=dynamic
mkdir -p /mnt/new-root
mount -t ubifs /dev/ubi0_$N_TMP_UBIVOL /mnt/new-root


sleep 1
lsof

mkdir -p /mnt/new-root/proc
mkdir -p /mnt/new-root/sys
mkdir -p /mnt/new-root/tmp
mkdir -p /mnt/new-root/dev
mkdir -p /mnt/new-root/var
mkdir -p /mnt/new-root/var/tmp
mkdir -p /mnt/new-root/etc
mkdir -p /mnt/new-root/bin
mkdir -p /mnt/new-root/sbin
mkdir -p /mnt/new-root/usr/bin
mkdir -p /mnt/new-root/lib
mkdir -p /mnt/new-root/data

mount --bind /proc /mnt/new-root/proc
mount --bind /sys /mnt/new-root/sys
mount --bind /tmp /mnt/new-root/tmp
mount --bind /dev /mnt/new-root/dev
mount --bind /var /mnt/new-root/var
mount --bind /var/tmp /mnt/new-root/var/tmp
mount -- /data /mnt/new-root/data


cp /bin/busybox /mnt/new-root/bin
cp /bin/bcmbusybox /mnt/new-root/bin
cp /bin/ubi* /mnt/new-root/bin
cp /bin/bcm_* /mnt/new-root/bin
cp /lib/libc.* /mnt/new-root/lib
cp /lib/libbcm_flashutil.so /mnt/new-root/lib
cp /lib/libbcm_boardctl.so /mnt/new-root/lib
cp /lib/libbcm_util.so /mnt/new-root/lib
cp /sbin/rc /mnt/new-root/bin
cp /lib/libsys_util.so /mnt/new-root/lib
cp /lib/libgen_util.so /mnt/new-root/lib
cp /lib/librt.* /mnt/new-root/lib
cp /lib/libdl.* /mnt/new-root/lib
cp /lib/libpthread.* /mnt/new-root/lib
cp /lib/ld-linux* /mnt/new-root/lib
cp /lib/libm.so.6 /mnt/new-root/lib
cp /usr/lib/libptcsrv.so /mnt/new-root/lib
cp /lib/libgcc_s.so.1 /mnt/new-root/lib
cp /usr/lib/libconn_diag.so /mnt/new-root/lib
cp /lib/libnvram.so /mnt/new-root/lib
cp /usr/lib/libshared.so /mnt/new-root/lib
cp /lib/libcrypt.so.1 /mnt/new-root/lib
cp /lib/libwlcsm.so /mnt/new-root/lib
cp /usr/lib/libwpa_client.so /mnt/new-root/lib
cp /lib/libbcm.so /mnt/new-root/lib
cp /usr/lib/libbcm.so /mnt/new-root/lib
cp /usr/lib/libcrypto.so.1.1 /mnt/new-root/lib
cp /usr/lib/libssl.so.1.1 /mnt/new-root/lib
cp /usr/lib/libbwdpi.so /mnt/new-root/lib
cp /lib/libdisk.so /mnt/new-root/lib
cp /usr/lib/libcfgmnt.so /mnt/new-root/lib
cp /usr/lib/libvpn.so /mnt/new-root/lib
cp /usr/lib/libasc.so /mnt/new-root/lib
cp /usr/lib/libletsencrypt.so /mnt/new-root/lib
cp /usr/lib/libcurl.so.4 /mnt/new-root/lib
cp /usr/lib/libamas-utils.so /mnt/new-root/lib
cp /usr/lib/liblldpctl.so.4 /mnt/new-root/lib
cp /usr/lib/libjansson.so.4 /mnt/new-root/lib
cp /lib/libasuslog.so /mnt/new-root/lib
cp /usr/lib/libjson-c.so.2 /mnt/new-root/lib
cp /usr/lib/libsqlite3.so.0 /mnt/new-root/lib
cp /rom/etc/init.d/fwupg_flashing.sh /mnt/new-root/etc
cp /rom/etc/get_rootfs_dev.sh /mnt/new-root/etc

if [ ! -f /mnt/new-root/bin/busybox ]
then
	echo "ERROR !! not enough space on /new-root"
	exit
fi

cd /mnt/new-root/bin

ln -sf busybox ash
ln -sf busybox bash
ln -sf busybox cat
ln -sf busybox chmod
ln -sf busybox cp
ln -sf busybox dmesg
ln -sf busybox echo
ln -sf busybox grep
ln -sf busybox kill
ln -sf busybox mount
ln -sf busybox ps
ln -sf busybox rm
ln -sf busybox sh
ln -sf busybox sleep
ln -sf busybox sync
ln -sf busybox stty
ln -sf busybox umount
ln -sf busybox vi
ln -sf busybox cut
ln -sf busybox ls
ln -sf busybox ln
ln -sf busybox fuser
ln -sf busybox lsof
ln -sf busybox which 

#usr bin
ln -sf busybox du
ln -sf busybox killall
ln -sf busybox sha256sum
ln -sf busybox test
ln -sf busybox tftp
ln -sf busybox which
ln -sf busybox [
ln -sf busybox ]
cd -


cd /mnt/new-root/sbin
ln -sf ../bin/busybox arp
ln -sf ../bin/busybox chroot
ln -sf ../bin/busybox halt
ln -sf ../bin/busybox ifconfig
#ln -sf ../bin/busybox init
ln -sf ../bin/busybox pivot_root
ln -sf ../bin/busybox reboot
ln -sf ../bin/busybox expr
ln -sf ../bin/rc init
cd -


cp /bin/busybox /tmp
ln -sf /tmp/busybox /tmp/sh
mount
df


#inittab in new rootfs
#echo '::sysinit:/bin/sh -l -c "/data/fwupg_flashing.sh"' > /mnt/new-root/etc/inittab
echo '::sysinit:/bin/sh -l -c "/etc/fwupg_flashing.sh"' > /mnt/new-root/etc/inittab
echo '::shutdown:/bin/sh -l -c "bcm_boot_launcher stop"' >> /mnt/new-root/etc/inittab
sync


mkdir -p /mnt/new-root/old-root
cd /mnt/new-root
pivot_root . old-root
exec chroot . /tmp/sh -c 'echo $$; kill -QUIT 1' <dev/console >dev/console 2>&1

