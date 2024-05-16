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

#ifndef __SPDSVC_TR471_H_INCLUDED__
#define __SPDSVC_TR471_H_INCLUDED__

enum _tr471_flow_state_t {
    TR471_FLOW_STATE_INACTIVE,
    TR471_FLOW_STATE_ACTIVE_SW,
    TR471_FLOW_STATE_ACTIVE_HW
};

typedef enum  {
    TR471_THREAD_MODE_INVALID,
    TR471_THREAD_MODE_SW_GEN,
    TR471_THREAD_MODE_HW_DONE
}_tr471_thread_mode_t;

typedef struct {
    void *fkb_p;                /* fkb */
    unsigned char   *data;      /* data pointer */
    unsigned int    len;        /* packet length */
    int             l3_offset;  /* L3 header offset from data */
    int             l4_offset;  /* L4 header offset from data */
    int             lHdr_offset;/* Load Hdr offset from data */
} spdsvc_tr471_ref_pkt_info_t;

typedef struct {
    int installed;
    int is_hw_tx;
    void *netdev_p;
    spdsvc_tr471_ref_pkt_info_t ref_pkt_info;
    int is_v6;
    enum _tr471_flow_state_t flow_state;
    uint32_t tx_count;
    tr471_rx_queue_write_t rx_queue_write;
    spdsvc_tuple_t tr471_tx_tuple; /* Used only to compare */
} spdsvc_tr471_t;

enum tr471_NF_pkt_result_t {
    TR471_NF_PKT_PASS, /* NF_ACCEPT */
    TR471_NF_PKT_DROP, /* NF_STOLEN + Free pkt */
    TR471_NF_PKT_FWDED,/* NF_STOLEN + Pkt Forwarded */
    TR471_NF_PKT_LHDR  /* NF_STOLEN + Pass lHdr + Free Pkt */
};
extern spdsvc_tr471_t _spdsvc_tr471_state_g;

int tr471_send_tx_pkts(spdsvc_t *spdsvc_p);
void spdsvc_fc_blog_flowevent(unsigned long event, void *info);
int spdsvc_tr471_store_ref_pkt_info(void *skb, spdsvc_tr471_ref_pkt_info_t *ref_info_p);
void spdsvc_tr471_reset_state_machine(void);
int  spdsvc_tr471_get_result(spdsvc_result_t *result_p);
void spdsvc_tr471_generator_setup(void);
int spdsvc_tr471_analyzer_enable(spdsvc_socket_t *socket_p, spdsvc_config_tr471_t *tr471_p);
int spdsvc_tr471_analyzer_disable(void);
int spdsvc_tr471_start_generator(void);
void spdsvc_tr471_generator_burst_cmpl(void);
void spdsvc_tr471_generator_disable(void);
int spdsvc_tr471_update_ref_pkt_lHdr(struct loadHdr *lHdr_p);

/* TODO - temporary */
void spdsvc_tr471_gen_start(void);
unsigned int spdsvc_tr471_process_out_udp(void *skb, void *payload_p, Blog_t **blog_pp, int is_v6);
unsigned int spdsvc_tr471_process_in_udp(void *skb, void *payload_p, void **skb_dev_pp, Blog_t **blog_pp, int is_v6);

#define tr471_record_pktgen_time(start, cmpl) tr471_update_pktgen_time(spdsvc_g.tr471.firstBurst, spdsvc_g.tr471.burstsize, start, cmpl)
#define tr471_record_tsk_wkup_time(start, cmpl) tr471_update_task_wkup_time(spdsvc_g.tr471.firstBurst, spdsvc_g.tr471.burstsize, start, cmpl)
#define tr471_record_total_burst_time(start, cmpl) tr471_update_total_time(spdsvc_g.tr471.firstBurst, spdsvc_g.tr471.burstsize, start, cmpl)
#define tr471_record_burst_gen_set_time(start, cmpl) tr471_update_burst_gen_set_time(spdsvc_g.tr471.firstBurst, spdsvc_g.tr471.burstsize, start, cmpl)

#endif // __SPDSVC_FC_H_INCLUDED__
