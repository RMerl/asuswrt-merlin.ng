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
 * @file rlm_rediswho.c
 * @brief Session tracking using redis.
 *
 * @copyright 2000,2006  The FreeRADIUS server project
 * @copyright 2011  TekSavvy Solutions <gabe@teksavvy.com>
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>

#include <ctype.h>

#include <rlm_redis.h>

typedef struct rlm_rediswho_t {
	char const *xlat_name;
	CONF_SECTION *cs;

	char *redis_instance_name;
	REDIS_INST *redis_inst;

	/*
	 * 	expiry time in seconds if no updates are received for a user
	 */
	int expiry_time;

	/*
	 *	How many session updates to keep track of per user
	 */
	int trim_count;
} rlm_rediswho_t;

static CONF_PARSER module_config[] = {
	{ "redis-instance-name", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED,
	  offsetof(rlm_rediswho_t, redis_instance_name), NULL, NULL},
	{ "redis_module_instance", PW_TYPE_STRING_PTR,
	  offsetof(rlm_rediswho_t, redis_instance_name), NULL, "redis"},

	{ "trim-count", PW_TYPE_INTEGER | PW_TYPE_DEPRECATED,
	  offsetof(rlm_rediswho_t, trim_count), NULL, NULL},
	{ "trim_count", PW_TYPE_INTEGER,
	  offsetof(rlm_rediswho_t, trim_count), NULL, "-1"},

	{ NULL, -1, 0, NULL, NULL}
};

/*
 *	Query the database executing a command with no result rows
 */
static int rediswho_command(char const *fmt, REDISSOCK **dissocket_p,
			    rlm_rediswho_t *inst, REQUEST *request)
{
	REDISSOCK *dissocket;
	int result = 0;

	if (!fmt) {
		return 0;
	}

	if (inst->redis_inst->redis_query(dissocket_p, inst->redis_inst,
					  fmt, request) < 0) {

		ERROR("rediswho_command: database query error in: '%s'", fmt);
		return -1;

	}
	dissocket = *dissocket_p;

	switch (dissocket->reply->type) {
	case REDIS_REPLY_INTEGER:
		DEBUG("rediswho_command: query response %lld\n",
		      dissocket->reply->integer);
		if (dissocket->reply->integer > 0)
			result = dissocket->reply->integer;
		break;
	case REDIS_REPLY_STATUS:
	case REDIS_REPLY_STRING:
		DEBUG("rediswho_command: query response %s\n",
		      dissocket->reply->str);
		break;
	default:
		break;
	}

	(inst->redis_inst->redis_finish_query)(dissocket);

	return result;
}

static int mod_instantiate(CONF_SECTION *conf, void *instance)
{
	module_instance_t *modinst;
	rlm_rediswho_t *inst = instance;

	inst->xlat_name = cf_section_name2(conf);

	if (!inst->xlat_name)
		inst->xlat_name = cf_section_name1(conf);

	inst->cs = conf;

	modinst = find_module_instance(cf_section_find("modules"),
				       inst->redis_instance_name, 1);
	if (!modinst) {
		ERROR("rediswho: failed to find module instance \"%s\"",
		       inst->redis_instance_name);
		return -1;
	}

	if (strcmp(modinst->entry->name, "rlm_redis") != 0) {
		ERROR("rediswho: Module \"%s\""
		       " is not an instance of the redis module",
		       inst->redis_instance_name);
		return -1;
	}

	inst->redis_inst = (REDIS_INST *) modinst->insthandle;

	return 0;
}

static int mod_accounting_all(REDISSOCK **dissocket_p,
				   rlm_rediswho_t *inst, REQUEST *request,
				   char const *insert,
				   char const *trim,
				   char const *expire)
{
	int result;

	result = rediswho_command(insert, dissocket_p, inst, request);
	if (result < 0) {
		return RLM_MODULE_FAIL;
	}

	/* Only trim if necessary */
	if (inst->trim_count >= 0 && result > inst->trim_count) {
		if (rediswho_command(trim, dissocket_p,
				     inst, request) < 0) {
			return RLM_MODULE_FAIL;
		}
	}

	if (rediswho_command(expire, dissocket_p, inst, request) < 0) {
		return RLM_MODULE_FAIL;
	}

	return RLM_MODULE_OK;
}

static rlm_rcode_t mod_accounting(void * instance, REQUEST * request)
{
	rlm_rcode_t rcode;
	VALUE_PAIR * vp;
	DICT_VALUE *dv;
	CONF_SECTION *cs;
	char const *insert, *trim, *expire;
	rlm_rediswho_t *inst = (rlm_rediswho_t *) instance;
	REDISSOCK *dissocket;

	vp = pairfind(request->packet->vps, PW_ACCT_STATUS_TYPE, 0, TAG_ANY);
	if (!vp) {
		RDEBUG("Could not find account status type in packet.");
		return RLM_MODULE_NOOP;
	}

	dv = dict_valbyattr(vp->da->attr, vp->da->vendor, vp->vp_integer);
	if (!dv) {
		RDEBUG("Unknown Acct-Status-Type %u", vp->vp_integer);
		return RLM_MODULE_NOOP;
	}

	cs = cf_section_sub_find(inst->cs, dv->name);
	if (!cs) {
		RDEBUG("No subsection %s", dv->name);
		return RLM_MODULE_NOOP;
	}

	dissocket = fr_connection_get(inst->redis_inst->pool);
	if (!dissocket) {
		RDEBUG("cannot allocate redis connection");
		return RLM_MODULE_FAIL;
	}

	insert = cf_pair_value(cf_pair_find(cs, "insert"));
	trim = cf_pair_value(cf_pair_find(cs, "trim"));
	expire = cf_pair_value(cf_pair_find(cs, "expire"));

	rcode = mod_accounting_all(&dissocket, inst, request,
					insert,
					trim,
					expire);

	if (dissocket) fr_connection_release(inst->redis_inst->pool, dissocket);

	return rcode;
}


module_t rlm_rediswho = {
	RLM_MODULE_INIT,
	"rediswho",
	RLM_TYPE_THREAD_SAFE,	/* type */
	sizeof(rlm_rediswho_t),
	module_config,
	mod_instantiate,	/* instantiation */
	NULL, 			/* detach */
	{
		NULL, /* authentication */
		NULL, /* authorization */
		NULL, /* preaccounting */
		mod_accounting, /* accounting */
		NULL, /* checksimul */
		NULL, /* pre-proxy */
		NULL, /* post-proxy */
		NULL /* post-auth */
	},
};
