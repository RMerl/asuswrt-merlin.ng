/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
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

#include "rdd.h"
#include "rdd_defs.h"
extern rdd_wan_tx_pointers_table_t *wan_tx_pointers_table;
extern uint32_t g_rate_cntrls_pool_idx;
extern rdd_wan_physical_port_t g_wan_physical_port;
extern uint8_t g_broadcom_switch_mode;
extern rdd_bridge_port_t g_broadcom_switch_physical_port;
extern bdmf_fastlock int_lock_irq;
extern bdmf_fastlock cpu_message_lock;

#if !defined(RDD_BASIC) && (defined(DSL_63138) || defined(DSL_63148) || defined(WL4908))
static uint16_t ds_free_packet_descriptors_pool_guaranteed_pool_size = DS_FREE_PACKET_DESCRIPTOR_POOL_MIN_GUARANTEED_POOL_SIZE;
static uint16_t ds_free_packet_descriptors_pool_non_guaranteed_pool_size = RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_SIZE - DS_FREE_PACKET_DESCRIPTOR_POOL_MIN_GUARANTEED_POOL_SIZE;

static int rdd_tm_ds_free_packet_descriptors_pool_size_update (void);
#endif

#ifdef DS_SRAM_TX_QUEUES
void rdd_ds_free_packet_descriptors_pool_init(void)
{
    RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_DTS *ds_free_packet_descriptors_pool;
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS *free_packet_descriptors_pool_descriptor;
    RDD_PACKET_DESCRIPTOR_DTS *packet_descriptor;
    uint32_t next_packet_descriptor_address;
    uint32_t i;

    ds_free_packet_descriptors_pool = RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_PTR();

    /* create the free packet descriptors pool as a list of packet descriptors */
    for (i = 0; i < RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_SIZE; i++)
    {
        packet_descriptor = &(ds_free_packet_descriptors_pool->entry[i].packet_descriptor);

        /* the last packet descriptor should point to NULL, the others points to the next packet descriptor */
        if (i == (RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_SIZE - 1))
        {
            next_packet_descriptor_address = 0;
        }
        else
        {
            next_packet_descriptor_address =
                DS_FREE_PACKET_DESCRIPTORS_POOL_ADDRESS + ((i + 1) * sizeof(RDD_PACKET_DESCRIPTOR_DTS));
        }

        RDD_PACKET_DESCRIPTOR_NEXT_PACKET_DESCRIPTOR_POINTER_WRITE(next_packet_descriptor_address, packet_descriptor);
    }

    free_packet_descriptors_pool_descriptor =
       (RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) +
            FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ADDRESS);

    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_HEAD_POINTER_WRITE(DS_FREE_PACKET_DESCRIPTORS_POOL_ADDRESS,
        free_packet_descriptors_pool_descriptor);

    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_TAIL_POINTER_WRITE(DS_FREE_PACKET_DESCRIPTORS_POOL_ADDRESS +
        (RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_SIZE - 1) * sizeof(RDD_PACKET_DESCRIPTOR_DTS),
            free_packet_descriptors_pool_descriptor);

#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908)
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_INGRESS_COUNTER_WRITE(
        RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_SIZE - 1, free_packet_descriptors_pool_descriptor);
#endif /*DSL*/
}
#endif

#ifdef US_SRAM_TX_QUEUES
void rdd_us_free_packet_descriptors_pool_init(void)
{
    RDD_US_FREE_PACKET_DESCRIPTORS_POOL_DTS *us_free_packet_descriptors_pool;
    RDD_PACKET_DESCRIPTOR_DTS *packet_descriptor;
    uint32_t next_packet_descriptor_address;
    uint32_t i;

    us_free_packet_descriptors_pool = RDD_US_FREE_PACKET_DESCRIPTORS_POOL_PTR();

    /* create the free packet descriptors pool as a stack of packet descriptors */
    for (i = 0; i < RDD_US_FREE_PACKET_DESCRIPTORS_POOL_SIZE; i++)
    {
        packet_descriptor = &(us_free_packet_descriptors_pool->entry[i].packet_descriptor);

        /* the last packet descriptor should point to NULL, the others points to the next packet descriptor */
        if (i == (RDD_US_FREE_PACKET_DESCRIPTORS_POOL_SIZE - 1))
        {
            next_packet_descriptor_address = 0;
        }
        else
        {
            next_packet_descriptor_address =
                US_FREE_PACKET_DESCRIPTORS_POOL_ADDRESS + ((i + 1) * sizeof(RDD_PACKET_DESCRIPTOR_DTS));
        }

        RDD_PACKET_DESCRIPTOR_NEXT_PACKET_DESCRIPTOR_POINTER_WRITE(next_packet_descriptor_address, packet_descriptor);
    }
}
#endif

int rdd_wan_tx_init(void)
{
    RDD_WAN_CHANNELS_0_7_TABLE_DTS *wan_channels_0_7_table;
    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_DTS *wan_channel_0_7_descriptor;
#if !defined(WL4908)
    RDD_WAN_CHANNELS_8_39_TABLE_DTS *wan_channels_8_39_table;
    RDD_WAN_CHANNEL_8_39_DESCRIPTOR_DTS *wan_channel_8_39_descriptor;
#endif
#if defined(WL4908)
    RDD_RATE_CONTROLLER_EXPONENT_TABLE_DTS  *exponent_table_ptr;
    RDD_RATE_CONTROLLER_EXPONENT_ENTRY_DTS  *exponent_entry_ptr;
#endif
    RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS *dummy_us_rate_cntrl_descriptor;
    RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS *dummy_wan_tx_queue_descriptor;
    rdd_wan_channel_id_t wan_channel;
    uint32_t rate_cntrl;
    uint32_t tx_queue;
    uint32_t *bbh_tx_wan_channel_index;

    /* initialize WAN TX pointers table */
    wan_tx_pointers_table = (rdd_wan_tx_pointers_table_t *)bdmf_alloc(sizeof(rdd_wan_tx_pointers_table_t));

    if (wan_tx_pointers_table == NULL)
        return BDMF_ERR_NOMEM;

    memset(wan_tx_pointers_table, 0, sizeof(rdd_wan_tx_pointers_table_t));

    /* reset the dummy segmentation descriptors threshold to zero in order to drop packets */
    dummy_wan_tx_queue_descriptor = (RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_1_OFFSET) +
        DUMMY_WAN_TX_QUEUE_DESCRIPTOR_ADDRESS - sizeof(RUNNER_COMMON));

    RDD_WAN_TX_QUEUE_DESCRIPTOR_PACKET_THRESHOLD_WRITE(0, dummy_wan_tx_queue_descriptor);
    RDD_WAN_TX_QUEUE_DESCRIPTOR_PROFILE_PTR_WRITE(0, dummy_wan_tx_queue_descriptor);

    /* all the queues of the dummy rate controller will point to the dummy queue */
    dummy_us_rate_cntrl_descriptor =
       (RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_1_OFFSET) +
       DUMMY_US_RATE_CONTROLLER_DESCRIPTOR_ADDRESS - sizeof(RUNNER_COMMON));

    for (tx_queue = 0; tx_queue < RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_NUMBER; tx_queue++)
    {
        RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_WRITE(DUMMY_WAN_TX_QUEUE_DESCRIPTOR_ADDRESS,
            dummy_us_rate_cntrl_descriptor, tx_queue);
    }

    /* connect all the tconts to the dummy rate rate controller */
    wan_channels_0_7_table = RDD_WAN_CHANNELS_0_7_TABLE_PTR();

    for (wan_channel = RDD_WAN_CHANNEL_0; wan_channel <= RDD_WAN_CHANNEL_0_7_MAX; wan_channel++)
    {
        wan_channel_0_7_descriptor = &(wan_channels_0_7_table->entry[wan_channel]);

        for (rate_cntrl = RDD_RATE_CNTRL_0; rate_cntrl <= RDD_RATE_CNTRL_31; rate_cntrl++)
        {
            RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLER_ADDR_WRITE(DUMMY_US_RATE_CONTROLLER_DESCRIPTOR_ADDRESS,
                wan_channel_0_7_descriptor, rate_cntrl);
        }
    }

#if !defined(WL4908)
    wan_channels_8_39_table = RDD_WAN_CHANNELS_8_39_TABLE_PTR();

    for (wan_channel = RDD_WAN_CHANNEL_8; wan_channel <= RDD_WAN_CHANNEL_39; wan_channel++)
    {
        wan_channel_8_39_descriptor = &(wan_channels_8_39_table->entry[wan_channel - RDD_WAN_CHANNEL_8]);

        for (rate_cntrl = RDD_RATE_CNTRL_0; rate_cntrl <= RDD_RATE_CNTRL_3; rate_cntrl++)
        {
            RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLER_ADDR_WRITE(DUMMY_US_RATE_CONTROLLER_DESCRIPTOR_ADDRESS,
                wan_channel_8_39_descriptor, rate_cntrl);
        }
    }
#endif
    bbh_tx_wan_channel_index = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) +
        BBH_TX_WAN_CHANNEL_INDEX_ADDRESS);

    MWRITE_32(bbh_tx_wan_channel_index, RDD_WAN_CHANNEL_0);
    g_rate_cntrls_pool_idx = 0;

#if defined(WL4908)
    /* initialize exponents table */
    exponent_table_ptr = (RDD_RATE_CONTROLLER_EXPONENT_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_RATE_CONTROLLER_EXPONENT_TABLE_ADDRESS);

    exponent_entry_ptr = &(exponent_table_ptr->entry[0]);
    RDD_RATE_CONTROLLER_EXPONENT_ENTRY_EXPONENT_WRITE ( RDD_US_RATE_CONTROL_EXPONENT0, exponent_entry_ptr);

    exponent_entry_ptr = &(exponent_table_ptr->entry[1] );
    RDD_RATE_CONTROLLER_EXPONENT_ENTRY_EXPONENT_WRITE ( RDD_US_RATE_CONTROL_EXPONENT1, exponent_entry_ptr);

    exponent_entry_ptr = &(exponent_table_ptr->entry[2]);
    RDD_RATE_CONTROLLER_EXPONENT_ENTRY_EXPONENT_WRITE(RDD_US_RATE_CONTROL_EXPONENT2, exponent_entry_ptr);
#endif

    return BDMF_ERR_OK;
}

#ifndef G9991
void rdd_eth_tx_init(void)
{
    RDD_ETH_TX_MAC_TABLE_DTS *eth_tx_mac_table;
    RDD_ETH_TX_MAC_DESCRIPTOR_DTS *eth_tx_mac_descriptor;
    RDD_ETH_TX_QUEUES_TABLE_DTS *eth_tx_queues_table;
    RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS *eth_tx_queue_descriptor;
    RDD_ETH_TX_QUEUES_POINTERS_TABLE_DTS *eth_tx_queues_pointers_table;
    RDD_ETH_TX_QUEUE_POINTERS_ENTRY_DTS *eth_tx_queue_pointers_entry;
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS *free_packet_descriptors_pool_descriptor;
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    RDD_ETH_TX_LOCAL_REGISTERS_DTS *eth_tx_local_registers;
    RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_DTS *eth_tx_local_registers_entry;
#endif
    uint16_t eth_tx_queue_address;
    uint16_t mac_descriptor_address;
    uint32_t emac;
    uint32_t tx_queue;

    eth_tx_mac_table = RDD_ETH_TX_MAC_TABLE_PTR();

    eth_tx_queues_table = RDD_ETH_TX_QUEUES_TABLE_PTR();

    eth_tx_queues_pointers_table = RDD_ETH_TX_QUEUES_POINTERS_TABLE_PTR();

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    eth_tx_local_registers = RDD_ETH_TX_LOCAL_REGISTERS_PTR();
#endif

    for (emac = RDD_LAN0_VPORT; emac <= RDD_LAN_VPORT_LAST; emac++)
    {
        eth_tx_mac_descriptor = &(eth_tx_mac_table->entry[emac]);

#ifdef OREN
        RDD_ETH_TX_MAC_DESCRIPTOR_TX_TASK_NUMBER_WRITE((LAN0_TX_THREAD_NUMBER + (emac - RDD_LAN0_VPORT)),
            eth_tx_mac_descriptor);
        RDD_ETH_TX_MAC_DESCRIPTOR_PACKET_COUNTERS_PTR_WRITE(ETH_TX_MAC_TABLE_ADDRESS +
            emac * sizeof(RDD_ETH_TX_MAC_DESCRIPTOR_DTS) + RDD_EMAC_DESCRIPTOR_EGRESS_COUNTER_OFFSET,
            eth_tx_mac_descriptor);
#endif
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
        RDD_ETH_TX_MAC_DESCRIPTOR_TX_TASK_NUMBER_WRITE(LAN_TX_THREAD_NUMBER, eth_tx_mac_descriptor);
        RDD_ETH_TX_MAC_DESCRIPTOR_EMAC_MASK_WRITE((1 << emac), eth_tx_mac_descriptor);
        if (emac != RDD_LAN_VPORT_LAST)
            RDD_ETH_TX_MAC_DESCRIPTOR_GPIO_FLOW_CONTROL_VECTOR_PTR_WRITE((RDD_GPIO_IO_ADDRESS + (emac - RDD_LAN0_VPORT)), eth_tx_mac_descriptor);
        else
            RDD_ETH_TX_MAC_DESCRIPTOR_GPIO_FLOW_CONTROL_VECTOR_PTR_WRITE(RDD_GPIO_IO_ADDRESS + RDD_VPORT_ID_7, eth_tx_mac_descriptor);
        RDD_ETH_TX_MAC_DESCRIPTOR_PACKET_COUNTERS_PTR_0_WRITE(ETH_TX_MAC_TABLE_ADDRESS +
            RDD_LAN0_VPORT * sizeof(RDD_ETH_TX_MAC_DESCRIPTOR_DTS) + RDD_EMAC_DESCRIPTOR_EGRESS_COUNTER_OFFSET,
            eth_tx_mac_descriptor);
        RDD_ETH_TX_MAC_DESCRIPTOR_PACKET_COUNTERS_PTR_1_WRITE(ETH_TX_MAC_TABLE_ADDRESS +
            RDD_LAN1_VPORT * sizeof(RDD_ETH_TX_MAC_DESCRIPTOR_DTS) + RDD_EMAC_DESCRIPTOR_EGRESS_COUNTER_OFFSET,
            eth_tx_mac_descriptor);
        RDD_ETH_TX_MAC_DESCRIPTOR_PACKET_COUNTERS_PTR_2_WRITE(ETH_TX_MAC_TABLE_ADDRESS +
            RDD_LAN2_VPORT * sizeof(RDD_ETH_TX_MAC_DESCRIPTOR_DTS) + RDD_EMAC_DESCRIPTOR_EGRESS_COUNTER_OFFSET,
            eth_tx_mac_descriptor);
        RDD_ETH_TX_MAC_DESCRIPTOR_BBH_DESTINATION_0_WRITE(BBH_PERIPHERAL_ETH0_TX, eth_tx_mac_descriptor);
        RDD_ETH_TX_MAC_DESCRIPTOR_BBH_DESTINATION_1_WRITE(BBH_PERIPHERAL_ETH1_TX, eth_tx_mac_descriptor);
        RDD_ETH_TX_MAC_DESCRIPTOR_BBH_DESTINATION_2_WRITE(BBH_PERIPHERAL_ETH2_TX, eth_tx_mac_descriptor);
        if (emac != RDD_LAN_VPORT_LAST)
            RDD_ETH_TX_MAC_DESCRIPTOR_EGRESS_PORT_WRITE((emac - RDD_LAN0_VPORT), eth_tx_mac_descriptor);
        else
            RDD_ETH_TX_MAC_DESCRIPTOR_EGRESS_PORT_WRITE(RDD_VPORT_ID_7, eth_tx_mac_descriptor);
#endif
        RDD_ETH_TX_MAC_DESCRIPTOR_RATE_LIMITER_ID_WRITE(RDD_RATE_LIMITER_DISABLED, eth_tx_mac_descriptor);

        for (tx_queue = 0; tx_queue < RDD_EMAC_NUMBER_OF_QUEUES; tx_queue++)
        {
            eth_tx_queue_address = ETH_TX_QUEUES_TABLE_ADDRESS +
                ((emac - RDD_LAN0_VPORT) * RDD_EMAC_NUMBER_OF_QUEUES + tx_queue) * sizeof(RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS);

            mac_descriptor_address = ETH_TX_MAC_TABLE_ADDRESS + emac * sizeof(RDD_ETH_TX_MAC_DESCRIPTOR_DTS);

            eth_tx_queue_pointers_entry =
                &(eth_tx_queues_pointers_table->entry[emac * RDD_EMAC_NUMBER_OF_QUEUES + tx_queue]);

            RDD_ETH_TX_QUEUE_POINTERS_ENTRY_ETH_MAC_POINTER_WRITE(mac_descriptor_address, eth_tx_queue_pointers_entry);
            RDD_ETH_TX_QUEUE_POINTERS_ENTRY_TX_QUEUE_POINTER_WRITE(eth_tx_queue_address, eth_tx_queue_pointers_entry);

            eth_tx_queue_descriptor = &(eth_tx_queues_table->entry[(emac - RDD_LAN0_VPORT) * RDD_EMAC_NUMBER_OF_QUEUES + tx_queue]);

            RDD_ETH_TX_QUEUE_DESCRIPTOR_QUEUE_MASK_WRITE(1 << tx_queue , eth_tx_queue_descriptor);
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
            RDD_ETH_TX_QUEUE_DESCRIPTOR_INDEX_WRITE((emac * RDD_EMAC_NUMBER_OF_QUEUES) + tx_queue, eth_tx_queue_descriptor);
#endif
        }
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
        eth_tx_local_registers_entry = &(eth_tx_local_registers->entry[emac]);

        RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_EMAC_DESCRIPTOR_PTR_WRITE(ETH_TX_MAC_TABLE_ADDRESS +
            emac * sizeof(RDD_ETH_TX_MAC_DESCRIPTOR_DTS), eth_tx_local_registers_entry);

        RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_ETH_TX_QUEUES_POINTERS_TABLE_PTR_WRITE(ETH_TX_QUEUES_POINTERS_TABLE_ADDRESS +
            emac * RDD_EMAC_NUMBER_OF_QUEUES * sizeof(RDD_ETH_TX_QUEUE_POINTERS_ENTRY_DTS), eth_tx_local_registers_entry);
#endif
    }

    free_packet_descriptors_pool_descriptor =
        (RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) +
        FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ADDRESS);
#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908)
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_EGRESS_COUNTER_WRITE(0, free_packet_descriptors_pool_descriptor);
#else /*DSL:*/
    /*Initial values, will be updated by rdd_tm_ds_free_packet_descriptors_pool_size_update.*/
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_GUARANTEED_THRESHOLD_WRITE ( DS_FREE_PACKET_DESCRIPTOR_POOL_GUARANTEED_QUEUE_THRESHOLD, free_packet_descriptors_pool_descriptor );
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_GUARANTEED_FREE_COUNT_WRITE (DS_FREE_PACKET_DESCRIPTOR_POOL_MIN_GUARANTEED_POOL_SIZE, free_packet_descriptors_pool_descriptor );
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_NON_GUARANTEED_FREE_COUNT_WRITE ( RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_SIZE - DS_FREE_PACKET_DESCRIPTOR_POOL_MIN_GUARANTEED_POOL_SIZE, free_packet_descriptors_pool_descriptor );
#endif /*DSL*/
}
#endif

static void rdd_wan_channel_0_7_cfg(rdd_wan_channel_id_t wan_channel, rdd_wan_channel_schedule_t schedule_mode,
    rdd_peak_schedule_mode_t peak_schedule_mode)
{
    RDD_WAN_CHANNELS_0_7_TABLE_DTS *wan_channels_table;
    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_DTS *wan_channel_descriptor;
    rdd_wan_tx_pointers_entry_t *wan_tx_pointers_entry;
    rdd_rate_cntrl_id_t rate_cntrl;
    uint32_t queue_id;

    wan_channels_table = RDD_WAN_CHANNELS_0_7_TABLE_PTR();
    wan_channel_descriptor = &(wan_channels_table->entry[wan_channel]);

    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_SCHEDULE_WRITE(schedule_mode, wan_channel_descriptor);
    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_PEAK_SCHEDULING_MODE_WRITE(peak_schedule_mode, wan_channel_descriptor);
    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_ACK_PENDING_WRITE(1, wan_channel_descriptor);
#ifdef OREN
    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_ACK_PENDING_EPON_WRITE(1, wan_channel_descriptor);
#endif

#if defined(WL4908)
    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_BBH_DESTINATION_WRITE(BBH_PERIPHERAL_WAN_TX, wan_channel_descriptor);
#else 
    switch (g_wan_physical_port)
    {
    case RDD_WAN_PHYSICAL_PORT_GPON:
    case RDD_WAN_PHYSICAL_PORT_ETH5:
    case RDD_WAN_PHYSICAL_PORT_EPON:
        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_BBH_DESTINATION_WRITE((BBH_PERIPHERAL_WAN_TX + (wan_channel << 7)),
            wan_channel_descriptor);
        break;

    case RDD_WAN_PHYSICAL_PORT_ETH4:
        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_BBH_DESTINATION_WRITE(BBH_PERIPHERAL_ETH4_TX, wan_channel_descriptor);
        break;

    case RDD_WAN_PHYSICAL_PORT_ETH0:
        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_BBH_DESTINATION_WRITE(BBH_PERIPHERAL_ETH0_TX, wan_channel_descriptor);
        break;

    case RDD_WAN_PHYSICAL_PORT_ETH1:
        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_BBH_DESTINATION_WRITE(BBH_PERIPHERAL_ETH1_TX, wan_channel_descriptor);
        break;

    case RDD_WAN_PHYSICAL_PORT_ETH2:
        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_BBH_DESTINATION_WRITE(BBH_PERIPHERAL_ETH2_TX, wan_channel_descriptor);
        break;

    case RDD_WAN_PHYSICAL_PORT_ETH3:
        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_BBH_DESTINATION_WRITE(BBH_PERIPHERAL_ETH3_TX, wan_channel_descriptor);
        break;
    }
#endif

    for (rate_cntrl = RDD_RATE_CNTRL_0; rate_cntrl <= RDD_RATE_CNTRL_31; rate_cntrl++)
    {
        for (queue_id = RDD_TX_QUEUE_0; queue_id <= RDD_TX_QUEUE_7; queue_id++)
        {
            /* write the allocated wan channel descriptor address to the wan tx pointers table */
            wan_tx_pointers_entry = &(wan_tx_pointers_table->entry[wan_channel][rate_cntrl][queue_id]);

            wan_tx_pointers_entry->wan_channel_ptr = WAN_CHANNELS_0_7_TABLE_ADDRESS +
                wan_channel * sizeof(RDD_WAN_CHANNEL_0_7_DESCRIPTOR_DTS);
        }
    }
}

#if !defined(WL4908)
static void rdd_wan_channel_8_39_cfg(rdd_wan_channel_id_t wan_channel, rdd_wan_channel_schedule_t schedule_mode,
    rdd_peak_schedule_mode_t peak_schedule_mode)
{
    RDD_WAN_CHANNELS_8_39_TABLE_DTS *wan_channels_table;
    RDD_WAN_CHANNEL_8_39_DESCRIPTOR_DTS *wan_channel_descriptor;
    rdd_wan_tx_pointers_entry_t *wan_tx_pointers_entry;
    rdd_rate_cntrl_id_t rate_cntrl;
    uint32_t queue_id;

    wan_channels_table = RDD_WAN_CHANNELS_8_39_TABLE_PTR();
    wan_channel_descriptor = &(wan_channels_table->entry[wan_channel - RDD_WAN_CHANNEL_8]);

    RDD_WAN_CHANNEL_8_39_DESCRIPTOR_SCHEDULE_WRITE(schedule_mode, wan_channel_descriptor);
    RDD_WAN_CHANNEL_8_39_DESCRIPTOR_PEAK_SCHEDULING_MODE_WRITE(peak_schedule_mode, wan_channel_descriptor);
    RDD_WAN_CHANNEL_8_39_DESCRIPTOR_ACK_PENDING_WRITE(1, wan_channel_descriptor);
#ifdef OREN
    RDD_WAN_CHANNEL_8_39_DESCRIPTOR_ACK_PENDING_EPON_WRITE(1, wan_channel_descriptor);
#endif
    RDD_WAN_CHANNEL_8_39_DESCRIPTOR_BBH_DESTINATION_WRITE((BBH_PERIPHERAL_WAN_TX + (wan_channel << 7)),
        wan_channel_descriptor);

    for (rate_cntrl = RDD_RATE_CNTRL_0; rate_cntrl <= RDD_RATE_CNTRL_3; rate_cntrl++)
    {
        for (queue_id = RDD_TX_QUEUE_0; queue_id <= RDD_TX_QUEUE_7; queue_id++)
        {
            /* write the allocated wan channel descriptor address to the wan tx pointers table */
            wan_tx_pointers_entry = &(wan_tx_pointers_table->entry[wan_channel][rate_cntrl][queue_id]);

            wan_tx_pointers_entry->wan_channel_ptr = WAN_CHANNELS_8_39_TABLE_ADDRESS +
                (wan_channel - RDD_WAN_CHANNEL_8) * sizeof(RDD_WAN_CHANNEL_8_39_DESCRIPTOR_DTS);
        }
    }
}
#endif

#if defined(WL4908)
/* set the timer for upstream rate control budget allocation, in each
 * iteration 16 rate controllers get budget, this API should be called only
 * when at least one wan channel is working at rate control scheduling.
 */
static int rdd_us_rate_cntrl_timer_set(void)
{
    RDD_SYSTEM_CONFIGURATION_DTS           *system_cfg_register;
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_TIMER_TARGET           runner_timer_target_register;
    RUNNER_REGS_CFG_CPU_WAKEUP             runner_cpu_wakeup_register;

    RUNNER_REGS_1_CFG_TIMER_TARGET_READ ( runner_timer_target_register );

    runner_timer_target_register.timer_0_2 = RUNNER_REGS_CFG_TIMER_TARGET_TIMER_0_2_MAIN_CORE_VALUE;

    RUNNER_REGS_1_CFG_TIMER_TARGET_WRITE ( runner_timer_target_register );
#endif

    system_cfg_register = (RDD_SYSTEM_CONFIGURATION_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_SYSTEM_CONFIGURATION_ADDRESS);
#if !defined(FIRMWARE_INIT)
#ifdef RUNNER_FWTRACE
    /* For the Runner FW Trace, we have changed the timer granularity.  Therefore we need to adjust each timer's limit value */
    RDD_SYSTEM_CONFIGURATION_US_RATE_CONTROLLER_TIMER_WRITE(((RDD_US_RATE_CONTROL_TIMER_INTERVAL / 8) * (1000/TIMER_PERIOD_NS)) - 1, system_cfg_register);
#else
    /* budget allocation is taking place every 500 micro second, 16 rate controllers each cycle allocation */
    RDD_SYSTEM_CONFIGURATION_US_RATE_CONTROLLER_TIMER_WRITE(RDD_US_RATE_CONTROL_TIMER_INTERVAL / 8 - 1, system_cfg_register);
#endif
    runner_cpu_wakeup_register.req_trgt = RATE_CONTROLLER_BUDGET_ALLOCATOR_THREAD_NUMBER / 32;
    runner_cpu_wakeup_register.thread_num = RATE_CONTROLLER_BUDGET_ALLOCATOR_THREAD_NUMBER % 32;
    runner_cpu_wakeup_register.urgent_req = 0;

    RUNNER_REGS_1_CFG_CPU_WAKEUP_WRITE(runner_cpu_wakeup_register);
#else
    RDD_SYSTEM_CONFIGURATION_US_RATE_CONTROLLER_TIMER_WRITE (8, system_cfg_register);
#endif
    return 0;
}
#endif


int rdd_wan_channel_cfg(rdd_wan_channel_id_t wan_channel, rdd_wan_channel_schedule_t schedule_mode,
    rdd_peak_schedule_mode_t  peak_schedule_mode)
{
#if defined(WL4908)
    uint32_t api_first_time_call = 1;
#endif

    /* check the validity of the input parameters - wan channel index */
    if (wan_channel > RDD_WAN_CHANNEL_MAX)
        return BDMF_ERR_RANGE;

#if !defined(WL4908)
    if (wan_channel <= RDD_WAN_CHANNEL_7)
        rdd_wan_channel_0_7_cfg(wan_channel, schedule_mode, peak_schedule_mode);
    else
        rdd_wan_channel_8_39_cfg(wan_channel, schedule_mode, peak_schedule_mode);
#else
    rdd_wan_channel_0_7_cfg(wan_channel, schedule_mode, peak_schedule_mode);
#endif

#if defined(WL4908)
    /* TODO */
    if ((schedule_mode == RDD_WAN_CHANNEL_SCHEDULE_RATE_CONTROL) && api_first_time_call)
    {
        rdd_us_rate_cntrl_timer_set();
        api_first_time_call = 0;
    }
#endif
    return BDMF_ERR_OK;
}

#if defined(WL4908)
int rdd_wan_channel_rate_limiter_cfg(rdd_wan_channel_id_t channel_id, bdmf_boolean rate_limiter_enabled,
    rdpa_tm_orl_prty prio)
{
    RDD_WAN_CHANNELS_0_7_TABLE_DTS                         *wan_channels_0_7_table;
    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_DTS                     *wan_channel_0_7_descriptor;
#if !defined(WL4908)
    RDD_WAN_CHANNELS_8_39_TABLE_DTS                        *wan_channels_8_39_table;
    RDD_WAN_CHANNEL_8_39_DESCRIPTOR_DTS                    *wan_channel_8_39_descriptor;
#endif
    rdd_wan_tx_pointers_entry_t                            *wan_tx_pointers_entry;
    RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_TABLE_DTS  *rate_limiter_wan_channel_ptr_table;
    RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_ENTRY_DTS  *rate_limiter_wan_channel_ptr_entry;
    bdmf_boolean                                           limiter_enabled;
    uint16_t                                               entry_wan_channel_descriptor_address;
    uint16_t                                               wan_channel_pointer_index;
    static uint32_t                                        policer_number_of_connected_wan_channels = 0;
    int                                                    rdd_error;
#if !defined(FIRMWARE_INIT)
    unsigned long                                          flags;
#endif

#if !defined(WL4908)
    if ( channel_id <= RDD_WAN_CHANNEL_7 )
    {
        wan_channels_0_7_table     = RDD_WAN_CHANNELS_0_7_TABLE_PTR();
        wan_channel_0_7_descriptor = &(wan_channels_0_7_table->entry[channel_id]);

        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_LIMITER_MODE_READ ( limiter_enabled, wan_channel_0_7_descriptor );

        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_LIMITER_MODE_WRITE ( rate_limiter_enabled, wan_channel_0_7_descriptor );
        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_LIMITER_PRIORITY_WRITE ( ( prio & rate_limiter_enabled ), wan_channel_0_7_descriptor );
    }
    else
    {
        wan_channels_8_39_table     = RDD_WAN_CHANNELS_8_39_TABLE_PTR();
        wan_channel_8_39_descriptor = &(wan_channels_8_39_table->entry[channel_id - RDD_WAN_CHANNEL_8]);

        RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_LIMITER_MODE_READ ( limiter_enabled, wan_channel_8_39_descriptor );

        RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_LIMITER_MODE_WRITE ( rate_limiter_enabled, wan_channel_8_39_descriptor );
        RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_LIMITER_PRIORITY_WRITE ( ( prio & rate_limiter_enabled ), wan_channel_8_39_descriptor );
    }
#else
    wan_channels_0_7_table     = RDD_WAN_CHANNELS_0_7_TABLE_PTR();
    wan_channel_0_7_descriptor = &(wan_channels_0_7_table->entry[channel_id]);

    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_LIMITER_MODE_READ ( limiter_enabled, wan_channel_0_7_descriptor );

    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_LIMITER_MODE_WRITE ( rate_limiter_enabled, wan_channel_0_7_descriptor );
    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_LIMITER_PRIORITY_WRITE ( ( prio & rate_limiter_enabled ), wan_channel_0_7_descriptor );
#endif

    rate_limiter_wan_channel_ptr_table = ( RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_TABLE_ADDRESS );

    rdd_error = BDMF_ERR_OK;

    if ( rate_limiter_enabled == 1 )
    {
        if ( limiter_enabled == 1 )
        {
            return ( BDMF_ERR_OK );
        }

        wan_tx_pointers_entry = &( wan_tx_pointers_table->entry[ channel_id ][ 0 ][ 0 ] );

        rate_limiter_wan_channel_ptr_entry = &( rate_limiter_wan_channel_ptr_table->entry[ policer_number_of_connected_wan_channels ] );
        RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_ENTRY_WAN_CHANNEL_PTR_WRITE ( wan_tx_pointers_entry->wan_channel_ptr, rate_limiter_wan_channel_ptr_entry );

        policer_number_of_connected_wan_channels++;
    }
    else
    {
        wan_tx_pointers_entry = &( wan_tx_pointers_table->entry[ channel_id ][ 0 ][ 0 ] );

#if !defined(WL4908)
        for ( wan_channel_pointer_index = RDD_WAN_CHANNEL_0; wan_channel_pointer_index <= RDD_WAN_CHANNEL_39; wan_channel_pointer_index++ )
#else
        for ( wan_channel_pointer_index = RDD_WAN_CHANNEL_0; wan_channel_pointer_index <= RDD_WAN_CHANNEL_0_7_MAX; wan_channel_pointer_index++ )
#endif
        {
            rate_limiter_wan_channel_ptr_entry = &( rate_limiter_wan_channel_ptr_table->entry[ wan_channel_pointer_index ] );
            RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_ENTRY_WAN_CHANNEL_PTR_READ ( entry_wan_channel_descriptor_address, rate_limiter_wan_channel_ptr_entry );

            if ( entry_wan_channel_descriptor_address == wan_tx_pointers_entry->wan_channel_ptr )
            {
                policer_number_of_connected_wan_channels--;

                rate_limiter_wan_channel_ptr_entry = &( rate_limiter_wan_channel_ptr_table->entry[ policer_number_of_connected_wan_channels ] );
                RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_ENTRY_WAN_CHANNEL_PTR_READ ( entry_wan_channel_descriptor_address, rate_limiter_wan_channel_ptr_entry );

                rate_limiter_wan_channel_ptr_entry = &( rate_limiter_wan_channel_ptr_table->entry[ wan_channel_pointer_index ] );
                RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_ENTRY_WAN_CHANNEL_PTR_WRITE ( entry_wan_channel_descriptor_address, rate_limiter_wan_channel_ptr_entry );

                rate_limiter_wan_channel_ptr_entry = &( rate_limiter_wan_channel_ptr_table->entry[ policer_number_of_connected_wan_channels ] );
                RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_ENTRY_WAN_CHANNEL_PTR_WRITE ( 0, rate_limiter_wan_channel_ptr_entry );

                break;
            }
        }

#if !defined(FIRMWARE_INIT)

        if ( limiter_enabled == 1 )
        {
            bdmf_fastlock_lock_irq(&int_lock_irq, flags);

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
            rdd_error = rdd_cpu_tx_send_message ( RDD_CPU_TX_MESSAGE_UPDATE_PD_POOL_QUOTA, FAST_RUNNER_B, RUNNER_PRIVATE_1_OFFSET, channel_id, 0, 0, 1/*wait*/ );
#else
            rdd_error = rdd_cpu_tx_send_message ( RDD_CPU_TX_MESSAGE_ACTIVATE_TCONT, FAST_RUNNER_B, RUNNER_PRIVATE_1_OFFSET, channel_id, 0, 0, 1/*wait*/ );
#endif
            bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
        }
#endif

    }

    return ( rdd_error );
}


static void rdd_rate_controller_params_set(RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS *rate_cntrl_descriptor,
    rdd_rate_cntrl_params_t *params)
{
    RDD_RATE_CONTROLLER_EXPONENT_TABLE_DTS *exponent_table_ptr;
    RDD_RATE_CONTROLLER_EXPONENT_ENTRY_DTS *exponent_entry_ptr;
    uint32_t exponent_list[RDD_US_RATE_CONTROL_EXPONENT_NUM];
    uint32_t exponent_table_index;
    uint32_t peak_budget_exponent;
    uint32_t peak_limit_exponent;
    uint32_t sustain_budget;
    uint32_t peak_budget;
    uint32_t peak_limit;

    /* read exponents table */
    exponent_table_ptr = (RDD_RATE_CONTROLLER_EXPONENT_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_RATE_CONTROLLER_EXPONENT_TABLE_ADDRESS);
    for(exponent_table_index = 0; exponent_table_index < RDD_US_RATE_CONTROL_EXPONENT_NUM; exponent_table_index++)
    {
        exponent_entry_ptr = &exponent_table_ptr->entry[exponent_table_index];
        RDD_RATE_CONTROLLER_EXPONENT_ENTRY_EXPONENT_READ(exponent_list[exponent_table_index], exponent_entry_ptr);
    }

    /* convert sustain rate to allocation units */
    sustain_budget = rdd_budget_to_alloc_unit(params->sustain_budget, RDD_US_RATE_CONTROL_TIMER_INTERVAL, 0);

    /* convert peak budget to allocation unit and divide to exponent and mantissa */
    peak_budget = rdd_budget_to_alloc_unit(params->peak_budget.rate, RDD_US_RATE_CONTROL_TIMER_INTERVAL, 0);
    peak_budget_exponent = rdd_get_exponent(params->peak_budget.rate, 14, RDD_US_RATE_CONTROL_EXPONENT_NUM, exponent_list);
    peak_budget = peak_budget >> exponent_list[peak_budget_exponent];

    /* convert peak limit to allocation unit and divide to exponent and mantissa */
    peak_limit_exponent = rdd_get_exponent(params->peak_budget.limit, 14, RDD_US_RATE_CONTROL_EXPONENT_NUM, exponent_list);
    peak_limit = params->peak_budget.limit >> exponent_list[peak_limit_exponent];

    /* write rate control parameters to descriptor */
    RDD_US_RATE_CONTROLLER_DESCRIPTOR_ALLOCATED_PEAK_BUDGET_EXPONENT_WRITE(peak_budget_exponent, rate_cntrl_descriptor);
    RDD_US_RATE_CONTROLLER_DESCRIPTOR_ALLOCATED_PEAK_BUDGET_WRITE(peak_budget, rate_cntrl_descriptor);
    RDD_US_RATE_CONTROLLER_DESCRIPTOR_PEAK_BUDGET_LIMIT_EXPONENT_WRITE(peak_limit_exponent, rate_cntrl_descriptor);
    RDD_US_RATE_CONTROLLER_DESCRIPTOR_PEAK_BUDGET_LIMIT_WRITE(peak_limit, rate_cntrl_descriptor);
    RDD_US_RATE_CONTROLLER_DESCRIPTOR_ALLOCATED_SUSTAIN_BUDGET_WRITE(sustain_budget, rate_cntrl_descriptor);
    RDD_US_RATE_CONTROLLER_DESCRIPTOR_PEAK_WEIGHT_WRITE(params->peak_weight >> 8, rate_cntrl_descriptor);
}
#endif

int rdd_rate_cntrl_cfg(rdd_wan_channel_id_t wan_channel, rdd_rate_cntrl_id_t rate_cntrl,
    rdd_rate_cntrl_params_t *rate_cntrl_params)
{
    RDD_WAN_CHANNELS_0_7_TABLE_DTS *wan_channels_0_7_table;
    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_DTS *wan_channel_0_7_descriptor;
#if !defined(WL4908)
    RDD_WAN_CHANNELS_8_39_TABLE_DTS *wan_channels_8_39_table;
    RDD_WAN_CHANNEL_8_39_DESCRIPTOR_DTS *wan_channel_8_39_descriptor;
#endif
#if defined(WL4908)
    RDD_BUDGET_ALLOCATOR_TABLE_DTS *budget_allocator_table;
    RDD_BUDGET_ALLOCATOR_ENTRY_DTS *budget_allocator_entry;
    uint32_t wan_channel_scheduling;
    uint16_t active_rate_cntrls;
#endif
    RDD_US_RATE_CONTROLLERS_TABLE_DTS *rate_cntrls_table;
    RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS *rate_cntrl_descriptor;
    rdd_wan_tx_pointers_entry_t *wan_tx_pointers_entry;
    uint32_t rate_cntrl_mask;
    uint32_t tx_queue;
    uint32_t i;

    /* check the validity of the input parameters - wan channel and Rate controller indexes */
    if (wan_channel > RDD_WAN_CHANNEL_MAX)
        return BDMF_ERR_RANGE;

    rate_cntrls_table = RDD_US_RATE_CONTROLLERS_TABLE_PTR();

    /* find a free rate controller descriptor in the table */
    for (i = 0; i < RDD_US_RATE_CONTROLLERS_TABLE_SIZE; i++)
    {
        rate_cntrl_descriptor =
            &(rate_cntrls_table->entry[(i + g_rate_cntrls_pool_idx) % RDD_US_RATE_CONTROLLERS_TABLE_SIZE]);

        RDD_US_RATE_CONTROLLER_DESCRIPTOR_RATE_CONTROLLER_MASK_READ(rate_cntrl_mask, rate_cntrl_descriptor);

        if (rate_cntrl_mask == 0)
            break;
    }

    /* all the rate controller descriptors are occupied */
    if (i == RDD_US_RATE_CONTROLLERS_TABLE_SIZE)
        return BDMF_ERR_NOMEM;

    i = (i + g_rate_cntrls_pool_idx) % RDD_US_RATE_CONTROLLERS_TABLE_SIZE;

    MEMSET(rate_cntrl_descriptor, 0, sizeof(RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS));

    for (tx_queue = RDD_TX_QUEUE_0; tx_queue <= RDD_TX_QUEUE_7; tx_queue++)
    {
        /* write the allocated rate controller descriptor address to the wan tx queues pointers table */
        wan_tx_pointers_entry = &(wan_tx_pointers_table->entry[wan_channel][rate_cntrl][tx_queue]);

        wan_tx_pointers_entry->rate_cntrl_ptr = US_RATE_CONTROLLERS_TABLE_ADDRESS +
            i * sizeof(RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS);

        /* priority queues are not initialized - therfore packets should be dropped */
        RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_WRITE(DUMMY_WAN_TX_QUEUE_DESCRIPTOR_ADDRESS,
            rate_cntrl_descriptor, tx_queue);
    }

    /* initialize the hardcoded parameters of the rate controller descriptor */
    RDD_US_RATE_CONTROLLER_DESCRIPTOR_RATE_CONTROLLER_MASK_WRITE((1 << rate_cntrl), rate_cntrl_descriptor);

#if defined(WL4908)
    rdd_rate_controller_params_set(rate_cntrl_descriptor, rate_cntrl_params);
#endif

    RDD_US_RATE_CONTROLLER_DESCRIPTOR_PEAK_BURST_FLAG_WRITE(1, rate_cntrl_descriptor);

    /* cfguration handler assumption: the wan channel was already cfgured */
    RDD_US_RATE_CONTROLLER_DESCRIPTOR_WAN_CHANNEL_PTR_WRITE(wan_tx_pointers_entry->wan_channel_ptr,
        rate_cntrl_descriptor);

#if !defined(WL4908)
    if (wan_channel <= RDD_WAN_CHANNEL_7)
    {
        wan_channels_0_7_table = RDD_WAN_CHANNELS_0_7_TABLE_PTR();
        wan_channel_0_7_descriptor = &(wan_channels_0_7_table->entry[wan_channel]);

        /* set the inverse connection: wan channel to it's rate controller */
        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLER_ADDR_WRITE(US_RATE_CONTROLLERS_TABLE_ADDRESS +
            i * sizeof(RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS), wan_channel_0_7_descriptor, rate_cntrl);
    }
    else
    {
        wan_channels_8_39_table = RDD_WAN_CHANNELS_8_39_TABLE_PTR();
        wan_channel_8_39_descriptor = &(wan_channels_8_39_table->entry[wan_channel - RDD_WAN_CHANNEL_8]);

        /* set the inverse connection: wan channel to it's rate controller */
        RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLER_ADDR_WRITE(US_RATE_CONTROLLERS_TABLE_ADDRESS +
            i * sizeof(RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS), wan_channel_8_39_descriptor, rate_cntrl);
    }
#else
    wan_channels_0_7_table = RDD_WAN_CHANNELS_0_7_TABLE_PTR();
    wan_channel_0_7_descriptor = &(wan_channels_0_7_table->entry[wan_channel]);

    /* set the inverse connection: wan channel to it's rate controller */
    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLER_ADDR_WRITE(US_RATE_CONTROLLERS_TABLE_ADDRESS +
        i * sizeof(RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS), wan_channel_0_7_descriptor, rate_cntrl);
#if defined(WL4908)
    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_SCHEDULE_READ(wan_channel_scheduling, wan_channel_0_7_descriptor);
#endif
#endif

#if defined(WL4908)
    /* budget allocator optimization - spread allocation between groups of 16 rate controller descriptors */
    g_rate_cntrls_pool_idx =(i + RDD_RATE_CONTROLLERS_BUDGET_SET_SIZE) & ~(RDD_RATE_CONTROLLERS_BUDGET_SET_SIZE - 1);

    if ( wan_channel_scheduling )
    {
        budget_allocator_table = (RDD_BUDGET_ALLOCATOR_TABLE_DTS *)(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_RATE_CONTROL_BUDGET_ALLOCATOR_TABLE_ADDRESS);
        budget_allocator_entry = &budget_allocator_table->entry[i / RDD_RATE_CONTROLLERS_BUDGET_SET_SIZE];
        RDD_BUDGET_ALLOCATOR_ENTRY_ACTIVE_RATE_CONTROLLERS_READ(active_rate_cntrls, budget_allocator_entry);
        active_rate_cntrls |= 1 << (i % RDD_RATE_CONTROLLERS_BUDGET_SET_SIZE);
        RDD_BUDGET_ALLOCATOR_ENTRY_ACTIVE_RATE_CONTROLLERS_WRITE(active_rate_cntrls, budget_allocator_entry);
    }
#endif

    return BDMF_ERR_OK;
}

#if defined(WL4908)
int rdd_rate_cntrl_modify(rdd_wan_channel_id_t wan_channel, rdd_rate_cntrl_id_t rate_cntrl,
    rdd_rate_cntrl_params_t *rate_cntrl_params)
{
    RDD_WAN_CHANNELS_0_7_TABLE_DTS         *wan_channels_0_7_table;
    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_DTS     *wan_channel_0_7_descriptor;
#if !defined(WL4908)
    RDD_WAN_CHANNELS_8_39_TABLE_DTS        *wan_channels_8_39_table;
    RDD_WAN_CHANNEL_8_39_DESCRIPTOR_DTS    *wan_channel_8_39_descriptor;
#endif
    rdd_wan_tx_pointers_entry_t            *wan_tx_pointers_entry;
    RDD_BUDGET_ALLOCATOR_TABLE_DTS         *budget_allocator_table;
    RDD_BUDGET_ALLOCATOR_ENTRY_DTS         *budget_allocator_entry;
    uint16_t                               active_rate_cntrls;
    uint32_t                               wan_channel_scheduling;
    uint32_t                               idx;

    /* check the validity of the input parameters - wan channel and Rate controller indexes */
    if ( wan_channel > RDD_WAN_CHANNEL_MAX )
        return BDMF_ERR_RANGE;

#if !defined(WL4908)
    if (wan_channel <= RDD_WAN_CHANNEL_7)
    {
        if (rate_cntrl > RDD_RATE_CNTRL_31)
            return BDMF_ERR_RANGE;
    }
    else
    {
        if (rate_cntrl > RDD_RATE_CNTRL_3)
            return BDMF_ERR_RANGE;
    }
#endif

    wan_tx_pointers_entry = &(wan_tx_pointers_table->entry[wan_channel][rate_cntrl][0]);

    /* verify that the rate controller was configured before */
    if (wan_tx_pointers_entry->rate_cntrl_ptr == 0)
        return BDMF_ERR_RANGE;

    rdd_rate_controller_params_set( ( RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) +
                                       wan_tx_pointers_entry->rate_cntrl_ptr - sizeof ( RUNNER_COMMON ) ),
                                    rate_cntrl_params );

    /*get rate controller index*/
    idx = ( wan_tx_pointers_entry->rate_cntrl_ptr - US_RATE_CONTROLLERS_TABLE_ADDRESS ) / sizeof ( RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS );

#if !defined(WL4908)
    if ( wan_channel <= RDD_WAN_CHANNEL_7 )
    {
        wan_channels_0_7_table     = RDD_WAN_CHANNELS_0_7_TABLE_PTR();
        wan_channel_0_7_descriptor = &(wan_channels_0_7_table->entry[wan_channel]);

        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_SCHEDULE_READ ( wan_channel_scheduling, wan_channel_0_7_descriptor );
    }
    else
    {
        wan_channels_8_39_table     = RDD_WAN_CHANNELS_8_39_TABLE_PTR();
        wan_channel_8_39_descriptor = &(wan_channels_8_39_table->entry[wan_channel - RDD_WAN_CHANNEL_8]);

        RDD_WAN_CHANNEL_8_39_DESCRIPTOR_SCHEDULE_READ ( wan_channel_scheduling, wan_channel_8_39_descriptor );
    }
#else
    wan_channels_0_7_table = RDD_WAN_CHANNELS_0_7_TABLE_PTR();
    wan_channel_0_7_descriptor = &(wan_channels_0_7_table->entry[wan_channel]);

    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_SCHEDULE_READ ( wan_channel_scheduling, wan_channel_0_7_descriptor );
#endif

    /* budget allocator optimization - spread allocation between groups of 16 rate controller descriptors */
    budget_allocator_table = (RDD_BUDGET_ALLOCATOR_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_RATE_CONTROL_BUDGET_ALLOCATOR_TABLE_ADDRESS);
    budget_allocator_entry = &budget_allocator_table->entry[idx / 16];

    RDD_BUDGET_ALLOCATOR_ENTRY_ACTIVE_RATE_CONTROLLERS_READ ( active_rate_cntrls, budget_allocator_entry );
    
    if ( wan_channel_scheduling )
    {
        active_rate_cntrls |= ( 1 << ( idx % 16 ) );
    }
    else
    {
        active_rate_cntrls &= ~( 1 << ( idx % 16 ) );
    }

    RDD_BUDGET_ALLOCATOR_ENTRY_ACTIVE_RATE_CONTROLLERS_WRITE ( active_rate_cntrls, budget_allocator_entry );
    return BDMF_ERR_OK;
}

void rdd_rate_controller_sustain_budget_limit_config(uint32_t  sustain_budget_limit)
{
    RDD_SYSTEM_CONFIGURATION_DTS *system_cfg;

    system_cfg = (RDD_SYSTEM_CONFIGURATION_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_SYSTEM_CONFIGURATION_ADDRESS);
    RDD_SYSTEM_CONFIGURATION_US_RATE_LIMIT_SUSTAIN_BUDGET_LIMIT_WRITE(sustain_budget_limit >> 8, system_cfg);
}
#endif

int rdd_rate_cntrl_remove(rdd_wan_channel_id_t wan_channel, rdd_rate_cntrl_id_t rate_cntrl)
{
    RDD_WAN_CHANNELS_0_7_TABLE_DTS *wan_channels_0_7_table;
    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_DTS *wan_channel_0_7_descriptor;
#if !defined(WL4908)
    RDD_WAN_CHANNELS_8_39_TABLE_DTS *wan_channels_8_39_table;
    RDD_WAN_CHANNEL_8_39_DESCRIPTOR_DTS *wan_channel_8_39_descriptor;
#endif
#if defined(WL4908)
    RDD_US_RATE_CONTROLLERS_TABLE_DTS *rate_cntrls_table;
    RDD_BUDGET_ALLOCATOR_TABLE_DTS *budget_allocator_table;
    RDD_BUDGET_ALLOCATOR_ENTRY_DTS *budget_allocator_entry;
    uint32_t wan_channel_scheduling;
    uint16_t active_rate_cntrls;
    uint16_t i;
#endif
    RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS *rate_cntrl_descriptor;
    rdd_wan_tx_pointers_entry_t *wan_tx_pointers_entry;
    uint32_t tx_queue;

    /* check the validity of the input parameters - wan channel and Rate controller indexes */
    if (wan_channel > RDD_WAN_CHANNEL_MAX)
        return BDMF_ERR_RANGE;

#if !defined(WL4908)
    if (wan_channel <= RDD_WAN_CHANNEL_7)
    {
        if (rate_cntrl > RDD_RATE_CNTRL_31)
            return BDMF_ERR_RANGE;
    }
    else
    {
        if (rate_cntrl > RDD_RATE_CNTRL_3)
            return BDMF_ERR_RANGE;
    }
#endif

    wan_tx_pointers_entry = &(wan_tx_pointers_table->entry[wan_channel][rate_cntrl][0]);

    /* verify that the rate controller was cfgured before */
    if (wan_tx_pointers_entry->rate_cntrl_ptr == 0)
        return BDMF_ERR_RANGE;

    /* disconnect the queue from the rate controller */
#if !defined(WL4908)
    if (wan_channel <= RDD_WAN_CHANNEL_7)
    {
        wan_channels_0_7_table = RDD_WAN_CHANNELS_0_7_TABLE_PTR();
        wan_channel_0_7_descriptor = &(wan_channels_0_7_table->entry[wan_channel]);

        /* set the inverse connection: wan channel to it's rate controller */
        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLER_ADDR_WRITE(DUMMY_US_RATE_CONTROLLER_DESCRIPTOR_ADDRESS,
            wan_channel_0_7_descriptor, rate_cntrl);
    }
    else
    {
        wan_channels_8_39_table = RDD_WAN_CHANNELS_8_39_TABLE_PTR();
        wan_channel_8_39_descriptor = &(wan_channels_8_39_table->entry[wan_channel - RDD_WAN_CHANNEL_8]);

        /* set the inverse connection: wan channel to it's rate controller */
        RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLER_ADDR_WRITE(DUMMY_US_RATE_CONTROLLER_DESCRIPTOR_ADDRESS,
            wan_channel_8_39_descriptor, rate_cntrl);
    }
#else
    wan_channels_0_7_table = RDD_WAN_CHANNELS_0_7_TABLE_PTR();
    wan_channel_0_7_descriptor = &(wan_channels_0_7_table->entry[wan_channel]);

    /* set the inverse connection: wan channel to it's rate controller */
    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLER_ADDR_WRITE(DUMMY_US_RATE_CONTROLLER_DESCRIPTOR_ADDRESS,
        wan_channel_0_7_descriptor, rate_cntrl);
#if defined(WL4908)
    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_SCHEDULE_READ(wan_channel_scheduling, wan_channel_0_7_descriptor);
#endif
#endif

    rate_cntrl_descriptor = (RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_1_OFFSET) +
        wan_tx_pointers_entry->rate_cntrl_ptr - sizeof(RUNNER_COMMON));

    RDD_US_RATE_CONTROLLER_DESCRIPTOR_RATE_CONTROLLER_MASK_WRITE(0, rate_cntrl_descriptor);

    for (tx_queue = RDD_TX_QUEUE_7; tx_queue < RDD_TX_QUEUE_7; tx_queue++)
    {
        wan_tx_pointers_entry = &(wan_tx_pointers_table->entry[wan_channel][rate_cntrl][tx_queue]);

        wan_tx_pointers_entry->rate_cntrl_ptr = 0;
    }

#if defined(WL4908)
    rate_cntrls_table = (RDD_US_RATE_CONTROLLERS_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_1_OFFSET) + US_RATE_CONTROLLERS_TABLE_ADDRESS - sizeof(RUNNER_COMMON));

    /* budget allocator optimization */
    if(wan_channel_scheduling)
    {
        budget_allocator_table = (RDD_BUDGET_ALLOCATOR_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_RATE_CONTROL_BUDGET_ALLOCATOR_TABLE_ADDRESS);
        for(i = 0; i < RDD_US_RATE_CONTROLLERS_TABLE_SIZE; i++)
        {
            if(rate_cntrl_descriptor == &rate_cntrls_table->entry[i])
                break;
        }

        budget_allocator_entry = &budget_allocator_table->entry[i / 16];
        RDD_BUDGET_ALLOCATOR_ENTRY_ACTIVE_RATE_CONTROLLERS_READ(active_rate_cntrls, budget_allocator_entry);
        active_rate_cntrls &= ~(1 << (i % 16));
        RDD_BUDGET_ALLOCATOR_ENTRY_ACTIVE_RATE_CONTROLLERS_WRITE(active_rate_cntrls, budget_allocator_entry);
    }
#endif

    return BDMF_ERR_OK;
}

#if defined(WL4908)
void rdd_us_overall_rate_limiter_cfg(rdd_rate_limit_params_t *budget)
{
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_TIMER_TARGET  runner_timer_target_register;
#endif
    RDD_US_RATE_LIMITER_TABLE_DTS *rate_limiter_table_ptr;
    RDD_RATE_LIMITER_ENTRY_DTS    *rate_limiter_entry_ptr;
    static uint32_t               api_first_time_call = 1;
    uint32_t                      rate;
    uint32_t                      limit;
    uint32_t                      exponent_index;
    uint32_t                      exponent_list[] = { 0, UPSTREAM_RATE_LIMITER_EXPONENT };
    
    rate_limiter_table_ptr = ( RDD_US_RATE_LIMITER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_RATE_LIMITER_TABLE_ADDRESS );

    rate_limiter_entry_ptr = &( rate_limiter_table_ptr->entry[ UPSTREAM_RATE_LIMITER_ID ] );

    if ( budget->rate == 0 )
    {
        /* stop limiting immediately */
        RDD_RATE_LIMITER_ENTRY_CURRENT_BUDGET_WRITE ( 0xFFFFFFFF, rate_limiter_entry_ptr );
    }

    /* devise mantissa and exponent of budget limit and write them to memory */
    exponent_index = rdd_get_exponent ( budget->limit, 15, 2, exponent_list );

    limit = budget->limit >> exponent_list[ exponent_index ];

    RDD_RATE_LIMITER_ENTRY_BUDGET_LIMIT_WRITE ( limit, rate_limiter_entry_ptr );
    RDD_RATE_LIMITER_ENTRY_BUDGET_LIMIT_EXP_WRITE ( exponent_index, rate_limiter_entry_ptr );

    /* devise mantissa and exponent of allocated budget and write them to memory */
    rate = rdd_budget_to_alloc_unit ( budget->rate, UPSTREAM_RATE_LIMITER_TIMER_PERIOD, 0 );

    exponent_index = rdd_get_exponent ( rate, 15, 2, exponent_list );

    rate >>= exponent_list[ exponent_index ];

    RDD_RATE_LIMITER_ENTRY_ALLOCATED_BUDGET_WRITE ( rate, rate_limiter_entry_ptr );
    RDD_RATE_LIMITER_ENTRY_ALLOCATED_BUDGET_EXP_WRITE ( exponent_index, rate_limiter_entry_ptr );
	
    if ( api_first_time_call )
    {
#if !defined(FIRMWARE_INIT)
        RUNNER_REGS_1_CFG_TIMER_TARGET_READ ( runner_timer_target_register );

        runner_timer_target_register.timer_4_6 = RUNNER_REGS_CFG_TIMER_TARGET_TIMER_4_6_PICO_CORE_VALUE;

        RUNNER_REGS_1_CFG_TIMER_TARGET_WRITE ( runner_timer_target_register );

        rdd_timer_task_config ( rdpa_dir_us, UPSTREAM_RATE_LIMITER_TIMER_PERIOD, UPSTREAM_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID );
#else
        rdd_timer_task_config ( rdpa_dir_us, 400, UPSTREAM_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID );
#endif

        api_first_time_call = 0;
    }

    return;
}
#endif

int rdd_wan_tx_queue_cfg(rdd_wan_channel_id_t wan_channel, rdd_rate_cntrl_id_t rate_cntrl,
    rdd_tx_queue_id_t tx_queue, uint16_t packet_threshold, rdd_queue_profile_id_t profile_id, uint8_t counter_id)
{
    RDD_WAN_TX_QUEUES_TABLE_DTS *wan_tx_queues_table;
    RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS *wan_tx_queue_descriptor;
    RDD_US_RATE_CONTROLLERS_TABLE_DTS *rate_cntrls_table;
    RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS *rate_cntrl_descriptor;
    rdd_wan_tx_pointers_entry_t *wan_tx_pointers_entry;
    uint32_t rate_cntrl_pool_idx;
    uint16_t wan_channel_queue_mask;
    uint16_t queue_profile_address;
    uint32_t i;

    /* check the validity of the input parameters - wan channel, Rate controller and queue indexes */
    if (wan_channel > RDD_WAN_CHANNEL_MAX)
        return BDMF_ERR_RANGE;

#if !defined(WL4908)
    if (wan_channel <= RDD_WAN_CHANNEL_7)
    {
        if (rate_cntrl > RDD_RATE_CNTRL_31)
            return BDMF_ERR_RANGE;
    }
    else
    {
        if (rate_cntrl > RDD_RATE_CNTRL_3)
            return BDMF_ERR_RANGE;
    }
#endif

    if (tx_queue > RDD_TX_QUEUE_7)
        return BDMF_ERR_RANGE;

    wan_tx_queues_table = RDD_WAN_TX_QUEUES_TABLE_PTR();

    /* find a free wan tx queue descriptor in the wan tx queues table */
    for (i = 0; i < RDD_WAN_TX_QUEUES_TABLE_SIZE; i++)
    {
        wan_tx_queue_descriptor = &(wan_tx_queues_table->entry[i]);

        RDD_WAN_TX_QUEUE_DESCRIPTOR_QUEUE_MASK_READ(wan_channel_queue_mask , wan_tx_queue_descriptor);

        if (wan_channel_queue_mask == 0)
            break;
    }

    /* all the wan tx queues descriptors are occupied */
    if (i == RDD_WAN_TX_QUEUES_TABLE_SIZE)
        return BDMF_ERR_NOMEM;

    MEMSET(wan_tx_queue_descriptor, 0, sizeof(RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS));

    /* write the allocated tx queue descriptor address to the wan tx queues pointers table */
    wan_tx_pointers_entry = &(wan_tx_pointers_table->entry[wan_channel][rate_cntrl][tx_queue]);

    wan_tx_pointers_entry->wan_tx_queue_ptr = WAN_TX_QUEUES_TABLE_ADDRESS +
        i * sizeof(RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS);

    /* initialize the hardcoded parameters of the wan tx queue descriptor */
#if defined(WL4908)
    RDD_WAN_TX_QUEUE_DESCRIPTOR_INDEX_WRITE ( i, wan_tx_queue_descriptor );
#endif
    RDD_WAN_TX_QUEUE_DESCRIPTOR_QUEUE_MASK_WRITE((1 << tx_queue), wan_tx_queue_descriptor);

    /* initialize the cfgured parameters of the wan tx queue descriptor */
    RDD_WAN_TX_QUEUE_DESCRIPTOR_PACKET_THRESHOLD_WRITE(packet_threshold, wan_tx_queue_descriptor);

    if (profile_id == RDD_QUEUE_PROFILE_DISABLED)
        queue_profile_address = 0;
    else
        queue_profile_address = US_QUEUE_PROFILE_TABLE_ADDRESS + profile_id * sizeof(RDD_QUEUE_PROFILE_DTS);

    RDD_WAN_TX_QUEUE_DESCRIPTOR_PROFILE_PTR_WRITE(queue_profile_address, wan_tx_queue_descriptor);
    RDD_WAN_TX_QUEUE_DESCRIPTOR_PACKET_THRESHOLD_WRITE(packet_threshold, wan_tx_queue_descriptor);

    /* cfguration handler assumption: the rate controller was already cfgured */
    RDD_WAN_TX_QUEUE_DESCRIPTOR_RATE_CONTROLLER_PTR_WRITE(wan_tx_pointers_entry->rate_cntrl_ptr,
        wan_tx_queue_descriptor);

    rate_cntrls_table = RDD_US_RATE_CONTROLLERS_TABLE_PTR();

    rate_cntrl_pool_idx = (wan_tx_pointers_entry->rate_cntrl_ptr - US_RATE_CONTROLLERS_TABLE_ADDRESS) /
        sizeof(RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS);

    rate_cntrl_descriptor = &(rate_cntrls_table->entry[rate_cntrl_pool_idx]);

    /* set the inverse connection: rate controller to it's queue */
    RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_WRITE(WAN_TX_QUEUES_TABLE_ADDRESS +
        i * sizeof(RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS), rate_cntrl_descriptor, tx_queue);

    return BDMF_ERR_OK;
}

int rdd_wan_tx_queue_modify(rdd_wan_channel_id_t wan_channel, rdd_rate_cntrl_id_t rate_cntrl,
    rdd_tx_queue_id_t tx_queue, uint16_t packet_threshold, rdd_queue_profile_id_t profile_id, uint8_t counter_id)
{
    RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS *wan_tx_queue_descriptor;
    rdd_wan_tx_pointers_entry_t *wan_tx_pointers_entry;
    uint16_t queue_profile_address;

    /* check the validity of the input parameters - wan channel, Rate controller and queue indexes */
    if (wan_channel > RDD_WAN_CHANNEL_MAX)
        return BDMF_ERR_RANGE;

#if !defined(WL4908)
    if (wan_channel <= RDD_WAN_CHANNEL_7)
    {
        if (rate_cntrl > RDD_RATE_CNTRL_31)
            return BDMF_ERR_RANGE;
    }
    else
    {
        if (rate_cntrl > RDD_RATE_CNTRL_3)
            return BDMF_ERR_RANGE;
    }
#endif

    if (tx_queue > RDD_TX_QUEUE_7)
        return BDMF_ERR_RANGE;

    wan_tx_pointers_entry = &(wan_tx_pointers_table->entry[wan_channel][rate_cntrl][tx_queue]);

    /* verify that the wan channel queue was cfgured before */
    if (wan_tx_pointers_entry->wan_tx_queue_ptr == 0)
        return BDMF_ERR_RANGE;

    wan_tx_queue_descriptor = (RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_1_OFFSET) +
        wan_tx_pointers_entry->wan_tx_queue_ptr - sizeof(RUNNER_COMMON));

    /* modify the packet threshold and profile of the wan channel queue */
    if (profile_id == RDD_QUEUE_PROFILE_DISABLED)
        queue_profile_address = 0;
    else
        queue_profile_address = US_QUEUE_PROFILE_TABLE_ADDRESS + profile_id * sizeof(RDD_QUEUE_PROFILE_DTS);

    RDD_WAN_TX_QUEUE_DESCRIPTOR_PACKET_THRESHOLD_WRITE(packet_threshold, wan_tx_queue_descriptor);
    RDD_WAN_TX_QUEUE_DESCRIPTOR_PROFILE_PTR_WRITE(queue_profile_address, wan_tx_queue_descriptor);

    return BDMF_ERR_OK;
}

int rdd_wan_tx_queue_remove(rdd_wan_channel_id_t wan_channel, rdd_rate_cntrl_id_t rate_cntrl,
    rdd_tx_queue_id_t tx_queue)
{
    RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS *rate_cntrl_descriptor;
    RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS *wan_tx_queue_descriptor;
    rdd_wan_tx_pointers_entry_t *wan_tx_pointers_entry;

    /* check the validity of the input parameters - wan channel, Rate controller and queue indexes */
    if (wan_channel > RDD_WAN_CHANNEL_MAX)
        return BDMF_ERR_RANGE;

#if !defined(WL4908)
    if (wan_channel <= RDD_WAN_CHANNEL_7)
    {
        if (rate_cntrl > RDD_RATE_CNTRL_31)
            return BDMF_ERR_RANGE;
    }
    else
    {
        if (rate_cntrl > RDD_RATE_CNTRL_3)
            return BDMF_ERR_RANGE;
    }
#endif

    if (tx_queue > RDD_TX_QUEUE_7)
        return BDMF_ERR_RANGE;

    wan_tx_pointers_entry = &(wan_tx_pointers_table->entry[wan_channel][rate_cntrl][tx_queue]);

    /* verify that the queue was cfgured before */
    if (wan_tx_pointers_entry->wan_tx_queue_ptr == 0)
        return BDMF_ERR_RANGE;

    /* disconnect the queue from the rate controller */
    rate_cntrl_descriptor = (RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_1_OFFSET) +
        wan_tx_pointers_entry->rate_cntrl_ptr - sizeof(RUNNER_COMMON));

    /* packets that will continue to arrive to this queue will be dropped */
    RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_WRITE(DUMMY_WAN_TX_QUEUE_DESCRIPTOR_ADDRESS,
        rate_cntrl_descriptor, tx_queue);

    wan_tx_queue_descriptor = (RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_1_OFFSET) +
        wan_tx_pointers_entry->wan_tx_queue_ptr - sizeof(RUNNER_COMMON));

    RDD_WAN_TX_QUEUE_DESCRIPTOR_QUEUE_MASK_WRITE(0, wan_tx_queue_descriptor);

    wan_tx_pointers_entry->wan_tx_queue_ptr = 0;

    return BDMF_ERR_OK;
}

void rdd_wan_tx_queue_get_status(rdd_wan_channel_id_t wan_channel, rdd_rate_cntrl_id_t rate_cntrl,
    rdd_tx_queue_id_t tx_queue, 
#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908)
    uint16_t *number_of_packets
#else
    rdpa_stat_1way_t *stat 
#endif
				 )
{
    RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS *wan_tx_queue_descriptor;
    rdd_wan_tx_pointers_entry_t *wan_tx_pointers_entry;

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    int rdd_error = BDMF_ERR_OK;
    uint16_t discarded_packets;
#endif

    /* find dynamically allocated queue and read its size */
    wan_tx_pointers_entry = &(wan_tx_pointers_table->entry[wan_channel][rate_cntrl][tx_queue]);

    wan_tx_queue_descriptor = (RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_1_OFFSET) +
        wan_tx_pointers_entry->wan_tx_queue_ptr - sizeof(RUNNER_COMMON));

#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908)
    RDD_WAN_TX_QUEUE_DESCRIPTOR_PACKET_COUNTER_READ(*number_of_packets, wan_tx_queue_descriptor);
#else
    RDD_WAN_TX_QUEUE_DESCRIPTOR_INDEX_READ ( tx_queue, wan_tx_queue_descriptor );

    rdd_error = rdd_4_bytes_counter_get ( WAN_TX_QUEUES_PACKETS_GROUP, tx_queue, &stat->passed.packets );
    assert(rdd_error==0);

    rdd_error = rdd_4_bytes_counter_get ( WAN_TX_QUEUES_BYTES_GROUP, tx_queue, &stat->passed.bytes );
    assert(rdd_error==0);

    rdd_error = rdd_2_bytes_counter_get ( WAN_TX_QUEUES_DROPPED_PACKETS_GROUP, tx_queue, &discarded_packets );
    stat->discarded.packets = discarded_packets;
    assert(rdd_error==0);

    rdd_error = rdd_4_bytes_counter_get ( WAN_TX_QUEUES_DROPPED_BYTES_GROUP, tx_queue, &stat->discarded.bytes );
    assert(rdd_error==0);
#endif /*DSL*/
}

int rdd_lan_vport_cfg(rdd_vport_id_t port, rdd_rate_limiter_t rate_limiter)
{
    RDD_ETH_TX_MAC_TABLE_DTS *eth_tx_mac_table;
    RDD_ETH_TX_MAC_DESCRIPTOR_DTS *eth_tx_mac_descriptor;

    /* check the validity of the input parameters - virtual port id */
    if (port < RDD_LAN0_VPORT || port > RDD_LAN_VPORT_LAST)
        return BDMF_ERR_RANGE;

    eth_tx_mac_table = RDD_ETH_TX_MAC_TABLE_PTR();
    eth_tx_mac_descriptor = &(eth_tx_mac_table->entry[port]);

    RDD_ETH_TX_MAC_DESCRIPTOR_RATE_LIMITER_ID_WRITE(rate_limiter, eth_tx_mac_descriptor);

    return BDMF_ERR_OK;
}

int rdd_lan_vport_tx_queue_cfg(rdd_vport_id_t port, rdd_tx_queue_id_t queue_id, uint16_t packet_threshold,
    rdd_queue_profile_id_t profile_id)
{
#ifndef G9991
    RDD_ETH_TX_QUEUES_TABLE_DTS *eth_tx_queues_table;
    RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS *eth_tx_queue_descriptor;
#else
    RDD_ETH_TX_QUEUES_POINTERS_TABLE_DTS *eth_tx_queues_pointers_table;
    RDD_ETH_TX_QUEUE_POINTERS_ENTRY_DTS *eth_tx_queue_pointers_entry;
    RDD_DDR_QUEUE_DESCRIPTOR_DTS *eth_tx_queue_descriptor;
    uint16_t eth_tx_queue_descriptor_offset;
#endif
    uint16_t queue_profile_address;

    /* check the validity of the input parameters - virtual port id */
    if (port < RDD_LAN0_VPORT || port > RDD_LAN_VPORT_LAST)
        return BDMF_ERR_PARM;

    /* check the validity of the input parameters - port tx queue id */
    if (queue_id > RDD_TX_QUEUE_LAST)
        return BDMF_ERR_PARM;

#ifndef G9991
    eth_tx_queues_table = RDD_ETH_TX_QUEUES_TABLE_PTR();
    eth_tx_queue_descriptor = &(eth_tx_queues_table->entry[(port - RDD_LAN0_VPORT) * RDD_EMAC_NUMBER_OF_QUEUES + queue_id]);
#else
    eth_tx_queues_pointers_table = RDD_ETH_TX_QUEUES_POINTERS_TABLE_PTR();
    eth_tx_queue_pointers_entry = &(eth_tx_queues_pointers_table->entry[port * RDD_EMAC_NUMBER_OF_QUEUES + queue_id]);

    RDD_ETH_TX_QUEUE_POINTERS_ENTRY_TX_QUEUE_POINTER_READ(eth_tx_queue_descriptor_offset, eth_tx_queue_pointers_entry);

    eth_tx_queue_descriptor = (RDD_DDR_QUEUE_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) +
        eth_tx_queue_descriptor_offset);
#endif

    if (profile_id == RDD_QUEUE_PROFILE_DISABLED)
        queue_profile_address = 0;
    else
    {
        queue_profile_address = DS_QUEUE_PROFILE_TABLE_ADDRESS + profile_id * sizeof(RDD_QUEUE_PROFILE_DTS);
    }

    RDD_ETH_TX_QUEUE_DESCRIPTOR_PACKET_THRESHOLD_WRITE(packet_threshold, eth_tx_queue_descriptor);
    RDD_ETH_TX_QUEUE_DESCRIPTOR_PROFILE_PTR_WRITE(queue_profile_address, eth_tx_queue_descriptor);

#if !defined(RDD_BASIC) && (defined(DSL_63138) || defined(DSL_63148) || defined(WL4908))
    /*Don't update packet descriptors pool sizes in case of RDD_BASIC (CFE). In RDD_BASIC
     *case, the current function is invoked before the Runner cores are woken up, and the
     *rdd_tm_ds_free_packet_descriptors_pool_size_update() function below sends a CPU message
     *to apply the pool size changes.*/
    return rdd_tm_ds_free_packet_descriptors_pool_size_update();
#else
    return BDMF_ERR_OK;
#endif
}

int rdd_lan_vport_tx_queue_status_get(rdd_vport_id_t port, rdd_tx_queue_id_t queue_id,
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    rdpa_stat_1way_t *stat 
#else
    uint16_t *number_of_packets
#endif
)
{
    RDD_ETH_TX_QUEUES_TABLE_DTS *eth_tx_queues_table;
    RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS *eth_tx_queue_descriptor;

    /* check the validity of the input parameters - virtual port id */
    if (port < RDD_LAN0_VPORT || port > RDD_LAN_VPORT_LAST)
        return BDMF_ERR_PARM;

    /* check the validity of the input parameters - port tx queue id */
    if (queue_id > RDD_TX_QUEUE_LAST)
        return BDMF_ERR_PARM;

    eth_tx_queues_table = RDD_ETH_TX_QUEUES_TABLE_PTR();
    eth_tx_queue_descriptor = &(eth_tx_queues_table->entry[(port - RDD_LAN0_VPORT) * RDD_EMAC_NUMBER_OF_QUEUES + queue_id]);

#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908)
    RDD_ETH_TX_QUEUE_DESCRIPTOR_INGRESS_PACKET_COUNTER_READ(*number_of_packets, eth_tx_queue_descriptor);
#else
    {    
      uint8_t  tx_queue;
      uint16_t discarded_packets;
      int rdd_error;

      RDD_ETH_TX_QUEUE_DESCRIPTOR_INDEX_READ ( tx_queue, eth_tx_queue_descriptor );
    
      rdd_error = rdd_4_bytes_counter_get ( LAN_TX_QUEUES_PACKETS_GROUP, tx_queue, &stat->passed.packets );
      if ( rdd_error != BDMF_ERR_OK )
        return ( rdd_error );

      rdd_error = rdd_4_bytes_counter_get ( LAN_TX_QUEUES_BYTES_GROUP, tx_queue, &stat->passed.bytes );
      if ( rdd_error != BDMF_ERR_OK )
        return ( rdd_error );

      rdd_error = rdd_2_bytes_counter_get ( LAN_TX_QUEUES_DROPPED_PACKETS_GROUP, tx_queue, &discarded_packets );
      stat->discarded.packets = discarded_packets;

      if ( rdd_error != BDMF_ERR_OK )
        return ( rdd_error );

      rdd_error = rdd_4_bytes_counter_get ( LAN_TX_QUEUES_DROPPED_BYTES_GROUP, tx_queue, &stat->discarded.bytes );
    }
#endif /*DSL*/
    return 0;
}

void rdd_eth_tx_ddr_queue_addr_cfg(rdd_emac_id_t emac_id, rdd_tx_queue_id_t queue_id, uint32_t ddr_addr, uint16_t queue_size, uint8_t counter_id)
{
#ifdef G9991
    RDD_G9991_DDR_QUEUE_ADDRESSES_TABLE_DTS *table;

    table = (RDD_G9991_DDR_QUEUE_ADDRESSES_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) +
        G9991_DDR_QUEUE_ADDRESSES_TABLE_ADDRESS);

    /* set DDR q base address for each queue */
    ddr_addr = ddr_addr & 0x1FFFFFFF;
    MWRITE_32(&(table->entry[(emac_id * 4) + queue_id]), ddr_addr);
#endif
}

void rdd_queue_profile_cfg(rdpa_traffic_dir direction, rdd_queue_profile_id_t profile_id,
    rdd_queue_profile_t *queue_profile)
{
    RDD_DS_QUEUE_PROFILE_TABLE_DTS *ds_queue_profile_table;
    RDD_US_QUEUE_PROFILE_TABLE_DTS *us_queue_profile_table;
    RDD_QUEUE_PROFILE_DTS *queue_profile_ptr;
    uint32_t max_drop_probability;
    uint32_t low_red_interval_length;
    uint32_t high_red_interval_length;

    if (direction == rdpa_dir_ds)
    {
        ds_queue_profile_table = RDD_DS_QUEUE_PROFILE_TABLE_PTR();
        queue_profile_ptr = &(ds_queue_profile_table->entry[profile_id]);
    }
    else
    {
        us_queue_profile_table = RDD_US_QUEUE_PROFILE_TABLE_PTR();
        queue_profile_ptr = &(us_queue_profile_table->entry[profile_id]);
    }

    RDD_QUEUE_PROFILE_US_FLOW_CONTROL_MODE_WRITE(queue_profile->us_flow_control_mode, queue_profile_ptr);
    RDD_QUEUE_PROFILE_MAX_LOW_THRESHOLD_WRITE(queue_profile->low_priority_class.max_threshold, queue_profile_ptr);
    RDD_QUEUE_PROFILE_MIN_HIGH_THRESHOLD_WRITE(queue_profile->high_priority_class.min_threshold, queue_profile_ptr);
    RDD_QUEUE_PROFILE_MAX_HIGH_THRESHOLD_WRITE(queue_profile->high_priority_class.max_threshold, queue_profile_ptr);

    max_drop_probability = queue_profile->max_drop_probability * 65535 / 100;
    low_red_interval_length = queue_profile->low_priority_class.max_threshold -
        queue_profile->low_priority_class.min_threshold;

    high_red_interval_length = queue_profile->high_priority_class.max_threshold -
        queue_profile->high_priority_class.min_threshold;

    if (low_red_interval_length > 0)
    {
        RDD_QUEUE_PROFILE_LOW_DROP_CONSTANT_WRITE(max_drop_probability / low_red_interval_length, queue_profile_ptr);
        RDD_QUEUE_PROFILE_LOW_LARGE_INTERVAL_FLAG_WRITE(((low_red_interval_length > 0xFF) ? 1 : 0),
            queue_profile_ptr);
    }

    if (high_red_interval_length > 0)
    {
        RDD_QUEUE_PROFILE_HIGH_DROP_CONSTANT_WRITE(max_drop_probability / high_red_interval_length, queue_profile_ptr);
        RDD_QUEUE_PROFILE_HIGH_LARGE_INTERVAL_FLAG_WRITE(((high_red_interval_length > 0xFF) ? 1 : 0),
            queue_profile_ptr);
    }
}

void rdd_drop_precedence_cfg(rdpa_traffic_dir direction, uint16_t eligibility_vector)
{
    RDD_SYSTEM_CONFIGURATION_DTS *system_cfg;

    if (direction == rdpa_dir_ds)
    {
        system_cfg = (RDD_SYSTEM_CONFIGURATION_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_SYSTEM_CONFIGURATION_ADDRESS);

        RDD_SYSTEM_CONFIGURATION_DROP_PRECEDENCE_ELIGIBILITY_VECTOR_WRITE(eligibility_vector, system_cfg);
    }
    else
    {
        system_cfg = (RDD_SYSTEM_CONFIGURATION_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_SYSTEM_CONFIGURATION_ADDRESS);

        RDD_SYSTEM_CONFIGURATION_DROP_PRECEDENCE_ELIGIBILITY_VECTOR_WRITE(eligibility_vector, system_cfg);
    }
}

int rdd_mdu_mode_pointer_get(rdd_emac_id_t emac_id, uint16_t *mdu_mode_ptr)
{
    if (emac_id < RDD_EMAC_ID_LAN_START || emac_id >= RDD_EMAC_ID_COUNT)
        return BDMF_ERR_RANGE;

    *mdu_mode_ptr = ETH_TX_MAC_TABLE_ADDRESS + RDD_EMAC_EGRESS_COUNTER_OFFSET +
#if !defined(WL4908)
        sizeof(RDD_ETH_TX_MAC_DESCRIPTOR_DTS) * (g_broadcom_switch_mode ? g_broadcom_switch_physical_port : emac_id);
#else
        emac_id * sizeof(RDD_ETH_TX_MAC_DESCRIPTOR_DTS);
#endif

    return 0;
}

int rdd_wan_tx_queue_flush(rdd_wan_channel_id_t wan_channel_id, rdd_rate_cntrl_id_t rate_controller_id,
    rdd_tx_queue_id_t queue_id, bdmf_boolean is_wait)
{
    rdd_wan_tx_pointers_entry_t *wan_tx_pointers_entry_ptr;
    unsigned long flags;
    int rdd_error = 0;

    /* check the validity of the input parameters - wan channel and queue index */
    if (wan_channel_id > RDD_WAN_CHANNEL_MAX)
        return BDMF_ERR_PARM;

#if !defined(WL4908)
    if (wan_channel_id <= RDD_WAN_CHANNEL_7)
    {
        if (rate_controller_id > RDD_RATE_CNTRL_31)
            return BDMF_ERR_PARM;
    }
    else
    {
        if (rate_controller_id > RDD_RATE_CNTRL_3)
            return BDMF_ERR_PARM;
    }
#endif
    if (queue_id > RDD_TX_QUEUE_7)
        return BDMF_ERR_PARM;

    wan_tx_pointers_entry_ptr = &(wan_tx_pointers_table->entry[wan_channel_id][rate_controller_id][queue_id]);

    /* verify that the queue was configured before */
    if (wan_tx_pointers_entry_ptr->wan_tx_queue_ptr == 0)
        return BDMF_ERR_NOENT;

    bdmf_fastlock_lock(&cpu_message_lock);

    rdd_error = rdd_cpu_tx_send_message(RDD_CPU_TX_MESSAGE_FLUSH_WAN_TX_QUEUE, FAST_RUNNER_B, RUNNER_PRIVATE_1_OFFSET,
        ((wan_tx_pointers_entry_ptr->wan_tx_queue_ptr - WAN_TX_QUEUES_TABLE_ADDRESS) / sizeof(RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS)), 0, 0, is_wait);
    bdmf_fastlock_unlock(&cpu_message_lock);

    return rdd_error;
}

int rdd_eth_tx_queue_flush(rdd_vport_id_t port, rdd_tx_queue_id_t queue_id, bdmf_boolean is_wait)
{
    int  rdd_error;

    /* check the validity of the input parameters - virtual port id */
    if (port < RDD_LAN0_VPORT || port > RDD_LAN_VPORT_LAST)
        return BDMF_ERR_RANGE;

    /* check the validity of the input parameters - emac tx queue id */
    if (queue_id > RDD_TX_QUEUE_LAST)
        return BDMF_ERR_PARM;

    bdmf_fastlock_lock(&cpu_message_lock);

    rdd_error = rdd_cpu_tx_send_message(RDD_CPU_TX_MESSAGE_FLUSH_ETH_QUEUE, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, port, queue_id, 0, is_wait);

    bdmf_fastlock_unlock(&cpu_message_lock);
    return rdd_error;
}

int rdd_flow_control_send_xon(rdd_vport_id_t vport)
{
    int rdd_error;

    if ((vport < RDD_LAN0_VPORT) || (vport > RDD_LAN4_VPORT))
        return BDMF_ERR_PARM;

    bdmf_fastlock_lock(&cpu_message_lock);

    rdd_error = rdd_cpu_tx_send_message(RDD_CPU_TX_MESSAGE_SEND_XON_FRAME, FAST_RUNNER_B, RUNNER_PRIVATE_1_OFFSET, vport, 0, 0, 1);

    bdmf_fastlock_unlock(&cpu_message_lock);

    return rdd_error;
}

int rdd_wan_channel_byte_counter_read(rdd_wan_channel_id_t wan_channel_id, uint32_t *byte_counter)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int rdd_free_packet_descriptors_pool_size_get ( uint32_t  *downstream_size,
                                                uint32_t  *upstream_size )
{
    *downstream_size = RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_SIZE;
    *upstream_size = RDD_US_FREE_PACKET_DESCRIPTORS_POOL_SIZE;

    return ( BDMF_ERR_OK );

}

#if !defined(RDD_BASIC) && (defined(DSL_63138) || defined(DSL_63148) || defined(WL4908))
/*This function retrieves the number of DS queues configured by RDPA*/
static uint32_t rdd_tm_calc_num_ds_queues(void)
{
    uint32_t num_ds_queues = 0;

    RDD_ETH_TX_QUEUES_POINTERS_TABLE_DTS *eth_tx_queues_pointers_table_ptr = 
        ( RDD_ETH_TX_QUEUES_POINTERS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + ETH_TX_QUEUES_POINTERS_TABLE_ADDRESS );
    uint32_t emac;
    uint32_t tx_queue;

    /*It would be faster to maintain this info in a local data structure instead of retrieving it from RDP SRAM, but chose to avoid
     *data redundancy. This function is not performance critical.*/
    for (emac = RDD_LAN0_VPORT; emac <= RDD_LAN_VPORT_LAST; emac++)
    {
        for (tx_queue = 0; tx_queue <= RDD_EMAC_NUMBER_OF_QUEUES; tx_queue ++)
        {
            RDD_ETH_TX_QUEUE_POINTERS_ENTRY_DTS *eth_tx_queue_pointers_entry_ptr = 
                &( eth_tx_queues_pointers_table_ptr->entry[ emac * RDD_EMAC_NUMBER_OF_QUEUES + tx_queue ] );
            uint16_t eth_tx_queue_descriptor_offset;
            RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS *eth_tx_queue_descriptor_ptr;
            uint16_t packet_threshold;

            RDD_ETH_TX_QUEUE_POINTERS_ENTRY_TX_QUEUE_POINTER_READ ( eth_tx_queue_descriptor_offset, eth_tx_queue_pointers_entry_ptr );
            eth_tx_queue_descriptor_ptr = 
                ( RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + eth_tx_queue_descriptor_offset );
            RDD_ETH_TX_QUEUE_DESCRIPTOR_PACKET_THRESHOLD_READ ( packet_threshold, eth_tx_queue_descriptor_ptr );
 
            /*Queues that are configured by RDPA have a greater-than-zero packet_threshold.
             *Unconfigured queues have a packet_threshold value of zero*/
            if (packet_threshold != 0)
            {
                ++num_ds_queues;
            }
        }
    }

    return num_ds_queues;
}

/*The downstream packet descriptor pool is divided into a guaranteed and a non-guaranteed pool. The guaranteed pool guarantees a minimal 
 *amount (DS_FREE_PACKET_DESCRIPTOR_POOL_GUARANTEED_QUEUE_THRESHOLD) of packet descriptors to each DS queue. If a queue fills up beyond this
 *guaranteed amount it gets a packet descriptor from the non-guaranteed pool, if descriptors are available (not guaranteed).
 *The guaranteed pool size is a function of the number of DS queues used in the system. All remaining resource go into the non-guaranteed pool.
 *This function updates the guaranteed and non-guaranteed pool size based on the number of DS queues used.*/
static int rdd_tm_ds_free_packet_descriptors_pool_size_update (void)
{
    int rdd_error = BDMF_ERR_OK;
    uint32_t num_ds_queues = rdd_tm_calc_num_ds_queues();
    uint16_t new_guaranteed_pool_size, new_non_guaranteed_pool_size;
    uint32_t incr=0, delta=0;
    unsigned long flags;

    new_guaranteed_pool_size = (uint16_t)num_ds_queues*DS_FREE_PACKET_DESCRIPTOR_POOL_GUARANTEED_QUEUE_THRESHOLD;
    /*To ensure reasonable behavior in corner cases, the guaranteed pool size can not go below DS_FREE_PACKET_DESCRIPTOR_POOL_MIN_GUARANTEED_POOL_SIZE.*/
    if (new_guaranteed_pool_size < DS_FREE_PACKET_DESCRIPTOR_POOL_MIN_GUARANTEED_POOL_SIZE)
    {
        new_guaranteed_pool_size = DS_FREE_PACKET_DESCRIPTOR_POOL_MIN_GUARANTEED_POOL_SIZE;
    }

    new_non_guaranteed_pool_size = RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_SIZE - new_guaranteed_pool_size;

    /*Runner FW works with a guaranteed/non-guaranteed pool free count which is initialized to the pool size at boot time.
     *To implement pool size changes on a live system, the new pool sizes have to be expressed as delta relative to the old pool sizes
     *so that the current free count can be incremented/decremented with that delta.
     *An alternative would have been to have the Runner work with an allocation count (instead of a free count) and a pool size threshold.
     *This method would simplify implementation of pool size updates, but would also require additional Runner cycles when enqueuing packets.
     *Since this is in the critical path chose the approach that was more efficient in terms of Runner cycles.*/
    if (new_guaranteed_pool_size > ds_free_packet_descriptors_pool_guaranteed_pool_size)
    {
        incr = 1;
        delta = new_guaranteed_pool_size - ds_free_packet_descriptors_pool_guaranteed_pool_size;
    }
    else if (new_guaranteed_pool_size < ds_free_packet_descriptors_pool_guaranteed_pool_size)
    {
        incr = 0;
        delta = ds_free_packet_descriptors_pool_guaranteed_pool_size - new_guaranteed_pool_size;
    }
  
    /*Only update if there's actually a change*/
    if (delta!=0)
    {
        /*Pool size increments/decrements are non-atomic operations. Doing this from the host CPU would create a race condition with
         *the Runner A Pico core operating on the same variables. To avoid this issue we send a message to the cpu_tx_downstream_pico thread.
         *This Pico thread then applies the pool size increment/decrement.*/
        bdmf_fastlock_lock_irq(&int_lock_irq, flags);
        rdd_error = rdd_cpu_tx_send_message ( RDD_CPU_TX_MESSAGE_UPDATE_PD_POOL_QUOTA, PICO_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, 
					      incr, delta, 0, 1/*wait*/ );
        if (rdd_error == BDMF_ERR_OK)
        {
            ds_free_packet_descriptors_pool_guaranteed_pool_size = new_guaranteed_pool_size;
            ds_free_packet_descriptors_pool_non_guaranteed_pool_size = new_non_guaranteed_pool_size;
        }
        bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
    }

    return rdd_error;
}
#endif
