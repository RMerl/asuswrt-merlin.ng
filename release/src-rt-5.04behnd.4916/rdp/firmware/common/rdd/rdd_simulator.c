/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
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
#ifdef RDP_SIM
#include "bdmf_system.h"
#include "rdd.h"
#include "access_macros.h"
#include "rdp_mm.h"
#include "rdd_simulator.h"
#include "rdd_common.h"
#include "rdp_drv_natc.h"
#include "rdp_drv_xpm.h"
#include "xrdp_autogens.h"
#include <system/SharedMem.h>

uint8_t *cpu_rx_ring_base_addr_ptr;
extern bdmf_phys_addr_t g_rsv_phys_base_addr;
extern void *g_rsv_base_addr;
extern uint32_t g_rsv_alloc_total_size;

#define SET_FORMAT "set %s %08x %08x\n"

void save_table(FILE *table_f, char *table_name, uint32_t *table, uint32_t table_offset, uint32_t table_size,
    int skip_empty)
{
    uint32_t i, val;

    for (i = 0; i < table_size / 4; i++)
    {
        val = MGET_32(table + i);
        if (!val && skip_empty)
            continue;
        fprintf(table_f, SET_FORMAT, table_name, (unsigned int)(table_offset + i * 4), val);
    }
}

#define SAVE_DDR_TABLE(table, offset, size) save_table(ddr_tables, "ddr", table, (uint32_t)(offset), size, 1)

int rdd_sim_save_ddr_tables(void)
{
    FILE *ddr_tables;

    ddr_tables = fopen("ddr_tables.dfp", "w+b");
    if (!ddr_tables)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to open ddr_tables.dfp");

    SAVE_DDR_TABLE(g_rsv_base_addr, g_rsv_phys_base_addr, g_rsv_alloc_total_size);

    fclose(ddr_tables);
    return 0;
}

int rdd_sim_save_hw_cfg(void)
{
    FILE *sim_cfg;

    sim_cfg = fopen("sim_cfg", "w+b");
    if (!sim_cfg)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to open sim_cfg");

    fclose(sim_cfg);
    return 0;
}

void rdd_sim_save_tx_pointers(uint16_t *buffer)
{
    ;
}

uintptr_t xrdp_virt2phys(const ru_block_rec *ru_block, uint8_t addr_idx)
{
    return RDD_IO_VIRT_TO_PHYS(ru_block->addr[addr_idx]);
}


#define DRV_COMMON_SHMEM_NO_FIXED_OFFSET 0xffffffff



static int _get_env_var(char *varname, char loc[], int loc_len, char *def)
{
    char *ptr = getenv(varname);

    if (ptr)
    {
        if (strlen(ptr) < loc_len)
            strcpy(loc, ptr);
        else
            return BDMF_ERR_OVERFLOW;
    }
    else
        strcpy(loc, def);

    return 0;
}


/* allocate memory segments for simulation */
int rdd_sim_alloc_segments(void)
{
    bdmf_phys_addr_t phy;

    char shmemdir[512];

    if (_get_env_var("SHMEM_DIR", shmemdir, sizeof(shmemdir), "."))
        return BDMF_ERR_OVERFLOW;

#if 1
#if (CHIP_VER < RDP_GEN_100)
    sharedmem_create_x(shmemdir, SIM_MEM_SIZE + sizeof(Status_t) + DEFAULT_LAYOUT_AREA_SIZE_XRDP, /* emulator */ 0, 
                                       /* layout_size */ DEFAULT_LAYOUT_AREA_SIZE_XRDP);
#else
    sharedmem_create_x(shmemdir, SIM_MEM_SIZE + sizeof(Status_t) + DEFAULT_LAYOUT_AREA_SIZE_LOTUS + 
                    0xB00000 + 0xb00000/* Extra space for TM blocks defined in surplus in the simulator */, 
                                        /* emulator */ 0, /* layout_size */ DEFAULT_LAYOUT_AREA_SIZE_LOTUS);

    sharedmem_non_chip_alloc("WAKEUP_TASKS", -1, "", "uint8_t", sizeof(uint8_t), (NUM_OF_PROCESSING_RUNNERS + NUM_OF_TM_RUNNERS) * 16);
#endif

    soc_base_address = sharedmem_alloc(-1, "FULL_REG_SPACE", -1, "", "Runner_t", /*struct_size*/ SIM_MEM_SIZE, /*#*/ 1,
                                       /*hw_addr*/ SOC_BASE_ADDRESS, /*endian*/ 0, 
                                       DRV_COMMON_SHMEM_NO_FIXED_OFFSET, /* overlap_check */ 1);
    if (!soc_base_address)
        return BDMF_ERR_NORES;

#else /* CRDP-TODO */

    sharedmem_create(shmemdir, 0x9c00000 /*CRDP_BLOCK_SIZE: TODO*/, "REDWOOD", -1, 1, 0);

    soc_base_address = (uint8_t *)sharedmem_getBasePtr() + sharedmem_getAllocatedSize();

    struct block_ptr_t 
    {
        char *block_name;
        char *sub_block_name;
        const ru_block_rec *blk_ptr;
        char *struct_name;
        uint32_t big_endian;
    };
    
    struct block_ptr_t blocks[] = { 
                                    { "RNR_PP", "REGS",  &RU_BLK(CRDP_RCG_RNR_REGS),  "rdp_crdp_rcg_rnr_regs_t",    0 },
                                    { "RNR_PP", "CNTXT", &RU_BLK(CRDP_RCG_RNR_CNTXT), "rdp_crdp_rcg_rnr_cntxt_t",   1 },
                                    { "RNR_PP", "CMN",   &RU_BLK(CRDP_RCG_RNR_CMN),   "rdp_crdp_rcg_rnr_cmn_t",     0 },
                                    { "RNR_PP", "INST",  &RU_BLK(CRDP_RCG_RNR_INST),  "rdp_crdp_rcg_rnr_inst_t",    0 },
                                    { "RNR_PP", "PRED",  &RU_BLK(CRDP_RCG_RNR_PRED),  "rdp_crdp_rcg_rnr_pred_t",    0 },
                                    { "RNR_PP", "MEM",   &RU_BLK(CRDP_RCG_RNR_MEM),   "rdp_crdp_rcg_rnr_mem_t",     0 },
                                    { "RNR_TM", "REGS",  &RU_BLK(CRDP_EGR_RNR_REGS),  "rdp_crdp_egr_rnr_regs_t",    0 },
                                    { "RNR_TM", "CNTXT", &RU_BLK(CRDP_EGR_RNR_CNTXT), "rdp_crdp_egr_rnr_cntxt_t",   1 },
                                    { "RNR_TM", "CMN",   &RU_BLK(CRDP_EGR_RNR_CMN),   "rdp_crdp_egr_rnr_cmn_t",     0 },
                                    { "RNR_TM", "INST",  &RU_BLK(CRDP_EGR_RNR_INST),  "rdp_crdp_egr_rnr_inst_t",    0 },
                                    { "RNR_TM", "PRED",  &RU_BLK(CRDP_EGR_RNR_PRED),  "rdp_crdp_egr_rnr_pred_t",    0 },
                                    { "RNR_TM", "MEM",   &RU_BLK(CRDP_EGR_RNR_MEM),   "rdp_crdp_egr_rnr_mem_t",     0 },
                                    { "BAC_IF", "",      &RU_BLK(BAC_IF),             "rdp_bac_if_t",               0 },
                                    { "BBH",    "RX",    &RU_BLK(BBH_RX),             "rdp_bbh_rx_t",               0 },
                                    { "BUFMNG", "REGS",  &RU_BLK(BUFMNG),             "rdp_bufmng_t",               0 },
                                    { "CNPL",   "",      &RU_BLK(CNPL),               "rdp_crdp_cnpl_t",            0 },
                                    { "BBHTX_CP", "REGS", &RU_BLK(CRDP_DS_CMN_BBH_TX), "rdp_crdp_ds_cmn_bbh_tx_t",  0 },
                                    { "PFB",    "DS_CMN", &RU_BLK(CRDP_DS_CMN_PFB),   "rdp_crdp_ds_cmn_pfb_t",      0 },
                                    { "SBPM",   "DS_CMN", &RU_BLK(CRDP_DS_CMN_SBPM),  "rdp_crdp_ds_cmn_sbpm_t",     0 },
                                    { "BBHTX",  "REGS",  &RU_BLK(CRDP_EGR_BBH_TX),    "rdp_crdp_egr_bbh_tx_t",      0 },
                                    { "PFB",    "EGR",   &RU_BLK(CRDP_EGR_PFB),       "rdp_crdp_egr_pfb_t",         0 },
                                    { "PFB",    "ING",   &RU_BLK(CRDP_ING_PFB),       "rdp_crdp_ing_pfb_t",         0 },
                                    { "SBPM",   "ING",   &RU_BLK(CRDP_ING_SBPM),      "rdp_crdp_ing_sbpm_t",        0 },
                                    { "DSPTCHR", "",     &RU_BLK(DSPTCHR),            "rdp_dsptchr_t",              0 },
                                    { "DSPTCHR_CMN", "", &RU_BLK(DSPTCHR_CMN),        "rdp_dsptchr_cmn_t",          0 },
                                    { "FPM",    "REGS",  &RU_BLK(FPM),                "rdp_fpm_t",                  0 },
                                    { "FPMINI", "REGS",  &RU_BLK(FPM_FPMINI),         "rdp_fpm_fpmini_t",           0 },
                                    { "GSU",    "REGS",  &RU_BLK(GSU_REG),            "rdp_gsu_reg_t",              0 },
                                    { "GSU",    "MEM",   &RU_BLK(GSU_MEM),            "rdp_gsu_mem_t",              0 },
                                    { "HASH", "",        &RU_BLK(HASH),               "rdp_hash_t",                 0 },
                                    { "PSRAM",  "MEM",   &RU_BLK(PSRAM_MEM),          "rdp_psram_mem_t",            0 },
                                    { "QM",     "REGS",  &RU_BLK(QM),                 "rdp_qm_t",                   0 },
                                    { "TCAM", "",        &RU_BLK(TCAM),               "rdp_tcam_t",                 0 },
                                  };

    /* TODO: Check if these blocks need to be added:    CRDP_DS_CMN_PSRAM 
     *                                                  CRDP_DS_CMN_UBUS_RESPONDER_BRDG
     *                                                  CRDP_EGR_CNPL
     *                                                  CRDP_EGR_UBUS_RESPONDER_BRDG
     *                                                  CRDP_ING_PSRAM
     *                                                  CRDP_ING_UBUS_RESPONDER_BRDG
     *                                                  CRDP_RCG_UBUS_RESPONDER_BRDG
     *                                                  NAT_CACHE */

    int block_num = sizeof(blocks)/sizeof(struct block_ptr_t);

    int i, b;

    for (b = 0; b < block_num; b++)
    {
        for (i = 0; i < blocks[b].blk_ptr->addr_count; i++)
        {
            if (!sharedmem_chipalloc(blocks[b].block_name, i, blocks[b].sub_block_name, blocks[b].struct_name,
                                              blocks[b].blk_ptr->size[0], 1, blocks[b].blk_ptr->addr[i], blocks[b].big_endian,
                                              DRV_COMMON_SHMEM_NO_FIXED_OFFSET, 1))
            {
                return BDMF_ERR_NORES;
            }
        }
    }

    sharedmem_non_chip_alloc("WAKEUP_TASKS", -1, "", "uint8_t", sizeof(uint8_t), (NUM_OF_PROCESSING_RUNNERS + NUM_OF_TM_RUNNERS) * 16);
#endif
    cpu_rx_ring_base_addr_ptr = rdp_mm_aligned_alloc(RDD_CPU_RING_DESCRIPTORS_TABLE_SIZE * RDD_CPU_RING_BYTE_SIZE,
      &phy);
    if (!cpu_rx_ring_base_addr_ptr)
        return BDMF_ERR_INTERNAL;

    return 0;
}

int _segment_file_init(const char *file_name, const char *mode, uint8_t *segment_mem, int segment_size)
{
    FILE *segment_file;

    segment_file = fopen(file_name, mode);

    if (!segment_file)
    {
        printf("Error: Can't open %s\n", file_name);
        return -1;
    }

    fwrite(segment_mem, sizeof(uint8_t), segment_size, segment_file);

    fclose(segment_file);
    return 0;
}

int segment_file_init(const char *file_name, const char *mode, int segment, int segment_size)
{
    return _segment_file_init(file_name, mode, (uint8_t *)DEVICE_ADDRESS(segment), segment_size);
}
#endif /*RDP_SIM*/
