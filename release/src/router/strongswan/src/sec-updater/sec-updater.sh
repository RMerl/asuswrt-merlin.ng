#!/bin/sh

DIR="/etc/pts"
DISTS_DIR="$DIR/dists"
DATE=`date +%Y%m%d-%H%M`
UBUNTU="http://security.ubuntu.com/ubuntu"
UBUNTU_VERSIONS="focal bionic"
UBUNTU_DIRS="main multiverse restricted universe"
UBUNTU_ARCH="binary-amd64"
DEBIAN="http://security.debian.org"
DEBIAN_VERSIONS="buster"
DEBIAN_DIRS="main contrib non-free"
DEBIAN_ARCH="binary-amd64 binary-armhf"
RASPBIAN="http://archive.raspberrypi.org/debian"
RASPBIAN_VERSIONS="buster"
RASPBIAN_DIRS="main"
RASPBIAN_ARCH="binary-armhf"
CMD=/usr/sbin/sec-updater
CMD_LOG="$DIR/logs/$DATE-sec-update.log"
DEL_LOG=1

mkdir -p $DIR/dists
cd $DIR/dists

# Download Ubuntu distribution information

for v in $UBUNTU_VERSIONS
do
  for a in $UBUNTU_ARCH
  do
    mkdir -p $v-security/$a $v-updates/$a
    for d in $UBUNTU_DIRS
    do
      wget -nv $UBUNTU/dists/$v-security/$d/$a/Packages.xz -O $v-security/$a/Packages-$d.xz
      unxz -f $v-security/$a/Packages-$d.xz
      wget -nv $UBUNTU/dists/$v-updates/$d/$a/Packages.xz  -O $v-updates/$a/Packages-$d.xz
      unxz -f $v-updates/$a/Packages-$d.xz
    done
  done
done

# Download Debian distribution information

for v in $DEBIAN_VERSIONS
do
  for a in $DEBIAN_ARCH
  do
    mkdir -p $v-updates/$a
    for d in $DEBIAN_DIRS
    do
      wget -nv $DEBIAN/dists/$v/updates/$d/$a/Packages.xz  -O $v-updates/$a/Packages-$d.xz
      unxz -f $v-updates/$a/Packages-$d.xz
    done
  done
done

# Download Raspbian distribution information

for v in $RASPBIAN_VERSIONS
do
  for a in $RASPBIAN_ARCH
  do
    mkdir -p $v-raspbian/$a
    for d in $RASPBIAN_DIRS
    do
      wget -nv $RASPBIAN/dists/$v/$d/$a/Packages.gz  -O $v-raspbian/$a/Packages-$d.gz
      gunzip -f $v-raspbian/$a/Packages-$d.gz
    done
  done
done

# Run sec-updater in distribution information

for f in focal-security/binary-amd64/*
do
  echo "security: $f"
  $CMD --os "Ubuntu 20.04" --arch "x86_64" --file $f --security \
       --uri $UBUNTU >> $CMD_LOG 2>&1
  if [ $? -eq 0 ]
  then
    DEL_LOG=0
  fi
done

for f in focal-updates/binary-amd64/*
do
  echo "updates:  $f"
  $CMD --os "Ubuntu 20.04" --arch "x86_64" --file $f \
       --uri $UBUNTU >> $CMD_LOG 2>&1
  if [ $? -eq 0 ]
  then
    DEL_LOG=0
  fi
done

for f in bionic-security/binary-amd64/*
do
  echo "security: $f"
  $CMD --os "Ubuntu 18.04" --arch "x86_64" --file $f --security \
       --uri $UBUNTU >> $CMD_LOG 2>&1
  if [ $? -eq 0 ]
  then
    DEL_LOG=0
  fi
done

for f in bionic-updates/binary-amd64/*
do
  echo "updates:  $f"
  $CMD --os "Ubuntu 18.04" --arch "x86_64" --file $f \
       --uri $UBUNTU >> $CMD_LOG 2>&1
  if [ $? -eq 0 ]
  then
    DEL_LOG=0
  fi
done

for f in buster-updates/binary-amd64/*
do
  echo "security: $f"
  $CMD --os "Debian 10" --arch "x86_64" --file $f --security \
       --uri $DEBIAN >> $CMD_LOG 2>&1
  if [ $? -eq 0 ]
  then
    DEL_LOG=0
  fi
done

for f in buster-updates/binary-armhf/*
do
  echo "security: $f"
  $CMD --os "Debian 10" --arch "armhf" --file $f --security \
       --uri $DEBIAN >> $CMD_LOG 2>&1
  if [ $? -eq 0 ]
  then
    DEL_LOG=0
  fi
done

for f in buster-raspbian/binary-armhf/*
do
  echo "security: $f"
  $CMD --os "Raspbian 10" --arch "armhf" --file $f --security \
       --uri $RASPBIAN >> $CMD_LOG 2>&1
  if [ $? -eq 0 ]
  then
    DEL_LOG=0
  fi
done

# Delete log file if no security updates were found

if [ $DEL_LOG -eq 1 ]
then
  rm $CMD_LOG
  echo "no security updates found"
fi
