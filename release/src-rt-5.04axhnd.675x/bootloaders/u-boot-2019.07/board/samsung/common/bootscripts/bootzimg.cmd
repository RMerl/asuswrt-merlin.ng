setenv kernelname zImage;
setenv boot_kernel "setenv bootargs \"${console} root=/dev/mmcblk${mmcrootdev}p${mmcrootpart} rootfstype=${rootfstype} rootwait ${opts}\";
load mmc ${mmcbootdev}:${mmcbootpart} 0x40007FC0 '${kernelname}';
if load mmc ${mmcbootdev}:${mmcbootpart} 40800000 ${fdtfile}; then
	bootz 0x40007FC0 - 40800000;
else
	echo Warning! Booting without DTB: '${fdtfile}'!;
	bootz 0x40007FC0 -;
fi;"
run boot_kernel;