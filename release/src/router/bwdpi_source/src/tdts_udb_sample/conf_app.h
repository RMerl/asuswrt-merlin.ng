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

#ifndef __CONF_APP_H__
#define __CONF_APP_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "udb/tmcfg_udb.h"

#include "udb/ioctl/udb_ioctl_common.h"

#include "linux/list.h"

#define ASUS_PATH               "/tmp/bwdpi/" // ASUS only for search correct catid / devid / appid
#define APP_INF_DB_PATH         ASUS_PATH"bwdpi.app.db"
#define APP_CAT_DB_PATH         ASUS_PATH"bwdpi.cat.db"
#define DEV_OS_DB_PATH          ASUS_PATH"bwdpi.devdb.db"
#define RULE_DB_PATH            ASUS_PATH"bwdpi.rule.db"

//#define APP_INF_DB_PATH		"bwdpi.app.db"
//#define APP_CAT_DB_PATH		"bwdpi.cat.db"
//#define DEV_OS_DB_PATH		"bwdpi.devdb.db"
//#define RULE_DB_PATH		"bwdpi.rule.db"

#define CONF_PATH_MAX_LEN	128

#define MAC_OCTET_FMT "%02X:%02X:%02X:%02X:%02X:%02X"
#define MAC_OCTET_EXPAND(o) \
	(uint8_t) o[0], (uint8_t) o[1], (uint8_t) o[2], (uint8_t) o[3], (uint8_t) o[4], (uint8_t) o[5]

#define IPV4_OCTET_FMT "%u.%u.%u.%u"
#define IPV4_OCTET_EXPAND(o) (uint8_t) o[0], (uint8_t) o[1], (uint8_t) o[2], (uint8_t) o[3]

#define IPV6_OCTET_FMT "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X"
#define IPV6_OCTET_EXPAND(o) \
	(uint8_t) o[0], (uint8_t) o[1], (uint8_t) o[2], (uint8_t) o[3], 	\
	(uint8_t) o[4], (uint8_t) o[5], (uint8_t) o[6], (uint8_t) o[7], 	\
	(uint8_t) o[8], (uint8_t) o[9], (uint8_t) o[10], (uint8_t) o[11], 	\
	(uint8_t) o[12], (uint8_t) o[13], (uint8_t) o[14], (uint8_t) o[15]

#define DBG(fmt, args...)
//#define DBG(fmt, args...) 	fprintf(stderr, fmt, ##args);
#define ERR(fmt, args...) 	fprintf(stderr, "Error: " fmt, ##args);

#define PRT_DEVID(_name, _val) \
do { \
	printf("%-15s : %-4u\n", _name, _val); \
} while (0)

#define PRT_DEVID_STR(_name, _val, _str) \
do { \
	printf("%-15s : %-4u (%s)\n", _name, _val, (_val) ? _str : "Unknown"); \
} while (0)

typedef int (*action_cb_t)(void);

struct delegate
{
	int action;
	char *name;
	void (*show_help)(char *base);
	int (*parse_arg)(int, char **);
	action_cb_t cb;
};

#define CMD_OPTIONS_MAX 16
#define CMD_OPTS_STR_MAX 32

#define OPTS_IDX_INC(__i) \
do { \
	if ((__i) >= CMD_OPTIONS_MAX) \
	{ \
		ERR("exceed max command options\n"); \
		return -1; \
	} \
	(__i)++; \
} while (0)

struct cmd_option
{
	struct delegate opts[CMD_OPTIONS_MAX];
	char *help;
};

#define ERR_UNKNOWN_ARGS(_i, _c, _v) \
do { \
	int i = 0; \
	fprintf(stderr, "unknown arguments:"); \
	for (i = _i; i < _c; i++) \
		fprintf(stderr, " %s", _v[i]); \
	fprintf(stderr, "\n"); \
} while (0)

#define HELP_INDENT_L 18
#define HELP_INDENT_S 6

/*
 * app info
 */
typedef struct app_inf
{
	unsigned cat_id;
	unsigned app_id;
	unsigned beh_id;
	char* app_name;

	struct list_head list;
} app_inf_t;

/*
 * app cat info
 */
typedef struct app_cat
{
	unsigned cat_id;
	char* cat_name;

	struct list_head list;
} app_cat_t;

/*
 * device OS info
 */
typedef struct dev_os
{
	unsigned vendor_id;
	unsigned os_id;
	unsigned class_id;
	unsigned type_id;
	unsigned dev_id;
	unsigned family_id;
	char* vendor_name;
	char* os_name;
	char* class_name;
	char* type_name;
	char* dev_name;
	char* family_name;
	struct list_head list;
} dev_os_t;

typedef struct qos_tc_cls
{
	int id;
	int p_id;
	uint32_t sent_bytes;
	uint32_t rate;
} qos_tc_cls_t;

/*
 * qos app stats
 */
typedef struct qos_app_stat
{
	uint8_t uid;
	uint8_t catid;
	uint16_t appid;

	uint32_t acc_dl_bytes;
	uint32_t inc_dl_bytes;
	uint32_t acc_ul_bytes;
	uint32_t inc_ul_bytes;

	uint32_t qos_flag;
	uint8_t paid;
	uint16_t bndwidth;

	uint8_t checked;

	qos_tc_cls_t *tc_dl_cls;
	qos_tc_cls_t *tc_ul_cls;
} qos_app_stat_t;

#define ENV_SHN_HOME_DIR "SHN_HOME_DIR"

static __attribute__((unused)) char *shn_path(const char *path)
{	
	static char abs_path[CONF_PATH_MAX_LEN];
	char *home = getenv(ENV_SHN_HOME_DIR);

	memset(abs_path, 0, sizeof(abs_path));

	if (home)
	{
		if (strlen(home) + strlen(path) + 2 >= sizeof(abs_path))
		{
			ERR("path is too long!\n");
			goto __ret;
		}
		
		snprintf(abs_path, sizeof(abs_path) - 1, "%s/%s", home, path);
	}
	else
	{
		if (strlen(path) + 3 >= sizeof(abs_path))
		{
			ERR("path is too long!\n");
			goto __ret;
		}

		snprintf(abs_path, sizeof(abs_path) - 1, "./%s", path);
	}

__ret:
	return abs_path;
}

int init_app_inf(struct list_head *head);
void free_app_inf(struct list_head *head);
char* search_app_inf(struct list_head *head, unsigned cat_id, unsigned app_id);
int init_app_cat(struct list_head *head);
void free_app_cat(struct list_head *head);
char* search_app_cat(struct list_head *head, unsigned cat_id);
int init_rule_db(struct list_head *head);
void free_rule_db(struct list_head *head);
char* search_rule_db(struct list_head *head, unsigned cat_id);
int init_dev_os(struct list_head *head);
void free_dev_os(struct list_head *head);
dev_os_t* search_dev_os(struct list_head *head
	, unsigned vendor_id, unsigned os_id, unsigned class_id
	, unsigned type_id, unsigned dev_id
	, unsigned family_id);
#endif /* __CONF_APP_H__ */
