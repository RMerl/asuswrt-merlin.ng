/*
 * Copyright 2014 Trend Micro Incorporated
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software without 
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 */

#ifndef TDTS_CORE_API_H_
#define TDTS_CORE_API_H_

/*!
 * \internal
 *
 * \note Do not use.
 */
#define SW_NONE  0x0000
#define SW_IPS   (1 << 0)
#define SW_APP   (1 << 1)
#define SW_AV    (1 << 2)
#define SW_DEVID (1 << 3)
#define SW_SELF  (1 << 4)
#define SW_ADP   (1 << 5)
#define SW_SSL_DECRYPTED (1 << 6)
#define SW_URL_QUERY     (1 << 7)
#define SW_CC_QUERY      (1 << 8)

//----------------------------------------------------------------------------
// The information for requests.

#define TDTS_REQ_FLAG_IPS    SW_IPS
#define TDTS_REQ_FLAG_APPID  SW_APP
#define TDTS_REQ_FLAG_AV     SW_AV
#define TDTS_REQ_FLAG_DEVID  SW_DEVID
#define TDTS_REQ_FLAG_ADP    SW_ADP
#define TDTS_REQ_FLAG_SELF   SW_SELF
#define TDTS_REQ_FLAG_SSL    SW_SSL_DECRYPTED
#define TDTS_REQ_FLAG_URL_QUERY SW_URL_QUERY
#define TDTS_REQ_FLAG_CC_QUERY SW_CC_QUERY

#define TDTS_RES_TYPE_IPS    SW_IPS
#define TDTS_RES_TYPE_APPID  SW_APP
#define TDTS_RES_TYPE_AV     SW_AV
#define TDTS_RES_TYPE_DEVID  SW_DEVID
#define TDTS_RES_TYPE_ADP    SW_ADP
#define TDTS_RES_TYPE_SELF   SW_SELF
#define TDTS_RES_TYPE_SSL    SW_SSL_DECRYPTED
#define TDTS_RES_TYPE_URL_QUERY SW_URL_QUERY
#define TDTS_RES_TYPE_CC_QUERY SW_CC_QUERY

/*!
 * \internal
 *
 * \note Do not use.
 */
#define IS_REQ_IPS(__sw)   ((__sw)->req_flag & TDTS_REQ_FLAG_IPS)
#define IS_REQ_APP(__sw)   ((__sw)->req_flag & TDTS_REQ_FLAG_APPID)
#define IS_REQ_AV(__sw)    ((__sw)->req_flag & TDTS_REQ_FLAG_AV)
#define IS_REQ_DEVID(__sw) ((__sw)->req_flag & TDTS_REQ_FLAG_DEVID)
#define IS_REQ_DEVID_ONLY(__sw) (((__sw)->req_flag & (TDTS_REQ_FLAG_DEVID|TDTS_REQ_FLAG_IPS|TDTS_REQ_FLAG_APPID|TDTS_REQ_FLAG_AV|TDTS_REQ_FLAG_ADP)) == TDTS_REQ_FLAG_DEVID)

#define IS_REQ_ADP(__sw)  ((__sw)->req_flag & TDTS_REQ_FLAG_ADP)
#define IS_REQ_SELF(__sw) ((__sw)->req_flag & TDTS_REQ_FLAG_SELF)
#define IS_REQ_SSL(__sw)  ((__sw)->req_flag & TDTS_REQ_FLAG_SSL)

#define SET_REQ_IPS(__sw) ((__sw)->req_flag |= SW_IPS)
#define SET_REQ_APP(__sw) ((__sw)->req_flag |= SW_APP)
#define SET_REQ_AV(__sw)  ((__sw)->req_flag |= SW_AV)

//----------------------------------------------------------------------------
/*
 * The information for results.
 *
 * \internal Do not use.
 */

#define SET_RESULT_IPS(__sw)   ((__sw)->type |= SW_IPS)
#define SET_RESULT_DEVID(__sw) ((__sw)->type |= SW_DEVID)
#define SET_RESULT_APP(__sw)   ((__sw)->type |= SW_APP)
#define SET_RESULT_AV(__sw)    ((__sw)->type |= SW_AV)
#define SET_RESULT_ADP(__sw)	((__sw)->type |= SW_ADP)

#define IS_RESULT_IPS(__sw)   ((__sw)->type & SW_IPS)
#define IS_RESULT_DEVID(__sw) ((__sw)->type & SW_DEVID)
#define IS_RESULT_APP(__sw)   ((__sw)->type & SW_APP)

#define SW_FG_FINAL  0x0001	/* final */
#define SW_FG_NOTIA  0x0002	/* no interest (deprecated) */
#define SW_FG_NOINT  SW_FG_NOTIA /* no interest */
#define SW_FG_NOMORE 0x0004	/* no more */

#define IS_FLAGS_FINAL(__sw) ((__sw)->flags & SW_FG_FINAL)
#define IS_FLAGS_NOINT(__sw) ((__sw)->flags & SW_FG_NOINT)
#define IS_FLAGS_NOMORE(__sw) ((__sw)->flags & SW_FG_NOMORE)

#define SET_FLAGS_FINAL(__sw) ((__sw)->flags |= SW_FG_FINAL)
#define SET_FLAGS_NOINT(__sw) ((__sw)->flags |= SW_FG_NOINT)
#define SET_FLAGS_NOMORE(__sw) ((__sw)->flags |= SW_FG_NOMORE)

//----------------------------------------------------------------------------
#define TDTS_DEVID_MAX_HOST_NAME_LEN 32
//----------------------------------------------------------------------------

#if TMCFG_E_CORE_PORT_SCAN_DETECTION
typedef enum tdts_port_scan_type {
	TDTS_SCAN_TYPE_NONE = 0x0,
	TDTS_SCAN_TYPE_TCP_XMAS,
	TDTS_SCAN_TYPE_TCP_SYN,
	TDTS_SCAN_TYPE_TCP_FIN,
	TDTS_SCAN_TYPE_TCP_NULL,
	TDTS_SCAN_TYPE_TCP_UNKNOWN,
	TDTS_SCAN_TYPE_UDP,
	TDTS_SCAN_TYPE_BYPASS,
	TDTS_SCAN_TYPE_TCP_RST_ACK,
} tdts_port_scan_type_t;

typedef struct tdts_port_scan_tholds {
	unsigned short syn_thold;
	unsigned short null_thold;
	unsigned short fin_thold;
	unsigned short xmas_thold;
	unsigned short unknown_thold;
	unsigned short udp_thold;
} tdts_port_scan_tholds_t;
#endif

#if TMCFG_E_CORE_IP_SWEEP_DETECTION
typedef enum tdts_ip_sweep_result {
	TDTS_IP_SWEEP_NO_DETECTION = 0x0,
	TDTS_IP_SWEEP_DETECTED,
} tdts_ip_sweep_result_t;
#endif

#if TMCFG_E_CORE_TCP_CHECKSUM
typedef enum tdts_tcp_checksum_result {
	TDTS_TCP_CHECKSUM_CORRECT = 0x0,
	TDTS_TCP_CHECKSUM_ERROR,
} tdts_tcp_checksum_result_t;
#endif



typedef struct
{
	char *name;     // Attack name
	char *cat_name; // Attack category name

	uint32_t rule_id; // Rule ID

//	unsigned short cat_id; // Attack category ID
	uint16_t cat_id; // Attack category ID
	
	uint8_t proto;    // Protocol
	uint8_t severity; // Severity
} tdts_ips_matching_results_t;

typedef struct
{
#if TMCFG_E_CORE_PORT_SCAN_DETECTION
	tdts_port_scan_type_t pscan_type;
#endif

#if TMCFG_E_CORE_IP_SWEEP_DETECTION
	tdts_ip_sweep_result_t ip_sweep_result;
#endif

#if TMCFG_E_CORE_TCP_CHECKSUM
	tdts_tcp_checksum_result_t tcp_checksum_result;
#endif
} tdts_adp_matching_results_t;

typedef struct
{
	char *cat_name; // Category name
	char *app_name; // Application name
	char *beh_name; // Behavior name

	/*
	 * * behinst:
	 *   8 bit     16 bit     8 bit
	 * +-----------------------------+
	 * | cat id |  app id   | beh id |
	 * +-----------------------------+
	 */
	uint8_t cat_id;  // Category ID
	uint16_t app_id; // Application ID
	uint8_t beh_id;  // Behavior ID

	/* misc */
	uint32_t action; // Recommended action to take.
	uint32_t fwmark; // Firewall mark (deprecated)
} tdts_appid_matching_results_t;


typedef struct
{
#if (0 == TMCFG_E_CORE_RULE_FORMAT_V2)
	uint16_t vendor_id; //!< Vendor ID, e.g. "Microsoft"
	uint16_t name_id;   //!< OS name ID, e.g. "Windows XP"
	uint16_t class_id;  //!< OS class ID, e.g. "Windows Series"
	uint16_t cat_id;    //!< Device Category ID, e.g. "Phone", "TV"
	uint16_t dev_id;    //!< Device Name ID, e.g. "iPhone 4", "Windows Phone"
	uint16_t family_id; //!< Device family ID, e.g. "Handheld family", etc.
#else
	uint32_t devid_info_id;
#endif

	/* It's recommended to pick-up the higher prio rule. */
	uint16_t prio; //!< Priority of matched rule (0: highest prio, 65535: lowest prio).

	unsigned char host_name[TDTS_DEVID_MAX_HOST_NAME_LEN]; //!< Detected device host name in DHCP (if any).
} tdts_devid_matching_results_t;

typedef struct
{
	char *domain;
	unsigned domain_len;
	char *path;
	unsigned path_len;
	char *referer;
	unsigned referer_len;

	char cat[4];
	char score;
	char hook;
	unsigned char *mac;
	unsigned char act;
} tdts_url_matching_results_t;

typedef struct
{
	unsigned short type;
	unsigned short flags;

	int pkt_decoder_verdict;

	tdts_ips_matching_results_t ips;
	tdts_appid_matching_results_t appid;
	tdts_devid_matching_results_t devid;
	tdts_url_matching_results_t url;
	tdts_adp_matching_results_t adp;
} tdts_pkt_matching_results_t;

#define tdts_init_pkt_matching_results_url(__mr) \
	do { \
		(__mr)->url.domain = NULL; \
		(__mr)->url.domain_len = 0; \
		(__mr)->url.path = NULL; \
		(__mr)->url.path_len = 0; \
		(__mr)->url.referer = NULL; \
		(__mr)->url.referer_len = 0; \
	} while (0)

#define tdts_init_pkt_matching_results(_mr) \
	do { \
		(_mr)->type = 0; \
		(_mr)->flags = 0; \
		tdts_init_pkt_matching_results_url(_mr); \
	} while (0)

typedef enum
{
	TDTS_PKT_PARAMETER_PKT_TYPE_NONE = 0,
	TDTS_PKT_PARAMETER_PKT_TYPE_L2_ETHERNET,
	TDTS_PKT_PARAMETER_PKT_TYPE_L3_IP,
	TDTS_PKT_PARAMETER_PKT_TYPE_L3_IP6,
	TDTS_PKT_PARAMETER_PKT_TYPE_MAX
} tdts_pkt_parameter_pkt_type_t;

typedef struct pkt_parameter
{
	/*
	 * Callers' arguments to pass to TDTS.
	 */
	unsigned short req_flag;
	unsigned short reserved;

	tdts_pkt_parameter_pkt_type_t pkt_type;
	void *pkt_ptr;
	unsigned pkt_len;
	unsigned long pkt_time_sec;

	char hook;
	char cat[4];
	struct pkt_parameter *(*async_prepare)(struct pkt_parameter *);
	int (*async_send)(struct pkt_parameter *);

#if 0
#if TMCFG_APP_K_TDTS_HIT_RATE_NFFW
	/* Used to store the memory address of system packet */
	void *sys_pkt_ptr;
#endif
#endif

	void *private_ptr;

#if TMCFG_E_CORE_PORT_SCAN_DETECTION
	void *pscan_ctx;
	tdts_port_scan_tholds_t *pscan_tholds;
#endif
	
#if TMCFG_E_CORE_IP_SWEEP_DETECTION
	void *ip_sweep_ctx;
	unsigned int ip_sweep_thold;
#endif
	
#if TMCFG_E_CORE_TCP_CHECKSUM
	unsigned char tcp_checksum_enable;		// 0: disable, 1: enable
	unsigned char tcp_checksum_error_stop;	// 1: stop progress when tcp checksum error
#endif


	/*
	 * TDTS response for callers to read.
	 */
	tdts_pkt_matching_results_t results;
} tdts_pkt_parameter_t;


/*
 * IPS results.
 */
#define TDTS_PKT_PARAMETER_RES_IPS(_param) (&((_param)->results.ips))

#define TDTS_PKT_PARAMETER_RES_IPS_NAME(__param)     TDTS_PKT_PARAMETER_RES_IPS(__param)->name
#define TDTS_PKT_PARAMETER_RES_IPS_CAT_NAME(__param) TDTS_PKT_PARAMETER_RES_IPS(__param)->cat_name
#define TDTS_PKT_PARAMETER_RES_IPS_RULE_ID(__param)  TDTS_PKT_PARAMETER_RES_IPS(__param)->rule_id
#define TDTS_PKT_PARAMETER_RES_IPS_CAT_ID(__param)   TDTS_PKT_PARAMETER_RES_IPS(__param)->cat_id
#define TDTS_PKT_PARAMETER_RES_IPS_PROTO(__param)    TDTS_PKT_PARAMETER_RES_IPS(__param)->proto
#define TDTS_PKT_PARAMETER_RES_IPS_SEVERITY(__param) TDTS_PKT_PARAMETER_RES_IPS(__param)->severity

/*
 * APPID results
 */
#define TDTS_PKT_PARAMETER_RES_APPID(_param) (&((_param)->results.appid))
#define TDTS_PKT_PARAMETER_RES_APPID_CAT_ID(__param)   TDTS_PKT_PARAMETER_RES_APPID(__param)->cat_id
#define TDTS_PKT_PARAMETER_RES_APPID_CAT_NAME(__param) TDTS_PKT_PARAMETER_RES_APPID(__param)->cat_name
#define TDTS_PKT_PARAMETER_RES_APPID_APP_ID(__param)   TDTS_PKT_PARAMETER_RES_APPID(__param)->app_id
#define TDTS_PKT_PARAMETER_RES_APPID_APP_NAME(__param) TDTS_PKT_PARAMETER_RES_APPID(__param)->app_name
#define TDTS_PKT_PARAMETER_RES_APPID_BEH_ID(__param)   TDTS_PKT_PARAMETER_RES_APPID(__param)->beh_id
#define TDTS_PKT_PARAMETER_RES_APPID_BEH_NAME(__param) TDTS_PKT_PARAMETER_RES_APPID(__param)->beh_name
#define TDTS_PKT_PARAMETER_RES_APPID_ACTION(__param)   TDTS_PKT_PARAMETER_RES_APPID(__param)->action
#define TDTS_PKT_PARAMETER_RES_APPID_FWMARK(__param)   TDTS_PKT_PARAMETER_RES_APPID(__param)->fwmark

#define TDTS_PKT_PARAMETER_RES_APPID_CHECK_FINAL(__param)  IS_FLAGS_FINAL(&((__param)->results))
#define TDTS_PKT_PARAMETER_RES_APPID_CHECK_NOMORE(__param) IS_FLAGS_NOMORE(&((__param)->results))
#define TDTS_PKT_PARAMETER_RES_APPID_CHECK_NOINT(__param)  IS_FLAGS_NOINT(&((__param)->results))

/*
 * DEVID results
 */
#define TDTS_PKT_PARAMETER_RES_DEVID(_param) (&((_param)->results.devid))
#if (0 == TMCFG_E_CORE_RULE_FORMAT_V2)
#define TDTS_PKT_PARAMETER_RES_DEVID_VENDOR_ID(__param)     TDTS_PKT_PARAMETER_RES_DEVID(__param)->vendor_id
#define TDTS_PKT_PARAMETER_RES_DEVID_OS_NAME_ID(__param)    TDTS_PKT_PARAMETER_RES_DEVID(__param)->name_id
#define TDTS_PKT_PARAMETER_RES_DEVID_OS_CLASS_ID(__param)   TDTS_PKT_PARAMETER_RES_DEVID(__param)->class_id
#define TDTS_PKT_PARAMETER_RES_DEVID_DEV_CAT_ID(__param)    TDTS_PKT_PARAMETER_RES_DEVID(__param)->cat_id
#define TDTS_PKT_PARAMETER_RES_DEVID_DEV_ID(__param)        TDTS_PKT_PARAMETER_RES_DEVID(__param)->dev_id
#define TDTS_PKT_PARAMETER_RES_DEVID_DEV_FAMILY_ID(__param) TDTS_PKT_PARAMETER_RES_DEVID(__param)->family_id
#else
#define TDTS_PKT_PARAMETER_RES_DEVID_INFO_ID(__param)       TDTS_PKT_PARAMETER_RES_DEVID(__param)->devid_info_id
#endif
#define TDTS_PKT_PARAMETER_RES_DEVID_PRIO(__param)          TDTS_PKT_PARAMETER_RES_DEVID(__param)->prio
#define TDTS_PKT_PARAMETER_RES_DEVID_HOST_NAME(__param)     TDTS_PKT_PARAMETER_RES_DEVID(__param)->host_name


/*
 * URL-Extraction results
 */
#define TDTS_PKT_PARAMETER_RES_URL(_param) (&((_param)->results.url))
#define TDTS_PKT_PARAMETER_RES_URL_DOMAIN(__param)      TDTS_PKT_PARAMETER_RES_URL(__param)->domain
#define TDTS_PKT_PARAMETER_RES_URL_DOMAIN_LEN(__param)  TDTS_PKT_PARAMETER_RES_URL(__param)->domain_len
#define TDTS_PKT_PARAMETER_RES_URL_PATH(__param)        TDTS_PKT_PARAMETER_RES_URL(__param)->path
#define TDTS_PKT_PARAMETER_RES_URL_PATH_LEN(__param)    TDTS_PKT_PARAMETER_RES_URL(__param)->path_len
#define TDTS_PKT_PARAMETER_RES_URL_REFERER(__param)     TDTS_PKT_PARAMETER_RES_URL(__param)->referer
#define TDTS_PKT_PARAMETER_RES_URL_REFERER_LEN(__param) TDTS_PKT_PARAMETER_RES_URL(__param)->referer_len


/*
 * ADP results
 */
#define TDTS_PKT_PARAMETER_RES_ADP(_param) (&((_param)->results.adp))

static inline unsigned short __attribute__((unused)) tdts_check_pkt_parameter_res(
	const tdts_pkt_parameter_t *pkt_param, unsigned short res_type)
{
	return (pkt_param->results.type & res_type);
}

#if TMCFG_E_CORE_PORT_SCAN_DETECTION
#define TDTS_PORT_SCAN_TCP_SUCCESS		1
#define TDTS_PORT_SCAN_UDP_SUCCESS		2
#define TDTS_PORT_SCAN_DROP			3
#define TDTS_PORT_SCAN_OTHERS			0
#define TDTS_PORT_SCAN_SUCCESS			4

#define tdts_set_pkt_parameter_pscan_ctx(_param, _ctx) \
	do { \
		(_param)->pscan_ctx = _ctx; \
	} while (0)

#define tdts_set_pkt_parameter_pscan_tholds(_param, _tholds) \
	do { \
		(_param)->pscan_tholds = _tholds; \
	} while (0)

#define tdts_init_pkt_parameter_pscan_result(_param) \
	do { \
		(_param)->results.adp.pscan_type = TDTS_SCAN_TYPE_NONE; \
	} while (0)

#define tdts_init_port_scan_parameter(__param) \
	do { \
		tdts_set_pkt_parameter_pscan_ctx(__param, NULL); \
		tdts_set_pkt_parameter_pscan_tholds(__param, NULL); \
		tdts_init_pkt_parameter_pscan_result(__param); \
	} while (0)

typedef void (*port_scan_log_func)(void);

extern int tdts_core_port_scan_log_cb_set(port_scan_log_func cb);
extern void *tdts_core_port_scan_context_alloc(void);
extern int tdts_core_port_scan_context_dealloc(void *ctx);
#else
#define tdts_init_port_scan_parameter(__param)
#endif

#if TMCFG_E_CORE_IP_SWEEP_DETECTION
#define tdts_set_pkt_parameter_ip_sweep_ctx(_param, _ctx) \
	do { \
		(_param)->ip_sweep_ctx = _ctx; \
	} while (0)

#define tdts_set_pkt_parameter_ip_sweep_thold(_param, _thold) \
	do { \
		(_param)->ip_sweep_thold = _thold; \
	} while (0)

#define tdts_init_pkt_parameter_ip_sweep_result(_param) \
	do { \
		(_param)->results.adp.ip_sweep_result = TDTS_IP_SWEEP_NO_DETECTION; \
	} while (0)

#define tdts_init_ip_sweep_parameter(__param) \
	do { \
		tdts_set_pkt_parameter_ip_sweep_ctx(__param, NULL); \
		tdts_set_pkt_parameter_ip_sweep_thold(__param, 0); \
		tdts_init_pkt_parameter_ip_sweep_result(__param); \
	} while (0)

typedef void (*ip_sweep_log_func)(void);

extern int tdts_core_ip_sweep_log_cb_set(ip_sweep_log_func cb);
extern void *tdts_core_ip_sweep_context_alloc(void);
extern int tdts_core_ip_sweep_context_dealloc(void *ctx);
#else
#define tdts_init_ip_sweep_parameter(__param)
#endif

#if TMCFG_E_CORE_TCP_CHECKSUM

#define tdts_set_pkt_parameter_tcp_chksum_enable(_param, _enable) \
	do { \
		(_param)->tcp_checksum_enable = _enable; \
	} while (0)

#define tdts_set_pkt_parameter_tcp_chksum_stop(_param, _stop) \
	do { \
		(_param)->tcp_checksum_error_stop = _stop; \
	} while (0)

#define tdts_init_pkt_parameter_tcp_chksum_result(_param) \
	do { \
		(_param)->results.adp.tcp_checksum_result = TDTS_TCP_CHECKSUM_CORRECT; \
	} while (0)

#define tdts_init_tcp_chksum_parameter(__param) \
	do { \
		tdts_set_pkt_parameter_tcp_chksum_enable(__param, 0); \
		tdts_set_pkt_parameter_tcp_chksum_stop(__param, 0); \
		tdts_init_pkt_parameter_tcp_chksum_result(__param); \
	} while (0)
#else
#define tdts_init_tcp_chksum_parameter(__param)
#endif



/* req flag */
#define tdts_set_pkt_parameter_req_flag(__param, __req_flag) \
	do { \
		(__param)->req_flag = __req_flag; \
	} while (0)
#define tdts_get_pkt_parameter_req_flag(__param) ((__param)->req_flag)

/* pkt time */
#define tdts_set_pkt_parameter_pkt_time(__param, __sec) \
	do { \
		(__param)->pkt_time_sec = (unsigned long) (__sec); \
	} while (0)
#define tdts_get_pkt_parameter_pkt_time(__param, __sec) ((__param)->pkt_time_sec)

/* pkt param */
#define tdts_set_pkt_parameter(_param, _pkt, _pkt_len, _pkt_type) \
	do { \
		(_param)->pkt_type = _pkt_type; \
		(_param)->pkt_ptr = (void *) (_pkt); \
		(_param)->pkt_len = _pkt_len; \
		tdts_init_pkt_matching_results(&((_param)->results)); \
	} while (0)

/*!
 * \brief Initialize TDTS input packet parameters.
 * \param ___param A pointer to tdts_pkt_parameter_t
 * \param ___req_flag A bitmask value of TDTS_REQ_FLAG_*, e.g. TDTS_REQ_FLAG_APPID | TDTS_REQ_FLAG_DEVID
 * The flag is used to inform TDTS engine to do the specified tasks.
 * \param ___pkt_time Packet timestamp (in seconds). Just initialize as 0 if you don't need to implement local time-ticks.
 */
#define tdts_init_pkt_parameter(___param, ___req_flag, ___pkt_time) \
	do { \
		tdts_set_pkt_parameter(___param, NULL, 0, TDTS_PKT_PARAMETER_PKT_TYPE_NONE); \
		tdts_set_pkt_parameter_req_flag(___param, ___req_flag); \
		tdts_set_pkt_parameter_pkt_time(___param, ___pkt_time); \
		tdts_init_port_scan_parameter(___param); \
		tdts_init_ip_sweep_parameter(___param); \
		tdts_init_tcp_chksum_parameter(___param); \
	} while (0)

/*!
 * \briefly Specify input packet proto to pass to TDTS engine.
 */
#define tdts_set_pkt_parameter_l2_ether(__param, __pkt, __pkt_len) \
	tdts_set_pkt_parameter(__param, __pkt, __pkt_len, TDTS_PKT_PARAMETER_PKT_TYPE_L2_ETHERNET)

#define tdts_set_pkt_parameter_l3_ip(__param, __pkt, __pkt_len) \
	tdts_set_pkt_parameter(__param, __pkt, __pkt_len, TDTS_PKT_PARAMETER_PKT_TYPE_L3_IP)

#define tdts_set_pkt_parameter_l3_ip6(__param, __pkt, __pkt_len) \
	tdts_set_pkt_parameter(__param, __pkt, __pkt_len, TDTS_PKT_PARAMETER_PKT_TYPE_L3_IP6)


extern int tdts_core_pkt_processor(tdts_pkt_parameter_t *pkt_parameter_ptr);

extern int tdts_core_rule_parsing_trf_load(void *trf_ptr, unsigned trf_size);
extern void tdts_core_rule_parsing_trf_unload(void);

extern int tdts_core_init(void);
extern void tdts_core_cleanup(void);

extern int tdts_core_get_sig_ver(void *buf);
extern int tdts_core_rule_parsing_get_ptn_data_len(uint32_t *len);
extern int tdts_core_rule_parsing_ptn_data_copy(void *buf, uint32_t buf_len, uint32_t *buf_used_len);
extern void tdts_core_rule_parsing_ptn_data_free(void);
extern int tdts_core_get_matched_rule_buf_len(uint32_t *buf_len);
extern int tdts_core_get_matched_rule_list(char *buf, uint32_t buf_len);
extern int tdts_core_get_matched_rule_info(void *buf, uint32_t buf_len, uint32_t *buf_len_used);
extern int tdts_core_get_rule_and_ptn_num(unsigned *rule_num_ptr,unsigned *ptn_num_ptr);
extern int tdts_core_get_rule_mem_usage(unsigned long *mem_usage_ptr);

extern int tdts_core_system_setting_state_get(unsigned *state_ptr);
extern int tdts_core_system_setting_state_set(unsigned state);

extern int tdts_core_tcp_conn_remove(uint8_t  ip_ver, uint8_t *sip, uint8_t *dip, uint16_t sport, uint16_t dport);
extern int tdts_core_udp_conn_remove(uint8_t  ip_ver, uint8_t *sip, uint8_t *dip, uint16_t sport, uint16_t dport);
extern int tdts_core_system_setting_tcp_conn_max_set(unsigned int conn_max);
extern int tdts_core_system_setting_tcp_conn_max_get(unsigned int *conn_max_ptr);
extern int tdts_core_system_setting_tcp_conn_timeout_set(unsigned int timeout_sec);
extern int tdts_core_system_setting_tcp_conn_timeout_get(unsigned int *timeout_sec_ptr);
extern int tdts_core_system_setting_udp_conn_max_set(unsigned int conn_max);
extern int tdts_core_system_setting_udp_conn_max_get(unsigned int *conn_max_ptr);
extern int tdts_core_get_tcp_conn_num(uint32_t *conn_num_ptr);

extern int tdts_core_devid__data_len_get(unsigned *len_ptr);
extern int tdts_core_devid_data_copy(void *buf, uint32_t buf_len, uint32_t *buf_used_len);


extern int tdts_core_appid_get_cat_name(unsigned char cat_id, unsigned char *name_ptr,
	unsigned name_len_max);
extern int tdts_core_appid_get_nr_cat_name(void *buf, uint32_t buf_len, uint32_t *buf_used_len);
extern int tdts_core_appid_get_all_cat_name(void *buf, uint32_t buf_len, uint32_t *buf_used_len);
extern int tdts_core_appid_get_beh_name(unsigned char beh_id, unsigned char *name_ptr,
	unsigned name_len_max);
extern int tdts_core_appid_get_nr_beh_name(void *buf, uint32_t buf_len, uint32_t *buf_used_len);
extern int tdts_core_appid_get_all_beh_name(void *buf, uint32_t buf_len, uint32_t *buf_used_len);
extern int tdts_core_appid_get_app_name(unsigned char cat_id, unsigned short app_id,
	unsigned char *name_ptr, unsigned name_len_max);
extern int tdts_core_appid_get_nr_app_name(void *buf, uint32_t buf_len, uint32_t *buf_used_len);
extern int tdts_core_appid_get_all_app_name(void *buf, uint32_t buf_len, uint32_t *buf_used_len);
extern int tdts_core_appid_combination_check(unsigned char cat_id, unsigned short app_id, unsigned char beh_id,
	unsigned char *is_existed_ptr);
extern int tdts_core_appid_get_nr_appid(void *buf, uint32_t buf_len, uint32_t *buf_used_len);
extern int tdts_core_appid_get_all_appid(void *buf, uint32_t buf_len, uint32_t *buf_used_len);

extern int tdts_core_get_eng_ver(uint32_t *maj, uint32_t *mid, uint32_t *min, char *local, unsigned local_len);
extern int tdts_core_get_core_ver(uint32_t *maj, uint32_t *mid, uint32_t *min, char *local, unsigned local_len);
extern int tdts_core_ready_check(void);

extern int tdts_core_get_bndwth_num(void *buf);
extern int tdts_core_get_bndwth_ent(void *buf, uint32_t buf_len, uint32_t *buf_used_len);

//----------------------------------------------------------------------------
typedef uint32_t tdts_appid_behinst_t;
typedef uint32_t tdts_appid_appinst_t;

/*!
 * \brief Convert a behavior instance id into an app instance id.
 *
 * behinst:
 *   8 bit     16 bit     8 bit
 * +-----------------------------+
 * | cat id |  app id   | beh id |
 * +-----------------------------+
 *
 * appinst:
 *   8 bit     16 bit     8 bit
 * +-----------------------------+
 * | cat id |  app id   |  0x00  |
 * +-----------------------------+
 */
#define TDTS_APPID_BEH2APPINST(_behinst_id) ((_behinst_id) & 0xFFFFFF00)

#define _TDTS_CAT_BITSHIFT ((sizeof(((tdts_appid_matching_results_t *) 0)->app_id) * 8) + (sizeof(((tdts_appid_matching_results_t *) 0)->beh_id) * 8))
#define _TDTS_APP_BITSHIFT (sizeof(((tdts_appid_matching_results_t *) 0)->beh_id) * 8)
#define _TDTS_BEH_BITSHIFT (0)

/*!
 * \brief Get behinst from matched application IDs from ips_check.
 *
 * \param _cat_id Default category ID
 * \param _app_id Default app ID
 * \param _beh_id Default behavior ID
 *
 * \note Term: "Behavior Instance" is an application instance with behavior input
 */
#define TDTS_APPID_GET_BEHINST(_cat_id, _app_id, _beh_id) \
	(uint32_t) ( \
		(uint32_t) (((uint8_t) _cat_id) << _TDTS_CAT_BITSHIFT) | \
		(uint32_t) (((uint8_t) _app_id) << _TDTS_APP_BITSHIFT) | \
		(uint32_t) ((uint8_t) _beh_id << _TDTS_BEH_BITSHIFT) \
	)

/*!
 * \brief Unknown unknown app ID.
 */
#define TDTS_APPID_GET_UNKNOWN_BEHINST() TDTS_APPID_GET_BEHINST(0, 0, 0)

/*!
 * \brief Get appinst from matched application IDs from ips_check.
 *
 * \param __cat_id Default category ID
 * \param __app_id Defautl app ID
 *
 * \note Term: "Application Instance" is an application instance without behavior input
 */
#define TDTS_APPID_GET_APPINST(__cat_id, __app_id) TDTS_APPID_GET_BEHINST(__cat_id, __app_id, 0)

/*!
 * \brief Get cat ID by instance ID.
 * \param _behinst_id behinst
 */
#define TDTS_APPID_GET_CAT(_behinst_id) ((uint8_t) (((_behinst_id) & 0xFF000000) >> _TDTS_CAT_BITSHIFT))

/*!
 * \brief Get app ID by instance ID.
 * \param _behinst_id behinst
 */
#define TDTS_APPID_GET_APP(_behinst_id) ((uint16_t) (((_behinst_id) & 0x00FFFF00) >> _TDTS_APP_BITSHIFT))

/*!
 * \brief Get beh ID by instance ID.
 * \param _behinst_id behinst
 */
#define TDTS_APPID_GET_BEH(_behinst_id) ((uint8_t) (((_behinst_id) & 0x000000FF) >> _TDTS_BEH_BITSHIFT))

#define TDTS_APPID_MAX_NAME_LEN (TMCFG_E_CORE_APPID_MAX_NAME_LEN)
#define TDTS_APPID_MAX_CAT_NUM (TMCFG_E_CORE_APPID_MAX_CAT_NUM)
#define TDTS_APPID_MAX_APP_NUM (TMCFG_E_CORE_APPID_MAX_APP_NUM)
#define TDTS_APPID_MAX_BEH_NUM (TMCFG_E_CORE_APPID_MAX_BEH_NUM)

//----------------------------------------------------------------------------
#define TDTS_ENGINE_VER_MAJ (TMCFG_E_MAJ_VER)
#define TDTS_ENGINE_VER_MID (TMCFG_E_MID_VER)
#define TDTS_ENGINE_VER_MIN (TMCFG_E_MIN_VER)
//----------------------------------------------------------------------------
/*
 * Other r/w interface
 */
typedef struct
{
	uint16_t major;
	uint16_t minor;
	uint32_t time_stamp;

} __attribute__((packed)) tdts_core_sig_ver_t;

typedef struct
{
	uint16_t major;
	uint16_t middle;
	uint16_t minor;

} __attribute__((packed)) tdts_core_eng_ver_t;

typedef struct
{
	uint32_t ano_tbl_len;
	uint32_t sec_tbl_len;

	/* Put data in the following order */
	uint8_t ano_tbl_ptr[0];
	/*
	uint8_t sec_tbl_ptr[0];
	*/

} __attribute__((packed)) tdts_core_sig_shared_data_t;


typedef struct
{
	/* OS name table. */
	unsigned name_table_buf_len;
	unsigned name_table_max;

	/* OS vendor table. */
	unsigned vendor_table_buf_len;
	unsigned vendor_table_max;

	/* OS class table. */
	unsigned class_table_buf_len;
	unsigned class_table_max;

	/* BOOTP fingerprint table. */
	unsigned fp_table_buf_len;
	unsigned fp_table_max;

	/* Category (device type) table */
	unsigned cat_table_buf_len;
	unsigned cat_table_max;

	/* Category (device name) table */
	unsigned dev_table_buf_len;
	unsigned dev_table_max;

	/* Family (recommended qos type) table */
	unsigned family_table_buf_len;
	unsigned family_table_max;

	/* User-Agent Rule ID Mapping Table */
	unsigned uamap_table_buf_len;
	unsigned uamap_table_max;

	/* Put data in the following order */
	uint8_t name_table_buf[0];
	/*
	uint8_t vendor_table_buf[0];
	uint8_t class_table_buf[0];
	uint8_t fp_table_buf[0];
	uint8_t cat_table_buf[0];
	uint8_t dev_table_buf[0];
	uint8_t family_table_buf[0];
	uint8_t uamap_table_buf[0];
	*/
} __attribute__((packed)) tdts_core_devid_data_t;

typedef struct
{
	uint8_t cat_id;
	uint16_t app_id;

	uint8_t name[TDTS_APPID_MAX_NAME_LEN];
} __attribute__((packed)) tdts_core_app_name_t;

typedef struct
{
	uint8_t cat_id;
	uint16_t app_id;
	uint8_t beh_id;
} __attribute__((packed)) tdts_core_appdb_entry_t;

typedef struct
{
	uint32_t rule_id;
	uint32_t hit_num;
} __attribute__((packed)) tdts_core_rule_matching_t;

typedef struct
{
	uint8_t cat_id;
	uint16_t app_id;
	uint8_t type;
	uint16_t resol;
	uint16_t bndwth;
}  __attribute__((packed)) tdts_core_bndwth_db_entry_t;


//------------------------------------------------------------------------------
/*
 * flooding detection APIs
 */

// configuration of flood
typedef struct flood_spec
{
	unsigned int policy_id;			/*!< traffic clean policy id */
	unsigned short flood_type;		/*!< anomaly type of flood */
	unsigned short threshold;			/*!< max packet count per second */
	unsigned char log;				/*!< 0 = do not log, otherwise log */
	unsigned char log_action;				/*!< for log, record bitwise complicated action */
} __attribute__ ((packed)) flood_spec_t;

// packet information
typedef struct pkt_info
{
	unsigned char	ip_ver;
	unsigned char proto;				/*!< l4 protocol */
	unsigned short vlan_id;			/*!< vlan id */

	unsigned char *sip; 				/*!< Source IP */
	unsigned char *dip; 				/*!< Destination IP */

	unsigned short sport;
	unsigned short dport;

	unsigned short src_user;			/*!< src user id */
	unsigned short dst_user;			/*!< dst user id */
	/*
	 * DPDK specific.
	 */
	unsigned char in_port;			/*!< Input port id */
} __attribute__ ((packed)) pkt_info_t;

typedef enum
{
	TDTS_ANO_IP_FLOOD = 0,
	TDTS_ANO_TCP_FLOOD,
	TDTS_ANO_UDP_FLOOD,
	TDTS_ANO_ICMP_FLOOD,
	TDTS_ANO_IGMP_FLOOD,
	TDTS_ANO_TCP_SYN_FLOOD_DST,
	TDTS_ANO_TCP_SYN_FLOOD_SRC,
	TDTS_ANO_FLOOD_MAX
} tdts_ano_flood_type_t;

typedef enum
{
	TDTS_ANO_FLOOD_PASS = 0,
	TDTS_ANO_FLOOD_OVER_LIMIT
} tdts_ano_flood_det_result_t;

/* the log call back function prototype */
typedef int (*flood_log_cb)
(
	const unsigned int policy_id,
	const unsigned int signature_id,
	const unsigned char  interface,
	const unsigned char  protocol,
	const unsigned short vlan,
	const unsigned char  ip_version,
	const unsigned char  sip[16],
	const unsigned short sport,
	const unsigned char  dip[16],
	const unsigned short dport,
	const unsigned short src_os,
	const unsigned short dst_os,
	const unsigned short src_device,
	const unsigned short dst_device,
	const unsigned short src_user,
	const unsigned short dst_user,
	const unsigned char  action,
	const unsigned int count,
	const unsigned char  aggregation,
	const unsigned char  extra
);

extern int tdts_core_flood_mem_init(void *void_ptr, unsigned mem_size, unsigned record_max);
extern int tdts_core_flood_record(void *void_ptr, unsigned record_max, flood_spec_t *spec, pkt_info_t *pkt);
extern void tdts_core_flood_record_output_and_init(void *void_ptr, unsigned record_max, unsigned int signature_id,
													unsigned short flood_type, flood_log_cb log_cb);

extern int tdts_core_suspect_list_init(void *void_ptr, unsigned mem_size, unsigned entry_num);

extern int tdts_core_insert_suspect_list_entry(void *void_ptr, unsigned entry_num,
												unsigned char ip_ver, unsigned char *dip, unsigned short dport,
												long (*get_curr_time)(void));

extern int tdts_core_suspect_list_query(void *void_ptr, unsigned entry_num, pkt_info_t *pkt,
									unsigned timeout, long (*get_curr_time)(void));

//------------------------------------------------------------------------------

#endif // TDTS_CORE_API_H_

