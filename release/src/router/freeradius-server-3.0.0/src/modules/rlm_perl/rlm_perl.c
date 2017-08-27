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
 * @file rlm_perl.c
 * @brief Translates requests between the server an a perl interpreter.
 *
 * @copyright 2002,2006  The FreeRADIUS server project
 * @copyright 2002  Boian Jordanov <bjordanov@orbitel.bg>
 */
RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>
#include <freeradius-devel/rad_assert.h>

#ifdef INADDR_ANY
#undef INADDR_ANY
#endif
#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>
#include <dlfcn.h>
#include <semaphore.h>

#ifdef __APPLE__
extern char **environ;
#endif

/*
 *	Define a structure for our module configuration.
 *
 *	These variables do not need to be in a structure, but it's
 *	a lot cleaner to do so, and a pointer to the structure can
 *	be used as the instance handle.
 */
typedef struct rlm_perl_t {
	/* Name of the perl module */
	char	*module;

	/* Name of the functions for each module method */
	char	*func_authorize;
	char	*func_authenticate;
	char	*func_accounting;
	char	*func_start_accounting;
	char	*func_stop_accounting;
	char	*func_preacct;
	char	*func_checksimul;
	char	*func_detach;
	char	*func_xlat;
#ifdef WITH_PROXY
	char	*func_pre_proxy;
	char	*func_post_proxy;
#endif
	char	*func_post_auth;
#ifdef WITH_COA
	char	*func_recv_coa;
	char	*func_send_coa;
#endif
	char	*xlat_name;
	char	*perl_flags;
	PerlInterpreter *perl;
	pthread_key_t	*thread_key;

	pthread_mutex_t clone_mutex;
} rlm_perl_t;
/*
 *	A mapping of configuration file names to internal variables.
 */
#define RLM_PERL_CONF(_x) { "func_" STRINGIFY(_x), PW_TYPE_STRING_PTR, \
			offsetof(rlm_perl_t,func_##_x), NULL, STRINGIFY(_x)}

static const CONF_PARSER module_config[] = {
	{ "module",  PW_TYPE_FILE_INPUT | PW_TYPE_DEPRECATED,
	  offsetof(rlm_perl_t,module), NULL,  NULL},
	{ "filename",  PW_TYPE_FILE_INPUT | PW_TYPE_REQUIRED,
	  offsetof(rlm_perl_t,module), NULL,  NULL},

	RLM_PERL_CONF(authorize),
	RLM_PERL_CONF(authenticate),
	RLM_PERL_CONF(post_auth),
	RLM_PERL_CONF(accounting),
	RLM_PERL_CONF(preacct),
	RLM_PERL_CONF(checksimul),
	RLM_PERL_CONF(detach),
	RLM_PERL_CONF(xlat),

#ifdef WITH_PROXY
	RLM_PERL_CONF(pre_proxy),
	RLM_PERL_CONF(post_proxy),
#endif
#ifdef WITH_COA
	RLM_PERL_CONF(recv_coa),
	RLM_PERL_CONF(send_coa),
#endif
	{ "perl_flags", PW_TYPE_STRING_PTR,
	  offsetof(rlm_perl_t,perl_flags), NULL, NULL},

	{ "func_start_accounting", PW_TYPE_STRING_PTR,
	  offsetof(rlm_perl_t,func_start_accounting), NULL, NULL},

	{ "func_stop_accounting", PW_TYPE_STRING_PTR,
	  offsetof(rlm_perl_t,func_stop_accounting), NULL, NULL},

	{ NULL, -1, 0, NULL, NULL }		/* end the list */
};

/*
 * man perlembed
 */
EXTERN_C void boot_DynaLoader(pTHX_ CV* cv);

#ifdef USE_ITHREADS
#define dl_librefs "DynaLoader::dl_librefs"
#define dl_modules "DynaLoader::dl_modules"
static void rlm_perl_clear_handles(pTHX)
{
	AV *librefs = get_av(dl_librefs, false);
	if (librefs) {
		av_clear(librefs);
	}
}

static void **rlm_perl_get_handles(pTHX)
{
	I32 i;
	AV *librefs = get_av(dl_librefs, false);
	AV *modules = get_av(dl_modules, false);
	void **handles;

	if (!librefs) return NULL;

	if (!(AvFILL(librefs) >= 0)) {
		return NULL;
	}

	handles = (void **)rad_malloc(sizeof(void *) * (AvFILL(librefs)+2));

	for (i=0; i<=AvFILL(librefs); i++) {
		void *handle;
		SV *handle_sv = *av_fetch(librefs, i, false);

		if(!handle_sv) {
			ERROR("Could not fetch $%s[%d]!\n",
			       dl_librefs, (int)i);
			continue;
		}
		handle = (void *)SvIV(handle_sv);

		if (handle) {
			handles[i] = handle;
		}
	}

	av_clear(modules);
	av_clear(librefs);

	handles[i] = (void *)0;

	return handles;
}

static void rlm_perl_close_handles(void **handles)
{
	int i;

	if (!handles) {
		return;
	}

	for (i=0; handles[i]; i++) {
		DEBUG("close %p\n", handles[i]);
		dlclose(handles[i]);
	}

	free(handles);
}

DIAG_OFF(shadow)
static void rlm_perl_destruct(PerlInterpreter *perl)
{
	dTHXa(perl);

	PERL_SET_CONTEXT(perl);

	PL_perl_destruct_level = 2;

	PL_origenviron = environ;


	{
		dTHXa(perl);
	}
	/*
	 * FIXME: This shouldn't happen
	 *
	 */
	while (PL_scopestack_ix > 1 ){
		LEAVE;
	}

	perl_destruct(perl);
	perl_free(perl);
}
DIAG_ON(shadow)

static void rlm_destroy_perl(PerlInterpreter *perl)
{
	void	**handles;

	dTHXa(perl);
	PERL_SET_CONTEXT(perl);

	handles = rlm_perl_get_handles(aTHX);
	if (handles) rlm_perl_close_handles(handles);
	rlm_perl_destruct(perl);
}

/* Create Key */
static void rlm_perl_make_key(pthread_key_t *key)
{
	pthread_key_create(key, (void*)rlm_destroy_perl);
}

static PerlInterpreter *rlm_perl_clone(PerlInterpreter *perl, pthread_key_t *key)
{
	int ret;

	PerlInterpreter *interp;
	UV clone_flags = 0;

	PERL_SET_CONTEXT(perl);

	interp = pthread_getspecific(*key);
	if (interp) return interp;

	interp = perl_clone(perl, clone_flags);
	{
		dTHXa(interp);
	}
#if PERL_REVISION >= 5 && PERL_VERSION <8
	call_pv("CLONE",0);
#endif
	ptr_table_free(PL_ptr_table);
	PL_ptr_table = NULL;

	PERL_SET_CONTEXT(aTHX);
    	rlm_perl_clear_handles(aTHX);

	ret = pthread_setspecific(*key, interp);
	if (ret != 0) {
		DEBUG("rlm_perl: Failed associating interpretor with thread %s", strerror(ret));

		rlm_perl_destruct(interp);
		return NULL;
	}

	return interp;
}
#endif

/*
 *
 * This is wrapper for radlog
 * Now users can call radiusd::radlog(level,msg) wich is the same
 * calling radlog from C code.
 * Boyan
 */
static XS(XS_radiusd_radlog)
{
	dXSARGS;
	if (items !=2)
		croak("Usage: radiusd::radlog(level, message)");
	{
		int     level;
		char    *msg;

		level = (int) SvIV(ST(0));
		msg   = (char *) SvPV(ST(1), PL_na);

		/*
		 *	Because 'msg' is a 'char *', we don't want '%s', etc.
		 *	in it to give us printf-style vulnerabilities.
		 */
		radlog(level, "rlm_perl: %s", msg);
	}
	XSRETURN_NO;
}

static void xs_init(pTHX)
{
	char const *file = __FILE__;

	/* DynaLoader is a special case */
	newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);

	newXS("radiusd::radlog",XS_radiusd_radlog, "rlm_perl");
}

/*
 * The xlat function
 */
static ssize_t perl_xlat(void *instance, REQUEST *request, char const *fmt, char *out, size_t freespace)
{

	rlm_perl_t	*inst= (rlm_perl_t *) instance;
	char		*tmp;
	char const	*p, *q;
	int		count;
	size_t		ret = 0;
	STRLEN		n_a;

#ifdef USE_ITHREADS
	PerlInterpreter *interp;

	pthread_mutex_lock(&inst->clone_mutex);
	interp = rlm_perl_clone(inst->perl, inst->thread_key);
	{
		dTHXa(interp);
		PERL_SET_CONTEXT(interp);
	}
	pthread_mutex_unlock(&inst->clone_mutex);
#else
	PERL_SET_CONTEXT(inst->perl);
#endif
	{
		dSP;
		ENTER;SAVETMPS;

		PUSHMARK(SP);

		p = fmt;
		while ((q = strchr(p, ' '))) {
			XPUSHs(sv_2mortal(newSVpv(p, p - q)));

			p = q + 1;
		}

		PUTBACK;

		count = call_pv(inst->func_xlat, G_SCALAR | G_EVAL);

		SPAGAIN;
		if (SvTRUE(ERRSV)) {
			REDEBUG("Exit %s", SvPV(ERRSV,n_a));
			(void)POPs;
		} else if (count > 0) {
			tmp = POPp;
			strlcpy(out, tmp, freespace);
			ret = strlen(out);

			RDEBUG("Len is %zu , out is %s freespace is %zu", ret, out, freespace);
		}

		PUTBACK ;
		FREETMPS ;
		LEAVE ;

	}

	return ret;
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
 *
 *	Boyan:
 *	Setup a hashes wich we will use later
 *	parse a module and give him a chance to live
 *
 */
static int mod_instantiate(CONF_SECTION *conf, void *instance)
{
	rlm_perl_t       *inst = instance;
	AV		*end_AV;

	char **embed;
	char **envp = NULL;
	char const *xlat_name;
	int exitstatus = 0, argc=0;

	MEM(embed = talloc_zero_array(inst, char *, 4));

	/*
	 *	Create pthread key. This key will be stored in instance
	 */

#ifdef USE_ITHREADS
	pthread_mutex_init(&inst->clone_mutex, NULL);

	inst->thread_key = rad_malloc(sizeof(*inst->thread_key));
	memset(inst->thread_key,0,sizeof(*inst->thread_key));

	rlm_perl_make_key(inst->thread_key);
#endif

	char arg[] = "0";

	embed[0] = NULL;
	if (inst->perl_flags) {
		embed[1] = inst->perl_flags;
		embed[2] = inst->module;
		embed[3] = arg;
		argc = 4;
	} else {
		embed[1] = inst->module;
		embed[2] = arg;
		argc = 3;
	}

	PERL_SYS_INIT3(&argc, &embed, &envp);

	if ((inst->perl = perl_alloc()) == NULL) {
		ERROR("rlm_perl: No memory for allocating new perl !");
		return (-1);
	}

	perl_construct(inst->perl);

#ifdef USE_ITHREADS
	PL_perl_destruct_level = 2;

	{
		dTHXa(inst->perl);
	}
	PERL_SET_CONTEXT(inst->perl);
#endif

#if PERL_REVISION >= 5 && PERL_VERSION >=8
	PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
#endif

	exitstatus = perl_parse(inst->perl, xs_init, argc, embed, NULL);

	end_AV = PL_endav;
	PL_endav = Nullav;

	if(!exitstatus) {
		exitstatus = perl_run(inst->perl);
	} else {
		ERROR("rlm_perl: perl_parse failed: %s not found or has syntax errors. \n", inst->module);
		return (-1);
	}

	PL_endav = end_AV;

	xlat_name = cf_section_name2(conf);
	if (!xlat_name)
		xlat_name = cf_section_name1(conf);
	if (xlat_name) {
		xlat_register(xlat_name, perl_xlat, NULL, inst);
	}

	return 0;
}

/*
 *  	get the vps and put them in perl hash
 *  	If one VP have multiple values it is added as array_ref
 *  	Example for this is Cisco-AVPair that holds multiple values.
 *  	Which will be available as array_ref in $RAD_REQUEST{'Cisco-AVPair'}
 */
static void perl_store_vps(TALLOC_CTX *ctx, VALUE_PAIR *vps, HV *rad_hv)
{
	VALUE_PAIR *head, *sublist;
	AV *av;
	char const *name;
	char namebuf[256];
	char buffer[1024];
	int len;

	hv_undef(rad_hv);

	/*
	 *	Copy the valuepair list so we can remove attributes
	 *	we've already processed.  This is a horrible hack to
	 *	get around various other stupidity.
	 */
	head = paircopy(ctx, vps);

	while (head) {
		vp_cursor_t cursor;
		/*
		 *	Tagged attributes are added to the hash with name
		 *	<attribute>:<tag>, others just use the normal attribute
		 *	name as the key.
		 */
		if (head->da->flags.has_tag && (head->tag != 0)) {
			snprintf(namebuf, sizeof(namebuf), "%s:%d",
				 head->da->name, head->tag);
			name = namebuf;
		} else {
			name = head->da->name;
		}

		/*
		 *	Create a new list with all the attributes like this one
		 *	which are in the same tag group.
		 */
		sublist = NULL;
		pairfilter(ctx, &sublist, &head, head->da->attr, head->da->vendor, head->tag);

		paircursor(&cursor, &sublist);
		/*
		 *	Attribute has multiple values
		 */
		if (pairnext(&cursor)) {
			VALUE_PAIR *vp;

			av = newAV();
			for (vp = pairfirst(&cursor);
			     vp;
			     vp = pairnext(&cursor)) {
				len = vp_prints_value(buffer, sizeof(buffer), vp, 0);
				av_push(av, newSVpv(buffer, len));
			}
			(void)hv_store(rad_hv, name, strlen(name), newRV_noinc((SV *)av), 0);

			/*
			 *	Attribute has a single value, so its value just gets
			 *	added to the hash.
			 */
		} else {
			len = vp_prints_value(buffer, sizeof(buffer), sublist, 0);
			(void)hv_store(rad_hv, name, strlen(name), newSVpv(buffer, len), 0);
		}

		pairfree(&sublist);
	}

	rad_assert(!head);
}

/*
 *
 *     Verify that a Perl SV is a string and save it in FreeRadius
 *     Value Pair Format
 *
 */
static int pairadd_sv(TALLOC_CTX *ctx, VALUE_PAIR **vps, char *key, SV *sv, FR_TOKEN op)
{
	char	    *val;
	VALUE_PAIR      *vp;

	if (SvOK(sv)) {
		val = SvPV_nolen(sv);
		vp = pairmake(ctx, vps, key, val, op);
		if (vp != NULL) {
			DEBUG("rlm_perl: Added pair %s = %s", key, val);
			return 1;
		} else {
			EDEBUG("rlm_perl: Failed to create pair %s = %s", key, val);
		}
	}
	return 0;
}

/*
 *     Boyan :
 *     Gets the content from hashes
 */
static int get_hv_content(TALLOC_CTX *ctx, HV *my_hv, VALUE_PAIR **vps)
{
	SV		*res_sv, **av_sv;
	AV		*av;
	char		*key;
	I32		key_len, len, i, j;
	int		ret=0;

	*vps = NULL;
	for (i = hv_iterinit(my_hv); i > 0; i--) {
		res_sv = hv_iternextsv(my_hv,&key,&key_len);
		if (SvROK(res_sv) && (SvTYPE(SvRV(res_sv)) == SVt_PVAV)) {
			av = (AV*)SvRV(res_sv);
			len = av_len(av);
			for (j = 0; j <= len; j++) {
				av_sv = av_fetch(av, j, 0);
				ret = pairadd_sv(ctx, vps, key, *av_sv, T_OP_ADD) + ret;
			}
		} else ret = pairadd_sv(ctx, vps, key, res_sv, T_OP_EQ) + ret;
	}

	return ret;
}

/*
 * 	Call the function_name inside the module
 * 	Store all vps in hashes %RAD_CHECK %RAD_REPLY %RAD_REQUEST
 *
 */
static int do_perl(void *instance, REQUEST *request, char *function_name)
{

	rlm_perl_t	*inst = instance;
	VALUE_PAIR	*vp;
	int		exitstatus=0, count;
	STRLEN		n_a;

	HV		*rad_reply_hv;
	HV		*rad_check_hv;
	HV		*rad_config_hv;
	HV		*rad_request_hv;
#ifdef WITH_PROXY
	HV		*rad_request_proxy_hv;
	HV		*rad_request_proxy_reply_hv;
#endif

	/*
	 *	Radius has told us to call this function, but none
	 *	is defined.
	 */
	if (!function_name) return RLM_MODULE_FAIL;

#ifdef USE_ITHREADS
	pthread_mutex_lock(&inst->clone_mutex);

	PerlInterpreter *interp;

	interp = rlm_perl_clone(inst->perl,inst->thread_key);
	{
		dTHXa(interp);
		PERL_SET_CONTEXT(interp);
	}

	pthread_mutex_unlock(&inst->clone_mutex);
#else
	PERL_SET_CONTEXT(inst->perl);
#endif

	{
		dSP;

		ENTER;
		SAVETMPS;

		rad_reply_hv = get_hv("RAD_REPLY",1);
		rad_check_hv = get_hv("RAD_CHECK",1);
		rad_config_hv = get_hv("RAD_CONFIG",1);
		rad_request_hv = get_hv("RAD_REQUEST",1);

		perl_store_vps(request->reply, request->reply->vps, rad_reply_hv);
		perl_store_vps(request, request->config_items, rad_check_hv);
		perl_store_vps(request->packet, request->packet->vps, rad_request_hv);
		perl_store_vps(request, request->config_items, rad_config_hv);

#ifdef WITH_PROXY
		rad_request_proxy_hv = get_hv("RAD_REQUEST_PROXY",1);
		rad_request_proxy_reply_hv = get_hv("RAD_REQUEST_PROXY_REPLY",1);

		if (request->proxy != NULL) {
			perl_store_vps(request->proxy, request->proxy->vps, rad_request_proxy_hv);
		} else {
			hv_undef(rad_request_proxy_hv);
		}

		if (request->proxy_reply !=NULL) {
			perl_store_vps(request->proxy_reply, request->proxy_reply->vps, rad_request_proxy_reply_hv);
		} else {
			hv_undef(rad_request_proxy_reply_hv);
		}
#endif

		PUSHMARK(SP);
		/*
		 * This way %RAD_xx can be pushed onto stack as sub parameters.
		 * XPUSHs( newRV_noinc((SV *)rad_request_hv) );
		 * XPUSHs( newRV_noinc((SV *)rad_reply_hv) );
		 * XPUSHs( newRV_noinc((SV *)rad_check_hv) );
		 * PUTBACK;
		 */

		count = call_pv(function_name, G_SCALAR | G_EVAL | G_NOARGS);

		SPAGAIN;

		if (SvTRUE(ERRSV)) {
			ERROR("rlm_perl: perl_embed:: module = %s , func = %s exit status= %s\n",
			       inst->module,
			       function_name, SvPV(ERRSV,n_a));
			(void)POPs;
		}

		if (count == 1) {
			exitstatus = POPi;
			if (exitstatus >= 100 || exitstatus < 0) {
				exitstatus = RLM_MODULE_FAIL;
			}
		}


		PUTBACK;
		FREETMPS;
		LEAVE;

		vp = NULL;
		if ((get_hv_content(request->packet, rad_request_hv, &vp)) > 0 ) {
			pairfree(&request->packet->vps);
			request->packet->vps = vp;
			vp = NULL;

			/*
			 *	Update cached copies
			 */
			request->username = pairfind(request->packet->vps, PW_USER_NAME, 0, TAG_ANY);
			request->password = pairfind(request->packet->vps, PW_USER_PASSWORD, 0, TAG_ANY);
			if (!request->password)
				request->password = pairfind(request->packet->vps, PW_CHAP_PASSWORD, 0, TAG_ANY);
		}

		if ((get_hv_content(request->reply, rad_reply_hv, &vp)) > 0 ) {
			pairfree(&request->reply->vps);
			request->reply->vps = vp;
			vp = NULL;
		}

		if ((get_hv_content(request, rad_check_hv, &vp)) > 0 ) {
			pairfree(&request->config_items);
			request->config_items = vp;
			vp = NULL;
		}

#ifdef WITH_PROXY
		if (request->proxy &&
		    (get_hv_content(request->proxy, rad_request_proxy_hv, &vp) > 0)) {
			pairfree(&request->proxy->vps);
			request->proxy->vps = vp;
			vp = NULL;
		}

		if (request->proxy_reply &&
		    (get_hv_content(request->proxy_reply, rad_request_proxy_reply_hv, &vp) > 0)) {
			pairfree(&request->proxy_reply->vps);
			request->proxy_reply->vps = vp;
			vp = NULL;
		}
#endif

	}
	return exitstatus;
}

#define RLM_PERL_FUNC(_x) static rlm_rcode_t mod_##_x(void *instance, REQUEST *request) \
	{								\
		return do_perl(instance, request,			\
			       ((rlm_perl_t *)instance)->func_##_x); \
	}

RLM_PERL_FUNC(authorize)
RLM_PERL_FUNC(authenticate)
RLM_PERL_FUNC(post_auth)

RLM_PERL_FUNC(checksimul)

#ifdef WITH_PROXY
RLM_PERL_FUNC(pre_proxy)
RLM_PERL_FUNC(post_proxy)
#endif

#ifdef WITH_COA
RLM_PERL_FUNC(recv_coa)
RLM_PERL_FUNC(send_coa)
#endif

RLM_PERL_FUNC(preacct)

/*
 *	Write accounting information to this modules database.
 */
static rlm_rcode_t mod_accounting(void *instance, REQUEST *request)
{
	VALUE_PAIR	*pair;
	int 		acctstatustype=0;

	if ((pair = pairfind(request->packet->vps, PW_ACCT_STATUS_TYPE, 0, TAG_ANY)) != NULL) {
		acctstatustype = pair->vp_integer;
	} else {
		ERROR("Invalid Accounting Packet");
		return RLM_MODULE_INVALID;
	}

	switch (acctstatustype) {

	case PW_STATUS_START:

		if (((rlm_perl_t *)instance)->func_start_accounting) {
			return do_perl(instance, request,
				       ((rlm_perl_t *)instance)->func_start_accounting);
		} else {
			return do_perl(instance, request,
				       ((rlm_perl_t *)instance)->func_accounting);
		}
		break;

	case PW_STATUS_STOP:

		if (((rlm_perl_t *)instance)->func_stop_accounting) {
			return do_perl(instance, request,
				       ((rlm_perl_t *)instance)->func_stop_accounting);
		} else {
			return do_perl(instance, request,
				       ((rlm_perl_t *)instance)->func_accounting);
		}
		break;
	default:
		return do_perl(instance, request,
			       ((rlm_perl_t *)instance)->func_accounting);

	}
}


/*
 * Detach a instance give a chance to a module to make some internal setup ...
 */
DIAG_OFF(nested-externs)
static int mod_detach(void *instance)
{
	rlm_perl_t	*inst = (rlm_perl_t *) instance;
	int 		exitstatus = 0, count = 0;

#if 0
	/*
	 *	FIXME: Call this in the destruct function?
	 */
	{
		dTHXa(handle->clone);
		PERL_SET_CONTEXT(handle->clone);
		{
			dSP; ENTER; SAVETMPS; PUSHMARK(SP);
			count = call_pv(inst->func_detach, G_SCALAR | G_EVAL );
			SPAGAIN;

			if (count == 1) {
				exitstatus = POPi;
				/*
				 * FIXME: bug in perl
				 *
				 */
				if (exitstatus >= 100 || exitstatus < 0) {
					exitstatus = RLM_MODULE_FAIL;
				}
			}
			PUTBACK;
			FREETMPS;
			LEAVE;
		}
	}
#endif

	if (inst->func_detach) {
		dTHXa(inst->perl);
		PERL_SET_CONTEXT(inst->perl);
		{
			dSP; ENTER; SAVETMPS;
			PUSHMARK(SP);

			count = call_pv(inst->func_detach, G_SCALAR | G_EVAL );
			SPAGAIN;

			if (count == 1) {
				exitstatus = POPi;
				if (exitstatus >= 100 || exitstatus < 0) {
					exitstatus = RLM_MODULE_FAIL;
				}
			}
			PUTBACK;
			FREETMPS;
			LEAVE;
		}
	}

#ifdef USE_ITHREADS
	rlm_perl_destruct(inst->perl);
	pthread_mutex_destroy(&inst->clone_mutex);
#else
	perl_destruct(inst->perl);
	perl_free(inst->perl);
#endif

	PERL_SYS_TERM();
	return exitstatus;
}
DIAG_ON(nested-externs)

/*
 *	The module name should be the only globally exported symbol.
 *	That is, everything else should be 'static'.
 *
 *	If the module needs to temporarily modify it's instantiation
 *	data, the type should be changed to RLM_TYPE_THREAD_UNSAFE.
 *	The server will then take care of ensuring that the module
 *	is single-threaded.
 */
module_t rlm_perl = {
	RLM_MODULE_INIT,
	"perl",				/* Name */
#ifdef USE_ITHREADS
	RLM_TYPE_THREAD_SAFE,		/* type */
#else
	RLM_TYPE_THREAD_UNSAFE,
#endif
	sizeof(rlm_perl_t),
	module_config,
	mod_instantiate,		/* instantiation */
	mod_detach,			/* detach */
	{
		mod_authenticate,	/* authenticate */
		mod_authorize,		/* authorize */
		mod_preacct,		/* preacct */
		mod_accounting,	/* accounting */
		mod_checksimul,      	/* check simul */
#ifdef WITH_PROXY
		mod_pre_proxy,		/* pre-proxy */
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
