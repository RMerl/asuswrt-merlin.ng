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
 * @file rlm_files.c
 * @brief Process simple 'users' policy files.
 *
 * @copyright 2000,2006  The FreeRADIUS server project
 * @copyright 2000  Jeff Carneal <jeff@apex.net>
 */
RCSID("$Id$")

#include	<freeradius-devel/radiusd.h>
#include	<freeradius-devel/modules.h>

#include	<ctype.h>
#include	<fcntl.h>
#include	<limits.h>

typedef struct rlm_files_t {
	char *compat_mode;

	char *key;

	char *filename;
	fr_hash_table_t *common;

	/* autz */
	char *usersfile;
	fr_hash_table_t *users;


	/* authenticate */
	char *auth_usersfile;
	fr_hash_table_t *auth_users;

	/* preacct */
	char *acctusersfile;
	fr_hash_table_t *acctusers;

#ifdef WITH_PROXY
	/* pre-proxy */
	char *preproxy_usersfile;
	fr_hash_table_t *preproxy_users;

	/* post-proxy */
	char *postproxy_usersfile;
	fr_hash_table_t *postproxy_users;
#endif

	/* post-authenticate */
	char *postauth_usersfile;
	fr_hash_table_t *postauth_users;
} rlm_files_t;


/*
 *     See if a VALUE_PAIR list contains Fall-Through = Yes
 */
static int fallthrough(VALUE_PAIR *vp)
{
	VALUE_PAIR *tmp;
	tmp = pairfind(vp, PW_FALL_THROUGH, 0, TAG_ANY);

	return tmp ? tmp->vp_integer : 0;
}

static const CONF_PARSER module_config[] = {
	{ "filename",	   PW_TYPE_FILE_INPUT,
	  offsetof(rlm_files_t,filename), NULL, NULL },
	{ "usersfile",	   PW_TYPE_FILE_INPUT,
	  offsetof(rlm_files_t,usersfile), NULL, NULL },
	{ "acctusersfile", PW_TYPE_FILE_INPUT,
	  offsetof(rlm_files_t,acctusersfile), NULL, NULL },
#ifdef WITH_PROXY
	{ "preproxy_usersfile", PW_TYPE_FILE_INPUT,
	  offsetof(rlm_files_t,preproxy_usersfile), NULL, NULL },
	{ "postproxy_usersfile", PW_TYPE_FILE_INPUT,
	  offsetof(rlm_files_t,postproxy_usersfile), NULL, NULL },
#endif
	{ "auth_usersfile", PW_TYPE_FILE_INPUT,
	  offsetof(rlm_files_t,auth_usersfile), NULL, NULL },
	{ "postauth_usersfile", PW_TYPE_FILE_INPUT,
	  offsetof(rlm_files_t,postauth_usersfile), NULL, NULL },
	{ "compat",	   PW_TYPE_STRING_PTR,
	  offsetof(rlm_files_t,compat_mode), NULL, "cistron" },
	{ "key",	   PW_TYPE_STRING_PTR,
	  offsetof(rlm_files_t,key), NULL, NULL },
	{ NULL, -1, 0, NULL, NULL }
};


static uint32_t pairlist_hash(void const *data)
{
	return fr_hash_string(((PAIR_LIST const *)data)->name);
}

static int pairlist_cmp(void const *a, void const *b)
{
	return strcmp(((PAIR_LIST const *)a)->name,
		      ((PAIR_LIST const *)b)->name);
}

static void my_pairlist_free(void *data)
{
	PAIR_LIST *pl = data;

	pairlist_free(&pl);
}


static int getusersfile(TALLOC_CTX *ctx, char const *filename, fr_hash_table_t **pht,
			char *compat_mode_str)
{
	int rcode;
	PAIR_LIST *users = NULL;
	PAIR_LIST *entry, *next;
	fr_hash_table_t *ht, *tailht;
	int order = 0;

	if (!filename) {
		*pht = NULL;
		return 0;
	}

	rcode = pairlist_read(ctx, filename, &users, 1);
	if (rcode < 0) {
		return -1;
	}

	/*
	 *	Walk through the 'users' file list, if we're debugging,
	 *	or if we're in compat_mode.
	 */
	if ((debug_flag) ||
	    (strcmp(compat_mode_str, "cistron") == 0)) {
		VALUE_PAIR *vp;
		int compat_mode = false;

		if (strcmp(compat_mode_str, "cistron") == 0) {
			compat_mode = true;
		}

		entry = users;
		while (entry) {
			vp_cursor_t cursor;
			if (compat_mode) {
				DEBUG("[%s]:%d Cistron compatibility checks for entry %s ...",
						filename, entry->lineno,
						entry->name);
			}

			/*
			 *	Look for improper use of '=' in the
			 *	check items.  They should be using
			 *	'==' for on-the-wire RADIUS attributes,
			 *	and probably ':=' for server
			 *	configuration items.
			 */
			for (vp = paircursor(&cursor, &entry->check); vp; vp = pairnext(&cursor)) {
				/*
				 *	Ignore attributes which are set
				 *	properly.
				 */
				if (vp->op != T_OP_EQ) {
					continue;
				}

				/*
				 *	If it's a vendor attribute,
				 *	or it's a wire protocol,
				 *	ensure it has '=='.
				 */
				if ((vp->da->vendor != 0) ||
						(vp->da->attr < 0x100)) {
					if (!compat_mode) {
						WDEBUG("[%s]:%d Changing '%s =' to '%s =='\n\tfor comparing RADIUS attribute in check item list for user %s",
								filename, entry->lineno,
								vp->da->name, vp->da->name,
								entry->name);
					} else {
						DEBUG("\tChanging '%s =' to '%s =='",
								vp->da->name, vp->da->name);
					}
					vp->op = T_OP_CMP_EQ;
					continue;
				}

				/*
				 *	Cistron Compatibility mode.
				 *
				 *	Re-write selected attributes
				 *	to be '+=', instead of '='.
				 *
				 *	All others get set to '=='
				 */
				if (compat_mode) {
					/*
					 *	Non-wire attributes become +=
					 *
					 *	On the write attributes
					 *	become ==
					 */
					if ((vp->da->attr >= 0x100) &&
							(vp->da->attr <= 0xffff) &&
							(vp->da->attr != PW_HINT) &&
							(vp->da->attr != PW_HUNTGROUP_NAME)) {
						DEBUG("\tChanging '%s =' to '%s +='", vp->da->name, vp->da->name);

						vp->op = T_OP_ADD;
					} else {
						DEBUG("\tChanging '%s =' to '%s =='", vp->da->name, vp->da->name);

						vp->op = T_OP_CMP_EQ;
					}
				}

			} /* end of loop over check items */

			/*
			 *	Look for server configuration items
			 *	in the reply list.
			 *
			 *	It's a common enough mistake, that it's
			 *	worth doing.
			 */
			for (vp = paircursor(&cursor, &entry->reply); vp; vp = pairnext(&cursor)) {
				/*
				 *	If it's NOT a vendor attribute,
				 *	and it's NOT a wire protocol
				 *	and we ignore Fall-Through,
				 *	then bitch about it, giving a
				 *	good warning message.
				 */
				 if ((vp->da->vendor == 0) &&
					(vp->da->attr > 0xff) &&
					(vp->da->attr > 1000)) {
					WDEBUG("[%s]:%d Check item \"%s\"\n"
					       "\tfound in reply item list for user \"%s\".\n"
					       "\tThis attribute MUST go on the first line"
					       " with the other check items", filename, entry->lineno, vp->da->name,
					       entry->name);
				}
			}

			entry = entry->next;
		}

	}

	ht = fr_hash_table_create(pairlist_hash, pairlist_cmp,
				    my_pairlist_free);
	if (!ht) {
		pairlist_free(&users);
		return -1;
	}

	tailht = fr_hash_table_create(pairlist_hash, pairlist_cmp,
					NULL);
	if (!tailht) {
		fr_hash_table_free(ht);
		pairlist_free(&users);
		return -1;
	}

	/*
	 *	Now that we've read it in, put the entries into a hash
	 *	for faster access.
	 */
	for (entry = users; entry != NULL; entry = next) {
		PAIR_LIST *tail;

		next = entry->next;
		entry->next = NULL;
		entry->order = order++;

		/*
		 *	Insert it into the hash table, and remember
		 *	the tail of the linked list.
		 */
		tail = fr_hash_table_finddata(tailht, entry);
		if (!tail) {
			/*
			 *	Insert it into the head & tail.
			 */
			if (!fr_hash_table_insert(ht, entry) ||
			    !fr_hash_table_insert(tailht, entry)) {
				pairlist_free(&next);
				fr_hash_table_free(ht);
				fr_hash_table_free(tailht);
				return -1;
			}
		} else {
			tail->next = entry;
			if (!fr_hash_table_replace(tailht, entry)) {
				pairlist_free(&next);
				fr_hash_table_free(ht);
				fr_hash_table_free(tailht);
				return -1;
			}
		}
	}

	fr_hash_table_free(tailht);
	*pht = ht;

	return 0;
}

/*
 *	Clean up.
 */
static int mod_detach(void *instance)
{
	rlm_files_t *inst = instance;
	fr_hash_table_free(inst->common);
	fr_hash_table_free(inst->users);
	fr_hash_table_free(inst->acctusers);
#ifdef WITH_PROXY
	fr_hash_table_free(inst->preproxy_users);
	fr_hash_table_free(inst->postproxy_users);
#endif
	fr_hash_table_free(inst->auth_users);
	fr_hash_table_free(inst->postauth_users);
	return 0;
}



/*
 *	(Re-)read the "users" file into memory.
 */
static int mod_instantiate(UNUSED CONF_SECTION *conf, void *instance)
{
	rlm_files_t *inst = instance;

#undef READFILE
#define READFILE(_x, _y) do { if (getusersfile(inst, inst->_x, &inst->_y, inst->compat_mode) != 0) { ERROR("Failed reading %s", inst->_x); mod_detach(inst);return -1;} } while (0)

	READFILE(filename, common);
	READFILE(usersfile, users);
	READFILE(acctusersfile, acctusers);

#ifdef WITH_PROXY
	READFILE(preproxy_usersfile, preproxy_users);
	READFILE(postproxy_usersfile, postproxy_users);
#endif

	READFILE(auth_usersfile, auth_users);
	READFILE(postauth_usersfile, postauth_users);

	return 0;
}

/*
 *	Common code called by everything below.
 */
static rlm_rcode_t file_common(rlm_files_t *inst, REQUEST *request,
		       char const *filename, fr_hash_table_t *ht,
		       VALUE_PAIR *request_pairs, VALUE_PAIR **reply_pairs)
{
	char const	*name, *match;
	VALUE_PAIR	*check_tmp;
	VALUE_PAIR	*reply_tmp;
	PAIR_LIST const *user_pl, *default_pl;
	int		found = 0;
	PAIR_LIST	my_pl;
	char		buffer[256];

	if (!inst->key) {
		VALUE_PAIR	*namepair;

		namepair = request->username;
		name = namepair ? namepair->vp_strvalue : "NONE";
	} else {
		int len;

		len = radius_xlat(buffer, sizeof(buffer), request, inst->key, NULL, NULL);
		if (len < 0) {
			return RLM_MODULE_FAIL;
		}

		name = len ? buffer : "NONE";
	}

	if (!ht) return RLM_MODULE_NOOP;

	my_pl.name = name;
	user_pl = fr_hash_table_finddata(ht, &my_pl);
	my_pl.name = "DEFAULT";
	default_pl = fr_hash_table_finddata(ht, &my_pl);

	/*
	 *	Find the entry for the user.
	 */
	while (user_pl || default_pl) {
		vp_cursor_t cursor;
		VALUE_PAIR *vp;
		PAIR_LIST const *pl;

		if (!default_pl && user_pl) {
			pl = user_pl;
			match = name;
			user_pl = user_pl->next;

		} else if (!user_pl && default_pl) {
			pl = default_pl;
			match = "DEFAULT";
			default_pl = default_pl->next;

		} else if (user_pl->order < default_pl->order) {
			pl = user_pl;
			match = name;
			user_pl = user_pl->next;

		} else {
			pl = default_pl;
			match = "DEFAULT";
			default_pl = default_pl->next;
		}

		check_tmp = paircopy(request, pl->check);
		for (vp = paircursor(&cursor, &check_tmp);
		     vp;
		     vp = pairnext(&cursor)) {
			if (radius_xlat_do(request, vp) < 0) {
				RWARN("Failed parsing expanded value for check item, skipping entry: %s", fr_strerror());
				pairfree(&check_tmp);
				continue;
			}
		}

		if (paircompare(request, request_pairs, pl->check, reply_pairs) == 0) {
			RDEBUG2("%s: Matched entry %s at line %d", filename, match, pl->lineno);
			found = 1;

			/* ctx may be reply or proxy */
			reply_tmp = paircopy(request, pl->reply);
			radius_xlat_move(request, reply_pairs, &reply_tmp);
			pairmove(request, &request->config_items, &check_tmp);

			/* Cleanup any unmoved valuepairs */
			pairfree(&reply_tmp);
			pairfree(&check_tmp);

			/*
			 *	Fallthrough?
			 */
			if (!fallthrough(pl->reply))
				break;
		}
	}

	/*
	 *	Remove server internal parameters.
	 */
	pairdelete(reply_pairs, PW_FALL_THROUGH, 0, TAG_ANY);

	/*
	 *	See if we succeeded.
	 */
	if (!found)
		return RLM_MODULE_NOOP; /* on to the next module */

	return RLM_MODULE_OK;

}


/*
 *	Find the named user in the database.  Create the
 *	set of attribute-value pairs to check and reply with
 *	for this user from the database. The main code only
 *	needs to check the password, the rest is done here.
 */
static rlm_rcode_t mod_authorize(void *instance, REQUEST *request)
{
	rlm_files_t *inst = instance;

	return file_common(inst, request, "users",
			   inst->users ? inst->users : inst->common,
			   request->packet->vps, &request->reply->vps);
}


/*
 *	Pre-Accounting - read the acct_users file for check_items and
 *	config_items. Reply items are Not Recommended(TM) in acct_users,
 *	except for Fallthrough, which should work
 */
static rlm_rcode_t mod_preacct(void *instance, REQUEST *request)
{
	rlm_files_t *inst = instance;

	return file_common(inst, request, "acct_users",
			   inst->acctusers ? inst->acctusers : inst->common,
			   request->packet->vps, &request->reply->vps);
}

#ifdef WITH_PROXY
static rlm_rcode_t file_preproxy(void *instance, REQUEST *request)
{
	rlm_files_t *inst = instance;

	return file_common(inst, request, "preproxy_users",
			   inst->preproxy_users ? inst->preproxy_users : inst->common,
			   request->packet->vps, &request->proxy->vps);
}

static rlm_rcode_t file_postproxy(void *instance, REQUEST *request)
{
	rlm_files_t *inst = instance;

	return file_common(inst, request, "postproxy_users",
			   inst->postproxy_users ? inst->postproxy_users : inst->common,
			   request->proxy_reply->vps, &request->reply->vps);
}
#endif

static rlm_rcode_t mod_authenticate(void *instance, REQUEST *request)
{
	rlm_files_t *inst = instance;

	return file_common(inst, request, "auth_users",
			   inst->auth_users ? inst->auth_users : inst->common,
			   request->packet->vps, &request->reply->vps);
}

static rlm_rcode_t mod_post_auth(void *instance, REQUEST *request)
{
	rlm_files_t *inst = instance;

	return file_common(inst, request, "postauth_users",
			   inst->postauth_users ? inst->postauth_users : inst->common,
			   request->packet->vps, &request->reply->vps);
}


/* globally exported name */
module_t rlm_files = {
	RLM_MODULE_INIT,
	"files",
	RLM_TYPE_CHECK_CONFIG_SAFE | RLM_TYPE_HUP_SAFE,
	sizeof(rlm_files_t),
	module_config,
	mod_instantiate,		/* instantiation */
	mod_detach,			/* detach */
	{
		mod_authenticate,	/* authentication */
		mod_authorize, 	/* authorization */
		mod_preacct,		/* preaccounting */
		NULL,			/* accounting */
		NULL,			/* checksimul */
#ifdef WITH_PROXY
		file_preproxy,		/* pre-proxy */
		file_postproxy,		/* post-proxy */
#else
		NULL, NULL,
#endif
		mod_post_auth		/* post-auth */
	},
};

