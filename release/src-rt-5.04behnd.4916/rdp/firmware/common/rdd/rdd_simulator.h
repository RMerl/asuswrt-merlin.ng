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
#ifndef XRDP
#define SIMULATOR_DDR_RING_OFFSET          0x01B00000
#define SIMULATOR_DDR_RING_NUM_OF_ENTRIES  10

void rdd_sim_save_tx_pointers(uint16_t *buffer);

#define SIMULATOR_DDR_SKB_DATA_POINTERS_OFFSET 0x01A00000
#define SIMULATOR_DDR_SKB_FREE_INDEXES_OFFSET  0x01A10000
#define SIMULATOR_DDR_RING_OFFSET              0x01B00000
#define SIMULATOR_DDR_RING_HOST_BUFFERS_OFFSET 0x01B00780
#define SIMULATOR_DDR_SKB_BUFFERS_OFFSET       0x01B80000
#define SIMULATOR_DDR_RING_NUM_OF_ENTRIES      10
#ifdef DSL_UNIT_TEST
#define SIMULATOR_DDR_PORT_HEADER_BUFFERS_OFFSET 0x01104000
#define SIMULATOR_DDR_WLAN_MCAST_DHD_LIST_OFFSET 0x01110000
#endif

#ifndef G9991_PRV
int rdd_ip_flow_ut(rdd_module_t *module, rdpa_ip_flow_key_t *key, void *context);
#endif
#define SIM_MEM_SIZE 0x100000
#else /* XRDP */
#if defined(BCM_DSL_XRDP)
#define WAN_BBH_BLOCK_SIZE 0x40000
#else
#define WAN_BBH_BLOCK_SIZE 0x4000
#endif

#define SIM_MEM_SIZE (RDP_BLOCK_SIZE + WAN_BBH_BLOCK_SIZE)

#define SIMULATOR_DDR_RING_NUM_OF_ENTRIES      16
#define SIMULATOR_DDR_RING_OFFSET              0x01B00000
#define RDD_CPU_RING_BYTE_SIZE  (sizeof(CPU_RX_DESCRIPTOR_STRUCT) * SIMULATOR_DDR_RING_NUM_OF_ENTRIES)
#endif

#ifndef XRDP
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
#else
#define SEG_CHK_ALLOC(p, s) \
    do {\
        p = (typeof(p))malloc(s);\
        if (!p) \
        { \
            printf("Failed to allocate " # p " (%u bytes)\n", (unsigned)(s));\
            return -1;\
        } \
        memset(p, 0, s); \
    } while (0)

#define SEG_CHK_FREE(p) \
    do {\
        if (p) \
            free((void *)p);\
    } while (0)
#endif


extern uint8_t *soc_base_address;
extern uint32_t natc_lkp_table_ptr;

int rdd_sim_alloc_segments(void);
void rdd_sim_free_segments(void);

int _segment_file_init(const char *file_name, const char *mode, uint8_t *segment_mem, int segment_size);
int segment_file_init(const char *file_name, const char *mode, int segment, int segment_size);

void rdd_save_wifi_dongle_config(void);

#endif /* _RDD_SIMULATOR_H */

