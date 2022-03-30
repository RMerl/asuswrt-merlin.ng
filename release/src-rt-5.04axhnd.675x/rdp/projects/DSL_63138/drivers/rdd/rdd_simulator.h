/*
   <:copyright-BRCM:2013-2016:DUAL/GPL:standard
   
      Copyright (c) 2013-2016 Broadcom 
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

#ifndef _RDD_SIMULATOR_H
#define _RDD_SIMULATOR_H

#include "access_macros.h"
#include "rdp_mm.h"

int rdd_sim_save_ddr_tables(void);
int rdd_sim_save_hw_cfg(void);
int bl_lilac_rdd_save_sim_config(void);
#define SIMULATOR_DDR_RING_OFFSET          0x01B00000
#define SIMULATOR_DDR_RING_NUM_OF_ENTRIES  10

void rdd_sim_save_tx_pointers(uint16_t *buffer);
void bl_lilac_rdd_copy_tx_pointers_table(uint16_t *buffer);

#define SIMULATOR_DDR_SKB_DATA_POINTERS_OFFSET 0x01A00000
#define SIMULATOR_DDR_SKB_FREE_INDEXES_OFFSET  0x01A10000
#define SIMULATOR_DDR_RING_OFFSET              0x01B00000
#define SIMULATOR_DDR_RING_HOST_BUFFERS_OFFSET 0x01B00780
#define SIMULATOR_DDR_SKB_BUFFERS_OFFSET       0x01B80000
#define SIMULATOR_DDR_RING_NUM_OF_ENTRIES      10
#define SIMULATOR_DDR_PORT_HEADER_BUFFERS_OFFSET 0x01104000
#define SIMULATOR_DDR_WLAN_MCAST_DHD_LIST_OFFSET 0x01110000

#define SIM_MEM_SIZE 0x100000

#define SEG_CHK_ALLOC(p, s) \
    do {\
        p = (typeof(p))bdmf_calloc(s);\
        if (!p) \
        { \
            printf("Failed to allocate " # p " (%u bytes)\n", (unsigned)(s));\
            return -1;\
        } \
    } while (0)

#define SEG_CHK_FREE(p) \
    do {\
        if (p) \
        bdmf_free((void *)p);\
    } while (0)


int rdd_sim_alloc_segments(void);
void rdd_sim_free_segments(void);

int _segment_file_init(const char *file_name, const char *mode, uint8_t *segment_mem, int segment_size);
int segment_file_init(const char *file_name, const char *mode, int segment, int segment_size);

void rdd_save_wifi_dongle_config(void);

#endif /* _RDD_SIMULATOR_H */

