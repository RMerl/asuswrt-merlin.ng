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

#include "bcm6858_drivers_lport_ag.h"
#include "bcm6858_lport_srds_ag.h"
int ag_drv_lport_srds_merlin_rev_get(uint16_t *serdes_rev)
{
    uint32_t reg_dual_serdes_revision=0;

#ifdef VALIDATE_PARMS
    if(!serdes_rev)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_SRDS, DUAL_SERDES_REVISION, reg_dual_serdes_revision);

    *serdes_rev = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_REVISION, SERDES_REV, reg_dual_serdes_revision);

    return 0;
}

int ag_drv_lport_srds_serdes_0_indir_acc_cntrl_0_set(uint8_t err, uint8_t start_busy, uint8_t r_w, uint16_t reg_data)
{
    uint32_t reg_serdes_0_indir_acc_cntrl_0=0;

#ifdef VALIDATE_PARMS
    if((err >= _1BITS_MAX_VAL_) ||
       (start_busy >= _1BITS_MAX_VAL_) ||
       (r_w >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_serdes_0_indir_acc_cntrl_0 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_CNTRL_0, ERR, reg_serdes_0_indir_acc_cntrl_0, err);
    reg_serdes_0_indir_acc_cntrl_0 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_CNTRL_0, START_BUSY, reg_serdes_0_indir_acc_cntrl_0, start_busy);
    reg_serdes_0_indir_acc_cntrl_0 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_CNTRL_0, R_W, reg_serdes_0_indir_acc_cntrl_0, r_w);
    reg_serdes_0_indir_acc_cntrl_0 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_CNTRL_0, REG_DATA, reg_serdes_0_indir_acc_cntrl_0, reg_data);

    RU_REG_WRITE(0, LPORT_SRDS, SERDES_0_INDIR_ACC_CNTRL_0, reg_serdes_0_indir_acc_cntrl_0);

    return 0;
}

int ag_drv_lport_srds_serdes_0_indir_acc_cntrl_0_get(uint8_t *err, uint8_t *start_busy, uint8_t *r_w, uint16_t *reg_data)
{
    uint32_t reg_serdes_0_indir_acc_cntrl_0=0;

#ifdef VALIDATE_PARMS
    if(!err || !start_busy || !r_w || !reg_data)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_SRDS, SERDES_0_INDIR_ACC_CNTRL_0, reg_serdes_0_indir_acc_cntrl_0);

    *err = RU_FIELD_GET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_CNTRL_0, ERR, reg_serdes_0_indir_acc_cntrl_0);
    *start_busy = RU_FIELD_GET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_CNTRL_0, START_BUSY, reg_serdes_0_indir_acc_cntrl_0);
    *r_w = RU_FIELD_GET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_CNTRL_0, R_W, reg_serdes_0_indir_acc_cntrl_0);
    *reg_data = RU_FIELD_GET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_CNTRL_0, REG_DATA, reg_serdes_0_indir_acc_cntrl_0);

    return 0;
}

int ag_drv_lport_srds_serdes_0_indir_acc_addr_0_set(uint32_t reg_addr)
{
    uint32_t reg_serdes_0_indir_acc_addr_0=0;

#ifdef VALIDATE_PARMS
#endif

    reg_serdes_0_indir_acc_addr_0 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_ADDR_0, REG_ADDR, reg_serdes_0_indir_acc_addr_0, reg_addr);

    RU_REG_WRITE(0, LPORT_SRDS, SERDES_0_INDIR_ACC_ADDR_0, reg_serdes_0_indir_acc_addr_0);

    return 0;
}

int ag_drv_lport_srds_serdes_0_indir_acc_addr_0_get(uint32_t *reg_addr)
{
    uint32_t reg_serdes_0_indir_acc_addr_0=0;

#ifdef VALIDATE_PARMS
    if(!reg_addr)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_SRDS, SERDES_0_INDIR_ACC_ADDR_0, reg_serdes_0_indir_acc_addr_0);

    *reg_addr = RU_FIELD_GET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_ADDR_0, REG_ADDR, reg_serdes_0_indir_acc_addr_0);

    return 0;
}

int ag_drv_lport_srds_serdes_0_indir_acc_mask_0_set(uint16_t reg_mask)
{
    uint32_t reg_serdes_0_indir_acc_mask_0=0;

#ifdef VALIDATE_PARMS
#endif

    reg_serdes_0_indir_acc_mask_0 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_MASK_0, REG_MASK, reg_serdes_0_indir_acc_mask_0, reg_mask);

    RU_REG_WRITE(0, LPORT_SRDS, SERDES_0_INDIR_ACC_MASK_0, reg_serdes_0_indir_acc_mask_0);

    return 0;
}

int ag_drv_lport_srds_serdes_0_indir_acc_mask_0_get(uint16_t *reg_mask)
{
    uint32_t reg_serdes_0_indir_acc_mask_0=0;

#ifdef VALIDATE_PARMS
    if(!reg_mask)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_SRDS, SERDES_0_INDIR_ACC_MASK_0, reg_serdes_0_indir_acc_mask_0);

    *reg_mask = RU_FIELD_GET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_MASK_0, REG_MASK, reg_serdes_0_indir_acc_mask_0);

    return 0;
}

int ag_drv_lport_srds_serdes_0_indir_acc_cntrl_1_set(uint8_t err, uint8_t start_busy, uint8_t r_w, uint16_t reg_data)
{
    uint32_t reg_serdes_0_indir_acc_cntrl_1=0;

#ifdef VALIDATE_PARMS
    if((err >= _1BITS_MAX_VAL_) ||
       (start_busy >= _1BITS_MAX_VAL_) ||
       (r_w >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_serdes_0_indir_acc_cntrl_1 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_CNTRL_1, ERR, reg_serdes_0_indir_acc_cntrl_1, err);
    reg_serdes_0_indir_acc_cntrl_1 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_CNTRL_1, START_BUSY, reg_serdes_0_indir_acc_cntrl_1, start_busy);
    reg_serdes_0_indir_acc_cntrl_1 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_CNTRL_1, R_W, reg_serdes_0_indir_acc_cntrl_1, r_w);
    reg_serdes_0_indir_acc_cntrl_1 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_CNTRL_1, REG_DATA, reg_serdes_0_indir_acc_cntrl_1, reg_data);

    RU_REG_WRITE(0, LPORT_SRDS, SERDES_0_INDIR_ACC_CNTRL_1, reg_serdes_0_indir_acc_cntrl_1);

    return 0;
}

int ag_drv_lport_srds_serdes_0_indir_acc_cntrl_1_get(uint8_t *err, uint8_t *start_busy, uint8_t *r_w, uint16_t *reg_data)
{
    uint32_t reg_serdes_0_indir_acc_cntrl_1=0;

#ifdef VALIDATE_PARMS
    if(!err || !start_busy || !r_w || !reg_data)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_SRDS, SERDES_0_INDIR_ACC_CNTRL_1, reg_serdes_0_indir_acc_cntrl_1);

    *err = RU_FIELD_GET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_CNTRL_1, ERR, reg_serdes_0_indir_acc_cntrl_1);
    *start_busy = RU_FIELD_GET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_CNTRL_1, START_BUSY, reg_serdes_0_indir_acc_cntrl_1);
    *r_w = RU_FIELD_GET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_CNTRL_1, R_W, reg_serdes_0_indir_acc_cntrl_1);
    *reg_data = RU_FIELD_GET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_CNTRL_1, REG_DATA, reg_serdes_0_indir_acc_cntrl_1);

    return 0;
}

int ag_drv_lport_srds_serdes_0_indir_acc_addr_1_set(uint32_t reg_addr)
{
    uint32_t reg_serdes_0_indir_acc_addr_1=0;

#ifdef VALIDATE_PARMS
#endif

    reg_serdes_0_indir_acc_addr_1 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_ADDR_1, REG_ADDR, reg_serdes_0_indir_acc_addr_1, reg_addr);

    RU_REG_WRITE(0, LPORT_SRDS, SERDES_0_INDIR_ACC_ADDR_1, reg_serdes_0_indir_acc_addr_1);

    return 0;
}

int ag_drv_lport_srds_serdes_0_indir_acc_addr_1_get(uint32_t *reg_addr)
{
    uint32_t reg_serdes_0_indir_acc_addr_1=0;

#ifdef VALIDATE_PARMS
    if(!reg_addr)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_SRDS, SERDES_0_INDIR_ACC_ADDR_1, reg_serdes_0_indir_acc_addr_1);

    *reg_addr = RU_FIELD_GET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_ADDR_1, REG_ADDR, reg_serdes_0_indir_acc_addr_1);

    return 0;
}

int ag_drv_lport_srds_serdes_0_indir_acc_mask_1_set(uint16_t reg_mask)
{
    uint32_t reg_serdes_0_indir_acc_mask_1=0;

#ifdef VALIDATE_PARMS
#endif

    reg_serdes_0_indir_acc_mask_1 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_MASK_1, REG_MASK, reg_serdes_0_indir_acc_mask_1, reg_mask);

    RU_REG_WRITE(0, LPORT_SRDS, SERDES_0_INDIR_ACC_MASK_1, reg_serdes_0_indir_acc_mask_1);

    return 0;
}

int ag_drv_lport_srds_serdes_0_indir_acc_mask_1_get(uint16_t *reg_mask)
{
    uint32_t reg_serdes_0_indir_acc_mask_1=0;

#ifdef VALIDATE_PARMS
    if(!reg_mask)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_SRDS, SERDES_0_INDIR_ACC_MASK_1, reg_serdes_0_indir_acc_mask_1);

    *reg_mask = RU_FIELD_GET(0, LPORT_SRDS, SERDES_0_INDIR_ACC_MASK_1, REG_MASK, reg_serdes_0_indir_acc_mask_1);

    return 0;
}

int ag_drv_lport_srds_serdes_1_indir_acc_cntrl_0_set(uint8_t err, uint8_t start_busy, uint8_t r_w, uint16_t reg_data)
{
    uint32_t reg_serdes_1_indir_acc_cntrl_0=0;

#ifdef VALIDATE_PARMS
    if((err >= _1BITS_MAX_VAL_) ||
       (start_busy >= _1BITS_MAX_VAL_) ||
       (r_w >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_serdes_1_indir_acc_cntrl_0 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_CNTRL_0, ERR, reg_serdes_1_indir_acc_cntrl_0, err);
    reg_serdes_1_indir_acc_cntrl_0 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_CNTRL_0, START_BUSY, reg_serdes_1_indir_acc_cntrl_0, start_busy);
    reg_serdes_1_indir_acc_cntrl_0 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_CNTRL_0, R_W, reg_serdes_1_indir_acc_cntrl_0, r_w);
    reg_serdes_1_indir_acc_cntrl_0 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_CNTRL_0, REG_DATA, reg_serdes_1_indir_acc_cntrl_0, reg_data);

    RU_REG_WRITE(0, LPORT_SRDS, SERDES_1_INDIR_ACC_CNTRL_0, reg_serdes_1_indir_acc_cntrl_0);

    return 0;
}

int ag_drv_lport_srds_serdes_1_indir_acc_cntrl_0_get(uint8_t *err, uint8_t *start_busy, uint8_t *r_w, uint16_t *reg_data)
{
    uint32_t reg_serdes_1_indir_acc_cntrl_0=0;

#ifdef VALIDATE_PARMS
    if(!err || !start_busy || !r_w || !reg_data)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_SRDS, SERDES_1_INDIR_ACC_CNTRL_0, reg_serdes_1_indir_acc_cntrl_0);

    *err = RU_FIELD_GET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_CNTRL_0, ERR, reg_serdes_1_indir_acc_cntrl_0);
    *start_busy = RU_FIELD_GET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_CNTRL_0, START_BUSY, reg_serdes_1_indir_acc_cntrl_0);
    *r_w = RU_FIELD_GET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_CNTRL_0, R_W, reg_serdes_1_indir_acc_cntrl_0);
    *reg_data = RU_FIELD_GET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_CNTRL_0, REG_DATA, reg_serdes_1_indir_acc_cntrl_0);

    return 0;
}

int ag_drv_lport_srds_serdes_1_indir_acc_addr_0_set(uint32_t reg_addr)
{
    uint32_t reg_serdes_1_indir_acc_addr_0=0;

#ifdef VALIDATE_PARMS
#endif

    reg_serdes_1_indir_acc_addr_0 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_ADDR_0, REG_ADDR, reg_serdes_1_indir_acc_addr_0, reg_addr);

    RU_REG_WRITE(0, LPORT_SRDS, SERDES_1_INDIR_ACC_ADDR_0, reg_serdes_1_indir_acc_addr_0);

    return 0;
}

int ag_drv_lport_srds_serdes_1_indir_acc_addr_0_get(uint32_t *reg_addr)
{
    uint32_t reg_serdes_1_indir_acc_addr_0=0;

#ifdef VALIDATE_PARMS
    if(!reg_addr)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_SRDS, SERDES_1_INDIR_ACC_ADDR_0, reg_serdes_1_indir_acc_addr_0);

    *reg_addr = RU_FIELD_GET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_ADDR_0, REG_ADDR, reg_serdes_1_indir_acc_addr_0);

    return 0;
}

int ag_drv_lport_srds_serdes_1_indir_acc_mask_0_set(uint16_t reg_mask)
{
    uint32_t reg_serdes_1_indir_acc_mask_0=0;

#ifdef VALIDATE_PARMS
#endif

    reg_serdes_1_indir_acc_mask_0 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_MASK_0, REG_MASK, reg_serdes_1_indir_acc_mask_0, reg_mask);

    RU_REG_WRITE(0, LPORT_SRDS, SERDES_1_INDIR_ACC_MASK_0, reg_serdes_1_indir_acc_mask_0);

    return 0;
}

int ag_drv_lport_srds_serdes_1_indir_acc_mask_0_get(uint16_t *reg_mask)
{
    uint32_t reg_serdes_1_indir_acc_mask_0=0;

#ifdef VALIDATE_PARMS
    if(!reg_mask)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_SRDS, SERDES_1_INDIR_ACC_MASK_0, reg_serdes_1_indir_acc_mask_0);

    *reg_mask = RU_FIELD_GET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_MASK_0, REG_MASK, reg_serdes_1_indir_acc_mask_0);

    return 0;
}

int ag_drv_lport_srds_serdes_1_indir_acc_cntrl_1_set(uint8_t err, uint8_t start_busy, uint8_t r_w, uint16_t reg_data)
{
    uint32_t reg_serdes_1_indir_acc_cntrl_1=0;

#ifdef VALIDATE_PARMS
    if((err >= _1BITS_MAX_VAL_) ||
       (start_busy >= _1BITS_MAX_VAL_) ||
       (r_w >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_serdes_1_indir_acc_cntrl_1 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_CNTRL_1, ERR, reg_serdes_1_indir_acc_cntrl_1, err);
    reg_serdes_1_indir_acc_cntrl_1 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_CNTRL_1, START_BUSY, reg_serdes_1_indir_acc_cntrl_1, start_busy);
    reg_serdes_1_indir_acc_cntrl_1 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_CNTRL_1, R_W, reg_serdes_1_indir_acc_cntrl_1, r_w);
    reg_serdes_1_indir_acc_cntrl_1 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_CNTRL_1, REG_DATA, reg_serdes_1_indir_acc_cntrl_1, reg_data);

    RU_REG_WRITE(0, LPORT_SRDS, SERDES_1_INDIR_ACC_CNTRL_1, reg_serdes_1_indir_acc_cntrl_1);

    return 0;
}

int ag_drv_lport_srds_serdes_1_indir_acc_cntrl_1_get(uint8_t *err, uint8_t *start_busy, uint8_t *r_w, uint16_t *reg_data)
{
    uint32_t reg_serdes_1_indir_acc_cntrl_1=0;

#ifdef VALIDATE_PARMS
    if(!err || !start_busy || !r_w || !reg_data)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_SRDS, SERDES_1_INDIR_ACC_CNTRL_1, reg_serdes_1_indir_acc_cntrl_1);

    *err = RU_FIELD_GET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_CNTRL_1, ERR, reg_serdes_1_indir_acc_cntrl_1);
    *start_busy = RU_FIELD_GET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_CNTRL_1, START_BUSY, reg_serdes_1_indir_acc_cntrl_1);
    *r_w = RU_FIELD_GET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_CNTRL_1, R_W, reg_serdes_1_indir_acc_cntrl_1);
    *reg_data = RU_FIELD_GET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_CNTRL_1, REG_DATA, reg_serdes_1_indir_acc_cntrl_1);

    return 0;
}

int ag_drv_lport_srds_serdes_1_indir_acc_addr_1_set(uint32_t reg_addr)
{
    uint32_t reg_serdes_1_indir_acc_addr_1=0;

#ifdef VALIDATE_PARMS
#endif

    reg_serdes_1_indir_acc_addr_1 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_ADDR_1, REG_ADDR, reg_serdes_1_indir_acc_addr_1, reg_addr);

    RU_REG_WRITE(0, LPORT_SRDS, SERDES_1_INDIR_ACC_ADDR_1, reg_serdes_1_indir_acc_addr_1);

    return 0;
}

int ag_drv_lport_srds_serdes_1_indir_acc_addr_1_get(uint32_t *reg_addr)
{
    uint32_t reg_serdes_1_indir_acc_addr_1=0;

#ifdef VALIDATE_PARMS
    if(!reg_addr)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_SRDS, SERDES_1_INDIR_ACC_ADDR_1, reg_serdes_1_indir_acc_addr_1);

    *reg_addr = RU_FIELD_GET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_ADDR_1, REG_ADDR, reg_serdes_1_indir_acc_addr_1);

    return 0;
}

int ag_drv_lport_srds_serdes_1_indir_acc_mask_1_set(uint16_t reg_mask)
{
    uint32_t reg_serdes_1_indir_acc_mask_1=0;

#ifdef VALIDATE_PARMS
#endif

    reg_serdes_1_indir_acc_mask_1 = RU_FIELD_SET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_MASK_1, REG_MASK, reg_serdes_1_indir_acc_mask_1, reg_mask);

    RU_REG_WRITE(0, LPORT_SRDS, SERDES_1_INDIR_ACC_MASK_1, reg_serdes_1_indir_acc_mask_1);

    return 0;
}

int ag_drv_lport_srds_serdes_1_indir_acc_mask_1_get(uint16_t *reg_mask)
{
    uint32_t reg_serdes_1_indir_acc_mask_1=0;

#ifdef VALIDATE_PARMS
    if(!reg_mask)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_SRDS, SERDES_1_INDIR_ACC_MASK_1, reg_serdes_1_indir_acc_mask_1);

    *reg_mask = RU_FIELD_GET(0, LPORT_SRDS, SERDES_1_INDIR_ACC_MASK_1, REG_MASK, reg_serdes_1_indir_acc_mask_1);

    return 0;
}

int ag_drv_lport_srds_dual_serdes_0_cntrl_set(const lport_srds_dual_serdes_0_cntrl *dual_serdes_0_cntrl)
{
    uint32_t reg_dual_serdes_0_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!dual_serdes_0_cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((dual_serdes_0_cntrl->serdes_test_en >= _1BITS_MAX_VAL_) ||
       (dual_serdes_0_cntrl->serdes_ln_offset >= _5BITS_MAX_VAL_) ||
       (dual_serdes_0_cntrl->serdes_prtad >= _5BITS_MAX_VAL_) ||
       (dual_serdes_0_cntrl->serdes_reset >= _1BITS_MAX_VAL_) ||
       (dual_serdes_0_cntrl->refclk_reset >= _1BITS_MAX_VAL_) ||
       (dual_serdes_0_cntrl->iddq >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_dual_serdes_0_cntrl = RU_FIELD_SET(0, LPORT_SRDS, DUAL_SERDES_0_CNTRL, SERDES_TEST_EN, reg_dual_serdes_0_cntrl, dual_serdes_0_cntrl->serdes_test_en);
    reg_dual_serdes_0_cntrl = RU_FIELD_SET(0, LPORT_SRDS, DUAL_SERDES_0_CNTRL, SERDES_LN_OFFSET, reg_dual_serdes_0_cntrl, dual_serdes_0_cntrl->serdes_ln_offset);
    reg_dual_serdes_0_cntrl = RU_FIELD_SET(0, LPORT_SRDS, DUAL_SERDES_0_CNTRL, SERDES_PRTAD, reg_dual_serdes_0_cntrl, dual_serdes_0_cntrl->serdes_prtad);
    reg_dual_serdes_0_cntrl = RU_FIELD_SET(0, LPORT_SRDS, DUAL_SERDES_0_CNTRL, SERDES_RESET, reg_dual_serdes_0_cntrl, dual_serdes_0_cntrl->serdes_reset);
    reg_dual_serdes_0_cntrl = RU_FIELD_SET(0, LPORT_SRDS, DUAL_SERDES_0_CNTRL, REFCLK_RESET, reg_dual_serdes_0_cntrl, dual_serdes_0_cntrl->refclk_reset);
    reg_dual_serdes_0_cntrl = RU_FIELD_SET(0, LPORT_SRDS, DUAL_SERDES_0_CNTRL, IDDQ, reg_dual_serdes_0_cntrl, dual_serdes_0_cntrl->iddq);

    RU_REG_WRITE(0, LPORT_SRDS, DUAL_SERDES_0_CNTRL, reg_dual_serdes_0_cntrl);

    return 0;
}

int ag_drv_lport_srds_dual_serdes_0_cntrl_get(lport_srds_dual_serdes_0_cntrl *dual_serdes_0_cntrl)
{
    uint32_t reg_dual_serdes_0_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!dual_serdes_0_cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_SRDS, DUAL_SERDES_0_CNTRL, reg_dual_serdes_0_cntrl);

    dual_serdes_0_cntrl->serdes_test_en = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_0_CNTRL, SERDES_TEST_EN, reg_dual_serdes_0_cntrl);
    dual_serdes_0_cntrl->serdes_ln_offset = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_0_CNTRL, SERDES_LN_OFFSET, reg_dual_serdes_0_cntrl);
    dual_serdes_0_cntrl->serdes_prtad = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_0_CNTRL, SERDES_PRTAD, reg_dual_serdes_0_cntrl);
    dual_serdes_0_cntrl->serdes_reset = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_0_CNTRL, SERDES_RESET, reg_dual_serdes_0_cntrl);
    dual_serdes_0_cntrl->refclk_reset = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_0_CNTRL, REFCLK_RESET, reg_dual_serdes_0_cntrl);
    dual_serdes_0_cntrl->iddq = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_0_CNTRL, IDDQ, reg_dual_serdes_0_cntrl);

    return 0;
}

int ag_drv_lport_srds_dual_serdes_0_status_get(lport_srds_dual_serdes_0_status *dual_serdes_0_status)
{
    uint32_t reg_dual_serdes_0_status=0;

#ifdef VALIDATE_PARMS
    if(!dual_serdes_0_status)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_SRDS, DUAL_SERDES_0_STATUS, reg_dual_serdes_0_status);

    dual_serdes_0_status->mod_def0 = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_0_STATUS, MOD_DEF0, reg_dual_serdes_0_status);
    dual_serdes_0_status->ext_sig_det = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_0_STATUS, EXT_SIG_DET, reg_dual_serdes_0_status);
    dual_serdes_0_status->pll_lock = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_0_STATUS, PLL_LOCK, reg_dual_serdes_0_status);
    dual_serdes_0_status->link_status = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_0_STATUS, LINK_STATUS, reg_dual_serdes_0_status);
    dual_serdes_0_status->cdr_lock = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_0_STATUS, CDR_LOCK, reg_dual_serdes_0_status);
    dual_serdes_0_status->rx_sigdet = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_0_STATUS, RX_SIGDET, reg_dual_serdes_0_status);

    return 0;
}

int ag_drv_lport_srds_dual_serdes_1_cntrl_set(const lport_srds_dual_serdes_1_cntrl *dual_serdes_1_cntrl)
{
    uint32_t reg_dual_serdes_1_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!dual_serdes_1_cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((dual_serdes_1_cntrl->serdes_test_en >= _1BITS_MAX_VAL_) ||
       (dual_serdes_1_cntrl->serdes_ln_offset >= _5BITS_MAX_VAL_) ||
       (dual_serdes_1_cntrl->serdes_prtad >= _5BITS_MAX_VAL_) ||
       (dual_serdes_1_cntrl->serdes_reset >= _1BITS_MAX_VAL_) ||
       (dual_serdes_1_cntrl->refclk_reset >= _1BITS_MAX_VAL_) ||
       (dual_serdes_1_cntrl->iddq >= _1BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_dual_serdes_1_cntrl = RU_FIELD_SET(0, LPORT_SRDS, DUAL_SERDES_1_CNTRL, SERDES_TEST_EN, reg_dual_serdes_1_cntrl, dual_serdes_1_cntrl->serdes_test_en);
    reg_dual_serdes_1_cntrl = RU_FIELD_SET(0, LPORT_SRDS, DUAL_SERDES_1_CNTRL, SERDES_LN_OFFSET, reg_dual_serdes_1_cntrl, dual_serdes_1_cntrl->serdes_ln_offset);
    reg_dual_serdes_1_cntrl = RU_FIELD_SET(0, LPORT_SRDS, DUAL_SERDES_1_CNTRL, SERDES_PRTAD, reg_dual_serdes_1_cntrl, dual_serdes_1_cntrl->serdes_prtad);
    reg_dual_serdes_1_cntrl = RU_FIELD_SET(0, LPORT_SRDS, DUAL_SERDES_1_CNTRL, SERDES_RESET, reg_dual_serdes_1_cntrl, dual_serdes_1_cntrl->serdes_reset);
    reg_dual_serdes_1_cntrl = RU_FIELD_SET(0, LPORT_SRDS, DUAL_SERDES_1_CNTRL, REFCLK_RESET, reg_dual_serdes_1_cntrl, dual_serdes_1_cntrl->refclk_reset);
    reg_dual_serdes_1_cntrl = RU_FIELD_SET(0, LPORT_SRDS, DUAL_SERDES_1_CNTRL, IDDQ, reg_dual_serdes_1_cntrl, dual_serdes_1_cntrl->iddq);

    RU_REG_WRITE(0, LPORT_SRDS, DUAL_SERDES_1_CNTRL, reg_dual_serdes_1_cntrl);

    return 0;
}

int ag_drv_lport_srds_dual_serdes_1_cntrl_get(lport_srds_dual_serdes_1_cntrl *dual_serdes_1_cntrl)
{
    uint32_t reg_dual_serdes_1_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!dual_serdes_1_cntrl)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_SRDS, DUAL_SERDES_1_CNTRL, reg_dual_serdes_1_cntrl);

    dual_serdes_1_cntrl->serdes_test_en = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_1_CNTRL, SERDES_TEST_EN, reg_dual_serdes_1_cntrl);
    dual_serdes_1_cntrl->serdes_ln_offset = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_1_CNTRL, SERDES_LN_OFFSET, reg_dual_serdes_1_cntrl);
    dual_serdes_1_cntrl->serdes_prtad = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_1_CNTRL, SERDES_PRTAD, reg_dual_serdes_1_cntrl);
    dual_serdes_1_cntrl->serdes_reset = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_1_CNTRL, SERDES_RESET, reg_dual_serdes_1_cntrl);
    dual_serdes_1_cntrl->refclk_reset = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_1_CNTRL, REFCLK_RESET, reg_dual_serdes_1_cntrl);
    dual_serdes_1_cntrl->iddq = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_1_CNTRL, IDDQ, reg_dual_serdes_1_cntrl);

    return 0;
}

int ag_drv_lport_srds_dual_serdes_1_status_get(lport_srds_dual_serdes_1_status *dual_serdes_1_status)
{
    uint32_t reg_dual_serdes_1_status=0;

#ifdef VALIDATE_PARMS
    if(!dual_serdes_1_status)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, LPORT_SRDS, DUAL_SERDES_1_STATUS, reg_dual_serdes_1_status);

    dual_serdes_1_status->mod_def0 = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_1_STATUS, MOD_DEF0, reg_dual_serdes_1_status);
    dual_serdes_1_status->ext_sig_det = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_1_STATUS, EXT_SIG_DET, reg_dual_serdes_1_status);
    dual_serdes_1_status->pll_lock = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_1_STATUS, PLL_LOCK, reg_dual_serdes_1_status);
    dual_serdes_1_status->link_status = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_1_STATUS, LINK_STATUS, reg_dual_serdes_1_status);
    dual_serdes_1_status->cdr_lock = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_1_STATUS, CDR_LOCK, reg_dual_serdes_1_status);
    dual_serdes_1_status->rx_sigdet = RU_FIELD_GET(0, LPORT_SRDS, DUAL_SERDES_1_STATUS, RX_SIGDET, reg_dual_serdes_1_status);

    return 0;
}

