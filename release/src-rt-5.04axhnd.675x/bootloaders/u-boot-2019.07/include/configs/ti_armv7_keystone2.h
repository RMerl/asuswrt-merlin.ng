/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common configuration header file for all Keystone II EVM platforms
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef __CONFIG_KS2_EVM_H
#define __CONFIG_KS2_EVM_H

#define CONFIG_SOC_KEYSTONE

/* U-Boot Build Configuration */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* U-Boot is a 2nd stage loader */

/* SoC Configuration */
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_SPL_TARGET		"u-boot-spi.gph"

/* Memory Configuration */
#define CONFIG_SYS_LPAE_SDRAM_BASE	0x800000000
#define CONFIG_MAX_RAM_BANK_SIZE	(2 << 30)       /* 2GB */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_ISW_ENTRY_ADDR - \
					GENERATED_GBL_DATA_SIZE)

#ifdef CONFIG_SYS_MALLOC_F_LEN
#define SPL_MALLOC_F_SIZE	CONFIG_SYS_MALLOC_F_LEN
#else
#define SPL_MALLOC_F_SIZE	0
#endif

/* SPL SPI Loader Configuration */
#define CONFIG_SPL_PAD_TO		65536
#define CONFIG_SPL_MAX_SIZE		(CONFIG_SPL_PAD_TO - 8)
#define CONFIG_SPL_BSS_START_ADDR	(CONFIG_ISW_ENTRY_ADDR + \
					CONFIG_SPL_MAX_SIZE)
#define CONFIG_SPL_BSS_MAX_SIZE		(32 * 1024)
#define CONFIG_SYS_SPL_MALLOC_START	(CONFIG_SPL_BSS_START_ADDR + \
					CONFIG_SPL_BSS_MAX_SIZE)
#define CONFIG_SYS_SPL_MALLOC_SIZE	(32 * 1024)
#define KEYSTONE_SPL_STACK_SIZE		(8 * 1024)
#define CONFIG_SPL_STACK		(CONFIG_SYS_SPL_MALLOC_START + \
					CONFIG_SYS_SPL_MALLOC_SIZE + \
					SPL_MALLOC_F_SIZE + \
					KEYSTONE_SPL_STACK_SIZE - 4)
#define CONFIG_SYS_SPI_U_BOOT_OFFS	CONFIG_SPL_PAD_TO

/* SRAM scratch space entries  */
#define SRAM_SCRATCH_SPACE_ADDR	CONFIG_SPL_STACK + 0x8

#define TI_SRAM_SCRATCH_BOARD_EEPROM_START	(SRAM_SCRATCH_SPACE_ADDR)
#define TI_SRAM_SCRATCH_BOARD_EEPROM_END	(SRAM_SCRATCH_SPACE_ADDR + 0x200)
#define KEYSTONE_SRAM_SCRATCH_SPACE_END		(TI_SRAM_SCRATCH_BOARD_EEPROM_END)

/* UART Configuration */
#define CONFIG_SYS_NS16550_MEM32
#if defined(CONFIG_SPL_BUILD) || !defined(CONFIG_DM_SERIAL)
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4
#endif
#define CONFIG_SYS_NS16550_COM1		KS2_UART0_BASE
#define CONFIG_SYS_NS16550_COM2		KS2_UART1_BASE

#ifndef CONFIG_SOC_K2G
#define CONFIG_SYS_NS16550_CLK		ks_clk_get_rate(KS2_CLK1_6)
#else
#define CONFIG_SYS_NS16550_CLK		ks_clk_get_rate(uart_pll_clk) / 2
#endif

/* SPI Configuration */
#define CONFIG_SYS_SPI_CLK		ks_clk_get_rate(KS2_CLK1_6)
#define CONFIG_SYS_SPI0
#define CONFIG_SYS_SPI_BASE		KS2_SPI0_BASE
#define CONFIG_SYS_SPI0_NUM_CS		4
#define CONFIG_SYS_SPI1
#define CONFIG_SYS_SPI1_BASE		KS2_SPI1_BASE
#define CONFIG_SYS_SPI1_NUM_CS		4
#define CONFIG_SYS_SPI2
#define CONFIG_SYS_SPI2_BASE		KS2_SPI2_BASE
#define CONFIG_SYS_SPI2_NUM_CS		4
#ifdef CONFIG_SPL_BUILD
#undef CONFIG_DM_SPI
#undef CONFIG_DM_SPI_FLASH
#endif

/* Network Configuration */
#define CONFIG_BOOTP_DEFAULT
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT		32
#define CONFIG_SYS_SGMII_REFCLK_MHZ	312
#define CONFIG_SYS_SGMII_LINERATE_MHZ	1250
#define CONFIG_SYS_SGMII_RATESCALE	2

/* Keyston Navigator Configuration */
#define CONFIG_TI_KSNAV
#define CONFIG_KSNAV_QM_BASE_ADDRESS		KS2_QM_BASE_ADDRESS
#define CONFIG_KSNAV_QM_CONF_BASE		KS2_QM_CONF_BASE
#define CONFIG_KSNAV_QM_DESC_SETUP_BASE		KS2_QM_DESC_SETUP_BASE
#define CONFIG_KSNAV_QM_STATUS_RAM_BASE		KS2_QM_STATUS_RAM_BASE
#define CONFIG_KSNAV_QM_INTD_CONF_BASE		KS2_QM_INTD_CONF_BASE
#define CONFIG_KSNAV_QM_PDSP1_CMD_BASE		KS2_QM_PDSP1_CMD_BASE
#define CONFIG_KSNAV_QM_PDSP1_CTRL_BASE		KS2_QM_PDSP1_CTRL_BASE
#define CONFIG_KSNAV_QM_PDSP1_IRAM_BASE		KS2_QM_PDSP1_IRAM_BASE
#define CONFIG_KSNAV_QM_MANAGER_QUEUES_BASE	KS2_QM_MANAGER_QUEUES_BASE
#define CONFIG_KSNAV_QM_MANAGER_Q_PROXY_BASE	KS2_QM_MANAGER_Q_PROXY_BASE
#define CONFIG_KSNAV_QM_QUEUE_STATUS_BASE	KS2_QM_QUEUE_STATUS_BASE
#define CONFIG_KSNAV_QM_LINK_RAM_BASE		KS2_QM_LINK_RAM_BASE
#define CONFIG_KSNAV_QM_REGION_NUM		KS2_QM_REGION_NUM
#define CONFIG_KSNAV_QM_QPOOL_NUM		KS2_QM_QPOOL_NUM

/* NETCP pktdma */
#define CONFIG_KSNAV_PKTDMA_NETCP
#define CONFIG_KSNAV_NETCP_PDMA_CTRL_BASE	KS2_NETCP_PDMA_CTRL_BASE
#define CONFIG_KSNAV_NETCP_PDMA_TX_BASE		KS2_NETCP_PDMA_TX_BASE
#define CONFIG_KSNAV_NETCP_PDMA_TX_CH_NUM	KS2_NETCP_PDMA_TX_CH_NUM
#define CONFIG_KSNAV_NETCP_PDMA_RX_BASE		KS2_NETCP_PDMA_RX_BASE
#define CONFIG_KSNAV_NETCP_PDMA_RX_CH_NUM	KS2_NETCP_PDMA_RX_CH_NUM
#define CONFIG_KSNAV_NETCP_PDMA_SCHED_BASE	KS2_NETCP_PDMA_SCHED_BASE
#define CONFIG_KSNAV_NETCP_PDMA_RX_FLOW_BASE	KS2_NETCP_PDMA_RX_FLOW_BASE
#define CONFIG_KSNAV_NETCP_PDMA_RX_FLOW_NUM	KS2_NETCP_PDMA_RX_FLOW_NUM
#define CONFIG_KSNAV_NETCP_PDMA_RX_FREE_QUEUE	KS2_NETCP_PDMA_RX_FREE_QUEUE
#define CONFIG_KSNAV_NETCP_PDMA_RX_RCV_QUEUE	KS2_NETCP_PDMA_RX_RCV_QUEUE
#define CONFIG_KSNAV_NETCP_PDMA_TX_SND_QUEUE	KS2_NETCP_PDMA_TX_SND_QUEUE

/* Keystone net */
#define CONFIG_KSNET_MAC_ID_BASE		KS2_MAC_ID_BASE_ADDR
#define CONFIG_KSNET_NETCP_BASE			KS2_NETCP_BASE
#define CONFIG_KSNET_SERDES_SGMII_BASE		KS2_SGMII_SERDES_BASE
#define CONFIG_KSNET_SERDES_SGMII2_BASE		KS2_SGMII_SERDES2_BASE
#define CONFIG_KSNET_SERDES_LANES_PER_SGMII	KS2_LANES_PER_SGMII_SERDES

#define CONFIG_AEMIF_CNTRL_BASE		KS2_AEMIF_CNTRL_BASE

/* I2C Configuration */
#define CONFIG_SYS_DAVINCI_I2C_SPEED	100000
#define CONFIG_SYS_DAVINCI_I2C_SLAVE	0x10 /* SMBus host address */
#define CONFIG_SYS_DAVINCI_I2C_SPEED1	100000
#define CONFIG_SYS_DAVINCI_I2C_SLAVE1	0x10 /* SMBus host address */
#define CONFIG_SYS_DAVINCI_I2C_SPEED2	100000
#define CONFIG_SYS_DAVINCI_I2C_SLAVE2	0x10 /* SMBus host address */

/* EEPROM definitions */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	6
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	20
#define CONFIG_ENV_EEPROM_IS_ON_I2C

/* NAND Configuration */
#define CONFIG_KEYSTONE_RBL_NAND
#define CONFIG_KEYSTONE_NAND_MAX_RBL_SIZE	CONFIG_ENV_OFFSET
#define CONFIG_SYS_NAND_MASK_CLE		0x4000
#define CONFIG_SYS_NAND_MASK_ALE		0x2000
#define CONFIG_SYS_NAND_CS			2
#define CONFIG_SYS_NAND_USE_FLASH_BBT
#define CONFIG_SYS_NAND_4BIT_HW_ECC_OOBFIRST

#define CONFIG_SYS_NAND_LARGEPAGE
#define CONFIG_SYS_NAND_BASE_LIST		{ 0x30000000, }
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define CONFIG_SYS_NAND_MAX_CHIPS		1
#define CONFIG_SYS_NAND_NO_SUBPAGE_WRITE

#define DFU_ALT_INFO_MMC \
	"dfu_alt_info_mmc=" \
	"MLO fat 0 1;" \
	"u-boot.img fat 0 1;" \
	"uEnv.txt fat 0 1\0"

/* DFU settings */
#define DFUARGS \
	"dfu_bufsiz=0x10000\0" \
	DFU_ALT_INFO_MMC \

/* U-Boot general configuration */
#define CONFIG_MX_CYCLIC
#define CONFIG_TIMESTAMP

/* EDMA3 */
#define CONFIG_TI_EDMA3

#define KERNEL_MTD_PARTS						\
	"mtdparts="							\
	SPI_MTD_PARTS

#define DEFAULT_FW_INITRAMFS_BOOT_ENV					\
	"name_fw_rd=k2-fw-initrd.cpio.gz\0"				\
	"set_rd_spec=setenv rd_spec ${rdaddr}:${filesize}\0"		\
	"init_fw_rd_net=dhcp ${rdaddr} ${tftp_root}/${name_fw_rd}; "	\
		"run set_rd_spec\0"					\
	"init_fw_rd_nfs=nfs ${rdaddr} ${nfs_root}/boot/${name_fw_rd}; "	\
		"run set_rd_spec\0"					\
	"init_fw_rd_ramfs=setenv rd_spec -\0"				\
	"init_fw_rd_ubi=ubifsload ${rdaddr} ${bootdir}/${name_fw_rd}; "	\
		"run set_rd_spec\0"					\

#define DEFAULT_PMMC_BOOT_ENV						\
	"set_name_pmmc=setenv name_pmmc ti-sci-firmware-${soc_variant}.bin\0" \
	"dev_pmmc=0\0"							\
	"get_pmmc_net=dhcp ${loadaddr} ${tftp_root}/${name_pmmc}\0"	\
	"get_pmmc_nfs=nfs ${loadaddr} ${nfs_root}/boot/${name_pmmc}\0"	\
	"get_pmmc_ramfs=run get_pmmc_net\0"				\
	"get_pmmc_mmc=load mmc ${bootpart} ${loadaddr} "		\
			"${bootdir}/${name_pmmc}\0"			\
	"get_pmmc_ubi=ubifsload ${loadaddr} ${bootdir}/${name_pmmc}\0"	\
	"run_pmmc=rproc init; rproc list; "				\
		"rproc load ${dev_pmmc} ${loadaddr} 0x${filesize}; "	\
		"rproc start ${dev_pmmc}\0"				\

#define CONFIG_EXTRA_ENV_SETTINGS					\
	DEFAULT_LINUX_BOOT_ENV						\
	CONFIG_EXTRA_ENV_KS2_BOARD_SETTINGS				\
	DFUARGS								\
	"bootdir=/boot\0" \
	"tftp_root=/\0"							\
	"nfs_root=/export\0"						\
	"mem_lpae=1\0"							\
	"addr_ubi=0x82000000\0"						\
	"addr_secdb_key=0xc000000\0"					\
	"name_kern=zImage\0"						\
	"addr_mon=0x87000000\0"						\
	"addr_non_sec_mon=0x0c087fc0\0"					\
	"addr_load_sec_bm=0x0c08c000\0"					\
	"run_mon=mon_install ${addr_mon}\0"				\
	"run_mon_hs=mon_install ${addr_non_sec_mon} "			\
			"${addr_load_sec_bm}\0"				\
	"run_kern=bootz ${loadaddr} ${rd_spec} ${fdtaddr}\0"		\
	"init_net=run args_all args_net\0"				\
	"init_nfs=setenv autoload no; dhcp; run args_all args_net\0"	\
	"init_ubi=run args_all args_ubi; "				\
		"ubi part ubifs; ubifsmount ubi:rootfs;\0"			\
	"get_fdt_net=dhcp ${fdtaddr} ${tftp_root}/${name_fdt}\0"	\
	"get_fdt_nfs=nfs ${fdtaddr} ${nfs_root}/boot/${name_fdt}\0"	\
	"get_fdt_ubi=ubifsload ${fdtaddr} ${bootdir}/${name_fdt}\0"		\
	"get_kern_net=dhcp ${loadaddr} ${tftp_root}/${name_kern}\0"	\
	"get_kern_nfs=nfs ${loadaddr} ${nfs_root}/boot/${name_kern}\0"	\
	"get_kern_ubi=ubifsload ${loadaddr} ${bootdir}/${name_kern}\0"		\
	"get_mon_net=dhcp ${addr_mon} ${tftp_root}/${name_mon}\0"	\
	"get_mon_nfs=nfs ${addr_mon} ${nfs_root}/boot/${name_mon}\0"	\
	"get_mon_ubi=ubifsload ${addr_mon} ${bootdir}/${name_mon}\0"	\
	"get_fit_net=dhcp ${fit_loadaddr} ${tftp_root}"			\
						"/${fit_bootfile}\0"	\
	"get_fit_nfs=nfs ${fit_loadaddr} ${nfs_root}/boot/${fit_bootfile}\0"\
	"get_fit_ubi=ubifsload ${fit_loadaddr} ${bootdir}/${fit_bootfile}\0"\
	"get_fit_mmc=load mmc ${bootpart} ${fit_loadaddr} "		\
					"${bootdir}/${fit_bootfile}\0"	\
	"get_uboot_net=dhcp ${loadaddr} ${tftp_root}/${name_uboot}\0"	\
	"get_uboot_nfs=nfs ${loadaddr} ${nfs_root}/boot/${name_uboot}\0" \
	"burn_uboot_spi=sf probe; sf erase 0 0x100000; "		\
		"sf write ${loadaddr} 0 ${filesize}\0"		\
	"burn_uboot_nand=nand erase 0 0x100000; "			\
		"nand write ${loadaddr} 0 ${filesize}\0"		\
	"args_all=setenv bootargs console=ttyS0,115200n8 rootwait=1 "	\
		KERNEL_MTD_PARTS					\
	"args_net=setenv bootargs ${bootargs} rootfstype=nfs "		\
		"root=/dev/nfs rw nfsroot=${serverip}:${nfs_root},"	\
		"${nfs_options} ip=dhcp\0"				\
	"nfs_options=v3,tcp,rsize=4096,wsize=4096\0"			\
	"get_fdt_ramfs=dhcp ${fdtaddr} ${tftp_root}/${name_fdt}\0"	\
	"get_kern_ramfs=dhcp ${loadaddr} ${tftp_root}/${name_kern}\0"	\
	"get_mon_ramfs=dhcp ${addr_mon} ${tftp_root}/${name_mon}\0"	\
	"get_fit_ramfs=dhcp ${fit_loadaddr} ${tftp_root}"		\
						"/${fit_bootfile}\0"	\
	"get_fs_ramfs=dhcp ${rdaddr} ${tftp_root}/${name_fs}\0"	\
	"get_ubi_net=dhcp ${addr_ubi} ${tftp_root}/${name_ubi}\0"	\
	"get_ubi_nfs=nfs ${addr_ubi} ${nfs_root}/boot/${name_ubi}\0"	\
	"burn_ubi=nand erase.part ubifs; "				\
		"nand write ${addr_ubi} ubifs ${filesize}\0"		\
	"init_ramfs=run args_all args_ramfs get_fs_ramfs\0"		\
	"args_ramfs=setenv bootargs ${bootargs} "			\
		"rdinit=/sbin/init rw root=/dev/ram0 "			\
		"initrd=0x808080000,80M\0"				\
	"no_post=1\0"							\
	"mtdparts=mtdparts=davinci_nand.0:"				\
		"1024k(bootloader)ro,512k(params)ro,-(ubifs)\0"

#ifndef CONFIG_BOOTCOMMAND
#ifndef CONFIG_TI_SECURE_DEVICE
#define CONFIG_BOOTCOMMAND						\
	"run init_${boot}; "						\
	"run get_mon_${boot} run_mon; "					\
	"run get_kern_${boot}; "					\
	"run init_fw_rd_${boot}; "					\
	"run get_fdt_${boot}; "						\
	"run run_kern"
#else
#define CONFIG_BOOTCOMMAND						\
	"run run_mon_hs; "						\
	"run init_${boot}; "						\
	"run get_fit_${boot}; "						\
	"bootm ${fit_loadaddr}#${name_fdt}"
#endif
#endif

/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>

/* we may include files below only after all above definitions */
#include <asm/arch/hardware.h>
#include <asm/arch/clock.h>
#ifndef CONFIG_SOC_K2G
#define CONFIG_SYS_HZ_CLOCK		ks_clk_get_rate(KS2_CLK1_6)
#else
#define CONFIG_SYS_HZ_CLOCK		get_external_clk(sys_clk)
#endif

#endif /* __CONFIG_KS2_EVM_H */
