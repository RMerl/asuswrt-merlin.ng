# This is an example file to generate boot.scr - a boot script for U-Boot
# Generate boot.scr:
# ./tools/mkimage -c none -A arm -T script -d autoboot.cmd boot.scr
#
# It requires a list of environment variables to be defined before load:
# platform dependent: board_name, fdtfile, console
# system dependent: mmcbootdev, mmcbootpart, mmcrootdev, mmcrootpart, rootfstype
#
setenv fdtaddr     "40800000"
setenv initrdname  "uInitrd"
setenv initrdaddr  "42000000"
setenv loaddtb     "load mmc ${mmcbootdev}:${mmcbootpart} ${fdtaddr} ${fdtfile}"
setenv loadinitrd  "load mmc ${mmcbootdev}:${mmcbootpart} ${initrdaddr} ${initrdname}"
setenv loadkernel  "load mmc ${mmcbootdev}:${mmcbootpart} '${kerneladdr}' '${kernelname}'"
setenv kernel_args "setenv bootargs ${console} root=/dev/mmcblk${mmcrootdev}p${mmcrootpart} rootfstype=${rootfstype} rootwait ${opts}"

#### Routine: check_dtb - check that target.dtb exists on boot partition
setenv check_dtb "
if test -e mmc '${mmcbootdev}':'${mmcbootpart}' '${fdtfile}'; then
	run loaddtb;
	setenv fdt_addr ${fdtaddr};
else
	echo Warning! Booting without DTB: '${fdtfile}'!;
	setenv fdt_addr;
fi;"

#### Routine: check_ramdisk - check that uInitrd exists on boot partition
setenv check_ramdisk "
if test -e mmc '${mmcbootdev}':'${mmcbootpart}' '${initrdname}'; then
	echo "Found ramdisk image.";
	run loadinitrd;
	setenv initrd_addr ${initrdaddr};
else
	echo Warning! Booting without RAMDISK: '${initrdname}'!;
	setenv initrd_addr -;
fi;"

#### Routine: boot_fit - check that env $board_name is set and boot proper config of ITB image
setenv setboot_fit "
if test -e '${board_name}'; then
	setenv fdt_addr ;
	setenv initrd_addr ;
	setenv kerneladdr  0x42000000;
	setenv kernelname  Image.itb;
	setenv itbcfg      "\"#${board_name}\"";
	setenv imgbootcmd  bootm;
else
	echo Warning! Variable: \$board_name is undefined!;
fi"

#### Routine: setboot_uimg - prepare env to boot uImage
setenv setboot_uimg "
	setenv kerneladdr 0x40007FC0;
	setenv kernelname uImage;
	setenv itbcfg     ;
	setenv imgbootcmd bootm;
	run check_dtb;
	run check_ramdisk;"

#### Routine: setboot_zimg - prepare env to boot zImage
setenv setboot_zimg "
	setenv kerneladdr 0x40007FC0;
	setenv kernelname zImage;
	setenv itbcfg     ;
	setenv imgbootcmd bootz;
	run check_dtb;
	run check_ramdisk;"

#### Routine: boot_img - boot the kernel after env setup
setenv boot_img "
	run loadkernel;
	run kernel_args;
	'${imgbootcmd}' '${kerneladdr}${itbcfg}' '${initrd_addr}' '${fdt_addr}';"

#### Routine: autoboot - choose proper boot path
setenv autoboot "
if test -e mmc ${mmcbootdev}:${mmcbootpart} Image.itb; then
	echo Found kernel image: Image.itb;
	run setboot_fit;
	run boot_img;
elif test -e mmc ${mmcbootdev}:${mmcbootpart} zImage; then
	echo Found kernel image: zImage;
	run setboot_zimg;
	run boot_img;
elif test -e mmc ${mmcbootdev}:${mmcbootpart} uImage; then
	echo Found kernel image: uImage;
	run setboot_uimg;
	run boot_img;
fi;"

#### Execute the defined autoboot macro
run autoboot
