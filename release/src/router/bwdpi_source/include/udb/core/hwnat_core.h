#ifndef __HWNAT_CORE_H__
#define __HWNAT_CORE_H__

#define QCA_HWNAT

typedef struct conn_acct
{
	struct {
		uint64_t packets;
		uint64_t bytes;
	} ori;

	struct {
		uint64_t packets;
		uint64_t bytes;
	} rep;

	#define CT_DIR_ORIGINAL_DOWNLOAD 1
	#define CT_DIR_ORIGINAL_UPLOAD 2
	
	uint8_t ori_dir; // 1:download, 2:upload
} conn_acct_t;

typedef struct accel_conn
{
	uint8_t ip_ver;
	uint8_t proto;

	uint16_t sport;
	uint16_t dport;

	union {
		uint32_t all;
		uint32_t ipv4;
		uint8_t ipv6[16];
	} sip;

	union {
		uint32_t all;
		uint32_t ipv4;
		uint8_t ipv6[16];
	} dip;

	uint8_t src_mac[6];
	uint8_t dest_mac[6];
} accel_conn_t;

typedef struct ecm_cb
{
	void (*get_conn_acct)(void *ct, conn_acct_t *conn_acct);
	void (*pcc_permit_accel)(accel_conn_t *accel_conn);
	void (*pcc_deny_accel)(accel_conn_t *accel_conn);
	
	void (*my_nf_conntrack_get)(void *ct);
	void (*my_nf_conntrack_put)(void *ct);
	
} ecm_cb_t;

extern ecm_cb_t *ecm_cb;

int udb_core_okay_to_accel_v4(accel_conn_t *accel_conn);
int udb_core_okay_to_accel_v6(accel_conn_t *accel_conn);
void udb_core_register_ecm_cb(ecm_cb_t *ecm_cb_ptr);
void cte_recycle_timer_handler(unsigned long unused);
void udb_core_set_cte_timeout(unsigned int time);

#endif /* __HWNAT_CORE_H__ */
