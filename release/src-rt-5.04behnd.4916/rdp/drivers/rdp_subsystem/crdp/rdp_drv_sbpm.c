/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
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

#include "rdp_subsystem_common.h"
#include "rdp_drv_sbpm.h"
#include "rdp_common.h"
#include "rdp_drv_fpm.h"
#include "xrdp_autogens.h"


#define SBPM_MAX_NUM_OF_ITERS 100
#define SBPM_ING_MAX_NUM_OF_BNS 2048
#define SBPM__EGR_MAX_NUM_OF_BNS 8192
#define INVALID_SBPM_14BITS_MAX_VAL_ ((1U<<14)-1)
#define SCAN_LINKED_LIST_MAX_NUM_OF_CHUNKS  200

static bdmfmon_handle_t debug_sbpm_cli_dir;

bdmf_error_t drv_sbpm_init(void)
{
    uint32_t sbpm_index;
    /*uint32_t bb_id; */
    bdmf_error_t rc = BDMF_ERR_OK;

    for (sbpm_index = 0; sbpm_index < NUM_OF_SBPM_INGRESS; sbpm_index++)
    {
        rc = rc ? rc : ag_drv_crdp_ing_sbpm_regs_init_free_list_set(sbpm_index, SBPM_INIT_OFFSET_ING, 0, 0, 0);
        rc = rc ? rc : ag_drv_crdp_ing_sbpm_regs_gl_trsh_set(sbpm_index, ING_GL_TRSH, 10);
        rc = rc ? rc : ag_drv_crdp_ing_sbpm_regs_gl_excl_high_trsh_set(sbpm_index, ING_GL_EXCL_HIGH_TRSH, 10);
        rc = rc ? rc : ag_drv_crdp_ing_sbpm_regs_gl_excl_med_trsh_set(sbpm_index, ING_GL_EXCL_MED_TRSH, 10);
        rc = rc ? rc : ag_drv_crdp_ing_sbpm_regs_gl_excl_low_trsh_set(sbpm_index, ING_GL_EXCL_LOW_TRSH, 10);
        BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: _sbpm_init ingress. index: %d\n", sbpm_index);
    }

    rc = rc ? rc : ag_drv_crdp_ing_sbpm_regs_excl_mask_set(0, BB_ID_DISPREST0_INGPIPE0_INGRESS0_HALF0_ID);
    rc = rc ? rc : ag_drv_crdp_ing_sbpm_regs_excl_mask_set(1, BB_ID_DISPREST0_INGPIPE1_INGRESS0_HALF0_ID);
    rc = rc ? rc : ag_drv_crdp_ing_sbpm_regs_excl_mask_set(2, BB_ID_DISPREST0_INGPIPE2_INGRESS0_HALF0_ID);

    for (sbpm_index = 0; sbpm_index < NUM_OF_SBPM_EGRESS; sbpm_index++)
    {
        rc = rc ? rc : ag_drv_crdp_ds_cmn_sbpm_regs_init_free_list_set(sbpm_index, SBPM_INIT_OFFSET_EGR, 0, 0, 0);
        rc = rc ? rc : ag_drv_crdp_ds_cmn_sbpm_regs_gl_trsh_set(sbpm_index, DS_CMN_GL_TRSH, 10);
        rc = rc ? rc : ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_high_trsh_set(sbpm_index, DS_CMN_EXCL_HIGH_TRSH, 10);
        rc = rc ? rc : ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_med_trsh_set(sbpm_index, DS_CMN_EXCL_MED_TRSH, 10);
        rc = rc ? rc : ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_low_trsh_set(sbpm_index, DS_CMN_EXCL_LOW_TRSH, 10);
        BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: _sbpm_init egress. index: %d\n", sbpm_index);
    }

    rc = rc ? rc : ag_drv_crdp_ds_cmn_sbpm_regs_excl_mask_set(0, BB_ID_QMING1_CMNEGR0_COMMON0_HALF0_ID);
    return rc;
}

/* TODO - implement host actions for spd test*/
bdmf_error_t drv_sbpm_alloc_and_connect_next_bn(uint32_t size, uint32_t headroom, uint8_t *data, uint16_t next_bn,
    bdmf_boolean inc_next_mc_counter, uint16_t *bn)
{
    return 0;
}

bdmf_error_t drv_sbpm_alloc_list(uint32_t size, uint32_t headroom, uint8_t *data,
    uint16_t *bn0, uint16_t *bn1, uint8_t *bns_num)
{
   return 0;
}

bdmf_error_t drv_sbpm_free_list(sbpm_type_e sbpm_type, uint8_t sbpm_id, uint16_t head_bn, uint16_t last_bn, uint8_t num_bns)
{
    return 0;
}

#ifdef USE_BDMF_SHELL
void _drv_crdp_sbpm_parse_isr(bdmf_session_handle session, uint8_t sbpm_index, uint8_t is_ingress , crdp_ing_sbpm_intr_ctrl_isr *intr_ctrl_isr)
{
    const char * sbpm_str[] = { "Egress", "Ingress"};

    if (intr_ctrl_isr->bac_underrun)
    {
        bdmf_session_print(session, "\n[Error]%s %d: buffer underrun\n", sbpm_str[is_ingress], sbpm_index);
    }
    if (intr_ctrl_isr->mcst_overflow)
    {
        bdmf_session_print(session, "\n[Error]%s %d: multicast overflow\n", sbpm_str[is_ingress], sbpm_index);
    }

}

bdmf_error_t drv_sbpm_cli_sanity_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t nParms)
{
    uint8_t sbpm_index;
    bdmf_error_t rc = BDMF_ERR_OK;

    for (sbpm_index = 0; sbpm_index < NUM_OF_SBPM_INGRESS; sbpm_index++)
    {
        crdp_ing_sbpm_intr_ctrl_isr intr_ctrl_isr = {};
        rc = rc ? rc : ag_drv_crdp_ing_sbpm_intr_ctrl_isr_get(sbpm_index, &intr_ctrl_isr);

        if (!rc)
            _drv_crdp_sbpm_parse_isr(session, sbpm_index, 1, &intr_ctrl_isr);
    }
    for (sbpm_index = 0; sbpm_index < NUM_OF_SBPM_EGRESS; sbpm_index++)
    {
        crdp_ds_cmn_sbpm_intr_ctrl_isr intr_ctrl_isr = {};
        rc = rc ? rc : ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_isr_get(sbpm_index, &intr_ctrl_isr);        

        if (!rc)
            _drv_crdp_sbpm_parse_isr(session, sbpm_index, 0, (crdp_ing_sbpm_intr_ctrl_isr*)&intr_ctrl_isr);
    }
    return rc;
}

int drv_sbpm_cli_sanity_check(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    return drv_sbpm_cli_sanity_get(session, parm, n_parms);
}

/** Print regs of specified (sbpm_id) entry
 * @param param[0] Input: sbpm_id 
 * @param param[1] Input: sbpm_type: ingress/egress 
 */
static bdmf_error_t _sbpm_debug_cli_sbpm_print_regs(bdmf_session_handle sess, const bdmfmon_cmd_parm_t parm[], uint16_t nParms)
{
    bdmf_error_t rc;
    uint8_t sbpm_id = parm[0].value.unumber;
    sbpm_type_e sbpm_type = parm[1].value.unumber;

    /* Thresholds */
    uint16_t gl_trsh_l, gl_trsh_h;

    /* Interrupt */
    crdp_ing_sbpm_intr_ctrl_isr intr_ctrl_isr_ing;
    crdp_ds_cmn_sbpm_intr_ctrl_isr intr_ctrl_isr_ds_cmn;
    uint32_t iem;

    /* Init offset */
    uint16_t init_base_addr;
    bdmf_boolean init_start;
    bdmf_boolean init_bsy;
    bdmf_boolean init_rdy;

    if (sbpm_type == SBPM_TYPE_INGRESS)
    {
        /* Thresholds */
        rc = ag_drv_crdp_ing_sbpm_regs_gl_trsh_get(sbpm_id, &gl_trsh_l, &gl_trsh_h);

        /* Interrupts */
        rc = rc ? rc : ag_drv_crdp_ing_sbpm_intr_ctrl_isr_get(sbpm_id, &intr_ctrl_isr_ing);
        rc = rc ? rc : ag_drv_crdp_ing_sbpm_intr_ctrl_ier_get(sbpm_id, &iem);

        /* Init free list */
        rc = rc ? rc : ag_drv_crdp_ing_sbpm_regs_init_free_list_get(sbpm_id, &init_base_addr, &init_start, &init_bsy, &init_rdy);
    }
    else
    {
        /* Thresholds */
        rc = ag_drv_crdp_ds_cmn_sbpm_regs_gl_trsh_get(sbpm_id, &gl_trsh_l, &gl_trsh_h);

        /* Interrupts */        
        rc = rc ? rc : ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_isr_get(sbpm_id, &intr_ctrl_isr_ds_cmn);
        rc = rc ? rc : ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_ier_get(sbpm_id, &iem);

        intr_ctrl_isr_ing.bac_underrun = intr_ctrl_isr_ds_cmn.bac_underrun;
        intr_ctrl_isr_ing.mcst_overflow = intr_ctrl_isr_ds_cmn.mcst_overflow;
        /* Init free list */
        rc = rc ? rc : ag_drv_crdp_ing_sbpm_regs_init_free_list_get(sbpm_id, &init_base_addr, &init_start, &init_bsy, &init_rdy);
    }

    bdmf_session_print(sess, "-------------------------------------------------------------------\n");
    bdmf_session_print(sess, "   print_regs: sbpm_id = %d, type %s\n", sbpm_id, (sbpm_type == SBPM_TYPE_INGRESS ? "INGRESS" : "EGRESS"));
    bdmf_session_print(sess, "-------------------------------------------------------------------\n");
    bdmf_session_print(sess, "Thresholds:\n");
    bdmf_session_print(sess, "  gl_trsh:            gl_bat = %-5d, gl_bah = %-5d\n", gl_trsh_l, gl_trsh_h);
    bdmf_session_print(sess, "Interrupts:\n");
    bdmf_session_print(sess, "  iem  = %d\n", iem);
    bdmf_session_print(sess, "  bac_underrun  = %d\n", intr_ctrl_isr_ing.bac_underrun);
    bdmf_session_print(sess, "  mcst_overflow  = %d\n", intr_ctrl_isr_ing.mcst_overflow);
    bdmf_session_print(sess, "Init Offset:\n");
    bdmf_session_print(sess, "  init_base_addr = %d\n", init_base_addr);
    bdmf_session_print(sess, "  init_bsy = %d\n", init_bsy);
    bdmf_session_print(sess, "  init_rdy = %d\n", init_rdy);
    bdmf_session_print(sess, "  init_start = %d\n", init_start);    
    bdmf_session_print(sess, "-------------------------------------------------------------------\n");
    return rc;
}

static bdmf_error_t drv_sbpm_alloc_single(sbpm_type_e sbpm_type, uint8_t sbpm_id, uint16_t *bn, uint16_t sbpm_invalid_num)
{
    bdmf_error_t rc;
    int num_of_iters;
    crdp_ing_sbpm_regs_bn_alloc_rply regs_bn_alloc_rply_ing;
    crdp_ds_cmn_sbpm_regs_bn_alloc_rply regs_bn_alloc_rply_egr;
    bdmf_boolean alloc_bn_valid;
    bdmf_boolean rdy;
    uint16_t sa = 0;

    *bn = sbpm_invalid_num;

    rc = (sbpm_type == SBPM_TYPE_INGRESS ? 
         ag_drv_crdp_ing_sbpm_regs_bn_alloc_set(sbpm_id, sa):
         ag_drv_crdp_ds_cmn_sbpm_regs_bn_alloc_set(sbpm_id, sa));

    BDMF_TRACE_RET_IF_COND(rc , rc, "drv_sbpm_alloc_single failed: sbpm id %d, err %d\n", sbpm_id, rc);

    for (num_of_iters = 0; num_of_iters < SBPM_MAX_NUM_OF_ITERS; num_of_iters++)
    {
        if (sbpm_type == SBPM_TYPE_INGRESS)
        {
            rc = ag_drv_crdp_ing_sbpm_regs_bn_alloc_rply_get(sbpm_id, &regs_bn_alloc_rply_ing);

            alloc_bn_valid = regs_bn_alloc_rply_ing.alloc_bn_valid;
            rdy = regs_bn_alloc_rply_ing.rdy;
        }
        else
        {
            rc = ag_drv_crdp_ds_cmn_sbpm_regs_bn_alloc_rply_get(sbpm_id, &regs_bn_alloc_rply_egr);

            alloc_bn_valid = regs_bn_alloc_rply_egr.alloc_bn_valid;
            rdy = regs_bn_alloc_rply_egr.rdy;
        }

        if (rdy && alloc_bn_valid)
            break;

        bdmf_usleep(10);
    }

    if (num_of_iters < SBPM_MAX_NUM_OF_ITERS)
    {
        *bn = (sbpm_type == SBPM_TYPE_INGRESS ?  
               regs_bn_alloc_rply_ing.alloc_bn : regs_bn_alloc_rply_egr.alloc_bn);
    }
    else
        rc = BDMF_ERR_INTERNAL;

    return rc;
}

static bdmf_error_t drv_sbpm_connect_single(sbpm_type_e sbpm_type, uint8_t sbpm_id, int16_t bn, uint16_t next_bn)
{
    bdmf_error_t rc;
    int num_of_iters;
    bdmf_boolean connect_ack, busy, rdy;

    rc = (sbpm_type == SBPM_TYPE_INGRESS ? 
         ag_drv_crdp_ing_sbpm_regs_bn_connect_set(sbpm_id, bn, BDMF_TRUE /*ack_req*/, next_bn) :
         ag_drv_crdp_ds_cmn_sbpm_regs_bn_connect_set(sbpm_id, bn, BDMF_TRUE /*ack_req*/, next_bn));
    BDMF_TRACE_RET_IF_COND(rc , rc, "drv_sbpm_connect_single failed: connect_set. sbpm id %d, reply get err %d\n", sbpm_id, rc);

    for (num_of_iters = 0; num_of_iters < SBPM_MAX_NUM_OF_ITERS; num_of_iters++)
    {
        rc = (sbpm_type == SBPM_TYPE_INGRESS ? 
            ag_drv_crdp_ing_sbpm_regs_bn_connect_rply_get(sbpm_id, &connect_ack, &busy, &rdy) :
            ag_drv_crdp_ds_cmn_sbpm_regs_bn_connect_rply_get(sbpm_id, &connect_ack, &busy, &rdy));

        BDMF_TRACE_RET_IF_COND(rc , rc, "drv_sbpm_connect_single failed: rply_get. sbpm id %d, reply get err %d\n", sbpm_id, rc);

        if (rdy && connect_ack)
            break;

        bdmf_usleep(10);
    }

    if (num_of_iters == SBPM_MAX_NUM_OF_ITERS)
        return BDMF_ERR_INTERNAL;

    return BDMF_ERR_OK;
}

static bdmf_error_t drv_sbpm_get_next_bn(sbpm_type_e sbpm_type, uint8_t sbpm_id, int16_t bn, uint16_t *next_bn, uint16_t sbpm_invalid_num)
{
    bdmf_error_t rc;
    int num_of_iters;
    bdmf_boolean reply_busy;
    bdmf_boolean reply_bn_valid;
    uint16_t reply_next_bn;
    bdmf_boolean reply_rdy;

    *next_bn = sbpm_invalid_num;

    rc = (sbpm_type == SBPM_TYPE_INGRESS ? 
         ag_drv_crdp_ing_sbpm_regs_get_next_set(sbpm_id, bn) :
         ag_drv_crdp_ds_cmn_sbpm_regs_get_next_set(sbpm_id, bn));

    BDMF_TRACE_RET_IF_COND(rc , rc, "drv_sbpm_get_next_bn failed: sbpm id %d, err %d\n", sbpm_id, rc);

    for (num_of_iters = 0; num_of_iters < SBPM_MAX_NUM_OF_ITERS; num_of_iters++)
    {
        if (sbpm_type == SBPM_TYPE_INGRESS)
        {
            ag_drv_crdp_ing_sbpm_regs_get_next_rply_get(sbpm_id, &reply_bn_valid, &reply_next_bn, &reply_busy, &reply_rdy);
        }
        else
        {
            ag_drv_crdp_ds_cmn_sbpm_regs_get_next_rply_get(sbpm_id, &reply_bn_valid, &reply_next_bn, &reply_busy, &reply_rdy);
        }

        BDMF_TRACE_RET_IF_COND(rc , rc, "drv_sbpm_get_next_bn failed: sbpm id %d, reply get err %d, \n", sbpm_id, rc);

        /* TODO - why !reply_busy , shouldnt be reply_busy?*/
        if ((!reply_busy) || (reply_rdy != 1))
            break;

        bdmf_usleep(10);
    }

    if (reply_bn_valid)
    {
        /* todo - need to check, i dont have it anymore ?*/
        /*if (!reply_bn_null)*/
        *next_bn = reply_next_bn;
    }

    return rc;
}

static void cli_print_list(bdmf_session_handle sess, uint16_t head_bn, uint32_t print_bns, uint16_t* p_bn_next, uint16_t* p_head_count, uint16_t* p_is_reported, uint16_t sbpm_invalid_num)
{
    uint16_t curr_bn;
    uint16_t i;
    uint16_t tmp_count;

    bdmf_session_print(sess, "\nHead BN: %d, num of BNs: %d\n", head_bn, p_head_count[head_bn]);
    
    for (curr_bn = head_bn, i = 0, tmp_count = 0;
         (curr_bn != sbpm_invalid_num) && (i < p_head_count[head_bn]); i++)
    {
        if (print_bns)
        {
            bdmf_session_print(sess, "%d ", curr_bn);
            tmp_count++;
        }

        p_is_reported[curr_bn] = 1;
        curr_bn = p_bn_next[curr_bn];
        
        if ((curr_bn != sbpm_invalid_num) && print_bns)
        {
            bdmf_session_print(sess, "=> ");

            if (tmp_count >= 20)
            {
                tmp_count = 0;
                bdmf_session_print(sess, "\n");
            }
        }
    }
    bdmf_session_print(sess, "\n");
}

/** Print regs of specified (sbpm_id) entry
 * @param param[0] Input: sbpm_id 
 * @param param[1] Input: sbpm_type: ingress/egress 
 */
static bdmf_error_t _sbpm_debug_cli_sbpm_scan_linked_list(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t nParms)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t sbpm_id = parm[0].value.number;
    sbpm_type_e sbpm_type = parm[1].value.number;
    uint32_t print_free = parm[2].value.number;
    uint32_t print_used = parm[3].value.number;
    uint32_t print_bns = parm[4].value.number;
    uint16_t curr_bn, bn;
    uint16_t sbpm_invalid_num = (sbpm_type == SBPM_TYPE_INGRESS ? SBPM_INVALID_BUFFER_NUMBER : (SBPM__EGR_MAX_NUM_OF_BNS-1));
    uint16_t sbpm_max_num_bns = (sbpm_type == SBPM_TYPE_INGRESS ? SBPM_ING_MAX_NUM_OF_BNS : SBPM__EGR_MAX_NUM_OF_BNS);
    bdmf_session_print(session, "-------------------------------------------------------------------------------------\n");
    bdmf_session_print(session, "   scan_linked_list: sbpm_id = %d, type %s, print_free %d, print_used %d, print_bns %d\n", sbpm_id, (sbpm_type == SBPM_TYPE_INGRESS ? "INGRESS" : "EGRESS"), print_free, print_used, print_bns);
    bdmf_session_print(session, "-------------------------------------------------------------------------------------\n");

    if (print_free)
    {
        uint16_t free_list_head_bn[SCAN_LINKED_LIST_MAX_NUM_OF_CHUNKS], new_bn;
        uint16_t free_list_last_bn[SCAN_LINKED_LIST_MAX_NUM_OF_CHUNKS];
        uint16_t free_list_count_bn[SCAN_LINKED_LIST_MAX_NUM_OF_CHUNKS];
        uint16_t free_cnt = 0;
        uint16_t free_list_index = 0;
        uint16_t free_list_tmp_cnt;
        int i;

        bdmf_session_print(session, "Print Free: Allocating free BNs\n");
        
        do
        {        
            /* Allocate head */
            rc = drv_sbpm_alloc_single(sbpm_type, sbpm_id, &free_list_head_bn[free_list_index], sbpm_invalid_num);
            curr_bn = free_list_head_bn[free_list_index];
            new_bn = curr_bn;
            free_list_tmp_cnt = 1;

            if (curr_bn != sbpm_invalid_num)
                free_cnt = free_cnt + 1;

            while ((new_bn != sbpm_invalid_num) && (rc == BDMF_ERR_OK) && (free_list_tmp_cnt < 127))
            {
                rc = drv_sbpm_alloc_single(sbpm_type, sbpm_id, &new_bn, sbpm_invalid_num);

                if ((new_bn != sbpm_invalid_num) && (rc == BDMF_ERR_OK))
                {
                    free_cnt = free_cnt + 1;
                    free_list_tmp_cnt = free_list_tmp_cnt + 1;

                    rc = drv_sbpm_connect_single(sbpm_type, sbpm_id, curr_bn, new_bn);

                    BDMF_TRACE_RET_IF_COND(rc , rc, "_sbpm_debug_cli_sbpm_scan_linked_list: connect failed: sbpm id %d, reply get err %d, \n", sbpm_id, rc);
                    curr_bn = new_bn;
                }
            }
            bdmf_session_print(session, "In progress: allocated free_cnt %d\n", free_cnt);

            if (curr_bn != sbpm_invalid_num)
                rc = drv_sbpm_connect_single(sbpm_type, sbpm_id, curr_bn, INVALID_SBPM_14BITS_MAX_VAL_);

            free_list_last_bn[free_list_index] = curr_bn;
            free_list_count_bn[free_list_index] = free_list_tmp_cnt;
            free_list_index = free_list_index + 1;

        } while ((new_bn != sbpm_invalid_num) && (rc == BDMF_ERR_OK));

        bdmf_session_print(session, "Total allocated free_cnt %d\n\n", free_cnt);

        /* Free */
        for (i = 0; i < free_list_index; i++)
        {
            if (print_bns)
                bdmf_session_print(session, "Allocated list %d: Head %d, last %d, count %d\n", i, free_list_head_bn[i], free_list_last_bn[i], free_list_count_bn[i]);

            if (free_list_head_bn[i] == free_list_last_bn[i])
                rc = drv_sbpm_free_list(sbpm_type, sbpm_id, free_list_head_bn[i], INVALID_SBPM_14BITS_MAX_VAL_, 1);
            else
                rc = drv_sbpm_free_list(sbpm_type, sbpm_id, free_list_head_bn[i], free_list_last_bn[i], free_list_count_bn[i]);    
        }

        /* Verification check */
        {
            uint16_t gl_trsh_l, tmp;

            if (sbpm_type == SBPM_TYPE_INGRESS)
            {
                rc = ag_drv_crdp_ing_sbpm_regs_gl_trsh_get(sbpm_id, &gl_trsh_l, &tmp);
            }
            else
            {
                rc = ag_drv_crdp_ds_cmn_sbpm_regs_gl_trsh_get(sbpm_id, &gl_trsh_l, &tmp);
            }

            bdmf_session_print(session, "\nVerification check:\n");
            bdmf_session_print(session, "  FAIL: gl_trsh_l %d \n\n", gl_trsh_l);
        }
    }

    /* Print used */
    if (print_used)
    {
        uint16_t count, next_bn;
        uint16_t* p_head_count;
        uint16_t* p_bn_next;
        uint16_t* p_is_reported;
        uint16_t* p_is_head;

        bdmf_session_print(session, "\nPrint used:\n");

        p_head_count = (uint16_t *)malloc(sbpm_max_num_bns * sizeof(uint16_t));
        p_is_reported = (uint16_t *)malloc(sbpm_max_num_bns * sizeof(uint16_t));
        p_bn_next = (uint16_t *)malloc(sbpm_max_num_bns * sizeof(uint16_t));
        p_is_head = (uint16_t *)malloc(sbpm_max_num_bns * sizeof(uint16_t));

        for (bn = 0; bn < sbpm_max_num_bns; bn++)
        {
            p_bn_next[bn] = sbpm_invalid_num;
            p_head_count[bn] = 1;
            p_is_reported[bn] = 0;
            p_is_head[bn] = 1;
        }

        /* Print lists */
        for (bn = 0; bn < sbpm_max_num_bns-1; bn++)
        {
            if (p_bn_next[bn] != sbpm_invalid_num) /* Already checked this BN */
                continue;

            for (count = 1, curr_bn = bn; ; count++)
            {
                if (count >= sbpm_max_num_bns)
                {
                    bdmf_session_print(session, "Error: Overrun when testing BN %d (part of %d). BUG?\n", curr_bn, bn);
                    p_head_count[bn] = 15;
                    cli_print_list(session, bn, print_bns, p_bn_next, p_head_count, p_is_reported, sbpm_invalid_num);
                    return BDMF_ERR_INTERNAL;
                }

                rc = drv_sbpm_get_next_bn(sbpm_type, sbpm_id, curr_bn, &next_bn, sbpm_invalid_num);

                p_bn_next[curr_bn] = next_bn;

                if (next_bn == sbpm_invalid_num)
                    break;

                if (p_head_count[next_bn] > 1) /* Already counted it, reuse */
                {
                    count += p_head_count[next_bn];
                    p_head_count[next_bn] = 1; /* Don't count it twice */
                    break;
                }
                curr_bn = next_bn;
            }
            p_head_count[bn] = count;
        }

        bdmf_session_print(session, "Lists:\n");
        bdmf_session_print(session, "======\n");
        for (bn = 0; bn < sbpm_max_num_bns-1; bn++)
        {
            if (p_head_count[bn] > 1)
                cli_print_list(session, bn, print_bns, p_bn_next, p_head_count, p_is_reported, sbpm_invalid_num);
        } 
        bdmf_session_print(session, "\n");

        for (bn = 0; bn < sbpm_max_num_bns-1; bn++)
        {
            if (!p_is_reported[bn])
                cli_print_list(session, bn, print_bns, p_bn_next, p_head_count, p_is_reported, sbpm_invalid_num);
        }
        bdmf_session_print(session, "\n");
#if 0  /* CRDP-TODO */
        if (p_bn_next)
            free(p_bn_next);
        if (p_head_count)
            free(p_head_count);
        if (p_is_reported)
            free(p_is_reported);
#endif
    }

    return rc;
}

/** Create SBPM directory under deBug. Add commands 
*/
void drv_sbpm_cli_init(bdmfmon_handle_t driver_dir)
{
    static bdmfmon_enum_val_t sbpm_type_table[] =
    {
        { .name = "ingress", SBPM_TYPE_INGRESS },
        { .name = "egress", SBPM_TYPE_EGRESS },
        BDMFMON_ENUM_LAST
    };

    debug_sbpm_cli_dir = bdmfmon_dir_add(driver_dir, "sbpm", "SBPM debug CLI commands", BDMF_ACCESS_ADMIN, NULL);
    BDMFMON_MAKE_CMD(debug_sbpm_cli_dir, "sanity", "SBPM interupts",
                drv_sbpm_cli_sanity_check,
                BDMFMON_PARM_LIST_TERMINATOR
                );
    
  
    BDMFMON_MAKE_CMD(debug_sbpm_cli_dir, "print_regs", "Print SBPM regs",
                _sbpm_debug_cli_sbpm_print_regs,
                BDMFMON_MAKE_PARM("id", "SBPM id", BDMFMON_PARM_UDECIMAL, 0),
                BDMFMON_MAKE_PARM_ENUM("type", "type", sbpm_type_table, 0)
                );

    BDMFMON_MAKE_CMD(debug_sbpm_cli_dir, "scan_linked_list", "Scan SBPM Linked List",
                _sbpm_debug_cli_sbpm_scan_linked_list,
                BDMFMON_MAKE_PARM("id", "SBPM id", BDMFMON_PARM_UDECIMAL, 0),
                BDMFMON_MAKE_PARM_ENUM("type", "type", sbpm_type_table, 0),
                BDMFMON_MAKE_PARM("print_free", "Print free list", BDMFMON_PARM_UDECIMAL, 0),
                BDMFMON_MAKE_PARM("print_used", "Print used lists", BDMFMON_PARM_UDECIMAL, 0),
                BDMFMON_MAKE_PARM("print_bns", "Print bns in used lists", BDMFMON_PARM_UDECIMAL, 0)
                );
}

#endif /* USE_BDMF_SHELL */

