#!/bin/sh

# list of SATA/AHCI and usb modules (watch out depends)
# in targets/fs.src/etc/init.d/bcm-base-drivers.list
mod='btusbdrv uas usb-storage usblp xhci-plat-hcd xhci-hcd '
mod=$mod'ohci-platform ohci-pci ohci-hcd ehci-platform ehci-pci ehci-hcd usbcore usb-common '
mod=$mod'ahci_platform libahci_platform ahci libahci libata '
mod=$mod'bcm_usb bcm_sata '

case $1 in
suspend)
	grep ^/dev/sd /proc/mounts | while read d r t o; do
		umount $r || umount -f $r
	done
	while read h s c c i i l l; do
		case $h in Host:)
		echo scsi remove-single-device ${s#scsi} $c $i $l > /proc/scsi/scsi ;;
		esac
	done < /proc/scsi/scsi
	for m in ${mod//-/_}; do
		grep -e"^\<$m\>" /proc/modules && rmmod -w $m && echo $m removed;
	done
	;;
resume)
	grep -e${mod// /.ko -e}KERNELVER= /rom/etc/init.d/bcm-base-drivers.sh | while read i f; do
		m=${f##*/} && m=${m%.ko} && m=${m//-/_};
		if grep -e"^\<$m\>" /proc/modules; then
			echo $m already loaded;
		else
			$i $f && echo $m inserted;
		fi
	done
	;;
modules)
    for m in ${mod//-/_}; do grep -e"^\<$m\>" /proc/modules; done
	;;
esac
