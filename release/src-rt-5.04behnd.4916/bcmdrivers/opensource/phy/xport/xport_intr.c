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
#include "bcm6888_xport_intr_ag.h"
#include "xport_defs.h"
#include "xport_intr.h"

#define ENTITY_ID_COUNT 7

typedef struct intr_handler_s
{
    xport_intr_info_s info;
    uint32_t register_num;
    uint32_t mask;
    int (*status_get_cb)(const struct intr_handler_s *, uint32_t);
    xport_intr_cb_t isr_cb;
    void *isr_priv;
} intr_handler_s;

intr_handler_s
    tx_timesync_fifo_h0 = { .info = {.intr_id = XPORT_TIMESYNC_FIFO}, .register_num = 0,
        .mask = XPORT_INTR_CPU_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_MASK},
    tx_timesync_fifo_h1 = { .info = {.intr_id = XPORT_TIMESYNC_FIFO}, .register_num = 1,
        .mask = XPORT_INTR_CPU_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_MASK},
    tx_timesync_fifo_h2 = { .info = {.intr_id = XPORT_TIMESYNC_FIFO}, .register_num = 2,
        .mask = XPORT_INTR_CPU_SET_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_MASK};

/* Interrupt info registry - 3 registers, 32 bits each */
static intr_handler_s *bit_to_handler[XPORT_NUM_OF_PORTS_PER_XLMAC][32] =
{
    [0] =
    {
       [XPORT_INTR_CPU_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT + 0] = &tx_timesync_fifo_h0,
       [XPORT_INTR_CPU_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT + 1] = &tx_timesync_fifo_h0,
       [XPORT_INTR_CPU_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT + 2] = &tx_timesync_fifo_h0,
       [XPORT_INTR_CPU_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT + 3] = &tx_timesync_fifo_h0,
    },
    [1] =
    {
       [XPORT_INTR_CPU_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT + 0] = &tx_timesync_fifo_h1,
       [XPORT_INTR_CPU_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT + 1] = &tx_timesync_fifo_h1,
       [XPORT_INTR_CPU_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT + 2] = &tx_timesync_fifo_h1,
       [XPORT_INTR_CPU_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT + 3] = &tx_timesync_fifo_h1,
    },
    [2] =
    {
       [XPORT_INTR_CPU_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT + 0] = &tx_timesync_fifo_h2,
       [XPORT_INTR_CPU_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT + 1] = &tx_timesync_fifo_h2,
       [XPORT_INTR_CPU_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT + 2] = &tx_timesync_fifo_h2,
       [XPORT_INTR_CPU_STATUS_TX_TIMESYNC_FIFO_ENTRY_VALID_INTR_FIELD_SHIFT + 3] = &tx_timesync_fifo_h2,
    },
};

static intr_handler_s *id_to_handler[XPORT_NUM_OF_PORTS_PER_XLMAC][XPORT_INTR_LAST][ENTITY_ID_COUNT] =
{
    [0][XPORT_TIMESYNC_FIFO] = {&tx_timesync_fifo_h0},
    [1][XPORT_TIMESYNC_FIFO] = {&tx_timesync_fifo_h1},
    [2][XPORT_TIMESYNC_FIFO] = {&tx_timesync_fifo_h2},
};

#define INTR_DISABLE 0
#define INTR_ENABLE 1

static void intr0_mask_set(uint32_t mask)
{
    RU_REG_WRITE(0, XPORT_INTR, CPU_MASK_SET, mask);
}

static void intr1_mask_set(uint32_t mask)
{
    RU_REG_WRITE(1, XPORT_INTR, CPU_MASK_SET, mask);
}

static void intr2_mask_set(uint32_t mask)
{
    RU_REG_WRITE(2, XPORT_INTR, CPU_MASK_SET, mask);
}

static void intr0_mask_clear(uint32_t mask)
{
    RU_REG_WRITE(0, XPORT_INTR, CPU_MASK_CLEAR, mask);
}

static void intr1_mask_clear(uint32_t mask)
{
    RU_REG_WRITE(1, XPORT_INTR, CPU_MASK_CLEAR, mask);
}

static void intr2_mask_clear(uint32_t mask)
{
    RU_REG_WRITE(2, XPORT_INTR, CPU_MASK_CLEAR, mask);
}

static void (* const intr_enable_op[XPORT_NUM_OF_PORTS_PER_XLMAC][2])(uint32_t) =
{
    {intr0_mask_set, intr0_mask_clear},
    {intr1_mask_set, intr1_mask_clear},
    {intr2_mask_set, intr2_mask_clear},
};

static irqreturn_t xport_isr(int irq, void *priv)
{
    uint32_t status_reg_val = 0, active_bit;
    uint32_t register_num = (uint64_t)priv;
    uint32_t mask;

    RU_REG_READ(register_num, XPORT_INTR, CPU_STATUS, status_reg_val);
    RU_REG_READ(register_num, XPORT_INTR, CPU_MASK_STATUS, mask);

    status_reg_val &= ~mask; /* ignore masked interrupts */
    while (status_reg_val)
    {
        intr_handler_s *handler;

        active_bit = ffs(status_reg_val) - 1;
        handler = bit_to_handler[register_num][active_bit];

        if (handler)
        {
            if (handler->status_get_cb)
                handler->info.status = handler->status_get_cb(handler, status_reg_val);
            else
                handler->info.status = status_reg_val & handler->mask;

            if (handler->isr_cb)
                handler->isr_cb(&handler->info, handler->isr_priv);
            status_reg_val &= ~(handler->mask);
        }
        else
            status_reg_val &= ~(1U << active_bit);
    }

    return IRQ_HANDLED;
}

static void do_intr_clear(const intr_handler_s *intr_handler)
{
    RU_REG_WRITE(intr_handler->register_num, XPORT_INTR, CPU_CLEAR, intr_handler->mask);
}

int xport_intr_init(int irq0, int irq1, int irq2)
{
#ifdef CONFIG_BCM_PTP_1588
    uint32_t mask = 0x000ff0ff; /* don't mask ts_fifo xport ints that were enabled by xport_intr_enable() */
    uint32_t clr  = 0x00000f00;
#else
    uint32_t mask = 0x000fffff;
    uint32_t clr  = 0x00000000;
#endif
    int res;

    RU_REG_WRITE(0, XPORT_INTR, CPU_MASK_SET, mask);
    RU_REG_WRITE(1, XPORT_INTR, CPU_MASK_SET, mask);
    RU_REG_WRITE(2, XPORT_INTR, CPU_MASK_SET, mask);
    RU_REG_WRITE(0, XPORT_INTR, CPU_MASK_CLEAR, clr);
    RU_REG_WRITE(1, XPORT_INTR, CPU_MASK_CLEAR, clr);
    RU_REG_WRITE(2, XPORT_INTR, CPU_MASK_CLEAR, clr);
    if ((res = BcmHalMapInterrupt((FN_HANDLER)xport_isr, (void *)0, irq0)))
        return res;
    if ((res = BcmHalMapInterrupt((FN_HANDLER)xport_isr, (void *)1, irq1)))
        return res;
    if ((res = BcmHalMapInterrupt((FN_HANDLER)xport_isr, (void *)2, irq2)))
        return res;

    return 0;
}

int xport_intr_register(XPORT_INTR_ID intr_id, uint32_t entity_id, xport_intr_cb_t isr_cb, void *priv)
{
    intr_handler_s *intr_handler;

    if (entity_id >= ENTITY_ID_COUNT || intr_id >= XPORT_INTR_LAST)
        return XPORT_ERR_PARAM; /* wrong param */

    intr_handler = id_to_handler[(uint64_t)priv][intr_id][entity_id];
    if (intr_handler->isr_cb)
        return XPORT_ERR_STATE; /* Already registered */

    intr_handler->isr_cb = isr_cb;
    intr_handler->isr_priv = priv;

    return XPORT_ERR_OK;
}
EXPORT_SYMBOL(xport_intr_register);

void xport_intr_unregister(XPORT_INTR_ID intr_id, uint32_t entity_id, uint32_t register_num)
{
    intr_handler_s *intr_handler;

    if (entity_id >= ENTITY_ID_COUNT || intr_id >= XPORT_INTR_LAST)
        return;
    intr_handler = id_to_handler[register_num][intr_id][entity_id];
    if (!intr_handler)
        return;

    intr_enable_op[intr_handler->register_num][INTR_DISABLE](intr_handler->mask);
    intr_handler->isr_cb = NULL;
}

void xport_intr_enable(XPORT_INTR_ID intr_id, uint32_t entity_id, int enable, uint32_t register_num)
{
    const intr_handler_s *intr_handler;

    if (entity_id >= ENTITY_ID_COUNT || intr_id >= XPORT_INTR_LAST)
        return;
    intr_handler = id_to_handler[register_num][intr_id][entity_id];
    if (!intr_handler)
        return;

    intr_enable_op[intr_handler->register_num][enable](intr_handler->mask);
}
EXPORT_SYMBOL(xport_intr_enable);

void xport_intr_clear(XPORT_INTR_ID intr_id, uint32_t entity_id, uint32_t register_num)
{
     const intr_handler_s *intr_handler;

    if (entity_id >= ENTITY_ID_COUNT || intr_id >= XPORT_INTR_LAST)
        return;
    intr_handler = id_to_handler[register_num][intr_id][entity_id];
    if (!intr_handler)
        return;

    do_intr_clear(intr_handler);
}
EXPORT_SYMBOL(xport_intr_clear);

