/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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

#include "bcm63158_drivers_xport_ag.h"
#include "bcm63158_xport_mib_reg_ag.h"
int ag_drv_xport_mib_reg_dir_acc_data_write_set(uint32_t write_data)
{
    uint32_t reg_dir_acc_data_write=0;

#ifdef VALIDATE_PARMS
#endif

    reg_dir_acc_data_write = RU_FIELD_SET(0, XPORT_MIB_REG, DIR_ACC_DATA_WRITE, WRITE_DATA, reg_dir_acc_data_write, write_data);

    RU_REG_WRITE(0, XPORT_MIB_REG, DIR_ACC_DATA_WRITE, reg_dir_acc_data_write);

    return 0;
}

int ag_drv_xport_mib_reg_dir_acc_data_write_get(uint32_t *write_data)
{
    uint32_t reg_dir_acc_data_write=0;

#ifdef VALIDATE_PARMS
    if(!write_data)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, DIR_ACC_DATA_WRITE, reg_dir_acc_data_write);

    *write_data = RU_FIELD_GET(0, XPORT_MIB_REG, DIR_ACC_DATA_WRITE, WRITE_DATA, reg_dir_acc_data_write);

    return 0;
}

int ag_drv_xport_mib_reg_dir_acc_data_read_set(uint32_t read_data)
{
    uint32_t reg_dir_acc_data_read=0;

#ifdef VALIDATE_PARMS
#endif

    reg_dir_acc_data_read = RU_FIELD_SET(0, XPORT_MIB_REG, DIR_ACC_DATA_READ, READ_DATA, reg_dir_acc_data_read, read_data);

    RU_REG_WRITE(0, XPORT_MIB_REG, DIR_ACC_DATA_READ, reg_dir_acc_data_read);

    return 0;
}

int ag_drv_xport_mib_reg_dir_acc_data_read_get(uint32_t *read_data)
{
    uint32_t reg_dir_acc_data_read=0;

#ifdef VALIDATE_PARMS
    if(!read_data)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, DIR_ACC_DATA_READ, reg_dir_acc_data_read);

    *read_data = RU_FIELD_GET(0, XPORT_MIB_REG, DIR_ACC_DATA_READ, READ_DATA, reg_dir_acc_data_read);

    return 0;
}

int ag_drv_xport_mib_reg_indir_acc_addr_0_set(const xport_mib_reg_indir_acc_addr_0 *indir_acc_addr_0)
{
    uint32_t reg_indir_acc_addr_0=0;

#ifdef VALIDATE_PARMS
    if(!indir_acc_addr_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((indir_acc_addr_0->err >= _1BITS_MAX_VAL_) ||
       (indir_acc_addr_0->start_busy >= _1BITS_MAX_VAL_) ||
       (indir_acc_addr_0->r_w >= _1BITS_MAX_VAL_) ||
       (indir_acc_addr_0->reg_port_id >= _2BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_indir_acc_addr_0 = RU_FIELD_SET(0, XPORT_MIB_REG, INDIR_ACC_ADDR_0, ERR, reg_indir_acc_addr_0, indir_acc_addr_0->err);
    reg_indir_acc_addr_0 = RU_FIELD_SET(0, XPORT_MIB_REG, INDIR_ACC_ADDR_0, START_BUSY, reg_indir_acc_addr_0, indir_acc_addr_0->start_busy);
    reg_indir_acc_addr_0 = RU_FIELD_SET(0, XPORT_MIB_REG, INDIR_ACC_ADDR_0, R_W, reg_indir_acc_addr_0, indir_acc_addr_0->r_w);
    reg_indir_acc_addr_0 = RU_FIELD_SET(0, XPORT_MIB_REG, INDIR_ACC_ADDR_0, REG_PORT_ID, reg_indir_acc_addr_0, indir_acc_addr_0->reg_port_id);
    reg_indir_acc_addr_0 = RU_FIELD_SET(0, XPORT_MIB_REG, INDIR_ACC_ADDR_0, REG_OFFSET, reg_indir_acc_addr_0, indir_acc_addr_0->reg_offset);

    RU_REG_WRITE(0, XPORT_MIB_REG, INDIR_ACC_ADDR_0, reg_indir_acc_addr_0);

    return 0;
}

int ag_drv_xport_mib_reg_indir_acc_addr_0_get(xport_mib_reg_indir_acc_addr_0 *indir_acc_addr_0)
{
    uint32_t reg_indir_acc_addr_0=0;

#ifdef VALIDATE_PARMS
    if(!indir_acc_addr_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, INDIR_ACC_ADDR_0, reg_indir_acc_addr_0);

    indir_acc_addr_0->err = RU_FIELD_GET(0, XPORT_MIB_REG, INDIR_ACC_ADDR_0, ERR, reg_indir_acc_addr_0);
    indir_acc_addr_0->start_busy = RU_FIELD_GET(0, XPORT_MIB_REG, INDIR_ACC_ADDR_0, START_BUSY, reg_indir_acc_addr_0);
    indir_acc_addr_0->r_w = RU_FIELD_GET(0, XPORT_MIB_REG, INDIR_ACC_ADDR_0, R_W, reg_indir_acc_addr_0);
    indir_acc_addr_0->reg_port_id = RU_FIELD_GET(0, XPORT_MIB_REG, INDIR_ACC_ADDR_0, REG_PORT_ID, reg_indir_acc_addr_0);
    indir_acc_addr_0->reg_offset = RU_FIELD_GET(0, XPORT_MIB_REG, INDIR_ACC_ADDR_0, REG_OFFSET, reg_indir_acc_addr_0);

    return 0;
}

int ag_drv_xport_mib_reg_indir_acc_data_low_0_set(uint32_t data_low)
{
    uint32_t reg_indir_acc_data_low_0=0;

#ifdef VALIDATE_PARMS
#endif

    reg_indir_acc_data_low_0 = RU_FIELD_SET(0, XPORT_MIB_REG, INDIR_ACC_DATA_LOW_0, DATA_LOW, reg_indir_acc_data_low_0, data_low);

    RU_REG_WRITE(0, XPORT_MIB_REG, INDIR_ACC_DATA_LOW_0, reg_indir_acc_data_low_0);

    return 0;
}

int ag_drv_xport_mib_reg_indir_acc_data_low_0_get(uint32_t *data_low)
{
    uint32_t reg_indir_acc_data_low_0=0;

#ifdef VALIDATE_PARMS
    if(!data_low)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, INDIR_ACC_DATA_LOW_0, reg_indir_acc_data_low_0);

    *data_low = RU_FIELD_GET(0, XPORT_MIB_REG, INDIR_ACC_DATA_LOW_0, DATA_LOW, reg_indir_acc_data_low_0);

    return 0;
}

int ag_drv_xport_mib_reg_indir_acc_data_high_0_set(uint32_t data_high)
{
    uint32_t reg_indir_acc_data_high_0=0;

#ifdef VALIDATE_PARMS
#endif

    reg_indir_acc_data_high_0 = RU_FIELD_SET(0, XPORT_MIB_REG, INDIR_ACC_DATA_HIGH_0, DATA_HIGH, reg_indir_acc_data_high_0, data_high);

    RU_REG_WRITE(0, XPORT_MIB_REG, INDIR_ACC_DATA_HIGH_0, reg_indir_acc_data_high_0);

    return 0;
}

int ag_drv_xport_mib_reg_indir_acc_data_high_0_get(uint32_t *data_high)
{
    uint32_t reg_indir_acc_data_high_0=0;

#ifdef VALIDATE_PARMS
    if(!data_high)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, INDIR_ACC_DATA_HIGH_0, reg_indir_acc_data_high_0);

    *data_high = RU_FIELD_GET(0, XPORT_MIB_REG, INDIR_ACC_DATA_HIGH_0, DATA_HIGH, reg_indir_acc_data_high_0);

    return 0;
}

int ag_drv_xport_mib_reg_indir_acc_addr_1_set(const xport_mib_reg_indir_acc_addr_1 *indir_acc_addr_1)
{
    uint32_t reg_indir_acc_addr_1=0;

#ifdef VALIDATE_PARMS
    if(!indir_acc_addr_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((indir_acc_addr_1->err >= _1BITS_MAX_VAL_) ||
       (indir_acc_addr_1->start_busy >= _1BITS_MAX_VAL_) ||
       (indir_acc_addr_1->r_w >= _1BITS_MAX_VAL_) ||
       (indir_acc_addr_1->reg_port_id >= _2BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_indir_acc_addr_1 = RU_FIELD_SET(0, XPORT_MIB_REG, INDIR_ACC_ADDR_1, ERR, reg_indir_acc_addr_1, indir_acc_addr_1->err);
    reg_indir_acc_addr_1 = RU_FIELD_SET(0, XPORT_MIB_REG, INDIR_ACC_ADDR_1, START_BUSY, reg_indir_acc_addr_1, indir_acc_addr_1->start_busy);
    reg_indir_acc_addr_1 = RU_FIELD_SET(0, XPORT_MIB_REG, INDIR_ACC_ADDR_1, R_W, reg_indir_acc_addr_1, indir_acc_addr_1->r_w);
    reg_indir_acc_addr_1 = RU_FIELD_SET(0, XPORT_MIB_REG, INDIR_ACC_ADDR_1, REG_PORT_ID, reg_indir_acc_addr_1, indir_acc_addr_1->reg_port_id);
    reg_indir_acc_addr_1 = RU_FIELD_SET(0, XPORT_MIB_REG, INDIR_ACC_ADDR_1, REG_OFFSET, reg_indir_acc_addr_1, indir_acc_addr_1->reg_offset);

    RU_REG_WRITE(0, XPORT_MIB_REG, INDIR_ACC_ADDR_1, reg_indir_acc_addr_1);

    return 0;
}

int ag_drv_xport_mib_reg_indir_acc_addr_1_get(xport_mib_reg_indir_acc_addr_1 *indir_acc_addr_1)
{
    uint32_t reg_indir_acc_addr_1=0;

#ifdef VALIDATE_PARMS
    if(!indir_acc_addr_1)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, INDIR_ACC_ADDR_1, reg_indir_acc_addr_1);

    indir_acc_addr_1->err = RU_FIELD_GET(0, XPORT_MIB_REG, INDIR_ACC_ADDR_1, ERR, reg_indir_acc_addr_1);
    indir_acc_addr_1->start_busy = RU_FIELD_GET(0, XPORT_MIB_REG, INDIR_ACC_ADDR_1, START_BUSY, reg_indir_acc_addr_1);
    indir_acc_addr_1->r_w = RU_FIELD_GET(0, XPORT_MIB_REG, INDIR_ACC_ADDR_1, R_W, reg_indir_acc_addr_1);
    indir_acc_addr_1->reg_port_id = RU_FIELD_GET(0, XPORT_MIB_REG, INDIR_ACC_ADDR_1, REG_PORT_ID, reg_indir_acc_addr_1);
    indir_acc_addr_1->reg_offset = RU_FIELD_GET(0, XPORT_MIB_REG, INDIR_ACC_ADDR_1, REG_OFFSET, reg_indir_acc_addr_1);

    return 0;
}

int ag_drv_xport_mib_reg_indir_acc_data_low_1_set(uint32_t data_low)
{
    uint32_t reg_indir_acc_data_low_1=0;

#ifdef VALIDATE_PARMS
#endif

    reg_indir_acc_data_low_1 = RU_FIELD_SET(0, XPORT_MIB_REG, INDIR_ACC_DATA_LOW_1, DATA_LOW, reg_indir_acc_data_low_1, data_low);

    RU_REG_WRITE(0, XPORT_MIB_REG, INDIR_ACC_DATA_LOW_1, reg_indir_acc_data_low_1);

    return 0;
}

int ag_drv_xport_mib_reg_indir_acc_data_low_1_get(uint32_t *data_low)
{
    uint32_t reg_indir_acc_data_low_1=0;

#ifdef VALIDATE_PARMS
    if(!data_low)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, INDIR_ACC_DATA_LOW_1, reg_indir_acc_data_low_1);

    *data_low = RU_FIELD_GET(0, XPORT_MIB_REG, INDIR_ACC_DATA_LOW_1, DATA_LOW, reg_indir_acc_data_low_1);

    return 0;
}

int ag_drv_xport_mib_reg_indir_acc_data_high_1_set(uint32_t data_high)
{
    uint32_t reg_indir_acc_data_high_1=0;

#ifdef VALIDATE_PARMS
#endif

    reg_indir_acc_data_high_1 = RU_FIELD_SET(0, XPORT_MIB_REG, INDIR_ACC_DATA_HIGH_1, DATA_HIGH, reg_indir_acc_data_high_1, data_high);

    RU_REG_WRITE(0, XPORT_MIB_REG, INDIR_ACC_DATA_HIGH_1, reg_indir_acc_data_high_1);

    return 0;
}

int ag_drv_xport_mib_reg_indir_acc_data_high_1_get(uint32_t *data_high)
{
    uint32_t reg_indir_acc_data_high_1=0;

#ifdef VALIDATE_PARMS
    if(!data_high)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, INDIR_ACC_DATA_HIGH_1, reg_indir_acc_data_high_1);

    *data_high = RU_FIELD_GET(0, XPORT_MIB_REG, INDIR_ACC_DATA_HIGH_1, DATA_HIGH, reg_indir_acc_data_high_1);

    return 0;
}

int ag_drv_xport_mib_reg_cntrl_set(uint8_t eee_cnt_mode, uint8_t saturate_en, uint8_t cor_en, uint8_t cnt_rst)
{
    uint32_t reg_cntrl=0;

#ifdef VALIDATE_PARMS
    if((eee_cnt_mode >= _4BITS_MAX_VAL_) ||
       (saturate_en >= _4BITS_MAX_VAL_) ||
       (cor_en >= _4BITS_MAX_VAL_) ||
       (cnt_rst >= _4BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_cntrl = RU_FIELD_SET(0, XPORT_MIB_REG, CNTRL, EEE_CNT_MODE, reg_cntrl, eee_cnt_mode);
    reg_cntrl = RU_FIELD_SET(0, XPORT_MIB_REG, CNTRL, SATURATE_EN, reg_cntrl, saturate_en);
    reg_cntrl = RU_FIELD_SET(0, XPORT_MIB_REG, CNTRL, COR_EN, reg_cntrl, cor_en);
    reg_cntrl = RU_FIELD_SET(0, XPORT_MIB_REG, CNTRL, CNT_RST, reg_cntrl, cnt_rst);

    RU_REG_WRITE(0, XPORT_MIB_REG, CNTRL, reg_cntrl);

    return 0;
}

int ag_drv_xport_mib_reg_cntrl_get(uint8_t *eee_cnt_mode, uint8_t *saturate_en, uint8_t *cor_en, uint8_t *cnt_rst)
{
    uint32_t reg_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!eee_cnt_mode || !saturate_en || !cor_en || !cnt_rst)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, CNTRL, reg_cntrl);

    *eee_cnt_mode = RU_FIELD_GET(0, XPORT_MIB_REG, CNTRL, EEE_CNT_MODE, reg_cntrl);
    *saturate_en = RU_FIELD_GET(0, XPORT_MIB_REG, CNTRL, SATURATE_EN, reg_cntrl);
    *cor_en = RU_FIELD_GET(0, XPORT_MIB_REG, CNTRL, COR_EN, reg_cntrl);
    *cnt_rst = RU_FIELD_GET(0, XPORT_MIB_REG, CNTRL, CNT_RST, reg_cntrl);

    return 0;
}

int ag_drv_xport_mib_reg_eee_pulse_duration_cntrl_set(uint8_t cnt)
{
    uint32_t reg_eee_pulse_duration_cntrl=0;

#ifdef VALIDATE_PARMS
#endif

    reg_eee_pulse_duration_cntrl = RU_FIELD_SET(0, XPORT_MIB_REG, EEE_PULSE_DURATION_CNTRL, CNT, reg_eee_pulse_duration_cntrl, cnt);

    RU_REG_WRITE(0, XPORT_MIB_REG, EEE_PULSE_DURATION_CNTRL, reg_eee_pulse_duration_cntrl);

    return 0;
}

int ag_drv_xport_mib_reg_eee_pulse_duration_cntrl_get(uint8_t *cnt)
{
    uint32_t reg_eee_pulse_duration_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!cnt)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, EEE_PULSE_DURATION_CNTRL, reg_eee_pulse_duration_cntrl);

    *cnt = RU_FIELD_GET(0, XPORT_MIB_REG, EEE_PULSE_DURATION_CNTRL, CNT, reg_eee_pulse_duration_cntrl);

    return 0;
}

int ag_drv_xport_mib_reg_gport0_max_pkt_size_set(uint16_t max_pkt_size)
{
    uint32_t reg_gport0_max_pkt_size=0;

#ifdef VALIDATE_PARMS
    if((max_pkt_size >= _14BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gport0_max_pkt_size = RU_FIELD_SET(0, XPORT_MIB_REG, GPORT0_MAX_PKT_SIZE, MAX_PKT_SIZE, reg_gport0_max_pkt_size, max_pkt_size);

    RU_REG_WRITE(0, XPORT_MIB_REG, GPORT0_MAX_PKT_SIZE, reg_gport0_max_pkt_size);

    return 0;
}

int ag_drv_xport_mib_reg_gport0_max_pkt_size_get(uint16_t *max_pkt_size)
{
    uint32_t reg_gport0_max_pkt_size=0;

#ifdef VALIDATE_PARMS
    if(!max_pkt_size)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, GPORT0_MAX_PKT_SIZE, reg_gport0_max_pkt_size);

    *max_pkt_size = RU_FIELD_GET(0, XPORT_MIB_REG, GPORT0_MAX_PKT_SIZE, MAX_PKT_SIZE, reg_gport0_max_pkt_size);

    return 0;
}

int ag_drv_xport_mib_reg_gport1_max_pkt_size_set(uint16_t max_pkt_size)
{
    uint32_t reg_gport1_max_pkt_size=0;

#ifdef VALIDATE_PARMS
    if((max_pkt_size >= _14BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gport1_max_pkt_size = RU_FIELD_SET(0, XPORT_MIB_REG, GPORT1_MAX_PKT_SIZE, MAX_PKT_SIZE, reg_gport1_max_pkt_size, max_pkt_size);

    RU_REG_WRITE(0, XPORT_MIB_REG, GPORT1_MAX_PKT_SIZE, reg_gport1_max_pkt_size);

    return 0;
}

int ag_drv_xport_mib_reg_gport1_max_pkt_size_get(uint16_t *max_pkt_size)
{
    uint32_t reg_gport1_max_pkt_size=0;

#ifdef VALIDATE_PARMS
    if(!max_pkt_size)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, GPORT1_MAX_PKT_SIZE, reg_gport1_max_pkt_size);

    *max_pkt_size = RU_FIELD_GET(0, XPORT_MIB_REG, GPORT1_MAX_PKT_SIZE, MAX_PKT_SIZE, reg_gport1_max_pkt_size);

    return 0;
}

int ag_drv_xport_mib_reg_gport2_max_pkt_size_set(uint16_t max_pkt_size)
{
    uint32_t reg_gport2_max_pkt_size=0;

#ifdef VALIDATE_PARMS
    if((max_pkt_size >= _14BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gport2_max_pkt_size = RU_FIELD_SET(0, XPORT_MIB_REG, GPORT2_MAX_PKT_SIZE, MAX_PKT_SIZE, reg_gport2_max_pkt_size, max_pkt_size);

    RU_REG_WRITE(0, XPORT_MIB_REG, GPORT2_MAX_PKT_SIZE, reg_gport2_max_pkt_size);

    return 0;
}

int ag_drv_xport_mib_reg_gport2_max_pkt_size_get(uint16_t *max_pkt_size)
{
    uint32_t reg_gport2_max_pkt_size=0;

#ifdef VALIDATE_PARMS
    if(!max_pkt_size)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, GPORT2_MAX_PKT_SIZE, reg_gport2_max_pkt_size);

    *max_pkt_size = RU_FIELD_GET(0, XPORT_MIB_REG, GPORT2_MAX_PKT_SIZE, MAX_PKT_SIZE, reg_gport2_max_pkt_size);

    return 0;
}

int ag_drv_xport_mib_reg_gport3_max_pkt_size_set(uint16_t max_pkt_size)
{
    uint32_t reg_gport3_max_pkt_size=0;

#ifdef VALIDATE_PARMS
    if((max_pkt_size >= _14BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_gport3_max_pkt_size = RU_FIELD_SET(0, XPORT_MIB_REG, GPORT3_MAX_PKT_SIZE, MAX_PKT_SIZE, reg_gport3_max_pkt_size, max_pkt_size);

    RU_REG_WRITE(0, XPORT_MIB_REG, GPORT3_MAX_PKT_SIZE, reg_gport3_max_pkt_size);

    return 0;
}

int ag_drv_xport_mib_reg_gport3_max_pkt_size_get(uint16_t *max_pkt_size)
{
    uint32_t reg_gport3_max_pkt_size=0;

#ifdef VALIDATE_PARMS
    if(!max_pkt_size)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, GPORT3_MAX_PKT_SIZE, reg_gport3_max_pkt_size);

    *max_pkt_size = RU_FIELD_GET(0, XPORT_MIB_REG, GPORT3_MAX_PKT_SIZE, MAX_PKT_SIZE, reg_gport3_max_pkt_size);

    return 0;
}

int ag_drv_xport_mib_reg_ecc_cntrl_set(uint8_t tx_mib_ecc_en, uint8_t rx_mib_ecc_en)
{
    uint32_t reg_ecc_cntrl=0;

#ifdef VALIDATE_PARMS
    if((tx_mib_ecc_en >= _1BITS_MAX_VAL_) ||
       (rx_mib_ecc_en >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_ecc_cntrl = RU_FIELD_SET(0, XPORT_MIB_REG, ECC_CNTRL, TX_MIB_ECC_EN, reg_ecc_cntrl, tx_mib_ecc_en);
    reg_ecc_cntrl = RU_FIELD_SET(0, XPORT_MIB_REG, ECC_CNTRL, RX_MIB_ECC_EN, reg_ecc_cntrl, rx_mib_ecc_en);

    RU_REG_WRITE(0, XPORT_MIB_REG, ECC_CNTRL, reg_ecc_cntrl);

    return 0;
}

int ag_drv_xport_mib_reg_ecc_cntrl_get(uint8_t *tx_mib_ecc_en, uint8_t *rx_mib_ecc_en)
{
    uint32_t reg_ecc_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!tx_mib_ecc_en || !rx_mib_ecc_en)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, ECC_CNTRL, reg_ecc_cntrl);

    *tx_mib_ecc_en = RU_FIELD_GET(0, XPORT_MIB_REG, ECC_CNTRL, TX_MIB_ECC_EN, reg_ecc_cntrl);
    *rx_mib_ecc_en = RU_FIELD_GET(0, XPORT_MIB_REG, ECC_CNTRL, RX_MIB_ECC_EN, reg_ecc_cntrl);

    return 0;
}

int ag_drv_xport_mib_reg_force_sb_ecc_err_set(const xport_mib_reg_force_sb_ecc_err *force_sb_ecc_err)
{
    uint32_t reg_force_sb_ecc_err=0;

#ifdef VALIDATE_PARMS
    if(!force_sb_ecc_err)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((force_sb_ecc_err->force_tx_mem3_serr >= _1BITS_MAX_VAL_) ||
       (force_sb_ecc_err->force_tx_mem2_serr >= _1BITS_MAX_VAL_) ||
       (force_sb_ecc_err->force_tx_mem1_serr >= _1BITS_MAX_VAL_) ||
       (force_sb_ecc_err->force_tx_mem0_serr >= _1BITS_MAX_VAL_) ||
       (force_sb_ecc_err->force_rx_mem4_serr >= _1BITS_MAX_VAL_) ||
       (force_sb_ecc_err->force_rx_mem3_serr >= _1BITS_MAX_VAL_) ||
       (force_sb_ecc_err->force_rx_mem2_serr >= _1BITS_MAX_VAL_) ||
       (force_sb_ecc_err->force_rx_mem1_serr >= _1BITS_MAX_VAL_) ||
       (force_sb_ecc_err->force_rx_mem0_serr >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_force_sb_ecc_err = RU_FIELD_SET(0, XPORT_MIB_REG, FORCE_SB_ECC_ERR, FORCE_TX_MEM3_SERR, reg_force_sb_ecc_err, force_sb_ecc_err->force_tx_mem3_serr);
    reg_force_sb_ecc_err = RU_FIELD_SET(0, XPORT_MIB_REG, FORCE_SB_ECC_ERR, FORCE_TX_MEM2_SERR, reg_force_sb_ecc_err, force_sb_ecc_err->force_tx_mem2_serr);
    reg_force_sb_ecc_err = RU_FIELD_SET(0, XPORT_MIB_REG, FORCE_SB_ECC_ERR, FORCE_TX_MEM1_SERR, reg_force_sb_ecc_err, force_sb_ecc_err->force_tx_mem1_serr);
    reg_force_sb_ecc_err = RU_FIELD_SET(0, XPORT_MIB_REG, FORCE_SB_ECC_ERR, FORCE_TX_MEM0_SERR, reg_force_sb_ecc_err, force_sb_ecc_err->force_tx_mem0_serr);
    reg_force_sb_ecc_err = RU_FIELD_SET(0, XPORT_MIB_REG, FORCE_SB_ECC_ERR, FORCE_RX_MEM4_SERR, reg_force_sb_ecc_err, force_sb_ecc_err->force_rx_mem4_serr);
    reg_force_sb_ecc_err = RU_FIELD_SET(0, XPORT_MIB_REG, FORCE_SB_ECC_ERR, FORCE_RX_MEM3_SERR, reg_force_sb_ecc_err, force_sb_ecc_err->force_rx_mem3_serr);
    reg_force_sb_ecc_err = RU_FIELD_SET(0, XPORT_MIB_REG, FORCE_SB_ECC_ERR, FORCE_RX_MEM2_SERR, reg_force_sb_ecc_err, force_sb_ecc_err->force_rx_mem2_serr);
    reg_force_sb_ecc_err = RU_FIELD_SET(0, XPORT_MIB_REG, FORCE_SB_ECC_ERR, FORCE_RX_MEM1_SERR, reg_force_sb_ecc_err, force_sb_ecc_err->force_rx_mem1_serr);
    reg_force_sb_ecc_err = RU_FIELD_SET(0, XPORT_MIB_REG, FORCE_SB_ECC_ERR, FORCE_RX_MEM0_SERR, reg_force_sb_ecc_err, force_sb_ecc_err->force_rx_mem0_serr);

    RU_REG_WRITE(0, XPORT_MIB_REG, FORCE_SB_ECC_ERR, reg_force_sb_ecc_err);

    return 0;
}

int ag_drv_xport_mib_reg_force_sb_ecc_err_get(xport_mib_reg_force_sb_ecc_err *force_sb_ecc_err)
{
    uint32_t reg_force_sb_ecc_err=0;

#ifdef VALIDATE_PARMS
    if(!force_sb_ecc_err)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, FORCE_SB_ECC_ERR, reg_force_sb_ecc_err);

    force_sb_ecc_err->force_tx_mem3_serr = RU_FIELD_GET(0, XPORT_MIB_REG, FORCE_SB_ECC_ERR, FORCE_TX_MEM3_SERR, reg_force_sb_ecc_err);
    force_sb_ecc_err->force_tx_mem2_serr = RU_FIELD_GET(0, XPORT_MIB_REG, FORCE_SB_ECC_ERR, FORCE_TX_MEM2_SERR, reg_force_sb_ecc_err);
    force_sb_ecc_err->force_tx_mem1_serr = RU_FIELD_GET(0, XPORT_MIB_REG, FORCE_SB_ECC_ERR, FORCE_TX_MEM1_SERR, reg_force_sb_ecc_err);
    force_sb_ecc_err->force_tx_mem0_serr = RU_FIELD_GET(0, XPORT_MIB_REG, FORCE_SB_ECC_ERR, FORCE_TX_MEM0_SERR, reg_force_sb_ecc_err);
    force_sb_ecc_err->force_rx_mem4_serr = RU_FIELD_GET(0, XPORT_MIB_REG, FORCE_SB_ECC_ERR, FORCE_RX_MEM4_SERR, reg_force_sb_ecc_err);
    force_sb_ecc_err->force_rx_mem3_serr = RU_FIELD_GET(0, XPORT_MIB_REG, FORCE_SB_ECC_ERR, FORCE_RX_MEM3_SERR, reg_force_sb_ecc_err);
    force_sb_ecc_err->force_rx_mem2_serr = RU_FIELD_GET(0, XPORT_MIB_REG, FORCE_SB_ECC_ERR, FORCE_RX_MEM2_SERR, reg_force_sb_ecc_err);
    force_sb_ecc_err->force_rx_mem1_serr = RU_FIELD_GET(0, XPORT_MIB_REG, FORCE_SB_ECC_ERR, FORCE_RX_MEM1_SERR, reg_force_sb_ecc_err);
    force_sb_ecc_err->force_rx_mem0_serr = RU_FIELD_GET(0, XPORT_MIB_REG, FORCE_SB_ECC_ERR, FORCE_RX_MEM0_SERR, reg_force_sb_ecc_err);

    return 0;
}

int ag_drv_xport_mib_reg_force_db_ecc_err_set(const xport_mib_reg_force_db_ecc_err *force_db_ecc_err)
{
    uint32_t reg_force_db_ecc_err=0;

#ifdef VALIDATE_PARMS
    if(!force_db_ecc_err)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((force_db_ecc_err->force_tx_mem3_derr >= _1BITS_MAX_VAL_) ||
       (force_db_ecc_err->force_tx_mem2_derr >= _1BITS_MAX_VAL_) ||
       (force_db_ecc_err->force_tx_mem1_derr >= _1BITS_MAX_VAL_) ||
       (force_db_ecc_err->force_tx_mem0_derr >= _1BITS_MAX_VAL_) ||
       (force_db_ecc_err->force_rx_mem4_derr >= _1BITS_MAX_VAL_) ||
       (force_db_ecc_err->force_rx_mem3_derr >= _1BITS_MAX_VAL_) ||
       (force_db_ecc_err->force_rx_mem2_derr >= _1BITS_MAX_VAL_) ||
       (force_db_ecc_err->force_rx_mem1_derr >= _1BITS_MAX_VAL_) ||
       (force_db_ecc_err->force_rx_mem0_derr >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_force_db_ecc_err = RU_FIELD_SET(0, XPORT_MIB_REG, FORCE_DB_ECC_ERR, FORCE_TX_MEM3_DERR, reg_force_db_ecc_err, force_db_ecc_err->force_tx_mem3_derr);
    reg_force_db_ecc_err = RU_FIELD_SET(0, XPORT_MIB_REG, FORCE_DB_ECC_ERR, FORCE_TX_MEM2_DERR, reg_force_db_ecc_err, force_db_ecc_err->force_tx_mem2_derr);
    reg_force_db_ecc_err = RU_FIELD_SET(0, XPORT_MIB_REG, FORCE_DB_ECC_ERR, FORCE_TX_MEM1_DERR, reg_force_db_ecc_err, force_db_ecc_err->force_tx_mem1_derr);
    reg_force_db_ecc_err = RU_FIELD_SET(0, XPORT_MIB_REG, FORCE_DB_ECC_ERR, FORCE_TX_MEM0_DERR, reg_force_db_ecc_err, force_db_ecc_err->force_tx_mem0_derr);
    reg_force_db_ecc_err = RU_FIELD_SET(0, XPORT_MIB_REG, FORCE_DB_ECC_ERR, FORCE_RX_MEM4_DERR, reg_force_db_ecc_err, force_db_ecc_err->force_rx_mem4_derr);
    reg_force_db_ecc_err = RU_FIELD_SET(0, XPORT_MIB_REG, FORCE_DB_ECC_ERR, FORCE_RX_MEM3_DERR, reg_force_db_ecc_err, force_db_ecc_err->force_rx_mem3_derr);
    reg_force_db_ecc_err = RU_FIELD_SET(0, XPORT_MIB_REG, FORCE_DB_ECC_ERR, FORCE_RX_MEM2_DERR, reg_force_db_ecc_err, force_db_ecc_err->force_rx_mem2_derr);
    reg_force_db_ecc_err = RU_FIELD_SET(0, XPORT_MIB_REG, FORCE_DB_ECC_ERR, FORCE_RX_MEM1_DERR, reg_force_db_ecc_err, force_db_ecc_err->force_rx_mem1_derr);
    reg_force_db_ecc_err = RU_FIELD_SET(0, XPORT_MIB_REG, FORCE_DB_ECC_ERR, FORCE_RX_MEM0_DERR, reg_force_db_ecc_err, force_db_ecc_err->force_rx_mem0_derr);

    RU_REG_WRITE(0, XPORT_MIB_REG, FORCE_DB_ECC_ERR, reg_force_db_ecc_err);

    return 0;
}

int ag_drv_xport_mib_reg_force_db_ecc_err_get(xport_mib_reg_force_db_ecc_err *force_db_ecc_err)
{
    uint32_t reg_force_db_ecc_err=0;

#ifdef VALIDATE_PARMS
    if(!force_db_ecc_err)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, FORCE_DB_ECC_ERR, reg_force_db_ecc_err);

    force_db_ecc_err->force_tx_mem3_derr = RU_FIELD_GET(0, XPORT_MIB_REG, FORCE_DB_ECC_ERR, FORCE_TX_MEM3_DERR, reg_force_db_ecc_err);
    force_db_ecc_err->force_tx_mem2_derr = RU_FIELD_GET(0, XPORT_MIB_REG, FORCE_DB_ECC_ERR, FORCE_TX_MEM2_DERR, reg_force_db_ecc_err);
    force_db_ecc_err->force_tx_mem1_derr = RU_FIELD_GET(0, XPORT_MIB_REG, FORCE_DB_ECC_ERR, FORCE_TX_MEM1_DERR, reg_force_db_ecc_err);
    force_db_ecc_err->force_tx_mem0_derr = RU_FIELD_GET(0, XPORT_MIB_REG, FORCE_DB_ECC_ERR, FORCE_TX_MEM0_DERR, reg_force_db_ecc_err);
    force_db_ecc_err->force_rx_mem4_derr = RU_FIELD_GET(0, XPORT_MIB_REG, FORCE_DB_ECC_ERR, FORCE_RX_MEM4_DERR, reg_force_db_ecc_err);
    force_db_ecc_err->force_rx_mem3_derr = RU_FIELD_GET(0, XPORT_MIB_REG, FORCE_DB_ECC_ERR, FORCE_RX_MEM3_DERR, reg_force_db_ecc_err);
    force_db_ecc_err->force_rx_mem2_derr = RU_FIELD_GET(0, XPORT_MIB_REG, FORCE_DB_ECC_ERR, FORCE_RX_MEM2_DERR, reg_force_db_ecc_err);
    force_db_ecc_err->force_rx_mem1_derr = RU_FIELD_GET(0, XPORT_MIB_REG, FORCE_DB_ECC_ERR, FORCE_RX_MEM1_DERR, reg_force_db_ecc_err);
    force_db_ecc_err->force_rx_mem0_derr = RU_FIELD_GET(0, XPORT_MIB_REG, FORCE_DB_ECC_ERR, FORCE_RX_MEM0_DERR, reg_force_db_ecc_err);

    return 0;
}

int ag_drv_xport_mib_reg_rx_mem0_ecc_status_set(uint8_t mem_addr, uint8_t double_bit_ecc_err, uint8_t multi_ecc_err, uint8_t ecc_err)
{
    uint32_t reg_rx_mem0_ecc_status=0;

#ifdef VALIDATE_PARMS
    if((mem_addr >= _4BITS_MAX_VAL_) ||
       (double_bit_ecc_err >= _1BITS_MAX_VAL_) ||
       (multi_ecc_err >= _1BITS_MAX_VAL_) ||
       (ecc_err >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_rx_mem0_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, RX_MEM0_ECC_STATUS, MEM_ADDR, reg_rx_mem0_ecc_status, mem_addr);
    reg_rx_mem0_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, RX_MEM0_ECC_STATUS, DOUBLE_BIT_ECC_ERR, reg_rx_mem0_ecc_status, double_bit_ecc_err);
    reg_rx_mem0_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, RX_MEM0_ECC_STATUS, MULTI_ECC_ERR, reg_rx_mem0_ecc_status, multi_ecc_err);
    reg_rx_mem0_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, RX_MEM0_ECC_STATUS, ECC_ERR, reg_rx_mem0_ecc_status, ecc_err);

    RU_REG_WRITE(0, XPORT_MIB_REG, RX_MEM0_ECC_STATUS, reg_rx_mem0_ecc_status);

    return 0;
}

int ag_drv_xport_mib_reg_rx_mem0_ecc_status_get(uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err)
{
    uint32_t reg_rx_mem0_ecc_status=0;

#ifdef VALIDATE_PARMS
    if(!mem_addr || !double_bit_ecc_err || !multi_ecc_err || !ecc_err)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, RX_MEM0_ECC_STATUS, reg_rx_mem0_ecc_status);

    *mem_addr = RU_FIELD_GET(0, XPORT_MIB_REG, RX_MEM0_ECC_STATUS, MEM_ADDR, reg_rx_mem0_ecc_status);
    *double_bit_ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, RX_MEM0_ECC_STATUS, DOUBLE_BIT_ECC_ERR, reg_rx_mem0_ecc_status);
    *multi_ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, RX_MEM0_ECC_STATUS, MULTI_ECC_ERR, reg_rx_mem0_ecc_status);
    *ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, RX_MEM0_ECC_STATUS, ECC_ERR, reg_rx_mem0_ecc_status);

    return 0;
}

int ag_drv_xport_mib_reg_rx_mem1_ecc_status_set(uint8_t mem_addr, uint8_t double_bit_ecc_err, uint8_t multi_ecc_err, uint8_t ecc_err)
{
    uint32_t reg_rx_mem1_ecc_status=0;

#ifdef VALIDATE_PARMS
    if((mem_addr >= _4BITS_MAX_VAL_) ||
       (double_bit_ecc_err >= _1BITS_MAX_VAL_) ||
       (multi_ecc_err >= _1BITS_MAX_VAL_) ||
       (ecc_err >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_rx_mem1_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, RX_MEM1_ECC_STATUS, MEM_ADDR, reg_rx_mem1_ecc_status, mem_addr);
    reg_rx_mem1_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, RX_MEM1_ECC_STATUS, DOUBLE_BIT_ECC_ERR, reg_rx_mem1_ecc_status, double_bit_ecc_err);
    reg_rx_mem1_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, RX_MEM1_ECC_STATUS, MULTI_ECC_ERR, reg_rx_mem1_ecc_status, multi_ecc_err);
    reg_rx_mem1_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, RX_MEM1_ECC_STATUS, ECC_ERR, reg_rx_mem1_ecc_status, ecc_err);

    RU_REG_WRITE(0, XPORT_MIB_REG, RX_MEM1_ECC_STATUS, reg_rx_mem1_ecc_status);

    return 0;
}

int ag_drv_xport_mib_reg_rx_mem1_ecc_status_get(uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err)
{
    uint32_t reg_rx_mem1_ecc_status=0;

#ifdef VALIDATE_PARMS
    if(!mem_addr || !double_bit_ecc_err || !multi_ecc_err || !ecc_err)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, RX_MEM1_ECC_STATUS, reg_rx_mem1_ecc_status);

    *mem_addr = RU_FIELD_GET(0, XPORT_MIB_REG, RX_MEM1_ECC_STATUS, MEM_ADDR, reg_rx_mem1_ecc_status);
    *double_bit_ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, RX_MEM1_ECC_STATUS, DOUBLE_BIT_ECC_ERR, reg_rx_mem1_ecc_status);
    *multi_ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, RX_MEM1_ECC_STATUS, MULTI_ECC_ERR, reg_rx_mem1_ecc_status);
    *ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, RX_MEM1_ECC_STATUS, ECC_ERR, reg_rx_mem1_ecc_status);

    return 0;
}

int ag_drv_xport_mib_reg_rx_mem2_ecc_status_set(uint8_t mem_addr, uint8_t double_bit_ecc_err, uint8_t multi_ecc_err, uint8_t ecc_err)
{
    uint32_t reg_rx_mem2_ecc_status=0;

#ifdef VALIDATE_PARMS
    if((mem_addr >= _4BITS_MAX_VAL_) ||
       (double_bit_ecc_err >= _1BITS_MAX_VAL_) ||
       (multi_ecc_err >= _1BITS_MAX_VAL_) ||
       (ecc_err >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_rx_mem2_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, RX_MEM2_ECC_STATUS, MEM_ADDR, reg_rx_mem2_ecc_status, mem_addr);
    reg_rx_mem2_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, RX_MEM2_ECC_STATUS, DOUBLE_BIT_ECC_ERR, reg_rx_mem2_ecc_status, double_bit_ecc_err);
    reg_rx_mem2_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, RX_MEM2_ECC_STATUS, MULTI_ECC_ERR, reg_rx_mem2_ecc_status, multi_ecc_err);
    reg_rx_mem2_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, RX_MEM2_ECC_STATUS, ECC_ERR, reg_rx_mem2_ecc_status, ecc_err);

    RU_REG_WRITE(0, XPORT_MIB_REG, RX_MEM2_ECC_STATUS, reg_rx_mem2_ecc_status);

    return 0;
}

int ag_drv_xport_mib_reg_rx_mem2_ecc_status_get(uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err)
{
    uint32_t reg_rx_mem2_ecc_status=0;

#ifdef VALIDATE_PARMS
    if(!mem_addr || !double_bit_ecc_err || !multi_ecc_err || !ecc_err)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, RX_MEM2_ECC_STATUS, reg_rx_mem2_ecc_status);

    *mem_addr = RU_FIELD_GET(0, XPORT_MIB_REG, RX_MEM2_ECC_STATUS, MEM_ADDR, reg_rx_mem2_ecc_status);
    *double_bit_ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, RX_MEM2_ECC_STATUS, DOUBLE_BIT_ECC_ERR, reg_rx_mem2_ecc_status);
    *multi_ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, RX_MEM2_ECC_STATUS, MULTI_ECC_ERR, reg_rx_mem2_ecc_status);
    *ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, RX_MEM2_ECC_STATUS, ECC_ERR, reg_rx_mem2_ecc_status);

    return 0;
}

int ag_drv_xport_mib_reg_rx_mem3_ecc_status_set(uint8_t mem_addr, uint8_t double_bit_ecc_err, uint8_t multi_ecc_err, uint8_t ecc_err)
{
    uint32_t reg_rx_mem3_ecc_status=0;

#ifdef VALIDATE_PARMS
    if((mem_addr >= _4BITS_MAX_VAL_) ||
       (double_bit_ecc_err >= _1BITS_MAX_VAL_) ||
       (multi_ecc_err >= _1BITS_MAX_VAL_) ||
       (ecc_err >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_rx_mem3_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, RX_MEM3_ECC_STATUS, MEM_ADDR, reg_rx_mem3_ecc_status, mem_addr);
    reg_rx_mem3_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, RX_MEM3_ECC_STATUS, DOUBLE_BIT_ECC_ERR, reg_rx_mem3_ecc_status, double_bit_ecc_err);
    reg_rx_mem3_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, RX_MEM3_ECC_STATUS, MULTI_ECC_ERR, reg_rx_mem3_ecc_status, multi_ecc_err);
    reg_rx_mem3_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, RX_MEM3_ECC_STATUS, ECC_ERR, reg_rx_mem3_ecc_status, ecc_err);

    RU_REG_WRITE(0, XPORT_MIB_REG, RX_MEM3_ECC_STATUS, reg_rx_mem3_ecc_status);

    return 0;
}

int ag_drv_xport_mib_reg_rx_mem3_ecc_status_get(uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err)
{
    uint32_t reg_rx_mem3_ecc_status=0;

#ifdef VALIDATE_PARMS
    if(!mem_addr || !double_bit_ecc_err || !multi_ecc_err || !ecc_err)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, RX_MEM3_ECC_STATUS, reg_rx_mem3_ecc_status);

    *mem_addr = RU_FIELD_GET(0, XPORT_MIB_REG, RX_MEM3_ECC_STATUS, MEM_ADDR, reg_rx_mem3_ecc_status);
    *double_bit_ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, RX_MEM3_ECC_STATUS, DOUBLE_BIT_ECC_ERR, reg_rx_mem3_ecc_status);
    *multi_ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, RX_MEM3_ECC_STATUS, MULTI_ECC_ERR, reg_rx_mem3_ecc_status);
    *ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, RX_MEM3_ECC_STATUS, ECC_ERR, reg_rx_mem3_ecc_status);

    return 0;
}

int ag_drv_xport_mib_reg_rx_mem4_ecc_status_set(uint8_t mem_addr, uint8_t double_bit_ecc_err, uint8_t multi_ecc_err, uint8_t ecc_err)
{
    uint32_t reg_rx_mem4_ecc_status=0;

#ifdef VALIDATE_PARMS
    if((mem_addr >= _4BITS_MAX_VAL_) ||
       (double_bit_ecc_err >= _1BITS_MAX_VAL_) ||
       (multi_ecc_err >= _1BITS_MAX_VAL_) ||
       (ecc_err >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_rx_mem4_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, RX_MEM4_ECC_STATUS, MEM_ADDR, reg_rx_mem4_ecc_status, mem_addr);
    reg_rx_mem4_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, RX_MEM4_ECC_STATUS, DOUBLE_BIT_ECC_ERR, reg_rx_mem4_ecc_status, double_bit_ecc_err);
    reg_rx_mem4_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, RX_MEM4_ECC_STATUS, MULTI_ECC_ERR, reg_rx_mem4_ecc_status, multi_ecc_err);
    reg_rx_mem4_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, RX_MEM4_ECC_STATUS, ECC_ERR, reg_rx_mem4_ecc_status, ecc_err);

    RU_REG_WRITE(0, XPORT_MIB_REG, RX_MEM4_ECC_STATUS, reg_rx_mem4_ecc_status);

    return 0;
}

int ag_drv_xport_mib_reg_rx_mem4_ecc_status_get(uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err)
{
    uint32_t reg_rx_mem4_ecc_status=0;

#ifdef VALIDATE_PARMS
    if(!mem_addr || !double_bit_ecc_err || !multi_ecc_err || !ecc_err)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, RX_MEM4_ECC_STATUS, reg_rx_mem4_ecc_status);

    *mem_addr = RU_FIELD_GET(0, XPORT_MIB_REG, RX_MEM4_ECC_STATUS, MEM_ADDR, reg_rx_mem4_ecc_status);
    *double_bit_ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, RX_MEM4_ECC_STATUS, DOUBLE_BIT_ECC_ERR, reg_rx_mem4_ecc_status);
    *multi_ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, RX_MEM4_ECC_STATUS, MULTI_ECC_ERR, reg_rx_mem4_ecc_status);
    *ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, RX_MEM4_ECC_STATUS, ECC_ERR, reg_rx_mem4_ecc_status);

    return 0;
}

int ag_drv_xport_mib_reg_tx_mem0_ecc_status_set(uint8_t mem_addr, uint8_t double_bit_ecc_err, uint8_t multi_ecc_err, uint8_t ecc_err)
{
    uint32_t reg_tx_mem0_ecc_status=0;

#ifdef VALIDATE_PARMS
    if((mem_addr >= _4BITS_MAX_VAL_) ||
       (double_bit_ecc_err >= _1BITS_MAX_VAL_) ||
       (multi_ecc_err >= _1BITS_MAX_VAL_) ||
       (ecc_err >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_tx_mem0_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, TX_MEM0_ECC_STATUS, MEM_ADDR, reg_tx_mem0_ecc_status, mem_addr);
    reg_tx_mem0_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, TX_MEM0_ECC_STATUS, DOUBLE_BIT_ECC_ERR, reg_tx_mem0_ecc_status, double_bit_ecc_err);
    reg_tx_mem0_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, TX_MEM0_ECC_STATUS, MULTI_ECC_ERR, reg_tx_mem0_ecc_status, multi_ecc_err);
    reg_tx_mem0_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, TX_MEM0_ECC_STATUS, ECC_ERR, reg_tx_mem0_ecc_status, ecc_err);

    RU_REG_WRITE(0, XPORT_MIB_REG, TX_MEM0_ECC_STATUS, reg_tx_mem0_ecc_status);

    return 0;
}

int ag_drv_xport_mib_reg_tx_mem0_ecc_status_get(uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err)
{
    uint32_t reg_tx_mem0_ecc_status=0;

#ifdef VALIDATE_PARMS
    if(!mem_addr || !double_bit_ecc_err || !multi_ecc_err || !ecc_err)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, TX_MEM0_ECC_STATUS, reg_tx_mem0_ecc_status);

    *mem_addr = RU_FIELD_GET(0, XPORT_MIB_REG, TX_MEM0_ECC_STATUS, MEM_ADDR, reg_tx_mem0_ecc_status);
    *double_bit_ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, TX_MEM0_ECC_STATUS, DOUBLE_BIT_ECC_ERR, reg_tx_mem0_ecc_status);
    *multi_ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, TX_MEM0_ECC_STATUS, MULTI_ECC_ERR, reg_tx_mem0_ecc_status);
    *ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, TX_MEM0_ECC_STATUS, ECC_ERR, reg_tx_mem0_ecc_status);

    return 0;
}

int ag_drv_xport_mib_reg_tx_mem1_ecc_status_set(uint8_t mem_addr, uint8_t double_bit_ecc_err, uint8_t multi_ecc_err, uint8_t ecc_err)
{
    uint32_t reg_tx_mem1_ecc_status=0;

#ifdef VALIDATE_PARMS
    if((mem_addr >= _4BITS_MAX_VAL_) ||
       (double_bit_ecc_err >= _1BITS_MAX_VAL_) ||
       (multi_ecc_err >= _1BITS_MAX_VAL_) ||
       (ecc_err >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_tx_mem1_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, TX_MEM1_ECC_STATUS, MEM_ADDR, reg_tx_mem1_ecc_status, mem_addr);
    reg_tx_mem1_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, TX_MEM1_ECC_STATUS, DOUBLE_BIT_ECC_ERR, reg_tx_mem1_ecc_status, double_bit_ecc_err);
    reg_tx_mem1_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, TX_MEM1_ECC_STATUS, MULTI_ECC_ERR, reg_tx_mem1_ecc_status, multi_ecc_err);
    reg_tx_mem1_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, TX_MEM1_ECC_STATUS, ECC_ERR, reg_tx_mem1_ecc_status, ecc_err);

    RU_REG_WRITE(0, XPORT_MIB_REG, TX_MEM1_ECC_STATUS, reg_tx_mem1_ecc_status);

    return 0;
}

int ag_drv_xport_mib_reg_tx_mem1_ecc_status_get(uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err)
{
    uint32_t reg_tx_mem1_ecc_status=0;

#ifdef VALIDATE_PARMS
    if(!mem_addr || !double_bit_ecc_err || !multi_ecc_err || !ecc_err)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, TX_MEM1_ECC_STATUS, reg_tx_mem1_ecc_status);

    *mem_addr = RU_FIELD_GET(0, XPORT_MIB_REG, TX_MEM1_ECC_STATUS, MEM_ADDR, reg_tx_mem1_ecc_status);
    *double_bit_ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, TX_MEM1_ECC_STATUS, DOUBLE_BIT_ECC_ERR, reg_tx_mem1_ecc_status);
    *multi_ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, TX_MEM1_ECC_STATUS, MULTI_ECC_ERR, reg_tx_mem1_ecc_status);
    *ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, TX_MEM1_ECC_STATUS, ECC_ERR, reg_tx_mem1_ecc_status);

    return 0;
}

int ag_drv_xport_mib_reg_tx_mem2_ecc_status_set(uint8_t mem_addr, uint8_t double_bit_ecc_err, uint8_t multi_ecc_err, uint8_t ecc_err)
{
    uint32_t reg_tx_mem2_ecc_status=0;

#ifdef VALIDATE_PARMS
    if((mem_addr >= _4BITS_MAX_VAL_) ||
       (double_bit_ecc_err >= _1BITS_MAX_VAL_) ||
       (multi_ecc_err >= _1BITS_MAX_VAL_) ||
       (ecc_err >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_tx_mem2_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, TX_MEM2_ECC_STATUS, MEM_ADDR, reg_tx_mem2_ecc_status, mem_addr);
    reg_tx_mem2_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, TX_MEM2_ECC_STATUS, DOUBLE_BIT_ECC_ERR, reg_tx_mem2_ecc_status, double_bit_ecc_err);
    reg_tx_mem2_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, TX_MEM2_ECC_STATUS, MULTI_ECC_ERR, reg_tx_mem2_ecc_status, multi_ecc_err);
    reg_tx_mem2_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, TX_MEM2_ECC_STATUS, ECC_ERR, reg_tx_mem2_ecc_status, ecc_err);

    RU_REG_WRITE(0, XPORT_MIB_REG, TX_MEM2_ECC_STATUS, reg_tx_mem2_ecc_status);

    return 0;
}

int ag_drv_xport_mib_reg_tx_mem2_ecc_status_get(uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err)
{
    uint32_t reg_tx_mem2_ecc_status=0;

#ifdef VALIDATE_PARMS
    if(!mem_addr || !double_bit_ecc_err || !multi_ecc_err || !ecc_err)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, TX_MEM2_ECC_STATUS, reg_tx_mem2_ecc_status);

    *mem_addr = RU_FIELD_GET(0, XPORT_MIB_REG, TX_MEM2_ECC_STATUS, MEM_ADDR, reg_tx_mem2_ecc_status);
    *double_bit_ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, TX_MEM2_ECC_STATUS, DOUBLE_BIT_ECC_ERR, reg_tx_mem2_ecc_status);
    *multi_ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, TX_MEM2_ECC_STATUS, MULTI_ECC_ERR, reg_tx_mem2_ecc_status);
    *ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, TX_MEM2_ECC_STATUS, ECC_ERR, reg_tx_mem2_ecc_status);

    return 0;
}

int ag_drv_xport_mib_reg_tx_mem3_ecc_status_set(uint8_t mem_addr, uint8_t double_bit_ecc_err, uint8_t multi_ecc_err, uint8_t ecc_err)
{
    uint32_t reg_tx_mem3_ecc_status=0;

#ifdef VALIDATE_PARMS
    if((mem_addr >= _4BITS_MAX_VAL_) ||
       (double_bit_ecc_err >= _1BITS_MAX_VAL_) ||
       (multi_ecc_err >= _1BITS_MAX_VAL_) ||
       (ecc_err >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_tx_mem3_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, TX_MEM3_ECC_STATUS, MEM_ADDR, reg_tx_mem3_ecc_status, mem_addr);
    reg_tx_mem3_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, TX_MEM3_ECC_STATUS, DOUBLE_BIT_ECC_ERR, reg_tx_mem3_ecc_status, double_bit_ecc_err);
    reg_tx_mem3_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, TX_MEM3_ECC_STATUS, MULTI_ECC_ERR, reg_tx_mem3_ecc_status, multi_ecc_err);
    reg_tx_mem3_ecc_status = RU_FIELD_SET(0, XPORT_MIB_REG, TX_MEM3_ECC_STATUS, ECC_ERR, reg_tx_mem3_ecc_status, ecc_err);

    RU_REG_WRITE(0, XPORT_MIB_REG, TX_MEM3_ECC_STATUS, reg_tx_mem3_ecc_status);

    return 0;
}

int ag_drv_xport_mib_reg_tx_mem3_ecc_status_get(uint8_t *mem_addr, uint8_t *double_bit_ecc_err, uint8_t *multi_ecc_err, uint8_t *ecc_err)
{
    uint32_t reg_tx_mem3_ecc_status=0;

#ifdef VALIDATE_PARMS
    if(!mem_addr || !double_bit_ecc_err || !multi_ecc_err || !ecc_err)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, XPORT_MIB_REG, TX_MEM3_ECC_STATUS, reg_tx_mem3_ecc_status);

    *mem_addr = RU_FIELD_GET(0, XPORT_MIB_REG, TX_MEM3_ECC_STATUS, MEM_ADDR, reg_tx_mem3_ecc_status);
    *double_bit_ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, TX_MEM3_ECC_STATUS, DOUBLE_BIT_ECC_ERR, reg_tx_mem3_ecc_status);
    *multi_ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, TX_MEM3_ECC_STATUS, MULTI_ECC_ERR, reg_tx_mem3_ecc_status);
    *ecc_err = RU_FIELD_GET(0, XPORT_MIB_REG, TX_MEM3_ECC_STATUS, ECC_ERR, reg_tx_mem3_ecc_status);

    return 0;
}

