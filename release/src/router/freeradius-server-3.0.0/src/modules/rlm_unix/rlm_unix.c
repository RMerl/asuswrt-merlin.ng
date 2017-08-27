/*
 *   This program is is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License, version 2 if the
 *   License as published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/**
 * $Id$
 * @file rlm_unix.c
 * @brief Unixy things
 *
 * authentication: Unix user authentication
 * accounting:     Functions to write radwtmp file.
 * Also contains handler for "Group".
 *
 * @copyright 2000,2006  The FreeRADIUS server project
 * @copyright 2000  Jeff Carneal <jeff@apex.net>
 * @copyright 2000  Alan Curry <pacman@world.std.com>
 */
RCSID("$Id$")

#include	<freeradius-devel/radiusd.h>

#include	<grp.h>
#include	<pwd.h>
#include	<sys/stat.h>

#include "config.h"

#ifdef HAVE_SHADOW_H
#  include	<shadow.h>
#endif

#ifdef OSFC2
#  include	<sys/security.h>
#  include	<prot.h>
#endif

#ifdef OSFSIA
#  include	<sia.h>
#  include	<siad.h>
#endif

#include	<freeradius-devel/modules.h>
#include	<freeradius-devel/sysutmp.h>

static char trans[64] =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
#define ENC(c) trans[c]

struct unix_instance {
	char const *radwtmp;
};

static const CONF_PARSER module_config[] = {
	{ "radwtmp",  PW_TYPE_FILE_OUTPUT | PW_TYPE_REQUIRED,
	  offsetof(struct unix_instance,radwtmp), NULL,   "NULL" },

	{ NULL, -1, 0, NULL, NULL }		/* end the list */
};


/*
 *	The Group = handler.
 */
static int groupcmp(UNUSED void *instance, REQUEST *req, UNUSED VALUE_PAIR *request,
		    VALUE_PAIR *check, UNUSED VALUE_PAIR *check_pairs,
		    UNUSED VALUE_PAIR **reply_pairs)
{
	struct passwd	*pwd;
	struct group	*grp;
	char		**member;
	int		retval;

	/*
	 *	No user name, doesn't compare.
	 */
	if (!req->username) {
		return -1;
	}

	pwd = getpwnam(req->username->vp_strvalue);
	if (!pwd)
		return -1;

	grp = getgrnam(check->vp_strvalue);
	if (!grp)
		return -1;

	retval = (pwd->pw_gid == grp->gr_gid) ? 0 : -1;
	if (retval < 0) {
		for (member = grp->gr_mem; *member && retval; member++) {
			if (strcmp(*member, pwd->pw_name) == 0)
				retval = 0;
		}
	}
	return retval;
}


/*
 *	Read the config
 */
static int mod_instantiate(UNUSED CONF_SECTION *conf, void *instance)
{
	struct unix_instance *inst = instance;

	/* FIXME - delay these until a group file has been read so we know
	 * groupcmp can actually do something */
	paircompare_register(dict_attrbyvalue(PW_GROUP, 0), dict_attrbyvalue(PW_USER_NAME, 0), false, groupcmp, inst);
#ifdef PW_GROUP_NAME /* compat */
	paircompare_register(dict_attrbyvalue(PW_GROUP_NAME, 0), dict_attrbyvalue(PW_USER_NAME, 0),
			true, groupcmp, inst);
#endif

	return 0;
}


/*
 *	Pull the users password from where-ever, and add it to
 *	the given vp list.
 */
static rlm_rcode_t mod_authorize(UNUSED void *instance, REQUEST *request)
{
	char const	*name;
	char const	*encrypted_pass;
#ifdef HAVE_GETSPNAM
	struct spwd	*spwd = NULL;
#endif
#ifdef OSFC2
	struct pr_passwd *pr_pw;
#else
	struct passwd	*pwd;
#endif
#ifdef HAVE_GETUSERSHELL
	char		*shell;
#endif
	VALUE_PAIR	*vp;

	/*
	 *	We can only authenticate user requests which HAVE
	 *	a User-Name attribute.
	 */
	if (!request->username) {
		return RLM_MODULE_NOOP;
	}

	name = request->username->vp_strvalue;
	encrypted_pass = NULL;

#ifdef OSFC2
	if ((pr_pw = getprpwnam(name)) == NULL)
		return RLM_MODULE_NOTFOUND;
	encrypted_pass = pr_pw->ufld.fd_encrypt;

	/*
	 *	Check if account is locked.
	 */
	if (pr_pw->uflg.fg_lock!=1) {
		AUTH("rlm_unix: [%s]: account locked", name);
		return RLM_MODULE_USERLOCK;
	}
#else /* OSFC2 */
	if ((pwd = getpwnam(name)) == NULL) {
		return RLM_MODULE_NOTFOUND;
	}
	encrypted_pass = pwd->pw_passwd;
#endif /* OSFC2 */

#ifdef HAVE_GETSPNAM
	/*
	 *      See if there is a shadow password.
	 *
	 *	Only query the _system_ shadow file if the encrypted
	 *	password from the passwd file is < 10 characters (i.e.
	 *	a valid password would never crypt() to it).  This will
	 *	prevents users from using NULL password fields as things
	 *	stand right now.
	 */
	if ((!encrypted_pass) || (strlen(encrypted_pass) < 10)) {
		if ((spwd = getspnam(name)) == NULL) {
			return RLM_MODULE_NOTFOUND;
		}
		encrypted_pass = spwd->sp_pwdp;
	}
#endif	/* HAVE_GETSPNAM */

/*
 *	These require 'pwd != NULL', which isn't true on OSFC2
 */
#ifndef OSFC2
#ifdef DENY_SHELL
	/*
	 *	Users with a particular shell are denied access
	 */
	if (strcmp(pwd->pw_shell, DENY_SHELL) == 0) {
		RAUTH("rlm_unix: [%s]: invalid shell", name);
		return RLM_MODULE_REJECT;
	}
#endif

#ifdef HAVE_GETUSERSHELL
	/*
	 *	Check /etc/shells for a valid shell. If that file
	 *	contains /RADIUSD/ANY/SHELL then any shell will do.
	 */
	while ((shell = getusershell()) != NULL) {
		if (strcmp(shell, pwd->pw_shell) == 0 ||
		    strcmp(shell, "/RADIUSD/ANY/SHELL") == 0) {
			break;
		}
	}
	endusershell();
	if (!shell) {
		RAUTH("[%s]: invalid shell [%s]",
		       name, pwd->pw_shell);
		return RLM_MODULE_REJECT;
	}
#endif
#endif /* OSFC2 */

#if defined(HAVE_GETSPNAM) && !defined(M_UNIX)
	/*
	 *      Check if password has expired.
	 */
	if (spwd && spwd->sp_lstchg > 0 && spwd->sp_max >= 0 &&
	    (request->timestamp / 86400) > (spwd->sp_lstchg + spwd->sp_max)) {
		RAUTH("[%s]: password has expired", name);
		return RLM_MODULE_REJECT;
	}
	/*
	 *      Check if account has expired.
	 */
	if (spwd && spwd->sp_expire > 0 &&
	    (request->timestamp / 86400) > spwd->sp_expire) {
		RAUTH("[%s]: account has expired", name);
		return RLM_MODULE_REJECT;
	}
#endif

#if defined(__FreeBSD__) || defined(bsdi) || defined(_PWF_EXPIRE)
	/*
	 *	Check if password has expired.
	 */
	if ((pwd->pw_expire > 0) &&
	    (request->timestamp > pwd->pw_expire)) {
		RAUTH("[%s]: password has expired", name);
		return RLM_MODULE_REJECT;
	}
#endif

	/*
	 *	We might have a passwordless account.
	 *
	 *	FIXME: Maybe add Auth-Type := Accept?
	 */
	if (encrypted_pass[0] == 0)
		return RLM_MODULE_NOOP;

	vp = pairmake_config("Crypt-Password", encrypted_pass, T_OP_SET);
	if (!vp) return RLM_MODULE_FAIL;

	return RLM_MODULE_UPDATED;
}


/*
 *	UUencode 4 bits base64. We use this to turn a 4 byte field
 *	(an IP address) into 6 bytes of ASCII. This is used for the
 *	wtmp file if we didn't find a short name in the naslist file.
 */
static char *uue(void *in)
{
	int i;
	static unsigned char res[7];
	unsigned char *data = (unsigned char *)in;

	res[0] = ENC( data[0] >> 2 );
	res[1] = ENC( ((data[0] << 4) & 060) + ((data[1] >> 4) & 017) );
	res[2] = ENC( ((data[1] << 2) & 074) + ((data[2] >> 6) & 03) );
	res[3] = ENC( data[2] & 077 );

	res[4] = ENC( data[3] >> 2 );
	res[5] = ENC( (data[3] << 4) & 060 );
	res[6] = 0;

	for(i = 0; i < 6; i++) {
		if (res[i] == ' ') res[i] = '`';
		if (res[i] < 32 || res[i] > 127)
			printf("uue: protocol error ?!\n");
	}
	return (char *)res;
}


/*
 *	Unix accounting - write a wtmp file.
 */
static rlm_rcode_t mod_accounting(void *instance, REQUEST *request)
{
	VALUE_PAIR	*vp;
	vp_cursor_t	cursor;
	FILE		*fp;
	struct utmp	ut;
	time_t		t;
	char		buf[64];
	char const	*s;
	int		delay = 0;
	int		status = -1;
	int		nas_address = 0;
	int		framed_address = 0;
#ifdef USER_PROCESS
	int		protocol = -1;
#endif
	int		nas_port = 0;
	int		port_seen = 0;
	struct unix_instance *inst = (struct unix_instance *) instance;

	/*
	 *	No radwtmp.  Don't do anything.
	 */
	if (!inst->radwtmp) {
		RDEBUG2("No radwtmp file configured.  Ignoring accounting request.");
		return RLM_MODULE_NOOP;
	}

	if (request->packet->src_ipaddr.af != AF_INET) {
		RDEBUG2("IPv6 is not supported!");
		return RLM_MODULE_NOOP;
	}

	/*
	 *	Which type is this.
	 */
	if ((vp = pairfind(request->packet->vps, PW_ACCT_STATUS_TYPE, 0, TAG_ANY))==NULL) {
		RDEBUG("no Accounting-Status-Type attribute in request.");
		return RLM_MODULE_NOOP;
	}
	status = vp->vp_integer;

	/*
	 *	FIXME: handle PW_STATUS_ALIVE like 1.5.4.3 did.
	 */
	if (status != PW_STATUS_START &&
	    status != PW_STATUS_STOP)
		return RLM_MODULE_NOOP;

	/*
	 *	We're only interested in accounting messages
	 *	with a username in it.
	 */
	if (pairfind(request->packet->vps, PW_USER_NAME, 0, TAG_ANY) == NULL)
		return RLM_MODULE_NOOP;

	t = request->timestamp;
	memset(&ut, 0, sizeof(ut));

	/*
	 *	First, find the interesting attributes.
	 */
	for (vp = paircursor(&cursor, &request->packet->vps);
	     vp;
	     vp = pairnext(&cursor)) {
		if (!vp->da->vendor) switch (vp->da->attr) {
			case PW_USER_NAME:
				if (vp->length >= sizeof(ut.ut_name)) {
					memcpy(ut.ut_name, vp->vp_strvalue, sizeof(ut.ut_name));
				} else {
					strlcpy(ut.ut_name, vp->vp_strvalue, sizeof(ut.ut_name));
				}
				break;
			case PW_LOGIN_IP_HOST:
			case PW_FRAMED_IP_ADDRESS:
				framed_address = vp->vp_ipaddr;
				break;
#ifdef USER_PROCESS
			case PW_FRAMED_PROTOCOL:
				protocol = vp->vp_integer;
				break;
#endif
			case PW_NAS_IP_ADDRESS:
				nas_address = vp->vp_ipaddr;
				break;
			case PW_NAS_PORT:
				nas_port = vp->vp_integer;
				port_seen = 1;
				break;
			case PW_ACCT_DELAY_TIME:
				delay = vp->vp_ipaddr;
				break;
		}
	}

	/*
	 *	We don't store !root sessions, or sessions
	 *	where we didn't see a NAS-Port attribute.
	 */
	if (strncmp(ut.ut_name, "!root", sizeof(ut.ut_name)) == 0 || !port_seen)
		return RLM_MODULE_NOOP;

	/*
	 *	If we didn't find out the NAS address, use the
	 *	originator's IP address.
	 */
	if (nas_address == 0) {
		nas_address = request->packet->src_ipaddr.ipaddr.ip4addr.s_addr;
	}
	s = request->client->shortname;
	if (!s || s[0] == 0) s = uue(&(nas_address));

#ifdef __linux__
	/*
	 *	Linux has a field for the client address.
	 */
	ut.ut_addr = framed_address;
#endif
	/*
	 *	We use the tty field to store the terminal servers' port
	 *	and address so that the tty field is unique.
	 */
	snprintf(buf, sizeof(buf), "%03d:%s", nas_port, s);
	strlcpy(ut.ut_line, buf, sizeof(ut.ut_line));

	/*
	 *	We store the dynamic IP address in the hostname field.
	 */
#ifdef UT_HOSTSIZE
	if (framed_address) {
		ip_ntoa(buf, framed_address);
		strlcpy(ut.ut_host, buf, sizeof(ut.ut_host));
	}
#endif
#ifdef HAVE_UTMPX_H
	ut.ut_xtime = t- delay;
#else
	ut.ut_time = t - delay;
#endif
#ifdef USER_PROCESS
	/*
	 *	And we can use the ID field to store
	 *	the protocol.
	 */
	if (protocol == PW_PPP)
		strcpy(ut.ut_id, "P");
	else if (protocol == PW_SLIP)
		strcpy(ut.ut_id, "S");
	else
		strcpy(ut.ut_id, "T");
	ut.ut_type = status == PW_STATUS_STOP ? DEAD_PROCESS : USER_PROCESS;
#endif
	if (status == PW_STATUS_STOP)
		ut.ut_name[0] = 0;

	/*
	 *	Write a RADIUS wtmp log file.
	 *
	 *	Try to open the file if we can't, we don't write the
	 *	wtmp file. If we can try to write. If we fail,
	 *	return RLM_MODULE_FAIL ..
	 */
	if ((fp = fopen(inst->radwtmp, "a")) != NULL) {
		if ((fwrite(&ut, sizeof(ut), 1, fp)) != 1) {
			fclose(fp);
			return RLM_MODULE_FAIL;
		}
		fclose(fp);
	} else
		return RLM_MODULE_FAIL;

	return RLM_MODULE_OK;
}

/* globally exported name */
module_t rlm_unix = {
	RLM_MODULE_INIT,
	"System",
	RLM_TYPE_THREAD_UNSAFE | RLM_TYPE_CHECK_CONFIG_SAFE,
	sizeof(struct unix_instance),
	module_config,
	mod_instantiate,		/* instantiation */
	NULL,				/* detach */
	{
		NULL,		    /* authentication */
		mod_authorize,       /* authorization */
		NULL,		 /* preaccounting */
		mod_accounting,      /* accounting */
		NULL,		  /* checksimul */
		NULL,			/* pre-proxy */
		NULL,			/* post-proxy */
		NULL			/* post-auth */
	},
};
