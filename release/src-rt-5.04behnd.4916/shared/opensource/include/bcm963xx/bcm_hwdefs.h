/*
<:copyright-BRCM:2012:DUAL/GPL:standard 

   Copyright (c) 2012 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/                       

/***********************************************************************/
/*                                                                     */
/*   MODULE:  bcm_hwdefs.h                                             */
/*   PURPOSE: Define all base device addresses and HW specific macros. */
/*                                                                     */
/***********************************************************************/
#ifndef _BCM_HWDEFS_H
#define _BCM_HWDEFS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef INC_BTRM_BOOT
#define INC_BTRM_BOOT         0
#endif

#define	DYING_GASP_API

/*****************************************************************************/
/*                    Physical Memory Map                                    */
/*****************************************************************************/
#if defined(_BCM94908_) || defined(CONFIG_BCM94908) || defined(_BCM963158_) || defined(CONFIG_BCM963158)
#define DRAM_DEF_SIZE           0x08000000      /* 4908 minimum 128MB*/
#elif defined(_BCM96858_) || defined(CONFIG_BCM96858)
#define DRAM_DEF_SIZE           0x20000000      /* 6858 512MB*/
#else
#ifndef CONFIG_BRCM_QEMU
#define DRAM_DEF_SIZE           0x04000000      /* Universal value across all bcm93xx 64 MB*/
#else
#define DRAM_DEF_SIZE           0x08000000      /* Universal value across all bcm93xx 128 MB*/
#endif
#endif
#define PHYS_DRAM_BASE          0x00000000      /* Dynamic RAM Base */
#if defined(_BCM963158_) || defined(CONFIG_BCM963158)
#define PHYS_DRAM_SPLIT_SIZE    0x80000000
#define PHYS_DRAM_BASE_2        0x100000000     /* Dynamic RAM Base for memory above SPLIT SIZE */
#endif
#if defined(CONFIG_BRCM_IKOS)
#define PHYS_FLASH_BASE         0x18000000      /* Flash Memory     */
#else
#define PHYS_FLASH_BASE         0x1FC00000      /* Flash Memory     */
#endif

/*****************************************************************************/
/* Note that the addresses above are physical addresses and that programs    */
/* have to use converted addresses defined below:                            */
/*****************************************************************************/
/* For ARM cacheablity is set in the mmu entry. cfe rom set ddr to none-cacheable, cfe ram 
   set to cacheable 
*/
#define DRAM_BASE           PHYS_DRAM_BASE   
#define DRAM_BASE_NOCACHE   PHYS_DRAM_BASE   
#if defined(PHYS_DRAM_BASE_2)
#define DRAM_BASE_2         PHYS_DRAM_BASE_2
#endif
/* Binary images are always built for a standard MIPS boot address */
#define IMAGE_BASE          (0xA0000000 | PHYS_FLASH_BASE)

/* Some chips don't support alternative boot vector */
#if defined(CONFIG_BRCM_IKOS)
#define BOOT_OFFSET         0
#if defined(CONFIG_BCM63138_SIM) || defined(CONFIG_BCM63148_SIM)
#define FLASH_BASE          0xFFFD0000
#else
#define FLASH_BASE          (0xA0000000 | PHYS_FLASH_BASE)  /* uncached Flash  */
#endif
#else //CONFIG_BRCM_IKOS
/* linux use block number for SPI flash and most of the NAND code. Only use of the direct
mappaed address is for NAND in the kerSysEarlyFlashInit in board driver bcm63xx_flash.c */
#define FLASH_BASE          0x00000000
#define FLASH_BASE_NAND     0xFFE00000
#define BOOT_OFFSET         (FLASH_BASE - IMAGE_BASE)
#endif

/*****************************************************************************/
/*  ARM Kernel boot requires MACH_ID and ATAG_LOC                            */
/*****************************************************************************/
#if defined(CONFIG_ARM)
#if defined (CONFIG_BCM963138) || defined (_BCM963138_)
#define BCM63XX_MACH_ID   0x270A
#elif defined (CONFIG_BCM963148) || defined (_BCM963148_)
#define BCM63XX_MACH_ID   0x2709
#elif defined (CONFIG_BCM96846) || defined (_BCM96846_)
#define BCM63XX_MACH_ID   0x270B
#else
#define BCM63XX_MACH_ID   0x270B
#endif
#endif

/*****************************************************************************/
/*  Select the PLL value to get the desired CPU clock frequency.             */
/*****************************************************************************/
#if defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM963178_) || defined(CONFIG_BCM963178) || \
	defined(_BCM963158_) || defined(CONFIG_BCM963158) || defined(_BCM96878_) || defined(CONFIG_BCM96878) || \
	defined(CONFIG_BCM96858) || defined(_BCM96858_) || defined(_BCM96856_) || defined(CONFIG_BCM96856) || \
	defined(_BCM947622_) || defined(CONFIG_BCM947622) || defined(CONFIG_BCM963146) || defined(_BCM963146_)|| \
	defined(CONFIG_BCM94912) || defined(_BCM94912_) || defined(_BCM96855_) || defined(CONFIG_BCM96855) || \
	defined(_BCM96756_) || defined(CONFIG_BCM96756) || defined(_BCM96888_) || defined(CONFIG_BCM96888) || \
    defined(CONFIG_BCM96813) || defined(_BCM96813_)
#define FPERIPH            200000000
#else
#define FPERIPH            50000000
#endif

#define FPERIPH_WD         50000000

/*****************************************************************************/
/* Board memory type offset                                                  */
/*****************************************************************************/
#define ONEK                            1024
 
#if defined(_BCM963138_) || defined(CONFIG_BCM963138) || defined(_BCM963148_) || defined(CONFIG_BCM963148)
#define FLASH_LENGTH_SECURE_BOOT_ROM    (64*6*ONEK)
#else
#define FLASH_LENGTH_SECURE_BOOT_ROM    (64*ONEK)
#endif

#if (INC_BTRM_BOOT==1)	/* SPI NOR secure boot build */
#define FLASH_LENGTH_BOOT_ROM           FLASH_LENGTH_SECURE_BOOT_ROM
#else 			/* SPI NOR legacy build */
#if defined(_BCM96858_) || defined(CONFIG_BCM96858)
#define FLASH_LENGTH_BOOT_ROM           (64*5*ONEK)
#else 
#if defined(_BCM963178_) || defined(CONFIG_BCM963178) || \
    defined(_BCM947622_) || defined(CONFIG_BCM947622) || \
    defined(_BCM96756_) || defined(CONFIG_BCM96756)
/*
 when build cfe_fs_kernel nor image, bcmImageBuilder will add address for the first rootfs exactly
 after cfe, e.g. cfe is 128k, and in the brcm format and the first filetag for the first rootfs will be 0x20000
 but in the code, the fInfo.flash_rootfs_start_offset = FLASH_LENGTH_BOOT_ROM, that is 192k in previous
 setting, this will make update spi nor flash image( and spi nor .w image) fail in getting the first filetag
 so change it to 128k for now. will further investigate the impact 
 this is the case for both enable bootrom but not enable secure boot. but I think for legacy spi flash image, it 
 should also has this problem?
*/
/*as 128K is not enough for one spi cferom, need to change it back to 192k*/
#define FLASH_LENGTH_BOOT_ROM           (64*3*ONEK)
#else
#define FLASH_LENGTH_BOOT_ROM           (64*3*ONEK)
#endif
#endif
#endif

/*****************************************************************************/
/*       NVRAM Offset and definition                                         */
/*****************************************************************************/
#define NVRAM_DATA_REL_OFFSET           0x0580
#if defined(_BCM963138_) || defined(CONFIG_BCM963138) || \
    defined(_BCM963148_) || defined(CONFIG_BCM963148) || \
    defined(_BCM94908_) || defined(CONFIG_BCM94908) || \
    defined(_BCM96858_) || defined(CONFIG_BCM96858) || \
    defined(_BCM963158_) || defined(CONFIG_BCM963158) || \
    defined(_BCM96846_) || defined(CONFIG_BCM96846) || \
    defined(_BCM96878_) || defined(CONFIG_BCM96878) || \
    defined(_BCM96855_) || defined(CONFIG_BCM96855) || \
    defined(_BCM963178_) || defined(CONFIG_BCM963178) || \
    defined(_BCM947622_) || defined(CONFIG_BCM947622) || \
    defined(_BCM96856_) || defined(CONFIG_BCM96856) || \
    defined(_BCM96756_) || defined(CONFIG_BCM96756)  
/* 63138 image always start from 64KB offset in the flash, the first 64KB is used for PMC */
#if defined(CONFIG_CFE_IMAGE_OFFSET_ZERO) || defined(CONFIG_BCM_IMAGE_OFFSET_ZERO)
#define IMAGE_OFFSET                    0 
#else	
#define IMAGE_OFFSET                    0x10000
#endif
#define NVRAM_SECTOR                    (IMAGE_OFFSET/((unsigned int)flash_get_sector_size(0)))
#define NVRAM_DATA_OFFSET               (NVRAM_DATA_REL_OFFSET+(IMAGE_OFFSET-(IMAGE_OFFSET/((unsigned int)flash_get_sector_size(0)))*((unsigned int)flash_get_sector_size(0))))
#else
#define IMAGE_OFFSET                    0                    
#define NVRAM_SECTOR                    0
#define NVRAM_DATA_OFFSET               NVRAM_DATA_REL_OFFSET
#endif

#define NVRAM_DATA_ID                   0x0f1e2d3c  // This is only for backwards compatability

#define NVRAM_LENGTH                    ONEK     // 1k nvram
#define NVRAM_VERSION_NUMBER    	6


#define NVRAM_FULL_LEN_VERSION_NUMBER   5 /* version in which the checksum was
                                             placed at the end of the NVRAM
                                             structure */

#define NVRAM_BOOTLINE_LEN              256
#define NVRAM_BOARD_ID_STRING_LEN       64
#define NVRAM_MAC_ADDRESS_LEN           6

#define NVRAM_GPON_SERIAL_NUMBER_LEN    13
#define NVRAM_GPON_PASSWORD_LEN         11
#define NVRAM_XGPON_PASSWORD_LEN        37


#define  WLAN_FEATURE_DHD_NIC_ENABLE     0x01
#define  WLAN_FEATURE_DHD_MFG_ENABLE     0x02
#define  WLAN_FEATURE_LAST_FEATURE       0x80

#define  WLAN_MFG_PARTITION_ISNAND       0x01
#define  WLAN_MFG_PARTITION_MFGSET       0x02
#define  WLAN_MFG_PARTITION_HASSIZE      0x04
#define  WLAN_BOOTMODE_IS_UBOOT          0x08
#define  WLAN_MFG_PARTITION_NAME         "misc3"
#define  WLAN_MFG_PARTITION_INDEX         2

#define NVRAM_WLAN_PARAMS_LEN      256
#define NVRAM_WPS_DEVICE_PIN_LEN   8

#define NVRAM_BOOTLDR_SIG_LEN           256
#define NVRAM_BOOTLDR_SIG_OFFSET        1024

#define NVRAM_CV_KEY_LEN                514
#define NVRAM_MFG_CV_KEY_OFFSET         1280
#define NVRAM_OP_CV_KEY_OFFSET          1794

#define NVRAM_ENC_KEY_LEN               32

#define NVRAM_BOOTLDR_ENC_KEY_OFFSET    2308
#define NVRAM_IMAGE_ENC_KEY_OFFSET      2340

#define NVRAM_ENC_IV_LEN                32

#define NVRAM_BOOTLDR_ENC_IV_OFFSET     2372
#define NVRAM_IMAGE_ENC_IV_OFFSET       2404

#define NVRAM_SECURITY_CREDENTIALS_LEN  2044

#define THREAD_NUM_ADDRESS_OFFSET       (NVRAM_DATA_OFFSET + 4 + NVRAM_BOOTLINE_LEN + 16)
#define THREAD_NUM_ADDRESS              (0x80000000 + THREAD_NUM_ADDRESS_OFFSET)

#define DEFAULT_BOOTLINE    "e=192.168.1.1:ffffff00 h=192.168.1.100 g= r=f f=vmlinux i=bcm963xx_fs_kernel d=1 p=0 c= a= "
#define DEFAULT_BOARD_IP    "192.168.1.1"
#define DEFAULT_MAC_NUM     10
#define DEFAULT_BOARD_MAC   "00:10:18:00:00:00"
#define DEFAULT_TP_NUM      0

#define DEFAULT_PSI_SIZE    128
#define DEFAULT_GPON_SN     "BRCM12345678"
#define DEFAULT_GPON_PW     "          "
#define DEFAULT_LOG_SIZE    0
#define DEFAULT_FLASHBLK_SIZE  64
#define MAX_FLASHBLK_SIZE      128
#define DEFAULT_AUXFS_PERCENT 0
#define MAX_AUXFS_PERCENT   80
#define MAX_NOR_AUXFS_SIZE  255
#define NOR_AUFS_SIZE_UNIT (4*ONEK)
#define DEFAUT_BACKUP_PSI  0

#define DEFAULT_WPS_DEVICE_PIN     "12345670"
#ifdef DSLAX82U
#define DEFAULT_WLAN_DEVICE_FEATURE  2
#else
#define DEFAULT_WLAN_DEVICE_FEATURE  0
#endif

#define DEFAULT_VOICE_BOARD_ID     "NONE"

#define MIN_RNRTBLS_SIZE 8
#if defined(_BCM963138_) || defined(_BCM963148_)
/* any better value for these two? */
#define MAX_TM_SIZE	150
#define MAX_MC_SIZE	15
#define MAX_RDP_PARAM1_SIZE MAX_TM_SIZE
#define MAX_RDP_PARAM2_SIZE MAX_MC_SIZE
#define MIN_RDP_PARAM2_SIZE 0
#define DEFAULT_NVRAM_RDP_TMSIZE	21
#define DEFAULT_NVRAM_RDP_MCSIZE	4
#define DEFAULT_NVRAM_RDP_PARAM1 DEFAULT_NVRAM_RDP_TMSIZE
#define DEFAULT_NVRAM_RDP_PARAM2 DEFAULT_NVRAM_RDP_MCSIZE
#elif defined(_BCM94908_)
#define MAX_BUFFER_MEMORY_SIZE		64
#define MAX_FLOW_MEMORY_SIZE		74
#define MAX_RDP_PARAM1_SIZE	MAX_BUFFER_MEMORY_SIZE
#define MAX_RDP_PARAM2_SIZE	MAX_FLOW_MEMORY_SIZE
#define MIN_RDP_PARAM2_SIZE 0
#define DEFAULT_NVRAM_BUFFER_MEMORY_SIZE	16
#define DEFAULT_NVRAM_FLOW_MEMORY_SIZE		44
#define DEFAULT_NVRAM_RDP_PARAM1	DEFAULT_NVRAM_BUFFER_MEMORY_SIZE
#define DEFAULT_NVRAM_RDP_PARAM2	DEFAULT_NVRAM_FLOW_MEMORY_SIZE
#elif defined(_BCM963158_)
#define MAX_BUFFER_MEMORY_SIZE		128
#define MAX_FLOW_MEMORY_SIZE		74
#define MIN_FLOW_MEMORY_SIZE        10
#define MAX_RDP_PARAM1_SIZE	MAX_BUFFER_MEMORY_SIZE
#define MAX_RDP_PARAM2_SIZE	MAX_FLOW_MEMORY_SIZE
#define MIN_RDP_PARAM2_SIZE     MIN_FLOW_MEMORY_SIZE
#define DEFAULT_NVRAM_BUFFER_MEMORY_SIZE	32
#define DEFAULT_NVRAM_FLOW_MEMORY_SIZE		12
#define DEFAULT_NVRAM_RDP_PARAM1	DEFAULT_NVRAM_BUFFER_MEMORY_SIZE
#define DEFAULT_NVRAM_RDP_PARAM2	DEFAULT_NVRAM_FLOW_MEMORY_SIZE
#elif defined(_BCM96858_) || defined (_BCM96856_)
#define MAX_FPMPOOL_SIZE 128
#define MAX_RNRTBLS_SIZE 16
#define MAX_RDP_PARAM1_SIZE MAX_FPMPOOL_SIZE
#define MAX_RDP_PARAM2_SIZE MAX_RNRTBLS_SIZE
#define MIN_RDP_PARAM2_SIZE MIN_RNRTBLS_SIZE
#define DEFAULT_NVRAM_RDP_FPMPOOLSIZE    32
#define DEFAULT_NVRAM_RDP_RNRTBLSSIZE    8
#define DEFAULT_NVRAM_RDP_PARAM1 DEFAULT_NVRAM_RDP_FPMPOOLSIZE
#define DEFAULT_NVRAM_RDP_PARAM2 DEFAULT_NVRAM_RDP_RNRTBLSSIZE
#elif defined(_BCM96846_)
#define MAX_FPMPOOL_SIZE 32
#define MAX_RNRTBLS_SIZE 8
#define MAX_RDP_PARAM1_SIZE MAX_FPMPOOL_SIZE
#define MAX_RDP_PARAM2_SIZE MAX_RNRTBLS_SIZE
#define MIN_RDP_PARAM2_SIZE MIN_RNRTBLS_SIZE
#define DEFAULT_NVRAM_RDP_FPMPOOLSIZE    16
#define DEFAULT_NVRAM_RDP_RNRTBLSSIZE    4
#define DEFAULT_NVRAM_RDP_PARAM1 DEFAULT_NVRAM_RDP_FPMPOOLSIZE
#define DEFAULT_NVRAM_RDP_PARAM2 DEFAULT_NVRAM_RDP_RNRTBLSSIZE
#elif defined(_BCM96878_)
#define MAX_FPMPOOL_SIZE 32
#define MAX_RNRTBLS_SIZE 8
#define MAX_RDP_PARAM1_SIZE MAX_FPMPOOL_SIZE
#define MAX_RDP_PARAM2_SIZE MAX_RNRTBLS_SIZE
#define MIN_RDP_PARAM2_SIZE MIN_RNRTBLS_SIZE
#define DEFAULT_NVRAM_RDP_FPMPOOLSIZE    16
#define DEFAULT_NVRAM_RDP_RNRTBLSSIZE    4
#define DEFAULT_NVRAM_RDP_PARAM1 DEFAULT_NVRAM_RDP_FPMPOOLSIZE
#define DEFAULT_NVRAM_RDP_PARAM2 DEFAULT_NVRAM_RDP_RNRTBLSSIZE
#else
#define MAX_TM_SIZE	0
#define MAX_MC_SIZE	0
#define DEFAULT_NVRAM_RDP_TMSIZE	0
#define DEFAULT_NVRAM_RDP_MCSIZE	0
#define MAX_RDP_PARAM1_SIZE MAX_TM_SIZE
#define MAX_RDP_PARAM2_SIZE MAX_MC_SIZE
#define DEFAULT_NVRAM_RDP_PARAM1 DEFAULT_NVRAM_RDP_TMSIZE
#define DEFAULT_NVRAM_RDP_PARAM2 DEFAULT_NVRAM_RDP_MCSIZE

#endif

#define NVRAM_MAC_COUNT_MAX         64
#define NVRAM_MAX_PSI_SIZE          512
#define NVRAM_MAX_SYSLOG_SIZE       256

#define NP_BOOT             0
#define NP_ROOTFS_1         1
#define NP_ROOTFS_2         2
#define NP_DATA             3
#define NP_BBT              4
#define NP_TOTAL            5

#define NAND_DATA_SIZE_KB       4096
#define NAND_BBT_THRESHOLD_KB   (512 * 1024)
#define NAND_BBT_SMALL_SIZE_KB  1024
#define NAND_BBT_BIG_SIZE_KB    4096

#define NAND_CFE_RAM_NAME           "cferam.000"
#define NAND_CFE_RAM_SECBT_NAME     "secram.000"
#define NAND_CFE_RAM_SECBT_MFG_NAME "secmfg.000"
#define NAND_HASH_BIN_NAME       "hashes.bin"
#define NAND_HASH_SECBT_NAME     "hashes.fld"
#define NAND_HASH_SECBT_MFG_NAME "hashes.mfg"
#define NAND_RFS_OFS_NAME           "NAND_RFS_OFS"
#define NAND_COMMAND_NAME           "NANDCMD"
#define NAND_BOOT_STATE_FILE_NAME   "boot_state_x"
#define NAND_SEQ_MAGIC              0x53510000


#define BCM_MAX_EXTRA_PARTITIONS 4

#define NAND_FULL_PARTITION_SEARCH  0

#if (NAND_FULL_PARTITION_SEARCH == 1)
#define MAX_NOT_JFFS2_VALUE         0 /* infinite */
#else
#define MAX_NOT_JFFS2_VALUE         10
#endif

#ifndef _LANGUAGE_ASSEMBLY

#define DT_ROOT_NODE                "/"
#define DT_MEMORY_NODE              "memory"

#define DT_CHOSEN_NODE              "chosen"
#define DT_INITRD_START_PROP        "linux,initrd-start"
#define DT_INITRD_END_PROP          "linux,initrd-end"
#define DT_BOOTARGS_PROP            "bootargs"
#define INITRD_DEF_SIZE              SZ_16M 
#define DT_INITRD_START              INITRD_DEF_LOAD_ADDR 
#define DT_INITRD_END                INITRD_DEF_SIZE 
#define DT_BOOTARGS                  " root=/dev/ram0 ro "
#define DT_BOOTARGS_UPD_OPT          "ramdisk_size="
#define DT_BOOTARGS_MAX_SIZE         512

#define PLC_SDRAM_SIZE              0x0200000

struct allocs_rdp {
    unsigned char param1_size;
    unsigned char param2_size;
    unsigned char reserved[2];
};

struct allocs_dhd {
    unsigned char dhd_size[3];
    unsigned char reserved;
};

typedef struct
{
    unsigned int ulVersion;
    char szBootline[NVRAM_BOOTLINE_LEN];
    char szBoardId[16];
    unsigned int ulMainTpNum;
    unsigned int ulPsiSize;
    unsigned int ulNumMacAddrs;
    unsigned char ucaBaseMacAddr[NVRAM_MAC_ADDRESS_LEN];
    char pad;
    char backupPsi;  /**< if 0x01, allocate space for a backup PSI */
    unsigned int ulCheckSumV4;
    char gponSerialNumber[NVRAM_GPON_SERIAL_NUMBER_LEN];
    char gponPassword[NVRAM_GPON_PASSWORD_LEN];
    char wpsDevicePin[NVRAM_WPS_DEVICE_PIN_LEN];
    char wlanParams[NVRAM_WLAN_PARAMS_LEN];
    unsigned int ulSyslogSize; /**< number of KB to allocate for persistent syslog */
    unsigned int ulNandPartOfsKb[NP_TOTAL];
    unsigned int ulNandPartSizeKb[NP_TOTAL];
    char szVoiceBoardId[16];
    unsigned int afeId[2];
    unsigned short opticRxPwrReading;   // optical initial rx power reading
    unsigned short opticRxPwrOffset;    // optical rx power offset
    unsigned short opticTxPwrReading;   // optical initial tx power reading
    unsigned char ucUnused2[58];
    unsigned char ucFlashBlkSize;
    unsigned char ucAuxFSPercent;
    unsigned char ucUnused3[2];
    unsigned int ulBoardStuffOption;   // board options. bit0-3 is for DECT    
    union {
        unsigned int reserved;
        struct allocs_rdp alloc_rdp;
    } allocs;
    unsigned int ulMemoryConfig;
    struct partition_info {
	/*
		2MSB represent the 
			00 = MB 
			01 = GB
			10 = reserved
			11 = reserved
		14LSB represent multiple of 2MSB
	*/
	
	unsigned short size;
    } part_info[BCM_MAX_EXTRA_PARTITIONS];
    struct allocs_dhd alloc_dhd;

    /* Add any new non-secure related elements here */
# define NVFEAT_AVSDISABLED  (1<<0)     // bit indicating avs should be disabled
# define NVFEAT_RESERVED    (1<<31)     // bit reserved for erasure detection
    unsigned int ulFeatures;            // feature bitmask
    char chUnused[268]; /* Adjust chUnused such that everything above + chUnused[] + ulCheckSum = 1k */
    unsigned int ulCheckSum;
} NVRAM_DATA, *PNVRAM_DATA;

#define NVRAM_ULVERSION                  "ulVersion"
#define NVRAM_SZBOOTLINE                 "szBootline"
#define NVRAM_SZBOARDID                  "boardid"
#define NVRAM_ULMAINTPNUM                "ulMainTpNum"
#define NVRAM_ULPSISIZE                  "ulPsiSize"
#define NVRAM_ULNUMMACADDRS              "nummacaddrs"
#define NVRAM_UCABASEMACADDR             "ethaddr"
#define NVRAM_PAD                        "pad"
#define NVRAM_BACKUPPSI                  "backupPsi"
#define NVRAM_ULCHECKSUMV4               "ulCheckSumV4"
#define NVRAM_GPONSERIALNUMBER           "gponsn"
#define NVRAM_GPONPASSWORD               "gponpswd"
#define NVRAM_WPSDEVICEPIN               "wpsDevicePin"
#define NVRAM_WLANPARAMS                 "wlanParams"
#define NVRAM_WLFEATURE                  "wlFeature"
#define NVRAM_ULSYSLOGSIZE               "ulSyslogSize"
#define NVRAM_ULNANDPARTOFSKB            "ulNandPartOfsKb"
#define NVRAM_ULNANDPARTSIZEKB           "ulNandPartSizeKb"
#define NVRAM_SZVOICEBOARDID             "voiceboardid"
#define NVRAM_AFEID                      "afeId"
#define NVRAM_OPTICRXPWRREADING          "opticRxPwrReading"
#define NVRAM_OPTICRXPWROFFSET           "opticRxPwrOffset"
#define NVRAM_OPTICTXPWRREADING          "opticTxPwrReading"
#define NVRAM_UCUNUSED2                  "ucUnused2"
#define NVRAM_UCFLASHBLKSIZE             "ucFlashBlkSize"
#define NVRAM_UCAUXFSPERCENT             "ucAuxFSPercent"
#define NVRAM_UCUNUSED3                  "ucUnused3"
#define NVRAM_ULBOARDSTUFFOPTION         "ulBoardStuffOption"
#define NVRAM_ALLOCS                     "allocs"
#define NVRAM_ULMEMORYCONFIG             "ulMemoryConfig"
#define NVRAM_PART_INFO                  "part_info"
#define NVRAM_ALLOC_DHD                  "alloc_dhd"
#define NVRAM_ULFEATURES                 "ulFeatures"
#define NVRAM_CHUNUSED                   "chUnused"
#define NVRAM_ULCHECKSUM                 "ulCheckSum"

#define NVRAM_DATA_SIGN "nVrAmDaT"
#define BOOT_BLOCK_MIRROR_INFO_FILE "/data/bootblock_data.info"
#define MAX_BOOT_BLOCK_MIRROR_LOOKUP_OFFSET 20*1024*1024
#define BOOT_BLOCK_MIRROR_INFO_VERSION_MISMATCH -1
#define MAX_MIRRORS 20
#define IMG_NVRAM 1
#define IMG_CFEROM 2

#define IMG_MISSING 0xfffe
#define CRC_MISMATCH 0xfffe

typedef struct
{
        int active_idx;
        unsigned long offset[MAX_MIRRORS];
        unsigned long image_size[MAX_MIRRORS];
        unsigned short image_type[MAX_MIRRORS];
#define MAX_MIRROR_RECOVERY_ATTEMPTS 10
        unsigned short write_fail_count[MAX_MIRRORS];
        int cferom_crc;
}BOOT_BLOCK_MIRROR_INFO;



#define NVRAM_FLASH_WRITE		0
#define NVRAM_SKIP_FLASH_WRITE		1

#endif

/*****************************************************************************/
/*       Misc Offsets                                                        */
/*****************************************************************************/
#define CFE_VERSION_REL_OFFSET       0x10
#define CFE_VERSION_OFFSET           (NVRAM_DATA_OFFSET-CFE_VERSION_REL_OFFSET)
#define CFE_VERSION_MARK             "cfe-v"
#define CFE_VERSION_MARK_SIZE        5
#define CFE_VERSION_SIZE             5

/*****************************************************************************/
/*       Scratch Pad Defines                                                 */
/*****************************************************************************/
/* SP - Persistent Scratch Pad format:
       sp header        : 32 bytes
       tokenId-1        : 8 bytes
       tokenId-1 len    : 4 bytes
       tokenId-1 data    
       ....
       tokenId-n        : 8 bytes
       tokenId-n len    : 4 bytes
       tokenId-n data    
*/

#define MAGIC_NUM_LEN       8
#define MAGIC_NUMBER        "gOGoBrCm"
#define TOKEN_NAME_LEN      16
#define SP_VERSION          1
#define CFE_NVRAM_DATA2_LEN 20

#ifndef _LANGUAGE_ASSEMBLY
typedef struct _SP_HEADER
{
    char SPMagicNum[MAGIC_NUM_LEN];             // 8 bytes of magic number
    int SPVersion;                              // version number
    char NvramData2[CFE_NVRAM_DATA2_LEN];       // not related to scratch pad
                                                // additional NVRAM_DATA
} SP_HEADER, *PSP_HEADER;                       // total 32 bytes

typedef struct _TOKEN_DEF
{
    char tokenName[TOKEN_NAME_LEN];
    int tokenLen;
} SP_TOKEN, *PSP_TOKEN;

#endif

/*****************************************************************************/
/*       Boot Loader Parameters                                              */
/*****************************************************************************/

#define BLPARMS_MAGIC               0x424c504d

#define BOOTED_IMAGE_ID_NAME        "boot_image"

#define BOOTED_NEW_IMAGE            1
#define BOOTED_OLD_IMAGE            2
#define BOOTED_ONLY_IMAGE           3
#define BOOTED_PART1_IMAGE          4
#define BOOTED_PART2_IMAGE          5

#define BOOT_SET_NEW_IMAGE          '0'
#define BOOT_SET_OLD_IMAGE          '1'
#define BOOT_SET_NEW_IMAGE_ONCE     '2'
#define BOOT_GET_BOOTED_IMAGE_ID    '4'
#define BOOT_SET_PART1_IMAGE        '5'
#define BOOT_SET_PART2_IMAGE        '6'
#define BOOT_SET_PART1_IMAGE_ONCE   '7'
#define BOOT_SET_PART2_IMAGE_ONCE   '8'
#define BOOT_GET_BOOT_IMAGE_STATE   '9'
#define BOOT_SET_OLD_IMAGE_ONCE     'A'

#define BOOT_STATES	    4	// the number of boot states

#ifndef _LANGUAGE_ASSEMBLY
typedef struct				
{
    char bootState;
    unsigned char *p;
    int block;
    unsigned int version;
    unsigned int inode;
} JFFS2_SEARCH_STRUCT;
#endif

#define BOARD_ID_NAME               "BOARD_ID"
#define VOICE_BOARD_ID_NAME         "VOICE_ID"
#define BOARD_STUFF_NAME            "STUFF_ID"
#define CFE_VER_NAME		    "CFE_V_ID"
#define BOOTLOADER_NAME             "BOOTLOADER"

#define BOOTLOADER_CFE_STR          "brcmcfe"

#define FLASH_PARTDEFAULT_REBOOT    0x00000000
#define FLASH_PARTDEFAULT_NO_REBOOT 0x00000001
#define FLASH_PART1_REBOOT          0x00010000
#define FLASH_PART1_NO_REBOOT       0x00010001
#define FLASH_PART2_REBOOT          0x00020000
#define FLASH_PART2_NO_REBOOT       0x00020001

#define FLASH_IS_NO_REBOOT(X)       ((X) & 0x0000ffff)
#define FLASH_GET_PARTITION(X)      ((unsigned long) (X) >> 16)

/*****************************************************************************/
/*       Split Partition Parameters                                          */
/*****************************************************************************/
#define BCM_BCMFS_TAG		"BcmFs-"
#define BCM_BCMFS_TYPE_UBIFS	"ubifs"
#define BCM_BCMFS_TYPE_JFFS2	"jffs2"
#define BCM_BCMFS_TYPE_SQUBIFS	"ubifs_sq" /* squashfs over ubi. Note: the prefix must be same with 'BCM_BCMFS_TYPE_UBIFS' !!!
                                              This because BCM Upgrade LIB(bcm_flashutil) did the judgement on 'BCM_BCMFS_TYPE_UBIFS' 
                                              by 'strncmp'. So, the goal is compatible with the legacy software who will feel nothing difference
                                              with ubifs. It will make the upgrade from old image(ubi or jffs2, maybe deployed in field) to new 
                                              image(squashfs over ubi) smoothly. */

/*****************************************************************************/
/*       Global Shared Parameters                                            */
/*****************************************************************************/

#define BRCM_MAX_CHIP_NAME_LEN	16
#define BRCM_MAX_BOOTFS_FILENAME_LEN  (32)

#define UBOOT_HEADER_LEN 12
#define UBOOT_MAX_ENV_LEN 16*1024


#ifdef __cplusplus
}
#endif

#endif /* _BCM_HWDEFS_H */

