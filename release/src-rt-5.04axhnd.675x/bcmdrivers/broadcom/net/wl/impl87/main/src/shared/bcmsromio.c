/*
 * Routines to access SPROM and to parse SROM/CIS variables.
 *
 * Copyright (C) 2022, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: bcmsromio.c $
 */

#if defined(BCA_SROMMAP)

#include <bcm_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <bcmdevs.h>
#include <pcicfg.h>
#include <siutils.h>
#include <bcmsrom.h>
#include <bcmsrom_tbl.h>
#include <bcmsromio.h>

#if !defined(BCMDONGLEHOST)
extern void BCMATTACHFN(varbuf_init)(varbuf_t *b, char *buf, uint size);
extern int BCMATTACHFN(varbuf_append)(varbuf_t *b, const char *fmt, ...);
extern uint mask_shift(uint16 mask);
extern uint mask_width(uint16 mask);
void _initvars_srom_pci(uint8 sromrev, uint16 *srom, uint off, varbuf_t *b);
#endif

#define MAX_BOARD_ID_SIZE 64

#define BOARD_ID_LOC1     "/proc/nvram/boardid"
#define BOARD_ID_LOC2     "/proc/environment/boardid" /* uBoot */
#define SROM_CUSTOM_FILE  "/data/.wlsromcustomerfile.nvm"
#define CMN_NVRAM_VARS_FILE    "/etc/wlan/bcmcmn_nvramvars.bin"

#define MAX_SROM_FILE_SIZE SROM_MAX
#define MAX_TOT_NVRAM_FILE_NAME_SIZE 256
#define NVRAM_FILE_NAME_SIZE 64
char nvramloaded[] = "/tmp/nvramloaded";
char nvvarloaded[] = "/tmp/nvvarloaded";

typedef struct entry_struct {
	unsigned short offset;
	unsigned short value;
} Entry_struct;

typedef struct adapter_struct {
	unsigned char id[SI_DEVPATH_BUFSZ];
	unsigned short entry_num;
	struct entry_struct  entry[1];
} Adapter_struct;

#define get_userspace_file(fpp, fln, buff, bufl, flag) \
	(__get_userspace_file((fpp), (fln), (buff), (bufl), (flag), 0))
#define get_userspace_file_mod(fpp, fln, buff, bufl, flag, perm) \
	(__get_userspace_file((fpp), (fln), (buff), (bufl), (flag), (perm)))

int
__get_userspace_file(struct file **fpp, uchar *file_location, char *buff,
		int *buf_len, int flag, int permission)
{
	mm_segment_t fs;
	int size =  0;
	int ret  = 0;

	*fpp = filp_open(file_location, flag, permission);
	if (!IS_ERR(*fpp))
	{
		fs = get_fs();
		set_fs(get_ds());
		(*fpp)->f_pos = 0;

		size = kernel_read((*fpp), (void *)buff, *buf_len,
				&(*fpp)->f_pos);
		set_fs(fs);
		*buf_len = size;

		ret = 1;
	} else {
#if defined(BCMDBG)
		printf("read%s %s ret:%ld\n", (flag & O_CREAT) ? "/create" : "",
			file_location, PTR_ERR((*fpp)));
#endif
		*buf_len = 0;
		*fpp = NULL;
	}
	return ret;
}

void
append_userspace_file(struct file **fpp, char *fname, int size)
{
	mm_segment_t fs;

	fs = get_fs();
	set_fs(get_ds());
	kernel_write((*fpp), fname, size, &(*fpp)->f_pos);
	set_fs(fs);
	return;
}

int
read_userfile(char *fname, void *buf, int *size)
{
	struct file *fp = NULL;
	int ret = -1;

	if (get_userspace_file(&fp, fname, buf, size, O_RDONLY)) {
		printf("wl: reading %s (size=%d)\n", fname, *size);
		filp_close(fp, NULL);
		ret = 0;
	}
	return ret;
}

int
findMatching(char *buf, char *words, int size)
{
	int i = 0;
	char *cur = buf;
	while (i < size/NVRAM_FILE_NAME_SIZE) {
		if (!strcmp(cur, words))
			return 1;
		else
			cur += NVRAM_FILE_NAME_SIZE;
		i++;
	}
	return 0;
}

int
get_boardid(void *buf)
{
	int ret = -1;
	int size = MAX_BOARD_ID_SIZE;

	/* read boardid from /proc */
	if ((ret = read_userfile(BOARD_ID_LOC1, buf, &size)) != 0) {
		if ((ret = read_userfile(BOARD_ID_LOC2, buf, &size)) != 0) {
			printf("%s: failed!\n", __FUNCTION__);
		}
	}
	return ret;
}

int
read_srom_custom_file(void *buf, int *size)
{
	int ret = -1;

	/* read srom customer file */
	if ((ret = read_userfile(SROM_CUSTOM_FILE, buf, size)) != 0) {
		printf("%s: failed!\n", __FUNCTION__);
	}
	return ret;
}

int
BCMATTACHFN(sprom_update_params)(si_t *sih, uint16 *buf)
{
	uint16 adapter_num = 0, entry_num = 0, pos = 0;
	struct adapter_struct *adapter_ptr = NULL;
	struct entry_struct *entry_ptr = NULL;
	char id[SI_DEVPATH_BUFSZ];
	int i = 0, j = 0;
	int ret = 0;

	char devpath[SI_DEVPATH_BUFSZ];
	char *params = NULL;
	int param_len = MAX_SROM_FILE_SIZE;

	params = kzalloc(MAX_SROM_FILE_SIZE, GFP_KERNEL);
	if (!params)
		return -1;

	si_devpath(sih, devpath, sizeof(devpath));
	if (read_srom_custom_file(params, &param_len) != 0)
		return -1;

	/* params format
		adapter_num    uint16
		devId1         char 16
		entry_num      uint16
		offset1        uint16
		value1         uint16
		offset2        uint16
		value2         uint16
		...
		devId2         char 16
		entry_num      uint16
		offset1        uint16
		value2         uint16
	*/
	adapter_num = *(uint16 *)(params);
	pos = 2;

	for (i = 0; (i < adapter_num) && (pos < param_len); i++) {
		adapter_ptr = (struct adapter_struct *)(params + pos);
		strncpy(id, adapter_ptr->id, SI_DEVPATH_BUFSZ);
		id[sizeof(id) - 1] = '\0';
		entry_num = adapter_ptr->entry_num;
		if (!strncmp(id, devpath, strlen(devpath))) {
			entry_ptr = (struct entry_struct *)&(adapter_ptr->entry);
			printf("wl: updating srom from flash...\n");
			for (j = 0; j < entry_num; j++, entry_ptr++) {
				buf[entry_ptr->offset] = entry_ptr->value;
			}
			ret = 1;
			break;
		}

		/* goto next adapter parameter */
		pos += SI_DEVPATH_BUFSZ + sizeof(uint16) +
			entry_num * sizeof(uint16) * 2;
	}
	kfree(params);
	return ret;
}

int
readSromFile(si_t *sih, uint chipId, void *buf, uint nbytes, char *pBoardId)
{
	struct file *fp = NULL;
	char fname[64] = {0};
	char BoardId[32] = {0};
	char *base = NULL;
	int size = 0;
	int ret = -1;
	char devpath[SI_DEVPATH_BUFSZ];
	struct file *nvramloaded_fp = NULL;
	char buff[MAX_TOT_NVRAM_FILE_NAME_SIZE] = {0};
	char srommap_path[MAX_TOT_NVRAM_FILE_NAME_SIZE] = "/etc/wlan";

	size = MAX_TOT_NVRAM_FILE_NAME_SIZE;

	if (!get_userspace_file_mod(&nvramloaded_fp, nvramloaded,
		buff, &size, O_RDWR|O_CREAT, 0644)) {
		return ret;
	}

	si_devpath(sih, devpath, sizeof(devpath));
	printf("wl: ID=%s\n", devpath);

	if (pBoardId)
		sprintf(BoardId, "_%s", pBoardId);

#ifdef SROMMAP_PATH
	printf("defined SROMMAP_PATH=%s\n", SROMMAP_PATH);
	memset(srommap_path, 0, MAX_TOT_NVRAM_FILE_NAME_SIZE);
	strncpy(srommap_path, SROMMAP_PATH, MAX_TOT_NVRAM_FILE_NAME_SIZE);
#endif /* SROMMAP_PATH */

	if ((chipId & 0xff00) == 0x4300 || (chipId & 0xff00) == 0x6300) {
		int i = 1;
		sprintf(fname, "%s/bcm%04x%s_map.bin", srommap_path, chipId,
				BoardId);
		while (findMatching(buff, fname, size))
			sprintf(fname, "%s/bcm%04x%s_wl%d_map.bin",
					srommap_path, chipId, BoardId, i++);
	} else if ((chipId / 1000) == 43) {
		int i = 1;
		sprintf(fname, "%s/bcm%d%s_map.bin", srommap_path, chipId,
				BoardId);
		while (findMatching(buff, fname, size))
			sprintf(fname, "%s/bcm%d%s_wl%d_map.bin", srommap_path,
					chipId, BoardId, i++);
	} else {
		filp_close(nvramloaded_fp, NULL);
		return ret;
	}

	base = kmalloc(MAX_SROM_FILE_SIZE, GFP_KERNEL);
	if (!base) {
		printf("%s: failed to malloc.\n", __FUNCTION__);
		return ret;
	}

	size = MAX_SROM_FILE_SIZE;

	if (get_userspace_file(&fp, fname, base, &size, O_RDONLY)) {
		printf("wl: loading %s\n", fname);

		bcopy(base, buf, MIN(size, nbytes));
		ret = 0;

		filp_close(fp, NULL);
		kfree(base);

		/* update loaded... */
		append_userspace_file(&nvramloaded_fp, fname,
				NVRAM_FILE_NAME_SIZE);
	}

	if (nvramloaded_fp)
		filp_close(nvramloaded_fp, NULL);

	return ret;
}

int
BCMATTACHFN(init_srom_sw_map)(si_t *sih, uint chipId, void *buf, uint nbytes)
{
	int ret = -1;
	char pszBoardId[MAX_BOARD_ID_SIZE] = {0};

	ASSERT(nbytes <= MAX_SROM_FILE_SIZE);

	get_boardid(pszBoardId);
	printf("%s: BoardId=%s\n", __FUNCTION__, pszBoardId);

	/* Two kinds of srom files needs be checked.
	 *  First is chip+board depended (e.g bcm<chipid>_<boardid>_map.bin).
	 *  If it is not found, the second one, chip depended (e.g.
	 *  bcm<chipid>_map.bin), needs be checked.
	 */
	if ((ret = readSromFile(sih, chipId, buf, nbytes, pszBoardId)) != 0) {
		ret = readSromFile(sih, chipId, buf, nbytes, NULL);
	}

	return ret;
}

int
read_sromfile(void *swmap, void *buf, uint offset, uint nbytes)
{
	bcopy((char*)swmap + offset, (char*)buf, nbytes);
	return 0;
}

int
BCMATTACHFN(init_nvramvars_cmn)(si_t *sih, void *buf)
{
	int size = MAXSZ_NVRAM_VARS;

	/* read common nvram */
	return read_userfile(CMN_NVRAM_VARS_FILE, buf, &size);
}

static uint
BCMATTACHFN(srom_vars_num)(char *vars)
{
	uint num = 0;
	char *s;

	for (s = vars; s && *s;) {
		if (strcmp(s, "END") == 0)
			break;
		num ++;
		s += strlen(s) + 1;
	}
	return num; /* NOT include the "END\0" */
}

int
BCMATTACHFN(init_nvramvars_chip)(si_t *sih, uint chipId, void *buf)
{
	char fname[NVRAM_FILE_NAME_SIZE] = {0};
	char chip_name[32] = {0};
	char BoardId[MAX_BOARD_ID_SIZE] = {0};
	int ret = -1, i = 0, found = 0;
	struct file *nvvarloaded_fp = NULL;
	char buff[MAX_TOT_NVRAM_FILE_NAME_SIZE] = {0};
	int size = MAX_TOT_NVRAM_FILE_NAME_SIZE;

	/* read nvram per chip */
	if ((chipId & 0xff00) == 0x4300 || (chipId & 0xff00) == 0x6300)
		sprintf(chip_name, "bcm%04x", chipId);
	else if ((chipId / 1000) == 43)
		sprintf(chip_name, "bcm%d", chipId);
	else if ((chipId / 1000) == 53) // 47189 53573
		sprintf(chip_name, "bcm%d", 47189);
	else
		return ret;

	get_userspace_file_mod(&nvvarloaded_fp, nvvarloaded, buff, &size,
		O_RDWR|O_CREAT, 0644);

	if (get_boardid(BoardId) == 0)
		i = 0;
	else /* no board ID ?! , just try bcmXXXX_nvramvars.bin */
		i = 2;

	while (i < 4) {
		switch (i) {
		case 0: /* for wl0 , no append wl0 on filename */
			sprintf(fname, "/etc/wlan/%s_%s_nvramvars.bin",
					chip_name, BoardId);
			break;
		case 1: /* for wl1 , append wlx on filename */
			sprintf(fname, "/etc/wlan/%s_%s_wl%d_nvramvars.bin",
					chip_name, BoardId, i);
			break;
		case 2: /* for last try, check filename with chipID only */
			sprintf(fname, "/etc/wlan/%s_nvramvars.bin", chip_name);
			break;
		default:
			i++; /* no def, try next one */
			continue;
			break;
		}

		/* check file is first time to load or not */
		if (findMatching(buff, fname, size) == 0) {
			ret = read_userfile(fname, buf, &size);
			if (ret == 0) {
				printf("Apply NVRAMVARS:%s\n", fname);
				/* update loaded... */
				append_userspace_file(&nvvarloaded_fp, fname,
					NVRAM_FILE_NAME_SIZE);
				found = 1;
				break;
			}
		}
		i++;
	}

	if (nvvarloaded_fp) {
		filp_close(nvvarloaded_fp, NULL);
		nvvarloaded_fp = NULL;
	}

	if (!found) {
		return -1;
	}

	return ret;
}

int
BCMATTACHFN(init_nvramvars)(si_t *sih, osl_t *osh, varbuf_t *b)
{
	int i, off = 0;
	int err = 0;
	char *buf;

	buf = MALLOC(osh, MAXSZ_NVRAM_VARS);
	ASSERT(buf != NULL);
	if (!buf) {
		printf("%s: alloc buf failed for nvram vars!\n", __FUNCTION__);
		goto exit;
	}
	bzero(buf, MAXSZ_NVRAM_VARS);
	err = init_nvramvars_cmn(sih, buf);
	if (err)
		goto next_chip;
	for (i = 0; i < srom_vars_num(buf); i++) {
#if defined(BCMDBG)
		printf("[%d] %s\n", i, buf + off);
#endif
		off += varbuf_append(b, buf + off);
	}

next_chip:
	off = 0;
	bzero(buf, MAXSZ_NVRAM_VARS);
	err = init_nvramvars_chip(sih, sih->chip, buf);
	if (err)
		goto exit;
	for (i = 0; i < srom_vars_num(buf); i++) {
#if defined(BCMDBG)
		printf("[%d] %s\n", i, buf + off);
#endif
		off += varbuf_append(b, buf + off);
	}

exit:
	if (buf)
		MFREE(osh, buf, MAXSZ_NVRAM_VARS);
	return 0;
}

/* The nvramloaded file saves the loaded srommap info when wl module is loaded.
 * The file needs to be reset/zero out when wl module is unloaded.
 * wlan driver may be loaded/unloaded during WLAN Deep Power Down activity.
 */
void BCMATTACHFN(reinit_loaded_srommap)(void)
{
	struct file *nvramloaded_fp = NULL;
	char buff_loaded[MAX_TOT_NVRAM_FILE_NAME_SIZE] = {0};
	int size = 0;

	size = sizeof(buff_loaded);
	if (!get_userspace_file_mod(&nvramloaded_fp, nvramloaded, buff_loaded,
		&size, O_RDWR, 0644))
	{
		/* no such loaded file exists */
		return;
	}

	memset(buff_loaded, 0, sizeof(buff_loaded));

	if (nvramloaded_fp) {
		nvramloaded_fp->f_pos = 0;
		append_userspace_file(&nvramloaded_fp, buff_loaded, sizeof(buff_loaded));
		filp_close(nvramloaded_fp, NULL);
	}
}
#endif /* BCA_SROMMAP */
