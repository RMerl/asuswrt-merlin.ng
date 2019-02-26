#!/bin/sh

DIR="/etc/pts"
DISTS_DIR="$DIR/dists"
DATE=`date +%Y%m%d-%H%M`
UBUNTU="http://security.ubuntu.com/ubuntu"
UBUNTU_VERSIONS="bionic xenial"
UBUNTU_DIRS="main multiverse restricted universe"
UBUNTU_ARCH="binary-amd64"
DEBIAN="http://security.debian.org"
DEBIAN_VERSIONS="stretch jessie wheezy"
DEBIAN_DIRS="main contrib non-free"
DEBIAN_ARCH="binary-amd64 binary-armhf"
RASPIAN="http://archive.raspberrypi.org/debian"
RASPIAN_VERSIONS="jessie wheezy"
RASPIAN_DIRS="main"
RASPIAN_ARCH="binary-armhf"
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
      if [ $v = "stretch" ]
      then
        wget -nv $DEBIAN/dists/$v/updates/$d/$a/Packages.xz  -O $v-updates/$a/Packages-$d.xz
        unxz -f $v-updates/$a/Packages-$d.xz
      else
        wget -nv $DEBIAN/dists/$v/updates/$d/$a/Packages.bz2  -O $v-updates/$a/Packages-$d.bz2
        bunzip2 -f $v-updates/$a/Packages-$d.bz2
      fi
    done
  done
done

# Download Raspian distribution information

for v in $RASPIAN_VERSIONS
do
  for a in $RASPIAN_ARCH
  do
    mkdir -p $v-raspian/$a
    for d in $RASPIAN_DIRS
    do
      wget -nv $RASPIAN/dists/$v/$d/$a/Packages.gz  -O $v-raspian/$a/Packages-$d.gz
      gunzip -f $v-raspian/$a/Packages-$d.gz
    done
  done
done

# Run sec-updater in distribution information

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

for f in xenial-security/binary-amd64/*
do
  echo "security: $f"
  $CMD --os "Ubuntu 16.04" --arch "x86_64" --file $f --security \
       --uri $UBUNTU >> $CMD_LOG 2>&1
  if [ $? -eq 0 ]
  then
    DEL_LOG=0
  fi
done

for f in xenial-updates/binary-amd64/*
do
  echo "updates:  $f"
  $CMD --os "Ubuntu 16.04" --arch "x86_64" --file $f \
       --uri $UBUNTU >> $CMD_LOG 2>&1
  if [ $? -eq 0 ]
  then
    DEL_LOG=0
  fi
done

for f in stretch-updates/binary-amd64/*
do
  echo "security: $f"
  $CMD --os "Debian 9.0" --arch "x86_64" --file $f --security \
       --uri $DEBIAN >> $CMD_LOG 2>&1
  if [ $? -eq 0 ]
  then
    DEL_LOG=0
  fi
done

for f in jessie-updates/binary-amd64/*
do
  echo "security: $f"
  $CMD --os "Debian 8.0" --arch "x86_64" --file $f --security \
       --uri $DEBIAN >> $CMD_LOG 2>&1
  if [ $? -eq 0 ]
  then
    DEL_LOG=0
  fi
done

for f in wheezy-updates/binary-amd64/*
do
  echo "security: $f"
  $CMD --os "Debian 7.0" --arch "x86_64" --file $f --security \
       --uri $DEBIAN >> $CMD_LOG 2>&1
  if [ $? -eq 0 ]
  then
    DEL_LOG=0
  fi
done

for f in stretch-updates/binary-armhf/*
do
  echo "security: $f"
  $CMD --os "Debian 9.0" --arch "armhf" --file $f --security \
       --uri $DEBIAN >> $CMD_LOG 2>&1
  if [ $? -eq 0 ]
  then
    DEL_LOG=0
  fi
done

for f in jessie-updates/binary-armhf/*
do
  echo "security: $f"
  $CMD --os "Debian 8.0" --arch "armhf" --file $f --security \
       --uri $DEBIAN >> $CMD_LOG 2>&1
  if [ $? -eq 0 ]
  then
    DEL_LOG=0
  fi
done

for f in wheezy-updates/binary-armhf/*
do
  echo "security: $f"
  $CMD --os "Debian 7.0" --arch "armhf" --file $f --security \
       --uri $DEBIAN >> $CMD_LOG 2>&1
  if [ $? -eq 0 ]
  then
    DEL_LOG=0
  fi
done

for f in jessie-raspian/binary-armhf/*
do
  echo "security: $f"
  $CMD --os "Debian 8.0" --arch "armv7l" --file $f --security \
       --uri $RASPIAN >> $CMD_LOG 2>&1
  if [ $? -eq 0 ]
  then
    DEL_LOG=0
  fi
done

for f in wheezy-raspian/binary-armhf/*
do
  echo "security: $f"
  $CMD --os "Debian 7.11" --arch "armv7l" --file $f --security \
       --uri $RASPIAN >> $CMD_LOG 2>&1
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
