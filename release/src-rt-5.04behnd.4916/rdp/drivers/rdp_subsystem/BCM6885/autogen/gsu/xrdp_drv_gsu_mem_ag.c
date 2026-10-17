/*
   Copyright (c) 2015 Broadcom
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


#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_gsu_mem_ag.h"

#define BLOCK_ADDR_COUNT_BITS 1
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_gsu_mem_pd_fifo_entry_set(uint8_t gsu_mem_id, uint32_t pd_fifo, uint32_t val)
{
    uint32_t reg_pd_fifo_entry = 0;

#ifdef VALIDATE_PARMS
    if ((gsu_mem_id >= BLOCK_ADDR_COUNT) ||
       (pd_fifo >= 10240))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pd_fifo_entry = RU_FIELD_SET(gsu_mem_id, GSU_MEM, PD_FIFO_ENTRY, VAL, reg_pd_fifo_entry, val);

    RU_REG_RAM_WRITE(gsu_mem_id, pd_fifo, GSU_MEM, PD_FIFO_ENTRY, reg_pd_fifo_entry);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_mem_pd_fifo_entry_get(uint8_t gsu_mem_id, uint32_t pd_fifo, uint32_t *val)
{
    uint32_t reg_pd_fifo_entry;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_mem_id >= BLOCK_ADDR_COUNT) ||
       (pd_fifo >= 10240))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(gsu_mem_id, pd_fifo, GSU_MEM, PD_FIFO_ENTRY, reg_pd_fifo_entry);

    *val = RU_FIELD_GET(gsu_mem_id, GSU_MEM, PD_FIFO_ENTRY, VAL, reg_pd_fifo_entry);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_mem_queue_data_entry_set(uint8_t gsu_mem_id, uint32_t queue_data, uint32_t val)
{
    uint32_t reg_queue_data_entry = 0;

#ifdef VALIDATE_PARMS
    if ((gsu_mem_id >= BLOCK_ADDR_COUNT) ||
       (queue_data >= 5120))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_queue_data_entry = RU_FIELD_SET(gsu_mem_id, GSU_MEM, QUEUE_DATA_ENTRY, VAL, reg_queue_data_entry, val);

    RU_REG_RAM_WRITE(gsu_mem_id, queue_data, GSU_MEM, QUEUE_DATA_ENTRY, reg_queue_data_entry);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_mem_queue_data_entry_get(uint8_t gsu_mem_id, uint32_t queue_data, uint32_t *val)
{
    uint32_t reg_queue_data_entry;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_mem_id >= BLOCK_ADDR_COUNT) ||
       (queue_data >= 5120))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(gsu_mem_id, queue_data, GSU_MEM, QUEUE_DATA_ENTRY, reg_queue_data_entry);

    *val = RU_FIELD_GET(gsu_mem_id, GSU_MEM, QUEUE_DATA_ENTRY, VAL, reg_queue_data_entry);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_mem_node_mask_vector_entry_set(uint8_t gsu_mem_id, uint32_t node_mask, uint8_t entry_0_val, uint8_t entry_1_val, uint8_t entry_2_val, uint8_t entry_3_val)
{
    uint32_t reg_node_mask_vector_entry = 0;

#ifdef VALIDATE_PARMS
    if ((gsu_mem_id >= BLOCK_ADDR_COUNT) ||
       (node_mask >= 4864) ||
       (entry_0_val >= _3BITS_MAX_VAL_) ||
       (entry_1_val >= _3BITS_MAX_VAL_) ||
       (entry_2_val >= _3BITS_MAX_VAL_) ||
       (entry_3_val >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_node_mask_vector_entry = RU_FIELD_SET(gsu_mem_id, GSU_MEM, NODE_MASK_VECTOR_ENTRY, ENTRY_0_VAL, reg_node_mask_vector_entry, entry_0_val);
    reg_node_mask_vector_entry = RU_FIELD_SET(gsu_mem_id, GSU_MEM, NODE_MASK_VECTOR_ENTRY, ENTRY_1_VAL, reg_node_mask_vector_entry, entry_1_val);
    reg_node_mask_vector_entry = RU_FIELD_SET(gsu_mem_id, GSU_MEM, NODE_MASK_VECTOR_ENTRY, ENTRY_2_VAL, reg_node_mask_vector_entry, entry_2_val);
    reg_node_mask_vector_entry = RU_FIELD_SET(gsu_mem_id, GSU_MEM, NODE_MASK_VECTOR_ENTRY, ENTRY_3_VAL, reg_node_mask_vector_entry, entry_3_val);

    RU_REG_RAM_WRITE(gsu_mem_id, node_mask, GSU_MEM, NODE_MASK_VECTOR_ENTRY, reg_node_mask_vector_entry);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_mem_node_mask_vector_entry_get(uint8_t gsu_mem_id, uint32_t node_mask, uint8_t *entry_0_val, uint8_t *entry_1_val, uint8_t *entry_2_val, uint8_t *entry_3_val)
{
    uint32_t reg_node_mask_vector_entry;

#ifdef VALIDATE_PARMS
    if (!entry_0_val || !entry_1_val || !entry_2_val || !entry_3_val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_mem_id >= BLOCK_ADDR_COUNT) ||
       (node_mask >= 4864))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(gsu_mem_id, node_mask, GSU_MEM, NODE_MASK_VECTOR_ENTRY, reg_node_mask_vector_entry);

    *entry_0_val = RU_FIELD_GET(gsu_mem_id, GSU_MEM, NODE_MASK_VECTOR_ENTRY, ENTRY_0_VAL, reg_node_mask_vector_entry);
    *entry_1_val = RU_FIELD_GET(gsu_mem_id, GSU_MEM, NODE_MASK_VECTOR_ENTRY, ENTRY_1_VAL, reg_node_mask_vector_entry);
    *entry_2_val = RU_FIELD_GET(gsu_mem_id, GSU_MEM, NODE_MASK_VECTOR_ENTRY, ENTRY_2_VAL, reg_node_mask_vector_entry);
    *entry_3_val = RU_FIELD_GET(gsu_mem_id, GSU_MEM, NODE_MASK_VECTOR_ENTRY, ENTRY_3_VAL, reg_node_mask_vector_entry);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_mem_context_data_entry_set(uint8_t gsu_mem_id, uint32_t context_data, uint32_t val)
{
    uint32_t reg_context_data_entry = 0;

#ifdef VALIDATE_PARMS
    if ((gsu_mem_id >= BLOCK_ADDR_COUNT) ||
       (context_data >= 8960))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_context_data_entry = RU_FIELD_SET(gsu_mem_id, GSU_MEM, CONTEXT_DATA_ENTRY, VAL, reg_context_data_entry, val);

    RU_REG_RAM_WRITE(gsu_mem_id, context_data, GSU_MEM, CONTEXT_DATA_ENTRY, reg_context_data_entry);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_mem_context_data_entry_get(uint8_t gsu_mem_id, uint32_t context_data, uint32_t *val)
{
    uint32_t reg_context_data_entry;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_mem_id >= BLOCK_ADDR_COUNT) ||
       (context_data >= 8960))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(gsu_mem_id, context_data, GSU_MEM, CONTEXT_DATA_ENTRY, reg_context_data_entry);

    *val = RU_FIELD_GET(gsu_mem_id, GSU_MEM, CONTEXT_DATA_ENTRY, VAL, reg_context_data_entry);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_mem_input_info_entry_set(uint8_t gsu_mem_id, uint32_t input_info, uint32_t val)
{
    uint32_t reg_input_info_entry = 0;

#ifdef VALIDATE_PARMS
    if ((gsu_mem_id >= BLOCK_ADDR_COUNT) ||
       (input_info >= 5120))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_input_info_entry = RU_FIELD_SET(gsu_mem_id, GSU_MEM, INPUT_INFO_ENTRY, VAL, reg_input_info_entry, val);

    RU_REG_RAM_WRITE(gsu_mem_id, input_info, GSU_MEM, INPUT_INFO_ENTRY, reg_input_info_entry);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_gsu_mem_input_info_entry_get(uint8_t gsu_mem_id, uint32_t input_info, uint32_t *val)
{
    uint32_t reg_input_info_entry;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((gsu_mem_id >= BLOCK_ADDR_COUNT) ||
       (input_info >= 5120))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(gsu_mem_id, input_info, GSU_MEM, INPUT_INFO_ENTRY, reg_input_info_entry);

    *val = RU_FIELD_GET(gsu_mem_id, GSU_MEM, INPUT_INFO_ENTRY, VAL, reg_input_info_entry);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_pd_fifo_entry,
    bdmf_address_queue_data_entry,
    bdmf_address_node_mask_vector_entry,
    bdmf_address_context_data_entry,
    bdmf_address_input_info_entry,
}
bdmf_address;

static int ag_drv_gsu_mem_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_gsu_mem_pd_fifo_entry:
        ag_err = ag_drv_gsu_mem_pd_fifo_entry_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_gsu_mem_queue_data_entry:
        ag_err = ag_drv_gsu_mem_queue_data_entry_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_gsu_mem_node_mask_vector_entry:
        ag_err = ag_drv_gsu_mem_node_mask_vector_entry_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber, parm[6].value.unumber);
        break;
    case cli_gsu_mem_context_data_entry:
        ag_err = ag_drv_gsu_mem_context_data_entry_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_gsu_mem_input_info_entry:
        ag_err = ag_drv_gsu_mem_input_info_entry_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_gsu_mem_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_gsu_mem_pd_fifo_entry:
    {
        uint32_t val;
        ag_err = ag_drv_gsu_mem_pd_fifo_entry_get(parm[1].value.unumber, parm[2].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_gsu_mem_queue_data_entry:
    {
        uint32_t val;
        ag_err = ag_drv_gsu_mem_queue_data_entry_get(parm[1].value.unumber, parm[2].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_gsu_mem_node_mask_vector_entry:
    {
        uint8_t entry_0_val;
        uint8_t entry_1_val;
        uint8_t entry_2_val;
        uint8_t entry_3_val;
        ag_err = ag_drv_gsu_mem_node_mask_vector_entry_get(parm[1].value.unumber, parm[2].value.unumber, &entry_0_val, &entry_1_val, &entry_2_val, &entry_3_val);
        bdmf_session_print(session, "entry_0_val = %u = 0x%x\n", entry_0_val, entry_0_val);
        bdmf_session_print(session, "entry_1_val = %u = 0x%x\n", entry_1_val, entry_1_val);
        bdmf_session_print(session, "entry_2_val = %u = 0x%x\n", entry_2_val, entry_2_val);
        bdmf_session_print(session, "entry_3_val = %u = 0x%x\n", entry_3_val, entry_3_val);
        break;
    }
    case cli_gsu_mem_context_data_entry:
    {
        uint32_t val;
        ag_err = ag_drv_gsu_mem_context_data_entry_get(parm[1].value.unumber, parm[2].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_gsu_mem_input_info_entry:
    {
        uint32_t val;
        ag_err = ag_drv_gsu_mem_input_info_entry_get(parm[1].value.unumber, parm[2].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_gsu_mem_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t gsu_mem_id = parm[1].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        uint32_t pd_fifo = gtmv(m, 13);
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_gsu_mem_pd_fifo_entry_set(%u %u %u)\n", gsu_mem_id, pd_fifo,
            val);
        ag_err = ag_drv_gsu_mem_pd_fifo_entry_set(gsu_mem_id, pd_fifo, val);
        if (!ag_err)
            ag_err = ag_drv_gsu_mem_pd_fifo_entry_get(gsu_mem_id, pd_fifo, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_mem_pd_fifo_entry_get(%u %u %u)\n", gsu_mem_id, pd_fifo,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t queue_data = gtmv(m, 12);
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_gsu_mem_queue_data_entry_set(%u %u %u)\n", gsu_mem_id, queue_data,
            val);
        ag_err = ag_drv_gsu_mem_queue_data_entry_set(gsu_mem_id, queue_data, val);
        if (!ag_err)
            ag_err = ag_drv_gsu_mem_queue_data_entry_get(gsu_mem_id, queue_data, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_mem_queue_data_entry_get(%u %u %u)\n", gsu_mem_id, queue_data,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t node_mask = gtmv(m, 12);
        uint8_t entry_0_val = gtmv(m, 3);
        uint8_t entry_1_val = gtmv(m, 3);
        uint8_t entry_2_val = gtmv(m, 3);
        uint8_t entry_3_val = gtmv(m, 3);
        bdmf_session_print(session, "ag_drv_gsu_mem_node_mask_vector_entry_set(%u %u %u %u %u %u)\n", gsu_mem_id, node_mask,
            entry_0_val, entry_1_val, entry_2_val, entry_3_val);
        ag_err = ag_drv_gsu_mem_node_mask_vector_entry_set(gsu_mem_id, node_mask, entry_0_val, entry_1_val, entry_2_val, entry_3_val);
        if (!ag_err)
            ag_err = ag_drv_gsu_mem_node_mask_vector_entry_get(gsu_mem_id, node_mask, &entry_0_val, &entry_1_val, &entry_2_val, &entry_3_val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_mem_node_mask_vector_entry_get(%u %u %u %u %u %u)\n", gsu_mem_id, node_mask,
                entry_0_val, entry_1_val, entry_2_val, entry_3_val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (entry_0_val != gtmv(m, 3) || entry_1_val != gtmv(m, 3) || entry_2_val != gtmv(m, 3) || entry_3_val != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t context_data = gtmv(m, 13);
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_gsu_mem_context_data_entry_set(%u %u %u)\n", gsu_mem_id, context_data,
            val);
        ag_err = ag_drv_gsu_mem_context_data_entry_set(gsu_mem_id, context_data, val);
        if (!ag_err)
            ag_err = ag_drv_gsu_mem_context_data_entry_get(gsu_mem_id, context_data, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_mem_context_data_entry_get(%u %u %u)\n", gsu_mem_id, context_data,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t input_info = gtmv(m, 12);
        uint32_t val = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_gsu_mem_input_info_entry_set(%u %u %u)\n", gsu_mem_id, input_info,
            val);
        ag_err = ag_drv_gsu_mem_input_info_entry_set(gsu_mem_id, input_info, val);
        if (!ag_err)
            ag_err = ag_drv_gsu_mem_input_info_entry_get(gsu_mem_id, input_info, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_gsu_mem_input_info_entry_get(%u %u %u)\n", gsu_mem_id, input_info,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    bdmf_session_print(session, "successes=%u failures=%u\n", test_success_cnt, test_failure_cnt);

    return ag_err;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_gsu_mem_cli_ext_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m, input_method = parm[0].value.unumber;
    uint8_t gsu_mem_id = parm[2].value.unumber;
    bdmfmon_cmd_parm_t *p_start, *p_stop;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t ext_test_success_cnt = 0;
    uint32_t ext_test_failure_cnt = 0;
    uint32_t start_idx = 0;
    uint32_t stop_idx = 0;

    p_start = bdmfmon_cmd_find(session, "start_idx");
    p_stop = bdmfmon_cmd_find(session, "stop_idx");

    if (p_start)
        start_idx = p_start->value.unumber;
    if (p_stop)
        stop_idx = p_stop->value.unumber;

    if ((start_idx > stop_idx) && (stop_idx != 0))
    {
        bdmf_session_print(session, "ERROR: start_idx must be less than stop_idx\n");
        return BDMF_ERR_PARM;
    }

    m = bdmf_test_method_high; /* "Initialization" method */
    switch (parm[1].value.unumber)
    {
    case cli_gsu_mem_pd_fifo_entry:
    {
        uint32_t max_pd_fifo = 10240;
        uint32_t pd_fifo = gtmv(m, 13);
        uint32_t val = gtmv(m, 32);

        if ((start_idx >= max_pd_fifo) || (stop_idx >= max_pd_fifo))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_pd_fifo);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (pd_fifo = 0; pd_fifo < max_pd_fifo; pd_fifo++)
        {
            bdmf_session_print(session, "ag_drv_gsu_mem_pd_fifo_entry_set(%u %u %u)\n", gsu_mem_id, pd_fifo,
                val);
            ag_err = ag_drv_gsu_mem_pd_fifo_entry_set(gsu_mem_id, pd_fifo, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", pd_fifo);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        val = gtmv(m, 32);

        for (pd_fifo = start_idx; pd_fifo <= stop_idx; pd_fifo++)
        {
            bdmf_session_print(session, "ag_drv_gsu_mem_pd_fifo_entry_set(%u %u %u)\n", gsu_mem_id, pd_fifo,
                val);
            ag_err = ag_drv_gsu_mem_pd_fifo_entry_set(gsu_mem_id, pd_fifo, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", pd_fifo);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (pd_fifo = 0; pd_fifo < max_pd_fifo; pd_fifo++)
        {
            if (pd_fifo < start_idx || pd_fifo > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_gsu_mem_pd_fifo_entry_get(gsu_mem_id, pd_fifo, &val);

            bdmf_session_print(session, "ag_drv_gsu_mem_pd_fifo_entry_get(%u %u %u)\n", gsu_mem_id, pd_fifo,
                val);

            if (val != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", pd_fifo);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of pd_fifo_entry completed. Number of tested entries %u.\n", max_pd_fifo);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_gsu_mem_queue_data_entry:
    {
        uint32_t max_queue_data = 5120;
        uint32_t queue_data = gtmv(m, 12);
        uint32_t val = gtmv(m, 32);

        if ((start_idx >= max_queue_data) || (stop_idx >= max_queue_data))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_queue_data);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (queue_data = 0; queue_data < max_queue_data; queue_data++)
        {
            bdmf_session_print(session, "ag_drv_gsu_mem_queue_data_entry_set(%u %u %u)\n", gsu_mem_id, queue_data,
                val);
            ag_err = ag_drv_gsu_mem_queue_data_entry_set(gsu_mem_id, queue_data, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", queue_data);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        val = gtmv(m, 32);

        for (queue_data = start_idx; queue_data <= stop_idx; queue_data++)
        {
            bdmf_session_print(session, "ag_drv_gsu_mem_queue_data_entry_set(%u %u %u)\n", gsu_mem_id, queue_data,
                val);
            ag_err = ag_drv_gsu_mem_queue_data_entry_set(gsu_mem_id, queue_data, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", queue_data);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (queue_data = 0; queue_data < max_queue_data; queue_data++)
        {
            if (queue_data < start_idx || queue_data > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_gsu_mem_queue_data_entry_get(gsu_mem_id, queue_data, &val);

            bdmf_session_print(session, "ag_drv_gsu_mem_queue_data_entry_get(%u %u %u)\n", gsu_mem_id, queue_data,
                val);

            if (val != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", queue_data);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of queue_data_entry completed. Number of tested entries %u.\n", max_queue_data);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_gsu_mem_node_mask_vector_entry:
    {
        uint32_t max_node_mask = 4864;
        uint32_t node_mask = gtmv(m, 12);
        uint8_t entry_0_val = gtmv(m, 3);
        uint8_t entry_1_val = gtmv(m, 3);
        uint8_t entry_2_val = gtmv(m, 3);
        uint8_t entry_3_val = gtmv(m, 3);

        if ((start_idx >= max_node_mask) || (stop_idx >= max_node_mask))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_node_mask);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (node_mask = 0; node_mask < max_node_mask; node_mask++)
        {
            bdmf_session_print(session, "ag_drv_gsu_mem_node_mask_vector_entry_set(%u %u %u %u %u %u)\n", gsu_mem_id, node_mask,
                entry_0_val, entry_1_val, entry_2_val, entry_3_val);
            ag_err = ag_drv_gsu_mem_node_mask_vector_entry_set(gsu_mem_id, node_mask, entry_0_val, entry_1_val, entry_2_val, entry_3_val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", node_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        entry_0_val = gtmv(m, 3);
        entry_1_val = gtmv(m, 3);
        entry_2_val = gtmv(m, 3);
        entry_3_val = gtmv(m, 3);

        for (node_mask = start_idx; node_mask <= stop_idx; node_mask++)
        {
            bdmf_session_print(session, "ag_drv_gsu_mem_node_mask_vector_entry_set(%u %u %u %u %u %u)\n", gsu_mem_id, node_mask,
                entry_0_val, entry_1_val, entry_2_val, entry_3_val);
            ag_err = ag_drv_gsu_mem_node_mask_vector_entry_set(gsu_mem_id, node_mask, entry_0_val, entry_1_val, entry_2_val, entry_3_val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", node_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (node_mask = 0; node_mask < max_node_mask; node_mask++)
        {
            if (node_mask < start_idx || node_mask > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_gsu_mem_node_mask_vector_entry_get(gsu_mem_id, node_mask, &entry_0_val, &entry_1_val, &entry_2_val, &entry_3_val);

            bdmf_session_print(session, "ag_drv_gsu_mem_node_mask_vector_entry_get(%u %u %u %u %u %u)\n", gsu_mem_id, node_mask,
                entry_0_val, entry_1_val, entry_2_val, entry_3_val);

            if (entry_0_val != gtmv(m, 3) || entry_1_val != gtmv(m, 3) || entry_2_val != gtmv(m, 3) || entry_3_val != gtmv(m, 3) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", node_mask);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of node_mask_vector_entry completed. Number of tested entries %u.\n", max_node_mask);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_gsu_mem_context_data_entry:
    {
        uint32_t max_context_data = 8960;
        uint32_t context_data = gtmv(m, 13);
        uint32_t val = gtmv(m, 32);

        if ((start_idx >= max_context_data) || (stop_idx >= max_context_data))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_context_data);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (context_data = 0; context_data < max_context_data; context_data++)
        {
            bdmf_session_print(session, "ag_drv_gsu_mem_context_data_entry_set(%u %u %u)\n", gsu_mem_id, context_data,
                val);
            ag_err = ag_drv_gsu_mem_context_data_entry_set(gsu_mem_id, context_data, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", context_data);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        val = gtmv(m, 32);

        for (context_data = start_idx; context_data <= stop_idx; context_data++)
        {
            bdmf_session_print(session, "ag_drv_gsu_mem_context_data_entry_set(%u %u %u)\n", gsu_mem_id, context_data,
                val);
            ag_err = ag_drv_gsu_mem_context_data_entry_set(gsu_mem_id, context_data, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", context_data);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (context_data = 0; context_data < max_context_data; context_data++)
        {
            if (context_data < start_idx || context_data > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_gsu_mem_context_data_entry_get(gsu_mem_id, context_data, &val);

            bdmf_session_print(session, "ag_drv_gsu_mem_context_data_entry_get(%u %u %u)\n", gsu_mem_id, context_data,
                val);

            if (val != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", context_data);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of context_data_entry completed. Number of tested entries %u.\n", max_context_data);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_gsu_mem_input_info_entry:
    {
        uint32_t max_input_info = 5120;
        uint32_t input_info = gtmv(m, 12);
        uint32_t val = gtmv(m, 32);

        if ((start_idx >= max_input_info) || (stop_idx >= max_input_info))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_input_info);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (input_info = 0; input_info < max_input_info; input_info++)
        {
            bdmf_session_print(session, "ag_drv_gsu_mem_input_info_entry_set(%u %u %u)\n", gsu_mem_id, input_info,
                val);
            ag_err = ag_drv_gsu_mem_input_info_entry_set(gsu_mem_id, input_info, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", input_info);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        val = gtmv(m, 32);

        for (input_info = start_idx; input_info <= stop_idx; input_info++)
        {
            bdmf_session_print(session, "ag_drv_gsu_mem_input_info_entry_set(%u %u %u)\n", gsu_mem_id, input_info,
                val);
            ag_err = ag_drv_gsu_mem_input_info_entry_set(gsu_mem_id, input_info, val);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", input_info);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (input_info = 0; input_info < max_input_info; input_info++)
        {
            if (input_info < start_idx || input_info > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_gsu_mem_input_info_entry_get(gsu_mem_id, input_info, &val);

            bdmf_session_print(session, "ag_drv_gsu_mem_input_info_entry_get(%u %u %u)\n", gsu_mem_id, input_info,
                val);

            if (val != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", input_info);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of input_info_entry completed. Number of tested entries %u.\n", max_input_info);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }

    return ag_err;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_gsu_mem_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int chip_rev_idx = RU_CHIP_REV_IDX_GET();
    uint32_t i;
    uint32_t j;
    uint32_t index1_start = 0;
    uint32_t index1_stop;
    uint32_t index2_start = 0;
    uint32_t index2_stop;
    bdmfmon_cmd_parm_t *cliparm;
    const ru_reg_rec *reg;
    const ru_block_rec *blk;
    const char *enum_string = bdmfmon_enum_parm_stringval(session, &parm[0]);

    if(!enum_string)
        return BDMF_ERR_INTERNAL;

    switch (parm[0].value.unumber)
    {
    case bdmf_address_pd_fifo_entry: reg = &RU_REG(GSU_MEM, PD_FIFO_ENTRY); blk = &RU_BLK(GSU_MEM); break;
    case bdmf_address_queue_data_entry: reg = &RU_REG(GSU_MEM, QUEUE_DATA_ENTRY); blk = &RU_BLK(GSU_MEM); break;
    case bdmf_address_node_mask_vector_entry: reg = &RU_REG(GSU_MEM, NODE_MASK_VECTOR_ENTRY); blk = &RU_BLK(GSU_MEM); break;
    case bdmf_address_context_data_entry: reg = &RU_REG(GSU_MEM, CONTEXT_DATA_ENTRY); blk = &RU_BLK(GSU_MEM); break;
    case bdmf_address_input_info_entry: reg = &RU_REG(GSU_MEM, INPUT_INFO_ENTRY); blk = &RU_BLK(GSU_MEM); break;
    default :
        return BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    if ((cliparm = bdmfmon_cmd_find(session, "index1")))
    {
        index1_start = cliparm->value.unumber;
        index1_stop = index1_start + 1;
    }
    else
        index1_stop = blk->addr_count;
    if ((cliparm = bdmfmon_cmd_find(session, "index2")))
    {
        index2_start = cliparm->value.unumber;
        index2_stop = index2_start + 1;
    }
    else
        index2_stop = reg->ram_count;
    if (index1_stop > blk->addr_count)
    {
        bdmf_session_print(session, "index1 (%u) is out of range (%u).\n", index1_stop, blk->addr_count);
        return BDMF_ERR_RANGE;
    }
    if (index2_stop > (reg->ram_count))
    {
        bdmf_session_print(session, "index2 (%u) is out of range (%u).\n", index2_stop, reg->ram_count);
        return BDMF_ERR_RANGE;
    }
    if (reg->ram_count)
        for (i = index1_start; i < index1_stop; i++)
        {
            bdmf_session_print(session, "index1 = %u\n", i);
            for (j = index2_start; j < index2_stop; j++)
                bdmf_session_print(session, TAB "(%5u) 0x%08X\n", j, ((blk->addr[i] + reg->addr[chip_rev_idx]) + j));
        }
    else
        for (i = index1_start; i < index1_stop; i++)
            bdmf_session_print(session, "(%3u) 0x%08X\n", i, blk->addr[i]+reg->addr[chip_rev_idx]);
    return 0;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

bdmfmon_handle_t ag_drv_gsu_mem_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "gsu_mem", "gsu_mem", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_pd_fifo_entry[] = {
            BDMFMON_MAKE_PARM("gsu_mem_id", "gsu_mem_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pd_fifo", "pd_fifo", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_queue_data_entry[] = {
            BDMFMON_MAKE_PARM("gsu_mem_id", "gsu_mem_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("queue_data", "queue_data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_node_mask_vector_entry[] = {
            BDMFMON_MAKE_PARM("gsu_mem_id", "gsu_mem_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("node_mask", "node_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("entry_0_val", "entry_0_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("entry_1_val", "entry_1_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("entry_2_val", "entry_2_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("entry_3_val", "entry_3_val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_context_data_entry[] = {
            BDMFMON_MAKE_PARM("gsu_mem_id", "gsu_mem_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("context_data", "context_data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_input_info_entry[] = {
            BDMFMON_MAKE_PARM("gsu_mem_id", "gsu_mem_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("input_info", "input_info", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "pd_fifo_entry", .val = cli_gsu_mem_pd_fifo_entry, .parms = set_pd_fifo_entry },
            { .name = "queue_data_entry", .val = cli_gsu_mem_queue_data_entry, .parms = set_queue_data_entry },
            { .name = "node_mask_vector_entry", .val = cli_gsu_mem_node_mask_vector_entry, .parms = set_node_mask_vector_entry },
            { .name = "context_data_entry", .val = cli_gsu_mem_context_data_entry, .parms = set_context_data_entry },
            { .name = "input_info_entry", .val = cli_gsu_mem_input_info_entry, .parms = set_input_info_entry },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_gsu_mem_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_pd_fifo_entry[] = {
            BDMFMON_MAKE_PARM("gsu_mem_id", "gsu_mem_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pd_fifo", "pd_fifo", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_queue_data_entry[] = {
            BDMFMON_MAKE_PARM("gsu_mem_id", "gsu_mem_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("queue_data", "queue_data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_node_mask_vector_entry[] = {
            BDMFMON_MAKE_PARM("gsu_mem_id", "gsu_mem_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("node_mask", "node_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_context_data_entry[] = {
            BDMFMON_MAKE_PARM("gsu_mem_id", "gsu_mem_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("context_data", "context_data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_input_info_entry[] = {
            BDMFMON_MAKE_PARM("gsu_mem_id", "gsu_mem_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("input_info", "input_info", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "pd_fifo_entry", .val = cli_gsu_mem_pd_fifo_entry, .parms = get_pd_fifo_entry },
            { .name = "queue_data_entry", .val = cli_gsu_mem_queue_data_entry, .parms = get_queue_data_entry },
            { .name = "node_mask_vector_entry", .val = cli_gsu_mem_node_mask_vector_entry, .parms = get_node_mask_vector_entry },
            { .name = "context_data_entry", .val = cli_gsu_mem_context_data_entry, .parms = get_context_data_entry },
            { .name = "input_info_entry", .val = cli_gsu_mem_input_info_entry, .parms = get_input_info_entry },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_gsu_mem_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            { .name = "high", .val = ag_drv_cli_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_gsu_mem_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0)            BDMFMON_MAKE_PARM("gsu_mem_id", "gsu_mem_id", BDMFMON_PARM_UNUMBER, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_cmd_parm_t ext_test_default[] = {
            BDMFMON_MAKE_PARM("gsu_mem_id", "gsu_mem_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "pd_fifo_entry", .val = cli_gsu_mem_pd_fifo_entry, .parms = ext_test_default},
            { .name = "queue_data_entry", .val = cli_gsu_mem_queue_data_entry, .parms = ext_test_default},
            { .name = "node_mask_vector_entry", .val = cli_gsu_mem_node_mask_vector_entry, .parms = ext_test_default},
            { .name = "context_data_entry", .val = cli_gsu_mem_context_data_entry, .parms = ext_test_default},
            { .name = "input_info_entry", .val = cli_gsu_mem_input_info_entry, .parms = ext_test_default},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "ext_test", "ext_test", ag_drv_gsu_mem_cli_ext_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0),
            BDMFMON_MAKE_PARM("start_idx", "array index to start test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("stop_idx", "array index to stop test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "PD_FIFO_ENTRY", .val = bdmf_address_pd_fifo_entry },
            { .name = "QUEUE_DATA_ENTRY", .val = bdmf_address_queue_data_entry },
            { .name = "NODE_MASK_VECTOR_ENTRY", .val = bdmf_address_node_mask_vector_entry },
            { .name = "CONTEXT_DATA_ENTRY", .val = bdmf_address_context_data_entry },
            { .name = "INPUT_INFO_ENTRY", .val = bdmf_address_input_info_entry },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_gsu_mem_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index1", "gsu_mem_id", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
