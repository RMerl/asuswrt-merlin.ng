/*
	add all TrendMicro include and extern function
*/

#include <conf_app.h>
#include <ioc_anomaly.h>
#include <ioc_common.h>
#include <ioc_patrol_tq.h>
#include <ioc_qos.h>
#include <ioc_vp.h>
#include <ioc_wrs.h>
#include <list.h>
#include <poison.h>

extern int get_fw_user_app_rate(app_ioctl_entry_t **output, uint32_t *used_len);
extern int get_fw_app_patrol(app_patrol_list_ioc_entry_t **output);
extern int set_fw_app_patrol(void *input, unsigned int length);
extern int set_fw_patrol_tq(void *input, unsigned int length);
extern int get_fw_patrol_tq(void **output);
extern int get_fw_patrol_tq_log(void **output, unsigned int *buf_used_len, int flag);
extern int get_fw_user_domain_list(void **output, unsigned int *buf_used_len, int flag);
extern int set_fw_user_domain_clear(void);
extern int set_fw_wrs_app_check(unsigned char flag);
extern int set_fw_wrs_cc_check(unsigned char flag);
extern int set_fw_wrs_sec_check(unsigned char flag);
extern int get_fw_wrs_url_list(void **output, unsigned int *buf_used_len);
extern int set_fw_vp_check(unsigned char flag);
extern int get_fw_vp_list(void **output, unsigned int *buf_used_len);
extern int set_fw_vp(void *input, unsigned int length);
extern int get_fw_anomaly_list(void **output, unsigned int *buf_used_len, int flag);
extern int set_fw_iqos_state(unsigned char flag);
extern int set_fw_iqos(void *input, unsigned int length);
extern int set_udb_conf(void *input, unsigned int length);
extern int get_udb_conf(char **output);
extern int get_wrs_url(void);
extern int get_app_patrol(void);
extern int get_fw_app_bw_clear(app_bw_ioctl_entry_t **output, uint32_t *used_len);
