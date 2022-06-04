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
 * xport_intr.c
 *
 * XPORT interrupt layer
 *
 */

#include <linux/bitops.h>
#include "bcm_intr.h"
#include "ru.h"
#include "bcm63158_xport_intr_ag.h"
#include "xport_defs.h"
#include "xport_intr.h"

#define ENTITY_ID_COUNT 7

typedef struct _intr_handler_s intr_handler_s;

typedef struct _intr_handler_s
{
    xport_intr_info_s info;
    uint32_t register_num;
    uint32_t mask;
    int (*status_get_cb)(intr_handler_s *, uint32_t);
    xport_intr_cb_t isr_cb;
    void *isr_priv;
} intr_handler_s;

static int port_link_status_get(intr_handler_s *handler, uint32_t reg_val);

intr_handler_s
    mdio_done_h = { .info = {.intr_id = XPORT_MDIO_DONE}, .mask = XPORT_INTR_0_CPU_STATUS_MDIO_DONE_INTR_FIELD_MASK},
    mdio_error_h = { .info = {.intr_id = XPORT_MDIO_READ_ERROR}, .mask = XPORT_INTR_0_CPU_STATUS_MDIO_ERR_INTR_FIELD_MASK};

intr_handler_s
    port0_link_h = { .info = {.intr_id = XPORT_PORT_LINK, .entity_id = 0}, .register_num = 1,
        .mask = (1U << XPORT_INTR_1_CPU_STATUS_LINK_DOWN_INTR_FIELD_SHIFT) | (1U << XPORT_INTR_1_CPU_STATUS_LINK_UP_INTR_FIELD_SHIFT), 
        .status_get_cb = port_link_status_get},
    port1_link_h = { .info = {.intr_id = XPORT_PORT_LINK, .entity_id = 1}, .register_num = 1,
        .mask = (1U << (XPORT_INTR_1_CPU_STATUS_LINK_DOWN_INTR_FIELD_SHIFT + 1)) | (1U << (XPORT_INTR_1_CPU_STATUS_LINK_UP_INTR_FIELD_SHIFT + 1)), 
        .status_get_cb = port_link_status_get},
    port2_link_h = { .info = {.intr_id = XPORT_PORT_LINK, .entity_id = 2}, .register_num = 1, 
        .mask = (1U << (XPORT_INTR_1_CPU_STATUS_LINK_DOWN_INTR_FIELD_SHIFT + 2)) | (1U << (XPORT_INTR_1_CPU_STATUS_LINK_UP_INTR_FIELD_SHIFT + 2)), 
        .status_get_cb = port_link_status_get},
    port3_link_h = { .info = {.intr_id = XPORT_PORT_LINK, .entity_id = 3}, .register_num = 1, 
        .mask = (1U << (XPORT_INTR_1_CPU_STATUS_LINK_DOWN_INTR_FIELD_SHIFT + 3)) | (1U << (XPORT_INTR_1_CPU_STATUS_LINK_UP_INTR_FIELD_SHIFT + 3)), 
        .status_get_cb = port_link_status_get},
    port4_link_h = { .info = {.intr_id = XPORT_PORT_LINK, .entity_id = 4}, .register_num = 1, 
        .mask = (1U << (XPORT_INTR_1_CPU_STATUS_LINK_DOWN_INTR_FIELD_SHIFT + 4)) | (1U << (XPORT_INTR_1_CPU_STATUS_LINK_UP_INTR_FIELD_SHIFT + 4)), 
        .status_get_cb = port_link_status_get},
    port5_link_h = { .info = {.intr_id = XPORT_PORT_LINK, .entity_id = 5}, .register_num = 1,
        .mask = (1U << (XPORT_INTR_1_CPU_STATUS_LINK_DOWN_INTR_FIELD_SHIFT + 5)) | (1U << (XPORT_INTR_1_CPU_STATUS_LINK_UP_INTR_FIELD_SHIFT + 5)), 
        .status_get_cb = port_link_status_get},
    port6_link_h = { .info = {.intr_id = XPORT_PORT_LINK, .entity_id = 6}, .register_num = 1, 
        .mask = (1U << (XPORT_INTR_1_CPU_STATUS_LINK_DOWN_INTR_FIELD_SHIFT + 6)) | (1U << (XPORT_INTR_1_CPU_STATUS_LINK_UP_INTR_FIELD_SHIFT + 6)), 
        .status_get_cb = port_link_status_get},
    port0_remote_fault_h = { .info = {.intr_id = XPORT_REMOTE_FAULT, .entity_id = 0}, .register_num = 1, 
        .mask = (1U << XPORT_INTR_1_CPU_STATUS_RX_REMOTE_FAULT_INTR_FIELD_SHIFT)},
    port4_remote_fault_h = { .info = {.intr_id = XPORT_REMOTE_FAULT, .entity_id = 1}, .register_num = 1, 
        .mask = (1U << (XPORT_INTR_1_CPU_STATUS_RX_REMOTE_FAULT_INTR_FIELD_SHIFT + 1))};

intr_handler_s
    tx_timesync_fifo_h = { .info = {.intr_id = XPORT_TIMESYNC_FIFO}, .mask = XPORT_INTR_0_CPU_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_MASK};

/* Interrupt info registry - 2 registers, 32 bits each */
static intr_handler_s *bit_to_handler[2][32] = 
{
[0] = 
   {
       [XPORT_INTR_0_CPU_STATUS_MDIO_DONE_INTR_FIELD_SHIFT] = &mdio_done_h,
       [XPORT_INTR_0_CPU_STATUS_MDIO_ERR_INTR_FIELD_SHIFT] = &mdio_error_h,
       [XPORT_INTR_0_CPU_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT] = &tx_timesync_fifo_h,
       [XPORT_INTR_0_CPU_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT+1] = &tx_timesync_fifo_h,
       [XPORT_INTR_0_CPU_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT+2] = &tx_timesync_fifo_h,
       [XPORT_INTR_0_CPU_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT+3] = &tx_timesync_fifo_h,
       [XPORT_INTR_0_CPU_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT+4] = &tx_timesync_fifo_h,
       [XPORT_INTR_0_CPU_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT+5] = &tx_timesync_fifo_h,
       [XPORT_INTR_0_CPU_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT+6] = &tx_timesync_fifo_h,
       [XPORT_INTR_0_CPU_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT+7] = &tx_timesync_fifo_h,

   },
[1] = 
   {
       [XPORT_INTR_1_CPU_STATUS_LINK_UP_INTR_FIELD_SHIFT] = &port0_link_h,
       [XPORT_INTR_1_CPU_STATUS_LINK_UP_INTR_FIELD_SHIFT + 1] = &port1_link_h,
       [XPORT_INTR_1_CPU_STATUS_LINK_UP_INTR_FIELD_SHIFT + 2] = &port2_link_h,
       [XPORT_INTR_1_CPU_STATUS_LINK_UP_INTR_FIELD_SHIFT + 3] = &port3_link_h,
       [XPORT_INTR_1_CPU_STATUS_LINK_UP_INTR_FIELD_SHIFT + 4] = &port4_link_h,
       [XPORT_INTR_1_CPU_STATUS_LINK_UP_INTR_FIELD_SHIFT + 5] = &port5_link_h,
       [XPORT_INTR_1_CPU_STATUS_LINK_UP_INTR_FIELD_SHIFT + 6] = &port6_link_h,
       [XPORT_INTR_1_CPU_STATUS_LINK_DOWN_INTR_FIELD_SHIFT] = &port0_link_h,
       [XPORT_INTR_1_CPU_STATUS_LINK_DOWN_INTR_FIELD_SHIFT + 1] = &port1_link_h,
       [XPORT_INTR_1_CPU_STATUS_LINK_DOWN_INTR_FIELD_SHIFT + 2] = &port2_link_h,
       [XPORT_INTR_1_CPU_STATUS_LINK_DOWN_INTR_FIELD_SHIFT + 3] = &port3_link_h,
       [XPORT_INTR_1_CPU_STATUS_LINK_DOWN_INTR_FIELD_SHIFT + 4] = &port4_link_h,
       [XPORT_INTR_1_CPU_STATUS_LINK_DOWN_INTR_FIELD_SHIFT + 5] = &port5_link_h,
       [XPORT_INTR_1_CPU_STATUS_LINK_DOWN_INTR_FIELD_SHIFT + 6] = &port6_link_h,
       [XPORT_INTR_1_CPU_STATUS_RX_REMOTE_FAULT_INTR_FIELD_SHIFT] = &port0_remote_fault_h,
       [XPORT_INTR_1_CPU_STATUS_RX_REMOTE_FAULT_INTR_FIELD_SHIFT + 1] = &port4_remote_fault_h
    }
};

static intr_handler_s *id_to_handler[XPORT_INTR_LAST][ENTITY_ID_COUNT] = 
{
    [XPORT_MDIO_DONE] = {&mdio_done_h},
    [XPORT_MDIO_READ_ERROR] = {&mdio_error_h},
    [XPORT_PORT_LINK] = {&port0_link_h, &port1_link_h, &port2_link_h, &port3_link_h, &port4_link_h, &port5_link_h, &port6_link_h},
    [XPORT_REMOTE_FAULT] = {&port0_remote_fault_h, &port4_remote_fault_h},
    [XPORT_TIMESYNC_FIFO] = {&tx_timesync_fifo_h}
};

#define INTR_DISABLE 0
#define INTR_ENABLE 1

static void intr0_mask_set(uint32_t mask);
static void intr1_mask_set(uint32_t mask);
static void intr0_mask_clear(uint32_t mask);
static void intr1_mask_clear(uint32_t mask);

void (*intr_enable_op[2][2])(uint32_t) = 
{
    {intr0_mask_set, intr0_mask_clear}, 
    {intr1_mask_set, intr1_mask_clear}  
};

static int port_link_status_get(intr_handler_s *handler, uint32_t reg_val)
{
    if (reg_val & XPORT_INTR_1_CPU_STATUS_LINK_UP_INTR_FIELD_MASK)
        return 1;
    return 0;
}

static void intr0_mask_set(uint32_t mask)
{
    RU_REG_WRITE(0, XPORT_INTR, 0_CPU_MASK_SET, mask);
}

static void intr1_mask_set(uint32_t mask)
{
    RU_REG_WRITE(0, XPORT_INTR, 1_CPU_MASK_SET, mask);
}

static void intr0_mask_clear(uint32_t mask)
{
    RU_REG_WRITE(0, XPORT_INTR, 0_CPU_MASK_CLEAR, mask);
}

static void intr1_mask_clear(uint32_t mask)
{
    RU_REG_WRITE(0, XPORT_INTR, 1_CPU_MASK_CLEAR, mask);
}

static int xport_isr(int irq, void *priv)
{
    uint32_t status_reg_val = 0, active_bit;
    uint32_t register_num = (uint64_t)priv;
    uint32_t mask;

    if (register_num == 0)
    {
        RU_REG_READ(0, XPORT_INTR, 0_CPU_STATUS, status_reg_val);
        RU_REG_READ(0, XPORT_INTR, 0_CPU_MASK_STATUS, mask);
    }
    else
    {
        RU_REG_READ(0, XPORT_INTR, 1_CPU_STATUS, status_reg_val);
        RU_REG_READ(0, XPORT_INTR, 1_CPU_MASK_STATUS, mask);
    }

    while (status_reg_val)
    {
        intr_handler_s *handler;

        active_bit = ffs(status_reg_val) - 1;
        handler = bit_to_handler[register_num][active_bit];

        if (!handler || !(status_reg_val & ~mask)) 
            goto cont;

        if (handler->status_get_cb)
            handler->info.status = handler->status_get_cb(handler, status_reg_val);
        else
            handler->info.status = status_reg_val & handler->mask;

        if (handler->isr_cb)
            handler->isr_cb(&handler->info, handler->isr_priv);
cont:
        /* prepare status for next iteration */
        status_reg_val &= handler ? ~(handler->mask) : ~(1U << active_bit);
    }

    return IRQ_HANDLED;
}

static void do_intr_clear(intr_handler_s *intr_handler)
{ 
    if (intr_handler->register_num == 0)
        RU_REG_WRITE(0, XPORT_INTR, 0_CPU_CLEAR, intr_handler->mask);
    else
        RU_REG_WRITE(0, XPORT_INTR, 1_CPU_CLEAR, intr_handler->mask);
}

int xport_intr_init(void)
{
    uint32_t mask = 0xffffffff;
    int res = 0;

    RU_REG_WRITE(0, XPORT_INTR, 0_CPU_MASK_SET, mask);
    RU_REG_WRITE(0, XPORT_INTR, 1_CPU_MASK_SET, mask);
    if ((res = BcmHalMapInterrupt((FN_HANDLER)xport_isr, (void *)0, INTERRUPT_ID_XPORT_INTR_0_CPU_OUT)))
        return res;
    if ((res = BcmHalMapInterrupt((FN_HANDLER)xport_isr, (void *)1, INTERRUPT_ID_XPORT_INTR_1_CPU_OUT)))
        return res;

    return res;
}

int xport_intr_register(XPORT_INTR_ID intr_id, uint32_t entity_id, xport_intr_cb_t isr_cb, void *priv)
{
    intr_handler_s *intr_handler;

    if (entity_id >= ENTITY_ID_COUNT || intr_id >= XPORT_INTR_LAST)
        return XPORT_ERR_PARAM; /* wrong param */

    intr_handler = id_to_handler[intr_id][entity_id];
    if (intr_handler->isr_cb)
        return XPORT_ERR_STATE; /* Already registered */

    intr_handler->isr_cb = isr_cb;
    intr_handler->isr_priv = priv;

    return XPORT_ERR_OK;
}
EXPORT_SYMBOL(xport_intr_register);

void xport_intr_unregister(XPORT_INTR_ID intr_id, uint32_t entity_id)
{
    intr_handler_s *intr_handler;

    if (entity_id >= ENTITY_ID_COUNT || intr_id >= XPORT_INTR_LAST)
        return;
    intr_handler = id_to_handler[entity_id][entity_id];
    if (!intr_handler)
        return;

    intr_enable_op[intr_handler->register_num][INTR_DISABLE](intr_handler->mask);
    intr_handler->isr_cb = NULL;
}

void xport_intr_enable(XPORT_INTR_ID intr_id, uint32_t entity_id, int enable)
{
    intr_handler_s *intr_handler;

    if (entity_id >= ENTITY_ID_COUNT || intr_id >= XPORT_INTR_LAST)
        return;
    intr_handler = id_to_handler[intr_id][entity_id];
    if (!intr_handler)
        return;

    intr_enable_op[intr_handler->register_num][enable](intr_handler->mask);
}
EXPORT_SYMBOL(xport_intr_enable);

void xport_intr_clear(XPORT_INTR_ID intr_id, uint32_t entity_id)
{
     intr_handler_s *intr_handler;

    if (entity_id >= ENTITY_ID_COUNT || intr_id >= XPORT_INTR_LAST)
        return;
    intr_handler = id_to_handler[intr_id][entity_id];
    if (!intr_handler)
        return;

    do_intr_clear(intr_handler);
}
EXPORT_SYMBOL(xport_intr_clear);

