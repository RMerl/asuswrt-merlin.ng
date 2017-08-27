/*
<:copyright-BRCM:2002:GPL/GPL:standard

   Copyright (c) 2002 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

#include "board.h"
#include "dsldsp_operation.h"
#include <linux/fs.h>
#include <linux/unistd.h>
#include <linux/delay.h>
#include <asm/uaccess.h>

void enter_and_exit_dsp_reset(void)
{
    unsigned short gpio_dsl_dsp_reset;

    BpGetGpioAttachedDevReset(&gpio_dsl_dsp_reset);
    kerSysSetGpioDir(gpio_dsl_dsp_reset);
    printk("Enter dsl dsp reset.\n");
    kerSysSetGpioState(gpio_dsl_dsp_reset, kGpioInactive);
    udelay(1);
    printk("Exit dsl dsp reset.\n");
    kerSysSetGpioState(gpio_dsl_dsp_reset, kGpioActive);
    udelay(1);
}

int download_dsp_booter(void)
{
    struct file *fp;
    mm_segment_t fs;
    int i=0,j=0,len;
    char data;
    char string[512];
    int strLen=512;
    unsigned int booterSize=0;
    /*int dbgOn=10;*/

    unsigned short sprom_clk, sprom_data;
    BpGetGpioSpromClk(&sprom_clk);
    BpGetGpioSpromData(&sprom_data);

    printk("Loading booter into DSP\n");
    /*init GPIO, configure gpio as output*/
    kerSysSetGpioDir(sprom_data);
    kerSysSetGpioDir(sprom_clk);
    enter_and_exit_dsp_reset();
    /*initial data and clk state is high*/
    kerSysSetGpioState(sprom_data,kGpioActive);
    kerSysSetGpioState(sprom_clk,kGpioActive);
    /*read booter.sprom, and write to DSP*/
    fp=filp_open("/etc/xdsl/booter.sprom",O_RDONLY,0);
    if(!IS_ERR(fp))
    {
        printk("reading booter.sprom\n");
        fs=get_fs();
        set_fs(get_ds());
        fp->f_pos=0;
        while((len=(int)__vfs_read(fp,(void *)string,strLen,&fp->f_pos))>0)
        {
            /*booterSize+=len;*/
            for(;len>0;len--)
            {
                data=string[j];
                for(i=0;i<8;i++) /*bit in Byte*/
                {
                    /*output MSB bit at first*/
                    /*if(data&(0x1<<(7-i))) kerSysSetGpioState(sprom_data,kGpioActive);*/
                    /*reverse bit order*/
                    if(data&(0x1<<i)) kerSysSetGpioState(sprom_data,kGpioActive);
                    else kerSysSetGpioState(sprom_data,kGpioInactive);

                    udelay(1);	/*data setup time*/
                    /*clk falling edge,latch data*/
                    kerSysSetGpioState(sprom_clk,kGpioInactive);
                    udelay(1);
                    kerSysSetGpioState(sprom_clk,kGpioActive);	/*pull high clk*/
                    udelay(1);
                }
                j++;
                booterSize++;
            }
            j=0;
        }
        printk("DSP booter loading finished! booterSize=%d\n",booterSize);
        filp_close(fp,NULL);
        set_fs(fs);
    }
    else printk("open '%s' fail\n","booter.sprom");

    kerSysSetGpioState(sprom_clk,kGpioInactive);
    return 0;
}
