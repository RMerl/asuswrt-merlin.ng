#!/bin/sh

echo ">>>>> Creating static device nodes <<<<<"

#Create standard character devices
mknod /dev/kmem c 1 2
mknod /dev/tty0 c 4 0
mknod /dev/tty1 c 4 1
mknod /dev/ttyH0 c 4 70
mknod /dev/ptm c 128 1
mknod /dev/printer0 c 180 0

# Create Broadcom Proprietary devices   ALL major numbers not assigned by kernel.org must be > 256
mknod /dev/dect c 3051 0
mknod /dev/dectdbg c 3051 1
mknod /dev/slicslac c 3050 1
mknod /dev/ac97 c 3060 0

mknod /dev/pwrmngt c        3000 0
mknod /dev/bcmfap c         3001 0
mknod /dev/fcache c         3002 0
mknod /dev/ingqos c         3003 0
mknod /dev/bpm c            3004 0
mknod /dev/bcmarl c         3005 0
mknod /dev/chipinfo c       3006 0
mknod /dev/bcm_user_tod c   3007 0
mknod /dev/tms c            3008 0
mknod /dev/pktrunner c      3009 0
mknod /dev/otp c            3010 0
mknod /dev/ext_bonding c    3011 0
mknod /dev/fbond c          3012 0
mknod /dev/bcmepon c        3013 0
mknod /dev/laser_dev c      3014 0
mknod /dev/ivi c            3015 0
mknod /dev/bcmomcipm c      3016 0
mknod /dev/bcm_user_ploam c 3017 0
mknod /dev/bcm_omci c       3018 0
mknod /dev/bcm_ploam c      3019 0
mknod /dev/opticaldetect c  3020 0
mknod /dev/bdmf_shell c     3021 0
mknod /dev/rgs_logger c     3022 0
mknod /dev/bcmvlan c        3023 0
mknod /dev/buzzz c          3024 0
mknod /dev/sysperf c        3025 0
mknod /dev/gmac c           3026 0
mknod /dev/bcmtm c          3027 0
mknod /dev/spdsvc c         3028 0
mknod /dev/bcmxtmcfg0 c     3029 0
mknod /dev/bcmprof c        3030 0
mknod /dev/bcm c            3031 0
mknod /dev/bcmatm0 c        3032 0
mknod /dev/bcmadsl0 c       3033 0
mknod /dev/bcmadsl1 c       3033 1
mknod /dev/detector c       3034 0
#node /dev/bcmrdpa c        3037 0 is created by rdpa_cmd.ko
mknod /dev/blog c           3038 0
mknod /dev/archer c         3039 0
mknod /dev/wantypedetect c  3040 0
mknod /dev/sotp c           3041 0

# ADD NEW PROPIETARY DEVICE NODES ABOVE THIS LINE -- ALL MAJOR NUMBERS NOT ASSIGNED BY KERNEL.ORG MUST BE > 256

# Create Broadcom Opensource devices -- ALL MAJOR NUMBERS NOT ASSIGNED BY KERNEL.ORG MUST BE > 256
mknod /dev/btusb0 c 180 194
# All NEW OPENSOURCE DRIVERS SHOULD CREATE SYSFS CLASS/DEVICE
# ENTRIES SO THAT MDEV CAN DYNAMICALLY CREATE DEVICE NODES 

