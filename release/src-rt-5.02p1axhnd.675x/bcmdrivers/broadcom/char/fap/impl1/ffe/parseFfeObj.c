
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define SWAP_WORDS

#define FFE_OBJ_STR_SIZE 13
#define FFE_INST_STR_SIZE 16

#define FFE_INST_MEM_DEPTH     2048
#define FFE_INST_MEM_SIZE_32   (FFE_INST_MEM_DEPTH * 2)

#if 0
#define debugPrint(fmt, arg...) printf(fmt, ##arg)
#else
#define debugPrint(fmt, arg...)
#endif

typedef unsigned long uint32_t;

typedef enum {
    FFE_FILE_OBJ,
    FFE_FILE_PATCH,
    FFE_FILE_MAX
} ffeFileType_t;

static uint32_t ffeMemBuf[FFE_INST_MEM_SIZE_32];

/* str must not contain '\n' */
static int str2bin(char *str, char *buf, int bufSize)
{
    unsigned int len = strlen(str);
    char temp[3];
    int i;

    if(len & 1)
    {
        printf("ERROR: Odd string length (%d)\n", len);
        return -1;
    }

    debugPrint("s: %s\n", str);

    temp[2] = '\0';

    if((len/2) > bufSize)
    {
        len = bufSize;
    }
    else
    {
        len /= 2;
    }

    for(i=0; i<len; ++i)
    {
        strncpy(temp, str, 2);
        str += 2;
        buf[i] = strtoul(temp, NULL, 16);

//            debugPrint("temp %s -> val 0x%x\n", temp, val[i]);
    }

#if 1
    debugPrint("b: ");
    for(i=0; i<len; ++i)
    {
        {
            debugPrint("%02x", (unsigned char)buf[i]);
        }
    }
    debugPrint("\n\n");
#endif

    return 0;
}

static inline uint32_t changeEndian(uint32_t val)
{
    return ( ((val << 24) & 0xFF000000 ) |
             ((val <<  8) & 0x00FF0000 ) |
             ((val >>  8) & 0x0000FF00 ) |
             ((val >> 24) & 0x000000FF ) );
}

static int parseImemFile(ffeFileType_t ffeFileType, char *fileName,
                         uint32_t *buf, uint32_t *bufSize)
{
    int ret = 0;
    FILE *fp;
    int i;
    char strBuf[FFE_INST_STR_SIZE*2];
    unsigned int index = 0;
    const int objStart = FFE_INST_STR_SIZE - FFE_OBJ_STR_SIZE;
    char *retS;

    if(ffeFileType >= FFE_FILE_MAX)
    {
        printf("ERROR: Invalid File Type <%d>\n", ffeFileType);
        ret = -1;
        goto out;
    }

    fp = fopen(fileName, "r");
    if(fp == NULL)
    {
        printf("ERROR: Failed to Open %s: %s\n", fileName, strerror(errno));
        ret = -1;
        goto out;
    }

    /* Initialize buffer */
    for(i=0; i<objStart; ++i)
    {
        strBuf[i] = '0';
    }

    while(1)
    {
        if(ffeFileType == FFE_FILE_PATCH)
        {
            retS = fgets(strBuf, FFE_INST_STR_SIZE*2, fp);
        }
        else
        {
            retS = fgets(&strBuf[objStart], (FFE_INST_STR_SIZE*2)-objStart, fp);
        }

//        if(fgets(&strBuf[objStart], (FFE_INST_STR_SIZE*2)-objStart, fp) == NULL)
        if(retS == NULL)
        {
            if(feof(fp))
            {
                debugPrint("Found %d Instructions\n", index/2);
                break;
            }
            else
            {
                printf("ERROR: fgets()\n");
                ret = -1;
                break;
            }
        }

        /* get rid of the newline character */
        strBuf[FFE_INST_STR_SIZE] = '\0';

        ret = str2bin(strBuf, (char *)(&buf[index]), 2 * sizeof(uint32_t));
        if(ret)
        {
            goto out;
        }

#if defined(LITTLE_ENDIAN)
        buf[index] = changeEndian(buf[index]);
        buf[index+1] = changeEndian(buf[index+1]);
#endif
#if defined(SWAP_WORDS)
        {
            uint32_t temp = buf[index];
            buf[index] = buf[index+1];
            buf[index+1] = temp;
        }
#endif
        debugPrint("%u : 0x%08X, 0x%08X\n\n", index, buf[index], buf[index+1]);

        index += 2;

        if(index >= *bufSize)
        {
            printf("ERROR: Buffer is too Small <%d>\n", *bufSize);
            break;
        }
    }

    *bufSize = index;

    if(fclose(fp))
    {
        printf("ERROR: Failed to Close %s: %s\n",
               fileName, strerror(errno));
    }

out:
    return ret;
}

static char nameBuf[80];

int main(int argc, char **argv)
{
    int nRet;
    uint32_t size;
    char *moduleName;
    FILE *fp;
    int i;

    argc--, argv++;

    moduleName = *argv;
    snprintf(nameBuf, 80, "%s.o", moduleName);

    size = FFE_INST_MEM_SIZE_32;

    nRet = parseImemFile(FFE_FILE_OBJ, nameBuf, ffeMemBuf, &size);
    if(nRet == -1)
    {
        goto out;
    }

    snprintf(nameBuf, 80, "%s.h", moduleName);
    fp = fopen(nameBuf, "w");
    if(fp == NULL)
    {
        printf("ERROR: Failed to Open %s: %s\n",
               nameBuf, strerror(errno));
        nRet = -1;
        goto out;
    }

    fprintf(fp, "uint32 %s[] = {\n", moduleName);
    for(i=0; i<size; i+=2)
    {
        fprintf(fp, "0x%08X, 0x%08X,\n", ffeMemBuf[i], ffeMemBuf[i+1]);
    }
    fprintf(fp, "};\n", moduleName);

    if(fclose(fp))
    {
        printf("ERROR: Failed to Close %s: %s\n",
               nameBuf, strerror(errno));
    }

out:
    return nRet;
}
