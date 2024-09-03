/***********************************************************************
 *
 *  Copyright (c) 2021  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2012:DUAL/GPL:standard
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
 *
 ************************************************************************/

#include "soc/mcm/enum_max.h"
#include "soc/mcm/enum_types.h"
#include "soc/mem.h"
#include "soc/memory.h"
#include "soc/field.h"
#include <soc/mcm/allenum.h>
#include "soc/mcm/memregs.h"
#include "soc/feature.h"
#include "soc/68880/bchp_regs_int.h"
#include "xflow_macsec_defs.h"
#include "xflow_macsec.h"
#include "xflow_macsec_esw_defs.h"
#include "xflow_macsec_common.h"
#include "xflow_macsec_firelight.h"
#include "xflow_macsec_cfg_params.h"
#include "macsec_dev.h"
#include "macsec_types.h"
#include "macsec_macros.h"
#include "macsec_debug.h"

extern soc_mem_info_t *soc_memories[];
extern int soc_mem_read_slim(int unit, soc_mem_t mem, int mem_entry, void *entry_data);
/*
 * Given the string identifier of the desired memory table,
 * dump the table for debug.
 */
void debug_dump_table(macsec_dev_t *mdev, unsigned char *mem_name, int index)
{
    int x, i, bfield = 0;
    unsigned char mem_entry[512] = {};
    soc_mem_t mem;
    int table_size;
    soc_mem_info_t *meminfo;
    int scan = 1;


    printk("**************************** Dumping table: %s *******************************\n", mem_name);

    if(!strncmp(mem_name, "TDM_CALENDAR", 12))
    {
        scan = 32;
        mem = MACSEC_TDM_CALENDARm;
    }
    else if(!strncmp(mem_name, "TDM_2_CALENDAR", 14))
        mem = MACSEC_MACSEC_TDM_2_CALENDARm;
    else if(!strncmp(mem_name, "ESEC_SC_TABLE", 13))
        mem = ESEC_SC_TABLEm;
    else if(!strncmp(mem_name, "ESEC_SA_TABLE", 13))
        mem = ESEC_SA_TABLEm;
    else if(!strncmp(mem_name, "ESEC_SA_HASH_TABLE", 18))
        mem = ESEC_SA_HASH_TABLEm;
    else if(!strncmp(mem_name, "SUB_PORT_MAP_TABLE", 18))
        mem = SUB_PORT_MAP_TABLEm;
    else if(!strncmp(mem_name, "SUB_PORT_POLICY_TABLE", 21))
        mem = SUB_PORT_POLICY_TABLEm;
    else if(!strncmp(mem_name, "ISEC_SC_TABLE", 13))
        mem = ISEC_SC_TABLEm;
    else if(!strncmp(mem_name, "ISEC_SA_TABLE", 13))
        mem = ISEC_SA_TABLEm;
    else if(!strncmp(mem_name, "ISEC_SA_HASH_TABLE", 18))
        mem = ISEC_SA_HASH_TABLEm;
    else if(!strncmp(mem_name, "ISEC_SP_TCAM_KEY", 16))
        mem = ISEC_SP_TCAM_KEYm;
    else if(!strncmp(mem_name, "ISEC_SP_TCAM_MASK", 17))
        mem = ISEC_SP_TCAM_MASKm;
    else if(!strncmp(mem_name, "ISEC_SC_TCAM", 12))
        mem = ISEC_SC_TCAMm;
    else if(!strncmp(mem_name, "ESEC_MIB_MISC", 13))
        mem = ESEC_MIB_MISCm;
    else if(!strncmp(mem_name, "ESEC_MIB_ROLLOVER_FIFO", 22))
        mem = ESEC_MIB_ROLLOVER_FIFOm;
    else if(!strncmp(mem_name, "ESEC_MIB_SC_UNCTRL", 18))
        mem = ESEC_MIB_SC_UNCTRLm;
    else if(!strncmp(mem_name, "ESEC_MIB_SC_CTRL", 16))
        mem = ESEC_MIB_SC_CTRLm;
    else if(!strncmp(mem_name, "ESEC_MIB_SC", 11))
        mem = ESEC_MIB_SCm;
    else if(!strncmp(mem_name, "ESEC_MIB_SA", 11))
        mem = ESEC_MIB_SAm;
    else if(!strncmp(mem_name, "ISEC_SPTCAM_HIT_COUNT", 21))
        mem = ISEC_SPTCAM_HIT_COUNTm;
    else if(!strncmp(mem_name, "ISEC_SCTCAM_HIT_COUNT", 21))
        mem = ISEC_SCTCAM_HIT_COUNTm;
    else if(!strncmp(mem_name, "ISEC_PORT_COUNTERS", 18))
        mem = ISEC_PORT_COUNTERSm;
    else if(!strncmp(mem_name, "ISEC_MIB_SP_UNCTRL", 18))
        mem = ISEC_MIB_SP_UNCTRLm;
    else if(!strncmp(mem_name, "ISEC_MIB_SP_CTRL_1", 18))
        mem = ISEC_MIB_SP_CTRL_1m;
    else if(!strncmp(mem_name, "ISEC_MIB_SP_CTRL_2", 18))
        mem = ISEC_MIB_SP_CTRL_2m;
    else if(!strncmp(mem_name, "ISEC_MIB_SC", 11))
        mem = ISEC_MIB_SCm;
    else if(!strncmp(mem_name, "ISEC_MIB_SA", 11))
        mem = ISEC_MIB_SAm;
    else
    {
        printk("ERROR: Unrecognized memory!\n");
        return;
    }

    for(x = 0; x < scan; x++)
    {
        printk("Table Index = %d\n",(x + index));
        soc_mem_read_slim(mdev->macsec_unit, mem, index + x, (void *)mem_entry);
        meminfo = soc_memories[mem];

        table_size = meminfo->data_bits;
    
        if(meminfo->data_bits % 8)
            table_size += (8 - (meminfo->data_bits % 8));

        bfield = (table_size - 1);
        table_size = (table_size / 8);
//        for(i = table_size; i > 0; i--) {
//            printk(KERN_CONT "|  %02d   ", i);
//        }
//                printk(KERN_CONT "|\n");
        printk(KERN_CONT "| %02d - %02d |\n", table_size, 1);
//        for(i = table_size; i > 0; i--)
//        {
//            printk(KERN_CONT "|%3d:%3d", bfield, bfield - 7);
//            bfield -= 8;
//            if ((i % 32) == 0)
//                printk(KERN_CONT "|\n");
//        }
        printk(KERN_CONT "| %03d:%03d |\n", bfield, 0);
        printk(KERN_CONT "|");
        for(i = table_size; i > 0; i--)
            printk(KERN_CONT "%02X", mem_entry[i - 1]);
        printk(KERN_CONT "|\n");
    }
    printk("**************************************************************************\n");

}

enum {
    DBG_TABLE,
    DBG_MIBS,
    DBG_CONFIG,
};

int xflow_macsec_debug_mode_handler(macsec_dev_t *mdev, int type, char *name, int mem_idx)
{
//    int i;
//    int mem_sel = 0;
//    int mem_size = 0;
//    soc_ubus_reg_s reg;
//    int reg_start = 0;
//    int reg_end = 0;
//    unsigned int rval;

    switch (type) {
    case DBG_TABLE:
        debug_dump_table(mdev, name, mem_idx);
        break;
    case DBG_MIBS:
        if(strncmp(name, "isec", 4) == 0)
        {
            debug_dump_table(mdev, "ISEC_SPTCAM_HIT_COUNT", mem_idx);
            debug_dump_table(mdev, "ISEC_SCTCAM_HIT_COUNT", mem_idx);
            debug_dump_table(mdev, "ISEC_PORT_COUNTERS", mem_idx);
            debug_dump_table(mdev, "ISEC_MIB_SP_UNCTRL", mem_idx);
            debug_dump_table(mdev, "ISEC_MIB_SP_CTRL_1", mem_idx);
            debug_dump_table(mdev, "ISEC_MIB_SP_CTRL_2", mem_idx);
            debug_dump_table(mdev, "ISEC_MIB_SC", mem_idx);
            debug_dump_table(mdev, "ISEC_MIB_SA", mem_idx);
        }
        if(strncmp(name, "esec", 4) == 0)
        {
            debug_dump_table(mdev, "ESEC_MIB_MISC", mem_idx);
            debug_dump_table(mdev, "ESEC_MIB_SC_UNCTRL", mem_idx);
            debug_dump_table(mdev, "ESEC_MIB_SC_CTRL", mem_idx);
            debug_dump_table(mdev, "ESEC_MIB_SC", mem_idx);
            debug_dump_table(mdev, "ESEC_MIB_SA", mem_idx);
        }
        break;
    case DBG_CONFIG:
        if(strncmp(name, "isec", 4) == 0)
        {
            debug_dump_table(mdev, "SUB_PORT_MAP_TABLE", mem_idx);
            debug_dump_table(mdev, "SUB_PORT_POLICY_TABLE", mem_idx);
            debug_dump_table(mdev, "ISEC_SC_TABLE", mem_idx);
            debug_dump_table(mdev, "ISEC_SA_TABLE", mem_idx);
            debug_dump_table(mdev, "ISEC_SA_HASH_TABLE", mem_idx);
            debug_dump_table(mdev, "ISEC_SP_TCAM_KEY", mem_idx);
            debug_dump_table(mdev, "ISEC_SP_TCAM_MASK", mem_idx);
            debug_dump_table(mdev, "ISEC_SC_TCAM", mem_idx);
        }
        if(strncmp(name, "esec", 4) == 0)
        {
            debug_dump_table(mdev, "ESEC_SC_TABLE", mem_idx);
            debug_dump_table(mdev, "ESEC_SA_TABLE", mem_idx);
            debug_dump_table(mdev, "ESEC_SA_HASH_TABLE", mem_idx);
        }
        break;
    }
#if 0
        else if(strncmp(argv[2], "reg_block", 5) == 0)
        {
            if(argc < 5)
                return 0;

            sscanf(argv[3], "0x%x", &reg_start);
            sscanf(argv[4], "0x%x", &reg_end);

            printk("dumping reg block: from %08x to %08x\n", reg_start, reg_end);
            for(i = reg_start; i < reg_end; i += 4)
            {
                reg.offset = (i - 0xd0000000);
                soc_ubus_reg32_get(0, &reg, REG_PORT_ANY, &rval);
                printk("%08x: %08x\n",(reg.offset + 0xd0000000), rval);
            }
        }
    }
#endif
    return 0;
}
