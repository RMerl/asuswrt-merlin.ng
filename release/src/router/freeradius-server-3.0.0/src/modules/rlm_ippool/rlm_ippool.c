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
 * @file rlm_ippool.c
 * @brief Allocates an IPv4 address from a pool stored in a GDBM database.
 *
 * @copyright 2000,2006  The FreeRADIUS server project
 * @copyright 2002  Kostas Kalevras <kkalev@noc.ntua.gr>
 */
RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>
#include <freeradius-devel/rad_assert.h>

#include "config.h"
#include <ctype.h>

#ifdef WITH_DHCP
#include <freeradius-devel/dhcp.h>
#endif

#include "../../include/md5.h"

#include <gdbm.h>

#ifdef NEEDS_GDBM_SYNC
#	define GDBM_SYNCOPT GDBM_SYNC
#else
#	define GDBM_SYNCOPT 0
#endif

#ifdef GDBM_NOLOCK
#define GDBM_IPPOOL_OPTS (GDBM_SYNCOPT | GDBM_NOLOCK)
#else
#define GDBM_IPPOOL_OPTS (GDBM_SYNCOPT)
#endif

#define MAX_NAS_NAME_SIZE 64

/*
 *	Define a structure for our module configuration.
 *
 *	These variables do not need to be in a structure, but it's
 *	a lot cleaner to do so, and a pointer to the structure can
 *	be used as the instance handle.
 */
typedef struct rlm_ippool_t {
	char *filename;
	char *ip_index;
	char *name;
	char *key;
	uint32_t range_start;
	uint32_t range_stop;
	uint32_t netmask;
	time_t max_timeout;
	int cache_size;
	int override;
	GDBM_FILE gdbm;
	GDBM_FILE ip;
#ifdef HAVE_PTHREAD_H
	pthread_mutex_t op_mutex;
#endif
} rlm_ippool_t;

#ifndef HAVE_PTHREAD_H
/*
 *	This is easier than ifdef's throughout the code.
 */
#define pthread_mutex_init(_x, _y)
#define pthread_mutex_destroy(_x)
#define pthread_mutex_lock(_x)
#define pthread_mutex_unlock(_x)
#endif

typedef struct ippool_info {
	uint32_t	ipaddr;
	char		active;
	char		cli[32];
	char		extra;
	time_t		timestamp;
	time_t		timeout;
} ippool_info;

typedef struct ippool_key {
	char key[16];
} ippool_key;

static const CONF_PARSER module_config[] = {
	{ "session-db", PW_TYPE_FILE_OUTPUT | PW_TYPE_DEPRECATED, offsetof(rlm_ippool_t,filename), NULL, NULL },
	{ "filename", PW_TYPE_FILE_OUTPUT | PW_TYPE_REQUIRED, offsetof(rlm_ippool_t,filename), NULL, NULL },

	{ "ip-index", PW_TYPE_STRING_PTR | PW_TYPE_DEPRECATED, offsetof(rlm_ippool_t,ip_index), NULL, NULL },
	{ "ip_index", PW_TYPE_STRING_PTR | PW_TYPE_REQUIRED, offsetof(rlm_ippool_t,ip_index), NULL, NULL },

	{ "key", PW_TYPE_STRING_PTR | PW_TYPE_REQUIRED,
	  offsetof(rlm_ippool_t,key), NULL, "%{NAS-IP-Address} %{NAS-Port}" },

	{ "range-start", PW_TYPE_IPADDR | PW_TYPE_DEPRECATED, offsetof(rlm_ippool_t,range_start), NULL, NULL },
	{ "range_start", PW_TYPE_IPADDR, offsetof(rlm_ippool_t,range_start), NULL, "0" },

	{ "range-stop", PW_TYPE_IPADDR | PW_TYPE_DEPRECATED, offsetof(rlm_ippool_t,range_stop), NULL, NULL },
	{ "range_stop", PW_TYPE_IPADDR, offsetof(rlm_ippool_t,range_stop), NULL, "0" },

	{ "netmask", PW_TYPE_IPADDR, offsetof(rlm_ippool_t,netmask), NULL, "0" },

	{ "cache-size", PW_TYPE_INTEGER | PW_TYPE_DEPRECATED, offsetof(rlm_ippool_t,cache_size), NULL, NULL },
	{ "cache_size", PW_TYPE_INTEGER, offsetof(rlm_ippool_t,cache_size), NULL, "1000" },

	{ "override", PW_TYPE_BOOLEAN, offsetof(rlm_ippool_t,override), NULL, "no" },

	{ "maximum-timeout", PW_TYPE_INTEGER | PW_TYPE_DEPRECATED, offsetof(rlm_ippool_t,max_timeout), NULL, NULL },
	{ "maximum_timeout", PW_TYPE_INTEGER, offsetof(rlm_ippool_t,max_timeout), NULL, "0" },

	{ NULL, -1, 0, NULL, NULL }
};

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
	rlm_ippool_t *inst = instance;
	int cache_size;
	ippool_info entry;
	ippool_key key;
	datum key_datum;
	datum data_datum;
	char const *cli = "0";
	char const *pool_name = NULL;

	cache_size = inst->cache_size;

	rad_assert(inst->filename && *inst->filename);
	rad_assert(inst->ip_index && *inst->ip_index);

	inst->range_start = htonl(inst->range_start);
	inst->range_stop = htonl(inst->range_stop);
	inst->netmask = htonl(inst->netmask);
	if (inst->range_start == 0 || inst->range_stop == 0 || \
	    inst->range_start >= inst->range_stop ) {
		cf_log_err_cs(conf, "Invalid data range");
		return -1;
	}

	inst->gdbm = gdbm_open(inst->filename, sizeof(int),
			GDBM_WRCREAT | GDBM_IPPOOL_OPTS, 0600, NULL);
	if (!inst->gdbm) {
		ERROR("rlm_ippool: Failed to open file %s: %s",
				inst->filename, strerror(errno));
		return -1;
	}
	inst->ip = gdbm_open(inst->ip_index, sizeof(int),
			GDBM_WRCREAT | GDBM_IPPOOL_OPTS, 0600, NULL);
	if (!inst->ip) {
		ERROR("rlm_ippool: Failed to open file %s: %s",
				inst->ip_index, strerror(errno));
		return -1;
	}
	if (gdbm_setopt(inst->gdbm, GDBM_CACHESIZE, &cache_size, sizeof(int)) == -1)
		ERROR("rlm_ippool: Failed to set cache size");
	if (gdbm_setopt(inst->ip, GDBM_CACHESIZE, &cache_size, sizeof(int)) == -1)
		ERROR("rlm_ippool: Failed to set cache size");

	key_datum = gdbm_firstkey(inst->gdbm);
	if (!key_datum.dptr){
			/*
			 * If the database does not exist initialize it.
			 * We set the nas/port pairs to not existent values and
			 * active = 0
			 */
		int rcode;
		uint32_t i, j;
		uint32_t or_result;
		char str[32];
		char init_str[17];

		DEBUG("rlm_ippool: Initializing database");
		for(i=inst->range_start,j=~0;i<=inst->range_stop;i++,j--){

			/*
			 * Net and Broadcast addresses are excluded
			 */
			or_result = i | inst->netmask;
			if (~inst->netmask != 0 &&
				(or_result == inst->netmask ||
			    (~or_result == 0))) {
				DEBUG("rlm_ippool: IP %s excluded",
				      ip_ntoa(str, ntohl(i)));
				continue;
			}

			sprintf(init_str,"%016d",j);
			DEBUG("rlm_ippool: Initialized bucket: %s",init_str);
			memcpy(key.key, init_str,16);
			key_datum.dptr = (char *) &key;
			key_datum.dsize = sizeof(ippool_key);

			entry.ipaddr = ntohl(i);
			entry.active = 0;
			entry.extra = 0;
			entry.timestamp = 0;
			entry.timeout = 0;
			strcpy(entry.cli,cli);

			data_datum.dptr = (char *) &entry;
			data_datum.dsize = sizeof(ippool_info);

			rcode = gdbm_store(inst->gdbm, key_datum, data_datum, GDBM_REPLACE);
			if (rcode < 0) {
				ERROR("rlm_ippool: Failed storing data to %s: %s",
						inst->filename, gdbm_strerror(gdbm_errno));
				gdbm_close(inst->gdbm);
				gdbm_close(inst->ip);
				return -1;
			}
		}
	}
	else
		free(key_datum.dptr);

	/* Add the ip pool name */
	inst->name = NULL;
	pool_name = cf_section_name2(conf);
	if (pool_name != NULL)
		inst->name = strdup(pool_name);

	pthread_mutex_init(&inst->op_mutex, NULL);

	return 0;
}


/*
 *	Check for an Accounting-Stop
 *	If we find one and we have allocated an IP to this nas/port combination, deallocate it.
 */
static rlm_rcode_t mod_accounting(void *instance, REQUEST *request)
{
	rlm_ippool_t *inst = instance;
	datum key_datum;
	datum data_datum;
	datum save_datum;
	int acctstatustype = 0;
	int rcode;
	ippool_info entry;
	ippool_key key;
	int num = 0;
	VALUE_PAIR *vp;
	char str[32];
	uint8_t key_str[17];
	char hex_str[35];
	char xlat_str[MAX_STRING_LEN];
	FR_MD5_CTX md5_context;


	if ((vp = pairfind(request->packet->vps, PW_ACCT_STATUS_TYPE, 0, TAG_ANY)) != NULL)
		acctstatustype = vp->vp_integer;
	else {
		RDEBUG("Could not find account status type in packet. Return NOOP.");
		return RLM_MODULE_NOOP;
	}
	switch(acctstatustype){
		case PW_STATUS_STOP:
			if (radius_xlat(xlat_str, sizeof(xlat_str), request, inst->key, NULL, NULL) < 0){
				return RLM_MODULE_NOOP;
			}
			fr_MD5Init(&md5_context);
			fr_MD5Update(&md5_context, (uint8_t *)xlat_str,
			 strlen(xlat_str));
			fr_MD5Final(key_str, &md5_context);
			key_str[16] = '\0';
			fr_bin2hex(hex_str, key_str, 16);
			hex_str[32] = '\0';
			RDEBUG("MD5 on 'key' directive maps to: %s",hex_str);
			memcpy(key.key,key_str,16);
			break;
		default:
			/* We don't care about any other accounting packet */
			RDEBUG("This is not an Accounting-Stop. Return NOOP.");

			return RLM_MODULE_NOOP;
	}

	RDEBUG("Searching for an entry for key: '%s'",xlat_str);
	key_datum.dptr = (char *) &key;
	key_datum.dsize = sizeof(ippool_key);

	pthread_mutex_lock(&inst->op_mutex);
	data_datum = gdbm_fetch(inst->gdbm, key_datum);
	if (data_datum.dptr != NULL){

		/*
		 * If the entry was found set active to zero
		 */
		memcpy(&entry, data_datum.dptr, sizeof(ippool_info));
		free(data_datum.dptr);
		RDEBUG("Deallocated entry for ip: %s",ip_ntoa(str,entry.ipaddr));
		entry.active = 0;
		entry.timestamp = 0;
		entry.timeout = 0;

		/*
		 * Save the reference to the entry
		 */
		save_datum.dptr = key_datum.dptr;
		save_datum.dsize = key_datum.dsize;

		data_datum.dptr = (char *) &entry;
		data_datum.dsize = sizeof(ippool_info);

		rcode = gdbm_store(inst->gdbm, key_datum, data_datum, GDBM_REPLACE);
		if (rcode < 0) {
			ERROR("rlm_ippool: Failed storing data to %s: %s",
					inst->filename, gdbm_strerror(gdbm_errno));
			pthread_mutex_unlock(&inst->op_mutex);
			return RLM_MODULE_FAIL;
		}

		/*
		 * Decrease allocated count from the ip index
		 */
		key_datum.dptr = (char *) &entry.ipaddr;
		key_datum.dsize = sizeof(uint32_t);
		data_datum = gdbm_fetch(inst->ip, key_datum);
		if (data_datum.dptr != NULL){
			memcpy(&num, data_datum.dptr, sizeof(int));
			free(data_datum.dptr);
			if (num >0){
				num--;
				RDEBUG("num: %d",num);
				data_datum.dptr = (char *) &num;
				data_datum.dsize = sizeof(int);
				rcode = gdbm_store(inst->ip, key_datum, data_datum, GDBM_REPLACE);
				if (rcode < 0) {
					ERROR("rlm_ippool: Failed storing data to %s: %s",
							inst->ip_index, gdbm_strerror(gdbm_errno));
					pthread_mutex_unlock(&inst->op_mutex);
					return RLM_MODULE_FAIL;
				}
				if (num >0 && entry.extra == 1){
					/*
					 * We are doing MPPP and we still have nas/port entries referencing
					 * this ip. Delete this entry so that eventually we only keep one
					 * reference to this ip.
					 */
					gdbm_delete(inst->gdbm,save_datum);
				}
			}
		}
		pthread_mutex_unlock(&inst->op_mutex);
	}
	else{
		pthread_mutex_unlock(&inst->op_mutex);
		RDEBUG("Entry not found");
	}

	return RLM_MODULE_OK;
}

static rlm_rcode_t mod_post_auth(UNUSED void *instance, UNUSED REQUEST *request)
{
	rlm_ippool_t *inst = instance;
	int delete = 0;
	int found = 0;
	int mppp = 0;
	int extra = 0;
	int rcode;
	int num = 0;
	datum key_datum;
	datum nextkey;
	datum data_datum;
	datum save_datum;
	ippool_key key;
	ippool_info entry;
	VALUE_PAIR *vp;
	char const *cli = NULL;
	char str[32];
	uint8_t key_str[17];
	char hex_str[35];
	char xlat_str[MAX_STRING_LEN];
	FR_MD5_CTX md5_context;
#ifdef WITH_DHCP
	int dhcp = false;
#endif
	int attr_ipaddr = PW_FRAMED_IP_ADDRESS;
	int attr_ipmask = PW_FRAMED_IP_NETMASK;
	int vendor_ipaddr = 0;

	/* Check if Pool-Name attribute exists. If it exists check our name and
	 * run only if they match
	 */
	if ((vp = pairfind(request->config_items, PW_POOL_NAME, 0, TAG_ANY)) != NULL){
		if (!inst->name || (strcmp(inst->name,vp->vp_strvalue) && strcmp(vp->vp_strvalue,"DEFAULT")))
			return RLM_MODULE_NOOP;
	} else {
		RDEBUG("Could not find Pool-Name attribute.");
		return RLM_MODULE_NOOP;
	}


	/*
	 * Find the caller id
	 */
	if ((vp = pairfind(request->packet->vps, PW_CALLING_STATION_ID, 0, TAG_ANY)) != NULL)
		cli = vp->vp_strvalue;

#ifdef WITH_DHCP
	if (request->listener->type == RAD_LISTEN_DHCP) {
		dhcp = 1;
		attr_ipaddr = PW_DHCP_YOUR_IP_ADDRESS;
		vendor_ipaddr = DHCP_MAGIC_VENDOR;
		attr_ipmask = PW_DHCP_SUBNET_MASK;
	}
#endif

	if (radius_xlat(xlat_str, sizeof(xlat_str), request, inst->key, NULL, NULL) < 0){
		return RLM_MODULE_NOOP;
	}

	fr_MD5Init(&md5_context);
	fr_MD5Update(&md5_context, (uint8_t *)xlat_str, strlen(xlat_str));
	fr_MD5Final(key_str, &md5_context);
	key_str[16] = '\0';
	fr_bin2hex(hex_str, key_str, 16);
	hex_str[32] = '\0';
	RDEBUG("MD5 on 'key' directive maps to: %s",hex_str);
	memcpy(key.key,key_str,16);

	RDEBUG("Searching for an entry for key: '%s'",hex_str);
	key_datum.dptr = (char *) &key;
	key_datum.dsize = sizeof(ippool_key);

	pthread_mutex_lock(&inst->op_mutex);
	data_datum = gdbm_fetch(inst->gdbm, key_datum);
	if (data_datum.dptr != NULL){
		/*
		 * If there is a corresponding entry in the database with active=1 it is stale.
		 * Set active to zero
		 */
		found = 1;
		memcpy(&entry, data_datum.dptr, sizeof(ippool_info));
		free(data_datum.dptr);
		if (entry.active){
			RDEBUG("Found a stale entry for ip: %s",ip_ntoa(str,entry.ipaddr));
			entry.active = 0;
			entry.timestamp = 0;
			entry.timeout = 0;

			/*
			 * Save the reference to the entry
			 */
			save_datum.dptr = key_datum.dptr;
			save_datum.dsize = key_datum.dsize;

			data_datum.dptr = (char *) &entry;
			data_datum.dsize = sizeof(ippool_info);

			rcode = gdbm_store(inst->gdbm, key_datum, data_datum, GDBM_REPLACE);
			if (rcode < 0) {
				ERROR("rlm_ippool: Failed storing data to %s: %s",
					inst->filename, gdbm_strerror(gdbm_errno));
				pthread_mutex_unlock(&inst->op_mutex);
				return RLM_MODULE_FAIL;
			}
			/* Decrease allocated count from the ip index */

			key_datum.dptr = (char *) &entry.ipaddr;
			key_datum.dsize = sizeof(uint32_t);
			data_datum = gdbm_fetch(inst->ip, key_datum);
			if (data_datum.dptr != NULL){
				memcpy(&num, data_datum.dptr, sizeof(int));
				free(data_datum.dptr);
				if (num >0){
					num--;
					RDEBUG("num: %d",num);
					data_datum.dptr = (char *) &num;
					data_datum.dsize = sizeof(int);
					rcode = gdbm_store(inst->ip, key_datum, data_datum, GDBM_REPLACE);
					if (rcode < 0) {
						ERROR("rlm_ippool: Failed storing data to %s: %s",
								inst->ip_index, gdbm_strerror(gdbm_errno));
						pthread_mutex_unlock(&inst->op_mutex);
						return RLM_MODULE_FAIL;
					}
					if (num >0 && entry.extra == 1){
						/*
						 * We are doing MPPP and we still have nas/port entries referencing
						 * this ip. Delete this entry so that eventually we only keep one
						 * reference to this ip.
						 */
						gdbm_delete(inst->gdbm,save_datum);
					}
				}
			}
		}
	}

	pthread_mutex_unlock(&inst->op_mutex);

	/*
	 * If there is a Framed-IP-Address (or Dhcp-Your-IP-Address)
	 * attribute in the reply, check for override
	 */
	if (pairfind(request->reply->vps, attr_ipaddr, vendor_ipaddr, TAG_ANY) != NULL) {
		RDEBUG("Found IP address attribute in reply attribute list.");
		if (inst->override)
		{
			RDEBUG("Override supplied IP address");
			pairdelete(&request->reply->vps, attr_ipaddr, vendor_ipaddr, TAG_ANY);
		} else {
			/* Abort */
			RDEBUG("override is set to no. Return NOOP.");
			return RLM_MODULE_NOOP;
		}
	}

	/*
	 * Walk through the database searching for an active=0 entry.
	 * We search twice. Once to see if we have an active entry with the same caller_id
	 * so that MPPP can work ok and then once again to find a free entry.
	 */

	pthread_mutex_lock(&inst->op_mutex);

	key_datum.dptr = NULL;
	if (cli != NULL){
		key_datum = gdbm_firstkey(inst->gdbm);
		while(key_datum.dptr){
			data_datum = gdbm_fetch(inst->gdbm, key_datum);
			if (data_datum.dptr){
				memcpy(&entry,data_datum.dptr, sizeof(ippool_info));
				free(data_datum.dptr);
				/*
		 		* If we find an entry for the same caller-id with active=1
		 		* then we use that for multilink (MPPP) to work properly.
		 		*/
				if (strcmp(entry.cli,cli) == 0 && entry.active){
					mppp = 1;
					break;
				}
			}
			nextkey = gdbm_nextkey(inst->gdbm, key_datum);
			free(key_datum.dptr);
			key_datum = nextkey;
		}
	}

	if (!key_datum.dptr){
		key_datum = gdbm_firstkey(inst->gdbm);
		while(key_datum.dptr){
			data_datum = gdbm_fetch(inst->gdbm, key_datum);
			if (data_datum.dptr){
				memcpy(&entry,data_datum.dptr, sizeof(ippool_info));
				free(data_datum.dptr);

				/*
				 * Find an entry with active == 0
				 * or an entry that has expired
				 */
				if (entry.active == 0 || (entry.timestamp && ((entry.timeout &&
				request->timestamp >= (entry.timestamp + entry.timeout)) ||
				(inst->max_timeout && request->timestamp >= (entry.timestamp + inst->max_timeout))))){
					datum tmp;

					tmp.dptr = (char *) &entry.ipaddr;
					tmp.dsize = sizeof(uint32_t);
					data_datum = gdbm_fetch(inst->ip, tmp);

					/*
					 * If we find an entry in the ip index and the number is zero (meaning
					 * that we haven't allocated the same ip address to another nas/port pair)
					 * or if we don't find an entry then delete the session entry so
					 * that we can change the key
					 * Else we don't delete the session entry since we haven't yet deallocated the
					 * corresponding ip address and we continue our search.
					 */

					if (data_datum.dptr){
						memcpy(&num,data_datum.dptr, sizeof(int));
						free(data_datum.dptr);
						if (num == 0){
							delete = 1;
							break;
						}
					}
					else{
						delete = 1;
						break;
					}
				}
			}
			nextkey = gdbm_nextkey(inst->gdbm, key_datum);
			free(key_datum.dptr);
			key_datum = nextkey;
		}
	}
	/*
	 * If we have found a free entry set active to 1 then add a Framed-IP-Address attribute to
	 * the reply
	 * We keep the operation mutex locked until after we have set the corresponding entry active
	 */
	if (key_datum.dptr){
		if (found && !mppp){
			/*
			 * Found == 1 means we have the nas/port combination entry in our database
			 * We exchange the ip address between the nas/port entry and the free entry
			 * Afterwards we will save the free ip address to the nas/port entry.
			 * That is:
			 *  ---------------------------------------------
			 *  - NAS/PORT Entry  |||| Free Entry  ||| Time
			 *  -    IP1		 IP2(Free)    BEFORE
			 *  -    IP2(Free)	   IP1	  AFTER
			 *  ---------------------------------------------
			 *
			 * We only do this if we are NOT doing MPPP
			 *
			 */
			datum key_datum_tmp;
			datum data_datum_tmp;
			ippool_key key_tmp;

			memcpy(key_tmp.key,key_str,16);
			key_datum_tmp.dptr = (char *) &key_tmp;
			key_datum_tmp.dsize = sizeof(ippool_key);

			data_datum_tmp = gdbm_fetch(inst->gdbm, key_datum_tmp);
			if (data_datum_tmp.dptr != NULL){

				rcode = gdbm_store(inst->gdbm, key_datum, data_datum_tmp, GDBM_REPLACE);
				free(data_datum_tmp.dptr);
				if (rcode < 0) {
					ERROR("rlm_ippool: Failed storing data to %s: %s",
						inst->filename, gdbm_strerror(gdbm_errno));
						pthread_mutex_unlock(&inst->op_mutex);
					return RLM_MODULE_FAIL;
				}
			}
		}
		else{
			/*
			 * We have not found the nas/port combination
			 */
			if (delete){
				/*
		 	  	 * Delete the entry so that we can change the key
			 	 * All is well. We delete one entry and we add one entry
		 	 	 */
				gdbm_delete(inst->gdbm, key_datum);
			}
			else{
				/*
				 * We are doing MPPP. (mppp should be 1)
				 * We don't do anything.
				 * We will create an extra not needed entry in the database in this case
				 * but we don't really care since we always also use the ip_index database
				 * when we search for a free entry.
				 * We will also delete that entry on the accounting section so that we only
				 * have one nas/port entry referencing each ip
				 */
				if (mppp)
					extra = 1;
				if (!mppp)
					ERROR("rlm_ippool: mppp is not one. Please report this behaviour.");
			}
		}
		free(key_datum.dptr);
		entry.active = 1;
		entry.timestamp = request->timestamp;
		if ((vp = pairfind(request->reply->vps, PW_SESSION_TIMEOUT, 0, TAG_ANY)) != NULL) {
			entry.timeout = (time_t) vp->vp_integer;
#ifdef WITH_DHCP
			if (dhcp) {
				vp = radius_paircreate(request, &request->reply->vps,
						       PW_DHCP_IP_ADDRESS_LEASE_TIME, DHCP_MAGIC_VENDOR);
				vp->vp_integer = entry.timeout;
				pairdelete(&request->reply->vps, PW_SESSION_TIMEOUT, 0, TAG_ANY);
			}
#endif
		} else {
			entry.timeout = 0;
		}
		if (extra)
			entry.extra = 1;
		data_datum.dptr = (char *) &entry;
		data_datum.dsize = sizeof(ippool_info);
		memcpy(key.key, key_str, 16);
		key_datum.dptr = (char *) &key;
		key_datum.dsize = sizeof(ippool_key);

		DEBUG2("rlm_ippool: Allocating ip to key: '%s'",hex_str);
		rcode = gdbm_store(inst->gdbm, key_datum, data_datum, GDBM_REPLACE);
		if (rcode < 0) {
			ERROR("rlm_ippool: Failed storing data to %s: %s",
				inst->filename, gdbm_strerror(gdbm_errno));
				pthread_mutex_unlock(&inst->op_mutex);
			return RLM_MODULE_FAIL;
		}

		/* Increase the ip index count */
		key_datum.dptr = (char *) &entry.ipaddr;
		key_datum.dsize = sizeof(uint32_t);
		data_datum = gdbm_fetch(inst->ip, key_datum);
		if (data_datum.dptr){
			memcpy(&num,data_datum.dptr,sizeof(int));
			free(data_datum.dptr);
		} else
			num = 0;
		num++;
		RDEBUG("num: %d",num);
		data_datum.dptr = (char *) &num;
		data_datum.dsize = sizeof(int);
		rcode = gdbm_store(inst->ip, key_datum, data_datum, GDBM_REPLACE);
		if (rcode < 0) {
			ERROR("rlm_ippool: Failed storing data to %s: %s",
				inst->ip_index, gdbm_strerror(gdbm_errno));
			pthread_mutex_unlock(&inst->op_mutex);
			return RLM_MODULE_FAIL;
		}
		pthread_mutex_unlock(&inst->op_mutex);


		RDEBUG("Allocated ip %s to client key: %s",ip_ntoa(str,entry.ipaddr),hex_str);
		vp = radius_paircreate(request, &request->reply->vps,
				       attr_ipaddr, vendor_ipaddr);
		vp->vp_ipaddr = entry.ipaddr;

		/*
		 *	If there is no Framed-Netmask attribute in the
		 *	reply, add one
		 */
		if (pairfind(request->reply->vps, attr_ipmask, vendor_ipaddr, TAG_ANY) == NULL) {
			vp = radius_paircreate(request, &request->reply->vps,
					       attr_ipmask, vendor_ipaddr);
			vp->vp_ipaddr = ntohl(inst->netmask);
		}

	}
	else{
		pthread_mutex_unlock(&inst->op_mutex);
		RDEBUG("No available ip addresses in pool.");
		return RLM_MODULE_NOTFOUND;
	}

	return RLM_MODULE_OK;
}

static int mod_detach(void *instance)
{
	rlm_ippool_t *inst = instance;

	gdbm_close(inst->gdbm);
	gdbm_close(inst->ip);
	pthread_mutex_destroy(&inst->op_mutex);
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
module_t rlm_ippool = {
	RLM_MODULE_INIT,
	"ippool",
	RLM_TYPE_THREAD_SAFE,		/* type */
	sizeof(rlm_ippool_t),
	module_config,
	mod_instantiate,		/* instantiation */
	mod_detach,			/* detach */
	{
		NULL,			/* authentication */
		NULL,		 	/* authorization */
		NULL,			/* preaccounting */
		mod_accounting,	/* accounting */
		NULL,			/* checksimul */
		NULL,			/* pre-proxy */
		NULL,			/* post-proxy */
		mod_post_auth		/* post-auth */
	},
};
