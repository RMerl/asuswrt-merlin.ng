/*
 *  ike_conf.c - module config loading functions
 *
 *  This file is part of rlm_eap_ikev2 freeRADIUS module which implements
 *  EAP-IKEv2 protocol functionality.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Copyright (C) 2005-2006 Krzysztof Rzecki <krzysztof.rzecki@ccns.pl>
 *  Copyright (C) 2005-2006 Rafal Mijal <rafal.mijal@ccns.pl>
 *  Copyright (C) 2005-2006 Piotr Marnik <piotr.marnik@ccns.pl>
 *  Copyright (C) 2005-2006 Pawel Matejski <pawel.matejski@ccns.pl>
 *  Copyright 1999-2007 The FreeRADIUS server project
 *
 */

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/rad_assert.h>
#include "ike_conf.h"
#include "eap.h"
#include "logging_impl.h"

static int rad_load_transforms(struct Protocol *prot,CONF_SECTION *cf);

struct config_transform
{
	char const *name;
	u_int8_t type;
	int exist_flag;
};

enum {
	OPT_INTEGRITY	= 0x01,
	OPT_PRF		= 0x02,
	OPT_ENCRYPTION	= 0x04,
	OPT_DHGROUP	= 0x08,
	OPT_NEEDED	= OPT_INTEGRITY | OPT_PRF | OPT_ENCRYPTION | OPT_DHGROUP

};



static struct config_transform config_transforms[] =
{
	 {"integrity",	IKEv2_TRT_INTEGRITY_ALGORITHM,		OPT_INTEGRITY},
	 {"prf",	IKEv2_TRT_PSEUDO_RANDOM_FUNCTION,	OPT_PRF},
	 {"encryption",	IKEv2_TRT_ENCRYPTION_ALGORITHM,		OPT_ENCRYPTION},
	 {"dhgroup",	IKEv2_TRT_DIFFIE_HELLMAN_GROUP,		OPT_DHGROUP },
	 {NULL, 0, 0} /* end of list */

};



/*
 *	Copied from rlm_files, and NOT under the same copyright
 *	as the rest of the module!
 *
 *	Also, it is UNNECESSARY to read the "users" file here!
 *	Doing this shows a misunderstanding of how the server works.
 */
int getusersfile(TALLOC_CTX *ctx, char const *filename, PAIR_LIST **pair_list, char const *compat_mode_str)
{
	int rcode;
	PAIR_LIST *users = NULL;

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
		PAIR_LIST *entry;
		VALUE_PAIR *vp;
		int compat_mode = false;

		if (strcmp(compat_mode_str, "cistron") == 0) {
			compat_mode = true;
		}

		entry = users;
		while (entry) {
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
			for (vp = entry->check; vp != NULL; vp = vp->next) {
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
				if ((vp->da->vendor!= 0) ||
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
						DEBUG("\tChanging '%s =' to '%s +='",
								vp->da->name, vp->da->name);
						vp->op = T_OP_ADD;
					} else {
						DEBUG("\tChanging '%s =' to '%s =='",
								vp->da->name, vp->da->name);
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
			for (vp = entry->reply; vp != NULL; vp = vp->next) {
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
					       " with the other check items",
					       filename, entry->lineno, vp->da->name, entry->name);
				}
			}

			entry = entry->next;
		}
	}

	*pair_list = users;
	return 0;
}

/**
 * Load all proposals from 'propsals' subsection
 */

int rad_load_proposals(ikev2_ctx *i2,CONF_SECTION *cf)
{
	rad_assert(i2!=NULL && cf!=NULL);

	CONF_SECTION *cf_prop=NULL;
	cf=cf_subsection_find_next(cf,NULL,"proposals");
	if(!cf) {
	ERROR(IKEv2_LOG_PREFIX "Can't find proposals section");
	return -1;
	}
	int nprop=0;
	for(
		cf_prop=cf_subsection_find_next(cf,NULL,"proposal");
		cf_prop;
		cf_prop=cf_subsection_find_next(cf,cf_prop,"proposal")
	   ) {
	nprop++;
	struct Proposal *prop;
	struct Protocol *prot;
	prop=AddProposal(&i2->suppProp);
	prot=AddProtocol(prop,IKEv2_PID_IKE_SA,0,0);
	if(rad_load_transforms(prot,cf_prop)) {
		ERROR(IKEv2_LOG_PREFIX "Failed to load proposal (%d)",
			nprop);
		return -1;
	}
	}
	if(!nprop) {
	ERROR(IKEv2_LOG_PREFIX "Can't find any proposal");
	return -1;
	}
	return 0;

}



/**
 * Load transforms from protocol subsection
 */

static int rad_load_transforms(struct Protocol *prot, CONF_SECTION *cf)
{
	CONF_PAIR *cp;
	int option_exists = 0;
	int i = 0;

	rad_assert(prot);
	rad_assert(cf);

	DEBUG(IKEv2_LOG_PREFIX "Begin load transforms");

	while(config_transforms[i].name)
	{
		uint8_t id;
		uint16_t keylen;

		for(cp = cf_pair_find(cf,config_transforms[i].name);
		    cp;
		    cp = cf_pair_find_next(cf,cp,config_transforms[i].name)) {
			if (TransformFromName(cf_pair_value(cp),config_transforms[i].type,&id,&keylen)) {
				ERROR(IKEv2_LOG_PREFIX "Unsupported %s transform: %s ",
				      config_transforms[i].name,cf_pair_value(cp));
				return -1;
			}

			if (!AddTransform(prot,config_transforms[i].type,id,keylen)) {
				ERROR(IKEv2_LOG_PREFIX "Problem with transform %s:%s",
				      config_transforms[i].name,cf_pair_value(cp));
				return -1;
			}
			option_exists |= config_transforms[i].exist_flag;
		}
		i++;
	}

	if ((option_exists & OPT_NEEDED) != OPT_NEEDED) {
		ERROR(IKEv2_LOG_PREFIX "Not all mandatory transforms are set properly");
		DEBUG(IKEv2_LOG_PREFIX "Option flags: 0x%02X",option_exists);

		return -1;
	}
	return 0;
}


void rad_update_shared_seclist(struct sharedSecList **list, char const *id, VALUE_PAIR *items,
				   int default_client_authtype)
{
	rad_assert(list && id);

	char *ike_id;
	char *secret = NULL;
	int id_type = 0;
	int authtype = default_client_authtype;
	VALUE_PAIR *vp;

	memcpy(&ike_id, &id, sizeof(id));

	if (!items) {
		AddSharedSec(list, 0, ike_id, NULL, default_client_authtype);

		return;
	}

	//idtype
	vp = pairfind(items, RAD_EAP_IKEV2_IDTYPE, 0, TAG_ANY);
	if (!vp) {
		DEBUG(IKEv2_LOG_PREFIX "[%s] -- Id type not set", id);
	} else {
		id_type = vp->vp_integer;
		if (!id_type) {
			DEBUG(IKEv2_LOG_PREFIX "[%s] -- Not valid id type", id);
		}
	}

	//secret
	vp = pairfind(items, RAD_EAP_IKEV2_SECRET, 0, TAG_ANY);
	if (!vp || !vp->length) {
		DEBUG(IKEv2_LOG_PREFIX "[%s] -- Secret not set", id);
	} else {
		secret = vp->vp_strvalue;
	}

	//authtype
	vp = pairfind(items, RAD_EAP_IKEV2_AUTHTYPE, 0, TAG_ANY);
	if (vp && vp->length) {
		authtype = AuthtypeFromName(vp->vp_strvalue);

		if (authtype == -1) {
			ERROR(IKEv2_LOG_PREFIX "Unsupported 'EAP-IKEv2-AuthType' value (%s),using 'both'",
			      vp->vp_strvalue);
			authtype = IKEv2_AUTH_BOTH;
		}

	}

	AddSharedSec(list, id_type, ike_id, secret, authtype);
}

/**
 * load user credentials from raddb/users (read directly from users file)
 */

int rad_load_credentials(TALLOC_CTX *ctx, ikev2_ctx *i2,char *filename,char *authtype_name)
{
	rad_assert(i2 && filename && authtype_name);
	int authtype;

	authtype=AuthtypeFromName(authtype_name);
	if(authtype==-1) {
	ERROR(IKEv2_LOG_PREFIX "Unsupported 'default_auth_type' value (%s), using both",authtype_name);
	authtype=IKEv2_AUTH_BOTH;
	}

	PAIR_LIST *users=NULL;
	if(getusersfile(ctx, filename,&users,"no")!=0) {
	ERROR(IKEv2_LOG_PREFIX "Error while loading %s userfile",filename);
	return -1;
	}
	PAIR_LIST *tusers=users;
	while(tusers) {
	if(strcmp(tusers->name,"DEFAULT")) {
		rad_update_shared_seclist(&i2->sslist,tusers->name,tusers->check,authtype);
	}
	tusers=tusers->next;
	}
	pairlist_free(&users);
	//print sslist
//	struct sharedSecList *sslist=i2->sslist;
//	while(sslist) {
//	ERROR("sslist:id=%s",sslist->id);
//	ERROR("sslist:idlen=%d",sslist->idlen);
//	ERROR("sslist:pwd=%s",sslist->pwd);
//	ERROR("sslist:pwdlen=%d",sslist->pwdlen);
//	ERROR("sslist:idtype= %d",sslist->idtype);
//	ERROR("sslist:authtype=%d",sslist->authtype);
//	sslist=sslist->next;
//	}
	return 0;


}

int rad_get_authtype(char* authtype_name)
{
	rad_assert(authtype_name);
	if(!strcmp(authtype_name,"cert")) {
	DEBUG(IKEv2_LOG_PREFIX "Using  server auth type: cert");
	return IKEv2_AUTH_CERT;
	}
	if(!strcmp(authtype_name,"secret")) {
	DEBUG(IKEv2_LOG_PREFIX "Using server auth type: secret");
	return IKEv2_AUTH_SK;
	}
	AUTH(IKEv2_LOG_PREFIX "Unsupported server auth type: %s",authtype_name);
	AUTH(IKEv2_LOG_PREFIX "Using server auth type: secret (default)");
	return IKEv2_AUTH_SK;
}

int file_exists(char *filename)
{
	int result=0;
	FILE *fp=fopen(filename,"r");
	if(fp) {
	result=1;
	fclose(fp);
	}
	return result;
}



