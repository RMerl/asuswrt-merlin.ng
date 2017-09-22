#!/bin/sh

unset LD_LIBRARY_PATH
unset LD_PRELOAD

echo "Info: Checking for prerequisites and creating folders..."

MARCH=$(uname -m)
OS=$(uname -o)
if [ $OS == ASUSWRT-Merlin -a $MARCH == aarch64 ]; then
   echo "Asuswrt-Merlin HND platform detected"
    location="/tmp/opt/"
else
    location="/opt/"
fi

if [ -d $location ]
then
    echo "Warning: Folder $location exists!"
else
    mkdir $location
fi
# no need to create many folders. entware-opt package creates most
for folder in bin etc lib/opkg tmp var/lock usr share doc include sbin root home libexec html
do
  if [ -d "$location$folder" ]
  then
    echo "Warning: Folder $location$folder exists!"
    echo "Warning: If something goes wrong please clean $location folder and try again."
  else
    mkdir -p $location$folder
  fi
done

echo "Info: Opkg package manager deployment..."
CURARCH="armv7"
DLOADER="ld-linux.so.3"
URL=http://entware-3x.zyxmon.org/binaries/$CURARCH/installer/2017
wget $URL/opkg -O /opt/bin/opkg
chmod 755 /opt/bin/opkg
wget $URL/opkg.conf -O /opt/etc/opkg.conf
wget $URL/ld-2.25.so -O /opt/lib/ld-2.25.so
wget $URL/libc-2.25.so -O /opt/lib/libc-2.25.so
wget $URL/libgcc_s.so.1 -O /opt/lib/libgcc_s.so.1
wget $URL/libpthread-2.25.so -O /opt/lib/libpthread-2.25.so
cd /opt/lib
chmod 755 ld-2.25.so
ln -s ld-2.25.so $DLOADER
ln -s libc-2.25.so libc.so.6
ln -s libpthread-2.25.so libpthread.so.0

echo "Info: Basic packages installation..."
/opt/bin/opkg update
/opt/bin/opkg install entware-opt
/opt/bin/opkg install opkg

# Fix for multiuser environment
chmod 777 /opt/tmp

# now try create symlinks - it is a std installation
if [ -f /etc/passwd ]
then
    ln -sf /etc/passwd /opt/etc/passwd
else
    cp /opt/etc/passwd.1 /opt/etc/passwd
fi

if [ -f /etc/group ]
then
    ln -sf /etc/group /opt/etc/group
else
    cp /opt/etc/group.1 /opt/etc/group
fi

if [ -f /etc/shells ]
then
    ln -sf /etc/shells /opt/etc/shells
else
    cp /opt/etc/shells.1 /opt/etc/shells
fi

if [ -f /etc/shadow ]
then
    ln -sf /etc/shadow /opt/etc/shadow
fi

if [ -f /etc/gshadow ]
then
    ln -sf /etc/gshadow /opt/etc/gshadow
fi

if [ -f /etc/localtime ]
then
    ln -sf /etc/localtime /opt/etc/localtime
fi

echo "Info: Congratulations!"
echo "Info: If there are no errors above then Entware-3x was successfully initialized."
echo "Info: Add /opt/bin & /opt/sbin to your PATH variable"
echo "Info: Add '/opt/etc/init.d/rc.unslung start' to startup script for Entware-3x services to start"
echo "Info: Found a Bug? Please report at https://github.com/Entware-for-kernel-3x/Entware-ng-3x/issues"

