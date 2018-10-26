
#include "bcm63xx_util.h"
#include "bcm_auth_if.h"
#include "bcm63xx_auth.h"
#include "flash_api.h"
#include "shared_utils.h"
#include "jffs2.h"
#include "lib_math.h"
#include "bcm_otp.h"
#include "rom_parms.h"
#if INC_EMMC_FLASH_DRIVER        
#include "dev_emmcflash.h"
#endif

#define MAX_HASH_BLOCK_SIZE       0x4000

unsigned char *hash_block_start = NULL;

int find_boot_hash(unsigned int *content_len, unsigned char *hash,unsigned char *hash_block_start, char *fname)
{
   unsigned char *cp;
   struct boot_hash_tlv thead;
   cp = hash_block_start;
   do {
      memcpy(&thead,cp,sizeof(thead));
      // printf("type %x len %d options %x\n",thead.type, thead.length, thead.options );
      if ((thead.type == BOOT_HASH_TYPE_NAME_LEN_SHA256) 
            && (strlen((const char *)&cp[sizeof(thead)]) == strlen(fname)) 
            && (strncmp((const char *)&cp[sizeof(thead)],fname,strlen(fname)) == 0)) {
         memcpy(content_len, cp + thead.length - SHA256_S_DIGEST8 - sizeof(int),sizeof(int));
         // printf("got it name %s content len %d\n",&cp[sizeof(thead)],*content_len);
         memcpy(hash, cp + thead.length - SHA256_S_DIGEST8, SHA256_S_DIGEST8);
         return(1);
      }
   cp = cp + thead.length;
   } while ( thead.type != BOOT_HASH_TYPE_END) ;
   return(0);
}

int load_hash_block(int start_blk, int end_blk)
{
    unsigned char *hash_block_buf = NULL;
    unsigned int hash_block_size = 0;
    char hash_file_name[BRCM_MAX_BOOTFS_FILENAME_LEN]; 
    Booter1AuthArgs authArgs;
    int ret = -1;

    memset(hash_file_name, 0x00, sizeof(hash_file_name));
    strncpy(hash_file_name, NAND_HASH_BIN_NAME, sizeof(hash_file_name)-1);
#if (INC_NAND_FLASH_DRIVER == 1) || (INC_SPI_NAND_DRIVER == 1)
    if (start_blk == 0 && end_blk == 0) {
        unsigned int blk_cnt = 0;
        if (!get_rootfs_offset(bootInfo.bootPartition, (unsigned int*)&start_blk, (unsigned int*)&blk_cnt)) {
             printf("%s: Error can't find rootfs partition\n", __func__);
             return -1;
        }
        end_blk = start_blk + blk_cnt;
    }
#endif

    if (bcm_otp_is_boot_secure()) {
       CFE_RAM_ROM_PARMS_AUTH_PARM_GETM(&authArgs);
       memset(hash_file_name, 0x00, sizeof(hash_file_name));
       strncpy(hash_file_name, NAND_HASH_SECBT_NAME, sizeof(hash_file_name)-1);
#if defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96856_)
       if ( bcm_otp_is_boot_mfg_secure() )
       {
           memset(hash_file_name, 0x00, sizeof(hash_file_name));
           strncpy(hash_file_name, NAND_HASH_SECBT_MFG_NAME, sizeof(hash_file_name)-1);
       }
#endif // devices supporting mfg mode
#if (INC_NAND_FLASH_DRIVER == 1) || (INC_SPI_NAND_DRIVER == 1)
       if(( flash_get_flash_type() ==  FLASH_IFC_NAND ) || ( flash_get_flash_type() ==  FLASH_IFC_SPINAND )) {
           ret = cfe_fs_find_file(hash_file_name,
                              strlen(hash_file_name),
                              start_blk, end_blk - start_blk,
                              &hash_block_buf, &hash_block_size, 0);
       }
#endif
#if (INC_EMMC_FLASH_DRIVER == 1)
       if( flash_get_flash_type() == FLASH_IFC_UNSUP_EMMC ) {
           ret = emmc_load_bootfs_file(hash_file_name, strlen(hash_file_name), 
                              &hash_block_buf, &hash_block_size);
       }
#endif
#if (INC_SPI_FLASH_DRIVER == 1)
       ret = -1;
#endif
       if( ret ) {
           printf("missing hash bin file %s\n", hash_file_name);
           return -1;
       }

       // printf("Got hash file ...  size is %d mod %d\n",hash_block_size,SEC_S_MODULUS);
       if (sec_verify_signature((uint8_t const*)(&hash_block_buf[SEC_S_MODULUS]), hash_block_size-SEC_S_MODULUS, &hash_block_buf[0], authArgs.manu)) {
               printf("FAILED AUTH\n");
               hash_block_start = NULL;
               return -1;
       }
       printf("PASSED AUTH\n");
       hash_block_start = KMALLOC(hash_block_size, sizeof(int));
       memcpy(hash_block_start,&hash_block_buf[SEC_S_MODULUS],hash_block_size);
    } else {
#if (INC_NAND_FLASH_DRIVER == 1) || (INC_SPI_NAND_DRIVER == 1)
       if(( flash_get_flash_type() ==  FLASH_IFC_NAND ) || ( flash_get_flash_type() ==  FLASH_IFC_SPINAND )) {
           ret =  cfe_fs_find_file(hash_file_name,
                              strlen(hash_file_name),
                              start_blk, end_blk - start_blk,
                              &hash_block_buf, &hash_block_size, 0);
       }
#endif
#if (INC_EMMC_FLASH_DRIVER == 1)
       if( flash_get_flash_type()  == FLASH_IFC_UNSUP_EMMC ) {
           ret = emmc_load_bootfs_file(hash_file_name, strlen(hash_file_name), 
                              &hash_block_buf, &hash_block_size);
       }
#endif
#if (INC_SPI_FLASH_DRIVER == 1)
       ret = -1;
#endif
       if( ret ) {
           printf("missing hash bin file %s\n", hash_file_name);
           hash_block_size = 0;
           return -1;
       }

       hash_block_start = KMALLOC(hash_block_size, sizeof(int));
       memcpy(hash_block_start,hash_block_buf,hash_block_size);
    } 

    return ret;
}
