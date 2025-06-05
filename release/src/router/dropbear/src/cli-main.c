/*
 * Dropbear - a SSH2 server
 * SSH client implementation
 * 
 * Copyright (c) 2002,2003 Matt Johnston
 * Copyright (c) 2004 by Mihnea Stoenescu
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */

#include "includes.h"
#include "dbutil.h"
#include "runopts.h"
#include "session.h"
#include "dbrandom.h"
#include "crypto_desc.h"
#include "netio.h"
#include "fuzz.h"

#if DROPBEAR_CLI_PROXYCMD
static void cli_proxy_cmd(int *sock_in, int *sock_out, pid_t *pid_out);
static void kill_proxy_sighandler(int signo);
#endif

#if defined(DBMULTI_dbclient) || !DROPBEAR_MULTI
#if defined(DBMULTI_dbclient) && DROPBEAR_MULTI
int cli_main(int argc, char ** argv) {
#else
int main(int argc, char ** argv) {
#endif

	int sock_in, sock_out;
	struct dropbear_progress_connection *progress = NULL;
	pid_t proxy_cmd_pid = 0;

	_dropbear_exit = cli_dropbear_exit;
	_dropbear_log = cli_dropbear_log;

	disallow_core();

	seedrandom();
	crypto_init();

	cli_getopts(argc, argv);

#ifndef DISABLE_SYSLOG
	if (opts.usingsyslog) {
		startsyslog("dbclient");
	}
#endif

        if (cli_opts.bind_address) {
		DEBUG1(("connect to: user=%s host=%s/%s bind_address=%s:%s", cli_opts.username,
			cli_opts.remotehost, cli_opts.remoteport, cli_opts.bind_address, cli_opts.bind_port))
	} else {
		DEBUG1(("connect to: user=%s host=%s/%s",cli_opts.username,cli_opts.remotehost,cli_opts.remoteport))
	}

	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		dropbear_exit("signal() error");
	}

#if DROPBEAR_CLI_PROXYCMD
	if (cli_opts.proxycmd || cli_opts.proxyexec) {
		cli_proxy_cmd(&sock_in, &sock_out, &proxy_cmd_pid);
		if (signal(SIGINT, kill_proxy_sighandler) == SIG_ERR ||
			signal(SIGTERM, kill_proxy_sighandler) == SIG_ERR ||
			signal(SIGHUP, kill_proxy_sighandler) == SIG_ERR) {
			dropbear_exit("signal() error");
		}
	} else
#endif
	{
		progress = connect_remote(cli_opts.remotehost, cli_opts.remoteport,
			cli_connected, &ses, cli_opts.bind_address, cli_opts.bind_port,
			DROPBEAR_PRIO_LOWDELAY);
		sock_in = sock_out = -1;
	}

	cli_session(sock_in, sock_out, progress, proxy_cmd_pid);

	/* not reached */
	return -1;
}
#endif /* DBMULTI stuff */

#if DROPBEAR_CLI_PROXYCMD
static void shell_proxy_cmd(const void *user_data_cmd) {
	const char *cmd = user_data_cmd;
	char *usershell;

	usershell = m_strdup(get_user_shell());
	run_shell_command(cmd, ses.maxfd, usershell);
	dropbear_exit("Failed to run '%s'\n", cmd);
}

static void exec_proxy_cmd(const void *unused) {
	(void)unused;
	run_command(cli_opts.proxyexec[0], cli_opts.proxyexec, ses.maxfd);
	dropbear_exit("Failed to run '%s'\n", cli_opts.proxyexec[0]);
}

static void cli_proxy_cmd(int *sock_in, int *sock_out, pid_t *pid_out) {
	char * cmd_arg = NULL;
	void (*exec_fn)(const void *user_data) = NULL;
	int ret;

	/* exactly one of cli_opts.proxycmd or cli_opts.proxyexec should be set */

	/* File descriptor "-j &3" */
	if (cli_opts.proxycmd && *cli_opts.proxycmd == '&') {
		char *p = cli_opts.proxycmd + 1;
		int sock = strtoul(p, &p, 10);
		/* must be a single number, and not stdin/stdout/stderr */
		if (sock > 2 && sock < 1024 && *p == '\0') {
			*sock_in = sock;
			*sock_out = sock;
			goto cleanup;
		}
	}

	if (cli_opts.proxycmd) {
		/* Normal proxycommand */
		size_t shell_cmdlen;
		/* So that spawn_command knows which shell to run */
		fill_passwd(cli_opts.own_user);

		shell_cmdlen = strlen(cli_opts.proxycmd) + 6; /* "exec " + command + '\0' */
		cmd_arg = m_malloc(shell_cmdlen);
		snprintf(cmd_arg, shell_cmdlen, "exec %s", cli_opts.proxycmd);
		exec_fn = shell_proxy_cmd;
	} else {
		/* No shell */
		exec_fn = exec_proxy_cmd;
	}

	ret = spawn_command(exec_fn, cmd_arg, sock_out, sock_in, NULL, pid_out);
	if (ret == DROPBEAR_FAILURE) {
		dropbear_exit("Failed running proxy command");
		*sock_in = *sock_out = -1;
	}

cleanup:
	m_free(cli_opts.proxycmd);
	m_free(cmd_arg);
	if (cli_opts.proxyexec) {
		char **a = NULL;
		for (a = cli_opts.proxyexec; *a; a++) {
			m_free_direct(*a);
		}
		m_free(cli_opts.proxyexec);
	}
}

static void kill_proxy_sighandler(int UNUSED(signo)) {
	kill_proxy_command();
	_exit(1);
}

#endif /* DROPBEAR_CLI_PROXYCMD */
