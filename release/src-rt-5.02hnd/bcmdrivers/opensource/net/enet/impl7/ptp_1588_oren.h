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
#ifndef _PTP_1588_OREN_H
#define _PTP_1588_OREN_H

#define PTP_1588_EVENT_PORT    319
#define PTP_1588_GENERAL_PORT  320
#define CLOCK_IDENTITY_LENGTH 8

#pragma pack(push, 1)
struct sourcePortIdentity {
    char clockIdentity[CLOCK_IDENTITY_LENGTH];
    __be16 portNumber;
}; 

struct Timestamp {
    uint16_t seconds_msb;
    uint32_t seconds_lsb;
    uint32_t nanoseconds;
};

struct sync_msg {
    struct Timestamp originTimestamp;
};

struct follow_up_msg {
    struct Timestamp preciseOriginTimestamp;
    uint8_t suffix[0];
};

struct delay_resp_msg {
    struct Timestamp receiveTimestamp;
    struct sourcePortIdentity requestingPortIdentity;
    uint8_t suffix[0];
};

struct delay_req_msg {
    struct Timestamp originTimestamp;
};

struct ptphdr {
    __be16 transportSpecific      : 4;
    __be16 messageType            : 4;
    __be16 versionPTP             : 8;
    __be16 messageLength;
    __be16 domainNumber           : 8;
    __be16 reserved1              : 8;
    __be16 flagField;
    uint64_t correctionfield;
    __be32 reserved2;
    struct sourcePortIdentity portID;
    __be16 sequenceId;
    __be16 controlField : 8;
    __be16 logMessageInterval :8;
    union {
        struct sync_msg sync;
        struct follow_up_msg follow_up;
        struct delay_resp_msg delay_resp;
        struct delay_req_msg delay_req;
    };
};

union time_of_day
{
    struct
    {
        uint64_t sec            : 34;        
        uint64_t nanosec        : 30;
    };
    struct
    {
        uint32_t tod_high;
        uint32_t tod_low;
    };
};

#pragma pack(pop)

struct ptp_1588_list {
    struct list_head list;
    bdmf_sysb sysb; 
    rdpa_cpu_tx_info_t info; 
    char *ptp_header; 
};

enum {
    PTP_1588_SYNC = 0,
    PTP_1588_DELAY_REQ,
    PTP_1588_PDELAY_REQ,
    PTP_1588_PDELAY_RESP,
    PTP_1588_FOLLOW_UP = 8,
    PTP_1588_DELAY_RESP,
    PTP_1588_PDELAY_RESP_FOLLOW_UP,
    PTP_1588_ANNOUNCE,
    PTP_1588_SIGNALING,
    PTP_1588_MANAGEMENT,
};

enum {
    PTP_1588_TOD_LOW,
    PTP_1588_TOD_HIGH,
    PTP_1588_TOD_LAST,
};

enum {
    MASTER_PORT_EMAC0 = 1,
}; /* taken from MS1588.MASTER.REGS.M_CFG.TXSTRBSEL */

enum
{
    SERIAL_NUMBER_FW_TABLE_SIZE = 16,
    SERIAL_NUMBER_HW_TABLE_SIZE = 256,
    SERIAL_NUMBER_HW_INVALID_VALUE, 
    SERIAL_NUMBER_HW_TABLE_LAST = SERIAL_NUMBER_HW_INVALID_VALUE,
};

#define PTP_POP_CMD(_sn) \
    do { \
        volatile MS1588_MASTER_REGS_M_TSPOPCMD __tspop = {.tsfifopop = 1}; \
        volatile MS1588_MASTER_REGS_M_TSVALID __ts_sn; \
        MS1588_MASTER_REGS_M_TSPOPCMD_WRITE(__tspop); \
        MS1588_MASTER_REGS_M_TSVALID_READ(__ts_sn); \
        if (__ts_sn.tsvalid) \
            _sn = __ts_sn.sn; \
        else \
            _sn = SERIAL_NUMBER_HW_INVALID_VALUE; \
    } while (0)

#define DEC_AND_REMAIN(x) (x-1+SERIAL_NUMBER_HW_TABLE_SIZE)%SERIAL_NUMBER_HW_TABLE_SIZE
#define INC_AND_REMAIN(x) (x+1)%SERIAL_NUMBER_HW_TABLE_SIZE
#define CAST_2_CHAR(str)  (char *)(str)
/* checksum is 0 staticly. */
#define UPDATE_UDP_CHECKSUM(ptp_ptr, ptp_over_eth) \
    do { \
        if (!ptp_over_eth) \
        { \
            struct udphdr *uh = (struct udphdr *)(CAST_2_CHAR(ptp_ptr) - sizeof(struct udphdr)); \
            uh->check=0; \
            cache_flush_len(&uh->check, sizeof(uh->check)); \
	} \
    } while(0)

#define IS_ENC_TYPE_L2(ptp_header, l2_header) (((void*)ptp_header - l2_header) == ETH_HLEN)
#endif

