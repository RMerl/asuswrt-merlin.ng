#ifndef _BCM63xx_BOOT_H_
#define _BCM63xx_BOOT_H_

#define CFE_BOOT_FILE_NAME_MAX_LENGTH   (sizeof(NAND_CFE_RAM_NAME)+10)

typedef struct _cfe_rom_media_params
{
	/* shared resources */
	int       boot_secure;
	char      boot_file_hash[SHA256_S_DIGEST8]  __attribute__ ((aligned (4)));
	int       boot_file_hash_valid;
	int       boot_file_flags;
	char      boot_file_name[CFE_BOOT_FILE_NAME_MAX_LENGTH];
	char      hash_file_name[CFE_BOOT_FILE_NAME_MAX_LENGTH];
	
	
	/* emmc private */
	/* nand private */
	/* spi nand private */
	/* nor private */
}cfe_rom_media_params;


void boot_media(void);
void parse_boot_hashes(char *hashes, cfe_rom_media_params *media_params);


#endif /*_BCM63xx_BOOT_H_*/
