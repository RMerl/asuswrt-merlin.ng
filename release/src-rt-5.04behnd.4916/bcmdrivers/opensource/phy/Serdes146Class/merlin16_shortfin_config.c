/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2017:DUAL/GPL:standard

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

/*
 *  Created on: Sep 2017
 *      Author: li.xu@broadcom.com
 */

/*
 * Phy drivers for 10G Active Ethernet Serdes
 */
#include "phy_drv_dsl_serdes.h"
#include "Serdes146Class/phy_drv_merlin16.h"
#include "Serdes146Class/merlin16_shortfin_ucode_image.h"
#include "Serdes146Class/M1_merlin.h"
#include "merlin_shortfin/include/public/merlin16_shortfin_fields_public.h"
#include "serdes_access.h"

#define set_mask_read_data(d) udelay(1000)
#define clr_mask_read_data() udelay(1000)

static int log_level = 2;

#define print_log2(fmt, args...) do { \
    if (log_level >= 2) printk(fmt, ##args); \
} while(0)

static phy_serdes_t *log_serdes;
#define print_log(fmt, args...) do { \
    phy_serdes_t *phy_serdes = phy_drv_serdes146_get_serdes(CoreNum, PortNum); \
    if (!phy_serdes) phy_serdes = log_serdes; else log_serdes = phy_serdes; \
    if (log_level >= 2 && phy_serdes && phy_serdes->print_log) printk(fmt, ##args); \
} while(0)

#define error_log(fmt, args...) do {if (log_level >= 1) printk("*** %s:%d: Waring:", __func__,__LINE__); printk(KERN_CONT fmt, ##args);} while(0)
// #define FATAL_DEBUG
#if !defined(FATAL_DEBUG)
#define fatal_log(fmt, args...) do {if (log_level >= 0) printk("******* %s:%s:%d: FATAL ERROR: ", __FILE__,__func__,__LINE__); printk(KERN_CONT fmt, ##args); BUG();} while(0)
#else
#define fatal_log(fmt, args...) do {if (log_level >= 0) printk("******* %s:%s:%d: FATAL ERROR: ", __FILE__,__func__,__LINE__); printk(KERN_CONT fmt, ##args); while(1) {msleep(3000);}} while(0)
#endif

static int parse_sim_opts(char *str)
{
    int i;
    static struct ss {char *s; int ret;} opt[64] = {
        {"-d VERBOSE", 1},
        {"-d MERLIN_LOAD_FIRMWARE", 1},
        {"-d ML_REFCLK_50", 1},
        {"-d separate_vco", 1},
    };

    for (i=0; i<ARRAY_SIZE(opt); i++)
    {
        if (opt[i].s == 0)
            break;
        if (strcmp(str, opt[i].s) == 0)
            return (opt[i].ret == 1? 1: 0);
    }

    if (i >= ARRAY_SIZE(opt))
        fatal_log("Error:Option array overflow: %s\n", str);
    else {
        opt[i].s = str;
        opt[i].ret = 3;
        print_log2("No option found for: \"%s\", added default value 0 in index %d\n", str, i);
    }
    return 0;
}


int merlin_pmi_write16_delay(uint32_t CoreNum, uint32_t PortNum, uint32_t DEV_ADDR, uint32_t REG_ADDR, uint16_t DATA, uint16_t MASK, bool delay_acked)
{
    static phy_dev_t phy_dev = {0};

    phy_dev.core_index = CoreNum;
    phy_dev.lane_index = PortNum;
    return serdes_access_write_mask(&phy_dev, DEV_ADDR, REG_ADDR, (~MASK) & 0xffff, 0, DATA);
}

uint16 merlin_pmi_read16_delay(uint32 CoreNum, uint32 PortNum, uint32 DEV_ADDR, uint32 REG_ADDR, bool DELAY_ACKED)
{
    static phy_dev_t phy_dev = {0};
    uint16 DATA;

    phy_dev.core_index = CoreNum;
    phy_dev.lane_index = PortNum;
    serdes_access_read(&phy_dev, DEV_ADDR, REG_ADDR, &DATA);
    return DATA;
}

/***********************************************/
/*  Microcode Load into Program RAM Functions  */
/***********************************************/
static void merlin_load_firmware (phy_dev_t *phy_dev)
{
    int ret = 0;
    uint32 CoreNum = phy_dev->core_index;
    uint32 PortNum = phy_dev->usxgmii_m_index;

#if !defined(CONFIG_BCM96765)
    ret |= wrc_merlin16_shortfin_mdio_multi_prts_en(phy_dev, 0);
    ret |= wrc_merlin16_shortfin_mdio_brcst_port_addr(phy_dev, phy_dev->addr);
#endif

    ret |= merlin16_shortfin_uc_reset(phy_dev, 1); 
    ret |= merlin16_shortfin_ucode_mdio_load(phy_dev, merlin16_shortfin_ucode_image, MERLIN16_SHORTFIN_UCODE_IMAGE_SIZE);
    ret |= merlin16_shortfin_ucode_load_verify(phy_dev, merlin16_shortfin_ucode_image, MERLIN16_SHORTFIN_UCODE_IMAGE_SIZE);
    ret |= merlin16_shortfin_uc_reset(phy_dev, 0);
    ret |= merlin16_shortfin_wait_uc_active(phy_dev);
    ret |= merlin16_shortfin_ucode_crc_verify(phy_dev, MERLIN16_SHORTFIN_UCODE_IMAGE_SIZE, MERLIN16_SHORTFIN_UCODE_IMAGE_CRC);
    ret |= merlin16_shortfin_init_merlin16_shortfin_info(phy_dev);
    print_log("MerlinSupport::%s(): ret = %d \n", __func__, ret);
}

static int _merlin_core_power_op(phy_dev_t *phy_dev, int power_level)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if (power_level == phy_serdes->cur_power_level)
        return 0;

    phy_serdes->cur_power_level = power_level;

    return serdes_access_config(phy_dev, power_level == SERDES_POWER_UP);
}

int merlin16_shortfin_set_shared_clock(phy_dev_t *phy_dev)
{
    serdes_access_config(phy_dev, 0);
    return 0;
}

static void serdes_core_reset(int CoreNum)
{
    int PortNum = 0;
    print_log("Toggle Serdes Core #%d PMD and uC reset.\n", CoreNum);
    merlin_pmi_write16(CoreNum, 0x0, 0x1, 0xd0f1, 0x0000, 0x0000);
    msleep(1);
    merlin_pmi_write16(CoreNum, 0x0, 0x1, 0xd0f1, 0x0001, 0x0000);
    msleep(1);
}

static void merlin_core_init(phy_dev_t *phy_dev)
{
    /*
       Core Initialization from Power down/Reset state
     */
    phy_serdes_t *phy_serdes = phy_dev->priv;
    uint32 CoreNum = phy_dev->core_index;
    uint32 PortNum = phy_dev->usxgmii_m_index;

    serdes_core_reset(phy_serdes->core_num);

    print_log("INFO MerlinSupport::%s(): END. Core #%d with PRTAD = %d, ln_offset_stap = %d\n", __func__, CoreNum, phy_dev->addr, 0);
}

int merlin_core_power_op(phy_dev_t *phy_dev, int power_level)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if (power_level == phy_serdes->cur_power_level)
        return 0;
    _merlin_core_power_op(phy_dev, power_level);
    if (power_level == SERDES_POWER_UP)
        merlin16_serdes_init(phy_dev);
    else
        phy_serdes->inited = 1;
    return 0;
}

#ifdef NOTUSEDYET
static void merlin_uc_reset(uint32 CoreNum, uint8_t enable)
{
    if (enable) {
        merlin_pmi_write16(CoreNum, 0x0, 0x1, 0xD225, 0x8201, 0x0000);  //[13:8]micro_dr_size: 6'd2  : Data RAM = 2KB,  Code RAM = 32KB
    } else {
        /*disable micro_pmi_hp_fast_read_en to deal with uC crash issue at high/low temperature */
        merlin_pmi_write16(CoreNum, 0x0, 0x1, 0xd228, 0x0000, 0xfffe); /* disable micro_pmi_hp_fast_read_en */

        /* De-assert micro reset - Start executing code */
        merlin_pmi_write16(CoreNum, 0x0, 0x1, 0xd200, 0x0001, 0xfffe); /* Enable clock to microcontroller subsystem */
        merlin_pmi_write16(CoreNum, 0x0, 0x1, 0xd201, 0x0001, 0xfffe); /* De-assert reset to microcontroller sybsystem */

        merlin_pmi_write16(CoreNum, 0x0, 0x1, 0xd200, 0x0002, 0xfffd); /* Enable clock to M0 core */
        merlin_pmi_write16(CoreNum, 0x0, 0x1, 0xd201, 0x0002, 0xfffd); /* De-assert reset to micro to start executing microcode */
    }
}
#endif

static void merlin_wait_uc_active (uint32 CoreNum)
{
    uint32 PortNum = 0;
    uint32 pmi_rd_data;
    uint32 L_CoreNum = CoreNum;

    //delay
    print_log("MerlinSupport::%s():    wait 50us comclks for micro to be up...\n", __func__);
    timeout_ns (12000);  //12us

    pmi_rd_data = merlin_pmi_read16_delay(L_CoreNum, 0x0, 0x1, 0xd0f4, true);
    if ((pmi_rd_data & 0x8000) == 0x8000) { //uc_active@[15]
        print_log("MerlinSupport::%s():  Checking uc_active passed ...\n", __func__);
    } else {
        fatal_log("MerlinSupport::%s():  Checking uc_active failed !!!\n", __func__);
    }

    //addition to the 3.1 Core Initialization from Power down state
    timeout_ns (20000);  //20us
    msleep(100);
    pmi_rd_data = merlin_pmi_read16(L_CoreNum, 0x0, 0x1, 0xd00d);
    if ((pmi_rd_data & 0x0080) == 0x0080) { //uc_dsc_ready_for_cmd@[7]
        if ((pmi_rd_data & 0x0040) == 0x0040) { //bit 6 is uc_dsc_error_found
            fatal_log("MerlinSupport::%s():  us_dsc_ready for cmd is set and uc_dsc_error is also set !!!\n", __func__);
        } else {
            print_log("MerlinSupport::%s():  micro is ready for command ...\n", __func__);
        }
    } else {
        fatal_log("MerlinSupport::%s():  uc_dsc_ready_for_cmd is not set !!!\n", __func__);
    }
}

typedef struct save_regs_s {
    prog_seq_tbl *ent;
    uint16_t val;
} save_regs_t;

static save_regs_t saved_regs[3][128];
static int register_dont_save;
static void save_regs(prog_seq_tbl *ent, int CoreNum)
{
    static int entp[3] = {0};
    prog_seq_tbl *tbl_ent;
    save_regs_t *save_reg;
    int i;
    int core_active;

    if (register_dont_save)
        return;

    if (ent == NULL)
    {
        core_active = (merlin_pmi_read16(CoreNum, 0, 1, 0xd081) & 1) > 0;
        if (core_active)
            merlin_pmi_write16(CoreNum, 0, 1, 0xd081, 0, 0xfffe); /* Put Serdes in Reset first */
        for (i = 0; i < entp[CoreNum]; i++)
        {
            save_reg = &saved_regs[CoreNum][i];
            tbl_ent = save_reg->ent;
            merlin_pmi_write16(CoreNum, 0, tbl_ent->dev_addr, tbl_ent->reg_addr, save_reg->val, 0);
        }
        if (core_active)
            merlin_pmi_write16(CoreNum, 0, 1, 0xd081, 1, 0xfffe); /* Restore Core Active Status */
        entp[CoreNum] = 0;
        return;
    }

#define PRINT_DUPLICATED_REG_SAVE 0
    for (i = 0; i < entp[CoreNum]; i++) /* Check duplicated saved registers */
    {
        save_reg = &saved_regs[CoreNum][i];
        tbl_ent = save_reg->ent;
        if (ent->reg_addr == tbl_ent->reg_addr && ent->dev_addr == tbl_ent->dev_addr)
        {
#if PRINT_DUPLICATED_REG_SAVE
            printk("Register already saved entry:0x%lx, %s, dev:%d, reg:0x%x, val:0x%x\n"
                    "                   New entry:0x%lx, %s, dev:%d, reg:0x%x, val:0x%x\n",
                    (uint64_t)tbl_ent, tbl_ent->reg_desc, tbl_ent->dev_addr, tbl_ent->reg_addr, tbl_ent->data,
                    (uint64_t)ent, ent->reg_desc, ent->dev_addr, ent->reg_addr, ent->data);
#endif
            return;
        }
    }

    i = entp[CoreNum];
    if (++entp[CoreNum] >= ARRAY_SIZE(saved_regs[0]))
        BUG_CHECK("Error: Merline register saving array overflow\n");

    save_reg = &saved_regs[CoreNum][i];
    save_reg->ent = ent;
    save_reg->val = merlin_pmi_read16(CoreNum, 0x0, ent->dev_addr, ent->reg_addr);
}
#define restore_regs(CoreNum) save_regs(0, CoreNum)

int parse_seq_fun(prog_seq_tbl *prog_seq_tbl_ptr, uint32 CoreNum, uint32 PortNum, int *arg)
{
    int args = 0;
    int *parm_p = &prog_seq_tbl_ptr->dev_addr;

    for (args=0; args<4; args++, arg++, parm_p++) {
        switch(*parm_p) {
            case SEQ_FUN_ARG_CORE:
                *arg = CoreNum;
                break;
            case SEQ_TYPE_FUN_ARG_NONE:
                goto end;
        }
    }
end:
    return args;
}

static void merlin_reg_prog(prog_seq_tbl *prog_seq_tbl_ptr, uint32 CoreNum, uint32 PortNum)
{
    uint32 i;
    uint32 data_mask;
    uint32 iter;
    void (*fun0)(void);
    void (*fun1)(int arg1);
    void (*fun2)(int arg1, int arg2);
    void (*fun3)(int arg1, int arg2, int arg3);
    void (*fun4)(int arg1, int arg2, int arg3, int arg4);
    int arg[4]= {0, 0, 0, 0};
    int _log_level = log_level;

    log_level = 0;
    for(;strcmp(prog_seq_tbl_ptr->reg_desc, ""); prog_seq_tbl_ptr++) {
        if(parse_sim_opts("-d VERBOSE")) {
            print_log("MerlinSupport::%s():%s\n", __func__, prog_seq_tbl_ptr->reg_desc);
        }

        if (SEQ_TYPE_GET(prog_seq_tbl_ptr->reg_desc) == SEQ_TYPE_FUN) {
            fun0 = SEQ_TYPE_FUNC_GET(prog_seq_tbl_ptr->reg_desc, void (*)(void));
            switch(parse_seq_fun(prog_seq_tbl_ptr, CoreNum, PortNum, arg)) {
                case 0:
                    fun0();
                    continue;
                case 1:
                    fun1 = (void (*)(int))fun0;
                    fun1(arg[0]);
                    continue;
                case 2:
                    fun2 = (void (*)(int, int))fun0;
                    fun2(arg[0], arg[1]);
                    continue;
                case 3:
                    fun3 = (void (*)(int, int, int))fun0;
                    fun3(arg[0], arg[1], arg[2]);
                    continue;
                case 4:
                    fun4 = (void (*)(int, int, int, int))fun0;
                    fun4(arg[0], arg[1], arg[2], arg[3]);
                    continue;
            }
            continue;
        }

        if (SEQ_TYPE_GET(prog_seq_tbl_ptr->reg_desc) == SEQ_TYPE_NEST_SEQ) {
            merlin_reg_prog(SEQ_TYPE_NSEQ_GET(prog_seq_tbl_ptr->reg_desc), CoreNum, PortNum);
            continue;
        }

        if (!strcmp(prog_seq_tbl_ptr->reg_desc, "timeout_100ns")) {
            iter = prog_seq_tbl_ptr->dev_addr & 0xffff;
            print_log("MerlinSupport::%s(timeout_100ns): %d loops \n", __func__, iter);
            print_log("BEGIN : \n");
            for (i=0; i<iter;i++) {
                timeout_ns(100);
            }
            print_log(" DONE with %d loops of timeout_100ns \n", iter);
            continue;
        }

        data_mask = (~prog_seq_tbl_ptr->data_bitEn) & 0xffff;
        print_log("... Writing 0x%04x with data mask of 0x%04x to core #%d lane #%d, dev address 0x%02x, register address 0x%04x\n", prog_seq_tbl_ptr->data, data_mask, CoreNum, PortNum, prog_seq_tbl_ptr->dev_addr, prog_seq_tbl_ptr->reg_addr);
        save_regs(prog_seq_tbl_ptr, CoreNum);
        merlin_pmi_write16(CoreNum, PortNum, prog_seq_tbl_ptr->dev_addr, prog_seq_tbl_ptr->reg_addr, prog_seq_tbl_ptr->data, data_mask);
    }
    log_level = _log_level;
}

static void merlin_switch_a_to_b(phy_dev_t *phy_dev, uint32 CoreNum, uint32 PortNum)
{
    switch(phy_dev->usxgmii_m_type)
    {
        case USXGMII_M_10G_S:
            merlin_reg_prog(usxgmii_switch_pcs_from_A_to_B_1port_10g, CoreNum, PortNum);
            break;
        case USXGMII_M_10G_D:
            merlin_reg_prog(usxgmii_switch_pcs_from_A_to_B_2port_10g, CoreNum, PortNum);
            break;
        case USXGMII_M_10G_Q:
            merlin_reg_prog(usxgmii_switch_pcs_from_A_to_B_4port_10g, CoreNum, PortNum);
            break;
        default:
            printkwarn("Warning: Serdes addr %d Wrong USXGMII-M mode configuraton: %d.", 
                phy_dev->addr, phy_dev->usxgmii_m_type);
    }
}

int merlin16_read_txfir_reg(phy_dev_t *phy_dev, phy_txfir_t *txfir)
{
    int core_num = phy_dev->core_index;
    txfir->main_reg = merlin_pmi_read16(core_num, 0x0, 1, 0xd11e);
    txfir->pre_reg = merlin_pmi_read16(core_num, 0x0, 1, 0xd11d);
    txfir->post1_reg = merlin_pmi_read16(core_num, 0x0, 1, 0xd11c);
    txfir->post2_reg = merlin_pmi_read16(core_num, 0x0, 1, 0xd11b);
    txfir->hpf_reg = merlin_pmi_read16(core_num, 0x0, 1, 0xd0a2);
    return 0;
}

int merlin16_write_txfir_reg(phy_dev_t *phy_dev, phy_txfir_t *txfir)
{
    int core_num = phy_dev->core_index;
    merlin_pmi_write16(core_num, 0x0, 1, 0xd11e, txfir->main_reg, 0);
    merlin_pmi_write16(core_num, 0x0, 1, 0xd11d, txfir->pre_reg, 0);
    merlin_pmi_write16(core_num, 0x0, 1, 0xd11c, txfir->post1_reg, 0);
    merlin_pmi_write16(core_num, 0x0, 1, 0xd11b, txfir->post2_reg, 0);
    merlin_pmi_write16(core_num, 0x0, 1, 0xd0a2, txfir->hpf_reg, 0);
    return 0;
}

static void merlin_cfg_core_ram_var (uint32 CoreNum, uint16_t vco_rate)
{
    uint32_t   core_var_ram_base = 0x0;
    uint16_t   an_los_workaround = 0x0<<6;
    uint16_t   core_cfg_from_pcs = 0x1; //This is set to 1 by default when using HSIP Config Interface
    uint16_t   core_cfg_vco_rate = 0x0;
    uint16_t   core_config_word  = 0x0;
    uint32 PortNum = 0;

    core_var_ram_base = 0x20000000 + 0x200; //merlin16_shortfin_internal.h:#define CORE_VAR_RAM_BASE (0x200)

    //--------------   Merlin16_Programmers_Guide.docx (RAM filed)  --------------------
    //Core
    //vco_rate[7:0]
    //core_cfg_from_pcs:
    //merlin16_shortfin_internal.c merlin16_shortfin_INTERNAL_update_uc_core_config_word()
    /*
       [15:8] : reserved2
       [7]: reserved1
       [6]:an_los_workaround
       [5:1]:vco_rate
       [0]:core_cfg_from_pcs
     */

    //optional
    core_cfg_from_pcs = 0x0;

    core_cfg_vco_rate = vco_rate << 1;
    core_config_word = an_los_workaround + core_cfg_vco_rate + core_cfg_from_pcs;
    print_log("MerlinSupport::%s():  program core_config_word 0x%x to ram address 0x%x ...\n", __func__, core_config_word, core_var_ram_base);
    merlin_pmi_write16(CoreNum, 0x0, 0x1, 0xd205,((core_var_ram_base >> 16) & 0xffff), 0x0000);  //[15:0]: lsw data address      /* Upper 16bits of start address of data ram
    merlin_pmi_write16(CoreNum, 0x0, 0x1, 0xd204,((core_var_ram_base >>  0) & 0xffff), 0x0000);  //[15:0]: msw data address      /* lower 16bits of start address of data ram
    merlin_pmi_write16(CoreNum, 0x0, 0x1, 0xd206, core_config_word, 0x0000);

}

static void merlin_reset_datapath_core (uint32 CoreNum)
{
    uint32 PortNum = 0;
    print_log("MerlinSupport::%s(): Datapath Reset (core) in progress \n", __func__);
    merlin_reg_prog(datapath_reset_core, CoreNum, 0x0);
}

static void merline_usxgmii_mac_credit_enable(uint32 CoreNum, uint32 PortNum)
{
    print_log("MerlinSupport::%s(): Enable MAC credit for USXGMII-M mode.\n", __func__);
    merlin_reg_prog(usxgmii_mac_crdit_enable, CoreNum, 0x0);
}

static void merlin_lane_config_speed(uint32 CoreNum, uint32 PortNum, uint32 LNK_SPD, uint32 an_enabled)
{
    if (LNK_SPD != MLN_SPD_AN_USXGMII_MP_SLAVE && an_enabled)
    {
        merlin_pmi_write16 (CoreNum, PortNum, 0x3, 0x9107, 0x0000 , 0x7fff );
        merlin_pmi_write16 (CoreNum, PortNum, 0x3, 0x9250, 0x029a , 0x0000 );
        merlin_pmi_write16 (CoreNum, PortNum, 0x3, 0x9251, 0x029a , 0x0000 );
        merlin_pmi_write16 (CoreNum, PortNum, 0x3, 0x9252, 0xa000 , 0x0000 );
        merlin_pmi_write16 (CoreNum, PortNum, 0x3, 0x9253, 0x10ee , 0x0000 );
        merlin_pmi_write16 (CoreNum, PortNum, 0x3, 0x9254, 0xa000 , 0x0000 );
        merlin_pmi_write16 (CoreNum, PortNum, 0x3, 0x9255, 0x0bb8 , 0x0000 );
        merlin_pmi_write16 (CoreNum, PortNum, 0x3, 0x9256, 0x1b00 , 0x0000 );
        merlin_pmi_write16 (CoreNum, PortNum, 0x3, 0x9257, 0x8235 , 0x0000 );
        merlin_pmi_write16 (CoreNum, PortNum, 0x3, 0x9258, 0x8235 , 0x0000 );
        merlin_pmi_write16 (CoreNum, PortNum, 0x3, 0x925a, 0x8235 , 0x0000 );
        merlin_pmi_write16 (CoreNum, PortNum, 0x3, 0x925c, 0x000f , 0x0000 );
    }
    /*
       timeout_ns(100); //delay for simulation debug only
    //---- for simulation speed up -----

    print_log("MerlinSupport::%s(): Core #%d Lane #%d PMD Lock Speed Up programming \n", __func__, CoreNum, PortNum);
    merlin_reg_prog(pmd_lock_speedup, CoreNum, PortNum);

    timeout_ns(100); //delay for simulation debug only
    //----------------------------------
     */

    print_log("MerlinSupport::%s(#%x):  \n", __func__,LNK_SPD);
    //CL36
    if (LNK_SPD == MLN_SPD_FORCE_5G) {  //Force CL36 5G
        print_log("MerlinSupport::CONFIGURING FOR FORCE 5G\n");
        merlin_reg_prog(force_speed_5g, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_FORCE_5G_XGMII) {  //Force CL36 5G
        print_log("MerlinSupport::CONFIGURING FOR FORCE 5G (XGMII) \n");
        merlin_reg_prog(force_speed_5g_xgmii, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_FORCE_2P5G) {  //Force 2P5G
        print_log("MerlinSupport::CONFIGURING FOR FORCE 2P5G\n");
        if (parse_sim_opts("-d ML_C_VCO_10P3125")) {
            print_log("... Writing 0x%04x with data mask of 0x%04x to core #%d lane #%d, dev address 0x%02x, register address 0x%04x\n", 0x21, 0xc000, CoreNum, PortNum, 0x3, 0x9270);
            merlin_pmi_write16(CoreNum, PortNum, 0x3, 0x9270, 0x0021, 0xc000);
        }
        merlin_reg_prog(force_speed_2p5g, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_FORCE_2P5G_XGMII) {  //Force 2P5G XGMII
        print_log("MerlinSupport::CONFIGURING FOR FORCE 2P5G (XGMII)\n");
        merlin_reg_prog(force_speed_2p5g_xgmii, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_FORCE_1G) {  //Force 1G
        print_log("MerlinSupport::CONFIGURING FOR FORCE 1G\n");
        merlin_reg_prog(force_speed_1g, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_FORCE_1G_XGMII) {  //Force 1G XGMII
        if (parse_sim_opts("-d ML_C_VCO_10P3125")) { //force 10.3125GHz VCO
            print_log("MerlinSupport::RE-PROGRAM CL127 CREDIT For 10.3125GHz VCO\n");
            merlin_reg_prog(cl127_1g_credit_10p3125_VCO, CoreNum, PortNum);
        }
        print_log("MerlinSupport::CONFIGURING FOR FORCE 1G (XGMII)\n");
        merlin_reg_prog(force_speed_1g_xgmii, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_FORCE_1G_KX1) {  //Force 1G KX
        print_log("MerlinSupport::CONFIGURING FOR FORCE 1G_KX\n");
        merlin_reg_prog(force_speed_1g_kx1, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_FORCE_1G_KX1_XGMII) {  //Force 1G KX XGMII
        print_log("MerlinSupport::RE-PROGRAM CL127 CREDIT For 10.3125GHz VCO\n");
        if (parse_sim_opts("-d ML_C_VCO_10P3125")) { //force 10.3125GHz VCO
            merlin_reg_prog(cl127_1g_credit_10p3125_VCO, CoreNum, PortNum);
        }
        print_log("MerlinSupport::CONFIGURING FOR FORCE 1G_KX (XGMII)\n");
        merlin_reg_prog(force_speed_1g_kx1_xgmii, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_1G_KX_IEEE_CL73) {  //AN 1G KX - IEEE CL73
        print_log("MerlinSupport::CONFIGURING FOR AN 1G KX - IEEE CL73\n");
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        merlin_reg_prog(auto_neg_1g_kx_ieee_cl73, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_1G_KX_IEEE_CL73_XGMII) {  //AN 1G KX (XGMII) - IEEE CL73
        print_log("MerlinSupport::CONFIGURING FOR AN 1G KX (XGMII) - IEEE CL73\n");
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        merlin_reg_prog(auto_neg_1g_kx_ieee_cl73_xgmii, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_1G_USER_CL73) {  //AN 1G - USER CL73
        print_log("MerlinSupport::CONFIGURING FOR AN 1G - USER SPACE CL73\n");
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        merlin_reg_prog(auto_neg_1g_user_cl73, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_1G_USER_CL73_XGMII) {  //AN 1G (XGMII) - USER CL73
        print_log("MerlinSupport::CONFIGURING FOR AN 1G (XGMII) - USER SPACE CL73\n");
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        merlin_reg_prog(auto_neg_1g_user_cl73_xgmii, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_1G_IEEE_CL37) {  //AN 1G - IEEE CL37
        print_log("MerlinSupport::CONFIGURING FOR AN 1G - IEEE CL37\n");
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        merlin_reg_prog(auto_neg_1g_ieee_cl37, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_1G_IEEE_CL37_XGMII) {  //AN 1G (XGMII) - IEEE CL37
        print_log("MerlinSupport::RE-PROGRAM CL127 CREDIT For 10.3125GHz VCO\n");
        if (parse_sim_opts("-d ML_C_VCO_10P3125")) { //force 10.3125GHz VCO
            merlin_reg_prog(cl127_1g_credit_10p3125_VCO, CoreNum, PortNum);
        }
        print_log("MerlinSupport::CONFIGURING FOR AN 1G (XGMII) - IEEE CL37\n");
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        merlin_reg_prog(auto_neg_1g_ieee_cl37_xgmii, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_1G_USER_CL37) {  //AN 1G - USER CL37
        print_log("MerlinSupport::CONFIGURING FOR AN 1G - USER SPACE CL37\n");
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        merlin_reg_prog(auto_neg_1g_user_cl37, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_1G_USER_CL37_XGMII) {  //AN 1G (XGMII) - USER CL37
        print_log("MerlinSupport::RE-PROGRAM CL127 CREDIT For 10.3125GHz VCO\n");
        if (parse_sim_opts("-d ML_C_VCO_10P3125")) { //force 10.3125GHz VCO
            merlin_reg_prog(cl127_1g_credit_10p3125_VCO, CoreNum, PortNum);
        }
        print_log("MerlinSupport::CONFIGURING FOR AN 1G (XGMII)- USER SPACE CL37\n");
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        merlin_reg_prog(auto_neg_1g_user_cl37_xgmii, CoreNum, PortNum);
        //CL49
    } else if (LNK_SPD == MLN_SPD_FORCE_10G_R) {         //Force CL49 10G BASE-R
        print_log("MerlinSupport::CONFIGURING FOR FORCE 10G BASE-R \n");
        merlin_reg_prog(force_speed_10g_R, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_FORCE_10G_R_CL74) {  //Force CL49 10G BASE-R + CL74 FEC
        print_log("MerlinSupport::CONFIGURING FOR FORCE 10G BASE-R with CL74 FEC\n");
        merlin_reg_prog(force_speed_10g_R_cl74_fec, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_10G_KR_IEEE_CL73) {  //AN CL49 10G BASE-KR with IEEE CL73
        print_log("MerlinSupport::CONFIGURING FOR AN 10G KR - IEEE CL73\n");
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        merlin_reg_prog(auto_neg_10g_kr_cl73, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_10G_KR_IEEE_CL73_CL74) {  //AN CL49 10G BASE-KR +CL74 FEC with IEEE CL73
        print_log("MerlinSupport::CONFIGURING FOR AN 10G KR + CL74 FEC - IEEE CL73\n");
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        merlin_reg_prog(auto_neg_10g_kr_cl73_cl74_fec, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_10G_USER_CL73) {  //AN CL49 10G with User Space CL73
        print_log("MerlinSupport::CONFIGURING FOR AN 10G - User Space CL73 \n");
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        merlin_reg_prog(auto_neg_10g_user_cl73, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_10G_USER_CL73_CL74) {  //AN CL49 10G with User Space CL73
        print_log("MerlinSupport::CONFIGURING FOR AN 10G + CL74 FEC - User Space CL73 \n");
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        merlin_reg_prog(auto_neg_10g_user_cl73_cl74_fec, CoreNum, PortNum);
        //CL127
    } else if (LNK_SPD == MLN_SPD_FORCE_5G_X) {  //Force CL127 5GBASE-X
        print_log("MerlinSupport::CONFIGURING FOR FORCE 5G CL127 BASE-X\n");
        merlin_reg_prog(force_speed_5g_x, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_FORCE_2P5G_X) {  //Force CL127 2.5GBASE-X
        print_log("MerlinSupport::CONFIGURING FOR FORCE 2P5G CL127 BASE-X\n");
        if (parse_sim_opts("-d ML_C_VCO_10P3125")) {
            print_log("... Writing 0x%04x with data mask of 0x%04x to core #%d lane #%d, dev address 0x%02x, register address 0x%04x\n", 0x21, 0xc000, CoreNum, PortNum, 0x3, 0x9284);
            merlin_pmi_write16(CoreNum, PortNum, 0x3, 0x9284, 0x0021, 0xc000);
            print_log("... Writing 0x%04x with data mask of 0x%04x to core #%d lane #%d, dev address 0x%02x, register address 0x%04x\n", 0x10, 0xc000, CoreNum, PortNum, 0x3, 0x9285);
            merlin_pmi_write16(CoreNum, PortNum, 0x3, 0x9285, 0x0010, 0xe000);
        }
        merlin_reg_prog(force_speed_2p5g_x, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_FORCE_1G_X) {  //Force CL127 1GBASE-X
        print_log("MerlinSupport::CONFIGURING FOR FORCE 1G CL127 BASE-X\n");
        merlin_reg_prog(force_speed_1g_x, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_2P5G_KX_IEEE_CL73) {  //AN CL127 2.5G BASE-KR with IEEE CL73
        print_log("MerlinSupport::RE-PROGRAM CL127 CREDIT For 10.3125GHz VCO\n");
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        merlin_reg_prog(cl127_2p5g_credit_10p3125_VCO, CoreNum, PortNum);
        print_log("MerlinSupport::CONFIGURING FOR AN 5G KR - IEEE CL73\n");
        merlin_reg_prog(auto_neg_2p5g_kx_cl73, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_2P5G_USER_CL73) {  //AN CL127 2.5G with User Space CL73
        print_log("MerlinSupport::RE-PROGRAM CL127 CREDIT For 10.3125GHz VCO\n");
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        merlin_reg_prog(cl127_2p5g_credit_10p3125_VCO, CoreNum, PortNum);
        print_log("MerlinSupport::CONFIGURING FOR AN 5G - User Space CL73 \n");
        merlin_reg_prog(auto_neg_2p5g_user_cl73, CoreNum, PortNum);
        //CL129
    } else if (LNK_SPD == MLN_SPD_FORCE_5G_R) {  //Force CL129 5GBASE-R
        print_log("MerlinSupport::CONFIGURING FOR FORCE 5G BASE-R\n");
        merlin_reg_prog(force_speed_5g_R, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_FORCE_2P5G_R) {  //Force 2P5G BASE-R
        print_log("MerlinSupport::CONFIGURING FOR FORCE 2P5G BASE-R\n");
        merlin_reg_prog(force_speed_2p5g_R, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_FORCE_1G_R) {  //Force 1G BASE-R
        print_log("MerlinSupport::CONFIGURING FOR FORCE 1G BASE-R\n");
        merlin_reg_prog(force_speed_1g_R, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_5G_KR_IEEE_CL73) {  //AN CL129 5G BASE-KR with IEEE CL73
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        print_log("MerlinSupport::CONFIGURING FOR AN 5G KR - IEEE CL73\n");
        merlin_reg_prog(auto_neg_5g_kr_cl73, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_5G_USER_CL73) {  //AN CL129 5G with User Space CL73
        print_log("MerlinSupport::CONFIGURING FOR AN 5G - User Space CL73 \n");
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        merlin_reg_prog(auto_neg_5g_user_cl73, CoreNum, PortNum);
        //SGMII
    } else if (LNK_SPD == MLN_SPD_FORCE_100M) {  //Force 100M SGMII
        print_log("MerlinSupport::CONFIGURING FOR FORCE 100M SGMII\n");
        merlin_reg_prog(force_speed_100m, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_FORCE_10M) {  //Force 10M SGMII
        print_log("MerlinSupport::CONFIGURING FOR FORCE 10M SGMII\n");
        merlin_reg_prog(force_speed_10m, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_1G_SGMII) {  //AN 1G SGMII
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        print_log("MerlinSupport::CONFIGURING FOR AN 1G SLAVE SGMII\n");
    } else if (LNK_SPD == MLN_SPD_AN_100M_SGMII) {  //AN 100M SGMII
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        print_log("MerlinSupport::CONFIGURING FOR AN 100M SLAVE SGMII\n");
    } else if (LNK_SPD == MLN_SPD_AN_10M_SGMII) {  //AN 10M SGMII
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        print_log("MerlinSupport::CONFIGURING FOR AN 100M SGMII\n");
    } else if (LNK_SPD == MLN_SPD_AN_SGMII_SLAVE) {  //AN SGMII SLAVE
        print_log("MerlinSupport::CONFIGURING FOR SGMII AN SLAVE\n");
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        merlin_reg_prog(sgmii_an_slave, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_SGMII_SLAVE_XGMII) {  //AN SGMII SLAVE
        print_log("MerlinSupport::CONFIGURING FOR SGMII AN SLAVE (XGMII) \n");
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        merlin_reg_prog(sgmii_an_slave_xgmii, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_FORCE_10G_USXGMII) {  //Force 10G USXGMII
        print_log("MerlinSupport::CONFIGURING FOR FORCE 10G USXGMII\n");
        merlin_reg_prog(force_speed_10g_usxgmii, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_FORCE_10G_CL74_USXGMII) {  //Force 10G USXGMII + CL74 FEC
        print_log("MerlinSupport::CONFIGURING FOR FORCE 10G USXGMII with CL74 FEC\n");
        merlin_reg_prog(force_speed_10g_usxgmii_cl74_fec, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_FORCE_5G_USXGMII) {  //Force 5G USXGMII
        print_log("MerlinSupport::CONFIGURING FOR FORCE 5G USXGMII\n");
        merlin_reg_prog(force_speed_5g_usxgmii, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_FORCE_2P5G_USXGMII) {  //Force 2P5G USXGMII
        print_log("MerlinSupport::CONFIGURING FOR FORCE 2P5G USXGMII\n");
        merlin_reg_prog(force_speed_2p5g_usxgmii, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_FORCE_1G_USXGMII) {  //Force 1G USXGMII
        print_log("MerlinSupport::CONFIGURING FOR FORCE 1G USXGMII\n");
        merlin_reg_prog(force_speed_1g_usxgmii, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_FORCE_100M_USXGMII) {  //Force 100M USXGMII
        print_log("MerlinSupport::CONFIGURING FOR FORCE 100M USXGMII\n");
        merlin_reg_prog(force_speed_100m_usxgmii, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_USXGMII_MASTER) {  //USXGMII AN MASTER
        print_log("MerlinSupport::CONFIGURING FOR USXGMII AN MASTER\n");
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        merlin_reg_prog(usxgmii_an_master, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_USXGMII_MP_SLAVE) {  //USXGMII_M AN SLAVE
        print_log("MerlinSupport::CONFIGURING FOR USXGMII_M AN SLAVE\n");
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        merlin_reg_prog(usxgmii_mp_an_slave, CoreNum, PortNum);
        merlin_reg_prog(set_usxgmii_mp_an_enable, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_USXGMII_SLAVE) {  //USXGMII AN SLAVE
        print_log("MerlinSupport::CONFIGURING FOR USXGMII AN SLAVE\n");
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        merlin_reg_prog(usxgmii_an_slave, CoreNum, PortNum);
        merlin_reg_prog(set_usxgmii_an_enable, CoreNum, PortNum);
    } else if (LNK_SPD == MLN_SPD_AN_ALL_SPEEDS_IEEE_CL73) {
        merlin_reg_prog(AN_speedup, CoreNum, PortNum);   //---- for simulation speed up -----
        merlin_reg_prog(auto_neg_ieee_cl73, CoreNum, PortNum);
        //merlin_reg_prog(auto_neg_10g_kr_cl73, CoreNum, PortNum);
    } else {
        fatal_log("ERROR !!! MerlinSupport::%s():Merlin Core #%d lane #%d Mode %x is not supported \n", __func__, CoreNum, PortNum, LNK_SPD);
    }

    //release per-lane's ln_dp_s_rstb. Merlin Programmers Guide.pdf subchapter 3.1 #10.c

    //for B0: move to here due the this register can only be written after applied datapath_reset_lane
    timeout_ns(100); //delay for simulation debug only

    //---- for simulation speed up -----

    print_log("MerlinSupport::%s(): Core #%d Lane #%d PMD Lock Speed Up programming \n", __func__, CoreNum, PortNum);
    // merlin_reg_prog(pmd_lock_speedup, CoreNum, PortNum);
    //
    //----------------------------------

    timeout_ns(100); //delay for simulation debug only

    print_log("INFO MerlinSupport::%s(): END Merlin core #%d lane #%d Initialization procedure\n", __func__, CoreNum, PortNum);

}

void merlin_chk_pll_lock (uint32 CoreNum)
{
    static phy_dev_t phy_dev = {0};
    int PortNum = 0;
    phy_dev.core_index = CoreNum;

    print_log("MerlinSupport::%s(): Checking Core #%d PLL Lock Status \n", __func__, CoreNum);
    if (serdes_access_wait_pll_lock(&phy_dev))
        fatal_log("ERROR !!! MerlinSupport::%s(): Merlin Core #%d PLL lock checking has timed-out\n", __func__, CoreNum);
    else
        print_log("MerlinSupport::%s(): PLL Locked\n", __func__);
}

void merlin_cfg_lane_ram_var (phy_dev_t *phy_dev, uint16_t an_enabled)
{

    uint32_t   lane_var_ram_base = 0x0;
    uint16_t   lane_config_word  = 0x0;
    uint16_t   cl72_auto_polarity_en = 0x0;
    uint16_t   lane_cfg_from_pcs = 0x0;
    uint16_t   lane_an_enabled   = 0x0;
    uint16_t   lane_dfe_on       = 0x0;
    uint32 CoreNum = phy_dev->core_index;
    uint32 PortNum = phy_dev->usxgmii_m_index;

    lane_var_ram_base = 0x20000000 + 0x300; //merlin16_shortfin_internal.h:#define LANE_VAR_RAM_BASE (0x300)
    //--------------   Merlin16_Programmers_Guide.docx (RAM field)  --------------------

    //Lane RX
    //lane_cfg_from_pcs: 1: get certain lane configuration from AN/PCS HW status (typicall when AN is enabled) 0: lane configuration is independent of PCS HW status and derived only from user programming
    //an_enabled: 1: CL73 or CL37 AN is enabled. 0: AN is not enabled (HW CL72 would operate in forced mode, This would cause PMD micro to restart the linnk automatically upon PMD link failure)
    //dfe_on: 1: DFE enabled; 0 DFE is not used
    //force_brdfe_on: Please use 0
    //media_type[1:0]: 00-pcb trace; 01-copper; 10-optics
    //unreliable_los: This field is used only when media_type is 'optical'; 1: assume that LOS cannot be trusted; 0: Assume that LOS is reliable
    //scramblings_dis: Currently RX adaptation is disabled, please request if required.
    //cl72_auto_polarity_en: 1: During CL72 if framelock is not achieved within 1ms it will toggle the "rx_pmd_dp_invert" register. It will continue to toggle until lock; 0: no auto polarity selection (default)
    //cl72_restart_timeout_en: This will enable a pmd_rx_restart after 600ms if the linnk has failed to complete training. 0: no restart (default)
    /*
       [15:10] reserved
       [9]: cl72_restart_timeout_en
       [8]: cl72_auto_polarity_en
       [7]: scrambling_dis
       [6]: unreliable_los
       [5:4]: media_type
       [3]: force_brdfe_on
       [2]: dfe_on
       [1]: an_enabled
       [0]: lane_cfg_from_pcs
1G: default (all 0)
XFI+: default (all 0)
->8G-12.5GKR without AN: dfe_on=1* (lane register cl72_ieee_training_enable = 1)
SFI copper: dfe_on =1; media_type = "copper_cable"
->AN(1G/10G) or (1G/2.5G): an_enabled = 1*; lane_cfg_from_pcs = 1*
nPPI or SFI optical: dfe_on = 0; media_type = "optical", unrealiable_los based on application
     */
    //the firmware test is customized for testing 10GBASE-KR with CL72 enabled.
    //cl72_auto_polarity_en = uint16_t(0x1<<8);
    lane_an_enabled = (an_enabled << 1);
    if (an_enabled) {
        lane_cfg_from_pcs = (0x1);
    } else {
        lane_dfe_on = (0x1 << 2);
    }

    if (phy_dev->usxgmii_m_type != USXGMII_S)
    {
        print_log("USXGMII-M Step5: Set Ram variable\n");
        lane_config_word  = 0x14;
    }
    else
        lane_config_word = cl72_auto_polarity_en + lane_dfe_on + lane_an_enabled + lane_cfg_from_pcs;

    print_log("MerlinSupport::%s():  program lane_config_word 0x%x to ram address 0x%x ...\n", __func__, lane_config_word, lane_var_ram_base);
    merlin_pmi_write16(CoreNum, 0x0, 0x1, 0xd205,((lane_var_ram_base >> 16) & 0xffff), 0x0000);  //[15:0]: lsw data address  /* Upper 16bits of start address of data ram
    merlin_pmi_write16(CoreNum, 0x0, 0x1, 0xd204,((lane_var_ram_base >>  0) & 0xffff), 0x0000);  //[15:0]: msw data address  /* lower 16bits of start address of data ram
    merlin_pmi_write16(CoreNum, 0x0, 0x1, 0xd206, lane_config_word, 0x0000);
}

#if 0
static void check_xport0_port0_link_status(void)
{
    uint32 d32;
    uint32 polling;
    uint32 counter;
    polling = 1;
    counter = 0;

    while (polling)
    {
        d32 = pChipEnv->tb_rw->read32(0x837f2004);
        if (d32 == 0x5)
        {
            print_log("XPORT port 0 link up successfully\n");
            polling =0;
        }
        else
        {
            timeout_ns(10000);
            counter = counter+1;
            if (counter>30) fatal_log("XPORT port0 link up out of time real value = %x \n", d32);
        }
    }
    polling = 1;
    counter = 0;
    while (polling)
    {
        d32 = pChipEnv->tb_rw->read32(0x837f6004);
        if (d32 == 0x1)
        {
            print_log("XPORT port 1   link up successfully\n");
            polling =0;
        }
        else
        {
            timeout_ns(10000);
            counter = counter+1;
            if (counter>30) fatal_log("XPORT port1 link up out of time real value = %x \n", d32);
        }
    }
}
#endif

#if 0
static int merlin_get_current_inter_phy_type(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    uint32_t m4_speed = phy_serdes->serdes_speed_mode;

    if (!phy_dev->link || phy_serdes->config_speed == PHY_SPEED_AUTO)
        return INTER_PHY_TYPE_UNKNOWN;

    switch (m4_speed) {
        case MLN_SPD_FORCE_100M:
            return INTER_PHY_TYPE_SGMII;
        case MLN_SPD_FORCE_1G:
            return INTER_PHY_TYPE_1000BASE_X;
        case MLN_SPD_FORCE_1G_X:
            return INTER_PHY_TYPE_1GBASE_X;
        case MLN_SPD_FORCE_1G_R:
            return INTER_PHY_TYPE_1GBASE_R;

        case MLN_SPD_FORCE_2P5G_X:
            return INTER_PHY_TYPE_2P5GBASE_X;
        case MLN_SPD_FORCE_2P5G_R:
            return INTER_PHY_TYPE_2P5GBASE_R;
        case MLN_SPD_FORCE_2P5G:
            return INTER_PHY_TYPE_2500BASE_X;

        case MLN_SPD_FORCE_5G_R:
            return INTER_PHY_TYPE_5GBASE_R;
        case MLN_SPD_FORCE_5G_X:
            return INTER_PHY_TYPE_5GBASE_X;
        case MLN_SPD_FORCE_5G:
            return INTER_PHY_TYPE_5000BASE_X;

        case MLN_SPD_FORCE_10G_R:
            return INTER_PHY_TYPE_10GBASE_R;

        case MLN_SPD_AN_SGMII_SLAVE:
        case MLN_SPD_AN_1G_SGMII:
        case MLN_SPD_AN_100M_SGMII:
        case MLN_SPD_AN_10M_SGMII:
            return INTER_PHY_TYPE_SGMII;

        case MLN_SPD_FORCE_10G_USXGMII:
        case MLN_SPD_FORCE_5G_USXGMII:
        case MLN_SPD_FORCE_2P5G_USXGMII:
        case MLN_SPD_FORCE_1G_USXGMII:
        case MLN_SPD_FORCE_100M_USXGMII:
        case MLN_SPD_AN_USXGMII_SLAVE:
            return INTER_PHY_TYPE_USXGMII;

        case MLN_SPD_AN_ALL_SPEEDS_IEEE_CL73:
            return INTER_PHY_TYPE_MLTI_SPEED_BASE_X_AN;

        default:
            break;
    }
    return INTER_PHY_TYPE_UNKNOWN;
}
#endif

void merlin_chk_lane_link_status(phy_dev_t *phy_dev)
{
    uint32 rd_data;
    phy_serdes_t *phy_serdes = phy_dev->priv;
    int i;
    int old_link = phy_dev->link;

    /* Should work for both USXGMII M and legacy, => PCS Link */
    {
        for(i=0; i<10; i++) {  /* To filter out certain speed false link up */
            phy_dsl_serdes_access_get_an_status(phy_dev);
            if (phy_dev->link == 0)
                break;
            msleep(1);
        }
    }

    if (!phy_dev->link)
        goto end;

    /* Below is for link up only */
    for (i=0; i<10; i++)
    {
        phy_dsl_serdes_access_get_speed(phy_dev);
        if (phy_dev->speed >= PHY_SPEED_10 && phy_dev->speed <= PHY_SPEED_10000)  // Wait for speed result
            break;
        msleep(1);
    }

    if (!(phy_dev->speed >= PHY_SPEED_10 && phy_dev->speed <= PHY_SPEED_10000))   /* False link up */
    {
        phy_dev->link = 0;
        goto end;
    }

    phy_dev->duplex = PHY_DUPLEX_FULL;

    /* If it first time link up and the speed is 5G or 10G, we do intensive check
        to filter out false link up */
    if (!old_link && (phy_dev->speed == PHY_SPEED_10000 || phy_dev->speed == PHY_SPEED_5000 ||
        (phy_dev->speed == PHY_SPEED_2500 && phy_dev->current_inter_phy_type == INTER_PHY_TYPE_2P5GBASE_R)))
    {
        /* Read to clear latch bit; then wait 500ms */
        rd_data = merlin_pmi_read16(phy_serdes->core_num, 0x0, 3, 0xc466);
        msleep(500);
        rd_data = merlin_pmi_read16(phy_serdes->core_num, 0x0, 3, 0xc466);

        // Checking BAD_R_TYPE, R_TYPE_E, Latched_RX_E or current RX_E
        if ((rd_data & (1<<7)) || (rd_data&0x7) == 4
                || ((rd_data>>12) & 0xf) == 0xf || ((rd_data>>12) & 0xf) == 0 )
        {
            phy_dev->link = 0;
            printk("Serdes %d False Link Up with Error Symbol 0x%x at 3.c466h at speed %dMbps\n",
                    phy_dev->addr, rd_data, phy_dev->speed);
            goto end;
        }
    }

    /* Check the mis-link up between our 1000Base-X vs. SGMII */
    if (phy_dev->current_inter_phy_type == INTER_PHY_TYPE_1000BASE_X)
    {
        rd_data = merlin_pmi_read16(phy_serdes->core_num, 0x0, 3, 0xc46d); /* Status1 */
        if (rd_data == 0x1fff) {
            phy_dev->link = 0;
            printk("Serdes %d 1000Base-X False Link Up with 3.c46dh = 0x%04x\n",
                    phy_dev->addr, rd_data);
            goto end;
        }
    }

end:
    return;
}

/* clear 3c4b1.b6 fast tiemer */
/* Work around for some USXGMII link partner which will lock up */
static void usxgmii_workaround(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    int CoreNum = phy_serdes->core_num;

    merlin_pmi_write16(CoreNum, 0x0, 0x3, 0xc4b0, 0x1601, 0x0000);  // Set AN message to 10G
    merlin_pmi_write16(CoreNum, 0x0, 0x3, 0xc4b2, 0x0200, 0xfcff);  // Enable Master mode
    merlin_pmi_write16(CoreNum, 0x0, 0x3, 0xc4b1, 0x1000, 0xefff);  // Toggle AN restart
    merlin_pmi_write16(CoreNum, 0x0, 0x3, 0xc4b1, 0x0000, 0xefff);
    msleep(1);
    merlin_pmi_write16(CoreNum, 0x0, 0x3, 0xc4b2, 0x0000, 0xfcff);  // Disable Master mode
    msleep(1);
}

static int phy_speed_to_merlin_speed(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    int CoreNum = phy_serdes->core_num;

    switch(phy_dev->current_inter_phy_type) {
        case INTER_PHY_TYPE_USXGMII:
            if (phy_dev->an_enabled == 0)
                return -1;
            else
                phy_serdes->serdes_speed_mode = MLN_SPD_AN_USXGMII_SLAVE;
            break;
        case INTER_PHY_TYPE_USXGMII_MP:
            if (phy_dev->an_enabled == 0)
                return -1;
            else
                phy_serdes->serdes_speed_mode = MLN_SPD_AN_USXGMII_MP_SLAVE;
            break;
        case INTER_PHY_TYPE_MLTI_SPEED_BASE_X_AN:
            if (phy_dev->an_enabled == 0)
                return -1;
            else
                phy_serdes->serdes_speed_mode = MLN_SPD_AN_ALL_SPEEDS_IEEE_CL73;
            break;
        case INTER_PHY_TYPE_SGMII:
            if (phy_serdes->sfp_module_type != SFP_FIXED_PHY) 
                phy_serdes->serdes_speed_mode = MLN_SPD_AN_SGMII_SLAVE;
            else    /* Copper PHY, no AN support */
            {
                if (phy_serdes->config_speed == PHY_SPEED_100)
                    phy_serdes->serdes_speed_mode = MLN_SPD_FORCE_100M;
                else
                    phy_serdes->serdes_speed_mode = MLN_SPD_FORCE_1G;
            }
            break;

        case INTER_PHY_TYPE_10GBASE_R:
            if (phy_dev->an_enabled == 0)
                phy_serdes->serdes_speed_mode = MLN_SPD_FORCE_10G_R;
            else
                phy_serdes->serdes_speed_mode = MLN_SPD_AN_10G_KR_IEEE_CL73_CL74;
            break;

        case INTER_PHY_TYPE_5000BASE_X:
            if (phy_dev->an_enabled == 1)
                return -1;
            else
                phy_serdes->serdes_speed_mode = MLN_SPD_FORCE_5G;
            break;
        case INTER_PHY_TYPE_5GBASE_X:
            if (phy_dev->an_enabled == 1)
                return -1;
            else
                phy_serdes->serdes_speed_mode = MLN_SPD_FORCE_5G_X;
            break;
        case INTER_PHY_TYPE_5GBASE_R:
            if (phy_dev->an_enabled == 0)
                phy_serdes->serdes_speed_mode = MLN_SPD_FORCE_5G_R;
            else
                phy_serdes->serdes_speed_mode = MLN_SPD_AN_5G_KR_IEEE_CL73;
            break;

        case INTER_PHY_TYPE_2500BASE_X:
            if (phy_dev->an_enabled == 1)
                return -1;
            else
                phy_serdes->serdes_speed_mode = MLN_SPD_FORCE_2P5G;
            break;
        case INTER_PHY_TYPE_2P5GBASE_R:
            if (phy_dev->an_enabled == 1)
                return -1;
            else
                phy_serdes->serdes_speed_mode = MLN_SPD_FORCE_2P5G_R;
            break;
        case INTER_PHY_TYPE_2P5GBASE_X:
            if (phy_dev->an_enabled == 0)
                phy_serdes->serdes_speed_mode = MLN_SPD_FORCE_2P5G_X;
            else
                phy_serdes->serdes_speed_mode = MLN_SPD_AN_2P5G_KX_IEEE_CL73;
            break;

        case INTER_PHY_TYPE_1000BASE_X:
            if (phy_dev->an_enabled == 0)
                phy_serdes->serdes_speed_mode = MLN_SPD_FORCE_1G;
            else
                phy_serdes->serdes_speed_mode = MLN_SPD_AN_1G_IEEE_CL37;
            break;
        case INTER_PHY_TYPE_1GBASE_X:
            if (phy_dev->an_enabled == 1)
                return -1;
            else
                phy_serdes->serdes_speed_mode = MLN_SPD_FORCE_1G_X;
            break;
        case INTER_PHY_TYPE_1GBASE_R:
            if (phy_dev->an_enabled == 1)
                return -1;
            else
                phy_serdes->serdes_speed_mode = MLN_SPD_FORCE_1G_R;
            break;

        default:
            return -1;
    }

    merlin_pmi_write16(CoreNum, 0x0, 0x7, 0x0010, 
        (phy_serdes->adv_caps & (PHY_CAP_PAUSE|PHY_CAP_PAUSE_ASYM)), ~(PHY_CAP_PAUSE|PHY_CAP_PAUSE_ASYM));

    return 0;
}

#if 0
static void merlin_pcs_loopback(int CoreNum)
{
    print_log("MerlinSupport::%s(): PCS Loopback Core #%d \n", __func__, CoreNum);
    merlin_pmi_write16(CoreNum, 0x0, 0x3, 0x9109, 0x1001, 0xefff);
}
#endif

static int merline_mphy_speed_set_port(phy_dev_t *phy_dev)
{
    int CoreNum = phy_dev->core_index;
    int PortNum = phy_dev->usxgmii_m_index;
    phy_serdes_t *phy_serdes = phy_dev->priv;

    print_log("USXGMII-M Step7: Release PCS reset and set port speed of Core %d, Port %d.\n", CoreNum, PortNum);
    merlin_lane_config_speed(phy_dev->core_index, phy_dev->usxgmii_m_index, 
        phy_serdes->serdes_speed_mode, phy_dev->an_enabled);

    /* Enable LPI passing through */
    merlin_reg_prog(usxgmii_mp_lpi_enable, CoreNum, PortNum);

    print_log("USXGMII-M Step13: Release port reset of Core %d, Port %d.\n", CoreNum, PortNum);
    merlin_reg_prog(usxgmii_port_reset, phy_dev->core_index, phy_dev->usxgmii_m_index);
    return 0;
}

static int merlin_set_status_for_speed_change(phy_dev_t *phy_dev)
{
    merlin_reg_prog(change_speed, phy_dev->core_index, 0);
    return 0;
}

static int merline_speed_set_core(phy_dev_t *phy_dev)
{
    bool   vco_12p5g;
    int     vco_rate_real;
    uint16_t vco_rate;
    phy_serdes_t *phy_serdes = phy_dev->priv;
    uint32 CoreNum = phy_dev->core_index;
    uint32 PortNum = phy_dev->usxgmii_m_index;
    uint32 UBAUD = 0; // UBAUD : USXGMII baud rate
    uint32 M4_SPEED = phy_serdes->serdes_speed_mode;
    int phy_mode = phy_dev->current_inter_phy_type;

    /*
       0xC301[0] definition in merlinU
0: 10G baud rate
1:  5G baue rate
     */

    if (phy_dev_is_mphy(phy_dev) && (!mphy_is_master(phy_dev) || phy_serdes->inited >= 2))
        return merline_mphy_speed_set_port(phy_dev);

    merlin_set_status_for_speed_change(phy_dev);

    print_log("MerlinSupport::%s(): Step 7 Config Speed to %d\n", __func__, M4_SPEED);
    //---Step 7.b wait for uc_active = 1
    //print_log("MerlinSupport::%s(): Step 7.b wait for uc_active = 1 \n", __func__);

    //--Step 7.c Initialize software information table for the micro: merlin16_shortfin_init_merlin16_shortfin_info()
    //print_log("MerlinSupport::%s(): Step 7.c Initialize software information table for the micro \n", __func__);

    restore_regs(CoreNum);  // Restore all registers to default values for new speed programing
    //all 8b/10b will operate at VCO of 12.5GHz
    if (parse_sim_opts("-d ML_C_VCO_10P3125") ) {
        vco_12p5g = false;
    } else {
        /* To reduce EMI and save the energy consumption, we will use 10.315G as much as possible */
        vco_12p5g = phy_mode == INTER_PHY_TYPE_1GBASE_X ||
            phy_mode == INTER_PHY_TYPE_2P5GBASE_X || phy_mode == INTER_PHY_TYPE_2500BASE_X ||
            phy_mode == INTER_PHY_TYPE_5GBASE_X || phy_mode == INTER_PHY_TYPE_5000BASE_X;
        /*
        vco_12p5g = (M4_SPEED == MLN_SPD_FORCE_5G)       || (M4_SPEED == MLN_SPD_FORCE_5G_XGMII)       ||  //CL36
            (M4_SPEED == MLN_SPD_FORCE_2P5G)     || (M4_SPEED == MLN_SPD_FORCE_2P5G_XGMII)     ||
            (M4_SPEED == MLN_SPD_FORCE_1G)       || (M4_SPEED == MLN_SPD_FORCE_1G_XGMII)       ||
            (M4_SPEED == MLN_SPD_FORCE_1G_KX1)   || (M4_SPEED == MLN_SPD_FORCE_1G_KX1_XGMII)   ||
            // CL73 not working for 12p5g (M4_SPEED == MLN_SPD_AN_1G_KX_IEEE_CL73)  || (M4_SPEED == MLN_SPD_AN_1G_KX_IEEE_CL73_XGMII)      ||
            // CL73 not working for 12p5g (M4_SPEED == MLN_SPD_AN_1G_USER_CL73)||
            (M4_SPEED == MLN_SPD_AN_1G_IEEE_CL37)||
            (M4_SPEED == MLN_SPD_AN_1G_USER_CL37)||
            (M4_SPEED == MLN_SPD_FORCE_100M)     || (M4_SPEED == MLN_SPD_FORCE_10M)            ||
            (M4_SPEED == MLN_SPD_FORCE_5G_X)     || (M4_SPEED == MLN_SPD_FORCE_2P5G_X)         ||  //CL127
            (M4_SPEED == MLN_SPD_FORCE_1G_X)     ||
            (M4_SPEED == MLN_SPD_AN_1G_SGMII)    ||                                                //SGMII
            (M4_SPEED == MLN_SPD_AN_100M_SGMII)  || (M4_SPEED == MLN_SPD_AN_10M_SGMII)         ||
            (M4_SPEED == MLN_SPD_AN_SGMII_SLAVE) || (M4_SPEED == MLN_SPD_AN_SGMII_SLAVE_XGMII)
            ;
            */
    }


    //--- Step 8 PLL/PMD setup configuration
    print_log("--- Step 8 PLL/PMD setup configuration for speed %d, mode %d.\n", phy_serdes->current_speed, phy_mode);
    if (parse_sim_opts("-d separate_vco")) {
        if (vco_12p5g) {
            print_log("MerlinSupport::%s(): Initialize 12.5G VCO programming \n", __func__);
            merlin_reg_prog(Initialize_12p5_VCO, CoreNum, 0x0);
        }
    } else { // Do nothing
#if 0 // Simulation code; removed
        if (parse_sim_opts("-d ML_C_VCO_12P5") || vco_12p5g) {
            print_log("MerlinSupport::%s(): Initialize 12.5G VCO programming \n", __func__);
            merlin_reg_prog(Initialize_12p5_VCO, CoreNum, 0x0);
        }
#endif
    }

#if defined(CONFIG_BCM96765)
    /* For 80MHZ reference clock */
    if (vco_12p5g)
    {
        print_log("MerlinSupport::%s(): PMD Setup 80MHz, 12.5GHz VCO programming \n", __func__);
        merlin_reg_prog(PMD_setup_80_12p5_VCO, CoreNum, 0x0); // If 12G
    }
    else
    {
        print_log("MerlinSupport::%s(): PMD Setup 80MHz, 10.3125GHz VCO programming \n", __func__);
        merlin_reg_prog(PMD_setup_80_10p3125_VCO, CoreNum, 0x0); // If 10G
    }
#else
    if (parse_sim_opts("-d ML_REFCLK_50")) {
        if (parse_sim_opts("-d separate_vco"))
        {
            if (vco_12p5g)
            {
                print_log("MerlinSupport::%s(): PMD Setup 50MHz, 12.5GHz VCO programming \n", __func__);
                merlin_reg_prog(PMD_setup_50_12p5_VCO, CoreNum, 0x0); // If not 10G
            }
            else
            {
                print_log("MerlinSupport::%s(): PMD Setup 50MHz, 10.3125GHz VCO programming \n", __func__);
                merlin_reg_prog(PMD_setup_50_10p3125_VCO, CoreNum, 0x0); // If 10G
            }

        }
        else
        {
            if (parse_sim_opts("-d ML_C_VCO_12P5") || vco_12p5g) {
                print_log("MerlinSupport::%s(): PMD Setup 50MHz, 12.5GHz VCO programming \n", __func__);
                merlin_reg_prog(PMD_setup_50_12p5_VCO, CoreNum, 0x0);
            } else {
                print_log("MerlinSupport::%s(): PMD Setup 50MHz, 10.3125GHz VCO programming \n", __func__);
                merlin_reg_prog(PMD_setup_50_10p3125_VCO, CoreNum, 0x0);
            }

        }
    } else if (parse_sim_opts("-d ML_REFCLK_400")) {
        if (parse_sim_opts("-d ML_C_VCO_12P5") || vco_12p5g) {
            print_log("MerlinSupport::%s(): PMD Setup 400MHz, 12.5GHz VCO programming \n", __func__);
            merlin_reg_prog(PMD_setup_400_12p5_VCO, CoreNum, 0x0);
        } else {
            print_log("MerlinSupport::%s(): PMD Setup 400MHz, 10.3125GHz VCO programming \n", __func__);
            merlin_reg_prog(PMD_setup_400_10p3125_VCO, CoreNum, 0x0);
        }
    } else {  //default to 156.25MHz
        if (parse_sim_opts("-d ML_C_VCO_12P5") || vco_12p5g) {
            print_log("MerlinSupport::%s(): PMD Setup 156.25MHz, 12.5GHz VCO programming \n", __func__);
            merlin_reg_prog(PMD_setup_156p25_12p5_VCO, CoreNum, 0x0);
        } else {
            print_log("MerlinSupport::%s(): PMD Setup 156.25MHz, 10.3125GHz VCO programming \n", __func__);
            merlin_reg_prog(PMD_setup_156p25_10p3125_VCO, CoreNum, 0x0); //CoreNum = 0x1f for broadcast
        }
    }
#endif

    /* Wait UC active just in case VCO change caused UC status change */
    merlin_wait_uc_active (CoreNum);
    if (parse_sim_opts("-d MERLIN_LOAD_FIRMWARE") ) {
        //---Step 9. Configure Core level regsiter
        print_log("MerlinSupport::%s(): Step 9. Configure Core level regsiter \n", __func__);
        //1. hearbeat_count_1us : set before firmware download (already done in step 3)
        //2. PLL: (already done in step 8.)
        //3. RefClk related
        //4. rx/tx_lane_addr

        //--- Step 10. Set core_congif_from_pcs
        print_log("MerlinSupport::%s(): Step 10. Set core_congif_from_pcs \n", __func__);
        if (parse_sim_opts("-d ML_C_VCO_12P5") || vco_12p5g) {
            vco_rate_real = (12.5 * 4.0) - 22.0;     //28
        } else {
            vco_rate_real = (10.3125 * 4.0) - 22.0;  //19.25
        }
        vco_rate = vco_rate_real;
        print_log("MerlinSupport::%s(): RAM variable vco_rate is %d \n", __func__, vco_rate);
        merlin_cfg_core_ram_var (CoreNum, vco_rate);
    }

    //----------------------------------
    //USXGMII-specific
    if (UBAUD == 0x1) { //USXGMII 5G baud rate duplacated symbol; 84881 MAX speed; SET_USXGMII(MAX_SPEED,)
        // For FORCE SPEED; Only USXGMII mode
        print_log("MerlinSupport::CONFIGURING USXGMII Baud Rate to 5G\n");
        merlin_reg_prog(set_usxgmii_5g_baud, CoreNum, 0x0);
    }

    //--- Step 12. Reset Datapath (core)
    if (phy_dev->usxgmii_m_type != USXGMII_S)   /* Enable MAC credit for USXGMII-M mode before core reset */
        merline_usxgmii_mac_credit_enable(CoreNum, 0);

    //merlin_pcs_loopback(CoreNum);
    print_log("MerlinSupport::%s(): Step 12. Reset Datapath (core) \n", __func__);
    merlin_reset_datapath_core (CoreNum);

    //--- Step 13. Lane Configuration
    print_log("MerlinSupport::%s(): Step 13. Lane Configuration \n", __func__);
    if (parse_sim_opts("-d MERLIN_LOAD_FIRMWARE")) {
        //--- Step 13.a. Configure lane registers
        print_log("MerlinSupport::%s(): Step 13.a. Configure lane registers \n", __func__);
        //Lane General
        //1. cl72_ieee_traning_enable
#if 0
        if (parse_sim_opts("-d ML_C_FORCE_10G_R") ||  // 10G speed
                parse_sim_opts("-d ML_C_FORCE_10G_R_CL74") ||
                parse_sim_opts("-d ML_C_AN_10G_KR_IEEE_CL73") ||
                parse_sim_opts("-d ML_C_AN_10G_KR_IEEE_CL73_CL74") ||
                parse_sim_opts("-d ML_C_AN_10G_USER_CL73") ||
                parse_sim_opts("-d ML_C_AN_10G_USER_CL73_CL74") ) {
            merlin_pmi_write16(CoreNum, 0x0, 0x1, 0x0096, 0x0002, 0xfffd);  //[1]:cl72_ieee_training_enable  1: enable 10GBASE-KR; 0: disable 10GBASE-KR  start-up protocol; To improve signal
            // 3.7.2.2 10GKR AN only; CL72; IEEE training
        }
#endif
        //2.eee_mode_en: Enable EEE functionality (no officially suppported)
        //3.osr_mode_frc, osr_mode_frc_val:  override OSR mode input pin from PCS/AN
        //4.rx_pmd_dp_invert, tx_pmd_dp_invert: polarity inversion for RX and TX lane

        //TX Lane*
        //1. Analog TX (FIR & HPF): API merlin16_tx_analog_functions.c. It is required to configure the TXFIR according to desired electrical specificaitons
        if (phy_serdes->current_speed == PHY_SPEED_10000)
            merlin_pmi_write16(CoreNum, 0x0, 0x1, 0xd0a2, 0x0007, 0xfff0);  //[3:0]:en_hpf = 0x7;
        else
            merlin_pmi_write16(CoreNum, 0x0, 0x1, 0xd0a2, 0x0000, 0xfff0);  //[3:0]:en_hpf = 0x7;


        //--- Step 13.b. Configure lane's micro RAM vaiables
        #if 0
        print_log("MerlinSupport::%s(): Step 13.b. Configure lane's micro RAM vaiables \n", __func__);
        if (parse_sim_opts("-d ML_C_AN_1G_IEEE_CL37") ||    // If it is AN mode
                parse_sim_opts("-d ML_C_AN_1G_USER_CL37") ||
                parse_sim_opts("-d ML_C_AN_1G_KX_IEEE_CL73") ||
                parse_sim_opts("-d ML_C_AN_1G_USER_CL73") ||
                parse_sim_opts("-d ML_C_AN_10G_KR_IEEE_CL73") ||
                parse_sim_opts("-d ML_C_AN_10G_KR_IEEE_CL73_CL74") || // FEC, TX/RX
                parse_sim_opts("-d ML_C_AN_10G_USER_CL73") ||
                parse_sim_opts("-d ML_C_AN_10G_USER_CL73_CL74") ||
                parse_sim_opts("-d ML_C_AN_1G_SGMII") ||
                parse_sim_opts("-d ML_C_AN_100M_SGMII") ||
                parse_sim_opts("-d ML_C_AN_10M_SGMII") ||
                parse_sim_opts("-d ML_C_AN_10G_USXGMII") ||
                parse_sim_opts("-d ML_C_AN_5G_USXGMII") ||
                parse_sim_opts("-d ML_C_AN_2P5G_USXGMII") ||
                parse_sim_opts("-d ML_C_AN_1G_USXGMII") ||
                parse_sim_opts("-d ML_C_AN_100M_USXGMII")) {
            an_enabled = 0x1;
        }
        #endif
        print_log("MerlinSupport::%s(): RAM variable an_enabled is %d \n", __func__, phy_dev->an_enabled);
        merlin_cfg_lane_ram_var (phy_dev, phy_dev->an_enabled);  // set ram var
    }

    //Program lane's speed mode
    if (phy_dev->usxgmii_m_type != USXGMII_S)
    {
        print_log("USXGMII-M Step6: Data path reset lane.\n");
        merlin_reg_prog(datapath_reset_lane, CoreNum, 0);   

        print_log("USXGMII-M Step7: Release PCS reset and set port speed of Core %d, Port %d.\n", CoreNum, PortNum);
        merlin_reg_prog(usxgmii_pcs_reset_and_enable, CoreNum, 0);
        merlin_lane_config_speed(CoreNum, PortNum, M4_SPEED, phy_dev->an_enabled);
    }
    else
        merlin_lane_config_speed(CoreNum, 0x0, M4_SPEED, phy_dev->an_enabled);

    //--- --- check PLL lock status // Need to move
    merlin_chk_pll_lock (CoreNum);

    /* Enable LPI passing through */
    merlin_reg_prog(lpi_enable, CoreNum, 0x0);
    if (phy_dev->usxgmii_m_type != USXGMII_S)
    {
    /* Don't enable this for now */
    #if 0
        print_log("USXGMII-M Step10: Enable RX IPG repair.\n");
        merlin_pmi_write16(CoreNum, 0, 3, 0xc692, (0x2<<14), (~(0x3<<14))&0xffff);
    #endif

        print_log("USXGMII-M Step13: Release port reset of Core %d, Port %d.\n", CoreNum, PortNum);
        /* Enable LPI passing through */
        merlin_reg_prog(usxgmii_mp_lpi_enable, CoreNum, PortNum);

        merlin_reg_prog(usxgmii_port_reset, CoreNum, PortNum);
    }

    return 0;
}

int merlin_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    int old_serdes_speed_mode = phy_serdes->serdes_speed_mode;
    int rc;

    if (phy_speed_to_merlin_speed(phy_dev) == -1)
        return 0;

    if (phy_serdes->cur_power_level == SERDES_POWER_DOWN)
        return 0;

    /* If we are moving away from USXGMII mode; do USXGMII work around */
    if (phy_serdes->sfp_module_type != SFP_FIXED_PHY &&
        old_serdes_speed_mode == MLN_SPD_AN_USXGMII_SLAVE &&
        phy_serdes->serdes_speed_mode != old_serdes_speed_mode)
        usxgmii_workaround(phy_dev);

    /* Keep USXGMII(_MP) mode stable for shared MSBUS clock */
    if (phy_serdes->serdes_speed_mode == old_serdes_speed_mode &&
        phy_serdes->inited == 3)
        return 0;

    rc = merline_speed_set_core(phy_dev);
    return rc;
}

// void merlin_chip_init(uint32 CoreNum, uint32 M4_SPEED, uint32 UBAUD, bool LINK_CHK)
int merlin16_serdes_init(phy_dev_t *phy_dev)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    phy_dev_t *sa__ = phy_dev;
    uint32 CoreNum = phy_dev->core_index;
    uint32 PortNum = phy_dev->usxgmii_m_index;
    phy_speed_t saved_config_speed = phy_serdes->config_speed;
    int saved_xfi_mode = phy_dev->current_inter_phy_type;

    //--- Step 0 powerup/reset sequence
    print_log("--- Step 0 powerup/reset sequence of core #%d at address %d\n", CoreNum, phy_dev->addr);

    if (phy_serdes->inited < 2)    /* Power On initialization, do a through power reset */
    {
        _merlin_core_power_op(phy_dev, SERDES_POWER_UP);    /* Make next power down work without bypassed */
        _merlin_core_power_op(phy_dev, SERDES_POWER_DOWN);
        _merlin_core_power_op(phy_dev, SERDES_POWER_UP);
    }

    register_dont_save = 1;
    if (mphy_is_master(phy_dev))
        merlin_switch_a_to_b(phy_dev, CoreNum, 0);

    register_dont_save = 0;

    merlin_core_init(phy_dev);
    timeout_ns(1000);

    // FEC: Coding, mismatching will not link up
    // Training might link up with mismatching

    if (parse_sim_opts("-d MERLIN_LOAD_FIRMWARE") ) {
        //--- --- Load firmware ?  TBD
        //---Step 6. Micro code load and verify
        print_log("MerlinSupport::%s(): Step 6. Micro code load and verify \n", __func__);
        merlin_load_firmware(phy_dev);
    }

    if (phy_serdes->signal_detect_gpio != -1)
        if (wr_ext_los_en(1))
            print_log("wr_ext_los_en() returns error\n");

    /* Set default 1G speed to fully exercise hardware status */
    phy_serdes->config_speed = PHY_SPEED_1000;
    phy_dev->current_inter_phy_type = INTER_PHY_TYPE_1000BASE_X;

    merlin_speed_set(phy_dev, phy_serdes->config_speed, phy_dev->duplex);

    phy_serdes->config_speed = saved_config_speed;
    phy_dev->current_inter_phy_type = saved_xfi_mode;

    print_log("INFO MerlinSupport::%s(): END Merlin Initialization procedure\n", __func__);

    phy_serdes->inited = 1;
    return 0;
}

int merlin_ext_signal_detected(phy_dev_t *phy_dev)
{
    phy_dev_t *sa__ = phy_dev;
    err_code_t __err;
    phy_serdes_t *phy_serdes = phy_dev->priv;
    uint32_t detected;
    int invert = rd_ext_los_inv() > 0;

    /* Flip RX_LOS to check valid RX_LOS level we can see the differential signal on RX_D line */
    detected = (merlin_pmi_read16(phy_dev->core_index, 0, 1, 0xd0c8) & (1<<0)) > 0;
    if (!detected)
    {
        wr_ext_los_inv(!invert);
        msleep(1);
        detected = (merlin_pmi_read16(phy_dev->core_index, 0, 1, 0xd0c8) & (1<<0)) > 0;
        if (!detected)
            wr_ext_los_inv(phy_serdes->signal_detect_invert);
    }

    return detected;
}
