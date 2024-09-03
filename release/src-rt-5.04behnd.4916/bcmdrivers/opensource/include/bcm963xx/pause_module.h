/*
    <:copyright-BRCM:2021:DUAL/GPL:standard
    
       Copyright (c) 2021 Broadcom 
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

#ifndef __PAUSE_H_INCLUDED__
#include <linux/fs.h>

__attribute__((unused)) static void wait_for_any(void)
{
    struct file *fp = NULL;
    char buf[1000];
    loff_t offset = 0;

    fp = filp_open("/dev/console", O_RDONLY, 0);
    if (!fp)
        return;

    printk("hit Enter to continue...\n");
    kernel_read(fp, buf, 1, &offset);
    filp_close(fp, NULL);
}
#endif
