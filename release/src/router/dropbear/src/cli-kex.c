/*
 * Dropbear - a SSH2 server
 * 
 * Copyright (c) 2002-2004 Matt Johnston
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
#include "session.h"
#include "dbutil.h"
#include "algo.h"
#include "buffer.h"
#include "session.h"
#include "kex.h"
#include "ssh.h"
#include "packet.h"
#include "bignum.h"
#include "dbrandom.h"
#include "runopts.h"
#include "signkey.h"
#include "ecc.h"


static void checkhostkey(const unsigned char* keyblob, unsigned int keybloblen);
#define MAX_KNOWNHOSTS_LINE 4500

static void cli_kex_free_param(void) {
#if DROPBEAR_NORMAL_DH
	if (cli_ses.dh_param) {
		free_kexdh_param(cli_ses.dh_param);
		cli_ses.dh_param = NULL;
	}
#endif
#if DROPBEAR_ECDH
	if (cli_ses.ecdh_param) {
		free_kexecdh_param(cli_ses.ecdh_param);
		cli_ses.ecdh_param = NULL;
	}
#endif
#if DROPBEAR_CURVE25519
	if (cli_ses.curve25519_param) {
		free_kexcurve25519_param(cli_ses.curve25519_param);
		cli_ses.curve25519_param = NULL;
	}
#endif
#if DROPBEAR_PQHYBRID
	if (cli_ses.pqhybrid_param) {
		free_kexpqhybrid_param(cli_ses.pqhybrid_param);
		cli_ses.pqhybrid_param = NULL;
	}
#endif
}

void send_msg_kexdh_init() {
	TRACE(("send_msg_kexdh_init()"))	

	CHECKCLEARTOWRITE();

#if DROPBEAR_FUZZ
	if (fuzz.fuzzing && fuzz.skip_kexmaths) {
		return;
	}
#endif

	cli_kex_free_param();

	buf_putbyte(ses.writepayload, SSH_MSG_KEXDH_INIT);
	switch (ses.newkeys->algo_kex->mode) {
#if DROPBEAR_NORMAL_DH
		case DROPBEAR_KEX_NORMAL_DH:
			cli_ses.dh_param = gen_kexdh_param();
			buf_putmpint(ses.writepayload, &cli_ses.dh_param->pub);
			break;
#endif
#if DROPBEAR_ECDH
		case DROPBEAR_KEX_ECDH:
			cli_ses.ecdh_param = gen_kexecdh_param();
			buf_put_ecc_raw_pubkey_string(ses.writepayload, &cli_ses.ecdh_param->key);
			break;
#endif
#if DROPBEAR_CURVE25519
		case DROPBEAR_KEX_CURVE25519:
			cli_ses.curve25519_param = gen_kexcurve25519_param();
			buf_putstring(ses.writepayload, cli_ses.curve25519_param->pub, CURVE25519_LEN);
			break;
#endif
#if DROPBEAR_PQHYBRID
		case DROPBEAR_KEX_PQHYBRID:
			cli_ses.pqhybrid_param = gen_kexpqhybrid_param();
			buf_putbufstring(ses.writepayload, cli_ses.pqhybrid_param->concat_public);
			break;
#endif
	}

	encrypt_packet();
}

/* Handle a diffie-hellman key exchange reply. */
void recv_msg_kexdh_reply() {

	sign_key *hostkey = NULL;
	unsigned int keytype, keybloblen;
	unsigned char* keyblob = NULL;

	TRACE(("enter recv_msg_kexdh_reply"))
	
#if DROPBEAR_FUZZ
	if (fuzz.fuzzing && fuzz.skip_kexmaths) {
		return;
	}
#endif

	if (cli_ses.kex_state != KEXDH_INIT_SENT) {
		dropbear_exit("Received out-of-order kexdhreply");
	}
	keytype = ses.newkeys->algo_hostkey;
	TRACE(("keytype is %d", keytype))

	hostkey = new_sign_key();
	keybloblen = buf_getint(ses.payload);

	keyblob = buf_getptr(ses.payload, keybloblen);
	if (!ses.kexstate.donefirstkex) {
		/* Only makes sense the first time */
		checkhostkey(keyblob, keybloblen);
	}

	if (buf_get_pub_key(ses.payload, hostkey, &keytype) != DROPBEAR_SUCCESS) {
		TRACE(("failed getting pubkey"))
		dropbear_exit("Bad KEX packet");
	}

	/* Derive the shared secret */
	switch (ses.newkeys->algo_kex->mode) {
#if DROPBEAR_NORMAL_DH
		case DROPBEAR_KEX_NORMAL_DH:
			{
			DEF_MP_INT(dh_f);
			m_mp_init(&dh_f);
			if (buf_getmpint(ses.payload, &dh_f) != DROPBEAR_SUCCESS) {
				TRACE(("failed getting mpint"))
				dropbear_exit("Bad KEX packet");
			}

			kexdh_comb_key(cli_ses.dh_param, &dh_f, hostkey);
			mp_clear(&dh_f);
			}
			break;
#endif
#if DROPBEAR_ECDH
		case DROPBEAR_KEX_ECDH:
			{
			buffer *ecdh_qs = buf_getstringbuf(ses.payload);
			kexecdh_comb_key(cli_ses.ecdh_param, ecdh_qs, hostkey);
			buf_free(ecdh_qs);
			}
			break;
#endif
#if DROPBEAR_CURVE25519
		case DROPBEAR_KEX_CURVE25519:
			{
			buffer *ecdh_qs = buf_getstringbuf(ses.payload);
			kexcurve25519_comb_key(cli_ses.curve25519_param, ecdh_qs, hostkey);
			buf_free(ecdh_qs);
			}
			break;
#endif
#if DROPBEAR_PQHYBRID
		case DROPBEAR_KEX_PQHYBRID:
			{
			buffer *q_s = buf_getstringbuf(ses.payload);
			kexpqhybrid_comb_key(cli_ses.pqhybrid_param, q_s, hostkey);
			buf_free(q_s);
			}
			break;
#endif
	}

	/* Clear the local parameter */
	cli_kex_free_param();

	if (buf_verify(ses.payload, hostkey, ses.newkeys->algo_signature, 
			ses.hash) != DROPBEAR_SUCCESS) {
		dropbear_exit("Bad hostkey signature");
	}

	sign_key_free(hostkey);
	hostkey = NULL;

	send_msg_newkeys();
	ses.requirenext = SSH_MSG_NEWKEYS;
	TRACE(("leave recv_msg_kexdh_init"))
}

static void ask_to_confirm(const unsigned char* keyblob, unsigned int keybloblen,
	const char* algoname) {

	char* fp = NULL;
	FILE *tty = NULL;
	int response = 'z';

	fp = sign_key_fingerprint(keyblob, keybloblen);

	if (!cli_opts.ask_hostkey) {
		dropbear_log(LOG_INFO, "\nHost '%s' key unknown.\n(%s fingerprint %s)",
				cli_opts.remotehost,
				algoname,
				fp);
		dropbear_exit("Not accepted automatically");
	}

	if (cli_opts.always_accept_key) {
		dropbear_log(LOG_INFO, "\nHost '%s' key accepted unconditionally.\n(%s fingerprint %s)\n",
				cli_opts.remotehost,
				algoname,
				fp);
		m_free(fp);
		return;
	}

	fprintf(stderr, "\nHost '%s' is not in the trusted hosts file.\n(%s fingerprint %s)\n",
			cli_opts.remotehost, 
			algoname,
			fp);
	m_free(fp);
	if (cli_opts.batch_mode) {
		dropbear_exit("Didn't validate host key");
	}

	fprintf(stderr, "Do you want to continue connecting? (y/n) ");
	tty = fopen(_PATH_TTY, "r");
	if (tty) {
		response = getc(tty);
		fclose(tty);
	} else {
		response = getc(stdin);
		/* flush stdin buffer */
		while ((getchar()) != '\n');
	}

	if (response == 'y') {
		return;
	}

	dropbear_exit("Didn't validate host key");
}

static FILE* open_known_hosts_file(int * readonly)
{
	FILE * hostsfile = NULL;
	char * filename = NULL;
	char * homedir = NULL;
	
	homedir = getenv("HOME");

	if (!homedir) {
		struct passwd * pw = NULL;
		pw = getpwuid(getuid());
		if (pw) {
			homedir = pw->pw_dir;
		}
	}

	if (homedir) {
		unsigned int len;
		len = strlen(homedir);
		filename = m_malloc(len + 18); /* "/.ssh/known_hosts" and null-terminator*/

		snprintf(filename, len+18, "%s/.ssh", homedir);
		/* Check that ~/.ssh exists - easiest way is just to mkdir */
		if (mkdir(filename, S_IRWXU) != 0) {
			if (errno != EEXIST) {
				dropbear_log(LOG_INFO, "Warning: failed creating %s/.ssh: %s",
						homedir, strerror(errno));
				TRACE(("mkdir didn't work: %s", strerror(errno)))
				goto out;
			}
		}

		snprintf(filename, len+18, "%s/.ssh/known_hosts", homedir);
		hostsfile = fopen(filename, "a+");
		
		if (hostsfile != NULL) {
			*readonly = 0;
			fseek(hostsfile, 0, SEEK_SET);
		} else {
			/* We mightn't have been able to open it if it was read-only */
			if (errno == EACCES || errno == EROFS) {
					TRACE(("trying readonly: %s", strerror(errno)))
					*readonly = 1;
					hostsfile = fopen(filename, "r");
			}
		}
	}

	if (hostsfile == NULL) {
		TRACE(("hostsfile didn't open: %s", strerror(errno)))
		dropbear_log(LOG_WARNING, "Failed to open %s/.ssh/known_hosts",
				homedir);
		goto out;
	}	

out:
	m_free(filename);
	return hostsfile;
}

static void checkhostkey(const unsigned char* keyblob, unsigned int keybloblen) {

	FILE *hostsfile = NULL;
	int readonly = 0;
	unsigned int hostlen, algolen;
	unsigned long len;
	const char *algoname = NULL;
	char * fingerprint = NULL;
	buffer * line = NULL;
	int ret;

	if (cli_opts.no_hostkey_check) {
		dropbear_log(LOG_INFO, "Caution, skipping hostkey check for %s\n", cli_opts.remotehost);
		return;
	}

	algoname = signkey_name_from_type(ses.newkeys->algo_hostkey, &algolen);

	hostsfile = open_known_hosts_file(&readonly);
	if (!hostsfile)	{
		ask_to_confirm(keyblob, keybloblen, algoname);
		/* ask_to_confirm will exit upon failure */
		return;
	}
	
	line = buf_new(MAX_KNOWNHOSTS_LINE);
	hostlen = strlen(cli_opts.remotehost);

	do {
		if (buf_getline(line, hostsfile) == DROPBEAR_FAILURE) {
			TRACE(("failed reading line: prob EOF"))
			break;
		}

		/* The line is too short to be sensible */
		/* "30" is 'enough to hold ssh-dss plus the spaces, ie so we don't
		 * buf_getfoo() past the end and die horribly - the base64 parsing
		 * code is what tiptoes up to the end nicely */
		if (line->len < (hostlen+30) ) {
			TRACE(("line is too short to be sensible"))
			continue;
		}

		/* Compare hostnames */
		if (strncmp(cli_opts.remotehost, (const char *) buf_getptr(line, hostlen),
					hostlen) != 0) {
			continue;
		}

		buf_incrpos(line, hostlen);
		if (buf_getbyte(line) != ' ') {
			/* there wasn't a space after the hostname, something dodgy */
			TRACE(("missing space afte matching hostname"))
			continue;
		}

		if (strncmp((const char *) buf_getptr(line, algolen), algoname, algolen) != 0) {
			TRACE(("algo doesn't match"))
			continue;
		}

		buf_incrpos(line, algolen);
		if (buf_getbyte(line) != ' ') {
			TRACE(("missing space after algo"))
			continue;
		}

		/* Now we're at the interesting hostkey */
		ret = cmp_base64_key(keyblob, keybloblen, (const unsigned char *) algoname, algolen,
						line, &fingerprint);

		if (ret == DROPBEAR_SUCCESS) {
			/* Good matching key */
			DEBUG1(("server match %s", fingerprint))
			goto out;
		}

		/* The keys didn't match. eep. Note that we're "leaking"
		   the fingerprint strings here, but we're exiting anyway */
		dropbear_exit("\n\n%s host key mismatch for %s !\n"
					"Fingerprint is %s\n"
					"Expected %s\n"
					"If you know that the host key is correct you can\nremove the bad entry from ~/.ssh/known_hosts", 
					algoname,
					cli_opts.remotehost,
					sign_key_fingerprint(keyblob, keybloblen),
					fingerprint ? fingerprint : "UNKNOWN");
	} while (1); /* keep going 'til something happens */

	/* Key doesn't exist yet */
	ask_to_confirm(keyblob, keybloblen, algoname);

	/* If we get here, they said yes */

	if (readonly) {
		TRACE(("readonly"))
		goto out;
	}

	if (!cli_opts.no_hostkey_check) {
		/* put the new entry in the file */
		fseek(hostsfile, 0, SEEK_END); /* In case it wasn't opened append */
		buf_setpos(line, 0);
		buf_setlen(line, 0);
		buf_putbytes(line, (const unsigned char *) cli_opts.remotehost, hostlen);
		buf_putbyte(line, ' ');
		buf_putbytes(line, (const unsigned char *) algoname, algolen);
		buf_putbyte(line, ' ');
		len = line->size - line->pos;
		/* The only failure with base64 is buffer_overflow, but buf_getwriteptr
		 * will die horribly in the case anyway */
		base64_encode(keyblob, keybloblen, buf_getwriteptr(line, len), &len);
		buf_incrwritepos(line, len);
		buf_putbyte(line, '\n');
		buf_setpos(line, 0);
		fwrite(buf_getptr(line, line->len), line->len, 1, hostsfile);
		/* We ignore errors, since there's not much we can do about them */
	}

out:
	if (hostsfile != NULL) {
		fclose(hostsfile);
	}
	if (line != NULL) {
		buf_free(line);
	}
	m_free(fingerprint);
}

void recv_msg_ext_info(void) {
	/* This message is not client-specific in the protocol but Dropbear only handles
	a server-sent message at present. */
	unsigned int num_ext;
	unsigned int i;

	TRACE(("enter recv_msg_ext_info"))

	/* Must be after the first SSH_MSG_NEWKEYS */
	TRACE(("last %d, donefirst %d, donescond %d", ses.lastpacket, ses.kexstate.donefirstkex, ses.kexstate.donesecondkex))
	if (!(ses.lastpacket == SSH_MSG_NEWKEYS && !ses.kexstate.donesecondkex)) {
		TRACE(("leave recv_msg_ext_info: ignoring packet received at the wrong time"))
		return;
	}

	num_ext = buf_getint(ses.payload);
	TRACE(("received SSH_MSG_EXT_INFO with %d items", num_ext))

	for (i = 0; i < num_ext; i++) {
		unsigned int name_len;
		char *ext_name = buf_getstring(ses.payload, &name_len);
		TRACE(("extension %d name '%s'", i, ext_name))
		if (cli_ses.server_sig_algs == NULL
				&& name_len == strlen(SSH_SERVER_SIG_ALGS)
				&& strcmp(ext_name, SSH_SERVER_SIG_ALGS) == 0) {
			cli_ses.server_sig_algs = buf_getbuf(ses.payload);
		} else {
			/* valid extension values could be >MAX_STRING_LEN */
			buf_eatstring(ses.payload);
		}
		m_free(ext_name);
	}
	TRACE(("leave recv_msg_ext_info"))
}
