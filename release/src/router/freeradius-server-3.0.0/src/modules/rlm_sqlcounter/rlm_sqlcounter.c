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
 * @file rlm_sqlcounter.c
 * @brief Tracks data usage and other counters using SQL.
 *
 * @copyright 2001,2006  The FreeRADIUS server project
 * @copyright 2001  Alan DeKok <aland@ox.org>
 */
RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>
#include <freeradius-devel/rad_assert.h>

#include <ctype.h>

#define MAX_QUERY_LEN 1024

/*
 *	Note: When your counter spans more than 1 period (ie 3 months
 *	or 2 weeks), this module probably does NOT do what you want! It
 *	calculates the range of dates to count across by first calculating
 *	the End of the Current period and then subtracting the number of
 *	periods you specify from that to determine the beginning of the
 *	range.
 *
 *	For example, if you specify a 3 month counter and today is June 15th,
 *	the end of the current period is June 30. Subtracting 3 months from
 *	that gives April 1st. So, the counter will sum radacct entries from
 *	April 1st to June 30. Then, next month, it will sum entries from
 *	May 1st to July 31st.
 *
 *	To fix this behavior, we need to add some way of storing the Next
 *	Reset Time.
 */

/*
 *	Define a structure for our module configuration.
 *
 *	These variables do not need to be in a structure, but it's
 *	a lot cleaner to do so, and a pointer to the structure can
 *	be used as the instance handle.
 */
typedef struct rlm_sqlcounter_t {
	char		*counter_name;  //!< Daily-Session-Time.
	char		*check_name;  	//!< Max-Daily-Session.
	char		*reply_name;  	//!< Session-Timeout.
	char		*key_name;  	//!< User-Name.
	char		*sqlmod_inst;	//!< Instance of SQL module to use,
					//!< usually just 'sql'.
	char		*query;		//!< SQL query to retrieve current
					//!< session time.
	char		*reset;  	//!< Daily, weekly, monthly,
					//!< never or user defined.
	time_t		reset_time;
	time_t		last_reset;
	DICT_ATTR const	*key_attr;	//!< Attribute number for key field.
	DICT_ATTR const	*dict_attr;	//!< Attribute number for the counter.
	DICT_ATTR const	*reply_attr;	//!< Attribute number for the reply.
} rlm_sqlcounter_t;

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
	{ "sql-module-instance", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED,
	  offsetof(rlm_sqlcounter_t,sqlmod_inst), NULL, NULL },
	{ "sql_module_instance", PW_TYPE_STRING_PTR | PW_TYPE_REQUIRED,
	  offsetof(rlm_sqlcounter_t,sqlmod_inst), NULL, NULL },

	{ "key", PW_TYPE_STRING_PTR | PW_TYPE_ATTRIBUTE,
	  offsetof(rlm_sqlcounter_t,key_name), NULL, NULL },
	{ "query", PW_TYPE_STRING_PTR | PW_TYPE_REQUIRED,
	  offsetof(rlm_sqlcounter_t,query), NULL, NULL },
	{ "reset", PW_TYPE_STRING_PTR | PW_TYPE_REQUIRED,
	  offsetof(rlm_sqlcounter_t,reset), NULL,  NULL },

	{ "counter-name", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED,
	  offsetof(rlm_sqlcounter_t,counter_name), NULL,  NULL },
	{ "counter_name", PW_TYPE_STRING_PTR | PW_TYPE_REQUIRED,
	  offsetof(rlm_sqlcounter_t,counter_name), NULL,  NULL },

	{ "check-name", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED,
	  offsetof(rlm_sqlcounter_t,check_name), NULL, NULL },
	{ "check_name", PW_TYPE_STRING_PTR | PW_TYPE_REQUIRED,
	  offsetof(rlm_sqlcounter_t,check_name), NULL, NULL },

	{ "reply-name", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED,
	  offsetof(rlm_sqlcounter_t,reply_name), NULL, NULL },
	{ "reply_name", PW_TYPE_STRING_PTR | PW_TYPE_ATTRIBUTE,
	  offsetof(rlm_sqlcounter_t,reply_name), NULL, "Session-Timeout" },

	{ NULL, -1, 0, NULL, NULL }
};

static int find_next_reset(rlm_sqlcounter_t *inst, time_t timeval)
{
	int ret = 0;
	size_t len;
	unsigned int num = 1;
	char last = '\0';
	struct tm *tm, s_tm;
	char sCurrentTime[40], sNextTime[40];

	tm = localtime_r(&timeval, &s_tm);
	len = strftime(sCurrentTime, sizeof(sCurrentTime), "%Y-%m-%d %H:%M:%S", tm);
	if (len == 0) *sCurrentTime = '\0';
	tm->tm_sec = tm->tm_min = 0;

	rad_assert(inst->reset != NULL);

	if (isdigit((int) inst->reset[0])){
		len = strlen(inst->reset);
		if (len == 0)
			return -1;
		last = inst->reset[len - 1];
		if (!isalpha((int) last))
			last = 'd';
		num = atoi(inst->reset);
		DEBUG("rlm_sqlcounter: num=%d, last=%c",num,last);
	}
	if (strcmp(inst->reset, "hourly") == 0 || last == 'h') {
		/*
		 *  Round up to the next nearest hour.
		 */
		tm->tm_hour += num;
		inst->reset_time = mktime(tm);
	} else if (strcmp(inst->reset, "daily") == 0 || last == 'd') {
		/*
		 *  Round up to the next nearest day.
		 */
		tm->tm_hour = 0;
		tm->tm_mday += num;
		inst->reset_time = mktime(tm);
	} else if (strcmp(inst->reset, "weekly") == 0 || last == 'w') {
		/*
		 *  Round up to the next nearest week.
		 */
		tm->tm_hour = 0;
		tm->tm_mday += (7 - tm->tm_wday) +(7*(num-1));
		inst->reset_time = mktime(tm);
	} else if (strcmp(inst->reset, "monthly") == 0 || last == 'm') {
		tm->tm_hour = 0;
		tm->tm_mday = 1;
		tm->tm_mon += num;
		inst->reset_time = mktime(tm);
	} else if (strcmp(inst->reset, "never") == 0) {
		inst->reset_time = 0;
	} else {
		return -1;
	}

	len = strftime(sNextTime, sizeof(sNextTime),"%Y-%m-%d %H:%M:%S",tm);
	if (len == 0) *sNextTime = '\0';
	DEBUG2("rlm_sqlcounter: Current Time: %" PRId64 " [%s], Next reset %" PRId64 " [%s]",
	       (int64_t) timeval, sCurrentTime, (int64_t) inst->reset_time, sNextTime);

	return ret;
}


/*  I don't believe that this routine handles Daylight Saving Time adjustments
    properly.  Any suggestions?
*/

static int find_prev_reset(rlm_sqlcounter_t *inst, time_t timeval)
{
	int ret = 0;
	size_t len;
	unsigned int num = 1;
	char last = '\0';
	struct tm *tm, s_tm;
	char sCurrentTime[40], sPrevTime[40];

	tm = localtime_r(&timeval, &s_tm);
	len = strftime(sCurrentTime, sizeof(sCurrentTime), "%Y-%m-%d %H:%M:%S", tm);
	if (len == 0) *sCurrentTime = '\0';
	tm->tm_sec = tm->tm_min = 0;

	rad_assert(inst->reset != NULL);

	if (isdigit((int) inst->reset[0])){
		len = strlen(inst->reset);
		if (len == 0)
			return -1;
		last = inst->reset[len - 1];
		if (!isalpha((int) last))
			last = 'd';
		num = atoi(inst->reset);
		DEBUG("rlm_sqlcounter: num=%d, last=%c",num,last);
	}
	if (strcmp(inst->reset, "hourly") == 0 || last == 'h') {
		/*
		 *  Round down to the prev nearest hour.
		 */
		tm->tm_hour -= num - 1;
		inst->last_reset = mktime(tm);
	} else if (strcmp(inst->reset, "daily") == 0 || last == 'd') {
		/*
		 *  Round down to the prev nearest day.
		 */
		tm->tm_hour = 0;
		tm->tm_mday -= num - 1;
		inst->last_reset = mktime(tm);
	} else if (strcmp(inst->reset, "weekly") == 0 || last == 'w') {
		/*
		 *  Round down to the prev nearest week.
		 */
		tm->tm_hour = 0;
		tm->tm_mday -= (7 - tm->tm_wday) +(7*(num-1));
		inst->last_reset = mktime(tm);
	} else if (strcmp(inst->reset, "monthly") == 0 || last == 'm') {
		tm->tm_hour = 0;
		tm->tm_mday = 1;
		tm->tm_mon -= num - 1;
		inst->last_reset = mktime(tm);
	} else if (strcmp(inst->reset, "never") == 0) {
		inst->reset_time = 0;
	} else {
		return -1;
	}
	len = strftime(sPrevTime, sizeof(sPrevTime), "%Y-%m-%d %H:%M:%S", tm);
	if (len == 0) *sPrevTime = '\0';
	DEBUG2("rlm_sqlcounter: Current Time: %" PRId64 " [%s], Prev reset %" PRId64 " [%s]",
	       (int64_t) timeval, sCurrentTime, (int64_t) inst->last_reset, sPrevTime);

	return ret;
}


/*
 *	Replace %<whatever> in a string.
 *
 *	%b	last_reset
 *	%e	reset_time
 *	%k	key_name
 *	%S	sqlmod_inst
 *
 */

static size_t sqlcounter_expand(char *out, int outlen, char const *fmt, rlm_sqlcounter_t *inst)
{
	int c,freespace;
	char const *p;
	char *q;
	char tmpdt[40]; /* For temporary storing of dates */

	q = out;
	for (p = fmt; *p ; p++) {
	/* Calculate freespace in output */
	freespace = outlen - (q - out);
		if (freespace <= 1)
			return -1;
		c = *p;
		if ((c != '%') && (c != '\\')) {
			*q++ = *p;
			continue;
		}
		if (*++p == '\0') break;
		if (c == '\\') switch(*p) {
			case '\\':
				*q++ = *p;
				break;
			case 't':
				*q++ = '\t';
				break;
			case 'n':
				*q++ = '\n';
				break;
			default:
				*q++ = c;
				*q++ = *p;
				break;

		} else if (c == '%') switch(*p) {

			case '%':
				*q++ = *p;
				break;
			case 'b': /* last_reset */
				snprintf(tmpdt, sizeof(tmpdt), "%" PRId64, (int64_t) inst->last_reset);
				strlcpy(q, tmpdt, freespace);
				q += strlen(q);
				break;
			case 'e': /* reset_time */
				snprintf(tmpdt, sizeof(tmpdt), "%" PRId64, (int64_t) inst->reset_time);
				strlcpy(q, tmpdt, freespace);
				q += strlen(q);
				break;
			case 'k': /* Key Name */
				WDEBUG2("Please replace '%%k' with '${key}'");
				strlcpy(q, inst->key_name, freespace);
				q += strlen(q);
				break;
			default:
				*q++ = '%';
				*q++ = *p;
				break;
		}
	}
	*q = '\0';

	DEBUG2("sqlcounter_expand:  '%s'", out);

	return strlen(out);
}


/*
 *	See if the counter matches.
 */
static int sqlcounter_cmp(void *instance, REQUEST *request, UNUSED VALUE_PAIR *req , VALUE_PAIR *check,
			  UNUSED VALUE_PAIR *check_pairs, UNUSED VALUE_PAIR **reply_pairs)
{
	rlm_sqlcounter_t *inst = instance;
	int counter;

	char *p;
	char query[MAX_QUERY_LEN];
	char *expanded = NULL;

	size_t len;

	len = snprintf(query, sizeof(query), "%%{%s:%s}", inst->sqlmod_inst, query);
	if (len >= sizeof(query) - 1) {
		REDEBUG("Insufficient query buffer space");

		return RLM_MODULE_FAIL;
	}

	p = query + len;

	/* first, expand %k, %b and %e in query */
	len = sqlcounter_expand(p, p - query, inst->query, inst);
	if (len <= 0) {
		REDEBUG("Insufficient query buffer space");

		return RLM_MODULE_FAIL;
	}

	p += len;

	if ((p - query) < 2) {
		REDEBUG("Insufficient query buffer space");

		return RLM_MODULE_FAIL;
	}

	p[0] = '}';
	p[1] = '\0';

	/* Finally, xlat resulting SQL query */
	if (radius_axlat(&expanded, request, query, NULL, NULL) < 0) {
		return RLM_MODULE_FAIL;
	}

	counter = atoi(expanded);
	talloc_free(expanded);

	return counter - check->vp_integer;
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
	rlm_sqlcounter_t *inst = instance;
	DICT_ATTR const *dattr;
	ATTR_FLAGS flags;
	time_t now;

	rad_assert(inst->query && *inst->query);

	dattr = dict_attrbyname(inst->key_name);
	rad_assert(dattr != NULL);
	if (dattr->vendor != 0) {
		cf_log_err_cs(conf, "Configuration item 'key' cannot be a VSA");
		return -1;
	}
	inst->key_attr = dattr;

	dattr = dict_attrbyname(inst->reply_name);
	rad_assert(dattr != NULL);
	if (dattr->vendor != 0) {
		cf_log_err_cs(conf, "Configuration item 'reply_name' cannot be a VSA");
		return -1;
	}
	inst->reply_attr = dattr;

	DEBUG2("rlm_sqlcounter: Reply attribute %s is number %d",
		       inst->reply_name, dattr->attr);

	/*
	 *  Create a new attribute for the counter.
	 */
	rad_assert(inst->counter_name && *inst->counter_name);
	memset(&flags, 0, sizeof(flags));
	dict_addattr(inst->counter_name, -1, 0, PW_TYPE_INTEGER, flags);
	dattr = dict_attrbyname(inst->counter_name);
	if (!dattr) {
		cf_log_err_cs(conf, "Failed to create counter attribute %s",
				inst->counter_name);
		return -1;
	}
	if (dattr->vendor != 0) {
		cf_log_err_cs(conf, "Counter attribute must not be a VSA");
		return -1;
	}
	inst->dict_attr = dattr;

	/*
	 * Create a new attribute for the check item.
	 */
	rad_assert(inst->check_name && *inst->check_name);
	dict_addattr(inst->check_name, 0, PW_TYPE_INTEGER, -1, flags);
	dattr = dict_attrbyname(inst->check_name);
	if (!dattr) {
		cf_log_err_cs(conf, "Failed to create check attribute %s",
				inst->check_name);
		return -1;
	}
	DEBUG2("rlm_sqlcounter: Check attribute %s is number %d",
			inst->check_name, dattr->attr);

	now = time(NULL);
	inst->reset_time = 0;

	if (find_next_reset(inst,now) == -1) {
		cf_log_err_cs(conf, "Invalid reset '%s'", inst->reset);
		return -1;
	}

	/*
	 *  Discover the beginning of the current time period.
	 */
	inst->last_reset = 0;

	if (find_prev_reset(inst, now) < 0) {
		cf_log_err_cs(conf, "Invalid reset '%s'", inst->reset);
		return -1;
	}

	/*
	 *	Register the counter comparison operation.
	 */
	paircompare_register(inst->dict_attr, NULL, true, sqlcounter_cmp, inst);

	return 0;
}

/*
 *	Find the named user in this modules database.  Create the set
 *	of attribute-value pairs to check and reply with for this user
 *	from the database. The authentication code only needs to check
 *	the password, the rest is done here.
 */
static rlm_rcode_t mod_authorize(UNUSED void *instance, UNUSED REQUEST *request)
{
	rlm_sqlcounter_t *inst = instance;
	int rcode = RLM_MODULE_NOOP;
	unsigned int counter;
	DICT_ATTR const *dattr;
	VALUE_PAIR *key_vp, *check_vp;
	VALUE_PAIR *reply_item;
	char msg[128];

	char *p;
	char query[MAX_QUERY_LEN];
	char *expanded = NULL;

	size_t len;

	/*
	 *	Before doing anything else, see if we have to reset
	 *	the counters.
	 */
	if (inst->reset_time && (inst->reset_time <= request->timestamp)) {
		/*
		 *	Re-set the next time and prev_time for this counters range
		 */
		inst->last_reset = inst->reset_time;
		find_next_reset(inst,request->timestamp);
	}

	/*
	 *      Look for the key.  User-Name is special.  It means
	 *      The REAL username, after stripping.
	 */
	RDEBUG2("Entering module authorize code");
	key_vp = ((inst->key_attr->vendor == 0) && (inst->key_attr->attr == PW_USER_NAME)) ?
			request->username :
			pairfind(request->packet->vps, inst->key_attr->attr, inst->key_attr->vendor, TAG_ANY);
	if (!key_vp) {
		RDEBUG2("Could not find Key value pair");
		return rcode;
	}

	/*
	 *      Look for the check item
	 */
	if ((dattr = dict_attrbyname(inst->check_name)) == NULL) {
		return rcode;
	}
	/* DEBUG2("rlm_sqlcounter: Found Check item attribute %d", dattr->attr); */
	if ((check_vp = pairfind(request->config_items, dattr->attr, dattr->vendor, TAG_ANY)) == NULL) {
		RDEBUG2("Could not find Check item value pair");
		return rcode;
	}

	len = snprintf(query, sizeof(query), "%%{%s:%s}", inst->sqlmod_inst, query);
	if (len >= sizeof(query) - 1) {
		REDEBUG("Insufficient query buffer space");

		return RLM_MODULE_FAIL;
	}

	p = query + len;

	/* first, expand %k, %b and %e in query */
	len = sqlcounter_expand(p, p - query, inst->query, inst);
	if (len <= 0) {
		REDEBUG("Insufficient query buffer space");

		return RLM_MODULE_FAIL;
	}

	p += len;

	if ((p - query) < 2) {
		REDEBUG("Insufficient query buffer space");

		return RLM_MODULE_FAIL;
	}

	p[0] = '}';
	p[1] = '\0';

	/* Finally, xlat resulting SQL query */
	if (radius_axlat(&expanded, request, query, NULL, NULL) < 0) {
		return RLM_MODULE_FAIL;
	}

	if (sscanf(expanded, "%u", &counter) != 1) {
		RDEBUG2("No integer found in string \"%s\"", expanded);
		return RLM_MODULE_NOOP;
	}

	talloc_free(expanded);

	/*
	 * Check if check item > counter
	 */
	if (check_vp->vp_integer > counter) {
		unsigned int res = check_vp->vp_integer - counter;

		RDEBUG2("Check item is greater than query result");
		/*
		 *	We are assuming that simultaneous-use=1. But
		 *	even if that does not happen then our user
		 *	could login at max for 2*max-usage-time Is
		 *	that acceptable?
		 */

		/*
		 *	If we are near a reset then add the next
		 *	limit, so that the user will not need to login
		 *	again.  Do this only for Session-Timeout.
		 */
		if ((inst->reply_attr->attr == PW_SESSION_TIMEOUT) && inst->reset_time &&
		    (res >= (inst->reset_time - request->timestamp))) {
			res = inst->reset_time - request->timestamp;
			res += check_vp->vp_integer;
		}

		/*
		 *	Limit the reply attribute to the minimum of
		 *	the existing value, or this new one.
		 */
		reply_item = pairfind(request->reply->vps, inst->reply_attr->attr, inst->reply_attr->vendor, TAG_ANY);
		if (reply_item) {
			if (reply_item->vp_integer > res) {
				reply_item->vp_integer = res;
			}
		} else {
			reply_item = radius_paircreate(request, &request->reply->vps, inst->reply_attr->attr,
						       inst->reply_attr->vendor);
			reply_item->vp_integer = res;
		}

		rcode = RLM_MODULE_OK;

		RDEBUG2("Authorized user %s, check_item=%u, counter=%u", key_vp->vp_strvalue, check_vp->vp_integer,
			counter);
		RDEBUG2("Sent Reply-Item for user %s, Type=%s, value=%u", key_vp->vp_strvalue, inst->reply_name,
			reply_item->vp_integer);
	}
	else{
		RDEBUG2("(Check item - counter) is less than zero");

		/*
		 * User is denied access, send back a reply message
		 */
		snprintf(msg, sizeof(msg), "Your maximum %s usage time has been reached", inst->reset);
		pairmake_reply("Reply-Message", msg, T_OP_EQ);

		REDEBUG("Maximum %s usage time reached",
				   inst->reset);
		rcode = RLM_MODULE_REJECT;

		RDEBUG2("Rejected user %s, check_item=%u, counter=%u", key_vp->vp_strvalue,
		        check_vp->vp_integer,counter);
	}

	return rcode;
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
module_t rlm_sqlcounter = {
	RLM_MODULE_INIT,
	"SQL Counter",
	RLM_TYPE_THREAD_SAFE,		/* type */
	sizeof(rlm_sqlcounter_t),
	module_config,
	mod_instantiate,		/* instantiation */
	NULL,				/* detach */
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

