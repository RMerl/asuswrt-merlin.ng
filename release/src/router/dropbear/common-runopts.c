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
#include "auth.h"
#include "algo.h"
#include "dbrandom.h"

runopts opts; /* GLOBAL */

/* returns success or failure, and the keytype in *type. If we want
 * to restrict the type, type can contain a type to return */
int readhostkey(const char * filename, sign_key * hostkey,
	enum signkey_type *type) {

	int ret = DROPBEAR_FAILURE;
	buffer *buf;

	buf = buf_new(MAX_PRIVKEY_SIZE);

	if (buf_readfile(buf, filename) == DROPBEAR_FAILURE) {
		goto out;
	}
	buf_setpos(buf, 0);

	addrandom(buf_getptr(buf, buf->len), buf->len);

	if (buf_get_priv_key(buf, hostkey, type) == DROPBEAR_FAILURE) {
		goto out;
	}

	ret = DROPBEAR_SUCCESS;
out:

	buf_burn_free(buf);
	return ret;
}

#if DROPBEAR_USER_ALGO_LIST
void
parse_ciphers_macs() {
	int printed_help = 0;
	if (opts.cipher_list) {
		if (strcmp(opts.cipher_list, "help") == 0) {
			char *ciphers = algolist_string(sshciphers);
			dropbear_log(LOG_INFO, "Available ciphers: %s", ciphers);
			m_free(ciphers);
			printed_help = 1;
		} else {
			if (check_user_algos(opts.cipher_list, sshciphers, "cipher") == 0) {
				dropbear_exit("No valid ciphers specified for '-c'");
			}
		}
	}

	if (opts.mac_list) {
		if (strcmp(opts.mac_list, "help") == 0) {
			char *macs = algolist_string(sshhashes);
			dropbear_log(LOG_INFO, "Available MACs: %s", macs);
			m_free(macs);
			printed_help = 1;
		} else {
			if (check_user_algos(opts.mac_list, sshhashes, "MAC") == 0) {
				dropbear_exit("No valid MACs specified for '-m'");
			}
		}
	}
	if (printed_help) {
		dropbear_exit(".");
	}
}
#endif

void print_version() {
	fprintf(stderr, "Dropbear v%s\n", DROPBEAR_VERSION);
}

void parse_recv_window(const char* recv_window_arg) {
	int ret;
	unsigned int rw;

	ret = m_str_to_uint(recv_window_arg, &rw);
	if (ret == DROPBEAR_FAILURE || rw == 0 || rw > MAX_RECV_WINDOW) {
		if (rw > MAX_RECV_WINDOW) {
			opts.recv_window = MAX_RECV_WINDOW;
		}
		dropbear_log(LOG_WARNING, "Bad recv window '%s', using %d",
			recv_window_arg, opts.recv_window);
	} else {
		opts.recv_window = rw;
	}

}

/* Splits addr:port. Handles IPv6 [2001:0011::4]:port style format.
   Returns first/second parts as malloced strings, second will
   be NULL if no separator is found.
   :port  ->  (NULL, "port")
   port  ->   (port, NULL)
   addr:port  (addr, port)
   addr: ->   (addr, "")
   Returns DROPBEAR_SUCCESS/DROPBEAR_FAILURE */
int split_address_port(const char* spec, char **first, char ** second) {
	char *spec_copy = NULL, *addr = NULL, *colon = NULL;
	int ret = DROPBEAR_FAILURE;

	*first = NULL;
	*second = NULL;
	spec_copy = m_strdup(spec);
	addr = spec_copy;

	if (*addr == '[') {
		addr++;
		colon = strchr(addr, ']');
		if (!colon) {
			dropbear_log(LOG_WARNING, "Bad address '%s'", spec);
			goto out;
		}
		*colon = '\0';
		colon++;
		if (*colon == '\0') {
			/* No port part */
			colon = NULL;
		} else if (*colon != ':') {
			dropbear_log(LOG_WARNING, "Bad address '%s'", spec);
			goto out;
		}
	} else {
		/* search for ':', that separates address and port */
		colon = strrchr(addr, ':');
	}

	/* colon points to ':' now, or is NULL */
	if (colon) {
		/* Split the address/port */
		*colon = '\0';
		colon++;
		*second = m_strdup(colon);
	}
	if (strlen(addr)) {
		*first = m_strdup(addr);
	}
	ret = DROPBEAR_SUCCESS;

out:
	m_free(spec_copy);
	return ret;
}
