#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>
#include <syslog.h>
#include "shared.h"

#define	NVSW_ERR_STRING_MAX_LEN	50
#define	JFFS_SYS		"/jffs/.sys"
#define	NVSW_DIR		"NVSW"
#define	NVSW_PATH		JFFS_SYS"/"NVSW_DIR
#define	NVSW_CFGFILE		NVSW_PATH"/nvsw_cfg"
#define	NVSW_MAX_SNAPCOUNT	1	/* 99 means no nvswid got */
#define	NVSW_SNAP_PREFIX	"nvsw_"

#if defined(PRTAX57_GO) || defined(RTBE58_GO)
#define	BTNSW_DISABLED		0
#define	BTNSW_NV_SWITCH		1
#define	BTNSW_LED_ONOFF		2
#define	BTNSW_WIFI_ONOFF	3
#define	BTNSW_VPN_ONOFF		4
#endif

static int nvsw_switching_val;	// this value is valid during init time

typedef struct {
	unsigned char nvsw_target_idx;	// 0: original BK, 1-NVSW_MAX_SNAPCOUNT: 1-N snapshot
					// for PRTAX57_GO, if BTNSW_NV_SWITCH is enabled, this value should be > 0
	unsigned char last_status;	// 0: OK, 1: file missing, 2: nvswid err, 3: system err
#if defined(PRTAX57_GO) || defined(RTBE58_GO)
	unsigned char btnsw_meaning;    // BTNSW_DISABLED, BTNSW_NV_SWITCH, BTNSW_LED_ONOFF...
#endif
	char debug_message[NVSW_ERR_STRING_MAX_LEN+1]; // short string message
} __attribute__ ((__packed__)) nvswcfg, *nvswcfg_pt;


#define	NVSW_STASH_COMMENT_LEN	30
#define	NVSW_STASH_BINARY_LEN	30
typedef union {
	struct {
		unsigned char sw_mode;    // nvram sw_mode
		unsigned char aimesh_re;  // nvram re_mode
		unsigned char wisp_mode;  // return value of wisp_mode()
		unsigned char rp_mode;    // return value of repeater_mode()
		unsigned char mb_mode;    // return value of mediabridge_mode()
	} __attribute__ ((__packed__)) args;
	unsigned char bytes[NVSW_STASH_BINARY_LEN];
} cro_args, *cro_args_pt;

typedef struct {
	unsigned char comment[NVSW_STASH_COMMENT_LEN];
	cro_args bin;
} __attribute__ ((__packed__)) nvsw_stash, *nvsw_stash_pt;

#if defined(PRTAX57_GO)
static int go_need_nv_switch_byBTNSW(unsigned char target_idx, unsigned char cur_idx, unsigned char *chgto)
/* input parameter :
 *    target_idx should be greather than 0
 *    cur_idx is current nvswid value, 99 make this function always return 1 (need to change)
 *    chagto will be set to 0 or target_idx according to BTNSW
 * output :
 *    switch btn booton value 0(BTN OFF) && current_idx == 0 , return true(1)
 *    switch btn booton value 1(BTN ON) && current_idx > 0 , return true(1)
 *     Note. new definition, BTN ON -> home/profile 0, BTN OFF -> travel/profile N
 * otherwise, return false(0) */
{
	char sw_btn;
	if (f_read("/proc/device-tree/sw_btn", &sw_btn, 1) == 1) {
		if (sw_btn == '0' && cur_idx != target_idx) {	// SWITCH OFF && not snapshot now
			*chgto = target_idx;			// change to snapshot index
			return 1;
		}
		else if(sw_btn == '1' && cur_idx > 0) {		// SIWTCH ON & is snapshot now
			*chgto = 0;				// change back to original
			return 1;
		}
		else
			return 0;

	} else {
		_dprintf("[NVSW]: read sw_btn fail!!!\n");
		return 0;
	}
}
#elif defined(RTBE58_GO)
#define PRE_NVSW_EXIST(filename) (access((filename), F_OK) == 0)
#define GET_PRE_ID ({\
	int i, prev = 0; char pre[64];\
	for (i = 0; i <= NVSW_MAX_SNAPCOUNT; i++){\
		snprintf(pre, sizeof(pre), "%s/%d", NVSW_PATH, i);\
		if(PRE_NVSW_EXIST(pre)) {prev = i; break;}\
		else prev = 0;}\
	prev;\
})
static int go_need_nv_switch_byBTNSW(unsigned char target_idx, unsigned char cur_idx, unsigned char *chgto)
{
	char sw_btn;
	sw_btn = button_pressed(BTN_SWITCH)? '0' : '1';
	if (cur_idx == 99) {
			*chgto = cur_idx;
			return 1;
	}
	if (sw_btn == '0' && cur_idx != target_idx) {	// SWITCH OFF && not snapshot now
			*chgto = target_idx;			// change to snapshot index
			return 1;
		}
		else if(sw_btn == '1' && cur_idx > 0) {		// SIWTCH ON & is snapshot now
			*chgto = 0;				// change back to original
			return 1;
		}
		else
			return 0;
}

void nvsw_write_current_state(int switch_current_state)
{
	char state[64];
	snprintf(state, sizeof(state), "%s/%d", NVSW_PATH, switch_current_state);
	eval("rm", NVSW_PATH"/0", NVSW_PATH"/1");
	eval("touch", state);
	return;
}
#endif

static unsigned int nvsw_mtd_partsize(char *nvram_mtd_name)
{
	int mtd_fd;
	mtd_info_t mtd_info;

	if ((mtd_fd = open(nvram_mtd_name, O_RDWR|O_SYNC)) < 0)
		return 0;
	if (ioctl(mtd_fd, MEMGETINFO, &mtd_info) != 0)
		return 0;
	close(mtd_fd);
	return mtd_info.size;
}

static void fill_cross_args(cro_args_pt pt)
{
	pt->args.sw_mode = (unsigned char)nvram_get_int("sw_mode");
	pt->args.aimesh_re = (unsigned char)nvram_get_int("re_mode");
	pt->args.wisp_mode = (unsigned char)wisp_mode();
	pt->args.rp_mode = (unsigned char)repeater_mode();
	pt->args.mb_mode = (unsigned char)mediabridge_mode();
}

/* NOTE: you need to free the return pointer if success */
static nvsw_stash_pt nvsw_get_stash(unsigned char idx)
{
	char snapshot_name[PATH_MAX];
	unsigned long fsize;
	nvsw_stash_pt spt;
	spt = (nvsw_stash_pt) malloc(sizeof(nvsw_stash));
	if (!spt) // system error...
		return NULL;
	sprintf(snapshot_name, "%s/%s%d.sth", NVSW_PATH, NVSW_SNAP_PREFIX, idx);
	fsize = f_size(snapshot_name);
	if (fsize == sizeof(nvsw_stash) && f_read(snapshot_name, spt, fsize) == fsize) {
		return spt;
	} else {
		free(spt);
		return NULL;
	}
}

static int nvsw_update_stash(unsigned char idx, char *comment, cro_args_pt pt)
{
	char snapshot_name[PATH_MAX];
	unsigned long fsize;
	nvsw_stash sth;
	sprintf(snapshot_name, "%s/%s%d.sth", NVSW_PATH, NVSW_SNAP_PREFIX, idx);
	fsize = f_size(snapshot_name);
	if (fsize == sizeof(nvsw_stash) && f_read(snapshot_name, &sth, fsize) == fsize) {
		;
	} else {
		memset(&sth, 0x00, sizeof(nvsw_stash));
	}
	if (comment) {
		strncpy(sth.comment, comment, NVSW_STASH_COMMENT_LEN-1);
		sth.comment[NVSW_STASH_COMMENT_LEN-1] = '\0';
	} else {
#if !defined(PRTAX57_GO) || !defined(RTBE58_GO)// GO keep original comment
		sth.comment[0] = '\0';
#endif
	}
	if (pt) {
		memcpy(sth.bin.bytes, pt->bytes, sizeof(pt->args));
	} else {
		memset(sth.bin.bytes, 0x00, sizeof(sth.bin.args));
	}
	if (f_write(snapshot_name, &sth, sizeof(nvsw_stash), 0, 0) != sizeof(nvsw_stash))
		return -1;
	sync();
	return 0;
}

/*
static int nvsw_update_comment(unsigned char idx, char *comment)
{
	if (comment) {
		char snapshot_name[PATH_MAX];
		unsigned long fsize;
		nvsw_stash sth;
		sprintf(snapshot_name, "%s/%s%d.sth", NVSW_PATH, NVSW_SNAP_PREFIX, idx);
		fsize = f_size(snapshot_name);
		if (fsize == sizeof(nvsw_stash) && f_read(snapshot_name, &sth, fsize) == fsize) {
			strncpy(sth.comment, comment, NVSW_STASH_COMMENT_LEN-1);
			sth.comment[NVSW_STASH_COMMENT_LEN-1] = '\0';
		} else {
			return -2;
		}
		if (f_write(snapshot_name, &sth, sizeof(nvsw_stash), 0, 0) != sizeof(nvsw_stash))
			return -3;
		return 0;
	}
	return -1;
}
*/

static int nvsw_update_args(unsigned char idx, cro_args_pt pt)
{
	char snapshot_name[PATH_MAX];
	unsigned long fsize;
	nvsw_stash sth;
	sprintf(snapshot_name, "%s/%s%d.sth", NVSW_PATH, NVSW_SNAP_PREFIX, idx);
	fsize = f_size(snapshot_name);
	if (fsize == sizeof(nvsw_stash) && f_read(snapshot_name, &sth, fsize) == fsize) {
		cro_args_pt pt2 = &sth.bin;
		if (memcmp(pt->bytes, pt2->bytes, sizeof(pt->args)) != 0) {
			_dprintf("\n[NVSW]: p[%u] args from %02x:%02x:%02x:%02x:%02x to %02x:%02x:%02x:%02x:%02x\n", idx, \
					pt2->bytes[0], pt2->bytes[1], pt2->bytes[2], pt2->bytes[3], pt2->bytes[4], \
					pt->bytes[0], pt->bytes[1], pt->bytes[2], pt->bytes[3], pt->bytes[4]);
			memcpy(pt2->bytes, pt->bytes, sizeof(pt->args));
			if (f_write(snapshot_name, &sth, sizeof(nvsw_stash), 0, 0) != sizeof(nvsw_stash))
				return -3;
		}
	} else {
		return -2;
	}
	return 0;
}

static int nvsw_init(nvswcfg_pt cfgpt, unsigned char *nvswid_pt, char *nvram_mtd_name, int restore_default)
{
	unsigned long fsize;
	char msg[NVSW_ERR_STRING_MAX_LEN+1] = {0};
	unsigned char status = 0;
	unsigned char changed_to_idx;
	cro_args cross_val;

	fsize = f_size(NVSW_CFGFILE);
	if (restore_default)
		*nvswid_pt = 99;
	else {
#if !defined(RTCONFIG_HND_ROUTER_BE_4916)
		*nvswid_pt = (unsigned char)nvram_get_int("nvswid");
#elif defined(RTBE58_GO)
		*nvswid_pt = (unsigned char)GET_PRE_ID;
#endif
	}
	// check cfg file status
	if (fsize == sizeof(nvswcfg) && f_read(NVSW_CFGFILE, cfgpt, fsize) == fsize) {
#if defined(PRTAX57_GO) || defined(RTBE58_GO)
		int need_change = 0;
		if (cfgpt->btnsw_meaning == BTNSW_NV_SWITCH && cfgpt->nvsw_target_idx > 0 &&
				go_need_nv_switch_byBTNSW(cfgpt->nvsw_target_idx, *nvswid_pt, &changed_to_idx)) {
			need_change = 1;
		} else if (cfgpt->btnsw_meaning != BTNSW_NV_SWITCH && *nvswid_pt != 0) {
			need_change = 1;
			changed_to_idx = 0; // set to original bakup
		}
		if (need_change)
#else
		changed_to_idx = cfgpt->nvsw_target_idx; // always set to target_idx, target_idx used as GO's switch value
		if (cfgpt->nvsw_target_idx != *nvswid_pt && *nvswid_pt !=99 )
#endif
		{
			// sanity check
			char snapshot_name[PATH_MAX];
			unsigned int nvram_mtd_size;
			if (cfgpt->nvsw_target_idx > NVSW_MAX_SNAPCOUNT) {
				snprintf(msg, NVSW_ERR_STRING_MAX_LEN, "id err, target:[%d],nvsw:[%d]!", cfgpt->nvsw_target_idx, *nvswid_pt);
				status = 2;
				goto bad;
			}
#if !defined(RTCONFIG_HND_ROUTER_BE_4916)
			nvram_mtd_size = nvsw_mtd_partsize(nvram_mtd_name);
			if (nvram_mtd_size == 0) {
				snprintf(msg, NVSW_ERR_STRING_MAX_LEN, "err open nvram mtd[%s]", nvram_mtd_name);
				status = 3;
				goto bad;
			}

			sprintf(snapshot_name, "%s/%s%d.img", NVSW_PATH, NVSW_SNAP_PREFIX, changed_to_idx);
			fsize = f_size(snapshot_name);
			if (fsize != nvram_mtd_size) {
				snprintf(msg, NVSW_ERR_STRING_MAX_LEN, "err snapshot[%d] size[%lu][%u]", changed_to_idx, fsize, nvram_mtd_size);
				status = 1;
				goto bad;
			}
#endif
		}
		return 0;
	}
bad:
	// destroy all inavlid & create an empty cfg
	if (!check_if_dir_exist(JFFS_SYS))
		system("mkdir -p "JFFS_SYS);
	_dprintf("NVSW: init empty cfg...\n");
	system("rm -rf "NVSW_PATH);
	system("mkdir "NVSW_PATH);
	memset(cfgpt, 0x00, sizeof(nvswcfg));
	cfgpt->last_status = status;
	strncpy(cfgpt->debug_message, msg, NVSW_ERR_STRING_MAX_LEN);
	f_write(NVSW_CFGFILE, cfgpt, sizeof(nvswcfg), 0, 0);
	nvram_set_int("nvswid", 0);
#if defined(RTBE58_GO)
	nvsw_write_current_state(0);
#endif
	*nvswid_pt = 0;
	/// clear cro_args
	memset(&cross_val, 0x00, sizeof(cro_args));
	nvsw_update_stash(0, "Home", &cross_val);
	return -1;
}

#define	NVRAM_MTD_TMP_FILE	"/tmp/nvram_mtd"
static int got_nvram_mtd(char *buf, int max_len)
{
	int len;
	char *rpt;

	if (max_len < 11) return -1;
	system("cat /proc/mtd  | grep nvram | awk '{print $1}' | sed s/://g > "NVRAM_MTD_TMP_FILE);
	len = sprintf(buf, "/dev/");
	rpt = &buf[len];
	len = f_read(NVRAM_MTD_TMP_FILE, rpt, max_len-len);
	unlink(NVRAM_MTD_TMP_FILE);
	if (len == 0) {
		return -1;
	}
	rpt[len] = '\0';
	while (len && rpt[len-1] == '\n') {
		len--;
		rpt[len] = '\0';
	}
	return 0;
}

#define	MISC_ARCHIVE_JFFS_ITEMS	"openvpn .cert .le"
static void archive_op_misc(unsigned char idx, int create)
{
	if (create)
		doSystem("tar cf %s/%s%d.tar -C /jffs %s", NVSW_PATH, NVSW_SNAP_PREFIX, idx, MISC_ARCHIVE_JFFS_ITEMS);
	else {
		doSystem("cd /jffs && rm -rf %s", MISC_ARCHIVE_JFFS_ITEMS);
		doSystem("tar xf %s/%s%d.tar -C /jffs", NVSW_PATH, NVSW_SNAP_PREFIX, idx);
	}
}

// only for MT798X now
int reload_nvram(void)
{
	int ret;
	int nvram_fd;

	if ((nvram_fd = open("/dev/nvram", O_RDWR)) < 0)
		return -1;

	ret = ioctl(nvram_fd, 0x0003, 0);     // get the real nvram space size
	close(nvram_fd);
	if (ret) {
		_dprintf("[NVSW]:ioctl nvram reload fail!!\n");
		return -1;
	}
	return 0;
}

/* new version skip reset btn reset to destroy NVSW
 * so add a way for ATE Set_RestoreDefault to detroy NVSW */
void nvsw_destroy(void)
{
	system("rm -rf "NVSW_PATH);
}

/* new version skip reset btn reset to destroy NVSW
 * so add a way to move NVSW between /tmp & /jffs */
void nvsw_move(int toram)
{
	if (toram)
		doSystem("mv %s /tmp", NVSW_PATH);
	else {
		if (!check_if_dir_exist(JFFS_SYS))
			system("mkdir -p "JFFS_SYS);
		doSystem("mv /tmp/%s %s", NVSW_DIR, JFFS_SYS);
	}
}

int nvsw_switching(int *outval, int restore_default)
{
	nvswcfg cfg;
	unsigned char cur_nvswid;
	char nvram_mtd_name[40];
	unsigned char changed_to_idx;
	int need_change = 0;
	cro_args cross_val;

#if !defined(RTCONFIG_HND_ROUTER_BE_4916)
	if (got_nvram_mtd(nvram_mtd_name, sizeof(nvram_mtd_name)) != 0) {
		*outval = 0xffff;
		nvsw_switching_val = *outval;
		return -2;
	}
	// _dprintf("[NVSW]: nvram_mtd is %s\n", nvram_mtd_name);
#else
	nvram_mtd_name[0] = '\0';
#endif

	if (nvsw_init(&cfg, &cur_nvswid, nvram_mtd_name, restore_default) != 0) {
		_dprintf("[NVSW] init:%d (%s)\n", cfg.last_status, cfg.debug_message);
		*outval = 0;
		nvsw_switching_val = *outval;
#if defined(RTBE58_GO)
		nvsw_write_current_state(0);
#endif
		return -1;
	}

	_dprintf("[NVSW]: btnsw_val:%u, target_id:%u\n", cfg.btnsw_meaning, cfg.nvsw_target_idx);
#if defined(PRTAX57_GO) || defined(RTBE58_GO)
	if (cfg.btnsw_meaning == BTNSW_NV_SWITCH && cfg.nvsw_target_idx > 0) {
		if (go_need_nv_switch_byBTNSW(cfg.nvsw_target_idx, cur_nvswid, &changed_to_idx) && cur_nvswid != changed_to_idx)
			need_change = 1;
	}
	else if (cur_nvswid != 0) { // change back to original
		need_change = 1;
		changed_to_idx = 0;
	}
#else
	if (cfg.nvsw_target_idx != cur_nvswid) {
		need_change = 1;
		changed_to_idx = cfg.nvsw_target_idx; // for NON-GO case
	}
#endif
	_dprintf("[NVSW]: cur_nvswid:%d, %s:%u\n", cur_nvswid, need_change?"change-to":"keep", need_change?changed_to_idx:cur_nvswid);

	if (need_change) {
#if defined(RTBE58_GO)
		unsigned char chg_nvswid;
		char nvram_file_name[64];
		// backup current nvram
		if (!restore_default) {
			doSystem("nvram save %s/%s%d.CFG", NVSW_PATH, NVSW_SNAP_PREFIX, cur_nvswid);
		}
		// restore nvram
		snprintf(nvram_file_name, sizeof(nvram_file_name), "%s/%s%d.CFG", NVSW_PATH, NVSW_SNAP_PREFIX, changed_to_idx);
		if (access(nvram_file_name, F_OK) == 0) {
			doSystem("nvram restore %s/%s%d.CFG", NVSW_PATH, NVSW_SNAP_PREFIX, changed_to_idx);
		}
		else {
			_dprintf("[NVSW]: %s/%s%d.CFG not exist, restore default to create new profile\n", NVSW_PATH, NVSW_SNAP_PREFIX, changed_to_idx);
			nvram_set("restore_defaults", "1");
		}

		if (restore_default) { // come frome reset default
			nvram_set_int("nvswid", changed_to_idx);
			nvsw_write_current_state(changed_to_idx);
			// clear cro_args
			memset(&cross_val, 0x00, sizeof(cro_args));
			nvsw_update_args(changed_to_idx, &cross_val);
		}
		else {
			chg_nvswid = (unsigned char)GET_PRE_ID;
			if (chg_nvswid != changed_to_idx) { // come frome upload config?
				_dprintf("[NVSW]: nvswid different in image, target:[%u], nv:[%u]!\n", changed_to_idx, chg_nvswid);
				nvram_set_int("nvswid", changed_to_idx); // overwrite
				nvsw_write_current_state(changed_to_idx);
			}
		}
		// update other nvram if needed
		fill_cross_args(&cross_val);
		nvsw_update_args(changed_to_idx, &cross_val);
		*outval = cur_nvswid;
		*outval = (*outval << 8) | changed_to_idx;
		nvsw_switching_val = *outval;
	} else {
		//// update other nvram if needed
		fill_cross_args(&cross_val);
		if (nvsw_update_args(cur_nvswid, &cross_val)) {
			if (cur_nvswid == 0 && cfg.btnsw_meaning != 1) // old firmware, update profile 0
				nvsw_update_stash(0, "Home", &cross_val);
		}
	}
#elif
		unsigned char chg_nvswid;
		// 1. make sure nvram module not operating
		// modprobe_r("nvram_linux");
		//stop_nvram(nvram_mtd_name);
		// 2. backup current nvram to snapshot image
		if (!restore_default) {
			doSystem("dd if=%s of=%s/%s%d.img", nvram_mtd_name, NVSW_PATH, NVSW_SNAP_PREFIX, cur_nvswid);
			// 2.5. archive some extra settings, e.g. /jffs/openvpn, /jffs/.cert, /jffs/.le
			archive_op_misc(cur_nvswid, 1);
		}
		// 3. erase nvram partition
		eval("mtd-erase", "-d", "nvram");
		// 4. write target nvram image to nvram partition
		//doSystem("dd if=%s/%s%d.img of=%s", NVSW_PATH, NVSW_SNAP_PREFIX, changed_to_idx, nvram_mtd_name);
		doSystem("mtd-write -i %s/%s%d.img -d nvram", NVSW_PATH, NVSW_SNAP_PREFIX, changed_to_idx);
		// 5. react nvram module
                // modprobe("nvram_linux");
		reload_nvram();
		// 5.5. extract some extra settings
		if (!restore_default)
			archive_op_misc(changed_to_idx, 0);
		// 6. verify nvswid & reset some nvram if needed
		if (restore_default) { // come frome reset default
			nvram_set_int("nvswid", changed_to_idx);
			/// clear cro_args
			memset(&cross_val, 0x00, sizeof(cro_args));
			nvsw_update_args(changed_to_idx, &cross_val);
		}
		else {
			chg_nvswid = (unsigned char)nvram_get_int("nvswid");
			if (chg_nvswid != changed_to_idx) { // come frome upload config?
				_dprintf("[NVSW]: nvswid different in image, target:[%u], nv:[%u]!\n", changed_to_idx, chg_nvswid);
				nvram_set_int("nvswid", changed_to_idx); // overwrite
			}
		}
		//// update other nvram if needed
		fill_cross_args(&cross_val);
		nvsw_update_args(changed_to_idx, &cross_val);
		*outval = cur_nvswid;
		*outval = (*outval << 8) | changed_to_idx;
		nvsw_switching_val = *outval;
	} else {
		//// update other nvram if needed
		fill_cross_args(&cross_val);
		if (nvsw_update_args(cur_nvswid, &cross_val)) {
			if (cur_nvswid == 0 && cfg.btnsw_meaning != 1) // old firmware, update profile 0
				nvsw_update_stash(0, "Home", &cross_val);
		}
	}
#endif
#if defined(PRTAX57_GO) || defined(RTBE58_GO)
	if (restore_default) {
#if 0  // keep btnsw_meaning==1 in travel mode
		if (changed_to_idx > 0) // travel profile, keep btnsw_meaning
			;
		else { // home profile, clear btnsw_meaning
			cfg.btnsw_meaning = 0;
			f_write(NVSW_CFGFILE, &cfg, sizeof(nvswcfg), 0, 0);
		}
#else // keep btnsw_meaning==1 in both home/travel mode
		if (cfg.btnsw_meaning == BTNSW_NV_SWITCH) // keep btnsw_meaning
			;
		else { // clear btnsw_meaning
			cfg.btnsw_meaning = 0;
			f_write(NVSW_CFGFILE, &cfg, sizeof(nvswcfg), 0, 0);
		}
#endif
	}
	// overwrite btnsw by btnsw_meaning
	if (!nvram_get("btnsw_onoff") || ((unsigned char)nvram_get_int("btnsw_onoff") != cfg.btnsw_meaning)) {
		nvram_set_int("nvsw_ov_onoff", cfg.btnsw_meaning);
	} else {
		nvram_unset("nvsw_ov_onoff");
	}
#endif
	return 0;
}

int nvsw_set(unsigned char idx, unsigned char btnsw_meaning)
{
	nvswcfg cfg;
	unsigned long fsize;

	if (idx > NVSW_MAX_SNAPCOUNT)
		return -2;
#if defined(PRTAX57_GO) || defined(RTBE58_GO)
	if (btnsw_meaning == BTNSW_NV_SWITCH && idx == 0) // must give target idx if enabled
		return -2;
#endif
	fsize = f_size(NVSW_CFGFILE);
	if (fsize == sizeof(nvswcfg) && f_read(NVSW_CFGFILE, &cfg, fsize) == fsize) {
		int changed = 0;
		cro_args cross_val;
		if (cfg.nvsw_target_idx != idx) {
			cfg.nvsw_target_idx = idx;
			changed++;
		}
#if defined(PRTAX57_GO) || defined(RTBE58_GO)
		if (cfg.btnsw_meaning != btnsw_meaning) {
			cfg.btnsw_meaning = btnsw_meaning;
			changed++;
		}
#endif
		if (changed)
			f_write(NVSW_CFGFILE, &cfg, sizeof(nvswcfg), 0, 0);

		sync();
		//// update other nvram if needed
		fill_cross_args(&cross_val);
#if defined(RTBE58_GO)
		nvsw_update_args((unsigned char)GET_PRE_ID, &cross_val);
#else
		nvsw_update_args((unsigned char)nvram_get_int("nvswid"), &cross_val);
#endif
		return 0;
	}
	return -1;
}

int nvsw_cfg_get(unsigned char *target_idx, unsigned char *max_idx, unsigned char *btnsw_meaning)
{
	nvswcfg cfg;
	unsigned long fsize;

	fsize = f_size(NVSW_CFGFILE);
	if (fsize == sizeof(nvswcfg) && f_read(NVSW_CFGFILE, &cfg, fsize) == fsize) {
		*target_idx = cfg.nvsw_target_idx;
		*max_idx = NVSW_MAX_SNAPCOUNT;
#if defined(PRTAX57_GO) || defined(RTBE58_GO)
		*btnsw_meaning = cfg.btnsw_meaning;
#endif
		return 0;
	}
	return -1;
}

int nvsw_clone_nvimg(unsigned char idx, char *comment)
{
	unsigned char cur_nvswid;
	char nvram_mtd_name[40];
	int ret = 0;
	cro_args cross_val;

	if (idx > NVSW_MAX_SNAPCOUNT || idx == 0)
		return -2;

	cur_nvswid = (unsigned char)nvram_get_int("nvswid");
	if (got_nvram_mtd(nvram_mtd_name, sizeof(nvram_mtd_name)) != 0) {
		return -3;
	}
	// 1. change nvswid to target idx
	nvram_set_int("nvswid", idx);
	nvram_commit();
	// 2. snapshot nvram image
	ret = doSystem("dd if=%s of=%s/%s%d.img", nvram_mtd_name, NVSW_PATH, NVSW_SNAP_PREFIX, idx);
	// 3. write comment file
	fill_cross_args(&cross_val);
	ret |= nvsw_update_stash(idx, comment, &cross_val);
	// 3.5. archive some extra settings, e.g. /jffs/openvpn, /jffs/.cert, /jffs/.le
	archive_op_misc(idx, 1);
	// 4. write back nvswid
	nvram_set_int("nvswid", cur_nvswid);
	nvram_commit();
	if (ret == 0)
		return 0;
	return -1;
}

int nvsw_def_nvimg(unsigned char idx, char *comment)
{
	char nvram_mtd_name[40];
	char snapshot_name[PATH_MAX];
	unsigned int nvram_mtd_size;
	int ret = 0;

	if (idx > NVSW_MAX_SNAPCOUNT)
		return -2;

#if !defined(RTCONFIG_HND_ROUTER_BE_4916)
	if (got_nvram_mtd(nvram_mtd_name, sizeof(nvram_mtd_name)) != 0)
		return -3;

	nvram_mtd_size = nvsw_mtd_partsize(nvram_mtd_name);
	if (nvram_mtd_size == 0)
		return -4;

	ret = doSystem("tr '\\000' '\\377' < /dev/zero | dd of=%s/%s%d.img bs=1 count=%u", NVSW_PATH, NVSW_SNAP_PREFIX, idx, nvram_mtd_size);
#endif
	ret |= nvsw_update_stash(idx, comment, NULL);

	// remove misc archive
	sprintf(snapshot_name, "%s/%s%d.tar", NVSW_PATH, NVSW_SNAP_PREFIX, idx);
	unlink(snapshot_name);

	if (ret == 0)
		return 0;
	return -1;
}

int nvsw_get_info(unsigned char idx, char *comment_buf, int comment_len, unsigned char *args, int args_len)
{
	nvsw_stash_pt spt;
	if (idx > NVSW_MAX_SNAPCOUNT)
		return -2;
	spt = nvsw_get_stash(idx);
	if (!spt)
		return -1;
	if (comment_buf && comment_len > 0) {
		if (comment_len > NVSW_STASH_COMMENT_LEN) {
			memcpy(comment_buf, spt->comment, NVSW_STASH_COMMENT_LEN);
			comment_buf[NVSW_STASH_COMMENT_LEN] = '\0';
		} else {
			memcpy(comment_buf, spt->comment, comment_len-1);
			comment_buf[comment_len-1] = '\0';
		}
	}
	if (args && args_len > 0) {
		if (args_len >= sizeof(spt->bin.args)) {
			memcpy(args, spt->bin.bytes, sizeof(spt->bin.args));
		} else {
			_dprintf("[NVSW]: WARNING, args too small[%d],need[%d]!!!\n", args_len, sizeof(spt->bin.args));
			memcpy(args, spt->bin.bytes, args_len);
		}
	}
	free(spt);
	return 0;
}

int nvsw_statuslog_clear(void)
{
	nvswcfg cfg;
	unsigned long fsize;
	int ret;

	fsize = f_size(NVSW_CFGFILE);
	openlog("[NVSW]", 0, 0);
	if (fsize == sizeof(nvswcfg) && f_read(NVSW_CFGFILE, &cfg, fsize) == fsize) {
#if defined(PRTAX57_GO) || defined(RTBE58_GO)
		syslog(LOG_NOTICE, "btnsw:%u", cfg.btnsw_meaning);
#endif
		syslog(LOG_NOTICE, "target:%u, last_status:%u", cfg.nvsw_target_idx, cfg.last_status);
		if (cfg.last_status) {
			syslog(LOG_INFO, "err msg(%s)", cfg.debug_message);
			cfg.last_status = 0;
			memset(cfg.debug_message, 0x00, NVSW_ERR_STRING_MAX_LEN);
			f_write(NVSW_CFGFILE, &cfg, sizeof(nvswcfg), 0, 0);
		} else if (nvsw_switching_val) {
			syslog(LOG_NOTICE, "SWITCH from %d to %d", (nvsw_switching_val >> 8) & 0xff, nvsw_switching_val & 0xff);
		}
		ret = 0;
	} else {
		syslog(LOG_INFO, "[NVSW]read cfg fail!!");
		ret = -1;
	}
	closelog();
	return ret;
}

int nvsw_rm_nvimg(unsigned char idx)
{
	nvswcfg cfg;
	unsigned long fsize;

	if (idx > NVSW_MAX_SNAPCOUNT || idx == 0)
		return -2;

	fsize = f_size(NVSW_CFGFILE);
	if (fsize == sizeof(nvswcfg) && f_read(NVSW_CFGFILE, &cfg, fsize) == fsize) {
		char snapshot_name[PATH_MAX];
		sprintf(snapshot_name, "%s/%s%d.img", NVSW_PATH, NVSW_SNAP_PREFIX, idx);
		unlink(snapshot_name);
		sprintf(snapshot_name, "%s/%s%d.sth", NVSW_PATH, NVSW_SNAP_PREFIX, idx);
		unlink(snapshot_name);
		sprintf(snapshot_name, "%s/%s%d.tar", NVSW_PATH, NVSW_SNAP_PREFIX, idx);
		unlink(snapshot_name);
#if defined(PRTAX57_GO) || defined(RTBE58_GO)
		if (cfg.btnsw_meaning == BTNSW_NV_SWITCH && cfg.nvsw_target_idx == idx) {
			cfg.btnsw_meaning = BTNSW_DISABLED;
			cfg.nvsw_target_idx = 0;
			f_write(NVSW_CFGFILE, &cfg, sizeof(nvswcfg), 0, 0);
		}
#else
		if (cfg.nvsw_target_idx == idx) {
			cfg.nvsw_target_idx = 0;
			f_write(NVSW_CFGFILE, &cfg, sizeof(nvswcfg), 0, 0);
		}
#endif
		return 0;
	} else {
		return -1;
	}
}

static void nvsw_dump_all(void)
{
	unsigned char target_idx, max_idx, btnsw_meaning;
	if (nvsw_cfg_get(&target_idx, &max_idx, &btnsw_meaning)==0) {
		unsigned char i, nvswid, showid, mode[5];
		unsigned char comment_buf[50];
		int ret;
#if defined(RTBE58_GO)
		nvswid = GET_PRE_ID;
#else
		nvswid = nvram_get_int("nvswid");
#endif
		printf("btnsw_val:%u, cur_id:%d, target id:%u (max:%u)\n", btnsw_meaning, nvswid, target_idx, max_idx);
#if defined(PRTAX57_GO) || defined(RTBE58_GO)
		showid = nvswid;
#else
		showid = target_idx;
#endif
		printf("============================================\n");
		for (i = 0; i <= max_idx; i++) {
			ret = nvsw_get_info(i, comment_buf, sizeof(comment_buf), mode, sizeof(mode));
			if (ret)
				printf("%cprofile %u: %s [%s], --\n", (i==showid)? '*':' ', i, "N/A", "N/A");
			else
				printf("%cprofile %u: %s [%s], sw_mode[%u]/re[%u]/wisp[%u]/rp[%u]/mb[%u]\n", (i==showid)? '*':' ', \
						i, "OK.", comment_buf, mode[0], mode[1], mode[2], mode[3], mode[4]);
		}
	}
}



int nvsw_get_json_all(char *json_buffer, size_t json_buffer_size)
{
	int result = -1;
	if(!json_buffer || json_buffer_size == 0) {
		return result;
	}
	unsigned char target_idx, max_idx, btnsw_meaning;
	if (nvsw_cfg_get(&target_idx, &max_idx, &btnsw_meaning)==0) {
		unsigned char i, nvswid, showid, mode[5];
		unsigned char comment_buf[50];
		int ret;
#if defined(RTBE58_GO)
		nvswid = GET_PRE_ID;
#else
		nvswid = nvram_get_int("nvswid");
#endif
		size_t str_len = 0;
		str_len += snprintf(json_buffer + str_len, json_buffer_size - str_len,
			"{"
			"\"btnsw_val\": \"%u\","
			"\"cur_id\": \"%d\","
			"\"target_id\": \"%u\",",
			btnsw_meaning, nvswid, target_idx);
#if defined(PRTAX57_GO) || defined(RTBE58_GO)
		showid = nvswid;
#else
		showid = target_idx;
#endif
		str_len += snprintf(json_buffer + str_len, json_buffer_size - str_len, "\"profile\": [");
		for (i = 0; i <= max_idx; i++) {
			ret = nvsw_get_info(i, comment_buf, sizeof(comment_buf), mode, sizeof(mode));
			if (ret){
				str_len += snprintf(json_buffer + str_len, json_buffer_size - str_len,
				"{"
				"\"profile_id\": \"%d\","
				"\"status\": \"N/A\","
				"\"op_mode\": \"N/A\","
				"\"sw_mode\": \"0\","
				"\"re\": \"0\","
				"\"wisp\": \"0\","
				"\"rp\": \"0\","
				"\"mb\": \"0\""
				"}%s",
				i, (i < max_idx) ? "," : "");
			}
			else{
				str_len += snprintf(json_buffer + str_len, json_buffer_size - str_len,
				"{"
				"\"profile_id\": \"%d\","
				"\"status\": \"OK\","
				"\"op_mode\": \"%s\","
				"\"sw_mode\": \"%u\","
				"\"re\": \"%u\","
				"\"wisp\": \"%u\","
				"\"rp\": \"%u\","
				"\"mb\": \"%u\""
				"}%s",
				i, comment_buf, mode[0], mode[1], mode[2], mode[3], mode[4], (i < max_idx) ? "," : "");
			}
		}
		str_len += snprintf(json_buffer + str_len, json_buffer_size - str_len, "]");
		str_len += snprintf(json_buffer + str_len, json_buffer_size - str_len, "}");
		if (str_len >= json_buffer_size) {
			printf("Buffer overflow. JSON content is too large.\n");
		}
		else {
			result = 1;
		}
	}
	return result;
}

int nvsw_cmd(int argc, char **argv)
/* cmd format:
 * [nvsw] dump
 * [nvsw] set TARGET_IDX BTNSW_VAL
 * [nvsw] img clone IDX [COMMENT]
 * [nvsw] img default IDX [COMMENT]
 * [nvsw] img comment IDX NEW_COMMENT
 * [nvsw] img del idx
*/
{
	int ret = -1;
	if (argc < 1)
		goto usage_out;

	if (!strcmp(argv[0], "set")) {
		if (argc < 3)
			goto usage_out;
		ret = nvsw_set(atoi(argv[1]), atoi(argv[2]));
	} else if (!strcmp(argv[0], "dump")) {
		nvsw_dump_all();
		ret = 0;
#if 0
	} else if (!strcmp(argv[0], "img")) {
		if (!strcmp(argv[1], "clone") || !strcmp(argv[1], "default")) {
			if (argc < 3)
				goto usage_out;
			if (argv[1][0] == 'c')
				ret = nvsw_clone_nvimg(atoi(argv[2]), (argc > 3)? argv[3]:NULL);
			else if (argv[1][0] == 'd')
				ret = nvsw_def_nvimg(atoi(argv[2]), (argc > 3)? argv[3]:NULL);
		} else if (!strcmp(argv[1], "comment")) {
			if (argc < 4)
				goto usage_out;
			ret = nvsw_update_comment(atoi(argv[2]), argv[3]);
		} else if (!strcmp(argv[1], "del")) {
			if (argc < 3)
				goto usage_out;
			ret = nvsw_rm_nvimg(atoi(argv[2]));
		}
#endif
	} else
		goto usage_out;

	if (ret) {
		printf("ERR: cmd ret %d\n", ret);
		return ret;
	}
usage_out:
	if (ret) {
		printf("Usage:\n");
		printf(" nvsw dump\n");
#if 0
		printf(" nvsw set TARGET_IDX BTNSW_VAL\n");
		printf(" nvsw img clone IDX [COMMENT]\n");
		printf(" nvsw img default IDX [COMMENT]\n");
		printf(" nvsw img comment IDX NEW_COMMENT\n");
		printf(" nvsw img del idx\n");
		printf("==============================\n");
		printf("Note:\n");
		printf(" BTNSW_VAL=> 0:disabled, 1:profile switch, 2: LED ON/OFF...\n");
		printf(" IDX, must between 1-max\n\n");
#endif
	}
	return 0;
}
