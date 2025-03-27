/***********************************************************************
 * <:copyright-BRCM:2007:DUAL/GPL:standard
 * 
 *    Copyright (c) 2007 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // types
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <mtd/mtd-user.h>

#include "bcm_flashutil.h"
#include "bcm_flashutil_nor.h"


int norIsNewFlashLayout(void)
{
    FILE *fp;
    char line[256]={0};
    char *spinorflash = SPI_NOR_FLASH_NAME;
    int new_layout = 0;
    
    fp = fopen("/proc/cmdline","r");
    if (fp == NULL)
    {
       fprintf(stderr, "Could not open /proc/cmdline");
       return 0;
    }
    
    /* Search for 'spinorflash0' from cmdline*/
    while(fgets(line, sizeof(line), fp))
    {
        if(strstr(line, spinorflash))
        {
            new_layout = 1;
            break;
        }
    }
    fclose(fp);
    return new_layout;
}

uint64_t spinorGetAvailSpace(const char* mtdpart) 
{
    uint64_t loader_size;
    char sysfs_path[128];
    sprintf(sysfs_path, "/sys/class/mtd/%s/size",mtdpart);
    loader_size = getSysfsBytes(sysfs_path);
    return loader_size;
}

static void print_imgupdate_progress()
{
    fprintf(stderr, ".");
}

int norWriteFileToMtdPar(const char* filename,const char* mtdpart)
{
    mtd_info_t  mtd_info;
    int mtd_fd;
    erase_info_t ei;
    char* buff;
    int file_fd;
    int more =1;
    int read_couter=0;
    int ret = 0;

    if ((mtd_fd = open(mtdpart, O_SYNC|O_RDWR)) < 0)
    {
       fprintf(stderr, "ERROR!!! Could not open %s\n", mtdpart);
       return (-1);
    }
    ioctl(mtd_fd, MEMGETINFO, &mtd_info);
    ei.length = mtd_info.erasesize;
    if ( (buff = malloc(mtd_info.erasesize)) == 0)
    {
         fprintf(stderr, "ERROR!!! Could not allocate buffer!\n");
         close(mtd_fd);
         return (-1);
    }
    if ((file_fd = open(filename, O_RDONLY)) < 0)
    {
        fprintf(stderr, "ERROR!!! Could not open %s\n", filename);
        close(mtd_fd);
        free(buff);
        return (-1);
    }
    for(ei.start = 0; ei.start < mtd_info.size; ei.start += mtd_info.erasesize)
    {
        ioctl(mtd_fd, MEMUNLOCK, &ei);
        if( ioctl(mtd_fd, MEMERASE, &ei) < 0)
        {
            fprintf(stderr, "ERROR!!! Fail to erase partition %s\n",mtdpart);
            ret = -1;
            break;
        }
        if(more)
        {
            read_couter = read(file_fd, buff, mtd_info.erasesize);
            if( read_couter > 0)
            {
                if( write(mtd_fd,buff,read_couter) != read_couter)
                {
                    fprintf(stderr,"ERROR!!! Fail to flash into  partition %s\n",mtdpart);
                    ret = -1;
                    break; 
                }
            }
            if((read_couter == 0) || (read_couter < mtd_info.erasesize))
            more = 0;
        }
        print_imgupdate_progress();
    }
    close(mtd_fd);
    close(file_fd);
    free(buff);
    return ret;
}
