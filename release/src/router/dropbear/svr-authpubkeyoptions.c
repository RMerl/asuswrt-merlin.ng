/*
 * Dropbear - a SSH2 server
 * 
 * Copyright (c) 2008 Frederic Moulins
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
 * SOFTWARE. 
 *
 * This file incorporates work covered by the following copyright and  
 * permission notice:
 *
 * 	Author: Tatu Ylonen <ylo@cs.hut.fi>
 * 	Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 * 	              All rights reserved
 * 	As far as I am concerned, the code I have written for this software
 * 	can be used freely for any purpose.  Any derived versions of this
 * 	software must be clearly marked as such, and if the derived work is
 * 	incompatible with the protocol description in the RFC file, it must be
 * 	called by a name other than "ssh" or "Secure Shell".
 *
 * This copyright and permission notice applies to the code parsing public keys
 * options string which can also be found in OpenSSH auth-options.c file 
 * (auth_parse_options).
 *
 */

/* Process pubkey options during a pubkey auth request */
#include "includes.h"
#include "session.h"
#include "dbutil.h"
#include "signkey.h"
#include "auth.h"
#include "runopts.h"

#if DROPBEAR_SVR_PUBKEY_OPTIONS_BUILT

/* Returns 1 if pubkey allows agent forwarding,
 * 0 otherwise */
int svr_pubkey_allows_agentfwd() {
	if (ses.authstate.pubkey_options 
		&& ses.authstate.pubkey_options->no_agent_forwarding_flag) {
		return 0;
	}
	return 1;
}

/* Returns 1 if pubkey allows tcp forwarding,
 * 0 otherwise */
int svr_pubkey_allows_tcpfwd() {
	if (ses.authstate.pubkey_options 
		&& ses.authstate.pubkey_options->no_port_forwarding_flag) {
		return 0;
	}
	return 1;
}

/* Returns 1 if pubkey allows x11 forwarding,
 * 0 otherwise */
int svr_pubkey_allows_x11fwd() {
	if (ses.authstate.pubkey_options 
		&& ses.authstate.pubkey_options->no_x11_forwarding_flag) {
		return 0;
	}
	return 1;
}

/* Returns 1 if pubkey allows pty, 0 otherwise */
int svr_pubkey_allows_pty() {
	if (ses.authstate.pubkey_options 
		&& ses.authstate.pubkey_options->no_pty_flag) {
		return 0;
	}
	return 1;
}

/* Returns 1 if pubkey allows local tcp fowarding to the provided destination,
 * 0 otherwise */
int svr_pubkey_allows_local_tcpfwd(const char *host, unsigned int port) {
	if (ses.authstate.pubkey_options
		&& ses.authstate.pubkey_options->permit_open_destinations) {
		m_list_elem *iter = ses.authstate.pubkey_options->permit_open_destinations->first;
		while (iter) {
			struct PermitTCPFwdEntry *entry = (struct PermitTCPFwdEntry*)iter->item;
			if (strcmp(entry->host, host) == 0) {
				if ((entry->port == PUBKEY_OPTIONS_ANY_PORT) || (entry->port == port)) {
					return 1;
				}
			}

			iter = iter->next;
		}

		return 0;
	}

	return 1;
}

/* Set chansession command to the one forced 
 * by any 'command' public key option. */
void svr_pubkey_set_forced_command(struct ChanSess *chansess) {
	if (ses.authstate.pubkey_options && ses.authstate.pubkey_options->forced_command) {
		TRACE(("Forced command '%s'", ses.authstate.pubkey_options->forced_command))
		if (chansess->cmd) {
			/* original_command takes ownership */
			chansess->original_command = chansess->cmd;
			chansess->cmd = NULL;
		} else {
			chansess->original_command = m_strdup("");
		}
		chansess->cmd = m_strdup(ses.authstate.pubkey_options->forced_command);
#if LOG_COMMANDS
		dropbear_log(LOG_INFO, "Command forced to '%s'", chansess->original_command);
#endif
	}
}

/* Free potential public key options */
void svr_pubkey_options_cleanup() {
	if (ses.authstate.pubkey_options) {
		if (ses.authstate.pubkey_options->forced_command) {
			m_free(ses.authstate.pubkey_options->forced_command);
		}
		if (ses.authstate.pubkey_options->permit_open_destinations) {
			m_list_elem *iter = ses.authstate.pubkey_options->permit_open_destinations->first;
			while (iter) {
				struct PermitTCPFwdEntry *entry = (struct PermitTCPFwdEntry*)list_remove(iter);
				m_free(entry->host);
				m_free(entry);
				iter = ses.authstate.pubkey_options->permit_open_destinations->first;
			}
			m_free(ses.authstate.pubkey_options->permit_open_destinations);
		}
		m_free(ses.authstate.pubkey_options);
	}
	if (ses.authstate.pubkey_info) {
		m_free(ses.authstate.pubkey_info);
	}
}

/* helper for svr_add_pubkey_options. returns DROPBEAR_SUCCESS if the option is matched,
   and increments the options_buf */
static int match_option(buffer *options_buf, const char *opt_name) {
	const unsigned int len = strlen(opt_name);
	if (options_buf->len - options_buf->pos < len) {
		return DROPBEAR_FAILURE;
	}
	if (strncasecmp((const char *) buf_getptr(options_buf, len), opt_name, len) == 0) {
		buf_incrpos(options_buf, len);
		return DROPBEAR_SUCCESS;
	}
	return DROPBEAR_FAILURE;
}

/* Parse pubkey options and set ses.authstate.pubkey_options accordingly.
 * Returns DROPBEAR_SUCCESS if key is ok for auth, DROPBEAR_FAILURE otherwise */
int svr_add_pubkey_options(buffer *options_buf, int line_num, const char* filename) {
	int ret = DROPBEAR_FAILURE;

	TRACE(("enter addpubkeyoptions"))

	ses.authstate.pubkey_options = (struct PubKeyOptions*)m_malloc(sizeof( struct PubKeyOptions ));

	buf_setpos(options_buf, 0);
	while (options_buf->pos < options_buf->len) {
		if (match_option(options_buf, "no-port-forwarding") == DROPBEAR_SUCCESS) {
			dropbear_log(LOG_WARNING, "Port forwarding disabled.");
			ses.authstate.pubkey_options->no_port_forwarding_flag = 1;
			goto next_option;
		}
		if (match_option(options_buf, "no-agent-forwarding") == DROPBEAR_SUCCESS) {
#if DROPBEAR_SVR_AGENTFWD
			dropbear_log(LOG_WARNING, "Agent forwarding disabled.");
			ses.authstate.pubkey_options->no_agent_forwarding_flag = 1;
#endif
			goto next_option;
		}
		if (match_option(options_buf, "no-X11-forwarding") == DROPBEAR_SUCCESS) {
#if DROPBEAR_X11FWD
			dropbear_log(LOG_WARNING, "X11 forwarding disabled.");
			ses.authstate.pubkey_options->no_x11_forwarding_flag = 1;
#endif
			goto next_option;
		}
		if (match_option(options_buf, "no-pty") == DROPBEAR_SUCCESS) {
			dropbear_log(LOG_WARNING, "Pty allocation disabled.");
			ses.authstate.pubkey_options->no_pty_flag = 1;
			goto next_option;
		}
		if (match_option(options_buf, "restrict") == DROPBEAR_SUCCESS) {
			dropbear_log(LOG_WARNING, "Restrict option set");
			ses.authstate.pubkey_options->no_port_forwarding_flag = 1;
#if DROPBEAR_SVR_AGENTFWD
			ses.authstate.pubkey_options->no_agent_forwarding_flag = 1;
#endif
#if DROPBEAR_X11FWD
			ses.authstate.pubkey_options->no_x11_forwarding_flag = 1;
#endif
			ses.authstate.pubkey_options->no_pty_flag = 1;
			goto next_option;
		}
		if (match_option(options_buf, "command=\"") == DROPBEAR_SUCCESS) {
			int escaped = 0;
			const unsigned char* command_start = buf_getptr(options_buf, 0);

			if (ses.authstate.pubkey_options->forced_command) {
				/* multiple command= options */
				goto bad_option;
			}

			while (options_buf->pos < options_buf->len) {
				const char c = buf_getbyte(options_buf);
				if (!escaped && c == '"') {
					const int command_len = buf_getptr(options_buf, 0) - command_start;
					ses.authstate.pubkey_options->forced_command = m_malloc(command_len);
					memcpy(ses.authstate.pubkey_options->forced_command,
							command_start, command_len-1);
					ses.authstate.pubkey_options->forced_command[command_len-1] = '\0';
					goto next_option;
				}
				escaped = (!escaped && c == '\\');
			}
			dropbear_log(LOG_WARNING, "Badly formatted command= authorized_keys option");
			goto bad_option;
		}

		if (match_option(options_buf, "permitopen=\"") == DROPBEAR_SUCCESS) {
			int valid_option = 0;
			const unsigned char* permitopen_start = buf_getptr(options_buf, 0);

			if (!ses.authstate.pubkey_options->permit_open_destinations) {
				ses.authstate.pubkey_options->permit_open_destinations = list_new();
			}

			while (options_buf->pos < options_buf->len) {
				const char c = buf_getbyte(options_buf);
				if (c == '"') {
					char *spec = NULL;
					char *portstring = NULL;
					const int permitopen_len = buf_getptr(options_buf, 0) - permitopen_start;
					struct PermitTCPFwdEntry *entry =
							(struct PermitTCPFwdEntry*)m_malloc(sizeof(struct PermitTCPFwdEntry));

					list_append(ses.authstate.pubkey_options->permit_open_destinations, entry);
					spec = m_malloc(permitopen_len);
					memcpy(spec, permitopen_start, permitopen_len - 1);
					spec[permitopen_len - 1] = '\0';
					if ((split_address_port(spec, &entry->host, &portstring) == DROPBEAR_SUCCESS)
						&& entry->host && portstring) {
						if (strcmp(portstring, "*") == 0) {
							valid_option = 1;
							entry->port = PUBKEY_OPTIONS_ANY_PORT;
							TRACE(("local port forwarding allowed to host '%s'", entry->host));
						} else if (m_str_to_uint(portstring, &entry->port) == DROPBEAR_SUCCESS) {
							valid_option = 1;
							TRACE(("local port forwarding allowed to host '%s' and port '%u'",
									entry->host, entry->port));
						}
					}

					m_free(spec);
					m_free(portstring);
					break;
				}
			}

			if (valid_option) {
				goto next_option;
			} else {
				dropbear_log(LOG_WARNING, "Badly formatted permitopen= authorized_keys option");
				goto bad_option;
			}
		}

		if (match_option(options_buf, "no-touch-required") == DROPBEAR_SUCCESS) {
#if DROPBEAR_SK_ECDSA || DROPBEAR_SK_ED25519
			dropbear_log(LOG_WARNING, "No user presence check required for U2F/FIDO key.");
			ses.authstate.pubkey_options->no_touch_required_flag = 1;
#endif
			goto next_option;
		}
		if (match_option(options_buf, "verify-required") == DROPBEAR_SUCCESS) {
#if DROPBEAR_SK_ECDSA || DROPBEAR_SK_ED25519
			dropbear_log(LOG_WARNING, "User verification required for U2F/FIDO key.");
			ses.authstate.pubkey_options->verify_required_flag = 1;
#endif
			goto next_option;
		}

next_option:
		/*
		 * Skip the comma, and move to the next option
		 * (or break out if there are no more).
		 */
		if (options_buf->pos < options_buf->len 
				&& buf_getbyte(options_buf) != ',') {
			goto bad_option;
		}
		/* Process the next option. */
	}
	/* parsed all options with no problem */
	ret = DROPBEAR_SUCCESS;
	goto end;

bad_option:
	ret = DROPBEAR_FAILURE;
	svr_pubkey_options_cleanup();
	dropbear_log(LOG_WARNING, "Bad public key options at %s:%d", filename, line_num);

end:
	TRACE(("leave addpubkeyoptions"))
	return ret;
}

#endif
