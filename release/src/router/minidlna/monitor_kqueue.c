/*
 * Copyright (c) 2017 Gleb Smirnoff <glebius@FreeBSD.org>
 * Copyright (c) 2013 Bernard Spil <brnrd@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/stat.h>
#include <sys/event.h>
#include <limits.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "event.h"
#include "log.h"
#include "monitor.h"
#include "minidlnatypes.h"
#include "upnpglobalvars.h"
#include "sql.h"
#include "utils.h"

struct watch {
	struct event	ev;
	const char	*path;
	bool		isdir;
};

static void
dir_vnode_process(struct event *ev, u_int fflags)
{
	struct watch *wt;
	const char *path;
	char *sql, **result, tmp_path[PATH_MAX], *esc_name;
	int rows, result_path_len, i;
	DIR* d;
	struct dirent *entry;
	bool found_flag;

	wt = (struct watch *)ev->data;
	path = wt->path;

	if (fflags & NOTE_DELETE) {
		DPRINTF(E_DEBUG, L_INOTIFY, "Path [%s] deleted.\n", path);
		close(ev->fd);
		free(wt);
		monitor_remove_directory(0, path);
		free(path);
		return;
	} else if ((fflags & (NOTE_WRITE | NOTE_LINK)) ==
	    (NOTE_WRITE | NOTE_LINK)) {

		DPRINTF(E_DEBUG, L_INOTIFY, "Directory [%s] content updated\n",
		    path);
		sql = sqlite3_mprintf("SELECT PATH from DETAILS where "
		    "(PATH > '%q/' and PATH <= '%q/%c') and SIZE IS NULL",
		    path, path, 0xFF);
		DPRINTF(E_DEBUG, L_INOTIFY, "SQL: %s\n", sql);
		if ((sql_get_table(db, sql, &result, &rows, NULL) !=
		    SQLITE_OK)) {
			DPRINTF(E_WARN, L_INOTIFY,
			    "Read state [%s]: Query failed\n", path);
			goto err1;
		}

		for (i = 1; i <= rows; i++) {
			DPRINTF(E_DEBUG, L_INOTIFY,
			    "Indexed content: %s\n", result[i]);
			if (access(result[i], R_OK) == -1)
				monitor_remove_directory(0, result[i]);
		}

		if ((d = opendir(path)) == NULL) {
			DPRINTF(E_ERROR, L_INOTIFY, "Can't list [%s] (%s)\n",
			    path, strerror(errno));
			goto err2;
		}

		for (entry = readdir(d); entry != NULL; entry = readdir(d)) {
			if ((entry->d_type != DT_DIR) ||
			    (strcmp(entry->d_name, "..") == 0) ||
			    (strcmp(entry->d_name, ".") == 0))
				continue;

			result_path_len = snprintf(tmp_path, PATH_MAX,
			    "%s/%s", path, entry->d_name);
			if (result_path_len >= PATH_MAX) {
				DPRINTF(E_ERROR, L_INOTIFY,
				    "File path too long for %s!",
				    entry->d_name);
				continue;
			}

			DPRINTF(E_DEBUG, L_INOTIFY, "Walking %s\n", tmp_path);
			found_flag = false;
			for (i = 1; i <= rows; i++) {
				if (strcmp(result[i], tmp_path) == 0) {
					found_flag = true;
					break;
				}
			}
			if (!found_flag) {
				esc_name = strdup(entry->d_name);
				if (esc_name == NULL) {
					DPRINTF(E_ERROR, L_INOTIFY,
					    "strdup error");
					continue;
				}
				esc_name = modifyString(esc_name, "&", "&amp;amp;", 0);
				monitor_insert_directory(1, esc_name, tmp_path);
				free(esc_name);
			}
		}
	} else if (fflags & NOTE_WRITE) {

		DPRINTF(E_DEBUG, L_INOTIFY, "File [%s] content updated\n",
		    path);
		sql = sqlite3_mprintf("SELECT PATH from DETAILS where "
		    "(PATH > '%q/' and PATH <= '%q/%c') and SIZE IS NOT NULL",
		    path, path, 0xFF);
		if (sql_get_table(db, sql, &result, &rows, NULL) != SQLITE_OK) {
			DPRINTF(E_WARN, L_INOTIFY,
			    "Read state [%s]: Query failed\n", path);
			goto err1;
		}

		for (i = 1; i <= rows; i++) {
			DPRINTF(E_DEBUG, L_INOTIFY,
			    "Indexed content: %s\n", result[i]);
			if (access(result[i], R_OK) == -1)
				monitor_remove_file(result[i]);
		}

		if ((d = opendir(path)) == NULL) {
			DPRINTF(E_ERROR, L_INOTIFY,
			    "Can't list [%s] (%s)\n", path, strerror(errno));
			goto err2;
		}

		for (entry = readdir(d); entry != NULL; entry = readdir(d)) {
			if ((entry->d_type != DT_REG) &&
			    (entry->d_type != DT_LNK))
				continue;

			result_path_len = snprintf(tmp_path, PATH_MAX, "%s/%s",
			    path, entry->d_name);
			if (result_path_len >= PATH_MAX) {
				DPRINTF(E_ERROR, L_INOTIFY,
				    "File path too long for %s!",
				    entry->d_name);
				continue;
			}
			DPRINTF(E_DEBUG, L_INOTIFY, "Walking %s\n", tmp_path);
			found_flag = false;
			for (i = 1; i <= rows; i++)
				if (strcmp(result[i], tmp_path) == 0) {
					found_flag = true;
					break;
				}
			if (!found_flag ) {
				struct stat st;

				if (stat(tmp_path, &st) != 0) {
					DPRINTF(E_ERROR, L_INOTIFY,
					    "stat(%s): %s\n", tmp_path,
					    strerror(errno));
					continue;
				}
				esc_name = strdup(entry->d_name);
				if (esc_name == NULL) {
					DPRINTF(E_ERROR, L_INOTIFY,
					    "strdup error");
					continue;
				}
				esc_name = modifyString(esc_name, "&", "&amp;amp;", 0);
				if (S_ISDIR(st.st_mode))
					monitor_insert_directory(1, esc_name, tmp_path);
				else
					monitor_insert_file(esc_name, tmp_path);
				free(esc_name);
			}
		}
	} else
		return;

	closedir(d);
err2:
	sqlite3_free_table(result);
err1:
	sqlite3_free(sql);
}

int
monitor_add_watch(int fd __unused, const char *path)
{
	struct watch *wt;
	struct event *ev;
	int wd;

	wd = open(path, O_RDONLY);
	if (wd < 0) {
		DPRINTF(E_ERROR, L_INOTIFY, "open(%s) [%s]\n",
		    path, strerror(errno));
		return (errno);
	}

	if ((wt = malloc(sizeof(struct watch))) == NULL) {
		DPRINTF(E_ERROR, L_INOTIFY, "malloc() error\n");
		close(wd);
		return (ENOMEM);
	}
	if ((wt->path = strdup(path)) == NULL) {
		DPRINTF(E_ERROR, L_INOTIFY, "strdup() error\n");
		close(wd);
		free(wt);
		return (ENOMEM);
	}
	wt->isdir = true;
	ev = &wt->ev;
	ev->data = wt;
	ev->fd = wd;
	ev->rdwr = EVENT_VNODE;
	ev->process_vnode = dir_vnode_process;

	DPRINTF(E_DEBUG, L_INOTIFY, "kqueue add_watch [%s]\n", path);
	event_module.add(ev);

	return (0);
}

int
monitor_remove_watch(int fd __unused, const char *path __unused)
{

	return (0);
}

/*
 * XXXGL: this function has some copypaste with inotify_create_watches().
 * We need to push more code to platform independent start_monitor()
 * in minidlna.c.
 */
void
monitor_start()
{
	struct media_dir_s *media_path;
	char **result;
	int rows, i;

	DPRINTF(E_DEBUG, L_INOTIFY, "kqueue monitoring starting\n");
	for (media_path = media_dirs; media_path != NULL;
	    media_path = media_path->next)
		monitor_add_watch(0, media_path->path);
	sql_get_table(db, "SELECT PATH from DETAILS where MIME is NULL and PATH is not NULL", &result, &rows, NULL);
	for (i = 1; i <= rows; i++ )
		monitor_add_watch(0, result[i]);
	sqlite3_free_table(result);
}

void
monitor_stop()
{
}
