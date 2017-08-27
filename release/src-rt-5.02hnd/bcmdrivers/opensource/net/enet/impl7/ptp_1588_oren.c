/*
 Copyright 2002-2010 Broadcom Corp. All Rights Reserved.

 <:label-BRCM:2011:DUAL/GPL:standard    
 
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

#include <linux/kthread.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include "rdpa_api.h"
#include "ptp_1588_oren.h"

#include "rdp_ms1588.h"
#include "time_sync.h"

static union time_of_day tx_sn_tod[SERIAL_NUMBER_HW_TABLE_LAST];  
static struct sourcePortIdentity tx_sn_portID[SERIAL_NUMBER_HW_TABLE_LAST];

static union time_of_day rx_sn_tod[SERIAL_NUMBER_HW_TABLE_SIZE];  
static struct sourcePortIdentity rx_sn_portID[SERIAL_NUMBER_HW_TABLE_SIZE];

static int sync_follow_counter = 0;
static int delay_req_res_counter = 0;
static struct sourcePortIdentity dummy_portID;

spinlock_t ptplock_tx;

LIST_HEAD(head_list);

static int tx_thread_cond = 0;
DECLARE_WAIT_QUEUE_HEAD(tx_thread_wq);

static int pkt_sent_cond = 0;
DECLARE_WAIT_QUEUE_HEAD(pkt_sent_wq);

struct task_struct *ptp_1588_kthread;


static void debug_print_data(void *pHead, uint32_t len)
{
    uint32_t i;
    uint8_t *c = (uint8_t *)pHead;

    for (i = 0; i < len; ++i) {
        if (i % 16 == 0)
            printk("\n");
        printk("0x%02X, ", *c++);
    }
    printk("\n");
}

int ptp_1588_rx_pkt_store_timestamp(unsigned char *pBuf, int len, uint16_t ptp_index)
{
    char *offset;
    struct ptphdr *ptp_header;
    int index, sn;
    rdpa_system_tod_t tod;
    struct vlan_ethhdr *eh;
    int tags_length = 0;

    eh = (struct vlan_ethhdr *) pBuf;

    if (eh->h_vlan_proto == htons(ETH_P_8021Q)) 
    {
        tags_length = VLAN_HLEN;
    }

    if ((eh->h_vlan_proto == htons(ETH_P_1588)) || 
        (eh->h_vlan_encapsulated_proto == htons(ETH_P_1588))) 
    {
        offset = pBuf + ETH_HLEN + tags_length;
    } 
    else  /* 
             the function is called only when reason == ...ptp_1588, 
             therefore no need in additional eth type checks
           */
    {
        struct iphdr *iph = (struct iphdr *)(pBuf + ETH_HLEN + tags_length);
        struct udphdr *uh = (struct udphdr *)(CAST_2_CHAR(iph) + iph->ihl*4);

        offset = (CAST_2_CHAR(uh) + sizeof(struct udphdr));
    }

    ptp_header = (struct ptphdr *)offset;
    /* only looking for those packets */
    if ((ptp_header->messageType != PTP_1588_DELAY_REQ) && (ptp_header->messageType != PTP_1588_PDELAY_REQ))
        return 0;

    for (index = SERIAL_NUMBER_HW_TABLE_SIZE, sn = delay_req_res_counter; index; index--, sn = INC_AND_REMAIN(sn))
    {
        if (!memcmp(&rx_sn_portID[sn], &dummy_portID, sizeof(struct sourcePortIdentity)))
        {
            delay_req_res_counter = sn;
            break;
        }
    }
    if (!index)
        printk(KERN_ERR "ERR: PTP 1588 module - no room in RX table. will override an existing entry");

    memcpy(&rx_sn_portID[delay_req_res_counter], &ptp_header->portID, sizeof(struct sourcePortIdentity));


    time_sync_get_t4(&tod, ptp_index);
    rx_sn_tod[delay_req_res_counter].sec = tod.sec_ls;
    rx_sn_tod[delay_req_res_counter].nanosec = tod.nsec;
    delay_req_res_counter = INC_AND_REMAIN(delay_req_res_counter);

    return 0;
}

static FN_HANDLER_RT sync_interrupt_handler(int irq_num, void *ptp_info)
{
    int sn_hw_counter;
    PTP_POP_CMD(sn_hw_counter);

    pkt_sent_cond = 1;
    wake_up_interruptible(&pkt_sent_wq);

#ifdef CONFIG_BCM96838
    BcmHalInterruptEnable(INTERRUPT_ID_RDP_MS1588);
#endif
    return BDMF_IRQ_HANDLED; 
}

static int update_followup_timestamp(struct ptphdr *ptp_header, int ptp_over_eth)
{
    int index, sn;

    for (index = SERIAL_NUMBER_HW_TABLE_SIZE, sn = DEC_AND_REMAIN(sync_follow_counter);
        index; index--, sn = DEC_AND_REMAIN(sn))
    {
        if (!memcmp(&ptp_header->portID, &tx_sn_portID[sn], sizeof(struct sourcePortIdentity)))
        {
            UPDATE_UDP_CHECKSUM(ptp_header, ptp_over_eth);
            ptp_header->follow_up.preciseOriginTimestamp.seconds_lsb = tx_sn_tod[sn].sec;
            ptp_header->follow_up.preciseOriginTimestamp.nanoseconds = tx_sn_tod[sn].nanosec;
	    cache_flush_len(&ptp_header->follow_up.preciseOriginTimestamp, sizeof(ptp_header->follow_up.preciseOriginTimestamp));
            memset(&tx_sn_portID[sn], 0, sizeof(struct sourcePortIdentity));

            return 0;
        }
    }

    printk(KERN_ERR "ERR: PTP 1588 module - no match between sync and follow-up");
    debug_print_data(&(ptp_header->portID), sizeof(struct sourcePortIdentity));

    return BDMF_ERR_NOENT;
}

static int update_delay_res_timestamp(struct ptphdr *ptp_header, int ptp_over_eth)
{
    int index, sn;

    for (index = SERIAL_NUMBER_HW_TABLE_SIZE, sn = DEC_AND_REMAIN(delay_req_res_counter);
        index; index--, sn = DEC_AND_REMAIN(sn))
    {
        if (!memcmp(&ptp_header->delay_resp.requestingPortIdentity, &rx_sn_portID[sn], sizeof(struct sourcePortIdentity)))
        {
            UPDATE_UDP_CHECKSUM(ptp_header, ptp_over_eth);            
            ptp_header->delay_resp.receiveTimestamp.seconds_lsb = rx_sn_tod[sn].sec;
            ptp_header->delay_resp.receiveTimestamp.nanoseconds = rx_sn_tod[sn].nanosec;
	    cache_flush_len(&ptp_header->delay_resp.receiveTimestamp, sizeof(ptp_header->delay_resp.receiveTimestamp));            
            memset(&rx_sn_portID[sn], 0, sizeof(struct sourcePortIdentity));

            return 0;
        }
    }

    printk(KERN_ERR "ERR: PTP 1588 module - no match between delay request and delay respond");
    debug_print_data(&(ptp_header->delay_resp.requestingPortIdentity), sizeof(struct sourcePortIdentity));

    return BDMF_ERR_NOENT;
}

static int ptp_1588_tx_thread(void *arg)
{
    int rc;
    struct ptp_1588_list *ptp_list_entry, *temp;
    struct ptphdr *ptp_header;

    MS1588_MASTER_REGS_M_CFG config_port;

    while (1)
    {
        wait_event_interruptible(tx_thread_wq, tx_thread_cond);

        if (kthread_should_stop())
        {
            printk(KERN_INFO "ptp_1588_tx_thread should stop\n");
            break;
        }

        list_for_each_entry_safe(ptp_list_entry, temp, &head_list, list)
        {
            spin_lock_bh(&ptplock_tx);            
            list_del(&ptp_list_entry->list);
            spin_unlock_bh(&ptplock_tx);
            ptp_header = (struct ptphdr *)ptp_list_entry->ptp_header;

            if (ptp_header->messageType == PTP_1588_SYNC)
            {
                int index, sn;
                rdpa_system_tod_t tod;

                for (index = SERIAL_NUMBER_HW_TABLE_SIZE, sn = sync_follow_counter;
                    index; index--, sn = INC_AND_REMAIN(sn))
                {
                    if (!memcmp(&tx_sn_portID[sn], &dummy_portID, sizeof(struct sourcePortIdentity)))
                    {
                        sync_follow_counter = sn;
                        break;
                    }
                }
                if (!index)
                    printk(KERN_ERR "ERR: PTP 1588 module - no room in TX table. will override an existing entry");

                config_port.txstrbsel = ptp_list_entry->info.port - rdpa_if_lan0 + MASTER_PORT_EMAC0;

                config_port.tsfifoth = 1;
                MS1588_MASTER_REGS_M_CFG_WRITE(config_port);

                rc = rdpa_cpu_send_sysb_ptp(ptp_list_entry->sysb, &ptp_list_entry->info);
                if (rc)
                {
                    printk(KERN_ERR "ERR: PTP 1588 module - error from driver = %d\n", rc);
                    goto unlock_exit;
                }
                wait_event_interruptible(pkt_sent_wq, pkt_sent_cond);
                pkt_sent_cond = 0;
                memcpy (&tx_sn_portID[sync_follow_counter], &ptp_header->portID, sizeof(struct sourcePortIdentity));

                time_sync_get_t1(&tod);
                tx_sn_tod[sync_follow_counter].sec = tod.sec_ls;
                tx_sn_tod[sync_follow_counter].nanosec = tod.nsec;

                sync_follow_counter = INC_AND_REMAIN(sync_follow_counter);
            }
            else if (ptp_header->messageType == PTP_1588_FOLLOW_UP)
            {
                update_followup_timestamp(ptp_header, IS_ENC_TYPE_L2(ptp_header, bdmf_sysb_data(ptp_list_entry->sysb)));
                rc = rdpa_cpu_send_sysb(ptp_list_entry->sysb, &ptp_list_entry->info);
                if (rc)
                    printk(KERN_ERR "ERR: PTP 1588 module - error sending packet (%d)\n",rc);
            }
            else
            {
                rc = rdpa_cpu_send_sysb(ptp_list_entry->sysb, &ptp_list_entry->info);  
                if (rc)
                    printk(KERN_ERR "ERR: PTP 1588 module - error sending packet (%d)\n",rc);
            }
unlock_exit:
            kfree(ptp_list_entry);
        }
        tx_thread_cond = 0; 
        
    }
    return 0;
} 

int ptp_1588_cpu_send_sysb(bdmf_sysb sysb, rdpa_cpu_tx_info_t *info, char *ptp_header)
{
    struct ptp_1588_list *list_entry;
    int rc;

    if (((struct ptphdr *)ptp_header)->messageType == PTP_1588_DELAY_RESP)
    {
        update_delay_res_timestamp((struct ptphdr *)ptp_header, IS_ENC_TYPE_L2(ptp_header, bdmf_sysb_data(sysb)));
        rc = rdpa_cpu_send_sysb(sysb, info);    
        if (rc)
            printk(KERN_ERR "ERR: PTP 1588 module - error sending packet (%d)\n",rc);
        return rc;
    }

    list_entry = kmalloc(sizeof(struct ptp_1588_list), GFP_ATOMIC);

    if (!list_entry)
    {
        printk(KERN_ERR "ERR: PTP 1588 module - no available memory for new entry list\n");
        return BDMF_ERR_NOMEM;
    }

    memset(list_entry, 0, sizeof(struct ptp_1588_list));
    INIT_LIST_HEAD(&list_entry->list);

    list_entry->sysb = sysb;

    memcpy(&list_entry->info, info, sizeof(rdpa_cpu_tx_info_t));
    list_entry->ptp_header = ptp_header;

    spin_lock_bh(&ptplock_tx);
    list_add_tail(&list_entry->list, &head_list);
    spin_unlock_bh(&ptplock_tx);
    tx_thread_cond = 1;
    wake_up_interruptible(&tx_thread_wq);

    return 0;
}

int is_pkt_ptp_1588(pNBuff_t pNBuff, char **ptp_offset)
{
    int is_ptp = 0;
    struct vlan_ethhdr *eh;
    int tags_length = 0;

    if (IS_SKBUFF_PTR(pNBuff))  
    {
        eh = (struct vlan_ethhdr *)PNBUFF_2_SKBUFF(pNBuff)->data;  
    }
    else
    {
        eh = (struct vlan_ethhdr *)PNBUFF_2_FKBUFF(pNBuff)->data;
    }
    
    if (eh->h_vlan_proto == htons(ETH_P_8021Q)) 
    {
        tags_length = VLAN_HLEN;
    }

    if ((eh->h_vlan_proto == htons(ETH_P_IP)) || 
        (eh->h_vlan_encapsulated_proto == htons(ETH_P_IP)))
    {
        struct iphdr *iph = (struct iphdr *)(CAST_2_CHAR(eh) + ETH_HLEN + tags_length);

        if (iph->protocol == IPPROTO_UDP)
        {
            struct udphdr *uh = (struct udphdr *)(CAST_2_CHAR(iph) + iph->ihl*4);

            if (((uh->source == PTP_1588_EVENT_PORT) && (uh->dest == PTP_1588_EVENT_PORT)) ||
                ((uh->source == PTP_1588_GENERAL_PORT) && (uh->dest == PTP_1588_GENERAL_PORT)))
            {
                *ptp_offset = CAST_2_CHAR(uh) + sizeof(struct udphdr);
                is_ptp = 1;
            }
        }
    }
    else if ((eh->h_vlan_proto == htons(ETH_P_1588)) || 
             (eh->h_vlan_encapsulated_proto == htons(ETH_P_1588)))
    {
        *ptp_offset = CAST_2_CHAR(eh) + ETH_HLEN + tags_length;
        is_ptp = 1;
    }

    return is_ptp;
}

void ptp_1588_uninit(void)
{
    kthread_stop(ptp_1588_kthread);
}

int ptp_1588_init(void)
{
    ptp_1588_kthread = kthread_run(ptp_1588_tx_thread, NULL, "ptp_1588_tx_thread");
    if (IS_ERR(ptp_1588_kthread)) 
    {
        printk(KERN_ERR "ERR: PTP 1588 module - could not create ptp thread\n");
        return BDMF_ERR_NOMEM;
    }

    time_sync_register_ptp_1588_isr_callback(sync_interrupt_handler);

    return 0;
}

