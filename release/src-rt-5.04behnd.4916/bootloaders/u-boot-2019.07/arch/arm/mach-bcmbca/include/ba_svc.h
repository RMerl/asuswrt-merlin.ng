/* SPDX-License-Identifier: GPL-2.0+
  *
  * Copyright 2019 Broadcom Ltd.
 */

#ifndef _BA_SVC_H_
#define _BA_SVC_H_

#include <linux/types.h>
#include "itc_rpc.h"

enum ba_svc_func_idx {
    BA_SVC_CPU_ID,
    BA_SVC_CPU_NAME,
    BA_SVC_RUN_STATE_ID,
    BA_SVC_RUN_STATE_NAME,
    BA_SVC_GET_RUN_STATE,
    BA_SVC_NOTIFY_RUN_STATE,
    BA_SVC_REQUEST_RUN_STATE,
    BA_SVC_REQUEST_RUN_STATE_RESPONSE,
    BA_SVC_SET_RUN_STATE,
    BA_SVC_BOOT_FROM_ADDR,
    BA_SVC_XPORT_SET_PWR,
    BA_SVC_RNR_SRAM_DONE,  /* Not implemented yet */
    BA_SVC_WOL_INTR_EN,    /* Not implemented yet */
    BA_SVC_GET_SMCBL_VER,
    BA_SVC_GET_SMCBL_VER_HASH,
    BA_SVC_GET_SMCOS_VER,
    BA_SVC_GET_SMCOS_VER_HASH,
    BA_SVC_RPRT_BOOT_SUCCESS,
    BA_SVC_GET_BOOT_FAIL_CNT,
    BA_SVC_FUNC_MAX
};

enum ba_req_rs_rsp {
	BA_SVC_RESPONSE_READY,
	BA_SVC_RESPONSE_BUSY,
	BA_SVC_RESPONSE_MAX
};

#define BA_SVC_CPU_ALL		"ALL"
#define BA_SVC_CPU_RG		"RG"
#define BA_SVC_CPU_CM		"CM"
#define BA_SVC_CPU_GFAP		"GFAP"
#define BA_SVC_CPU_BNE		"BNE"
#define BA_SVC_CPU_TPMI		"TPMI"

extern uint32_t ba_cpu_all;
extern uint32_t ba_cpu_rg;
extern uint32_t ba_cpu_cm;
extern uint32_t ba_cpu_gfap;
extern uint32_t ba_cpu_bne;
extern uint32_t ba_cpu_tpmi;

#define BA_SVC_RS_OFF		"OFF"
#define BA_SVC_RS_RESET		"RESET"
#define BA_SVC_RS_BOOT		"BOOT"
#define BA_SVC_RS_SHUTDOWN	"SHUTDOWN"
#define BA_SVC_RS_RUNNING	"RUNNING"
#define BA_SVC_RS_READY		"READY"

extern uint32_t ba_rs_off;
extern uint32_t ba_rs_reset;
extern uint32_t ba_rs_boot;
extern uint32_t ba_rs_shutdown;
extern uint32_t ba_rs_running;
extern uint32_t ba_rs_ready;

#define INVALID_ID	(0xffffffff)

struct ba_msg {
	uint32_t	hdr;
	union {
		uint32_t	rsvd0;
		struct {
			uint8_t	cpu_id;
			uint8_t	rs_id;
			union {
				uint8_t	be_rude:1;
				uint8_t	rsvd1:7;

				uint8_t	response:4;
				uint8_t rsvd2:4;
			};
			uint8_t	rc:8;
		};
	};
	union {
		uint32_t	rsvd3[2];
		char		name[8];
	};
};


/* 
    This is configurable parameter in git core.abbrev parameter currently it configure to 9 use more 3 byte for spare
    In case of changing lenght of this parameters need  to change it also in the SMC_OS and SMC_OS
*/
#define HASH_SHORT_SIZE         (12)

typedef struct 
{
    uint16_t       smcos_major_ver;
    uint16_t       smcos_minor_ver;
    uint16_t       smcos_rev;
    uint16_t       pon_major_ver;
    uint16_t       pon_minor_ver;
    uint16_t       pon_patch_ver; 
    char           smcos_ver_hash[HASH_SHORT_SIZE]; 
} smcos_ver_t;

typedef struct 
{
    uint16_t       smcbl_major_ver;
    uint16_t       smcbl_minor_ver;
    uint16_t       smcbl_rev;
    uint16_t       ponbl_major_ver;
    uint16_t       ponbl_minor_ver;
    char           smcbl_ver_hash[HASH_SHORT_SIZE];
} smcbl_ver_t;

#define BA_SVC_RESET_BOOT_WDOG	1
#define BA_SVC_RESET_BOOT_COUNT	2

/* ba_svc rpc message manipulation helpers */
static inline uint8_t ba_svc_msg_get_retcode(rpc_msg *msg)
{
	struct ba_msg *ba_msg = (struct ba_msg *)msg;
	return ba_msg->rc;
}
static inline void ba_svc_msg_set_retcode(rpc_msg *msg, uint8_t v)
{
	struct ba_msg *ba_msg = (struct ba_msg *)msg;
	ba_msg->rc = v;
}

/* ba svc functions */
int ba_svc_cpu_id(char *cpu_name, uint32_t *cpu_id);
int ba_svc_cpu_name(uint32_t cpu_id, char *cpu_name);
int ba_svc_run_state_id(char *rs_name, uint32_t *rs_id);
int ba_svc_run_state_name(uint32_t rs_id, char *rs_name);
int ba_svc_get_run_state(uint32_t cpu_id, uint32_t *rs_id);
int ba_svc_notify_run_state(uint32_t cpu_id, uint32_t rs_id);
int ba_svc_request_run_state(uint32_t cpu_id, uint32_t rs_id, bool be_rude);
int ba_svc_request_run_state_response(uint32_t cpu_id, uint32_t rs_id,
	enum ba_req_rs_rsp response);
int ba_svc_init(void);
int ba_svc_boot_secondary(uint32_t cpu_mask, uint32_t vector);
int ba_xport_set_state(uint8_t port_id, uint8_t enable);
int ba_get_smcbl_ver(smcbl_ver_t  *smcbl_ver);
int ba_get_smcos_ver(smcos_ver_t  *smcos_ver);
int bcm_rpc_ba_report_boot_success(uint32_t flags);
int bcm_rpc_ba_get_boot_fail_cnt(void);

#endif
