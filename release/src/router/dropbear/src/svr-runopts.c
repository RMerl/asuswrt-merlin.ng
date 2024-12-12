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

#include "includes.h"
#include "runopts.h"
#include "signkey.h"
#include "buffer.h"
#include "dbutil.h"
#include "algo.h"
#include "ecdsa.h"

#include <grp.h>

svr_runopts svr_opts; /* GLOBAL */

static void printhelp(const char * progname);
static void addportandaddress(const char* spec);
static void loadhostkey(const char *keyfile, int fatal_duplicate);
static void addhostkey(const char *keyfile);
static void load_banner();

static void printhelp(const char * progname) {

	fprintf(stderr, "Dropbear server v%s https://matt.ucc.asn.au/dropbear/dropbear.html\n"
					"Usage: %s [options]\n"
					"-b bannerfile	Display the contents of bannerfile"
					" before user login\n"
					"		(default: none)\n"
					"-r keyfile      Specify hostkeys (repeatable)\n"
					"		defaults: \n"
#if DROPBEAR_DSS
					"		- dss %s\n"
#endif
#if DROPBEAR_RSA
					"		- rsa %s\n"
#endif
#if DROPBEAR_ECDSA
					"		- ecdsa %s\n"
#endif
#if DROPBEAR_ED25519
					"		- ed25519 %s\n"
#endif
#if DROPBEAR_DELAY_HOSTKEY
					"-R		Create hostkeys as required\n" 
#endif
					"-F		Don't fork into background\n"
					"-e		Pass on server process environment to child process\n"
#ifdef DISABLE_SYSLOG
					"(Syslog support not compiled in, using stderr)\n"
#else
					"-E		Log to stderr rather than syslog\n"
#endif
#if DO_MOTD
					"-m		Don't display the motd on login\n"
#endif
					"-w		Disallow root logins\n"
#ifdef HAVE_GETGROUPLIST
					"-G		Restrict logins to members of specified group\n"
#endif
#if DROPBEAR_SVR_PASSWORD_AUTH || DROPBEAR_SVR_PAM_AUTH
					"-s		Disable password logins\n"
					"-g		Disable password logins for root\n"
					"-B		Allow blank password logins\n"
					"-t		Enable two-factor authentication (both password and public key required)\n"
#endif
					"-T		Maximum authentication tries (default %d)\n"
#if DROPBEAR_SVR_LOCALANYFWD
					"-j		Disable local port forwarding\n"
#endif
#if DROPBEAR_SVR_REMOTETCPFWD
					"-k		Disable remote port forwarding\n"
					"-a		Allow connections to forwarded ports from any host\n"
					"-c command	Force executed command\n"
#endif
					"-p [address:]port\n"
					"		Listen on specified tcp port (and optionally address),\n"
					"		up to %d can be specified\n"
					"		(default port is %s if none specified)\n"
					"-P PidFile	Create pid file PidFile\n"
					"		(default %s)\n"
#ifdef SO_BINDTODEVICE
					"-l <interface>\n"
					"		interface to bind on\n"
#endif
#if INETD_MODE
					"-i		Start for inetd\n"
#endif
					"-W <receive_window_buffer> (default %d, larger may be faster, max 10MB)\n"
					"-K <keepalive>  (0 is never, default %d, in seconds)\n"
					"-I <idle_timeout>  (0 is never, default %d, in seconds)\n"
					"-z    disable QoS\n"
#if DROPBEAR_PLUGIN
                                        "-A <authplugin>[,<options>]\n"
                                        "               Enable external public key auth through <authplugin>\n"
#endif
					"-V    Version\n"
#if DEBUG_TRACE
					"-v    verbose (repeat for more verbose)\n"
#endif
					,DROPBEAR_VERSION, progname,
#if DROPBEAR_DSS
					DSS_PRIV_FILENAME,
#endif
#if DROPBEAR_RSA
					RSA_PRIV_FILENAME,
#endif
#if DROPBEAR_ECDSA
					ECDSA_PRIV_FILENAME,
#endif
#if DROPBEAR_ED25519
					ED25519_PRIV_FILENAME,
#endif
					MAX_AUTH_TRIES,
					DROPBEAR_MAX_PORTS, DROPBEAR_DEFPORT, DROPBEAR_PIDFILE,
					DEFAULT_RECV_WINDOW, DEFAULT_KEEPALIVE, DEFAULT_IDLE_TIMEOUT);
}

void svr_getopts(int argc, char ** argv) {

	unsigned int i, j;
	char ** next = NULL;
	int nextisport = 0;
	char* recv_window_arg = NULL;
	char* keepalive_arg = NULL;
	char* idle_timeout_arg = NULL;
	char* maxauthtries_arg = NULL;
	char* reexec_fd_arg = NULL;
	char* keyfile = NULL;
	char c;
#if DROPBEAR_PLUGIN
        char* pubkey_plugin = NULL;
#endif


	/* see printhelp() for options */
	svr_opts.bannerfile = NULL;
	svr_opts.banner = NULL;
	svr_opts.forced_command = NULL;
	svr_opts.forkbg = 1;
	svr_opts.norootlogin = 0;
#ifdef HAVE_GETGROUPLIST
	svr_opts.restrict_group = NULL;
	svr_opts.restrict_group_gid = 0;
#endif
	svr_opts.noauthpass = 0;
	svr_opts.norootpass = 0;
	svr_opts.allowblankpass = 0;
	svr_opts.multiauthmethod = 0;
	svr_opts.maxauthtries = MAX_AUTH_TRIES;
	svr_opts.inetdmode = 0;
	svr_opts.portcount = 0;
	svr_opts.hostkey = NULL;
	svr_opts.delay_hostkey = 0;
	svr_opts.pidfile = expand_homedir_path(DROPBEAR_PIDFILE);
#if DROPBEAR_SVR_LOCALANYFWD
	svr_opts.nolocaltcp = 0;
#endif
#if DROPBEAR_SVR_REMOTETCPFWD
	svr_opts.noremotetcp = 0;
#endif
#if DROPBEAR_PLUGIN
        svr_opts.pubkey_plugin = NULL;
        svr_opts.pubkey_plugin_options = NULL;
#endif
	svr_opts.pass_on_env = 0;
	svr_opts.reexec_childpipe = -1;

#ifndef DISABLE_ZLIB
	opts.compress_mode = DROPBEAR_COMPRESS_DELAYED;
#endif 

	/* not yet
	opts.ipv4 = 1;
	opts.ipv6 = 1;
	*/
#if DO_MOTD
	svr_opts.domotd = 1;
#endif
#ifndef DISABLE_SYSLOG
	opts.usingsyslog = 1;
#endif
	opts.recv_window = DEFAULT_RECV_WINDOW;
	opts.keepalive_secs = DEFAULT_KEEPALIVE;
	opts.idle_timeout_secs = DEFAULT_IDLE_TIMEOUT;
	
#if DROPBEAR_SVR_REMOTETCPFWD
	opts.listen_fwd_all = 0;
#endif
	opts.disable_ip_tos = 0;

	for (i = 1; i < (unsigned int)argc; i++) {
		if (argv[i][0] != '-' || argv[i][1] == '\0')
			dropbear_exit("Invalid argument: %s", argv[i]);

		for (j = 1; (c = argv[i][j]) != '\0' && !next && !nextisport; j++) {
			switch (c) {
				case 'b':
					next = &svr_opts.bannerfile;
					break;
				case 'c':
					next = &svr_opts.forced_command;
					break;
				case 'd':
				case 'r':
					next = &keyfile;
					break;
				case 'R':
					svr_opts.delay_hostkey = 1;
					break;
				case 'F':
					svr_opts.forkbg = 0;
					break;
#ifndef DISABLE_SYSLOG
				case 'E':
					opts.usingsyslog = 0;
					break;
#endif
				case 'e':
					svr_opts.pass_on_env = 1;
					break;

#if DROPBEAR_SVR_LOCALANYFWD
				case 'j':
					svr_opts.nolocaltcp = 1;
					break;
#endif
#if DROPBEAR_SVR_REMOTETCPFWD
				case 'k':
					svr_opts.noremotetcp = 1;
					break;
				case 'a':
					opts.listen_fwd_all = 1;
					break;
#endif
#if INETD_MODE
				case 'i':
					svr_opts.inetdmode = 1;
					break;
#endif
#if DROPBEAR_DO_REEXEC && NON_INETD_MODE
				/* For internal use by re-exec */
				case '2':
					next = &reexec_fd_arg;
					break;
#endif
				case 'p':
					nextisport = 1;
					break;
				case 'P':
					next = &svr_opts.pidfile;
					break;
#ifdef SO_BINDTODEVICE
				case 'l':
					next = &svr_opts.interface;
					break;
#endif
#if DO_MOTD
				/* motd is displayed by default, -m turns it off */
				case 'm':
					svr_opts.domotd = 0;
					break;
#endif
				case 'w':
					svr_opts.norootlogin = 1;
					break;
#ifdef HAVE_GETGROUPLIST
				case 'G':
					next = &svr_opts.restrict_group;
					break;
#endif
				case 'W':
					next = &recv_window_arg;
					break;
				case 'K':
					next = &keepalive_arg;
					break;
				case 'I':
					next = &idle_timeout_arg;
					break;
				case 'T':
					next = &maxauthtries_arg;
					break;
#if DROPBEAR_SVR_PASSWORD_AUTH || DROPBEAR_SVR_PAM_AUTH
				case 's':
					svr_opts.noauthpass = 1;
					break;
				case 'g':
					svr_opts.norootpass = 1;
					break;
				case 'B':
					svr_opts.allowblankpass = 1;
					break;
				case 't':
					svr_opts.multiauthmethod = 1;
					break;
#endif
				case 'h':
					printhelp(argv[0]);
					exit(EXIT_SUCCESS);
					break;
				case 'u':
					/* backwards compatibility with old urandom option */
					break;
#if DROPBEAR_PLUGIN
                                case 'A':
                                        next = &pubkey_plugin;
                                        break;
#endif
#if DEBUG_TRACE
				case 'v':
					debug_trace++;
					break;
#endif
				case 'V':
					print_version();
					exit(EXIT_SUCCESS);
					break;
				case 'z':
					opts.disable_ip_tos = 1;
					break;
				default:
					fprintf(stderr, "Invalid option -%c\n", c);
					printhelp(argv[0]);
					exit(EXIT_FAILURE);
					break;
			}
		}

		if (!next && !nextisport)
			continue;

		if (c == '\0') {
			i++;
			j = 0;
			if (!argv[i]) {
				dropbear_exit("Missing argument");
			}
		}

		if (nextisport) {
			addportandaddress(&argv[i][j]);
			nextisport = 0;
		} else if (next) {
			*next = &argv[i][j];
			if (*next == NULL) {
				dropbear_exit("Invalid null argument");
			}
			next = NULL;

			if (keyfile) {
				addhostkey(keyfile);
				keyfile = NULL;
			}
		}
	}

	/* Set up listening ports */
	if (svr_opts.portcount == 0) {
		svr_opts.ports[0] = m_strdup(DROPBEAR_DEFPORT);
		svr_opts.addresses[0] = m_strdup(DROPBEAR_DEFADDRESS);
		svr_opts.portcount = 1;
	}

	if (svr_opts.bannerfile) {
		load_banner();
	}

#ifdef HAVE_GETGROUPLIST
	if (svr_opts.restrict_group) {
		struct group *restrictedgroup = getgrnam(svr_opts.restrict_group);

		if (restrictedgroup){
			svr_opts.restrict_group_gid = restrictedgroup->gr_gid;
		} else {
			dropbear_exit("Cannot restrict logins to group '%s' as the group does not exist", svr_opts.restrict_group);
		}
	}
#endif

	if (recv_window_arg) {
		parse_recv_window(recv_window_arg);
	}

	if (maxauthtries_arg) {
		unsigned int val = 0;
		if (m_str_to_uint(maxauthtries_arg, &val) == DROPBEAR_FAILURE 
			|| val == 0) {
			dropbear_exit("Bad maxauthtries '%s'", maxauthtries_arg);
		}
		svr_opts.maxauthtries = val;
	}


	if (keepalive_arg) {
		unsigned int val;
		if (m_str_to_uint(keepalive_arg, &val) == DROPBEAR_FAILURE) {
			dropbear_exit("Bad keepalive '%s'", keepalive_arg);
		}
		opts.keepalive_secs = val;
	}

	if (idle_timeout_arg) {
		unsigned int val;
		if (m_str_to_uint(idle_timeout_arg, &val) == DROPBEAR_FAILURE) {
			dropbear_exit("Bad idle_timeout '%s'", idle_timeout_arg);
		}
		opts.idle_timeout_secs = val;
	}

	if (svr_opts.forced_command) {
		dropbear_log(LOG_INFO, "Forced command set to '%s'", svr_opts.forced_command);
	}

	if (svr_opts.interface) {
		dropbear_log(LOG_INFO, "Binding to interface '%s'", svr_opts.interface);
	}

	if (reexec_fd_arg) {
		if (m_str_to_uint(reexec_fd_arg, &svr_opts.reexec_childpipe) == DROPBEAR_FAILURE
			|| svr_opts.reexec_childpipe < 0) {
			dropbear_exit("Bad -2");
		}
	}

	if (svr_opts.multiauthmethod && svr_opts.noauthpass) {
		dropbear_exit("-t and -s are incompatible");
	}

#if DROPBEAR_PLUGIN
	if (pubkey_plugin) {
		svr_opts.pubkey_plugin = m_strdup(pubkey_plugin);
		char *args = strchr(svr_opts.pubkey_plugin, ',');
		if (args) {
			*args='\0';
			++args;
		}
		svr_opts.pubkey_plugin_options = args;
	}
#endif
}

static void addportandaddress(const char* spec) {
	char *port = NULL, *address = NULL;

	if (svr_opts.portcount >= DROPBEAR_MAX_PORTS) {
		return;
	}

	if (split_address_port(spec, &address, &port) == DROPBEAR_FAILURE) {
		dropbear_exit("Bad -p argument");
	}

	/* A bare port */
	if (!port) {
		port = address;
		address = NULL;
	}

	if (!address) {
		/* no address given -> fill in the default address */
		address = m_strdup(DROPBEAR_DEFADDRESS);
	}

	if (port[0] == '\0') {
		/* empty port -> exit */
		dropbear_exit("Bad port");
	}
	svr_opts.ports[svr_opts.portcount] = port;
	svr_opts.addresses[svr_opts.portcount] = address;
	svr_opts.portcount++;
}

static void disablekey(enum signature_type type) {
	int i;
	TRACE(("Disabling key type %d", type))
	for (i = 0; sigalgs[i].name != NULL; i++) {
		if ((int)sigalgs[i].val == (int)type) {
			sigalgs[i].usable = 0;
			break;
		}
	}
}

static void loadhostkey_helper(const char *name, void** src, void** dst, int fatal_duplicate) {
	if (*dst) {
		if (fatal_duplicate) {
			dropbear_exit("Only one %s key can be specified", name);
		}
	} else {
		*dst = *src;
		*src = NULL;
	}

}

/* Must be called after syslog/etc is working */
static void loadhostkey(const char *keyfile, int fatal_duplicate) {
	sign_key * read_key = new_sign_key();
	char *expand_path = expand_homedir_path(keyfile);
	enum signkey_type type = DROPBEAR_SIGNKEY_ANY;
	if (readhostkey(expand_path, read_key, &type) == DROPBEAR_FAILURE) {
		if (!svr_opts.delay_hostkey) {
			dropbear_log(LOG_WARNING, "Failed loading %s", expand_path);
		}
	}
	m_free(expand_path);

#if DROPBEAR_RSA
	if (type == DROPBEAR_SIGNKEY_RSA) {
		loadhostkey_helper("RSA", (void**)&read_key->rsakey, (void**)&svr_opts.hostkey->rsakey, fatal_duplicate);
	}
#endif

#if DROPBEAR_DSS
	if (type == DROPBEAR_SIGNKEY_DSS) {
		loadhostkey_helper("DSS", (void**)&read_key->dsskey, (void**)&svr_opts.hostkey->dsskey, fatal_duplicate);
	}
#endif

#if DROPBEAR_ECDSA
#if DROPBEAR_ECC_256
	if (type == DROPBEAR_SIGNKEY_ECDSA_NISTP256) {
		loadhostkey_helper("ECDSA256", (void**)&read_key->ecckey256, (void**)&svr_opts.hostkey->ecckey256, fatal_duplicate);
	}
#endif
#if DROPBEAR_ECC_384
	if (type == DROPBEAR_SIGNKEY_ECDSA_NISTP384) {
		loadhostkey_helper("ECDSA384", (void**)&read_key->ecckey384, (void**)&svr_opts.hostkey->ecckey384, fatal_duplicate);
	}
#endif
#if DROPBEAR_ECC_521
	if (type == DROPBEAR_SIGNKEY_ECDSA_NISTP521) {
		loadhostkey_helper("ECDSA521", (void**)&read_key->ecckey521, (void**)&svr_opts.hostkey->ecckey521, fatal_duplicate);
	}
#endif
#endif /* DROPBEAR_ECDSA */

#if DROPBEAR_ED25519
	if (type == DROPBEAR_SIGNKEY_ED25519) {
		loadhostkey_helper("ed25519", (void**)&read_key->ed25519key, (void**)&svr_opts.hostkey->ed25519key, fatal_duplicate);
	}
#endif

	sign_key_free(read_key);
	TRACE(("leave loadhostkey"))
}

static void addhostkey(const char *keyfile) {
	if (svr_opts.num_hostkey_files >= MAX_HOSTKEYS) {
		dropbear_exit("Too many hostkeys");
	}
	svr_opts.hostkey_files[svr_opts.num_hostkey_files] = m_strdup(keyfile);
	svr_opts.num_hostkey_files++;
}


void load_all_hostkeys() {
	int i;
	int any_keys = 0;
#if DROPBEAR_ECDSA
	int loaded_any_ecdsa = 0;
#endif

	svr_opts.hostkey = new_sign_key();

	for (i = 0; i < svr_opts.num_hostkey_files; i++) {
		char *hostkey_file = svr_opts.hostkey_files[i];
		loadhostkey(hostkey_file, 1);
		m_free(hostkey_file);
	}

	/* Only load default host keys if a host key is not specified by the user */
	if (svr_opts.num_hostkey_files == 0) {
#if DROPBEAR_RSA
		loadhostkey(RSA_PRIV_FILENAME, 0);
#endif

#if DROPBEAR_DSS
		loadhostkey(DSS_PRIV_FILENAME, 0);
#endif

#if DROPBEAR_ECDSA
		loadhostkey(ECDSA_PRIV_FILENAME, 0);
#endif
#if DROPBEAR_ED25519
		loadhostkey(ED25519_PRIV_FILENAME, 0);
#endif
	}

#if DROPBEAR_RSA
	if (!svr_opts.delay_hostkey && !svr_opts.hostkey->rsakey) {
#if DROPBEAR_RSA_SHA256
		disablekey(DROPBEAR_SIGNATURE_RSA_SHA256);
#endif
#if DROPBEAR_RSA_SHA1
		disablekey(DROPBEAR_SIGNATURE_RSA_SHA1);
#endif
	} else {
		any_keys = 1;
	}
#endif

#if DROPBEAR_DSS
	if (!svr_opts.delay_hostkey && !svr_opts.hostkey->dsskey) {
		disablekey(DROPBEAR_SIGNATURE_DSS);
	} else {
		any_keys = 1;
	}
#endif

#if DROPBEAR_ECDSA
	/* We want to advertise a single ecdsa algorithm size.
	- If there is a ecdsa hostkey at startup we choose that that size.
	- If we generate at runtime we choose the default ecdsa size.
	- Otherwise no ecdsa keys will be advertised */

	/* check if any keys were loaded at startup */
	loaded_any_ecdsa = 
		0
#if DROPBEAR_ECC_256
		|| svr_opts.hostkey->ecckey256
#endif
#if DROPBEAR_ECC_384
		|| svr_opts.hostkey->ecckey384
#endif
#if DROPBEAR_ECC_521
		|| svr_opts.hostkey->ecckey521
#endif
		;
	any_keys |= loaded_any_ecdsa;

	/* Or an ecdsa key could be generated at runtime */
	any_keys |= svr_opts.delay_hostkey;

	/* At most one ecdsa key size will be left enabled */
#if DROPBEAR_ECC_256
	if (!svr_opts.hostkey->ecckey256
		&& (!svr_opts.delay_hostkey || loaded_any_ecdsa || ECDSA_DEFAULT_SIZE != 256 )) {
		disablekey(DROPBEAR_SIGNATURE_ECDSA_NISTP256);
	}
#endif
#if DROPBEAR_ECC_384
	if (!svr_opts.hostkey->ecckey384
		&& (!svr_opts.delay_hostkey || loaded_any_ecdsa || ECDSA_DEFAULT_SIZE != 384 )) {
		disablekey(DROPBEAR_SIGNATURE_ECDSA_NISTP384);
	}
#endif
#if DROPBEAR_ECC_521
	if (!svr_opts.hostkey->ecckey521
		&& (!svr_opts.delay_hostkey || loaded_any_ecdsa || ECDSA_DEFAULT_SIZE != 521 )) {
		disablekey(DROPBEAR_SIGNATURE_ECDSA_NISTP521);
	}
#endif
#endif /* DROPBEAR_ECDSA */

#if DROPBEAR_ED25519
	if (!svr_opts.delay_hostkey && !svr_opts.hostkey->ed25519key) {
		disablekey(DROPBEAR_SIGNATURE_ED25519);
	} else {
		any_keys = 1;
	}
#endif
#if DROPBEAR_SK_ECDSA
	disablekey(DROPBEAR_SIGNATURE_SK_ECDSA_NISTP256);
#endif 
#if DROPBEAR_SK_ED25519
	disablekey(DROPBEAR_SIGNATURE_SK_ED25519);
#endif

	if (!any_keys) {
		dropbear_exit("No hostkeys available. 'dropbear -R' may be useful or run dropbearkey.");
	}
}

static void load_banner() {
	struct stat buf;
	if (stat(svr_opts.bannerfile, &buf) != 0) {
		dropbear_log(LOG_WARNING, "Error opening banner file '%s'",
				svr_opts.bannerfile);
		return;
	}

	if (buf.st_size > MAX_BANNER_SIZE) {
		dropbear_log(LOG_WARNING, "Banner file too large, max is %d bytes",
				MAX_BANNER_SIZE);
		return;
	}

	svr_opts.banner = buf_new(buf.st_size);
	if (buf_readfile(svr_opts.banner, svr_opts.bannerfile) != DROPBEAR_SUCCESS) {
		dropbear_log(LOG_WARNING, "Error reading banner file '%s'",
				svr_opts.bannerfile);
		buf_free(svr_opts.banner);
		svr_opts.banner = NULL;
		return;
	}
	buf_setpos(svr_opts.banner, 0);

}
