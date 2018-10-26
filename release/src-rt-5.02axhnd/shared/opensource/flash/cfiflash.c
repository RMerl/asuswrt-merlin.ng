/*
    Copyright 2000-2010 Broadcom Corporation
   <:label-BRCM:2012:DUAL/GPL:standard
   
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

/** Includes. */
#ifdef _CFE_                                                
#include "lib_types.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "cfe_timer.h"
#include "bcm_map_part.h"
#define CFI_USLEEP(x) cfe_usleep(x)
#define printk  printf
#else       // linux
#include <linux/version.h>
#include <linux/init.h>
#include <linux/param.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <asm/delay.h>
#include <bcm_map_part.h>       
#define CFI_USLEEP(x) udelay(x)
#endif

#include "bcmtypes.h"
#include "bcm_hwdefs.h"
#include "flash_api.h"

/** Defines. **/
#ifndef NULL
#define NULL 0
#endif

#ifndef _CFE_
struct semaphore cfi_flash_lock;
#define CF_FLASH_SCHED_BYTES 512
static int cfi_flash_sched = 0; /* voluntary schedule() disabled by default */
#endif

#define MAXSECTORS  1024      /* maximum number of sectors supported */

/* Standard Boolean declarations */
#define TRUE            1
#define FALSE           0

/* Define different type of flash */
#define FLASH_UNDEFINED 0
#define FLASH_AMD       1
#define FLASH_INTEL     2
#define FLASH_SST       3

/* Command codes for the flash_command routine */
#define FLASH_RESET     0       /* reset to read mode */
#define FLASH_READ_ID   1       /* read device ID */
#define FLASH_CFIQUERY  2       /* CFI query */
#define FLASH_UB        3       /* go into unlock bypass mode */
#define FLASH_PROG      4       /* program a unsigned short */
#define FLASH_UBRESET   5       /* reset to read mode from unlock bypass mode */
#define FLASH_SERASE    6       /* sector erase */
#define FLASH_WRITE_BUF 7       /* write to buffer */

/* Return codes from flash_status */
#define STATUS_READY    0       /* ready for action */
#define STATUS_TIMEOUT  1       /* operation timed out */

/* A list of AMD compatible device ID's - add others as needed */
#define ID_AM29DL800T   0x224A
#define ID_AM29DL800B   0x22CB
#define ID_AM29LV800T   0x22DA
#define ID_AM29LV800B   0x225B
#define ID_AM29LV400B   0x22BA
#define ID_AM29LV200BT  0x223B

#define ID_AM29LV160B   0x2249
#define ID_AM29LV160T   0x22C4

#define ID_AM29LV320T   0x22F6
#define ID_MX29LV320AT  0x22A7
#define ID_AM29LV320B   0x22F9
#define ID_MX29LV320AB  0x22A8
#define ID_MX29LV640BT  0x22C9

#define ID_AM29LV320M   0x227E
#define ID_AM29LV320MB  0x2200
#define ID_AM29LV320MT  0x2201

#define ID_SST39VF200A  0x2789
#define ID_SST39VF400A  0x2780
#define ID_SST39VF800A  0x2781
#define ID_SST39VF1601  0x234B
#define ID_SST39VF3201  0x235B
#define ID_SST39VF6401  0x236B

/* A list of Intel compatible device ID's - add others as needed */
#define ID_I28F160C3T   0x88C2
#define ID_I28F160C3B   0x88C3
#define ID_I28F320C3T   0x88C4
#define ID_I28F320C3B   0x88C5
#define ID_I28F640J3	0x8916

#define ID_M29W640FB   0x22FD

#define CFI_FLASH_DEVICES                   \
         {{ID_AM29DL800T, "AM29DL800T"},    \
          {ID_AM29DL800B, "AM29DL800B"},    \
          {ID_AM29LV800T, "AM29LV800T"},    \
          {ID_AM29LV800B, "AM29LV800B"},    \
          {ID_AM29LV400B, "AM29LV400B"},    \
          {ID_AM29LV200BT, "AM29LV200BT"},  \
          {ID_AM29LV160B, "AM29LV160B"},    \
          {ID_AM29LV160T, "AM29LV160T"},    \
          {ID_AM29LV320T, "AM29LV320T"},    \
          {ID_MX29LV320AT, "MX29LV320AT"},  \
          {ID_AM29LV320B, "AM29LV320B"},    \
          {ID_MX29LV320AB, "MX29LV320AB"},  \
          {ID_AM29LV320M, "AM29LV320M"},    \
          {ID_AM29LV320MB, "AM29LV320MB"},  \
          {ID_AM29LV320MT, "AM29LV320MT"},  \
          {ID_MX29LV640BT, "MX29LV640BT"},  \
          {ID_SST39VF200A, "SST39VF200A"},  \
          {ID_SST39VF400A, "SST39VF400A"},  \
          {ID_SST39VF800A, "SST39VF800A"},  \
          {ID_SST39VF1601, "SST39VF1601"},  \
          {ID_SST39VF3201, "SST39VF3201"},  \
          {ID_SST39VF6401, "SST39VF6401"},  \
          {ID_I28F160C3T, "I28F160C3T"},    \
          {ID_I28F160C3B, "I28F160C3B"},    \
          {ID_I28F320C3T, "I28F320C3T"},    \
          {ID_I28F320C3B, "I28F320C3B"},    \
          {ID_I28F640J3,  "I28F640J3"},     \
          {ID_M29W640FB, "STM29W640FB"},    \
          {0, ""}                           \
        }

/** Structs. **/
/* A structure for identifying a flash part.  There is one for each
 * of the flash part definitions.  We need to keep track of the
 * sector organization, the address register used, and the size
 * of the sectors.
 */
struct flashinfo {
     char *name;         /* "Am29DL800T", etc. */
     unsigned long addr; /* physical address, once translated */
     int areg;           /* Can be set to zero for all parts */
     int nsect;          /* # of sectors -- 19 in LV, 22 in DL */
     int bank1start;     /* first sector # in bank 1 */
     int bank2start;     /* first sector # in bank 2, if DL part */
 struct {
    long size;           /* # of bytes in this sector */
    long base;           /* offset from beginning of device */
    int bank;            /* 1 or 2 for DL; 1 for LV */
     } sec[MAXSECTORS];  /* per-sector info */
     int write_buffer_size; /* max size of multi byte write */
};

/*
 * This structure holds all CFI query information as defined
 * in the JEDEC standard. All information up to 
 * primary_extended_query is standard among all manufactures
 * with CFI enabled devices.
 */

struct cfi_query {
    int num_erase_blocks;       /* Number of sector defs. */
	long device_size;		/* Device size in bytes */
    struct {
      unsigned long sector_size;    /* byte size of sector */
      int num_sectors;      /* Num sectors of this size */
    } erase_block[8];       /* Max of 256, but 8 is good */
    int write_buffer_size; /* max size of multi byte write */
};

/** Prototypes. **/
int cfi_flash_init(flash_device_info_t **flash_info);
static int cfi_flash_sector_erase_int(unsigned short sector);
static int cfi_flash_read_buf(unsigned short sector, int offset,
    unsigned char *buffer, int numbytes);
static int cfi_flash_write_buf(unsigned short sector, int offset,
    unsigned char *buffer, int numbytes);
static int cfi_flash_get_numsectors(void);
static int cfi_flash_get_sector_size(unsigned short sector);
static unsigned char *cfi_flash_get_memptr(unsigned short sector);
static int cfi_flash_get_blk(int addr);
static int cfi_flash_get_total_size(void);
static int cfi_flash_dev_specific_cmd(unsigned int command, void * inBuf, void * outBuf);
static void cfi_flash_command(int command, unsigned short sector, int offset,
    unsigned short data, unsigned short *);
static int cfi_flash_write(unsigned short sector, int offset, unsigned char *buf,
    int nbytes);
static int cfi_flash_write_to_buffer(unsigned short sector, unsigned char *buf);
static int cfi_flash_wait(unsigned short sector, int offset,unsigned short data);
static unsigned short cfi_flash_get_device_id(void);
static int cfi_flash_get_cfi(struct cfi_query *query, unsigned short *cfi_struct,
    int flashFamily);
static int cfi_memcmp_sched(unsigned char *s1, unsigned char *s2, int nbytes);

/** Variables. **/
static flash_device_info_t flash_cfi_dev =
    {
        0xffff,
        FLASH_IFC_PARALLEL,
        "",
        cfi_flash_sector_erase_int, // fn_flash_sector_erase_int in flash_api.c
        cfi_flash_read_buf,         // fn_flash_read_buf in flash_api.c
        cfi_flash_write_buf,        // fn_flash_write_buf in flash_api.c
        cfi_flash_get_numsectors,   // fn_flash_get_numsectors in flash_api.c
        cfi_flash_get_sector_size,  // fn_flash_get_sector_size in flash_api.c
        cfi_flash_get_memptr,       // fn_flash_get_memptr in flash_api.c
        cfi_flash_get_blk,          // fn_flash_get_blk in flash_api.c
        cfi_flash_get_total_size,   // fn_flash_get_total_size in flash_api.c
        cfi_flash_dev_specific_cmd  // fn_flash_dev_specific_cmd in flash_api.c
    };

/*********************************************************************/
/* 'meminfo' should be a pointer, but most C compilers will not      */
/* allocate static storage for a pointer without calling             */
/* non-portable functions such as 'new'.  We also want to avoid      */
/* the overhead of passing this pointer for every driver call.       */
/* Systems with limited heap space will need to do this.             */
/*********************************************************************/
static struct flashinfo meminfo; /* Flash information structure */
static int flashFamily = FLASH_UNDEFINED;
static int totalSize = 0;
static struct cfi_query query;

static unsigned short cfi_data_struct_29W160[] = {
    0x0020, 0x0049, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0x0051, 0x0052, 0x0059, 0x0002, 0x0000, 0x0040, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0027, 0x0036, 0x0000, 0x0000, 0x0004,
    0x0000, 0x000a, 0x0000, 0x0004, 0x0000, 0x0003, 0x0000, 0x0015,
    0x0002, 0x0000, 0x0000, 0x0000, 0x0004, 0x0000, 0x0000, 0x0040,
    0x0000, 0x0001, 0x0000, 0x0020, 0x0000, 0x0000, 0x0000, 0x0080,
    0x0000, 0x001e, 0x0000, 0x0000, 0x0001, 0xffff, 0xffff, 0xffff,
    0x0050, 0x0052, 0x0049, 0x0031, 0x0030, 0x0000, 0x0002, 0x0001,
    0x0001, 0x0004, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0x0002,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0x0888, 0x252b, 0x8c84, 0x7dbc, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
};

static UINT16 cfi_data_struct_29W200[] = {
    0x0020, 0x0049, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0x0051, 0x0052, 0x0059, 0x0002, 0x0000, 0x0040, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0027, 0x0036, 0x0000, 0x0000, 0x0004,
    0x0000, 0x000a, 0x0000, 0x0004, 0x0000, 0x0003, 0x0000, 0x0015,
    0x0002, 0x0000, 0x0000, 0x0000, 0x0004, 0x0000, 0x0000, 0x0040,
    0x0000, 0x0001, 0x0000, 0x0020, 0x0000, 0x0000, 0x0000, 0x0080,
    0x0000, 0x0002, 0x0000, 0x0000, 0x0001, 0xffff, 0xffff, 0xffff,
    0x0050, 0x0052, 0x0049, 0x0031, 0x0030, 0x0000, 0x0002, 0x0001,
    0x0001, 0x0004, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0x0002,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0x0888, 0x252b, 0x8c84, 0x7dbc, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
};

static UINT16 cfi_data_struct_26LV800B[] = {
    0x0020, 0x0049, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0x0051, 0x0052, 0x0059, 0x0002, 0x0000, 0x0040, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0027, 0x0036, 0x0000, 0x0000, 0x0004,
    0x0000, 0x000a, 0x0000, 0x0004, 0x0000, 0x0003, 0x0000, 0x0015,
    0x0002, 0x0000, 0x0000, 0x0000, 0x0004, 0x0000, 0x0000, 0x0040,
    0x0000, 0x0001, 0x0000, 0x0020, 0x0000, 0x0000, 0x0000, 0x0080,
    0x0000, 0x000e, 0x0000, 0x0000, 0x0001, 0xffff, 0xffff, 0xffff,
    0x0050, 0x0052, 0x0049, 0x0031, 0x0030, 0x0000, 0x0002, 0x0001,
    0x0001, 0x0004, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0x0002,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0x0888, 0x252b, 0x8c84, 0x7dbc, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
};

static struct flash_name_from_id fnfi[] = CFI_FLASH_DEVICES;

/*********************************************************************/
/* Init_flash is used to build a sector table from the information   */
/* provided through the CFI query.  This information is translated   */
/* from erase_block information to base:offset information for each  */
/* individual sector. This information is then stored in the meminfo */
/* structure, and used throughout the driver to access sector        */
/* information.                                                      */
/*                                                                   */
/* This is more efficient than deriving the sector base:offset       */
/* information every time the memory map switches (since on the      */
/* development platform can only map 64k at a time).  If the entire  */
/* flash memory array can be mapped in, then the addition static     */
/* allocation for the meminfo structure can be eliminated, but the   */
/* drivers will have to be re-written.                               */
/*                                                                   */
/* The meminfo struct occupies 653 bytes of heap space, depending    */
/* on the value of the define MAXSECTORS.  Adjust to suit            */
/* application                                                       */ 
/*********************************************************************/
int cfi_flash_init(flash_device_info_t **flash_info)
{
    struct flash_name_from_id *fnfi_ptr;
    int i=0, j=0, count=0;
    int basecount=0L;
    unsigned short device_id;
    int flipCFIGeometry = FALSE;

    *flash_info = &flash_cfi_dev;

#ifndef _CFE_
    sema_init(&cfi_flash_lock, 1);
#endif

    /* First, assume
     * a single 8k sector for sector 0.  This is to allow
     * the system to perform memory mapping to the device,
     * even though the actual physical layout is unknown.
     * Once mapped in, the CFI query will produce all
     * relevant information.
     */
    meminfo.addr = 0L;
    meminfo.areg = 0;
    meminfo.nsect = 1;
    meminfo.bank1start = 0;
    meminfo.bank2start = 0;
    
    meminfo.sec[0].size = 8192;
    meminfo.sec[0].base = 0x00000;
    meminfo.sec[0].bank = 1;
        
    cfi_flash_command(FLASH_RESET, 0, 0, 0, NULL);

    flash_cfi_dev.flash_device_id = device_id = cfi_flash_get_device_id();
    flash_cfi_dev.flash_device_name[0] = '\0';
    switch (device_id) {
        case ID_I28F160C3B:
        case ID_I28F320C3B:
        case ID_I28F160C3T:
        case ID_I28F320C3T:
	case ID_I28F640J3:
            flashFamily = FLASH_INTEL;
            break;
        case ID_AM29DL800B:
        case ID_AM29LV800B:
        case ID_AM29LV400B:   
        case ID_AM29LV160B:
        case ID_AM29LV320B:
        case ID_MX29LV320AB:
        case ID_AM29LV320MB:
        case ID_AM29DL800T:
        case ID_AM29LV800T:
        case ID_AM29LV160T:
        case ID_AM29LV320T:
        case ID_MX29LV320AT:
        case ID_AM29LV320MT:
        case ID_AM29LV200BT:
        case ID_MX29LV640BT:
        case ID_M29W640FB:
            flashFamily = FLASH_AMD;
            break;
        case ID_SST39VF200A:
        case ID_SST39VF400A:
        case ID_SST39VF800A:
        case ID_SST39VF1601:
        case ID_SST39VF3201:
        case ID_SST39VF6401:
            flashFamily = FLASH_SST;
            break;
        default:
            return FLASH_API_ERROR;           
    }

    if (cfi_flash_get_cfi(&query, 0, flashFamily) == -1) {
        switch(device_id) {
        case ID_AM29LV160T:
        case ID_AM29LV160B:
            cfi_flash_get_cfi(&query, cfi_data_struct_29W160, flashFamily);
            break;
        case ID_AM29LV200BT:
            cfi_flash_get_cfi(&query, cfi_data_struct_29W200, flashFamily);
            break;
        case ID_AM29LV800B:
            cfi_flash_get_cfi(&query, cfi_data_struct_26LV800B, flashFamily);
            strcpy( flash_cfi_dev.flash_device_name, "MX26LV800B" ); 
            break;
        default:
            return FLASH_API_ERROR;           
        }
    }

    // need to determine if it top or bottom boot here
    switch (device_id)
    {
        case ID_AM29DL800B:
        case ID_AM29LV800B:
        case ID_AM29LV400B:   
        case ID_AM29LV160B:
        case ID_AM29LV320B:
        case ID_MX29LV320AB:
        case ID_AM29LV320MB:
        case ID_I28F160C3B:
        case ID_I28F320C3B:
        case ID_I28F640J3:
        case ID_I28F160C3T:
        case ID_I28F320C3T:
        case ID_SST39VF3201:
        case ID_SST39VF6401:
        case ID_SST39VF200A:
        case ID_SST39VF400A:
        case ID_SST39VF800A:
        case ID_M29W640FB:
            flipCFIGeometry = FALSE;
            break;
        case ID_AM29DL800T:
        case ID_AM29LV800T:
        case ID_AM29LV160T:
        case ID_AM29LV320T:
        case ID_MX29LV320AT:
        case ID_AM29LV320MT:
        case ID_AM29LV200BT:
        case ID_SST39VF1601:
        case ID_MX29LV640BT:
            flipCFIGeometry = TRUE;
            break;
        default:
            return FLASH_API_ERROR;           
    }

    count=0;basecount=0L;

    if (!flipCFIGeometry)
    {

       for (i=0; i<query.num_erase_blocks && basecount < query.device_size; i++) {
            for(j=0; j<query.erase_block[i].num_sectors; j++) {
                meminfo.sec[count].size = (int) query.erase_block[i].sector_size;
                meminfo.sec[count].base = (int) basecount;
                basecount += (int) query.erase_block[i].sector_size;
                count++;
            }
        }
    }
    else
    {
        for (i = (query.num_erase_blocks - 1); i >= 0 && basecount < query.device_size; i--) {
            for(j=0; j<query.erase_block[i].num_sectors; j++) {
                meminfo.sec[count].size = (int) query.erase_block[i].sector_size;
                meminfo.sec[count].base = (int) basecount;
                basecount += (int) query.erase_block[i].sector_size;
                count++;
            }
        }
    }

    meminfo.nsect = count;
    totalSize = meminfo.sec[count-1].base + meminfo.sec[count-1].size;

    if( flash_cfi_dev.flash_device_name[0] == '\0' ) {
        for( fnfi_ptr = fnfi; fnfi_ptr->fnfi_id != 0; fnfi_ptr++ ) {
            if( fnfi_ptr->fnfi_id == device_id ) {
                strcpy( flash_cfi_dev.flash_device_name, fnfi_ptr->fnfi_name ); 
                break;
            }
        }
    }

    meminfo.write_buffer_size = query.write_buffer_size;

    return (FLASH_API_OK);
}

/*********************************************************************/
/* Flash_sector_erase_int() is identical to flash_sector_erase(),    */
/* except it will wait until the erase is completed before returning */
/* control to the calling function.  This can be used in cases which */
/* require the program to hold until a sector is erased, without     */
/* adding the wait check external to this function.                  */
/*********************************************************************/
static int cfi_flash_sector_erase_int(unsigned short sector)
{
    int i;

#ifndef _CFE_
    down(&cfi_flash_lock);
#endif

    for( i = 0; i < 3; i++ ) {
        cfi_flash_command(FLASH_SERASE, sector, 0, 0, NULL);
        if (cfi_flash_wait(sector, 0, 0xffff) == STATUS_READY)
            break;
    }

#ifndef _CFE_
    up(&cfi_flash_lock);
#endif

    return(FLASH_API_OK);
}

/*********************************************************************/
/* flash_read_buf() reads buffer of data from the specified          */
/* offset from the sector parameter.                                 */
/*********************************************************************/
static int cfi_flash_read_buf(unsigned short sector, int offset,
    unsigned char *buffer, int numbytes)
{
    unsigned char *fwp;
    unsigned int size_read = 0;
#ifndef _CFE_
    unsigned int bytes_read = CF_FLASH_SCHED_BYTES;

    down(&cfi_flash_lock);
#endif

    fwp = (unsigned char *) cfi_flash_get_memptr(sector);

    while (numbytes) {
        *buffer++ = *(fwp + offset);
        numbytes--;
        fwp++;
        size_read++;
#ifndef _CFE_
        if (!in_interrupt()) {
                /* Voluntary schedule() if task tagged as need_sched or 512 bytes read */
                /* Must force need_resched or else schedule() is in-effective */
            if ( cfi_flash_sched ) {
                if ( (need_resched()) || (bytes_read >= CF_FLASH_SCHED_BYTES) ) {
                    bytes_read=0;
                    set_tsk_need_resched(current);  /* fake need_resched() to force schedule() */
                    schedule();
                }
                else
                    bytes_read++;
            }
        }
#endif
    }

#ifndef _CFE_
    up(&cfi_flash_lock);
#endif

    return size_read;
}

/*********************************************************************/
/* cfi_memcmp_sched: invokes memcmp with schedule() invocations      */
/*********************************************************************/
static int cfi_memcmp_sched(unsigned char *s1, unsigned char *s2, int nb)
{
#ifndef _CFE_
    const unsigned int sched_chunk = 4 * 1024;
    size_t nbytes;
    int ret = 0;

    while ( nb > 0 )
    {
        if (!in_interrupt()) {
            set_tsk_need_resched(current);
            schedule();
        }

        nbytes = (nb > sched_chunk) ? sched_chunk : nb;

        if ( (ret = memcmp( s1, s2, nbytes )) != 0 )
            break;

        s1 += sched_chunk;
        s2 += sched_chunk;
        nb  -= sched_chunk;
    }

    return ret;
#else
    return memcmp( s1, s2, nb );
#endif
}

/*********************************************************************/
/* flash_write_buf() utilizes                                        */
/* the unlock bypass mode of the flash device.  This can remove      */
/* significant overhead from the bulk programming operation, and     */
/* when programming bulk data a sizeable performance increase can be */
/* observed.                                                         */
/*********************************************************************/
static int cfi_flash_write_buf(unsigned short sector, int offset,
    unsigned char *buffer, int numbytes)
{
    int ret = FLASH_API_ERROR;
    int i;
    unsigned char *p = cfi_flash_get_memptr(sector) + offset;
    int write_buf_method = 0;

    if( meminfo.write_buffer_size != 0 && offset == 0 &&
        (ID_M29W640FB != cfi_flash_get_device_id()) &&
        numbytes == cfi_flash_get_sector_size(sector) )
    {
        write_buf_method = 1;
    }

    /* After writing the flash block, compare the contents to the source
     * buffer.  Try to write the sector successfully up to three times.
     */
    for( i = 0; i < 3; i++ ) {
        if( write_buf_method )
            ret = cfi_flash_write_to_buffer(sector, buffer);
        else
            ret = cfi_flash_write(sector, offset, buffer, numbytes);
        if( !cfi_memcmp_sched( p, buffer, numbytes ) )
            break;
        /* Erase and try again */
        cfi_flash_sector_erase_int(sector);
        ret = FLASH_API_ERROR;
    }

    if( ret == FLASH_API_ERROR )
        printk( "Flash write error.  Verify failed\n" );

    return( ret );
}

/*********************************************************************/
/* Usefull funtion to return the number of sectors in the device.    */
/* Can be used for functions which need to loop among all the        */
/* sectors, or wish to know the number of the last sector.           */
/*********************************************************************/
static int cfi_flash_get_numsectors(void)
{
    return meminfo.nsect;
}

/*********************************************************************/
/* flash_get_sector_size() is provided for cases in which the size   */
/* of a sector is required by a host application.  The sector size   */
/* (in bytes) is returned in the data location pointed to by the     */
/* 'size' parameter.                                                 */
/*********************************************************************/
static int cfi_flash_get_sector_size(unsigned short sector)
{
    return meminfo.sec[sector].size;
}

/*********************************************************************/
/* The purpose of flash_get_memptr() is to return a memory pointer   */
/* which points to the beginning of memory space allocated for the   */
/* flash.  All function pointers are then referenced from this       */
/* pointer.                                  */
/*                                                                   */
/* Different systems will implement this in different ways:          */
/* possibilities include:                                            */
/*  - A direct memory pointer                                        */
/*  - A pointer to a memory map                                      */
/*  - A pointer to a hardware port from which the linear             */
/*    address is translated                                          */
/*  - Output of an MMU function / service                            */
/*                                                                   */
/* Also note that this function expects the pointer to a specific    */
/* sector of the device.  This can be provided by dereferencing      */
/* the pointer from a translated offset of the sector from a         */
/* global base pointer (e.g. flashptr = base_pointer + sector_offset)*/
/*                                                                   */
/* Important: Many AMD flash devices need both bank and or sector    */
/* address bits to be correctly set (bank address bits are A18-A16,  */
/* and sector address bits are A18-A12, or A12-A15).  Flash parts    */
/* which do not need these bits will ignore them, so it is safe to   */
/* assume that every part will require these bits to be set.         */
/*********************************************************************/
static unsigned char *cfi_flash_get_memptr(unsigned short sector)
{
    unsigned char *memptr = (unsigned char*)
        (FLASH_BASE + meminfo.sec[sector].base);

    return (memptr);
}

/*********************************************************************/
/* The purpose of flash_get_blk() is to return a the block number    */
/* for a given memory address.                                       */
/*********************************************************************/
static int cfi_flash_get_blk(int addr)
{
    int blk_start, i;
    int last_blk = cfi_flash_get_numsectors();
    int relative_addr = addr - (int) FLASH_BASE;

    for(blk_start=0, i=0; i < relative_addr && blk_start < last_blk; blk_start++)
        i += cfi_flash_get_sector_size(blk_start);

    if( i > relative_addr )
    {
        blk_start--;        // last blk, dec by 1
    }
    else
        if( blk_start == last_blk )
        {
            printk("Address is too big.\n");
            blk_start = -1;
        }

    return( blk_start );
}

/************************************************************************/
/* The purpose of flash_get_total_size() is to return the total size of */
/* the flash                                                            */
/************************************************************************/
static int cfi_flash_get_total_size(void)
{
    return totalSize;
}

/************************************************************************/
/* cfi_flash_dev_specific_cmd Triggers a device specific feature,             */
/* used to access non-standard features.                                */
/************************************************************************/
static int cfi_flash_dev_specific_cmd(unsigned int command, void * inBuf, void * outBuf)
{
    return 0;
}

/*********************************************************************/
/* Flash_command() is the main driver function.  It performs         */
/* every possible command available to AMD B revision                */
/* flash parts. Note that this command is not used directly, but     */
/* rather called through the API wrapper functions provided below.   */
/*********************************************************************/
static void cfi_flash_command(int command, unsigned short sector, int offset,
    unsigned short data, unsigned short *dataptr)
{
    volatile unsigned short *flashptr;
    volatile unsigned short *flashbase;
    unsigned short i, len;

    flashptr = (unsigned short *) cfi_flash_get_memptr(sector);
    flashbase = (unsigned short *) cfi_flash_get_memptr(0);
    
    switch (flashFamily) {
    case FLASH_UNDEFINED:
        /* These commands should work for AMD, Intel and SST flashes */
        switch (command) {
        case FLASH_RESET:
            flashptr[0] = 0xF0;
            flashptr[0] = 0xFF;
            break;
        case FLASH_READ_ID:
            flashptr[0x5555] = 0xAA;       /* unlock 1 */
            flashptr[0x2AAA] = 0x55;       /* unlock 2 */
            flashptr[0x5555] = 0x90;
            break;
        case FLASH_CFIQUERY:
            flashbase[0x5555] = 0xAA;       /* unlock 1 */
            flashbase[0x2AAA] = 0x55;       /* unlock 2 */
            flashbase[0x5555] = 0x90;
            break;
        default:
            break;
        }
        break;
    case FLASH_AMD:
        switch (command) {
        case FLASH_RESET:
            flashptr[0] = 0xF0;
            break;
        case FLASH_READ_ID:
            flashptr[0x555] = 0xAA;       /* unlock 1 */
            flashptr[0x2AA] = 0x55;       /* unlock 2 */
            flashptr[0x555] = 0x90;
            break;
        case FLASH_CFIQUERY:
            flashptr[0x55] = 0x98;
            break;
        case FLASH_UB:
	     /* the new Spansion S29GL-S chip does not support unlock bypass mode. 
             disable it to support both the old S29GL-P and new chip. This cmd 
             is only used for word write. So it won't affect the performance that
             much. For large buffer, we use write buf for better performance.*/
#ifdef UB_SUPPORT
            flashptr[0x555] = 0xAA;       /* unlock 1 */
            flashptr[0x2AA] = 0x55;       /* unlock 2 */
            flashptr[0x555] = 0x20;
#endif
            break;
        case FLASH_PROG:
#ifdef UB_SUPPORT
            flashptr[0] = 0xA0;
            flashptr[offset/2] = data;
#else
            flashbase[0x555] = 0xAA;       /* unlock 1 */
            flashbase[0x2AA] = 0x55;       /* unlock 2 */
            flashbase[0x555] = 0xa0;
            flashptr[offset/2] = data;
#endif
            break;
        case FLASH_UBRESET:
#ifdef UB_SUPPORT
            flashptr[0] = 0x90;
            flashptr[0] = 0x00;
#endif
            break;
        case FLASH_SERASE:
            flashptr[0x555] = 0xAA;       /* unlock 1 */
            flashptr[0x2AA] = 0x55;       /* unlock 2 */
            flashptr[0x555] = 0x80;
            flashptr[0x555] = 0xAA;
            flashptr[0x2AA] = 0x55;
            flashptr[0] = 0x30;
            break;
        case FLASH_WRITE_BUF:
            flashptr[0x555] = 0xAA;       /* unlock 1 */
            flashptr[0x2AA] = 0x55;       /* unlock 2 */
            flashptr[0] = 0x25;
            offset /= 2;
            len = data / 2;               /* data is bytes to program */
            flashptr[0] = len - 1;
            for( i = 0; i < len; i++ )
                flashptr[offset + i] = *dataptr++;
            flashptr[0] = 0x29;
            break;
        default:
            break;
        }
        break;
    case FLASH_INTEL:
        switch (command) {
        case FLASH_RESET:
            flashptr[0] = 0xFF;
            break;
        case FLASH_READ_ID:
            flashptr[0] = 0x90;
            break;
        case FLASH_CFIQUERY:
            flashptr[0] = 0x98;
            break;
        case FLASH_PROG:
            flashptr[0] = 0x40;
            flashptr[offset/2] = data;
            break;
        case FLASH_SERASE:
            flashptr[0] = 0x60;
            flashptr[0] = 0xD0;
            flashptr[0] = 0x20;
            flashptr[0] = 0xD0;
            break;
        default:
            break;
        }
        break;
    case FLASH_SST:
        switch (command) {
        case FLASH_RESET:
            flashbase[0x5555] = 0xAA;       /* unlock 1 */
            flashbase[0x2AAA] = 0x55;       /* unlock 2 */
            flashbase[0x5555] = 0xf0;
            break;
        case FLASH_READ_ID:
            flashbase[0x5555] = 0xAA;       /* unlock 1 */
            flashbase[0x2AAA] = 0x55;       /* unlock 2 */
            flashbase[0x5555] = 0x90;
            break;
        case FLASH_CFIQUERY:
            flashbase[0x5555] = 0xAA;       /* unlock 1 */
            flashbase[0x2AAA] = 0x55;       /* unlock 2 */
            flashbase[0x5555] = 0x98;
            break;
        case FLASH_UB:
            break;
        case FLASH_PROG:
            flashbase[0x5555] = 0xAA;       /* unlock 1 */
            flashbase[0x2AAA] = 0x55;       /* unlock 2 */
            flashbase[0x5555] = 0xa0;
            flashptr[offset/2] = data;
            break;
        case FLASH_UBRESET:
            break;
        case FLASH_SERASE:
            flashbase[0x5555] = 0xAA;       /* unlock 1 */
            flashbase[0x2AAA] = 0x55;       /* unlock 2 */
            flashbase[0x5555] = 0x80;
            flashbase[0x5555] = 0xAA;
            flashbase[0x2AAA] = 0x55;
            flashptr[0] = 0x30;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

/*********************************************************************/
/* flash_write extends the functionality of flash_program() by       */
/* providing an faster way to program multiple data words, without   */
/* needing the function overhead of looping algorithms which         */
/* program word by word.  This function utilizes fast pointers       */
/* to quickly loop through bulk data.                                */
/*********************************************************************/
static int cfi_flash_write(unsigned short sector, int offset, unsigned char *buf,
    int nbytes)
{
    unsigned short *src;
    src = (unsigned short *)buf;

#ifndef _CFE_
    down(&cfi_flash_lock);
#endif

    if ((nbytes | offset) & 1) {
#ifndef _CFE_
        up(&cfi_flash_lock);
#endif
        return FLASH_API_ERROR;
    }

    cfi_flash_command(FLASH_UB, 0, 0, 0, NULL);
    while (nbytes > 0) {
        cfi_flash_command(FLASH_PROG, sector, offset, *src, NULL);
        if (cfi_flash_wait(sector, offset, *src) != STATUS_READY)
            break;
        offset +=2;
        nbytes -=2;
        src++;
    }
    cfi_flash_command(FLASH_UBRESET, 0, 0, 0, NULL);

#ifndef _CFE_
    up(&cfi_flash_lock);
#endif
    
    return (unsigned char*)src - buf;
}

/*********************************************************************/
/* flash_write_to_buffer                                             */
/*********************************************************************/
static int cfi_flash_write_to_buffer(unsigned short sector, unsigned char *buf)
{
    int nbytes = cfi_flash_get_sector_size(sector);
    int offset;

#ifndef _CFE_
    down(&cfi_flash_lock);
#endif

    for( offset = 0; offset < nbytes; offset += meminfo.write_buffer_size ) {
        cfi_flash_command(FLASH_WRITE_BUF, sector, offset, (unsigned short)
            meminfo.write_buffer_size, (unsigned short *) &buf[offset]);
        if (cfi_flash_wait(sector, 0, 0) != STATUS_READY)
            break;
    }

#ifndef _CFE_
    up(&cfi_flash_lock);
#endif
    
    return offset;
}

/*********************************************************************/
/* flash_wait utilizes the DQ6, DQ5, and DQ2 polling algorithms      */
/* described in the flash data book.  It can quickly ascertain the   */
/* operational status of the flash device, and return an             */
/* appropriate status code (defined in flash.h)                      */
/*********************************************************************/
static int cfi_flash_wait(unsigned short sector, int offset, unsigned short data)
{
    volatile unsigned short *flashptr; /* flash window */
    unsigned short d1;

    flashptr = (unsigned short *) cfi_flash_get_memptr(sector);

    if (flashFamily == FLASH_AMD || flashFamily == FLASH_SST) {
        do {
#ifndef _CFE_
            if (!in_interrupt()) {
                set_tsk_need_resched(current);  /* fake need_resched() to force schedule() */
                schedule();
            }
#endif            
            d1 = *flashptr;    /* read data */
            d1 ^= *flashptr;   /* read it again and see what toggled */
            if (d1 == 0)       /* no toggles, nothing's happening */
                return STATUS_READY;
        } while (!(d1 & 0x20));

        d1 = *flashptr;        /* read data */
        d1 ^= *flashptr;   /* read it again and see what toggled */

        if (d1 != 0) {
            cfi_flash_command(FLASH_RESET, 0, 0, 0, NULL);
            return STATUS_TIMEOUT;
        }
    } else if (flashFamily == FLASH_INTEL) {
        flashptr[0] = 0x70;
        /* Wait for completion */

        do {
#ifndef _CFE_
            if (!in_interrupt()) {
                set_tsk_need_resched(current);  /* fake need_resched() to force schedule() */
                schedule();
            }
#endif
        } while (!(*flashptr & 0x80));
        if (*flashptr & 0x30) {
            flashptr[0] = 0x50;
            cfi_flash_command(FLASH_RESET, 0, 0, 0, NULL);
            return STATUS_TIMEOUT;
        }
        flashptr[0] = 0x50;
        cfi_flash_command(FLASH_RESET, 0, 0, 0, NULL);
    }
    
    return STATUS_READY;
}

/*********************************************************************/
/* flash_get_device_id() will perform an autoselect sequence on the  */
/* flash device, and return the device id of the component.          */
/* This function automatically resets to read mode.                  */
/*********************************************************************/
static unsigned short cfi_flash_get_device_id(void)
{
    volatile unsigned short *fwp; /* flash window */
    unsigned short answer;
    
    fwp = (unsigned short *) cfi_flash_get_memptr(0);
    
    cfi_flash_command(FLASH_READ_ID, 0, 0, 0, NULL);
    answer = *(fwp + 1);
    if (answer == ID_AM29LV320M) {
        answer = *(fwp + 0xe);
        answer = *(fwp + 0xf);
    }
    
    cfi_flash_command(FLASH_RESET, 0, 0, 0, NULL);
    return( (unsigned short) answer );
}

/*********************************************************************/
/* flash_get_cfi() is the main CFI workhorse function.  Due to it's  */
/* complexity and size it need only be called once upon              */
/* initializing the flash system.  Once it is called, all operations */
/* are performed by looking at the meminfo structure.                */
/* All possible care was made to make this algorithm as efficient as */
/* possible.  90% of all operations are memory reads, and all        */
/* calculations are done using bit-shifts when possible              */
/*********************************************************************/
static int cfi_flash_get_cfi(struct cfi_query *query, unsigned short *cfi_struct,
    int flashFamily)
{
    volatile unsigned short *fwp; /* flash window */
    int i=0, temp=0;

    cfi_flash_command(FLASH_CFIQUERY, 0, 0, 0, NULL);
    
    if (cfi_struct == 0)
        fwp = (unsigned short *) cfi_flash_get_memptr(0);
    else
        fwp = cfi_struct;
    
    /* Initial house-cleaning */
    for(i=0; i < 8; i++) {
        query->erase_block[i].sector_size = 0;
        query->erase_block[i].num_sectors = 0;
    }
    
    /* If not 'QRY', then we dont have a CFI enabled device in the socket */
    if( fwp[0x10] != 'Q' &&
        fwp[0x11] != 'R' &&
        fwp[0x12] != 'Y') {
        cfi_flash_command(FLASH_RESET, 0, 0, 0, NULL);
        return(FLASH_API_ERROR);
    }
    
    temp = fwp[0x27];
    query->device_size = (int) (((int)1) << temp);
    
    query->num_erase_blocks = fwp[0x2C];
    if(flashFamily == FLASH_SST)
        query->num_erase_blocks = 1;
    
    for(i=0; i < query->num_erase_blocks; i++) {
        query->erase_block[i].num_sectors =
            fwp[(0x2D+(4*i))] + (fwp[0x2E + (4*i)] << 8);
        query->erase_block[i].num_sectors++;
        query->erase_block[i].sector_size =
            256 * (256 * fwp[(0x30+(4*i))] + fwp[(0x2F+(4*i))]);
    }

    /* TBD. Add support for other flash families. */
    if(flashFamily == FLASH_AMD)
        query->write_buffer_size = (1 << fwp[0x2a]);
    else
        query->write_buffer_size = 0;
    
    cfi_flash_command(FLASH_RESET, 0, 0, 0, NULL);
    return(FLASH_API_OK);
}

#ifndef _CFE_
static int __init cfi_flash_sched_init(void) {
    cfi_flash_sched = 1;    /* voluntary schedule() enabled */
    return 0;
}
late_initcall(cfi_flash_sched_init);
#endif
