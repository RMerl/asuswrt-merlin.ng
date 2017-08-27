#!/bin/sh

DIR="/etc/pts"
DATE=`date +%Y%m%d-%H%M`
UBUNTU="http://security.ubuntu.com/ubuntu/dists"
UBUNTU_VERSIONS="trusty saucy raring quantal precise lucid"
UBUNTU_DIRS="main multiverse restricted universe"
UBUNTU_ARCH="binary-amd64 binary-i386"
DEBIAN="http://security.debian.org/dists"
DEBIAN_VERSIONS="jessie wheezy squeeze"
DEBIAN_DIRS="main contrib non-free"
DEBIAN_ARCH="binary-amd64 binary-i386"
PACMAN=/usr/libexec/ipsec/pacman
PACMAN_LOG="$DIR/$DATE-pacman.log"

mkdir -p $DIR/dists
cd $DIR/dists

for v in $UBUNTU_VERSIONS
do
  for a in $UBUNTU_ARCH
  do
    mkdir -p $v-security/$a $v-updates/$a
    for d in $UBUNTU_DIRS
    do
  	  wget $UBUNTU/$v-security/$d/$a/Packages.bz2 -O $v-security/$a/Packages-$d.bz2
      bunzip2 -f $v-security/$a/Packages-$d.bz2
  	  wget $UBUNTU/$v-updates/$d/$a/Packages.bz2  -O $v-updates/$a/Packages-$d.bz2
      bunzip2 -f $v-updates/$a/Packages-$d.bz2
	done
  done
done

for v in $DEBIAN_VERSIONS
do
  for a in $DEBIAN_ARCH
  do
    mkdir -p $v-updates/$a
    for d in $DEBIAN_DIRS
    do
  	  wget $DEBIAN/$v/updates/$d/$a/Packages.bz2  -O $v-updates/$a/Packages-$d.bz2
      bunzip2 -f $v-updates/$a/Packages-$d.bz2
	done
  done
done

for f in trusty-security/binary-amd64/*
do
  $PACMAN --product "Ubuntu 14.04 x86_64" --file $f --security >> $PACMAN_LOG
done
echo
for f in trusty-updates/binary-amd64/*
do
  $PACMAN --product "Ubuntu 14.04 x86_64" --file $f >> $PACMAN_LOG
done
echo
for f in trusty-security/binary-i386/*
do
  $PACMAN --product "Ubuntu 14.04 i686" --file $f --security >> $PACMAN_LOG
done
echo
for f in trusty-updates/binary-i386/*
do
  $PACMAN --product "Ubuntu 14.04 i686" --file $f >> $PACMAN_LOG
done
echo

for f in saucy-security/binary-amd64/*
do
  $PACMAN --product "Ubuntu 13.10 x86_64" --file $f --security >> $PACMAN_LOG
done
echo
for f in saucy-updates/binary-amd64/*
do
  $PACMAN --product "Ubuntu 13.10 x86_64" --file $f >> $PACMAN_LOG
done
echo
for f in saucy-security/binary-i386/*
do
  $PACMAN --product "Ubuntu 13.10 i686" --file $f --security >> $PACMAN_LOG
done
echo
for f in saucy-updates/binary-i386/*
do
  $PACMAN --product "Ubuntu 13.10 i686" --file $f >> $PACMAN_LOG
done
echo

for f in raring-security/binary-amd64/*
do
  $PACMAN --product "Ubuntu 13.04 x86_64" --file $f --security >> $PACMAN_LOG
done
echo
for f in raring-updates/binary-amd64/*
do
  $PACMAN --product "Ubuntu 13.04 x86_64" --file $f >> $PACMAN_LOG
done
echo
for f in raring-security/binary-i386/*
do
  $PACMAN --product "Ubuntu 13.04 i686" --file $f --security >> $PACMAN_LOG
done
echo
for f in raring-updates/binary-i386/*
do
  $PACMAN --product "Ubuntu 13.04 i686" --file $f >> $PACMAN_LOG
done
echo

for f in quantal-security/binary-amd64/*
do
  $PACMAN --product "Ubuntu 12.10 x86_64" --file $f --security >> $PACMAN_LOG
done
echo
for f in quantal-updates/binary-amd64/*
do
  $PACMAN --product "Ubuntu 12.10 x86_64" --file $f >> $PACMAN_LOG
done
echo
for f in quantal-security/binary-i386/*
do
  $PACMAN --product "Ubuntu 12.10 i686" --file $f --security >> $PACMAN_LOG
done
echo
for f in quantal-updates/binary-i386/*
do
  $PACMAN --product "Ubuntu 12.10 i686" --file $f >> $PACMAN_LOG
done
echo

for f in precise-security/binary-amd64/*
do
  $PACMAN --product "Ubuntu 12.04 x86_64" --file $f --security >> $PACMAN_LOG
done
echo
for f in precise-updates/binary-amd64/*
do
  $PACMAN --product "Ubuntu 12.04 x86_64" --file $f >> $PACMAN_LOG
done
echo
for f in precise-security/binary-i386/*
do
  $PACMAN --product "Ubuntu 12.04 i686" --file $f --security >> $PACMAN_LOG
done
echo
for f in precise-updates/binary-i386/*
do
  $PACMAN --product "Ubuntu 12.04 i686" --file $f >> $PACMAN_LOG
done
echo

for f in lucid-security/binary-amd64/*
do
  $PACMAN --product "Ubuntu 10.04 x86_64" --file $f --security >> $PACMAN_LOG
done
echo
for f in lucid-updates/binary-amd64/*
do
  $PACMAN --product "Ubuntu 10.04 x86_64" --file $f >> $PACMAN_LOG
done
echo
for f in lucid-security/binary-i386/*
do
  $PACMAN --product "Ubuntu 10.04 i686" --file $f --security >> $PACMAN_LOG
done
echo
for f in lucid-updates/binary-i386/*
do
  $PACMAN --product "Ubuntu 10.04 i686" --file $f >> $PACMAN_LOG
done
echo

for f in jessie-updates/binary-amd64/*
do
  $PACMAN --product "Debian 8.0 x86_64" --file $f --security >> $PACMAN_LOG
done
echo
for f in jessie-updates/binary-i386/*
do
  $PACMAN --product "Debian 8.0 i686" --file $f --security >> $PACMAN_LOG
done

for f in wheezy-updates/binary-amd64/*
do
  $PACMAN --product "Debian 7.0 x86_64" --file $f --security >> $PACMAN_LOG
done
echo
for f in wheezy-updates/binary-i386/*
do
  $PACMAN --product "Debian 7.0 i686" --file $f --security >> $PACMAN_LOG
done

for f in squeeze-updates/binary-amd64/*
do
  $PACMAN --product "Debian 6.0 x86_64" --file $f --security >> $PACMAN_LOG
done
echo
for f in squeeze-updates/binary-i386/*
do
  $PACMAN --product "Debian 6.0 i686" --file $f --security >> $PACMAN_LOG
done

cp $DIR/config.db $DIR/config.db-$DATE
