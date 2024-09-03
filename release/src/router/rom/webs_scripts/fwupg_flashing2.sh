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
# <<Broadcom-WL-IPTag/Proprietary,Open:.*>>
#


echo "===== fwupg_flashing2.sh ====="
echo "now path is " 
pwd
echo
ls -al
echo

PPIDS=`fuser -m /old-root`
echo "kill process who is still using /old-root: $PPIDS"


for pp in $PPIDS
do
#echo "pp is $pp"
echo "killing pid $pp"
kill -9 $pp
done

umount /old-root/mnt/defaults
umount /old-root/proc
umount /old-root/data
umount /old-root/dev/pts
umount /old-root/sys/kernel/debug
umount /old-root/sys
umount /old-root/var
umount /old-root/tmp/mnt/defaults
umount /old-root/tmp/mnt
umount /old-root/tmp
umount /old-root/mnt
umount /old-root/dev/pts
umount /old-root/dev
umount /old-root/tmp/mnt/defaults
umount /old-root/tmp/mnt
umount /old-root/jffs
umount /old-root/tmp/mnt/defaults
umount /old-root/tmp/mnt
umount /old-root

ret="$?"
echo "ret result: $ret"

echo "chk ps"
echo
sync
sleep 1
ps
echo
echo "chk lsof"
lsof

echo
echo "lsof end"
echo
echo
echo

echo "don't flashing new image in this script"
#/bin/bcm_flasher /tmp/linux.trx 1
#echo
#echo
#echo "flashing done"


