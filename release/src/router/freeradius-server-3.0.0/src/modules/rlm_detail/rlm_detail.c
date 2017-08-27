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
 * @file rlm_detail.c
 * @brief Write plaintext versions of packets to flatfiles.
 *
 * @copyright 2000,2006  The FreeRADIUS server project
 */
RCSID("$Id$")

#include	<freeradius-devel/radiusd.h>
#include	<freeradius-devel/modules.h>
#include	<freeradius-devel/rad_assert.h>
#include	<freeradius-devel/detail.h>

#include	<ctype.h>
#include	<fcntl.h>
#include	<sys/stat.h>

#ifdef HAVE_FNMATCH_H
#include	<fnmatch.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_GRP_H
#include <grp.h>
#endif

#define DIRLEN	8192		//!< Maximum path length.

/** Instance configuration for rlm_detail
 *
 * Holds the configuration and preparsed data for a instance of rlm_detail.
 */
typedef struct detail_instance {
	char	*filename;	//!< File/path to write to.
	int	perm;		//!< Permissions to use for new files.
	char	*group;		//!< Group to use for new files.

	int	dirperm;	//!< Directory permissions to use for new files.

	char	*header;	//!< Header format.
	int	locking;	//!< Whether the file should be locked.

	int	log_srcdst;	//!< Add IP src/dst attributes to entries.

	fr_hash_table_t *ht;	//!< Holds suppressed attributes.
} detail_instance_t;

static const CONF_PARSER module_config[] = {
	{ "detailfile",    PW_TYPE_FILE_OUTPUT | PW_TYPE_DEPRECATED,
	  offsetof(struct detail_instance,filename), NULL, NULL },
	{ "filename",    PW_TYPE_FILE_OUTPUT | PW_TYPE_REQUIRED,
	  offsetof(struct detail_instance,filename), NULL, "%A/%{Client-IP-Address}/detail" },
	{ "header",    PW_TYPE_STRING_PTR,
	  offsetof(struct detail_instance,header), NULL, "%t" },
	{ "detailperm",    PW_TYPE_INTEGER | PW_TYPE_DEPRECATED,
	  offsetof(struct detail_instance,perm), NULL, NULL },
	{ "permissions",    PW_TYPE_INTEGER,
	  offsetof(struct detail_instance,perm), NULL, "0600" },
	{ "group",	 PW_TYPE_STRING_PTR,
	  offsetof(struct detail_instance,group), NULL,  NULL},
	{ "dir_permissions", PW_TYPE_INTEGER,
	  offsetof(struct detail_instance,dirperm),    NULL, "0755" },
	{ "locking",       PW_TYPE_BOOLEAN,
	  offsetof(struct detail_instance,locking),    NULL, "no" },
	{ "log_packet_header",       PW_TYPE_BOOLEAN,
	  offsetof(struct detail_instance,log_srcdst),    NULL, "no" },
	{ NULL, -1, 0, NULL, NULL }
};


/*
 *	Clean up.
 */
static int mod_detach(void *instance)
{
	struct detail_instance *inst = instance;
	if (inst->ht) fr_hash_table_free(inst->ht);
	return 0;
}


static uint32_t detail_hash(void const *data)
{
	DICT_ATTR const *da = data;
	return fr_hash(&da, sizeof(da));
}

static int detail_cmp(void const *a, void const *b)
{
	DICT_ATTR const *one = a;
	DICT_ATTR const *two = b;

	return one - two;
}


/*
 *	(Re-)read radiusd.conf into memory.
 */
static int mod_instantiate(CONF_SECTION *conf, void *instance)
{
	detail_instance_t *inst = instance;
	CONF_SECTION	*cs;

	/*
	 *	Suppress certain attributes.
	 */
	cs = cf_section_sub_find(conf, "suppress");
	if (cs) {
		CONF_ITEM	*ci;

		inst->ht = fr_hash_table_create(detail_hash, detail_cmp, NULL);

		for (ci = cf_item_find_next(cs, NULL);
		     ci != NULL;
		     ci = cf_item_find_next(cs, ci)) {
			char const	*attr;
			DICT_ATTR const	*da;

			if (!cf_item_is_pair(ci)) continue;

			attr = cf_pair_attr(cf_itemtopair(ci));
			if (!attr) continue; /* pair-anoia */

			da = dict_attrbyname(attr);
			if (!da) {
				cf_log_err_cs(conf, "No such attribute '%s'",
					      attr);
			}

			if (!fr_hash_table_insert(inst->ht, da)) {
				ERROR("rlm_detail: Failed trying to remember %s", attr);
				return -1;
			}
		}
	}

	return 0;
}

/*
 *	Do detail, compatible with old accounting
 */
static rlm_rcode_t do_detail(void *instance, REQUEST *request, RADIUS_PACKET *packet, int compat)
{
	int		outfd;
	char		timestamp[256];
	char		buffer[DIRLEN];
	char		*p;
	struct stat	st;
	int		locked;
	int		lock_count;
	struct timeval	tv;
	VALUE_PAIR	*vp;
	off_t		fsize;
	FILE		*fp;

#ifdef HAVE_GRP_H
	gid_t		gid;
	struct group	*grp;
	char		*endptr;
#endif

	struct detail_instance *inst = instance;

	rad_assert(request != NULL);

	/*
	 *	Nothing to log: don't do anything.
	 */
	if (!packet) {
		return RLM_MODULE_NOOP;
	}

	/*
	 *	Create a directory for this nas.
	 *
	 *	Generate the path for the detail file.  Use the
	 *	same format, but truncate at the last /.  Then
	 *	feed it through radius_xlat() to expand the
	 *	variables.
	 */
	if (radius_xlat(buffer, sizeof(buffer), request, inst->filename, NULL, NULL) < 0) {
	    return RLM_MODULE_FAIL;
	}
	RDEBUG2("%s expands to %s", inst->filename, buffer);

#ifdef HAVE_FNMATCH_H
#ifdef FNM_FILE_NAME
	/*
	 *	If we read it from a detail file, and we're about to
	 *	write it back to the SAME detail file directory, then
	 *	suppress the write.  This check prevents an infinite
	 *	loop.
	 */
	if ((request->listener->type == RAD_LISTEN_DETAIL) &&
	    (fnmatch(((listen_detail_t *)request->listener->data)->filename,
		     buffer, FNM_FILE_NAME | FNM_PERIOD ) == 0)) {
		RWDEBUG2("Suppressing infinite loop.");
		return RLM_MODULE_NOOP;
	}
#endif
#endif

	/*
	 *	Grab the last directory delimiter.
	 */
	p = strrchr(buffer,'/');

	/*
	 *	There WAS a directory delimiter there, and the file
	 *	doesn't exist, so we must create it the directories..
	 */
	if (p) {
		*p = '\0';

		/*
		 *	Always try to create the directory.  If it
		 *	exists, rad_mkdir() will check via stat(), and
		 *	return immediately.
		 *
		 *	This catches the case where some idiot deleted
		 *	a directory that the server was using.
		 */
		if (rad_mkdir(buffer, inst->dirperm) < 0) {
			RERROR("rlm_detail: Failed to create directory %s: %s", buffer, strerror(errno));
			return RLM_MODULE_FAIL;
		}

		*p = '/';
	} /* else there was no directory delimiter. */

	locked = 0;
	lock_count = 0;
	do {
		/*
		 *	Open & create the file, with the given
		 *	permissions.
		 */
		if ((outfd = open(buffer, O_WRONLY | O_APPEND | O_CREAT,
				  inst->perm)) < 0) {
			RERROR("rlm_detail: Couldn't open file %s: %s",
			       buffer, strerror(errno));
			return RLM_MODULE_FAIL;
		}

		/*
		 *	If we fail to aquire the filelock in 80 tries
		 *	(approximately two seconds) we bail out.
		 */
		if (inst->locking) {
			lseek(outfd, 0L, SEEK_SET);
			if (rad_lockfd_nonblock(outfd, 0) < 0) {
				close(outfd);
				tv.tv_sec = 0;
				tv.tv_usec = 25000;
				select(0, NULL, NULL, NULL, &tv);
				lock_count++;
				continue;
			}

			/*
			 *	The file might have been deleted by
			 *	radrelay while we tried to acquire
			 *	the lock (race condition)
			 */
			if (fstat(outfd, &st) != 0) {
				RERROR("rlm_detail: Couldn't stat file %s: %s",
				       buffer, strerror(errno));
				close(outfd);
				return RLM_MODULE_FAIL;
			}
			if (st.st_nlink == 0) {
				RDEBUG2("File %s removed by another program, retrying",
				      buffer);
				close(outfd);
				lock_count = 0;
				continue;
			}

			RDEBUG2("Acquired filelock, tried %d time(s)",
			      lock_count + 1);
			locked = 1;
		}
	} while (inst->locking && !locked && lock_count < 80);

	if (inst->locking && !locked) {
		close(outfd);
		RERROR("rlm_detail: Failed to acquire filelock for %s, giving up",
		       buffer);
		return RLM_MODULE_FAIL;
	}


#ifdef HAVE_GRP_H
	if (inst->group != NULL) {
		gid = strtol(inst->group, &endptr, 10);
		if (*endptr != '\0') {
			grp = getgrnam(inst->group);
			if (!grp) {
				RDEBUG2("rlm_detail: Unable to find system group \"%s\"", inst->group);
				goto skip_group;
			}
			gid = grp->gr_gid;
		}

		if (chown(buffer, -1, gid) == -1) {
			RDEBUG2("rlm_detail: Unable to change system group of \"%s\"", buffer);
		}
	}

 skip_group:
#endif

	/*
	 *	Post a timestamp
	 */
	fsize = lseek(outfd, 0L, SEEK_END);
	if (fsize < 0) {
		RERROR("rlm_detail: Failed to seek to the end of detail file %s",
			buffer);
		close(outfd);
		return RLM_MODULE_FAIL;
	}

	if (radius_xlat(timestamp, sizeof(timestamp), request, inst->header, NULL, NULL) < 0) {
		close(outfd);
		return RLM_MODULE_FAIL;
	}

	/*
	 *	Open the FP for buffering.
	 */
	if ((fp = fdopen(outfd, "a")) == NULL) {
		RERROR("rlm_detail: Couldn't open file %s: %s",
			       buffer, strerror(errno));
		close(outfd);
		return RLM_MODULE_FAIL;
	}

	fprintf(fp, "%s\n", timestamp);

	/*
	 *	Write the information to the file.
	 */
	if (!compat) {
		/*
		 *	Print out names, if they're OK.
		 *	Numbers, if not.
		 */
		if ((packet->code > 0) &&
		    (packet->code < FR_MAX_PACKET_CODE)) {
			fprintf(fp, "\tPacket-Type = %s\n",
				fr_packet_codes[packet->code]);
		} else {
			fprintf(fp, "\tPacket-Type = %d\n", packet->code);
		}
	}

	if (inst->log_srcdst) {
		VALUE_PAIR src_vp, dst_vp;

		memset(&src_vp, 0, sizeof(src_vp));
		memset(&dst_vp, 0, sizeof(dst_vp));
		src_vp.op = dst_vp.op = T_OP_EQ;

		switch (packet->src_ipaddr.af) {
		case AF_INET:
			src_vp.da = dict_attrbyvalue(PW_PACKET_SRC_IP_ADDRESS, 0);
			src_vp.vp_ipaddr = packet->src_ipaddr.ipaddr.ip4addr.s_addr;

			dst_vp.da = dict_attrbyvalue(PW_PACKET_DST_IP_ADDRESS, 0);
			dst_vp.vp_ipaddr = packet->dst_ipaddr.ipaddr.ip4addr.s_addr;
			break;
		case AF_INET6:
			src_vp.da = dict_attrbyvalue(PW_PACKET_SRC_IPV6_ADDRESS, 0);
			memcpy(&src_vp.vp_ipv6addr,
			       &packet->src_ipaddr.ipaddr.ip6addr,
			       sizeof(packet->src_ipaddr.ipaddr.ip6addr));
			dst_vp.da = dict_attrbyvalue(PW_PACKET_DST_IPV6_ADDRESS, 0);
			memcpy(&dst_vp.vp_ipv6addr,
			       &packet->dst_ipaddr.ipaddr.ip6addr,
			       sizeof(packet->dst_ipaddr.ipaddr.ip6addr));
			break;
		default:
			break;
		}

		vp_print(fp, &src_vp);
		vp_print(fp, &dst_vp);

		src_vp.da = dict_attrbyvalue(PW_PACKET_SRC_PORT, 0);
		src_vp.vp_integer = packet->src_port;
		dst_vp.da = dict_attrbyvalue(PW_PACKET_DST_PORT, 0);
		dst_vp.vp_integer = packet->dst_port;

		vp_print(fp, &src_vp);
		vp_print(fp, &dst_vp);
	}

	{
		vp_cursor_t cursor;
		/* Write each attribute/value to the log file */
		for (vp = paircursor(&cursor, &packet->vps);
		     vp;
		     vp = pairnext(&cursor)) {
			if (inst->ht &&
			    fr_hash_table_finddata(inst->ht, &vp->da)) continue;

			/*
			 *	Don't print passwords in old format...
			 */
			if (compat && !vp->da->vendor && (vp->da->attr == PW_USER_PASSWORD)) continue;

			/*
			 *	Print all of the attributes.
			 */
			vp_print(fp, vp);
		}
	}

	/*
	 *	Add non-protocol attributes.
	 */
	if (compat) {
#ifdef WITH_PROXY
		if (request->proxy) {
			char proxy_buffer[128];

			inet_ntop(request->proxy->dst_ipaddr.af,
				  &request->proxy->dst_ipaddr.ipaddr,
				  proxy_buffer, sizeof(proxy_buffer));
			fprintf(fp, "\tFreeradius-Proxied-To = %s\n",
				proxy_buffer);
			RDEBUG("Freeradius-Proxied-To = %s",
				proxy_buffer);
		}
#endif

		fprintf(fp, "\tTimestamp = %ld\n",
			(unsigned long) request->timestamp);
	}

	fprintf(fp, "\n");

	/*
	 *	If we can't flush it to disk, truncate the file and
	 *	return an error.
	 */
	if (fflush(fp) != 0) {
		int ret;
		ret = ftruncate(outfd, fsize);
		if (ret) {
			REDEBUG4("Failed truncating detail file: %s", strerror(ret));
		}

		fclose(fp);
		return RLM_MODULE_FAIL;
	}

	fclose(fp);

	/*
	 *	And everything is fine.
	 */
	return RLM_MODULE_OK;
}

/*
 *	Accounting - write the detail files.
 */
static rlm_rcode_t mod_accounting(void *instance, REQUEST *request)
{
#ifdef WITH_DETAIL
	if (request->listener->type == RAD_LISTEN_DETAIL &&
	    strcmp(((struct detail_instance *)instance)->filename,
		   ((listen_detail_t *)request->listener->data)->filename) == 0) {
		RDEBUG("Suppressing writes to detail file as the request was just read from a detail file.");
		return RLM_MODULE_NOOP;
	}
#endif

	return do_detail(instance,request,request->packet, true);
}

/*
 *	Incoming Access Request - write the detail files.
 */
static rlm_rcode_t mod_authorize(void *instance, REQUEST *request)
{
	return do_detail(instance,request,request->packet, false);
}

/*
 *	Outgoing Access-Request Reply - write the detail files.
 */
static rlm_rcode_t mod_post_auth(void *instance, REQUEST *request)
{
	return do_detail(instance,request,request->reply, false);
}

#ifdef WITH_COA
/*
 *	Incoming CoA - write the detail files.
 */
static rlm_rcode_t mod_recv_coa(void *instance, REQUEST *request)
{
	return do_detail(instance,request,request->packet, false);
}

/*
 *	Outgoing CoA - write the detail files.
 */
static rlm_rcode_t mod_send_coa(void *instance, REQUEST *request)
{
	return do_detail(instance,request,request->reply, false);
}
#endif

/*
 *	Outgoing Access-Request to home server - write the detail files.
 */
#ifdef WITH_PROXY
static rlm_rcode_t mod_pre_proxy(void *instance, REQUEST *request)
{
	if (request->proxy &&
	    request->proxy->vps) {
		return do_detail(instance,request,request->proxy, false);
	}

	return RLM_MODULE_NOOP;
}


/*
 *	Outgoing Access-Request Reply - write the detail files.
 */
static rlm_rcode_t mod_post_proxy(void *instance, REQUEST *request)
{
	if (request->proxy_reply &&
	    request->proxy_reply->vps) {
		return do_detail(instance,request,request->proxy_reply, false);
	}

	/*
	 *	No reply: we must be doing Post-Proxy-Type = Fail.
	 *
	 *	Note that we just call the normal accounting function,
	 *	to minimize the amount of code, and to highlight that
	 *	it's doing normal accounting.
	 */
	if (!request->proxy_reply) {
		rlm_rcode_t rcode;

		rcode = mod_accounting(instance, request);
		if (rcode == RLM_MODULE_OK) {
			request->reply->code = PW_ACCOUNTING_RESPONSE;
		}
		return rcode;
	}

	return RLM_MODULE_NOOP;
}
#endif


/* globally exported name */
module_t rlm_detail = {
	RLM_MODULE_INIT,
	"detail",
	RLM_TYPE_THREAD_UNSAFE | RLM_TYPE_CHECK_CONFIG_SAFE | RLM_TYPE_HUP_SAFE,
	sizeof(detail_instance_t),
	module_config,
	mod_instantiate,		/* instantiation */
	mod_detach,			/* detach */
	{
		NULL,			/* authentication */
		mod_authorize, 	/* authorization */
		NULL,			/* preaccounting */
		mod_accounting,	/* accounting */
		NULL,			/* checksimul */
#ifdef WITH_PROXY
		mod_pre_proxy,      	/* pre-proxy */
		mod_post_proxy,	/* post-proxy */
#else
		NULL, NULL,
#endif
		mod_post_auth		/* post-auth */
#ifdef WITH_COA
		, mod_recv_coa,
		mod_send_coa
#endif
	},
};

