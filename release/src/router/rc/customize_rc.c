/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*
 * This is the implementation of a routine to notify the rc driver that it
 * should take some action.
 *
 * Copyright 2019, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of ASUSTeK Inc.;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of ASUSTeK Inc..
 */
#include <shared.h>

#ifdef RTCONFIG_ISP_CUSTOMIZE
#include <rc.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/statvfs.h>

#define PACKAGE_FILE "/jffs/package.tar.gz"
#define PACKAGE_HIDDEN_FILE "/jffs/.package.tar.gz"
#define PACKAGE_FOLDER "/jffs/.package"
#define PACKAGE_SIG_FOLDER PACKAGE_FOLDER"/sig"
#define PACKAGE_CORRUPT_FOLDER PACKAGE_FOLDER"/corrupt"
#define PACKAGE_SETTING PACKAGE_FOLDER"/settings.ini"
#define PACKAGE_VERSION PACKAGE_FOLDER"/version.conf"

typedef enum {
	STATUS_PASS = 0,
	STATUS_NOT_FOUND = 1,
	STATUS_VERIFY_FAILED = 2,
	STATUS_SPACE_INSUFFICIENT = 3
} VERIFY_STATUS;

static pid_t get_proc_ppid(pid_t pid)
{
	FILE *fp;
	char proc_status_path[NAME_MAX];
	char line_buf[300];
	char ppid[32];
	snprintf(proc_status_path, sizeof(proc_status_path), "/proc/%d/status", pid);
	//_dprintf("path=[%s]\n", proc_status_path);
	fp = fopen(proc_status_path, "r");
	if (fp) {
		while (fgets(line_buf, sizeof(line_buf), fp)) {
			//_dprintf("line_buf=[%s]\n", line_buf);
			if (strstr(line_buf, "PPid:")) {
				sscanf(line_buf, "%*s%s", ppid);
				fclose(fp);
				return atoi(ppid);
			}
		}
		fclose(fp);
	}
	return 0;
}

static int has_permission()
{
	/*
		The only permitted process tree of ATE command.
		----telnetd
		--------sh
		------------ATE
	*/
	pid_t pid = getppid();
	char pname[NAME_MAX];
	memset(pname, 0, sizeof(pname));
	psname(pid, pname, sizeof(pname));
	if (!strcmp(pname, "sh")) {
		memset(pname, 0, sizeof(pname));
		psname(get_proc_ppid(pid), pname, sizeof(pname));
		//_dprintf("has_permission() ppname=[%s(%d)]\n", ppname, get_proc_pid(pid));
		if (!strcmp(pname, "telnetd"))
			return 1;
		else
			return 0;
	} else
		return 0;
}

int get_file_hash(const char *file, char *out, int *len)
{
	FILE *fp;
	int line_len;
	char buf[100];

	if (!ate_factory_mode() || !has_permission())
		return -1;

	if (file)
	{
		if (!f_exists(file))
			return -2;

		sprintf(buf, "md5sum %s | cut -b-32", file);
		fp = popen(buf, "r");
		if (fp) {
			memset(buf, 0, sizeof(buf));
			line_len = fread(buf, 1, sizeof(buf), fp);
			pclose(fp);
			if (line_len > 1 && *len > line_len) {
				buf[line_len-1] = '\0';
				//_dprintf("get hash code=[%s]\n",buf);

				if (out && len) {
					snprintf(out, line_len, "%s", buf);
					*len = line_len;
				}

				return 0;
			}
		}
		return -3;
	}
	else
		_dprintf("read hash-source error !!\n");

	return -4;
}

int get_package_hash(char *out, int *len)
{
	return get_file_hash(PACKAGE_HIDDEN_FILE, out, len);
}

int get_package_version(char *out, int *len)
{
#define LINE_MAX (1024)
#define VER_ISP "ISP"
#define VER_MODEL "MODEL"
#define VER_VERSION "VERSION"
	char line_buffer[LINE_MAX];
	char *key_name,*value;
	char isp[16] = {0}, model[16] = {0}, ver[16] = {0};

	if (!ate_factory_mode() || !has_permission())
		return -1;

	if (!f_exists(PACKAGE_VERSION))
		return -2;

	FILE *fp = fopen(PACKAGE_VERSION,"r");
	if(fp) {
		while(fgets(line_buffer, LINE_MAX, fp) != NULL) {
			if(line_buffer[0]==';') continue;
			value = line_buffer;
			key_name = strsep(&value, "=");
			if (!strcmp(key_name, VER_ISP)) {
				//_dprintf("customize_setting_file=%s, name=%s. FOUND: value=%s\n", 
				//	PACKAGE_SETTING, name, value);
				snprintf(isp, sizeof(isp), "%s", trimNL(value));
			} else if (!strcmp(key_name, VER_MODEL)) {
				snprintf(model, sizeof(model), "%s", trimNL(value));
			} else if (!strcmp(key_name, VER_VERSION)) {
				snprintf(ver, sizeof(ver), "%s", trimNL(value));
			}
		}
		fclose(fp);

		if (strlen(isp) && strlen(model) && strlen(ver)) {
			if (*len > (strlen(isp) + strlen(model) + strlen(ver)))
			{
				if (out && len) {
					snprintf(out, *len, "Isp%s-%s-%s", isp, model, ver);
					*len = strlen(isp) + strlen(model) + strlen(ver);
				}
				//_dprintf("package version=%s\n", out);
			}
			else
				return -5;
		}
		else
			return -4;
	} else 
		return -3;

	return 0;

}

static unsigned long get_tar_real_size(const char *path)
{
	char line_buffer[LINE_MAX];
	char cmd[256];
	snprintf(cmd, sizeof(cmd), "tar -tzvf %s | awk '{s+=$3} END{print s}'", PACKAGE_HIDDEN_FILE);
	FILE *fp = popen(cmd,"r");
	if(fp) {
		while(fgets(line_buffer, LINE_MAX, fp) != NULL) {
			if (strlen(line_buffer))
				return (unsigned long)strtol(trimNL(line_buffer), NULL, 10);
		}
	}
	return 0;
}

static unsigned long get_free_space(const char *path)
{
	struct statvfs buf;

	if (statvfs(path, &buf) != 0) {
		return 0;
	}
	/*struct stat buf2;

	if (statvfs(path, &buf2) != 0) {
		return 0;
	}*/
	/*_dprintf("f_bsize=%lu\nf_frsize=%lu\nf_blocks=%lu\nf_bfree=%lu\n"
		"f_bavail=%lu\nf_files=%lu\nf_ffree=%lu\nf_favail=%lu\nf_fsid=%lu\nf_flag=%lu\nf_namemax=%lu\n", 
		buf.f_bsize, buf.f_frsize, buf.f_blocks, buf.f_bfree, 
		buf.f_bavail, buf.f_files, buf.f_ffree, buf.f_favail, buf.f_fsid, buf.f_flag, buf.f_namemax);
	_dprintf("st_blksize=%lu\n", buf2.st_blksize);*/

	return buf.f_bfree * buf.f_frsize;
}

#define WRITE_VERIFY_RESULT(f, s) \
{ \
	if (out) { \
		result_len += snprintf(result, *len - result_len, "%s_%d;", f, s); \
		result = out + result_len; \
	} \
}

int verify_package(char *out, int *len)
{
	FILE *fp_tar = NULL;
	//FILE *fp_verify = NULL;
	char cmd[256], buf1[256]/*, buf2[256]*/;

	char fpath[NAME_MAX+1];
	//char *fname[NAME_MAX+1];
	char fsig[NAME_MAX+1];
	char fcorrupt[NAME_MAX+1];

	char *result = out;
	int result_len = 0;
	int verify_failed = 0;

	if (!ate_factory_mode() || !has_permission())
		return -1;

	if (!f_exists(PACKAGE_HIDDEN_FILE) || !d_exists(PACKAGE_FOLDER))
		return -2;

	if (out)
		memset(out, 0, *len);

	snprintf(cmd, sizeof(fsig), "tar -ztf %s", PACKAGE_HIDDEN_FILE);
	if ((fp_tar = popen(cmd, "r")) != NULL) {
		memset(buf1, 0, sizeof(buf1));
		while(fgets(buf1, sizeof(buf1), fp_tar) != NULL) {
			snprintf(fpath, sizeof(fpath), "/jffs/%s", trimNL(buf1));
			snprintf(fcorrupt, sizeof(fcorrupt), "%s/%s", PACKAGE_CORRUPT_FOLDER, basename(fpath));

			// Skip directory or .package/sig folder
			if (d_exists(fpath) || !strncmp(fpath, PACKAGE_SIG_FOLDER, strlen(PACKAGE_SIG_FOLDER)))
				continue;

			_dprintf("file=[%s]\n", fpath);

			if (f_exists(fpath)) {
				//int verified = 0;

				snprintf(fsig, sizeof(fsig), "%s/%s.bin", PACKAGE_SIG_FOLDER, basename(fpath));

				/*snprintf(cmd, sizeof(cmd), "package_verify %s %s", fpath, fsig);

				if (f_exists(fsig) && (fp_verify = popen(cmd, "r")) != NULL) {
					memset(buf2, 0, sizeof(buf2));
					while(fgets(buf2, sizeof(buf2), fp_verify) != NULL) {
						//_dprintf("verify_package, result=%s. \n", buf2);
						if (!strcmp(trimNL(buf2), "package verify OK")) {
							verified = 1;
							break;
						}
					}
					pclose(fp_verify);
				}*/

				if (!check_package_sign(fpath, fsig)) {  // verify failed
					if (!d_exists(PACKAGE_CORRUPT_FOLDER))
						mkdir(PACKAGE_CORRUPT_FOLDER, 777);

					// move file to corrupt folder
					rename(fpath, fcorrupt);
					verify_failed = 1;
					WRITE_VERIFY_RESULT(basename(fpath), STATUS_VERIFY_FAILED);
				} else {
					WRITE_VERIFY_RESULT(basename(fpath), STATUS_PASS);
				}
			} else if (f_exists(fcorrupt)) {
				verify_failed = 1;
				WRITE_VERIFY_RESULT(basename(fpath), STATUS_VERIFY_FAILED);
			} else {
				verify_failed = 1;
				WRITE_VERIFY_RESULT(basename(fpath), STATUS_NOT_FOUND);
			}
		}
		pclose(fp_tar);
	}

	if (out && result_len) {
		// check free space if verify failed.
		if (verify_failed) {
			unsigned long free_space = get_free_space("/jffs");
			unsigned long space_need = get_tar_real_size(PACKAGE_HIDDEN_FILE);
			_dprintf("free_space=[%lu], space_need=[%lu]\n", free_space, space_need);
			if (free_space <= space_need)
				WRITE_VERIFY_RESULT("/jffs", STATUS_SPACE_INSUFFICIENT);
		}
		out[result_len-1] = '\0';
		*len = result_len - 1;
		_dprintf("verify_result=[%s]\n", out);
	}

	if (verify_failed) {
		return 0;
	}
	else
		return 1;
}

int delete_file(const char *file)
{
	if (!ate_factory_mode() || !has_permission())
		return -1;

	if (!f_exists(file) && !d_exists(file))
		return -2;

	if (f_exists(file)) {
		remove(file);
	}

	if(d_exists(file)) {
		remove(file);
	}

	return 0;
}

static void unload_package(char *path)
{
	char *unmount_argv[] = {"umount", "-l", path, NULL};
	char *rm_argv[] = {"rm", "-rf", path, NULL};
	_eval(unmount_argv, NULL, 0, NULL);
	_eval(rm_argv, NULL, 0, NULL);
}

int delete_package()
{
	if (!ate_factory_mode() || !has_permission())
		return -1;

	if (!f_exists(PACKAGE_HIDDEN_FILE) && !d_exists(PACKAGE_FOLDER))
		return -2;

	if (f_exists(PACKAGE_HIDDEN_FILE))
		unload_package(PACKAGE_HIDDEN_FILE);

	if(d_exists(PACKAGE_FOLDER))
		unload_package(PACKAGE_FOLDER);

	return 0;
}

static void load_package(char *path)
{
	char *bind_argv[] = {"mount", "--bind", path, path, NULL};
	char *remount_argv[] = {"mount", "-o", "remount,ro,bind", path, NULL};

	_eval(bind_argv, NULL, 0, NULL);
	_eval(remount_argv, NULL, 0, NULL);
}

void load_customize_package()
{
	int verified = 0;

	if (f_exists(PACKAGE_FILE))
		rename(PACKAGE_FILE, PACKAGE_HIDDEN_FILE);

	if(d_exists(PACKAGE_FOLDER)) {
		_dprintf("ISP package folder exists.\n");

		verified = verify_package(NULL, NULL);
	}

	if (!d_exists(PACKAGE_FOLDER) || verified != 1) {
		// decompress
		if (f_exists(PACKAGE_HIDDEN_FILE)) {
			load_package(PACKAGE_HIDDEN_FILE);
			// remove first
			char *rm_argv[] = {"rm", "-rf", PACKAGE_FOLDER, NULL};
			_eval(rm_argv, NULL, 0, NULL);

			char *decomp_argv[] = {"tar", "-xzf", PACKAGE_HIDDEN_FILE, "-C", "/jffs", NULL};
			_eval(decomp_argv, NULL, 0, NULL);
		} else
			return;
	}

	load_package(PACKAGE_FOLDER);
}

/* returns number of strings replaced.
*/
static int replacestr(char *line, const char *end, const char *search, const char *replace)
{
        int count;
        char *sp; // start of pattern

        if ((sp = strstr(line, search)) == NULL) {
                return(0);
        }
        count = 1;
        int sLen = strlen(search);
        int rLen = strlen(replace);
 	int tLen = strlen(sp) - sLen;
        char *src = sp + sLen;
        char *dst = sp + rLen;

	if(dst+tLen > end-1)
		return 0;

	memmove(dst, src, tLen);
	*(dst+tLen) = '\0';
        memcpy(sp, replace, rLen);

        count += replacestr(sp + rLen, end, search, replace);

        return(count);
}

void package_restore_defaults()
{
#define NVRAM_LINE_MAX (16384)
	char line_buffer[NVRAM_LINE_MAX];
	char multi_line_buffer[NVRAM_LINE_MAX];
	char *key_name,*value;
	int line_len, multi_buffer_left = 0;
	char *line_start, *line_current;
	int rep_count = 0;

	FILE *fp=fopen(PACKAGE_SETTING,"r");
	if(fp) {
		line_start = line_current = &multi_line_buffer[0];
		multi_buffer_left = sizeof(multi_line_buffer);
		while(fgets(line_buffer,NVRAM_LINE_MAX,fp) != NULL) {
			if(!strlen(line_buffer)) 
				continue;

			rep_count = replacestr(line_buffer, line_buffer+sizeof(line_buffer), "\\", "");
			if((line_len = strlen(line_buffer)) > 0) {
				if (multi_buffer_left > line_len) { // Check if buffer is enough.
					multi_buffer_left -= snprintf(line_current, multi_buffer_left, "%s", line_buffer);
					if (rep_count > 0) { // If '\\' occurs in line_buffer, read next line continuely.
						/*_dprintf("rep_count=[%d], line_len=[%d], line_current=[%s]\n", 
							rep_count, line_len, line_current);*/
						//_dprintf("[%d][%s]", multi_buffer_left, line_current);
						line_current = line_start + (sizeof(multi_line_buffer) - multi_buffer_left);
						continue;
					} else {
						// move cursor to start
						/*_dprintf("multi_buffer_left=[%d]\n", multi_buffer_left);
						int i;
						for (i=0; i < (NVRAM_LINE_MAX-multi_buffer_left); i++)
							_dprintf("%c", line_start[i]);
						_dprintf("\n");*/
						line_current = &multi_line_buffer[0];
						multi_buffer_left = sizeof(multi_line_buffer);
					}
				}
			}

			if(line_start[0]==';' || line_start[0]=='#')
				continue;

			value = line_start;
			key_name = strsep(&value,"=");
			nvram_set(key_name, trimNL(value));
			/*_dprintf("restore_package_default, name=%s, value=%s\n", 
				key_name, trimNL(value));*/
		}
		fclose(fp);
		nvram_commit();
	}
}
#endif
