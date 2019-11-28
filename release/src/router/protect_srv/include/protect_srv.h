 /*
 * Copyright 2017, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#ifndef __protect_srv_h__
#define __protect_srv_h__

/* SOCKET SERVER DEFINE SETTING 
---------------------------------*/
#define PROTECT_SRV_MAX_SOCKET_CLIENT         5
#define PTCSRV_PTHREAD_STACK_SIZE             0x100000
#define PROTECT_SRV_SOCKET_PATH               "/var/run/protect_srv_socket"
#define PROTECT_SRV_PID_PATH                  "/var/run/protect_srv.pid"
#define PROTECT_SRV_LOG_FILE                  "/tmp/protect_srv.log"
#define PROTECT_SRV_DEBUG                     "/tmp/PTCSRV_DEBUG"

/*  IPTABLES SETTING 
---------------------------------*/
#define PROTECT_SRV_RULE_CHAIN                "PTCSRV"
#define PROTECT_SRV_RULE_FILE                 "/tmp/ipt_protectSrv_rule"

typedef enum {
	PROTECTION_SERVICE_SSH=0,
	PROTECTION_SERVICE_TELNET
} PTCSRV_SVC_TYPE_T;

typedef enum {
	UNSTATUS=-1,
	RPT_FAIL=0,
	RPT_SUCCESS
} RPT_STATE_T;

/* NOTIFY PROTECTION STATE REPORT STRUCTURE
---------------------------------*/
typedef struct __ptcsrv_state_report__t_
{
	int        s_type;             /* Service type */
	int        status;             /* Login succeeded/failed */
	char       addr[64];           /* Record address */
	char       msg[256];           /* Info */

} PTCSRV_STATE_REPORT_T;

typedef enum {
	PTCSRV_S_RPT=0,
	PTCSRV_G_WAN_STAT,
	PTCSRV_G_LAN_STAT,
	PTCSRV_G_WAN_RECORD,
	PTCSRV_G_LAN_RECORD,

} PTCSRV_DATA_TYPE_T;

typedef struct __ptcsrv_sock_data_t_
{
	int        d_type;             /* Data type */
	union {
	PTCSRV_STATE_REPORT_T report;  /* State report */
	PTCSRV_SVC_TYPE_T s_type;      /* Service type */
	};

} PTCSRV_SOCK_DATA_T;

typedef struct __ptcsrv_stat_data_t_
{
	int        lock_rule_cnt;      /* current rule count */
	int        lock_rule_peak;     /* peak rule count */
	int        record_cnt;         /* current ip record count */

} PTCSRV_STAT_DATA_T;

typedef struct __ptcsrv_login_record_t_ {
	char       addr[16];           /* Record address */
	size_t     fail_cnt;           /* Retry counter */
	long       fail_time;          /* latest failed time of this address */
	long       lock_time;          /* start time of locking this address */
	int        locked;             /* status of locking rule */
	int        lock_it;            /* the address should to be locked */
} PTCSRV_LOGIN_RECORD_T;

#endif
