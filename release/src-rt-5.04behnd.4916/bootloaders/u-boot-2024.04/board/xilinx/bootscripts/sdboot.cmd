# This is an example file to generate boot.scr - a boot script for U-Boot
# This example only target for qspi boot, sameway it can be created for boot
# devices like nand.
# Generate boot.scr:
# ./tools/mkimage -c none -A arm -T script -d sdboot.cmd boot.scr
#
# It requires a list of environment variables used below to be defined
# before load
#
mmc dev $devnum && mmcinfo && run uenvboot || run sdroot$devnum;load mmc $devnum:$partid $fdt_addr system.dtb && load mmc $devnum:$partid $kernel_addr Image && booti $kernel_addr - $fdt_addr
