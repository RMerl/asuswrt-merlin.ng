/*
 * util.c	Various utility functions.
 *
 * Version:     $Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2000,2006  The FreeRADIUS server project
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/rad_assert.h>

#include <ctype.h>
#include <signal.h>

#include <sys/stat.h>
#include <fcntl.h>

/*
 *	The signal() function in Solaris 2.5.1 sets SA_NODEFER in
 *	sa_flags, which causes grief if signal() is called in the
 *	handler before the cause of the signal has been cleared.
 *	(Infinite recursion).
 *
 *	The same problem appears on HPUX, so we avoid it, if we can.
 *
 *	Using sigaction() to reset the signal handler fixes the problem,
 *	so where available, we prefer that solution.
 */

void (*reset_signal(int signo, void (*func)(int)))(int)
{
#ifdef HAVE_SIGACTION
	struct sigaction act, oact;

	memset(&act, 0, sizeof(act));
	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
#ifdef  SA_INTERRUPT		/* SunOS */
	act.sa_flags |= SA_INTERRUPT;
#endif
	if (sigaction(signo, &act, &oact) < 0)
		return SIG_ERR;
	return oact.sa_handler;
#else

	/*
	 *	re-set by calling the 'signal' function, which
	 *	may cause infinite recursion and core dumps due to
	 *	stack growth.
	 *
	 *	However, the system is too dumb to implement sigaction(),
	 *	so we don't have a choice.
	 */
	signal(signo, func);

	return NULL;
#endif
}

/*
 *	Per-request data, added by modules...
 */
struct request_data_t {
	request_data_t	*next;

	void		*unique_ptr;
	int		unique_int;
	void		*opaque;
	bool		free_opaque;
};

/*
 *	Add opaque data (with a "free" function) to a REQUEST.
 *
 *	The unique ptr is meant to be a module configuration,
 *	and the unique integer allows the caller to have multiple
 *	opaque data associated with a REQUEST.
 */
int request_data_add(REQUEST *request,
		     void *unique_ptr, int unique_int,
		     void *opaque, bool free_opaque)
{
	request_data_t *this, **last, *next;

	/*
	 *	Some simple sanity checks.
	 */
	if (!request || !opaque) return -1;

	this = next = NULL;
	for (last = &(request->data); *last != NULL; last = &((*last)->next)) {
		if (((*last)->unique_ptr == unique_ptr) &&
		    ((*last)->unique_int == unique_int)) {
			this = *last;

			next = this->next;

			/*
			 *	If caller requires custom behaviour on free
			 *	they must set a destructor.
			 */
			if (this->opaque && this->free_opaque) {
				talloc_free(this->opaque);
			}
			break;				/* replace the existing entry */
		}
	}

	if (!this) this = talloc_zero(request, request_data_t);

	this->next = next;
	this->unique_ptr = unique_ptr;
	this->unique_int = unique_int;
	this->opaque = opaque;
	if (free_opaque) {
		this->free_opaque = free_opaque;
	}

	*last = this;

	return 0;
}


/*
 *	Get opaque data from a request.
 */
void *request_data_get(REQUEST *request,
		       void *unique_ptr, int unique_int)
{
	request_data_t **last;

	if (!request) return NULL;

	for (last = &(request->data); *last != NULL; last = &((*last)->next)) {
		if (((*last)->unique_ptr == unique_ptr) &&
		    ((*last)->unique_int == unique_int)) {
			request_data_t *this = *last;
			void *ptr = this->opaque;

			/*
			 *	Remove the entry from the list, and free it.
			 */
			*last = this->next;
			talloc_free(this);
			return ptr; 		/* don't free it, the caller does that */
		}
	}

	return NULL;		/* wasn't found, too bad... */
}


/*
 *	Get opaque data from a request without removing it.
 */
void *request_data_reference(REQUEST *request,
		       void *unique_ptr, int unique_int)
{
	request_data_t **last;

	for (last = &(request->data); *last != NULL; last = &((*last)->next)) {
		if (((*last)->unique_ptr == unique_ptr) &&
		    ((*last)->unique_int == unique_int)) {
			request_data_t *this = *last;
			void *ptr = this->opaque;

			return ptr;
		}
	}

	return NULL;		/* wasn't found, too bad... */
}


/*
 *	Free a REQUEST struct.
 */
void request_free(REQUEST **request_ptr)
{
	REQUEST *request;

	if (!request_ptr || !*request_ptr) {
		return;
	}

	request = *request_ptr;

	rad_assert(!request->in_request_hash);
#ifdef WITH_PROXY
	rad_assert(!request->in_proxy_hash);
#endif
	rad_assert(!request->ev);

#ifdef WITH_COA
	if (request->coa) {
		request->coa->parent = NULL;
		request_free(&request->coa);
	}

	if (request->parent && (request->parent->coa == request)) {
		request->parent->coa = NULL;
	}
#endif

#ifndef NDEBUG
	request->magic = 0x01020304;	/* set the request to be nonsense */
#endif
	request->client = NULL;
#ifdef WITH_PROXY
	request->home_server = NULL;
#endif
	talloc_free(request);
	*request_ptr = NULL;
}

/*
 *	Free a request.
 */
int request_opaque_free(REQUEST *request)
{
	request_free(&request);

	return 0;
}

/*
 *	Check a filename for sanity.
 *
 *	Allow only uppercase/lowercase letters, numbers, and '-_/.'
 */
int rad_checkfilename(char const *filename)
{
	if (strspn(filename, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_/.") == strlen(filename)) {
		return 0;
	}

	return -1;
}

/** Check if file exists
 *
 * @param filename to check.
 * @return 0 if the file does not exist, 1 if the file exists, -1 if the file
 *	exists but there was an error opening it. errno value should be usable
 *	for error messages.
 */
int rad_file_exists(char const *filename)
{
	int des;
	int ret = 1;

	if ((des = open(filename, O_RDONLY)) == -1) {
		if (errno == ENOENT) {
			ret = 0;
		} else {
			ret = -1;
		}
	} else {
		close(des);
	}

	return ret;
}

/*
 *	Create possibly many directories.
 *
 *	Note that the input directory name is NOT a constant!
 *	This is so that IF an error is returned, the 'directory' ptr
 *	points to the name of the file which caused the error.
 */
int rad_mkdir(char *directory, mode_t mode)
{
	int rcode;
	char *p;
	struct stat st;

	/*
	 *	If the directory exists, don't do anything.
	 */
	if (stat(directory, &st) == 0) {
		return 0;
	}

	/*
	 *	Look for the LAST directory name.  Try to create that,
	 *	failing on any error.
	 */
	p = strrchr(directory, FR_DIR_SEP);
	if (p != NULL) {
		*p = '\0';
		rcode = rad_mkdir(directory, mode);

		/*
		 *	On error, we leave the directory name as the
		 *	one which caused the error.
		 */
		if (rcode < 0) {
			if (errno == EEXIST) return 0;
			return rcode;
		}

		/*
		 *	Reset the directory delimiter, and go ask
		 *	the system to make the directory.
		 */
		*p = FR_DIR_SEP;
	} else {
		return 0;
	}

	/*
	 *	Having done everything successfully, we do the
	 *	system call to actually go create the directory.
	 */
	rcode = mkdir(directory, mode & 0777);
	if (rcode < 0) {
		return rcode;
	}

	/*
	 *	Set things like sticky bits that aren't supported by
	 *	mkdir.
	 */
	if (mode & ~0777) {
		rcode = chmod(directory, mode);
	}

	return rcode;
}


/*
 *	Allocate memory, or exit.
 *
 *	This call ALWAYS succeeds!
 */
void *rad_malloc(size_t size)
{
	void *ptr = malloc(size);

	if (ptr == NULL) {
		ERROR("no memory");
		fr_exit(1);
	}

	return ptr;
}


void *rad_calloc(size_t size)
{
	void *ptr = rad_malloc(size);
	memset(ptr, 0, size);
	return ptr;
}

void rad_const_free(void const *ptr)
{
	void *tmp;
	if (!ptr) return;

	memcpy(&tmp, &ptr, sizeof(tmp));
	talloc_free(tmp);
}


/*
 *	Logs an error message and aborts the program
 *
 */

void NEVER_RETURNS rad_assert_fail (char const *file, unsigned int line,
				    char const *expr)
{
	ERROR("ASSERT FAILED %s[%u]: %s", file, line, expr);
	abort();
}


/*
 *	Create a new REQUEST data structure.
 */
REQUEST *request_alloc(TALLOC_CTX *ctx)
{
	REQUEST *request;

	request = talloc_zero(ctx, REQUEST);
#ifndef NDEBUG
	request->magic = REQUEST_MAGIC;
#endif
#ifdef WITH_PROXY
	request->proxy = NULL;
#endif
	request->reply = NULL;
#ifdef WITH_PROXY
	request->proxy_reply = NULL;
#endif
	request->config_items = NULL;
	request->username = NULL;
	request->password = NULL;
	request->timestamp = time(NULL);
	request->options = debug_flag; /* Default to global debug level */

	request->module = "";
	request->component = "<core>";
	request->radlog = radlog_request;

	return request;
}


/*
 *	Create a new REQUEST, based on an old one.
 *
 *	This function allows modules to inject fake requests
 *	into the server, for tunneled protocols like TTLS & PEAP.
 */
REQUEST *request_alloc_fake(REQUEST *request)
{
	REQUEST *fake;

	fake = request_alloc(request);

	fake->number = request->number;
#ifdef HAVE_PTHREAD_H
	fake->child_pid = request->child_pid;
#endif
	fake->parent = request;
	fake->root = request->root;
	fake->client = request->client;

	/*
	 *	For new server support.
	 *
	 *	FIXME: Key instead off of a "virtual server" data structure.
	 *
	 *	FIXME: Permit different servers for inner && outer sessions?
	 */
	fake->server = request->server;

	fake->packet = rad_alloc(request, 1);
	if (!fake->packet) {
		request_free(&fake);
		return NULL;
	}

	fake->reply = rad_alloc(request, 0);
	if (!fake->reply) {
		request_free(&fake);
		return NULL;
	}

	fake->master_state = REQUEST_ACTIVE;
	fake->child_state = REQUEST_RUNNING;

	/*
	 *	Fill in the fake request.
	 */
	fake->packet->sockfd = -1;
	fake->packet->src_ipaddr = request->packet->src_ipaddr;
	fake->packet->src_port = request->packet->src_port;
	fake->packet->dst_ipaddr = request->packet->dst_ipaddr;
	fake->packet->dst_port = 0;

	/*
	 *	This isn't STRICTLY required, as the fake request MUST NEVER
	 *	be put into the request list.  However, it's still reasonable
	 *	practice.
	 */
	fake->packet->id = fake->number & 0xff;
	fake->packet->code = request->packet->code;
	fake->timestamp = request->timestamp;
	fake->packet->timestamp = request->packet->timestamp;

	/*
	 *	Required for new identity support
	 */
	fake->listener = request->listener;

	/*
	 *	Fill in the fake reply, based on the fake request.
	 */
	fake->reply->sockfd = fake->packet->sockfd;
	fake->reply->src_ipaddr = fake->packet->dst_ipaddr;
	fake->reply->src_port = fake->packet->dst_port;
	fake->reply->dst_ipaddr = fake->packet->src_ipaddr;
	fake->reply->dst_port = fake->packet->src_port;
	fake->reply->id = fake->packet->id;
	fake->reply->code = 0; /* UNKNOWN code */

	/*
	 *	Copy debug information.
	 */
	fake->options = request->options;
	fake->radlog = request->radlog;

	return fake;
}

#ifdef WITH_COA
REQUEST *request_alloc_coa(REQUEST *request)
{
	if (!request || request->coa) return NULL;

	/*
	 *	Originate CoA requests only when necessary.
	 */
	if ((request->packet->code != PW_AUTHENTICATION_REQUEST) &&
	    (request->packet->code != PW_ACCOUNTING_REQUEST)) return NULL;

	request->coa = request_alloc_fake(request);
	if (!request->coa) return NULL;

	request->coa->packet->code = 0; /* unknown, as of yet */
	request->coa->child_state = REQUEST_RUNNING;
	request->coa->proxy = rad_alloc(request->coa, 0);
	if (!request->coa->proxy) {
		request_free(&request->coa);
		return NULL;
	}

	return request->coa;
}
#endif

/*
 *	Copy a quoted string.
 */
int rad_copy_string(char *to, char const *from)
{
	int length = 0;
	char quote = *from;

	do {
		if (*from == '\\') {
			*(to++) = *(from++);
			length++;
		}
		*(to++) = *(from++);
		length++;
	} while (*from && (*from != quote));

	if (*from != quote) return -1; /* not properly quoted */

	*(to++) = quote;
	length++;
	*to = '\0';

	return length;
}

/*
 *	Copy a quoted string but without the quotes. The length
 *	returned is the number of chars written; the number of
 *	characters consumed is 2 more than this.
 */
int rad_copy_string_bare(char *to, char const *from)
{
	int length = 0;
	char quote = *from;

	from++;
	while (*from && (*from != quote)) {
		if (*from == '\\') {
			*(to++) = *(from++);
			length++;
		}
		*(to++) = *(from++);
		length++;
	}

	if (*from != quote) return -1; /* not properly quoted */

	*to = '\0';

	return length;
}


/*
 *	Copy a %{} string.
 */
int rad_copy_variable(char *to, char const *from)
{
	int length = 0;
	int sublen;

	*(to++) = *(from++);
	length++;

	while (*from) {
		switch (*from) {
		case '"':
		case '\'':
			sublen = rad_copy_string(to, from);
			if (sublen < 0) return sublen;
			from += sublen;
			to += sublen;
			length += sublen;
			break;

		case '}':	/* end of variable expansion */
			*(to++) = *(from++);
			*to = '\0';
			length++;
			return length; /* proper end of variable */

		case '\\':
			*(to++) = *(from++);
			*(to++) = *(from++);
			length += 2;
			break;

		case '%':	/* start of variable expansion */
			if (from[1] == '{') {
				*(to++) = *(from++);
				length++;

				sublen = rad_copy_variable(to, from);
				if (sublen < 0) return sublen;
				from += sublen;
				to += sublen;
				length += sublen;
				break;
			} /* else FIXME: catch %%{ ?*/

			/* FALL-THROUGH */
		default:
			*(to++) = *(from++);
			length++;
			break;
		}
	} /* loop over the input string */

	/*
	 *	We ended the string before a trailing '}'
	 */

	return -1;
}

#ifndef USEC
#define USEC 1000000
#endif

int rad_pps(int *past, int *present, time_t *then, struct timeval *now)
{
	int pps;

	if (*then != now->tv_sec) {
		*then = now->tv_sec;
		*past = *present;
		*present = 0;
	}

	/*
	 *	Bootstrap PPS by looking at a percentage of
	 *	the previous PPS.  This lets us take a moving
	 *	count, without doing a moving average.  If
	 *	we're a fraction "f" (0..1) into the current
	 *	second, we can get a good guess for PPS by
	 *	doing:
	 *
	 *	PPS = pps_now + pps_old * (1 - f)
	 *
	 *	It's an instantaneous measurement, rather than
	 *	a moving average.  This will hopefully let it
	 *	respond better to sudden spikes.
	 *
	 *	Doing the calculations by thousands allows us
	 *	to not overflow 2^32, AND to not underflow
	 *	when we divide by USEC.
	 */
	pps = USEC - now->tv_usec; /* useconds left in previous second */
	pps /= 1000;		   /* scale to milliseconds */
	pps *= *past;		   /* multiply by past count to get fraction */
	pps /= 1000;		   /* scale to usec again */
	pps += *present;	   /* add in current count */

	return pps;
}

/** Split string into words and expand each one
 *
 * @param request Current request.
 * @param cmd string to split.
 * @param max_argc the maximum number of arguments to split into.
 * @param argv Where to write the pointers into argv_buf.
 * @param can_fail If false, stop processing if any of the xlat expansions fail.
 * @param argv_buflen size of argv_buf.
 * @param argv_buf temporary buffer we used to mangle/expand cmd.
 *	Pointers to offsets of this buffer will be written to argv.
 * @return argc or -1 on failure.
 */

int rad_expand_xlat(REQUEST *request, char const *cmd,
		    int max_argc, char *argv[], bool can_fail,
		    size_t argv_buflen, char *argv_buf)
{
	char const *from;
	char *to;
	int argc = -1;
	int i;
	int left;

	if (strlen(cmd) > (argv_buflen - 1)) {
		ERROR("rad_expand_xlat: Command line is too long");
		return -1;
	}

	/*
	 *	Check for bad escapes.
	 */
	if (cmd[strlen(cmd) - 1] == '\\') {
		ERROR("rad_expand_xlat: Command line has final backslash, without a following character");
		return -1;
	}

	strlcpy(argv_buf, cmd, argv_buflen);

	/*
	 *	Split the string into argv's BEFORE doing radius_xlat...
	 */
	from = cmd;
	to = argv_buf;
	argc = 0;
	while (*from) {
		int length;

		/*
		 *	Skip spaces.
		 */
		if ((*from == ' ') || (*from == '\t')) {
			from++;
			continue;
		}

		argv[argc] = to;
		argc++;

		if (argc >= (max_argc - 1)) break;

		/*
		 *	Copy the argv over to our buffer.
		 */
		while (*from && (*from != ' ') && (*from != '\t')) {
			if (to >= argv_buf + argv_buflen - 1) {
				ERROR("rad_expand_xlat: Ran out of space in command line");
				return -1;
			}

			switch (*from) {
			case '"':
			case '\'':
				length = rad_copy_string_bare(to, from);
				if (length < 0) {
					ERROR("rad_expand_xlat: Invalid string passed as argument");
					return -1;
				}
				from += length+2;
				to += length;
				break;

			case '%':
				if (from[1] == '{') {
					*(to++) = *(from++);

					length = rad_copy_variable(to, from);
					if (length < 0) {
						ERROR("rad_expand_xlat: Invalid variable expansion passed as argument");
						return -1;
					}
					from += length;
					to += length;
				} else { /* FIXME: catch %%{ ? */
					*(to++) = *(from++);
				}
				break;

			case '\\':
				if (from[1] == ' ') from++;
				/* FALL-THROUGH */

			default:
				*(to++) = *(from++);
			}
		} /* end of string, or found a space */

		*(to++) = '\0';	/* terminate the string */
	}

	/*
	 *	We have to have SOMETHING, at least.
	 */
	if (argc <= 0) {
		ERROR("rad_expand_xlat: Empty command line.");
		return -1;
	}

	/*
	 *	Expand each string, as appropriate.
	 */
	left = argv_buf + argv_buflen - to;
	for (i = 0; i < argc; i++) {
		int sublen;

		/*
		 *	Don't touch argv's which won't be translated.
		 */
		if (strchr(argv[i], '%') == NULL) continue;

		if (!request) continue;

		sublen = radius_xlat(to, left - 1, request, argv[i], NULL, NULL);
		if (sublen <= 0) {
			if (can_fail) {
				/*
				 *	Fail to be backwards compatible.
				 *
				 *	It's yucky, but it won't break anything,
				 *	and it won't cause security problems.
				 */
				sublen = 0;
			} else {
				ERROR("rad_expand_xlat: xlat failed");
				return -1;
			}
		}

		argv[i] = to;
		to += sublen;
		*(to++) = '\0';
		left -= sublen;
		left--;

		if (left <= 0) {
			ERROR("rad_expand_xlat: Ran out of space while expanding arguments.");
			return -1;
		}
	}
	argv[argc] = NULL;

	return argc;
}

const FR_NAME_NUMBER pair_lists[] = {
	{ "request",		PAIR_LIST_REQUEST },
	{ "reply",		PAIR_LIST_REPLY },
	{ "config",		PAIR_LIST_CONTROL },
	{ "control",		PAIR_LIST_CONTROL },
#ifdef WITH_PROXY
	{ "proxy-request",	PAIR_LIST_PROXY_REQUEST },
	{ "proxy-reply",	PAIR_LIST_PROXY_REPLY },
#endif
#ifdef WITH_COA
	{ "coa",		PAIR_LIST_COA },
	{ "coa-reply",		PAIR_LIST_COA_REPLY },
	{ "disconnect",		PAIR_LIST_DM },
	{ "disconnect-reply",	PAIR_LIST_DM_REPLY },
#endif
	{  NULL , -1 }
};

const FR_NAME_NUMBER request_refs[] = {
	{ "outer",		REQUEST_OUTER },
	{ "current",		REQUEST_CURRENT },
	{ "parent",		REQUEST_PARENT },
	{  NULL , -1 }
};


/** Resolve attribute name to a list.
 *
 * Check the name string for qualifiers that specify a list and return
 * an pair_lists_t value for that list. This value may be passed to
 * radius_list, along with the current request, to get a pointer to the
 * actual list in the request.
 *
 * If qualifiers were consumed, write a new pointer into name to the
 * char after the last qualifier to be consumed.
 *
 * radius_list_name should be called before passing a name string that
 * may contain qualifiers to dict_attrbyname.
 *
 * @see dict_attrbyname
 *
 * @param[in,out] name of attribute.
 * @param[in] unknown the list to return if no qualifiers were found.
 * @return PAIR_LIST_UNKOWN if qualifiers couldn't be resolved to a list.
 */
pair_lists_t radius_list_name(char const **name, pair_lists_t unknown)
{
	char const *p = *name;
	char const *q;
	pair_lists_t output;

	/* This should never be a NULL pointer or zero length string */
	rad_assert(name && *name);

	/*
	 *	We couldn't determine the list if:
	 *
	 * 	A colon delimiter was found, but the next char was a
	 *	number, indicating a tag, not a list qualifier.
	 *
	 *	No colon was found and the first char was upper case
	 *	indicating an attribute.
	 *
	 */
	q = strchr(p, ':');
	if (((q && (q[1] >= '0') && (q[1] <= '9'))) ||
	    (!q && isupper((int) *p))) {
		return unknown;
	}

	if (q) {
		*name = (q + 1);	/* Consume the list and delimiter */
		return fr_substr2int(pair_lists, p, PAIR_LIST_UNKNOWN, (q - p));
	}

	q = (p + strlen(p));	/* Consume the entire string */
	output = fr_substr2int(pair_lists, p, PAIR_LIST_UNKNOWN, (q - p));
	if (output != PAIR_LIST_UNKNOWN) {
		*name = q;
		return output;
	}

	return unknown;
}


/** Resolve attribute name to a request.
 *
 * Check the name string for qualifiers that reference a parent request and
 * write the pointer to this request to 'request'.
 *
 * If qualifiers were consumed, write a new pointer into name to the
 * char after the last qualifier to be consumed.
 *
 * radius_ref_request should be called before radius_list_name.
 *
 * @see radius_list_name
 * @param[in,out] name of attribute.
 * @param[in] def default request ref to return if no request qualifier is present.
 * @return one of the REQUEST_* definitions or REQUEST_UNKOWN
 */
request_refs_t radius_request_name(char const **name, request_refs_t def)
{
	char *p;
	int request;

	p = strchr(*name, '.');
	if (!p) {
		return def;
	}

	/*
	 *	We may get passed "127.0.0.1".
	 */
	request = fr_substr2int(request_refs, *name, REQUEST_UNKNOWN,
				p - *name);

	/*
	 *	If we get a valid name, skip it.
	 */
	if (request != REQUEST_UNKNOWN) {
		*name = p + 1;
		return request;
	}

	/*
	 *	Otherwise leave it alone, and return the caller's
	 *	default.
	 */
	return def;
}

/** Resolve request to a request.
 *
 * Resolve name to a current request.
 *
 * @see radius_list
 * @param[in,out] context Base context to use, and to write the result back to.
 * @param[in] name (request) to resolve to.
 * @return 0 if request is valid in this context, else -1.
 */
int radius_request(REQUEST **context, request_refs_t name)
{
	REQUEST *request = *context;

	switch (name) {
		case REQUEST_CURRENT:
			return 0;

		case REQUEST_PARENT:	/* for future use in request chaining */
		case REQUEST_OUTER:
			if (!request->parent) {
				REDEBUG("Specified request \"%s\" is not available in this context",
				        fr_int2str(request_refs, name, "<INVALID>"));
				return -1;
			}

			*context = request->parent;

			break;

		case REQUEST_UNKNOWN:
		default:
			rad_assert(0);
			return -1;
	}

	return 0;
}

/** Adds subcapture values to request data
 *
 * Allows use of %{n} expansions.
 *
 * @param request Current request.
 * @param compare Result returned by regexec.
 * @param value The original value.
 * @param rxmatch Pointers into value.
 */
void rad_regcapture(REQUEST *request, int compare, char const *value, regmatch_t rxmatch[])
{
	int i;
	char *p;
	size_t len;

	if (compare == REG_NOMATCH) {
		return;
	}

	/*
	 *	Add new %{0}, %{1}, etc.
	 */
	for (i = 0; i <= REQUEST_MAX_REGEX; i++) {
		/*
		 *	Didn't match: delete old match, if it existed.
		 */
		if (rxmatch[i].rm_so == -1) {
			p = request_data_get(request, request, REQUEST_DATA_REGEX | i);
			if (p) {
				RDEBUG4("%%{%i}: Clearing old value \"%s\"", i, p);
				talloc_free(p);
			} else {
				RDEBUG4("%%{%i}: Was empty", i);
			}

			continue;
		}

		len = rxmatch[i].rm_eo - rxmatch[i].rm_so;
		p = talloc_array(request, char, len + 1);
		memcpy(p, value + rxmatch[i].rm_so, len);
		p[len] = '\0';

		RDEBUG4("%%{%i}: Inserting new value \"%s\"", i, p);
		/*
		 *	Copy substring, and add it to
		 *	the request.
		 *
		 *	Note that we don't check
		 *	for out of memory, which is
		 *	the only error we can get...
		 */
		request_data_add(request, request, REQUEST_DATA_REGEX | i, p, true);
	}
}

