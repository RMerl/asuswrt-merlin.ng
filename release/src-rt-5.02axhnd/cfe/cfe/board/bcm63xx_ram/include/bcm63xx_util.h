/***************************************************************************
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
 ***************************************************************************
 * File Name  : bcm63xx_util.h 
 *
 * Created on :  04/18/2002  seanl
 ***************************************************************************/

#if !defined(_BCM63XX_UTIL_H_)
#define _BCM63XX_UTIL_H_

#include "lib_types.h"
#include "lib_string.h"
#include "lib_queue.h"
#include "lib_malloc.h"
#include "lib_printf.h"
#include "lib_crc.h"
#include "cfe_iocb.h"
#include "cfe_device.h"
#include "cfe_console.h"
#include "cfe_devfuncs.h"
#include "cfe_timer.h"
#include "cfe_ioctl.h"
#include "cfe_error.h"
#include "env_subr.h"
#include "ui_command.h"
#include "cfe.h"
#include "net_ebuf.h"
#include "net_ether.h"
#include "net_api.h"
#include "cfe_fileops.h"
#include "bsp_config.h"
#include "cfe_mem.h"
#include "cfe_loader.h"
#include "addrspace.h"
#include "initdata.h"

#include "dev_bcm63xx_flash.h"
#include "bcm_hwdefs.h"
#include "bcmTag.h"
#include "boardparms.h"
#include "boardparms_voice.h"
#include "bcm_map.h"
#include "bcm_memory.h"

extern unsigned long cfe_sdramsize;

void board_bootdevice_init(void);


#ifndef INC_BTRM_BOOT
#define INC_BTRM_BOOT         0
#endif

#define NAND_FLASH_BOOT_IMAGE_LZ      "vmlinux.lz"
#define NAND_FLASH_BOOT_IMAGE_LZ4     "vmlinux.lz4"
#define NAND_FLASH_BOOT_SIG_NAME      "vmlinux.sig"
/* constant C char strings length */
#define C_CSTRLEN(s)                     ((sizeof(s)/sizeof(char))-1)

/* Boot FS file load opitons */
#define BOOT_FILE_LOAD_OPT_COMPRESS       0x1 
#define BOOT_FILE_LOAD_OPT_HASHVERIFY     0x2
#define BOOT_FILE_LOAD_OPT_FLUSHCACHE     0x4

#ifdef CONFIG_CFE_SUPPORT_HASH_BLOCK
#define BOOT_FILE_LOAD_OPT_DEFAULT        BOOT_FILE_LOAD_OPT_HASHVERIFY
#else
#define BOOT_FILE_LOAD_OPT_DEFAULT        0x0
#endif

#ifdef CFG_DT 
#define DTB_SOC_NAME CFG_DTB_IMAGE 
#define DTB_SOC_NAME_LEN  (sizeof(DTB_SOC_NAME)-1)
#endif
#define MAX_NUM_BT_BLKS                 8
#define MAX_BOOT_PARTITION_SIZE         (1024 * 1024) // max gen3 bootrom direct memory access is 1 meg

/* For system with more than 16MB or more, use mem_topofmem as the starting point of image buffer.
   For system with only 8MB memory(6318), keep the old address - phy address 0x0000-0000
   mem_topofmem = end_of_cfe_ram_code_data_bbs+stack+heap
*/   

#define MAX_PROMPT_LEN      50         // assume no one wants to type more than 50 chars 
#define MAX_MAC_STR_LEN     19         // mac address string 18+1 in regular format   
#define PROMPT_DEFINE_LEN   2
#define MASK_LEN            8           // vxworks like ffffff00

typedef struct 
{
   char* promptName;
   char* errorPrompt;
   char  promptDefine[PROMPT_DEFINE_LEN];
   char  parameter[MAX_PROMPT_LEN];
   int   maxValueLength;
   int   (*func)(char *);
   int   enabled;
} PARAMETER_SETTING, *PPARAMETER_SETTING;

#define IP_PROMPT           "Invalid ip address. eg. 192.168.1.200[:ffffff00]"
#define RUN_FROM_PROMPT     "f = jump to flash; h = tftpd from host"
#define HOST_FN_PROMPT      "eg. vmlinux" 
#define FLASH_FN_PROMPT     "eg. bcm963xx_fs_kernel"
#define RAMFS_FN_PROMPT     "eg. ramfs"
#define DTB_FN_PROMPT       "eg. 9xxx.dtb"
#define BOOT_DELAY_PROMPT   "range 0-9, 0=forever prompt"
#define BOOT_PARTITION_PROMPT "1 = latest image, 2 = previous image"
#define AFE_PROMPT           "Invalid AFE ID eg. 0x10608100"
#define DDR_PROMPT           "Invalid DDR override eg. 0x10608100"
#define RD_ADDR_PROMPT       "Invalid RD Addr eg. 0x80800000"

// error input prompts
#define BOARDID_STR_PROMPT   "Invalid board ID"
#define MAC_CT_PROMPT        "Invalid MAC addresses number: 1 - 64"
#define MAC_ADDR_PROMPT      "Invalid MAC address format:  eg. 12:34:56:ab:cd:ef or 123456abcdef"
#define PSI_SIZE_PROMPT      "Invalid PSI size: (1-64) Kbytes"
#define BACKUP_PSI_PROMPT    "Enable Backup PSI (0 or 1)"
#define SYSLOG_SIZE_PROMPT   "Invalid System Log size: (0-256) Kbytes"
#define FLASHBLK_SIZE_PROMPT    "Invalid flash block size (1-128) Kbytes"
#define AUXFS_PERCENT_PROMPT    "Invalid auxillary file system percent (0-80)"
#define CPU_TP_PROMPT        "Invalid thread number: [0|1]"
#define GPON_SN_PROMPT       "Invalid GPON Serial Number"
#define GPON_PW_PROMPT       "Invalid GPON Password"
#define WPS_DEVICE_PIN_PROMPT       "Invalid WPS Device Pin"
#define WLAN_DEVICE_FEATURE_PROMPT  "Invalid WLAN Feature Value"
#define PARTITION_STR_PROMPT   "Invalid parition size"

// bootline definition:
// Space is the deliminator of the parameters.  Currently supports following parameters:
// t=xxx.xxx.xxx.xxx h=xxx.xxx.xxx.xxx g=xxx.xxx.xxx.xxx  r=f/h (run from flash or host)
// f=vmlinux (if r=h) i=bcm963xx_fs_kernel d=3 (default delay, range 0-9, 0=forever prompt)

#define BOOT_IP_LEN         18  // "t=xxx.xxx.xxx.xxx"
#define BOOT_FILENAME_LEN   50	// "f=vmlinux"

typedef struct				
{
    char boardIp[BOOT_IP_LEN];
    char boardMask[BOOT_IP_LEN];        // set for the board only and ignore for the host/gw. fmt :ffffff00
    char hostIp[BOOT_IP_LEN];
    char gatewayIp[BOOT_IP_LEN];
    char runFrom;
    char hostFileName[BOOT_FILENAME_LEN];
    char flashFileName[BOOT_FILENAME_LEN];
    char ramfsFileName[BOOT_FILENAME_LEN];
    char dtbFileName[BOOT_FILENAME_LEN];
    int  bootDelay;
    char bootPartition;
    unsigned int rdAddr;
} BOOT_INFO, *PBOOT_INFO;

extern BOOT_INFO bootInfo;

/* bcm image header */
typedef struct bcm_image_hdr {
    uint32_t la;
    uint32_t entrypt;
    uint32_t len;
    uint32_t magic;
    uint32_t len_uncomp;
} bcm_image_hdr_t;

#define LED_OFF 0
#define LED_ON  1

extern int getBootLine(int setdef);
extern int setDefaultBootline(void);
extern int printSysInfo(void);
extern int changeBootLine(void);
extern int changeAfeId(void);
extern int changeDDRConfig(void);
extern int nvFeatureGet(int tst);
extern int nvFeatureSet(int set, int clr);
#if defined(_BCM94908_) || defined(_BCM963158_)
extern int getAVSConfig(void);
extern void setAVSConfig(int on);
#endif
extern void dumpHex(unsigned char *start, int len);
extern BOOT_INFO bootInfo;
extern void enet_init(void);
extern int verifyTag(PFILE_TAG pTag, int verbose);
extern unsigned int getRomParam(void);
extern int flashImage(unsigned char *ptr);
extern int  writeWholeImage(unsigned char *ptr, int size);
#if INC_NAND_FLASH_DRIVER == 1 || INC_SPI_PROG_NAND == 1 || INC_SPI_NAND_DRIVER == 1
extern int  writeWholeImageDataPartition(unsigned char *ptr, int size, char *partition_number );
extern int  writeWholeImageRaw(unsigned char *ptr, int size, uint32_t offset);
char nandCommand(char command, uint16_t value);
#endif
extern void bcm63xx_run(int breakIntoCfe, int autorun);
extern void bcm63xx_run_ex(int breakIntoCfe, int chk_memcfg, int autorun);
extern int getPartitionFromTag( PFILE_TAG pTag );
extern PFILE_TAG getTagFromPartition(int imageNumber);
extern PFILE_TAG getBootImageTag(void);
extern int findBootImageDirEntry(int state);
#if defined(_BCM94908_) || defined(_BCM96858_) || defined (_BCM963158_) || defined (_BCM96846_) || defined (_BCM96856_)
extern int findBootBlock(void);
#endif
extern int parsexdigit(char str);
extern int setBoardParam(void);
extern int setBoardOptions(void);
extern int setGponBoardParam(void);
extern int setWpsDevicePinBoardParam(void);
extern int setWlanDeviceFeatureBoardParam(void);
extern int setVoiceBoardParam(void);
extern int getBoardParam(void);
extern void displayBoardParam(void);
extern void displayPromptUsage(void);
extern int processPrompt(PPARAMETER_SETTING promptPtr, int promptCt);
extern int yesno(void);
extern int bcm63xx_cfe_rawload(cfe_loadargs_t *la);
extern int bcm63xx_cfe_elfload(cfe_loadargs_t *la);
extern void initGpioPinMux(void);
extern void setLed ( unsigned short led, unsigned short led_state );
extern void setAllLedsOff(void);
extern void setPowerOnLedOn(void);
extern void setBreakIntoCfeLed(void);
void softReset(unsigned int delay);
extern uint32_t getNumBootBlks(const NVRAM_DATA* pNvramData);
extern int validateNandPartTbl(uint32_t frcUpdt, uint32_t numBtBlks);
extern int validateMemoryConfig(const NVRAM_DATA *nvramData, unsigned int* memcfg );
extern void writeNvramData(PNVRAM_DATA pNvramData);
extern int readNvramData(PNVRAM_DATA pNvramData);
extern unsigned long cfe_get_mempool_ptr(void);
extern unsigned int cfe_get_mempool_size(void);
extern void erase_misc_partition(int pnum, const NVRAM_DATA *NVRAM);
#if (INC_NAND_FLASH_DRIVER == 1) || (INC_SPI_NAND_DRIVER == 1)
extern int cfe_fs_find_file(const char* fname, unsigned int fsize, 
                    unsigned int blk, unsigned int bcnt, 
                    unsigned char** dst, unsigned int *dsize,
                    unsigned int dst_offs);
extern int get_rootfs_offset(char boot_state, unsigned int *bstart, unsigned int* bcnt);
extern int cfe_fs_fetch_file(const char* fname, unsigned int fnsize, 
               unsigned char** file, unsigned int* file_size);
#else
extern int getDtbFromTag(unsigned char* dtbBuf, unsigned int* size);
#endif
#if defined(_BCM963138_) || defined(_BCM963148_) || defined(_BCM94908_) || defined (_BCM963158_)
int cfe_set_cpu_freq(int freqMHz);
#endif
#if (BPCM_CFE_CMD==1)
int cfe_read_bpcm(uint8_t bus_id, uint8_t addr_id, uint8_t offset, uint32_t *val);
int cfe_write_bpcm(uint8_t bus_id, uint8_t addr_id, uint8_t offset, uint32_t val);
#endif
extern int setPartitionSizes(void);
#define PARTI_INFO_FORMAT(size, sz_bit) ((size & 0x3fff) | (sz_bit << 14))
#define PARTI_INFO_SIZE_BITS(size) (size>>14)
#define PARTI_INFO_SIZE(size) (size&0x3ff)
extern unsigned long convert_to_data_partition_entry_to_bytes(uint16 size);
void cfe_stack_check(void);
void cfe_stack_info(void);
void cfe_mem_info(void);
int cfe_heap_info(void);
int getSeqNum(int partition);
int commit( int partition, char *string );
unsigned long min_data_partition_size_kb(void);
int cfe_load_boot_file(const char *fname, unsigned char *buf, unsigned int* buflen, unsigned int options);
#ifdef USE_LZ4_DECOMPRESSOR
extern int LZ4_decompress_fast (const char* source, char* dest, int originalSize);
#endif
extern int decompressLZMA(unsigned char *in, unsigned insize, unsigned char *out, unsigned outsize);
extern unsigned int LzmaGetUncompSize(unsigned char *in);

#if defined(_BCM963138_)
int get_nr_cpus(unsigned int *);
#endif

/* UI Command types */
typedef enum {
    BRCM_UI_CMD_TYPE_RESTRICTED,    /* Restructed UI commands may not be available in all operating modes */
    BRCM_UI_CMD_TYPE_NORMAL         /* Normal UI commands will be available in all modes, see is_ui_cmd_restricted() for logic */
} brcm_ui_cmd_type_t;

int brcm_cmd_addcmd(char *command,
    int (*func)(ui_cmdline_t *,int argc,char *argv[]),
    void *ref,
    char *help,
    char *usage,
    char *switches,
    brcm_ui_cmd_type_t cmd_type);
#endif // _BCM63XX_UTIL_H_

