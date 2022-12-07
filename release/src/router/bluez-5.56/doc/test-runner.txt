Notes for test-runner usage
***************************


Kernel configuration
====================

The test-runner tool requires a kernel that is at least build with these
minimal options for a successful boot.

	CONFIG_VIRTIO=y
	CONFIG_VIRTIO_PCI=y

	CONFIG_NET=y
	CONFIG_INET=y

	CONFIG_NET_9P=y
	CONFIG_NET_9P_VIRTIO=y

	CONFIG_9P_FS=y
	CONFIG_9P_FS_POSIX_ACL=y

	CONFIG_SERIAL_8250=y
	CONFIG_SERIAL_8250_CONSOLE=y
	CONFIG_SERIAL_8250_PCI=y
	CONFIG_SERIAL_8250_NR_UARTS=4

	CONFIG_TMPFS=y
	CONFIG_TMPFS_POSIX_ACL=y
	CONFIG_TMPFS_XATTR=y

	CONFIG_DEVTMPFS=y
	CONFIG_DEBUG_FS=y

For Bluetooth functionality:

	CONFIG_BT=y
	CONFIG_BT_BREDR=y
	CONFIG_BT_RFCOMM=y
	CONFIG_BT_BNEP=y
	CONFIG_BT_HIDP=y
	CONFIG_BT_LE=y

	CONFIG_BT_HCIVHCI=y

	CONFIG_CRYPTO_CMAC=y
	CONFIG_CRYPTO_USER_API=y
	CONFIG_CRYPTO_USER_API_HASH=y
	CONFIG_CRYPTO_USER_API_SKCIPHER=y

	CONFIG_UNIX=y

	CONFIG_UHID=y


These options should be installed as .config in the kernel source directory
followed by this command.

	make olddefconfig

After that a default kernel with the required options can be built. More
option (like the Bluetooth subsystem) can be enabled on top of this.

Lock debuging
-------------

To catch locking related issues the following set of kernel config
options may be useful:

	CONFIG_LOCKDEP_SUPPORT=y
	CONFIG_DEBUG_SPINLOCK=y
	CONFIG_DEBUG_LOCK_ALLOC=y
	CONFIG_PROVE_LOCKING=y
	CONFIG_LOCKDEP=y
	CONFIG_DEBUG_MUTEXES=y
