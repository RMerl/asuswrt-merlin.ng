# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2018
# Lukasz Majewski, DENX Software Engineering, lukma@denx.de
# This is an example file to generate boot.scr - a boot script for U-Boot
# Generate boot.scr:
# ./tools/mkimage -c none -A arm -T script -d tpcboot.cmd boot.scr
#


# Input envs (to be set in environment)
# Mandatory:
# kernel_file = "fitImage"
# boardname = "XXXX"  // set automatically in u-boot
# boardsoc = "imx6q"  // set automatically in u-boot
#
# Optional:
# bootcmd_force = "nfs" "tftp_kernel"
# If not set - eMMC/SD boot

# Generic setup
setenv mmcroot "/dev/mmcblk${devnum}p2 rootwait rw"
setenv displayargs ""
setenv mmcargs "setenv bootargs console=${console} ${smp} root=${mmcroot} \
	${displayargs}"
setenv miscadj "
if test '${boardsoc}' = 'imx53'; then
       setenv bootargs '${bootargs} di=${dig_in} key1=${key1}';
fi;"
setenv nfsadj "
if test '${boardsoc}' = 'imx53'; then
   if test '${boardtype}' = 'hsc'; then
       setenv bootargs '${bootargs} dsa_core.blacklist=yes';
   fi;
fi;"
setenv boot_fitImage "
	setenv fdt_conf 'conf@${boardsoc}-${boardname}.dtb';
	setenv itbcfg "\"#\${fdt_conf}\"";
	print itbcfg;
	bootm '${loadaddr}${itbcfg}';"

#------------------------------------------------------------
#
# Provide default 'bootcmd' command
#------------------------------------------------------------
setenv bootcmd "
if test -e ${devtype} ${devnum}:${distro_bootpart} ${kernel_file}; then
	echo Found kernel image: ${kernel_file};
	if load ${devtype} ${devnum}:${distro_bootpart} ${loadaddr} \
	   ${kernel_file}; then
		run mmcargs;
		run miscadj;
		run boot_fitImage;
	fi;
fi;"

#------------------------------------------------------------
#
# Provide 'boot_tftp_kernel' command
#------------------------------------------------------------
setenv download_kernel "dhcp ${loadaddr} ${kernel_file}"

setenv boot_tftp_kernel "
if run download_kernel; then
	run mmcargs;
	run miscadj;
	run boot_fitImage;
fi"

#------------------------------------------------------------
#
# Provide 'boot_nfs' command
#------------------------------------------------------------
setenv nfsargs "setenv bootargs root=/dev/nfs rw nfsroot='${rootpath}',nolock,nfsvers=3"
setenv addip "setenv bootargs '${bootargs}' ip='${ipaddr}':'${serverip}':'${gatewayip}':'${netmask}':'${hostname}':eth0:on"

setenv boot_nfs "
if run download_kernel; then
	run nfsargs;
	run addip;
	run nfsadj;
	setenv bootargs '${bootargs}' console=${console};

	run boot_fitImage;
fi"

#------------------------------------------------------------
#
# Set correct boot flow
#------------------------------------------------------------

setenv bcmd "
if test ! -n ${bootcmd_force}; then
	run bootcmd;
fi;
if test ${bootcmd_force} = nfs; then
	run boot_nfs;
else if test ${bootcmd_force} = tftp_kernel; then
	run boot_tftp_kernel;
     fi;
fi"

run bcmd
