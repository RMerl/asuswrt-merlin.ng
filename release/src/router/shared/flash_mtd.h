#ifndef __FLASH_MTD_H
#define __FLASH_MTD_H

#include <bcmnvram.h>

#define NUM_INFO 16
#define MAX_READ_CNT 0x10000

#define BOOTLOADER_MTD_NAME	"Bootloader"
#if defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ)
#define NVRAM_MTD_NAME		MTD_OF_NVRAM
#else
#define NVRAM_MTD_NAME		"nvram"
#endif
#if defined(RTCONFIG_LANTIQ)
#define FACTORY_MTD_NAME	"data2"
#elif defined(RTCONFIG_ALPINE)
#define FACTORY_MTD_NAME	"data0"
#else
#define FACTORY_MTD_NAME	"Factory"
#endif
#define CALDATA_MTD_NAME	"caldata"

struct mtd_info {
	char dev[8];
	int size;
	int erasesize;
	int writesize;
	char name[12];
	char type;
};

//extern void flash_mtd_usage(char *cmd);
extern int flash_mtd_init_info(void);
extern int flash_mtd_open(int num, int flags);
extern int flash_mtd_read(int offset, int count);
extern int MTDPartitionRead(const char *mtd_name, const unsigned char *buf, int offset, int count);
extern int FactoryRead(const unsigned char *buf, int offset, int count);
extern int linuxRead(const unsigned char *buf, int offset, int count);
extern int FlashRead(const unsigned char *dst, int src, int count);
extern int CalRead(const unsigned char *buf, int offset, int count);
extern int FRead(const unsigned char *buf, int addr, int count);
extern int flash_mtd_write(int offset, int value);
extern int MTDPartitionWrite(const char *mtd_name, const unsigned char *buf, int offset, int count);
extern int FactoryWrite(const unsigned char *buf, int offset, int count);
extern int FWrite(const unsigned char *buf, int addr, int count);
extern int FlashWrite(const unsigned char *src, int offset, int count);
extern int flash_mtd_erase(int start, int end);
//extern int flash_mtd_main(int argc, char *argv[]);


#endif
