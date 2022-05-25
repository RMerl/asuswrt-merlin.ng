/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :> 
*/


#ifndef _RDPA_CMD_LIST_H_
#define _RDPA_CMD_LIST_H_

/* Packet_header_buffer_size is determined in runner packet buffer assignment xml (project_data_structure.xml)
 *   - for 63146/4912/6855/Gen6:     112 byte  (110 + 2)
 *   - for 63158/xrdp:               160 byte  (98 + 62)
 *   - for 63138/148/4908/rdp_Gen3:  128 byte  (110 + 18)
 * 
 * Packet_header_buffer_size =  Packet_header_max_length + Packet_headroom
 *   The max possible packet header lenght is derived from (SBPM buffer length - SBPM headroom), 
 *   which is (128 byte - 18 byte) = 110 byte. This value can be further reduced due to GPE modifications requiremnet.
 *   - for 63146/4912/6855/Gen6:     110 byte max_pkt_header_length + 2 byte pkt_headroom (make it 4-byte algined)
 *   - for 63158/xrdp:               98 byte max_pkt_header_length + 62 byte pkt_headroom (Required by GPE SRAM_target)
 *   - for 63138/148/4908/rdp_Gen3:  110 byte max_pkt_header_length + 18 byte pkt_headroom (Required by GPE DDR_target)
 * 
 * CMDLIST_SIZE is determined in runner flow context assignment xml (project_data_structure.xml)
 *   Flow context includes: first portion as fix context and second portion as cmdlist byte array.
 *   The CMDLIST_SIZE:
 *     - for 63146/4912:               104 byte  (XPE, only needs 4-byte aligned)
 *     - for 63158/xrdp:               104 byte  (GPE_SRAM_target, 8-byte alignment helps modification performance)
 *     - for 63138/148/4908/rdp_Gen3:  104 byte  (GPE_DDR_target, only needs 4-byte aligned) 
 *     - for 6855/UFC:                 100 byte  (XPE, only needs 4-byte aligned)
 * 
 * CMDLIST_OFFSET is derived from CMDLIST_SIZE
 *   CMDLIST_OFFSET is the starting offset of cmdlist array, hence it starts from:
 *   CMDLIST_OFFSET = Total flow context - CMDLIST_SIZE
 *   - for 63138/148/4908/rdp_Gen3:  NATC downloads 128 byte, so CMDLIST_OFFSET = 128 - CMDLIST_SIZE
 *   - for xrdp_Gen4/5/6:  NATC downloads 124 byte (4byte overhead), so CMDLIST_OFFSET = 124 - CMDLIST_SIZE
 * 
 * RDPA_CMD_LIST_HEADROOM
 *   - for 63138/148/4908/rdp_Gen3:  the amount of headroom in the BPM buffer which is ddr_packet_headroom_size + LILAC_RDD_PACKET_DDR_OFFSET (18 bytes)
 *     ddr_packet_headroom_size = RDPA_DS_LITE_HEADROOM_SIZE (40B in 5.02L07 and 48B in 5.04L0x. Increase 8B for IPv6 dest option hdr)
 */

#if defined(BCM63158)
#define RDPA_CMD_LIST_PACKET_HEADER_MAX_LENGTH   98
#define RDPA_CMD_LIST_PACKET_BUFFER_OFFSET       0
#define RDPA_CMD_LIST_PACKET_HEADER_OFFSET       62
#define RDPA_CMD_LIST_HEADROOM                   RDPA_CMD_LIST_PACKET_HEADER_OFFSET
#define RDPA_CMD_LIST_BASIC_UCAST_LIST_SIZE      104
#elif defined(BCM63146) || defined(BCM4912) || defined(BCM6813) || defined(BCM6855)
#define RDPA_CMD_LIST_PACKET_HEADER_MAX_LENGTH   110
#define RDPA_CMD_LIST_PACKET_BUFFER_OFFSET       0
#define RDPA_CMD_LIST_PACKET_HEADER_OFFSET       2
#if defined(RDP_UFC)
#define RDPA_CMD_LIST_BASIC_UCAST_LIST_SIZE      100
#else
#define RDPA_CMD_LIST_BASIC_UCAST_LIST_SIZE      104
#endif
#define RDPA_CMD_LIST_HEADROOM                   RDPA_CMD_LIST_PACKET_HEADER_OFFSET
#else
#define RDPA_CMD_LIST_PACKET_HEADER_MAX_LENGTH   110
#define RDPA_CMD_LIST_PACKET_BUFFER_OFFSET       0
#define RDPA_CMD_LIST_PACKET_HEADER_OFFSET       18
#define RDPA_CMD_LIST_HEADROOM                   66
#define RDPA_CMD_LIST_BASIC_UCAST_LIST_SIZE      104
#endif

#if defined(XRDP)
#if defined(RDP_UFC)
#define RDPA_CMD_LIST_UCAST_LIST_OFFSET          24
#else
#define RDPA_CMD_LIST_UCAST_LIST_OFFSET          (124 - RDPA_CMD_LIST_BASIC_UCAST_LIST_SIZE)
#endif
#else
#define RDPA_CMD_LIST_UCAST_LIST_OFFSET          (128 - RDPA_CMD_LIST_BASIC_UCAST_LIST_SIZE)
#endif

#if defined(CONFIG_BCM_RDPA_CNTXT_EXT_SUPPORT)
#define RDPA_CMD_LIST_UCAST_LIST_SIZE            (2*RDPA_CMD_LIST_BASIC_UCAST_LIST_SIZE)
#else
#define RDPA_CMD_LIST_UCAST_LIST_SIZE            (RDPA_CMD_LIST_BASIC_UCAST_LIST_SIZE)
#endif
#define RDPA_CMD_LIST_UCAST_LIST_SIZE_32         (RDPA_CMD_LIST_UCAST_LIST_SIZE / 4)

#define RDPA_CMD_LIST_MCAST_L2_LIST_OFFSET       0
#define RDPA_CMD_LIST_MCAST_L2_LIST_SIZE         64
#define RDPA_CMD_LIST_MCAST_L2_LIST_SIZE_32      (RDPA_CMD_LIST_MCAST_L2_LIST_SIZE / 4)

#define RDPA_CMD_LIST_MCAST_L3_LIST_OFFSET       52
#define RDPA_CMD_LIST_MCAST_L3_LIST_SIZE         20
#define RDPA_CMD_LIST_MCAST_L3_LIST_SIZE_32      (RDPA_CMD_LIST_MCAST_L3_LIST_SIZE / 4)


typedef struct {
    uint8_t *rdd_cmd_list_p;        /* Pointer to rdd cmd list array    - Input/Output */
    uint8_t *rdpa_cmd_list_p;       /* Pointer to rdpa cmd list array   - Input/Output */
    uint32_t rdd_cmd_list_max_size; /* Max size of rdd cmd list array   - Input */
    uint32_t rdpa_cmd_list_len;     /* Rdpa cmd list len                - Input */
    uint32_t rdpa_cmd_list_data_len;/* Rdpa cmd list data len           - Input */
    uint32_t rdd_cmd_list_offset;   /* Offset of cmd list array in rdd  - Input */
    uint32_t rdd_cmd_list_len_32;   /* Cmd list len in 32bit words      - Output */
    uint32_t rdd_q_bytes_cnt;       /* q_bytes_cnt for XPE              - Output */
    uint32_t spdsvc:1;              /* SpdSvc Flag                      - Input */
    uint32_t tunnel_header:1;       /* Tunnel header flag               - Input */
    uint32_t rsvd_bits:30;          /* Reserved bits                    - Input */
} cmd_list_update_params_t;

int rdpa_cmd_list_update_context(cmd_list_update_params_t *p, int *overflow);

#endif /* _RDPA_CMD_LIST_H_ */
