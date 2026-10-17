/*
 * DDR Setup
 */
#define CFG_SYS_SDRAM_BASE		0x00000000 /* DDR is system memory */

#define CFG_SYS_DDR_SDRAM_CLK_CNTL	(DDR_SDRAM_CLK_CNTL_SS_EN | \
					DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05)

/*
 * The reserved memory
 */
#define CFG_SYS_FLASH_BASE		0xF0000000

/* Reserve 768 kB for Mon */

/*
 * Initial RAM Base Address Setup
 */
#define CFG_SYS_INIT_RAM_ADDR	0xE6000000 /* Initial RAM address */
#define CFG_SYS_INIT_RAM_SIZE	0x1000 /* End of used area in RAM */
/*
 * Init Local Bus Memory Controller:
 *
 * Bank Bus     Machine PortSz  Size  Device
 * ---- ---     ------- ------  -----  ------
 *  0   Local   GPCM    16 bit  256MB FLASH
 *  1   Local   GPCM     8 bit  128MB GPIO/PIGGY
 *
 */

/*
 * FLASH on the Local Bus
 */
#define CFG_SYS_FLASH_SIZE		256 /* max FLASH size is 256M */

#define CFG_SYS_FLASH_BANKS_LIST { CFG_SYS_FLASH_BASE }

#define CFG_SYS_KMBEC_FPGA_BASE   0xE8000000

#if defined(CONFIG_CMD_NAND)
#define CFG_SYS_NAND_BASE		CFG_SYS_KMBEC_FPGA_BASE
#endif

#if defined(CONFIG_TARGET_KMCOGE5NE) || defined(CONFIG_TARGET_KMETER1)
/*
 * System IO Setup
 */
#define CFG_SYS_SICRH		(SICRH_UC1EOBI | SICRH_UC2E1OBI)

#define CFG_SYS_DDRCDR (\
	DDRCDR_EN | \
	DDRCDR_Q_DRN)
#else
/*
 * System IO Config
 */
#define CFG_SYS_SICRL	SICRL_IRQ_CKS

#define CFG_SYS_DDRCDR (\
	DDRCDR_EN | \
	DDRCDR_PZ_MAXZ | \
	DDRCDR_NZ_MAXZ | \
	DDRCDR_M_ODR)
#endif

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_SYS_BOOTMAPSZ		(8 << 20)
