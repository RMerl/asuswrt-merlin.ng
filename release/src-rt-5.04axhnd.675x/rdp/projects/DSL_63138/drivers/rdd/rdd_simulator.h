/*
   <:copyright-BRCM:2013-2016:DUAL/GPL:standard
   
      Copyright (c) 2013-2016 Broadcom 
      All Rights Reserved
   
   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:
   
      As a special exception, the copyright holders of this software give
      you permission to link this software with independent modules, and
      to copy and distribute the resulting executable under terms of your
      choice, provided that you also meet, for each linked independent
      module, the terms and conditions of the license of that module.
      An independent module is a module which is not derived from this
      software.  The special exception does not apply to any modifications
      of the software.
   
   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.
   
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

