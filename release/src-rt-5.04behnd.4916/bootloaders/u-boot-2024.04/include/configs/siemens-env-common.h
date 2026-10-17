/* SPDX-License-Identifier: GPL-2.0+ */

/* Common env settings */

/** set_bootargs()
 * input:
 *	console:	string, tty, etc.
 *	baudrate:	string, tty baudrate
 *	testargs:	string
 *	optargs:	string
 * output:
 *	bootargs:	string, default boot string
 */
#define ENV_BOOTARGS_DEFAULT "set_bootargs=" \
	"setenv bootargs " \
		"console=${console} " \
		"${testargs} " \
		"${optargs}\0"

/** set_bootargs_net()
 * input:
 *	kernel_name:
 *	dtb_name:
 *	project_dir:
 * output:
 */
#define ENV_NET_FCT_NETARGS "set_bootargs_net=" \
	"run set_bootargs;" \
	"setenv bootfile ${project_dir}/boot/${kernel_name};" \
	"setenv bootdtb ${project_dir}/boot/${dtb_name_nfs}.dtb;" \
	"setenv rootpath /home/projects/${project_dir}/;" \
	"setenv bootargs ${bootargs} " \
		"root=/dev/nfs " \
		"nfsroot=${serverip}:${rootpath},${nfsopts} " \
		"ip=${ipaddr}:${serverip}:" \
			"${gatewayip}:${netmask}:${hostname}:eth0:off\0"

/** net_nfs()
 * input:
 * output:
 */
#define ENV_NET_FCT_BOOT "net_nfs=" \
	"echo Booting from network ...; " \
	"run set_bootargs_net; " \
	"tftpboot ${dtb_loadaddr} ${serverip}:${bootdtb};" \
	"if test $? -eq 1;" \
	"then " \
		"echo Loading default.dtb!;" \
		"tftpboot ${dtb_loadaddr} ${serverip}:${project_dir}/boot/${dtb_name_default}.dtb;" \
	"fi;" \
	"tftpboot ${kernel_loadaddr} ${serverip}:${bootfile};" \
	"printenv bootargs;" \
	"booti ${kernel_loadaddr} - ${dtb_loadaddr}\0"

/** check_update()
 * input:
 *	upgrade_available:	[0|1],	if set to 1 check bootcount variables
 *	bootcount:		int,	bootcount
 *	bootlimit:		int,	limit cootcount
 *	toggle_partition():	-	toggles active partition set
 * output:
 *	upgrade_available:	[0|1],	set to 0 if bootcount > bootlimit
 */
#define ENV_FCT_CHECK_UPGRADE "check_upgrade="\
	"if test ${upgrade_available} -eq 1; " \
	"then " \
		"echo  upgrade_available is set; " \
		"if test ${bootcount} -gt ${bootlimit}; " \
		"then " \
			"setenv upgrade_available 0;" \
			"echo toggle partition;" \
			"run toggle_partition;" \
		"fi;" \
	"fi;\0"

/** toggle_partition()
 * input:
 *	partitionset_active:	[A|B],	selected partition set
 * output:
 *	partitionset_active:	[A|B],	toggle
 */
#define ENV_FCT_TOGGLE_PARTITION "toggle_partition="\
	"setenv ${partitionset_active} true;" \
	"if test -n ${A}; " \
	"then " \
		"setenv partitionset_active B; " \
		"env delete A; " \
	"fi;" \
	"if test -n ${B}; "\
	"then " \
		"setenv partitionset_active A; " \
		"env delete B; " \
	"fi;" \
	"saveenv\0"

/** set_partition()
 * input:
 *	partitionset_active:	[A|B],	selected partition set
 *	rootfs_name:		string,	mmc device file in kernel, e.g. /dev/mmcblk0
 * output:
 *	mmc_active_vol:	string,	mmc partition device file in kernel, e.g. /dev/mmcblk0p2
 *	mmc_part_nr:		int,	partition number of mmc, e.g. /dev/mmcblk0p2 --> 2
 */
#define ENV_EMMC_FCT_SET_ACTIVE_PARTITION "set_partition=" \
	"setenv ${partitionset_active} true;" \
	"if test -n ${A}; " \
	"then " \
		"setenv mmc_part_nr 1;" \
	"fi;" \
	"if test -n ${B}; " \
	"then " \
		"setenv mmc_part_nr 2;" \
	"fi;" \
	"setenv mmc_active_vol ${rootfs_name}p${mmc_part_nr} \0"

/** set_bootargs_mmc()
 * input:
 *	bootargs:		string, default bootargs
 *	mmc_active_vol		string, mmc partition device file in kernel, e.g. /dev/mmcblk0p2
 *	ip_method:		string, [none|?]
 * output:
 *	bootargs:		string
 */
#define ENV_EMMC_FCT_SET_EMMC_BOOTARGS "set_bootargs_mmc=" \
	"setenv bootargs ${bootargs} " \
		"root=${mmc_active_vol} rw " \
		"rootdelay=1 rootwait " \
		"rootfstype=ext4 " \
		"ip=${ip_method} \0"

/** mmc_load_bootfiles()
 * input:
 *	mmc_part_nr:
 *	dtb_loadaddr:
 *	dtb_name:
 *	kernel_loadaddr:
 *	kernel_name:
 */
#define ENV_EMMC_FCT_LOADFROM_EMMC "mmc_load_bootfiles=" \
	"echo Loading from eMMC ...;" \
	"ext4load mmc 0:${mmc_part_nr} ${dtb_loadaddr} boot/${dtb_name}.dtb;" \
	"if test $? -eq 1;" \
	"then " \
		"echo Loading default.dtb!;" \
		"ext4load mmc 0:${mmc_part_nr} ${dtb_loadaddr} boot/${dtb_name_default}.dtb;" \
	"fi;" \
	"ext4load mmc 0:${mmc_part_nr} ${kernel_loadaddr} boot/${kernel_name};" \
	"printenv bootargs;\0"

/** mmc_boot()
 * input:
 *	mmc_part_nr:
 *	dtb_loadaddr:
 *	dtb_name:
 *	kernel_loadaddr:
 *	kernel_name:
 */
#define ENV_EMMC_FCT_EMMC_BOOT "mmc_boot=" \
	"run set_bootargs;" \
	"run check_upgrade; " \
	"run set_partition;" \
	"run set_bootargs_mmc;" \
	"run mmc_load_bootfiles;" \
	"echo Booting from eMMC ...; " \
	"booti ${kernel_loadaddr} - ${dtb_loadaddr} \0"

#define ENV_EMMC_ALIASES "" \
	"flash_self=run mmc_boot\0" \
	"flash_self_test=setenv testargs test; " \
		"run mmc_boot\0"

#define ENV_COMMON "" \
	"project_dir=targetdir/rootfs\0" \
	"serverip=192.168.251.2\0" \
	"ipaddr=192.168.251.1\0" \
	"dtb_name_nfs=default\0" \
	"dtb_name_default=default\0" \
	"kernel_name=Image\0" \
	"partitionset_active=A\0" \
	"dtb_loadaddr=0x83000000\0" \
	"kernel_loadaddr=0x80280000\0" \
	"ip_method=none\0" \
	"rootfs_name=/dev/mmcblk0\0" \
	"upgrade_available=0\0" \
	"bootlimit=3\0" \
	"altbootcmd=run bootcmd\0" \
	"optargs=\0" \

/**********************************************************************/

#define ENV_EMMC	ENV_EMMC_FCT_EMMC_BOOT \
			ENV_EMMC_FCT_LOADFROM_EMMC \
			ENV_EMMC_FCT_SET_EMMC_BOOTARGS \
			ENV_EMMC_FCT_SET_ACTIVE_PARTITION \
			ENV_FCT_CHECK_UPGRADE \
			ENV_EMMC_ALIASES \
			ENV_FCT_TOGGLE_PARTITION

#define ENV_NET		ENV_NET_FCT_BOOT \
			ENV_NET_FCT_NETARGS \
			ENV_BOOTARGS_DEFAULT
