/***********************************************************************
 *
 *  Copyright (c) 2008  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/

/* This file is used to write data wlan params into  nvram */

/*
Input file format

ID=pci/bus/slot
offset=value
...
ID=pci/bus/slot
offset=value
...
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include "board.h"
#include <bcm_hwdefs.h>
#include <stddef.h>
#define WLAN_FEATURE_POS  (offsetof(NVRAM_DATA,wlanParams)+NVRAM_WLAN_PARAMS_LEN-1)
#define NVRAM_WLAN_FEATURE_TAG "WLANFEATURE"
#define RESET "\033[0m"
#define BOLDBLACK "\033[1m\033[30m"


#define d_printf(args...)	{ if (dbg) printf(args); }


#define MAX_ENTRY_SIZE (600)
#define MAX_ADAPTERS   (4)
#define NVRAM_SPACE_SIZE  (1024)
#define NAND_TEST_FILE "/proc/nvram/wl_nand_manufacturer"
#define SI_DEVPATH_BUFSZ  (16)

typedef struct entry_struct {
    unsigned short offset;
    unsigned short value;
} Entry_struct;

typedef struct adapter_struct {
    unsigned char id[SI_DEVPATH_BUFSZ];            /*format pci/0/1 */
    unsigned short entry_num;
    struct entry_struct entry[MAX_ENTRY_SIZE];
} Adapter_struct;

typedef struct params_struct {
    unsigned short adapter_num;
    struct adapter_struct adapter[MAX_ADAPTERS];
} Params_struct;

#define BOARD_DEVICE_NAME  "/dev/brcmboard"
#define NVRAM_WLAN_TAG "WLANDATA"

unsigned char dbg= 0, readOnly= 0, nvramClean = 0,show=0;
char *tftpserver=NULL;
char *nvram_map_dir="/etc/wlan";
int bypass_prompt=0;
int b_nand=0;
int b_m=0;
int nand_indicator_value=0;
int partition3_has_size=0;

#define CMD_MAX_LEN (32)
#define BUFF_SIZE (512)
char buff[BUFF_SIZE];

#define SYSTEM_RUN(cmd,s1,s2,err) do { \
        snprintf(buff,BUFF_SIZE,"%s %s %s 2>&1 >/dev/null",cmd,s1,s2); \
        if(system(buff)) { \
            return  err; \
         }} while(0)

typedef enum {
    ERR_FILENAME_TOOLONG=1,
    ERR_RUNTFTP_CMD,
    ERR_FINDFILE,
    ERR_REMOUNT_RW,
    ERR_REMOUNT_RO,
    ERR_CPFILE,
} UPDATE_ERR;

/*Write parameter into Flash*/
int  boardIoctl(int boardIoctl,
                BOARD_IOCTL_ACTION action,
                char *string,
                int strLen,
                int offset,
                void *data)
{
    BOARD_IOCTL_PARMS ioctlParms;
    int boardFd = 0;
    int rc;
    int ret=0;


    boardFd = open(BOARD_DEVICE_NAME, O_RDWR);

    if ( boardFd != -1 ) {
        ioctlParms.string = string;
        ioctlParms.strLen = strLen;
        ioctlParms.offset = offset;
        ioctlParms.action = action;
        ioctlParms.buf    = data;
        ioctlParms.result = -1;

        rc = ioctl(boardFd, boardIoctl, &ioctlParms);


        if (rc < 0) {
            printf("boardIoctl=0x%x action=%d rc=%d\n", boardIoctl, action, rc);
            ret = -1;
        }

    } else {
        printf("Unable to open device %s", BOARD_DEVICE_NAME);
        ret = -1;
    }

    close(boardFd);
    return ret;
}

static int nand_exist(void)
{
    FILE *fp;
    char buf[16];
    fp= fopen(NAND_TEST_FILE,"r");
    if(fp) {
        if(fgets(buf,16,fp)!=NULL) {
            sscanf(buf,"%d",&nand_indicator_value);
            partition3_has_size = nand_indicator_value&WLAN_MFG_PARTITION_HASSIZE;
            b_nand = nand_indicator_value&WLAN_MFG_PARTITION_ISNAND;
        }
        fclose(fp);
    }
    return 0;
}

static int sure_to_continue(char *prompt)
{
    char answer='N';
    if(bypass_prompt) return 1;
    printf(prompt);
    scanf("%c",&answer);
    if(answer=='Y' ||  answer=='y')
        return 1;
    return 0;
}
static void help(void)
{
    printf("Usage: \n");
    printf("nvramUpdate [-m] [-s tftpdserver] [-h] {DataFileName | Recover | Read | Show | Clean | Feature} \n");
    printf("==============================\n");
    printf("-m:       Option -m for {Clean} and {Data} indicate manufacture automation to bypass prompt\n");
    printf("-s:       Option -s tftpdserver where nvram map binary file are loaded from\n");
    printf("-d:       Option -d to specify where the nvram map binary to put on system, default to /etc/wlan\n");
    printf("-h:       Option -h for help\n");
    printf("==============================\n");
    printf("Clean:    Clean up Nvram\n");
    printf("Read:     Read back Calibration data in binary format\n");
    printf("Show:     Read back Calibration data in input ASCII file format\n");
    printf("Recover:  Copy all nvramMap .bin from backup to /etc/wlan \n");
    printf("Feature:  Get/Set wl feature settings\n");
    printf("DataFileName: Nvram data file name,when ended with \".bin\",it indicates as a complete nvram MAP\n");
    printf("              it will be loaded to /etc/wlan or the directory where -d is given, if file name is\n");
    printf("              not ended with \".bin\",it will be partial change and formated as:\n");
    printf("    ###################\n");
    printf("    ID=pci/bus/slot/ Or\n");
    printf("    ID=pci/domain/bus/slot/\n");
    printf("    offset1=value1\n");
    printf("    ......\n");
    printf("    ID=pci/bus/slot/\n");
    printf("    offset1=value1\n");
    printf("    ......\n");
    printf("    offsetn=valuen\n");
    printf("    ###################\n\n\n");
    printf("    Example: 52=38fc \n");
    printf("    value is 16bit "BOLDBLACK"hexical"RESET" data\n");
    printf("    Offset is "BOLDBLACK"decimal"RESET", 16bit aligment offset(not 8bit alignment offset)\n");
    printf("    The ID will print in console when driver load \n");
}

UPDATE_ERR handle_new_fwnv_update(char * const fwnv_bin_file) {

    FILE *fp;
    int err=0;
	char fullpath[256];
	char *fwname=basename(fwnv_bin_file);
    if(!strncmp(fwnv_bin_file,"Recover",7)) {
        if(!partition3_has_size) {
            fprintf(stderr,"There is no backuped bin map\n");
            return ERR_REMOUNT_RO;
        } else {
            SYSTEM_RUN("mount -t ubifs ubi:rootfs_ubifs /", "-o","remount,rw",ERR_REMOUNT_RW);
            SYSTEM_RUN("cp -f ","/mnt/mfg_data/*.bin",nvram_map_dir,ERR_CPFILE);
			system("sync");
            SYSTEM_RUN("mount -t ubifs ubi:rootfs_ubifs /" ,"-o","remount,ro",ERR_REMOUNT_RO);
        }
    }
    else {
        if(tftpserver) {
            /* if tftpserver is given,will get the file from ftp server */
            if(strlen(tftpserver)+strlen(fwnv_bin_file)<=(BUFF_SIZE-CMD_MAX_LEN)) {
                SYSTEM_RUN("cd /var;tftp -g -r",fwnv_bin_file,tftpserver,ERR_RUNTFTP_CMD);
                snprintf(fullpath,256,"/var/%s",fwname);
            }
            else return ERR_FILENAME_TOOLONG;
        } else
            snprintf(fullpath,256,"%s",fwnv_bin_file);

        if(!err) {
            fp=fopen(fullpath,"r");
            if(!fp) {
                fprintf(stderr, "%s:%d:	WL binmap:%s  does not exist!! \n",__FUNCTION__,__LINE__,fullpath);
                return ERR_FINDFILE;
            }
            /*now remount file system to overwrite binmap*/
            SYSTEM_RUN("mount -t ubifs ubi:rootfs_ubifs /", "-o","remount,rw",ERR_REMOUNT_RW);
			SYSTEM_RUN("cp -f ",fullpath,nvram_map_dir,ERR_CPFILE);
			if(partition3_has_size)
				SYSTEM_RUN("cp -f ",fullpath,"/mnt/mfg_data",ERR_CPFILE);
			system("sync");
            SYSTEM_RUN("mount -t ubifs ubi:rootfs_ubifs /" ,"-o","remount,ro",ERR_REMOUNT_RO);
        }
    }
    return 0;
}

int handle_legacy_fwnv_update(int argc, char* const argv[])
{
    FILE *fp;
    char name[32], *data_array,nvram_array[NVRAM_SPACE_SIZE];
    unsigned int offset, value;
    char line[64], *ptr;
    int cnt=0,al=0;

    struct params_struct params;

    struct adapter_struct *adapter_ptr=NULL;
    struct entry_struct *entry_ptr = NULL;
    int i=0, j=0;

    int size =0;
    unsigned short *data_ptr;

    int show_feature=0;
    unsigned char wlanfeature=DEFAULT_WLAN_DEVICE_FEATURE;

    memset(&params, 0, sizeof(params) );

    /*Parse the argv parameters*/
    if ( argc < 2 ) {
        help();
        return -1;
    }


    if ( argc > 1 ) {
        if ( !strcmp( argv[1], "Clean" ) ) {
            if(b_nand && !b_m)  {
                printf("\nSet to Manufacturer Mode)(Feature bit 0x2) to clean!\n");
                return 0;
            }
            if(!sure_to_continue("Are you sure to clean?(Y/N):"))
                return 0;
            nvramClean = 1;
        } else if ( !strcmp( argv[1], "Read" ) ) {
            dbg =1;
            readOnly = 1;
        } else if ( !strcmp( argv[1], "Show" ) ) {
            dbg =1;
            readOnly = 1;
            show=1;
        } else if ( !strcmp( argv[1], "Feature" ) ) {
            readOnly = 1;
        }
    }

    if ( !strcmp( argv[1], "Feature" ) ) {
        if ( argc > 2 ) {
            char *feature_str=NULL;
            int scan_ret=0;

            if ( !strcmp(argv[2], "Debug") ) {
                dbg = 1;
                if(argc>3) feature_str=argv[3];
            } else  {
                dbg=0;
                feature_str=argv[2];
            }

            if(feature_str) {
                if(strstr(feature_str,"0x"))
                    scan_ret=sscanf(feature_str,"0x%hhx",&wlanfeature);
                else
                    scan_ret=sscanf(feature_str,"%hhu",&wlanfeature);

                if(scan_ret) {
                    /* if resetbit (bit 7) is set, then it will reset to default value */
                    if(wlanfeature&0x80) {
                        wlanfeature= DEFAULT_WLAN_DEVICE_FEATURE;
                        printf("Feature resetbit is set, reset wlan feature to:%d\n",wlanfeature);
                    }
                    ptr = malloc( strlen(NVRAM_WLAN_FEATURE_TAG)+sizeof(unsigned char));
                    if ( ptr == NULL ) {
                        printf("Memory Allocation Failure\n");
                        return 1;
                    }
                    memset( ptr, 0,  strlen(NVRAM_WLAN_FEATURE_TAG)+sizeof(unsigned char) );
                    strncpy(ptr, NVRAM_WLAN_FEATURE_TAG, strlen(NVRAM_WLAN_FEATURE_TAG) );
                    *(unsigned char *)&ptr[strlen(NVRAM_WLAN_FEATURE_TAG)]=wlanfeature;
                    /*Save to flash*/
                    boardIoctl(BOARD_IOCTL_FLASH_WRITE,
                               NVRAM,
                               ptr,
                               strlen(NVRAM_WLAN_FEATURE_TAG)+sizeof(unsigned char),
                               0, 0);
                    free(ptr);
                    printf("\n #### NOTE:Reboot is needed to get Feature work!##### \n");
                }
            }

        } else
            dbg=0;

        show_feature=1;
    }


    /* Cleanup the Cal data Nvram*/
    if ( nvramClean ) {
        if(b_nand && b_m ) {
            unlink(WL_SROM_CUSTOMER_FILE);
            unlink(WL_SROM_DEFAULT_FILE);
        } else {
            ptr = malloc( sizeof(params) );
            size = 0;
            if ( ptr == NULL ) {
                printf("Memory Allocation Failure\n");
                return 1;
            }

            memset( ptr, 0, sizeof(params) );
            strncpy(ptr, NVRAM_WLAN_TAG, strlen(NVRAM_WLAN_TAG) );

            /*Save to flash*/
            boardIoctl(BOARD_IOCTL_FLASH_WRITE,
                       NVRAM,
                       ptr,
                       strlen(NVRAM_WLAN_TAG),
                       0, 0);
            free(ptr);
        }
        printf("Cal Data Nvram Clean up\n");
        return 0;
    }

    /*Flash the cal data parameter*/
    if ( !readOnly ) {

        if(b_nand && !b_m) {
            printf("\nSet to Manufacturer Mode)(Feature bit 0x2) to update Nvram!\n");
            return 0;
        }

        fp=fopen(argv[1],"r");
        if ( fp == NULL ) {
            printf("File %s could not be read\n", argv[1]);
            return -1;
        }

        if(!sure_to_continue("Are you sure to overwrite?(Y/N):"))
            return 0;

        /*Parse the input file and create the data structure written into Flash*/
        while( !feof(fp) ) {
            cnt = fscanf(fp, "%s\n", line );

            if ( cnt != 1 ) /*Skip empty line*/
                continue;

            d_printf("line=%s\n", line );
            ptr= strchr(line, '=');

            /*Comment line*/
            if ( ptr == NULL )
                continue;

            *ptr = '\0';

            strncpy( name, line, sizeof(name) );

            d_printf("[%s]=", name);

            /*Start a new Adapter parameters*/
            if ( !strcmp(name, "ID") ) {
                adapter_ptr= (params.adapter + params.adapter_num );

                strcpy((char *)(adapter_ptr->id), (ptr+1) );
                d_printf("[%s]\n", ptr+1);
                adapter_ptr->entry_num =0;

                entry_ptr = adapter_ptr->entry;
                params.adapter_num++;
                if ( params.adapter_num >=MAX_ADAPTERS) {
                    printf("Too many Adapters in the config file\n");
                    fclose(fp);
                    return 1;
                }
                continue;
            }

            if ( params.adapter_num == 0 ) {
                printf("The fist line should be ID=xxx to specify the adapter ID\n");
                fclose(fp);
                return -1;
            }

            /*Fetch a new entry and save it*/
            sscanf( name, "%d", &offset );
            sscanf( (ptr+1), "%x", &value );

            d_printf("[%x]\n", value );

            entry_ptr->offset = (unsigned short)(offset&0xffff) ;
            entry_ptr->value = (unsigned short)(value&0xffff) ;
            adapter_ptr->entry_num++;
            if ( adapter_ptr->entry_num >= MAX_ENTRY_SIZE) {
                printf("Too many Entry in the config file\n");
                fclose(fp);
                return 1;
            }
            entry_ptr++;

        }

        fclose(fp);


        /*Transfer into a memory area, in order to save to flash*/
        ptr = malloc( sizeof(params) );
        size = 0;
        if ( ptr == NULL ) {
            printf("Memory Allocation Failure\n");
            return 1;
        }

        memset( ptr, 0, sizeof(params) );

        strncpy(ptr, NVRAM_WLAN_TAG, strlen(NVRAM_WLAN_TAG) );

        data_ptr = (unsigned short *)( ptr + strlen(NVRAM_WLAN_TAG) ) ;

        *data_ptr = params.adapter_num;
        data_ptr++;

        for ( i=0; i< params.adapter_num; i++ ) {
            d_printf("\nadapter[%d] parames\n", i);
            d_printf("ID=%s\n", params.adapter[i].id );
            strcpy( (char *)data_ptr, (char *)(params.adapter[i].id) );
            data_ptr += 8;

            d_printf("entry_num=%d\n", params.adapter[i].entry_num);
            *data_ptr = params.adapter[i].entry_num;
            data_ptr++;
            for ( j=0; j<params.adapter[i].entry_num; j++ ) {
                d_printf("offset=%d value =0x%x\n",
                         params.adapter[i].entry[j].offset,
                         params.adapter[i].entry[j].value);
                *data_ptr = params.adapter[i].entry[j].offset;
                data_ptr++;
                *data_ptr = params.adapter[i].entry[j].value;
                data_ptr++;
            }
        }

        size = (char *)data_ptr - ptr;

        if ( dbg ) {
            d_printf("mem data[%x]:\n", size);

            for ( i=0; i<size; i++ ) {
                if ( i%16 == 0 ) {
                    d_printf("\n%04x--", i);
                }
                d_printf("[%02x]", (unsigned char)*(ptr+i) );
            }
        }

        if((!b_nand)&&(size>NVRAM_WLAN_PARAMS_LEN-1)) {
            printf("\n\n==NOTE:there is limited storage for calibration data==\n");
            printf("==only %d bytes out of %d  will be saved        ==\n\n",
                   NVRAM_WLAN_PARAMS_LEN-1,size);
        }

        if(b_nand) {
            unlink(WL_SROM_CUSTOMER_FILE);
            unlink(WL_SROM_DEFAULT_FILE);
        }

        /*Save to flash*/
        boardIoctl(BOARD_IOCTL_FLASH_WRITE,
                   NVRAM,
                   ptr,
                   size,
                   0, 0);
        free(ptr);
        printf("Overwrite Calibration data done\n\n");
    }

    /* if show_feature OR not NAND and debug,to read from nvram */
    if(show_feature || (!b_nand && dbg)) {
        boardIoctl(BOARD_IOCTL_FLASH_READ,
                   NVRAM,
                   nvram_array,
                   NVRAM_SPACE_SIZE,
                   0, 0);
        if(show_feature)
            printf("\nFeature values:0x%02x\r\n",
                   *(unsigned char *)&nvram_array[WLAN_FEATURE_POS]);
        data_array=&(nvram_array[offsetof(NVRAM_DATA,wlanParams)]);
        al=NVRAM_WLAN_PARAMS_LEN-1;
    }

    if(b_nand & dbg) {
        fp=fopen(WL_SROM_CUSTOMER_FILE,"r");
        if(fp) {
            fseek(fp,0,SEEK_END);
            al=ftell(fp);
            fseek(fp,0L,SEEK_SET);
            if(al<=0) {
                fclose(fp);
                fprintf(stderr,"%s:%d calibration \
                            file size is zero \r\n",__FUNCTION__,__LINE__ );
                unlink(WL_SROM_CUSTOMER_FILE);
                return -1;
            }
        } else {
            printf("No wl calibration data available\n");
            return 0;
        }

        if(al>NVRAM_SPACE_SIZE) {
            data_array=malloc(al);
            if(!data_array) {
                fprintf(stderr,"%s:%d could not allocate %d memory \r\n",__FUNCTION__,__LINE__,al);
                fclose(fp);
                return -1;
            }
        } else
            data_array=nvram_array;

        fread(data_array,1,al,fp);
        fclose(fp);
    }

    if(dbg && al>2 ) {
        if(show) {
            Params_struct *pAdapters=(Params_struct *)data_array;
            cnt=0;
            if (pAdapters->adapter_num >=MAX_ADAPTERS) {
                d_printf("Too many Adapters in the config file\n");
                return 1;
            }
            d_printf("\n#Calibration Data for %d adapters on Board\n#======\n",pAdapters->adapter_num);
            for ( i=0; i<pAdapters->adapter_num; i++) {
                if(cnt>=al) break;
                adapter_ptr=(Adapter_struct *)((char *)pAdapters->adapter+cnt);
                d_printf("ID=%s\n",adapter_ptr->id);
                for(j=0; j<adapter_ptr->entry_num; j++)
                    d_printf("%d=%x\n",adapter_ptr->entry[j].offset, adapter_ptr->entry[j].value);
                cnt += SI_DEVPATH_BUFSZ + sizeof(unsigned short)+ adapter_ptr->entry_num*sizeof(Entry_struct);
                d_printf("\n");
            }
            d_printf("#=====\n\n");
        } else {
            d_printf("\nCalibration Data:\n======");
            for ( i=0; i<al; i++ ) {
                if ( i%16 == 0 ) {
                    d_printf("\n%04x--", i);
                }
                d_printf("[%02x]", (unsigned char)data_array[i] );
            }
            d_printf("\n======\n\n");
        }
    }
    if(b_nand && al>NVRAM_SPACE_SIZE)
        free(data_array);
    return 0;
}


int main(int argc, char ** argv)
{
    int c;
    while ((c = getopt (argc, argv, "s:mhd:")) != -1) {
        switch(c) {
        case 's':
            tftpserver=optarg;
            break;
        case 'd':
            nvram_map_dir=optarg;
            break;
        case 'm':
            bypass_prompt=1;
            break;
        case 'h':
            break;
        default:
            break;
        }
    }

    if(argc<=optind) {
        help();
        return 1;
    }

    nand_exist();

    if (strstr(argv[optind],".bin") || !strncmp(argv[optind],"Recover",7)) {
        int ret=0;
        if(!b_nand) {
            printf("Just work for NAND system,OR you may not give partition 3 assigned size\n");
            return -1;
        }

        b_m = nand_indicator_value&WLAN_MFG_PARTITION_MFGSET;

        if( !b_m)  {
            printf("\nSet to Manufacturer Mode)(Feature bit 0x2) to clean!\n");
            return 0;
        }

        if(!sure_to_continue("Are you sure to overwrite?(Y/N):"))
            return 0;

        ret=handle_new_fwnv_update(argv[optind]);
        switch(ret) {
        case 0:
            printf("Done!!\n");
            break;
        case ERR_FILENAME_TOOLONG:
            fprintf(stderr, "filename is too long\n");
            break;
        case ERR_RUNTFTP_CMD:
            fprintf(stderr, "Failed to get file from tftpserver\n");
            break;
        case ERR_REMOUNT_RW:
            fprintf(stderr,"Failed to remount fs to rw\n");
            break;
        case ERR_REMOUNT_RO:
            fprintf(stderr,"Failed to remount fs to ro\n");
            break;
        }
        return ret;
    }
    else  {

        if(!partition3_has_size) {
            b_nand=0;
            b_m=1;
        } else if(b_nand)
            b_m = nand_indicator_value&WLAN_MFG_PARTITION_MFGSET;
        return handle_legacy_fwnv_update(argc+1-optind,argv+optind-1);
    }

}



