
/***************************************************************************
 * File Name  : bcm_ubi.h
 *
 * Description: This file contains definitions and prototypes for UBI
 * interface
 ***************************************************************************/

#if !defined(_BCM_UBI_H)
#define _BCM_UBI_H

#ifdef _CFE_
#include "lib_byteorder.h"
#include "bcm63xx_util.h"
#include "shared_utils.h"
#include "jffs2.h"
#else // Linux

#ifndef __KERNEL__
#include "bcmTag.h"
#define be32_to_cpu be32toh
#endif


#define getCrc32 crc_getCrc32

#include <linux/jffs2.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/* Erase counter header magic number (ASCII "UBI#") */
#define UBI_EC_HDR_MAGIC  0x55424923
/* Volume identifier header magic number (ASCII "UBI!") */
#define UBI_VID_HDR_MAGIC 0x55424921

/* Sizes of UBI headers */
#define UBI_EC_HDR_SIZE  sizeof(struct ubi_ec_hdr)
#define UBI_VID_HDR_SIZE sizeof(struct ubi_vid_hdr)

/* struct ubi_ec_hdr - UBI erase counter header. */
struct ubi_ec_hdr {
	unsigned int magic;
	unsigned char version;
	unsigned char pad1[3];
	unsigned char ec[8];
	unsigned int vid_hdr_offset;
	unsigned int data_offset;
	unsigned char pad2[36];
	unsigned int hdr_crc;
} __attribute__ ((packed));

/* struct ubi_vid_hdr - on-flash UBI volume identifier header. */
struct ubi_vid_hdr {
	unsigned int magic;
	unsigned char version;
	unsigned char vol_type;
	unsigned char copy_flag;
	unsigned char compat;
	unsigned int vol_id;
	unsigned int lnum; // LBA number
	unsigned int leb_ver; /* obsolete, to be removed, don't use */
	unsigned int data_size;
	unsigned int used_ebs;
	unsigned int data_pad;
	unsigned int data_crc;
        unsigned char pad[24];
	unsigned int hdr_crc;
} __attribute__ ((packed));


#define VOLID_METADATA      1
#define VOLID_METADATA_COPY 2
// UBIFILES are CFERAM and/or VMLINUX in blob format
#define VOLID_UBIFILES      10
#define VOLID_UBIFS         0


unsigned int parse_ubi(
    unsigned char * start, // pointer to start of image buffer
    unsigned char * buf,
    unsigned int start_blk,
    unsigned int end_blk,
    unsigned int blk_size,
    int volume_id, // volume id can be from 0 to 127, -1 signifies we are bypassing verification of the header (reading  UBI data instead of direct from mtd which has UBI header data)
    char * name,
    char * data,
    char ** dataP,
    int header,
    unsigned int (*readblk)(unsigned char * start, unsigned int blk, unsigned int offset, unsigned int blk_size, unsigned char * buf, unsigned int amount, void * mtd, int mtd_fd),
    unsigned int (*writeblk)(unsigned char * start, unsigned int blk, unsigned int offset, unsigned int blk_size, unsigned char * buf, unsigned int amount, void * mtd, int mtd_fd),
    unsigned int (*eraseblk)(unsigned int blk, unsigned int blk_size, void * mtd, int mtd_fd),
    void * mtd,
    int mtd_fd
);

#define ubi_get_ubifile_size(buf, sblk, eblk, blk_size, name, readblk)  parse_ubi(0, buf, sblk, eblk, blk_size, VOLID_UBIFILES, \
                                                                                            name, NULL, NULL, -1, readblk, NULL, NULL, NULL, 0) 

#ifdef _CFE_

// print 16 or 32 bit values as hex to console
#define BOARD_SETLED_HEX16(x) board_setleds( ( ((x & 0xF000) << 12) + (((x & 0xF000) > 0x9000) ? 0x37000000 : 0x30000000) ) | \
                               ( ((x & 0xF00 ) << 8 ) + (((x & 0xF00 ) > 0x900 ) ?   0x370000 :   0x300000) ) | \
                               ( ((x & 0xF0  ) << 4 ) + (((x & 0xF0  ) > 0x90  ) ?     0x3700 :     0x3000) ) | \
                               ( ((x & 0xF   )      ) + (((x & 0xF   ) > 0x9   ) ?       0x37 :       0x30) ) )

#define BOARD_SETLED_HEX32(x) BOARD_SETLED_HEX16((int)x>>16);BOARD_SETLED_HEX16((int)x)

#endif


#ifdef __cplusplus
}
#endif

static inline int check_jffs_ubi_magic(unsigned char *buff);

#define je16_to_cpu_bcm(x) ((x).v16)
#define je32_to_cpu(x) ((x).v32)

static inline int check_jffs_ubi_magic(unsigned char *buff)
{
    int ret=0;
    struct jffs2_raw_dirent *pdir;

    struct ubi_ec_hdr *ec = (struct ubi_ec_hdr *) buff;
    pdir = (struct jffs2_raw_dirent *) buff;

#if 0
    printk("pdir->magic %x %x\n",je16_to_cpu(pdir->magic), JFFS2_MAGIC_BITMASK);
    printk("ec->magic %x %x\n", be32_to_cpu(ec->magic), UBI_EC_HDR_MAGIC);
    printk("ec->hdr_crc %x \n", ec->hdr_crc);
#endif

    if( je16_to_cpu_bcm(pdir->magic) == JFFS2_MAGIC_BITMASK || ( (be32_to_cpu(ec->magic) == UBI_EC_HDR_MAGIC) && (getCrc32((void *)ec, UBI_EC_HDR_SIZE-4,CRC32_INIT_VALUE) == be32_to_cpu(ec->hdr_crc)) )) {
       ret=1;
    }

return ret;
}



#endif /* _BCM_UBI_H */

