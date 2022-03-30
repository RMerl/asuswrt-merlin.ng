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

#include <linux/uaccess.h> /*copy_from_user*/
#include <linux/module.h>
#include <linux/version.h>

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

#ifdef CONFIG_BCM_LOG

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

static void dumpHexData(void *start, uint32_t len, uint32_t unitSize)
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
            bcm_printk(" ");
        if (i % (16/unitSize) == 0)
            bcm_printk("\n0x%p : ", curPtr);

        switch (unitSize) {
          case UNIT_SIZE_BYTES:
          {  
            bcm_printk("%02X ", *curPtr);
            break;
          }
          case UNIT_SIZE_HALFWORDS:
          {
            uint16_t *cur16Ptr = (uint16_t*)curPtr;
            bcm_printk("%04X ", *cur16Ptr);
            break;
          }
          case UNIT_SIZE_WORDS:
          {
            uint32_t *cur32Ptr = (uint32_t*)curPtr;
            bcm_printk("%08X ", *cur32Ptr);
            break;
          }
          default:
            break;
        }

        curPtr += unitSize;
    }

    bcm_printk("\n");
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
                bcm_printk ("Global Log Level : %d\n", bcmLog_getGlobalLogLevel());
                break;
            } )

        BCM_LOGCODE(
            case 'i':
            {
                if (argc == 1) {
                  int logId;
                  for(logId=0; logId<BCM_LOG_ID_MAX; logId++) {
                    pModInfo = &modInfo[logId];
                    bcm_printk("Name      : %s\n", pModInfo->name);
                    bcm_printk("Id        : %d, Log Level : %d\n", pModInfo->logId, bcmLog_getLogLevel(pModInfo->logId));
                  }
                }
                else if((argc==2) && ((pModInfo=getModInfoByName(arg[0])) != NULL)) {
                    bcm_printk("Name      : %s\n", pModInfo->name);
                    bcm_printk("Id        : %d, Log Level : %d\n", pModInfo->logId, bcmLog_getLogLevel(pModInfo->logId));
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
                    bcm_printk("Name      : %s\n", pModInfo->name);
                    bcm_printk("Id        : %d, DataDump Level : %d\n", pModInfo->logId, pModInfo->ddLevel);
                  }
                }
                else if((argc==2) && ((pModInfo=getModInfoByName(arg[0])) != NULL)) {
                    bcm_printk("Name      : %s\n", pModInfo->name);
                    bcm_printk("Id        : %d, DataDump Level : %d\n", pModInfo->logId, pModInfo->ddLevel);
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
                bcm_printk ("Global Datadump Level : %d\n", globalDataDumpLevel);
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
            dumpHexData((void *)addr, len, unitSize);
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
            bcm_printk("Done.\n");
          } else {
            BCM_LOG_ERROR(BCM_LOG_ID_LOG, "Invalid Command: %s", kbuf);
          }
          break;
        }

        default:
        {
          bcm_printk("Usage:\n");

          BCM_LOGCODE(
            bcm_printk("g <level>               : Set global log level\n");
            bcm_printk("r                       : Get global log level\n");
            bcm_printk("l <module_name> <level> : Set the log level of a module\n");
            bcm_printk("i [<module_name>]       : Get module information\n");
          )

          BCM_DATADUMPCODE(
            bcm_printk("h <level>               : Set global datadump level\n");
            bcm_printk("s                       : Get global datadump level\n");
            bcm_printk("d <module_name> <level> : Set data dump detail level\n");
            bcm_printk("e [<module_name>]       : Get data dump detail level\n");
          )

          bcm_printk("m <hexaddr> [<length> [<unitsize>]]: Dump a memory region\n");
          bcm_printk("w <hexaddr> <val> [<length> [<unitsize>]]: Write to a memory region\n");
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
    bcm_printk("---DataDump Start---\n");
    if (qids[qID] == 0) {
        BCM_LOG_ERROR(BCM_LOG_ID_LOG, "DataDump qID %d not registered.\n", qID);
    }
    else {
        printFun = printFuns[qID*MAX_NUM_DATADUMP_IDS + dataDumpID];
        bcm_printk("qID: %s, DataDump ID: %s, numBytes: %d\n", qids[qID], dataDumpName, numBytes);
        if (printFun) {
            buf[0]=0;
            (*printFun)(dataDumpID, ptr, numBytes, buf, PRINTBUF_SIZE);
            bcm_printk(buf);
        }
        else {
            uint32_t *data = ptr;
            uint8_t *dataBytes;
            int i=0;

            while (i+16<=numBytes) {
                bcm_printk("%4.4x: 0x%8.8x 0x%8.8x 0x%8.8x 0x%8.8x\n", i, data[i/4], data[i/4+1], data[i/4+2], data[i/4+3]);
                i+=16;
            }

            if (i+4<=numBytes) {
                bcm_printk("%4.4x: ", i);
                while (i+4<=numBytes) {
                    bcm_printk("0x%8.8x ", data[i/4]);
                    i+=4;
                }
            }

            if (i< numBytes) {
               if (i % 16 == 0) {
                   bcm_printk("%4.4x: ", i);
               }

                dataBytes = (uint8_t*)&data[i/4];
                bcm_printk("0x");
                while (i<numBytes) {
                    bcm_printk("%2.2x", *dataBytes++);
                    ++i;
                }
                bcm_printk("\n");
            }
        }
    }
    bcm_printk("\n---DataDump End---\n");
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

EXPORT_SYMBOL(bcmLog_ddIsEnabled);
EXPORT_SYMBOL(bcm_dataDumpRegPrinter);
EXPORT_SYMBOL(bcm_dataDump);
EXPORT_SYMBOL(bcm_dataDumpCreateQ);
EXPORT_SYMBOL(bcm_dataDumpDeleteQ);
#endif /* BCM_DATADUMP_SUPPORTED */

int __init bcmLog_init( void ) {
    struct proc_dir_entry *p;

    p = proc_create(PROC_ENTRY_NAME, 0, NULL, &log_proc_fops);
    if (!p) {
        bcm_printk("bcmlog: unable to create /proc/%s!\n", PROC_ENTRY_NAME);
        return -1;
    }

    bcm_printk("Broadcom Logger %s\n", VER_STR);
    return 0;
}

subsys_initcall(bcmLog_init);

#endif /* CONFIG_BCM_LOG */

/* This fucntion is same as printk, but is defined always
 * and should be used in binary only modules to avoid
 * dependency on CONFIG_PRINTK. It can be used in place of 
 * regular printk as well.
 *
 * All driver should use/define macro based on this function and
 * should not create new clones. 
 */
__visible int bcm_printk(const char *fmt, ...)
{
#ifdef CONFIG_PRINTK
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(4, 20, 0))
extern int vprintk_func(const char *fmt, va_list args);
    va_list args;
    int r;

    va_start(args, fmt);
    r = vprintk_func(fmt, args);
    va_end(args);

    return r;
#else
#error "bcm_printk implemented required for this kernel version"
#endif
#else
    return 0;
#endif
}

/* This fucntion is same as seq_printf, but is defined always
 * and should be used in binary only modules to avoid
 * dependency on CONFIG_PRINTK. It can be used in place of 
 * regular seq_printf as well.
 *
 * All driver should use/define macro based on this function and
 * should not create new clones. 
 */
__visible void bcm_seq_printf(struct seq_file *s, const char *fmt, ...)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(4, 20, 0))
extern void seq_vprintf(struct seq_file *s, const char *fmt, va_list args);
    va_list args;

    va_start(args, fmt);
    seq_vprintf(s, fmt, args);
    va_end(args);
#else
#error "bcm_seq_printf implemented required for this kernel version"
#endif
}

EXPORT_SYMBOL(bcmLog_logIsEnabled);
EXPORT_SYMBOL(bcmLog_setGlobalLogLevel);
EXPORT_SYMBOL(bcmLog_getGlobalLogLevel);
EXPORT_SYMBOL(bcmLog_setLogLevel);
EXPORT_SYMBOL(bcmLog_getLogLevel);
EXPORT_SYMBOL(bcmLog_getModName);
EXPORT_SYMBOL(bcmLog_registerLevelChangeCallback);
EXPORT_SYMBOL(bcmFun_reg);
EXPORT_SYMBOL(bcmFun_dereg);
EXPORT_SYMBOL(bcmFun_get);

EXPORT_SYMBOL(bcm_printk);
EXPORT_SYMBOL(bcm_seq_printf);
