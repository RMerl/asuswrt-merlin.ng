
/***************************************************************************
 * File Name  : bcm_ubi.h
 *
 * Description: This file contains definitions and prototypes for UBI
 * interface
 ***************************************************************************/

#if !defined(_BCM_UBI_H)
#define _BCM_UBI_H

#ifndef __KERNEL__
#include <endian.h>
#include "bcmTag.h"
#define be32_to_cpu be32toh
#endif

/*
 * kernel and userspace files which include this header must map getCrc32 
 * to their implementation of the crc32 function.
 */

#include <linux/jffs2.h>

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

/*
 * Volume type constants used in the volume identifier header.
 *
 */
enum {
	UBI_VID_DYNAMIC = 1,
	UBI_VID_STATIC  = 2
};

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

