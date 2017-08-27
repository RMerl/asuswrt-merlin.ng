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
 * @file rlm_logintime.c
 * @brief Allow login only during a given timeslot.
 *
 * @copyright 2001,2006  The FreeRADIUS server project
 * @copyright 2004  Kostas Kalevras <kkalev@noc.ntua.gr>
 */
RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>

#include <ctype.h>

/* timestr.c */
int		timestr_match(char const *, time_t);

/*
 *	Define a structure for our module configuration.
 *
 *	These variables do not need to be in a structure, but it's
 *	a lot cleaner to do so, and a pointer to the structure can
 *	be used as the instance handle.
 */
typedef struct rlm_logintime_t {
	int min_time;
} rlm_logintime_t;

/*
 *	A mapping of configuration file names to internal variables.
 *
 *	Note that the string is dynamically allocated, so it MUST
 *	be freed.  When the configuration file parse re-reads the string,
 *	it free's the old one, and strdup's the new one, placing the pointer
 *	to the strdup'd string into 'config.string'.  This gets around
 *	buffer over-flows.
 */
static const CONF_PARSER module_config[] = {
  { "minimum-timeout", PW_TYPE_INTEGER | PW_TYPE_DEPRECATED, offsetof(rlm_logintime_t,min_time), NULL, NULL},
  { "minimum_timeout", PW_TYPE_INTEGER, offsetof(rlm_logintime_t,min_time), NULL, "60" },

  { NULL, -1, 0, NULL, NULL }
};


/*
 *      Compare the current time to a range.
 */
static int timecmp(UNUSED void *instance, REQUEST *req, UNUSED VALUE_PAIR *request, VALUE_PAIR *check,
		   UNUSED VALUE_PAIR *check_pairs, UNUSED VALUE_PAIR **reply_pairs)
{
	/*
	 *      If there's a request, use that timestamp.
	 */
	if (timestr_match(check->vp_strvalue,
	req ? req->timestamp : time(NULL)) >= 0)
		return 0;

	return -1;
}


/*
 *	Time-Of-Day support
 */
static int time_of_day(UNUSED void *instance, REQUEST *req, UNUSED VALUE_PAIR *request, VALUE_PAIR *check,
		       UNUSED VALUE_PAIR *check_pairs, UNUSED VALUE_PAIR **reply_pairs)
{
	int scan;
	int hhmmss, when;
	char const *p;
	struct tm *tm, s_tm;

	/*
	 *	Must be called with a request pointer.
	 */
	if (!req) return -1;

	if (strspn(check->vp_strvalue, "0123456789: ") != strlen(check->vp_strvalue)) {
		DEBUG("rlm_logintime: Bad Time-Of-Day value \"%s\"",
		      check->vp_strvalue);
		return -1;
	}

	tm = localtime_r(&req->timestamp, &s_tm);
	hhmmss = (tm->tm_hour * 3600) + (tm->tm_min * 60) + tm->tm_sec;

	/*
	 *	Time of day is a 24-hour clock
	 */
	p = check->vp_strvalue;
	scan = atoi(p);
	p = strchr(p, ':');
	if ((scan > 23) || !p) {
		DEBUG("rlm_logintime: Bad Time-Of-Day value \"%s\"",
		      check->vp_strvalue);
		return -1;
	}
	when = scan * 3600;
	p++;

	scan = atoi(p);
	if (scan > 59) {
		DEBUG("rlm_logintime: Bad Time-Of-Day value \"%s\"",
		      check->vp_strvalue);
		return -1;
	}
	when += scan * 60;

	p = strchr(p, ':');
	if (p) {
		scan = atoi(p + 1);
		if (scan > 59) {
			DEBUG("rlm_logintime: Bad Time-Of-Day value \"%s\"",
			      check->vp_strvalue);
			return -1;
		}
		when += scan;
	}

	fprintf(stderr, "returning %d - %d\n",
		hhmmss, when);

	return hhmmss - when;
}

/*
 *      Check if account has expired, and if user may login now.
 */
static rlm_rcode_t mod_authorize(void *instance, REQUEST *request)
{
	rlm_logintime_t *inst = instance;
	VALUE_PAIR *ends, *timeout;
	int left;

	ends = pairfind(request->config_items, PW_LOGIN_TIME, 0, TAG_ANY);
	if (!ends) {
		return RLM_MODULE_NOOP;
	}

	/*
	 *      Authentication is OK. Now see if this user may login at this time of the day.
	 */
	RDEBUG("Checking Login-Time");

	/*
	 *	Compare the time the request was received with the current Login-Time value
	 */
	left = timestr_match(ends->vp_strvalue, request->timestamp);

	/*
	 *      Do nothing, login time is not controlled (unendsed).
	 */
	if (left == 0) {
		return RLM_MODULE_OK;
	}

	/*
	 *      The min_time setting is to deal with NAS that won't allow Session-Timeout values below a certain value
	 *	For example some Alcatel Lucent products won't allow a Session-Timeout < 300 (5 minutes).
	 *
	 *	We don't know were going to get another chance to lock out the user, so we need to do it now.
	 */
	if (left < inst->min_time) {
		REDEBUG("Login outside of allowed time-slot (session end %s, with lockout %i seconds before)",
			ends->vp_strvalue, inst->min_time);

		return RLM_MODULE_USERLOCK;
	}

	/* else left > inst->min_time */

	/*
	 *	There's time left in the users session, inform the NAS by including a Session-Timeout
	 *	attribute in the reply, or modifying the existing one.
	 */
	RDEBUG("Login within allowed time-slot, %i seconds left in this session", left);

	timeout = pairfind(request->reply->vps, PW_SESSION_TIMEOUT, 0, TAG_ANY);
	if (timeout) {	/* just update... */
		if (timeout->vp_integer > (unsigned int) left) {
			timeout->vp_integer = left;
		}
	} else {
		timeout = radius_paircreate(request, &request->reply->vps, PW_SESSION_TIMEOUT, 0);
		timeout->vp_integer = left;
	}

	RDEBUG("reply:Session-Timeout set to %i", left);

	return RLM_MODULE_OK;
}


/*
 *	Do any per-module initialization that is separate to each
 *	configured instance of the module.  e.g. set up connections
 *	to external databases, read configuration files, set up
 *	dictionary entries, etc.
 *
 *	If configuration information is given in the config section
 *	that must be referenced in later calls, store a handle to it
 *	in *instance otherwise put a null pointer there.
 */
static int mod_instantiate(CONF_SECTION *conf, void *instance)
{
	rlm_logintime_t *inst = instance;

	if (inst->min_time == 0) {
		cf_log_err_cs(conf, "Invalid value '0' for minimum_timeout");
		return -1;
	}

	/*
	 * Register a Current-Time comparison function
	 */
	paircompare_register(dict_attrbyvalue(PW_CURRENT_TIME, 0), NULL, true, timecmp, inst);
	paircompare_register(dict_attrbyvalue(PW_TIME_OF_DAY, 0), NULL, true, time_of_day, inst);

	return 0;
}

static int mod_detach(UNUSED void *instance)
{
	paircompare_unregister(dict_attrbyvalue(PW_CURRENT_TIME, 0), timecmp);
	paircompare_unregister(dict_attrbyvalue(PW_TIME_OF_DAY, 0), time_of_day);
	return 0;
}

/*
 *	The module name should be the only globally exported symbol.
 *	That is, everything else should be 'static'.
 *
 *	If the module needs to temporarily modify it's instantiation
 *	data, the type should be changed to RLM_TYPE_THREAD_UNSAFE.
 *	The server will then take care of ensuring that the module
 *	is single-threaded.
 */
module_t rlm_logintime = {
	RLM_MODULE_INIT,
	"logintime",
	RLM_TYPE_CHECK_CONFIG_SAFE,   	/* type */
	sizeof(rlm_logintime_t),
	module_config,
	mod_instantiate,		/* instantiation */
	mod_detach,		/* detach */
	{
		NULL,			/* authentication */
		mod_authorize, 	/* authorization */
		NULL,			/* preaccounting */
		NULL,			/* accounting */
		NULL,			/* checksimul */
		NULL,			/* pre-proxy */
		NULL,			/* post-proxy */
		NULL			/* post-auth */
	},
};
