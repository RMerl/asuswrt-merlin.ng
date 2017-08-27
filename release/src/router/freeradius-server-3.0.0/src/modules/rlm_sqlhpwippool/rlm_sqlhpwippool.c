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
 * @file rlm_sqlhpwippool.c
 * @brief Allocates an IPv4 address from pools defined in ASN Netvim.
 *
 * Current bugs/limits:
 * - probably works only with newer versions of MySQL (subqueries)
 * - requires FreeRADIUS' SQL user to have proper permissions on proper tables
 *   from Netvim database
 * - of course uses dirty hacks to get access to the database
 * - queries and table names are not configurable
 * - IPv4 only (I don't even care about IPv6 by now)
 * - pool names (fetched from database) are not "escaped"
 * - you have to set encoding of radius.acctuniqueid to same as
 *   netvim.ips.rsv_by
 *
 * @copyright 2005-2006 Pawel Foremski <pjf@asn.pl>,
 * @copyright 2000-2006 The FreeRADIUS server project
 */
#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>
#include <freeradius-devel/modpriv.h>

#include <ctype.h>

#include "rlm_sql.h"

#define VENDORPEC_ASN 23782
#define ASN_IP_POOL_NAME 1

#define RLM_NETVIM_LOG_FMT "rlm_sqlhpwippool(%s, line %u): %s"
#define RLM_NETVIM_MAX_ROWS 1000000
#define RLM_NETVIM_TMP_PREFIX "auth-tmp-"

static char const rcsid[] = "$Id$";

typedef struct rlm_sqlhpwippool_t {
	char const *myname;	 /* name of this instance */
	rlm_sql_t *sqlinst;	  /* rlm_sql_t for requested instance */
	rlm_sql_module_t *db;       /* here the fun takes place ;-) */
#ifdef HAVE_PTHREAD_D
	pthread_mutex_t mutex;      /* used "with" sync_after */
#endif
	int sincesync;	      /* req. done so far since last free IP sync. */

	/* from config */
	char *sql_module_instance;	 /* rlm_sql instance to use */
	char *db_name;	      /* netvim database */
	int no_free_fail;	     /* fail if no free IP addresses found */
	int free_after;	      /* how many seconds an IP should not be used after
				       freeing */
	int sync_after;	      /* how often to sync with radacct */
} rlm_sqlhpwippool_t;

/* char *name, int type,
 * size_t offset, void *data, char *dflt */
static CONF_PARSER module_config[] = {
	{ "sql_module_instance",       PW_TYPE_STRING_PTR,
	  offsetof(rlm_sqlhpwippool_t, sql_module_instance),       NULL, "sql" },
	{ "db_name",	    PW_TYPE_STRING_PTR,
	  offsetof(rlm_sqlhpwippool_t, db_name),	    NULL, "netvim" },
	{ "no_free_fail",	 PW_TYPE_BOOLEAN,
	  offsetof(rlm_sqlhpwippool_t, no_free_fail),	 NULL, "yes" },
	{ "free_after",	  PW_TYPE_INTEGER,
	  offsetof(rlm_sqlhpwippool_t, free_after),	  NULL, "300" },
	{ "sync_after",	  PW_TYPE_INTEGER,
	  offsetof(rlm_sqlhpwippool_t, sync_after),	  NULL, "25" },
	{ NULL, -1, 0, NULL, NULL } /* end */
};

/* wrapper around radlog which adds prefix with module and instance name */
static int nvp_log(unsigned int line, rlm_sqlhpwippool_t *data, int lvl,
		   char const *fmt, ...)
{
	va_list ap;
	int r;
	char pfmt[4096];

	/* prefix log message with RLM_NETVIM_LOG_FMT */
	snprintf(pfmt, sizeof(pfmt), RLM_NETVIM_LOG_FMT,
		 data->myname, line, fmt);

	va_start(ap, fmt);
	r = vradlog(lvl, pfmt, ap);
	va_end(ap);

	return r;
}
/* handy SQL query tool */
DIAG_OFF(format-nonliteral)
static int nvp_vquery(unsigned int line, rlm_sqlhpwippool_t *data,
		      rlm_sql_handle_t *sqlsock, char const *fmt, va_list ap)
{
	char query[MAX_QUERY_LEN];
	vsnprintf(query, MAX_QUERY_LEN, fmt, ap);
	if (rlm_sql_query(&sqlsock, data->sqlinst, query)) {
		nvp_log(__LINE__, data, L_ERR, "nvp_vquery(): query from line %u: %s",
			line, (char const *)(data->db->sql_error)(sqlsock, data->sqlinst->config));
		return 0;
	}

	return 1;
}
DIAG_ON(format-nonliteral)

/* wrapper around nvp_vquery */
static int nvp_query(unsigned int line, rlm_sqlhpwippool_t *data,
		    rlm_sql_handle_t *sqlsock, char const *fmt, ...)
{
	int r;
	va_list ap;

	va_start(ap, fmt);
	r = nvp_vquery(line, data, sqlsock, fmt, ap);
	va_end(ap);

	return r;
}

/* handy wrapper around data->db->sql_finish_query() */
static int nvp_finish(rlm_sqlhpwippool_t *data, rlm_sql_handle_t *sqlsock)
{
	return (data->db->sql_finish_query)(sqlsock, data->sqlinst->config);
}

/* executes query and fetches first row
 * -1 on no results
 *  0 on db error
 *  1 on success */
static int nvp_select(unsigned int line, rlm_sqlhpwippool_t *data,
		      rlm_sql_handle_t *sqlsock, char const *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (!nvp_vquery(line, data, sqlsock, fmt, ap)) {
		va_end(ap);
		return 0;
	}
	va_end(ap);

	if ((data->db->sql_store_result)(sqlsock, data->sqlinst->config)) {
		nvp_log(__LINE__, data, L_ERR,
			"nvp_select(): error while saving results of query from line %u",
			line);
		return 0;
	}

	if ((data->db->sql_num_rows)(sqlsock, data->sqlinst->config) < 1) {
		nvp_log(__LINE__, data, L_DBG,
			"nvp_select(): no results in query from line %u", line);
		return -1;
	}

	if ((data->db->sql_fetch_row)(sqlsock, data->sqlinst->config)) {
		nvp_log(__LINE__, data, L_ERR, "nvp_select(): couldn't fetch row "
					       "from results of query from line %u",
			line);
		return 0;
	}

	return 1;
}

static int nvp_select_finish(rlm_sqlhpwippool_t *data, rlm_sql_handle_t *sqlsock)
{
	return ((data->db->sql_free_result)(sqlsock, data->sqlinst->config) ||
		nvp_finish(data, sqlsock));
}

/* frees IPs of closed sessions (eg. by external modifications to db) */
static int nvp_freeclosed(rlm_sqlhpwippool_t *data, rlm_sql_handle_t *sqlsock)
{
	if (!nvp_query(__LINE__, data, sqlsock,
	    "UPDATE `%s`.`ips`, `radacct` "
	    	"SET "
	    		"`ips`.`rsv_until` = `radacct`.`acctstoptime` + INTERVAL %u SECOND "
	    	"WHERE "
	    		"`radacct`.`acctstoptime` IS NOT NULL AND "   /* session is closed */
	    		"("				    /* address is being used */
	    			"`ips`.`pid` IS NOT NULL AND "
	    			"(`rsv_until` = 0 OR `rsv_until` > NOW())"
	    		") AND "
	    		"`radacct`.`acctuniqueid` = `ips`.`rsv_by`",
	    data->db_name, data->free_after)) {
		return 0;
	}

	nvp_finish(data, sqlsock);
	return 1;
}

/* updates number of free IP addresses in pools */
static int nvp_syncfree(rlm_sqlhpwippool_t *data, rlm_sql_handle_t *sqlsock)
{
	if (!nvp_query(__LINE__, data, sqlsock,
	    "UPDATE `%s`.`ip_pools` "
	    	"SET `ip_pools`.`free` = "
	    		"(SELECT COUNT(*) "
	    			"FROM `%1$s`.`ips` "
	    			"WHERE "
	    				"`ips`.`ip` BETWEEN "
	    					"`ip_pools`.`ip_start` AND `ip_pools`.`ip_stop` AND "
	    				"("
	    					"`ips`.`pid` IS NULL OR "
	    					"(`ips`.`rsv_until` > 0 AND `ips`.`rsv_until` < NOW())"
	    				"))",
	    data->db_name)) {
		return 0;
	}

	nvp_finish(data, sqlsock);
	return 1;
}

/* cleanup IP pools and sync them with radacct */
static int nvp_cleanup(rlm_sqlhpwippool_t *data)
{
	rlm_sql_handle_t *sqlsock;

	/* initialize the SQL socket */
	sqlsock = sql_get_socket(data->sqlinst);
	if (!sqlsock) {
		nvp_log(__LINE__, data, L_ERR, "nvp_cleanup(): error while "
					       "requesting new SQL connection");
		return 0;
	}

	/* free IPs of closed sessions */
	if (!nvp_freeclosed(data, sqlsock)) {
		sql_release_socket(data->sqlinst, sqlsock);
		return 0;
	}

	/* add sessions opened in the meantime */
	if (!nvp_query(__LINE__, data, sqlsock,
	    "UPDATE `%s`.`ips`, `radacct` "
	    	"SET "
	    		"`ips`.`pid` = 0, "
	    		"`ips`.`rsv_by` = `radacct`.`acctuniqueid`, "
	    		"`ips`.`rsv_since` = `radacct`.`acctstarttime`, "
	    		"`ips`.`rsv_until` = 0 "
	    	"WHERE "
	    		"`radacct`.`acctstoptime` IS NULL AND "     /* session is opened */
	    		"`ips`.`ip` = INET_ATON(`radacct`.`framedipaddress`) AND "
	    		"("
	    			"`ips`.`pid` IS NULL OR "
/*	    			"(`ips`.`rsv_until` > 0 AND `ips.`rsv_until` < NOW()) " */
	    			"`ips`.`rsv_until` != 0"   /* no acct pkt received yet */
	    		")",
	    data->db_name)) {
		sql_release_socket(data->sqlinst, sqlsock);
		return 0;
	}
	else {
		nvp_finish(data, sqlsock);
	}

	/* count number of free IP addresses in IP pools */
	if (!nvp_syncfree(data, sqlsock)) {
		sql_release_socket(data->sqlinst, sqlsock);
		return 0;
	}

	sql_release_socket(data->sqlinst, sqlsock);
	return 1;
}

/* standard foobar code */
static int mod_instantiate(CONF_SECTION *conf, void *instance)
{
	rlm_sqlhpwippool_t *inst = instance;
	module_instance_t *sqlinst;

	/* save my name */
	inst->myname = cf_section_name2(conf);
	if (!inst->myname) {
		inst->myname = "(no name)";
	}

	inst->sincesync = 0;

	sqlinst = find_module_instance(cf_section_find("modules"), (inst->sql_module_instance), 1 );
	if (!sqlinst) {
		nvp_log(__LINE__, inst, L_ERR,
			"mod_instantiate(): cannot find module instance "
			"named \"%s\"",
			inst->sql_module_instance);
		return -1;
	}

	/* check if the given instance is really a rlm_sql instance */
	if (strcmp(sqlinst->entry->name, "rlm_sql") != 0) {
		nvp_log(__LINE__, inst, L_ERR,
			"mod_instantiate(): given instance (%s) is not "
			"an instance of the rlm_sql module",
			inst->sql_module_instance);
		return -1;
	}

	/* save pointers to useful "objects" */
	inst->sqlinst = (rlm_sql_t *) sqlinst->insthandle;
	inst->db = (rlm_sql_module_t *) inst->sqlinst->module;

	return ((nvp_cleanup(inst)) ? 0 : -1);
}

/* assign new IP address, if required */
static rlm_rcode_t mod_post_auth(void *instance, REQUEST *request)
{
	VALUE_PAIR *vp;
	char const *pname;       /* name of requested IP pool */
	uint32_t nasip;	     /* NAS IP in host byte order */
	struct in_addr ip = {0};    /* reserved IP for client (net. byte order) */
	rlm_sql_handle_t *sqlsock;
	unsigned long s_gid,	/* _s_elected in sql result set */
		      s_prio,       /* as above */
		      s_pid,	/* as above */
		      gid,	  /* real integer value */
		      pid,	  /* as above */
		      weights_sum, used_sum, ip_start, ip_stop, connid;
	long prio;

	rlm_sqlhpwippool_t *inst = (rlm_sqlhpwippool_t *) instance;

	/* if IP is already there, then nothing to do */
	vp = pairfind(request->reply->vps, PW_FRAMED_IP_ADDRESS, 0, TAG_ANY);
	if (vp) {
		nvp_log(__LINE__, inst, L_DBG,
			"mod_post_auth(): IP address "
			"already in the reply packet - exiting");
		return RLM_MODULE_NOOP;
	}

	/* if no pool name, we don't need to do anything */
	vp = pairfind(request->reply->vps, ASN_IP_POOL_NAME, VENDORPEC_ASN, TAG_ANY);
	if (vp) {
		pname = vp->vp_strvalue;
		nvp_log(__LINE__, inst, L_DBG,
			"mod_post_auth(): pool name = '%s'",
			pname);
	}
	else {
		nvp_log(__LINE__, inst, L_DBG,
			"mod_post_auth(): no IP pool name - exiting");
		return RLM_MODULE_NOOP;
	}

	/* if no NAS IP address, assign 0 */
	vp = pairfind(request->packet->vps, PW_NAS_IP_ADDRESS, 0, TAG_ANY);
	if (vp) {
		nasip = ntohl(vp->vp_ipaddr);
	}
	else {
		nasip = 0;
		nvp_log(__LINE__, inst, L_DBG,
			"mod_post_auth(): no NAS IP address in "
			"the request packet - using \"0.0.0.0/0\" (any)");
	}

	/* get our database connection */
	sqlsock = sql_get_socket(inst->sqlinst);
	if (!sqlsock) {
		nvp_log(__LINE__, inst, L_ERR,
			"mod_post_auth(): error while requesting an SQL socket");
		return RLM_MODULE_FAIL;
	}

	/* get connection id as temporary unique integer */
	if (nvp_select(__LINE__, inst, sqlsock, "SELECT CONNECTION_ID()") < 1) {
		nvp_log(__LINE__, inst, L_ERR, "mod_post_auth(): WTF ;-)!");
		nvp_select_finish(inst, sqlsock);
		sql_release_socket(inst->sqlinst, sqlsock);
		return RLM_MODULE_FAIL;
	}

	connid = strtoul(sqlsock->row[0], (char **) NULL, 10);
	nvp_select_finish(inst, sqlsock);

	/* synchronize with radacct db, if needed */
	if (++inst->sincesync >= inst->sync_after
#ifdef HAVE_PTHREAD_D
	    && (pthread_mutex_trylock(&inst->mutex)) == 0
#endif
	   ) {
		int r;

		inst->sincesync = 0;

		nvp_log(__LINE__, inst, L_DBG,
			"mod_post_auth(): syncing with radacct table");

		r = (nvp_freeclosed(inst, sqlsock) && nvp_syncfree(inst, sqlsock));

#ifdef HAVE_PTHREAD_D
		pthread_mutex_unlock(&inst->mutex);
#endif

		if (!r) {
			nvp_log(__LINE__, inst, L_ERR,
				"mod_post_auth(): synchronization failed");
			sql_release_socket(inst->sqlinst, sqlsock);
			return RLM_MODULE_FAIL;
		}
	}

	for (s_gid = 0; s_gid < RLM_NETVIM_MAX_ROWS && !(ip.s_addr); s_gid++) {
		nvp_log(__LINE__, inst, L_DBG,
			"mod_post_auth(): selecting gid on position %lu",
			s_gid);

		/* find the most specific group which NAS belongs to */
		switch (nvp_select(__LINE__, inst, sqlsock,
		       "SELECT `host_groups`.`gid` "
		       	"FROM "
		       		"`%s`.`host_groups`, "
		       		"`%1$s`.`gid_ip`, "
		       		"`%1$s`.`ids` "
		       	"WHERE "
		       		"`host_groups`.`gid` = `ids`.`id` AND "
		       		"`ids`.`enabled` = 1 AND "
		       		"`host_groups`.`gid` = `gid_ip`.`gid` AND "
		       		"%lu BETWEEN `gid_ip`.`ip_start` AND `gid_ip`.`ip_stop` "
		       	"ORDER BY (`gid_ip`.`ip_stop` - `gid_ip`.`ip_start`) ASC "
		       	"LIMIT %lu, 1",
		       inst->db_name, nasip, s_gid)) {
			case -1:
				nvp_log(__LINE__, inst, L_ERR,
					"mod_post_auth(): couldn't find "
					"any more matching host groups");
				goto end_gid;		  /* exit the main loop */
			case 0:
				sql_release_socket(inst->sqlinst, sqlsock);
				return RLM_MODULE_FAIL;
		}

		/* store the group ID and free memory occupied by results */
		gid = strtoul(sqlsock->row[0], (char **) NULL, 10);
		nvp_select_finish(inst, sqlsock);

		for (s_prio = 0; s_prio < RLM_NETVIM_MAX_ROWS && !(ip.s_addr); s_prio++) {
			nvp_log(__LINE__, inst, L_DBG,
				"mod_post_auth(): selecting prio on position %lu",
				s_prio);

			/* prepare to search for best fit pool */
			switch (nvp_select(__LINE__, inst, sqlsock,
				"SELECT "
					"`ip_pools`.`prio`, "
					"SUM(`ip_pools`.`weight`) AS `weights_sum`, "
					"(SUM(`ip_pools`.`total`) - "
						"SUM(`ip_pools`.`free`)) AS `used_sum` "
					"FROM "
						"`%s`.`ip_pools`, "
						"`%1$s`.`ids`, "
					 	"`%1$s`.`pool_names` "
					"WHERE "
						"`ip_pools`.`gid` = %lu AND "
						"`ids`.`id` = `ip_pools`.`pid` AND "
						"`ids`.`enabled` = 1 AND "
						"`pool_names`.`pnid` = `ip_pools`.`pnid` AND "
						"`pool_names`.`name` = '%s' AND "
						"`ip_pools`.`free` > 0 "
					"GROUP BY `prio` "
					"ORDER BY `prio` ASC "
					"LIMIT %lu, 1",
				inst->db_name, gid, pname, s_prio)) {
				case -1:
					nvp_log(__LINE__, inst, L_DBG,
						"mod_post_auth(): couldn't find "
						"any more matching pools for gid = %u",
						gid);
					goto end_prio;	       /* select next gid */
				case 0:
					sql_release_socket(inst->sqlinst, sqlsock);
					return RLM_MODULE_FAIL;
			}

			/* store the prio and weights sum */
			prio = strtol(sqlsock->row[0], (char **) NULL, 10);
			weights_sum = strtoul(sqlsock->row[1], (char **) NULL, 10);
			used_sum = strtoul(sqlsock->row[2], (char **) NULL, 10);

			/* free memory */
			nvp_select_finish(inst, sqlsock);

			for (s_pid = 0; s_pid < RLM_NETVIM_MAX_ROWS && !(ip.s_addr); s_pid++) {
				nvp_log(__LINE__, inst, L_DBG,
					"mod_post_auth(): selecting PID on position %lu",
					s_pid);

				/* search for best fit pool */
				switch (nvp_select(__LINE__, inst, sqlsock,
					"SELECT "
						"`ip_pools`.`pid`, "
						"`ip_pools`.`ip_start`, "
						"`ip_pools`.`ip_stop` "
						"FROM "
							"`%s`.`ip_pools`, "
							"`%1$s`.`ids`, "
							"`%1$s`.`pool_names` "
						"WHERE "
							"`ip_pools`.`gid` = %lu AND "
							"`ids`.`id` = `ip_pools`.`pid` AND "
							"`ids`.`enabled` = 1 AND "
							"`pool_names`.`pnid` = `ip_pools`.`pnid` AND "
							"`pool_names`.`name` = '%s' AND "
							"`ip_pools`.`free` > 0 AND "
							"`prio` = %ld "
						"ORDER BY (`weight`/%lu.0000 - (`total` - `free`)/%lu) DESC "
						"LIMIT %lu, 1",
					inst->db_name, gid, pname, prio,
					weights_sum, used_sum, s_pid)) {
					case -1:
						nvp_log(__LINE__, inst, L_DBG,
							"mod_post_auth(): couldn't find any more "
							"matching pools of prio = %ld for gid = %lu",
							prio, gid);
						goto end_pid;	      /* select next prio */
					case 0:
						sql_release_socket(inst->sqlinst, sqlsock);
						return RLM_MODULE_FAIL;
				}

				/* store the inst and free memory occupied by results */
				pid = strtoul(sqlsock->row[0], (char **) NULL, 10);
				ip_start = strtoul(sqlsock->row[1], (char **) NULL, 10);
				ip_stop = strtoul(sqlsock->row[2], (char **) NULL, 10);
				nvp_select_finish(inst, sqlsock);

				/* reserve an IP address */
				if (!nvp_query(__LINE__, inst, sqlsock,
				    "UPDATE `%s`.`ips` "
				    	"SET "
				    		"`pid` = %lu, "
				    		"`rsv_since` = NOW(), "
				    		"`rsv_by` = '" RLM_NETVIM_TMP_PREFIX "%lu', "
				    		"`rsv_until` = NOW() + INTERVAL %d SECOND "
				    	"WHERE "
				    		"`ip` BETWEEN %lu AND %lu AND "
				    		"("
				    			"`pid` IS NULL OR "
				    			"(`rsv_until` > 0 AND `rsv_until` < NOW())"
				    		") "
				    	"ORDER BY RAND() "
				    	"LIMIT 1",
				    inst->db_name, pid, connid, inst->free_after, ip_start, ip_stop)) {
					sql_release_socket(inst->sqlinst, sqlsock);
					return RLM_MODULE_FAIL;
				}
				else {
					nvp_finish(inst, sqlsock);
				}

				/* select assigned IP address */
				switch (nvp_select(__LINE__, inst, sqlsock,
					"SELECT `ip` "
						"FROM `%s`.`ips` "
						"WHERE `rsv_by` = '" RLM_NETVIM_TMP_PREFIX "%lu' "
						"ORDER BY `rsv_since` DESC "
						"LIMIT 1",
					inst->db_name, connid)) {
					case -1:
						nvp_log(__LINE__, inst, L_ERR,
							"mod_post_auth(): couldn't reserve an IP address "
							"from pool of pid = %lu (prio = %ld, gid = %lu)",
							pid, prio, gid);
						continue;			    /* select next pid */
					case 0:
						sql_release_socket(inst->sqlinst, sqlsock);
						return RLM_MODULE_FAIL;
				}

				/* update free IPs count */
				if (!nvp_query(__LINE__, inst, sqlsock,
				    "UPDATE `%s`.`ip_pools` "
				    	"SET "
				    		"`free` = `free` - 1 "
				    	"WHERE "
				    		"`pid` = %lu "
			    	"LIMIT 1",
				    inst->db_name, pid)) {
					sql_release_socket(inst->sqlinst, sqlsock);
					return RLM_MODULE_FAIL;
				}
				else {
					nvp_finish(inst, sqlsock);
				}

				/* get assigned IP and free memory */
				ip.s_addr = htonl(strtoul(sqlsock->row[0], (char **) NULL, 10));
				nvp_select_finish(inst, sqlsock);
			} /* pid */
end_pid: continue;	   /* stupid */
		} /* prio */
end_prio: continue;	  /* stupid */
	} /* gid */
end_gid:

	/* release SQL socket */
	sql_release_socket(inst->sqlinst, sqlsock);

	/* no free IP address found */
	if (!ip.s_addr) {
		nvp_log(__LINE__, inst, L_INFO,
			"mod_post_auth(): no free IP address found!");

		if (inst->no_free_fail) {
			nvp_log(__LINE__, inst, L_DBG, "mod_post_auth(): rejecting user");
			return RLM_MODULE_REJECT;
		}
		else {
			nvp_log(__LINE__, inst, L_DBG, "mod_post_auth(): exiting");
			return RLM_MODULE_NOOP;
		}
	}

	/* add IP address to reply packet */
	vp = radius_paircreate(request, &request->reply->vps,
			       PW_FRAMED_IP_ADDRESS, 0);
	vp->vp_ipaddr = ip.s_addr;

	nvp_log(__LINE__, inst, L_DBG, "mod_post_auth(): returning %s",
		inet_ntoa(ip));
	return RLM_MODULE_OK;
}

static rlm_rcode_t mod_accounting(void *instance, REQUEST *request)
{
	VALUE_PAIR *vp;
	rlm_sql_handle_t *sqlsock;
	struct in_addr nasip;      /* NAS IP */
	char const *sessid;     /* unique session id */
	char nasipstr[16];	 /* NAS IP in string format */
	uint32_t framedip = 0;     /* client's IP, host byte order */
	uint32_t acct_type;

	rlm_sqlhpwippool_t *inst = (rlm_sqlhpwippool_t *) instance;

	/* if no unique session ID, don't even try */
	vp = pairfind(request->packet->vps, PW_ACCT_UNIQUE_SESSION_ID, 0, TAG_ANY);
	if (vp) {
		sessid = vp->vp_strvalue;
	}
	else {
		nvp_log(__LINE__, inst, L_ERR,
			"mod_accounting(): unique session ID not found");
		return RLM_MODULE_FAIL;
	}

	vp = pairfind(request->packet->vps, PW_ACCT_STATUS_TYPE, 0, TAG_ANY);
	if (vp) {
		acct_type = vp->vp_integer;
	}
	else {
		nvp_log(__LINE__, inst, L_ERR, "mod_accounting(): "
					       "couldn't find type of accounting packet");
		return RLM_MODULE_FAIL;
	}

	if (!(acct_type == PW_STATUS_START ||
	      acct_type == PW_STATUS_ALIVE ||
	      acct_type == PW_STATUS_STOP  ||
	      acct_type == PW_STATUS_ACCOUNTING_OFF ||
	      acct_type == PW_STATUS_ACCOUNTING_ON)) {
		return RLM_MODULE_NOOP;
	}

	/* connect to database */
	sqlsock = sql_get_socket(inst->sqlinst);
	if (!sqlsock) {
		nvp_log(__LINE__, inst, L_ERR,
			"mod_accounting(): couldn't connect to database");
		return RLM_MODULE_FAIL;
	}


	switch (acct_type) {
		case PW_STATUS_START:
		case PW_STATUS_ALIVE:
			vp = pairfind(request->packet->vps, PW_FRAMED_IP_ADDRESS, 0, TAG_ANY);
			if (!vp) {
				nvp_log(__LINE__, inst, L_ERR, "mod_accounting(): no framed IP");
				sql_release_socket(inst->sqlinst, sqlsock);
				return RLM_MODULE_FAIL;
			}

			framedip = ntohl(vp->vp_ipaddr);

			if (!nvp_query(__LINE__, inst, sqlsock,
			    "UPDATE `%s`.`ips` "
			    	"SET "
			    		"`rsv_until` = 0, "
			    		"`rsv_by` = '%s' "
			    	"WHERE `ip` = %lu",
			    inst->db_name, sessid, framedip)) {
				sql_release_socket(inst->sqlinst, sqlsock);
				return RLM_MODULE_FAIL;
			}
			nvp_finish(inst, sqlsock);
			break;

		case PW_STATUS_STOP:
			if (!nvp_query(__LINE__, inst, sqlsock,
			    "UPDATE `%s`.`ips`, `%1$s`.`ip_pools` "
			    	"SET "
			    		"`ips`.`rsv_until` = NOW() + INTERVAL %u SECOND, "
			    		"`ip_pools`.`free` = `ip_pools`.`free` + 1 "
			    	"WHERE "
			    		"`ips`.`rsv_by` = '%s' AND "
			    		"`ips`.`ip` BETWEEN `ip_pools`.`ip_start` AND `ip_pools`.`ip_stop`",
			    inst->db_name, inst->free_after, sessid)) {
				sql_release_socket(inst->sqlinst, sqlsock);
				return RLM_MODULE_FAIL;
			}
			nvp_finish(inst, sqlsock);
		break;

		case PW_STATUS_ACCOUNTING_OFF:
		case PW_STATUS_ACCOUNTING_ON:
			vp = pairfind(request->packet->vps, PW_NAS_IP_ADDRESS, 0, TAG_ANY);
			if (!vp) {
				nvp_log(__LINE__, inst, L_ERR, "mod_accounting(): no NAS IP");
				sql_release_socket(inst->sqlinst, sqlsock);
				return RLM_MODULE_FAIL;
			}

			nasip.s_addr = vp->vp_ipaddr;
			strlcpy(nasipstr, inet_ntoa(nasip), sizeof(nasipstr));

			if (!nvp_query(__LINE__, inst, sqlsock,
			    "UPDATE `%s`.`ips`, `radacct` "
			    	"SET `ips`.`rsv_until` = NOW() + INTERVAL %u SECOND "
			    	"WHERE "
			    		"`radacct`.`nasipaddress` = '%s' AND "
			    		"`ips`.`rsv_by` = `radacct`.`acctuniqueid`",
			    inst->db_name, inst->free_after, nasipstr)) {
				sql_release_socket(inst->sqlinst, sqlsock);
				return RLM_MODULE_FAIL;
			}
			nvp_finish(inst, sqlsock);

			break;
	}

	sql_release_socket(inst->sqlinst, sqlsock);
	return RLM_MODULE_OK;
}

module_t rlm_sqlhpwippool = {
	RLM_MODULE_INIT,
	"sqlhpwippool",			/* name */
	RLM_TYPE_THREAD_SAFE,		/* type */
	sizeof(rlm_sqlhpwippool_t),
	module_config,
	mod_instantiate,	/* instantiation */
	NULL,				/* detach */
	{
		NULL,			/* authentication */
		NULL,			/* authorization */
		NULL,			/* preaccounting */
		mod_accounting,/* accounting */
		NULL,			/* checksimul */
		NULL,			/* pre-proxy */
		NULL,			/* post-proxy */
		mod_post_auth	/* post-auth */
	},
};
