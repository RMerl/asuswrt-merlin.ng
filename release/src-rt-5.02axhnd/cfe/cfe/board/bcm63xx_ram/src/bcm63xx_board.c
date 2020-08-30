/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  bcm63xx_board.c   utility functions for bcm63xx board
    *  
    *  Created on :  09/25/2002  seanl
    *
    *********************************************************************

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

*/
    
#include "bcm63xx_util.h"
#include "flash_api.h"
#include "bcm63xx_nvram.h"
#if defined(_BCM96858_) || defined(_BCM94908_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96856_)
#include "bcm_otp.h"
#endif
#include "shared_utils.h"

#define str(s)   #s
#define xstr(s)  str(s)

static int g_numBoardIdNames = 0;
static int g_numVoiceBoardIdNames = 0;

extern unsigned short g_force_mode;

static char *g_dectSupportOptions[] = {
   "No DECT",
   "Internal DECT",
   "External DECT"
};
static int g_numDectSupportOptions = 0;

static int parsehwaddr(char *, uint8_t *);
static int parseBoardIdStr(char *);
static int parseVoiceBoardIdStr(char *);
static int parseDectSupportStr(char *);
static int parsePsiSize(char *);
static int parseBackupPsi(char *tpStr);
static int parseSyslogSize(char *tpStr);
static int parseAuxFSPercent(char *);
#if defined(_BCM960333_) || defined(_BCM963268_) || defined(_BCM96838_) || defined(_BCM96848_) || defined(_BCM963381_) 
static int parseMainTp(char *);
#endif
static int parseMacAddrCount(char *);
static int parseMacAddr(char *);
static int charIsHex(char ch);
static int parseGponSN(char *snStr);
static int parseGponPW(char *pwStr);
static int macNumToStr(unsigned char *, char *);
static void getGponBoardParam(void);
#if !defined(_BCM960333_) && !defined(_BCM963268_) && !defined(_BCM963381_) && !defined(_BCM947189_)
static void getAllocBoardParam(void);
static int parseParam1Size(char *ctStr);
static int parseParam2Size(char *ctStr);
static int parseDHDSize(char *ctStr);
static void prepareAllocDHDBoardParam(int index);
#endif

static int parsePartitionSizeStr(char *);

static int gponParamsInitialized(NVRAM_DATA *pNvramData);

static int parseWpsDevicePin(char *pinStr);
static void getWpsDevicePinBoardParam(void);
#define PARAM_IDX_BOARD_NAME                0
#define PARAM_IDX_NUM_MAC_ADDR              1
#define PARAM_IDX_BASE_MAC_ADDR             2
#define PARAM_IDX_PSI_SIZE                  3
#define PARAM_IDX_ENABLE_BACKUP_PSI         4
#define PARAM_IDX_SYSLOG_SIZE               5
#define PARAM_IDX_AUXFS_PERCENT             6
#define PARAM_IDX_MAIN_THREAD_NUM           7

#define PARAM_IDX_ALLOC_RDP_PARAM2          0
#define PARAM_IDX_ALLOC_RDP_PARAM1          1


#define PARAM_IDX_ALLOC_DHD1                2
#define PARAM_IDX_ALLOC_DHD2                3
#define PARAM_IDX_ALLOC_DHD3                4

#define PARAM_IDX_GPON_SN                   0
#define PARAM_IDX_GPON_PW                   1

#define PARAM_IDX_WPS_DEVICE_PIN            0

#define PARAM_IDX_VOICE_BOARD_NAME          0
#define PARAM_IDX_DECT_SUPPORT              1

static int  parseWlanDeviceFeature(char *feature_str);
static void getWlanDeviceFeatureBoardParam(void);
#define PARAM_IDX_WLAN_DEVICE_FEATURE       0

static PARAMETER_SETTING gWlanDeviceFeatureBoardParam[] =
{
    // prompt name                      Error Prompt Define Param  Validation function
    {"WLan Feature                      :", WLAN_DEVICE_FEATURE_PROMPT, "", "", 4,
        parseWlanDeviceFeature, TRUE},
    {NULL}
};

static int gNumWlanDeviceFeatureBoardParams = (sizeof(gWlanDeviceFeatureBoardParam) / sizeof(PARAMETER_SETTING))-1;
static int parseWlanDeviceFeature(char *feature_str) {
    int isHex=0;
    int wlan_feature=0;
    char *feature=feature_str;
    if(feature_str) {
        if(!strncmp(feature_str,"0x",2)) {
            feature_str+=2;
            isHex=1;
        }
        while(*feature_str) {
            if(((*feature_str)>='0' && (*feature_str)<='9') ||
               (isHex && (((*feature_str)>='a' && (*feature_str)<='f')||
                  ((*feature_str)>='A' && (*feature_str)<='F')))) {
                feature_str++;
                continue;
            } else return 1;
        }
        if(isHex)
                wlan_feature=xtoi(feature);
        else
                wlan_feature=atoi(feature);
        return (wlan_feature<0x80)?0:1;
    } else
        return 1;
}

static void getWlanDeviceFeatureBoardParam(void)
{
    unsigned char  wlan_feature;
    wlan_feature= NVRAM.wlanParams[NVRAM_WLAN_PARAMS_LEN-1];

    /* last bit is reserved for feature reset. when set,it will be restored back to default wlan feature :
     * DEFAULT_WLAN_DEVICE_FEATURE is defined  in /shared/opensource/include/bcm963xx/bcm_hwdefs.h.
     * 
     * the value is also restored to default whenever the value is 0 and default value is not 0
     */

    if((wlan_feature&0x80)||(wlan_feature==0 && DEFAULT_WLAN_DEVICE_FEATURE!=0)){
            NVRAM_SET(wlanParams[NVRAM_WLAN_PARAMS_LEN-1],char,DEFAULT_WLAN_DEVICE_FEATURE);
	    NVRAM_UPDATE(NULL);
            wlan_feature = DEFAULT_WLAN_DEVICE_FEATURE;
    }
    sprintf(gWlanDeviceFeatureBoardParam[PARAM_IDX_WLAN_DEVICE_FEATURE].parameter,"0x%02x",wlan_feature);
}

int setWlanDeviceFeatureBoardParam(void)
{
    int ret = 0;
    getWlanDeviceFeatureBoardParam();

    displayPromptUsage();
    if (processPrompt(gWlanDeviceFeatureBoardParam, gNumWlanDeviceFeatureBoardParams)) {
        char *feature_str=(char *)gWlanDeviceFeatureBoardParam[PARAM_IDX_WLAN_DEVICE_FEATURE].parameter;
        if(feature_str) {
            int wlan_feature=0;
            if(!strncmp(feature_str,"0x",2))
                wlan_feature=xtoi(feature_str);
            else
                wlan_feature=atoi(feature_str);
            NVRAM_SET(wlanParams[NVRAM_WLAN_PARAMS_LEN-1],char, (char)wlan_feature);
	    NVRAM_UPDATE(NULL);
            ret = 1;
        } 
    }
    return ret;
}

static PARAMETER_SETTING gBoardParam[] =
{
    // prompt name                      Error Prompt Define Param  Validation function
    {"Board Id                          :", BOARDID_STR_PROMPT, "", "", 2,
        parseBoardIdStr, TRUE},
    {"Number of MAC Addresses (1-64)    :", MAC_CT_PROMPT, "", "", 2,
        parseMacAddrCount, TRUE},
    {"Base MAC Address                  :", MAC_ADDR_PROMPT, "", "", 17,
        parseMacAddr, TRUE},
    {"PSI Size (1-512) KBytes           :", PSI_SIZE_PROMPT, "", "", 3,
        parsePsiSize, TRUE},
    {"Enable Backup PSI [0|1]           :", BACKUP_PSI_PROMPT, "", "", 1,
        parseBackupPsi, TRUE},
    {"System Log Size (0-256) KBytes    :", SYSLOG_SIZE_PROMPT, "", "", 3,
        parseSyslogSize, TRUE},
    {"Auxillary File System Size Percent:", AUXFS_PERCENT_PROMPT,
        "", "", 2, parseAuxFSPercent, TRUE},
#if defined(_BCM960333_) || defined(_BCM963268_) || defined(_BCM96838_) || defined(_BCM96848_) || defined(_BCM963381_)
    {"Main Thread Number [0|1]          :", CPU_TP_PROMPT, "", "", 1,
        parseMainTp, TRUE},
#endif
    {NULL}
};

static int gNumBoardParams = (sizeof(gBoardParam) / sizeof(PARAMETER_SETTING))-1;

static PARAMETER_SETTING gGponBoardParam[] =
{
    // prompt name                      Error Prompt Define Param  Validation function
    {"GPON Serial Number                :", GPON_SN_PROMPT, "", "", 12,
        parseGponSN, TRUE},
    {"GPON Password                     :", GPON_PW_PROMPT, "", "", 10,
        parseGponPW, TRUE},
    {NULL}
};

static int gNumGponBoardParams = (sizeof(gGponBoardParam) / sizeof(PARAMETER_SETTING))-1;
static int gGponParamsInitialized = 0;

#if defined(_BCM96838_) || defined(_BCM963138_) || defined(_BCM963148_) || defined(_BCM96848_)
static PARAMETER_SETTING gAllocBoardParam[] =
{
    {"MC memory allocation (MB)         :", NULL,
        "", "", 3, parseParam2Size, TRUE},
    {"TM memory allocation (MB)         :", NULL,
        "", "", 3, parseParam1Size, TRUE},
    {"DHD 0 memory allocation (MB)      :", NULL,
        "", "", 3, parseDHDSize, TRUE},
    {"DHD 1 memory allocation (MB)      :", NULL,
        "", "", 3, parseDHDSize, TRUE},
    {"DHD 2 memory allocation (MB)      :", NULL,
        "", "", 3, parseDHDSize, TRUE},
    {NULL}
};
static int gNumAllocBoardParams = (sizeof(gAllocBoardParam) / sizeof(PARAMETER_SETTING))-1;
#elif defined(_BCM94908_) || defined(_BCM963158_)
static PARAMETER_SETTING gAllocBoardParam[] =
{
    {"flow memory allocation (MB)       :", NULL,
        "", "", 3, parseParam2Size, TRUE},
    {"buffer memory allocation (MB)     :", NULL,
        "", "", 3, parseParam1Size, TRUE},
    {"DHD 0 memory allocation (MB)      :", NULL,
        "", "", 3, parseDHDSize, TRUE},
    {"DHD 1 memory allocation (MB)      :", NULL,
        "", "", 3, parseDHDSize, TRUE},
    {"DHD 2 memory allocation (MB)      :", NULL,
        "", "", 3, parseDHDSize, TRUE},
    {NULL}
};
static int gNumAllocBoardParams = (sizeof(gAllocBoardParam) / sizeof(PARAMETER_SETTING))-1;
#elif defined(_BCM96858_) || defined(_BCM96846_) || defined(_BCM96856_)
static PARAMETER_SETTING gAllocBoardParam[] =
{
    {"RNR_TBLS memory allocation (" xstr(MIN_RNRTBLS_SIZE) "-" xstr(MAX_RNRTBLS_SIZE) ") (MB) :", NULL,
        "", "", 3, parseParam2Size, TRUE},
    {"FPM_POOL memory allocation (MB)   :", NULL,
        "", "", 3, parseParam1Size, TRUE},
    {"DHD 0 memory allocation (MB)      :", NULL,
        "", "", 3, parseDHDSize, TRUE},
    {"DHD 1 memory allocation (MB)      :", NULL,
        "", "", 3, parseDHDSize, TRUE},
    {"DHD 2 memory allocation (MB)      :", NULL,
        "", "", 3, parseDHDSize, TRUE},
    {NULL}
};
static int gNumAllocBoardParams = (sizeof(gAllocBoardParam) / sizeof(PARAMETER_SETTING))-1;
#endif

static PARAMETER_SETTING gWpsDevicePinBoardParam[] =
{
    // prompt name                      Error Prompt Define Param  Validation function
    {"Device Pin                        :", WPS_DEVICE_PIN_PROMPT, "", "", 8,
        parseWpsDevicePin, TRUE},
    {NULL}
};

static int gNumWpsDevicePinBoardParams = (sizeof(gWpsDevicePinBoardParam) / sizeof(PARAMETER_SETTING))-1;
static int gWpsDevicePinInitialized = 0;

static PARAMETER_SETTING gVoiceBoardParam[] =
{
    // prompt name                      Error Prompt Define Param  Validation function
    {"Voice Board Configuration (0-#    :", BOARDID_STR_PROMPT, "", "", 2, parseVoiceBoardIdStr, FALSE},
    {"DECT Type Installed (0-#          :", "Invalid option", "", "", 1, parseDectSupportStr, FALSE},
    {NULL}
};

static int gNumVoiceBoardParams = (sizeof(gVoiceBoardParam) / sizeof(PARAMETER_SETTING))-1;
static int gVoiceSupported = 0;

static PARAMETER_SETTING gPartitionsParam[] =
{
    // prompt name                      Error Prompt Define Param  Validation function
    {"Partition 1 Size (MB)             :", PARTITION_STR_PROMPT, "", "", 10,
        parsePartitionSizeStr, TRUE},
    {"Partition 2 Size (MB)             :", PARTITION_STR_PROMPT, "", "", 10,
        parsePartitionSizeStr, TRUE},
    {"Partition 3 Size (MB)             :", PARTITION_STR_PROMPT, "", "", 10,
        parsePartitionSizeStr, TRUE},
    {"Partition 4 Size (MB) (Data)      :", PARTITION_STR_PROMPT, "", "", 10,
        parsePartitionSizeStr, TRUE},
    {NULL}
};

#if (INC_NAND_FLASH_DRIVER==1)
// This is same as BCM_MAX_EXTRA_PARTITIONS
static int gNumPartitionsParam = (sizeof(gPartitionsParam) / sizeof(PARAMETER_SETTING))-1;
#endif

static int parsehwaddr(char *str,uint8_t *hwaddr)
{
    int digit1,digit2;
    int idx = 6;
    
    if (strlen(str) == (MAX_MAC_STR_LEN - 7)) {       // no ':' mac input format ie. 021800100801
        while (*str && (idx > 0)) {
            digit1 = parsexdigit(*str);
            if (digit1 < 0)
                return -1;
            str++;
            if (!*str)
                return -1;
            digit2 = parsexdigit(*str);
            if (digit2 < 0)
                return -1;
            *hwaddr++ = (digit1 << 4) | digit2;
            idx--;
            str++;
        }
        return 0;
    }

    if (strlen(str) != MAX_MAC_STR_LEN-2)
        return -1;
    if (*(str+2) != ':' || *(str+5) != ':' || *(str+8) != ':' || *(str+11) != ':' || *(str+14) != ':')
        return -1;
    
    while (*str && (idx > 0)) {
        digit1 = parsexdigit(*str);
        if (digit1 < 0)
            return -1;
        str++;
        if (!*str)
            return -1;

        if (*str == ':') {
            digit2 = digit1;
            digit1 = 0;
        }
        else {
            digit2 = parsexdigit(*str);
            if (digit2 < 0)
                return -1;
            str++;
        }

        *hwaddr++ = (digit1 << 4) | digit2;
        idx--;

        if (*str == ':')
            str++;
    }
    return 0;
}


static int parseMacAddr(char * macStr)
{
    unsigned char tmpBuf[MAX_PROMPT_LEN];
    
    return (parsehwaddr(macStr, tmpBuf));
}


static int parseBoardIdStr(char *boardIdStr)
{
    int ret = 1;
    int boardId;
    
    if (strlen (boardIdStr) != 0) {
        boardId = atoi(boardIdStr);
        if (boardId >= 0 && boardId < g_numBoardIdNames)
            ret = 0;
    }

    return ret;
}


static int parseVoiceBoardIdStr(char *boardIdStr)
{
    int ret = 1;
    int boardId;
    
    if (strlen (boardIdStr) != 0) {
        boardId = atoi(boardIdStr);
        if (boardId >= 0 && boardId < g_numVoiceBoardIdNames)
            ret = 0;
    }

    return ret;
}

static int parseDectSupportStr(char *supStr)
{
   int ret = 1;
   int sup;

   if(strlen(supStr) != 0) {
      sup = atoi(supStr);
      if(sup >= 0 && sup < g_numDectSupportOptions)
         ret = 0;
   }

   return ret;
}

static int parseMacAddrCount(char *ctStr)
{
    int count = atoi(ctStr);

    if (count >= 1 && count <= NVRAM_MAC_COUNT_MAX)
        return 0;
    else
        return 1;
}

static int parsePsiSize(char *tpStr)
{
    int psiSize = atoi(tpStr);

    if (psiSize >= 1 && psiSize <= NVRAM_MAX_PSI_SIZE)
        return 0;
    else
        return 1;
}

static int parseBackupPsi(char *tpStr)
{
    int enable = atoi(tpStr);

    if (enable == 0 || enable == 1)
        return 0;
    else
        return 1;
}

static int parseSyslogSize(char *tpStr)
{
    int syslogSize = atoi(tpStr);

    if (syslogSize >= 0 && syslogSize <= NVRAM_MAX_SYSLOG_SIZE)
        return 0;
    else
        return 1;
}


static int parseAuxFSPercent(char *ctStr)
{
    int percent = atoi(ctStr);
    
    if (percent >= 0 && percent <= MAX_AUXFS_PERCENT)
        return 0;
    else
        return 1;
}

#if defined(_BCM960333_) || defined(_BCM963268_) || defined(_BCM96838_) || defined(_BCM96848_) || defined(_BCM963381_) 
static int parseMainTp(char *tpStr)
{
    int tpNum = atoi(tpStr);

    if (tpNum == 0 || tpNum == 1)
        return 0;
    else
        return 1;
}
#endif

static int charIsHex(char ch)
{
    if (((ch >= '0') && (ch <= '9')) ||
        ((ch >= 'a') && (ch <= 'f')) ||
        ((ch >= 'A') && (ch <= 'F')))
        return 1;
    else
        return 0;
}

static int parseGponSN(char *snStr)
{
    int i;
    int ret = 0;

    if(strlen(snStr) == NVRAM_GPON_SERIAL_NUMBER_LEN-1) {
        for(i=4; i<NVRAM_GPON_SERIAL_NUMBER_LEN-1; ++i) {
            if(!charIsHex(snStr[i])) {
                ret = 1;
                break;
            }
        }
    }
    else {
        ret = 1;
    }

    return ret;
}

static int parseGponPW(char *pwStr)
{
    if(strlen(pwStr) == NVRAM_GPON_PASSWORD_LEN-1)
        return 0;
    else
        return 1;
}

#if !defined(_BCM960333_) && !defined(_BCM963268_) && !defined(_BCM963381_) && !defined(_BCM947189_)
static int parseParam1Size(char *ctStr)
{
    int percent = atoi(ctStr);
    
    if (percent >= 0 && percent <= MAX_RDP_PARAM1_SIZE)
        return 0;
    else
        return 1;
}

static int parseParam2Size(char *ctStr)
{
    int percent = atoi(ctStr);

    /* clear override bit for parsing size */
    if (percent & 0x80)
        percent &= 0x7f;

    if (percent >= MIN_RDP_PARAM2_SIZE && percent <= MAX_RDP_PARAM2_SIZE)
        return 0;
    else
        return 1;
}

static int parseDHDSize(char *ctStr)
{
	return 0;
}

#endif


static int parseWpsDevicePin(char *pinStr)
{
   unsigned char accum=0;
   unsigned char factor[NVRAM_WPS_DEVICE_PIN_LEN]={3,1,3,1,3,1,3,1};
   int i =0;
   
    /*Check Length*/
    if(strlen(pinStr) != NVRAM_WPS_DEVICE_PIN_LEN)
        return 1;

   /*valid checksum*/
   for ( i=0; i< NVRAM_WPS_DEVICE_PIN_LEN; i++ ) 
     accum += (pinStr[i]-'0')*factor[i];
   
   if ( (accum%10) ==0 )
       return 0;

   return 1;
}

static int __get_partition_size(char * param, int max_, int *size, char *sz_bits)
{
char *tmp_str=param;
int err=0;

   tmp_str = param; //gPartitionsParam[i].parameter;
   *sz_bits=0; // set this to default (MB)
   while((tmp_str-param) < max_ /*(MAX_PROMPT_LEN*/ && tmp_str[0] != '\0' && err != 1) {
      switch(tmp_str[0]) {
         case 'm':
         case 'M':
            *sz_bits=0;
            err=0x2;
            break;
         case 'g':
         case 'G':
            *sz_bits=1;
            err=0x2;
            break;
         case '0':case '1':case '2':case '3':case '4':
         case '5':case '6':case '7':case '8':case '9':
            //just continue;
            break;
         default:
            err=1;
            *sz_bits=0;
            break;
      }
      tmp_str++;
   }
   if(err != 1) {
      *size=atoi(param);
   }
   return err&0x1;
}

/*user friendly partition size */
static void __uf_partition_size(char *parameter, short max_, unsigned short size, unsigned short sz_bit )
{
char s;
   switch(sz_bit) {
      case 0:
         s='M';
         break;
      case 1: 
         s='G';
      default:
         s='E';
   }
if(s != 'E')
   sprintf(parameter, "%d%c",  size, s);

}
static int parsePartitionSizeStr(char *partSizeStr)
{
int ret = 0;
int size;
char sz_bits;

   if (strlen (partSizeStr) != 0) {
      ret=__get_partition_size(partSizeStr, MAX_PROMPT_LEN, &size, &sz_bits);
   }
   return ret;
}

int macNumToStr(unsigned char *macAddr, char *str)
{
   if (macAddr == NULL || str == NULL) 
       return 0;

   sprintf(str, "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
           macAddr[0], macAddr[1], macAddr[2], 
           macAddr[3], macAddr[4], macAddr[5]);
   return 1;
}

static int gponParamsInitialized(NVRAM_DATA *pNvramData)
{
    int i;
    int erased = 1;

    for(i=0; i<NVRAM_GPON_SERIAL_NUMBER_LEN-1; ++i) {
        if((pNvramData->gponSerialNumber[i] != (char)0xFF) &&
           (pNvramData->gponSerialNumber[i] != '\0')) {
            erased = 0;
            break;
        }
    }

    if(erased) {
        for(i=0; i<NVRAM_GPON_PASSWORD_LEN-1; ++i) {
            if((pNvramData->gponPassword[i] != (char)0xFF) &&
               (pNvramData->gponPassword[i] != '\0')) {
                erased = 0;
                break;
            }
        }
    }

    return (erased) ? 0 : 1;
}

static void getGponBoardParam(void)
{
    int erased;
    int i;
    int writeNvram = 0;
    erased = 1;
    for(i = 0; i < NVRAM_GPON_SERIAL_NUMBER_LEN-1; ++i) {
        if((NVRAM.gponSerialNumber[i] != (char)0xFF) &&
           (NVRAM.gponSerialNumber[i] != '\0')) {
            erased = 0;
            break;
        }
    }

    if(erased) {
      NVRAM_COPY_FIELD(gponSerialNumber, DEFAULT_GPON_SN, sizeof(DEFAULT_GPON_SN));
       writeNvram = 1;
    }

    erased = 1;
    for (i=0; i < NVRAM_GPON_PASSWORD_LEN-1; ++i) {
        if((NVRAM.gponPassword[i] != (char)0xFF) &&
           (NVRAM.gponPassword[i] != '\0')) {
            erased = 0;
            break;
        }
    }

    if(erased) {
	NVRAM_COPY_FIELD(gponPassword, DEFAULT_GPON_PW, sizeof(DEFAULT_GPON_PW));
        writeNvram = 1;
    }

    if(writeNvram) {
       NVRAM_UPDATE(NULL);
    }

    strcpy(gGponBoardParam[PARAM_IDX_GPON_SN].parameter, NVRAM.gponSerialNumber);
    strcpy(gGponBoardParam[PARAM_IDX_GPON_PW].parameter, NVRAM.gponPassword);
}


int setGponBoardParam(void)
{
    int ret = 0;
    getGponBoardParam();
    displayPromptUsage();
    if (processPrompt(gGponBoardParam, gNumGponBoardParams)) {

        // At least one field was changed
	NVRAM_COPY_FIELD(gponSerialNumber, gGponBoardParam[PARAM_IDX_GPON_SN].parameter,
			sizeof(NVRAM.gponSerialNumber));
	NVRAM_COPY_FIELD(gponPassword, gGponBoardParam[PARAM_IDX_GPON_PW].parameter,
			sizeof(NVRAM.gponPassword));
        // save the buf to nvram
	NVRAM_UPDATE(NULL);
        ret = 1;
    }
    return ret;
}

#if !defined(_BCM960333_) && !defined(_BCM963268_) && !defined(_BCM963381_) && !defined(_BCM947189_)
static void setDefaultAllocBoardParamNVRAM(void)
{
#ifdef DEFAULT_NVRAM_RDP_PARAM1
    NVRAM_SET(allocs.alloc_rdp.param1_size, unsigned char, DEFAULT_NVRAM_RDP_PARAM1);
#else
    NVRAM_SET(allocs.alloc_rdp.param1_size, unsigned char, 0);
#endif

#ifdef DEFAULT_NVRAM_RDP_PARAM2
    NVRAM_SET(allocs.alloc_rdp.param2_size, unsigned char, DEFAULT_NVRAM_RDP_PARAM2);
#else
    NVRAM_SET(allocs.alloc_rdp.param2_size, unsigned char, 0);
#endif

    NVRAM_SET(alloc_dhd.dhd_size[0], unsigned char, 0);
    NVRAM_SET(alloc_dhd.dhd_size[1], unsigned char, 0);
    NVRAM_SET(alloc_dhd.dhd_size[2], unsigned char, 0);

    if((flash_get_flash_type() == FLASH_IFC_NAND) || (flash_get_flash_type() == FLASH_IFC_SPINAND)) {
        NVRAM_SET(part_info[0].size, unsigned short,PARTI_INFO_FORMAT(20, 0));
        NVRAM_SET(part_info[1].size, unsigned short,PARTI_INFO_FORMAT(0, 0));
        NVRAM_SET(part_info[2].size, unsigned short,PARTI_INFO_FORMAT(0, 0));
        NVRAM_SET(part_info[3].size, unsigned short,PARTI_INFO_FORMAT((1 << (2+(flash_get_sector_size(0)>>18))), 0)); //minimum of 4MB
    }
    NVRAM_UPDATE(NULL);
}

static void prepareAllocDHDBoardParam(int index)
{
    int n;

    n = sprintf(gAllocBoardParam[PARAM_IDX_ALLOC_DHD1+index].parameter,"%d", NVRAM.alloc_dhd.dhd_size[index]&0x7f);
    if( NVRAM.alloc_dhd.dhd_size[index]&0x80 )
        sprintf(gAllocBoardParam[PARAM_IDX_ALLOC_DHD1+index].parameter+n, "(OVERRIDE)");
}

static void getAllocBoardParam(void)
{
    int i = 0;

    if ((NVRAM.allocs.alloc_rdp.param1_size > MAX_RDP_PARAM1_SIZE) ||
                (NVRAM.allocs.alloc_rdp.param2_size == 0xff) ||
                ((NVRAM.allocs.alloc_rdp.param2_size&0x7f) > MAX_RDP_PARAM2_SIZE)) {
#ifdef DEFAULT_NVRAM_RDP_PARAM1
        NVRAM_SET(allocs.alloc_rdp.param1_size, unsigned char, DEFAULT_NVRAM_RDP_PARAM1);
#else
        NVRAM_SET(allocs.alloc_rdp.param1_size, unsigned char, 0);
#endif

#ifdef DEFAULT_NVRAM_RDP_PARAM2
        NVRAM_SET(allocs.alloc_rdp.param2_size, unsigned char, DEFAULT_NVRAM_RDP_PARAM2);
#else
        NVRAM_SET(allocs.alloc_rdp.param2_size, unsigned char, 0);
#endif
			NVRAM_UPDATE(NULL);
    }
    else if(NVRAM.alloc_dhd.dhd_size[0] == 255 && NVRAM.alloc_dhd.dhd_size[1] == 255 &&
        NVRAM.alloc_dhd.dhd_size[2] == 255) {
        NVRAM_SET(alloc_dhd.dhd_size[0], unsigned char, 0);
        NVRAM_SET(alloc_dhd.dhd_size[1], unsigned char, 0);
        NVRAM_SET(alloc_dhd.dhd_size[2], unsigned char, 0);
	NVRAM_UPDATE(NULL);
    }

    sprintf(gAllocBoardParam[PARAM_IDX_ALLOC_RDP_PARAM1].parameter,"%d", NVRAM.allocs.alloc_rdp.param1_size);
    i = sprintf(gAllocBoardParam[PARAM_IDX_ALLOC_RDP_PARAM2].parameter,"%d", NVRAM.allocs.alloc_rdp.param2_size&0x7f);
    if (NVRAM.allocs.alloc_rdp.param2_size&0x80)
        sprintf(gAllocBoardParam[PARAM_IDX_ALLOC_RDP_PARAM2].parameter+i, "(OVERRIDE)");

    for( i = 0; i < 3; i++ )
        prepareAllocDHDBoardParam(i);
}
#endif


static int gWpsDevicePinParamsInitialized(const NVRAM_DATA *pNvramData)
{
    int i;

    for(i=0; i<NVRAM_WPS_DEVICE_PIN_LEN; ++i) {
        if(( (unsigned char)(pNvramData->wpsDevicePin[i])  > 0x39) ||
           ( (unsigned char)(pNvramData->wpsDevicePin[i])  < 0x30)  ) {
            return 0;
        }
    }

    return 1;
}

static void getWpsDevicePinBoardParam(void)
{
    if ( gWpsDevicePinParamsInitialized(NVRAM_RP) ) {
        memcpy(gWpsDevicePinBoardParam[PARAM_IDX_WPS_DEVICE_PIN].parameter, 
            NVRAM.wpsDevicePin, NVRAM_WPS_DEVICE_PIN_LEN);
        (gWpsDevicePinBoardParam[PARAM_IDX_WPS_DEVICE_PIN].parameter)[NVRAM_WPS_DEVICE_PIN_LEN] =
            '\0';
    }
    else {
        /*Set Default Device Pin*/
        /*memcpy(NVRAM.wpsDevicePin, DEFAULT_WPS_DEVICE_PIN, NVRAM_WPS_DEVICE_PIN_LEN);*/
	NVRAM_COPY_FIELD(wpsDevicePin, DEFAULT_WPS_DEVICE_PIN, NVRAM_WPS_DEVICE_PIN_LEN);
	NVRAM_UPDATE(NULL);
        memcpy(gWpsDevicePinBoardParam[PARAM_IDX_WPS_DEVICE_PIN].parameter, 
            NVRAM.wpsDevicePin,
            NVRAM_WPS_DEVICE_PIN_LEN);
        (gWpsDevicePinBoardParam[PARAM_IDX_WPS_DEVICE_PIN].parameter)[NVRAM_WPS_DEVICE_PIN_LEN] =
            '\0';
    }
}

int setWpsDevicePinBoardParam(void)
{
    int ret = 0;
    getWpsDevicePinBoardParam();
    displayPromptUsage();
    if (processPrompt(gWpsDevicePinBoardParam, gNumWpsDevicePinBoardParams)) {
	NVRAM_UPDATE_FIELD(wpsDevicePin, 
            gWpsDevicePinBoardParam[PARAM_IDX_WPS_DEVICE_PIN].parameter,
            NVRAM_WPS_DEVICE_PIN_LEN);
        
        ret = 1;
    }
    return ret;
}


static int getVoiceSupportStatus(NVRAM_DATA *pNvramData)
{
    int rc = 0;
    int dectType;

    if ( BpSetBoardId(pNvramData->szBoardId) == BP_SUCCESS ) {
        /* Retrieve voice daughtercard Ids */
        if(g_numVoiceBoardIdNames == 0)
           g_numVoiceBoardIdNames = BpGetNumVoiceBoardIds(pNvramData->szBoardId);
        
#if defined(_BCM96858_) || defined(_BCM94908_) || defined(_BCM96846_) || defined(_BCM96856_)
        unsigned int otp_val = 0;
        if( bcm_otp_is_pcm_disabled(&otp_val) == 0 && otp_val ) {
            /* PCM voice is disabled via OTP, clear voice related parameters */
            g_numVoiceBoardIdNames = 0;
        }
#endif  
         
        /* Enable/disable voice params */
        if ( ( BpGetVoipDspConfig( 0 ) == NULL ) || ( g_numVoiceBoardIdNames == 0 ) ) {
           gVoiceBoardParam[PARAM_IDX_VOICE_BOARD_NAME].enabled = FALSE;
        } else {
           gVoiceBoardParam[PARAM_IDX_VOICE_BOARD_NAME].enabled = TRUE;
           rc = 1;
        }

        /* Get DECT support */
        if( rc
#if defined(OTP_GET_USER_BIT) && defined(OTP_DECT_DISABLE)
            && !OTP_GET_USER_BIT(OTP_DECT_DISABLE)
#endif
            && (dectType = BpGetVoiceDectType(pNvramData->szBoardId)) != BP_VOICE_NO_DECT ) {
            g_numDectSupportOptions = dectType + 1;
            gVoiceBoardParam[PARAM_IDX_DECT_SUPPORT].enabled = TRUE;
        } else {
            gVoiceBoardParam[PARAM_IDX_DECT_SUPPORT].enabled = FALSE;
        }
#if defined(OTP_GET_USER_BIT) && defined(OTP_DECT_DISABLE)
       if( OTP_GET_USER_BIT(OTP_DECT_DISABLE) && (NVRAM.ulBoardStuffOption & DECT_SUPPORT_MASK))
       {
          NVRAM_SET(ulBoardStuffOption,unsigned int, (NVRAM.ulBoardStuffOption & ~DECT_SUPPORT_MASK));
          NVRAM_UPDATE(NULL);
       }
#endif
    }

    return rc;
}

static void getVoiceBoardParam(void)
{
    
    char *ptr;
    char tmp[10];
    if(g_numVoiceBoardIdNames == 0)
       g_numVoiceBoardIdNames = BpGetNumVoiceBoardIds(NVRAM.szBoardId);
    ptr = strchr(gVoiceBoardParam[PARAM_IDX_VOICE_BOARD_NAME].promptName, '#');
    if (ptr != NULL) {
        sprintf(tmp, "%d)", g_numVoiceBoardIdNames - 1);
        memcpy(ptr, tmp, strlen(tmp));
    }

    strcpy(gVoiceBoardParam[PARAM_IDX_VOICE_BOARD_NAME].parameter, NVRAM.szVoiceBoardId);

    // DECT support status
    if(gVoiceBoardParam[PARAM_IDX_DECT_SUPPORT].enabled == TRUE) {
       ptr = strchr(gVoiceBoardParam[PARAM_IDX_DECT_SUPPORT].promptName, '#');
       if(ptr != NULL) {
          sprintf(tmp, "%d)", g_numDectSupportOptions - 1);
          memcpy(ptr, tmp, strlen(tmp));
       }
    }

    sprintf(gVoiceBoardParam[PARAM_IDX_DECT_SUPPORT].parameter, "%d",
            NVRAM.ulBoardStuffOption & DECT_SUPPORT_MASK);
}

int setVoiceBoardParam(void)
{
    char dectSupportPrompt[200];
    int i;
    char *savedVoiceBoardIdPromptPtr;
    char *savedDectPromptPtr;
    int ret = 0;
    char *voiceBoardIdPrompt = KMALLOC(1024, sizeof(void*));
    if (!voiceBoardIdPrompt) {
        return ret;
    }
    memset(voiceBoardIdPrompt, 1024, '\0');
    getVoiceBoardParam();
    // Create prompt string with voice board ID name selection
    for (i = 0; i < g_numVoiceBoardIdNames; i++) {
       char *s = BpGetVoiceBoardIdNameByIndex(i, NVRAM.szBoardId);
       if (s) {
           printf("%-17s-- %d\n", s, i);
       }
    }
    strcpy (voiceBoardIdPrompt, gVoiceBoardParam[PARAM_IDX_VOICE_BOARD_NAME].promptName);

    // Save existing prompt string
    savedVoiceBoardIdPromptPtr = gVoiceBoardParam[PARAM_IDX_VOICE_BOARD_NAME].promptName;
    // Set newly created prompt string
    gVoiceBoardParam[PARAM_IDX_VOICE_BOARD_NAME].promptName = voiceBoardIdPrompt;

    // Convert board ID string to numeric value
    for (i = 0; i < g_numVoiceBoardIdNames; i++) {
       if (!strcmp(gVoiceBoardParam[PARAM_IDX_VOICE_BOARD_NAME].parameter, BpGetVoiceBoardIdNameByIndex(i, NVRAM.szBoardId))) {
          sprintf(gVoiceBoardParam[PARAM_IDX_VOICE_BOARD_NAME].parameter, "%d", i);
       }
    }

    // Save existing DECT prompt and create list of supported DECT options
    savedDectPromptPtr = gVoiceBoardParam[PARAM_IDX_DECT_SUPPORT].promptName;
    sprintf(dectSupportPrompt, "\n");
    for(i = 0; i < g_numDectSupportOptions; i++) {
       sprintf(&dectSupportPrompt[strlen(dectSupportPrompt)],
               "%-17s-- %d\n", g_dectSupportOptions[i], i);
    }
    sprintf(&dectSupportPrompt[strlen(dectSupportPrompt)], "%s", savedDectPromptPtr);
    gVoiceBoardParam[PARAM_IDX_DECT_SUPPORT].promptName = dectSupportPrompt;

    displayPromptUsage();
    if (processPrompt(gVoiceBoardParam, gNumVoiceBoardParams)) {
        // If any fields were changed, save the new value in nvram
        i = atoi(gVoiceBoardParam[PARAM_IDX_VOICE_BOARD_NAME].parameter);
	NVRAM_COPY_FIELD(szVoiceBoardId, BpGetVoiceBoardIdNameByIndex(i, NVRAM.szBoardId), NVRAM_BOARD_ID_STRING_LEN);
        if(gVoiceBoardParam[PARAM_IDX_DECT_SUPPORT].enabled == TRUE) {
	   NVRAM_SET(ulBoardStuffOption,unsigned int,
		     ((NVRAM.ulBoardStuffOption & ~DECT_SUPPORT_MASK) |
		      ((unsigned long)(atoi(gVoiceBoardParam[PARAM_IDX_DECT_SUPPORT].parameter)) & DECT_SUPPORT_MASK)));
        }
        // save the buf to nvram
	NVRAM_UPDATE(NULL);
        ret = 1;
    }

    // Restore saved prompts
    // Convert numeric value of voice board ID to string
    gVoiceBoardParam[PARAM_IDX_VOICE_BOARD_NAME].promptName = savedVoiceBoardIdPromptPtr;
    gVoiceBoardParam[PARAM_IDX_DECT_SUPPORT].promptName = savedDectPromptPtr;
    i = atoi(gVoiceBoardParam[PARAM_IDX_VOICE_BOARD_NAME].parameter);
    strcpy(gVoiceBoardParam[PARAM_IDX_VOICE_BOARD_NAME].parameter, BpGetVoiceBoardIdNameByIndex(i, NVRAM.szBoardId));
    KFREE(voiceBoardIdPrompt);
    return ret;
}


//
// getBoardParam:  convert the board param data and put them in the gBoardParam struct
//
int getBoardParam(void)
{
    NVRAM_DATA *nvramData;
    int ret = 0;
    
    nvramData = KMALLOC(sizeof(NVRAM_DATA),sizeof(void*));
    if (!nvramData) {
        return ret;
    }
    
    if (g_numBoardIdNames == 0)
        g_numBoardIdNames = BPGetNumBoardIds();

#if (SKIP_FLASH!=0)
    {
        printf("*** set default values without writing to NVRAM ***\n");
        // Set default values
        nvramData->ulVersion = NVRAM_VERSION_NUMBER;
        memset(nvramData->szBoardId,0x0,15 );
        strcpy(nvramData->szBoardId, BpGetBoardIdNameByIndex(0));
        nvramData->ulNumMacAddrs = DEFAULT_MAC_NUM;
        parsehwaddr(DEFAULT_BOARD_MAC, nvramData->ucaBaseMacAddr);
        nvramData->ulMainTpNum = DEFAULT_TP_NUM;
        nvramData->szVoiceBoardId[0] = '\0';
        nvramData->ulPsiSize = DEFAULT_PSI_SIZE;
        nvramData->backupPsi = 0;
        nvramData->ulSyslogSize = 0;
        nvramData->ucAuxFSPercent = DEFAULT_AUXFS_PERCENT;
        nvramData->allocs.reserved = 0;
#if !defined(_BCM960333_) && !defined(_BCM963268_) && !defined(_BCM963381_) && !defined(_BCM947189_)
        nvramData->allocs.alloc_rdp.param1_size = 0;
        nvramData->allocs.alloc_rdp.param2_size = 0;
#endif

        strcpy(gBoardParam[PARAM_IDX_BOARD_NAME].parameter, nvramData->szBoardId);
        sprintf(gBoardParam[PARAM_IDX_NUM_MAC_ADDR].parameter, "%d", nvramData->ulNumMacAddrs);
        macNumToStr(nvramData->ucaBaseMacAddr, gBoardParam[PARAM_IDX_BASE_MAC_ADDR].parameter);  
        sprintf(gBoardParam[PARAM_IDX_PSI_SIZE].parameter, "%d", nvramData->ulPsiSize);
        sprintf(gBoardParam[PARAM_IDX_ENABLE_BACKUP_PSI].parameter, "%d", nvramData->backupPsi);
        sprintf(gBoardParam[PARAM_IDX_SYSLOG_SIZE].parameter, "%d", nvramData->ulSyslogSize);
        sprintf(gBoardParam[PARAM_IDX_AUXFS_PERCENT].parameter, "%d", nvramData->ucAuxFSPercent);
        sprintf(gBoardParam[PARAM_IDX_MAIN_THREAD_NUM].parameter, "%d", nvramData->ulMainTpNum);
#if !defined(_BCM960333_) && !defined(_BCM963268_) && !defined(_BCM963381_) && !defined(_BCM947189_)
        sprintf(gAllocBoardParam[PARAM_IDX_ALLOC_RDP_PARAM1].parameter,"%d", nvramData->allocs.alloc_rdp.param1_size);
        sprintf(gAllocBoardParam[PARAM_IDX_ALLOC_RDP_PARAM2].parameter,"%d", nvramData->allocs.alloc_rdp.param2_size);

        sprintf(gAllocBoardParam[PARAM_IDX_ALLOC_DHD1].parameter,"%d", nvramData->alloc_dhd.dhd_size[0]);
        sprintf(gAllocBoardParam[PARAM_IDX_ALLOC_DHD2].parameter,"%d", nvramData->alloc_dhd.dhd_size[1]);
        sprintf(gAllocBoardParam[PARAM_IDX_ALLOC_DHD3].parameter,"%d", nvramData->alloc_dhd.dhd_size[2]);

#endif
        BpSetBoardId(nvramData->szBoardId);
        KFREE(nvramData);
        return ret;
    }
#endif
    NVRAM_COPY_TO(nvramData);
    if(!gGponParamsInitialized) {
        gGponParamsInitialized = gponParamsInitialized(nvramData);
    }

    /*WPS Device Pin Initialized?*/
    if(!gWpsDevicePinInitialized) {
        gWpsDevicePinInitialized = gWpsDevicePinParamsInitialized(nvramData);
    }

    if (nvramData->ulVersion == -1) {
        printf("*** default values ***\n");
        // Set default values
        nvramData->ulVersion = NVRAM_VERSION_NUMBER;
        nvramData->szBoardId[0] = '\0';
        nvramData->ulNumMacAddrs = DEFAULT_MAC_NUM;
        parsehwaddr(DEFAULT_BOARD_MAC, nvramData->ucaBaseMacAddr);
        nvramData->ulMainTpNum = DEFAULT_TP_NUM;
        nvramData->szVoiceBoardId[0] = '\0';
        nvramData->ulPsiSize = DEFAULT_PSI_SIZE;
        nvramData->backupPsi = 0;
        nvramData->ulSyslogSize = 0;
        nvramData->ucAuxFSPercent = DEFAULT_AUXFS_PERCENT;
        nvramData->allocs.reserved = 0;
        nvramData->wlanParams[NVRAM_WLAN_PARAMS_LEN-1]= DEFAULT_WLAN_DEVICE_FEATURE; /* set default wlan feature when nvram erased */
	NVRAM_UPDATE(nvramData);
#if !defined(_BCM960333_) && !defined(_BCM963268_) && !defined(_BCM963381_) && !defined(_BCM947189_)
        setDefaultAllocBoardParamNVRAM();
#endif
    }
    else if (nvramData->ulVersion != NVRAM_VERSION_NUMBER) {
        // When upgrading from older bootloader initialize new fields
        printf("*** Upgrading NVRAM (version %d to version %d) ***\n\n",
               nvramData->ulVersion, NVRAM_VERSION_NUMBER);
        nvramData->ulVersion = NVRAM_VERSION_NUMBER;
        if (nvramData->ulMainTpNum == -1)
            nvramData->ulMainTpNum = DEFAULT_TP_NUM;
        if ((nvramData->ulPsiSize == -1) || (nvramData->ulPsiSize == 0))
            nvramData->ulPsiSize = DEFAULT_PSI_SIZE;
        nvramData->szVoiceBoardId[0] = '\0';
        if (nvramData->backupPsi == -1)
           nvramData->backupPsi = 0;
        if (nvramData->ulSyslogSize == -1)
           nvramData->ulSyslogSize = 0;
        if ((int)nvramData->ucAuxFSPercent > MAX_AUXFS_PERCENT)
           nvramData->ucAuxFSPercent = DEFAULT_AUXFS_PERCENT;
	NVRAM_UPDATE(nvramData);
#if !defined(_BCM960333_) && !defined(_BCM963268_) && !defined(_BCM963381_) && !defined(_BCM947189_)
        setDefaultAllocBoardParamNVRAM();
#endif
        ret = 1;
    }

    /* Check voice related parameters */
    gVoiceSupported = getVoiceSupportStatus(nvramData);
    if( !gVoiceSupported && strlen(NVRAM.szVoiceBoardId) )
    {
        /* Clear stale voice params if no voice functionality in system */
        NVRAM_SET(szVoiceBoardId[0], char, '\0');
        NVRAM_UPDATE(NULL);
    }

    // When backupPsi and syslog were introduced, the NVRAM version number
    // was not bumped up.  So convert -1 (unitialized) to 0 so it looks better.
    if (nvramData->backupPsi == -1)
       nvramData->backupPsi = 0;
    if (nvramData->ulSyslogSize == -1)
       nvramData->ulSyslogSize = 0;
    
    strcpy(gBoardParam[PARAM_IDX_BOARD_NAME].parameter,nvramData->szBoardId);
    sprintf(gBoardParam[PARAM_IDX_NUM_MAC_ADDR].parameter, "%d", nvramData->ulNumMacAddrs);
    macNumToStr(nvramData->ucaBaseMacAddr, gBoardParam[PARAM_IDX_BASE_MAC_ADDR].parameter);  
    sprintf(gBoardParam[PARAM_IDX_PSI_SIZE].parameter, "%d", nvramData->ulPsiSize);
    sprintf(gBoardParam[PARAM_IDX_ENABLE_BACKUP_PSI].parameter, "%d", nvramData->backupPsi);
    sprintf(gBoardParam[PARAM_IDX_SYSLOG_SIZE].parameter, "%d", nvramData->ulSyslogSize);
    sprintf(gBoardParam[PARAM_IDX_AUXFS_PERCENT].parameter, "%d", nvramData->ucAuxFSPercent);
    sprintf(gBoardParam[PARAM_IDX_MAIN_THREAD_NUM].parameter, "%d", nvramData->ulMainTpNum);

#if !defined(_BCM960333_) && !defined(_BCM963268_) && !defined(_BCM963381_) && !defined(_BCM947189_)
    getAllocBoardParam();
#endif
    KFREE(nvramData);
    return ret;
}

//
// setPartitionSizes: Set extra paritition sizes
//
int setPartitionSizes(void)
{
   int ret = 0;
#if (INC_NAND_FLASH_DRIVER==1)
   int i;
   int size;
   char sz_bit;
   char  partition_changed_flag[BCM_MAX_EXTRA_PARTITIONS];
   displayPromptUsage();

   for(i=0;i<gNumPartitionsParam;i++) {
#if defined(_BCM963158_)
     // Force MISC1 to be 20MB
     if(i==0 && NVRAM.part_info[i].size == 0)
         __uf_partition_size(gPartitionsParam[i].parameter, MAX_PROMPT_LEN,
            20, 0);
     else
#endif
          __uf_partition_size(gPartitionsParam[i].parameter, MAX_PROMPT_LEN,
            PARTI_INFO_SIZE(NVRAM.part_info[i].size), PARTI_INFO_SIZE_BITS(NVRAM.part_info[i].size));
   }

   memset(partition_changed_flag, '\0', sizeof(partition_changed_flag));
   if (processPrompt(gPartitionsParam, gNumPartitionsParam)) {
      for(i=0;i<gNumPartitionsParam;i++) {
         //convert this to sz_bits and size
         if(__get_partition_size(gPartitionsParam[i].parameter, MAX_PROMPT_LEN, &size, &sz_bit) == 0) {
            // allow zero sized misc partitions other than DATA
            if(i != 3 || (i == 3 &&  !(size == 0 && sz_bit == 0)))
            {
               //note down if the parition size changed
               if(PARTI_INFO_FORMAT(size, sz_bit) != NVRAM_RP->part_info[i].size)
               {
                   partition_changed_flag[i]=1;   
               }
               NVRAM_SET(part_info[i].size, unsigned short,PARTI_INFO_FORMAT(size, sz_bit));
            }
         }
         else {
            ret=1;
            break;
         }
      }


      if(ret == 0) {
	 NVRAM_UPDATE(NULL);
         /* Only print flash information if booting from NAND */
         if((flash_get_flash_type() == FLASH_IFC_NAND) || (flash_get_flash_type() == FLASH_IFC_SPINAND)) {
	   if (!validateNandPartTbl(1, 0)) {
	       //NVRAM_UPDATE(NULL);
	   }
           //don't need to erase data partition again
           for (i=0;i<BCM_MAX_EXTRA_PARTITIONS-1;i++)
           {
               // erase only if force command turned on 
               if(!g_force_mode && partition_changed_flag[i] == 1)
                   erase_misc_partition(i, NVRAM_RP);
           }
         }
      }
   }
#endif
   return ret;
}
 
// 
// check if the board id is compatible with the chip id 
// return true if there is no compatible chip id defined in board parameter
// or any of compatible chip id(s) matches the hardware chip id  
//
static int checkCompatBoardId(uint32_t chipId)
{
    uint32_t compatId;
    int rc = 0, found = 0, match = 0;
    void* token = NULL;

    while( BpEnumCompatChipId(&token, &compatId) == BP_SUCCESS ) {
        found = 1;
        if( chipId == compatId )
            match = 1;
    }

    if( found == 0 || match == 1 )
        rc = 1;

    return rc;
}


//
// setBoardParam: Set the board Id string, mac addresses, psi size, etc...
//
#define DEFAULT_BOARD_ID_PROMPT_LEN 1024
#define INT_TO_S_LEN  10 
int setBoardParam(void)
{
    char *boardIdPrompt;
    NVRAM_DATA *nvramData;
    int i;
    char *savedBoardIdPromptPtr;
    int ret = 0;
    int ulPsiSize, ucAuxFSPercent;
    int changed = 0;
    uint32_t chipId = UtilGetChipId();

    if (getBoardParam()) {
        /* New NVRAM version */
        return ret;
    }
    boardIdPrompt = KMALLOC(DEFAULT_BOARD_ID_PROMPT_LEN, sizeof(void*));
    if (!boardIdPrompt) {
        return ret; 
    }
    memset(boardIdPrompt, DEFAULT_BOARD_ID_PROMPT_LEN, '\0');

    nvramData = KMALLOC(sizeof(NVRAM_DATA),sizeof(void*));
    if (!nvramData) {
        KFREE(boardIdPrompt); 
        return ret;
    }
    NVRAM_COPY_TO(nvramData);
    
    ulPsiSize = nvramData->ulPsiSize;
    ucAuxFSPercent = nvramData->ucAuxFSPercent;

    displayPromptUsage();
    // Create prompt string with board ID name selection
    for (i = 0; i < g_numBoardIdNames; i++) {
        int display = 0;
        char *cm = NULL;
        char *bid = BpGetBoardIdNameByIndex(i);

        BpSetBoardId(bid);
        if (BpGetComment(&cm) != BP_SUCCESS) {
            cm = "";
        }
        display = checkCompatBoardId(chipId);
        if (display) {
            printf ("%-27s%s%------- %2d\n", bid, cm, i);      
        }
    }
    strcpy (boardIdPrompt, gBoardParam[PARAM_IDX_BOARD_NAME].promptName);

    // Save existing prompt string
    savedBoardIdPromptPtr = gBoardParam[PARAM_IDX_BOARD_NAME].promptName;
    // Set newly created prompt string
    gBoardParam[PARAM_IDX_BOARD_NAME].promptName = boardIdPrompt;

    // Convert board ID string to numeric value
    for (i = 0; i < g_numBoardIdNames; i++) {
        if (!strcmp(gBoardParam[PARAM_IDX_BOARD_NAME].parameter, BpGetBoardIdNameByIndex( i ))) {
            sprintf(gBoardParam[PARAM_IDX_BOARD_NAME].parameter, "%d", i);
        }
    }

    changed =  processPrompt(gBoardParam, gNumBoardParams);
#if defined(_BCM963138_) || defined(_BCM963148_) || defined(_BCM94908_) || defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM947189_)
    /* make sure we verify memcfg with current select board */
    i = atoi(gBoardParam[PARAM_IDX_BOARD_NAME].parameter);
    BpSetBoardId(BpGetBoardIdNameByIndex( i ));
    {
       unsigned int memcfg;
       if (validateMemoryConfig(nvramData, &memcfg)) {
           nvramData->ulMemoryConfig = memcfg; 
           printf("Memory Configuration Changed -- REBOOT NEEDED\n");
           changed = 1;
       }
    }
#endif
    if (changed) {
        // At least one field was changed
        nvramData->ulVersion = NVRAM_VERSION_NUMBER;

        // Convert numeric value of board ID to string
        i = atoi(gBoardParam[PARAM_IDX_BOARD_NAME].parameter);
        strcpy(nvramData->szBoardId, BpGetBoardIdNameByIndex( i ));

        nvramData->ulNumMacAddrs = atoi(gBoardParam[PARAM_IDX_NUM_MAC_ADDR].parameter);
        parsehwaddr(gBoardParam[PARAM_IDX_BASE_MAC_ADDR].parameter, nvramData->ucaBaseMacAddr);
        nvramData->ulPsiSize = atoi(gBoardParam[PARAM_IDX_PSI_SIZE].parameter);
        nvramData->backupPsi = atoi(gBoardParam[PARAM_IDX_ENABLE_BACKUP_PSI].parameter);
        nvramData->ulSyslogSize = atoi(gBoardParam[PARAM_IDX_SYSLOG_SIZE].parameter);
        nvramData->ucAuxFSPercent = (unsigned char)
                   atoi(gBoardParam[PARAM_IDX_AUXFS_PERCENT].parameter);

        nvramData->ulMainTpNum = atoi(gBoardParam[PARAM_IDX_MAIN_THREAD_NUM].parameter);

        // save the buf to nvram
	NVRAM_UPDATE(nvramData);
        ret = 1;
    }

#if !defined(_BCM960333_) && !defined(_BCM963268_) && !defined(_BCM963381_) && !defined(_BCM947189_)
    if (processPrompt(gAllocBoardParam, gNumAllocBoardParams)) {
        // At least one field was changed
        nvramData->ulVersion = NVRAM_VERSION_NUMBER;
        nvramData->allocs.alloc_rdp.param1_size = atoi(gAllocBoardParam[PARAM_IDX_ALLOC_RDP_PARAM1].parameter);
        nvramData->allocs.alloc_rdp.param2_size = atoi(gAllocBoardParam[PARAM_IDX_ALLOC_RDP_PARAM2].parameter);

        nvramData->alloc_dhd.dhd_size[0] = atoi(gAllocBoardParam[PARAM_IDX_ALLOC_DHD1].parameter);
        nvramData->alloc_dhd.dhd_size[1] = atoi(gAllocBoardParam[PARAM_IDX_ALLOC_DHD2].parameter);
        nvramData->alloc_dhd.dhd_size[2] = atoi(gAllocBoardParam[PARAM_IDX_ALLOC_DHD3].parameter);

        // save the buf to nvram
	NVRAM_UPDATE(nvramData);
        ret = 1;
    }
#endif

    // restore gBoardParam
    // Convert numeric value of board ID to string
    gBoardParam[PARAM_IDX_BOARD_NAME].promptName = savedBoardIdPromptPtr;
    i = atoi(gBoardParam[PARAM_IDX_BOARD_NAME].parameter);
    strcpy(gBoardParam[PARAM_IDX_BOARD_NAME].parameter, BpGetBoardIdNameByIndex( i ));

    /* Set voice related parameters */
    gVoiceSupported = getVoiceSupportStatus(nvramData);
    if(gVoiceSupported) {
        printf("\n");
        ret += setVoiceBoardParam();
    } else {
        /* Clear main voice params if no voice functionality in system */
        if( strlen(NVRAM.szVoiceBoardId) ) {
            NVRAM_SET(szVoiceBoardId[0], char, '\0');
            NVRAM_UPDATE(NULL);
        }
    }

    if(gGponParamsInitialized) {
        printf("\n");
        ret += setGponBoardParam();
    }

    if(gWpsDevicePinInitialized) {
        printf("\n");
        ret += setWpsDevicePinBoardParam();
    }

    printf("\n");
    ret += setWlanDeviceFeatureBoardParam();

    /* Only print flash information if booting from NAND */
    if((flash_get_flash_type() == FLASH_IFC_NAND) || (flash_get_flash_type() == FLASH_IFC_SPINAND)) {
        ret+= setPartitionSizes();
    }

    // Check for changes in flash layout, ignoring any AUFS percentage changes
    // from 0xFF (uninitialized) to 0% (default).
    if ( (ulPsiSize != nvramData->ulPsiSize)
      || ( (ucAuxFSPercent != nvramData->ucAuxFSPercent) && 
           !( (ucAuxFSPercent > MAX_AUXFS_PERCENT) && 
              (nvramData->ucAuxFSPercent == DEFAULT_AUXFS_PERCENT) ) ) )
    {
#if (INC_NAND_FLASH_DRIVER==0)
       printf("Flash layout changed. Need to erase all flash except bootrom\n");
       ui_docommands("e a");
#endif
    }
    
    if (nvramData) {
        KFREE(nvramData);
    }
    if (boardIdPrompt)
        KFREE(boardIdPrompt); 
    return ret;
}

void displayBoardParam(void)
{
    int i;
    getBoardParam();
    for (i = 0; i < gNumBoardParams; i++) {
        if( gBoardParam[i].enabled ){
            printf("%s %s  \n", gBoardParam[i].promptName, gBoardParam[i].parameter);
            /* print out warning if any of two AFE ids is overridden just below the board id */
            if( i == PARAM_IDX_BOARD_NAME ){
                if( ((NVRAM.afeId[0] != 0x0) && (NVRAM.afeId[0] != 0xffffffff)) ||
                        ((NVRAM.afeId[1] != 0x0) && (NVRAM.afeId[1] != 0xffffffff)) ){
                    printf("Primary AFE ID OVERRIDE           : 0x%08x\n", NVRAM.afeId[0]);
                    printf("Bonding AFE ID OVERRIDE           : 0x%08x\n", NVRAM.afeId[1]);
                }
#if defined(_BCM963138_) || defined(_BCM963148_) || defined(_BCM94908_) || defined(_BCM96848_) || defined(_BCM963158_) || defined(_BCM96846_)
                if (NVRAM.ulMemoryConfig & BP_DDR_CONFIG_OVERRIDE) {
                    printf("MEMORY CONFIG OVERRIDE            : 0x%08x\n", NVRAM.ulMemoryConfig);
                }
#endif
            }
        }
    }

    if(gGponParamsInitialized) {
        getGponBoardParam();

        for (i = 0; i < gNumGponBoardParams; i++)
            if( gGponBoardParam[i].enabled )
                printf("%s \"%s\"  \n", gGponBoardParam[i].promptName, gGponBoardParam[i].parameter);
    }

#if !defined(_BCM960333_) && !defined(_BCM963268_) && !defined(_BCM963381_) && !defined(_BCM947189_)
    for (i = 0; i < gNumAllocBoardParams; i++)
        if( gAllocBoardParam[i].enabled )
            printf("%s %s  \n", gAllocBoardParam[i].promptName, gAllocBoardParam[i].parameter);
#endif

    /*Show WPS Device PIN */
    if(gWpsDevicePinInitialized) {
         getWpsDevicePinBoardParam();
    if ( gWpsDevicePinBoardParam[PARAM_IDX_WPS_DEVICE_PIN].enabled) 
         printf("%s \"%s\"  \n", gWpsDevicePinBoardParam[PARAM_IDX_WPS_DEVICE_PIN].promptName,  \
                                            gWpsDevicePinBoardParam[PARAM_IDX_WPS_DEVICE_PIN].parameter);
    }
    getWlanDeviceFeatureBoardParam();
    if ( gWlanDeviceFeatureBoardParam[PARAM_IDX_WLAN_DEVICE_FEATURE].enabled) 
         printf("%s %s  \n", gWlanDeviceFeatureBoardParam[PARAM_IDX_WLAN_DEVICE_FEATURE].promptName,  \
                                            gWlanDeviceFeatureBoardParam[PARAM_IDX_WLAN_DEVICE_FEATURE].parameter);

    if(gVoiceSupported) {
        getVoiceBoardParam();

        for (i = 0; i < gNumVoiceBoardParams; i++)
            if( gVoiceBoardParam[i].enabled )
                printf("%s %s  \n", gVoiceBoardParam[i].promptName, gVoiceBoardParam[i].parameter);
    }

    /* Only print flash information if booting from NAND */
    if((flash_get_flash_type() == FLASH_IFC_NAND) || (flash_get_flash_type() == FLASH_IFC_SPINAND)) {
        for(i=0;i<BCM_MAX_EXTRA_PARTITIONS-1;i++) {
            __uf_partition_size(gPartitionsParam[i].parameter,MAX_PROMPT_LEN,
                PARTI_INFO_SIZE(NVRAM.part_info[i].size), PARTI_INFO_SIZE_BITS(NVRAM.part_info[i].size));
            printf("%s %s  \n", gPartitionsParam[i].promptName, gPartitionsParam[i].parameter);
        }
        __uf_partition_size(gPartitionsParam[3].parameter,MAX_PROMPT_LEN,
            PARTI_INFO_SIZE(NVRAM.part_info[3].size), PARTI_INFO_SIZE_BITS(NVRAM.part_info[3].size));
        if(NVRAM.part_info[3].size == 0xffff ) {
            printf("%s %dMB  \n", gPartitionsParam[3].promptName, min_data_partition_size_kb()/1024);
        }
        else {
            printf("%s %s \n", gPartitionsParam[3].promptName, gPartitionsParam[i].parameter);
        }
   }
}
