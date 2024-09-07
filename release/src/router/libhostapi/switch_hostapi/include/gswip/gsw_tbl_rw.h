/******************************************************************************

   Copyright 2023 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#ifndef _GSW_TBL_RW_H_
#define _GSW_TBL_RW_H_

#include "gsw_types.h"

#pragma pack(push, 1)
#pragma scalar_storage_order little-endian

/** \brief GSWIP PCE Table Programming structure */
typedef struct {
	u16 key[34];
	u16 mask[4];
	u16 val[31];
	u16 table;
	u16 pcindex;
	u16 op_mode: 2;
	u16 extop: 1;
	u16 kformat: 1;
	u16 type: 1;
	u16 valid: 1;
	u16 group: 4;
} pctbl_prog_t;

/** \brief GSWIP Table structure to access all tables */
typedef struct {
	u32 tbl_entry;
	/** Start offset mem dump purpose */
	u32 tbl_addr;
	u32 tbl_id;
	u32 bm_numValues;
	pctbl_prog_t ptdata;
	//bmtbl_prog_t bmtable;
	//pmtbl_prog_t pmactable;
} GSW_table_t;

/** \brief GSWIP BM Table ID to access different tables */
typedef enum {
	CTP_PORT_RX_RMON 				= 0x00,
	CTP_PORT_TX_RMON 				= 0x01,
	BRIDGE_PORT_RX_RMON 				= 0x02,
	BRIDGE_PORT_TX_RMON				= 0x03,
	CTP_PORT_PCE_BYPASS_TX_RMON			= 0x04,
	FLOW_RX_RMON					= 0x05,
	FLOW_TX_RMON					= 0x06,
	WFQ_PARAM					= 0x08,
	PQM_THRESHOLD					= 0x09,
	PQM_PACKET_PTR					= 0x0A,
	SSL_NEXT_PTR_MEM				= 0x0B,
	SSL_HEADER_DES_MEM1				= 0x0C,
	SSL_HEADER_DES_MEM2				= 0x0D,
	BUF_MGR_Q_MAP_TABLE				= 0x0E,
	METER_RMON_COUNTER				= 0x19,
	ROUTING_RMON_COUNTER				= 0x1B,
	PMAC_RMON_COUNTER				= 0x1C,
} BM_Table_ID;

/** \brief GSWIP BM Table Address */
typedef union {
	u16 raw;
	struct {
		u16 b0: 1, b1: 1, b2: 1, b3: 1, b4: 1, b5: 1, b6: 1, b7: 1,
		    b8: 1, b9: 1, b10: 1, b11: 1, b12: 1, b13: 1, b14: 1, b15: 1;
	} bits;
	struct {
		u16 counterOffset: 6, portOffset: 10;
	} rmon;
	struct {
		u16 counterOffset: 4, counterpoint: 2, portOffset: 10;
	} vlanrmon;
	struct {
		u16 nQueueId: 7, reserved0: 9;
	} wfq;
	struct {
		u16 color_or_submode: 2, mode: 1, nQueueId: 7, reserved1: 6;
	} pqmThr;
	struct {
		u16 ptr: 11, reserved2: 5;
	} pqmPtr;
	struct {
		u16 ptr: 10, reserved3: 6;
	} ssl;
	struct {
		u16 nQueueId: 7, reserved4: 9;
	} qMapTbl;
	struct {
		u16 meterNo: 7, reserved5: 1, color: 2, reserved6: 6;
	} meterRmon;
	struct {
		u16 portNo: 4, counterType: 4, reserved7: 8;
	} routingRmon;
	struct {
		u16 channel_or_port: 5, count: 3, pmacNo: 3, reserved8: 5;
	} pmacRmon;
} BM_Table_Address;

/** \brief GSWIP BM Table programming structure */
typedef struct {
	BM_Table_ID tableID;
	BM_Table_Address adr;
	u16 value[10];
	gsw_bool_t b64bitMode;
	u8 numValues;
} bmtbl_prog_t;

/** \brief GSWIP PMAC Table programming structure */
typedef struct {
	u16 val[8];
	u16 ptaddr;
	u16 ptcaddr;
	u16 op_mode;
	u16 pmacId;
} pmtbl_prog_t;

int gsw_pce_table_write(const GSW_Device_t *dev, pctbl_prog_t *ptdata);
int gsw_pce_table_read(const GSW_Device_t *dev, pctbl_prog_t *ptdata);
int gsw_pce_table_read_next_valid(const GSW_Device_t *dev, pctbl_prog_t *ptdata);
int gsw_pce_table_read_next_change(const GSW_Device_t *dev, pctbl_prog_t *ptdata);
int gsw_pce_table_key_write(const GSW_Device_t *dev, pctbl_prog_t *ptdata);
int gsw_pce_table_key_read(const GSW_Device_t *dev, pctbl_prog_t *ptdata);
int gsw_pmac_table_read(const GSW_Device_t *dev, pmtbl_prog_t *ptdata);
int gsw_pmac_table_write(const GSW_Device_t *dev, pmtbl_prog_t *ptdata);
GSW_return_t gsw_bm_table_read(const GSW_Device_t *dev, bmtbl_prog_t *ptdata);
GSW_return_t gsw_bm_table_write(const GSW_Device_t *dev, bmtbl_prog_t *ptdata);

#pragma scalar_storage_order default
#pragma pack(pop)

#endif /* _GSW_TBL_RW_H_ */
