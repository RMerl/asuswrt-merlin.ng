/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
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

#ifndef __SPDSVC_PROCFS_H_INCLUDED__
#define __SPDSVC_PROCFS_H_INCLUDED__

#if defined(TR471_TIME_DATA_DEBUG)
/* Data collected for >800us, >700us ... >100us, >0us */
#define NUM_TR471_TIME_RECORDS   (10)
typedef struct {
    uint32_t burst_size;
    uint32_t time_usec;
    uint32_t first_burst_time_usec;
    uint32_t cnts[NUM_TR471_TIME_RECORDS];
}tr471_time_record_t;

extern uint64_t tr471_burst_enable_time_ns;     
extern uint64_t tr471_burst_gen_start_time_ns; 
extern uint64_t tr471_burst_gen_cmpl_time_ns;  
extern uint64_t tr471_task_wkup_time_ns;       
extern uint64_t tr471_burst_cmpl_time_ns;      
extern uint64_t tr471_burst_gen_tx_start_set_time_ns;
extern uint64_t tr471_burst_gen_tx_cmpl_set_time_ns;

void tr471_update_pktgen_time(int first_burst, uint32_t burst_size, uint64_t start_ns, uint64_t cmpl_ns);
void tr471_update_task_wkup_time(int first_burst, uint32_t burst_size, uint64_t start_ns, uint64_t cmpl_ns);
void tr471_update_total_time(int first_burst, uint32_t burst_size, uint64_t start_ns, uint64_t cmpl_ns);
void tr471_update_burst_gen_set_time(int first_burst, uint32_t burst_size, uint64_t start_ns, uint64_t cmpl_ns);

#else
#define tr471_update_pktgen_time(first_burst, burst_size, start_ns, cmpl_ns)
#define tr471_update_task_wkup_time(first_burst, burst_size, start_ns, cmpl_ns)
#define tr471_update_total_time(first_burst, burst_size, start_ns, cmpl_ns)
#define tr471_update_burst_gen_set_time(first_burst,burst_size,start_ns,cmpl_ns);
#endif

typedef struct {
    uint32_t reset_cnt;                 /* Times TR471 reset state m/c called */
    uint32_t pkt_store_cnt;             /* Times reference packet stored */
    uint32_t pkt_free_cnt;              /* Times reference packet freed */
    uint32_t error_cnt;                 /* Times error/unexpected event occured */
    uint32_t netdev_rx_cnt;             /* Times netdev_receive on spdsvc device called */
    uint32_t soft_gen_cnt;              /* Times software based generator started */
    uint32_t tx_via_fc_cnt;             /* Times packet got transmitted via FC flow without starting generator */
    uint32_t hw_gen_cnt;                /* Times hardware based generator started */
    uint32_t hw_gen_cmpl_cnt;           /* Times hardware generator completed */
    uint32_t hw_gen_chk_status;         /* Times hardware generator status checked for completion */
    uint32_t nf_out_hk_cnt;             /* Times NF-LOCAL-OUT hook called */
    uint32_t nf_out_hk_reg_cnt;         /* Times NF-LOCAL-OUT hook registered */
    uint32_t nf_out_hk_unreg_cnt;       /* Times NF-LOCAL-OUT hook unregistered */
    uint32_t nf_in_hk_cnt;              /* Times NF-LOCAL-IN hook called */
    uint32_t nf_in_hk_reg_cnt;          /* Times NF-LOCAL-IN hook registered */
    uint32_t nf_in_hk_unreg_cnt;        /* Times NF-LOCAL-IN hook unregistered */
    uint32_t blog_event_cnt;            /* Times Blog flow event received */
    uint32_t blog_event_fc_act_cnt;     /* Times Blog flow event for SW-flow activation */
    uint32_t blog_event_hw_act_cnt;     /* Times Blog flow event for HW-flow activation */
    uint32_t blog_event_fc_deact_cnt;   /* Times Blog flow event for SW-flow deactivation */
    uint32_t blog_event_hw_deact_cnt;   /* Times Blog flow event for HW-flow deactivation */
    uint32_t non_tr471_pkt_cnt;         /* Times non-TR471 packet received from NF-LOCAL-IN/OUT hook */
    uint32_t pkt_match_unfrag;          /* Times TR471 unfragmented packet received from NF-LOCAL-IN/OUT hook */
    uint32_t pkt_match_frag_first;      /* Times TR471 first fragment packet received from NF-LOCAL-IN/OUT hook */
    uint32_t pkt_match_frag_not_first;  /* Times TR471 non-first fragment packet received from NF-LOCAL-IN/OUT hook */
    uint32_t fwd_lhdr;                  /* Times Load Hdr sent to application */
    uint32_t nf_invalid_state;          /* Times NF-LOCAL-IN/OUT hook called in invalid state */
    uint32_t nf_invalid_mode;           /* Times NF-LOCAL-IN/OUT hook called in invalid mode */
    uint32_t nf_invalid_socket;         /* Times NF-LOCAL-IN/OUT hook called in invalid socket */
} spdsvc_tr471_procfs_data_t;

typedef struct {
    uint32_t spdsvc_state;              /* Current SPDSVC State */
    uint32_t ioctl_enable_cnt;          /* Times Enable IOCTL received */
    uint32_t ioctl_disable_cnt;         /* Times Disable IOCTL received */
    uint32_t ioctl_result_cnt;          /* Times Reslt IOCTL received */
    uint32_t ioctl_ovrhd_cnt;           /* Times Overhead IOCTL received */
    uint32_t buf_alloc;                 /* Buffer Alloc count*/
    uint32_t buf_free;                  /* Buffer Free count */
    spdsvc_tr471_procfs_data_t tr471_cnts;
} spdsvc_procfs_data_t;

extern spdsvc_procfs_data_t spdsvc_procfs_data_g;

#define SPDSVC_PROCFS_INC(f)    spdsvc_procfs_data_g.f++
#define TR471_PROCFS_INC(f)     spdsvc_procfs_data_g.tr471_cnts.f++

void spdsvc_procfs_handle_enable(void);
int spdsvc_procfs_construct(void);

#endif // __SPDSVC_PROCFS_H_INCLUDED__
