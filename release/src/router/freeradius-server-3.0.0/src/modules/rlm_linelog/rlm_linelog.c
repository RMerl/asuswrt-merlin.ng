/*
 * rlm_linelog.c
 *
 * Version:	$Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2004,2006  The FreeRADIUS server project
 * Copyright 2004  Alan DeKok <aland@freeradius.org>
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>
#include <freeradius-devel/rad_assert.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_GRP_H
#include <grp.h>
#endif

#ifdef HAVE_SYSLOG_H
#include <syslog.h>

#ifndef LOG_INFO
#define LOG_INFO (0)
#endif
#endif

/*
 *	Define a structure for our module configuration.
 */
typedef struct rlm_linelog_t {
	CONF_SECTION	*cs;
	char		*filename;
	char		*syslog_facility;
	int		facility;
	int		permissions;
	char		*group;
	char		*line;
	char		*reference;
} rlm_linelog_t;

/*
 *	A mapping of configuration file names to internal variables.
 *
 *	Note that the string is dynamically allocated, so it MUST
 *	be freed.  When the configuration file parse re-reads the string,
 *	it free's the old one, and strdup's the new one, placing the pointer
 *	to the strdup'd string into 'config.string'.  This gets around
 *	buffer over-flows.
 */
static const CONF_PARSER module_config[] = {
	{ "filename",  PW_TYPE_FILE_OUTPUT| PW_TYPE_REQUIRED,
	  offsetof(rlm_linelog_t,filename), NULL,  NULL},
	{ "syslog_facility",  PW_TYPE_STRING_PTR,
	  offsetof(rlm_linelog_t,syslog_facility), NULL,  NULL},
	{ "permissions",  PW_TYPE_INTEGER,
	  offsetof(rlm_linelog_t,permissions), NULL,  "0600"},
	{ "group",  PW_TYPE_STRING_PTR,
	  offsetof(rlm_linelog_t,group), NULL,  NULL},
	{ "format",  PW_TYPE_STRING_PTR,
	  offsetof(rlm_linelog_t,line), NULL,  NULL},
	{ "reference",  PW_TYPE_STRING_PTR,
	  offsetof(rlm_linelog_t,reference), NULL,  NULL},
	{ NULL, -1, 0, NULL, NULL }		/* end the list */
};


/*
 *	Instantiate the module.
 */
static int mod_instantiate(CONF_SECTION *conf, void *instance)
{
	rlm_linelog_t *inst = instance;

	rad_assert(inst->filename && *inst->filename);

#ifndef HAVE_SYSLOG_H
	if (strcmp(inst->filename, "syslog") == 0) {
		cf_log_err_cs(conf, "Syslog output is not supported on this system");
		return -1;
	}
#else
	inst->facility = 0;

	if (inst->syslog_facility) {
		inst->facility = fr_str2int(syslog_str2fac, inst->syslog_facility, -1);
		if (inst->facility < 0) {
			cf_log_err_cs(conf, "Invalid syslog facility '%s'",
				   inst->syslog_facility);
			return -1;
		}
	}

	inst->facility |= LOG_INFO;
#endif

	if (!inst->line) {
		cf_log_err_cs(conf, "Must specify a log format");
		return -1;
	}

	inst->cs = conf;
	return 0;
}


/*
 *	Escape unprintable characters.
 */
static size_t linelog_escape_func(UNUSED REQUEST *request,
		char *out, size_t outlen, char const *in,
		UNUSED void *arg)
{
	int len = 0;

	if (outlen == 0) return 0;
	if (outlen == 1) {
		*out = '\0';
		return 0;
	}

	while (in[0]) {
		if (in[0] >= ' ') {
			if (in[0] == '\\') {
				if (outlen <= 2) break;
				outlen--;
				*out++ = '\\';
				len++;
			}

			outlen--;
			if (outlen == 1) break;
			*out++ = *in++;
			len++;
			continue;
		}

		switch (in[0]) {
		case '\n':
			if (outlen <= 2) break;
			*out++ = '\\';
			*out++ = 'n';
			in++;
			len += 2;
			break;

		case '\r':
			if (outlen <= 2) break;
			*out++ = '\\';
			*out++ = 'r';
			in++;
			len += 2;
			break;

		default:
			if (outlen <= 4) break;
			snprintf(out, outlen,  "\\%03o", *in);
			in++;
			out += 4;
			outlen -= 4;
			len += 4;
			break;
		}
	}

	*out = '\0';
	return len;
}

static rlm_rcode_t do_linelog(void *instance, REQUEST *request)
{
	int fd = -1;
	char buffer[4096];
	char *p;
	char line[1024];
	rlm_linelog_t *inst = (rlm_linelog_t*) instance;
	char const *value = inst->line;

#ifdef HAVE_GRP_H
	gid_t gid;
	struct group *grp;
	char *endptr;
#endif

	if (inst->reference) {
		CONF_ITEM *ci;
		CONF_PAIR *cp;

		p = line + 1;

		if (radius_xlat(p, sizeof(line) - 2, request, inst->reference, linelog_escape_func,
		    NULL) < 0) {
			return RLM_MODULE_FAIL;
		}

		line[0] = '.';	/* force to be in current section */

		/*
		 *	Don't allow it to go back up
		 */
		if (line[1] == '.') goto do_log;

		ci = cf_reference_item(NULL, inst->cs, line);
		if (!ci) {
			RDEBUG2("No such entry \"%s\"", line);
			return RLM_MODULE_NOOP;
		}

		if (!cf_item_is_pair(ci)) {
			RDEBUG2("Entry \"%s\" is not a variable assignment ", line);
			goto do_log;
		}

		cp = cf_itemtopair(ci);
		value = cf_pair_value(cp);
		if (!value) {
			RDEBUG2("Entry \"%s\" has no value", line);
			goto do_log;
		}

		/*
		 *	Value exists, but is empty.  Don't log anything.
		 */
		if (!*value) return RLM_MODULE_OK;
	}

 do_log:
	/*
	 *	FIXME: Check length.
	 */
	if (strcmp(inst->filename, "syslog") != 0) {
		if (radius_xlat(buffer, sizeof(buffer), request, inst->filename, NULL, NULL) < 0) {
			return RLM_MODULE_FAIL;
		}

		/* check path and eventually create subdirs */
		p = strrchr(buffer,'/');
		if (p) {
			*p = '\0';
			if (rad_mkdir(buffer, 0700) < 0) {
				RERROR("rlm_linelog: Failed to create directory %s: %s", buffer, strerror(errno));
				return RLM_MODULE_FAIL;
			}
			*p = '/';
		}

		fd = open(buffer, O_WRONLY | O_APPEND | O_CREAT, inst->permissions);
		if (fd == -1) {
			ERROR("rlm_linelog: Failed to open %s: %s",
			       buffer, strerror(errno));
			return RLM_MODULE_FAIL;
		}

#ifdef HAVE_GRP_H
		if (inst->group != NULL) {
			gid = strtol(inst->group, &endptr, 10);
			if (*endptr != '\0') {
				grp = getgrnam(inst->group);
				if (!grp) {
					RDEBUG2("Unable to find system group \"%s\"", inst->group);
					goto skip_group;
				}
				gid = grp->gr_gid;
			}

			if (chown(buffer, -1, gid) == -1) {
				RDEBUG2("Unable to change system group of \"%s\"", buffer);
			}
		}
#endif
	}

 skip_group:

	/*
	 *	FIXME: Check length.
	 */
	if (radius_xlat(line, sizeof(line) - 1, request, value, linelog_escape_func, NULL) < 0) {
		if (fd > -1) {
			close(fd);
		}

		return RLM_MODULE_FAIL;
	}

	if (fd >= 0) {
		strcat(line, "\n");

		if (write(fd, line, strlen(line)) < 0) {
			EDEBUG("rlm_linelog: Failed writing: %s", strerror(errno));
			close(fd);
			return RLM_MODULE_FAIL;
		}

		close(fd);

#ifdef HAVE_SYSLOG_H
	} else {
		syslog(inst->facility, "%s", line);
#endif
	}

	return RLM_MODULE_OK;
}


/*
 *	Externally visible module definition.
 */
module_t rlm_linelog = {
	RLM_MODULE_INIT,
	"linelog",
	RLM_TYPE_CHECK_CONFIG_SAFE,   	/* type */
	sizeof(rlm_linelog_t),
	module_config,
	mod_instantiate,		/* instantiation */
	NULL,				/* detach */
	{
		do_linelog,	/* authentication */
		do_linelog,	/* authorization */
		do_linelog,	/* preaccounting */
		do_linelog,	/* accounting */
		NULL,		/* checksimul */
		do_linelog, 	/* pre-proxy */
		do_linelog,	/* post-proxy */
		do_linelog	/* post-auth */
#ifdef WITH_COA
		, do_linelog,	/* recv-coa */
		do_linelog	/* send-coa */
#endif
	},
};
