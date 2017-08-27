/*
 * Copyright (C) 2008-2009 Tobias Brunner
 * Copyright (C) 2007 Martin Willi
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>
#include <termios.h>
#include <stdarg.h>

#include <utils/debug.h>
#include <collections/linked_list.h>

#include "dumm.h"
#include "guest.h"
#include "mconsole.h"
#include "cowfs.h"

#define PERME (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#define PERM (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)

#define MASTER_DIR "master"
#define DIFF_DIR "diff"
#define UNION_DIR "union"
#define ARGS_FILE "args"
#define PID_FILE "pid"
#define KERNEL_FILE "linux"
#define LOG_FILE "boot.log"
#define NOTIFY_FILE "notify"
#define PTYS 0

typedef struct private_guest_t private_guest_t;

struct private_guest_t {
	/** implemented public interface */
	guest_t public;
	/** name of the guest */
	char *name;
	/** directory of guest */
	int dir;
	/** directory name of guest */
	char *dirname;
	/** additional args to pass to guest */
	char *args;
	/** pid of guest child process */
	int pid;
	/** state of guest */
	guest_state_t state;
	/** FUSE cowfs instance */
	cowfs_t *cowfs;
	/** mconsole to control running UML */
	mconsole_t *mconsole;
	/** list of interfaces attached to the guest */
	linked_list_t *ifaces;
};

ENUM(guest_state_names, GUEST_STOPPED, GUEST_STOPPING,
	"STOPPED",
	"STARTING",
	"RUNNING",
	"PAUSED",
	"STOPPING",
);

METHOD(guest_t, get_name, char*,
	private_guest_t *this)
{
	return this->name;
}

METHOD(guest_t, create_iface, iface_t*,
	private_guest_t *this, char *name)
{
	enumerator_t *enumerator;
	iface_t *iface;

	if (this->state != GUEST_RUNNING)
	{
		DBG1(DBG_LIB, "guest '%s' not running, unable to add interface",
			 this->name);
		return NULL;
	}

	enumerator = this->ifaces->create_enumerator(this->ifaces);
	while (enumerator->enumerate(enumerator, (void**)&iface))
	{
		if (streq(name, iface->get_guestif(iface)))
		{
			DBG1(DBG_LIB, "guest '%s' already has an interface '%s'",
				 this->name, name);
			enumerator->destroy(enumerator);
			return NULL;
		}
	}
	enumerator->destroy(enumerator);

	iface = iface_create(name, &this->public, this->mconsole);
	if (iface)
	{
		this->ifaces->insert_last(this->ifaces, iface);
	}
	return iface;
}

METHOD(guest_t, destroy_iface, void,
	private_guest_t *this, iface_t *iface)
{
	enumerator_t *enumerator;
	iface_t *current;

	enumerator = this->ifaces->create_enumerator(this->ifaces);
	while (enumerator->enumerate(enumerator, (void**)&current))
	{
		if (current == iface)
		{
			this->ifaces->remove_at(this->ifaces, enumerator);
			current->destroy(current);
			break;
		}
	}
	enumerator->destroy(enumerator);
}

METHOD(guest_t, create_iface_enumerator, enumerator_t*,
	private_guest_t *this)
{
	return this->ifaces->create_enumerator(this->ifaces);
}

METHOD(guest_t, get_state, guest_state_t,
	private_guest_t *this)
{
	return this->state;
}

METHOD(guest_t, get_pid, pid_t,
	private_guest_t *this)
{
	return this->pid;
}

/**
 * write format string to a buffer, and advance buffer position
 */
static char* write_arg(char **pos, size_t *left, char *format, ...)
{
	size_t len;
	char *res = NULL;
	va_list args;

	va_start(args, format);
	len = vsnprintf(*pos, *left, format, args);
	va_end(args);
	if (len < *left)
	{
		res = *pos;
		len++;
		*pos += len + 1;
		*left -= len + 1;
	}
	return res;
}

METHOD(guest_t, stop, void,
	private_guest_t *this, idle_function_t idle)
{
	if (this->state != GUEST_STOPPED)
	{
		this->state = GUEST_STOPPING;
		this->ifaces->destroy_offset(this->ifaces, offsetof(iface_t, destroy));
		this->ifaces = linked_list_create();
		kill(this->pid, SIGINT);
		while (this->state != GUEST_STOPPED)
		{
			if (idle)
			{
				idle();
			}
			else
			{
				usleep(50000);
			}
		}
		unlinkat(this->dir, PID_FILE, 0);
		this->pid = 0;
	}
}

/**
 * save pid in file
 */
void savepid(private_guest_t *this)
{
	FILE *file;

	file = fdopen(openat(this->dir, PID_FILE, O_RDWR | O_CREAT | O_TRUNC,
						 PERM), "w");
	if (file)
	{
		fprintf(file, "%d", this->pid);
		fclose(file);
	}
}

METHOD(guest_t, start, bool,
	private_guest_t *this, invoke_function_t invoke, void* data,
	idle_function_t idle)
{
	char buf[2048];
	char *notify;
	char *pos = buf;
	char *args[32];
	int i = 0;
	size_t left = sizeof(buf);

	memset(args, 0, sizeof(args));

	if (this->state != GUEST_STOPPED)
	{
		DBG1(DBG_LIB, "unable to start guest in state %N", guest_state_names,
			 this->state);
		return FALSE;
	}
	this->state = GUEST_STARTING;

	notify = write_arg(&pos, &left, "%s/%s", this->dirname, NOTIFY_FILE);

	args[i++] = write_arg(&pos, &left, "nice");
	args[i++] = write_arg(&pos, &left, "%s/%s", this->dirname, KERNEL_FILE);
	args[i++] = write_arg(&pos, &left, "root=/dev/root");
	args[i++] = write_arg(&pos, &left, "rootfstype=hostfs");
	args[i++] = write_arg(&pos, &left, "rootflags=%s/%s", this->dirname, UNION_DIR);
	args[i++] = write_arg(&pos, &left, "uml_dir=%s", this->dirname);
	args[i++] = write_arg(&pos, &left, "umid=%s", this->name);
	args[i++] = write_arg(&pos, &left, "mconsole=notify:%s", notify);
	args[i++] = write_arg(&pos, &left, "con=null");
	if (this->args)
	{
		args[i++] = this->args;
	}

	this->pid = invoke(data, &this->public, args, i);
	if (!this->pid)
	{
		this->state = GUEST_STOPPED;
		return FALSE;
	}
	savepid(this);

	/* open mconsole */
	this->mconsole = mconsole_create(notify, idle);
	if (this->mconsole == NULL)
	{
		DBG1(DBG_LIB, "opening mconsole at '%s' failed, stopping guest", buf);
		stop(this, NULL);
		return FALSE;
	}

	this->state = GUEST_RUNNING;
	return TRUE;
}

METHOD(guest_t, add_overlay, bool,
	private_guest_t *this, char *path)
{
	if (path == NULL)
	{
		return FALSE;
	}

	if (access(path, F_OK) != 0)
	{
		if (!mkdir_p(path, PERME))
		{
			DBG1(DBG_LIB, "creating overlay for guest '%s' failed: %m",
				 this->name);
			return FALSE;
		}
	}

	return this->cowfs->add_overlay(this->cowfs, path);
}

METHOD(guest_t, del_overlay, bool,
	private_guest_t *this, char *path)
{
	return this->cowfs->del_overlay(this->cowfs, path);
}

METHOD(guest_t, pop_overlay, bool,
	private_guest_t *this)
{
	return this->cowfs->pop_overlay(this->cowfs);
}

/**
 * Variadic version of the exec function
 */
static int vexec(private_guest_t *this, void(*cb)(void*,char*,size_t), void *data,
				 char *cmd, va_list args)
{
	char buf[1024];
	size_t len;

	if (this->mconsole)
	{
		len = vsnprintf(buf, sizeof(buf), cmd, args);

		if (len > 0 && len < sizeof(buf))
		{
			return this->mconsole->exec(this->mconsole, cb, data, buf);
		}
	}
	return -1;
}

METHOD(guest_t, exec, int,
	private_guest_t *this, void(*cb)(void*,char*,size_t), void *data,
	char *cmd, ...)
{
	int res;
	va_list args;
	va_start(args, cmd);
	res = vexec(this, cb, data, cmd, args);
	va_end(args);
	return res;
}

typedef struct {
	chunk_t buf;
	void (*cb)(void*,char*);
	void *data;
} exec_str_t;

/**
 * callback that combines chunks to a string. if a callback is given, the string
 * is split at newlines and the callback is called for each line.
 */
static void exec_str_cb(exec_str_t *data, char *buf, size_t len)
{
	if (!data->buf.ptr)
	{
		data->buf = chunk_alloc(len + 1);
		memcpy(data->buf.ptr, buf, len);
		data->buf.ptr[len] = '\0';
	}
	else
	{
		size_t newlen = strlen(data->buf.ptr) + len + 1;
		if (newlen > data->buf.len)
		{
			data->buf.ptr = realloc(data->buf.ptr, newlen);
			data->buf.len = newlen;
		}
		strncat(data->buf.ptr, buf, len);
	}

	if (data->cb)
	{
		char *nl;
		while ((nl = strchr(data->buf.ptr, '\n')) != NULL)
		{
			*nl++ = '\0';
			data->cb(data->data, data->buf.ptr);
			memmove(data->buf.ptr, nl, strlen(nl) + 1);
		}
	}
}

METHOD(guest_t, exec_str, int,
	private_guest_t *this, void(*cb)(void*,char*), bool lines, void *data,
	char *cmd, ...)
{
	int res;
	va_list args;
	va_start(args, cmd);
	if (cb)
	{
		exec_str_t exec = { chunk_empty, NULL, NULL };
		if (lines)
		{
			exec.cb = cb;
			exec.data = data;
		}
		res = vexec(this, (void(*)(void*,char*,size_t))exec_str_cb, &exec, cmd, args);
		if (exec.buf.ptr)
		{
			if (!lines || strlen(exec.buf.ptr) > 0)
			{
				/* return the complete string or the remaining stuff in the
				 * buffer (i.e. when there was no newline at the end) */
				cb(data, exec.buf.ptr);
			}
			chunk_free(&exec.buf);
		}
	}
	else
	{
		res = vexec(this, NULL, NULL, cmd, args);
	}
	va_end(args);
	return res;
}

METHOD(guest_t, sigchild, void,
	private_guest_t *this)
{
	DESTROY_IF(this->mconsole);
	this->mconsole = NULL;
	this->state = GUEST_STOPPED;
}

/**
 * umount the union filesystem
 */
static bool umount_unionfs(private_guest_t *this)
{
	if (this->cowfs)
	{
		this->cowfs->destroy(this->cowfs);
		this->cowfs = NULL;
		return TRUE;
	}
	return FALSE;
}

/**
 * mount the union filesystem
 */
static bool mount_unionfs(private_guest_t *this)
{
	char master[PATH_MAX];
	char diff[PATH_MAX];
	char mount[PATH_MAX];

	if (this->cowfs == NULL)
	{
		snprintf(master, sizeof(master), "%s/%s", this->dirname, MASTER_DIR);
		snprintf(diff, sizeof(diff), "%s/%s", this->dirname, DIFF_DIR);
		snprintf(mount, sizeof(mount), "%s/%s", this->dirname, UNION_DIR);

		this->cowfs = cowfs_create(master, diff, mount);
		if (this->cowfs)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * load args configuration from file
 */
char *loadargs(private_guest_t *this)
{
	FILE *file;
	char buf[512], *args = NULL;

	file = fdopen(openat(this->dir, ARGS_FILE, O_RDONLY, PERM), "r");
	if (file)
	{
		if (fgets(buf, sizeof(buf), file))
		{
			args = strdup(buf);
		}
		fclose(file);
	}
	return args;
}

/**
 * save args configuration to file
 */
bool saveargs(private_guest_t *this, char *args)
{
	FILE *file;
	bool retval = FALSE;

	file = fdopen(openat(this->dir, ARGS_FILE, O_RDWR | O_CREAT | O_TRUNC,
						 PERM), "w");
	if (file)
	{
		if (fprintf(file, "%s", args) > 0)
		{
			retval = TRUE;
		}
		fclose(file);
	}
	return retval;
}

METHOD(guest_t, destroy, void,
	private_guest_t *this)
{
	stop(this, NULL);
	umount_unionfs(this);
	if (this->dir > 0)
	{
		close(this->dir);
	}
	this->ifaces->destroy(this->ifaces);
	free(this->dirname);
	free(this->args);
	free(this->name);
	free(this);
}

/**
 * generic guest constructor
 */
static private_guest_t *guest_create_generic(char *parent, char *name,
											 bool create)
{
	char cwd[PATH_MAX];
	private_guest_t *this;

	INIT(this,
		.public = {
			.get_name = _get_name,
			.get_pid = _get_pid,
			.get_state = _get_state,
			.create_iface = _create_iface,
			.destroy_iface = _destroy_iface,
			.create_iface_enumerator = _create_iface_enumerator,
			.start = _start,
			.stop = _stop,
			.add_overlay = _add_overlay,
			.del_overlay = _del_overlay,
			.pop_overlay = _pop_overlay,
			.exec = _exec,
			.exec_str = _exec_str,
			.sigchild = _sigchild,
			.destroy = _destroy,
		}
	);

	if (*parent == '/' || getcwd(cwd, sizeof(cwd)) == NULL)
	{
		if (asprintf(&this->dirname, "%s/%s", parent, name) < 0)
		{
			this->dirname = NULL;
		}
	}
	else
	{
		if (asprintf(&this->dirname, "%s/%s/%s", cwd, parent, name) < 0)
		{
			this->dirname = NULL;
		}
	}
	if (this->dirname == NULL)
	{
		free(this);
		return NULL;
	}
	if (create)
	{
		mkdir(this->dirname, PERME);
	}
	this->dir = open(this->dirname, O_DIRECTORY, PERME);
	if (this->dir < 0)
	{
		DBG1(DBG_LIB, "opening guest directory '%s' failed: %m", this->dirname);
		free(this->dirname);
		free(this);
		return NULL;
	}
	this->state = GUEST_STOPPED;
	this->ifaces = linked_list_create();
	this->name = strdup(name);

	return this;
}

/**
 * create a symlink to old called new in our working dir
 */
static bool make_symlink(private_guest_t *this, char *old, char *new)
{
	char cwd[PATH_MAX];
	char buf[PATH_MAX];

	if (*old == '/' || getcwd(cwd, sizeof(cwd)) == NULL)
	{
		snprintf(buf, sizeof(buf), "%s", old);
	}
	else
	{
		snprintf(buf, sizeof(buf), "%s/%s", cwd, old);
	}
	return symlinkat(buf, this->dir, new) == 0;
}


/**
 * create the guest instance, including required dirs and mounts
 */
guest_t *guest_create(char *parent, char *name, char *kernel,
					  char *master, char *args)
{
	private_guest_t *this = guest_create_generic(parent, name, TRUE);

	if (this == NULL)
	{
		return NULL;
	}

	if (!make_symlink(this, master, MASTER_DIR) ||
		!make_symlink(this, kernel, KERNEL_FILE))
	{
		DBG1(DBG_LIB, "creating master/kernel symlink failed: %m");
		destroy(this);
		return NULL;
	}

	if (mkdirat(this->dir, UNION_DIR, PERME) != 0 ||
		mkdirat(this->dir, DIFF_DIR, PERME) != 0)
	{
		DBG1(DBG_LIB, "unable to create directories for '%s': %m", name);
		destroy(this);
		return NULL;
	}

	this->args = args;
	if (args && !saveargs(this, args))
	{
		destroy(this);
		return NULL;
	}

	if (!mount_unionfs(this))
	{
		destroy(this);
		return NULL;
	}

	return &this->public;
}

/**
 * load an already created guest
 */
guest_t *guest_load(char *parent, char *name)
{
	private_guest_t *this = guest_create_generic(parent, name, FALSE);

	if (this == NULL)
	{
		return NULL;
	}

	this->args = loadargs(this);

	if (!mount_unionfs(this))
	{
		destroy(this);
		return NULL;
	}

	return &this->public;
}

