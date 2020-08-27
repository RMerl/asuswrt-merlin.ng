 /*
 * Copyright 2019, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#ifndef __libnuc029_h__
#define __libnuc029_h__

#include <sys/types.h>
#include <sys/stat.h>

#define NUC029_DEBUG            "/tmp/NUC029_DEBUG"
#define I2C_NODE                "/dev/i2c-"
#define I2C_DEVICE              0x30
#define BUFSIZE                 64

//NUC029 I2C Register Addr
#define REG_LED_MUTE           0x01
#define REG_KEY                0x02
#define REG_LED                0x10
#define REG_LED_RGB            0xF0
#define REG_LED_VOLUME         0x0A
#define REG_FW_VER             0xA0
#define REG_RESET_LDROM        0xA1
#define REG_MIC_GPIO_CTRL      0xB0
#define REG_WDT					0xA3
#define REG_KEY_LED_SWITCH		0xA4
#define REG_NIGHT_MODE			0xA5

#define REG_LDROM_CHECK        0xA2
#define REG_LDROM_BOOT_APROM   0xA3
#define REG_LDROM_FW           0xA4

#define MyDBG(fmt,args...) \
	if(isFileExist(NUC029_DEBUG) > 0) { \
		printf("[NUC029][%s:(%d)]"fmt, __FUNCTION__, __LINE__, ##args); \
	}

static int isFileExist(char *fname)
{
	struct stat fstat;
	
	if (lstat(fname,&fstat)==-1)
		return 0;
	if (S_ISREG(fstat.st_mode))
		return 1;
	
	return 0;
}

extern int read_register2str(unsigned char *reg, int bytes, unsigned char *data, int bufsize);
extern int read_register(unsigned char *reg, int bytes, unsigned char *data);
extern int write_register(unsigned char *data, int datasize);
extern int GetMcuVer(unsigned char *data, size_t size);
extern int SetVolumeLED(int vol);
extern int SetMuteLED(int val);
extern int SetLED(int val);
extern int SetRGB_LED(int R, int G, int B, int LED, int LED2);
extern int UpgradeAPROM(const char *name);
extern void print_buf(unsigned char *buf, int size);

#endif
