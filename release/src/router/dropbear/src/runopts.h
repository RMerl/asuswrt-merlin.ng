/*
 * Dropbear - a SSH2 server
 *
 * Copyright (c) 2002,2003 Matt Johnston
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

#ifndef DROPBEAR_RUNOPTS_H_
#define DROPBEAR_RUNOPTS_H_

#include "includes.h"
#include "signkey.h"
#include "buffer.h"
#include "auth.h"
#include "tcpfwd.h"

typedef struct runopts {

	int disable_ip_tos;
#if DROPBEAR_SVR_REMOTETCPFWD || DROPBEAR_CLI_LOCALTCPFWD \
    || DROPBEAR_CLI_REMOTETCPFWD
	int listen_fwd_all;
#endif
	unsigned int recv_window;
	long keepalive_secs; /* Time between sending keepalives. 0 is off */
	long idle_timeout_secs; /* Exit if no traffic is sent/received in this time */
	int usingsyslog;

#ifndef DISABLE_ZLIB
	/* Whether any compression is allowed. The specific method used
	 * varies between client and server, it will be set up by kex_setup_compress() */
	int allow_compress;
#endif

#if DROPBEAR_USER_ALGO_LIST
	const char *cipher_list;
	const char *mac_list;
#endif

} runopts;

extern runopts opts;

int readhostkey(const char * filename, sign_key * hostkey,
	enum signkey_type *type);
void load_all_hostkeys(void);

typedef struct svr_runopts {

	char * bannerfile;

	int forkbg;

	/* ports and addresses are arrays of the portcount
	listening ports. strings are malloced. */
	char *ports[DROPBEAR_MAX_PORTS];
	unsigned int portcount;
	char *addresses[DROPBEAR_MAX_PORTS];

	int inetdmode;
	/* Hidden "-2 childpipe_fd" flag indicates it's re-executing itself,
	   stores the childpipe preauth file descriptor. Set to -1 otherwise. */
	int reexec_childpipe;

	/* Flags indicating whether to use ipv4 and ipv6 */
	/* not used yet
	int ipv4;
	int ipv6;
	*/

#if DO_MOTD
	/* whether to print the MOTD */
	int domotd;
#endif
	int norootlogin;

#ifdef HAVE_GETGROUPLIST
	/* restrict_group is the group name if group restriction was enabled,
	NULL otherwise */
	char *restrict_group;
	/* restrict_group_gid is only valid if restrict_group is set */
	gid_t restrict_group_gid;
#endif

	int noauthpass;
	int norootpass;
	int allowblankpass;
	int multiauthmethod;
	unsigned int maxauthtries;

#if DROPBEAR_SVR_REMOTETCPFWD
	int noremotetcp;
#endif
#if DROPBEAR_SVR_LOCALANYFWD
	int nolocaltcp;
#endif

	sign_key *hostkey;

	int delay_hostkey;

	char *hostkey_files[MAX_HOSTKEYS];
	int num_hostkey_files;

	buffer * banner;
	char * pidfile;

	char * authorized_keys_dir;

	char * forced_command;
	char* interface;

#if DROPBEAR_PLUGIN
	/* malloced */
	char *pubkey_plugin;
	/* points into pubkey_plugin */
	char *pubkey_plugin_options;
#endif

	int pass_on_env;

} svr_runopts;

extern svr_runopts svr_opts;

void svr_getopts(int argc, char ** argv);
void loadhostkeys(void);

typedef struct cli_runopts {
	/* All non-const strings are malloced */

	const char *progname;
	char *remotehost;
	int remotehostfixed;
	char *remoteport;

	char *own_user;
	char *username;

	char *cmd;
	int wantpty;
	int always_accept_key;
	int no_hostkey_check;
	int ask_hostkey;
	int no_cmd;
	int quiet;
	int backgrounded;
	int is_subsystem;
#if DROPBEAR_CLI_PUBKEY_AUTH
	m_list *privkeys; /* Keys to use for public-key auth */
#endif
#if DROPBEAR_CLI_ANYTCPFWD
	int exit_on_fwd_failure;
#endif
	int disable_trivial_auth;
	/** Use a password authentication or a key auth only.
	For a BatchMode it's always -o PasswordAuthentication=no */
	int password_authentication;
	/* -o BatchMode=yes, suppress interactive questions */
	int batch_mode;
#if DROPBEAR_CLI_REMOTETCPFWD
	m_list * remotefwds;
#endif
#if DROPBEAR_CLI_LOCALTCPFWD
	m_list * localfwds;
#endif
#if DROPBEAR_CLI_AGENTFWD
	int agent_fwd;
	int agent_keys_loaded; /* whether pubkeys has been populated with a
							  list of keys held by the agent */
	int agent_fd; /* The agent fd is only set during authentication. Forwarded
	                 agent sessions have their own file descriptors */
#endif

#if DROPBEAR_CLI_NETCAT
	char *netcat_host;
	unsigned int netcat_port;
#endif
#if DROPBEAR_CLI_PROXYCMD
	char *proxycmd;
#endif
	const char *bind_arg;
	char *bind_address;
	char *bind_port;
	const char *keepalive_arg;
} cli_runopts;

extern cli_runopts cli_opts;
void cli_getopts(int argc, char ** argv);

#if DROPBEAR_USER_ALGO_LIST
void parse_ciphers_macs(void);
#endif

void print_version(void);
void parse_recv_window(const char* recv_window_arg);
int split_address_port(const char* spec, char **first, char ** second);

#if DROPBEAR_CLI_PUBKEY_AUTH
void loadidentityfile(const char* filename, int warnfail);
#endif

#if DROPBEAR_USE_SSH_CONFIG
void read_config_file(char* filename, FILE* config_file, cli_runopts* options);
#endif

#endif /* DROPBEAR_RUNOPTS_H_ */
