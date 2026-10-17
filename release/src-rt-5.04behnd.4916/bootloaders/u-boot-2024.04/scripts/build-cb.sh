#!/bin/sh
# SPDX-License-Identifier: GPL-2.0+
#
# Sample script to build a disk image suitable for use with coreboot. The image
# includes a kernel and initrd.
#
# YOU WILL NEED to modify this for your needs, e.g. select a kernel.
#
# Run this with:
# qemu-system-i386 -bios coreboot.rom -drive file=disk.img,if=virtio

qemu-img create -f raw disk.img 120M
mkfs.ext2 -F disk.img
sudo mkdir -p /mnt/rootfs
sudo mount -o loop disk.img /mnt/rootfs
sudo mkdir -p /mnt/rootfs/boot
sudo cp /boot/vmlinuz /mnt/rootfs/boot/.
sudo cp /boot/initrd.img /mnt/rootfs/boot/.
sudo umount /mnt/rootfs
