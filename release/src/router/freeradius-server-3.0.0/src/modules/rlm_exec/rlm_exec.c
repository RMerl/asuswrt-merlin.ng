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
 * @file rlm_exec.c
 * @brief Execute commands and parse the results.
 *
 * @copyright 2002,2006  The FreeRADIUS server project
 * @copyright 2002  Alan DeKok <aland@ox.org>
 */
RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>
#include <freeradius-devel/rad_assert.h>

/*
 *	Define a structure for our module configuration.
 */
typedef struct rlm_exec_t {
	char const	*xlat_name;
	int		bare;
	bool		wait;
	char		*program;
	char		*input;
	char		*output;
	pair_lists_t	input_list;
	pair_lists_t	output_list;
	char		*packet_type;
	unsigned int	packet_code;
	int		shell_escape;
} rlm_exec_t;

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
	{ "wait", PW_TYPE_BOOLEAN, offsetof(rlm_exec_t,wait), NULL, "yes" },
	{ "program",  PW_TYPE_STRING_PTR, offsetof(rlm_exec_t,program), NULL, NULL },
	{ "input_pairs", PW_TYPE_STRING_PTR, offsetof(rlm_exec_t,input), NULL, NULL },
	{ "output_pairs",  PW_TYPE_STRING_PTR, offsetof(rlm_exec_t,output), NULL, NULL },
	{ "packet_type", PW_TYPE_STRING_PTR, offsetof(rlm_exec_t,packet_type), NULL, NULL },
	{ "shell_escape", PW_TYPE_BOOLEAN,  offsetof(rlm_exec_t,shell_escape), NULL, "yes" },

	{ NULL, -1, 0, NULL, NULL }		/* end the list */
};

static char const special[] = "\\'\"`<>|; \t\r\n()[]?#$^&*=";

/*
 *	Escape special characters
 */
static size_t rlm_exec_shell_escape(UNUSED REQUEST *request, char *out, size_t outlen, char const *in,
				    UNUSED void *inst)
{
	char *q, *end;
	char const *p;

	q = out;
	end = out + outlen;
	p = in;

	while (*p) {
		if ((q + 3) >= end) break;

		if (strchr(special, *p) != NULL) {
			*(q++) = '\\';
		}
		*(q++) = *(p++);
	}

	*q = '\0';
	return q - out;
}

/** Process the exit code returned by one of the exec functions
 *
 * @param request Current request.
 * @param answer Output string from exec call.
 * @param len length of data in answer.
 * @param status code returned by exec call.
 * @return One of the RLM_MODULE_* values.
 */
static rlm_rcode_t rlm_exec_status2rcode(REQUEST *request, char *answer, size_t len, int status)
{
	if (status < 0) {
		return RLM_MODULE_FAIL;
	}

	/*
	 *	Exec'd programs are meant to return exit statuses that correspond
	 *	to the standard RLM_MODULE_* + 1.
	 *
	 *	This frees up 0, for success where it'd normally be reject.
	 */
	if (status == 0) {
		RDEBUG("Program executed successfully");

		return RLM_MODULE_OK;
	}

	if (status > RLM_MODULE_NUMCODES) {
		REDEBUG("Program returned invalid code (greater than max rcode) (%i > %i): %s",
			status, RLM_MODULE_NUMCODES, answer);
		goto fail;

		return RLM_MODULE_FAIL;
	}

	status--;	/* Lets hope no one ever re-enumerates RLM_MODULE_* */

	if (status == RLM_MODULE_FAIL) {
		fail:

		if (len > 0) {
			char *p = &answer[len - 1];

			/*
			 *	Trim off trailing returns
			 */
			while((p > answer) && ((*p == '\r') || (*p == '\n'))) {
				*p-- = '\0';
			}

			module_failure_msg(request, "%s", answer);
		}

		return RLM_MODULE_FAIL;
	}

	return status;
}

/*
 *	Do xlat of strings.
 */
static ssize_t exec_xlat(void *instance, REQUEST *request, char const *fmt, char *out, size_t outlen)
{
	int		result;
	rlm_exec_t	*inst = instance;
	VALUE_PAIR	**input_pairs = NULL;
	char *p;

	if (!inst->wait) {
		REDEBUG("'wait' must be enabled to use exec xlat");
		*out = '\0';
		return -1;
	}

	if (inst->input_list) {
		input_pairs = radius_list(request, inst->input_list);
		if (!input_pairs) {
			REDEBUG("Failed to find input pairs for xlat");
			*out = '\0';
			return -1;
		}
	}

	/*
	 *	FIXME: Do xlat of program name?
	 */
	result = radius_exec_program(request, fmt, inst->wait, inst->shell_escape,
				     out, outlen, input_pairs ? *input_pairs : NULL, NULL);
	if (result != 0) {
		out[0] = '\0';
		return -1;
	}

	for (p = out; *p != '\0'; p++) {
		if (*p < ' ') *p = ' ';
	}

	return strlen(out);
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
	char const *p;
	rlm_exec_t	*inst = instance;

	inst->xlat_name = cf_section_name2(conf);
	if (!inst->xlat_name) {
		inst->xlat_name = cf_section_name1(conf);
		inst->bare = 1;
	}

	xlat_register(inst->xlat_name, exec_xlat, rlm_exec_shell_escape, inst);

	/*
	 *	Check whether program actually exists
	 */

	if (inst->input) {
		p = inst->input;
		inst->input_list = radius_list_name(&p, PAIR_LIST_UNKNOWN);
		if ((inst->input_list == PAIR_LIST_UNKNOWN) || (*p != '\0')) {
			cf_log_err_cs(conf, "Invalid input list '%s'", inst->input);
			return -1;
		}
	}

	if (inst->output) {
		p = inst->output;
		inst->output_list = radius_list_name(&p, PAIR_LIST_UNKNOWN);
		if ((inst->output_list == PAIR_LIST_UNKNOWN) || (*p != '\0')) {
			cf_log_err_cs(conf, "Invalid output list '%s'", inst->output);
			return -1;
		}
	}

	/*
	 *	Sanity check the config.  If we're told to NOT wait,
	 *	then the output pairs must not be defined.
	 */
	if (!inst->wait &&
	    (inst->output != NULL)) {
		cf_log_err_cs(conf, "Cannot read output pairs if wait = no");
		return -1;
	}

	/*
	 *	Get the packet type on which to execute
	 */
	if (!inst->packet_type) {
		inst->packet_code = 0;
	} else {
		DICT_VALUE	*dval;

		dval = dict_valbyname(PW_PACKET_TYPE, 0, inst->packet_type);
		if (!dval) {
			cf_log_err_cs(conf, "Unknown packet type %s: See list of VALUEs for Packet-Type in "
				      "share/dictionary", inst->packet_type);
			return -1;
		}
		inst->packet_code = dval->value;
	}

	return 0;
}


/*
 *  Dispatch an exec method
 */
static rlm_rcode_t exec_dispatch(void *instance, REQUEST *request)
{
	rlm_exec_t	*inst = (rlm_exec_t *)instance;
	rlm_rcode_t	rcode;
	int		status;

	VALUE_PAIR	**input_pairs = NULL, **output_pairs = NULL;
	VALUE_PAIR	*answer = NULL;
	char		out[1024];

	/*
	 *	We need a program to execute.
	 */
	if (!inst->program) {
		ERROR("rlm_exec (%s): We require a program to execute", inst->xlat_name);
		return RLM_MODULE_FAIL;
	}

	/*
	 *	See if we're supposed to execute it now.
	 */
	if (!((inst->packet_code == 0) || (request->packet->code == inst->packet_code) ||
	      (request->reply->code == inst->packet_code)
#ifdef WITH_PROXY
	      || (request->proxy && (request->proxy->code == inst->packet_code)) ||
	      (request->proxy_reply && (request->proxy_reply->code == inst->packet_code))
#endif
		    )) {
		RDEBUG2("Packet type is not %s. Not executing.", inst->packet_type);

		return RLM_MODULE_NOOP;
	}

	/*
	 *	Decide what input/output the program takes.
	 */
	if (inst->input) {
		input_pairs = radius_list(request, inst->input_list);
		if (!input_pairs) {
			return RLM_MODULE_INVALID;
		}
	}

	if (inst->output) {
		output_pairs = radius_list(request, inst->output_list);
		if (!output_pairs) {
			return RLM_MODULE_INVALID;
		}
	}

	/*
	 *	This function does it's own xlat of the input program
	 *	to execute.
	 *
	 *	FIXME: if inst->program starts with %{, then
	 *	do an xlat ourselves.  This will allow us to do
	 *	program = %{Exec-Program}, which this module
	 *	xlat's into it's string value, and then the
	 *	exec program function xlat's it's string value
	 *	into something else.
	 */
	status = radius_exec_program(request, inst->program, inst->wait, inst->shell_escape,
				     out, sizeof(out),
				     input_pairs ? *input_pairs : NULL, &answer);
	rcode = rlm_exec_status2rcode(request, out, strlen(out), status);

	/*
	 *	Move the answer over to the output pairs.
	 *
	 *	If we're not waiting, then there are no output pairs.
	 */
	if (output_pairs) {
		pairmove(request, output_pairs, &answer);
	}
	pairfree(&answer);

	return rcode;
}


/*
 *	First, look for Exec-Program && Exec-Program-Wait.
 *
 *	Then, call exec_dispatch.
 */
static rlm_rcode_t mod_post_auth(void *instance, REQUEST *request)
{
	rlm_exec_t	*inst = (rlm_exec_t *) instance;
	rlm_rcode_t 	rcode;
	int		status;

	char		out[1024];
	bool		we_wait = false;
	VALUE_PAIR	*vp, *tmp;

	vp = pairfind(request->reply->vps, PW_EXEC_PROGRAM, 0, TAG_ANY);
	if (vp) {
		we_wait = false;
	} else if ((vp = pairfind(request->reply->vps, PW_EXEC_PROGRAM_WAIT, 0, TAG_ANY)) != NULL) {
		we_wait = true;
	}
	if (!vp) {
		if (!inst->program) {
			return RLM_MODULE_NOOP;
		}

		rcode = exec_dispatch(instance, request);
		goto finish;
	}

	tmp = NULL;
	status = radius_exec_program(request, vp->vp_strvalue, we_wait, inst->shell_escape,
				     out, sizeof(out),
				     request->packet->vps, &tmp);
	rcode = rlm_exec_status2rcode(request, out, strlen(out), status);

	/*
	 *	Always add the value-pairs to the reply.
	 */
	pairmove(request->reply, &request->reply->vps, &tmp);
	pairfree(&tmp);

	finish:
	switch (rcode) {
		case RLM_MODULE_FAIL:
		case RLM_MODULE_INVALID:
		case RLM_MODULE_REJECT:
			request->reply->code = PW_AUTHENTICATION_REJECT;
			break;
		default:
			break;
	}

	return rcode;
}

/*
 *	First, look for Exec-Program && Exec-Program-Wait.
 *
 *	Then, call exec_dispatch.
 */
static  rlm_rcode_t mod_accounting(void *instance, REQUEST *request)
{
	rlm_exec_t	*inst = (rlm_exec_t *) instance;
	int		status;

	char		out[1024];
	bool 		we_wait = false;
	VALUE_PAIR	*vp;

	/*
	 *	The "bare" exec module takes care of handling
	 *	Exec-Program and Exec-Program-Wait.
	 */
	if (!inst->bare) {
		return exec_dispatch(instance, request);
	}

	vp = pairfind(request->reply->vps, PW_EXEC_PROGRAM, 0, TAG_ANY);
	if (vp) {
		we_wait = true;
	} else if ((vp = pairfind(request->reply->vps, PW_EXEC_PROGRAM_WAIT, 0, TAG_ANY)) != NULL) {
		we_wait = false;
	}
	if (!vp) {
		return RLM_MODULE_NOOP;
	}

	status = radius_exec_program(request, vp->vp_strvalue, we_wait, inst->shell_escape,
				     out, sizeof(out),
				     request->packet->vps, NULL);
	return rlm_exec_status2rcode(request, out, strlen(out), status);
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
module_t rlm_exec = {
	RLM_MODULE_INIT,
	"exec",				/* Name */
	RLM_TYPE_CHECK_CONFIG_SAFE,   	/* type */
	sizeof(rlm_exec_t),
	module_config,
	mod_instantiate,		/* instantiation */
	NULL,				/* detach */
	{
		exec_dispatch,		/* authentication */
		exec_dispatch,		/* authorization */
		exec_dispatch,		/* pre-accounting */
		mod_accounting,		/* accounting */
		NULL,			/* check simul */
		exec_dispatch,		/* pre-proxy */
		exec_dispatch,		/* post-proxy */
		mod_post_auth		/* post-auth */
#ifdef WITH_COA
		, exec_dispatch,
		exec_dispatch
#endif
	},
};
