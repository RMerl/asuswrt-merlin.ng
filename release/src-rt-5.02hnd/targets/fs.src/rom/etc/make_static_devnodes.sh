#!/bin/sh

echo ">>>>> Creating static device nodes <<<<<"

#Create standard character devices
mknod /dev/kmem c 1 2
mknod /dev/tty0 c 4 0
mknod /dev/tty1 c 4 1
mknod /dev/ttyH0 c 4 66
mknod /dev/ptm c 128 1
mknod /dev/printer0 c 180 0

# Create Broadcom Proprietary devices   ALL major numbers not assigned by kernel.org must be > 256
mknod /dev/detector c 190 0
mknod /dev/dect c 197 0
mknod /dev/dectdbg c 197 1
mknod /dev/fbond c 204 0
mknod /dev/bcmatm0 c 205 0
mknod /dev/bcmadsl0 c 208 0
mknod /dev/bcmadsl1 c 208 1
mknod /dev/slicslac c 209 1
mknod /dev/bcm c 212 0
mknod /dev/rgs_logger c 216 0
mknod /dev/spdsvc c 220 0
mknod /dev/ac97 c 222 0
mknod /dev/bcmprof c 224 0
mknod /dev/sysperf c 227 0
mknod /dev/bcmxtmcfg0 c 228 0
mknod /dev/bcmtm c 229 0
mknod /dev/bcmomcipm c 231 0
mknod /dev/bcm_user_ploam c 235 0
mknod /dev/bcm_omci c 236 0
mknod /dev/bcm_ploam c 237 0
mknod /dev/bcmvlan c 238 0
mknod /dev/laser_dev c 239 0
mknod /dev/pwrmngt c      3000 0
mknod /dev/bcmfap c       3001 0
mknod /dev/fcache c       3002 0
mknod /dev/ingqos c       3003 0
mknod /dev/bpm c          3004 0
mknod /dev/bcmarl c       3005 0
mknod /dev/chipinfo c     3006 0
mknod /dev/bcm_user_tod c 3007 0
mknod /dev/tms c          3008 0
mknod /dev/pktrunner c    3009 0
mknod /dev/otp c          3010 0
mknod /dev/ext_bonding c  3011 0
mknod /dev/ivi c          3015 0
# ADD NEW PROPIETARY DEVICE NODES ABOVE THIS LINE 

# Create Broadcom Opensource devices -- ALL MAJOR NUMBERS NOT ASSIGNED BY KERNEL.ORG MUST BE > 256
mknod /dev/btusb0 c 180 194
mknod /dev/bounce c 213 0
mknod /dev/pmon c 214 0
mknod /dev/bdmf_shell c 215 0
mknod /dev/opticaldetect c 230 0
mknod /dev/spu c 233 0
mknod /dev/bcmmoca0 c 234 0
mknod /dev/bcmmoca1 c 234 1
mknod /dev/bcmepon c 247 0
mknod /dev/gmac c 249 0
mknod /dev/buzzz c 253 0
mknod /dev/baloo c 254 0
# All NEW OPENSOURCE DRIVERS SHOULD CREATE SYSFS CLASS/DEVICE
# ENTRIES SO THAT MDEV CAN DYNAMICALLY CREATE DEVICE NODES 

