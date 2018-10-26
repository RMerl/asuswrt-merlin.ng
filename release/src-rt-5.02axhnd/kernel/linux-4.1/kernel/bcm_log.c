#if defined(CONFIG_BCM_KF_LOG)
/*
* <:copyright-BRCM:2010:DUAL/GPL:standard
* 
*    Copyright (c) 2010 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
:>
*/

#include <asm/uaccess.h> /*copy_from_user*/
#include <linux/module.h>

#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include <linux/bcm_log.h>



#define VERSION     "0.1"
#define VER_STR     "v" VERSION

#define PROC_ENTRY_NAME "bcmlog"

#if defined(BCM_DATADUMP_SUPPORTED)
#define MAX_NUM_DATADUMP_IDS 20
#define MAX_NUM_QIDS 10
#define PRINTBUF_SIZE 0x10000
#endif

#define BCM_LOG_CHECK_LOG_ID(_logId)                                    \
    BCM_ASSERT((_logId) >= 0 && (_logId) < BCM_LOG_ID_MAX);

#define BCM_LOG_CHECK_LOG_LEVEL(_logLevel)                              \
    BCM_ASSERT((_logLevel) >= 0 && (_logLevel) < BCM_LOG_LEVEL_MAX);

#define BCM_LOG_CHECK_DD_LEVEL(_ddLevel)                                \
    BCM_ASSERT((_ddLevel) >= 0 && (_ddLevel) < BCM_LOG_DD_MAX);

static bcmLogLevel_t globalLogLevel = BCM_LOG_LEVEL_DEBUG;

static bcmLogModuleInfo_t modInfo[] = BCM_LOG_MODULE_INFO;

#if defined(BCM_DATADUMP_SUPPORTED)
static bcmLogDataDumpLevel_t globalDataDumpLevel = BCM_LOG_DD_IMPORTANT;
static Bcm_DataDumpPrintFunc *printFuns[MAX_NUM_DATADUMP_IDS*MAX_NUM_QIDS];
static char buf[PRINTBUF_SIZE];
static const char* qids[MAX_NUM_QIDS];
#endif

static bcmFun_t* funTable[BCM_FUN_ID_MAX];
static bcmLogSpiCallbacks_t spiFns = { .reserveSlave      = NULL,
                                       .syncTrans         = NULL,
                                       .kerSysSlaveWrite  = NULL,
                                       .kerSysSlaveRead   = NULL,
                                       .bpGet6829PortInfo = NULL};
static int spiDev = 0;

/**
 ** Local Functions
 **/

static char char2num(char in) {
    char out;

    if ((in >= '0') && (in <= '9'))
        out = (in - '0');
    else if ((in >= 'a') && (in <= 'f'))
        out = (in - 'a') + 10;
    else if ((in >= 'A') && (in <= 'F'))
        out = (in - 'A') + 10;
    else
        out = 0;

    return out;
}

static int ishex(char *str) {
  return str && (str[0]=='0') && (str[1]=='x');
}

static uint32_t str2val(char *str) {
    int i;
    int value;
    int base = ishex(str) ? 16 : 10;

    if (str == NULL) return(0);

    for (i=0,value=0; str[i]; i++) {
        value = (value*base) + char2num(str[i]);
    }

    return(value);
}

#define UNIT_SIZE_BYTES 1
#define UNIT_SIZE_HALFWORDS 2
#define UNIT_SIZE_WORDS 4
#define UNIT_SIZE_DWORDS 8

static void setMem(void *start, uint32_t val, uint32_t len, uint32_t unitSize) {
  int i;
  uint8_t* curPtr = start;

  BCM_ASSERT((unitSize == UNIT_SIZE_BYTES) ||
             (unitSize == UNIT_SIZE_HALFWORDS) ||
             (unitSize == UNIT_SIZE_WORDS) ||
             (unitSize == UNIT_SIZE_DWORDS));
  BCM_ASSERT(((uintptr_t)start&~(unitSize-1)) == (uintptr_t)start);

  for (i = 0; i < len; ++i) {
      switch (unitSize) {
        case UNIT_SIZE_BYTES:
        {
          *curPtr = (uint8_t)val;
          break;
        }
        case UNIT_SIZE_HALFWORDS:
        {
          uint16_t *cur16Ptr = (uint16_t*)curPtr;
          *cur16Ptr = (uint16_t)val;
          break;
        }
        case UNIT_SIZE_WORDS:
        {
          uint32_t *cur32Ptr = (uint32_t*)curPtr;
          *cur32Ptr = (uint32_t)val;
          break;
        }
#if defined (CONFIG_64BIT)
        case UNIT_SIZE_DWORDS:
        {
          uint64_t *cur64Ptr = (uint64_t*)curPtr;
          *cur64Ptr = (uint64_t)val;
          break;
        }
#endif
        default:
          break;
      }

      curPtr += unitSize;
  }
}

static void dumpHexData(void *start, uint32_t len, uint32_t unitSize, int bSpiRead)
{
    int i;
    unsigned int temp;
    /*Force natural alignment*/
    uint8_t* curPtr;

    BCM_ASSERT((unitSize == UNIT_SIZE_BYTES) ||
               (unitSize == UNIT_SIZE_HALFWORDS) ||
               (unitSize == UNIT_SIZE_WORDS) ||
               (unitSize == UNIT_SIZE_DWORDS));

    curPtr = (uint8_t*)((uintptr_t)start&(~(unitSize-1)));

    for (i = 0; i < len; ++i) {
        if (i % (4/unitSize) == 0)
            bcmPrint(" ");
        if (i % (16/unitSize) == 0)
            bcmPrint("\n0x%p : ", curPtr);

        switch (unitSize) {
          case UNIT_SIZE_BYTES:
          {
            if ( bSpiRead )
            {
               spiFns.kerSysSlaveRead(spiDev, (uintptr_t)curPtr, &temp, unitSize);
               bcmPrint("%02X ", (unsigned char)temp);
            }
            else
               
            {
               bcmPrint("%02X ", *curPtr);
            }
            break;
          }
          case UNIT_SIZE_HALFWORDS:
          {
            uint16_t *cur16Ptr = (uint16_t*)curPtr;
            if ( bSpiRead )
            {
               spiFns.kerSysSlaveRead(spiDev, (uintptr_t)curPtr, &temp, unitSize);
               bcmPrint("%04X ", (unsigned short)temp);
            }
            else
            {
               bcmPrint("%04X ", *cur16Ptr);
            }
            break;
          }
          case UNIT_SIZE_WORDS:
          {
            uint32_t *cur32Ptr = (uint32_t*)curPtr;
            if ( bSpiRead )
            {
               spiFns.kerSysSlaveRead(spiDev, (uintptr_t)curPtr, &temp, unitSize);
               bcmPrint("%08X ", (unsigned int)temp);
            }
            else
            {
               bcmPrint("%08X ", *cur32Ptr);
            }
            break;
          }
          default:
            break;
        }

        curPtr += unitSize;
    }

    bcmPrint("\n");
}

static bcmLogModuleInfo_t *getModInfoByName(char *name) {
    int logId;

    for(logId=0; logId<BCM_LOG_ID_MAX; logId++) {
        if(!strcmp(modInfo[logId].name, name))
            return &modInfo[logId];
    }

    return NULL;
}

static ssize_t log_proc_read(struct file *f,
                             char *buf,
                             size_t cnt,
                             loff_t *pos) {
    return 0;
}

static ssize_t log_proc_write(struct file *f, const char *buf, size_t cnt, loff_t *pos) {
    int i;
#define MAX_ARGS 5
#define MAX_ARG_SIZE 32
    typedef char arg_t[MAX_ARG_SIZE];
    arg_t arg[MAX_ARGS];
    int argc;
    char cmd;
    bcmLogModuleInfo_t *pModInfo;
#define LOG_WR_KBUF_SIZE 128
    char kbuf[LOG_WR_KBUF_SIZE];

    if ((cnt > LOG_WR_KBUF_SIZE-1) || (copy_from_user(kbuf, buf, cnt) != 0))
        return -EFAULT;

    kbuf[cnt]=0;

    argc = sscanf(kbuf, "%c %s %s %s %s %s", &cmd, arg[0], arg[1], arg[2], arg[3], arg[4]);

    for (i=0; i<MAX_ARGS; ++i) {
        arg[i][MAX_ARG_SIZE-1] = '\0';
    }

    BCM_LOG_INFO(BCM_LOG_ID_LOG, "WRITE: cmd: %c, argc: %d", cmd, argc);
    for (i=0; i<argc-1; ++i) {
        BCM_LOG_INFO(BCM_LOG_ID_LOG, "arg[%d]: %s ", i, arg[i]);
    }

    switch ( cmd ) {
        BCM_LOGCODE(
            case 'g':
            {
                bcmLogLevel_t logLevel = str2val(arg[0]);
                if(argc == 2 && logLevel >= 0 && logLevel < BCM_LOG_LEVEL_MAX)
                    bcmLog_setGlobalLogLevel(logLevel);
                else
                    BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Invalid Parameter '%s'\n", arg[0]);
                break;
            } )

        BCM_LOGCODE(
            case 'r':
            {
                bcmPrint ("Global Log Level : %d\n", bcmLog_getGlobalLogLevel());
                break;
            } )

        BCM_LOGCODE(
            case 'i':
            {
                if (argc == 1) {
                  int logId;
                  for(logId=0; logId<BCM_LOG_ID_MAX; logId++) {
                    pModInfo = &modInfo[logId];
                    bcmPrint("Name      : %s\n", pModInfo->name);
                    bcmPrint("Id        : %d, Log Level : %d\n", pModInfo->logId, bcmLog_getLogLevel(pModInfo->logId));
                  }
                }
                else if((argc==2) && ((pModInfo=getModInfoByName(arg[0])) != NULL)) {
                    bcmPrint("Name      : %s\n", pModInfo->name);
                    bcmPrint("Id        : %d, Log Level : %d\n", pModInfo->logId, bcmLog_getLogLevel(pModInfo->logId));
                } else {
                    BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Invalid Parameter '%s'\n", arg[0]);
                }
                break;
            } )

        BCM_LOGCODE(
            case 'l':
            {
                bcmLogLevel_t logLevel = str2val(arg[1]);
                if(argc == 3 && ((pModInfo=getModInfoByName(arg[0])) != NULL)) {
                    if(logLevel >= 0 && logLevel < BCM_LOG_LEVEL_MAX) {
                        bcmLog_setLogLevel( pModInfo->logId, logLevel);
                        break;
                    }
                }

                BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Invalid Parameters '%s' '%s'\n", arg[0], arg[1]);

                break;
            } )

        BCM_DATADUMPCODE(
            case 'd':
            {
                bcmLogDataDumpLevel_t ddLevel = str2val(arg[1]);
                if(argc == 3 && ((pModInfo=getModInfoByName(arg[0])) != NULL)) {
                    if(ddLevel >= 0 && ddLevel < BCM_LOG_DD_MAX) {
                        pModInfo->ddLevel = ddLevel;
                        break;
                    }
                }

                BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Invalid Parameters '%s' '%s'\n", arg[0], arg[1]);

                break;
            } )

        BCM_DATADUMPCODE(
            case 'e':
            {
                if (argc == 1) {
                  int logId;
                  for(logId=0; logId<BCM_LOG_ID_MAX; logId++) {
                    pModInfo = &modInfo[logId];
                    bcmPrint("Name      : %s\n", pModInfo->name);
                    bcmPrint("Id        : %d, DataDump Level : %d\n", pModInfo->logId, pModInfo->ddLevel);
                  }
                }
                else if((argc==2) && ((pModInfo=getModInfoByName(arg[0])) != NULL)) {
                    bcmPrint("Name      : %s\n", pModInfo->name);
                    bcmPrint("Id        : %d, DataDump Level : %d\n", pModInfo->logId, pModInfo->ddLevel);
                } else {
                    BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Invalid Parameter '%s'\n", arg[0]);
                }
                break;
            } )
        BCM_DATADUMPCODE(
            case 'h':
            {
                bcmLogDataDumpLevel_t ddLevel = str2val(arg[0]);
                if(argc == 2 && ddLevel >= 0 && ddLevel < BCM_LOG_DD_MAX)
                    globalDataDumpLevel = ddLevel;
                else
                    BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Invalid Parameter '%s'\n", arg[0]);
                break;
            } )
        BCM_LOGCODE(
            case 's':
            {
                bcmPrint ("Global Datadump Level : %d\n", globalDataDumpLevel);
                break;
            } )
        case 'm':
        {
          uintptr_t addr = 0;
          uint32_t len = 1;
          uint32_t unitSize = UNIT_SIZE_BYTES;
          int cmdValid = 1;

          if ((argc < 3) || (argc > 4)) {
            cmdValid = 0;
          }
          else {
            if (!ishex(arg[0])) {
              cmdValid = 0;
              BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Incorrect address: %s Must be in hex., starting with 0x\n", arg[0]);
            }
            else {
              addr = str2val(arg[0]);
            }

            if (argc >= 3)
              len = str2val(arg[1]);

            if (argc == 4) {
              switch (arg[2][0]) {
              case 'b':
                unitSize = UNIT_SIZE_BYTES;
                break;
              case 'h':
                unitSize = UNIT_SIZE_HALFWORDS;
                break;
              case 'w':
                unitSize = UNIT_SIZE_WORDS;
                break;
              default:
                BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Incorrect unit size '%s', must be 'b', 'h' or 'w'\n", arg[2]);
                cmdValid = 0;
              }
            }
          }

          if (cmdValid) {
            dumpHexData((void *)addr, len, unitSize, 0);
          } else {
            BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Invalid Command: %s", kbuf);
          }
          break;
        }

        case 'w':
        {
          uintptr_t addr = 0;
          uint32_t val = 0;
          uint32_t len = 1;
          uint32_t unitSize = UNIT_SIZE_BYTES;
          int cmdValid = 1;

          if ((argc < 3) || (argc > 5)) {
            cmdValid = 0;
          }
          else {
            if (!ishex(arg[0])) {
              cmdValid = 0;
              BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Incorrect address: %s Must be in hex., starting with 0x\n", arg[0]);
            }
            else {
              addr = str2val(arg[0]);
            }

            val = str2val(arg[1]);

            if (argc >= 4) {
              len = str2val(arg[2]);
            }

            if (argc == 5) {
              switch (arg[3][0]) {
              case 'b':
                unitSize = UNIT_SIZE_BYTES;
                break;
              case 'h':
                unitSize = UNIT_SIZE_HALFWORDS;
                break;
              case 'w':
                unitSize = UNIT_SIZE_WORDS;
                break;
              default:
                BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Incorrect unit size '%s', must be 'b', 'h' or 'w'\n", arg[3]);
                cmdValid = 0;
              }
            }
          }

          if ((addr&~(unitSize-1)) != addr) {
            BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Incorrect address alignment: 0x%lx\n", addr);
            cmdValid = 0;
          }

          if (cmdValid) {
            setMem((void *)addr, val, len, unitSize);
            bcmPrint("Done.\n");
          } else {
            BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Invalid Command: %s", kbuf);
          }
          break;
        }
        case 'p':
        {
            // Generic SPI commands
            // Should be usable with any SPI device
            // Leg(0)/HS(1), CS (0-3), CLK Speed, Write Data (hex), length
            uint32_t busnum = 0;
            uint32_t chipsel = 0;
            uint32_t clkspeed = 0;
            uint32_t writeint = 0;
            uint32_t length = 0;
            unsigned char txbuf[32];
            unsigned char rxbuf[32];
            int      cmdValid = 1;

            if (argc != 6) {
               cmdValid = 0;
            }
            else if (spiFns.syncTrans == NULL) {
               BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Attempt to use spi before registered\n");
               cmdValid = 0;
            }   
            else {
                if (!ishex(arg[3])) {
                    cmdValid = 0;
                    BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Incorrect write data: %s Must be in hex., starting with 0x\n", arg[3]);
                }
                else {
                    // Pad the write buffer with 0s to ensure it is a complete 4 bytes word
                    for (i=0;i<10;++i) {
                       if(arg[3][i] == 0) {
                           arg[3][i] = 0x30;
                       }
                    }
                    arg[3][10] = 0;

                    if ((length = str2val(arg[4])) > 32) {
                        cmdValid = 0;
                        BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Incorrect length: Must be <= 32\n");
                    }
                    else {
                        busnum = str2val(arg[0]);
                        chipsel  = str2val(arg[1]);
                        clkspeed  = str2val(arg[2]);
                        writeint  = str2val(arg[3]);
                        memset(txbuf,0,sizeof(txbuf));
                        memset(rxbuf,0,sizeof(txbuf));
                        txbuf[0] = (writeint >> 24) & 0x000000FF;
                        txbuf[1] = (writeint >> 16) & 0x000000FF;
                        txbuf[2] = (writeint >> 8) & 0x000000FF;
                        txbuf[3] = (writeint >> 0) & 0x000000FF;
                        if (0 != spiFns.reserveSlave(busnum, chipsel, clkspeed))
                        {
                            bcmPrint ("Spi device already reserved, clkspeed parameter ignored\n");
                        }
                        spiFns.syncTrans(txbuf, rxbuf, 0, length, busnum, chipsel);
                        bcmPrint ("Transmitted:\n");
                        dumpHexData((void *)txbuf, length, UNIT_SIZE_BYTES, 0);
                        bcmPrint ("Received:\n");
                        dumpHexData((void *)rxbuf, length, UNIT_SIZE_BYTES, 0);
                    }
                }
            }

            if (0 == cmdValid) {
              BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Invalid Command: %s", kbuf);
            }
            break;
        }
#if defined(CONFIG_BCM963268)
        // SPI command to read/write to an external BRCM chip configured as spi slave device (eg. 6829 for BHR)
        case 'u':
        {
            unsigned int  addr = 0;
            unsigned int  val  = 0;
            unsigned int  rwCount  = 0;
            int           unitSize = 0;
            int           loopCount;
            int           cmdValid = 1;
            if (spiFns.kerSysSlaveRead == NULL)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_LOG, "SPI slave not registered");
                cmdValid = 0;
            }
            if ((0 == cmdValid) || ((argc != 5) && (argc != 6)))
            {
               cmdValid = 0;
            }
            else
            {
            	if( ((spiDev = str2val(arg[0])) < 0) || !ishex(arg[1]) )
                {
                    cmdValid = 0;
                    BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Incorrect address: %s Must be in hex., starting with 0x or device number %s\n", arg[1], arg[0]);
                }
                else
                {
                    char  trSize;

                    addr    = str2val(arg[1]);
                    trSize  = arg[2][0];
                    rwCount = str2val(arg[3]);
                    if ( 6 == argc )
                       val = str2val(arg[4]);

                    switch (trSize)
                    {
                        case 'b':
                            unitSize = 1;
                            break;
                        case 'h':
                            unitSize = 2;
                            break;
                        case 'w':
                            unitSize = 4;
                            break;
                        default:
                           BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Incorrect unit size '%s', must be 'b', 'h' or 'w'\n", arg[2]);
                           cmdValid = 0;
                           unitSize = 0;
                           break;
                    }

                    if ( 1 == cmdValid )
                    {
                        if ( 5 == argc )
                        {
                           /* read operation */
                           dumpHexData((void *)addr, rwCount, unitSize, 1);
                        }
                        else
                        {
                           /* write operation */
                           for ( loopCount = 0; loopCount < rwCount; loopCount++ )
                           {
                               spiFns.kerSysSlaveWrite(spiDev, addr, val, unitSize);
                               addr += unitSize;
                           }
                        }
                    }
                }
            }

            if (0 == cmdValid)
            {
              BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Invalid Command: %s", kbuf);
            }
            break;
        }
#endif
        default:
        {
          bcmPrint("Usage:\n");

          BCM_LOGCODE(
            bcmPrint("g <level>               : Set global log level\n");
            bcmPrint("r                       : Get global log level\n");
            bcmPrint("l <module_name> <level> : Set the log level of a module\n");
            bcmPrint("i [<module_name>]       : Get module information\n");
          )

          BCM_DATADUMPCODE(
            bcmPrint("h <level>               : Set global datadump level\n");
            bcmPrint("s                       : Get global datadump level\n");
            bcmPrint("d <module_name> <level> : Set data dump detail level\n");
            bcmPrint("e [<module_name>]       : Get data dump detail level\n");
          )

          bcmPrint("m <hexaddr> [<length> [<unitsize>]]: Dump a memory region\n");
          bcmPrint("w <hexaddr> <val> [<length> [<unitsize>]]: Write to a memory region\n");
          break;
        }
    }

    return cnt;
}

static struct file_operations log_proc_fops = {
    .read = log_proc_read,
    .write = log_proc_write,
};



/**
 ** Helper Functions
 **/

bcmLogModuleInfo_t *bcmLog_logIsEnabled(bcmLogId_t logId, bcmLogLevel_t logLevel) {
    BCM_LOG_CHECK_LOG_ID(logId);
    BCM_LOG_CHECK_LOG_LEVEL(logLevel);

    if(globalLogLevel >= logLevel &&
       modInfo[logId].logLevel >= logLevel)
        return &modInfo[logId];

    return NULL;
}

#if defined(BCM_DATADUMP_SUPPORTED)
bcmLogModuleInfo_t *bcmLog_ddIsEnabled(bcmLogId_t logId, bcmLogDataDumpLevel_t ddLevel) {
    BCM_LOG_CHECK_LOG_ID(logId);
    BCM_LOG_CHECK_DD_LEVEL(ddLevel);

    if(globalDataDumpLevel >= ddLevel &&
       modInfo[logId].ddLevel >= ddLevel)
        return &modInfo[logId];

    return NULL;
}
#endif

EXPORT_SYMBOL(bcmLog_logIsEnabled);

/**
 ** Public API
 **/

void bcmLog_setGlobalLogLevel(bcmLogLevel_t logLevel) {

    bcmLogId_t logId;
    bcmLogLevel_t oldGlobalLevel;
    
    BCM_LOG_CHECK_LOG_LEVEL(logLevel);

    oldGlobalLevel = globalLogLevel;
    globalLogLevel = logLevel;

    for (logId = 0; logId < BCM_LOG_ID_MAX; logId++)
    {
        if (modInfo[logId].lcCallback)
        {
            bcmLogLevel_t oldLevel;
            bcmLogLevel_t newLevel;

            oldLevel = min(modInfo[logId].logLevel, oldGlobalLevel);
            newLevel = min(modInfo[logId].logLevel, globalLogLevel);
            if (oldLevel != newLevel)
            {
                modInfo[logId].lcCallback(logId, newLevel, modInfo[logId].lcCallbackCtx);
            }
        }
    }

    BCM_LOG_INFO(BCM_LOG_ID_LOG, "Global log level was set to %d", globalLogLevel);
}

bcmLogLevel_t bcmLog_getGlobalLogLevel(void) {
    return globalLogLevel;
}

void bcmLog_setLogLevel(bcmLogId_t logId, bcmLogLevel_t logLevel) {

    bcmLogLevel_t oldLocalLevel;

    BCM_LOG_CHECK_LOG_ID(logId);
    BCM_LOG_CHECK_LOG_LEVEL(logLevel);
    
    oldLocalLevel = modInfo[logId].logLevel;
    modInfo[logId].logLevel = logLevel;

    if (modInfo[logId].lcCallback)
    {
        bcmLogLevel_t newLevel;
        bcmLogLevel_t oldLevel;
       
        oldLevel = min(oldLocalLevel, globalLogLevel);
        newLevel = min(modInfo[logId].logLevel, globalLogLevel);   
        
        if (oldLevel != newLevel)
        {
            modInfo[logId].lcCallback(logId, newLevel, modInfo[logId].lcCallbackCtx);
        }
    }

    BCM_LOG_INFO(BCM_LOG_ID_LOG, "Log level of %s was set to %d",
                 modInfo[logId].name, modInfo[logId].logLevel);
}


void bcmLog_registerLevelChangeCallback(bcmLogId_t logId, bcmLogLevelChangeCallback_t callback, void *ctx) {
    BCM_LOG_CHECK_LOG_ID(logId);

    modInfo[logId].lcCallback = callback;
    modInfo[logId].lcCallbackCtx = ctx;
}


bcmLogLevel_t bcmLog_getLogLevel(bcmLogId_t logId) {
    BCM_LOG_CHECK_LOG_ID(logId);
    return modInfo[logId].logLevel;
}

char *bcmLog_getModName(bcmLogId_t logId) {
    BCM_LOG_CHECK_LOG_ID(logId);
    return modInfo[logId].name;
}

#if defined(BCM_DATADUMP_SUPPORTED)
/*Dummy implementation*/
void bcm_dataDumpRegPrinter(uint32_t qId, uint32_t dataDumpId, Bcm_DataDumpPrintFunc *printFun) {
    BCM_ASSERT(qId < MAX_NUM_QIDS);
    BCM_ASSERT(dataDumpId < MAX_NUM_DATADUMP_IDS);
    printFuns[qId*MAX_NUM_DATADUMP_IDS + dataDumpId] = printFun;
}

/*Dummy implementation*/
void bcm_dataDump(uint32_t qID, uint32_t dataDumpID, const char* dataDumpName, void *ptr, uint32_t numBytes) {
    Bcm_DataDumpPrintFunc* printFun;
    BCM_ASSERT( qID < MAX_NUM_QIDS);
    BCM_ASSERT( dataDumpID < MAX_NUM_DATADUMP_IDS);
    bcmPrint("---DataDump Start---\n");
    if (qids[qID] == 0) {
        BCM_LOG_ERROR(BCM_LOG_ID_LOG, "DataDump qID %d not registered.\n", qID);
    }
    else {
        printFun = printFuns[qID*MAX_NUM_DATADUMP_IDS + dataDumpID];
        bcmPrint("qID: %s, DataDump ID: %s, numBytes: %d\n", qids[qID], dataDumpName, numBytes);
        if (printFun) {
            buf[0]=0;
            (*printFun)(dataDumpID, ptr, numBytes, buf, PRINTBUF_SIZE);
            bcmPrint(buf);
        }
        else {
            uint32_t *data = ptr;
            uint8_t *dataBytes;
            int i=0;

            while (i+16<=numBytes) {
                bcmPrint("%4.4x: 0x%8.8x 0x%8.8x 0x%8.8x 0x%8.8x\n", i, data[i/4], data[i/4+1], data[i/4+2], data[i/4+3]);
                i+=16;
            }

            if (i+4<=numBytes) {
                bcmPrint("%4.4x: ", i);
                while (i+4<=numBytes) {
                    bcmPrint("0x%8.8x ", data[i/4]);
                    i+=4;
                }
            }

            if (i< numBytes) {
               if (i % 16 == 0) {
                   bcmPrint("%4.4x: ", i);
               }

                dataBytes = (uint8_t*)&data[i/4];
                bcmPrint("0x");
                while (i<numBytes) {
                    bcmPrint("%2.2x", *dataBytes++);
                    ++i;
                }
                bcmPrint("\n");
            }
        }
    }
    bcmPrint("\n---DataDump End---\n");
}

uint32_t bcm_dataDumpCreateQ(const char* qName) {
    int i;
    for (i=0; i<MAX_NUM_QIDS; ++i) {
        if (qids[i] == 0) {
            qids[i] = qName;
            return i;
        }
    }

    BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Can not create dataDump queue. Max. #qids reached.\n");
    return ~0U;
}

void bcm_dataDumpDeleteQ(uint32_t qid) {
    BCM_ASSERT( qid < MAX_NUM_QIDS);
    if (qids[qid] != 0) {
        qids[qid] = 0;
    }
    else {
        BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Can not delete dataDump queue. qid unknown.\n");
    }
}

void bcmFun_reg(bcmFunId_t funId, bcmFun_t *f) {
  BCM_ASSERT(f);
  BCM_ASSERT(funId < BCM_FUN_ID_MAX);

  funTable[funId] = f;
}

void bcmFun_dereg(bcmFunId_t funId) {
  BCM_ASSERT(funId < BCM_FUN_ID_MAX);

  funTable[funId] = 0;
}

bcmFun_t* bcmFun_get(bcmFunId_t funId) {
  BCM_ASSERT(funId < BCM_FUN_ID_MAX);

  return funTable[funId];
}

int bcm_set_affinity(int irq, int cpu_id, int force)
{
#if defined(CONFIG_SMP)
    struct cpumask mask;
    int ret;

    if(cpu_id < 0 || cpu_id >= NR_CPUS)
    {
        return -1;
    }

    cpumask_clear(&mask);

    cpumask_set_cpu(cpu_id, &mask);

    /* force parameter allow the set irq affinity on the cpu that has not power up yet */
    if(force)
    {
        ret = irq_force_affinity(irq, &mask);
    }
    else
    {
        ret = irq_set_affinity(irq, &mask);
    }

    return ret;
#else
    return 0;
#endif
}

int __init bcmLog_init( void ) {
    struct proc_dir_entry *p;

    p = proc_create(PROC_ENTRY_NAME, 0, NULL, &log_proc_fops);
    if (!p) {
        bcmPrint("bcmlog: unable to create /proc/%s!\n", PROC_ENTRY_NAME);
        return -1;
    }

    bcmPrint("Broadcom Logger %s\n", VER_STR);
    return 0;	
}

subsys_initcall(bcmLog_init);

void bcmLog_registerSpiCallbacks(bcmLogSpiCallbacks_t callbacks) 
{
    spiFns = callbacks;
    BCM_ASSERT(spiFns.reserveSlave != NULL);
    BCM_ASSERT(spiFns.syncTrans != NULL); 
}


EXPORT_SYMBOL(bcmLog_ddIsEnabled);
EXPORT_SYMBOL(bcm_dataDumpRegPrinter);
EXPORT_SYMBOL(bcm_dataDump);
EXPORT_SYMBOL(bcm_dataDumpCreateQ);
EXPORT_SYMBOL(bcm_dataDumpDeleteQ);
EXPORT_SYMBOL(bcmFun_reg);
EXPORT_SYMBOL(bcmFun_dereg);
EXPORT_SYMBOL(bcmFun_get);

EXPORT_SYMBOL(bcm_set_affinity);

#endif /*defined(BCM_DATADUMP_SUPPORTED)*/

EXPORT_SYMBOL(bcmLog_setGlobalLogLevel);
EXPORT_SYMBOL(bcmLog_getGlobalLogLevel);
EXPORT_SYMBOL(bcmLog_setLogLevel);
EXPORT_SYMBOL(bcmLog_getLogLevel);
EXPORT_SYMBOL(bcmLog_getModName);
EXPORT_SYMBOL(bcmLog_registerSpiCallbacks);
EXPORT_SYMBOL(bcmLog_registerLevelChangeCallback);

#endif
