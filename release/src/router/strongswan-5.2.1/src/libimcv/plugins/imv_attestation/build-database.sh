#!/bin/sh

p="Ubuntu 14.04 x86_64"
a="x86_64-linux-gnu"
k="3.13.0-37-generic"

for hash in sha1 sha256
do
  ipsec attest --add --product "$p" --$hash --dir  /sbin
  ipsec attest --add --product "$p" --$hash --dir  /usr/sbin
  ipsec attest --add --product "$p" --$hash --dir  /bin
  ipsec attest --add --product "$p" --$hash --dir  /usr/bin

  ipsec attest --add --product "$p" --$hash --file /etc/init.d/rc
  ipsec attest --add --product "$p" --$hash --file /etc/init.d/rcS
  ipsec attest --add --product "$p" --$hash --dir  /etc/network/if-pre-up.d
  ipsec attest --add --product "$p" --$hash --dir  /etc/network/if-up.d
  ipsec attest --add --product "$p" --$hash --dir  /etc/ppp/ip-down.d
  ipsec attest --add --product "$p" --$hash --dir  /etc/rcS.d
  ipsec attest --add --product "$p" --$hash --dir  /etc/rc2.d
  ipsec attest --add --product "$p" --$hash --file /etc/rc.local
  ipsec attest --add --product "$p" --$hash --dir  /etc/resolvconf/update.d
  ipsec attest --add --product "$p" --$hash --file /etc/resolvconf/update-libc.d/avahi-daemon
  ipsec attest --add --product "$p" --$hash --dir  /etc/update-motd.d

  ipsec attest --add --product "$p" --$hash --dir  /lib
  ipsec attest --add --product "$p" --$hash --file /lib/crda/setregdomain
  ipsec attest --add --product "$p" --$hash --dir  /lib/ebtables
  ipsec attest --add --product "$p" --$hash --file /lib/init/apparmor-profile-load
  ipsec attest --add --product "$p" --$hash --file /lib/resolvconf/list-records
  ipsec attest --add --product "$p" --$hash --dir  /lib/ufw
  ipsec attest --add --product "$p" --$hash --dir  /lib/udev
  ipsec attest --add --product "$p" --$hash --dir  /lib/systemd
  ipsec attest --add --product "$p" --$hash --dir  /lib/xtables
  ipsec attest --add --product "$p" --$hash --dir  /lib/$a
  ipsec attest --add --product "$p" --$hash --dir  /lib/$a/plymouth
  ipsec attest --add --product "$p" --$hash --dir  /lib/$a/plymouth/renderers
  ipsec attest --add --product "$p" --$hash --dir  /lib/$a/security

  ipsec attest --add --product "$p" --$hash --file /lib64/ld-linux-x86-64.so.2

  for file in `find /usr/lib -name *.so`
  do
    ipsec attest --add --product "$p" --$hash --file $file
  done

  for file in `find /usr/lib -name *service`
  do
    ipsec attest --add --product "$p" --$hash --file $file
  done

  ipsec attest --add --product "$p" --$hash --dir  /usr/lib
  ipsec attest --add --product "$p" --$hash --dir  /usr/lib/accountsservice
  ipsec attest --add --product "$p" --$hash --dir  /usr/lib/at-spi2-core
  ipsec attest --add --product "$p" --$hash --file /usr/lib/avahi/avahi-daemon-check-dns.sh
  ipsec attest --add --product "$p" --$hash --file /usr/lib/dbus-1.0/dbus-daemon-launch-helper
  ipsec attest --add --product "$p" --$hash --dir  /usr/lib/gvfs
  ipsec attest --add --product "$p" --$hash --file /usr/lib/firefox/firefox
  ipsec attest --add --product "$p" --$hash --dir  /usr/lib/NetworkManager
  ipsec attest --add --product "$p" --$hash --dir  /usr/lib/pm-utils/power.d
  ipsec attest --add --product "$p" --$hash --file /usr/lib/policykit-1/polkitd
  ipsec attest --add --product "$p" --$hash --file /usr/lib/thunderbird/thunderbird
  ipsec attest --add --product "$p" --$hash --dir  /usr/lib/ubuntu-release-upgrader
  ipsec attest --add --product "$p" --$hash --dir  /usr/lib/update-notifier

  ipsec attest --add --product "$p" --$hash --dir  /usr/lib/$a
  ipsec attest --add --product "$p" --$hash --file /usr/lib/$a/mesa/libGL.so.1.2.0
  ipsec attest --add --product "$p" --$hash --dir  /usr/lib/$a/samba
  ipsec attest --add --product "$p" --$hash --dir  /usr/lib/$a/sasl2

  ipsec attest --add --product "$p" --$hash --dir  /usr/share/language-tools

  ipsec attest --add --product "$p" --$hash --file /init \
                     --measdir /usr/share/initramfs-tools

  ipsec attest --add --product "$p" --$hash --file /scripts/functions \
                     --measdir /usr/share/initramfs-tools/scripts

  for file in `find /lib/modules/$k -name *.ko`
  do
    ipsec attest --add --product "$p" --$hash --file $file
  done
done

