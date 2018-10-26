
#if !defined(_BCM63XX_AUTH_H_)
#define _BCM63XX_AUTH_H_

#include "bcm_auth_if.h"

#define HASH_BLOCK_ROOTFS_ENTRY_NAME "rootfs"
#define BOOT_HASH_TYPE_END (0)
#define BOOT_HASH_TYPE_NAME_LEN_SHA256 (1)

#define BOOT_FILE_FLAG_HASH_BOOT   (1)
#define BOOT_FILE_FLAG_COMPRESSED  (1 << 1)
#define BOOT_FILE_FLAG_ENCRYPTED   (1 << 2)

struct boot_hash_tlv {
        unsigned int type;
        unsigned int length;
        unsigned int options;
};

extern unsigned char *hash_block_start ;


extern int find_boot_hash(unsigned int *content_len, unsigned char *hash,unsigned char *hash_block_start, char *fname);
extern int load_hash_block(int start_blk, int end_blk);

#endif
