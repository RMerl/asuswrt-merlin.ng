/*  *********************************************************************
    *
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
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  bcm63xx utility functions
    *  
    *  Created on :  04/18/2002  seanl
    *
    *********************************************************************
*/
    
#include "bcm_memory.h"
#include "bcm63xx_util.h"
#include "flash_api.h"
#include "jffs2.h"
#include "boardparms.h"
#include "rom_parms.h"
#include "bcm63xx_ipc.h"
#include "lib_byteorder.h"
#include "lib_malloc.h"
#if INC_EMMC_FLASH_DRIVER        
#include "dev_emmcflash.h"
#endif

#include "bcm63xx_nvram.h"
#if (INC_PMC_DRIVER==1)
#include "pmc_drv.h"
#include "BPCM.h"
#endif
#include "bcm_ubi.h"
#include "bcm_otp.h"
#include "btrm_if.h"
#include "bcm_map_part.h"
#include "shared_utils.h"
#include "bcm63xx_auth.h"
#if defined(BOARD_SEC_ARCH)
#include "bcm63xx_sec.h"
#endif



#define je16_to_cpu(x) ((x).v16)
#define je32_to_cpu(x) ((x).v32)

int cfe_fs_find_file(const char* fname, unsigned int fsize, 
                    unsigned int blk, unsigned int bcnt, 
                    unsigned char** dst, unsigned int *dsize,
                    unsigned int dst_offs); 
static void convertBootInfo(void);
static int checkChipId(int tagChipId, char *sig2);
static int brcm_enforce_ui_cmd_restrictions( void );
static void UpdateImageSequenceNumber( char *imageSequence );
static int flashImageSpiNor( PFILE_TAG pTag, uint8_t *imagePtr ); 
int get_rootfs_offset(char boot_state, unsigned int *bstart, unsigned int* bcnt);
int cfe_fs_fetch_file(const char* fname, unsigned int fnsize,
               unsigned char** file, unsigned int* file_size);
int cfe_load_image(const char* fname, void* ddr_addr, unsigned long max_size);

const int overhead_blocks = 8;
BOOT_INFO bootInfo;

extern unsigned short g_force_mode;
extern unsigned long mem_stacktop;
extern unsigned long mem_stackbottom;
extern unsigned long get_SP(void);
extern void _cfe_flushcache(int, uint8_t *, uint8_t *);

void cfe_stack_check(void)
{
    unsigned long _SP = get_SP();
    if (_SP >= mem_stacktop || _SP < mem_stackbottom) {
	printf("FATAL ERROR: stack was corrupted: [BOT: 0x%lx  SP: 0x%lx TOP: 0x%lx SZ: 0x%lx]. Halting...\n",
                             mem_stackbottom, _SP, mem_stacktop, CFG_STACK_SIZE);
	while(1); 
    }
    if (_SP - mem_stackbottom < CFG_STACK_WARN_TRESHOLD*16*sizeof(void*) 
                              || mem_stacktop - _SP == sizeof(void*)) {

	printf("Warning: stack near it's boundaries: [BOT:0x%lx TOP: 0x%lx SP: 0x%lx SZ: 0x%xl]\n",
                             mem_stackbottom, _SP, mem_stacktop, CFG_STACK_SIZE);
    }
}

void cfe_stack_info(void)
{
    printf("\n"); 
    printf("                Stack                        \n");
    printf("Range              :  0x%08lx - 0x%08lx size : 0x%08lx SP:0x%08lx\n", mem_stackbottom, 
                                                                   mem_stacktop, CFG_STACK_SIZE,get_SP());
    printf("\n"); 
}

int cfe_heap_info(void)
{
    int res;
    memstats_t stats;

    res = KMEMSTATS(&stats);

    xprintf("\n");
    xprintf("                Heap                         \n");
    xprintf("Range             :  0x%08lx - 0x%08lx \n", _VA(mem_heapstart), _VA(mem_topofmem));
    xprintf("Total bytes       :  %u\n",stats.mem_totalbytes);
    xprintf("Free bytes        :  %u\n",stats.mem_freebytes);
    xprintf("Free nodes        :  %u\n",stats.mem_freenodes);
    xprintf("Allocated bytes   :  %u\n",stats.mem_allocbytes);
    xprintf("Allocated nodes   :  %u\n",stats.mem_allocnodes);
    xprintf("Largest free node :  %u\n",stats.mem_largest);
    xprintf("Heap status       :  %s\n",(res == 0) ? "CONSISTENT" : "CORRUPT!");
    xprintf("\n");
    return res;
}

void cfe_mem_info(void)
{
    printf("\n"); 
    printf("                 System Memory              \n");
    printf("Memory             :  0x%08lx - 0x%08lx size %u\n", MEMORY_AREA_RSRV_ADDR, 
                MEMORY_AREA_RSRV_ADDR + cfe_get_sdram_size() - 1, cfe_get_sdram_size()); 
    printf("Reserved/Boot      :  0x%08lx - 0x%08lx \n", MEMORY_AREA_RSRV_ADDR, _VA(mem_bottomofmem));
    printf("Reserved           :  0x%08lx - 0x%08lx \n",_VA(mem_bottomofmem), _VA(mem_topofmem)); 
    printf("Free Memory        :  0x%08lx - 0x%08lx size %u\n", MEMORY_AREA_FREE_MEM, 
                                                          MEMORY_AREA_RSRV_ADDR + cfe_get_sdram_size() - 1,
                                                          (MEMORY_AREA_RSRV_ADDR + cfe_get_sdram_size()) - MEMORY_AREA_FREE_MEM);
    printf("\n");
}

unsigned long cfe_get_mempool_ptr(void)
{
    return MEMORY_AREA_FREE_MEM;
}

unsigned int cfe_get_mempool_size(void)
{
    return MEMORY_AREA_FREE_MEM_SIZE;
}

static int parseFilename(char *fn)
{
    if (strlen(fn) < BOOT_FILENAME_LEN)
        return 0;
    else
        return 1;
}

static int parseChoiceFh(char *choice)
{

#if ( defined(_BCM963138_)||defined(_BCM963148_) ) && (INC_SPI_PROG_NAND==1)
    if (*choice == 'n')
        return 0;
#endif
    if (*choice == 'f' || *choice == 'h' || *choice == 'c')
        return 0;
    else
        return 1;
}


static int parseBootPartition(char *choice)
{
    return( (*choice == BOOT_SET_NEW_IMAGE || *choice == BOOT_SET_OLD_IMAGE)
        ? 0 : 1 );
}

static int parseChoice09(char *choice)
{
    int bChoice = *choice - '0';

    if (bChoice >= 0 && bChoice <= 9)
        return 0;
    else
        return 1;
}

static int parseIpAddr(char *ipStr);
static int parseGwIpAddr(char *ipStr);
static int parseAfeId(char *afeIdStr);
#if defined(_BCM963138_) || defined(_BCM963148_) || defined(_BCM94908_) || defined(_BCM96848_) || \
    defined(_BCM963158_) || defined(_BCM96858_) || defined(_BCM96846_) || defined(_BCM96856_)
static int parseDdrConfig(char *afeIdStr);
#endif

#define PARAM_IDX_BOARD_IPADDR              0
#define PARAM_IDX_HOST_IPADDR               1
#define PARAM_IDX_GW_IPADDR                 2
#define PARAM_IDX_RUN_FROM                  3
#define PARAM_IDX_RUN_FILENAME              4
#define PARAM_IDX_FLASH_FILENAME            5
#define PARAM_IDX_BOOT_DELAY                6
#define PARAM_IDX_BOOT_IMAGE                7
#define PARAM_IDX_RAMFS_FILENAME            8
#define PARAM_IDX_RD_ADDR                   9
#define PARAM_IDX_DTB_FILENAME              10

static PARAMETER_SETTING gBootParam[] =
{
    // prompt name                  Error Prompt    Boot Define Boot Param  Validation function
    {"Board IP address                  :", IP_PROMPT       , "e=",
        "", 24, parseIpAddr, TRUE}, // index 0
    {"Host IP address                   :", IP_PROMPT       , "h=",
        "", 15, parseIpAddr, TRUE}, // index 1
    {"Gateway IP address                :", IP_PROMPT       , "g=",
        "", 15, parseGwIpAddr, TRUE}, // index 2
#if ( defined(_BCM963138_)||defined(_BCM963148_)) && (INC_SPI_PROG_NAND==1)
    {"Run from flash/host/nand/tftp (f/h/n/c)  :", RUN_FROM_PROMPT , "r=",
        "", 1, parseChoiceFh, TRUE}, // index 3
#else
    {"Run from flash/host/tftp (f/h/c)  :", RUN_FROM_PROMPT , "r=",
        "", 1, parseChoiceFh, TRUE}, // index 3
#endif
    {"Default host run file name        :", HOST_FN_PROMPT  , "f=",
        "", MAX_PROMPT_LEN - 1, parseFilename, TRUE}, // index 4
    {"Default host flash file name      :", FLASH_FN_PROMPT , "i=",
        "", MAX_PROMPT_LEN - 1, parseFilename, TRUE}, // index 5
    {"Boot delay (0-9 seconds)          :", BOOT_DELAY_PROMPT, "d=",
        "", 1, parseChoice09, TRUE}, // index 6
    {"Boot image (0=latest, 1=previous) :", BOOT_PARTITION_PROMPT, "p=",
        "", 1, parseBootPartition, TRUE}, // index 7
    {"Default host ramdisk file name    :", RAMFS_FN_PROMPT, "c=",
        "", MAX_PROMPT_LEN - 1, parseFilename, TRUE}, // index 8
    {"Default ramdisk store address     :", RD_ADDR_PROMPT, "a=",
        "", 12, parseAfeId, TRUE}, // index 9
    {"Default DTB file name             :", DTB_FN_PROMPT, "g=",
        "", MAX_PROMPT_LEN - 1, parseFilename, TRUE}, // index 10
    {NULL},
};

static int gNumBootParams = (sizeof(gBootParam) / sizeof(PARAMETER_SETTING))-1;

static PARAMETER_SETTING gAfeId[] =
{
    // prompt name                  Error Prompt    Boot Define Boot Param  Validation function
    {"Primary AFE ID                  :", AFE_PROMPT, "", "", 12, parseAfeId, TRUE}, // index 0
    {"Bonding AFE ID                  :", AFE_PROMPT, "", "", 12, parseAfeId, TRUE}, // index 1
    {NULL},
};
static int gAfeIdParams = (sizeof(gAfeId) / sizeof(PARAMETER_SETTING))-1;


#if defined(_BCM963138_) || defined(_BCM963148_) || defined(_BCM94908_) || defined(_BCM96848_) || \
    defined(_BCM963158_) || defined(_BCM96858_) || defined(_BCM96846_) || defined(_BCM96856_)
static PARAMETER_SETTING  gMemoryConfig[] =
{
    // prompt name                  Error Prompt    Boot Define Boot Param  Validation function
    {"DDR Config OVERRIDE             :", DDR_PROMPT, "", "", 12, parseDdrConfig, TRUE}, // index 0
    {NULL},
};
static int gMemoryConfigParams = (sizeof(gMemoryConfig) / sizeof(PARAMETER_SETTING))-1;
#endif

// move from lib_misc.c
int parseipaddr(const char *ipaddr,uint8_t *dest)
{
    int a,b,c,d;
    char *x;

    /* make sure it's all digits and dots. */
    x = (char *) ipaddr;
    while (*x) {
	if ((*x == '.') || ((*x >= '0') && (*x <= '9'))) {
	    x++;
	    continue;
	    }
	return -1;
	}

    x = (char *) ipaddr;
    a = lib_atoi(ipaddr);
    x = lib_strchr(x,'.');
    if (!x) return -1;
    b = lib_atoi(x+1);
    x = lib_strchr(x+1,'.');
    if (!x) return -1;
    c = lib_atoi(x+1);
    x = lib_strchr(x+1,'.');
    if (!x) return -1;
    d = lib_atoi(x+1);

    if ((a < 0) || (a > 255)) return -1;
    if ((b < 0) || (b > 255)) return -1;
    if ((c < 0) || (c > 255)) return -1;
    if ((d < 0) || (d > 255)) return -1;

    dest[0] = (uint8_t) a;
    dest[1] = (uint8_t) b;
    dest[2] = (uint8_t) c;
    dest[3] = (uint8_t) d;

    return 0;
}

#if 0
static const char hextable[16] = "0123456789ABCDEF";
void dumpHex(unsigned char *start, int len)
{
    unsigned char *ptr = start,
    *end = start + len;

    while (ptr < end)
    {
        long offset = ptr - start;
        unsigned char text[120],
        *p = text;
        while (ptr < end && p < &text[16 * 3])
        {
            *p++ = hextable[*ptr >> 4];
            *p++ = hextable[*ptr++ & 0xF];
            *p++ = ' ';
        }
        p[-1] = 0;
        printf("%4lX %s\n", offset, text);
    }
}

#endif

int parsexdigit(char str)
{
    int digit;

    if ((str >= '0') && (str <= '9')) 
        digit = str - '0';
    else if ((str >= 'a') && (str <= 'f')) 
        digit = str - 'a' + 10;
    else if ((str >= 'A') && (str <= 'F')) 
        digit = str - 'A' + 10;
    else 
        return -1;

    return digit;
}


// convert in = ffffff00 to out=255.255.255.0
// return 0 = OK, 1 failed.
static int convertMaskStr(char *in, char *out)
{
    int i;
    char twoHex[4];
    uint8_t dest[4];
    char mask[BOOT_IP_LEN];

    if (strlen(in) != MASK_LEN)      // mask len has to 8
        return 1;

    memset(twoHex, 0, sizeof(twoHex));
    for (i = 0; i < 4; i++)
    {
        twoHex[0] = (uint8_t)*in++;
        twoHex[1] = (uint8_t)*in++;
        if (parsexdigit(*twoHex) == -1)
            return 1;
        dest[i] = (uint8_t) xtoi(twoHex);
    }
    sprintf(mask, "%d.%d.%d.%d", dest[0], dest[1], dest[2], dest[3]);
    strcpy(out, mask);
    return 0;    
}

// return 0 - OK, !0 - Bad ip
static int parseIpAddr(char *ipStr)
{
    char *x;
    uint8_t dest[4];
    char mask[BOOT_IP_LEN];       
    char ipMaskStr[2*BOOT_IP_LEN];

    strcpy(ipMaskStr, ipStr);

    x = strchr(ipMaskStr,':');
    if (!x)                     // no mask
        return parseipaddr(ipMaskStr, dest);

    *x = '\0';

    if (parseipaddr(ipMaskStr, dest))        // ipStr is not ok
        return 1;

    x++;
    return convertMaskStr(x, mask);      // mask is not used here

}

// return 0 - OK, !0 - Bad ip
static int parseGwIpAddr(char *ipStr)
{
    int ret = 0;
    if( *ipStr )
        ret = parseIpAddr(ipStr);
    return(ret);
}

// return 0 - OK, !0 - Bad ip
static int parseAfeId(char *afeIdStr)
{
	return 0;
}

#if defined(_BCM963138_) || defined(_BCM963148_) || defined(_BCM94908_) || defined(_BCM96848_) || \
    defined(_BCM963158_) || defined(_BCM96858_) || defined(_BCM96846_) || defined(_BCM96856_)
static int parseDdrConfig(char *afeIdStr)
{
	return 0;
}
#endif

#if (SKIP_FLASH==0)
// port from ifconfig command in ui_netcmds.c
void enet_init(void)
{
    char devname[] = "eth0";
    uint8_t addr[IP_ADDR_LEN];
    int res;
    if (net_getparam(NET_DEVNAME) == NULL) {
        res = net_init(devname);		/* turn interface on */
        if (res < 0) {
            ui_showerror(res, "Could not activate network interface '%s'", devname);
            return;
        }
    }

    net_setparam(NET_HWADDR, (uint8_t*)NVRAM.ucaBaseMacAddr);

    parseipaddr(bootInfo.boardIp, addr);
    net_setparam(NET_IPADDR, addr);
    
    if (strlen(bootInfo.boardMask) > 0) {
        parseipaddr(bootInfo.boardMask, addr);
        net_setparam(NET_NETMASK, addr);
    }

    if (strlen(bootInfo.gatewayIp) > 0) {
        parseipaddr(bootInfo.gatewayIp, addr);
        net_setparam(NET_GATEWAY, addr);
    }

    net_setnetvars();
}
#else
void enet_init(void)
{
    char devname[] = "eth0";
    int res;

    if (net_getparam(NET_DEVNAME) == NULL) {
        res = net_init(devname);		/* turn interface on */
        if (res < 0) {
            ui_showerror(res, "Could not activate network interface '%s'", devname);
            return;
        }
    }

    net_setparam(NET_HWADDR, (unsigned char *) "\x00\x10\x18\x00\x00\x00");
    net_setparam(NET_IPADDR, (unsigned char *) "\xc0\xa8\x01\x01");
}
#endif

// return 0, ok. return -1 = wrong chip
static int checkChipId(int tagChipId, char *sig2)
{
    int result = 0;

#if defined (_BCM960333_)
    /* DUNA TODO: Provide a specific implementation */
#elif defined(_BCM947189_)
    /* 47189 TODO: Provide a specific implementation */
#else
    unsigned int chipId = (PERF->RevID & CHIP_ID_MASK) >> CHIP_ID_SHIFT;

    /* Force BCM63268 variants to be BCM63268) */
    if (( (chipId & 0xffffe) == 0x63168 )
     || ( (chipId & 0xffffe) == 0x63268 ))
        chipId = 0x63268;

    if (tagChipId == chipId)
        result = 0;
    else {
#if defined(CHIP_FAMILY_ID_HEX)
        if (tagChipId == CHIP_FAMILY_ID_HEX) {
	    result = 0;
        }
        else
#endif
        {
            printf("Chip Id error.  Image Chip Id = %04x, Board Chip Id = %04x.\n", tagChipId, chipId);
            if (g_force_mode != 0) {
                result = 0;
            } else {
                result = -1;
            }
        }
    }
#endif
    return result;
}

// return -1: fail.
//         0: OK.
int verifyTag( PFILE_TAG pTag, int verbose )
{
    UINT32 crc;
    FLASH_ADDR_INFO info;
    int tagVer, curVer;

    kerSysFlashAddrInfoGet( &info );

    tagVer = atoi(pTag->tagVersion);
    curVer = atoi(BCM_TAG_VER);

    if (tagVer != curVer)
    {
        if( verbose )
        {
            printf("Firmware tag version [%d] is not compatible with the current Tag version [%d].\n", \
                tagVer, curVer);
        }
        return -1;
    }

    if (checkChipId(xtoi(pTag->chipId), pTag->signiture_2) != 0)
        return -1;

    // check tag validate token first
    crc = CRC32_INIT_VALUE;
    crc = getCrc32((byte *) pTag, (UINT32)TAG_LEN-TOKEN_LEN, crc);      

    if (crc != (UINT32)(*(UINT32*)(pTag->tagValidationToken)))
    {
        if( verbose )
            printf("Illegal image ! Tag crc failed.\n");
        return -1;
    }
    return 0;
}

/*
* Returns content of memory location 
* where cfe rom param is stored
*/
unsigned int getRomParam(void)
{
#if INC_NAND_FLASH_DRIVER == 1 || INC_SPI_PROG_NAND == 1 || INC_SPI_NAND_DRIVER == 1 || INC_EMMC_FLASH_DRIVER == 1
        /* CFE ROM boot loader stored rom param at the
         * memory location before CFE RAM load address -16 offset 
         * Used later to enable run-time functionality.
         */
        return CFE_RAM_ROM_PARMS_GET_ROM_PARM;
#else
        return 0;
#endif
}

#if (INC_NAND_FLASH_DRIVER==0)
PFILE_TAG getTagFromPartition(int imageNumber)
{
    static unsigned char sectAddr1[sizeof(FILE_TAG)];
    static unsigned char sectAddr2[sizeof(FILE_TAG)];
    int blk = 0;
    UINT32 crc;
    PFILE_TAG pTag = NULL;
    unsigned char *pBase = flash_get_memptr(0);
    unsigned char *pSectAddr = NULL;
    unsigned int reserverdBytersAuxfs = flash_get_reserved_bytes_auxfs();
    unsigned int sectSize = (unsigned int) flash_get_sector_size(0);
    unsigned int offset;

    /* The image tag for the first image is always after the boot loader.
     * The image tag for the second image, if it exists, is at one half
     * of the flash size.
     */
    if( imageNumber == 1 )
    {

        FLASH_ADDR_INFO flash_info;

        kerSysFlashAddrInfoGet(&flash_info);
        blk = flash_get_blk((int)((uintptr_t)pBase+flash_info.flash_rootfs_start_offset));
        pSectAddr = sectAddr1;
    }
    else
        if( imageNumber == 2 )
        {
            offset = ((flash_get_total_size()-reserverdBytersAuxfs) / 2);

            /* make sure offset is on sector boundary, image starts on sector boundary */
            if( offset % sectSize )
                offset = ((offset/sectSize)+1)*sectSize;

            blk = flash_get_blk((int) ((uintptr_t)pBase + offset + IMAGE_OFFSET));
            pSectAddr = sectAddr2;
        }

    if( blk )
    {
        memset(pSectAddr, 0x00, sizeof(FILE_TAG));
        flash_read_buf((unsigned short) blk, 0, pSectAddr, sizeof(FILE_TAG));
        crc = CRC32_INIT_VALUE;
        crc = getCrc32(pSectAddr, (UINT32)TAG_LEN-TOKEN_LEN, crc);      
        pTag = (PFILE_TAG) pSectAddr;
        if (crc != (UINT32)(*(UINT32*)(pTag->tagValidationToken)))
            pTag = NULL;
    }

    return( pTag );
}

#ifdef CFG_DT
int getDtbFromTag(unsigned char* dtbBuf, unsigned int* size)
{
    int ret = 0;
    PFILE_TAG pTag = getBootImageTag();
    uint32_t* pDtbFlashAddr = NULL;
    uint32_t dtbSize, dtbCrc, ulCrc;
    
    if( pTag == NULL )
        return -1;

    if( atoi(pTag->tagVersion) < BCM_TAG_VER_DTB )
        return -2;

    dtbSize = atoi(pTag->dtbLen);
    if (*size < dtbSize) {
        return -3;
    }
    pDtbFlashAddr = (uint32_t*)(uintptr_t)(atoi(pTag->dtbAddress) + BOOT_OFFSET + IMAGE_OFFSET);
    ret = kerSysReadFromFlash( dtbBuf, (uintptr_t)pDtbFlashAddr, dtbSize);
    if( ret != dtbSize )  {
        return -4;
    }
    /* Check dtb CRC */
    dtbCrc = *(unsigned int *) (pTag->imageValidationToken + (CRC_LEN * 3));
    if( dtbCrc )
    {
        ulCrc = CRC32_INIT_VALUE;
        ulCrc = getCrc32((unsigned char *)dtbBuf, dtbSize, ulCrc);      
        if( ulCrc != dtbCrc)
        {
            printf("Dtb CRC error.  Corrupted image?\n");
            return -5;
        }
    }

    *size = dtbSize;

    return 0;
}
#endif
#else

static unsigned int read_blk(unsigned char * start, unsigned int block, unsigned int offset, unsigned int blk_size, unsigned char * buf, unsigned int amount, void * mtd, int mtd_fd)
{
    return(flash_read_buf(block, offset, buf + offset, amount));
}


static unsigned int write_blk(unsigned char * start, unsigned int block, unsigned int offset, unsigned int blk_size, unsigned char * buf, unsigned int amount, void * mtd, int mtd_fd)
{
    return(flash_write_buf(block, offset, buf + offset, amount));
}


static unsigned int erase_blk(unsigned int block, unsigned int blk_size, void * mtd, int mtd_fd)
{
    return(flash_sector_erase_int(block));
}


#define tag_not_searched    0
#define tag_not_found       1
#define tag_found           2
PFILE_TAG getTagFromPartition(int imageNumber)
{
    static FILE_TAG Tag1 = {{tag_not_searched}};
    static FILE_TAG Tag2 = {{tag_not_searched}};
    PFILE_TAG pTag = (imageNumber == 2) ? &Tag2 : &Tag1;
    PFILE_TAG ret = NULL;

    switch( pTag->tagVersion[0] )
    {
    case tag_not_searched:
    {
        int rootfs = (imageNumber == 2) ? NP_ROOTFS_2 : NP_ROOTFS_1;
        char fname[] = NAND_CFE_RAM_NAME;
        int fname_actual_len = strlen(fname);
        int fname_cmp_len = strlen(fname) - 3; /* last three are digits */
        unsigned char *p;
        int len = flash_get_sector_size(0);
        int num_blks = flash_get_numsectors();
        int i, done, start_blk, end_blk;
        struct jffs2_raw_dirent *pdir;
        struct ubi_ec_hdr *ec = NULL;
        unsigned int version = 0;
	
        NVRAM_DATA *nvramData = KMALLOC(sizeof(NVRAM_DATA),sizeof(void*));
        if (!nvramData) {
            return NULL;
        }
        NVRAM_COPY_TO(nvramData);

#if !defined(_BCM960333_) && !defined(_BCM96848_) && !defined(_BCM947189_)
        /* If full (field) secure boot is in play, the CFE RAM file is the encrypted version */
	if (bcm_otp_is_boot_secure())
	   strcpy(fname, NAND_CFE_RAM_SECBT_NAME);
#if defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || \
    defined(_BCM96846_) || defined(_BCM96856_)
    /* For these targets, the secure boot could be mfg mode or field mode. */
	else
    {
        if (bcm_otp_is_boot_mfg_secure())
            strcpy(fname, NAND_CFE_RAM_SECBT_MFG_NAME);
    }
#endif
#endif

        /* If the CFE ROM boot loader saved the sequence numbers at the
         * memory location before CFE RAM load address, then store them
         * in the TAG structure and skip searching for them.
         */
        /* 63138 and 63148 use 16KB below the cfe ram image as the mmu table,
         * so rfs number, cfe number has to be saved 16K down further */

        if( (CFE_RAM_ROM_PARMS_GET_SEQ_P1 & ~0xffff) == NAND_SEQ_MAGIC &&
            (CFE_RAM_ROM_PARMS_GET_SEQ_P2 & ~0xffff) == NAND_SEQ_MAGIC )
        {
            if( (CFE_RAM_ROM_PARMS_GET_SEQ_P1 & 0xffff) != 0xffff )
            {
                Tag1.tagVersion[0] = tag_found;
                sprintf(Tag1.imageSequence, "%d", (CFE_RAM_ROM_PARMS_GET_SEQ_P1 & 0xffff)%1000);
            }
            else
                Tag1.tagVersion[0] = tag_not_found;

            if( (CFE_RAM_ROM_PARMS_GET_SEQ_P2 & 0xffff) != 0xffff )
            {
                Tag2.tagVersion[0] = tag_found;
                sprintf(Tag2.imageSequence, "%d", (CFE_RAM_ROM_PARMS_GET_SEQ_P2 & 0xffff)%1000);
            }
            else
                Tag2.tagVersion[0] = tag_not_found;

            if(pTag->tagVersion[0] == tag_found)
                ret = pTag;

            break;
        }

        pTag->tagVersion[0] = tag_not_found;
        validateNandPartTbl(0, 0);

        if( NVRAM.ulNandPartOfsKb[rootfs] > 0 &&
            NVRAM.ulNandPartOfsKb[rootfs] < ((num_blks * len) / 1024) &&
            NVRAM.ulNandPartSizeKb[rootfs] > 0 &&
            NVRAM.ulNandPartSizeKb[rootfs] < ((num_blks * len) / 1024) )
        {
            const int max_not_jffs2 = MAX_NOT_JFFS2_VALUE;
            int read_len;
            int not_jffs2 = 0;

            unsigned int type = 0; // image type

            unsigned char* buf = KMALLOC(len + 1024, sizeof(void*));
            if (!buf) {
                KFREE(nvramData);
                return NULL;
            }
            start_blk = NVRAM.ulNandPartOfsKb[rootfs] / (len / 1024);
            end_blk =
                start_blk + (NVRAM.ulNandPartSizeKb[rootfs] / (len / 1024));

            /* Find the directory entry. */
            for( i = start_blk, done = 0; (i < end_blk) && (done == 0); i++ )
            {
                if( (read_len = flash_read_buf(i, 0, buf, 4)) > 0 )
                {
                    pdir = (struct jffs2_raw_dirent *) buf;
                    ec = (struct ubi_ec_hdr *) buf;

                    if(je16_to_cpu(pdir->magic) == JFFS2_MAGIC_BITMASK)
                    {
                        read_len = flash_read_buf(i, 0, buf, len);
                        if (!type)
                            type = JFFS2_IMAGE;
                    }
                    else if (be32_to_cpu(ec->magic) == UBI_EC_HDR_MAGIC)
                    {
                        if (!type)
                            type = UBI_IMAGE;
                    }
                    else
                        read_len = 0;
                }

                if( read_len > 0 )
                {
                    p = buf;
                    if (type == UBI_IMAGE)
                    {
                        char imageSequence[3];
                        int try;

                        for (try = 0; try < 2; try++)
                        {
                            if (parse_ubi(0, buf, i, end_blk, len, (try ? VOLID_METADATA_COPY : VOLID_METADATA), fname, imageSequence, 0, 0, read_blk, 0, 0, 0, 0) == 3)
                            {
                                pTag->imageSequence[0] = imageSequence[0];
                                pTag->imageSequence[1] = imageSequence[1];
                                pTag->imageSequence[2] = imageSequence[2];
                                pTag->imageSequence[3] = '\0';
                                pTag->tagVersion[0] = tag_found;

                                ret = pTag;
                            }
                        }

                        done = 1;
                    }
                    else while( p < buf + len )
                    {
                        pdir = (struct jffs2_raw_dirent *) p;
                        if( je16_to_cpu(pdir->magic) == JFFS2_MAGIC_BITMASK &&
                            je32_to_cpu(pdir->hdr_crc) == getCrc32(p,
                                sizeof(struct jffs2_unknown_node) - 4, 0) )
                        {
                            if (!type)
                                type = JFFS2_IMAGE;

                            if( je16_to_cpu(pdir->nodetype) ==
                                    JFFS2_NODETYPE_DIRENT &&
                                fname_actual_len == pdir->nsize &&
                                !memcmp(fname, pdir->name, fname_cmp_len) &&
                                je32_to_cpu(pdir->version) > version &&
                                je32_to_cpu(pdir->ino) != 0 )
                            {
                                unsigned char *seq =
                                    pdir->name + fname_cmp_len;
                                pTag->imageSequence[0] = seq[0];
                                pTag->imageSequence[1] = seq[1];
                                pTag->imageSequence[2] = seq[2];
                                pTag->imageSequence[3] = '\0';
                                pTag->tagVersion[0] = tag_found;

                                version = je32_to_cpu(pdir->version);

                                /* Setting 'done = 1' assumes there is
                                 * only one version of the directory
                                 * entry. There will be more than one
                                 * version if the file (cferam.xxx) is
                                 * renamed, modified, copied, etc after
                                 * it was initially flashed.
                                 */
                                done = NAND_FULL_PARTITION_SEARCH ^ 1;
                                ret = pTag;
                                if( done )
                                    break;
                            }

                            p += (je32_to_cpu(pdir->totlen) + 0x03) & ~0x03;
                            not_jffs2 = 0;
                        }
                        else
                            break;
                    }
                }
                else
                {
                    /* If no JFFS2 magic bitmask for consecutive blocks,
                     * assume this partion does not have a file system
                     * on it.
                     */
                    if( max_not_jffs2 > 0 &&  not_jffs2++ > max_not_jffs2 )
                        break;
                }
            }
            KFREE(buf);
        }
	KFREE(nvramData);
    }
    
    break;

    case tag_found:
        ret = pTag;
        break;

    case tag_not_found:
        ret = NULL;
        break;
    }

    return(ret);
}

/* Parses JFFS2 /data file system partition looking for the file name that
 * contains the current boot state.  If the supplied boot state parameter,
 * state, is -1, the boot state in the file name is returned.  Otherwise, the
 * file name is changed to use boot state parameter and written to flash.
 * kerSysGetBootImageState() calls this function with -1
 * kerSysSetBootImageState(state) calls this function
 */
int findBootImageDirEntry(int state)
{
    char fname[] = NAND_BOOT_STATE_FILE_NAME;
    int fname_actual_len = strlen(fname);
    int fname_cmp_len = strlen(fname) - 1; /* last one is digit */
    unsigned char *p;
    int len = flash_get_sector_size(0);
    int num_blks = flash_get_numsectors();
    int i, j, done, start_blk, end_blk;
    struct jffs2_raw_dirent *pdir;
    unsigned int version = 0;
    JFFS2_SEARCH_STRUCT jffs2_item[4];
    int ret = -1;
    struct ubi_ec_hdr *ec;

    validateNandPartTbl(0, 0);


    if( NVRAM.ulNandPartOfsKb[NP_DATA] > 0 &&
        NVRAM.ulNandPartOfsKb[NP_DATA] < ((num_blks * len) / 1024) )
    {
        int read_len;
        unsigned char *buf = KMALLOC(len + 1024, sizeof(void*));
        if (!buf) {
	    return ret;
        }

        start_blk = NVRAM.ulNandPartOfsKb[NP_DATA] / (len / 1024);
        end_blk = start_blk + (NVRAM.ulNandPartSizeKb[NP_DATA] / (len / 1024));

        for (j = 0; j < BOOT_STATES; j++)
        {
            jffs2_item[j].bootState = 0;
            jffs2_item[j].version = 0;
        }

        /* Find the directory entry. */
        for( i = start_blk, done = 0; (i < end_blk) && (done == 0); i++ )
        {
            if( (read_len = flash_read_buf(i, 0, buf, 4)) > 0 )
            {
                pdir = (struct jffs2_raw_dirent *) buf;
                ec = (struct ubi_ec_hdr *) buf;

                if( je16_to_cpu(pdir->magic) == JFFS2_MAGIC_BITMASK )
                    read_len = flash_read_buf(i, 0, buf, len);
                else if (be32_to_cpu(ec->magic) == UBI_EC_HDR_MAGIC)
                {
                    KFREE(buf);
                    return (state == -1) ? BOOT_SET_NEW_IMAGE : 0; // cannot read pureUBI data partition from CFE
                }
                else
                    read_len = 0;
            }

            if( read_len > 0 )
            {
                p = buf;
                while( p < buf + len )
                {
                    pdir = (struct jffs2_raw_dirent *) p;
                    if( je16_to_cpu(pdir->magic) == JFFS2_MAGIC_BITMASK &&
                        je32_to_cpu(pdir->hdr_crc) == getCrc32(p,
                            sizeof(struct jffs2_unknown_node) - 4, 0) )
                    {
                        if( je16_to_cpu(pdir->nodetype) ==
                                JFFS2_NODETYPE_DIRENT &&
                            fname_actual_len == pdir->nsize &&
                            !memcmp(fname, pdir->name, fname_cmp_len) )
                        {
                            int slot = 0;

                            for (j = 0; j < BOOT_STATES; j++)
                            { // check to see if we found this boot state
                                if (jffs2_item[j].bootState == pdir->name[fname_cmp_len])
                                {
                                    slot = j;
                                    break;
                                }
                                if (!jffs2_item[j].bootState)
                                    slot = j;
                            }

                            if (je32_to_cpu(pdir->version) > jffs2_item[slot].version)
                            { // save the directory entry
                                jffs2_item[slot].bootState = pdir->name[fname_cmp_len];
                                jffs2_item[slot].version = je32_to_cpu(pdir->version);
                                jffs2_item[slot].inode = je32_to_cpu(pdir->ino);
                                jffs2_item[slot].p = p;
                                jffs2_item[slot].block = i;
                            }
                        }

                        p += (je32_to_cpu(pdir->totlen) + 0x03) & ~0x03;
                    }
                    else
                        break;
                }
            }
        }

        i = -1;
        for (j = 0; j < BOOT_STATES; j++)
        { // find highest version boot state existing file
            if (jffs2_item[j].bootState && jffs2_item[j].inode && (jffs2_item[j].version > version) )
            {
                version = jffs2_item[j].version;
                i = j;
            }
        }

        if (i != -1)
        {
            if( state == -1 )
                /* Return the current boot state which is the
                 * last character of the file name.
                 */
                ret = (int) jffs2_item[i].bootState;
            else
            {
                /* Change the last character of the file name
                 * to the state parameter.
                 */
                pdir = (struct jffs2_raw_dirent *) jffs2_item[i].p;
                i = jffs2_item[i].block;
                read_len = flash_read_buf(i, 0, buf, len);
                pdir->name[fname_cmp_len] = (char) state;
                je32_to_cpu(pdir->name_crc) = getCrc32(
                    pdir->name, (unsigned int)
                    fname_actual_len, 0);
                flash_sector_erase_int(i);
                if( flash_write_buf(i, 0, buf, len) > 0 )
                    ret = 0;
                else
                    printf("Error updating boot state name\n");
            }
        }

        KFREE(buf);
    }
    return( ret );
}


/***********************************************************************
 * Function Name: getType
 * Description  : Returns the image type JFFS2 or pureUBI.
 * Returns      : Type or 0 for failure
 ***********************************************************************/
static int getType(int partition)
{
    int rootfs = (partition == 2) ? NP_ROOTFS_2 : NP_ROOTFS_1;
    int len = flash_get_sector_size(0);
    int num_blks = flash_get_numsectors();

    if( NVRAM.ulNandPartOfsKb[rootfs] > 0 &&
        NVRAM.ulNandPartOfsKb[rootfs] < ((num_blks * len) / 1024) &&
        NVRAM.ulNandPartSizeKb[rootfs] > 0 &&
        NVRAM.ulNandPartSizeKb[rootfs] < ((num_blks * len) / 1024) )
    {
        int i, start_blk, end_blk;
        struct jffs2_raw_dirent *pdir;
        struct ubi_ec_hdr *ec = NULL;
        unsigned char buf[4];

        start_blk = NVRAM.ulNandPartOfsKb[rootfs] / (len / 1024);
        end_blk = start_blk + (NVRAM.ulNandPartSizeKb[rootfs] / (len / 1024));

        /* Find the type. */
        for( i = start_blk; i < end_blk; i++ )
        {
            if( flash_read_buf(i, 0, buf, 4) > 0 )
            {
                pdir = (struct jffs2_raw_dirent *) buf;
                ec = (struct ubi_ec_hdr *) buf;

                if(je16_to_cpu(pdir->magic) == JFFS2_MAGIC_BITMASK)
                    return(JFFS2_IMAGE);

                if (be32_to_cpu(ec->magic) == UBI_EC_HDR_MAGIC)
                    return(UBI_IMAGE);
            }
        }
    }

    return(0);
}


/***********************************************************************
 * Function Name: getSeqNum
 * Description  : Reads the sequence number of the image.
 * Returns      : Sequence number or -1 for failure
 ***********************************************************************/
int getSeqNum(int partition)
{
    PFILE_TAG pTag = getTagFromPartition(partition);

    if (!pTag)
        return(-1);

    return(atoi(pTag->imageSequence));
}


/***********************************************************************
 * Function Name: commit
 * Description  : Gets or sets the commit flag of a pureUBI image.
 * Returns      : 0 - success, -1 - failure
 ***********************************************************************/
int commit( int partition, char *string )
{
    int rootfs = (partition == 2) ? NP_ROOTFS_2 : NP_ROOTFS_1;
    int len = flash_get_sector_size(0);
    int num_blks = flash_get_numsectors();
    int write = (*string != 0);
    int ret = -1;

    if( NVRAM.ulNandPartOfsKb[rootfs] > 0 &&
        NVRAM.ulNandPartOfsKb[rootfs] < ((num_blks * len) / 1024) &&
        NVRAM.ulNandPartSizeKb[rootfs] > 0 &&
        NVRAM.ulNandPartSizeKb[rootfs] < ((num_blks * len) / 1024) )
    {
        int i, start_blk, end_blk;
        unsigned char * buf;

        if (getType(partition) != UBI_IMAGE)
            return(-1);

        if ((buf = KMALLOC(len, 1)) == 0)
        {
            printf("ERROR!!! cannot allocate block memory for buffer\n");
            return(-1);
        }

        start_blk = NVRAM.ulNandPartOfsKb[rootfs] / (len / 1024);
        end_blk = start_blk + (NVRAM.ulNandPartSizeKb[rootfs] / (len / 1024));

        for (i = 0; i < 2; i++)
        {
            if (parse_ubi(0, buf, start_blk, end_blk, len, i ? VOLID_METADATA_COPY : VOLID_METADATA, "committed", string, 0, 0, read_blk, write ? write_blk : 0, write ? erase_blk : 0, 0, 0) == 1)
            {
                ret = 0;  // if either copy of metadata is committed then we are committed
                if (!write)
                    break;
            }
            else
                printf("ERROR!!! Cound not find image %d metadata volume\n", i);
        }

        KFREE(buf);
    }

    return( ret );
}

#endif // INC_NAND_FLASH_DRIVER

#if (INC_NAND_FLASH_DRIVER==1) || (INC_SPI_PROG_NAND==1)
#if defined(_BCM94908_) || defined(_BCM96858_)|| defined(_BCM963158_) || \
    defined(_BCM96846_) || defined(_BCM96856_)
int findBootBlock()
{
     int i, start_blk, end_blk;
     struct jffs2_raw_dirent *pdir;
     struct ubi_ec_hdr *ec;
     int read_len;
     unsigned char buf[4];

     start_blk = 1;
     end_blk = ((1024*1024*1)/flash_get_sector_size(0)) + 1; // look for the FS within 1 MB + first good block 

    /* Find the directory entry. */
     for( i = start_blk; (i < end_blk) ; i++ )
     {
         if( (flash_dev_specific_cmd(CHECK_BAD_BLOCK, &i, NULL) == 0) && (read_len = flash_read_buf(i, 0, buf, 4)) > 0 )
         {
             pdir = (struct jffs2_raw_dirent *) buf;
             ec = (struct ubi_ec_hdr *) buf;

             if( (je16_to_cpu(pdir->magic) == JFFS2_MAGIC_BITMASK) || (be32_to_cpu(ec->magic) == UBI_EC_HDR_MAGIC) )
             {
                 return i;
             }
         }
         else
         {
             //skip bad block, so increase the last block to look for
             end_blk++;
         }

     }

     return( -1 );
}
#endif
#endif

int getPartitionFromTag( PFILE_TAG pTag )
{
    int ret = 0;

    if( pTag )
    {
        PFILE_TAG pTag1 = getTagFromPartition(1);
        PFILE_TAG pTag2 = getTagFromPartition(2);
        int sequence = atoi(pTag->imageSequence);
        int sequence1 = (pTag1) ? atoi(pTag1->imageSequence) : -1;
        int sequence2 = (pTag2) ? atoi(pTag2->imageSequence) : -1;

        if( pTag1 && sequence == sequence1 )
            ret = 1;
        else
            if( pTag2 && sequence == sequence2 )
                ret = 2;
    }

    return( ret );
}

PFILE_TAG getBootImageTag(void)
{
    PFILE_TAG pTag = NULL;

    if(( flash_get_flash_type() ==  FLASH_IFC_NAND ) || ( flash_get_flash_type() ==  FLASH_IFC_SPINAND ))
    { /* pTag pointer is only compared to NULL for NAND flash boot. */
        pTag = (PFILE_TAG) 1;
    }
    else if(( flash_get_flash_type() == FLASH_IFC_SPI || flash_get_flash_type() == FLASH_IFC_HS_SPI ))
    {
        PFILE_TAG pTag1 = getTagFromPartition(1);
        PFILE_TAG pTag2 = getTagFromPartition(2);

        if( pTag1 && pTag2 )
        {
            /* Two images are flashed. */
            int sequence1 = atoi(pTag1->imageSequence);
            int sequence2 = atoi(pTag2->imageSequence);

            switch( bootInfo.bootPartition )
            {
            case BOOT_SET_OLD_IMAGE:
                pTag = (sequence2 < sequence1) ? pTag2 : pTag1;
                break;

            /* case BOOT_SET_NEW_IMAGE: */
            default:
                pTag = (sequence2 > sequence1) ? pTag2 : pTag1;
                break;
            }
        }
        else
            /* One image is flashed. */
            pTag = (pTag2) ? pTag2 : pTag1;
    }
#if INC_EMMC_FLASH_DRIVER
    else if( flash_get_flash_type() == FLASH_IFC_UNSUP_EMMC)
    {
        /* pTag pointer is only compared to NULL for NAND flash boot. */
        pTag = (PFILE_TAG) 1;
    }
#endif    

    return( pTag );
}

static void UpdateImageSequenceNumber( char *imageSequence )
{
    int newImageSequence = 0;
    PFILE_TAG pTag = getTagFromPartition(1);

    if( pTag )
        newImageSequence = atoi(pTag->imageSequence);

    pTag = getTagFromPartition(2);
    if( pTag && atoi(pTag->imageSequence) > newImageSequence )
        newImageSequence = atoi(pTag->imageSequence);

    newImageSequence++;
    sprintf(imageSequence, "%d", newImageSequence);
}

// return -1: fail.
//         0: OK.
int flashImage(uint8_t *imagePtr)
{
    UINT32 crc;
    PFILE_TAG pTag = (PFILE_TAG) imagePtr;
    int totalImageSize = 0;
   
#if !INC_EMMC_FLASH_DRIVER
    if(( flash_get_flash_type() == FLASH_IFC_NAND ) ||
       ( flash_get_flash_type() == FLASH_IFC_SPINAND )) {
        printf("ERROR: Image is not a valid NAND flash image.\n");
        return -1;
    }
#endif

    if( verifyTag( pTag, 1 ) == -1 )
        return -1;

    // check imageCrc
    totalImageSize = atoi(pTag->totalImageLen);
    crc = CRC32_INIT_VALUE;
    crc = getCrc32((imagePtr+TAG_LEN), (UINT32) totalImageSize, crc);      

    if (crc != (UINT32) (*(UINT32*)(pTag->imageValidationToken))) {
        printf(" Illegal image ! Image crc failed.\n");
        return -1;
    }

    if( flash_get_flash_type() == FLASH_IFC_SPI || flash_get_flash_type() == FLASH_IFC_HS_SPI )
        return flashImageSpiNor( pTag, imagePtr );
#if INC_EMMC_FLASH_DRIVER        
    else if(   (flash_get_flash_type() == FLASH_IFC_UNSUP_EMMC)
            || (flash_get_flash_type() == FLASH_IFC_NAND) 
            || (flash_get_flash_type() == FLASH_IFC_SPINAND))
    {
        /* Allow us to flash eMMC images even if strapped for NAND */
        return emmc_flash_image( pTag, imagePtr );
    }
#endif
    else
        return -1;			
}

static int flashImageSpiNor( PFILE_TAG pTag, uint8_t *imagePtr ) 
{
    FLASH_ADDR_INFO info;
    int cfeSize;
    int cfeAddr, rootfsAddr, kernelAddr, dtbAddr;
    int status = -1;
    int checkDtb = 0;
    NVRAM_DATA *nvramData;
    
    nvramData = KMALLOC(sizeof(NVRAM_DATA),sizeof(void*));
    if (!nvramData) {
        goto err_out;
    }

    kerSysFlashAddrInfoGet( &info );
      
    cfeSize = cfeAddr = rootfsAddr = kernelAddr = dtbAddr = 0;
    checkDtb = atoi(pTag->tagVersion) >= BCM_TAG_VER_DTB ? 1 : 0;

    // check cfe's existence
    cfeAddr = atoi(pTag->cfeAddress);
    if( checkDtb )
    {
        dtbAddr = atoi(pTag->dtbAddress);
        if( dtbAddr == 0 )
            checkDtb = 0;
    }

    if (cfeAddr)
    {
        cfeSize = atoi(pTag->cfeLen);
        if( (cfeSize <= 0) ) {
            printf("Illegal cfe size [%d].\n", cfeSize );
            goto err_out;
        }
        
        printf("\nFlashing CFE at flash 0x%x:", cfeAddr+BOOT_OFFSET+IMAGE_OFFSET);
        if ((status = kerSysBcmImageSet(cfeAddr+BOOT_OFFSET+IMAGE_OFFSET, imagePtr+TAG_LEN,
            cfeSize, 0)) != 0) {
            printf("Failed to flash CFE. Error: %d\n", status);
            goto err_out;
        }

        // Check if the new image has valid NVRAM
	if (NVRAM_READ_VERIFY(nvramData)) {
	    NVRAM_UPDATE(NULL);
	} else {
	    NVRAM_SYNC();
	}
    }

    // check root filesystem and kernel existence
    rootfsAddr = atoi(pTag->rootfsAddress);
    kernelAddr = atoi(pTag->kernelAddress);

    if( rootfsAddr && kernelAddr )
    {
        unsigned char *tagFs = imagePtr;
        unsigned int baseAddr = (uintptr_t) flash_get_memptr(0);
        unsigned int totalSize = (unsigned int) flash_get_total_size();
        unsigned int sectSize = (unsigned int) flash_get_sector_size(0);
        unsigned int reservedBytesAtEnd;
        unsigned int reserverdBytersAuxfs;
        unsigned int availableSizeOneImg;
        unsigned int reserveForTwoImages;
        unsigned int availableSizeTwoImgs;
        unsigned int newImgSize = atoi(pTag->rootfsLen)+atoi(pTag->kernelLen);
        PFILE_TAG pCurTag = getBootImageTag();
        UINT32 crc;
        unsigned int curImgSize = 0;
        unsigned int rootfsOffset = (unsigned int)rootfsAddr-IMAGE_BASE-TAG_LEN+IMAGE_OFFSET;
        FLASH_ADDR_INFO flash_info;

        /* update total image size if there is dtb at the end */
        if( checkDtb )
            newImgSize += atoi(pTag->dtbLen);

        kerSysFlashAddrInfoGet(&flash_info);
        if( rootfsOffset < flash_info.flash_rootfs_start_offset )
        {
            // Increase rootfs and kernel addresses by the difference between
            // rootfs offset and what it needs to be.
            rootfsAddr += flash_info.flash_rootfs_start_offset - rootfsOffset;
            kernelAddr += flash_info.flash_rootfs_start_offset - rootfsOffset;
            sprintf(pTag->rootfsAddress,"%u", (unsigned int) rootfsAddr);
            sprintf(pTag->kernelAddress,"%u", (unsigned int) kernelAddr);
            if( checkDtb )
            {
                dtbAddr += flash_info.flash_rootfs_start_offset - rootfsOffset;
                sprintf(pTag->dtbAddress,"%u", (unsigned int) dtbAddr);
            }
            crc = CRC32_INIT_VALUE;
            crc = getCrc32((unsigned char *)pTag,(UINT32)TAG_LEN-TOKEN_LEN,crc);
            *(unsigned int *) &pTag->tagValidationToken[0] = crc;
        }

        rootfsAddr += BOOT_OFFSET+IMAGE_OFFSET;
        kernelAddr += BOOT_OFFSET+IMAGE_OFFSET;
        if( checkDtb )
            dtbAddr += BOOT_OFFSET+IMAGE_OFFSET;
        reservedBytesAtEnd = flash_get_reserved_bytes_at_end(&flash_info);
        reserverdBytersAuxfs = flash_get_reserved_bytes_auxfs();
        availableSizeOneImg = totalSize - ((unsigned int)rootfsAddr-baseAddr) -
            reservedBytesAtEnd - reserverdBytersAuxfs;
        reserveForTwoImages =
            (flash_info.flash_rootfs_start_offset > reservedBytesAtEnd)
            ? flash_info.flash_rootfs_start_offset : reservedBytesAtEnd;

       // availableSizeTwoImgs = (totalSize / 2) - reserveForTwoImages;
       availableSizeTwoImgs = ((totalSize-reserverdBytersAuxfs)/ 2) - reserveForTwoImages - sectSize;

       printf("availableSizeOneImage=%dKB availableSizeTwoImgs=%dKB reserve=%dKB\n",
       availableSizeOneImg/1024, availableSizeTwoImgs/1024, reserveForTwoImages/1024);

        if( pCurTag )
        {
            curImgSize = atoi(pCurTag->rootfsLen) + atoi(pCurTag->kernelLen);
            if( atoi(pCurTag->tagVersion) >= BCM_TAG_VER_DTB )
                curImgSize += atoi(pCurTag->dtbLen);
        }

        if( newImgSize > availableSizeOneImg) {
            printf("Illegal image size %d.  Image size must not be greater "
                "than %d.\n", newImgSize, availableSizeOneImg);
            goto err_out;
        }

        // tag is always at the sector start of fs
        if (cfeAddr)
        {
            // will trash cfe memory, but cfe is already flashed
            tagFs = imagePtr + cfeSize;
            memcpy(tagFs, imagePtr, TAG_LEN);
        }

        // If the current image fits in half the flash space and the new
        // image to flash also fits in half the flash space, then flash it
        // in the partition that is not currently being used to boot from.
        if( curImgSize <= availableSizeTwoImgs &&
            newImgSize <= availableSizeTwoImgs &&
            getPartitionFromTag( pCurTag ) == 1 )
        {
            // Update rootfsAddr to point to the second boot partition.
           // int offset = (totalSize / 2) + TAG_LEN;
            int offset = ((totalSize-reserverdBytersAuxfs) / 2);

            /* make sure offset is on sector boundary, image starts on sector boundary */
            if( offset % sectSize )
                offset = ((offset/sectSize)+1)*sectSize;
            offset += TAG_LEN;

            if( checkDtb )
            {
                sprintf(((PFILE_TAG) tagFs)->dtbAddress, "%u",
                    (unsigned int) IMAGE_BASE + offset + (dtbAddr-rootfsAddr));
                dtbAddr = baseAddr + offset + (dtbAddr - rootfsAddr) + IMAGE_OFFSET;
            }   

            sprintf(((PFILE_TAG) tagFs)->kernelAddress, "%u",
                (unsigned int) IMAGE_BASE + offset + (kernelAddr-rootfsAddr));
            kernelAddr = baseAddr + offset + (kernelAddr - rootfsAddr) + IMAGE_OFFSET;

            sprintf(((PFILE_TAG) tagFs)->rootfsAddress, "%u",
                (unsigned int) IMAGE_BASE + offset);
            rootfsAddr = baseAddr + offset + IMAGE_OFFSET;
        }

        UpdateImageSequenceNumber( ((PFILE_TAG) tagFs)->imageSequence );
        crc = CRC32_INIT_VALUE;
        crc = getCrc32((unsigned char *)tagFs, (UINT32)TAG_LEN-TOKEN_LEN, crc);      

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        ((PFILE_TAG) tagFs)->tagValidationToken[0] = crc & 0xFF;
        ((PFILE_TAG) tagFs)->tagValidationToken[1] = (crc >> 8) & 0xFF;
        ((PFILE_TAG) tagFs)->tagValidationToken[2] = (crc >> 16) & 0xFF;
        ((PFILE_TAG) tagFs)->tagValidationToken[3] = (crc >> 24) & 0xFF;
#else
        ((PFILE_TAG) tagFs)->tagValidationToken[0] = (crc >> 24) & 0xFF;
        ((PFILE_TAG) tagFs)->tagValidationToken[1] = (crc >> 16) & 0xFF;
        ((PFILE_TAG) tagFs)->tagValidationToken[2] = (crc >> 8) & 0xFF;
        ((PFILE_TAG) tagFs)->tagValidationToken[3] = crc & 0xFF;
#endif

        printf("\nFlashing root file system and kernel at 0x%8.8lx: ",
            rootfsAddr - TAG_LEN);
        if( (status = kerSysBcmImageSet((rootfsAddr-TAG_LEN), tagFs,
            TAG_LEN + newImgSize, 0)) != 0 ) {
            printf("Failed to flash root file system. Error: %d\n", status);
            goto err_out;
        }
    }
    
    status = 0;
    printf(".\n*** Image flash done *** !\n");

err_out:
    if (nvramData)
      KFREE(nvramData);
    return status;
}

#if (INC_NAND_FLASH_DRIVER == 1 || INC_SPI_PROG_NAND == 1 || INC_SPI_NAND_DRIVER == 1)
static unsigned int readBlk(unsigned char * start, unsigned int block, unsigned int offset, unsigned int blk_size,  unsigned char * buf, unsigned int amount, void * mtd, int mtd_fd)
{
    memcpy(buf + offset, start + (block * blk_size) + offset, amount);
    return(amount);
}

static unsigned int writeBlk(unsigned char * start, unsigned int block, unsigned int offset, unsigned int blk_size, unsigned char * buf, unsigned int amount, void * mtd, int mtd_fd)
{
    memcpy(start + (block * blk_size) + offset, buf + offset, amount);
    return(amount);
}

static int nandUpdateImageSequenceNumber( uint8_t *imagePtr, int imageSize, int bootPart )
{
    unsigned char *buf, *p;
    char fname[] = NAND_CFE_RAM_NAME;
    char ubi_seqno_id[] = NAND_CFE_RAM_NAME;
    int fname_actual_len = strlen(fname);
    int fname_cmp_len = strlen(fname) - 3; /* last three are digits */
    int len = flash_get_sector_size(0);
    struct jffs2_raw_dirent *pdir;
    unsigned int version = 0;
    PFILE_TAG pTag = getTagFromPartition(bootPart);
    int seq = (pTag) ? atoi(pTag->imageSequence) : -1;
    int ret = 0;
    struct ubi_ec_hdr *ec;
    unsigned int type = 0; // image type

#if !defined(_BCM960333_) && !defined(_BCM96848_) && !defined(_BCM947189_)
    /* If full secure boot is in play, the CFE RAM file is the encrypted version */
    if (bcm_otp_is_boot_secure())
       strcpy(fname, NAND_CFE_RAM_SECBT_NAME);
#if defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || \
    defined(_BCM96846_) || defined(_BCM96856_)
    /* For these targets, the secure boot could be mfg mode or field mode. */
    else
    {
       if (bcm_otp_is_boot_mfg_secure())
          strcpy(fname, NAND_CFE_RAM_SECBT_MFG_NAME);
    }
#endif
#endif

    if( seq != -1 )
    {
        int done = 0;

        /* Increment the new highest sequence number. Add it to the CFE RAM
         * file name.
         */
        seq++;
        if (seq > 999)
            seq = 0;

        for(p = imagePtr; (p < imagePtr+imageSize) && (done == 0); p += len)
        {
            ec = (struct ubi_ec_hdr *) p;

            if ( (type == UBI_IMAGE) || (!type && (be32_to_cpu(ec->magic) == UBI_EC_HDR_MAGIC) && (getCrc32((void *)ec, UBI_EC_HDR_SIZE-4, -1) == be32_to_cpu(ec->hdr_crc))) )
            { // will only need to search through the blocks once since the image is built with volumes in order
                unsigned char * buffer;
                int i, try;
                char seqstr[3] = {(seq / 100) + '0', ((seq % 100) / 10) + '0', ((seq % 100) % 10) + '0'};

                if (!(buffer = (unsigned char *) KMALLOC(len, 1)))
                   return(ret);

                if (!type)
                    type = UBI_IMAGE;

                printf("Setting pureUBI image sequence number to %d and committing image\n", seq);

                for (i = 0; i < 2; i++)
                {
                    if (i)
                       try = VOLID_METADATA_COPY;
                    else
                       try = VOLID_METADATA;

                    if (parse_ubi(imagePtr, buffer, (p - imagePtr) / len, imageSize / len, len, try, ubi_seqno_id, seqstr, 0, 0, readBlk, writeBlk, 0, 0, 0) == 3)
                    {
                        if (parse_ubi(imagePtr, buffer, (p - imagePtr) / len, imageSize / len, len, try, "committed", "1", 0, 0, readBlk, writeBlk, 0, 0, 0) != 1)
                            printf("ERROR!!! Metadata image %d corrupt at committed entry!!!\n", i);
                    }
                    else
                    {
                        printf("ERROR!!! Metadata image %d corrupt at sequence number entry!!!\n", i);
                    }
                }

                KFREE(buffer);

                done = 1;
            }
            else
            {
                buf = p;
                while( buf < (p + len) )
                {
                    pdir = (struct jffs2_raw_dirent *) buf;
                    if( je16_to_cpu(pdir->magic) == JFFS2_MAGIC_BITMASK &&
                        je32_to_cpu(pdir->hdr_crc) == getCrc32(buf,
                            sizeof(struct jffs2_unknown_node) - 4, 0) )
                    {
                        if( je16_to_cpu(pdir->nodetype) == JFFS2_NODETYPE_DIRENT &&
                            fname_actual_len == pdir->nsize &&
                            !memcmp(fname, pdir->name, fname_cmp_len) &&
                            je32_to_cpu(pdir->version) > version &&
                            je32_to_cpu(pdir->ino) != 0 )
                        {
                            printf("Setting JFFS2 sequence number to %d\n", seq);
                            buf = pdir->name + fname_cmp_len;
                            buf[0] = (seq / 100) + '0';
                            buf[1] = ((seq % 100) / 10) + '0';
                            buf[2] = ((seq % 100) % 10) + '0';

                            je32_to_cpu(pdir->name_crc) = getCrc32(pdir->name,
                                (unsigned int) fname_actual_len, 0);

                            version = je32_to_cpu(pdir->version);

                            /* There is only one version of the directory entry. */
                            done = 1;
                            ret = (p - imagePtr) / len; /* block number */
                            break;
                        }

                        buf += (je32_to_cpu(pdir->totlen) + 0x03) & ~0x03;
                    }
                    else
                        break;
                }
            }
        }
    }

    return(ret);
}


char nandCommand(char command, uint16_t value)
{ // issue generic command to the NAND/controller
    while( (NAND->NandIntfcStatus & (NIS_CTLR_READY | NIS_FLASH_READY)) != (NIS_CTLR_READY | NIS_FLASH_READY) ) ; // wait for NAND

    switch(command)
    {
        case 'c': // command
            NAND->NandLlOpNand = 0xA0000 | value; // CLE and WE
            break;

        case 'a': // address
            NAND->NandLlOpNand = 0x60000 | value; // ALE and WE
            break;

        case 'w': // write
            NAND->NandLlOpNand = 0x20000 | value; // WE
            break;

        case 'i': // idle
            NAND->NandLlOpNand = 0x80000000 | value; // WE
            break;

        default: // read
            NAND->NandLlOpNand = 0x10000 | value; // RE
            break;
    }

    NAND->NandCmdStart = NCMD_LOW_LEVEL_OP;
    while( (NAND->NandIntfcStatus & (NIS_CTLR_READY | NIS_FLASH_READY)) != (NIS_CTLR_READY | NIS_FLASH_READY) ) ; // wait for NAND

    return(NAND->NandLlRdData);
}


int writeWholeImageRaw(uint8_t *imagePtr, int wholeImageSize, uint32_t offset)
{
    int status = -1;
    if((status =
        kerSysBcmImageSet(FLASH_BASE + offset, imagePtr, wholeImageSize, 1)) != 0) {
        printf("Failed to flash whole image. Error: %d\n", status);
    }
    return status;
}

// return -1: fail.
//         0: OK.
int writeWholeImageDataPartition(uint8_t *imagePtr, int wholeImageSize, char *partition_name)
{
    int status = -1, pnum, parts_to_iterate;
    unsigned long offset = 0;
    int imageSize = wholeImageSize - TOKEN_LEN;
    unsigned long total_dps, data_partition_size, ulBlockSizeKb;
    if(partition_name == NULL)
        goto err_out;

    pnum = partition_name[0]-'1';

    if(pnum < 0 || pnum > 3)
        goto err_out;

    if(( flash_get_flash_type() == FLASH_IFC_NAND ) ||
       ( flash_get_flash_type() == FLASH_IFC_SPINAND )) {
        data_partition_size = 0;
        if(NVRAM.part_info[pnum].size != 0xffff) {
            data_partition_size = convert_to_data_partition_entry_to_bytes(NVRAM.part_info[pnum].size);

        } else {
            printf("invalid partition size %d\n", NVRAM.part_info[pnum].size);
            goto err_out;
        }
        if(wholeImageSize > data_partition_size) {
            printf("Image size too big, invalid partition size\n");
            goto err_out;
        }

        parts_to_iterate = 3-pnum;
        pnum = BCM_MAX_EXTRA_PARTITIONS-2;//always start with 2,  iterate to 0 as per parts_to_iterate

        //printf("writeWholeImageDataPartition %d flash_get_sector_size %d flash_get_total_size %d\n", wholeImageSize, flash_get_sector_size(0), flash_get_total_size());
        //printf("partition_name %s\n", partition_name);


        //printf("Data partition offset %d[%x]\n", (NVRAM.ulNandPartOfsKb[NP_DATA] * 1024), (NVRAM.ulNandPartOfsKb[NP_DATA] * 1024));
        ulBlockSizeKb = flash_get_sector_size(0);
        total_dps = 0;

        while(parts_to_iterate > 0) {  

            data_partition_size=convert_to_data_partition_entry_to_bytes(NVRAM.part_info[pnum].size);
            if ((data_partition_size & ~(ulBlockSizeKb-1)) != data_partition_size)
                data_partition_size=data_partition_size+ulBlockSizeKb;

            data_partition_size = data_partition_size&~(ulBlockSizeKb-1);
            //printf("\n******************\n");
            //printf("pnum %d parts_to_iterate %d\n", pnum, parts_to_iterate);
            //printf("data_partition_size=%d[%x]\n", data_partition_size);
            //printf("******************\n");

            total_dps += data_partition_size; 
            pnum--;	
            parts_to_iterate--;	

        }
        offset = (NVRAM.ulNandPartOfsKb[NP_DATA] * 1024)-total_dps;
        //printf("offset %d[%x] total_dps %d[%x] \n", offset, offset, total_dps, total_dps);

        if((status =
	    kerSysBcmImageSet(FLASH_BASE + offset, imagePtr, imageSize, 1)) != 0) {
            printf("Failed to flash whole image. Error: %d\n", status);
            goto err_out;
        }
    }
    status = 0;
err_out:
    return status;
}

static int findBootBlockInImage(unsigned char *pImage, unsigned int imageSize)
{
    int i = 0;   
    int  blk_size = flash_get_sector_size(0);
    unsigned char* pImageEnd = pImage + imageSize;

    while( (*(unsigned short *)pImage != JFFS2_MAGIC_BITMASK)
        && (be32_to_cpu(*(unsigned int *)pImage) != UBI_EC_HDR_MAGIC) )
    {
        i++;
        pImage += blk_size;
        if( pImage >= pImageEnd )
        {
            i = -1;
            break;
        }
    } 

    return i;
}

static unsigned int available_space_in_partition(const NVRAM_DATA *nvram, int part_num)
{
    int j = 0, blk, good_blocks = 0, blk_size;
    int offset;
    if (nvram->ulNandPartOfsKb[part_num] == -1) {
       return 0;
    }
      
    blk_size = flash_get_sector_size(0);
    offset = nvram->ulNandPartOfsKb[part_num]*1024;
    j = offset/blk_size;
    blk = (offset+(nvram->ulNandPartSizeKb[part_num]*1024))/blk_size;
    good_blocks = blk-j;
    while(j < blk) {
          if (flash_dev_specific_cmd(CHECK_BAD_BLOCK, &j, NULL)) {
              good_blocks--;
          }
          j++;
    }
    return (good_blocks*blk_size);
}

/* check if image size fit the target rootfs.
   return 0 if fit and non zero if it does not 
*/
static int checkImageSizeForNand(int rootfs, unsigned char* imagePtr, unsigned int imageSize)
{
    int rc = 0;
    int sectsize = flash_get_sector_size(0);
    unsigned int avail_part_size = available_space_in_partition(NVRAM_RP, rootfs);
    int boot_img_blk = findBootBlockInImage(imagePtr, imageSize);
    int boot_img_size = boot_img_blk*sectsize;
    int boot_par_size = NVRAM.ulNandPartSizeKb[NP_BOOT];
    unsigned int required_fs_size = imageSize - boot_img_size + overhead_blocks*sectsize;
    int adjust_blk = 0;

    if( boot_img_blk < 0 )
        return -1;

    if (( rootfs == NP_ROOTFS_1 || rootfs == NP_ROOTFS_2)
        && (NVRAM.ulNandPartSizeKb[NP_ROOTFS_2] != -1) ) {
        if( boot_par_size != -1 && boot_img_size > (boot_par_size*1024) ) {
            /* boot partition increase such as unsecure to secure boot, fs partition will be reduced later */
            adjust_blk = boot_img_blk - (boot_par_size*1024/sectsize);
            if( adjust_blk%2 == 0 )
                adjust_blk = adjust_blk/2; 
            else
                adjust_blk = (adjust_blk+1)/2; 
        }

        if ( (avail_part_size-adjust_blk*sectsize) < required_fs_size ) {
            printf("\nFS Image(%d) plus reserve(%d) bigger than the partition(%d)\n", imageSize-boot_img_size, overhead_blocks*sectsize, avail_part_size-adjust_blk*sectsize);
            rc = -1;
        }
    }

    return rc;
}
#endif

#if (INC_NAND_FLASH_DRIVER == 1 || INC_SPI_PROG_NAND == 1 || INC_SPI_NAND_DRIVER == 1) && (INC_NVRAM_MIRRORING == 1)
/*
        search for nVrAmDaT, if exists, replace all NVRAM_DATA entries with inMemNvramData_buf
*/

#include "bcm_ubi.h"


static void nvram_data_mirror_search(uint8_t *hunt_ptr, int size_to_search, const NVRAM_DATA* const pnd)
{
    UINT32 crc;
    PNVRAM_DATA nv;

    if(hunt_ptr != NULL)
    {
        while(size_to_search > 2048)
        {
            if(check_jffs_ubi_magic(hunt_ptr))
                break;
            if(strncmp((const char *)hunt_ptr, NVRAM_DATA_SIGN, strlen(NVRAM_DATA_SIGN)) == 0)
            {
                nv=(PNVRAM_DATA) (hunt_ptr+strlen(NVRAM_DATA_SIGN));         
                crc=nv->ulCheckSum;
                nv->ulCheckSum = 0;
                nv->ulCheckSum = getCrc32((unsigned char *) nv, sizeof(NVRAM_DATA), CRC32_INIT_VALUE);
                if(crc == nv->ulCheckSum) {
                    printf("Found default NVRAM at %x\n", hunt_ptr);
                    memcpy(hunt_ptr+strlen(NVRAM_DATA_SIGN), (unsigned char *)pnd, sizeof(NVRAM_DATA));
                }
                else
                    nv->ulCheckSum = crc;
            }
            hunt_ptr+=1024;
            size_to_search-=1024;
        }
    }
}
#endif

// return -1: fail.
//         0: OK.
int writeWholeImage(uint8_t *imagePtr, int wholeImageSize)
{
    UINT32 crc;
    int status = -1;
    unsigned long offset = 0;
    int imageSize = wholeImageSize - TOKEN_LEN;
    unsigned char crcBuf[CRC_LEN];
    NVRAM_DATA *nvramData;
    WFI_TAG wfiTag;
#if (INC_NAND_FLASH_DRIVER == 1 || INC_SPI_PROG_NAND == 1 || INC_SPI_NAND_DRIVER == 1)
#if defined(_BCM963268_)
    uint8_t *imagePtrBegin = imagePtr;
#endif
#endif

    nvramData = KMALLOC(sizeof(NVRAM_DATA),sizeof(void*));
    if (!nvramData) {
        goto err_out;
    }

    memcpy(&wfiTag, imagePtr + imageSize, sizeof(wfiTag));
    
    if( (wfiTag.wfiVersion & WFI_ANY_VERS_MASK) == WFI_ANY_VERS &&
                          checkChipId(wfiTag.wfiChipId, NULL) != 0 ) {
         goto err_out;
    }
    
#if (INC_SPI_PROG_NAND==1)
    if( (flash_get_flash_type() != FLASH_IFC_NAND)  &&  (wfiTag.wfiFlashType >= WFI_NANDTYPE_FLASH_MIN &&
	wfiTag.wfiFlashType <= WFI_NANDTYPE_FLASH_MAX))
        flash_change_flash_type(FLASH_IFC_NAND);
#endif

    // if whole image size (plus TOKEN_LEN of crc) is greater than total flash size, return error
    if ((unsigned long) wholeImageSize > (flash_get_total_size() + TOKEN_LEN)) {
        printf("Image is too big\n");
        goto err_out;
    }

    // check tag validate token first
    crc = getCrc32(imagePtr, (UINT32)imageSize, CRC32_INIT_VALUE);
    memcpy(crcBuf, imagePtr + imageSize, CRC_LEN);
    if (memcmp(&crc, crcBuf, CRC_LEN) != 0) {
        printf("Illegal whole flash image\n");
        goto err_out;
    }
    // save existing NVRAM data into a local structure

#if defined(_BCM963138_) || defined(_BCM963148_) || defined(_BCM96858_) || defined(_BCM94908_)
#if defined (_BCM94908_)
// if 4908 is booting from NAND zero out the first 64K from the image
// which has the pre-Cferom binary
    if(flash_get_flash_type() == FLASH_IFC_NAND) {
        if( (*(unsigned short *)imagePtr != JFFS2_MAGIC_BITMASK)
               && (be32_to_cpu(*(unsigned int *)imagePtr) != UBI_EC_HDR_MAGIC)
            ) {
            unsigned char *img  = (unsigned char*)imagePtr;
            int  sect_size, rem;
            sect_size = flash_get_sector_size(0);
            rem = IMAGE_OFFSET%sect_size;
            memset((unsigned char*)img, '\0', rem);
        }
    }
// if 4908 is booting from SPI nand write the first block
// which has the pre-Cferom binary
    else if(flash_get_flash_type() != FLASH_IFC_SPINAND) {
#endif

    /* check if image has 64KB zero padding in beginning if no PMC */
    if ( (wfiTag.wfiFlags&WFI_FLAG_HAS_PMC) == 0 ) {
        unsigned int *img  = (unsigned int*)imagePtr;
        unsigned char * buf = NULL;
        int  sect_num, sect_size, rem, sect;

	if( *img == 0 && *(img+1) == 0 &&
	    *(img+2) == 0 && *(img+3) == 0 ) {
	   /* the first 64KB are for PMC in 631x8, 
              need to preserve that for cfe/linux image update
              if it is not for PMC image update. */
           sect_size = flash_get_sector_size(0);
           sect_num = IMAGE_OFFSET/sect_size;
           rem = IMAGE_OFFSET%sect_size;
	   buf = (unsigned char *) KMALLOC(sect_size, sizeof(long));
	   if (!buf) {
	       goto err_out;
	   }
	   for (sect = 0; sect < sect_num; sect++ ) {
                flash_read_buf(sect, 0, buf, sect_size);
                memcpy((unsigned char*)img, buf, sect_size);
                img = (unsigned int*)((unsigned char*)img + sect_size);
           }
           if (rem) {
                flash_read_buf(sect_num, 0, buf, rem);
                memcpy((unsigned char*)img, buf, rem);
           }
           KFREE(buf);
	     
	} else {
            /* does not have PMC at all, image data starting from 64KB offset */
	   if( (flash_get_flash_type() == FLASH_IFC_SPI) || (flash_get_flash_type() == FLASH_IFC_HS_SPI)  )
  	        offset = IMAGE_OFFSET;
	}
    }
#if defined (_BCM94908_) // if not spi_nand
  }
#endif
#endif

#if (INC_NAND_FLASH_DRIVER == 1 || INC_SPI_PROG_NAND == 1 || INC_SPI_NAND_DRIVER == 1)
    if(( flash_get_flash_type() == FLASH_IFC_NAND ) ||
       ( flash_get_flash_type() == FLASH_IFC_SPINAND )) {
        /* The CFE ROM boot loader saved the rootfs partition index at the
         * memory location before CFE RAM load address.
         */
        extern unsigned char _ftext;
        /* 63138 and 63148 use 16KB below the cfe ram image as the mmu table,
         * so rfs number, cfe number has to be saved 16K down further */
        /* Allow addition blocks to flash cfram block that has sequence number
         * and is flashed last.
         */

        int rootfs = (int) CFE_RAM_ROM_PARMS_GET_ROOTFS;
        //if board id is null, very possible that the board was erased
        //force wrting the image to the first partition
        if(((unsigned char )NVRAM.szBoardId[0]) == 0xff &&
	   ((unsigned char )NVRAM.szBoardId[1]) == 0xff) {
            rootfs = NP_ROOTFS_2;
            memcpy(nvramData, (NVRAM_DATA*) (imagePtr + NVRAM_DATA_OFFSET), sizeof(NVRAM_DATA));
#if defined(_BCM963138_) || defined(_BCM963148_)
            strncpy(nvramData->szBoardId, "PROMPT", NVRAM_BOARD_ID_STRING_LEN);
            nvramData->ulCheckSum = 0;
            nvramData->ulCheckSum = getCrc32((unsigned char *) nvramData, sizeof(NVRAM_DATA), CRC32_INIT_VALUE);
#endif
	    NVRAM_UPDATE(nvramData);
        }
        int blksize = flash_get_sector_size(0) / 1024;
        int sectsize = flash_get_sector_size(0);
        unsigned int i, cferam_blk, after_cferam, cferam_overhead;

        if( checkImageSizeForNand(rootfs == NP_ROOTFS_1 ? NP_ROOTFS_2 : NP_ROOTFS_1, imagePtr, imageSize) != 0 )
            goto err_out;

        if( (wfiTag.wfiVersion & WFI_ANY_VERS_MASK) == WFI_ANY_VERS &&
            wfiTag.wfiFlashType < WFI_NANDTYPE_FLASH_MIN &&
	    wfiTag.wfiFlashType > WFI_NANDTYPE_FLASH_MAX) {
            printf("\nERROR: Image does not support a NAND flash device.\n\n");
            goto err_out;
        }

        if( (wfiTag.wfiVersion & WFI_ANY_VERS_MASK) == WFI_ANY_VERS &&
	    blksize != WFI_NANDTYPE_TO_BKSIZE(wfiTag.wfiFlashType) ) {
            printf("\nERROR: NAND flash block size %dKB does not work with an "
		   "image built with %dKB block size\n\n", blksize, WFI_NANDTYPE_TO_BKSIZE(wfiTag.wfiFlashType));
            goto err_out;
        }


	uint32_t curBtBlkCnt = getNumBootBlks(NVRAM_RP);    /* number of boot blocks currently on flash */
	uint32_t newBtBlkCnt = 0;                           /* number of boot blocks in new image */

        if( (*(unsigned short *)imagePtr != JFFS2_MAGIC_BITMASK)
               && (be32_to_cpu(*(unsigned int *)imagePtr) != UBI_EC_HDR_MAGIC) 
            )
        {
	    newBtBlkCnt++;  /* New image has at least one boot block (headerless cferom in block 0) */

	    uint32_t nvramXferSize = sizeof(NVRAM_DATA);
#if defined(_BCM963268_)
	    // If upgrading an already-secure-boot 63268 target, do not preserve the old security credentials kept in nvram
	    // but rather use the ones already embedded within the new image (ie the last 2k of the 3k nvram)
            if( (wfiTag.wfiFlags & WFI_FLAG_SUPPORTS_BTRM) && bcm_otp_is_boot_secure() )
               nvramXferSize = 1024;
#endif
            /* Copy NVRAM data to image nvram area so it is preserved. */
            PNVRAM_DATA pnd = (PNVRAM_DATA) (imagePtr + NVRAM_DATA_OFFSET);
            memcpy((unsigned char *) pnd, (unsigned char *) NVRAM_RP, nvramXferSize);

#if (INC_NAND_FLASH_DRIVER == 1 || INC_SPI_PROG_NAND == 1 || INC_SPI_NAND_DRIVER == 1) && (INC_NVRAM_MIRRORING == 1)
            nvram_data_mirror_search(imagePtr, imageSize, NVRAM_RP);
#endif

            /* Recalculate the nvramData CRC. */
            pnd->ulCheckSum = 0;
            pnd->ulCheckSum = getCrc32((unsigned char *) pnd, sizeof(NVRAM_DATA), CRC32_INIT_VALUE);

#if defined(_BCM963268_)
            if (wfiTag.wfiFlags & WFI_FLAG_SUPPORTS_BTRM) {
               /* New image expects secure boot to be enabled. Two possibilities: migrating from unsecure to secure, */
               /* or already in secure boot mode and just upgrading to a new secure boot image */
               if ( wfiTag.wfiFlashType == WFI_NAND16_FLASH ) {
                  /* Small page nand. If migrating from unsecure to secure boot, boot blocks increases from 1 to > 1 */
	          /* Partition table and the new image's 3k nvram checksum will be updated later */
	          while ( (*(unsigned short *)(imagePtr + (sectsize*newBtBlkCnt)) != JFFS2_MAGIC_BITMASK)
                      && (be32_to_cpu(*(unsigned int *)(imagePtr + (sectsize*newBtBlkCnt))) != UBI_EC_HDR_MAGIC)
                      )  {
	             newBtBlkCnt++;
		     if ((newBtBlkCnt*sectsize) > imageSize) {
                        printf("Error: never found a JFFS2 marker\n");
                        goto err_out;
                     }
                  }
               } else {
                  if (!otp_is_boot_secure()) {
                     /* Large page nand migrating from unsecure to secure boot. Boot block count stays at 1 so the */
                     /* partition table will not change and the new image's 3k nvram ulCheckSum can be updated right now */
                     unsigned int *pChkSumIn3k = (unsigned int *)&pnd->ulCheckSum; 
		     pChkSumIn3k += (0x800/sizeof(unsigned int));
                     *pChkSumIn3k = 0;
                     *pChkSumIn3k = getCrc32((unsigned char *) pnd, (sizeof(NVRAM_DATA) * 3), CRC32_INIT_VALUE);
		  }
               }
            }
#endif


            /* Write all of the headerless cfe rom boot block(s) in */
#if defined(_BCM963138_) || defined(_BCM963148_) || defined (_BCM94908_) || defined (_BCM96858_)
            if( (wfiTag.wfiFlags&WFI_FLAG_HAS_PMC) )
                printf("Flash PMC.\n");
            StallPmc();
#endif
            if((status = kerSysBcmImageSet(FLASH_BASE, imagePtr, sectsize * newBtBlkCnt, 0)) != 0)
                printf("Failed to flash boot block(s). Error: %d\n", status);
#if defined(_BCM963138_) || defined(_BCM963148_) || defined (_BCM94908_) || defined (_BCM96858_)
            UnstallPmc();
#endif
            imagePtr += sectsize * newBtBlkCnt;
            imageSize -= sectsize * newBtBlkCnt;


#if defined(_BCM96838_) || defined(_BCM963381_) || defined(_BCM963138_) || defined (_BCM963148_) || defined (_BCM96848_)
	    /* These targets support version 2 bootrom, and therefore this whole image can  */
	    /* contain headered CFE bootloaders right after the legacy, headerless CFE ROM. */
	    uint32_t *pHdr = (uint32_t *)imagePtr;
            while ((*pHdr       == (uint32_t)BTRM_SBI_UNAUTH_MGC_NUM_1) &&
                   (*(pHdr + 1) == (uint32_t)BTRM_SBI_UNAUTH_MGC_NUM_2)) {
	          status = kerSysBcmImageSet(FLASH_BASE + (sectsize*newBtBlkCnt), imagePtr, sectsize, 0);
		  if(status) {
		     printf("Failed to flash headered cfe number %d into flash. Error: %d\n", newBtBlkCnt, status);
		  }
		  imagePtr += sectsize;
		  imageSize -= sectsize;
		  newBtBlkCnt++;
		  pHdr = (uint32_t *)imagePtr;
            }

	    /* If one or more headered CFE ROMS were written to flash, expand the boot partition out to a max of MAX_NUM_BT_BLKS */
	    if (newBtBlkCnt > 1) {
                while (( newBtBlkCnt < MAX_NUM_BT_BLKS ) && (*(unsigned short *)imagePtr != JFFS2_MAGIC_BITMASK)
                        && (be32_to_cpu(*(unsigned int *)imagePtr) != UBI_EC_HDR_MAGIC)
		      ) {
		         status = kerSysBcmImageSet(FLASH_BASE + (sectsize*newBtBlkCnt), imagePtr, sectsize, 0);
		         if(status) {
		            printf("Failed to flash boot padding in block number %d . Error: %d\n", newBtBlkCnt, status);
		         }
			 imagePtr += sectsize;
			 imageSize -= sectsize;
			 newBtBlkCnt++;
                }
            }

#elif defined (_BCM94908_) || defined (_BCM96858_) || defined(_BCM963158_) || \
        defined(_BCM96846_) || defined(_BCM96856_)
	    /* These targets support version 3 bootrom. Boot partition can be up to MAX_BOOT_PARTITION_SIZE in size */
            while (((sectsize*newBtBlkCnt)  < MAX_BOOT_PARTITION_SIZE)
                   && (*(unsigned short *)imagePtr != JFFS2_MAGIC_BITMASK)
                   && (be32_to_cpu(*(unsigned int *)imagePtr) != UBI_EC_HDR_MAGIC)
                  ) {
	              status = kerSysBcmImageSet(FLASH_BASE + (sectsize*newBtBlkCnt), imagePtr, sectsize, 0);
		      if (status) {
		          printf("Failed to flash block number %d . Error: %d\n", newBtBlkCnt, status);
		      }

	              imagePtr += sectsize;
		      imageSize -= sectsize;
		      newBtBlkCnt++;
            }
#endif
	}

#if 0
        /* imagePtr should point at a JFFS2_MAGIC_BITMASK at this point regardless if the .w is a jffs2 or ubi image.*/
	/* If the .w is ubi image, imagePtr should be pointing at the start of the bootfs portion which is JFFS2     */
        if( *(unsigned short *) imagePtr == JFFS2_MAGIC_BITMASK )
           printf("\nJFFS2_MAGIC_BITMASK located as expected\n\n");
	else
           printf("\nExpected a JFFS2_MAGIC_BITMASK marker at this point, but was not located \n\n");
#endif
        NVRAM_SYNC();
	/* If the new image is an fs-only image, or has the same number of  */
	/* boot blocks as the flash layout, just verify the partition table.*/
	/* If the new image has a different amount of boot blocks than the  */
	/* way the current flash layout is, redo the partition table        */
	if ((newBtBlkCnt == 0) || (curBtBlkCnt == newBtBlkCnt)) {
	     validateNandPartTbl(0, 0);
	} else {
	     validateNandPartTbl(1, newBtBlkCnt);
#if defined(_BCM963268_)
             if( (wfiTag.wfiFlags & WFI_FLAG_SUPPORTS_BTRM) && 
                 (!bcm_otp_is_btrm_boot()) && 
                 (wfiTag.wfiFlashType == WFI_NAND16_FLASH) ) {
                 /* small page nand migrating from unsecure to secure boot.    */
                 /* update the ulCheckSum within the new image's 3k nvram now  */
                 /* that the nvram's partition table info is finalized         */
                 /* Read the 1k nvram from flash(even though there is 3k of it)*/
                 /* Copy the 1k into the 1st 1k of the new image's nvram space */
		 PNVRAM_DATA pnd = (PNVRAM_DATA) (imagePtrBegin + NVRAM_DATA_OFFSET);
		 memcpy((unsigned char *) pnd, (unsigned char *) NVRAM_RP, sizeof(NVRAM_DATA));
		 /* Update the ulCheckSum_1k field */ 
		 pnd->ulCheckSum = 0;
		 pnd->ulCheckSum = getCrc32((unsigned char *) pnd, sizeof(NVRAM_DATA), CRC32_INIT_VALUE);
		 /* Update the ulChecksum field at the very end of the 3k      */
		 unsigned int *pChkSumIn3k = (unsigned int *)&pnd->ulCheckSum;
		 pChkSumIn3k += (0x800/sizeof(unsigned int));
		 *pChkSumIn3k = 0;
		 *pChkSumIn3k = getCrc32((unsigned char *) pnd, (sizeof(NVRAM_DATA) * 3), CRC32_INIT_VALUE);
		 /* rewrite block 0 from the image back into flash block 0     */
		 if((status = kerSysBcmImageSet(FLASH_BASE, imagePtrBegin, sectsize, 0)) != 0)
		   printf("Failed to flash boot block 0. Error: %d\n", status);
		 
		 NVRAM_SYNC();
	     }
#endif

        }


	/* Change 000 to be xxx+1 on either the cferam.000, secram.000, or secmfg.000 */
        /* bootloader within the new whole image, and return the block offset         */
        cferam_blk = nandUpdateImageSequenceNumber(imagePtr, imageSize, (rootfs == NP_ROOTFS_1) ? 1 : 2);

        /* rootfs is the partition that the CFE RAM booted from.  Write the
         * image to the other rootfs partition.
         */
	
	
        if(rootfs == NP_ROOTFS_1 && NVRAM.ulNandPartSizeKb[NP_ROOTFS_2] > 0 &&
            wfiTag.wfiVersion != WFI_VERSION_NAND_1MB_DATA )
            offset = NVRAM.ulNandPartOfsKb[NP_ROOTFS_2] * 1024;
        else
            offset = NVRAM.ulNandPartOfsKb[NP_ROOTFS_1] * 1024;

        after_cferam = (cferam_blk + 1) * sectsize;
        cferam_overhead = overhead_blocks * sectsize;

        /* Erase block with cferam JFFS2 directory entry so if flashing this
         * image does not finish, the partition will not be valid.
         */
        for( i = 0; i < (cferam_blk + 1 + overhead_blocks); i++ )
            flash_sector_erase_int((offset / sectsize) + i);

        /* Flash image after cferam directory entry. */
        printf("\nFlashing root file system at address 0x%8.8lx (flash offset 0x%8.8lx): ", FLASH_BASE + offset, offset);
        if((status = kerSysBcmImageSet(FLASH_BASE + offset + after_cferam +
            cferam_overhead, imagePtr + after_cferam, imageSize - after_cferam,
            1)) != 0) {
            printf("Failed to flash whole image. Error: %d\n", status);
	    goto err_out;
        }

        /* Flash block(s) up to and including the block with cferam JFFS2
         * directory entry.
         */
        if((status = kerSysBcmImageSet(FLASH_BASE + offset, imagePtr, 
                                              after_cferam, 0)) != 0) {
            printf("Failed to flash whole image. Error: %d\n", status);
	    goto err_out;
        }

        /* If an old image is being flashed, delete the data partition. */
        if(rootfs == NP_ROOTFS_1 && wfiTag.wfiVersion == WFI_VERSION_NAND_1MB_DATA) {
            int blkstart = NVRAM.ulNandPartOfsKb[NP_ROOTFS_2] / blksize;
            int blkend = blkstart + 
                (NVRAM.ulNandPartSizeKb[NP_ROOTFS_2] / blksize);

            printf("\nOld image, deleting data and secondary file system "
                "partitions\n");
            kerSysErasePsi();
            for( i = blkstart; i < blkend; i++ )
                flash_sector_erase_int(i);
        }
    }
    else 
#endif
    {
        /* NOR FLASH */
        if( (wfiTag.wfiVersion & WFI_ANY_VERS_MASK) == WFI_ANY_VERS &&
            wfiTag.wfiFlashType != WFI_NOR_FLASH ) {
            printf("\nERROR: Image does not support a NOR flash device.\n\n");
	    goto err_out;
        }

#if defined (_BCM963138_) || defined (_BCM963148_) || defined (_BCM94908_) || defined (_BCM96858_)
       if( offset == 0 ) {
            if( (wfiTag.wfiFlags&WFI_FLAG_HAS_PMC) )
                printf("Flash PMC.\n");
           StallPmc();
           status = kerSysBcmImageSet(FLASH_BASE+offset, imagePtr, IMAGE_OFFSET,0);
           UnstallPmc();

           if( status != 0 )  {
               printf("Flashing PMC failed\n");
	   }
           offset += IMAGE_OFFSET;
           imagePtr += IMAGE_OFFSET;
	   imageSize -= IMAGE_OFFSET;
       }
#endif

#if defined(_BCM963268_)
       if( (wfiTag.wfiFlags & WFI_FLAG_SUPPORTS_BTRM) ) {
          // New image is for secure boot ... check to see whether unsecure to secure migration is taking place
          if( otp_is_boot_secure() ) {
             // Upgrading an already-secure-boot 63268 target. 
	     // Copy new credentials into nvram buffer that might be written back into flash after image flashing is done
             unsigned char *pNewCreds = (unsigned char *)(imagePtr + NVRAM_DATA_OFFSET + NVRAM_BOOTLDR_SIG_OFFSET);
             unsigned char *pOldCreds = NULL; /*((unsigned char *)nvramData) + NVRAM_BOOTLDR_SIG_OFFSET;*/
	     memcpy(nvramData, NVRAM_RP, sizeof(NVRAM_DATA));
	     pOldCreds = ((unsigned char *)nvramData) + NVRAM_BOOTLDR_SIG_OFFSET;
             memcpy(pOldCreds, pNewCreds, NVRAM_SECURITY_CREDENTIALS_LEN);
             nvramData->ulCheckSum = 0;
             nvramData->ulCheckSum = getCrc32((unsigned char *) nvramData, sizeof(NVRAM_DATA), CRC32_INIT_VALUE);
	     // Copy existing NVRAM data (first 1k) to image nvram
             PNVRAM_DATA pnd = (PNVRAM_DATA) (imagePtr + NVRAM_DATA_OFFSET);
             memcpy((unsigned char *) pnd, (unsigned char *) nvramData, 1024); /* Do NOT change 1024 to be sizeof(NVRAM_DATA) */
             pnd->ulCheckSum = 0;
             pnd->ulCheckSum = getCrc32((unsigned char *) pnd, sizeof(NVRAM_DATA), CRC32_INIT_VALUE);
          }  else  {
             // Unsecure to secure migration taking place ... update the image's future 3k nvram checksum location
	     // using the old 1k nvram content and the new security credentials
             unsigned char *pNewNvramIn3k = (unsigned char *)(imagePtr + NVRAM_DATA_OFFSET);
             memcpy(pNewNvramIn3k, (unsigned char *)nvramData, sizeof(NVRAM_DATA));
             unsigned int *pChkSumIn3k = (unsigned int *)(imagePtr + NVRAM_DATA_OFFSET + (sizeof(NVRAM_DATA) * 3) - sizeof(unsigned int));
             *pChkSumIn3k = 0;
             *pChkSumIn3k = getCrc32( pNewNvramIn3k, (sizeof(NVRAM_DATA) * 3), CRC32_INIT_VALUE);
          }
       }
#endif

        printf("\nFlashing cfe, root file system and kernel at address 0x%8.8lx (flash offset 0x%8.8lx)\n",
                FLASH_BASE + offset, offset);
	status = kerSysBcmImageSet(FLASH_BASE+offset, imagePtr, imageSize, 1);
        if (status) {
            printf("Failed to flash whole image. Error: %d\n", status);
	    goto err_out;
        }
    }

      /* Check if the new image has valid NVRAM
         Also check if the new image still supports currently configured board ID
      */
      memset(nvramData, 0, sizeof(NVRAM_DATA));
      if (NVRAM_READ_VERIFY(nvramData)) {	
        /* Don't write NVRAM area if we are flashing tiny bridge image.
           unlike cfe.w, the tiny bridge .w image will not have NVRAM_DATA_ID set */
        if (*(unsigned int *) nvramData == NVRAM_DATA_ID) {
	    /* No need for NVRAM syncing since 
	       it's being done via update call*/
	    NVRAM_UPDATE(NULL);
	} 
    }
    
    NVRAM_SYNC();  
    status = 0;
 err_out:
    if (nvramData) {
        KFREE(nvramData);
    }
    return status;
}

void displayPromptUsage(void)
{
    printf("Press:  <enter> to use current value\r\n");
    printf("        '-' to go previous parameter\r\n");
    printf("        '.' to clear the current value\r\n");
    printf("        'x' to exit this command\r\n");
}

int processPrompt(PPARAMETER_SETTING promptPtr, int promptCt)
{
    char tmpBuf[MAX_PROMPT_LEN];
    int i = 0;
    int bChange = FALSE;

    memset(tmpBuf, 0, MAX_PROMPT_LEN);

    /* Ensure proper privelege levels are set for console access */
#if defined(BOARD_SEC_ARCH)
    cfe_sec_set_console_privLvl();
#endif    

    while (i < promptCt)
    {
        if( (promptPtr+i)->enabled == FALSE )
        {
            if( tmpBuf[0] == '-' )
            {
                if( i > 0 )
                {
                    i--;
                    continue;
                }
            }
            else
            {
                i++;
                continue;
            }
        }

        if (strlen((promptPtr+i)->parameter) > 0)
            printf("%s  %s  ", (promptPtr+i)->promptName, (promptPtr+i)->parameter);
        else
            printf("%s  %s", (promptPtr+i)->promptName, (promptPtr+i)->parameter);

	    memset(tmpBuf, 0, MAX_PROMPT_LEN);
	    console_readline ("", tmpBuf, (promptPtr+i)->maxValueLength + 1);

        switch (tmpBuf[0])
        {
            case '-':          // go back one parameter
                if (i > 0)
                    i--;
                break;
            case 'x':          // get out the b command
                if ((promptPtr+i)->func != 0)  // validate function is supplied, do a check
                {
                    if ((promptPtr+i)->func((promptPtr+i)->parameter))
                    {
                        printf("\n%s;  Try again!\n", (promptPtr+i)->errorPrompt);
                        break;
                    }
                }
                i = promptCt;
                break;
            case '.':          // clear the current parameter and advance
                if ((promptPtr+i)->func != 0)  // validate function is supplied, do a check
                {
                    if ((promptPtr+i)->func(""))
                    {
                        printf("\n%s;  Try again!\n", (promptPtr+i)->errorPrompt);
                        break;
                    }
                }
                memset((promptPtr+i)->parameter, 0, MAX_PROMPT_LEN);
                i++;
                bChange = TRUE;
                break;
            case 0:            // no input; use default if it is OK
                if ((promptPtr+i)->func != 0)  // validate function is supplied, do a check
                {
                    if ((promptPtr+i)->func((promptPtr+i)->parameter))
                    {
                        printf("\n%s;  Try again!\n", (promptPtr+i)->errorPrompt);
                        break;
                    }
                }
                i++;
                break;
            default:            // new parameter
                if ((promptPtr+i)->func != 0)  // validate function is supplied, do a check
                {
                    if ((promptPtr+i)->func(tmpBuf))
                    {
                        printf("\n%s;  Try again!\n", (promptPtr+i)->errorPrompt);
                        break;
                    }
                }
                memset((promptPtr+i)->parameter, 0, MAX_PROMPT_LEN);
                memcpy((promptPtr+i)->parameter, tmpBuf, strlen(tmpBuf));
                i++;
                bChange = TRUE;
        }
    }

    return bChange;

} // processPrompt


static void convertBootInfo(void)
{
    char *x;

    memset(&bootInfo, 0, sizeof(BOOT_INFO));
    strcpy(bootInfo.boardIp, gBootParam[PARAM_IDX_BOARD_IPADDR].parameter);

    if ((x = strchr(bootInfo.boardIp, ':')))        // has mask
    {
        *x = '\0';
        convertMaskStr((x+1), bootInfo.boardMask);
    }
    strcpy(bootInfo.hostIp, gBootParam[PARAM_IDX_HOST_IPADDR].parameter);
    if ((x = strchr(bootInfo.hostIp, ':')))        // ignore host mask
        *x = '\0';
    strcpy(bootInfo.gatewayIp, gBootParam[PARAM_IDX_GW_IPADDR].parameter);
    if ((x = strchr(bootInfo.gatewayIp, ':')))        // ignore gw mask
        *x = '\0';
    bootInfo.runFrom = gBootParam[PARAM_IDX_RUN_FROM].parameter[0];
    strcpy(bootInfo.hostFileName, gBootParam[PARAM_IDX_RUN_FILENAME].parameter);
    strcpy(bootInfo.flashFileName, gBootParam[PARAM_IDX_FLASH_FILENAME].parameter);
    strcpy(bootInfo.ramfsFileName, gBootParam[PARAM_IDX_RAMFS_FILENAME].parameter);
    strcpy(bootInfo.dtbFileName, gBootParam[PARAM_IDX_DTB_FILENAME].parameter);
    bootInfo.bootDelay = (int)(gBootParam[PARAM_IDX_BOOT_DELAY].parameter[0] - '0');
    bootInfo.bootPartition = gBootParam[PARAM_IDX_BOOT_IMAGE].parameter[0];
    bootInfo.rdAddr = lib_atoi(gBootParam[PARAM_IDX_RD_ADDR].parameter);
}

int getBootLine(int setdef)
{
    int i, rc = 0;
    char *curPtr;
    char *dPtr;
    char *bootline = NULL;
    bootline = KMALLOC(NVRAM_BOOTLINE_LEN*sizeof(char), sizeof(void*));
    if (!bootline) {
         goto failed;
    }
    memset(bootline, 0,NVRAM_BOOTLINE_LEN*sizeof(char));
#if (SKIP_FLASH!=0)
    rc = 1;
    strncpy(bootline, DEFAULT_BOOTLINE, NVRAM_BOOTLINE_LEN);
    curPtr = bootline;
    for (i = 0; (i < gNumBootParams) && (curPtr != 0); i++) {
        curPtr = strchr(curPtr, '='); 
        if (curPtr) { // found '=' and get the param.
            dPtr = strchr(curPtr, ' ');   // find param. delimiter ' '
            if (dPtr) {
                memset(gBootParam[i].parameter, 0, MAX_PROMPT_LEN);
                memcpy(gBootParam[i].parameter, curPtr+1, dPtr-curPtr-1);
            }
            // move to next param.
            curPtr = dPtr;  
        }
    } // for loop

    if (i < gNumBootParams) {
        if( setdef == 0 ) {
            rc = setDefaultBootline();
        } else {
	    rc = 1;
	    goto failed;
        }
    }

    if( rc == 0 ) {
        convertBootInfo();
    }
#endif

    if ((NVRAM.szBootline[0] == (char)0xff) || 
        (NVRAM.szBootline[0] == (char)0))
    {
        if( setdef == 0 ) {
            rc = setDefaultBootline();
        } else {
            rc = 1;
	    goto failed;
	}
    }
    memcpy(bootline, NVRAM.szBootline, NVRAM_BOOTLINE_LEN*sizeof(char)); 
    curPtr = bootline;
    for (i = 0; (i < gNumBootParams) && (curPtr != 0); i++) {
        curPtr = strchr(curPtr, '='); 
        if (curPtr) { // found '=' and get the param.
            dPtr = strchr(curPtr, ' ');   // find param. deliminator ' '
            if (dPtr) {
                memset(gBootParam[i].parameter, 0, MAX_PROMPT_LEN);
                memcpy(gBootParam[i].parameter, curPtr+1, dPtr-curPtr-1);
            }
            // move to next param.
            curPtr = dPtr;  
        }
    } // for loop
    NVRAM_COPY_FIELD(szBootline, bootline, sizeof(NVRAM.szBootline));
    if (i < gNumBootParams) {
        if( setdef == 0 )
            rc = setDefaultBootline();
        else
        {
	    rc = 1;
	    goto failed;
        }
    }

    if( rc == 0 ) {
#if !defined(_BCM947189_)
        if (cfe_mailbox_status_isset()) {
           unsigned int ver = CFE_MAILBOX_VER_GET(cfe_mailbox_message_get());
           if (ver >= CFE_API_VERSION_1_0 && ver < CFE_API_VERSION_MAX) {
               unsigned int rom_param = getRomParam();
	       if ((rom_param&NAND_IMAGESEL_OVERRIDE)== NAND_IMAGESEL_OVERRIDE) {
                    gBootParam[PARAM_IDX_BOOT_IMAGE].parameter[0] = (char) (rom_param & 0xff);
               }
           }
	}
#endif
 
        convertBootInfo();
    }

 failed:
    if( rc != 0 )
        printf("Failed to parse the boot line parameters! Please try to manually set it with c cmd\n");
    if (bootline) {
        KFREE(bootline); 
    }
    return rc;
}

// print the bootline and board parameter info and fill in the struct for later use
// 
int printSysInfo(void)
{
    int i;
    const ETHERNET_MAC_INFO *EnetInfos;

    // display the bootline info

#if ( INC_EMMC_FLASH_DRIVER == 0 )
    if( getTagFromPartition(1) == NULL || getTagFromPartition(2) == NULL )
        gBootParam[PARAM_IDX_BOOT_IMAGE].enabled = FALSE;
#endif

    for (i = 0; i < gNumBootParams; i++)
        if( gBootParam[i].enabled )
            printf("%s %s  \n", gBootParam[i].promptName, gBootParam[i].parameter);

    // display the board param
    displayBoardParam();

    if( (EnetInfos = BpGetEthernetMacInfoArrayPtr()) != NULL )
    {
        // Here we should print whether EMAC1 or EMAC2 is selected
    }

    printf("\n");

    return 0;
}


//**************************************************************************
// Function Name: changeBootLine
// Description  : Use vxWorks bootrom style parameter input method:
//                Press <enter> to use default, '-' to go to previous parameter  
//                Note: Parameter is set to current value in the menu.
// Returns      : None.
//**************************************************************************
int changeBootLine(void)
{
    int i;
    char boardIpSaved[BOOT_IP_LEN];
    char* bl = KMALLOC(NVRAM_BOOTLINE_LEN, sizeof(void*));
    if (!bl) {
      return -1;
    }
    strcpy(boardIpSaved, bootInfo.boardIp);

    if (processPrompt(gBootParam, gNumBootParams)) {
        char *blPtr = bl;/*NVRAM.szBootline;*/
        int paramLen;

        memset(blPtr, 0, NVRAM_BOOTLINE_LEN);
        for (i = 0; i < gNumBootParams; i++) {
            memcpy(blPtr, gBootParam[i].promptDefine, PROMPT_DEFINE_LEN);
            blPtr += PROMPT_DEFINE_LEN;
            paramLen = strlen(gBootParam[i].parameter);
            memcpy(blPtr, gBootParam[i].parameter, paramLen);
            blPtr += paramLen;
            if (!(gBootParam[i].parameter[0] == ' ')) {
                memcpy(blPtr, " ", 1);
                blPtr += 1;
            }
        }
	NVRAM_UPDATE_FIELD(szBootline, bl, NVRAM_BOOTLINE_LEN);
        kerSysSetBootImageState(gBootParam[PARAM_IDX_BOOT_IMAGE].parameter[0]);
    }

    getBootLine(0);

    // if board ip differs, do enet init
    if (strcmp(boardIpSaved, bootInfo.boardIp) != 0)
        enet_init();

    KFREE(bl);
    return 0;

}  

int setDefaultBootline(void)
{
    char boardIpSaved[BOOT_IP_LEN];
    int rc = -1;
    char* bl = KMALLOC(NVRAM_BOOTLINE_LEN*sizeof(char), sizeof(void*));
    if (!bl) {
        return -1;
    }
    strcpy(boardIpSaved, bootInfo.boardIp);

    memset(bl, 0, NVRAM_BOOTLINE_LEN);
    strncpy(bl, DEFAULT_BOOTLINE, NVRAM_BOOTLINE_LEN);
    
    printf("Use default boot line parameters: %s\n", DEFAULT_BOOTLINE);
    NVRAM_UPDATE_FIELD(szBootline, bl, NVRAM_BOOTLINE_LEN*sizeof(char));
    rc = getBootLine(1);

    // if board ip differs, do enet init
    if (rc == 0 && strcmp(boardIpSaved, bootInfo.boardIp) != 0) {
        enet_init();
    }
    KFREE(bl);
    return rc;
}

//**************************************************************************
// Function Name: changeAfeId
// Description  : Use vxWorks bootrom style parameter input method:
//                Press <enter> to use default, '-' to go to previous parameter  
//                Note: Parameter is set to current value in the menu.
// Returns      : None.
//**************************************************************************
static void hex2Str(unsigned int num, char *str)
{
	static const char hextable[16] = "0123456789ABCDEF";
	unsigned long     i, n; 
	str[0] = '0';
	str[1] = 'x';
	if (0 == num) {
	   str[2] = '0';
	   str[3] = 0;
	   return;
	}
	str +=2;
	n = num >> 28;
	i = 0;
	while (0 == n) {
	  num <<= 4;
	  n = num >> 28;
	  i++;
	}
	for (; i < 8; i++) {
	  *str++ = hextable[num >> 28];
	  num <<= 4;
	}
	*str = 0;
}

int changeAfeId(void)
{
    unsigned int* afe = KMALLOC(sizeof(NVRAM.afeId), sizeof(NVRAM.afeId));
    if (!afe) {
        return -1;
    }
    hex2Str(NVRAM.afeId[0], gAfeId[0].parameter);
    hex2Str(NVRAM.afeId[1], gAfeId[1].parameter);
    if (processPrompt(gAfeId, gAfeIdParams)) {
	afe[0] = lib_atoi(gAfeId[0].parameter);
	afe[1] = lib_atoi(gAfeId[1].parameter);
        NVRAM_UPDATE_FIELD(afeId, afe, 	sizeof(NVRAM.afeId));
        
    }
    KFREE(afe);
    return 0;
}

#if defined(_BCM963138_) || defined(_BCM963148_) || defined (_BCM94908_) || defined(_BCM96848_) \
    || defined(_BCM963158_) || defined(_BCM96858_) || defined(_BCM96846_) || defined(_BCM96856_)
int changeDDRConfig(void)
{
    unsigned int param;
    hex2Str(NVRAM.ulMemoryConfig, gMemoryConfig[0].parameter);
    if (processPrompt(gMemoryConfig, gMemoryConfigParams)) {
	param = lib_atoi(gMemoryConfig[0].parameter);
        NVRAM_UPDATE_FIELD(ulMemoryConfig, &param, sizeof(NVRAM.ulMemoryConfig));
        printf("Config Changed... REBOOT NEEDED\n");
    }
    return 0;
}
#endif

#if defined(_BCM963138_) || defined(_BCM963148_) || defined (_BCM94908_) || defined(_BCM96848_) \
    || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96856_) || defined(_BCM947189_)
// check nvram feature set for 'feature'
// no features set if nvram is corrupt or erased
int nvFeatureGet(int feature)
{
    return (NVRAM.ulFeatures == ~0) ? 0 : (NVRAM.ulFeatures & feature);
}

// modify nvram feature set by setting 'set' and clearing 'clr'
// return zero if modification doesn't change feature set
int nvFeatureSet(int set, int clr)
{
    unsigned int features;

    // initialize features if nvram corrupt or erased
    if (NVRAM.ulFeatures == ~0)
        NVRAM_SET(ulFeatures, unsigned int, 0);

    features = (NVRAM.ulFeatures & ~clr) | set;
    if (features == NVRAM.ulFeatures)
        return 0;
    NVRAM_SET(ulFeatures, unsigned int, features);
    NVRAM_UPDATE(NULL);
    return 1;
}

#if defined _BCM94908_ || defined _BCM963158_
int getAVSConfig(void)
{
    // enabled if nvram unreadable or not explicitly disabled
    return nvFeatureGet(NVFEAT_AVSDISABLED);
}

void setAVSConfig(int on)
{
    int set, clr;

    if (on) {
        set = 0;
        clr = NVFEAT_AVSDISABLED;
    } else {
        set = NVFEAT_AVSDISABLED;
        clr = 0;
    }
    // avsDisable won't take affect until next boot
    if (nvFeatureSet(set, clr))
        printf("Config Changed... REBOOT NEEDED\n");
}
#endif

#if !defined(_BCM96848_)
int validateMemoryConfig(const NVRAM_DATA* nv, unsigned int *memcfg) 
{
#if (SKIP_FLASH==0)
    unsigned int var;
    if (BP_SUCCESS == BpGetMemoryConfig( &var) ) {
        if ((var != nv->ulMemoryConfig)
            && (((nv->ulMemoryConfig & BP_DDR_CONFIG_OVERRIDE) == 0)
               || (nv->ulMemoryConfig == 0xffffffff))) {
            *memcfg = var;
            return 1;
        }
    }
#endif
    return 0;
}

#if !defined(_BCM96846_) && !defined(_BCM947189_)
void wait_biu_pll_lock(int bpcmaddr)
{
    PLL_STAT_REG stat_reg;

    do {
        ReadBPCMRegister(bpcmaddr, PLLBPCMRegOffset(stat), &stat_reg.Reg32);
    } while (stat_reg.Bits.lock == 0);

    return;
}

#define PLL_GET_CHANNEL_OFFSET(channel)  (PLLBPCMRegOffset(ch01_cfg) + ((channel/2)*sizeof(PLL_CHCFG_REG)>>2))
void set_biu_pll_post_divider(int bpcmaddr, int channel, int mdiv)
{
    PLL_CHCFG_REG pll_ch_cfg;
    int offset, mdiv_rb;

    if( channel < 0 || channel > 5 )
       return;

    offset = PLL_GET_CHANNEL_OFFSET(channel);

    ReadBPCMRegister(bpcmaddr, offset, &pll_ch_cfg.Reg32);
    mdiv_rb = channel&1 ? pll_ch_cfg.Bits.mdiv1 : pll_ch_cfg.Bits.mdiv0;
    if (mdiv_rb != mdiv) {
        if( channel&1 )
            pll_ch_cfg.Bits.mdiv1 = mdiv;
        else
            pll_ch_cfg.Bits.mdiv0 = mdiv;
        WriteBPCMRegister(bpcmaddr, offset, pll_ch_cfg.Reg32);
        cfe_usleep(1000);
        if( channel&1 )
            pll_ch_cfg.Bits.mdiv_override1 = 1;
        else
            pll_ch_cfg.Bits.mdiv_override0 = 1;
        WriteBPCMRegister(bpcmaddr, offset, pll_ch_cfg.Reg32);
        cfe_usleep(10000);
    }

    return;
}
#endif

int cfe_set_cpu_freq(int freqMHz)
{
#if defined(_BCM963138_)
    int mdiv = 0;
    int policy = 0;

    if( freqMHz < 200 || (freqMHz > 1000 && g_force_mode==0) )
    {
        printf("invalid cpu frequency %d, the supported freq is between 200MHz to 1000MHz\n", freqMHz);
	return -1;
    }

    /* enable write access the arm clk mananger */
    ARMCFG->proc_clk.wr_access |= (ARM_PROC_CLK_WR_ACCESS_PASSWORD<<ARM_PROC_CLK_WR_ACCESS_PASSWORD_SHIFT)|ARM_PROC_CLK_WR_ACCESS_CLKMGR_ACC;
    mdiv = 2000 / freqMHz;

    if( mdiv < 10 )	/* setting frequency between 200 MHz and 1 GHz */
    {
        /* set clk divider and enable pll */
	int ndiv = 2 * 1000 / 25; // ndiv based on 1GHz

        ARMCFG->proc_clk.pllarma = (ARMCFG->proc_clk.pllarma&~(ARM_PROC_CLK_PLLARMA_PDIV_MASK|ARM_PROC_CLK_PLLARMA_NDIV_MASK|ARM_PROC_CLK_PLLARMA_PWRDWN_SWOVRRIDE_MASK))|(2<<ARM_PROC_CLK_PLLARMA_PDIV_SHIFT)|(ndiv<<ARM_PROC_CLK_PLLARMA_NDIV_SHIFT)|ARM_PROC_CLK_PLLARMA_SOFT_RESETB_N;

        /* wait for pll to lock */
        while( (ARMCFG->proc_clk.pllarma&ARM_PROC_CLK_PLLARMA_PLL_LOCK_RAW) == 0 );

        /* enable post diveder */
        ARMCFG->proc_clk.pllarma |= ARM_PROC_CLK_PLLARMA_SOFT_POST_RESETB_N;
    }

    /* set the freq policy */
    policy = (freqMHz == 200) ? ARM_PROC_CLK_POLICY_FREQ_SYSCLK : ARM_PROC_CLK_POLICY_FREQ_ARMPLL_SLOW;
    ARMCFG->proc_clk.policy_freq = (ARMCFG->proc_clk.policy_freq&~ARM_PROC_CLK_POLICY_FREQ_MASK)|(policy<<ARM_PROC_CLK_POLICY3_FREQ_SHIFT) |(policy<<ARM_PROC_CLK_POLICY2_FREQ_SHIFT) |(policy<<ARM_PROC_CLK_POLICY1_FREQ_SHIFT) |(policy<<ARM_PROC_CLK_POLICY0_FREQ_SHIFT);

    /* setting the mdiv */
    ARMCFG->proc_clk.pllarmc &= 0xffffff00;
    ARMCFG->proc_clk.pllarmc |= mdiv;
    ARMCFG->proc_clk.pllarmc |= 0x800;

    /* enabled hardware clock gating */
    ARMCFG->proc_clk.core0_clkgate = (ARMCFG->proc_clk.core0_clkgate&~ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_MASK)|(ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_HW<<ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_SHIFT);
    ARMCFG->proc_clk.core1_clkgate = (ARMCFG->proc_clk.core1_clkgate&~ARM_PROC_CLK_CORE1_CLKGATE_GATING_SEL_MASK)|(ARM_PROC_CLK_CORE1_CLKGATE_GATING_SEL_HW<<ARM_PROC_CLK_CORE1_CLKGATE_GATING_SEL_SHIFT);
    ARMCFG->proc_clk.arm_switch_clkgate = (ARMCFG->proc_clk.arm_switch_clkgate&~ARM_PROC_CLK_SWITCH_CLKGATE_GATING_SEL_MASK)|(ARM_PROC_CLK_SWITCH_CLKGATE_GATING_SEL_HW<<ARM_PROC_CLK_SWITCH_CLKGATE_GATING_SEL_SHIFT);
    ARMCFG->proc_clk.arm_periph_clkgate = (ARMCFG->proc_clk.arm_periph_clkgate&~ARM_PROC_CLK_PERIPH_CLKGATE_GATING_SEL_MASK)|(ARM_PROC_CLK_PERIPH_CLKGATE_GATING_SEL_HW<<ARM_PROC_CLK_PERIPH_CLKGATE_GATING_SEL_SHIFT);
    ARMCFG->proc_clk.apb0_clkgate = (ARMCFG->proc_clk.apb0_clkgate&~ARM_PROC_CLK_APB0_CLKGATE_GATING_SEL_MASK)|(ARM_PROC_CLK_APB0_CLKGATE_GATING_SEL_HW<<ARM_PROC_CLK_APB0_CLKGATE_GATING_SEL_SHIFT);

    /* enable the new freq policy */
    ARMCFG->proc_clk.policy_ctl |= (ARM_PROC_CLK_POLICY_CTL_GO_AC|ARM_PROC_CLK_POLICY_CTL_GO);
    
    /* wait for policy to be activated */
    while(ARMCFG->proc_clk.policy_ctl&ARM_PROC_CLK_POLICY_CTL_GO);

#elif defined(_BCM963148_)
    uint32_t val;
    int mdiv0 = 2;

    /* we only support the following frequency:
     * 1) 375, 750, and 1500
     * 2) 125, 250, 500, and 1000 */
    val = B15CTRL->cpu_ctrl.clock_cfg;
    if ((freqMHz == 375) || (freqMHz == 750) || (freqMHz == 1500)) {
        mdiv0 = 2;
        val &= ~0xf;
        val |= (1500 / freqMHz) - 1;
        if (freqMHz == 1500)
            val &= ~0x10;
    } else if ((freqMHz == 125) || (freqMHz == 250) || (freqMHz == 500)  || (freqMHz == 1000)) {
        mdiv0 = 3;
        val &= ~0xf;
        val |= (1000 / freqMHz) - 1;
        if (freqMHz == 1000)
            val &= ~0x10;
    } else if (freqMHz == 7501) {
        /* this is a special setting of 750 MHz for hw team to try */
        mdiv0 = 4;
        val &= ~0xf;
        freqMHz = 750;
    } else {
        printf("%dMHz is not supported, stay with %dMHz\n", freqMHz, cfe_cpu_speed / 1000000);
        return -1;
    }

    set_biu_pll_post_divider(PMB_ADDR_B15_PLL, 0, mdiv0);

    B15CTRL->cpu_ctrl.clock_cfg = val;

#if 0
    PLL_CTRL_REG ctrl_reg;
     /* this code is used to switch from slow to fast VCO PLL / change VCO ndiv/pdiv */
    ReadBPCMRegister(PMB_ADDR_B15_PLL, PLLBPCMRegOffset(resets), &ctrl_reg.Reg32);

    /* set PLL_BYP_REQ (Bit[20]) in PLL_CNTRL */
    ctrl_reg.Bits.byp_wait = 1;
    WriteBPCMRegister(PMB_ADDR_B15_PLL, PLLBPCMRegOffset(resets), ctrl_reg.Reg32);
    cfe_usleep(1);

    /* clear PLL_BYP_REQ (Bit[32]) and de-assert both post and main PLL reset (Bit[1:0]) */
    ctrl_reg.Bits.byp_wait = 0;
    ctrl_reg.Bits.resetb = 0;
    ctrl_reg.Bits.post_resetb = 0;
    WriteBPCMRegister(PMB_ADDR_B15_PLL, PLLBPCMRegOffset(resets), ctrl_reg.Reg32);
    cfe_usleep(1);

    /* read and set main reset */
    ReadBPCMRegister(PMB_ADDR_B15_PLL, PLLBPCMRegOffset(resets), &ctrl_reg.Reg32);
    ctrl_reg.Bits.resetb = 1;
    WriteBPCMRegister(PMB_ADDR_B15_PLL, PLLBPCMRegOffset(resets), ctrl_reg.Reg32);

    wait_biu_pll_lock(PMB_ADDR_B15_PLL);

    ctrl_reg.Bits.post_resetb = 1;
    WriteBPCMRegister(PMB_ADDR_B15_PLL, PLLBPCMRegOffset(resets), ctrl_reg.Reg32);

#endif
#elif defined(_BCM94908_)
    PLL_CTRL_REG ctrl_reg;
    uint32_t clkcfg;

    /* only support 1800MHz and 400MHz for now */
    if( freqMHz != 1800 && freqMHz != 400)
    {
        printf("%dMHz is not supported, stay with %dMHz\n", freqMHz, cfe_cpu_speed / 1000000);
        return -1;
    }

    /* this code is used to switch from default slow cpu 400MHz to 1.8GHz */
    if( freqMHz == 1800 )
    {
        if( cfe_cpu_speed == 400*1000000 )
        {
            ReadBPCMRegister(PMB_ADDR_B53PLL, PLLBPCMRegOffset(resets), &ctrl_reg.Reg32);
            ctrl_reg.Bits.byp_wait = 0;
            WriteBPCMRegister(PMB_ADDR_B53PLL, PLLBPCMRegOffset(resets), ctrl_reg.Reg32);
        }
        /* adjust clock divider and safe mode setting for better perfomance */
        clkcfg = BIUCTRL->clock_cfg;
        clkcfg &= ~(BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_MASK|BIU_CPU_CTRL_CLK_CFG_SAFE_CLOCK_MODE_MASK|BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_MASK);
        clkcfg |= (BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_DIV2|BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_DIV1);
        BIUCTRL->clock_cfg = clkcfg;
    }

    if( freqMHz == 400 )
    {
        /* turn on safe mode setting */
        clkcfg = BIUCTRL->clock_cfg;
        clkcfg &= ~(BIU_CPU_CTRL_CLK_CFG_SAFE_CLOCK_MODE_MASK);
        clkcfg |= (BIU_CPU_CTRL_CLK_CFG_SAFE_CLOCK_MODE_MASK);
        BIUCTRL->clock_cfg = clkcfg;

        if( cfe_cpu_speed == 1800*1000000)
        {
            ReadBPCMRegister(PMB_ADDR_B53PLL, PLLBPCMRegOffset(resets), &ctrl_reg.Reg32);
            ctrl_reg.Bits.byp_wait = 1;
            WriteBPCMRegister(PMB_ADDR_B53PLL, PLLBPCMRegOffset(resets), ctrl_reg.Reg32);
        }
    }
#elif defined(_BCM963158_)
    PLL_CTRL_REG ctrl_reg;
    PLL_NDIV_REG ndiv_reg;
    PLL_PDIV_REG pdiv_reg;
    int mdiv = 2;
    int ndiv;

    /* cpufreq = Fvco/mdiv = ndiv*50MHz/mdiv */
    /* For clock below the nominal frequency, we use post divider mdiv = 2  
       cpufreq = ndiv*50MHz/2 where ndiv = 22..67 for 550MHz, 425MHz.. 1675
       For overclock, we use post divider mdiv=1 in order to keep vco frequency low
       cpufreq = ndiv*50MHz/1 where ndiv = 34 .. 44  for 1700MHz, 1750MHz.. 2200MHz */

    if( freqMHz < 550 || (freqMHz > 1675 && g_force_mode==0) || (freqMHz > 2200 && g_force_mode==1) )
    {
        printf("%dMHz is not supported, stay with %dMHz\n", freqMHz, cfe_cpu_speed / 1000000);
        return -1;
    }

    if( freqMHz > 1675 )
        mdiv = 1;
    ndiv = (mdiv*freqMHz)/50;

    if( g_force_mode )
        printf("request for %dMHz, mdiv %d ndiv %d\n", freqMHz, mdiv, ndiv);

    /* round freqMHz to the factor of 50/25MHz*/
    freqMHz = (50*ndiv)/mdiv;

    /* switch to bypass clock for cpu clock change first */
    ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), &ctrl_reg.Reg32);
    ctrl_reg.Bits.byp_wait = 1;
    WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), ctrl_reg.Reg32);

    /* assert pll reset */
    ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), &ctrl_reg.Reg32);
    ctrl_reg.Bits.master_reset = 1;
    WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), ctrl_reg.Reg32);

    /* change vco pll frequency */
    ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(pdiv), &pdiv_reg.Reg32);
    pdiv_reg.Bits.ndiv_pdiv_override = 1;
    WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(pdiv), pdiv_reg.Reg32);

    ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(ndiv), &ndiv_reg.Reg32);
    ndiv_reg.Bits.ndiv_int = ndiv;
    WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(ndiv), ndiv_reg.Reg32);

   /* de-assert pll reset */
    ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), &ctrl_reg.Reg32);
    ctrl_reg.Bits.master_reset = 0;
    WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), ctrl_reg.Reg32);

    /* wait for pll to lock */
    wait_biu_pll_lock(PMB_ADDR_BIU_PLL);

    /* set the post divider */
    set_biu_pll_post_divider(PMB_ADDR_BIU_PLL, 0, mdiv);

   /* switch back to VCO PLL clock */
    ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), &ctrl_reg.Reg32);
    ctrl_reg.Bits.byp_wait = 0;
    WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), ctrl_reg.Reg32);

#endif

    printf("cpu freq set to %dMHz\n", freqMHz);
    cfe_cpu_speed = freqMHz*1000000;

    return 0;
}
#endif
#endif

/*  code for reading and writing to BPCM register */
#if (BPCM_CFE_CMD==1)
int cfe_read_bpcm(uint8_t bus_id, uint8_t addr_id, uint8_t offset, uint32_t *val)
{
    int devAddr = (bus_id << PMB_BUS_ID_SHIFT) + addr_id;
    return ReadBPCMRegister(devAddr, offset, val);
}

int cfe_write_bpcm(uint8_t bus_id, uint8_t addr_id, uint8_t offset, uint32_t val)
{
    int devAddr = (bus_id << PMB_BUS_ID_SHIFT) + addr_id;
    return WriteBPCMRegister(devAddr, offset, val);
}
#endif

unsigned long convert_to_data_partition_entry_to_bytes(uint16_t size)
{
    unsigned long data_partition_size=0;

    switch(size>>14) {
    case 0:
        data_partition_size=1<<20;
        break;
    case 1:
        data_partition_size=1<<30;
        break;
    default:
        data_partition_size=0;
        break;
    }
    data_partition_size= data_partition_size * (size&0x3fff);
    return data_partition_size;
}

#if (INC_NAND_FLASH_DRIVER == 1) || (INC_SPI_NAND_DRIVER == 1) || (INC_SPI_PROG_NAND == 1)
enum CFE_FS_IMAGE_TYPE {
     CFE_FS_IMAGE_JFFS2=JFFS2_IMAGE, 
     CFE_FS_IMAGE_UBI=UBI_IMAGE, 
     CFE_FS_IMAGE_NOTSUPP
};

static int jffs2_is_jffs2(unsigned int magic)
{
    return magic == JFFS2_MAGIC_BITMASK; 
}


static int jffs2_is_dentry(unsigned int nodetype)
{ 
    return nodetype == JFFS2_NODETYPE_DIRENT;
}


static int jffs2_is_inode(unsigned int nodetype)
{
    return nodetype == JFFS2_NODETYPE_INODE;
}

static int jffs2_match_dentry(struct jffs2_raw_dirent *de, const char* name, unsigned int nsize)
{
    return nsize == de->nsize && !memcmp(name, de->name, nsize);      
}


static struct jffs2_raw_dirent* jffs2_get_dentry(unsigned char* buf,
                                            unsigned int size,
                                            const char* name,
                                            unsigned int nsize,
                                            unsigned int version,
                                            int first_match)
{
    struct jffs2_raw_dirent* de = (struct jffs2_raw_dirent*)buf, 
                  *dend = (struct jffs2_raw_dirent*)(buf + size), *res = NULL;
   /* Assumptions: buf and buf_size must be both aligned to 4 */ 
    while( de < dend ) {
             if (jffs2_is_jffs2(je16_to_cpu(de->magic)) && jffs2_is_dentry(je16_to_cpu(de->nodetype))) {

                 if( je32_to_cpu(de->version) > version  && je32_to_cpu(de->ino) != 0 && jffs2_match_dentry(de, name, nsize) ) {
                     res = de;
                     if (first_match) {
                         /*single version assumed*/
                         break;
                     }
                }
                de = (struct jffs2_raw_dirent*)((unsigned long)de + ((je32_to_cpu(de->totlen) + 0x03) & ~0x03));
             } else {
                de = (struct jffs2_raw_dirent*)((unsigned long)de + sizeof(unsigned int));
             }
   }
   return res;
}

static int jffs2_match_inode(struct jffs2_raw_inode *  inode, struct jffs2_raw_dirent* de)
{
   return (jffs2_is_inode(je16_to_cpu(inode->nodetype)) && je32_to_cpu(inode->ino) == je32_to_cpu(de->ino) &&
              je32_to_cpu(inode->dsize) &&  je32_to_cpu(inode->mtime) == je32_to_cpu(de->mctime));
}

static int jffs2_get_inode_size(unsigned char*  blk_buf, 
                               unsigned int bstart, 
                               unsigned int blkcnt,
                               unsigned int blk_sz,
                               struct jffs2_raw_dirent* de,
                               unsigned int* block_inode, 
                               unsigned int* file_size)
{
    /* Assumptions: buf, buf_size and blk_sz must be aligned to 4 and buf_size > blk_sz*/ 
    struct jffs2_raw_inode* inode = NULL, *iend = NULL;
    unsigned int blk;
    if (!blk_buf) {
        return -1; 
    }

    for (blk = bstart; blk < bstart + blkcnt; blk++) {
         if (flash_read_buf(blk, 0, blk_buf, blk_sz) <= 0) {
             continue;
         }
         inode = (struct jffs2_raw_inode *) blk_buf;
         iend = (struct jffs2_raw_inode*)(blk_buf + blk_sz);
         while (inode < iend) { 
               if (jffs2_is_jffs2(je16_to_cpu(inode->magic))) {
                   if (jffs2_match_inode(inode, de)) {
                       *file_size = je32_to_cpu(inode->isize);
                       *block_inode = blk; 
                       return 0;
                   }
                   inode = (struct jffs2_raw_inode*)((unsigned long)inode + ((je32_to_cpu(inode->totlen) + 0x03) & ~0x03));
               } else {
                   inode = (struct jffs2_raw_inode*)((unsigned long)inode + sizeof(unsigned int));
               }
         }
    }
    return 1;
}

static int jffs2_get_inode_file(unsigned int bstart, 
                               unsigned int blkcnt,
                               unsigned int blk_sz,
                               struct jffs2_raw_dirent* de,
                               unsigned char** file_data, 
                               unsigned int* file_size ,
                               unsigned int dst_offs)
{
    /* Assumptions: buf, buf_size and blk_sz must be aligned to 4 and buf_size > blk_sz*/ 
    struct jffs2_raw_inode* inode = NULL, *iend = NULL;
    unsigned int isize = 0, size = 0, blk, block_inode = 0;
    void*  blk_buf = KMALLOC(blk_sz, sizeof(void*)), *dst_buf = NULL;
    unsigned char *dst_end;
    if (!blk_buf) {
        return -1; 
    }

    if (jffs2_get_inode_size(blk_buf, bstart, blkcnt, blk_sz, de,
                               &block_inode, 
                               &isize)) {
        goto err_out;
    }

    if (isize > cfe_get_mempool_size()) {
        printf("Out of memory %u\n", cfe_get_mempool_size());
        goto err_out;
    }
    dst_buf = (void*)cfe_get_mempool_ptr();
    dst_end = (unsigned char*)dst_buf + isize;
    *file_size = isize;
    for (blk = block_inode; blk < bstart + blkcnt; blk++) {
         if (flash_read_buf(blk, 0, blk_buf, blk_sz) <= 0) {
             continue;
         }
         inode = (struct jffs2_raw_inode *) blk_buf;
         iend = (struct jffs2_raw_inode*)(blk_buf + blk_sz);
         while (inode < iend) { 
               if (jffs2_is_jffs2(je16_to_cpu(inode->magic))) {
                   if (jffs2_match_inode(inode, de)) {
                       unsigned char *ptr ;
                       size = je32_to_cpu(inode->dsize);
                       ptr = dst_buf + dst_offs + je32_to_cpu(inode->offset);
                       if (ptr <= dst_end) {
                           memcpy(ptr, inode->data, size);
                       }
                       isize -= size;
                       if (isize <= 0) {
                           KFREE(blk_buf);
                           *file_data = dst_buf; 
                           return 0;
                       }
                   }
                   inode = (struct jffs2_raw_inode*)((unsigned long)inode + ((je32_to_cpu(inode->totlen) + 0x03) & ~0x03));
               } else {
                   inode = (struct jffs2_raw_inode*)((unsigned long)inode + sizeof(unsigned int));
               }
         }
    }
err_out:
    KFREE(blk_buf);
    return -1;
}

static struct jffs2_raw_dirent* jffs2_find_dentry(unsigned char* blk_buf,
                           const char* name,
                           unsigned int nlen,
                           unsigned int bstart,
                           unsigned int bcnt,
                           unsigned int blk_sz) 
{
        unsigned int blk;
        /* deserved an assert() ... */
        if ( blk_sz ==0 ) {
             return NULL;
        }
        /**/
        for (blk = bstart; blk < bstart + bcnt ; blk++) {
            if (flash_read_buf(blk, 0, blk_buf, blk_sz) > 0) {
                struct jffs2_raw_dirent* de = jffs2_get_dentry(blk_buf, blk_sz, name, nlen, 0, 1);
                if (de) {
                   return de;
                } 
            }
        }
        return NULL;
}

static int jffs2_find_file(unsigned char* blk_buf,
                           const char* name,
                           unsigned int nlen,
                           unsigned int bstart,
                           unsigned int bcnt,
                           unsigned int blk_sz,
                           unsigned char **dst, 
                           unsigned int *dst_size,
                           unsigned int dst_offs)
{
        struct jffs2_raw_dirent *de = NULL;
        de = jffs2_find_dentry(blk_buf, name, 
                nlen, bstart, bcnt, blk_sz);
        if (!de) {
            return -1;
        }
       /* Buffer management: since we're not using dyn memory allocation - 
              buf has to be advanced for next read from nand */
        return jffs2_get_inode_file(bstart, bcnt, blk_sz, de, dst, dst_size, dst_offs);
}

static unsigned int read_blk_cb(unsigned char * start, unsigned int block, 
             unsigned int offset, unsigned int blk_size, 
             unsigned char * buf, unsigned int amount, void * mtd, int mtd_fd)
{
    return(flash_read_buf(block, offset, buf + offset, amount));
}
/* Caller should free pointer after use if needed */
static int ubi_find_file(unsigned char* blk_buf,  
                        unsigned int blk, unsigned int bcnt, unsigned int blk_sz, 
                        const char* fname, unsigned char** dst, unsigned int *dst_size, unsigned int dst_offs)
{
        int res = -1;
        void* dst_buf = NULL;
        unsigned int buf_size = ubi_get_ubifile_size(blk_buf, blk, blk + bcnt, 
                                                     blk_sz,(char*)fname, read_blk_cb);
        printf("%s: got %s  size %d\n",__func__, fname, buf_size);
        if (buf_size == 0 || buf_size > cfe_get_mempool_size() || 
           (dst_offs%sizeof(uintptr_t)) ) {
            return res;
        }

        dst_buf = (void*)cfe_get_mempool_ptr();
        if (!dst_buf) {
             return res;
        }
        res = parse_ubi(0, blk_buf, blk, blk + bcnt, blk_sz, VOLID_UBIFILES, 
                    (char*)fname, (char*)dst_buf+dst_offs, 0, 0, read_blk_cb, 0, 0, 0, 0); 
        if ( res ) {
           *dst = dst_buf;
           *dst_size = res;
        }
        
        return (res > 0? 0 : res);
}

static int cfe_fs_is_jffs2(unsigned int magic)
{
     return jffs2_is_jffs2(magic);
}


static int cfe_fs_is_ubi(unsigned int magic)
{
     return magic == UBI_EC_HDR_MAGIC;
}
/* Searches for FS magic starting from the beginning block of a partition 
   up to the size the end 

*/
static enum CFE_FS_IMAGE_TYPE cfe_fs_get_dentry_type(unsigned char *buf, 
                                                      unsigned int size, 
                                                      unsigned int part_blk_start, 
                                                      unsigned int bcnt, 
                                                      unsigned int* blk_de) 
{
    struct fs_dentry {
         union {
              struct jffs2_raw_dirent jffs2; 
              struct ubi_ec_hdr ec;
         };
    };

    struct fs_dentry* de = (struct fs_dentry*)buf;
    unsigned int blk = part_blk_start; 
    for( ; blk < blk + bcnt; blk++ ) {
        if( flash_read_buf(blk, 0, (void*)de, sizeof(struct fs_dentry)) > 0 ) {
            if( cfe_fs_is_jffs2(je16_to_cpu(((struct jffs2_raw_dirent*)de)->magic))) {
                *blk_de = blk; 
                return CFE_FS_IMAGE_JFFS2;
            }
            if (cfe_fs_is_ubi(be32_to_cpu(((struct ubi_ec_hdr*)de)->magic))) {
                *blk_de = blk; 
                return CFE_FS_IMAGE_UBI;
            }
        }
    }
    return  CFE_FS_IMAGE_NOTSUPP;
}


int cfe_fs_find_file(const char* fname, unsigned int fsize, 
                    unsigned int blk, unsigned int bcnt, 
                    unsigned char** dst, unsigned int *dsize,
                    unsigned int dst_offs) 
{ 
    unsigned char* de = NULL;
    int res = -1;
    unsigned int bstart = 0;
    unsigned int blk_sz = flash_get_sector_size(0);
    
    de = KMALLOC(blk_sz, sizeof(void*)); 
    if (!de) {
        return res;
    }

    switch(cfe_fs_get_dentry_type(de, blk_sz, blk, bcnt, &bstart))
    {
             case CFE_FS_IMAGE_JFFS2:
                  res = jffs2_find_file(de, fname, fsize, 
                                     bstart, bcnt, blk_sz, dst, dsize, dst_offs);
                  break; 
             case CFE_FS_IMAGE_UBI:
                  res = ubi_find_file(de, bstart, bcnt, blk_sz, fname, dst, dsize, dst_offs);
                  break;
             default:
                  break;
    }

    KFREE(de); 
    return res;
}


int get_rootfs_offset(char boot_state, unsigned int *bstart, unsigned int* bcnt)
{
    
    PFILE_TAG tag1 = getTagFromPartition(1);
    PFILE_TAG tag2 = getTagFromPartition(2);
    unsigned int rfs_part;
    validateNandPartTbl(0, 0);

    if( tag1 && tag2 ) {
        unsigned int seq1, seq2; 
        seq1 = atoi(tag1->imageSequence); 
        seq2 = atoi(tag2->imageSequence); 
        /* Deal with wrap around case */
        if (seq1 == 0 && seq2 == 999)
            seq1 = 1000;
        if (seq2 == 0 && seq1 == 999)
            seq2 = 1000;

        if( boot_state == BOOT_SET_NEW_IMAGE ) {
            rfs_part = (seq2 > seq1) ? NP_ROOTFS_2 : NP_ROOTFS_1;
        }
        else /* select previous image partition */ {
            rfs_part = (seq2 <= seq1) ? NP_ROOTFS_2 : NP_ROOTFS_1;
        }

    } else {
        rfs_part = tag1? NP_ROOTFS_1 : (tag2 ? NP_ROOTFS_2 : NP_TOTAL);
    }

    if (rfs_part >= 0 && rfs_part < NP_TOTAL) {
         unsigned int len = flash_get_sector_size(0) / 1024;
        *bstart = NVRAM.ulNandPartOfsKb[rfs_part] / len;
        *bcnt =  NVRAM.ulNandPartSizeKb[rfs_part] / len;
        return 1;
    }
    return 0;
}


/* finds dtb in a storage media such as nand and copies it to the destination*/
int cfe_fs_fetch_file(const char* fname, unsigned int fnsize,
               unsigned char** file, unsigned int* file_size)
{
     unsigned int  bstart, bcnt;
     if (!get_rootfs_offset(bootInfo.bootPartition, &bstart, &bcnt)) {
         printf("%s: Error can't find rootfs partition\n", __func__);
         return -1;
     }

     if (cfe_fs_find_file(fname, fnsize, bstart, bcnt, file, file_size, 0)) {
         printf("%s: Error locating %s image\n", __func__, fname);
         return -1;
     }

     return 0;
}

#endif

/* load file from bootfs. Caller allocate buf with maximum size. 
   return 0 success and *size contains the actual file size */
int cfe_load_boot_file(const char *fname, unsigned char *buf, unsigned int* buflen, unsigned int options)
{
    unsigned int size = 0, origsize = 0;
    unsigned char *data = 0;
    int rc = CFE_ERR;
#ifdef CONFIG_CFE_SUPPORT_HASH_BLOCK
    unsigned int content_len;
    unsigned char hash[SHA256_S_DIGEST8];
    int hashfound = 0;
#endif
    char brcmMagic[] = {'B','R','C','M'};
    uint32_t image_hdr_size = 0;
    bcm_image_hdr_t image_hdr;

#ifdef CONFIG_CFE_SUPPORT_HASH_BLOCK
    if (options&BOOT_FILE_LOAD_OPT_HASHVERIFY) {
        if (hash_block_start==NULL) {
            if (load_hash_block(0, 0) != 0)
                return CFE_ERR_NOHASH;
        }

       if (find_boot_hash(&content_len, hash, hash_block_start, (char*)fname)){
           printf("%s got hash for file %s\n", __func__, fname);
           hashfound = 1;
       } else {
           printf("%s Find hash for file %s failed\n", __func__, fname);
           hashfound = 0;
       }
    }
#endif

#if (INC_NAND_FLASH_DRIVER == 1) || (INC_SPI_NAND_DRIVER == 1)
    if(( flash_get_flash_type() ==  FLASH_IFC_NAND ) || ( flash_get_flash_type() ==  FLASH_IFC_SPINAND ))
        rc = cfe_fs_fetch_file(fname, strlen(fname), &data, &size);
#endif
#if (INC_EMMC_FLASH_DRIVER == 1)
    if( flash_get_flash_type() == FLASH_IFC_UNSUP_EMMC )
        rc = emmc_load_bootfs_file(fname, strlen(fname), &data, &size);
#endif
#if (INC_SPI_FLASH_DRIVER == 1)
     rc = -1;
#endif

    if (rc) {
        printf("ERROR: %s: Loading %s failed rc %d\n", __func__, fname, rc);
        return CFE_ERR_FILENOTFOUND;
    }
#ifdef CONFIG_CFE_SUPPORT_HASH_BLOCK
    if (options&BOOT_FILE_LOAD_OPT_HASHVERIFY) {
        if (hashfound == 0)
            return CFE_ERR_NOHASH;
        if (sec_verify_sha256((uint8_t const*)data, size, (const uint8_t *)hash)){
            printf("%s File %s Digest failed\n", __func__, fname);
            return CFE_ERR_HASHERR;
        } else
            printf("%s file %s Digest OK\n", __func__, fname);
    }
#endif

    if (options&BOOT_FILE_LOAD_OPT_COMPRESS) {
        memcpy(&image_hdr, (void*)data, sizeof(image_hdr));
        /* leagacy adjustments:
          new image format contains broadcom signature and uncompressed length.*/
        image_hdr_size = (image_hdr.magic==*(uint32_t*)brcmMagic)? sizeof(image_hdr) : sizeof(image_hdr)-sizeof(uint32_t);
        data += image_hdr_size;
#ifdef USE_LZ4_DECOMPRESSOR
        if (image_hdr.len_uncomp) {
            if (image_hdr.len_uncomp <= *buflen ) {
                rc = LZ4_decompress_fast((const char *)data, (char *)buf, image_hdr.len_uncomp) != image_hdr.len;
            } else {
                printf("%s lz4 uncompress file size %d large than buf size %d\n", __func__, image_hdr.len_uncomp, *buflen);
                rc = CFE_ERR_NOMEM;
            }
            *buflen = image_hdr.len_uncomp;
        } else
#endif
        {
            origsize = LzmaGetUncompSize((unsigned char*)data);
            if (origsize <= *buflen && origsize != 0 ) {
                rc = decompressLZMA((unsigned char*)data, image_hdr.len, (unsigned char*)buf, origsize);
            } else {
                printf("%s lzma uncompress file size %d large than buf size %d\n", __func__, origsize, *buflen);
                rc = CFE_ERR_NOMEM;
            }
            *buflen = origsize;
        }
        if (rc && rc != CFE_ERR_NOMEM) {
            printf("Failed to decompress %s image.  ret = %d Corrupted image?\n",image_hdr.len_uncomp? "LZ4":"LZMA",rc);
        }
    } else {
        if( *buflen >= size )
            memcpy(buf, data, size);
        else {
            printf("%s file size %d large than buf size %d\n", __func__, size, *buflen);
            rc = CFE_ERR_NOMEM;
        }
        *buflen = size;
    }

    if ( rc == CFE_OK && options&BOOT_FILE_LOAD_OPT_FLUSHCACHE) {
        _cfe_flushcache(CFE_CACHE_FLUSH_RANGE, buf, buf + *buflen - 1);
    }

    return rc;
}

/* this function returns the larger of 32 blocks worth of NAND reserved for the data partiton
   or the default NAND data partition size so that UBIFS will operate correctly within the data partition */
unsigned long min_data_partition_size_kb(void)
{
    int ulBlockSizeKb = flash_get_sector_size(0)/1024;

    return( ((NAND_DATA_SIZE_KB / ulBlockSizeKb) >= 32) ? NAND_DATA_SIZE_KB : ulBlockSizeKb * 32);
}

#if defined(_BCM963138_)
int get_nr_cpus(unsigned int *nr_cpus)
{
int i;
struct id_cpu_pair {
    int chip_id;
    int nr_cpus;
} uni_processor_family_ids[] = {
    {CHIP_63132_ID_HEX,1},
    {CHIP_FAMILY_ID_HEX,2},
    {0,0}
};

    if(!nr_cpus) return 1;

    *nr_cpus=2;
    for (i=0; uni_processor_family_ids[i].chip_id != 0; i++ )
    {
        if(UtilGetChipId() == uni_processor_family_ids[i].chip_id)
        {
            *nr_cpus=uni_processor_family_ids[i].nr_cpus;
            break;
        }
    }
    return 0;
}
#endif

static int brcm_enforce_ui_cmd_restrictions( void )
{
    int do_restrict = 0;
    /* Apply restriction processing logic here */

#if defined(BOARD_SEC_ARCH)
    /* restrict cmds if console is non-secure and we are in a secureboot environment */
    if ( cfe_sec_is_console_nonsec())
    {
        do_restrict = 1;
    }
#endif    
    return do_restrict;
}

int brcm_cmd_addcmd(char *command, int (*func)(ui_cmdline_t *,int argc,char *argv[]),
    void *ref, char *help, char *usage, char *switches, brcm_ui_cmd_type_t cmd_type)
{
    int ret = 0;

    /* Do not add command if type is RESTRICTED and restrictions are being enforced */
    if( !brcm_enforce_ui_cmd_restrictions() || (cmd_type != BRCM_UI_CMD_TYPE_RESTRICTED) )
        ret = cmd_addcmd(command, func, ref, help, usage, switches);

    return ret;
}

