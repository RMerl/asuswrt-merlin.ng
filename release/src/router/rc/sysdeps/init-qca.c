/*
	Copyright 2005, Broadcom Corporation
	All Rights Reserved.

	THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
	KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
	SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
	FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.

*/

#include "rc.h"

#include <termios.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <time.h>
#include <errno.h>
#include <paths.h>
#include <sys/wait.h>
#include <sys/reboot.h>
#include <sys/klog.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <linux/mii.h>
#include <wlutils.h>
#include <bcmdevs.h>
#include <string.h>
#include <stdint.h>

#include <shared.h>

#ifdef RTCONFIG_QCA
#include <qca.h>
#include <flash_mtd.h>
#if defined(RTCONFIG_QCA) && defined(RTCONFIG_SOC_IPQ40XX)
extern int bg;
#endif
#endif

#if defined(RTCONFIG_NEW_REGULATION_DOMAIN)
#error !!!!!!!!!!!QCA driver must use country code!!!!!!!!!!!
#endif

#if defined(RTCONFIG_GLOBAL_INI)
static const char *global_ini_params[] = {
	"vow_config", "carrier_vow_config", "fw_vow_stats_enable",
	"OL_ACBKMinfree", "OL_ACBEMinfree", "OL_ACVIMinfree", "OL_ACVOMinfree",
	"fw_dump_options", "enableuartprint", "max_descs", "max_peers",
	"cce_disable", "qwrap_enable", "otp_mod_param", "max_active_peers",
	"enable_smart_antenna", "sa_validate_sw", "nss_wifi_nxthop_cfg",
	"max_clients", "max_vaps", "enable_smart_antenna_da", "lteu_support",
	"tgt_sched_params", "eapol_minrate_set", "eapol_minrate_ac_set",
	"mesh_support", "beacon_offload_disable", "spectral_disable", "twt_enable",
	NULL
};
#endif

#if defined(RTCONFIG_SOC_IPQ8074) || \
    defined(RTCONFIG_SOC_IPQ40XX) || \
    defined(RTCONFIG_QCN550X) || \
    defined(RPAC51) || defined(MAPAC1750)
/* SPF4.0 (kernel v3.14) or above, including SPF5.2 (kernel v4.4) */
static const char *umac_params[] = {
	/*"atf_mode",*/ "atf_msdu_desc", "atf_peers", "atf_max_vdevs",
	"enable_mesh_peer_cap_update", "enable_pktlog_support",
	"wifiposenable",

	NULL
},
*qca_ol_params[] = {
#if defined(RTCONFIG_GLOBAL_INI)
	"allocram_track_max", "ar900b_20_targ_clk",
	"bmi", "cfg_iphdr_pad", "dfs_disable", "emu_type",
	"enable_mesh_support", "enable_tx_tcp_cksum", "frac",
	"intval", "low_mem_system", "max_vdevs",
	"nss_wifi_ol_skip_nw_process", "ol_scan_chanlist",
	"war1", "war1_allow_sleep",
#if defined(RTCONFIG_WIFI_QCA9990_QCA9990) || \
    defined(RTCONFIG_WIFI_QCA9994_QCA9994) || \
    defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	"qca9888_20_targ_clk", "preCACEn",
#endif
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	/* SPF10 ES QSDK */
	"enable_eapol_minrate", "set_eapol_minrate_ac", "interCACChan",
#endif
#else	/* !RTCONFIG_GLOBAL_INI */
	"OL_ACBEMinfree", "OL_ACBKMinfree", "OL_ACVIMinfree",
	"OL_ACVOMinfree", "allocram_track_max", "ar900b_20_targ_clk",
	"bmi", "cfg_iphdr_pad", "dfs_disable", "emu_type",
	"enable_mesh_support", "enable_smart_antenna",
	"enable_tx_tcp_cksum", "enableuartprint", "frac",
	"fw_dump_options", "intval", "low_mem_system",
	"max_active_peers", "max_clients", "max_descs", "max_peers",
	"max_vaps", "max_vdevs", "nss_wifi_ol_skip_nw_process",
	/*"nss_wifi_olcfg",*/ "ol_scan_chanlist", "otp_mod_param",
	"qwrap_enable", "sa_validate_sw", /*"testmode",*/
	"vow_config", "war1", "war1_allow_sleep",
#endif	/* RTCONFIG_GLOBAL_INI */

	NULL
},
*qdf_params[] = {
	"qdf_dbg_mask", "prealloc_disabled",

	NULL
};
#else
/* SPF3.0 (kernel v3.14)
 * QCA95xx ILQ1.x~2.x (kernel v3.3)
 * IPQ806X ILQ3.0~3.1 (kernel v3.4)
 */
static const char *umac_params[] = {
	"vow_config", "OL_ACBKMinfree", "OL_ACBEMinfree", "OL_ACVIMinfree",
	"OL_ACVOMinfree", "ar900b_emu", "frac", "intval",
	"fw_dump_options", "enableuartprint", "ar900b_20_targ_clk",
	"max_descs", "qwrap_enable", "otp_mod_param", "max_active_peers",
	"enable_smart_antenna", "max_vaps", "enable_smart_antenna_da",
#if defined(RTCONFIG_WIFI_QCA9990_QCA9990) || \
    defined(RTCONFIG_WIFI_QCA9994_QCA9994) || \
    defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	"qca9888_20_targ_clk", "lteu_support",
	"atf_msdu_desc", "atf_peers", "atf_max_vdevs",
#endif
#if defined(RTCONFIG_SOC_IPQ40XX)
	"atf_msdu_desc", "atf_peers", "atf_max_vdevs",
#endif

	NULL
}, *qdf_params[] __attribute__((unused)) = {
	NULL
}, *adf_params[]  __attribute__((unused)) = {
	"prealloc_disabled",
	NULL
};
#endif

enum qca_kmod_flags {
	QCA_WIFI_STICK_BIT = 0,
	QCA_WIFI_DA_BIT,
	QCA_WIFI_OL_BIT,
	QCA_WIFI_ADF_BIT,
	QCA_WIFI_QDF_BIT,
};

#define QWIFI_STICK	(1U << QCA_WIFI_STICK_BIT)
#define QWIFI_DA	(1U << QCA_WIFI_DA_BIT)
#define QWIFI_OL	(1U << QCA_WIFI_OL_BIT)
#define QWIFI_DAOL_TYPE	(QWIFI_DA | QWIFI_OL)
#define QWIFI_ADF	(1U << QCA_WIFI_ADF_BIT)
#define QWIFI_QDF	(1U << QCA_WIFI_QDF_BIT)
#define QWIFI_DF_TYPE	(QWIFI_ADF | QWIFI_QDF)

#define DPARM(v, s, len, fmt, args...)	{	\
	int l = snprintf(s, len, fmt, ##args);	\
	*v++ = s;				\
	len -= l;				\
	s += l + 1;				\
}

#define PTR_DPARM(pv, ps, plen, fmt, args...) {		\
	int l = snprintf(*ps, *plen, fmt, ##args);	\
	*(*pv)++ = *ps;					\
	*plen -= l;					\
	*ps += l + 1;					\
}

#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
static int do_cold_boot_calibration(char *mod);
#endif

static void ath_hal_param_hook(char ***pv, char **ps, int *plen);
static void umac_param_hook(char ***pv, char **ps, int *plen);
static void umac_tmode_param_hook(char ***pv, char **ps, int *plen);

#if defined(RTCONFIG_SOC_IPQ40XX) || \
    defined(RTCONFIG_WIFI_QCN5024_QCN5054) || \
    defined(RTCONFIG_QCN550X) || \
    defined(RPAC51) || defined(MAPAC1750)
static void qca_ol_param_hook(char ***pv, char **ps, int *plen);
static void qca_ol_tmode_param_hook(char ***pv, char **ps, int *plen);
#endif
#if defined(RTCONFIG_GLOBAL_INI)
static void wifi_3_0_post_hook(void);
#endif

static struct load_wifi_kmod_seq_s {
	char *kmod_name;
	/* 0:		Always load this module.
	 * bit0:	stick, never removed.
	 * bit1~4:	Direct-Attach/Offload/adf/qdf module.
	 * 		Load module if same bit is enabled in qca_wifi_type.
	 */
	unsigned int flags;
	unsigned int load_sleep;
	unsigned int remove_sleep;
	/* module-specific parameters and can be adjusted by define qca_XXX nvram variable. */
	const char **params;
	/* module-specific mission mode parameter generator function. */
	void (*mission_mode_param_hook_fn)(char ***pv, char **ps, int *plen);
	/* module-specific test mode parameter generator function. */
	void (*test_mode_param_hook_fn)(char ***pv, char **ps, int *plen);
	/* module-specific post hook function. */
	void (*post_fn)(void);
} load_wifi_kmod_seq[] = {
#if defined(RTCONFIG_SOC_IPQ40XX) \
 || defined(RTCONFIG_WIFI_QCN5024_QCN5054) \
 || defined(RTCONFIG_QCN550X) \
 || defined(RPAC51) || defined(MAPAC1750)
	/* /lib/wifi/qca-wifi-modules
	 * SPF3.0, kernel 3.14.x, 10.4 WiFi driver
	 * SPF5.2, kernel 4.4.x, 10.4 WiFi driver
	 * SPF8.0, kernel 4.4.x, 11.0 WiFi driver
	 */
	{ .kmod_name = "mem_manager", .flags = QWIFI_STICK },	/* If QCA WiFi configuration file has WIFI_MEM_MANAGER_SUPPORT=1 */
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054) && defined(RTCONFIG_GLOBAL_INI)
	/* IPQ8074A SPF10 */
	{ .kmod_name = "qdf", .flags = QWIFI_QDF,
		.params = qdf_params
	},
	{ .kmod_name = "asf" },
#else
	/* IPQ8074 SPF8 */
	{ .kmod_name = "asf" },
	{ .kmod_name = "adf", .flags = QWIFI_ADF },
	{ .kmod_name = "qdf", .flags = QWIFI_QDF,
		.params = qdf_params
	},
#endif
#if !defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	{ .kmod_name = "ath_dfs" },
	{ .kmod_name = "ath_spectral" },
#endif
	{ .kmod_name = "umac",
		.remove_sleep = 2, .params = umac_params,
		.mission_mode_param_hook_fn = umac_param_hook,
		.test_mode_param_hook_fn = umac_tmode_param_hook
	},
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	{ .kmod_name = "qca_spectral" },
#endif
	{ .kmod_name = "ath_hal", .flags = QWIFI_DA,
		.mission_mode_param_hook_fn = ath_hal_param_hook
	},
	{ .kmod_name = "ath_rate_atheros", .flags = QWIFI_DA },
	{ .kmod_name = "hst_tx99", .flags = QWIFI_DA  },
	{ .kmod_name = "ath_dev", .flags = QWIFI_DA },
	{ .kmod_name = "qca_da", .flags = QWIFI_DA },
	{ .kmod_name = "qca_ol", .flags = QWIFI_OL,
		.params = qca_ol_params,
		.mission_mode_param_hook_fn = qca_ol_param_hook,
		.test_mode_param_hook_fn = qca_ol_tmode_param_hook
	},
#if defined(RTCONFIG_GLOBAL_INI)
	{ .kmod_name = "wifi_3_0",
		.post_fn = wifi_3_0_post_hook
	},
	{ .kmod_name = "wifi_2_0" },
#endif
#else
	/* QCA9558 ILQ2.0, kernel 3.3.x, 10.2 WiFi driver
	 * IPQ8064 ILQ3.1, kernel 3.4.x, 10.4 WiFi driver
	 */
	{ .kmod_name = "asf" },
	{ .kmod_name = "adf",
		.params = adf_params
	},
	{ .kmod_name = "ath_hal",
		.mission_mode_param_hook_fn = ath_hal_param_hook
	},
	{ .kmod_name = "ath_rate_atheros" },
	{ .kmod_name = "ath_dfs" },
	{ .kmod_name = "ath_spectral" },
	{ .kmod_name = "hst_tx99" },
	{ .kmod_name = "ath_dev" },
	{ .kmod_name = "umac",
		.remove_sleep = 2,
		.params = umac_params,
		.mission_mode_param_hook_fn = umac_param_hook,
		.test_mode_param_hook_fn = umac_tmode_param_hook
	},
#endif

//	{ "ath_pktlog", 0, 0, 0, NULL },
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	{ .kmod_name = "smart_antenna" },
#endif
#if defined(RTCONFIG_WIGIG)
	{ .kmod_name = "wil6210",
#if defined(GTAXY16000)
		.flags = QWIFI_STICK
#endif
	},
#endif
};

/* Define QCA Wi-Fi modules type here.
 * If a model uses old Wi-Fi driver, e.g., 10.2, specify 0 here.
 * If a model uses new Wi-Fi driver, e.g., 10.4 and kernel v3.14 or above, specify all type of Wi-Fi of all band here.
 */
static unsigned int qca_wifi_type =
#if defined(RTCONFIG_WIFI_QCA9990_QCA9990) \
 || defined(RTCONFIG_WIFI_QCA9994_QCA9994)
	QWIFI_OL | QWIFI_ADF
#elif defined(RTCONFIG_SOC_IPQ40XX) \
   || defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	QWIFI_OL | QWIFI_QDF	/* All Wi-Fi unit are offload. */
#elif defined(RPAC51)
	QWIFI_DA | QWIFI_ADF	/* qca_ol is not loaded on RP-AC51.  It should use Direct-Attach Wi-Fi modules only. */
#elif defined(MAPAC1750) \
   || (defined(RTCONFIG_QCN550X) && defined(RTCONFIG_HAS_5G))
	QWIFI_DA | QWIFI_OL | QWIFI_QDF
#elif defined(RTCONFIG_QCN550X)
	QWIFI_DA | QWIFI_QDF
#else
	0		/* skip DA/OL module checking and load all QCA Wi-Fi modules */
#endif
;

static struct load_nat_accel_kmod_seq_s {
	char *kmod_name;
	unsigned int load_sleep;
	unsigned int remove_sleep;
} load_nat_accel_kmod_seq[] = {
#if defined(RTCONFIG_SOC_IPQ8064) || defined(RTCONFIG_SOC_IPQ8074)
#if defined(RTCONFIG_WIFI_QCA9994_QCA9994) || \
    defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	{ "shortcut_fe_drv", 0, 0 },
#endif
	{ "ecm", 0, 0 },
#else
        { "shortcut_fe_cm", 0, 0 },
#endif
};

static const struct irq_smp_affinity_s {
	int irq;
	unsigned int cpu_mask;
} wifi_irq_smp_affinity_tbl[] = {
#if defined(BRTAC828) || defined(RTAD7200)
#if LINUX_KERNEL_VERSION >= KERNEL_VERSION(4,4,0)
	{ 143, 1 },	/* wifi0 = 2G ==> core 0 */
	{ 176, 2 },	/* wifi1 = 5G ==> core 1 */
#else
	{ 68, 1 },	/* wifi0 = 2G ==> core 0 */
	{ 90, 2 },	/* wifi1 = 5G ==> core 1 */
#endif
#if defined(RTAD7200)
#if LINUX_KERNEL_VERSION >= KERNEL_VERSION(4,4,0)
	{ 178, 2 },	/* wil6210 = 60G ==> core 0 */
#else
	{ 1433, 2 },	/* wil6210 = 60G ==> core 0 */
#endif
#endif
#elif defined(RTCONFIG_SOC_IPQ40XX)
	{ 174, 2 },	/* wifi2 = 5G2 ==> core 2 */
	{ 200, 4 },	/* wifi0 = 2G  ==> core 3 */
	{ 201, 8 },	/* wifi1 = 5G  ==> core 4 */
#endif
	{ -1, 0 }	/* Last item. */
};

#if defined(RTCONFIG_GLOBAL_INI)
/* Translate multiple continuously NULL-terminated strings as newline-terminated strings at run-time,
 * and then write it to file.
 * @fp:
 * @s:	multiple continuously NULL-terminated strings, last string must end with two NULL character.
 * @return:
 * 	0:	success
 *  otherwise:	error
 */
static int write_cont_null_strings(FILE *fp, const char *s)
{
	int ret = 0;
	const char *b = s;

	if (!fp || !s)
		return -2;

	for (b = s; *b != '\0'; ) {
		fprintf(fp, "%s\n", b);
		b += strlen(b);
		if (*b == '\0' && *(b + 1) == '\0')
			break;
		b++;
	}

	return ret;
}

/* Update multiple parameters pointed by @params array to @filename .ini file
 * @params:	char* array, last item must be NULL, newline character will be ignored.
 * @return:
 * 	0:	success
 *  otherwise:	error
 */
int __update_ini_file(const char *filename, char **params)
{
	const unsigned char end_mark[2] = { 0, 0 };
	long flen = 0, params_tlen = 0;
	int ret = 0, i, found, copy;
	FILE *fp;
	size_t l, key_len;
	char line[MAX_INI_PARM_LINE_LEN], *p, **v, *data;
	struct buf_ctrl_s {
		char *buf, *b;
		size_t remain, tlen;
	} bc[2], *src, *dst;

	if (!params)
		return -1;

	/* Calculate buffer size and allocate two buffers. */
	for (v = params; *v != NULL; ++v)
		params_tlen += strlen(*v) + 1;
	if (!params_tlen)
		return 0;

	fp = fopen(filename, "r+");
	if (!fp)
		return -2;
	fseek(fp, 0, SEEK_END);
	flen = ftell(fp);

	for (i = 0, src = &bc[0]; i < ARRAY_SIZE(bc); ++i, ++src) {
		src->buf = malloc(flen + params_tlen);
		if (!src->buf) {
			ret = -3;
			break;
		}

		src->remain = src->tlen = flen + params_tlen + 1;
		*src->buf = '\0';
	}

	if (ret)
		goto exit_update_ini_file;

	/* Read @filename to 1-st buffer, comments and blank lines are removed. */
	src = &bc[0];
	src->b = src->buf;
	fseek(fp, 0, SEEK_SET);
	while (fgets(line, sizeof(line), fp)) {
		if (*line == '#' || *line == '\0' || *line == '\r' || *line == '\n' || *line == '=')
			continue;

		if (!strchr(line, '='))
			continue;
		/* replace '\n' with '\0' temporary. */
		if ((p = strchr(line, '\n')) != NULL)
			*p = '\0';

		l = snprintf(src->b, src->remain, "%s", line) + 1;
		src->b += l;
		src->remain -= l;
	}
	*src->b++ = '\0';
	src->remain--;

	dst = &bc[1];
	for (i = 0, v = &params[0]; *v != NULL; ++i, ++v) {
		/* get next legal parameter */
		while (*v != NULL) {
			if ((p = strchr(*v, '=')) && p != *v)
				break;

			dbg("%s: unknown format [%s] in params.\n", __func__, *v);
			v++;
		}
		if (*v == NULL) {
			memcpy(dst->buf, src->buf, src->tlen - src->remain);
			dst->remain = src->remain;
			break;
		}

		key_len = p - *v + 1;
		found = 0;
		src = &bc[0 ^ (i & 1)];
		dst = &bc[1 ^ (i & 1)];
		src->b = src->buf;
		dst->b = dst->buf;
		dst->remain = dst->tlen;
		memcpy(dst->b, end_mark, 2);
		while (*(src->b) != '\0') {
			copy = 1;
			data = src->b;
			if (!strncmp(src->b, *v, key_len)) {
				found++;
				if (found == 1) {
					/* replace parameter */
					data = *v;
				} else {
					/* skip duplicated parameter */
					copy = 0;
				}
			}

			if (copy) {
				l = snprintf(dst->b, dst->remain, "%s", data) + 1;
				dst->b += l;
				dst->remain -= l;
			}
			src->b += strlen(src->b) + 1;
		}
		if (!found){
			/* append new parameter */
			if ((p = strchr(*v, '\n')) != NULL) {
				l = p - *v;
				memmove(dst->b, *v, l);
				*(dst->b + l) = '\0';
				dst->b += l + 1;
				dst->remain -= l + 1;
			} else {
				l = snprintf(dst->b, dst->remain, "%s", *v) + 1;
				dst->b += l;
				dst->remain -= l;
			}
		}
		*dst->b++ = '\0';
		dst->remain--;
	}

	fseek(fp, 0, SEEK_SET);
	write_cont_null_strings(fp, dst->buf);
	fclose(fp);
	/* last '\0' is not wrote, same offset in file maybe non-zero value. */
	truncate(filename, dst->tlen - dst->remain - 1);

 exit_update_ini_file:

	for (i = 0, src = &bc[0]; i < ARRAY_SIZE(bc); ++i, ++src)
		if (src->buf)
			free(src->buf);

	return ret;
}
#endif

/* Generate parameters for ath_hal in mission mode. */
static void ath_hal_param_hook(char ***pv, char **ps, int *plen)
{
	int ce_level __attribute__((unused));
#if defined(RPAC51)
	const int default_ce_level = 0xa0;
#else
	const int default_ce_level __attribute__((unused)) = 0xce;
#endif

	if (!pv || !ps || !plen)
		return;

#if defined(RTCONFIG_WIFI_QCA9557_QCA9882) || defined(RTCONFIG_QCA953X) || (defined(RTCONFIG_QCA956X) && !defined(MAPAC1750))
	ce_level = nvram_get_int("ce_level");
	if (ce_level <= 0)
		ce_level = default_ce_level;
	PTR_DPARM(pv, ps, plen, "ce_level=%d", ce_level);
#endif
}

/* Generate parameters for umac in mission mode. */
static void umac_param_hook(char ***pv, char **ps, int *plen)
{
	if (!pv || !ps || !plen)
		return;

	if (atf_enabled())
		*(*pv)++ = "atf_mode=1";

#if defined(RTCONFIG_WIFI_QCA9990_QCA9990) \
 || defined(RTCONFIG_WIFI_QCA9994_QCA9994)
#if LINUX_KERNEL_VERSION < KERNEL_VERSION(3,14,0)
	*(*pv)++ = "msienable=0";
#endif
#elif defined(RPAC51)
	*(*pv)++ = "msienable=0";
	*(*pv)++ = "wifi_start_idx=1";	/* FIXME: The internal direct attach radio should always be wifi0 */
	*(*pv)++ = "qca9888_20_targ_clk=300000000";
#endif

#if defined(RTCONFIG_WIFI_QCA9990_QCA9990) \
 || defined(RTCONFIG_WIFI_QCA9994_QCA9994)
#if LINUX_KERNEL_VERSION < KERNEL_VERSION(3,14,0)
	/* IPQ806X ILQ3.1 */
	PTR_DPARM(pv, ps, plen, "nss_wifi_olcfg=%d", nss_wifi_offloading());
#endif
#endif
}

/* Generate parameters for umac in test mode. */
static void umac_tmode_param_hook(char ***pv, char **ps, int *plen)
{
	if (!pv || !ps || !plen)
		return;

#if defined(RTCONFIG_WIFI_QCA9990_QCA9990) \
 || defined(RTCONFIG_WIFI_QCA9994_QCA9994) \
 || defined(RPAC51)
	/* ILQ3.1 (kernel 3.3), SPF3.0 (kernel 3.14), or below. */
	*(*pv)++ = "testmode=1";
	*(*pv)++ = "ahbskip=1";
#elif defined(RTCONFIG_SOC_IPQ40XX) \
   || defined(RTCONFIG_QCN550X) \
   || defined(MAPAC1750)
	*(*pv)++ = "ahbskip=1";
#endif
}

#if defined(RTCONFIG_SOC_IPQ40XX) || \
    defined(RTCONFIG_WIFI_QCN5024_QCN5054) || \
    defined(RTCONFIG_QCN550X) || \
    defined(RPAC51) || defined(MAPAC1750)
/* Generate parameters for qca_ol in mission mode.  */
static void qca_ol_param_hook(char ***pv, char **ps, int *plen)
{
	int olcfg __attribute__((unused)) = nss_wifi_offloading();

	if (!pv || !ps || !plen)
		return;

#if !defined(RTCONFIG_GLOBAL_INI)
#if defined(RTCONFIG_WIFI_QCA9990_QCA9990) \
 || defined(RTCONFIG_WIFI_QCA9994_QCA9994)
#if LINUX_KERNEL_VERSION >= KERNEL_VERSION(3,14,0)
	/* IPQ806X SPF5.3 */
	PTR_DPARM(pv, ps, plen, "nss_wifi_olcfg=%d", olcfg());
#endif
#elif defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	/* IPQ807X SPF8.0 */
	PTR_DPARM(pv, ps, plen, "nss_wifi_olcfg=%d", !!olcfg);
	PTR_DPARM(pv, ps, plen, "nss_wifili_olcfg=%d", olcfg);
	*(*pv)++ = "rx_hash=0";
	*(*pv)++ = "intr_mitigation_enabled=1";		/* SPF10 obsoleted it */
#endif

#if defined(RTCONFIG_CFG80211)
	*(*pv)++ = "cfg80211_config=1";
#endif
#endif	/* !RTCONFIG_GLOBAL_INI */

#if defined(RTCONFIG_SOC_IPQ40XX)
	/* For lower memory system, e.g., IPQ40xx + 128MB RAM models. */
	if (get_meminfo_item("MemTotal") <= 131072)
		*(*pv)++ = "low_mem_system=1";
#endif


	/* Keep below statsment at end of this function. */
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	do_cold_boot_calibration("qca_ol");
#endif
}

/* Generate parameters for qca_ol in test mode.  */
static void qca_ol_tmode_param_hook(char ***pv, char **ps, int *plen)
{
	if (!pv || !ps || !plen)
		return;

#if !defined(RTCONFIG_GLOBAL_INI)
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	*(*pv)++ = "soc_probe_module_load=1";
	*(*pv)++ = "fbc_enabled=1";
	*(*pv)++ = "hw_mode_id=1";
	*(*pv)++ = "testmode=1";
	*(*pv)++ = "cfg80211_config=1";
#elif defined(RTCONFIG_SOC_IPQ40XX) \
   || defined(RTCONFIG_QCN550X) \
   || defined(MAPAC1750)
	*(*pv)++ = "testmode=1";
#endif
#endif	/* !RTCONFIG_GLOBAL_INI */


	/* Keep below statsment at end of this function. */
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	do_cold_boot_calibration("qca_ol");
#endif
}
#endif

#if defined(RTCONFIG_GLOBAL_INI)
/* Set correct hw_mode_id to @arg in accordance with /sys/class/net/socXXX/hw_modes.
 * @basedir:
 * @de:		struct dirent readed by readdir() in readdir_wrapper()
 * @arg:	integer pointer, must be initialized as negative integer.
 * @return:
 * 	0:	success or irrevelent file
 *     -1:	invalid parameter
 *  otherwise:	error
 */
static int __update_hw_mode_id(const char *basedir, const struct dirent *de, void *arg)
{
	int v = -1, *hw_mode_id = arg;
	char val[16];	/* see hw_modes in wifi_soc_hw_modes_show() */
	char path[sizeof("/sys/class/net/socXXX/hw_modesXXXXXX")];
	struct str2id_s {
		char *keyword;
		int val;
	} str2id_tbl[] = {
		{ "DBS_SBS:", 4 },
		{ "DBS:", 1 },
		{ "DBS_OR_SBS:", 5 },
		{ "SINGLE:", 0 },
		{ "SBS_PASSIVE:", 2 },
		{ "SBS:", 3 },
		{ NULL, -1 }
	}, *p;

	if (!arg)
		return -1;

	if (strncmp(de->d_name, "soc", 3))
		return 0;

	/* If socX/hw_modes absent, skip it. */
	snprintf(path, sizeof(path), "%s/%s/hw_modes", basedir, de->d_name);
	if (!f_exists(path))
		return 0;
	if (f_read_string(path, val, sizeof(val)) <= 0)
		return -2;

	/* Convert strings in hw_modes attribute as integer. see detect_qcawifi() */
	for (p = &str2id_tbl[0]; v < 0 && p->keyword; ++p) {
		if (!strstr(val, p->keyword) || strlen(val) < strlen(p->keyword))
			continue;

		v = p->val;
	}

	if (v < 0) {
		dbg("%s: unknown string [%s] in %s\n", __func__, val, path);
		return -3;
	}
	if (*hw_mode_id >= 0) {
		dbg("%s: hw_mode_id is updated more than one times\n", __func__);
	}
	*hw_mode_id = v;

	return 0;
}

static void wifi_3_0_post_hook(void)
{
	int hw_mode_id = -1;
	char hw_mode_id_str[sizeof("hw_mode_id=6XXX")] = { 0 };

	if (readdir_wrapper(SYS_CLASS_NET, "soc", __update_hw_mode_id, &hw_mode_id) || hw_mode_id < 0)
		return;

	snprintf(hw_mode_id_str, sizeof(hw_mode_id_str), "hw_mode_id=%d", hw_mode_id);
	update_ini_file(GLOBAL_I_INI, hw_mode_id_str);
}
#endif

static void __mknod(char *name, mode_t mode, dev_t dev)
{
	if (mknod(name, mode, dev)) {
		printf("## mknod %s mode 0%o fail! errno %d (%s)", name, mode, errno, strerror(errno));
	}
}

void init_devs(void)
{
	int status;

	__mknod("/dev/nvram", S_IFCHR | 0666, makedev(228, 0));
	__mknod("/dev/dk0", S_IFCHR | 0666, makedev(63, 0));
	__mknod("/dev/dk1", S_IFCHR | 0666, makedev(63, 1));
	__mknod("/dev/armem", S_IFCHR | 0660, makedev(1, 13));
#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2) || \
    defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2)
	__mknod("/dev/rtkswitch", S_IFCHR | 0666, makedev(206, 0));
#endif
#if (defined(PLN12) || defined(PLAC56) || defined(PLAC66U) || defined(RPAC66) || defined(RPAC51) || defined(MAPAC1750) || defined(RTN19) || defined(RTAC59U))
	eval("ln", "-sf", "/dev/mtdblock2", "/dev/caldata");	/* mtdblock2 = SPI flash, Factory MTD partition */
#elif (defined(RTAC58U) || defined(RT4GAC53U) || defined(RTAC82U))
	eval("ln", "-sf", "/dev/mtdblock3", "/dev/caldata");	/* mtdblock3 = cal in NAND flash, Factory MTD partition */
#else
	eval("ln", "-sf", "/dev/mtdblock3", "/dev/caldata");	/* mtdblock3 = Factory MTD partition */
#endif

	if ((status = WEXITSTATUS(modprobe("nvram_linux"))))
		printf("## modprove(nvram_linux) fail status(%d)\n", status);

#if defined(RTCONFIG_WIFI_SON) && defined(RTCONFIG_AMAS)
	if (nvram_match("wifison_ready", "1"))
		mount("overlayfs", "/www", "overlayfs", MS_MGC_VAL, "lowerdir=/www,upperdir=/www-sys");
#endif
}

void generate_switch_para(void)
{
#if defined(RTCONFIG_DUALWAN)
	int model;
	int wans_cap = get_wans_dualwan() & WANSCAP_WAN;
	int wanslan_cap = get_wans_dualwan() & WANSCAP_LAN;

	// generate nvram nvram according to system setting
	model = get_model();

	switch (model) {
	case MODEL_RTAC55U:
	case MODEL_RTAC55UHP:
	case MODEL_RT4GAC55U:
	case MODEL_RTN19:
	case MODEL_RTAC59U:
	case MODEL_RTAC88N:
		nvram_unset("vlan3hwname");
		if ((wans_cap && wanslan_cap)
		    )
			nvram_set("vlan3hwname", "et0");
		break;
	}
#endif
}

#if defined(RTCONFIG_SOC_IPQ8064) || defined(RTCONFIG_SOC_IPQ8074)
void tweak_wifi_ps(const char *wif)
{
	unsigned int all_cores = 3;	/* CPU0 and CPU1 */

#if defined(RTCONFIG_SOC_IPQ8074)
	all_cores = 0xF;		/* CPU0~3 */
#endif

	if (!strncmp(wif, WIF_2G, strlen(WIF_2G)) || !strcmp(wif, VPHY_2G))
		set_iface_ps(wif, 1);	/* 2G: CPU0 only */
	else
		set_iface_ps(wif, all_cores);	/* 5G/5G2/60G */
}
#endif

static void tweak_lan_wan_ps(void)
{
#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2)
	/* Reference to qca-nss-drv.init */
#if LINUX_KERNEL_VERSION >= KERNEL_VERSION(4,4,0)
	set_irq_smp_affinity_by_name("nss", 1, 2);		/* eth0, LAN, nss */
	set_irq_smp_affinity_by_name("nss_queue1", 1, 2);	/* eth1, LAN, nss_queue1 */
	set_irq_smp_affinity_by_name("nss", 2, 1);		/* eth2, WAN0, nss */
	set_irq_smp_affinity_by_name("nss_queue1", 2, 1);	/* eth3, WAN1, nss_queue1 */
#else
	set_irq_smp_affinity_by_name("nss", 1, 2);	/* 1st nss irq, eth0, LAN */
	set_irq_smp_affinity_by_name("nss", 2, 2);	/* 2nd nss irq, eth1, LAN */
	set_irq_smp_affinity_by_name("nss", 3, 1);	/* 3rd nss irq, eth2, WAN0 */
	set_irq_smp_affinity_by_name("nss", 4, 1);	/* 4th nss irq, eth3, WAN1 */
#endif

	set_iface_ps("eth0", 2);
	set_iface_ps("eth1", 2);
	set_iface_ps("eth2", 1);
	set_iface_ps("eth3", 1);
#elif defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2)
	/* Reference to qca-nss-drv.init */
#if LINUX_KERNEL_VERSION >= KERNEL_VERSION(4,4,0)
	set_irq_smp_affinity_by_name("nss", 1, 1);		/* eth0, WAN0, nss */
	set_irq_smp_affinity_by_name("nss_queue1", 1, 2);	/* eth1, LAN, nss_queue1 */
	set_irq_smp_affinity_by_name("nss", 2, 2);		/* eth2, LAN, nss */
	set_irq_smp_affinity_by_name("nss_queue2", 2, 1);	/* eth3, WAN1, nss_queue1 */
#else
	set_irq_smp_affinity_by_name("nss", 1, 1);	/* 1st nss irq, eth0, WAN0 */
	set_irq_smp_affinity_by_name("nss", 2, 2);	/* 2nd nss irq, eth1, LAN */
	set_irq_smp_affinity_by_name("nss", 3, 2);	/* 3rd nss irq, eth2, LAN */
	set_irq_smp_affinity_by_name("nss", 4, 1);	/* 4th nss irq, eth3, WAN1 */
#endif

	set_iface_ps("eth0", 1);
	set_iface_ps("eth1", 2);
	set_iface_ps("eth2", 2);
	set_iface_ps("eth3", 1);
#elif defined(RTCONFIG_SOC_IPQ8074)
	/* qca-nss-drv.init */
	set_irq_smp_affinity_by_name("nss_queue1", 0, 2);
	set_irq_smp_affinity_by_name("nss_queue2", 0, 4);
	set_irq_smp_affinity_by_name("nss_queue3", 0, 8);
	f_write_string("/proc/sys/dev/nss/rps/enable", "1", 0, 0);
#endif
}

#if defined(RTCONFIG_SWITCH_QCA8075_QCA8337_PHY_AQR107_AR8035_QCA8033)
/* Set default MAC address of WAN interfaces. */
static void set_wanifaces_hwaddr(void)
{
	int i;
	char *wan_ifaces[3] = { 0 };
	char macbuf[sizeof("00:11:22:33:44:55XXX")];
	unsigned char ea[6];

	switch (get_model()) {
	case MODEL_GTAXY16000:
	case MODEL_RTAX89U:
		wan_ifaces[0] = "eth3";
		wan_ifaces[1] = "eth5";
		wan_ifaces[2] = "eth4";
		break;
	}

	for (i = 0; i < 3; ++i) {
		if (wan_ifaces[i] == NULL)
			break;

		ether_atoe(get_wan_hwaddr(), ea);
		ea[5] += i;
		ether_etoa(ea, macbuf);
		eval("ifconfig", wan_ifaces[i], "hw", "ether", macbuf);
	}
}
#else
static inline void set_wanifaces_hwaddr(void) {}
#endif


static void init_switch_qca(void)
{
	char *qca_module_list[] = {
#if defined(RTCONFIG_SOC_IPQ8064)
#if defined(RTCONFIG_SWITCH_QCA8337N)
		"qca-ssdk",
#endif
		"qca-nss-gmac",
		"qca-nss-drv",
		"qca-nss-qdisc",
#elif defined(RTCONFIG_SOC_IPQ8074)
		/* 09-shortcut-fe */
		"shortcut-fe", "shortcut-fe-ipv6",

		/* 10-shortcut-fe-drv */
		"shortcut-fe-drv",
#if defined(RTCONFIG_SWITCH_QCA8337N) || defined(RTCONFIG_SWITCH_QCA8075_QCA8337_PHY_AQR107_AR8035_QCA8033)
		"qca-ssdk",
#endif
		"qca-nss-dp",
		"qca-nss-drv",
		"qca-mcs", "hyfi-bridging", "ecm",
#elif defined(RTCONFIG_SOC_IPQ40XX)
		"qrfs",
		"shortcut-fe", "shortcut-fe-ipv6", "shortcut-fe-cm",
		"qca-ssdk",
		"essedma",
/*		"qca-nss-gmac", "qca-nss-drv",	*/
#if defined(RTCONFIG_BT_CONN)
		"hyfi_qdisc", "hyfi-bridging",
#endif	/* RTCONFIG_BT_CONN */
#elif defined(MAPAC1750)
		"shortcut-fe", "shortcut-fe-ipv6", "shortcut-fe-cm",
		"qca-ssdk",
		"hyfi_qdisc", "hyfi-bridging",
#endif
#if defined(RTCONFIG_STRONGSWAN) ||  defined(RTCONFIG_QUICKSEC)
#if LINUX_KERNEL_VERSION >= KERNEL_VERSION(4,4,0)
		"ip_tunnel",
#endif
		"tunnel4", /*"tunnel6",*/
		"ah4", "esp4", "xfrm4_tunnel", "ipcomp",
		/*"ah6", "esp6", "xfrm6_tunnel", "ipcomp6", */
#if defined(RTCONFIG_SOC_IPQ8064)
#if LINUX_KERNEL_VERSION >= KERNEL_VERSION(4,4,0)
		"udp_tunnel", "ip6_udp_tunnel",
		"qca-nss-l2tpv2", "qca-nss-lag-mgr", "qca-nss-map-t",
		"qca-nss-pptp", "qca-nss-crypto", "qca-nss-crypto-tool", 
		"qca-nss-macsec", "qca-nss-ipsecmgr", "qca-nss-qdisc",
		"qca-nss-cfi-ocf", "qca-nss-cfi-cryptoapi",
		"qca-nss-ipsec", "qca-nss-tun6rd", "qca-nss-tunipip6",
#else
		"qca-nss-capwapmgr", "qca-nss-cfi-cryptoapi",
		"qca-nss-crypto-tool", "qca-nss-crypto",
		"qca-nss-profile-drv", "qca-nss-tun6rd",
		"qca-nss-tunipip6", "qca-nss-ipsec",
		"qca-nss-ipsecmgr", "qca-nss-cfi-ocf",
		"qca-nss-cfi-cryptoapi", 
#endif
#elif defined(RTCONFIG_SOC_IPQ8074)
		"qca-nss-bridge-mgr",
		"qca-nss-l2tpv2", "qca-nss-lag-mgr", "qca-nss-map-t",
		"qca-nss-pptp", "qca-nss-vlan-mgr",

		/* 52-qca-nss-crypto */
		"qca-nss-crypto", "qca-nss-crypto-tool",

		"qca-nss-ipsecmgr",
		"qca-nss-qdisc",

		/* 59-qca-nss-cfi */
		"qca-nss-cfi-ocf", "qca-nss-cfi-cryptoapi", "qca-nss-ipsec",

		"qca-nss-tun6rd",
		"qca-nss-tunipip6",
#endif
#endif	/* RTCONFIG_STRONGSWAN || RTCONFIG_QUICKSEC */
		NULL
	}, **qmod;
	char *argv[30] = {
		"modprobe", "-s", NULL
	}, **v;

	for (qmod = &qca_module_list[0]; *qmod != NULL; ++qmod) {
		if (module_loaded(*qmod))
			continue;

		v = &argv[2];
		*v++ = *qmod;
		if (!strcmp(*qmod, "qca-nss-cfi-cryptoapi")) {
			if (nvram_get_int("ipsec_hw_crypto_enable") == 0)
			continue;
		}
#if defined(RTCONFIG_SOC_IPQ8074)
		if (!strcmp(*qmod, "qca-ssdk")) {
			int mod_def0_pin = 46;
			char str[sizeof("d0_mod_def0_pin=57XXXXX")];

			if (nvram_match("sfp+_force_on", "1"))
				*v++ = "xgmac_autodet=N";

			if (nvram_match("HwId", "A"))
				mod_def0_pin = 57;
			snprintf(str, sizeof(str), "d0_mod_def0_pin=%d", mod_def0_pin);
			*v++ = str;
		}
#endif
#if defined(RTCONFIG_SOC_IPQ40XX)
		if (!strcmp(*qmod, "shortcut-fe-cm")) {
#if defined(MAPAC1300) || defined(MAPAC2200) || defined(VZWAC1300) || defined(RTAC95U)
			if ((sw_mode() != SW_MODE_ROUTER) && !nvram_match("cfg_master", "1"))
				continue;
#endif
			if (IS_NON_AQOS())
#if defined(RTCONFIG_BWDPI)
				*v++ ="skip_sfe=1";
#else
				continue;
#endif
		}
		else if (!strcmp(*qmod, "essedma")) {
			*v++ = *qmod;
			if (nvram_get_int("jumbo_frame_enable")) {
				*v++ = "overwrite_mode=1";
				*v++ = "page_mode=1";
			}

#if defined(RTAC58U) || defined(RT4GAC53U) /* for RAM 128MB */
			if (get_meminfo_item("MemTotal") <= 131072)
				*v++ = "reduce_rx_ring_size=1";
#endif

		}
#if defined(MAPAC1300) || defined(MAPAC2200) || defined(VZWAC1300) || defined(RTAC95U)
		else if (!strcmp(*qmod, "shortcut-fe")) {
			if(sw_mode() != SW_MODE_ROUTER)
				continue;
		}
		else if (!strcmp(*qmod, "shortcut-fe-ipv6")) {
			if(sw_mode() != SW_MODE_ROUTER)
				continue;
		}
#endif
#endif	/* RTCONFIG_SOC_IPQ40XX */

		*v++ = NULL;
		_eval(argv, NULL, 0, NULL);
	}

	char *wan0_ifname = nvram_safe_get("wan0_ifname");
	char *lan_ifname, *lan_ifnames, *ifname, *p;

#if defined(RTCONFIG_QCA953X) || defined(RTCONFIG_QCA956X) || defined(RTCONFIG_QCN550X)
	wan0_ifname = MII_IFNAME;
#endif

	tweak_lan_wan_ps();
	generate_switch_para();

	set_wanifaces_hwaddr();

#ifndef RTCONFIG_ETHBACKHAUL
	/* Set LAN MAC address to all LAN ethernet interface. */
	lan_ifname = nvram_safe_get("lan_ifname");
	if (!strncmp(lan_ifname, "br", 2) &&
	    !strstr(nvram_safe_get("lan_ifnames"), "bond"))
	{
		if ((lan_ifnames = strdup(nvram_safe_get("lan_ifnames"))) != NULL) {
			p = lan_ifnames;
			while ((ifname = strsep(&p, " ")) != NULL) {
				while (*ifname == ' ') ++ifname;
				SKIP_ABSENT_FAKE_IFACE(ifname);
				if (!strcmp(ifname, lan_ifname))
					continue;
				if (!strncmp(ifname, WIF_2G, strlen(WIF_2G))
				    || !strncmp(ifname, WIF_5G, strlen(WIF_5G))
				    || !strncmp(ifname, WIF_5G2, strlen(WIF_5G2))
				    || !strncmp(ifname, WIF_60G, strlen(WIF_60G))
				   )
					continue;
				if (*ifname == 0)
					break;
				eval("ifconfig", ifname, "hw", "ether", get_lan_hwaddr());
			}
			free(lan_ifnames);
		}
	}
#endif

	// TODO: replace to nvram controlled procedure later
#if !(defined(RTCONFIG_DETWAN) && defined(RTCONFIG_ETHBACKHAUL))		// not to change MAC
	if (strlen(wan0_ifname)) {
		eval("ifconfig", wan0_ifname, "hw", "ether", get_wan_hwaddr());
	}
#endif	/* ! RTCONFIG_ETHBACKHAUL */
	config_switch();

#ifdef RTCONFIG_SHP
	if (nvram_get_int("qos_enable") || nvram_get_int("lfp_disable_force")) {
		nvram_set("lfp_disable", "1");
	} else {
		nvram_set("lfp_disable", "0");
	}

	if (nvram_get_int("lfp_disable") == 0) {
		restart_lfp();
	}
#endif

#if defined(RTCONFIG_SOC_IPQ40XX)
	doSystem("ethtool -K eth0 gro off\n");
	doSystem("ethtool -K eth1 gro off\n");
	/* qrfs.init:
	 * enable Qualcomm Receiving Flow Steering (QRFS)
	 */
	f_write_string("/proc/qrfs/enable", "0", 0, 0); //for throughput, disable it
#endif
}

void enable_jumbo_frame(void)
{
	if (set_jumbo_frame) {
		set_jumbo_frame();
		return;
	}
#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2) || \
    defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2)
#elif defined(RTCONFIG_SOC_IPQ40XX)
	if (nvram_get_int("jumbo_frame_enable")) {
		eval("ifconfig", "eth0", "mtu", "9000");
		eval("ifconfig", "eth1", "mtu", "9000");
		eval("ssdk_sh", "misc", "frameMaxSize", "set", "9018");
	}
#else
	int mtu = 1518;	/* default value */
	char mtu_str[] = "9000XXX";

	if (!nvram_contains_word("rc_support", "switchctrl"))
		return;

	if (nvram_get_int("jumbo_frame_enable"))
		mtu = 9000;

	snprintf(mtu_str, sizeof(mtu_str), "%d", mtu);
	eval("swconfig", "dev", MII_IFNAME, "set", "max_frame_size", mtu_str);
#endif
}

void init_switch(void)
{
	int sfe_dev;
	char dev_num[6];

	init_switch_qca();

	if (( f_read_string("/sys/sfe_ipv4/debug_dev", dev_num, sizeof(dev_num)) && \
			(sfe_dev = atoi(dev_num)) > 0)) {
		unlink("/dev/sfe4");
		__mknod("/dev/sfe4", S_IFCHR | 0660, makedev(sfe_dev, 0));
	}
	if (( f_read_string("/sys/sfe_ipv6/debug_dev", dev_num, sizeof(dev_num)) && \
			(sfe_dev = atoi(dev_num)) > 0)) {
		unlink("/dev/sfe6");
		__mknod("/dev/sfe6", S_IFCHR | 0660, makedev(sfe_dev, 0));
	}

#if defined(RTCONFIG_SOC_IPQ8064) || defined(RTCONFIG_SOC_IPQ8074)
	init_ecm();
#endif
}

/**
 * Setup a VLAN.
 * @vid:	VLAN ID
 * @prio:	VLAN PRIO
 * @mask:	bit31~16:	untag mask
 * 		bit15~0:	port member mask
 * @return:
 * 	0:	success
 *  otherwise:	fail
 *
 * bit definition of untag mask/port member mask
 * 0:	Port 0, LANx port which is closed to WAN port in visual.
 * 1:	Port 1
 * 2:	Port 2
 * 3:	Port 3
 * 4:	Port 4, WAN port
 * 9:	Port 9, RGMII/MII port that is used to connect CPU and WAN port.
 * 	a. If you only have one RGMII/MII port and it is shared by WAN/LAN ports,
 * 	   you have to define two VLAN interface for WAN/LAN ports respectively.
 * 	b. If your switch chip choose another port as same feature, convert bit9
 * 	   to your own port in low-level driver.
 */
static int __setup_vlan(int vid, int prio, unsigned int mask)
{
	char vlan_str[] = "4096XXX";
	char prio_str[] = "7XXX";
	char mask_str[] = "0x00000000XXX";
	char *set_vlan_argv[] = { "rtkswitch", "36", vlan_str, NULL };
	char *set_prio_argv[] = { "rtkswitch", "37", prio_str, NULL };
	char *set_mask_argv[] = { "rtkswitch", "39", mask_str, NULL };

	if (vid > 4096) {
		_dprintf("%s: invalid vid %d\n", __func__, vid);
		return -1;
	}

	if (prio > 7)
		prio = 0;

	_dprintf("%s: vid %d prio %d mask 0x%08x\n", __func__, vid, prio, mask);

	if (vid >= 0) {
		snprintf(vlan_str, sizeof(vlan_str), "%d", vid);
		_eval(set_vlan_argv, NULL, 0, NULL);
	}

	if (prio >= 0) {
		snprintf(prio_str, sizeof(prio_str), "%d", prio);
		_eval(set_prio_argv, NULL, 0, NULL);
	}

	snprintf(mask_str, sizeof(mask_str), "0x%08x", mask);
	_eval(set_mask_argv, NULL, 0, NULL);

	return 0;
}

int config_switch_for_first_time = 1;
void config_switch(void)
{
	int model = get_model();
	int stbport;
	int controlrate_unknown_unicast;
	int controlrate_unknown_multicast;
	int controlrate_multicast;
	int controlrate_broadcast;
	int merge_wan_port_into_lan_ports;
#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2) || \
    defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2)
	char *str;
#endif

	dbG("link down all ports\n");
	eval("rtkswitch", "17");	// link down all ports

	switch (model) {
	case MODEL_RTAC55U:	/* fall through */
	case MODEL_RTAC55UHP:	/* fall through */
	case MODEL_RT4GAC55U:	/* fall through */
	case MODEL_RTN19:	/* fall through */
	case MODEL_RTAC59U:	/* fall through */
	case MODEL_RTAC58U:	/* fall through */
	case MODEL_RT4GAC53U:	/* fall through */
	case MODEL_RTAC82U:	/* fall through */
	case MODEL_MAPAC1300:	/* fall through */
	case MODEL_VZWAC1300:	/* fall through */
	case MODEL_MAPAC1750:	/* fall through */
	case MODEL_MAPAC2200:	/* fall through */
	case MODEL_RTAC88N:	/* fall through */
	case MODEL_RTAC95U:	/* fall through */
#ifdef RTCONFIG_ETHBACKHAUL
		merge_wan_port_into_lan_ports = 0;
#else
		merge_wan_port_into_lan_ports = 1;
#endif
		break;
	default:
		merge_wan_port_into_lan_ports = 0;
	}

	if (config_switch_for_first_time)
		config_switch_for_first_time = 0;
	else {
		dbG("software reset\n");
		eval("rtkswitch", "27");	// software reset
	}

	pre_config_switch();

#ifdef RTCONFIG_DEFAULT_AP_MODE
	if (!is_router_mode())
		system("rtkswitch 8 7"); // LLLLL
	else
#endif
	system("rtkswitch 8 0"); // init, rtkswitch 114,115,14,15 need it
	if (is_routing_enabled()) {
		char parm_buf[] = "XXX";

		stbport = safe_atoi(nvram_safe_get("switch_stb_x"));
		if (stbport < 0 || stbport > 6) stbport = 0;
		dbG("ISP Profile/STB: %s/%d\n", nvram_safe_get("switch_wantag"), stbport);
		/* stbport:	Model-independent	unifi_malaysia=1	otherwise
		 * 		IPTV STB port		(RT-N56U)		(RT-N56U)
		 * -----------------------------------------------------------------------
		 *	0:	N/A			LLLLW
		 *	1:	LAN1			LLLTW			LLLWW
		 *	2:	LAN2			LLTLW			LLWLW
		 *	3:	LAN3			LTLLW			LWLLW
		 *	4:	LAN4			TLLLW			WLLLW
		 *	5:	LAN1 + LAN2		LLTTW			LLWWW
		 *	6:	LAN3 + LAN4		TTLLW			WWLLW
		 */

		/* portmask in rtkswitch
		 * 	P9	P8	P7	P6	P5	P4	P3	P2	P1	P0
		 * 	MII-W	MII-L	-	-	-	WAN	LAN1	LAN2	LAN3	LAN4
		 */

		if (!nvram_match("switch_wantag", "none")&&!nvram_match("switch_wantag", "")) {
			//2012.03 Yau modify
			char tmp[128];
			char *p;
			int voip_port = 0;
			int t, vlan_val = -1, prio_val = -1;
			unsigned int mask = 0;

//			voip_port = safe_atoi(nvram_safe_get("voip_port"));
			voip_port = 3;
			if (voip_port < 0 || voip_port > 4)
				voip_port = 0;		

			/* Fixed Ports Now*/
			stbport = 4;	
			voip_port = 3;
	
			snprintf(tmp, sizeof(tmp), "rtkswitch 29 %d", voip_port);
			system(tmp);	

			if (!strncmp(nvram_safe_get("switch_wantag"), "unifi", 5)) {
				/* Added for Unifi. Cherry Cho modified in 2011/6/28.*/
				if(strstr(nvram_safe_get("switch_wantag"), "home")) {
					system("rtkswitch 38 1");		/* IPTV: P0 */
					/* Internet:	untag: P9;   port: P4, P9 */
					__setup_vlan(500, 0, 0x02000210);
					/* IPTV:	untag: P0;   port: P0, P4 */
					__setup_vlan(600, 0, 0x00010011);
				}
				else {
					/* No IPTV. Business package */
					/* Internet:	untag: P9;   port: P4, P9 */
					system("rtkswitch 38 0");
					__setup_vlan(500, 0, 0x02000210);
				}
			}
			else if (!strncmp(nvram_safe_get("switch_wantag"), "singtel", 7)) {
				/* Added for SingTel's exStream issues. Cherry Cho modified in 2011/7/19. */
				if(strstr(nvram_safe_get("switch_wantag"), "mio")) {
					/* Connect Singtel MIO box to P3 */
					system("rtkswitch 40 1");		/* admin all frames on all ports */
					system("rtkswitch 38 3");		/* IPTV: P0  VoIP: P1 */
					/* Internet:	untag: P9;   port: P4, P9 */
					__setup_vlan(10, 0, 0x02000210);
					/* VoIP:	untag: N/A;  port: P1, P4 */
					//VoIP Port: P1 tag
					__setup_vlan(30, 4, 0x00000012);
				}
				else {
					//Connect user's own ATA to lan port and use VoIP by Singtel WAN side VoIP gateway at voip.singtel.com
					system("rtkswitch 38 1");		/* IPTV: P0 */
					/* Internet:	untag: P9;   port: P4, P9 */
					__setup_vlan(10, 0, 0x02000210);
				}

				/* IPTV */
				__setup_vlan(20, 4, 0x00010011);		/* untag: P0;   port: P0, P4 */
			}
			else if (!strcmp(nvram_safe_get("switch_wantag"), "m1_fiber")) {
				//VoIP: P1 tag. Cherry Cho added in 2012/1/13.
				system("rtkswitch 40 1");			/* admin all frames on all ports */
				system("rtkswitch 38 2");			/* VoIP: P1  2 = 0x10 */
				/* Internet:	untag: P9;   port: P4, P9 */
				__setup_vlan(1103, 1, 0x02000210);
				/* VoIP:	untag: N/A;  port: P1, P4 */
				//VoIP Port: P1 tag
				__setup_vlan(1107, 1, 0x00000012);
			}
			else if (!strcmp(nvram_safe_get("switch_wantag"), "maxis_fiber")) {
				//VoIP: P1 tag. Cherry Cho added in 2012/11/6.
				system("rtkswitch 40 1");			/* admin all frames on all ports */
				system("rtkswitch 38 2");			/* VoIP: P1  2 = 0x10 */
				/* Internet:	untag: P9;   port: P4, P9 */
				__setup_vlan(621, 0, 0x02000210);
				/* VoIP:	untag: N/A;  port: P1, P4 */
				__setup_vlan(821, 0, 0x00000012);

				__setup_vlan(822, 0, 0x00000012);		/* untag: N/A;  port: P1, P4 */ //VoIP Port: P1 tag
			}
			else if (!strcmp(nvram_safe_get("switch_wantag"), "maxis_fiber_sp")) {
				//VoIP: P1 tag. Cherry Cho added in 2012/11/6.
				system("rtkswitch 40 1");			/* admin all frames on all ports */
				system("rtkswitch 38 2");			/* VoIP: P1  2 = 0x10 */
				/* Internet:	untag: P9;   port: P4, P9 */
				__setup_vlan(11, 0, 0x02000210);
				/* VoIP:	untag: N/A;  port: P1, P4 */
				//VoIP Port: P1 tag
				__setup_vlan(14, 0, 0x00000012);
			}
#ifdef RTCONFIG_MULTICAST_IPTV
			else if (!strcmp(nvram_safe_get("switch_wantag"), "movistar")) {
#if defined(RTCONFIG_SOC_IPQ40XX)
				doSystem("echo 10 > /proc/sys/net/edma/default_group1_vlan_tag");
#else
				/* Software bridge based IPTV implementation need to create VLAN interface
				 * in __setup_vlan() which is called by config_switch().  In this case,
				 * start_vlan() hasn't been executed.  So, software bridge based IPTV
				 * implementation should not enable BFL_ENETVLAN bit in boardflags and has
				 * to call set_wan_tag() here instead.
				 */
				if (sw_based_iptv() && !(nvram_get_int("boardflags") & BFL_ENETVLAN)) {
					char wan_base_if[IFNAMSIZ] = "";

					strlcpy(wan_base_if, get_wan_base_if(), sizeof(wan_base_if));
					set_wan_tag(wan_base_if);
				}
#endif
#if 0	//set in set_wan_tag() since (switch_stb_x > 6) and need vlan interface by vconfig.
				system("rtkswitch 40 1");			/* admin all frames on all ports */
				/* Internet/STB/VoIP:	untag: N/A;   port: P4, P9 */
				__setup_vlan(6, 0, 0x00000210);
				__setup_vlan(2, 0, 0x00000210);
				__setup_vlan(3, 0, 0x00000210);
#endif
			}
#endif
			else if (!strcmp(nvram_safe_get("switch_wantag"), "meo")) {
				system("rtkswitch 40 1");			/* admin all frames on all ports */
				system("rtkswitch 38 1");			/* VoIP: P0 */
				/* Internet/VoIP:	untag: P9;   port: P0, P4, P9 */
				__setup_vlan(12, 0, 0x02000211);
			}
			else if (!strcmp(nvram_safe_get("switch_wantag"), "vodafone")) {
				system("rtkswitch 40 1");			/* admin all frames on all ports */
				system("rtkswitch 38 3");			/* Vodafone: P0  IPTV: P1 */
				/* Internet:	untag: P9;   port: P4, P9 */
				__setup_vlan(100, 1, 0x02000211);
				/* IPTV:	untag: N/A;  port: P0, P4 */
				__setup_vlan(101, 0, 0x00000011);
				/* Vodafone:	untag: P1;   port: P0, P1, P4 */
				__setup_vlan(105, 1, 0x00020013);
			}
			else if (!strcmp(nvram_safe_get("switch_wantag"), "hinet")) { /* Hinet MOD */
				if (sw_bridge_iptv_different_switches()) {
					/* Bridge:	untag: P0, P4, P9;	port: P0, P4, P9
					 * WAN:		no VLAN (hacked in API for SW based IPTV)
					 * STB:		Ctag, return value of get_sw_bridge_iptv_vid().
					 */
					__setup_vlan(get_sw_bridge_iptv_vid(), 0, 0x02110211);
				} else {
					eval("rtkswitch", "8", "4");		/* LAN4 with WAN */
				}
			}
#if defined(RTAC58U)
			else if (!strcmp(nvram_safe_get("switch_wantag"), "stuff_fibre")) {
				system("rtkswitch 38 0");			//No IPTV and VoIP ports
				/* Internet:	untag: P9;   port: P4, P9 */
				__setup_vlan(10, 0, 0x02000210);
			}
#endif
			else {
				/* Cherry Cho added in 2011/7/11. */
				/* Initialize VLAN and set Port Isolation */
				if(strcmp(nvram_safe_get("switch_wan1tagid"), "") && strcmp(nvram_safe_get("switch_wan2tagid"), ""))
					system("rtkswitch 38 3");		// 3 = 0x11 IPTV: P0  VoIP: P1
				else if(strcmp(nvram_safe_get("switch_wan1tagid"), ""))
					system("rtkswitch 38 1");		// 1 = 0x01 IPTV: P0
				else if(strcmp(nvram_safe_get("switch_wan2tagid"), ""))
					system("rtkswitch 38 2");		// 2 = 0x10 VoIP: P1
				else
					system("rtkswitch 38 0");		//No IPTV and VoIP ports

				/*++ Get and set Vlan Information */
				if(strcmp(nvram_safe_get("switch_wan0tagid"), "") != 0) {
					// Internet on WAN (port 4)
					if ((p = nvram_get("switch_wan0tagid")) != NULL) {
						t = safe_atoi(p);
						if((t >= 2) && (t <= 4094))
							vlan_val = t;
					}

					if((p = nvram_get("switch_wan0prio")) != NULL && *p != '\0')
						prio_val = safe_atoi(p);

#if defined(RTCONFIG_SOC_IPQ40XX)
					if (vlan_val == 2)
						doSystem("echo 10 > /proc/sys/net/edma/default_group1_vlan_tag");
#endif
					__setup_vlan(vlan_val, prio_val, 0x02000210);
				}

				if(strcmp(nvram_safe_get("switch_wan1tagid"), "") != 0) {
					// IPTV on LAN4 (port 0)
					if ((p = nvram_get("switch_wan1tagid")) != NULL) {
						t = safe_atoi(p);
						if((t >= 2) && (t <= 4094))
							vlan_val = t;
					}

					if((p = nvram_get("switch_wan1prio")) != NULL && *p != '\0')
						prio_val = safe_atoi(p);

					if(!strcmp(nvram_safe_get("switch_wan1tagid"), nvram_safe_get("switch_wan2tagid")))
						mask = 0x00030013;	//IPTV=VOIP
					else
						mask = 0x00010011;	//IPTV Port: P0 untag 65553 = 0x10 011

					__setup_vlan(vlan_val, prio_val, mask);
				}	

				if(strcmp(nvram_safe_get("switch_wan2tagid"), "") != 0) {
					// VoIP on LAN3 (port 1)
					if ((p = nvram_get("switch_wan2tagid")) != NULL) {
						t = safe_atoi(p);
						if((t >= 2) && (t <= 4094))
							vlan_val = t;
					}

					if((p = nvram_get("switch_wan2prio")) != NULL && *p != '\0')
						prio_val = safe_atoi(p);

					if(!strcmp(nvram_safe_get("switch_wan1tagid"), nvram_safe_get("switch_wan2tagid")))
						mask = 0x00030013;	//IPTV=VOIP
					else
						mask = 0x00020012;	//VoIP Port: P1 untag

					__setup_vlan(vlan_val, prio_val, mask);
				}

			}
		}
		else
		{
			const int sw_br_vid = get_sw_bridge_iptv_vid();
			char *str __attribute__((unused));

			if (stbport) {
				snprintf(parm_buf, sizeof(parm_buf), "%d", stbport);
				eval("rtkswitch", "8", parm_buf);
			}
			if (sw_based_iptv()) {
				/* WAN:	no VLAN (hacked in API for SW based IPTV)
				 * STB:	according to switch_stb_x nvram variable.
				 */
				switch (stbport) {
				case 0:	/* none */
					break;
				case 1:	/* LAN1 */
					/* untag: P3, P4, P9;	port: P3, P4, P9 */
					__setup_vlan(sw_br_vid, 0, 0x02180218);
					break;
				case 2:	/* LAN2 */
					/* untag: P2, P4, P9;	port: P2, P4, P9 */
					__setup_vlan(sw_br_vid, 0, 0x02140214);
					break;
				case 3:	/* LAN3 */
					/* untag: P1, P4, P9;	port: P1, P4, P9 */
					__setup_vlan(sw_br_vid, 0, 0x02120212);
					break;
				case 4:	/* LAN4 */
					/* untag: P0, P4, P9;	port: P0, P4, P9 */
					__setup_vlan(sw_br_vid, 0, 0x02110211);
					break;
				case 5:	/* LAN1 & LAN2 */
					/* untag: P3, P2, P4, P9;	port: P3, P2, P4, P9 */
					__setup_vlan(sw_br_vid, 0, 0x021C021C);
					break;
				case 6:	/* LAN3 & LAN4 */
					/* untag: P1, P0, P4, P9;	port: P1, P0, P4, P9 */
					__setup_vlan(sw_br_vid, 0, 0x02130213);
					break;
				default:
					dbg("%s: unknown stb_stb_x %d\n", __func__, nvram_get_int("switch_stb_x"));
				}
			}
#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2) || \
    defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2)
			str = nvram_get("lan_trunk_0");
			if(str != NULL && str[0] != '\0')
			{
				eval("rtkswitch", "45", "0");
				eval("rtkswitch", "46", str);
			}

			str = nvram_get("lan_trunk_1");
			if(str != NULL && str[0] != '\0')
			{
				eval("rtkswitch", "45", "1");
				eval("rtkswitch", "46", str);
			}
#endif
		}

		/* unknown unicast storm control */
		if (!nvram_get("switch_ctrlrate_unknown_unicast"))
			controlrate_unknown_unicast = 0;
		else
			controlrate_unknown_unicast = safe_atoi(nvram_get("switch_ctrlrate_unknown_unicast"));
		if (controlrate_unknown_unicast < 0 || controlrate_unknown_unicast > 1024)
			controlrate_unknown_unicast = 0;
		if (controlrate_unknown_unicast)
		{
			snprintf(parm_buf, sizeof(parm_buf), "%d", controlrate_unknown_unicast);
			eval("rtkswitch", "22", parm_buf);
		}
	
		/* unknown multicast storm control */
		if (!nvram_get("switch_ctrlrate_unknown_multicast"))
			controlrate_unknown_multicast = 0;
		else
			controlrate_unknown_multicast = safe_atoi(nvram_get("switch_ctrlrate_unknown_multicast"));
		if (controlrate_unknown_multicast < 0 || controlrate_unknown_multicast > 1024)
			controlrate_unknown_multicast = 0;
		if (controlrate_unknown_multicast) {
			snprintf(parm_buf, sizeof(parm_buf), "%d", controlrate_unknown_multicast);
			eval("rtkswitch", "23", parm_buf);
		}
	
		/* multicast storm control */
		if (!nvram_get("switch_ctrlrate_multicast"))
			controlrate_multicast = 0;
		else
			controlrate_multicast = safe_atoi(nvram_get("switch_ctrlrate_multicast"));
		if (controlrate_multicast < 0 || controlrate_multicast > 1024)
			controlrate_multicast = 0;
		if (controlrate_multicast)
		{
			snprintf(parm_buf, sizeof(parm_buf), "%d", controlrate_multicast);
			eval("rtkswitch", "24", parm_buf);
		}
	
		/* broadcast storm control */
		if (!nvram_get("switch_ctrlrate_broadcast"))
			controlrate_broadcast = 0;
		else
			controlrate_broadcast = safe_atoi(nvram_get("switch_ctrlrate_broadcast"));
		if (controlrate_broadcast < 0 || controlrate_broadcast > 1024)
			controlrate_broadcast = 0;
		if (controlrate_broadcast) {
			snprintf(parm_buf, sizeof(parm_buf), "%d", controlrate_broadcast);
			eval("rtkswitch", "25", parm_buf);
		}
	}
#ifdef RTCONFIG_WIFI_SON
	else if ((access_point_mode() && nvram_match("cfg_master", "1")) && nvram_match("wifison_ready", "1"))
		; //not to merge_wan_port_into_lan_ports.
#endif
	else if (access_point_mode())
	{
		if (merge_wan_port_into_lan_ports)
			eval("rtkswitch", "8", "100");
	}
#if defined(RTCONFIG_WIRELESSREPEATER) && defined(RTCONFIG_PROXYSTA)
	else if (mediabridge_mode())
	{
		if (merge_wan_port_into_lan_ports)
			eval("rtkswitch", "8", "100");
	}
#endif

#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2) || \
    defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2)
	str = nvram_get("lan_hash_algorithm");
	if(str && *str != '\0') {
		eval("rtkswitch", "47", str);
	}
#endif

	dbG("link up wan port(s)\n");
	eval("rtkswitch", "114");	// link up wan port(s)

#if !defined(RTCONFIG_SOC_IPQ40XX)
	enable_jumbo_frame();
#endif

	post_config_switch();

#if defined(RTCONFIG_BLINK_LED)
	if (is_swports_bled("led_lan_gpio")) {
		update_swports_bled("led_lan_gpio", nvram_get_int("lanports_mask"));
	}
	if (is_swports_bled("led_wan_gpio")) {
		update_swports_bled("led_wan_gpio", nvram_get_int("wanports_mask"));
	}
#if defined(RTCONFIG_WANLEDX2)
	if (is_swports_bled("led_wan2_gpio")) {
		update_swports_bled("led_wan2_gpio", nvram_get_int("wan1ports_mask"));
	}
#endif
#endif
}

/*
 * @return:
 * 	0:	switch not found or failure.
 *  otherwise:	switch found and ok.
 */
int switch_exist(void)
{
#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2) || \
    defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2)
	int ret;
	unsigned int id[2];
	char *wan_ifname[2] = {
#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2)
		"eth2", "eth3"	/* BRT-AC828 SR1 ~ SR3 wan0, wan1 interface. */
#elif defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2)
		"eth0", "eth3"	/* BRT-AC828 SR4 or above wan0, wan1 interface. */
#endif
	};

	ret = eval("rtkswitch", "41");
	_dprintf("eval(rtkswitch, 41) ret(%d)\n", ret);
	id[0] = (mdio_read(wan_ifname[0], MII_PHYSID1) << 16) | mdio_read(wan_ifname[0], MII_PHYSID2);
	id[1] = (mdio_read(wan_ifname[1], MII_PHYSID1) << 16) | mdio_read(wan_ifname[1], MII_PHYSID2);
	_dprintf("phy0/1 id %08x/%08x\n", id[0], id[1]);

	return (!ret && id[0] == 0x004dd074 && id[1] == 0x004dd074);
#elif defined(RTCONFIG_SWITCH_QCA8075_QCA8337_PHY_AQR107_AR8035_QCA8033)
#if defined(GTAXY16000) || defined(RTAX89U)
	static const uint32_t     aqr_id[] = { 0x03a1b4e2, 0x31c31c41, 0 };	/* AQR107, AQR113 */
	static const uint32_t qca8075_id[] = { 0x004dd0b1, 0 };
	static const uint32_t qca8337_id[] = { 0x004dd036, 0 };
	static const uint32_t  ar8033_id[] = { 0x004dd074, 0 };
	static const struct phy_id_list_s {
		int phy;
		int phy_type;	/* 0: General; 1: AQR C45 PHY */
		const uint32_t *pval;
		uint32_t mask;
	} phy_id_lists[] = {
		{  7, 1,     aqr_id, 0xfffffff0 },	/*  AQR1xx: WAN2, 10G RJ-45 */
		{ 11, 0, qca8075_id, 0xffffffff },	/* QCA8075: WAN1 */
		{ 10, 0, qca8075_id, 0xffffffff },	/* QCA8075: LAN1 */
		{  9, 0, qca8075_id, 0xffffffff },	/* QCA8075: LAN2 */
		{  4, 0, qca8337_id, 0xffffffff },	/* QCA8337: LAN3 */
		{  3, 0, qca8337_id, 0xffffffff },	/* QCA8337: LAN4 */
		{  0, 0, qca8337_id, 0xffffffff },	/* QCA8337: LAN5 */
		{  1, 0, qca8337_id, 0xffffffff },	/* QCA8337: LAN6 */
		{  0, 0, qca8337_id, 0xffffffff },	/* QCA8337: LAN7 */
		{  6, 0,  ar8033_id, 0xffffffff },	/*  AR8033: LAN8 */
		{ -1, 0, NULL, 0 },
	}, *p;
	static const struct gmac_spd_s {
		int port;			/* IPQ8074 GMAC port. */
		int min_speed;			/* minimal speed of the GMAC. */
	} gmac_spd[] = {
		{ 1, 1000 },			/* port 1, QCA8337 */
		{ -1, -1},
	}, *q;
	time_t t1;
	uint32_t r, id1, id2;
	const uint32_t *pval;
	unsigned int id_reg_addr;
	int i, r1, ret = 1, val, retry, fw, build, right;
	char iface[IFNAMSIZ];
	char speed_cmd[sizeof("ssdk_sh swX port speed get XYYYYYY")];

	/* Check all switch/PHY ports, except SFP+ */
	for (p = &phy_id_lists[0]; p->phy >= 0; ++p) {
		if (p->phy == 7 && !is_aqr_phy_exist())
			continue;

		if (p->phy_type)
			id_reg_addr = 0x40070002;
		else
			id_reg_addr = 2;

		if ((id1 = read_phy_reg(p->phy, id_reg_addr)) < 0)
			id1 = 0;
		if ((id2 = read_phy_reg(p->phy, id_reg_addr + 1)) < 0)
			id2 = 0;
		r = ((id1 & 0xFFFF) << 16) | (id2 & 0xFFFF);
		for (right = 0, pval = p->pval; !right && *pval != 0; ++pval) {
			if ((r & p->mask) != (*pval & p->mask))
				continue;
			right++;
		}

		if (!right) {
			ret = 0;
			dbg("%s: PHY %d wrong ID: %08x\n", __func__, p->phy, r);
		}
	}

	/* Check all ethX interfaces. */
	for (i = 0; i < 6; ++i) {
		if (i == 5 && !is_aqr_phy_exist())
			continue;
		snprintf(iface, sizeof(iface), "eth%d", i);
		if (!iface_exist(iface)) {
			dbg("%s: eth%d not found!\n", __func__, i);
			ret = 0;
		}
	}

	if (!ret)
		return ret;

	t1 = uptime();
	while (pids("aq-fw-download") && (uptime() - t1) < 20) {
		dbg("%s: waiting aq-fw-download ...\n", __func__);
		sleep(1);
	}

	/* Check GMAC that are connected to QCA8337 switch.
	 * For those ports that are connected to Malibu (PHY) and AQR107,
	 * GMAC speed is negotiated at run-time.  Don't check them.
	 */
	for (q = &gmac_spd[0]; q->port > 0; ++q) {
		retry = 20;
		snprintf(speed_cmd, sizeof(speed_cmd), "ssdk_sh %s port speed get %d", SWID_IPQ807X, q->port);
		while (retry-- > 0) {
			/* Example:
			 * / # ssdk_sh port speed get 1
			 *
			 *  SSDK Init OK![speed]:1000(Mbps)
			 * operation done.
			 */
			if ((r1 = parse_ssdk_sh(speed_cmd, "%*[^:]:%d", 1, &val)) != 0) {
				dbg("%s: cmd [%s] val [%d], return %d\n", __func__, speed_cmd, val, ret);
				ret = 0;
				break;
			}

			if (val >= q->min_speed)
				break;

			sleep(1);
		}

		if (val < q->min_speed) {
			dbg("IPQ8074 port %d speed %d < %d\n", q->port, val, q->min_speed);
			logmessage("ATE", "IPQ8074 port %d speed %d < %d\n", q->port, val, q->min_speed);
			ret = 0;

			if (q->port == 1) {
				int phy_addr[] = { 5, 8, -1 }, *pp;

				/* Check register 0x11 of related PHYs (Malibu, AR8035) and registers of QCA8337 switch.  */
				for (pp = &phy_addr[0]; *pp >= 0; ++pp) {
					char port_str[4];
					char *phy_reg0x11[] = { "ssdk_sh", SWID_IPQ807X, "debug", "phy", "get", port_str, "0x11", NULL };

					snprintf(port_str, sizeof(port_str), "%d", *pp);
					dbg("%s: Checking PHY%d reg 0x11:\n", __func__, *pp);
					_eval(phy_reg0x11, ">/dev/console", 0, NULL);
				}

				for (i = 0; i <= 6; ++i) {
					char grp_str[4];
					char *qca8337_reg[] = { "ssdk_sh", SWID_QCA8337, "debug", "reg", "dump", grp_str, NULL };

					snprintf(grp_str, sizeof(grp_str), "%d", i);
					_eval(qca8337_reg, ">/dev/console", 0, NULL);
				}
			}
		}
	}

	if (!ret)
		return ret;

	/* Read AQR firmware version. (PHY7) */
	if (is_aqr_phy_exist()) {
		fw = read_phy_reg(7 & 0xFF, 0x401e0020);
		build = read_phy_reg(7 & 0xFF, 0x401ec885);
		if (fw < 0 || build < 0) {
			dbg("Can't get AQR PHY firmware version.\n");
			ret = 0;
		} else {
			dbg("AQR PHY firmware %d.%d build %d.%d\n",
				(fw >> 8) & 0xFF, fw & 0xFF, (build >> 4) & 0xF, build & 0xF);
		}
	}

	return ret;
#endif	/* GTAXY16000 || RTAX89U */
#elif defined(RTCONFIG_SOC_IPQ40XX)
//TBD
#else
	FILE *fp;
	char cmd[64], buf[512];
	int rlen;

#ifdef RTCONFIG_QCA8033
	snprintf(cmd, sizeof(cmd), "cat /proc/link_status");
#else
	snprintf(cmd, sizeof(cmd), "swconfig dev %s port 0 get link", MII_IFNAME);
#endif
	if ((fp = popen(cmd, "r")) == NULL) {
		return 0;
	}
	rlen = fread(buf, 1, sizeof(buf), fp);
	pclose(fp);
	if (rlen <= 1)
		return 0;

	buf[rlen-1] = '\0';
#ifdef RTCONFIG_QCA8033
	if (strstr(buf, "link up (1000"))
#else
	if (strstr(buf, "link:up speed:1000"))
#endif
		return 1;
	return 0;
#endif
}

#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
/* Implementation cold boot calibration.
 * @mod:	module name that can handle cold boot calibration.
 * @return:
 * 	-1:	invalid parameter.
 * 	-2:	can't judge daemon_support of cnss2 driver is enabled or not.
 */
static int do_cold_boot_calibration(char *mod)
{
#if defined(RTCONFIG_GLOBAL_INI)
	char cold_boot[4] = { 0 }, daemon[4] = { 0 };
	char internal_ini_fn[sizeof(GLOBAL_INI_TOPDIR "/int/internal/QCA8074V2_i.iniXXXXXX")] = { 0 };
#endif
	char val[4] = "N";

	if (!mod || *mod == '\0')
		return -1;

#if defined(RTCONFIG_GLOBAL_INI)
	/* SPF10 ES QSDK */
	get_internal_ini_filename(internal_ini_fn, sizeof(internal_ini_fn));
	if (!get_parameter_from_ini_file("enable_cold_boot_support", cold_boot, sizeof(cold_boot), internal_ini_fn)) {
		if (f_write_string(CNSS2_SYSFS_COLDBOOT, cold_boot, 0, 0) <= 0)
			return -2;
	}
	if (!get_parameter_from_ini_file("enable_daemon_support", daemon, sizeof(daemon), internal_ini_fn)) {
		if (f_write_string(CNSS2_SYSFS_DAEMON, daemon, 0, 0) <= 0)
			return -2;
		strlcpy(val, safe_atoi(daemon)? "Y" : "N", sizeof(val));
	}
	if (*cold_boot != '\0' && !safe_atoi(cold_boot)) {
		dbg("No cold_boot_support\b");
		return 0;
	}
#else
	/* Load cnssdaemon if daemon_support of cnss2 driver is enabled. */
	if (f_read_string("/sys/module/cnss2/parameters/daemon_support", val, sizeof(val)) <= 0)
		return -2;
#endif
	if (!strcmp(val, "N"))
		return 0;

	eval("mkdir", "-p", "/tmp/wifi");	/* /data/misc/wifi ==> /tmp/wifi */
	if (!pids("cnssdaemon")) {
		pid_t *pidList, *p;
		char oom_score_adj[sizeof("/proc/XXXXX/oom_score_adjXXX")];

		eval("cnssdaemon");
		pidList = find_pid_by_name("cnssdaemon");
		for (p = pidList; *p; p++) {
			snprintf(oom_score_adj, sizeof(oom_score_adj), "/proc/%d/oom_score_adj", *p);
			f_write_string(oom_score_adj, "-17", 0, 0);
		}
		free(pidList);
	}

	if (f_exists("/data/misc/wifi/wlfw_cal_01.bin"))
		return 0;

	if (module_loaded(mod))
		return -3;

	eval("modprobe", "-s", mod, "testmode=7");
	if (!module_loaded(mod))
		return -4;
#if defined(RTCONFIG_GLOBAL_INI)
	eval("modprobe", "-s", "wifi_3_0");
	eval("modprobe", "-r", "wifi_3_0");
#endif
	eval("modprobe", "-r", mod);
	if (module_loaded(mod))
		return -5;

	return 0;
}
#endif

#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)

/* total pbuf size is 160 bytes,allocate memory for 8712 pbufs */
#define RAM256_OLCFG_EXTRA_PBUF_CORE0		"1400000"
#define RAM256_OLCFG_2B_ON_HIGH_WATER_CORE0	"20432"
#define RAM256_OLCFG_2B_ON_WIFI_POOL_BUF	"0"
/* total pbuf size is 160 bytes,allocate memory for 27096 pbufs */
#define RAM512_OLCFG_EXTRA_PBUF_CORE0		"4400000"
#define RAM512_OLCFG_2B_ON_HIGH_WATER_CORE0	"38816"
#define RAM512_OLCFG_2B_ON_WIFI_POOL_BUF	"16384"

#if defined(GTAXY16000)			/* HK09 */
#define OLCFG_EXTRA_PBUF_CORE0		"9200000"
#define OLCFG_OFF_HIGH_WATER_CORE0	"8704"
#define OLCFG_OFF_WIFI_POOL_BUF		"0"
#define OLCFG_2B_ON_HIGH_WATER_CORE0	"68904"
#define OLCFG_2B_ON_WIFI_POOL_BUF	"32768"
#elif defined(RTAX89U)			/* HK01 */
#define OLCFG_EXTRA_PBUF_CORE0		"7800000"
#define OLCFG_OFF_HIGH_WATER_CORE0	"8704"
#define OLCFG_OFF_WIFI_POOL_BUF		"0"
#define OLCFG_2B_ON_HIGH_WATER_CORE0	"51008"
#define OLCFG_2B_ON_WIFI_POOL_BUF	"24576"
#else
#error FIXME
#endif

#else
#define OLCFG_EXTRA_PBUF_CORE0		"5939200"
#define OLCFG_OFF_HIGH_WATER_CORE0	"8704"
#define OLCFG_OFF_WIFI_POOL_BUF		"8576"
#define OLCFG_1B_ON_HIGH_WATER_CORE0	"43008"
#define OLCFG_1B_ON_WIFI_POOL_BUF	"20224"
#define OLCFG_2B_ON_HIGH_WATER_CORE0	"59392"
#define OLCFG_2B_ON_WIFI_POOL_BUF	"36608"
#endif

/**
 * Low level function to load QCA WiFi driver.
 * @testmode:	if true, load WiFi driver as test mode which is required in ATE mode.
 */
static void __load_wifi_driver(int testmode)
{
	char country[FACTORY_COUNTRY_CODE_LEN + 1], code_str[6], prefix[sizeof("wlXXXXX_")];
	int i, unit, len, olcfg __attribute__((unused)) = nss_wifi_offloading();;
	FILE *fp_wifi;
	const char **up;
	const struct irq_smp_affinity_s *irqp;
	const unsigned int daol_type = qca_wifi_type & QWIFI_DAOL_TYPE;
	const unsigned int df_type = qca_wifi_type & QWIFI_DF_TYPE;
	char param[512], *s = &param[0], qca_nv[64], *val, *stats_cmd;
	char vphy[IFNAMSIZ] = { 0 }, path_wifi[sizeof("/tmp/wifiXXXX.sh")];
	char *argv[50] = {
		"modprobe", "-s", NULL
	}, **v;
	struct load_wifi_kmod_seq_s *p = &load_wifi_kmod_seq[0];
	int band2fid[WL_NR_BANDS] = { 2, 5, 5, 60 };
	int total_mem __attribute__((unused)) = get_meminfo_item("MemTotal");
#if defined(RTCONFIG_CFG80211)
	const int cfg80211 __attribute__((unused)) = 1;
#else
	const int cfg80211 __attribute__((unused)) = 0;
#endif
#if defined(RTCONFIG_WIFI_QCA9990_QCA9990) || \
    defined(RTCONFIG_WIFI_QCA9994_QCA9994) || \
    defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	char buf[16];
	const char *extra_pbuf_core0 = "0";
	const char *n2h_high_water_core0 = OLCFG_OFF_HIGH_WATER_CORE0, *n2h_wifi_pool_buf = OLCFG_OFF_WIFI_POOL_BUF;
	int r, r0, r1, r2, l0, l1, l2;
#endif
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	const int wifi_stats = 1;	/* enable_ol_stats must be enabled on Hawkeye platform. */
#else
	const int wifi_stats = 0;
#endif

#if defined(RTCONFIG_GLOBAL_INI)
	// Copy /ini/* to /etc/Wireless/ini
	eval("mkdir", "-p", GLOBAL_INI_TOPDIR);
	system("cp -a /ini/* " GLOBAL_INI_TOPDIR);
#endif

#if defined(RTCONFIG_GLOBAL_INI)
	v = &argv[2];
	*param = '\0';
	s = &param[0];
	len = sizeof(param);

	if (!testmode) {
		DPARM(v, s, len, "cfg80211_config=%d", cfg80211);
	}
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	/* detect_qcawifi() */
	if (total_mem <= (256 * 1024)) {
		char *nr_dp_rings[] = {
			"dp_rxdma_monitor_buf_ring=128",
			"dp_rxdma_monitor_dst_ring=128",
			"dp_rxdma_monitor_desc_ring=128",
			NULL
		};

		__update_ini_file(QCA8074_I_INI, nr_dp_rings);
		__update_ini_file(QCA8074V2_I_INI, nr_dp_rings);
	}
	update_ini_file(GLOBAL_I_INI, "hw_mode_id=1");

	/* load_qcawifi() */
	*v++ = "qwrap_enable=0";	/* SPF10.0 FC */
	DPARM(v, s, len, "nss_wifi_olcfg=%d", !!olcfg);
	if (olcfg) {
		*v++ = "rx_hash=0";
	}
#endif
	for (up = global_ini_params; up != NULL && *up != NULL; up++) {
		snprintf(qca_nv, sizeof(qca_nv), "qca_%s", *up);
		if (!(val = nvram_get(qca_nv)))
			continue;
		DPARM(v, s, len, "%s=%s", *up, val);
	}
	if (testmode) {
		*v++ = "soc_probe_module_load=1";
		*v++ = "fbc_enabled=1";
		*v++ = "testmode=1";
		*v++ = "cfg80211_config=1";
	}
	__update_ini_file(GLOBAL_INI, &argv[2]);
#endif

#if defined(RTCONFIG_WIFI_QCA9990_QCA9990) || \
    defined(RTCONFIG_WIFI_QCA9994_QCA9994) || \
    defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	/* Always wait NSS ready whether NSS WiFi offloading is enabled or not. */
	for (i = 0, *buf = '\0'; i < 10; ++i) {
		r0 = f_read_string(N2H_HIGH_WATER_CORE0_FN, buf, sizeof(buf));
		if (r0 > 0 && strlen(buf) > 0)
			break;
		else {
			dbg(".");
			sleep(1);
		}
	}

	/* Always use maximum extra_pbuf_core0.
	 * Because it can't be changed if it is allocated, write non-zero value.
	 */
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	if (total_mem <= (256 * 1024)) {
		extra_pbuf_core0 = RAM256_OLCFG_EXTRA_PBUF_CORE0;
		n2h_high_water_core0 = RAM256_OLCFG_2B_ON_HIGH_WATER_CORE0;
		n2h_wifi_pool_buf = RAM256_OLCFG_2B_ON_WIFI_POOL_BUF;
	} else if (total_mem <= (512 * 1024)) {
		extra_pbuf_core0 = RAM512_OLCFG_EXTRA_PBUF_CORE0;
		n2h_high_water_core0 = RAM512_OLCFG_2B_ON_HIGH_WATER_CORE0;
		n2h_wifi_pool_buf = RAM512_OLCFG_2B_ON_WIFI_POOL_BUF;
	} else{
		extra_pbuf_core0 = OLCFG_EXTRA_PBUF_CORE0;
		if (olcfg) {
			n2h_high_water_core0 = OLCFG_2B_ON_HIGH_WATER_CORE0;
			n2h_wifi_pool_buf = OLCFG_2B_ON_WIFI_POOL_BUF;
		}
	}
#else
	if (olcfg == 1 || olcfg == 2 || olcfg == 4) {
		n2h_high_water_core0 = OLCFG_1B_ON_HIGH_WATER_CORE0;
		n2h_wifi_pool_buf = OLCFG_1B_ON_WIFI_POOL_BUF;
	} else if (olcfg) {
		n2h_high_water_core0 = OLCFG_2B_ON_HIGH_WATER_CORE0;
		n2h_wifi_pool_buf = OLCFG_2B_ON_WIFI_POOL_BUF;
	}
#endif

	l0 = strlen(extra_pbuf_core0);
	l1 = strlen(n2h_high_water_core0);
	l2 = strlen(n2h_wifi_pool_buf);
	for (i = 0; olcfg && i < 10; ++i) {
		f_read_string(N2H_HIGH_WATER_CORE0_FN, buf, sizeof(buf));

		*buf = '\0';
		r0 = l0;
		r = f_read_string(EXTRA_PBUF_CORE0_FN, buf, sizeof(buf));
		if (r > 0 && atol(buf)) {
			//dbg("%s: extra_pbuf_core0 is allocated!!! [%s]\n", __func__, buf);
		}
		if (r <= 0 || (r > 0 && atol(buf) < atol(extra_pbuf_core0)))
			r0 = f_write_string(EXTRA_PBUF_CORE0_FN, extra_pbuf_core0, 0, 0);

		r1 = f_write_string(N2H_HIGH_WATER_CORE0_FN, n2h_high_water_core0, 0, 0);
		r2 = f_write_string(N2H_WIFI_POOL_BUF_FN, n2h_wifi_pool_buf, 0, 0);
		if (r0 < l0 || r1 < l1 || r2 < l2) {
			dbg(".");
			sleep(1);
			continue;
		}
		break;
	}
#endif


	for (i = 0, len = sizeof(param), p = &load_wifi_kmod_seq[i]; len > 0 && i < ARRAY_SIZE(load_wifi_kmod_seq); ++i, ++p) {
		/* Don't load a DA/OL or adf/qdf module if it is not used. */
		if (qca_wifi_type) {
			if (((p->flags & QWIFI_DAOL_TYPE) && !(daol_type & p->flags))
			 || ((p->flags & QWIFI_DF_TYPE) && !(df_type & p->flags)))
			{
				// dbg(" %s: ignore %s. (flags %x/%x)\n", __func__, p->kmod_name, p->flags, qca_wifi_type);
				continue;
			}
		}
		if (module_loaded(p->kmod_name))
			continue;

		v = &argv[2];
		*v++ = p->kmod_name;
		*param = '\0';
		s = &param[0];

		/* If module-specific hook function for parameters adjustment is defined, execute it. */
		if (!testmode) {
			if (p->mission_mode_param_hook_fn)
				p->mission_mode_param_hook_fn(&v, &s, &len);
		} else {
			if (p->test_mode_param_hook_fn)
				p->test_mode_param_hook_fn(&v, &s, &len);
		}

		/* Extra parameters defined in qca_XXX nvram variables for experiment purpose. */
		for (up = p->params; up != NULL && *up != NULL; up++) {
			snprintf(qca_nv, sizeof(qca_nv), "qca_%s", *up);
			if (!(val = nvram_get(qca_nv)))
				continue;
			DPARM(v, s, len, "%s=%s", *up, val);
		}

		*v++ = NULL;
		_eval(argv, NULL, 0, NULL);

		if (p->load_sleep)
			sleep(p->load_sleep);

		if (p->post_fn)
			p->post_fn();
	}

	if (testmode)
		return;

	eval("mkdir", "-p", "/etc/Wireless/sh");
	for (unit = WL_2G_BAND; unit < MAX_NR_WL_IF; ++unit) {
		SKIP_ABSENT_BAND(unit);

		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		strlcpy(vphy, get_vphyifname(unit), sizeof(vphy));
		snprintf(path_wifi, sizeof(path_wifi), "/tmp/%s.sh", vphy);
		if (!(fp_wifi = fopen(path_wifi, "w"))) {
			dbg("%s: Can't open %s for writing!\n", __func__, path_wifi);
			continue;
		}

		fprintf(fp_wifi, "#!/bin/sh\n");
		switch (unit) {
#if defined(RTCONFIG_WIGIG)
		case WL_60G_BAND:
			/* Pure iw + nl80211 based band. */
			/* country code of 802.11ad Wigig can be handled by hostapd too. */
			strlcpy(country, nvram_safe_get("wl3_country_code"), sizeof(country));
			if (country_to_code(country, band2fid[unit], code_str, sizeof(code_str)) < 0)
				country_to_code("DB", band2fid[unit], code_str, sizeof(code_str));
			eval("iw", "reg", "set", code_str);

			tweak_wifi_ps(WIF_60G);
			break;
#endif
		default:
			/* Wireless Extension or cfg80211 + nl80211 based band. */

			/* Country */
			strlcpy(country, nvram_pf_safe_get(prefix, "country_code"), sizeof(country));
			if (country_to_code(country, band2fid[unit], code_str, sizeof(code_str)) < 0)
				country_to_code("DB", band2fid[unit], code_str, sizeof(code_str));
			fprintf(fp_wifi, IWPRIV " %s setCountryID %s\n", vphy, code_str);

			/* OL stats */
			stats_cmd = "enable_ol_stats";
#if defined(RTCONFIG_WIFI_QCA9557_QCA9882) || defined(RTCONFIG_QCA953X) || defined(RTCONFIG_QCA956X) || defined(RTCONFIG_QCN550X)
			if (unit == WL_2G_BAND)
				stats_cmd = "disablestats";
#endif
			fprintf(fp_wifi, IWPRIV " %s %s %d\n", vphy, stats_cmd, wifi_stats);

			/* TX power adjustment. */
			if (find_word(nvram_safe_get("rc_support"), "pwrctrl")) {
				fprintf(fp_wifi, IWPRIV " %s txpwrpc %s\n",
					vphy, nvram_pf_safe_get(prefix, "txpower"));
			}

#if defined(RTCONFIG_WIFI_SON)
			if (sw_mode()!=SW_MODE_REPEATER && nvram_match("wifison_ready", "1")) {
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
		        	if(strlen(nvram_safe_get("cfg_group"))){
#else
				if(nvram_get_int("x_Setting")) {
#endif
				eval(IWPRIV, (char*) VPHY_2G, "no_vlan", "1");
				eval(IWPRIV, (char*) VPHY_5G, "no_vlan", "1");
				//send RCSA to uplink/CAP/PAP when detect radar
				if(nvram_get_int("dfs_check_period"))
					eval(IWPRIV, (char*) VPHY_5G, "CSwOpts", "0x30");
#if defined(RTCONFIG_HAS_5G_2)
				if (nvram_get_int("ncb_enable"))
					doSystem("iwpriv %s ncb_enable %s", VPHY_5G, nvram_get("ncb_enable"));
				eval("iwpriv", (char*) VPHY_5G2, "no_vlan", "1");
#endif
				}
			}
#endif /* WIFI_SON */
			tweak_wifi_ps(vphy);
		}

		fclose(fp_wifi);
		chmod(path_wifi, 0777);
	}

	for (irqp = &wifi_irq_smp_affinity_tbl[0]; irqp->irq >= 0; ++irqp) {
		set_irq_smp_affinity(irqp->irq, irqp->cpu_mask);
	}

#if !defined(RTCONFIG_CFG80211)
	/* add acs channel weight */
	acs_ch_weight_param();
#endif	/* !RTCONFIG_CFG80211 */

#if defined(RTCONFIG_SOC_IPQ40XX)
	f_write_string("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "performance", 0, 0); // for throughput
	f_write_string("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq", "716000", 0, 0); // for throughput
#endif
}

void load_wifi_driver(void)
{
#if defined(RTCONFIG_SOC_IPQ8064) || defined(RTCONFIG_SOC_IPQ8074)
	f_write_string("/proc/sys/vm/pagecache_ratio", "25", 0, 0);
	f_write_string("/proc/net/skb_recycler/max_skbs", "2176", 0, 0);
#elif defined(RTCONFIG_SOC_IPQ40XX)
	/* For lower memory system, e.g., IPQ40xx + 128MB RAM models. */
	if (get_meminfo_item("MemTotal") <= 131072) {
		f_write_string("/proc/net/skb_recycler/flush", "1", 0, 0);
		f_write_string("/proc/net/skb_recycler/max_skbs", "10", 0, 0);
		f_write_string("/proc/net/skb_recycler/max_spare_skbs", "10", 0, 0);
	}
#elif defined(RTN19) || defined(RTAC59U) || defined(MAPAC1750)
	f_write_string("/proc/net/skb_recycler/flush", "1", 0, 0);
	f_write_string("/proc/net/skb_recycler/max_skbs", "256", 0, 0);
#endif

	__load_wifi_driver(0);
}

void load_testmode_wifi_driver(void)
{
	__load_wifi_driver(1);
}

void set_uuid(void)
{
	int len;
	char *p, uuid[60];
	FILE *fp;

	fp = popen("cat /proc/sys/kernel/random/uuid", "r");
	 if (fp) {
	    memset(uuid, 0, sizeof(uuid));
	    fread(uuid, 1, sizeof(uuid), fp);
	    for (len = strlen(uuid), p = uuid; len > 0; len--, p++) {
		    if (isxdigit(*p) || *p == '-')
			    continue;
		    *p = '\0';
		    break;
	    }
	    nvram_set("uuid",uuid);
	    pclose(fp);
	 }   
}

/**
 * Down all VAP interfaces, kill all related hostapd instance and
 * delete VAP interface if @unregister_vap is true.
 * @unregister_vap:	Unregister VAP interface if true, except WiGig interface.
 */
void deinit_all_vaps(const int unregister_vap)
{
	int unit, sunit, max_sunit;
	char wif[IFNAMSIZ], prefix[sizeof("wlX_YYY")];
	char hconf_path[sizeof("/etc/Wireless/conf/hostapd_X.confXXX") + IFNAMSIZ];
#if defined(RTCONFIG_SINGLE_HOSTAPD)
	char sock_path[sizeof("/var/run/hostapd/XXXXXX") + IFNAMSIZ];
#else
	char pid_path[sizeof("/var/run/hostapd_athXXX.pidYYYYYY")];
#endif

	for (unit = 0; unit < MAX_NR_WL_IF; ++unit) {
		SKIP_ABSENT_BAND(unit);
		max_sunit = num_of_mssid_support(unit);
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		for (sunit = 0; sunit <= max_sunit; ++sunit) {
			__get_wlifname(unit, sunit, wif);
			snprintf(hconf_path, sizeof(hconf_path), "/etc/Wireless/conf/hostapd_%s.conf", wif);
#if defined(RTCONFIG_SINGLE_HOSTAPD)
			snprintf(sock_path, sizeof(sock_path), "/var/run/hostapd/%s", wif);
			if (f_exists(sock_path))
				eval(QWPA_CLI, "-g", QHOSTAPD_CTRL_IFACE, "raw", "REMOVE", wif);
#else
			snprintf(pid_path, sizeof(pid_path), "/var/run/hostapd_%s.pid", wif);
			if (f_exists(pid_path))
				kill_pidfile_tk(pid_path);
#endif
			if (unregister_vap && f_exists(hconf_path))
				unlink(hconf_path);
			if (!iface_exist(wif))
				continue;
			ifconfig(wif, 0, NULL, NULL);
			if (!unregister_vap || unit == WL_60G_BAND)
				continue;
			destroy_vap(wif);
		}
	}

#if defined(RTCONFIG_SINGLE_HOSTAPD)
	kill_pidfile_tk(QHOSTAPD_PID_PATH);
#endif

	/* in case of pid file is gone...*/
	eval("killall", "hostapd");
}

/**
 * Rebuild main VAP of each bands.
 * @return:
 * 	0:	success
 *  otherwise:	error
 */
int rebuild_main_vap(void)
{
	int band;
	char vap[IFNAMSIZ];
#if defined(RTCONFIG_SINGLE_HOSTAPD)
	char sock_path[sizeof("/var/run/hostapd/XXXXXX") + IFNAMSIZ];
#else
	char pid_path[sizeof("/var/run/hostapd_athXXX.pidYYYYYY")];
#endif

	for (band = WL_2G_BAND; band < MAX_NR_WL_IF; ++band) {
		SKIP_ABSENT_BAND(band);
		__get_wlifname(band, 0, vap);
#if defined(RTCONFIG_SINGLE_HOSTAPD)
		snprintf(sock_path, sizeof(sock_path), "/var/run/hostapd/%s", vap);
		if (f_exists(sock_path))
			eval(QWPA_CLI, "-g", QHOSTAPD_CTRL_IFACE, "raw", "REMOVE", vap);
#else
		snprintf(pid_path, sizeof(pid_path), "/var/run/hostapd_%s.pid", vap);
		if (f_exists(pid_path))
			kill_pidfile_tk(pid_path);
#endif
		if (band == WL_60G_BAND)
			continue;
		if (iface_exist(vap))
			destroy_vap(vap);
		create_vap(vap, band, "ap");
	}

	return 0;
}

static int create_node=0;
void init_wl(void)
{
   	int unit;
	char *p, *ifname;
	char *wl_ifnames;
	char path_wifi[sizeof("/tmp/wifiXXXX.sh")];
#ifdef RTCONFIG_WIRELESSREPEATER
	int wlc_band;
#endif

#if defined(RTCONFIG_WIFI_DRV_DISABLE)
	if (nvram_match("lyra_disable_wifi_drv", "1"))
		return;
#endif

	if(!create_node)
	{ 
		load_wifi_driver();
		sleep(2);

		for (unit = WL_2G_BAND; unit < MAX_NR_WL_IF; ++unit) {
			SKIP_ABSENT_BAND(unit);

			snprintf(path_wifi, sizeof(path_wifi), "/tmp/%s.sh", get_vphyifname(unit));
			eval(path_wifi);
			unlink(path_wifi);
		}

		dbG("init_wl:create wi node\n");
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
		if(sw_mode() != SW_MODE_REPEATER)
			add_bh_network();
#endif
		if ((wl_ifnames = strdup(nvram_safe_get("lan_ifnames"))) != NULL) 
		{
			p = wl_ifnames;
			while ((ifname = strsep(&p, " ")) != NULL) {
				while (*ifname == ' ') ++ifname;
				if (*ifname == 0) break;
				SKIP_ABSENT_FAKE_IFACE(ifname);

				//create ath00x & ath10x 
				if (!strncmp(ifname,WIF_2G,strlen(WIF_2G))
#if defined(RTCONFIG_CONCURRENTREPEATER)
				     || !strncmp(ifname,STA_2G,strlen(STA_2G))
#endif
				   )
					unit = 0;
				else if (!strncmp(ifname,WIF_5G,strlen(WIF_5G))
#if defined(RTCONFIG_CONCURRENTREPEATER)
					|| !strncmp(ifname,STA_5G,strlen(STA_5G))
#endif
					)
					unit = 1;
				else if (!strncmp(ifname,WIF_5G2,strlen(WIF_5G2)))
					unit = 2;
#if defined(RTCONFIG_WIGIG)
				else if(strncmp(ifname,WIF_60G,strlen(WIF_60G))==0)
					unit = 3;
#endif
				else
			   		unit=-99;	

				switch (unit) {
				case WL_2G_BAND:	/* fall-through */
				case WL_5G_BAND:	/* fall-through */
				case WL_5G_2_BAND:
#if defined(RTCONFIG_CONCURRENTREPEATER)
					if (!strncmp(ifname, STA_2G, strlen(STA_2G)) || !strncmp(ifname, STA_5G, strlen(STA_5G))) {
						if(sw_mode() == SW_MODE_REPEATER) {
							create_vap(ifname, unit, "sta");
						}
					} else {
						create_vap(ifname, unit, "ap");
						sleep(1);
					}
#else
					create_vap(ifname, unit, "ap");
					sleep(1);
#if defined(RTCONFIG_REPEATER_STAALLBAND)
					if (sw_mode() == SW_MODE_REPEATER) {
						dbG("\ncreate a STA node %s from %s\n", get_staifname(unit), get_vphyifname(unit));
						doSystem("wlanconfig %s create wlandev %s wlanmode sta nosbeacon",
							get_staifname(unit), get_vphyifname(unit));
						sleep(1);
					}
#endif
#endif	    
					break;
#if defined(RTCONFIG_WIGIG)
				case WL_60G_BAND:
					/* Nothing to do. Both VAP and VPHY interfaces are created by driver automatically. */
					break;
#endif
				default:
					if (!strncmp(ifname, "eth", 3))
						break;
					dbg("%s: Unknown wl%d band, ifname [%s]!\n", __func__, unit, ifname);
				}
			}
			free(wl_ifnames);
		}
		create_node=1;
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
		if(sw_mode() != SW_MODE_REPEATER)
			del_bh_network();
#endif

#ifdef RTCONFIG_WIRELESSREPEATER
#if !defined(RTCONFIG_CONCURRENTREPEATER) && !defined(RTCONFIG_REPEATER_STAALLBAND)
		if(sw_mode()==SW_MODE_REPEATER)
		{ 
		  	wlc_band=nvram_get_int("wlc_band");
			if (wlc_band != WL_60G_BAND) {
				create_vap(get_staifname(wlc_band), wlc_band, "sta");
			}
		}      
#endif
#endif

		/* Calculate 40/80/160MHz bandwidth capability based on channel list.
		 * Main VAP must ready before doing this.
		 */
		for (unit = WL_2G_BAND; unit < MAX_NR_WL_IF; ++unit) {
			SKIP_ABSENT_BAND(unit);
			calculate_bw_of_each_channel(unit);
		}

#ifdef RTCONFIG_WIFI_SON
		if(sw_mode()!=SW_MODE_REPEATER && nvram_match("wifison_ready", "1")) {
			if((sw_mode() == SW_MODE_AP && !nvram_match("cfg_master", "1")) || nvram_match("wps_e_success", "1"))
			{
				if(nvram_get_int("x_Setting"))
				{
					_dprintf("=>init_wl: create sta vaps\n");
					doSystem("wlanconfig %s create wlandev %s wlanmode sta nosbeacon",
						get_staifname(1), get_vphyifname(1));
					sleep(1);
					ifconfig(get_staifname(1), IFUP, NULL, NULL);
				}
			}
		}
#endif
#ifdef RTCONFIG_AMAS
		if((sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")))
		{
			int i;
			char *sta_ifnames = strdup(nvram_safe_get("sta_ifnames"));
			char *skip_ifnames = strdup(nvram_safe_get("skip_ifnames"));
			_dprintf("=>init_wl: create sta vaps: %d\n", MAX_NR_WL_IF);
			for (i = 0; i < MAX_NR_WL_IF; i++) {
				char *sta = get_staifname(i);
				if (sta_ifnames && strstr(sta_ifnames, sta) == NULL)
					continue;
				if (skip_ifnames && strstr(skip_ifnames, sta) != NULL)
					continue;
				doSystem("wlanconfig %s create wlandev %s wlanmode sta nosbeacon",
					get_staifname(i), get_vphyifname(i));
			}
			free(sta_ifnames);
			free(skip_ifnames);
		}
#endif	/* RTCONFIG_AMAS */
	}

#ifdef RTCONFIG_WIFI_SON
#ifdef RTCONFIG_AMAS
	if(!nvram_match("wifison_ready", "1"))
		goto skip_wifison;
#endif	/* RTCONFIG_AMAS */

	if(sw_mode() == SW_MODE_REPEATER)
		goto skip_wifison;

	int i;
	if(sw_mode() == SW_MODE_AP && !nvram_match("cfg_master", "1")) //router->ap
	{
		if(nvram_get_int("x_Setting"))
		{
			char stamac[512];
			char *tmp_pt;
			i=1;
			{
				tmp_pt = get_stamac(i, stamac, sizeof(stamac));
				if(!tmp_pt)
				{
					char *sta = get_staifname(i);
					if(nvram_match("wl1_country_code", "GB"))
					{
						_dprintf("=> RE: hold for cac timeout...\n");
						while(get_cac_state())
						{
							_dprintf(".");
						}
						_dprintf("exit!!!\n");
					}
					_dprintf("=> switch router to ap : create %s\n", sta);
					doSystem("wlanconfig %s create wlandev %s wlanmode sta nosbeacon",
						sta, get_vphyifname(i));
					sleep(1);
					ifconfig(sta, IFUP, NULL, NULL);
				}
			}
		}
	}

	if(nvram_get_int("wl0.1_bss_enabled"))
	{
		eval("vconfig","set_name_type","DEV_PLUS_VID_NO_PAD");
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
		char vif[10];		
 		sprintf(vif,"%s.%s",WIF_5G_BH,"55");
                eval("vconfig", "add",WIF_5G_BH,"55");
                eval("ifconfig",vif,"up");
               	eval("brctl","addif",BR_GUEST,vif);
#else
		eval("vconfig", "add","ath1","55");
		eval("ifconfig","ath1.55","up");
		eval("brctl","addif",BR_GUEST,"ath1.55");
#endif
#ifdef RTCONFIG_ETHBACKHAUL
#if defined(RTCONFIG_DETWAN)
 		if (get_role()==0) // CAP
                {
                        char vif[15];
                        if (strcmp(CONFIGURED_WAN_NIC, DEFAULT_WAN_NIC)==0)
                                sprintf(vif,"%s",DEFAULT_LAN_NIC);
                        else
                                sprintf(vif,"%s",DEFAULT_WAN_NIC);
                
			eval("vconfig", "add",vif,"55");
			sprintf(vif,"%s.55",vif);
			eval("ifconfig",vif,"up");
			eval("brctl","addif",BR_GUEST,vif);
		}
		else
#endif
		{
			eval("vconfig", "add", MII_IFNAME, "55");
			eval("ifconfig", MII_IFNAME".55", "up");
			eval("brctl", "addif", BR_GUEST, MII_IFNAME".55");
		}
#endif
	}
	
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
        if(strlen(nvram_safe_get("cfg_group")))
        {
		char vif[10],cipher[100];
		if(nvram_match("wl1.1_bss_enabled","0"))
		{
                	nvram_set("wl1.1_bss_enabled","1");
                	nvram_set("wl1.1_expire_tmp","0");
                	nvram_set("wl1.1_auth_mode_x","psk2");
			memset(cipher,0,sizeof(cipher));
			addXOR(nvram_get("cfg_group"),cipher);
                	setting_hash_ap(cipher,strlen(nvram_get("cfg_group")));
                	nvram_commit();
		}
        	eval("vconfig","set_name_type","DEV_PLUS_VID_NO_PAD");
        	if(check_bh(WIF_5G_BH))
        	{
			sprintf(vif,"%s.%s",WIF_5G_BH,"1");
			eval("vconfig", "add",WIF_5G_BH,"1");
			eval("ifconfig",vif,"up");
			eval("brctl","addif",nvram_safe_get("lan_ifname"),vif);
        	}
       		if(!nvram_match("cfg_master","1"))
        	{
               		if(check_bh(STA_5G))
                	{
				sprintf(vif,"%s.%s",STA_5G,"1");
				eval("vconfig", "add",STA_5G,"1");
				eval("ifconfig",vif,"up");
				eval("brctl","addif",nvram_safe_get("lan_ifname"),vif);
                	}
		}
		
        }
#endif
skip_wifison: 
#endif
	return ;
}

void fini_wl(void)
{
	int i;
	struct load_wifi_kmod_seq_s *wp;
	char ifname[IFNAMSIZ];
#if defined(RTCONFIG_CFG80211)
	char path_pid[sizeof("/var/run/hostapd_cli-athX.pid") + IFNAMSIZ];
#endif

#if !defined(RTCONFIG_WIFI_SON)
#if defined(RTCONFIG_QCA) && defined(RTCONFIG_SOC_IPQ40XX)
        if(nvram_get_int("restwifi_qis")==0) //reduce the finish time of QIS 
        {
		stop_bluetooth_service();
		eval("killall", "hostapd");
#if defined(RTCONFIG_AMAS)
		if(sw_mode()==SW_MODE_ROUTER)
			nvram_set_int("cfg_master", 1);
#endif
		nvram_set("restwifi_qis","1");
		nvram_commit();
		bg=0;
                return ;
        }
	else
		bg=0;
#endif
#endif 

#if defined(RTCONFIG_QCA_LBD)
	stop_qca_lbd();
#endif

	stop_mcsd();

	dbG("fini_wl:destroy wi node\n");
#if defined(RTCONFIG_CFG80211)
	for (i = WL_2G_BAND; i < MAX_NR_WL_IF; ++i) {
		SKIP_ABSENT_BAND(i);

		__get_wlifname(i, 0, ifname);
		snprintf(path_pid, sizeof(path_pid), "/var/run/hostapd_cli-%s.pid", ifname);
		if (f_exists(path_pid))
			kill_pidfile(path_pid);
	}
#endif

	deinit_all_vaps(1);

	/* Delete all STA interfaces. */
	kill_wifi_wpa_supplicant(-1);
	for (i = 0; i < MAX_NR_WL_IF; ++i) {
		SKIP_ABSENT_BAND(i);

		strlcpy(ifname, get_staifname(i), sizeof(ifname));
		if (!iface_exist(ifname) || i == WL_60G_BAND)
			continue;

		ifconfig(ifname, 0, NULL, NULL);
		destroy_vap(ifname);
	}

	create_node=0;

	ifconfig(VPHY_2G, 0, NULL, NULL);
	ifconfig(VPHY_5G, 0, NULL, NULL);
#if defined(RTCONFIG_HAS_5G_2) || \
 	((defined(RTCONFIG_PCIE_QCA9888) || defined(RTCONFIG_PCIE_QCA9984)) && defined(RTCONFIG_SOC_IPQ40XX))
	ifconfig(VPHY_5G2, 0, NULL, NULL);
#endif
#if defined(RTCONFIG_WIGIG)
	ifconfig(WIF_60G, 0, NULL, NULL);
#endif
	for (i = ARRAY_SIZE(load_wifi_kmod_seq)-1, wp = &load_wifi_kmod_seq[i]; i >= 0; --i, --wp) {
		if (!module_loaded(wp->kmod_name))
			continue;

		if (!(wp->flags & QWIFI_STICK)) {
			eval("rmmod", wp->kmod_name);
			if (wp->remove_sleep)
				sleep(wp->remove_sleep);
		}
	}


}

static void chk_valid_country_code(char *country_code)
{
	if ((unsigned char)country_code[0]!=0xff)
	{
		//
	}
	else
	{
		strcpy(country_code, "DB");
	}
}

int get_mac_2g(unsigned char dst[])
{
	int bytes = 6;
	if (FRead(dst, OFFSET_MAC_ADDR_2G, bytes) < 0) {  // ET0/WAN is same as 2.4G
		_dprintf("%s: Fread Out of scope\n", __func__);
		return -1;
	}
	return 0;
}

int get_mac_5g(unsigned char dst[])
{
	int bytes = 6;
	if (FRead(dst, OFFSET_MAC_ADDR, bytes) < 0) { // ET1/LAN is same as 5G
		_dprintf("%s: Fread Out of scope\n", __func__);
		return -1;
	}
	return 0;
}

#if defined(RTCONFIG_HAS_5G_2)
int get_mac_5g_2(unsigned char dst[])
{
	int bytes = 6;
	if (FRead(dst, OFFSET_MAC_ADDR_5G_2, bytes) < 0) { // ET1/LAN is same as 5G
		_dprintf("%s: Fread Out of scope\n", __func__);
		return -1;
	}
	return 0;
}
#endif

#if defined(VZWAC1300) /* bad eeprom fix, temp workaround */
extern int update_qca_eeprom(unsigned int eeprom_size, unsigned int eeprom_csum_offset, unsigned int eeprom_offset, unsigned int data_offset, unsigned char *data, unsigned int len);
#endif
/**
 * Check @buf before setting it to @nv_name.
 * @return:
 * 	0:	success
 *     -1:	invalid parameter
 *   otherwise:	error
 */
static int sfunc_datecode(char *nv_name, unsigned char *buf)
{
	int year, month, day;
	char tmp[DATECODE_LENGTH + 1];

	if (!nv_name || *nv_name == '\0' || !buf)
		return -1;

	strlcpy(tmp, buf, 4 + 1);
	year = safe_atoi(tmp);
	strlcpy(tmp, buf + 4, 2 + 1);
	month = safe_atoi(tmp);
	strlcpy(tmp, buf + 6, 2 + 1);
	day = safe_atoi(tmp);

	if (year < 2018 || year > 2100 || month < 1 || month > 12 || day < 1 || day > 31) {
		nvram_set(nv_name, "");
		return 0;
	}

	nvram_set(nv_name, buf);
	return 0;
}

void init_syspara(void)
{
	unsigned char buffer[16];
	unsigned char *dst, *p;
	unsigned int bytes;
	char macaddr[] = "00:11:22:33:44:55";
	char macaddr2[] = "00:11:22:33:44:58";
	char country_code[FACTORY_COUNTRY_CODE_LEN+1];
	char pin[9];
	char productid[13];
	char fwver[8];
	char blver[32];
#ifdef RTCONFIG_ODMPID
#ifdef RTCONFIG_32BYTES_ODMPID
	char modelname[32];
#else
	char modelname[16];
#endif
#endif
#if defined(RTCONFIG_WIFI_DRV_DISABLE) /* for IPQ40XX */
	char disableWifiDrv;
#endif
	char ipaddr_lan[16];
	unsigned char factory_var_buf[256];
	struct factory_var_s {
		char *nv_name;
		unsigned int factory_offset;
		unsigned int length;
		int (*set_func)(char *nv_name, unsigned char *buf);
	} factory_var_tbl[] = {
		{ "HwId", OFFSET_HWID, HWID_LENGTH, NULL },
		{ "HwVer", OFFSET_HWVERSION, HWVERSION_LENGTH, NULL },
		{ "HwBom", OFFSET_HWBOM, HWBOM_LENGTH, NULL },
		{ "DCode", OFFSET_DATECODE, DATECODE_LENGTH, sfunc_datecode },

		{ NULL, 0, 0, NULL }
	}, *pfv;

#if defined(RT4GAC53U) || defined(MAPAC1750)
	boot_version_ck();
#endif
	set_basic_fw_name();

	/* /dev/mtd/2, RF parameters, starts from 0x40000 */
	dst = buffer;
	bytes = 6;
	memset(buffer, 0, sizeof(buffer));
	memset(country_code, 0, sizeof(country_code));
	memset(pin, 0, sizeof(pin));
	memset(productid, 0, sizeof(productid));
	memset(fwver, 0, sizeof(fwver));

	for (pfv = &factory_var_tbl[0]; pfv->nv_name && pfv->length; ++pfv) {
		if (pfv->length >= sizeof(factory_var_buf))
			continue;
		*factory_var_buf = 0xFF;
		if (FRead(factory_var_buf, pfv->factory_offset, pfv->length)) {
			nvram_set(pfv->nv_name, "");
			continue;
		}

		*(factory_var_buf + pfv->length) = '\0';
		if ((p = strchr(factory_var_buf, 0xFF)) != NULL)
			*p = '\0';
		if (*factory_var_buf == '\0' || *factory_var_buf == 0xFF) {
			nvram_set(pfv->nv_name, "");
			continue;
		}

		if (pfv->set_func) {
			pfv->set_func(pfv->nv_name, factory_var_buf);
		} else {
			nvram_set(pfv->nv_name, factory_var_buf);
		}
	}

	if (FRead(dst, OFFSET_MAC_ADDR_2G, bytes) < 0) {  // ET0/WAN is same as 2.4G
		_dprintf("READ MAC address 2G: Out of scope\n");
	} else {
		if (buffer[0] != 0xff)
			ether_etoa(buffer, macaddr);
	}

	if (FRead(dst, OFFSET_MAC_ADDR, bytes) < 0) { // ET1/LAN is same as 5G
		_dprintf("READ MAC address : Out of scope\n");
	} else {
		if (buffer[0] != 0xff)
			ether_etoa(buffer, macaddr2);
	}

#if defined(RTCONFIG_QCA_VAP_LOCALMAC)
	nvram_set("wl_mssid", "1");
#else
	if (!mssid_mac_validate(macaddr) || !mssid_mac_validate(macaddr2))
		nvram_set("wl_mssid", "0");
	else
		nvram_set("wl_mssid", "1");
#if defined(RTAC58U)
	if (check_mid("Hydra")) {
		nvram_set("wl_mssid", "1"); // Hydra's MAC may not be multible of 4
	}
#endif
#endif

#if defined(RTCONFIG_SOC_IPQ8064) || defined(RTCONFIG_SOC_IPQ8074) || defined(RTCONFIG_QCA_VAP_LOCALMAC)
#if defined(RTCONFIG_QCA_VAP_LOCALMAC)
	nvram_set("wl0macaddr", macaddr);
	nvram_set("wl1macaddr", macaddr2);
#endif
	/* Hack et0macaddr/et1macaddr after MAC address checking of wl_mssid. */
	ether_atoe(macaddr, buffer);
	buffer[5] += 1;
	ether_etoa(buffer, macaddr);
	ether_atoe(macaddr2, buffer);
	buffer[5] += 1;
	ether_etoa(buffer, macaddr2);
#endif

#if defined(RTCONFIG_QCA953X) || defined(RTCONFIG_QCA956X) || defined(RTCONFIG_QCN550X)
	/* set et1macaddr the same as et0macaddr (for cpu connect to switch only use single RGMII) */
	strcpy(macaddr2, macaddr);
#endif

#if 0	// single band
	nvram_set("et0macaddr", macaddr);
	nvram_set("et1macaddr", macaddr);
#else
	//TODO: separate for different chipset solution
	nvram_set("et0macaddr", macaddr);
	nvram_set("et1macaddr", macaddr2);
#endif

#if defined(VZWAC1300) /* bad eeprom fix, temp workaround */
	{
		unsigned char reg_dmn[4];
		FRead(reg_dmn, OFFSET_MAC_ADDR_2G+6, 4);
		if (reg_dmn[0]!=0 || reg_dmn[1]!=0 || reg_dmn[2]!=0 || reg_dmn[3]!=0) {
			_dprintf("******Fix non-zero reg_dmn of 2G!!!\n");
			memset(reg_dmn, 0x00, 4);
			update_qca_eeprom(QCA_2G_EEPROM_SIZE, QCA_2G_EEPROM_CSUM_OFFSET,
			ETH0_MAC_OFFSET & 0xF000, (ETH0_MAC_OFFSET & 0xFFF)+6, reg_dmn, 4);
		}
		FRead(reg_dmn, OFFSET_MAC_ADDR+6, 4);
		if (reg_dmn[0]!=0 || reg_dmn[1]!=0 || reg_dmn[2]!=0 || reg_dmn[3]!=0) {
			_dprintf("******Fix non-zero reg_dmn of 5G!!!\n");
			memset(reg_dmn, 0x00, 4);
			update_qca_eeprom(QCA_5G_EEPROM_SIZE, QCA_5G_EEPROM_CSUM_OFFSET,
			ETH1_MAC_OFFSET & 0xF000, (ETH1_MAC_OFFSET & 0xFFF)+6, reg_dmn, 4);
		}
	}
#endif
	country_code[0] = '\0';
	dst = (unsigned char*) country_code;
	bytes = FACTORY_COUNTRY_CODE_LEN;
	if (FRead(dst, OFFSET_COUNTRY_CODE, bytes)<0)
	{
		_dprintf("READ ASUS country code: Out of scope\n");
		nvram_set("wl_country_code", "DB");
	}
	else
	{
		dst[FACTORY_COUNTRY_CODE_LEN]='\0';
		chk_valid_country_code(country_code);
		nvram_set("wl_country_code", country_code);
		nvram_set("wl0_country_code", country_code);
		nvram_set("wl1_country_code", country_code);
#if defined(RTCONFIG_HAS_5G_2)
		nvram_set("wl2_country_code", country_code);
#endif
#if defined(RTCONFIG_WIGIG)
		nvram_set("wl3_country_code", country_code);
#endif
	}

	/* reserved for Ralink. used as ASUS pin code. */
	dst = (char *)pin;
	bytes = 8;
	if (FRead(dst, OFFSET_PIN_CODE, bytes) < 0) {
		_dprintf("READ ASUS pin code: Out of scope\n");
		nvram_set("wl_pin_code", "");
	} else {
		if ((unsigned char)pin[0] != 0xff)
			nvram_set("secret_code", pin);
		else
			nvram_set("secret_code", "12345670");
	}

#if defined(RTCONFIG_FITFDT)
	nvram_set("firmver", rt_version);
	nvram_set("productid", rt_buildname);
	strncpy(productid, rt_buildname, 12);
#else
	dst = buffer;
	bytes = 16;
	if (linuxRead(dst, 0x20, bytes) < 0) {	/* The "linux" MTD partition, offset 0x20. */
		fprintf(stderr, "READ firmware header: Out of scope\n");
		nvram_set("productid", "unknown");
		nvram_set("firmver", "unknown");
	} else {
		strncpy(productid, buffer + 4, 12);
		productid[12] = 0;
		snprintf(fwver, sizeof(fwver), "%d.%d.%d.%d", buffer[0], buffer[1], buffer[2],
			buffer[3]);
		nvram_set("productid", trim_r(productid));
		nvram_set("firmver", trim_r(fwver));
	}
#endif

#if defined(RTCONFIG_TCODE)
	/* Territory code */
	memset(buffer, 0, sizeof(buffer));
	if (FRead(buffer, OFFSET_TERRITORY_CODE, 5) < 0) {
		_dprintf("READ ASUS territory code: Out of scope\n");
		nvram_unset("territory_code");
	} else {
		/* [A-Z][A-Z]/[0-9][0-9] */
		if (buffer[2] != '/' ||
		    !isupper(buffer[0]) || !isupper(buffer[1]) ||
		    !isdigit(buffer[3]) || !isdigit(buffer[4]))
		{
			nvram_unset("territory_code");
		} else {
#if defined(MAPAC2200) || defined(MAPAC1300)
			if(strcmp(country_code, "CA") == 0 && strcmp(buffer, "US/01") == 0)
				strcpy(buffer, "CA/01");
#endif
			nvram_set("territory_code", buffer);
		}
	}

	/* PSK */
	memset(buffer, 0, sizeof(buffer));
	if (FRead(buffer, OFFSET_PSK, 14) < 0) {
		_dprintf("READ ASUS PSK: Out of scope\n");
		nvram_set("wifi_psk", "");
	} else {
		if ((buffer[0] == 0xff)|| !strcmp(buffer,"NONE"))
			nvram_set("wifi_psk", "");
		else
			nvram_set("wifi_psk", buffer);
	}
#if defined(RTAC58U)
	if (!strncmp(nvram_safe_get("territory_code"), "CX", 2))
		nvram_set("wifi_psk", nvram_safe_get("secret_code"));
#endif
#endif

	memset(buffer, 0, sizeof(buffer));
	FRead(buffer, OFFSET_BOOT_VER, 4);
	snprintf(blver, sizeof(blver), "%s-0%c-0%c-0%c-0%c", trim_r(productid), buffer[0],
		buffer[1], buffer[2], buffer[3]);
	nvram_set("blver", trim_r(blver));

	_dprintf("bootloader version: %s\n", nvram_safe_get("blver"));
	_dprintf("firmware version: %s\n", nvram_safe_get("firmver"));

	nvram_set("wl1_txbf_en", "0");

#ifdef RTCONFIG_ODMPID
#ifdef RTCONFIG_32BYTES_ODMPID
	FRead(modelname, OFFSET_32BYTES_ODMPID, 32);
	modelname[31] = '\0';
	if (modelname[0] != 0 && (unsigned char)(modelname[0]) != 0xff
	    && is_valid_hostname(modelname)
	    && strcmp(modelname, "ASUS")) {
		nvram_set("odmpid", modelname);
	} else
#endif
	{
		FRead(modelname, OFFSET_ODMPID, 16);
		modelname[15] = '\0';
		if (modelname[0] != 0 && (unsigned char)(modelname[0]) != 0xff
		    && is_valid_hostname(modelname)
		    && strcmp(modelname, "ASUS")) {
			nvram_set("odmpid", modelname);
		} else
			nvram_unset("odmpid");
	}
#else
	nvram_unset("odmpid");
#endif

#if defined(MAPAC2200) /* for Lyra */
	nvram_set("odmpid", "Lyra");
#elif defined(MAPAC1300)
	nvram_set("odmpid", "Lyra_Mini");
#elif defined(VZWAC1300)
	nvram_set("odmpid", "ASUSMESH-AC1300");
#endif

	nvram_set("firmver", rt_version);
	nvram_set("productid", rt_buildname);

#if !defined(RTCONFIG_TCODE) || defined(RPAC66)  || defined(RPAC51) // move the verification later bcz TCODE/LOC
	verify_ctl_table();
#endif

#ifdef RTCONFIG_QCA_PLC_UTILS
	getPLC_MAC(macaddr);
	nvram_set("plc_macaddr", macaddr);
#endif

#ifdef RTCONFIG_DEFAULT_AP_MODE
	char dhcp = '0';

	if (FRead(&dhcp, OFFSET_FORCE_DISABLE_DHCP, 1) < 0) {
		_dprintf("READ Disable DHCP: Out of scope\n");
	} else {
		if (dhcp == '1')
			nvram_set("ate_flag", "1");
		else
			nvram_set("ate_flag", "0");
	}
#endif

#ifdef RTCONFIG_AMAS
	char bdl;
	char bdlkey_buf[CFGSYNC_GROUPID_LEN+1];

	if (FRead(&bdl, OFFSET_AMAS_BUNDLE_FLAG, 1) < 0) {
		_dprintf("READ AiMesh bundle flag: Out of scope\n");
	} else {
		if ((bdl > AB_FLAG_NONE) && (bdl < AB_FLAG_MAX))
			nvram_set_int("amas_bdl", bdl);
		else
			nvram_unset("amas_bdl");
	}

	if (FRead(bdlkey_buf, OFFSET_AMAS_BUNDLE_KEY, CFGSYNC_GROUPID_LEN) < 0) {
		_dprintf("READ AiMesh bundle key: Out of scope\n");
	} else {
		if (!is_valid_group_id(bdlkey_buf)) {
			bdlkey_buf[0]='\0';
			nvram_unset("amas_bdlkey");
		}
		else {
			bdlkey_buf[CFGSYNC_GROUPID_LEN]='\0';
			nvram_set("amas_bdlkey", bdlkey_buf);
		}
	}
#endif

#if defined(RTCONFIG_WIFI_DRV_DISABLE) /* for IPQ40XX */
	if (FRead(&disableWifiDrv, OFFSET_DISABLE_WIFI_DRV, 1) < 0) {
		_dprintf("Out of scope\n");
	} else {
		if ((disableWifiDrv == 'Y') || (disableWifiDrv == 'y'))
			nvram_set("disableWifiDrv_fac", "1");
	}
#endif

	FRead(ipaddr_lan, OFFSET_IPADDR_LAN, sizeof(ipaddr_lan));
	ipaddr_lan[sizeof(ipaddr_lan)-1] = '\0';
	if ((unsigned char)(ipaddr_lan[0]) != 0xff && !illegal_ipv4_address(ipaddr_lan))
		nvram_set("IpAddr_Lan", ipaddr_lan);
	else
		nvram_unset("IpAddr_Lan");
}

#ifdef RTCONFIG_ATEUSB3_FORCE
void post_syspara(void)
{
	unsigned char buffer[16];
	buffer[0]='0';
	if (FRead(&buffer[0], OFFSET_FORCE_USB3, 1) < 0) {
		fprintf(stderr, "READ FORCE_USB3 address: Out of scope\n");
	}
	if (buffer[0]=='1')
		nvram_set("usb_usb3", "1");
}
#endif

void generate_wl_para(int unit, int subunit)
{
}

#if defined(RTCONFIG_SOC_QCA9557) || defined(RTCONFIG_QCA953X) || defined(RTCONFIG_QCA956X) || defined(RTCONFIG_QCN550X) || defined(RTCONFIG_SOC_IPQ40XX)
// only qca solution can reload it dynamically
// only happened when qca_sfe=1
// only loaded when unloaded, and unloaded when loaded
// in restart_firewall for fw_pt_l2tp/fw_pt_ipsec
// in restart_qos for qos_enable
// in restart_wireless for wlx_mrate_x, etc
void reinit_sfe(int unit)
{
	int prim_unit = wan_primary_ifunit();
	int act = 1,i;	/* -1/0/otherwise: ignore/remove sfe/load sfe */
	struct load_nat_accel_kmod_seq_s *p = &load_nat_accel_kmod_seq[0];
#if defined(RTCONFIG_DUALWAN)
	int nat_x = -1, l, t, link_wan = 1, link_wans_lan = 1;
	int wans_cap = get_wans_dualwan() & WANSCAP_WAN;
	int wanslan_cap = get_wans_dualwan() & WANSCAP_LAN;
	char nat_x_str[] = "wanX_nat_xXXXXXX";
#endif
#if defined(RTCONFIG_SOC_IPQ40XX) && defined(RTCONFIG_BWDPI)
	int handle_bwdpi = 0;
#endif

#if  defined(RTCONFIG_SOC_IPQ40XX) 
	act = nvram_get_int("qca_sfe");	
#else
	if (!nvram_get_int("qca_sfe"))
		return;
#endif

#if  defined(RTCONFIG_SOC_IPQ40XX) 
	if(nvram_get_int("url_enable_x")==1 && strlen(nvram_get("url_rulelist"))!=0)
		act = 0;

	if(nvram_get_int("keyword_enable_x")==1 && strlen(nvram_get("keyword_rulelist"))!=0)
		act = 0;
#endif

	/* If non-A.QoS is enabled, disable sfe. */
	if (IS_NON_AQOS())
		act = 0;

#if defined(RTCONFIG_QCA956X) && defined(RTCONFIG_BWDPI)
	/* For MAP-AC1750, not to integrate fast-path in stage 1 */
	if (check_bwdpi_nvram_setting() == 1)
		act = 0;
#endif

	/* URL filter and keyword filter are not compatible to IPQ806x NSS NAT acceleration and IPQ40XX shortcut-fe.
	 * But they do works with QCA955X shortcut-fe.
	 */
	if ((nvram_match("url_enable_x", "1") && !nvram_match("url_rulelist", "")) ||
	    (nvram_match("keyword_enable_x", "1") && !nvram_match("keyword_rulelist", "")))
		act = 0;

#if defined(RTAC58U)
	if (nvram_match("switch_wantag", "stuff_fibre"))
		act = 0;
#endif

	if (act > 0) {
#if defined(RTCONFIG_DUALWAN)
		if (unit < 0 || unit > WAN_UNIT_SECOND) {
			if ((wans_cap && wanslan_cap) /* ||
			    (wanslan_cap && (!nvram_match("switch_wantag", "none") && !nvram_match("switch_wantag", ""))) */
			   )
				act = 0;
		} else {
			snprintf(nat_x_str, sizeof(nat_x_str), "wan%d_nat_x", unit);
			nat_x = nvram_get_int(nat_x_str);
			if (unit == prim_unit && !nat_x)
				act = 0;
			else if ((wans_cap && wanslan_cap) /* ||
				 (wanslan_cap && (!nvram_match("switch_wantag", "none") && !nvram_match("switch_wantag", ""))) */
				)
				act = 0;
			else if (unit != prim_unit)
				act = -1;
		}
#else
		if (!is_nat_enabled())
			act = 0;
#endif
	}

#ifdef RTCONFIG_WIFI_SON
	if (nvram_match("wifison_ready", "1") &&
	   ((sw_mode() != SW_MODE_ROUTER) && !nvram_match("cfg_master", "1")))
		act = 0;
#endif
#if !defined(RT4GAC53U) /* for Gobi */
	if (act > 0) {
#if defined(RTCONFIG_DUALWAN)
		if (unit < 0 || unit > WAN_UNIT_SECOND || nvram_match("wans_mode", "lb")) {
			if (get_wans_dualwan() & WANSCAP_USB)
				act = 0;
		} else {
			if (unit == prim_unit && get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
				act = 0;
		}
#else
		if (dualwan_unit__usbif(prim_unit))
			act = 0;
#endif
	}
#endif

#if defined(RTCONFIG_DUALWAN)
	if (act != 0 &&
	    ((wans_cap && wanslan_cap) || (wanslan_cap && (!nvram_match("switch_wantag", "none") && !nvram_match("switch_wantag", ""))))
	   )
	{
		/* If WANS_LAN and WAN is enabled, WANS_LAN is link-up and WAN is not link-up, hw_nat MUST be removed.
		 * If hw_nat exists in such scenario, LAN PC can't connect to Internet through WANS_LAN correctly.
		 *
		 * FIXME:
		 * If generic IPTV feature is enabled, STB port and VoIP port are recognized as WAN port(s).
		 * In such case, we don't know whether real WAN port is link-up/down.
		 * Thus, if WAN is link-up and primary unit is not WAN, assume WAN is link-down.
		 */
		for (i = WAN_UNIT_FIRST; i < WAN_UNIT_MAX; ++i) {
			if ((t = get_dualwan_by_unit(i)) == WANS_DUALWAN_IF_USB)
				continue;

			l = wanport_status(i);
			switch (t) {
			case WANS_DUALWAN_IF_WAN:
				link_wan = l && (i == prim_unit);
				break;
			case WANS_DUALWAN_IF_DSL:
				link_wan = l;
				break;
			case WANS_DUALWAN_IF_LAN:
				link_wans_lan = l;
				break;
			default:
				_dprintf("%s: Unknown WAN type %d\n", __func__, t);
			}
		}

		if (!link_wan && link_wans_lan)
			act = 0;
	}

	_dprintf("%s:DUALWAN: unit %d,%d type %d iptv [%s] nat_x %d qos %d wans_mode %s link %d,%d: action %d.\n",
		__func__, unit, prim_unit, get_dualwan_by_unit(unit), nvram_safe_get("switch_wantag"), nat_x,
		nvram_get_int("qos_enable"), nvram_safe_get("wans_mode"),
		link_wan, link_wans_lan, act);
#else
	_dprintf("%s:WAN: unit %d,%d type %d nat_x %d qos %d: action %d.\n",
		__func__, unit, prim_unit, get_dualwan_by_unit(unit),
		nvram_get_int("wan0_nat_x"), nvram_get_int("qos_enable"), act);
#endif

	if (act < 0)
		return;

#if defined(RTCONFIG_SOC_IPQ40XX) && defined(RTCONFIG_BWDPI)
	if ((act == 0) && (check_bwdpi_nvram_setting() == 1)) {
		handle_bwdpi = 1;
		act = 1;
	}
#endif

	for (i = 0, p = &load_nat_accel_kmod_seq[i]; i < ARRAY_SIZE(load_nat_accel_kmod_seq); ++i, ++p) {
		if (!act) {
			/* remove sfe */
			if (!module_loaded(p->kmod_name))
				continue;

			modprobe_r(p->kmod_name);

			if (p->remove_sleep)
				sleep(p->load_sleep);

		} else {
#if defined(RTCONFIG_SOC_IPQ40XX) && defined(RTCONFIG_BWDPI)
			if (!strcmp(p->kmod_name,"shortcut_fe_cm") && handle_bwdpi) {
					stop_dpi_engine_service(1); /* DPI use SFE symbol, we must remove DPI at first */
					modprobe_r("shortcut_fe_cm");
			}
#endif
			/* load sfe */
			if (module_loaded(p->kmod_name))
			{

#if defined(RTCONFIG_SOC_IPQ40XX)
				if(nvram_get_int("MULTIFILTER_ENABLE")==1 &&
				   !strcmp(p->kmod_name,"shortcut_fe_cm"))
					modprobe_r("shortcut_fe_cm");
				else				
#endif
					continue;
			}
			
#if defined(RTCONFIG_SOC_IPQ40XX) && defined(RTCONFIG_BWDPI)
			if (!strcmp(p->kmod_name,"shortcut_fe_cm") && handle_bwdpi) {
				modprobe(p->kmod_name, "skip_sfe=1");
				start_dpi_engine_service();
			}
			else
#endif
				modprobe(p->kmod_name);
			if (p->load_sleep)
				sleep(p->load_sleep);
		}
	}
}
#endif	/* RTCONFIG_SOC_QCA9557 || RTCONFIG_QCA953X || RTCONFIG_QCA956X || RTCONFIG_QCN550X || RTCONFIG_SOC_IPQ40XX */

#if defined(RTCONFIG_SOC_IPQ8064) || defined(RTCONFIG_SOC_IPQ8074)
#define IPV46_CONN	4096
/**
 * Tell caller whether ecm should be loaded (non-zero value) or unloaded (zero value).
 * @return
 * 	0:	ecm should be unloaded
 *  otherwise:	ecm should be loaded
 */
int ecm_selection(void)
{
	int act = nvram_get_int("qca_sfe");	/* -1/0/otherwise: ignore/remove ecm/load ecm */

	/* Don't load ecm if NAT is not enabled. */
	if (!is_nat_enabled())
		act = 0;

	/* If QoS is enabled, disable ecm.
	 * Including AiProtection due to BWDPI dep. module is compatible to IPQ806x NSS NAT acceleration.
	 * dpi engine doesn't integrate QCA Hardware QoS, so A.QoS needs to "echo 1 >  ecm_nss_ipv[4/6]/stop"
	 */
	if (nvram_get_int("qos_enable") == 1)
		act = 0;

	/* If IPSec is enabled, disable ecm. */
	if (!nvram_match("qca_hwnat_ipsec", "1")) {
		if (nvram_get_int("ipsec_server_enable") == 1 || nvram_get_int("ipsec_client_enable") == 1)
			act = 0;
	}

	/* URL filter and keyword filter are not compatible to IPQ806x NSS NAT acceleration and IPQ40XX shortcut-fe.
	 * But they do works with QCA955X shortcut-fe.
	 */
	if ((nvram_match("url_enable_x", "1") && !nvram_match("url_rulelist", "")) ||
	    (nvram_match("keyword_enable_x", "1") && !nvram_match("keyword_rulelist", "")))
		act = 0;


	if (act > 0) {
		/* FIXME: IPTV, ISP profile, USB modem, etc. */
	}

	dbg("%s: nat_x %d qos %d: action %d.\n", __func__,
		nvram_get_int("wan0_nat_x"), nvram_get_int("qos_enable"), act);

	return act? 1 : 0;
}

void init_ecm(void)
{
	/* Always enable NSS RPS */
	f_write_string("/proc/sys/dev/nss/general/rps", "1", 0, 0);

	/* Turn off bridge firewall first. */
	f_write_string("/proc/sys/net/bridge/bridge-nf-call-ip6tables", "0", 0, 0);
	f_write_string("/proc/sys/net/bridge/bridge-nf-call-iptables", "0", 0, 0);
}

// ecm kernel module must be loaded before bonding interface creation!
// only qca solution can reload it dynamically
// only happened when qca_sfe=1
// only loaded when unloaded, and unloaded when loaded
// in restart_firewall for fw_pt_l2tp/fw_pt_ipsec
// in restart_qos for qos_enable
// in restart_wireless for wlx_mrate_x, etc
void reinit_ecm(int unit)
{
	int i, act, r1, r2;
	char val[4] = "0";
	struct load_nat_accel_kmod_seq_s *p = &load_nat_accel_kmod_seq[0];
#if defined(RTCONFIG_SOC_IPQ8064)
	const char *v4_stop_fn = "/sys/kernel/debug/ecm/ecm_nss_ipv4/stop", *v6_stop_fn = "/sys/kernel/debug/ecm/ecm_nss_ipv6/stop";
#elif defined(RTCONFIG_SOC_IPQ8074)
	const char *v4_stop_fn = "/sys/kernel/debug/ecm/front_end_ipv4_stop", *v6_stop_fn = "/sys/kernel/debug/ecm/front_end_ipv6_stop";
#endif

	act = !!ecm_selection();

	/* Always load ecm and related kernel modules.
	 * If hardware NAT should not enabled, stop ecm instead.
	 */
	for (i = 0, p = &load_nat_accel_kmod_seq[i]; i < ARRAY_SIZE(load_nat_accel_kmod_seq); ++i, ++p) {
		if (module_loaded(p->kmod_name))
			continue;

		dbg("%s: Load %s\n", __func__, p->kmod_name);
		modprobe(p->kmod_name);
		if (p->load_sleep)
			sleep(p->load_sleep);
	}
	snprintf(val, sizeof(val), "%d", !act);


	/* resume/stop ecm. */
	r1 = f_write_string(v4_stop_fn, val, 0, 0);
	r2 = f_write_string(v6_stop_fn, val, 0, 0);

	if (!act) {
		f_write_string("/sys/kernel/debug/ecm/ecm_db/defunct_all", "1", 0, 0);
	}
	if (r1 <= 0 || r2 <= 0) {
		dbg("%s ecm failed. (return %d/%d)\n", act? "Resume" : "Stop", r1, r2);
	}

	post_ecm();
}

void post_ecm(void)
{
	int act, redir, r;
	char val[4], tmp[16], ipv46_conn[16];

	snprintf(ipv46_conn, sizeof(ipv46_conn), "%d", IPV46_CONN);
	*tmp = '\0';
	r = f_read_string("/proc/sys/dev/nss/ipv4cfg/ipv4_conn", tmp, sizeof(tmp));
	if (r > 0 && safe_atoi(tmp) != IPV46_CONN)
		r = f_write_string("/proc/sys/dev/nss/ipv4cfg/ipv4_conn", ipv46_conn, 0, 0);
	*tmp = '\0';
	r = f_read_string("/proc/sys/dev/nss/ipv6cfg/ipv6_conn", tmp, sizeof(tmp));
	if (r > 0 && safe_atoi(tmp) != IPV46_CONN)
		r = f_write_string("/proc/sys/dev/nss/ipv6cfg/ipv6_conn", ipv46_conn, 0, 0);

	redir = act = ecm_selection();
#if defined(RTCONFIG_COOVACHILLI)
	if (nvram_match("captive_portal_enable","on") || nvram_match("captive_portal_adv_enable", "on"))
		redir = 0;
#endif
	snprintf(val, sizeof(val), "%d", !!redir);
	f_write_string("/proc/sys/dev/nss/general/redirect", val, 0, 0);

#if defined(RTCONFIG_SOC_IPQ8064)
	/* Limit ecm db usage */
	if (act) {
		f_write_string("/sys/kernel/debug/ecm/ecm_nss_ipv4/db_limit_mode", "1", 0, 0);
		f_write_string("/sys/kernel/debug/ecm/ecm_nss_ipv6/db_limit_mode", "1", 0, 0);
	}
#endif
}
#endif	/* RTCONFIG_SOC_IPQ8064 || RTCONFIG_SOC_IPQ8074 */

/** Whether wireless interface works.
 * @ifname:
 * @ate_wl_band:	ATE band number.  1:2G, 2:5G-1, 3:5G-2, etc
 * @return:
 * 	0:	Wireless interface absent or not work.
 *  otherwizse:	Wireless interface exist and works.
 */
int wl_exist(char *ifname, int ate_wl_band)
{
	int ret = 0, band = ate_wl_band - 1;
	char phy[sizeof("/sys/class/ieee80211/XXX") + IFNAMSIZ];

	if (!ifname || band < WL_2G_BAND || band >= WL_NR_BANDS) {
		dbg("%s: invalid parameter? (ifname %p, band %d)\n", __func__, ifname, band);
		return 0;
	}

	if (band == WL_60G_BAND) {
		snprintf(phy, sizeof(phy), "/sys/class/ieee80211/%s", get_vphyifname(band));
		if (!f_exists(phy) && !d_exists(phy))
			return 0;
	} else if (!iface_exist(get_vphyifname(band))) {
		dbg("%s: %s not found!\n", __func__, get_vphyifname(band));
		return 0;
	}

	if (!iface_exist(get_wififname(band))) {
		dbg("%s: %s not found!\n", __func__, get_wififname(band));
		return 0;
	}

	if (!ifname || band < WL_2G_BAND || band >= WL_NR_BANDS) {
		dbg("%s: invalid parameter? (ifname %p, band %d)\n", __func__, ifname, band);
		return 0;
	}

	/* "band" ranges from 1~3 (ate.c), but WL_2G_BAND is 0 */
	switch (band) {
	case WL_2G_BAND:	/* fall-through */
	case WL_5G_BAND:	/* fall-through */
	case WL_5G_2_BAND:
		ret = eval(IWPRIV, ifname, "get_driver_caps");
		dbg("eval(%s, %s, get_driver_caps) ret(%d)\n", IWPRIV, ifname, ret);
		break;
	case WL_60G_BAND:
		ret = eval("iw", ifname, "info");
		dbg("eval(%s, %s, info) ret(%d)\n", "iw", ifname, ret);
		break;
	};

	return (ret == 0);
}

void
set_wan_tag(char *interface)
{
#if defined(RTCONFIG_MULTICAST_IPTV)
	const int sw_iptv = sw_based_iptv();
#endif
	int model, wan_vid;
	char wan_dev[10], port_id[7];

	model = get_model();
	wan_vid = nvram_get_int("switch_wan0tagid");

	snprintf(wan_dev, sizeof(wan_dev), "vlan%d", wan_vid);

	switch(model) {
	case MODEL_BRTAC828:
	case MODEL_RTAD7200:
	case MODEL_RTAC55U:
	case MODEL_RTAC55UHP:
	case MODEL_RT4GAC55U:
	case MODEL_RTN19:
	case MODEL_RTAC59U:
	case MODEL_RTAC58U:
	case MODEL_RT4GAC53U:
	case MODEL_RTAC82U:
	case MODEL_RTAC88N:
	case MODEL_MAPAC1750:
	case MODEL_RTAC95U:
		ifconfig(interface, IFUP, 0, 0);
		if(wan_vid) { /* config wan port */
			eval("vconfig", "rem", "vlan2");
			snprintf(port_id, sizeof(port_id), "%d", wan_vid);
			eval("vconfig", "add", interface, port_id);
		}
		/* Set Wan port PRIO */
		if(nvram_invmatch("switch_wan0prio", "0"))
			eval("vconfig", "set_egress_map", wan_dev, "0", nvram_get("switch_wan0prio"));
#if defined(RTAC58U)
		if (nvram_match("switch_wantag", "stuff_fibre"))
			eval("vconfig", "set_egress_map", wan_dev, "3", "5");
#endif
		break;
	case MODEL_MAPAC1300:
	case MODEL_VZWAC1300:
	case MODEL_MAPAC2200:
#if defined(RTCONFIG_SOC_IPQ40XX)
		if (!strcmp(nvram_safe_get("switch_wantag"), "movistar")) {
			ifconfig(interface, IFUP, 0, 0);
			snprintf(port_id, sizeof(port_id), "%d", wan_vid);
			eval("vconfig", "add", interface, port_id);
			detwan_set_def_vid(interface, 10, 0, 0); // sync with Barton's modification
		} else {
			detwan_set_def_vid(interface, wan_vid, 1, 0);
		}
#endif	/* RTCONFIG_SOC_IPQ40XX */
		break;
	}

#ifdef RTCONFIG_MULTICAST_IPTV
	{
		int iptv_vid, voip_vid, iptv_prio, voip_prio, switch_stb;
		int mang_vid, mang_prio;

		iptv_vid  = nvram_get_int("switch_wan1tagid") & 0x0fff;
		voip_vid  = nvram_get_int("switch_wan2tagid") & 0x0fff;
		iptv_prio = nvram_get_int("switch_wan1prio") & 0x7;
		voip_prio = nvram_get_int("switch_wan2prio") & 0x7;
		mang_vid  = nvram_get_int("switch_wan3tagid") & 0x0fff;
		mang_prio = nvram_get_int("switch_wan3prio") & 0x7;

		switch_stb = nvram_get_int("switch_stb_x");
		if (switch_stb >= 7) {
			system("rtkswitch 40 1");			/* admin all frames on all ports */
#if defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2) || \
    defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2)
			/* Make sure the "admin all frames on all ports" is applied to Realtek switch. */
			system("rtkswitch 38 0");
#endif
			if(wan_vid) { /* config wan port */
				__setup_vlan(wan_vid, 0, 0x00000210);	/* config WAN & WAN_MAC port */
			}

			if (iptv_vid) { /* config IPTV on wan port */
				snprintf(wan_dev, sizeof(wan_dev), "vlan%d", iptv_vid);
				nvram_set("wan10_ifname", wan_dev);
				if (sw_iptv) {
					__setup_vlan(iptv_vid, iptv_prio, 0x00000210);	/* config WAN & WAN_MAC port */
				} else {
					snprintf(port_id, sizeof(port_id), "%d", iptv_vid);
					eval("vconfig", "add", interface, port_id);

					__setup_vlan(iptv_vid, iptv_prio, 0x00000210);	/* config WAN & WAN_MAC port */

					if (iptv_prio) { /* config priority */
						eval("vconfig", "set_egress_map", wan_dev, "0", (char *)iptv_prio);
					}
				}
			}
		}
		if (switch_stb >= 8) {
			if (voip_vid) { /* config voip on wan port */
				snprintf(wan_dev, sizeof(wan_dev), "vlan%d", voip_vid);
				nvram_set("wan11_ifname", wan_dev);
				if (sw_iptv) {
					__setup_vlan(voip_vid, voip_prio, 0x00000210);	/* config WAN & WAN_MAC port */
				} else {
					snprintf(port_id, sizeof(port_id), "%d", voip_vid);
					eval("vconfig", "add", interface, port_id);

					__setup_vlan(voip_vid, voip_prio, 0x00000210);	/* config WAN & WAN_MAC port */

					if (voip_prio) { /* config priority */
						eval("vconfig", "set_egress_map", wan_dev, "0", (char *)voip_prio);
					}
				}
			}
		}
		if (switch_stb >=9 ) {
			if (mang_vid) { /* config tr069 on wan port */
				snprintf(wan_dev, sizeof(wan_dev), "vlan%d", mang_vid);
				nvram_set("wan12_ifname", wan_dev);
				if (sw_iptv) {
					__setup_vlan(mang_vid, mang_prio, 0x00000210);	/* config WAN & WAN_MAC port */
				} else {
					snprintf(port_id, sizeof(port_id), "%d", mang_vid);
					eval("vconfig", "add", interface, port_id);

					__setup_vlan(mang_vid, mang_prio, 0x00000210);	/* config WAN & WAN_MAC port */

					if (mang_prio) { /* config priority */
						eval("vconfig", "set_egress_map", wan_dev, "0", (char *)iptv_prio);
					}
				}
			}
		}
#if defined(RTCONFIG_DETWAN)
		if (nvram_get("switch_wantag") && nvram_match("switch_wantag", "movistar")) {
			char buf[128];
			sprintf(buf, "vlan%s", nvram_safe_get("switch_wan0tagid"));
			nvram_set("wan_ifnames", buf);
			nvram_set("wan0_ifname", buf);
		}
#endif
	}
#endif
}

int start_thermald(void)
{
	char *thermald_argv[] = {"thermald",
#if defined(RTCONFIG_SOC_IPQ8064)
		"-c", "/etc/thermal/ipq-thermald-8064.conf",
#endif
		NULL};
	pid_t pid;

	return _eval(thermald_argv, NULL, 0, &pid);
}

#ifdef RTCONFIG_TAGGED_BASED_VLAN

/**/
void vlan_switch_accept_tagged(unsigned int port)
{
	char lanportset_str[16]={0};

	snprintf(lanportset_str, sizeof(lanportset_str), "%d", port);
	eval("rtkswitch","298",lanportset_str);
}
void vlan_switch_accept_untagged(unsigned int port)
{
	char lanportset_str[16]={0};

	snprintf(lanportset_str, sizeof(lanportset_str), "%d", port);
	eval("rtkswitch","299",lanportset_str);
}

/* set all ports accept all packets(tagged or untagged) */
void vlan_switch_accept_all(unsigned int port)
{
	char lanportset_str[16]={0};

	snprintf(lanportset_str, sizeof(lanportset_str), "%d", port);
	eval("rtkswitch","300",lanportset_str);
}

void vlan_switch_setup(int vlan_id, int vlan_prio, int lanportset)
{
	char vlan_id_str[12]={0},vlan_prio_str[12]={0};
	char lanportset_str[16]={0};

	snprintf(vlan_id_str, sizeof(vlan_id_str), "%d", vlan_id);
	snprintf(vlan_prio_str, sizeof(vlan_prio_str), "%d", vlan_prio);
	//lanportset |= ( 1<<15 );
	snprintf(lanportset_str, sizeof(lanportset_str), "0x%x", lanportset);

	eval("rtkswitch","301",vlan_id_str);
	eval("rtkswitch","302",vlan_prio_str);
	eval("rtkswitch","303",lanportset_str);
}

int vlan_switch_pvid_setup(int *pvid_list, int *pprio_list, int size)
{
	int i=0;
	char port_str[8]={0};
	char pvid_str[8]={0};
	char pprio_str[8]={0};

	if(!pvid_list && !pprio_list)
		return -1;

	for(i=0;i<size;i++)
	{
		memset(port_str,0,8);
		memset(pvid_str,0,8);
		memset(pprio_str,0,8);
		snprintf(port_str, sizeof(port_str), "%d", i);
		snprintf(pvid_str, sizeof(pvid_str), "%d", pvid_list[i]);
		snprintf(pprio_str, sizeof(pprio_str), "%d", pprio_list[i]);

		if(pvid_list[i] == 0 || pvid_list[i] == 1)
			continue;

		eval("rtkswitch","305",pvid_str);
		eval("rtkswitch","306",pprio_str);
		eval("rtkswitch","307",port_str);
	}

	return 0;
}
#if 0
void set_tagged_based_vlan_config(char *interface)
{
	char *nv, *nvp, *b;
	//char *enable, *vid, *priority, *portset, *wlmap, *subnet_name;
	//char *portset, *wlmap, *subnet_name;
	char *enable, *wanportset, *lanportset, *wl2gset, *wl5gset, *subnet_name, *vlan_name;
	int set_flag = (interface != NULL) ? 1 : 0;

	/* Clean some parameters for vlan */
	//clean_vlan_ifnames();

	printf("%s %d\n",__FUNCTION__,__LINE__);

	if (vlan_enable()) {
		nv = nvp = strdup(nvram_safe_get("vlan_rulelist"));

		if (nv) {
			//int vlan_tag = 4;
			int model;
			int br_index = 3;

			model = get_model();

			if (model != MODEL_BRTAC828){
				printf("model != MODEL_BRTAC828\n");
				return;
			}

			while ((b = strsep(&nvp, "<")) != NULL) {
				//int real_portset = 0;
				char tag_reg_val[7]={0}, vlan_id[5]={0},vlan_prio[2]={0}, lanportset_str[12]={0};
				//unsigned int vlan_entry_tmp = 0, tag_reg_val_tmp = 0;
				unsigned int wanportset_tmp=0,lanportset_tmp=0,wl2gset_tmp=0,wl5gset_tmp=0,wlset_tmp=0;
				int i = 0, vlan_id_tmp = 0,vlan_prio_tmp=0;
				int cpu_port = 0;
				

				if ((vstrsep(b, ">", &enable, &wanportset, &lanportset, &wl2gset, &wl5gset, &subnet_name, &vlan_name) != 7))
					continue;

				//_dprintf("%s: %s %s %s %s %s %s\n", __FUNCTION__, enable, vid, priority, portset, wlmap, subnet_name);
				printf("%s: %s %s %s %s %s %s %s\n", __FUNCTION__, enable, wanportset, lanportset
											, wl2gset, wl5gset, subnet_name, vlan_name);
				_dprintf("%s: %s %s %s %s %s %s %s\n", __FUNCTION__, enable, wanportset, lanportset
											, wl2gset, wl5gset, subnet_name, vlan_name);

				if (!strcmp(enable, "0") || strlen(enable) == 0)
					continue;
				if (!strcmp(subnet_name, "0") || strlen(subnet_name) == 0)
					continue;
				if (!strcmp(vlan_name, "0") || strlen(vlan_name) == 0)
					continue;
				wanportset_tmp = (unsigned int) strtol(wanportset,NULL,16);
				if( wanportset_tmp != 0 )
				{
					/* total:8 bits, bit0: WAN1 */
				}
				lanportset_tmp = (unsigned int) strtol(lanportset,NULL,16);
				if( (lanportset_tmp != 0) && (interface !=NULL) )
				{
					/* total:16 bits, bit0: LAN1, ..., bit15: reserve for the CPU port(user can not set) */
					/* 	rtkswitch 301 14
						rtkswitch 302 0
						rtkswitch 303 0x000000FF */

					lanportset_tmp |= ( 1<<15 );
					snprintf(lanportset_str, sizeof(lanportset_str), "0x%x", lanportset_tmp);

					get_vlan_info(vlan_name,vlan_id,vlan_prio);
					vlan_id_tmp = safe_atoi(vlan_id);
					vlan_prio_tmp = safe_atoi(vlan_prio);
					if( vlan_id_tmp < 1 || vlan_id_tmp > 4095 || vlan_prio_tmp < 0 || vlan_prio_tmp > 7 )
					{
						printf("VLAN vaule error vlan %d, prio %d\n",vlan_id_tmp,vlan_prio_tmp);
						continue;
					}
					eval("rtkswitch","301",vlan_id);
					eval("rtkswitch","302",vlan_prio);
					eval("rtkswitch","303",lanportset_str);

					printf("vconfig add %s %s\n", interface, vlan_id);
					eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
					eval("vconfig", "add", interface, vlan_id);

				}

				wlset_tmp = (unsigned int) strtol(wl2gset,NULL,16) | 
							( (unsigned int) strtol(wl5gset,NULL,16) << 16 );

				printf("wlset_tmp %x\n",wlset_tmp);

				set_vlan_ifnames(br_index, wlset_tmp, subnet_name, vlan_name);
				br_index ++;
			}

			free(nv);
		}
	}
	return;
}
#endif

#endif
