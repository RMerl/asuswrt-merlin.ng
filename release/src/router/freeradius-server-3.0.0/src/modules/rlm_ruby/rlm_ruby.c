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
 * @file rlm_ruby.c
 * @brief Translates requests between the server an a ruby interpreter.
 *
 * @note Maintainers note
 * @note Please don't use this module, Matz ruby was never designed for embedding.
 * @note This module leaks memory, and the ruby code installs signal handlers
 * @note which interfere with normal operation of the server. It's all bad...
 * @note mruby shows some promise, feel free to rewrite the module to use that.
 * @note https://github.com/mruby/mruby
 *
 * @copyright 2008 Andriy Dmytrenko aka Antti, BuzhNET
 */


RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>

/*
 *	Undefine any HAVE_* flags which may conflict
 *	ruby.h *REALLY* shouldn't #include its config.h file,
 *	but it does *sigh*.
 */
#undef HAVE_CRYPT

#include <ruby.h>

/*
 *	Define a structure for our module configuration.
 *
 *	These variables do not need to be in a structure, but it's
 *	a lot cleaner to do so, and a pointer to the structure can
 *	be used as the instance handle.
 */
typedef struct rlm_ruby_t {
#define RLM_RUBY_STRUCT(foo) unsigned long func_##foo

	RLM_RUBY_STRUCT(instantiate);
	RLM_RUBY_STRUCT(authorize);
	RLM_RUBY_STRUCT(authenticate);
	RLM_RUBY_STRUCT(preacct);
	RLM_RUBY_STRUCT(accounting);
	RLM_RUBY_STRUCT(checksimul);
	RLM_RUBY_STRUCT(pre_proxy);
	RLM_RUBY_STRUCT(post_proxy);
	RLM_RUBY_STRUCT(post_auth);
#ifdef WITH_COA
	RLM_RUBY_STRUCT(recv_coa);
	RLM_RUBY_STRUCT(send_coa);
#endif
	RLM_RUBY_STRUCT(detach);

	char *filename;
	char *module_name;
	VALUE module;

} rlm_ruby_t;

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
	{ "filename", PW_TYPE_FILE_INPUT | PW_TYPE_REQUIRED,
	  offsetof(struct rlm_ruby_t, filename), NULL, NULL},
	{ "module", PW_TYPE_STRING_PTR,
	  offsetof(struct rlm_ruby_t, module_name), NULL, "Radiusd"},
	{ NULL, -1, 0, NULL, NULL} /* end of module_config */
};


/*
 * radiusd Ruby functions
 */

/* radlog wrapper */

static VALUE radlog_rb(UNUSED VALUE self, VALUE msg_type, VALUE rb_msg) {
	int status;
	char *msg;
	status = FIX2INT(msg_type);
	msg = StringValuePtr(rb_msg);
	radlog(status, "%s", msg);
	return Qnil;
}

/* Tuple to value pair conversion */

static void add_vp_tuple(TALLOC_CTX *ctx, REQUEST *request, VALUE_PAIR **vpp, VALUE rb_value,
			 char const *function_name) {
	int i;
	long outertuplesize;
	VALUE_PAIR *vp;

	/* If the Ruby function gave us nil for the tuple, then just return. */
	if (NIL_P(rb_value)) {
		return;
	}

	if (TYPE(rb_value) != T_ARRAY) {
		REDEBUG("add_vp_tuple, %s: non-array passed", function_name);
		return;
	}

	/* Get the array size. */
	outertuplesize = RARRAY_LEN(rb_value);

	for (i = 0; i < outertuplesize; i++) {
		VALUE pTupleElement = rb_ary_entry(rb_value, i);

		if ((pTupleElement != 0) &&
		    (TYPE(pTupleElement) == T_ARRAY)) {

			/* Check if it's a pair */
			long tuplesize;

			if ((tuplesize = RARRAY_LEN(pTupleElement)) != 2) {
				REDEBUG("%s: tuple element %i is a tuple "
					" of size %li. must be 2\n", function_name,
					i, tuplesize);
			} else {
				VALUE pString1, pString2;

				pString1 = rb_ary_entry(pTupleElement, 0);
				pString2 = rb_ary_entry(pTupleElement, 1);

				if ((TYPE(pString1) == T_STRING) &&
				    (TYPE(pString2) == T_STRING)) {


					char const *s1, *s2;

					/* pairmake() will convert and find any
					 * errors in the pair.
					 */

					s1 = StringValuePtr(pString1);
					s2 = StringValuePtr(pString2);

					if ((s1 != NULL) && (s2 != NULL)) {
						DEBUG("%s: %s = %s ",
						       function_name, s1, s2);

						/* xxx Might need to support other T_OP */
						vp = pairmake(ctx, vpp, s1, s2, T_OP_EQ);
						if (vp != NULL) {
							DEBUG("%s: s1, s2 OK", function_name);
						} else {
							DEBUG("%s: s1, s2 FAILED", function_name);
						}
					} else {
						REDEBUG("%s: string conv failed", function_name);
					}

				} else {
					REDEBUG("%s: tuple element %d must be "
						"(string, string)", function_name, i);
				}
			}
		} else {
			REDEBUG("%s: tuple element %d is not a tuple\n",
				function_name, i);
		}
	}

}

/* This is the core Ruby function that the others wrap around.
 * Pass the value-pair print strings in a tuple.
 * xxx We're not checking the errors. If we have errors, what do we do?
 */

#define BUF_SIZE 1024
static rlm_rcode_t do_ruby(REQUEST *request, unsigned long func,
			   VALUE module, char const *function_name) {
	rlm_rcode_t rcode = RLM_MODULE_OK;
	vp_cursor_t cursor;

	char buf[BUF_SIZE]; /* same size as vp_print buffer */

	VALUE_PAIR *vp;
	VALUE rb_request, rb_result, rb_reply_items, rb_config_items, rbString1, rbString2;

	int n_tuple;
	DEBUG("Calling ruby function %s which has id: %lu\n", function_name, func);

	/* Return with "OK, continue" if the function is not defined.
	 * TODO: Should check with rb_respond_to each time, just because ruby can define function dynamicly?
	 */
	if (func == 0) {
		return rcode;
	}

	n_tuple = 0;

	if (request) {
		for (vp = paircursor(&cursor, &request->packet->vps);
		     vp;
		     vp = pairnext(&cursor)) {
		 	 n_tuple++;
		}
	}


	/*
	  Creating ruby array, that contains arrays of [name,value]
	  Maybe we should use hash instead? Can this names repeat?
	*/
	rb_request = rb_ary_new2(n_tuple);
	if (request) {
		for (vp = paircursor(&cursor, &request->packet->vps);
		     vp;
		     vp = pairnext(&cursor)) {
			VALUE tmp = rb_ary_new2(2);

			/* The name. logic from vp_prints, lib/print.c */
			if (vp->da->flags.has_tag) {
				snprintf(buf, BUF_SIZE, "%s:%d", vp->da->name, vp->tag);
			} else {
				strlcpy(buf, vp->da->name, sizeof(buf));
			}
			rbString1 = rb_str_new2(buf);
			vp_prints_value(buf, sizeof (buf), vp, '"');
			rbString2 = rb_str_new2(buf);

			rb_ary_push(tmp, rbString1);
			rb_ary_push(tmp, rbString2);
			rb_ary_push(rb_request, tmp);
		}
	}

	/* Calling corresponding ruby function, passing request and catching result */
	rb_result = rb_funcall(module, func, 1, rb_request);

	/*
	 *	Checking result, it can be array of type [result,
	 *	[array of reply pairs],[array of config pairs]],
	 *	It can also be just a fixnum, which is a result itself.
	 */
	if (TYPE(rb_result) == T_ARRAY) {
		if (!FIXNUM_P(rb_ary_entry(rb_result, 0))) {
			REDEBUG("First element of an array was not a "
				"FIXNUM (Which has to be a return_value)");

			rcode = RLM_MODULE_FAIL;
			goto finish;
		}

		rcode = FIX2INT(rb_ary_entry(rb_result, 0));

		/*
		 *	Only process the results if we were passed a request.
		 */
		if (request) {
			rb_reply_items = rb_ary_entry(rb_result, 1);
			rb_config_items = rb_ary_entry(rb_result, 2);

			add_vp_tuple(request->reply, request, &request->reply->vps,
				     rb_reply_items, function_name);
			add_vp_tuple(request, request, &request->config_items,
				     rb_config_items, function_name);
		}
	} else if (FIXNUM_P(rb_result)) {
		rcode = FIX2INT(rb_result);
	}

finish:
	return rcode;
}

static struct varlookup {
	char const* name;
	int value;
} constants[] = {
	{ "L_DBG", L_DBG},
	{ "L_AUTH", L_AUTH},
	{ "L_INFO", L_INFO},
	{ "L_ERR", L_ERR},
	{ "L_PROXY", L_PROXY},
	{ "RLM_MODULE_REJECT", RLM_MODULE_REJECT},
	{ "RLM_MODULE_FAIL", RLM_MODULE_FAIL},
	{ "RLM_MODULE_OK", RLM_MODULE_OK},
	{ "RLM_MODULE_HANDLED", RLM_MODULE_HANDLED},
	{ "RLM_MODULE_INVALID", RLM_MODULE_INVALID},
	{ "RLM_MODULE_USERLOCK", RLM_MODULE_USERLOCK},
	{ "RLM_MODULE_NOTFOUND", RLM_MODULE_NOTFOUND},
	{ "RLM_MODULE_NOOP", RLM_MODULE_NOOP},
	{ "RLM_MODULE_UPDATED", RLM_MODULE_UPDATED},
	{ "RLM_MODULE_NUMCODES", RLM_MODULE_NUMCODES},
	{ NULL, 0},
};

/*
 * Import a user module and load a function from it
 */
static int load_function(char const *f_name, unsigned long *func, VALUE module) {
	if (!f_name) {
		*func = 0;
	} else {
		*func = rb_intern(f_name);
		/* rb_intern returns a symbol of a function, not a function itself
		   it can be aplied to any recipient,
		   so we should check it for our module recipient
		*/
		if (!rb_respond_to(module, *func))
			*func = 0;
	}
	DEBUG("load_function %s, result: %lu", f_name, *func);
	return 0;
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
static int mod_instantiate(UNUSED CONF_SECTION *conf, void *instance)
{
	rlm_ruby_t *inst = instance;
	VALUE module;

	int idx;
	int status;

	/*
	 *	Initialize Ruby interpreter. Fatal error if this fails.
	 */
	ruby_init();
	ruby_init_loadpath();
	ruby_script("radiusd");

	/* disabling GC, it will eat your memory, but at least it will be stable. */
	rb_gc_disable();

	/*
	 *	Setup our 'radiusd' module.
	 */
	module = inst->module = rb_define_module(inst->module_name);
	if (!module) {
		EDEBUG("Ruby rb_define_module failed");

		return -1;
	}

	/*
	 *	Load constants into module
	 */
	for (idx = 0; constants[idx].name; idx++) {
		rb_define_const(module, constants[idx].name, INT2NUM(constants[idx].value));
	}

	/*
	 *	Expose some FreeRADIUS API functions as ruby functions
	 */
	rb_define_module_function(module, "radlog", radlog_rb, 2);

	DEBUG("Loading file %s...", inst->filename);
	rb_load_protect(rb_str_new2(inst->filename), 0, &status);
	if (status) {
		EDEBUG("Error loading file %s status: %d", inst->filename, status);

		return -1;
	}
	DEBUG("Loaded file %s", inst->filename);

	/*
	 *	Import user modules.
	 */
#define RLM_RUBY_LOAD(foo) if (load_function(#foo, &inst->func_##foo, inst->module)==-1) { \
		return -1; \
	}

	RLM_RUBY_LOAD(instantiate);
	RLM_RUBY_LOAD(authenticate);
	RLM_RUBY_LOAD(authorize);
	RLM_RUBY_LOAD(preacct);
	RLM_RUBY_LOAD(accounting);
	RLM_RUBY_LOAD(checksimul);
	RLM_RUBY_LOAD(pre_proxy);
	RLM_RUBY_LOAD(post_proxy);
	RLM_RUBY_LOAD(post_auth);
#ifdef WITH_COA
	RLM_RUBY_LOAD(recv_coa);
	RLM_RUBY_LOAD(send_coa);
#endif
	RLM_RUBY_LOAD(detach);

	/* Call the instantiate function.  No request.  Use the return value. */
	return do_ruby(NULL, inst->func_instantiate, inst->module, "instantiate");
}

#define RLM_RUBY_FUNC(foo) static rlm_rcode_t mod_##foo(void *instance, REQUEST *request) \
	{ \
		return do_ruby(request,	\
			       ((struct rlm_ruby_t *)instance)->func_##foo,((struct rlm_ruby_t *)instance)->module, \
			       #foo); \
	}

RLM_RUBY_FUNC(authorize)
RLM_RUBY_FUNC(authenticate)
RLM_RUBY_FUNC(preacct)
RLM_RUBY_FUNC(accounting)
RLM_RUBY_FUNC(checksimul)
RLM_RUBY_FUNC(pre_proxy)
RLM_RUBY_FUNC(post_proxy)
RLM_RUBY_FUNC(post_auth)
#ifdef WITH_COA
RLM_RUBY_FUNC(recv_coa)
RLM_RUBY_FUNC(send_coa)
#endif

static int mod_detach(UNUSED void *instance)
{
	ruby_finalize();
	ruby_cleanup(0);

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
module_t rlm_ruby = {
	RLM_MODULE_INIT,
	"ruby",
	RLM_TYPE_THREAD_UNSAFE, /* type, ok, let's be honest, MRI is not yet treadsafe */
	sizeof(rlm_ruby_t),
	module_config,
	mod_instantiate,		/* instantiation */
	mod_detach,			/* detach */
	{
		mod_authenticate,	/* authentication */
		mod_authorize,		/* authorization */
		mod_preacct,		/* preaccounting */
		mod_accounting,		/* accounting */
		mod_checksimul,		/* checksimul */
		mod_pre_proxy,		/* pre-proxy */
		mod_post_proxy,		/* post-proxy */
		mod_post_auth		/* post-auth */
#ifdef WITH_COA
		, mod_recv_coa,
		mod_send_coa
#endif
	},
};
