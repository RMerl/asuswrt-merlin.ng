/*
<:copyright-BRCM:2015:GPL/GPL:standard

   Copyright (c) 2015 Broadcom 
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
#include <linux/init.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/of_fdt.h>
#include <linux/sched.h>
#include <linux/sizes.h>

#ifndef CONFIG_MIPS
#include <asm/pgtable.h>
#include <asm/system_misc.h>
#endif

#include <board.h>

extern unsigned long memsize;
extern void check_if_rootfs_is_set(char *cmdline);
#ifdef CONFIG_BCM_CFE_XARGS_EARLY
extern void __init bl_xparms_setup(const unsigned char* blparms, unsigned int size);
#endif

#define BRCM_BLPARMS_PROP "brcm_blparms"
#ifdef CONFIG_OF
extern const void *__init of_get_flat_dt_prop(unsigned long node, const char *name,
                                       int *size);

static int __init dt_get_memory_total_size(unsigned long node, uint64_t* size)
{
    const __be32 *endp;
    const __be32 *reg;
    int regsize;
    uint64_t base;

    *size = 0x0;
    reg = of_get_flat_dt_prop(node, "reg", &regsize);
    if (reg == NULL)
        return -1;
    endp = reg + (regsize / sizeof(__be32));
    while ((endp - reg) >= (dt_root_addr_cells + dt_root_size_cells)) {
        base = dt_mem_next_cell(dt_root_addr_cells, &reg);
        *size += dt_mem_next_cell(dt_root_size_cells, &reg);
    }

    return 0;
}

int __init dt_get_memory_prop(unsigned long node, uint64_t* base, uint64_t* size)
{
    const __be32 *endp;
    const __be32 *reg;
    int regsize;

    reg = of_get_flat_dt_prop(node, "reg", &regsize);
    if (reg == NULL)
        return -1;
    endp = reg + (regsize / sizeof(__be32));
    while ((endp - reg) >= (dt_root_addr_cells + dt_root_size_cells)) {
        *base = dt_mem_next_cell(dt_root_addr_cells, &reg);
        *size = dt_mem_next_cell(dt_root_size_cells, &reg);
    }

    return 0;
}

/* This function will retrieve a specific data blob that has been appended to 
 * the root node of the dtb
 */
int __init bcm_get_root_propdata( const char * prop_name, char * data, int prop_size )
{
    int ret_size = 0;
    unsigned long node = of_get_flat_dt_root(); 
    uint8_t* prop_data = (uint8_t*)of_get_flat_dt_prop(node, prop_name, &ret_size);
    if( prop_data ) 
    {
        memcpy(data, prop_data, (prop_size < ret_size ? prop_size:ret_size));
        return 0;
    }
    else
    {
        return -1;
    }
}
EXPORT_SYMBOL(bcm_get_root_propdata);

/* This function will retrieve a specific data blob will return the pointer  */
int __init bcm_get_root_propdata_raw( const char * prop_name, char ** data, int *prop_size )
{
    int ret_size = 0;
    unsigned long node = of_get_flat_dt_root(); 
    uint8_t* prop_data = (uint8_t*)of_get_flat_dt_prop(node, prop_name, &ret_size);
    if( prop_data ) 
    {
        if(data != NULL && prop_size != NULL)
        {
            data[0]=prop_data;
            prop_size[0]=ret_size;
            return 0;
        }
    }
    return -1;
}
EXPORT_SYMBOL(bcm_get_root_propdata_raw);


/* This function scan through the device tree blob for any system related node.
 * data such as memory size, reserved memory, blparms and etc. Device related 
 * node should be handled by the corresponding device driver.
 */
int __init bcm_early_scan_dt(unsigned long node, const char *uname, int depth, void *data)
{
    uint64_t size = 0;
    int blsize = 0;
    uint8_t* bcm_blparms_buf = (uint8_t*)bcm_get_blparms();
    uint8_t* blparms = NULL;


    if ( node == of_get_flat_dt_root() ) { /* root node */ 
         blparms = (uint8_t*)of_get_flat_dt_prop(node, BRCM_BLPARMS_PROP, &blsize);
         if( blparms ) {
            memcpy(bcm_blparms_buf, blparms, blsize);
#ifdef CONFIG_BCM_CFE_XARGS_EARLY
            bl_xparms_setup(bcm_blparms_buf, blsize);
#endif
        }
    }

    if ( strncmp(uname, "memory", 6) == 0 ) {
        const char *type = of_get_flat_dt_prop(node, "device_type", NULL);
        if( type != NULL && strcmp(type, "memory") == 0 && dt_get_memory_total_size(node, &size) == 0 ) {
            memsize = size;
            /* found the the memory node and return 1 to finish the scan */
            return 1;
        }
    }

    return 0;
}

#endif
