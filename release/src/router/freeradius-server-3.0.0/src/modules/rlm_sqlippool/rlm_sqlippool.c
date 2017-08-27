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
 * @file rlm_sqlippool.c
 * @brief Allocates an IPv4 address from pools stored in SQL.
 *
 * @copyright 2002  Globe.Net Communications Limited
 * @copyright 2006  The FreeRADIUS server project
 * @copyright 2006  Suntel Communications
 */
RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/rad_assert.h>

#include <ctype.h>

#include <rlm_sql.h>

/*
 *	Define a structure for our module configuration.
 */
typedef struct rlm_sqlippool_t {
	char *sql_instance_name;

	int lease_duration;

	rlm_sql_t *sql_inst;

	char *pool_name;

	/* We ended up removing the init
	   queries so that its up to user
	   to create the db structure and put the required
	   information in there
	*/
				/* Allocation sequence */
	char *allocate_begin;	/* SQL query to begin */
	char *allocate_clear;	/* SQL query to clear an IP */
	char *allocate_find;	/* SQL query to find an unused IP */
	char *allocate_update;	/* SQL query to mark an IP as used */
	char *allocate_commit;	/* SQL query to commit */

	char *pool_check;	/* Query to check for the existence of the pool */

				/* Start sequence */
	char *start_begin;	/* SQL query to begin */
	char *start_update;	/* SQL query to update an IP entry */
	char *start_commit;	/* SQL query to commit */

				/* Alive sequence */
	char *alive_begin;	/* SQL query to begin */
	char *alive_update;	/* SQL query to update an IP entry */
	char *alive_commit;	/* SQL query to commit */

				/* Stop sequence */
	char *stop_begin;	/* SQL query to begin */
	char *stop_clear;	/* SQL query to clear an IP */
	char *stop_commit;	/* SQL query to commit */

				/* On sequence */
	char *on_begin;		/* SQL query to begin */
	char *on_clear;		/* SQL query to clear an entire NAS */
	char *on_commit;	/* SQL query to commit */

				/* Off sequence */
	char *off_begin;	/* SQL query to begin */
	char *off_clear;	/* SQL query to clear an entire NAS */
	char *off_commit;	/* SQL query to commit */

				/* Logging Section */
	char *log_exists;	/* There was an ip address already assigned */
	char *log_success;	/* We successfully allocated ip address from pool */
	char *log_clear;	/* We successfully deallocated ip address from pool */
	char *log_failed;	/* Failed to allocate ip from the pool */
	char *log_nopool;	/* There was no Framed-IP-Address but also no Pool-Name */

				/* Reserved to handle 255.255.255.254 Requests */
	char *defaultpool;	/* Default Pool-Name if there is none in the check items */

} rlm_sqlippool_t;

static CONF_PARSER message_config[] = {
	{ "exists", PW_TYPE_STRING_PTR, offsetof(rlm_sqlippool_t, log_exists), NULL, NULL },
	{ "success", PW_TYPE_STRING_PTR, offsetof(rlm_sqlippool_t, log_success), NULL, NULL },
	{ "clear", PW_TYPE_STRING_PTR, offsetof(rlm_sqlippool_t, log_clear), NULL, NULL },
	{ "failed", PW_TYPE_STRING_PTR, offsetof(rlm_sqlippool_t, log_failed), NULL, NULL },
	{ "nopool", PW_TYPE_STRING_PTR, offsetof(rlm_sqlippool_t, log_nopool), NULL, NULL },

	{ NULL, -1, 0, NULL, NULL }
};

/*
 *	A mapping of configuration file names to internal variables.
 *
 *	Note that the string is dynamically allocated, so it MUST
 *	be freed.  When the configuration file parse re-reads the string,
 *	it free's the old one, and strdup's the new one, placing the pointer
 *	to the strdup'd string into 'config.string'.  This gets around
 *	buffer over-flows.
 */
static CONF_PARSER module_config[] = {
	{"sql-instance-name",PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED,
	 offsetof(rlm_sqlippool_t,sql_instance_name), NULL, NULL},
	{"sql_module_instance",PW_TYPE_STRING_PTR | PW_TYPE_REQUIRED,
	 offsetof(rlm_sqlippool_t,sql_instance_name), NULL, "sql"},

	{ "lease-duration", PW_TYPE_INTEGER | PW_TYPE_DEPRECATED, offsetof(rlm_sqlippool_t,lease_duration), NULL, NULL},
	{ "lease_duration", PW_TYPE_INTEGER, offsetof(rlm_sqlippool_t,lease_duration), NULL, "86400"},

	{ "pool-name", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, offsetof(rlm_sqlippool_t, pool_name), NULL, NULL},
	{ "pool_name", PW_TYPE_STRING_PTR, offsetof(rlm_sqlippool_t, pool_name), NULL, ""},

	{ "default-pool", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, offsetof(rlm_sqlippool_t, defaultpool), NULL, NULL },
	{ "default_pool", PW_TYPE_STRING_PTR, offsetof(rlm_sqlippool_t, defaultpool), NULL, "main_pool" },


	{ "allocate-begin", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED,
	  offsetof(rlm_sqlippool_t,allocate_begin), NULL, NULL},
	{ "allocate_begin", PW_TYPE_STRING_PTR,
	  offsetof(rlm_sqlippool_t,allocate_begin), NULL, "START TRANSACTION" },

	{ "allocate-clear", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED,
	  offsetof(rlm_sqlippool_t,allocate_clear), NULL, NULL},
	{ "allocate_clear", PW_TYPE_STRING_PTR | PW_TYPE_REQUIRED,
	  offsetof(rlm_sqlippool_t,allocate_clear), NULL, "" },

	{ "allocate-find", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED,
	  offsetof(rlm_sqlippool_t,allocate_find), NULL, NULL},
	{ "allocate_find", PW_TYPE_STRING_PTR | PW_TYPE_REQUIRED,
	  offsetof(rlm_sqlippool_t,allocate_find), NULL, "" },

	{ "allocate-update", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED,
	  offsetof(rlm_sqlippool_t,allocate_update), NULL, NULL },
	{ "allocate_update", PW_TYPE_STRING_PTR | PW_TYPE_REQUIRED,
	  offsetof(rlm_sqlippool_t,allocate_update), NULL, "" },

	{ "allocate-commit", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED,
	  offsetof(rlm_sqlippool_t,allocate_commit), NULL, NULL },
	{ "allocate_commit", PW_TYPE_STRING_PTR,
	  offsetof(rlm_sqlippool_t,allocate_commit), NULL, "COMMIT" },


	{ "pool-check", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, offsetof(rlm_sqlippool_t,pool_check), NULL, NULL },
	{ "pool_check", PW_TYPE_STRING_PTR, offsetof(rlm_sqlippool_t,pool_check), NULL, "" },


	{ "start-begin", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, offsetof(rlm_sqlippool_t,start_begin), NULL, NULL },
	{ "start_begin", PW_TYPE_STRING_PTR, offsetof(rlm_sqlippool_t,start_begin), NULL, "START TRANSACTION" },

	{ "start-update", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, offsetof(rlm_sqlippool_t,start_update), NULL, NULL },
	{ "start_update", PW_TYPE_STRING_PTR | PW_TYPE_REQUIRED, offsetof(rlm_sqlippool_t,start_update), NULL, "" },

	{ "start-commit", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, offsetof(rlm_sqlippool_t,start_commit), NULL, NULL },
	{ "start_commit", PW_TYPE_STRING_PTR, offsetof(rlm_sqlippool_t,start_commit), NULL, "COMMIT" },


	{ "alive-begin", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, offsetof(rlm_sqlippool_t,alive_begin), NULL, NULL },
	{ "alive_begin", PW_TYPE_STRING_PTR, offsetof(rlm_sqlippool_t,alive_begin), NULL, "START TRANSACTION" },

	{ "alive-update", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, offsetof(rlm_sqlippool_t,alive_update), NULL, NULL },
	{ "alive_update", PW_TYPE_STRING_PTR | PW_TYPE_REQUIRED, offsetof(rlm_sqlippool_t,alive_update), NULL, "" },

	{ "alive-commit", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, offsetof(rlm_sqlippool_t,alive_commit), NULL, NULL },
	{ "alive_commit", PW_TYPE_STRING_PTR, offsetof(rlm_sqlippool_t,alive_commit), NULL, "COMMIT" },


	{ "stop-begin", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, offsetof(rlm_sqlippool_t,stop_begin), NULL, NULL },
	{ "stop_begin", PW_TYPE_STRING_PTR, offsetof(rlm_sqlippool_t,stop_begin), NULL, "START TRANSACTION" },

	{ "stop-clear", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, offsetof(rlm_sqlippool_t,stop_clear), NULL, NULL },
	{ "stop_clear", PW_TYPE_STRING_PTR | PW_TYPE_REQUIRED, offsetof(rlm_sqlippool_t,stop_clear), NULL, "" },

	{ "stop-commit", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, offsetof(rlm_sqlippool_t,stop_commit), NULL, NULL },
	{ "stop_commit", PW_TYPE_STRING_PTR, offsetof(rlm_sqlippool_t,stop_commit), NULL, "COMMIT" },


	{ "on-begin", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, offsetof(rlm_sqlippool_t,on_begin), NULL, NULL },
	{ "on_begin", PW_TYPE_STRING_PTR, offsetof(rlm_sqlippool_t,on_begin), NULL, "START TRANSACTION" },

	{ "on-clear", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, offsetof(rlm_sqlippool_t,on_clear), NULL, NULL },
	{ "on_clear", PW_TYPE_STRING_PTR | PW_TYPE_REQUIRED, offsetof(rlm_sqlippool_t,on_clear), NULL, "" },

	{ "on-commit", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, offsetof(rlm_sqlippool_t,on_commit), NULL, NULL },
	{ "on_commit", PW_TYPE_STRING_PTR, offsetof(rlm_sqlippool_t,on_commit), NULL, "COMMIT" },


	{ "off-begin", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, offsetof(rlm_sqlippool_t,off_begin), NULL, NULL },
	{ "off_begin", PW_TYPE_STRING_PTR, offsetof(rlm_sqlippool_t,off_begin), NULL, "START TRANSACTION" },

	{ "off-clear", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, offsetof(rlm_sqlippool_t,off_clear), NULL, NULL },
	{ "off_clear", PW_TYPE_STRING_PTR | PW_TYPE_REQUIRED, offsetof(rlm_sqlippool_t,off_clear), NULL, "" },

	{ "off-commit",PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, offsetof(rlm_sqlippool_t,off_commit), NULL, NULL },
	{ "off_commit", PW_TYPE_STRING_PTR, offsetof(rlm_sqlippool_t,off_commit), NULL, "COMMIT" },

	{ "messages", PW_TYPE_SUBSECTION, 0, NULL, (void const *) message_config },

	{ NULL, -1, 0, NULL, NULL }
};

/*
 *	Replace %<whatever> in a string.
 *
 *	%P	pool_name
 *	%I	param
 *	%J	lease_duration
 *
 */
static int sqlippool_expand(char * out, int outlen, char const * fmt,
			    rlm_sqlippool_t *data, char * param, int param_len)
{
	char *q;
	char const *p;
	char tmp[40]; /* For temporary storing of integers */

	q = out;
	for (p = fmt; *p ; p++) {
		int freespace;
		int c;

		/* Calculate freespace in output */
		freespace = outlen - (q - out);
		if (freespace <= 1)
			break;

		c = *p;
		if (c != '%' && c != '$' && c != '\\') {
			*q++ = *p;
			continue;
		}

		if (*++p == '\0')
			break;

		if (c == '\\') {
			switch(*p) {
			case '\\':
				*q++ = '\\';
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
			}
		}
		else if (c == '%') {
			switch(*p) {
			case '%':
				*q++ = *p;
				break;
			case 'P': /* pool name */
				strlcpy(q, data->pool_name, freespace);
				q += strlen(q);
				break;
			case 'I': /* IP address */
				if (param && param_len > 0) {
					if (param_len > freespace) {
						strlcpy(q, param, freespace);
						q += strlen(q);
					}
					else {
						memcpy(q, param, param_len);
						q += param_len;
					}
				}
				break;
			case 'J': /* lease duration */
				sprintf(tmp, "%d", data->lease_duration);
				strlcpy(q, tmp, freespace);
				q += strlen(q);
				break;
			default:
				*q++ = '%';
				*q++ = *p;
				break;
			}
		}
	}
	*q = '\0';

#if 0
	DEBUG2("sqlippool_expand: \"%s\"", out);
#endif

	return strlen(out);
}

/*
 * Query the database executing a command with no result rows
 */
static int sqlippool_command(char const * fmt, rlm_sql_handle_t * handle, rlm_sqlippool_t *data, REQUEST * request,
			     char * param, int param_len)
{
	char query[MAX_QUERY_LEN];
	char *expanded = NULL;

	int ret;

	/*
	 *	If we don't have a command, do nothing.
	 */
	if (!*fmt) return 0;

	/*
	 *	@todo this needs to die (should just be done in xlat expansion)
	 */
	sqlippool_expand(query, sizeof(query), fmt, data, param, param_len);

	if (radius_axlat(&expanded, request, query, data->sql_inst->sql_escape_func, data->sql_inst) < 0) {
		return 0;
	}

	ret = data->sql_inst->sql_query(&handle, data->sql_inst, expanded);
	if (!ret){
		REDEBUG("database query error in: '%s'", expanded);
		talloc_free(expanded);

		return 0;
	}
	talloc_free(expanded);

	(data->sql_inst->module->sql_finish_query)(handle, data->sql_inst->config);
	return 0;
}

/*
 *	Don't repeat yourself
 */
#undef DO
#define DO(_x) sqlippool_command(inst->_x, handle, inst, request, NULL, 0)

/*
 * Query the database expecting a single result row
 */
static int sqlippool_query1(char *out, int outlen, char const *fmt,
			    rlm_sql_handle_t *handle, rlm_sqlippool_t *data,
			    REQUEST *request, char *param, int param_len)
{
	char query[MAX_QUERY_LEN];
	char *expanded = NULL;

	int rlen, retval;

	/*
	 *	@todo this needs to die (should just be done in xlat expansion)
	 */
	sqlippool_expand(query, sizeof(query), fmt, data, param, param_len);

	rad_assert(request != NULL);

	*out = '\0';

	/*
	 *	Do an xlat on the provided string
	 */
	if (radius_axlat(&expanded, request, query, data->sql_inst->sql_escape_func, data->sql_inst) < 0) {
		return 0;
	}
	retval = data->sql_inst->sql_select_query(&handle, data->sql_inst, expanded);
	talloc_free(expanded);

	if (retval != 0){
		REDEBUG("database query error on '%s'", query);

		return 0;
	}

	if (!data->sql_inst->sql_fetch_row(&handle, data->sql_inst)) {
		if (handle->row) {
			if (handle->row[0]) {
				if ((rlen = strlen(handle->row[0])) < outlen) {
					strcpy(out, handle->row[0]);
					retval = rlen;
				} else {
					RDEBUG("insufficient string space");
				}
			} else {
				RDEBUG("row[0] returned NULL");
			}
		} else {
			RDEBUG("SQL query did not return any results");
		}
	} else {
		RDEBUG("SQL query did not succeed");
	}

	(data->sql_inst->module->sql_finish_select_query)(handle, data->sql_inst->config);

	return retval;
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
	module_instance_t *sqlinst;
	rlm_sqlippool_t *inst = instance;
	char const *pool_name = NULL;

	pool_name = cf_section_name2(conf);
	if (pool_name != NULL)
		inst->pool_name = talloc_strdup(inst, pool_name);
	else
		inst->pool_name = talloc_strdup(inst, "ippool");

	sqlinst = find_module_instance(cf_section_find("modules"),
				       inst->sql_instance_name, 1);
	if (!sqlinst) {
		cf_log_err_cs(conf, "failed to find sql instance named %s",
			   inst->sql_instance_name);
		return -1;
	}

	if (strcmp(sqlinst->entry->name, "rlm_sql") != 0) {
		cf_log_err_cs(conf, "Module \"%s\""
		       " is not an instance of the rlm_sql module",
		       inst->sql_instance_name);
		return -1;
	}

	inst->sql_inst = (rlm_sql_t *) sqlinst->insthandle;
	return 0;
}


/*
 *	If we have something to log, then we log it.
 *	Otherwise we return the retcode as soon as possible
 */
static int do_logging(REQUEST *request, char *str, int rcode)
{
	char *expanded = NULL;

	if (!str || !*str) return rcode;

	if (radius_axlat(&expanded, request, str, NULL, NULL) < 0) {
		return rcode;
	}

	pairmake_config("Module-Success-Message", expanded, T_OP_SET);

	talloc_free(expanded);

	return rcode;
}


/*
 *	Allocate an IP number from the pool.
 */
static rlm_rcode_t mod_post_auth(void *instance, REQUEST *request)
{
	rlm_sqlippool_t *inst = (rlm_sqlippool_t *) instance;
	char allocation[MAX_STRING_LEN];
	int allocation_len;
	uint32_t ip_allocation;
	VALUE_PAIR *vp;
	rlm_sql_handle_t *handle;
	fr_ipaddr_t ipaddr;

	/*
	 *	If there is a Framed-IP-Address attribute in the reply do nothing
	 */
	if (pairfind(request->reply->vps, PW_FRAMED_IP_ADDRESS, 0, TAG_ANY) != NULL) {
		RDEBUG("Framed-IP-Address already exists");

		return do_logging(request, inst->log_exists, RLM_MODULE_NOOP);
	}

	if (pairfind(request->config_items, PW_POOL_NAME, 0, TAG_ANY) == NULL) {
		RDEBUG("No Pool-Name defined.");

		return do_logging(request, inst->log_nopool, RLM_MODULE_NOOP);
	}

	handle = inst->sql_inst->sql_get_socket(inst->sql_inst);
	if (!handle) {
		REDEBUG("cannot get sql connection");
		return RLM_MODULE_FAIL;
	}

	if (inst->sql_inst->sql_set_user(inst->sql_inst, request, NULL) < 0) {
		return RLM_MODULE_FAIL;
	}

	DO(allocate_begin);
	DO(allocate_clear);

	allocation_len = sqlippool_query1(allocation, sizeof(allocation),
					  inst->allocate_find, handle,
					  inst, request, (char *) NULL, 0);

	/*
	 *	Nothing found...
	 */
	if (allocation_len == 0) {
		DO(allocate_commit);

		/*
		 *Should we perform pool-check ?
		 */
		if (inst->pool_check && *inst->pool_check) {

			/*
			 *Ok, so the allocate-find query found nothing ...
			 *Let's check if the pool exists at all
			 */
			allocation_len = sqlippool_query1(allocation, sizeof(allocation),
							  inst->pool_check, handle, inst, request,
							  (char *) NULL, 0);

			inst->sql_inst->sql_release_socket(inst->sql_inst, handle);

			if (allocation_len) {

				/*
				 *	Pool exists after all... So,
				 *	the failure to allocate the IP
				 *	address was most likely due to
				 *	the depletion of the pool. In
				 *	that case, we should return
				 *	NOTFOUND
				 */
				RDEBUG("pool appears to be full");
				return do_logging(request, inst->log_failed, RLM_MODULE_NOTFOUND);

			}

			/*
			 *	Pool doesn't exist in the table. It
			 *	may be handled by some other instance of
			 *	sqlippool, so we should just ignore this
			 *	allocation failure and return NOOP
			 */
			RDEBUG("IP address could not be allocated as no pool exists with that name.");
			return RLM_MODULE_NOOP;

		}

		inst->sql_inst->sql_release_socket(inst->sql_inst, handle);

		RDEBUG("IP address could not be allocated.");
		return do_logging(request, inst->log_failed, RLM_MODULE_NOOP);
	}

	/*
	 *	FIXME: Make it work with the ipv6 addresses
	 */
	if ((ip_hton(allocation, AF_INET, &ipaddr) < 0) ||
	    ((ip_allocation = ipaddr.ipaddr.ip4addr.s_addr) == INADDR_NONE)) {
		DO(allocate_commit);

		RDEBUG("Invalid IP number [%s] returned from instbase query.", allocation);
		inst->sql_inst->sql_release_socket(inst->sql_inst, handle);
		return do_logging(request, inst->log_failed, RLM_MODULE_NOOP);
	}

	/*
	 *	UPDATE
	 */
	sqlippool_command(inst->allocate_update, handle, inst, request,
			  allocation, allocation_len);

	RDEBUG("Allocated IP %s [%08x]", allocation, ip_allocation);

	vp = radius_paircreate(request, &request->reply->vps,
			       PW_FRAMED_IP_ADDRESS, 0);
	vp->vp_ipaddr = ip_allocation;

	DO(allocate_commit);

	inst->sql_inst->sql_release_socket(inst->sql_inst, handle);

	return do_logging(request, inst->log_success, RLM_MODULE_OK);
}

static int mod_accounting_start(rlm_sql_handle_t *handle,
				      rlm_sqlippool_t *inst, REQUEST *request)
{
	DO(start_begin);
	DO(start_update);
	DO(start_commit);

	return RLM_MODULE_OK;
}

static int mod_accounting_alive(rlm_sql_handle_t *handle,
				      rlm_sqlippool_t *inst, REQUEST *request)
{
	DO(alive_begin);
	DO(alive_update);
	DO(alive_commit);
	return RLM_MODULE_OK;
}

static int mod_accounting_stop(rlm_sql_handle_t *handle,
				      rlm_sqlippool_t *inst, REQUEST *request)
{
	DO(stop_begin);
	DO(stop_clear);
	DO(stop_commit);

	return do_logging(request, inst->log_clear, RLM_MODULE_OK);
}

static int mod_accounting_on(rlm_sql_handle_t *handle,
				      rlm_sqlippool_t *inst, REQUEST *request)
{
	DO(on_begin);
	DO(on_clear);
	DO(on_commit);

	return RLM_MODULE_OK;
}

static int mod_accounting_off(rlm_sql_handle_t *handle,
				      rlm_sqlippool_t *inst, REQUEST *request)
{
	DO(off_begin);
	DO(off_clear);
	DO(off_commit);

	return RLM_MODULE_OK;
}

/*
 *	Check for an Accounting-Stop
 *	If we find one and we have allocated an IP to this nas/port
 *	combination, then deallocate it.
 */
static rlm_rcode_t mod_accounting(void *instance, REQUEST *request)
{
	int rcode = RLM_MODULE_NOOP;
	VALUE_PAIR *vp;
	int acct_status_type;
	rlm_sqlippool_t *inst = (rlm_sqlippool_t *) instance;
	rlm_sql_handle_t *handle;

	vp = pairfind(request->packet->vps, PW_ACCT_STATUS_TYPE, 0, TAG_ANY);
	if (!vp) {
		RDEBUG("Could not find account status type in packet.");
		return RLM_MODULE_NOOP;
	}
	acct_status_type = vp->vp_integer;

	switch (acct_status_type) {
	case PW_STATUS_START:
	case PW_STATUS_ALIVE:
	case PW_STATUS_STOP:
	case PW_STATUS_ACCOUNTING_ON:
	case PW_STATUS_ACCOUNTING_OFF:
		break;		/* continue through to the next section */

	default:
		/* We don't care about any other accounting packet */
		return RLM_MODULE_NOOP;
	}

	handle = inst->sql_inst->sql_get_socket(inst->sql_inst);
	if (!handle) {
		RDEBUG("Cannot allocate sql connection");
		return RLM_MODULE_FAIL;
	}

	if (inst->sql_inst->sql_set_user(inst->sql_inst, request, NULL) < 0) {
		return RLM_MODULE_FAIL;
	}

	switch (acct_status_type) {
	case PW_STATUS_START:
		rcode = mod_accounting_start(handle, inst, request);
		break;

	case PW_STATUS_ALIVE:
		rcode = mod_accounting_alive(handle, inst, request);
		break;

	case PW_STATUS_STOP:
		rcode = mod_accounting_stop(handle, inst, request);
		break;

	case PW_STATUS_ACCOUNTING_ON:
		rcode = mod_accounting_on(handle, inst, request);
		break;

	case PW_STATUS_ACCOUNTING_OFF:
		rcode = mod_accounting_off(handle, inst, request);
		break;
	}

	inst->sql_inst->sql_release_socket(inst->sql_inst, handle);

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
module_t rlm_sqlippool = {
	RLM_MODULE_INIT,
	"sqlippool",
	RLM_TYPE_THREAD_SAFE,		/* type */
	sizeof(rlm_sqlippool_t),
	module_config,
	mod_instantiate,		/* instantiation */
	NULL,				/* detach */
	{
		NULL,			/* authentication */
		NULL,			/* authorization */
		NULL,			/* preaccounting */
		mod_accounting,	/* accounting */
		NULL,			/* checksimul */
		NULL,			/* pre-proxy */
		NULL,			/* post-proxy */
		mod_post_auth	/* post-auth */
	},
};
