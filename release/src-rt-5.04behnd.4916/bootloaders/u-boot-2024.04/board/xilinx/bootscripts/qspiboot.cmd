# This is an example file to generate boot.scr - a boot script for U-Boot
# This example only target for qspi boot, sameway it can be created for boot
# devices like nand.
# Generate boot.scr:
# ./tools/mkimage -c none -A arm -T script -d qspiboot.cmd boot.scr
#
# It requires a list of environment variables to be defined before load:
# fdt_addr, fdt_offset, fdt_size, kernel_addr, kernel_offset, kernel_size
#
sf probe 0 0 0 && sf read $fdt_addr $fdt_offset $fdt_size && sf read $kernel_addr $kernel_offset $kernel_size && booti $kernel_addr - $fdt_addr
