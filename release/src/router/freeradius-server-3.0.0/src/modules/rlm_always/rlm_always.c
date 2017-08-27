/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 */

/**
 * $Id$
 * @file rlm_always.c
 * @brief Return preconfigured fixed rcodes.
 *
 * @copyright 2000,2006  The FreeRADIUS server project
 */
RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>

/*
 *	The instance data for rlm_always is the list of fake values we are
 *	going to return.
 */
typedef struct rlm_always_t {
	char		*rcode_str;
	rlm_rcode_t	rcode;
	int		simulcount;
	int		mpp;
} rlm_always_t;

/*
 *	A mapping of configuration file names to internal variables.
 */
static const CONF_PARSER module_config[] = {
  { "rcode",      PW_TYPE_STRING_PTR, offsetof(rlm_always_t,rcode_str),
    NULL, "fail" },
  { "simulcount", PW_TYPE_INTEGER,    offsetof(rlm_always_t,simulcount),
    NULL, "0" },
  { "mpp",	PW_TYPE_BOOLEAN,    offsetof(rlm_always_t,mpp),
    NULL, "no" },

  { NULL, -1, 0, NULL, NULL }		/* end the list */
};

static rlm_rcode_t str2rcode(char const *s)
{
	if(!strcasecmp(s, "reject"))
		return RLM_MODULE_REJECT;
	else if(!strcasecmp(s, "fail"))
		return RLM_MODULE_FAIL;
	else if(!strcasecmp(s, "ok"))
		return RLM_MODULE_OK;
	else if(!strcasecmp(s, "handled"))
		return RLM_MODULE_HANDLED;
	else if(!strcasecmp(s, "invalid"))
		return RLM_MODULE_INVALID;
	else if(!strcasecmp(s, "userlock"))
		return RLM_MODULE_USERLOCK;
	else if(!strcasecmp(s, "notfound"))
		return RLM_MODULE_NOTFOUND;
	else if(!strcasecmp(s, "noop"))
		return RLM_MODULE_NOOP;
	else if(!strcasecmp(s, "updated"))
		return RLM_MODULE_UPDATED;
	else {
		ERROR("rlm_always: Unknown module rcode '%s'.\n", s);
		return RLM_MODULE_UNKNOWN;
	}
}

static int mod_instantiate(UNUSED CONF_SECTION *conf, void *instance)
{
	rlm_always_t *inst = instance;

	/*
	 *	Convert the rcode string to an int, and get rid of it
	 */
	inst->rcode = str2rcode(inst->rcode_str);
	if (inst->rcode == RLM_MODULE_UNKNOWN) {
		return -1;
	}

	return 0;
}

/*
 *	Just return the rcode ... this function is autz, auth, acct, and
 *	preacct!
 */
static rlm_rcode_t always_return(void *instance, UNUSED REQUEST *request)
{
	return ((struct rlm_always_t *)instance)->rcode;
}

#ifdef WITH_SESSION_MGMT
/*
 *	checksimul fakes some other variables besides the rcode...
 */
static rlm_rcode_t mod_checksimul(void *instance, REQUEST *request)
{
	struct rlm_always_t *inst = instance;

	request->simul_count = inst->simulcount;

	if (inst->mpp)
		request->simul_mpp = 2;

	return inst->rcode;
}
#endif

module_t rlm_always = {
	RLM_MODULE_INIT,
	"always",
	RLM_TYPE_CHECK_CONFIG_SAFE,   	/* type */
	sizeof(rlm_always_t),		/* config size */
	module_config,			/* configuration */
	mod_instantiate,		/* instantiation */
	NULL,				/* detach */
	{
		always_return,		/* authentication */
		always_return,		/* authorization */
		always_return,		/* preaccounting */
		always_return,		/* accounting */
#ifdef WITH_SESSION_MGMT
		mod_checksimul,	/* checksimul */
#else
		NULL,
#endif
		always_return,	       	/* pre-proxy */
		always_return,		/* post-proxy */
		always_return		/* post-auth */
#ifdef WITH_COA
		,
		always_return,		/* recv-coa */
		always_return		/* send-coa */
#endif
	},
};
