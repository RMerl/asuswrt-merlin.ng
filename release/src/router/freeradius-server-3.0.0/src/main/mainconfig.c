/*
 * mainconf.c	Handle the server's configuration.
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
 * Copyright 2002,2006-2007  The FreeRADIUS server project
 * Copyright 2002  Alan DeKok <aland@ox.org>
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>
#include <freeradius-devel/rad_assert.h>

#include <sys/stat.h>

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#ifdef HAVE_GRP_H
#include <grp.h>
#endif

#ifdef HAVE_SYS_PRCTL_H
#include <sys/prctl.h>
#endif

#ifdef HAVE_SYSLOG_H
#	include <syslog.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

struct main_config_t mainconfig;
char *debug_condition = NULL;
extern int log_dates_utc;

typedef struct cached_config_t {
	struct cached_config_t *next;
	time_t		created;
	CONF_SECTION	*cs;
} cached_config_t;

static cached_config_t	*cs_cache = NULL;

/*
 *	Temporary local variables for parsing the configuration
 *	file.
 */
#ifdef HAVE_SETUID
/*
 *	Systems that have set/getresuid also have setuid.
 */
static uid_t server_uid = 0;
static gid_t server_gid = 0;
static char const *uid_name = NULL;
static char const *gid_name = NULL;
#endif
static char const *chroot_dir = NULL;
static int allow_core_dumps = 0;
static char const *radlog_dest = NULL;

/*
 *	These are not used anywhere else..
 */
static char const *localstatedir = NULL;
static char const *prefix = NULL;
static char const *my_name = NULL;
static char const *sbindir = NULL;
static char const *run_dir = NULL;
static char *syslog_facility = NULL;
static int do_colourise = false;


/*
 *  Security configuration for the server.
 */
static const CONF_PARSER security_config[] = {
	{ "max_attributes",  PW_TYPE_INTEGER, 0, &fr_max_attributes, STRINGIFY(0) },
	{ "reject_delay",  PW_TYPE_INTEGER, 0, &mainconfig.reject_delay, STRINGIFY(0) },
	{ "status_server", PW_TYPE_BOOLEAN, 0, &mainconfig.status_server, "no"},
	{ NULL, -1, 0, NULL, NULL }
};


/*
 *	Logging configuration for the server.
 */
static const CONF_PARSER logdest_config[] = {
	{ "destination",  PW_TYPE_STRING_PTR, 0, &radlog_dest, "files" },
	{ "syslog_facility",  PW_TYPE_STRING_PTR, 0, &syslog_facility, STRINGIFY(0) },

	{ "file", PW_TYPE_STRING_PTR, 0, &mainconfig.log_file, "${logdir}/radius.log" },
	{ "requests", PW_TYPE_STRING_PTR, 0, &default_log.file, NULL },
	{ NULL, -1, 0, NULL, NULL }
};


static const CONF_PARSER serverdest_config[] = {
	{ "log", PW_TYPE_SUBSECTION, 0, NULL, (void const *) logdest_config },
	{ "log_file", PW_TYPE_STRING_PTR, 0, &mainconfig.log_file, NULL },
	{ "log_destination", PW_TYPE_STRING_PTR, 0, &radlog_dest, NULL },
	{ "use_utc", PW_TYPE_BOOLEAN, 0, &log_dates_utc, NULL },
	{ NULL, -1, 0, NULL, NULL }
};


static const CONF_PARSER log_config_nodest[] = {
	{ "stripped_names", PW_TYPE_BOOLEAN, 0, &log_stripped_names,"no" },
	{ "auth", PW_TYPE_BOOLEAN, 0, &mainconfig.log_auth, "no" },
	{ "auth_badpass", PW_TYPE_BOOLEAN, 0, &mainconfig.log_auth_badpass, "no" },
	{ "auth_goodpass", PW_TYPE_BOOLEAN, 0, &mainconfig.log_auth_goodpass, "no" },
	{ "msg_badpass", PW_TYPE_STRING_PTR, 0, &mainconfig.auth_badpass_msg, NULL},
	{ "msg_goodpass", PW_TYPE_STRING_PTR, 0, &mainconfig.auth_goodpass_msg, NULL},
	{ "colourise", PW_TYPE_BOOLEAN, 0, &do_colourise, NULL },
	{ "use_utc", PW_TYPE_BOOLEAN, 0, &log_dates_utc, NULL },

	{ NULL, -1, 0, NULL, NULL }
};


/*
 *  A mapping of configuration file names to internal variables
 */
static const CONF_PARSER server_config[] = {
	/*
	 *	FIXME: 'prefix' is the ONLY one which should be
	 *	configured at compile time.  Hard-coding it here is
	 *	bad.  It will be cleaned up once we clean up the
	 *	hard-coded defines for the locations of the various
	 *	files.
	 */
	{ "name",	       PW_TYPE_STRING_PTR, 0, &my_name,	  "radiusd"},
	{ "prefix",	     PW_TYPE_STRING_PTR, 0, &prefix,	    "/usr/local"},
	{ "localstatedir",      PW_TYPE_STRING_PTR, 0, &localstatedir,     "${prefix}/var"},
	{ "sbindir",	    PW_TYPE_STRING_PTR, 0, &sbindir,	    "${prefix}/sbin"},
	{ "logdir",	     PW_TYPE_STRING_PTR, 0, &radlog_dir,	"${localstatedir}/log"},
	{ "run_dir",	    PW_TYPE_STRING_PTR, 0, &run_dir,	   "${localstatedir}/run/${name}"},
	{ "libdir",	     PW_TYPE_STRING_PTR, 0, &radlib_dir,	"${prefix}/lib"},
	{ "radacctdir",	 PW_TYPE_STRING_PTR, 0, &radacct_dir,       "${logdir}/radacct" },
	{ "hostname_lookups",   PW_TYPE_BOOLEAN,    0, &fr_dns_lookups,      "no" },
	{ "max_request_time", PW_TYPE_INTEGER, 0, &mainconfig.max_request_time, STRINGIFY(MAX_REQUEST_TIME) },
	{ "cleanup_delay", PW_TYPE_INTEGER, 0, &mainconfig.cleanup_delay, STRINGIFY(CLEANUP_DELAY) },
	{ "max_requests", PW_TYPE_INTEGER, 0, &mainconfig.max_requests, STRINGIFY(MAX_REQUESTS) },
#ifdef DELETE_BLOCKED_REQUESTS
	{ "delete_blocked_requests", PW_TYPE_INTEGER, 0, &mainconfig.kill_unresponsive_children, STRINGIFY(false) },
#endif
	{ "pidfile", PW_TYPE_STRING_PTR, 0, &mainconfig.pid_file, "${run_dir}/radiusd.pid"},
	{ "checkrad", PW_TYPE_STRING_PTR, 0, &mainconfig.checkrad, "${sbindir}/checkrad" },

	{ "debug_level", PW_TYPE_INTEGER, 0, &mainconfig.debug_level, "0"},

#ifdef WITH_PROXY
	{ "proxy_requests", PW_TYPE_BOOLEAN, 0, &mainconfig.proxy_requests, "yes" },
#endif
	{ "log", PW_TYPE_SUBSECTION, 0, NULL, (void const *) log_config_nodest },

	/*
	 *	People with old configs will have these.  They are listed
	 *	AFTER the "log" section, so if they exist in radiusd.conf,
	 *	it will prefer "log_foo = bar" to "log { foo = bar }".
	 *	They're listed with default values of NULL, so that if they
	 *	DON'T exist in radiusd.conf, then the previously parsed
	 *	values for "log { foo = bar}" will be used.
	 */
	{ "log_auth", PW_TYPE_BOOLEAN | PW_TYPE_DEPRECATED, 0, &mainconfig.log_auth, NULL },
	{ "log_auth_badpass", PW_TYPE_BOOLEAN | PW_TYPE_DEPRECATED, 0, &mainconfig.log_auth_badpass, NULL },
	{ "log_auth_goodpass", PW_TYPE_BOOLEAN | PW_TYPE_DEPRECATED, 0, &mainconfig.log_auth_goodpass, NULL },
	{ "log_stripped_names", PW_TYPE_BOOLEAN | PW_TYPE_DEPRECATED, 0, &log_stripped_names, NULL },

	{  "security", PW_TYPE_SUBSECTION, 0, NULL, (void const *) security_config },

	{ NULL, -1, 0, NULL, NULL }
};

static const CONF_PARSER bootstrap_security_config[] = {
#ifdef HAVE_SETUID
	{ "user",  PW_TYPE_STRING_PTR, 0, &uid_name, NULL },
	{ "group",  PW_TYPE_STRING_PTR, 0, &gid_name, NULL },
#endif
	{ "chroot",  PW_TYPE_STRING_PTR, 0, &chroot_dir, NULL },
	{ "allow_core_dumps", PW_TYPE_BOOLEAN, 0, &allow_core_dumps, "no" },

	{ NULL, -1, 0, NULL, NULL }
};

static const CONF_PARSER bootstrap_config[] = {
	{  "security", PW_TYPE_SUBSECTION, 0, NULL, (void const *) bootstrap_security_config },

	/*
	 *	For backwards compatibility.
	 */
#ifdef HAVE_SETUID
	{ "user",  PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, 0, &uid_name, NULL },
	{ "group",  PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, 0, &gid_name, NULL },
#endif
	{ "chroot",  PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, 0, &chroot_dir, NULL },
	{ "allow_core_dumps", PW_TYPE_BOOLEAN | PW_TYPE_DEPRECATED, 0, &allow_core_dumps, NULL },

	{ NULL, -1, 0, NULL, NULL }
};



#define MAX_ARGV (256)


static size_t config_escape_func(UNUSED REQUEST *request, char *out, size_t outlen, char const *in, UNUSED void *arg)
{
	size_t len = 0;
	static char const *disallowed = "%{}\\'\"`";

	while (in[0]) {
		/*
		 *	Non-printable characters get replaced with their
		 *	mime-encoded equivalents.
		 */
		if ((in[0] < 32)) {
			if (outlen <= 3) break;

			snprintf(out, outlen, "=%02X", (unsigned char) in[0]);
			in++;
			out += 3;
			outlen -= 3;
			len += 3;
			continue;

		} else if (strchr(disallowed, *in) != NULL) {
			if (outlen <= 2) break;

			out[0] = '\\';
			out[1] = *in;
			in++;
			out += 2;
			outlen -= 2;
			len += 2;
			continue;
		}

		/*
		 *	Only one byte left.
		 */
		if (outlen <= 1) {
			break;
		}

		/*
		 *	Allowed character.
		 */
		*out = *in;
		out++;
		in++;
		outlen--;
		len++;
	}
	*out = '\0';
	return len;
}

/*
 *	Xlat for %{config:section.subsection.attribute}
 */
static ssize_t xlat_config(UNUSED void *instance, REQUEST *request, char const *fmt, char *out, size_t outlen)
{
	char const *value;
	CONF_PAIR *cp;
	CONF_ITEM *ci;
	char buffer[1024];

	/*
	 *	Expand it safely.
	 */
	if (radius_xlat(buffer, sizeof(buffer), request, fmt, config_escape_func, NULL) < 0) {
		return 0;
	}

	ci = cf_reference_item(request->root->config,
			       request->root->config, buffer);
	if (!ci || !cf_item_is_pair(ci)) {
		REDEBUG("Config item \"%s\" does not exist", fmt);
		*out = '\0';
		return -1;
	}

	cp = cf_itemtopair(ci);

	/*
	 *  Ensure that we only copy what's necessary.
	 *
	 *  If 'outlen' is too small, then the output is chopped to fit.
	 */
	value = cf_pair_value(cp);
	if (!value) {
		out[0] = '\0';
		return 0;
	}

	if (outlen > strlen(value)) {
		outlen = strlen(value) + 1;
	}

	strlcpy(out, value, outlen);

	return strlen(out);
}


/*
 *	Xlat for %{client:foo}
 */
static ssize_t xlat_client(UNUSED void *instance, REQUEST *request, char const *fmt, char *out, size_t outlen)
{
	char const *value = NULL;
	CONF_PAIR *cp;

	if (!fmt || !out || (outlen < 1)) return 0;

	if (!request || !request->client) {
		RWDEBUG("No client associated with this request");
		*out = '\0';
		return 0;
	}

	cp = cf_pair_find(request->client->cs, fmt);
	if (!cp || !(value = cf_pair_value(cp))) {
		if (strcmp(fmt, "shortname") == 0) {
			strlcpy(out, request->client->shortname, outlen);
			return strlen(out);
		}
		RDEBUG("Client does not contain config item \"%s\"", fmt);
		*out = '\0';
		return 0;
	}

	strlcpy(out, value, outlen);

	return strlen(out);
}


#ifdef HAVE_SYS_RESOURCE_H
static struct rlimit core_limits;
#endif

static void fr_set_dumpable(void)
{
	/*
	 *	If configured, turn core dumps off.
	 */
	if (!allow_core_dumps) {
#ifdef HAVE_SYS_RESOURCE_H
		struct rlimit no_core;


		no_core.rlim_cur = 0;
		no_core.rlim_max = 0;

		if (setrlimit(RLIMIT_CORE, &no_core) < 0) {
			ERROR("Failed disabling core dumps: %s",
			       strerror(errno));
		}
#endif
		return;
	}

	/*
	 *	Set or re-set the dumpable flag.
	 */
#ifdef HAVE_SYS_PRCTL_H
#ifdef PR_SET_DUMPABLE
	if (prctl(PR_SET_DUMPABLE, 1) < 0) {
		ERROR("Cannot re-enable core dumps: prctl(PR_SET_DUMPABLE) failed: '%s'",
		       strerror(errno));
	}
#endif
#endif

	/*
	 *	Reset the core dump limits to their original value.
	 */
#ifdef HAVE_SYS_RESOURCE_H
	if (setrlimit(RLIMIT_CORE, &core_limits) < 0) {
		ERROR("Cannot update core dump limit: %s",
		       strerror(errno));
	}
#endif
}

#ifdef HAVE_SETUID
static int doing_setuid = false;

#if defined(HAVE_SETRESUID) && defined (HAVE_GETRESUID)
void fr_suid_up(void)
{
	uid_t ruid, euid, suid;

	if (getresuid(&ruid, &euid, &suid) < 0) {
		ERROR("Failed getting saved UID's");
		fr_exit_now(1);
	}

	if (setresuid(-1, suid, -1) < 0) {
		ERROR("Failed switching to privileged user");
		fr_exit_now(1);
	}

	if (geteuid() != suid) {
		ERROR("Switched to unknown UID");
		fr_exit_now(1);
	}
}

void fr_suid_down(void)
{
	if (!doing_setuid) return;

	if (setresuid(-1, server_uid, geteuid()) < 0) {
		fprintf(stderr, "%s: Failed switching to uid %s: %s\n",
			progname, uid_name, strerror(errno));
		fr_exit_now(1);
	}

	if (geteuid() != server_uid) {
		fprintf(stderr, "%s: Failed switching uid: UID is incorrect\n",
			progname);
		fr_exit_now(1);
	}

	fr_set_dumpable();
}

void fr_suid_down_permanent(void)
{
	if (!doing_setuid) return;

	if (setresuid(server_uid, server_uid, server_uid) < 0) {
		ERROR("Failed in permanent switch to uid %s: %s",
		       uid_name, strerror(errno));
		fr_exit_now(1);
	}

	if (geteuid() != server_uid) {
		ERROR("Switched to unknown uid");
		fr_exit_now(1);
	}

	fr_set_dumpable();
}
#else
/*
 *	Much less secure...
 */
void fr_suid_up(void)
{
}
void fr_suid_down(void)
{
	if (!uid_name) return;

	if (setuid(server_uid) < 0) {
		fprintf(stderr, "%s: Failed switching to uid %s: %s\n",
			progname, uid_name, strerror(errno));
		fr_exit(1);
	}

	fr_set_dumpable();
}
void fr_suid_down_permanent(void)
{
	fr_set_dumpable();
}
#endif /* HAVE_SETRESUID && HAVE_GETRESUID */
#else  /* HAVE_SETUID */
void fr_suid_up(void)
{
}
void fr_suid_down(void)
{
	fr_set_dumpable();
}
void fr_suid_down_permanent(void)
{
	fr_set_dumpable();
}
#endif /* HAVE_SETUID */

#ifdef HAVE_SETUID

/*
 *  Do chroot, if requested.
 *
 *  Switch UID and GID to what is specified in the config file
 */
static int switch_users(CONF_SECTION *cs)
{
#ifdef HAVE_SYS_RESOURCE_H
	/*
	 *	Get the current maximum for core files.  Do this
	 *	before anything else so as to ensure it's properly
	 *	initialized.
	 */
	if (getrlimit(RLIMIT_CORE, &core_limits) < 0) {
		ERROR("Failed to get current core limit:  %s", strerror(errno));
		return 0;
	}
#endif

	/*
	 *	Don't do chroot/setuid/setgid if we're in debugging
	 *	as non-root.
	 */
	if (debug_flag && (getuid() != 0)) return 1;

	if (cf_section_parse(cs, NULL, bootstrap_config) < 0) {
		fprintf(stderr, "radiusd: Error: Failed to parse user/group information.\n");
		return 0;
	}


#ifdef HAVE_GRP_H
	/*  Set GID.  */
	if (gid_name) {
		struct group *gr;

		gr = getgrnam(gid_name);
		if (gr == NULL) {
			fprintf(stderr, "%s: Cannot get ID for group %s: %s\n",
				progname, gid_name, strerror(errno));
			return 0;
		}
		server_gid = gr->gr_gid;
	} else {
		server_gid = getgid();
	}
#endif

#ifdef HAVE_PWD_H
	/*  Set UID.  */
	if (uid_name) {
		struct passwd *pw;

		pw = getpwnam(uid_name);
		if (pw == NULL) {
			fprintf(stderr, "%s: Cannot get passwd entry for user %s: %s\n",
				progname, uid_name, strerror(errno));
			return 0;
		}

		if (getuid() == pw->pw_uid) {
			uid_name = NULL;
		} else {

			server_uid = pw->pw_uid;
#ifdef HAVE_INITGROUPS
			if (initgroups(uid_name, server_gid) < 0) {
				fprintf(stderr, "%s: Cannot initialize supplementary group list for user %s: %s\n",
					progname, uid_name, strerror(errno));
				return 0;
			}
#endif
		}
	} else {
		server_uid = getuid();
	}
#endif

	if (chroot_dir) {
		if (chroot(chroot_dir) < 0) {
			fprintf(stderr, "%s: Failed to perform chroot %s: %s",
				progname, chroot_dir, strerror(errno));
			return 0;
		}

		/*
		 *	Note that we leave chdir alone.  It may be
		 *	OUTSIDE of the root.  This allows us to read
		 *	the configuration from "-d ./etc/raddb", with
		 *	the chroot as "./chroot/" for example.  After
		 *	the server has been loaded, it does a "cd
		 *	${logdir}" below, so that core files (if any)
		 *	go to a logging directory.
		 *
		 *	This also allows the configuration of the
		 *	server to be outside of the chroot.  If the
		 *	server is statically linked, then the only
		 *	things needed inside of the chroot are the
		 *	logging directories.
		 */
	}

#ifdef HAVE_GRP_H
	/*  Set GID.  */
	if (gid_name && (setgid(server_gid) < 0)) {
		fprintf(stderr, "%s: Failed setting group to %s: %s",
			progname, gid_name, strerror(errno));
		return 0;
	}
#endif

#ifdef HAVE_SETUID
	/*
	 *	Just before losing root permissions, ensure that the
	 *	log files have the correct owner && group.
	 *
	 *	We have to do this because the log file MAY have been
	 *	specified on the command-line.
	 */
	if (uid_name || gid_name) {
		if ((default_log.dest == L_DST_FILES) &&
		    (default_log.fd < 0)) {
			default_log.fd = open(mainconfig.log_file,
					      O_WRONLY | O_APPEND | O_CREAT, 0640);
			if (default_log.fd < 0) {
				fprintf(stderr, "radiusd: Failed to open log file %s: %s\n", mainconfig.log_file, strerror(errno));
				return 0;
			}

			if (chown(mainconfig.log_file, server_uid, server_gid) < 0) {
				fprintf(stderr, "%s: Cannot change ownership of log file %s: %s\n",
					progname, mainconfig.log_file, strerror(errno));
				return 0;
			}
		}
	}

	if (uid_name) {
		doing_setuid = true;

		fr_suid_down();
	}
#endif

	/*
	 *	This also clears the dumpable flag if core dumps
	 *	aren't allowed.
	 */
	fr_set_dumpable();

	if (allow_core_dumps) {
		INFO("Core dumps are enabled.");
	}

	return 1;
}
#endif	/* HAVE_SETUID */


/*
 *	Read config files.
 *
 *	This function can ONLY be called from the main server process.
 */
int read_mainconfig(int reload)
{
	char const *p = NULL;
	CONF_SECTION *cs;
	struct stat statbuf;
	cached_config_t *cc;
	char buffer[1024];

	if (reload != 0) {
		ERROR("Reload is not implemented");
		return -1;
	}

	if (stat(radius_dir, &statbuf) < 0) {
		ERROR("Errors reading %s: %s",
		       radius_dir, strerror(errno));
		return -1;
	}

#ifdef S_IWOTH
	if ((statbuf.st_mode & S_IWOTH) != 0) {
		ERROR("Configuration directory %s is globally writable.  Refusing to start due to insecure configuration.",
		       radius_dir);
	  return -1;
	}
#endif

#ifdef S_IROTH
	if (0 && (statbuf.st_mode & S_IROTH) != 0) {
		ERROR("Configuration directory %s is globally readable.  Refusing to start due to insecure configuration.",
		       radius_dir);
		return -1;
	}
#endif
	INFO("Starting - reading configuration files ...");

	/* Initialize the dictionary */
	if (!mainconfig.dictionary_dir) mainconfig.dictionary_dir = radius_dir;
	DEBUG2("including dictionary file %s/%s", mainconfig.dictionary_dir, RADIUS_DICTIONARY);
	if (dict_init(mainconfig.dictionary_dir, RADIUS_DICTIONARY) != 0) {
		ERROR("Errors reading dictionary: %s",
				fr_strerror());
		return -1;
	}

	/* Read the configuration file */
	snprintf(buffer, sizeof(buffer), "%.200s/%.50s.conf",
		 radius_dir, mainconfig.name);
	if ((cs = cf_file_read(buffer)) == NULL) {
		ERROR("Errors reading or parsing %s", buffer);
		return -1;
	}

	/*
	 *	If there was no log destination set on the command line,
	 *	set it now.
	 */
	if (default_log.dest == L_DST_NULL) {
		if (cf_section_parse(cs, NULL, serverdest_config) < 0) {
			fprintf(stderr, "radiusd: Error: Failed to parse log{} section.\n");
			cf_file_free(cs);
			return -1;
		}

		if (!radlog_dest) {
			fprintf(stderr, "radiusd: Error: No log destination specified.\n");
			cf_file_free(cs);
			return -1;
		}

		default_log.dest = fr_str2int(log_str2dst, radlog_dest,
					      L_DST_NUM_DEST);
		if (default_log.dest == L_DST_NUM_DEST) {
			fprintf(stderr, "radiusd: Error: Unknown log_destination %s\n",
				radlog_dest);
			cf_file_free(cs);
			return -1;
		}

		if (default_log.dest == L_DST_SYSLOG) {
			/*
			 *	Make sure syslog_facility isn't NULL
			 *	before using it
			 */
			if (!syslog_facility) {
				fprintf(stderr, "radiusd: Error: Syslog chosen but no facility was specified\n");
				cf_file_free(cs);
				return -1;
			}
			mainconfig.syslog_facility = fr_str2int(syslog_str2fac, syslog_facility, -1);
			if (mainconfig.syslog_facility < 0) {
				fprintf(stderr, "radiusd: Error: Unknown syslog_facility %s\n",
					syslog_facility);
				cf_file_free(cs);
				return -1;
			}

#ifdef HAVE_SYSLOG_H
			/*
			 *	Call openlog only once, when the
			 *	program starts.
			 */
			openlog(progname, LOG_PID, mainconfig.syslog_facility);
#endif

		} else if (default_log.dest == L_DST_FILES) {
			if (!mainconfig.log_file) {
				fprintf(stderr, "radiusd: Error: Specified \"files\" as a log destination, but no log filename was given!\n");
				cf_file_free(cs);
				return -1;
			}
		}
	}

#ifdef HAVE_SETUID
	/*
	 *	Switch users as early as possible.
	 */
	if (!switch_users(cs)) fr_exit(1);
#endif

	/*
	 *	Open the log file AFTER switching uid / gid.  If we
	 *	did switch uid/gid, then the code in switch_users()
	 *	took care of setting the file permissions correctly.
	 */
	if ((default_log.dest == L_DST_FILES) &&
	    (default_log.fd < 0)) {
		default_log.fd = open(mainconfig.log_file,
					    O_WRONLY | O_APPEND | O_CREAT, 0640);
		if (default_log.fd < 0) {
			fprintf(stderr, "radiusd: Failed to open log file %s: %s\n", mainconfig.log_file, strerror(errno));
			cf_file_free(cs);
			return -1;
		}
	}

	/*
	 *	This allows us to figure out where, relative to
	 *	radiusd.conf, the other configuration files exist.
	 */
	if (cf_section_parse(cs, NULL, server_config) < 0) {
		return -1;
	}

	/*
	 *	We ignore colourization of output until after the
	 *	configuration files have been parsed.
	 */
	if (do_colourise) {
		p = getenv("TERM");
		if (!p || !isatty(default_log.fd) ||
		    (strstr(p, "xterm") == 0)) {
			mainconfig.colourise = false;
		} else {
			mainconfig.colourise = true;
		}
		p = NULL;
	}

	if (mainconfig.max_request_time == 0) mainconfig.max_request_time = 100;
	if (mainconfig.reject_delay > 5) mainconfig.reject_delay = 5;
	if (mainconfig.cleanup_delay > 5) mainconfig.cleanup_delay =5;

	/*
	 *	Free the old configuration items, and replace them
	 *	with the new ones.
	 *
	 *	Note that where possible, we do atomic switch-overs,
	 *	to ensure that the pointers are always valid.
	 */
	rad_assert(mainconfig.config == NULL);
	root_config = mainconfig.config = cs;

	DEBUG2("%s: #### Loading Realms and Home Servers ####", mainconfig.name);
	if (!realms_init(cs)) {
		return -1;
	}

	DEBUG2("%s: #### Loading Clients ####", mainconfig.name);
	if (!clients_parse_section(cs)) {
		return -1;
	}

	/*
	 *  Register the %{config:section.subsection} xlat function.
	 */
	xlat_register("config", xlat_config, NULL, NULL);
	xlat_register("client", xlat_client, NULL, NULL);

	/*
	 *	Starting the server, WITHOUT "-x" on the
	 *	command-line: use whatever is in the config
	 *	file.
	 */
	if (debug_flag == 0) {
		debug_flag = mainconfig.debug_level;
	}
	fr_debug_flag = debug_flag;

	/*
	 *  Go update our behaviour, based on the configuration
	 *  changes.
	 */

	/*
	 *	Sanity check the configuration for internal
	 *	consistency.
	 */
	if (mainconfig.reject_delay > mainconfig.cleanup_delay) {
		mainconfig.reject_delay = mainconfig.cleanup_delay;
	}
	if (mainconfig.reject_delay < 0) mainconfig.reject_delay = 0;

	/*  Reload the modules.  */
	if (setup_modules(reload, mainconfig.config) < 0) {
		return -1;
	}

	if (chroot_dir) {
		if (chdir(radlog_dir) < 0) {
			ERROR("Failed to 'chdir %s' after chroot: %s",
			       radlog_dir, strerror(errno));
			return -1;
		}
	}

	cc = rad_malloc(sizeof(*cc));
	memset(cc, 0, sizeof(*cc));

	cc->cs = cs;
	rad_assert(cs_cache == NULL);
	cs_cache = cc;

	return 0;
}

/*
 *	Free the configuration.  Called only when the server is exiting.
 */
int free_mainconfig(void)
{
	cached_config_t *cc, *next;

	virtual_servers_free(0);

	/*
	 *	Clean up the configuration data
	 *	structures.
	 */
	clients_free(NULL);
	realms_free();
	listen_free(&mainconfig.listen);

	/*
	 *	Free all of the cached configurations.
	 */
	for (cc = cs_cache; cc != NULL; cc = next) {
		next = cc->next;
		cf_file_free(cc->cs);
		free(cc);
	}

	dict_free();

	return 0;
}

void hup_logfile(void)
{
		int fd, old_fd;

		if (default_log.dest != L_DST_FILES) return;

 		fd = open(mainconfig.log_file,
			  O_WRONLY | O_APPEND | O_CREAT, 0640);
		if (fd >= 0) {
			/*
			 *	Atomic swap. We'd like to keep the old
			 *	FD around so that callers don't
			 *	suddenly find the FD closed, and the
			 *	writes go nowhere.  But that's hard to
			 *	do.  So... we have the case where a
			 *	log message *might* be lost on HUP.
			 */
			old_fd = default_log.fd;
			default_log.fd = fd;
			close(old_fd);
		}
}

void hup_mainconfig(void)
{
	cached_config_t *cc;
	CONF_SECTION *cs;
	char buffer[1024];

	INFO("HUP - Re-reading configuration files");

	/* Read the configuration file */
	snprintf(buffer, sizeof(buffer), "%.200s/%.50s.conf",
		 radius_dir, mainconfig.name);
	if ((cs = cf_file_read(buffer)) == NULL) {
		ERROR("Failed to re-read or parse %s", buffer);
		return;
	}

	cc = rad_malloc(sizeof(*cc));
	memset(cc, 0, sizeof(*cc));

	/*
	 *	Save the current configuration.  Note that we do NOT
	 *	free older ones.  We should probably do so at some
	 *	point.  Doing so will require us to mark which modules
	 *	are still in use, and which aren't.  Modules that
	 *	can't be HUPed always use the original configuration.
	 *	Modules that can be HUPed use one of the newer
	 *	configurations.
	 */
	cc->created = time(NULL);
	cc->cs = cs;
	cc->next = cs_cache;
	cs_cache = cc;

	/*
	 *	Re-open the log file.  If we can't, then keep logging
	 *	to the old log file.
	 *
	 *	The "open log file" code is here rather than in log.c,
	 *	because it makes that function MUCH simpler.
	 */
	hup_logfile();

	INFO("HUP - loading modules");

	/*
	 *	Prefer the new module configuration.
	 */
	module_hup(cf_section_sub_find(cs, "modules"));

	/*
	 *	Load new servers BEFORE freeing old ones.
	 */
	virtual_servers_load(cs);

	virtual_servers_free(cc->created - mainconfig.max_request_time * 4);
}
