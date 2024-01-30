/* Inadyn is a small and simple dynamic DNS (DDNS) client
 *
 * Copyright (C) 2003-2004  Narcis Ilisei <inarcis2002@hotpop.com>
 * Copyright (C) 2010-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, visit the Free Software Foundation
 * website at http://www.gnu.org/licenses/gpl-2.0.html or write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include <getopt.h>
#include <stdlib.h>
#include <pwd.h>		/* getpwnam() */
#include <grp.h>		/* getgrnam() */
#include <unistd.h>
#include <confuse.h>
#include <sys/stat.h>		/* mkdir() */


#include "log.h"
#include "ddns.h"
#include "error.h"
#include "ssl.h"

int    once = 0;
int    force = 0;		/* Only allowed with 'once' */
int    ignore_errors = 0;
int    startup_delay = DDNS_DEFAULT_STARTUP_SLEEP;
int    allow_ipv6 = 0;
int    secure_ssl = 1;		/* Strict cert validation by default */
int    broken_rtc = 0;		/* Validate certificate time by default */
char  *ca_trust_file = NULL;	/* Custom CA trust file/bundle PEM format */
int    verify_addr = 1;
char  *prognm = NULL;
char  *ident = PACKAGE_NAME;
char  *iface = NULL;
char  *use_iface = NULL;
char  *user_agent = DDNS_USER_AGENT;
char  *config = NULL;
char  *cache_dir = NULL;
char  *script_cmd = NULL;
char  *script_exec = NULL;
int exec_mode = EXEC_MODE_COMPAT;
char  *pidfile_name = NULL;
uid_t  uid = 0;
gid_t  gid = 0;
cfg_t *cfg;

extern cfg_t *conf_parse_file   (char *file, ddns_t *ctx);
extern void   conf_info_cleanup (void);


static int alloc_context(ddns_t **pctx)
{
	int rc = 0;
	ddns_t *ctx;

	if (!pctx)
		return RC_INVALID_POINTER;

	*pctx = (ddns_t *)malloc(sizeof(ddns_t));
	if (!*pctx)
		return RC_OUT_OF_MEMORY;

	do {
		ctx = *pctx;
		memset(ctx, 0, sizeof(ddns_t));

		/* Alloc space for http_to_ip_server data */
		ctx->work_buflen = DDNS_HTTP_RESPONSE_BUFFER_SIZE;
		ctx->work_buf = (char *)malloc(ctx->work_buflen);
		if (!ctx->work_buf) {
			rc = RC_OUT_OF_MEMORY;
			break;
		}

		/* Alloc space for request data */
		ctx->request_buflen = DDNS_HTTP_REQUEST_BUFFER_SIZE;
		ctx->request_buf = (char *)malloc(ctx->request_buflen);
		if (!ctx->request_buf) {
			rc = RC_OUT_OF_MEMORY;
			break;
		}

		ctx->cmd = NO_CMD;
		ctx->normal_update_period_sec = DDNS_DEFAULT_PERIOD;
		ctx->update_period = DDNS_DEFAULT_PERIOD;
		ctx->total_iterations = DDNS_DEFAULT_ITERATIONS;
		ctx->cmd_check_period = DDNS_DEFAULT_CMD_CHECK_PERIOD;

		ctx->initialized = 0;
	}
	while (0);

	if (rc) {

		if (ctx->work_buf)
			free(ctx->work_buf);

		if (ctx->request_buf)
			free(ctx->request_buf);

		free(ctx);
		*pctx = NULL;
	}

	return 0;
}

static void free_context(ddns_t *ctx)
{
	if (!ctx)
		return;

	if (ctx->work_buf) {
		free(ctx->work_buf);
		ctx->work_buf = NULL;
	}

	if (ctx->request_buf) {
		free(ctx->request_buf);
		ctx->request_buf = NULL;
	}

	conf_info_cleanup();
	free(ctx);
}

/* XXX: Should be called from forked child ... */
static int drop_privs(void)
{
	if (uid) {
		if (gid != getgid()) {
			if (setgid(gid))
				return 2;
		}

		if (uid != getuid()) {
			if (setuid(uid))
				return 1;
		}
	}

	return 0;
}

static void parse_privs(char *user)
{
	struct passwd *pw;
	char *group;

	if (!user) {
		pw = getpwuid(geteuid());
		if (!pw)
			user = getenv("LOGNAME");
		else
			user = pw->pw_name;
	}

	if (!user) {
		logit(LOG_INFO, "Cannot figure out username");
		return;
	}

	group = strstr(user, ":");
	if (group)
		*group++ = 0;

	pw = getpwnam(user);
	if (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}

	if (group) {
		struct group *gr = getgrnam(group);

		if (gr)
			gid = gr->gr_gid;
	}
}

static int compose_paths(void)
{
	/* Default .conf file path: "/etc" + '/' + "inadyn" + ".conf" */
	if (!config) {
		size_t len = strlen(SYSCONFDIR) + strlen(ident) + 7;

		config = malloc(len);
		if (!config) {
			logit(LOG_ERR, "Failed allocating memory, exiting.");
			return RC_OUT_OF_MEMORY;
		}
		snprintf(config, len, "%s/%s.conf", SYSCONFDIR, ident);
	}

	/* Default is to let pidfile() API construct PID file from ident */
	if (!pidfile_name)
		pidfile_name = strdup(ident);

	/* Default cache dir: "/var" + "/cache/" + "inadyn" */
	if (!cache_dir) {
		size_t len = strlen(LOCALSTATEDIR) + strlen(ident) + 8;

		cache_dir = malloc(len);
		if (!cache_dir) {
		nomem:
			logit(LOG_ERR, "Failed allocating memory, exiting.");
			return RC_OUT_OF_MEMORY;
		}
		snprintf(cache_dir, len, "%s/cache/%s", LOCALSTATEDIR, ident);

		if (access(cache_dir, W_OK)) {
			char *home, *tmp;

			home = getenv("HOME");
			if (!home) {
				logit(LOG_ERR, "Cannot create fallback cache dir: %s", strerror(errno));
				return 0;
			}

			/* Fallback cache dir: $HOME + "/.cache/" + "inadyn" */
			len = strlen(home) + strlen(ident) + 10;
			tmp = realloc(cache_dir, len);
			if (!tmp){
				free(cache_dir);
				goto nomem;
			} else {
				cache_dir = tmp;
			}

			snprintf(cache_dir, len, "%s/.cache/%s", home, ident);
			if (mkdir(cache_dir, 0755) && EEXIST != errno) {
				snprintf(cache_dir, len, "%s/.%s", home, ident);
				mkdir(cache_dir, 0755);
			}
		}
	}

	return 0;
}

static int usage(int code)
{
        char pidfn[80];

	DO(compose_paths());
	if (pidfile_name[0] != '/')
		snprintf(pidfn, sizeof(pidfn), "%s/%s.pid", RUNSTATEDIR, pidfile_name);
	else
		snprintf(pidfn, sizeof(pidfn), "%s", pidfile_name);

	fprintf(stderr, "Usage:\n %s [-1hnsvC] [-c CMD] [-e CMD] [-f FILE] [-i IFNAME] [-I NAME] [-l LVL] [-p USR:GRP] [-P FILE] [-t SEC]"
#ifndef DROP_VERBOSE_STRINGS
        "\n\n"
		" -1, --once                     Run only once, updates if too old or unknown\n"
		"     --force                    Force update, even if address has not changed\n"
		"     --cache-dir=PATH           Persistent cache dir of IP sent to providers.\n"
		"                                Default use ident NAME: %s/\n"
		" -c, --cmd=/path/to/cmd         Script or command to run to check IP\n"
		" -C, --continue-on-error        Ignore errors from DDNS provider\n"
		" -e, --exec=/path/to/cmd        Script to run on DDNS update\n"
		"     --exec-mode=MODE           Set script run mode: compat, event:\n"
		"                                - compat: successful DDNS update only, default\n"
		"                                - event: any update status\n"
#ifndef DROP_CHECK_CONFIG
		"     --check-config             Verify syntax of configuration file and exit\n"
#endif
		" -f, --config=FILE              Use FILE name for configuration, default uses\n"
		"                                ident NAME: %s\n"
		" -h, --help                     Show summary of command line options and exit\n"
		" -i, --iface=IFNAME             Check IP of IFNAME instead of external server\n"
		" -I, --ident=NAME               Identity for config file, PID file, cache dir,\n"
		"                                and syslog messages.  Defaults to: %s\n"
		" -j, --json                     JSON output format (-L only)\n"
		" -l, --loglevel=LEVEL           Set log level: none, err, info, notice*, debug\n"
		" -L, --list-providers           List available DDNS providers\n"
		" -n, --foreground               Run in foreground with logging to stdout/stderr\n"
		" -p, --drop-privs=USER[:GROUP]  Drop privileges after start to USER:GROUP\n"
		"     --no-pidfile               Do not create PID file, for use with systemd\n"
		" -P, --pidfile=FILE             File to store process ID for signaling %s\n"
		"                                Default uses ident NAME: %s\n"
		" -s, --syslog                   Log to syslog, default unless --foreground\n"
		" -S, --show-provider NAME       Show information about DDNS provider NAME\n"
		" -t, --startup-delay=SEC        Initial startup delay, default none\n"
		" -v, --version                  Show program version and exit\n\n"
		"Bug report address: %s\n",
		prognm, cache_dir, config,
		prognm, prognm, pidfn,
		PACKAGE_BUGREPORT
#else
		" --force --cache-dir=PATH --exec-mode=MODE"
#ifndef DROP_CHECK_CONFIG
		" --check-config"
#endif
		" --no-pidfile\n\n",
		prognm
#endif
		);
#ifdef PACKAGE_URL
	fprintf(stderr, "Project homepage: %s\n", PACKAGE_URL);
#endif

	return code;
}

static char *progname(char *arg0)
{
       char *nm;

       nm = strrchr(arg0, '/');
       if (nm)
	       nm++;
       else
	       nm = arg0;

       return nm;
}

int main(int argc, char *argv[])
{
	int c, restart, rc = 0;
	int use_syslog = 1;
#ifndef DROP_CHECK_CONFIG
	int check_config = 0;
#endif
	int list = 0, json = 0;
	int background = 1;
	static const struct option opt[] = {
		{ "once",              0, 0, '1' },
		{ "force",             0, 0, '4' },
		{ "cache-dir",         1, 0, 128 },
		{ "cmd",               1, 0, 'c' },
		{ "continue-on-error", 0, 0, 'C' },
		{ "exec",              1, 0, 'e' },
		{ "exec-mode",         1, 0, 130 },
		{ "config",            1, 0, 'f' },
		{ "check-config",      0, 0, 129 },
		{ "iface",             1, 0, 'i' },
		{ "ident",             1, 0, 'I' },
		{ "json",              0, 0, 'j' },
		{ "loglevel",          1, 0, 'l' },
		{ "list-providers",    0, 0, 'L' },
		{ "help",              0, 0, 'h' },
		{ "foreground",        0, 0, 'n' },
		{ "no-pidfile",        0, 0, 'N' },
		{ "pidfile",           1, 0, 'P' },
		{ "drop-privs",        1, 0, 'p' },
		{ "syslog",            0, 0, 's' },
		{ "show-provider",     0, 0, 'S' },
		{ "startup-delay",     1, 0, 't' },
		{ "version",           0, 0, 'v' },
		{ NULL,                0, 0, 0   }
	};
	ddns_t *ctx = NULL;

	/* Set up initial values for uid + gid */
	parse_privs(NULL);

	prognm = ident = progname(argv[0]);
	while ((c = getopt_long(argc, argv, "1c:Ce:f:h?i:I:jl:LnNp:P:sS:t:v", opt, NULL)) != EOF) {
		switch (c) {
		case '1':	/* --once */
			once = 1;
			break;

		case '4':	/* -- force */
			force = 1;
			break;

		case 128:	/* --cache-dir=PATH */
			cache_dir = strdup(optarg);
			break;

		case 'c':	/* --cmd=CMD */
			script_cmd = optarg;
			break;

		case 'C':	/* --continue-on-error */
			ignore_errors = 1;
			break;

		case 'e':	/* --exec=CMD */
			script_exec = optarg;
			break;

		case 130:	/* --exec-mode=MODE */
			if (!strcmp(optarg, "event"))
				exec_mode = EXEC_MODE_EVENT;
			else if (!strcmp(optarg, "compat"))
				exec_mode = EXEC_MODE_COMPAT;
			else
				return usage(1);
			break;

		case 'f':	/* --config=FILE */
			config = strdup(optarg);
			break;

#ifndef DROP_CHECK_CONFIG
		case 129:	/* --check-config */
			check_config = 1;
			background = 0;
			use_syslog--;
			break;
#endif

		case 'i':	/* --iface=IFNAME */
			use_iface = iface = optarg;
			break;

		case 'I':	/* --ident=NAME */
			ident = optarg;
			break;

		case 'j':
			json = 1;
			break;

		case 'l':	/* --loglevel=LEVEL */
			rc = log_level(optarg);
			if (-1 == rc)
				return usage(1);
			break;

		case 'L':
			list = 1;
			break;

		case 'n':	/* --foreground */
			background = 0;
			use_syslog--;
			break;

		case 'N':	/* --no-pidfile */
			optarg = "";
			/* fallthrough */
		case 'P':	/* --pidfile=NAME */
			if (pidfile_name)
				free(pidfile_name);
			pidfile_name = strdup(optarg);
			break;

		case 'p':	/* --drop-privs=USER[:GROUP] */
			parse_privs(optarg);
			break;

		case 's':	/* --syslog */
			use_syslog++;
			break;

		case 'S':
			return plugin_show(optarg);

		case 't':	/* --startup-delay=SEC */
			startup_delay = atoi(optarg);
			break;

		case 'v':
			puts(VERSION);
			return 0;

		case 'h':	/* --help */
		case ':':	/* Missing parameter for option. */
		case '?':	/* Unknown option. */
		default:
			return usage(0);
		}
	}

	if (list)
		return plugin_list(json);

	/* Figure out .conf file, cache directory, and PID file name */
	DO(compose_paths());

#ifndef DROP_CHECK_CONFIG
	if (check_config) {
		char pidfn[80];

		if (pidfile_name[0] == 0)
			strlcpy(pidfn, "<none>", sizeof(pidfn));
		else if (pidfile_name[0] != '/')
			snprintf(pidfn, sizeof(pidfn), "%s/%s.pid", RUNSTATEDIR, pidfile_name);
		else
			snprintf(pidfn, sizeof(pidfn), "%s", pidfile_name);

		logit(LOG_DEBUG, "config    : %s", config);
		logit(LOG_DEBUG, "pidfile   : %s", pidfn);
		logit(LOG_DEBUG, "cache-dir : %s", cache_dir);

		rc = alloc_context(&ctx);
		if (rc) {
			logit(LOG_ERR, "Failed allocating memory, cannot check configuration file.");
			return rc;
		}

		logit(LOG_DEBUG, "Checking configuration file %s", config);
		cfg = conf_parse_file(config, ctx);
		if (!cfg) {
			free_context(ctx);
			return RC_ERROR;
		}

		logit(LOG_DEBUG, "Configuration file OK");
		free_context(ctx);
		cfg_free(cfg);

		return RC_OK;
	}
#endif

	if (background) {
		if (daemon(0, 0) < 0) {
			logit(LOG_ERR, "Failed daemonizing %s: %s", ident, strerror(errno));
			return RC_OS_FORK_FAILURE;
		}
	}

	/* Enable syslog or console debugging */
	log_init(ident, use_syslog < 1 ? 0 : 1, background);

	/* Check permission to write PID and cache files */
	if (!once) {
		DO(os_check_perms());

		/* Only allowed with --once */
		force = 0;
	}

	if (drop_privs()) {
		logit(LOG_WARNING, "Failed dropping privileges: %s", strerror(errno));
		rc = RC_OS_CHANGE_PERSONA_FAILURE;
		goto leave;
	}

	/* "Hello!" Let user know we've started up OK */
	logit(LOG_NOTICE, "%s", VERSION_STRING);

	/* Prepare SSL library, if enabled */
	rc = ssl_init();
	if (rc)
		goto leave;

	do {
		restart = 0;

		rc = alloc_context(&ctx);
		if (rc != RC_OK)
			break;

		rc = os_install_signal_handler(ctx);
		if (rc) {
			free_context(ctx);
			break;
		}

		cfg = conf_parse_file(config, ctx);
		if (!cfg) {
			rc = RC_FILE_IO_MISSING_FILE;
			free_context(ctx);
			break;
		}

		rc = ddns_main_loop(ctx);
		if (rc == RC_RESTART)
			restart = 1;

		free_context(ctx);
		cfg_free(cfg);
	} while (restart);

	ssl_exit();
leave:
	if (rc)
		logit(LOG_ERR, "Error code %d: %s", rc, error_str(rc));

	log_exit();
	free(config);
	free(pidfile_name);
	free(cache_dir);

	return rc;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
