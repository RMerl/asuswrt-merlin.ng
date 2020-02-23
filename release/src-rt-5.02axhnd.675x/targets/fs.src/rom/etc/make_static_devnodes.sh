#!/bin/sh

echo ">>>>> Creating static device nodes <<<<<"

#Create standard character devices
mknod /dev/kmem c 1 2
mknod /dev/tty0 c 4 0
mknod /dev/tty1 c 4 1
mknod /dev/ttyH0 c 4 70
mknod /dev/ptm c 128 1
mknod /dev/printer0 c 180 0

# Create Broadcom Proprietary devices. Major numbers must fall in the range 256-512
mknod /dev/pwrmngt c        300 0
mknod /dev/bcmfap c         301 0
mknod /dev/fcache c         302 0
mknod /dev/ingqos c         303 0
mknod /dev/bpm c            304 0
mknod /dev/bcmarl c         305 0
mknod /dev/chipinfo c       306 0
mknod /dev/bcm_user_tod c   307 0
mknod /dev/tms c            308 0
mknod /dev/pktrunner c      309 0
mknod /dev/otp c            310 0
mknod /dev/ext_bonding c    311 0
mknod /dev/fbond c          312 0
mknod /dev/bcmepon c        313 0
mknod /dev/laser_dev c      314 0
mknod /dev/bcmomcipm c      316 0
mknod /dev/bcm_user_ploam c 317 0
mknod /dev/bcm_omci c       318 0
mknod /dev/bcm_ploam c      319 0
mknod /dev/opticaldetect c  320 0
mknod /dev/bdmf_shell c     321 0
mknod /dev/rgs_logger c     322 0
mknod /dev/bcmvlan c        323 0
mknod /dev/buzzz c          324 0
mknod /dev/sysperf c        325 0
mknod /dev/gmac c           326 0
mknod /dev/bcmtm c          327 0
mknod /dev/spdsvc c         328 0
mknod /dev/bcmxtmcfg0 c     329 0
mknod /dev/bcmprof c        330 0
mknod /dev/bcm c            331 0
mknod /dev/bcmatm0 c        332 0
mknod /dev/bcmadsl0 c       333 0
mknod /dev/bcmadsl1 c       333 1
mknod /dev/detector c       334 0 
#node /dev/bcmrdpa c        337 0 is created by rdpa_cmd.ko
mknod /dev/blog c           338 0
mknod /dev/archer c         339 0
mknod /dev/wantypedetect c  340 0
mknod /dev/sotp c           341 0
mknod /dev/slicslac c       350 1
mknod /dev/dect c           351 0
mknod /dev/dectdbg c        351 1
mknod /dev/ac97 c           360 0
# ADD NEW PROPIETARY DEVICE NODES ABOVE THIS LINE -- ALL PROPRIETARY MAJOR NUMBERS MUST BE IN THE RANGE 300-512

# All NEW OPENSOURCE DRIVERS SHOULD CREATE SYSFS CLASS/DEVICE
# ENTRIES SO THAT MDEV CAN DYNAMICALLY CREATE DEVICE NODES 

