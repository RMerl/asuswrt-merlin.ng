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
 * @file rlm_radutmp.c
 * @brief Tracks sessions.
 *
 * @copyright 2000-2013  The FreeRADIUS server project
 */
RCSID("$Id$")

#include	<freeradius-devel/radiusd.h>
#include	<freeradius-devel/radutmp.h>
#include	<freeradius-devel/modules.h>
#include	<freeradius-devel/rad_assert.h>

#include	<fcntl.h>
#include	<limits.h>

#include "config.h"

#define LOCK_LEN sizeof(struct radutmp)

static char const porttypes[] = "ASITX";

/*
 *	used for caching radutmp lookups in the accounting component. The
 *	session (checksimul) component doesn't use it, but probably should.
 */
typedef struct nas_port {
	uint32_t		nasaddr;
	unsigned int	port;
	off_t			offset;
	struct nas_port 	*next;
} NAS_PORT;

typedef struct rlm_radutmp_t {
	NAS_PORT	*nas_port_list;
	char		*filename;
	char		*username;
	int		case_sensitive;
	int		check_nas;
	int		permission;
	int		caller_id_ok;
} rlm_radutmp_t;

static const CONF_PARSER module_config[] = {
	{ "filename", PW_TYPE_FILE_OUTPUT | PW_TYPE_REQUIRED,
	  offsetof(rlm_radutmp_t,filename), NULL,  RADUTMP },
	{ "username", PW_TYPE_STRING_PTR | PW_TYPE_REQUIRED,
	  offsetof(rlm_radutmp_t,username), NULL,  "%{User-Name}"},
	{ "case_sensitive", PW_TYPE_BOOLEAN,
	  offsetof(rlm_radutmp_t,case_sensitive), NULL,  "yes"},
	{ "check_with_nas", PW_TYPE_BOOLEAN,
	  offsetof(rlm_radutmp_t,check_nas), NULL,  "yes"},
	{ "perm",     PW_TYPE_INTEGER | PW_TYPE_DEPRECATED,
	  offsetof(rlm_radutmp_t,permission), NULL,  NULL },
	{ "permissions",     PW_TYPE_INTEGER,
	  offsetof(rlm_radutmp_t,permission), NULL,  "0644" },
	{ "callerid", PW_TYPE_BOOLEAN | PW_TYPE_DEPRECATED,
	  offsetof(rlm_radutmp_t,caller_id_ok), NULL, NULL },
	{ "caller_id", PW_TYPE_BOOLEAN,
	  offsetof(rlm_radutmp_t,caller_id_ok), NULL, "no" },
	{ NULL, -1, 0, NULL, NULL }		/* end the list */
};

/*
 *	Zap all users on a NAS from the radutmp file.
 */
static rlm_rcode_t radutmp_zap(REQUEST *request, char const *filename, uint32_t nasaddr, time_t t)
{
	struct radutmp	u;
	int		fd;

	if (t == 0) time(&t);

	fd = open(filename, O_RDWR);
	if (fd < 0) {
		REDEBUG("Error accessing file %s: %s", filename, strerror(errno));
		return RLM_MODULE_FAIL;
	}

	/*
	*	Lock the utmp file, prefer lockf() over flock().
	*/
	if (rad_lockfd(fd, LOCK_LEN) < 0) {
		REDEBUG("Failed to acquire lock on file %s: %s", filename, strerror(errno));
		close(fd);
		return RLM_MODULE_FAIL;
	}

	/*
	*	Find the entry for this NAS / portno combination.
	*/
	while (read(fd, &u, sizeof(u)) == sizeof(u)) {
		if ((nasaddr != 0 && nasaddr != u.nas_address) || u.type != P_LOGIN) {
			continue;
		}
		/*
		 *	Match. Zap it.
		 */
		if (lseek(fd, -(off_t)sizeof(u), SEEK_CUR) < 0) {
			REDEBUG("radutmp_zap: negative lseek!");
			lseek(fd, (off_t)0, SEEK_SET);
		}
		u.type = P_IDLE;
		u.time = t;

		if (write(fd, &u, sizeof(u)) < 0) {
			REDEBUG("Failed writing: %s", strerror(errno));

			close(fd);
			return RLM_MODULE_FAIL;
		}
	}
	close(fd);	/* and implicitely release the locks */

	return RLM_MODULE_OK;
}

/*
 *	Lookup a NAS_PORT in the nas_port_list
 */
static NAS_PORT *nas_port_find(NAS_PORT *nas_port_list, uint32_t nasaddr, unsigned int port)
{
	NAS_PORT	*cl;

	for(cl = nas_port_list; cl; cl = cl->next) {
		if (nasaddr == cl->nasaddr &&
			port == cl->port)
			break;
	}

	return cl;
}


#ifdef WITH_ACCOUNTING
/*
 *	Store logins in the RADIUS utmp file.
 */
static rlm_rcode_t mod_accounting(void *instance, REQUEST *request)
{
	rlm_rcode_t	rcode = RLM_MODULE_OK;
	struct radutmp	ut, u;
	vp_cursor_t	cursor;
	VALUE_PAIR	*vp;
	int		status = -1;
	int		protocol = -1;
	time_t		t;
	int		fd = -1;
	int		port_seen = 0;
	int		off;
	rlm_radutmp_t	*inst = instance;
	char		ip_name[32]; /* 255.255.255.255 */
	char const	*nas;
	NAS_PORT	*cache;
	int		r;

	char		*filename = NULL;
	char		*expanded = NULL;

	if (request->packet->src_ipaddr.af != AF_INET) {
		DEBUG("rlm_radutmp: IPv6 not supported!");
		return RLM_MODULE_NOOP;
	}

	/*
	 *	Which type is this.
	 */
	if ((vp = pairfind(request->packet->vps, PW_ACCT_STATUS_TYPE, 0, TAG_ANY)) == NULL) {
		RDEBUG("No Accounting-Status-Type record.");
		return RLM_MODULE_NOOP;
	}
	status = vp->vp_integer;

	/*
	 *	Look for weird reboot packets.
	 *
	 *	ComOS (up to and including 3.5.1b20) does not send
	 *	standard PW_STATUS_ACCOUNTING_XXX messages.
	 *
	 *	Check for:  o no Acct-Session-Time, or time of 0
	 *		    o Acct-Session-Id of "00000000".
	 *
	 *	We could also check for NAS-Port, that attribute
	 *	should NOT be present (but we don't right now).
	 */
	if ((status != PW_STATUS_ACCOUNTING_ON) &&
	    (status != PW_STATUS_ACCOUNTING_OFF)) do {
		int check1 = 0;
		int check2 = 0;

		if ((vp = pairfind(request->packet->vps, PW_ACCT_SESSION_TIME, 0, TAG_ANY))
		     == NULL || vp->vp_date == 0)
			check1 = 1;
		if ((vp = pairfind(request->packet->vps, PW_ACCT_SESSION_ID, 0, TAG_ANY))
		     != NULL && vp->length == 8 &&
		     memcmp(vp->vp_strvalue, "00000000", 8) == 0)
			check2 = 1;
		if (check1 == 0 || check2 == 0) {
			break;
		}
		INFO("rlm_radutmp: converting reboot records.");
		if (status == PW_STATUS_STOP)
			status = PW_STATUS_ACCOUNTING_OFF;
		if (status == PW_STATUS_START)
			status = PW_STATUS_ACCOUNTING_ON;
	} while(0);

	time(&t);
	memset(&ut, 0, sizeof(ut));
	ut.porttype = 'A';
	ut.nas_address = htonl(INADDR_NONE);

	/*
	 *	First, find the interesting attributes.
	 */
	for (vp = paircursor(&cursor, &request->packet->vps);
	     vp;
	     vp = pairnext(&cursor)) {
		if (!vp->da->vendor) switch (vp->da->attr) {
			case PW_LOGIN_IP_HOST:
			case PW_FRAMED_IP_ADDRESS:
				ut.framed_address = vp->vp_ipaddr;
				break;
			case PW_FRAMED_PROTOCOL:
				protocol = vp->vp_integer;
				break;
			case PW_NAS_IP_ADDRESS:
				ut.nas_address = vp->vp_ipaddr;
				break;
			case PW_NAS_PORT:
				ut.nas_port = vp->vp_integer;
				port_seen = 1;
				break;
			case PW_ACCT_DELAY_TIME:
				ut.delay = vp->vp_integer;
				break;
			case PW_ACCT_SESSION_ID:
				/*
				 *	If length > 8, only store the
				 *	last 8 bytes.
				 */
				off = vp->length - sizeof(ut.session_id);
				/*
				 * 	Ascend is br0ken - it adds a \0
				 * 	to the end of any string.
				 * 	Compensate.
				 */
				if (vp->length > 0 &&
				    vp->vp_strvalue[vp->length - 1] == 0)
					off--;
				if (off < 0) off = 0;
				memcpy(ut.session_id, vp->vp_strvalue + off,
					sizeof(ut.session_id));
				break;
			case PW_NAS_PORT_TYPE:
				if (vp->vp_integer <= 4)
					ut.porttype = porttypes[vp->vp_integer];
				break;
			case PW_CALLING_STATION_ID:
				if(inst->caller_id_ok)
					strlcpy(ut.caller_id,
						vp->vp_strvalue,
						sizeof(ut.caller_id));
				break;
		}
	}

	/*
	 *	If we didn't find out the NAS address, use the
	 *	originator's IP address.
	 */
	if (ut.nas_address == htonl(INADDR_NONE)) {
		ut.nas_address = request->packet->src_ipaddr.ipaddr.ip4addr.s_addr;
		nas = request->client->shortname;

	} else if (request->packet->src_ipaddr.ipaddr.ip4addr.s_addr == ut.nas_address) {		/* might be a client, might not be. */
		nas = request->client->shortname;

	} else {
		/*
		 *	The NAS isn't a client, it's behind
		 *	a proxy server.  In that case, just
		 *	get the IP address.
		 */
		nas = ip_ntoa(ip_name, ut.nas_address);
	}

	/*
	 *	Set the protocol field.
	 */
	if (protocol == PW_PPP) {
		ut.proto = 'P';
	} else if (protocol == PW_SLIP) {
		ut.proto = 'S';
	} else {
		ut.proto = 'T';
	}

	ut.time = t - ut.delay;

	/*
	 *	Get the utmp filename, via xlat.
	 */
	filename = NULL;
	if (radius_axlat(&filename, request, inst->filename, NULL, NULL) < 0) {
		return RLM_MODULE_FAIL;
	}

	/*
	 *	See if this was a reboot.
	 *
	 *	Hmm... we may not want to zap all of the users when the NAS comes up, because of issues with receiving
	 *	UDP packets out of order.
	 */
	if (status == PW_STATUS_ACCOUNTING_ON && (ut.nas_address != htonl(INADDR_NONE))) {
		RIDEBUG("NAS %s restarted (Accounting-On packet seen)", nas);
		rcode = radutmp_zap(request, filename, ut.nas_address, ut.time);

		goto finish;
	}

	if (status == PW_STATUS_ACCOUNTING_OFF && (ut.nas_address != htonl(INADDR_NONE))) {
		RIDEBUG("NAS %s rebooted (Accounting-Off packet seen)", nas);
		rcode = radutmp_zap(request, filename, ut.nas_address, ut.time);

		goto finish;
	}

	/*
	 *	If we don't know this type of entry pretend we succeeded.
	 */
	if (status != PW_STATUS_START && status != PW_STATUS_STOP && status != PW_STATUS_ALIVE) {
		REDEBUG("NAS %s port %u unknown packet type %d)", nas, ut.nas_port, status);
		rcode = RLM_MODULE_NOOP;

		goto finish;
	}

	/*
	 *	Translate the User-Name attribute, or whatever else they told us to use.
	 */
	if (radius_axlat(&expanded, request, inst->username, NULL, NULL) < 0) {
		rcode = RLM_MODULE_FAIL;

		goto finish;
	}
	strlcpy(ut.login, expanded, RUT_NAMESIZE);
	TALLOC_FREE(expanded);

	/*
	 *	Perhaps we don't want to store this record into
	 *	radutmp. We skip records:
	 *
	 *	- without a NAS-Port (telnet / tcp access)
	 *	- with the username "!root" (console admin login)
	 */
	if (!port_seen) {
		RWDEBUG2("No NAS-Port seen.  Cannot do anything. Checkrad will probably not work!");
		rcode = RLM_MODULE_NOOP;

		goto finish;
	}

	if (strncmp(ut.login, "!root", RUT_NAMESIZE) == 0) {
		RDEBUG2("Not recording administrative user");
		rcode = RLM_MODULE_NOOP;

		goto finish;
	}

	/*
	 *	Enter into the radutmp file.
	 */
	fd = open(filename, O_RDWR|O_CREAT, inst->permission);
	if (fd < 0) {
		REDEBUG("Error accessing file %s: %s", filename, strerror(errno));
		rcode = RLM_MODULE_FAIL;

		goto finish;
	}

	/*
	 *	Lock the utmp file, prefer lockf() over flock().
	 */
	rad_lockfd(fd, LOCK_LEN);

	/*
	 *	Find the entry for this NAS / portno combination.
	 */
	if ((cache = nas_port_find(inst->nas_port_list, ut.nas_address, ut.nas_port)) != NULL) {
		lseek(fd, (off_t)cache->offset, SEEK_SET);
	}

	r = 0;
	off = 0;
	while (read(fd, &u, sizeof(u)) == sizeof(u)) {
		off += sizeof(u);
		if ((u.nas_address != ut.nas_address) || (u.nas_port != ut.nas_port)) {
			continue;
		}

		/*
		 *	Don't compare stop records to unused entries.
		 */
		if (status == PW_STATUS_STOP && u.type == P_IDLE) {
			continue;
		}

		if ((status == PW_STATUS_STOP) && strncmp(ut.session_id, u.session_id, sizeof(u.session_id)) != 0) {
			/*
			 *	Don't complain if this is not a
			 *	login record (some clients can
			 *	send _only_ logout records).
			 */
			if (u.type == P_LOGIN) {
				RWDEBUG("Logout entry for NAS %s port %u has wrong ID", nas, u.nas_port);
			}

			r = -1;
			break;
		}

		if ((status == PW_STATUS_START) && strncmp(ut.session_id, u.session_id, sizeof(u.session_id)) == 0  &&
		    u.time >= ut.time) {
			if (u.type == P_LOGIN) {
				INFO("rlm_radutmp: Login entry for NAS %s port %u duplicate",
				       nas, u.nas_port);
				r = -1;
				break;
			}

			RWDEBUG("Login entry for NAS %s port %u wrong order", nas, u.nas_port);
			r = -1;
			break;
		}

		/*
		 *	FIXME: the ALIVE record could need some more checking, but anyway I'd
		 *	rather rewrite this mess -- miquels.
		 */
		if ((status == PW_STATUS_ALIVE) && strncmp(ut.session_id, u.session_id, sizeof(u.session_id)) == 0  &&
		    u.type == P_LOGIN) {
			/*
			 *	Keep the original login time.
			 */
			ut.time = u.time;
		}

		if (lseek(fd, -(off_t)sizeof(u), SEEK_CUR) < 0) {
			RWDEBUG("negative lseek!");
			lseek(fd, (off_t)0, SEEK_SET);
			off = 0;
		} else {
			off -= sizeof(u);
		}

		r = 1;
		break;
	} /* read the file until we find a match */

	/*
	 *	Found the entry, do start/update it with
	 *	the information from the packet.
	 */
	if ((r >= 0) && (status == PW_STATUS_START || status == PW_STATUS_ALIVE)) {
		/*
		 *	Remember where the entry was, because it's
		 *	easier than searching through the entire file.
		 */
		if (!cache) {
			cache = talloc_zero(inst, NAS_PORT);
			if (cache) {
				cache->nasaddr = ut.nas_address;
				cache->port = ut.nas_port;
				cache->offset = off;
				cache->next = inst->nas_port_list;
				inst->nas_port_list = cache;
			}
		}

		ut.type = P_LOGIN;
		if (write(fd, &ut, sizeof(u)) < 0) {
			REDEBUG("Failed writing: %s", strerror(errno));

			rcode = RLM_MODULE_FAIL;
			goto finish;
		}
	}

	/*
	 *	The user has logged off, delete the entry by
	 *	re-writing it in place.
	 */
	if (status == PW_STATUS_STOP) {
		if (r > 0) {
			u.type = P_IDLE;
			u.time = ut.time;
			u.delay = ut.delay;
			if (write(fd, &u, sizeof(u)) < 0) {
				REDEBUG("Failed writing: %s", strerror(errno));

				rcode = RLM_MODULE_FAIL;
				goto finish;
			}
		} else if (r == 0) {
			RWDEBUG("Logout for NAS %s port %u, but no Login record", nas, ut.nas_port);
		}
	}

	finish:

	talloc_free(filename);

	if (fd > -1) {
		close(fd);	/* and implicitely release the locks */
	}

	return rcode;
}
#endif

#ifdef WITH_SESSION_MGMT
/*
 *	See if a user is already logged in. Sets request->simul_count to the
 *	current session count for this user and sets request->simul_mpp to 2
 *	if it looks like a multilink attempt based on the requested IP
 *	address, otherwise leaves request->simul_mpp alone.
 *
 *	Check twice. If on the first pass the user exceeds his
 *	max. number of logins, do a second pass and validate all
 *	logins by querying the terminal server (using eg. SNMP).
 */
static rlm_rcode_t mod_checksimul(void *instance, REQUEST *request)
{
	rlm_rcode_t	rcode = RLM_MODULE_OK;
	struct radutmp	u;
	int		fd = -1;
	VALUE_PAIR	*vp;
	uint32_t	ipno = 0;
	char const     	*call_num = NULL;
	rlm_radutmp_t	*inst = instance;

	char		*expanded = NULL;
	ssize_t		len;

	/*
	 *	Get the filename, via xlat.
	 */
	if (radius_axlat(&expanded, request, inst->filename, NULL, NULL) < 0) {
		return RLM_MODULE_FAIL;
	}

	fd = open(expanded, O_RDWR);
	if (fd < 0) {
		/*
		 *	If the file doesn't exist, then no users
		 *	are logged in.
		 */
		if (errno == ENOENT) {
			request->simul_count=0;
			return RLM_MODULE_OK;
		}

		/*
		 *	Error accessing the file.
		 */
		ERROR("rlm_radumtp: Error accessing file %s: %s", expanded, strerror(errno));

		rcode = RLM_MODULE_FAIL;

		goto finish;
	}

	TALLOC_FREE(expanded);

	len = radius_axlat(&expanded, request, inst->username, NULL, NULL);
	if (len < 0) {
		rcode = RLM_MODULE_FAIL;

		goto finish;
	}

	if (!len) {
		rcode = RLM_MODULE_NOOP;

		goto finish;
	}

	/*
	 *	WTF?  This is probably wrong... we probably want to
	 *	be able to check users across multiple session accounting
	 *	methods.
	 */
	request->simul_count = 0;

	/*
	 *	Loop over utmp, counting how many people MAY be logged in.
	 */
	while (read(fd, &u, sizeof(u)) == sizeof(u)) {
		if (((strncmp(expanded, u.login, RUT_NAMESIZE) == 0) ||
		    (!inst->case_sensitive && (strncasecmp(expanded, u.login, RUT_NAMESIZE) == 0))) &&
		     (u.type == P_LOGIN)) {
			++request->simul_count;
		}
	}

	/*
	 *	The number of users logged in is OK,
	 *	OR, we've been told to not check the NAS.
	 */
	if ((request->simul_count < request->simul_max) || !inst->check_nas) {
		rcode = RLM_MODULE_OK;

		goto finish;
	}
	lseek(fd, (off_t)0, SEEK_SET);

	/*
	 *	Setup some stuff, like for MPP detection.
	 */
	if ((vp = pairfind(request->packet->vps, PW_FRAMED_IP_ADDRESS, 0, TAG_ANY)) != NULL) {
		ipno = vp->vp_ipaddr;
	}

	if ((vp = pairfind(request->packet->vps, PW_CALLING_STATION_ID, 0, TAG_ANY)) != NULL) {
		call_num = vp->vp_strvalue;
	}

	/*
	 *	lock the file while reading/writing.
	 */
	rad_lockfd(fd, LOCK_LEN);

	/*
	 *	FIXME: If we get a 'Start' for a user/nas/port which is
	 *	listed, but for which we did NOT get a 'Stop', then
	 *	it's not a duplicate session.  This happens with
	 *	static IP's like DSL.
	 */
	request->simul_count = 0;
	while (read(fd, &u, sizeof(u)) == sizeof(u)) {
		if (((strncmp(expanded, u.login, RUT_NAMESIZE) == 0) || (!inst->case_sensitive &&
		    (strncasecmp(expanded, u.login, RUT_NAMESIZE) == 0))) && (u.type == P_LOGIN)) {
			char session_id[sizeof(u.session_id) + 1];
			char utmp_login[sizeof(u.login) + 1];

			strlcpy(session_id, u.session_id, sizeof(session_id));

			/*
			 *	The login name MAY fill the whole field,
			 *	and thus won't be zero-filled.
			 *
			 *	Note that we take the user name from
			 *	the utmp file, as that's the canonical
			 *	form.  The 'login' variable may contain
			 *	a string which is an upper/lowercase
			 *	version of u.login.  When we call the
			 *	routine to check the terminal server,
			 *	the NAS may be case sensitive.
			 *
			 *	e.g. We ask if "bob" is using a port,
			 *	and the NAS says "no", because "BOB"
			 *	is using the port.
			 */
			memset(utmp_login, 0, sizeof(utmp_login));
			memcpy(utmp_login, u.login, sizeof(u.login));

			/*
			 *	rad_check_ts may take seconds
			 *	to return, and we don't want
			 *	to block everyone else while
			 *	that's happening.  */
			rad_unlockfd(fd, LOCK_LEN);
			rcode = rad_check_ts(u.nas_address, u.nas_port, utmp_login, session_id);
			rad_lockfd(fd, LOCK_LEN);

			if (rcode == 0) {
				/*
				 *	Stale record - zap it.
				 */
				session_zap(request, u.nas_address, u.nas_port, expanded, session_id,
					    u.framed_address, u.proto, 0);
			}
			else if (rcode == 1) {
				/*
				 *	User is still logged in.
				 */
				++request->simul_count;

				/*
				 *	Does it look like a MPP attempt?
				 */
				if (strchr("SCPA", u.proto) && ipno && u.framed_address == ipno) {
					request->simul_mpp = 2;
				} else if (strchr("SCPA", u.proto) && call_num && !strncmp(u.caller_id, call_num,16)) {
					request->simul_mpp = 2;
				}
			} else {
				RWDEBUG("Failed to check the terminal server for user '%s'.", utmp_login);
				rcode = RLM_MODULE_FAIL;

				goto finish;
			}
		}
	}
	finish:

	talloc_free(expanded);

	if (fd > -1) {
		close(fd);		/* and implicitely release the locks */
	}

	return rcode;
}
#endif

/* globally exported name */
module_t rlm_radutmp = {
	RLM_MODULE_INIT,
	"radutmp",
	RLM_TYPE_THREAD_UNSAFE | RLM_TYPE_CHECK_CONFIG_SAFE | RLM_TYPE_HUP_SAFE,   	/* type */
	sizeof(rlm_radutmp_t),
	module_config,
	NULL,			       /* instantiation */
	NULL,			       /* detach */
	{
		NULL,		 /* authentication */
		NULL,		 /* authorization */
		NULL,		 /* preaccounting */
#ifdef WITH_ACCOUNTING
		mod_accounting,   /* accounting */
#else
		NULL,
#endif
#ifdef WITH_SESSION_MGMT
		mod_checksimul,	/* checksimul */
#else
		NULL,
#endif
		NULL,			/* pre-proxy */
		NULL,			/* post-proxy */
		NULL			/* post-auth */
	},
};

