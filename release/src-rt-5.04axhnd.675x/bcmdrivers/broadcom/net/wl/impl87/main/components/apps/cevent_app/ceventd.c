/*
 * Cevent app header
 *
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
 *
 * $Id: ceventd.c 788158 2020-06-23 09:14:25Z $
 */

#include "ceventd.h"

#define CEVENTD_PID_FILE "/var/run/ceventd.pid"
#define CEVENTD_DEFAULT_CONSOLE	0
#define CEVENTD_DEFAULT_SYSLOG	1
#define CEVENTD_LOCK_ATTEMPT_GAP	1 /* 1 second between consecutive lock attempts */
#define CEVENTD_MAX_LOCK_ATTEMPTS	8

static ca_wksp_t * gCwksp = NULL; /* used by signal handlers */

/* Gets the unsigned integer config val from nvram, if not found applies the default value */
uint32
ca_get_config_val_uint(const char *c, uint32 def)
{
	char *val = NULL;
	uint32 ret = def;

	val = nvram_safe_get(c);

	if (val && (val[0] != '\0')) {
		ret = strtoul(val, NULL, 0);
	} else {
		CA_DBG("NVRAM %s is not defined, used default value %d \n", c, def);
	}

	return ret;
}

void
ca_config_init_out_file(ca_wksp_t *cwksp, const char *nvram_val)
{
	char *nv_out_file_path = NULL;

	nv_out_file_path = nvram_safe_get(nvram_val);

	if (nv_out_file_path && (nv_out_file_path[0] != '\0')) {
		const int pathLen = strnlen(nv_out_file_path, CA_FILE_PATH_LEN);
		if (pathLen > 0 && pathLen < (CA_FILE_PATH_LEN - 4)) { /* -4 to accommodate .bak */
			ca_snprintf(cwksp->out_path, sizeof(cwksp->out_path), nv_out_file_path,
					CA_FILE_PATH_LEN);
			ca_snprintf(cwksp->out_bak_path, sizeof(cwksp->out_bak_path), "%s.bak",
					cwksp->out_path);
			CA_DBG("path:%s, bak:%s\n", cwksp->out_path, cwksp->out_bak_path);
			ca_out_file_init(cwksp);
		}
	}
}

static int
ca_wksp_init(ca_wksp_t *cwksp)
{
	int ret = BCME_OK;
	uint32 limit = 0;

	if (!cwksp) {
		CA_ERR("called with null cwksp\n");
		return BCME_ERROR;
	}

	cwksp->pid = getpid();
	cwksp->num_logs = 0;
	cwksp->watchdog_ts = ca_get_curr_time();

	cwksp->log_type = ca_get_config_val_uint("ceventd_log_type", CA_LOG_TYPE_SSV);
	CA_DBG("cwksp log type %u\n", cwksp->log_type);

	limit = ca_get_config_val_uint("ceventd_out_limit_kb",
			(CA_OUT_LIMIT_DEFAULT_KB << 10) /* kB to B */);
	cwksp->out_limit = (limit >= CA_OUT_LIMIT_MIN_KB && limit <= CA_OUT_LIMIT_MAX_KB) ?
		(limit << 10) : (CA_OUT_LIMIT_DEFAULT_KB << 10);
	CA_DBG("cwksp out limit %ub\n", cwksp->out_limit);

	ca_config_init_out_file(cwksp, "ceventd_out");

	cwksp->log_console = ca_get_config_val_uint("ceventd_log_console", CEVENTD_DEFAULT_CONSOLE);
	CA_DBG("log output to console\n");

	cwksp->log_syslogd = ca_get_config_val_uint("ceventd_log_syslog", CEVENTD_DEFAULT_SYSLOG);
	CA_DBG("send output to syslogd\n");

	/* open syslog. explicit opening is optional; helps add ident, pid prefixes and set flags */
	setlogmask(LOG_UPTO(LOG_NOTICE));
	openlog("CEVENT_APP", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	cwksp->eapd_fd = CA_DEFAULT_FD;
	cwksp->cli_fd = CA_DEFAULT_FD;

	if ((ret = ca_open_eapd(cwksp)) != BCME_OK) {
		return ret;
	}

	if ((ret = ca_cli_init(cwksp)) != BCME_OK) {
		return ret;
	}

	return ret;
}

static int
ca_wksp_deinit(ca_wksp_t *cwksp)
{
	ca_cli_deinit(cwksp);
	ca_close_eapd(cwksp);

	/* close syslog */
	closelog();

	/* close output file */
	if (cwksp->out && cwksp->out != stdout) {
		fclose(cwksp->out);
		CA_DBG("closed output file\n");
	}

	if (cwksp->pid_fd >= 0) {
		if (flock(cwksp->pid_fd, LOCK_UN) < 0) {
			perror("flock");
			CA_ERR("Error unlocking %s\n", CEVENTD_PID_FILE);
		}
	}

	return BCME_OK;
}

static void
ca_term_hdlr(int sig)
{
	if (gCwksp) {
		gCwksp->flags |= CA_WKSP_FLAG_SHUTDOWN;
	}

	return;
}

static void
ca_chld_hdlr(int sig)
{
	pid_t pid;

	while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
		CA_DBG("Reaped child process %d\n", pid);
	}
}

static void
ca_usr1_hdlr(int sig)
{
	if (gCwksp) {
		gCwksp->flags |= CA_WKSP_FLAG_USR1;
	}

	return;
}

/* watchdog - things to be done every peroidic interval */
int ca_watchdog(ca_wksp_t *cwksp)
{
	CA_DBG("\ntick %u @ %llus\n", cwksp->tick, cwksp->watchdog_ts/1000ULL);

	if (cwksp->out) {
		fflush(cwksp->out);
	}

	/* check user signal/command for shutdown */
	if (cwksp->flags & CA_WKSP_FLAG_SHUTDOWN) {
		/* clean up workspace */
		ca_wksp_deinit(cwksp);

		/* free workspace context */
		free(cwksp);

		CA_DBG("shutdown\n");
		return BCME_OK;
	}

	/* check user1 signal  */
	if (cwksp->flags & CA_WKSP_FLAG_USR1) {
		/* TODO: Any custom action can be done on such a signal */
		CA_DBG("CA_WKSP_FLAG_USR1\n");
		cwksp->flags &= ~CA_WKSP_FLAG_USR1;
	}

	return ++cwksp->tick;
}

/* listen to sockets and call handlers; call watchdog */
int
ca_main_loop(ca_wksp_t *cwksp)
{
	int ret;
	uint64 t_start;
	uint32 t_diff;
	FILE *pid_fp;

	/* init nas */
	ret = ca_wksp_init(cwksp);

	if (ret) {
		CA_ERR("Unable to initialize. Quitting\n");
		free(cwksp);
		return BCME_ERROR;
	}

	/* Daemonize */
	if (daemon(1, 1) == -1) {
		/* clean up workspace */
		ca_wksp_deinit(cwksp);
		/* free workspace context */
		perror("ca_main_loop: daemon\n");
		exit(errno);
	}

	pid_fp = fopen(CEVENTD_PID_FILE, "w");

	if (pid_fp != NULL) {
		if (fprintf(pid_fp, "%d\n", getpid()) < 0) {
			CA_ERR("Error writing pid to %s\n", CEVENTD_PID_FILE);
		}
		fclose(pid_fp);
	}

	signal(SIGTERM, ca_term_hdlr);
	signal(SIGCHLD, ca_chld_hdlr);
	signal(SIGUSR1, ca_usr1_hdlr);

	while (1) {
		t_start = ca_get_curr_time();
		t_diff = (uint32) (t_start - cwksp->watchdog_ts);
		if (t_diff >= CA_WATCHDOG_PERIOD_SEC * 1000UL) {
			cwksp->watchdog_ts = t_start;
			if (ca_watchdog(cwksp) <= 0) {
				break;
			}
		}
		/* receive on socket and process */
		ca_socket_process(cwksp);
	}

	return BCME_OK;
}

/* service main entry
 * Usage arg1 - [0-3] ::  0:least, ... 3:most verbose
 */
int
main(int argc, char *argv[])
{
	const char * nv_dbg = nvram_safe_get("ceventd_dbg");
	ca_wksp_t * cwksp = NULL;
	int pid_fd, locked, attempts = 0;

	if ((pid_fd = open(CEVENTD_PID_FILE, O_CREAT | O_RDWR | O_SYNC, 0666)) == -1) {
		perror("open");
		CA_ERR("Could not open/create %s\n", CEVENTD_PID_FILE);
		return BCME_ERROR;
	}

	do {
		if ((locked = flock(pid_fd, LOCK_EX | LOCK_NB)) && EWOULDBLOCK == errno) {
			perror("flock");
			CA_DBG("%s is locked, attempts: %d\n", CEVENTD_PID_FILE, attempts);
			sleep(1); /* reattempt lock after 1 sec */
		} else {
			CA_DBG("lock acquired for %s,  attempts: %d\n", CEVENTD_PID_FILE, attempts);
			break;
		}
	} while (attempts++ < CEVENTD_MAX_LOCK_ATTEMPTS);

	if (locked && EWOULDBLOCK == errno) {
		CA_ERR("Exiting. Daemon already running. (%s is locked) attempts:%d\n",
				CEVENTD_PID_FILE, attempts);
		return BCME_ERROR;
	}

	/* Simple single command line arg for verbosity [0-3]; no getopts; overrides nvram */
	if (argc > 1 && argv[1][0] >= '0' && argv[1][0] <= '3' && argv[1][1] == 0) {
		ca_d_verbose = argv[1][0] - '0';
	} else if (nv_dbg && nv_dbg[0] && nv_dbg[0] >= '0' &&
			nv_dbg[0] <= '3' && nv_dbg[1] == 0) {
		ca_d_verbose = nv_dbg[0] - '0';
	}

	if ((cwksp = (ca_wksp_t *) calloc(1, sizeof(*cwksp))) == NULL) {
		CA_ERR("workspace allocation failure\n");
		return BCME_NOMEM;
	}

	cwksp->pid_fd = pid_fd;
	gCwksp = cwksp; /* used by signal handlers */

	return ca_main_loop(cwksp);
}
