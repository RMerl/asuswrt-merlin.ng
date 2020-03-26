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

/* This file (auth.c) handles authentication requests, passing it to the
 * particular type (auth-passwd, auth-pubkey). */

#include <limits.h>

#include "includes.h"
#include "dbutil.h"
#include "session.h"
#include "buffer.h"
#include "ssh.h"
#include "packet.h"
#include "auth.h"
#include "runopts.h"
#include "dbrandom.h"

static int checkusername(const char *username, unsigned int userlen);

/* initialise the first time for a session, resetting all parameters */
void svr_authinitialise() {
	memset(&ses.authstate, 0, sizeof(ses.authstate));
#if DROPBEAR_SVR_PUBKEY_AUTH
	ses.authstate.authtypes |= AUTH_TYPE_PUBKEY;
#endif
#if DROPBEAR_SVR_PASSWORD_AUTH || DROPBEAR_SVR_PAM_AUTH
	if (!svr_opts.noauthpass) {
		ses.authstate.authtypes |= AUTH_TYPE_PASSWORD;
	}
#endif
}

/* Send a banner message if specified to the client. The client might
 * ignore this, but possibly serves as a legal "no trespassing" sign */
void send_msg_userauth_banner(const buffer *banner) {

	TRACE(("enter send_msg_userauth_banner"))

	CHECKCLEARTOWRITE();

	buf_putbyte(ses.writepayload, SSH_MSG_USERAUTH_BANNER);
	buf_putbufstring(ses.writepayload, banner);
	buf_putstring(ses.writepayload, "en", 2);

	encrypt_packet();

	TRACE(("leave send_msg_userauth_banner"))
}

/* handle a userauth request, check validity, pass to password or pubkey
 * checking, and handle success or failure */
void recv_msg_userauth_request() {

	char *username = NULL, *servicename = NULL, *methodname = NULL;
	unsigned int userlen, servicelen, methodlen;
	int valid_user = 0;

	TRACE(("enter recv_msg_userauth_request"))

	/* for compensating failure delay */
	gettime_wrapper(&ses.authstate.auth_starttime);

	/* ignore packets if auth is already done */
	if (ses.authstate.authdone == 1) {
		TRACE(("leave recv_msg_userauth_request: authdone already"))
		return;
	}

	/* send the banner if it exists, it will only exist once */
	if (svr_opts.banner) {
		send_msg_userauth_banner(svr_opts.banner);
		buf_free(svr_opts.banner);
		svr_opts.banner = NULL;
	}

	username = buf_getstring(ses.payload, &userlen);
	servicename = buf_getstring(ses.payload, &servicelen);
	methodname = buf_getstring(ses.payload, &methodlen);

	/* only handle 'ssh-connection' currently */
	if (servicelen != SSH_SERVICE_CONNECTION_LEN
			&& (strncmp(servicename, SSH_SERVICE_CONNECTION,
					SSH_SERVICE_CONNECTION_LEN) != 0)) {
		
		/* TODO - disconnect here */
		m_free(username);
		m_free(servicename);
		m_free(methodname);
		dropbear_exit("unknown service in auth");
	}

	/* check username is good before continuing. 
	 * the 'incrfail' varies depending on the auth method to
	 * avoid giving away which users exist on the system through
	 * the time delay. */
	if (checkusername(username, userlen) == DROPBEAR_SUCCESS) {
		valid_user = 1;
	}

	/* user wants to know what methods are supported */
	if (methodlen == AUTH_METHOD_NONE_LEN &&
			strncmp(methodname, AUTH_METHOD_NONE,
				AUTH_METHOD_NONE_LEN) == 0) {
		TRACE(("recv_msg_userauth_request: 'none' request"))
		if (valid_user
				&& svr_opts.allowblankpass
				&& !svr_opts.noauthpass
				&& !(svr_opts.norootpass && ses.authstate.pw_uid == 0) 
				&& ses.authstate.pw_passwd[0] == '\0') 
		{
			dropbear_log(LOG_NOTICE, 
					"Auth succeeded with blank password for '%s' from %s",
					ses.authstate.pw_name,
					svr_ses.addrstring);
#ifdef SECURITY_NOTIFY
			SEND_PTCSRV_EVENT(PROTECTION_SERVICE_SSH,
					RPT_SUCCESS, svr_ses.hoststring,
					"From dropbear , LOGIN SUCCESS(authnone)");
#endif
			send_msg_userauth_success();
			goto out;
		}
		else
		{
			/* 'none' has no failure delay */
			send_msg_userauth_failure(0, 0);
			goto out;
		}
	}
	
#if DROPBEAR_SVR_PASSWORD_AUTH
	if (!svr_opts.noauthpass &&
			!(svr_opts.norootpass && ses.authstate.pw_uid == 0) ) {
		/* user wants to try password auth */
		if (methodlen == AUTH_METHOD_PASSWORD_LEN &&
				strncmp(methodname, AUTH_METHOD_PASSWORD,
					AUTH_METHOD_PASSWORD_LEN) == 0) {
			svr_auth_password(valid_user);
			goto out;
		}
	}
#endif

#if DROPBEAR_SVR_PAM_AUTH
	if (!svr_opts.noauthpass &&
			!(svr_opts.norootpass && ses.authstate.pw_uid == 0) ) {
		/* user wants to try password auth */
		if (methodlen == AUTH_METHOD_PASSWORD_LEN &&
				strncmp(methodname, AUTH_METHOD_PASSWORD,
					AUTH_METHOD_PASSWORD_LEN) == 0) {
			svr_auth_pam(valid_user);
			goto out;
		}
	}
#endif

#if DROPBEAR_SVR_PUBKEY_AUTH
	/* user wants to try pubkey auth */
	if (methodlen == AUTH_METHOD_PUBKEY_LEN &&
			strncmp(methodname, AUTH_METHOD_PUBKEY,
				AUTH_METHOD_PUBKEY_LEN) == 0) {
		svr_auth_pubkey(valid_user);
		goto out;
	}
#endif

	/* nothing matched, we just fail with a delay */
#ifdef SECURITY_NOTIFY
	SEND_PTCSRV_EVENT(PROTECTION_SERVICE_SSH,
			RPT_FAIL, svr_ses.hoststring,
			"From dropbear , ACCOUNT FAIL");
#endif
	send_msg_userauth_failure(0, 1);

out:

	m_free(username);
	m_free(servicename);
	m_free(methodname);
}

#ifdef HAVE_GETGROUPLIST
/* returns DROPBEAR_SUCCESS or DROPBEAR_FAILURE */
static int check_group_membership(gid_t check_gid, const char* username, gid_t user_gid) {
	int ngroups, i, ret;
	gid_t *grouplist = NULL;
	int match = DROPBEAR_FAILURE;

	for (ngroups = 32; ngroups <= DROPBEAR_NGROUP_MAX; ngroups *= 2) {
		grouplist = m_malloc(sizeof(gid_t) * ngroups);

		/* BSD returns ret==0 on success. Linux returns ret==ngroups on success */
		ret = getgrouplist(username, user_gid, grouplist, &ngroups);
		if (ret >= 0) {
			break;
		}
		m_free(grouplist);
		grouplist = NULL;
	}

	if (!grouplist) {
		dropbear_log(LOG_ERR, "Too many groups for user '%s'", username);
		return DROPBEAR_FAILURE;
	}

	for (i = 0; i < ngroups; i++) {
		if (grouplist[i] == check_gid) {
			match = DROPBEAR_SUCCESS;
			break;
		}
	}
	m_free(grouplist);

	return match;
}
#endif

/* Check that the username exists and isn't disallowed (root), and has a valid shell.
 * returns DROPBEAR_SUCCESS on valid username, DROPBEAR_FAILURE on failure */
static int checkusername(const char *username, unsigned int userlen) {

	char* listshell = NULL;
	char* usershell = NULL;
	uid_t uid;

	TRACE(("enter checkusername"))
	if (userlen > MAX_USERNAME_LEN) {
		return DROPBEAR_FAILURE;
	}

	if (strlen(username) != userlen) {
		dropbear_exit("Attempted username with a null byte from %s",
			svr_ses.addrstring);
	}

	if (ses.authstate.username == NULL) {
		/* first request */
		fill_passwd(username);
		ses.authstate.username = m_strdup(username);
	} else {
		/* check username hasn't changed */
		if (strcmp(username, ses.authstate.username) != 0) {
			dropbear_exit("Client trying multiple usernames from %s",
				svr_ses.addrstring);
		}
	}

	/* avoids cluttering logs with repeated failure messages from
	consecutive authentication requests in a sesssion */
	if (ses.authstate.checkusername_failed) {
		TRACE(("checkusername: returning cached failure"))
		return DROPBEAR_FAILURE;
	}

	/* check that user exists */
	if (!ses.authstate.pw_name) {
		TRACE(("leave checkusername: user '%s' doesn't exist", username))
		dropbear_log(LOG_WARNING,
				"Login attempt for nonexistent user from %s",
				svr_ses.addrstring);
		ses.authstate.checkusername_failed = 1;
		return DROPBEAR_FAILURE;
	}

	/* check if we are running as non-root, and login user is different from the server */
	uid = geteuid();
	if (!(DROPBEAR_SVR_MULTIUSER && uid == 0) && uid != ses.authstate.pw_uid) {
		TRACE(("running as nonroot, only server uid is allowed"))
		dropbear_log(LOG_WARNING,
				"Login attempt with wrong user %s from %s",
				ses.authstate.pw_name,
				svr_ses.addrstring);
		ses.authstate.checkusername_failed = 1;
		return DROPBEAR_FAILURE;
	}

	/* check for non-root if desired */
	if (svr_opts.norootlogin && ses.authstate.pw_uid == 0) {
		TRACE(("leave checkusername: root login disabled"))
		dropbear_log(LOG_WARNING, "root login rejected");
		ses.authstate.checkusername_failed = 1;
		return DROPBEAR_FAILURE;
	}

	/* check for login restricted to certain group if desired */
#ifdef HAVE_GETGROUPLIST
	if (svr_opts.restrict_group) {
		if (check_group_membership(svr_opts.restrict_group_gid,
				ses.authstate.pw_name, ses.authstate.pw_gid) == DROPBEAR_FAILURE) {
			dropbear_log(LOG_WARNING,
				"Logins are restricted to the group %s but user '%s' is not a member",
				svr_opts.restrict_group, ses.authstate.pw_name);
			ses.authstate.checkusername_failed = 1;
			return DROPBEAR_FAILURE;
		}
	}
#endif /* HAVE_GETGROUPLIST */

	TRACE(("shell is %s", ses.authstate.pw_shell))

	/* check that the shell is set */
	usershell = ses.authstate.pw_shell;
	if (usershell[0] == '\0') {
		/* empty shell in /etc/passwd means /bin/sh according to passwd(5) */
		usershell = "/bin/sh";
	}

	/* check the shell is valid. If /etc/shells doesn't exist, getusershell()
	 * should return some standard shells like "/bin/sh" and "/bin/csh" (this
	 * is platform-specific) */
	setusershell();
	while ((listshell = getusershell()) != NULL) {
		TRACE(("test shell is '%s'", listshell))
		if (strcmp(listshell, usershell) == 0) {
			/* have a match */
			goto goodshell;
		}
	}
	/* no matching shell */
	endusershell();
	TRACE(("no matching shell"))
	ses.authstate.checkusername_failed = 1;
	dropbear_log(LOG_WARNING, "User '%s' has invalid shell, rejected",
				ses.authstate.pw_name);
	return DROPBEAR_FAILURE;
	
goodshell:
	endusershell();
	TRACE(("matching shell"))

	TRACE(("uid = %d", ses.authstate.pw_uid))
	TRACE(("leave checkusername"))
	return DROPBEAR_SUCCESS;
}

/* Send a failure message to the client, in responds to a userauth_request.
 * Partial indicates whether to set the "partial success" flag,
 * incrfail is whether to count this failure in the failure count (which
 * is limited. This function also handles disconnection after too many
 * failures */
void send_msg_userauth_failure(int partial, int incrfail) {

	buffer *typebuf = NULL;

	TRACE(("enter send_msg_userauth_failure"))

	CHECKCLEARTOWRITE();
	
	buf_putbyte(ses.writepayload, SSH_MSG_USERAUTH_FAILURE);

	/* put a list of allowed types */
	typebuf = buf_new(30); /* long enough for PUBKEY and PASSWORD */

	if (ses.authstate.authtypes & AUTH_TYPE_PUBKEY) {
		buf_putbytes(typebuf, (const unsigned char *)AUTH_METHOD_PUBKEY, AUTH_METHOD_PUBKEY_LEN);
		if (ses.authstate.authtypes & AUTH_TYPE_PASSWORD) {
			buf_putbyte(typebuf, ',');
		}
	}
	
	if (ses.authstate.authtypes & AUTH_TYPE_PASSWORD) {
		buf_putbytes(typebuf, (const unsigned char *)AUTH_METHOD_PASSWORD, AUTH_METHOD_PASSWORD_LEN);
	}

	buf_putbufstring(ses.writepayload, typebuf);

	TRACE(("auth fail: methods %d, '%.*s'", ses.authstate.authtypes,
				typebuf->len, typebuf->data))

	buf_free(typebuf);

	buf_putbyte(ses.writepayload, partial ? 1 : 0);
	encrypt_packet();

	if (incrfail) {
		/* The SSH_MSG_AUTH_FAILURE response is delayed to attempt to
		avoid user enumeration and slow brute force attempts.
		The delay is adjusted by the time already spent in processing
		authentication (ses.authstate.auth_starttime timestamp). */

		/* Desired total delay 300ms +-50ms (in nanoseconds).
		Beware of integer overflow if increasing these values */
		const unsigned int mindelay = 250000000;
		const unsigned int vardelay = 100000000;
		unsigned int rand_delay;
		struct timespec delay;

		gettime_wrapper(&delay);
		delay.tv_sec -= ses.authstate.auth_starttime.tv_sec;
		delay.tv_nsec -= ses.authstate.auth_starttime.tv_nsec;

		/* carry */
		if (delay.tv_nsec < 0) {
			delay.tv_nsec += 1000000000;
			delay.tv_sec -= 1;
		}

		genrandom((unsigned char*)&rand_delay, sizeof(rand_delay));
		rand_delay = mindelay + (rand_delay % vardelay);

		if (delay.tv_sec == 0 && delay.tv_nsec <= mindelay) {
			/* Compensate for elapsed time */
			delay.tv_nsec = rand_delay - delay.tv_nsec;
		} else {
			/* No time left or time went backwards, just delay anyway */
			delay.tv_sec = 0;
			delay.tv_nsec = rand_delay;
		}


#if DROPBEAR_FUZZ
		if (!fuzz.fuzzing)
#endif
		{
			while (nanosleep(&delay, &delay) == -1 && errno == EINTR) { /* Go back to sleep */ }
		}

		ses.authstate.failcount++;
	}

	if (ses.authstate.failcount >= svr_opts.maxauthtries) {
		char * userstr;
		/* XXX - send disconnect ? */
		TRACE(("Max auth tries reached, exiting"))

		if (ses.authstate.pw_name == NULL) {
			userstr = "is invalid";
		} else {
			userstr = ses.authstate.pw_name;
		}
		dropbear_exit("Max auth tries reached - user '%s' from %s",
				userstr, svr_ses.addrstring);
	}
	
	TRACE(("leave send_msg_userauth_failure"))
}

/* Send a success message to the user, and set the "authdone" flag */
void send_msg_userauth_success() {

	TRACE(("enter send_msg_userauth_success"))

	CHECKCLEARTOWRITE();

	buf_putbyte(ses.writepayload, SSH_MSG_USERAUTH_SUCCESS);
	encrypt_packet();

	/* authdone must be set after encrypt_packet() for 
	 * delayed-zlib mode */
	ses.authstate.authdone = 1;
	ses.connect_time = 0;


	if (ses.authstate.pw_uid == 0) {
		ses.allowprivport = 1;
	}

	/* Remove from the list of pre-auth sockets. Should be m_close(), since if
	 * we fail, we might end up leaking connection slots, and disallow new
	 * logins - a nasty situation. */							
	m_close(svr_ses.childpipe);

	TRACE(("leave send_msg_userauth_success"))

}
