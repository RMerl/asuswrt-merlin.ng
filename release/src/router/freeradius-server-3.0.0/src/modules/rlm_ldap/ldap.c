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
 * @file ldap.c
 * @brief LDAP module library functions.
 *
 * @author Arran Cudbard-Bell <a.cudbardb@freeradius.org>
 * @copyright 2013 Network RADIUS SARL <info@networkradius.com>
 * @copyright 2013 The FreeRADIUS Server Project.
 */
#include	<freeradius-devel/radiusd.h>
#include	<freeradius-devel/modules.h>
#include	<freeradius-devel/rad_assert.h>

#include	<stdarg.h>
#include	<ctype.h>

#include	<lber.h>
#include	<ldap.h>
#include	"ldap.h"



/** Converts "bad" strings into ones which are safe for LDAP
 *
 * This is a callback for xlat operations.
 *
 * Will escape any characters in input strings that would cause the string to be interpreted as part of a DN and or
 * filter. Escape sequence is @verbatim \<hex><hex> @endverbatim
 *
 * @param request The current request.
 * @param out Pointer to output buffer.
 * @param outlen Size of the output buffer.
 * @param in Raw unescaped string.
 * @param arg Any additional arguments (unused).
 */
size_t rlm_ldap_escape_func(UNUSED REQUEST *request, char *out, size_t outlen, char const *in, UNUSED void *arg)
{
	static char const encode[] = ",+\"\\<>;*=()";
	static char const hextab[] = "0123456789abcdef";
	size_t left = outlen;

	if (*in && ((*in == ' ') || (*in == '#'))) {
		goto encode;
	}

	while (*in) {
		/*
		 *	Encode unsafe characters.
		 */
		if (memchr(encode, *in, sizeof(encode) - 1)) {
			encode:

			/*
			 *	Only 3 or less bytes available.
			 */
			if (left <= 3) break;

			*out++ = '\\';
			*out++ = hextab[(*in >> 4) & 0x0f];
			*out++ = hextab[*in & 0x0f];
			in++;
			left -= 3;

			continue;
		}

		if (left <= 1) break;

		/*
		 *	Doesn't need encoding
		 */
		*out++ = *in++;
		left--;
	}

	*out = '\0';

	return outlen - left;
}

/** Check whether a string is a DN
 *
 * @param str to check.
 * @return true if string is a DN, else false.
 */
int rlm_ldap_is_dn(char const *str)
{
	return strrchr(str, ',') == NULL ? false : true;
}

/** Find the place at which the two DN strings diverge
 *
 * Returns the length of the non matching string in full.
 *
 * @param full DN.
 * @param part Partial DN as returned by ldap_parse_result.
 * @return the length of the portion of full which wasn't matched or -1 on error.
 */
static size_t rlm_ldap_common_dn(char const *full, char const *part)
{
	size_t f_len, p_len, i;

	if (!full) {
		return -1;
	}

	f_len = strlen(full);

	if (!part) {
		return -1;
	}

	p_len = strlen(part);
	if (!p_len) {
		return f_len;
	}

	if ((f_len < p_len) || !f_len) {
		return -1;
	}

	for (i = 0; i < p_len; i++) {
		if (part[p_len - i] != full[f_len - i]) {
			return -1;
		}
	}

	return f_len - p_len;
}

/** Combine and expand filters
 *
 * @param request Current request.
 * @param out Where to write the expanded string.
 * @param outlen Length of output buffer.
 * @param sub Array of subfilters (may contain NULLs).
 * @param sublen Number of potential subfilters in array.
 * @return length of expanded data.
 */
ssize_t rlm_ldap_xlat_filter(REQUEST *request, char const **sub, size_t sublen, char *out, size_t outlen)
{
	char buffer[LDAP_MAX_FILTER_STR_LEN + 1];
	char const *in = NULL;
	char *p = buffer;

	ssize_t len = 0;

	unsigned int i;
	int cnt = 0;

	/*
	 *	Figure out how many filter elements we need to integrate
	 */
	for (i = 0; i < sublen; i++) {
		if (sub[i] && *sub[i]) {
			in = sub[i];
			cnt++;
		}
	}

	if (!cnt) {
		out[0] = '\0';
		return 0;
	}

	if (cnt > 1) {
		if (outlen < 3) {
			goto oob;
		}

		p[len++] = '(';
		p[len++] = '&';

		for (i = 0; i < sublen; i++) {
			if (sub[i] && (*sub[i] != '\0')) {
				len += strlcpy(p + len, sub[i], outlen - len);

				if ((size_t) len >= outlen) {
					oob:
					REDEBUG("Out of buffer space creating filter");

					return -1;
				}
			}
		}

		if ((outlen - len) < 2) {
			goto oob;
		}

		p[len++] = ')';
		p[len] = '\0';

		in = buffer;
	}

	len = radius_xlat(out, outlen, request, in, rlm_ldap_escape_func, NULL);
	if (len < 0) {
		REDEBUG("Failed creating filter");

		return -1;
	}

	return len;
}

/** Return the error string associated with a handle
 *
 * @param conn to retrieve error from.
 * @return error string.
 */
char const *rlm_ldap_error_str(ldap_handle_t const *conn)
{
	int lib_errno;
	ldap_get_option(conn->handle, LDAP_OPT_ERROR_NUMBER, &lib_errno);
	if (lib_errno == LDAP_SUCCESS) {
		return "unknown";
	}

	return ldap_err2string(lib_errno);
}

/** Parse response from LDAP server dealing with any errors
 *
 * Should be called after an LDAP operation. Will check result of operation and if it was successful, then attempt
 * to retrieve and parse the result.
 *
 * Will also produce extended error output including any messages the server sent, and information about partial
 * DN matches.
 *
 * @param[in] inst of LDAP module.
 * @param[in] conn Current connection.
 * @param[in] msgid returned from last operation.
 * @param[in] dn Last search or bind DN.
 * @param[out] result Where to write result, if NULL result will be freed.
 * @param[out] error Where to write the error string, may be NULL, must not be freed.
 * @param[out] extra Where to write additional error string to, may be NULL (faster) or must be freed
 *	(with talloc_free).
 * @return One of the LDAP_PROC_* codes.
 */
static ldap_rcode_t rlm_ldap_result(ldap_instance_t const *inst, ldap_handle_t const *conn, int msgid, char const *dn,
				    LDAPMessage **result, char const **error, char **extra)
{
	ldap_rcode_t status = LDAP_PROC_SUCCESS;

	int lib_errno = LDAP_SUCCESS;	// errno returned by the library.
	int srv_errno = LDAP_SUCCESS;	// errno in the result message.

	char *part_dn = NULL;		// Partial DN match.
	char *our_err = NULL;		// Our extended error message.
	char *srv_err = NULL;		// Server's extended error message.
	char *p, *a;

	int freeit = false;		// Whether the message should be freed after being processed.
	int len;

	struct timeval tv;		// Holds timeout values.

	LDAPMessage *tmp_msg;		// Temporary message pointer storage if we weren't provided with one.

	char const *tmp_err;		// Temporary error pointer storage if we weren't provided with one.

	if (!error) {
		error = &tmp_err;
	}
	*error = NULL;

	if (extra) {
		*extra = NULL;
	}

	/*
	 *	We always need the result, but our caller may not
	 */
	if (!result) {
		result = &tmp_msg;
		freeit = true;
	}

	*result = NULL;

	/*
	 *	Check if there was an error sending the request
	 */
	ldap_get_option(conn->handle, LDAP_OPT_ERROR_NUMBER,
			&lib_errno);
	if (lib_errno != LDAP_SUCCESS) {
		goto process_error;
	}

	memset(&tv, 0, sizeof(tv));
	tv.tv_sec = inst->res_timeout;

	/*
	 *	Now retrieve the result and check for errors
	 *	ldap_result returns -1 on error, and 0 on timeout
	 */
	lib_errno = ldap_result(conn->handle, msgid, 1, &tv, result);
	if (lib_errno == 0) {
		lib_errno = LDAP_TIMEOUT;

		goto process_error;
	}

	if (lib_errno == -1) {
		ldap_get_option(conn->handle, LDAP_OPT_ERROR_NUMBER,
				&lib_errno);
		goto process_error;
	}

	/*
	 *	Parse the result and check for errors sent by the server
	 */
	lib_errno = ldap_parse_result(conn->handle, *result,
				      &srv_errno,
				      extra ? &part_dn : NULL,
				      extra ? &srv_err : NULL,
				      NULL, NULL, freeit);
	if (freeit) {
		*result = NULL;
	}

	if (lib_errno != LDAP_SUCCESS) {
		ldap_get_option(conn->handle, LDAP_OPT_ERROR_NUMBER,
				&lib_errno);
		goto process_error;
	}

	process_error:

	if ((lib_errno == LDAP_SUCCESS) && (srv_errno != LDAP_SUCCESS)) {
		lib_errno = srv_errno;
	} else if ((lib_errno != LDAP_SUCCESS) && (srv_errno == LDAP_SUCCESS)) {
		srv_errno = lib_errno;
	}

	switch (lib_errno) {
	case LDAP_SUCCESS:
		*error = "Success";

		break;

	case LDAP_NO_SUCH_OBJECT:
		*error = "The specified DN wasn't found, check base_dn and identity";

		status = LDAP_PROC_BAD_DN;

		if (!extra) break;

		/*
		 *	Build our own internal diagnostic string
		 */
		len = rlm_ldap_common_dn(dn, part_dn);
		if (len < 0) break;

		our_err = talloc_asprintf(conn, "Match stopped here: [%.*s]%s", len, dn, part_dn ? part_dn : "");

		goto error_string;

	case LDAP_INSUFFICIENT_ACCESS:
		*error = "Insufficient access. Check the identity and password configuration directives";

		status = LDAP_PROC_NOT_PERMITTED;
		break;

	case LDAP_UNWILLING_TO_PERFORM:
		*error = "Server was unwilling to perform";

		status = LDAP_PROC_NOT_PERMITTED;
		break;

	case LDAP_TIMEOUT:
		exec_trigger(NULL, inst->cs, "modules.ldap.timeout", true);

		*error = "Timed out while waiting for server to respond";

		status = LDAP_PROC_ERROR;
		break;

	case LDAP_FILTER_ERROR:
		*error = "Bad search filter";

		status = LDAP_PROC_ERROR;
		break;

	case LDAP_TIMELIMIT_EXCEEDED:
		exec_trigger(NULL, inst->cs, "modules.ldap.timeout", true);

		*error = "Time limit exceeded";
		/* FALL-THROUGH */

	case LDAP_BUSY:
	case LDAP_UNAVAILABLE:
	case LDAP_SERVER_DOWN:
		status = LDAP_PROC_RETRY;

		goto error_string;

	case LDAP_INVALID_CREDENTIALS:
	case LDAP_CONSTRAINT_VIOLATION:
		status = LDAP_PROC_REJECT;

		goto error_string;

	case LDAP_OPERATIONS_ERROR:
		*error = "Please set 'chase_referrals=yes' and 'rebind=yes'. See the ldap module configuration "
			 "for details.";

		/* FALL-THROUGH */
	default:
		status = LDAP_PROC_ERROR;

		error_string:

		if (!*error) {
			*error = ldap_err2string(lib_errno);
		}

		if (!extra || ((lib_errno == srv_errno) && !our_err && !srv_err)) {
			break;
		}

		/*
		 *	Output the error codes from the library and server
		 */
		p = talloc_zero_array(conn, char, 1);
		if (!p) break;

		if (lib_errno != srv_errno) {
			a = talloc_asprintf_append(p, "LDAP lib error: %s (%u), srv error: %s (%u). ",
				      		   ldap_err2string(lib_errno), lib_errno,
						   ldap_err2string(srv_errno), srv_errno);
			if (!a) {
				talloc_free(p);
				break;
			}

			p = a;
		}

		if (our_err) {
			a = talloc_asprintf_append_buffer(p, "%s. ", our_err);
			if (!a) {
				talloc_free(p);
				break;
			}

			p = a;
		}

		if (srv_err) {
			a = talloc_asprintf_append_buffer(p, "Server said: %s. ", srv_err);
			if (!a) {
				talloc_free(p);
				break;
			}

			p = a;
		}

		*extra = p;

		break;
	}

	/*
	 *	Cleanup memory
	 */
	if (srv_err) {
		ldap_memfree(srv_err);
	}

	if (part_dn) {
		ldap_memfree(part_dn);
	}

	if (our_err) {
		talloc_free(our_err);
	}

	if ((lib_errno || srv_errno) && *result) {
		ldap_msgfree(*result);
		*result = NULL;
	}

	return status;
}

/** Bind to the LDAP directory as a user
 *
 * Performs a simple bind to the LDAP directory, and handles any errors that occur.
 *
 * @param[in] inst rlm_ldap configuration.
 * @param[in] request Current request, this may be NULL, in which case all debug logging is done with radlog.
 * @param[in,out] pconn to use. May change as this function calls functions which auto re-connect.
 * @param[in] dn of the user, may be NULL to bind anonymously.
 * @param[in] password of the user, may be NULL if no password is specified.
 * @param[in] retry if the server is down.
 * @return one of the LDAP_PROC_* values.
 */
ldap_rcode_t rlm_ldap_bind(ldap_instance_t const *inst, REQUEST *request, ldap_handle_t **pconn, char const *dn,
			   char const *password, int retry)
{
	ldap_rcode_t	status;

	int		msgid;

	char const	*error = NULL;
	char 		*extra = NULL;

	rad_assert(*pconn && (*pconn)->handle);

	/*
	 *	Bind as anonymous user
	 */
	if (!dn) dn = "";

retry:
	msgid = ldap_bind((*pconn)->handle, dn, password, LDAP_AUTH_SIMPLE);
	/* We got a valid message ID */
	if (msgid >= 0) {
		if (request) {
			RDEBUG2("Waiting for bind result...");
		} else {
			DEBUG2("rlm_ldap (%s): Waiting for bind result...", inst->xlat_name);
		}
	}

	status = rlm_ldap_result(inst, *pconn, msgid, dn, NULL, &error, &extra);
	switch (status) {
	case LDAP_PROC_SUCCESS:
		LDAP_DBG_REQ("Bind successful");
		break;
	case LDAP_PROC_NOT_PERMITTED:
		LDAP_ERR_REQ("Bind was not permitted: %s", error);
		LDAP_EXT_REQ();

		break;

	case LDAP_PROC_REJECT:
		LDAP_ERR_REQ("Bind credentials incorrect: %s", error);
		LDAP_EXT_REQ();

		break;

	case LDAP_PROC_RETRY:
		if (retry) {
			*pconn = fr_connection_reconnect(inst->pool, *pconn);
			if (*pconn) {
				LDAP_DBGW_REQ("Bind with %s to %s:%d failed: %s. Got new socket, retrying...",
					      dn, inst->server, inst->port, error);

				talloc_free(extra); /* don't leak debug info */

				goto retry;
			}
		};

		status = LDAP_PROC_ERROR;

		/*
		 *	Were not allowed to retry, or there are no more
		 *	sockets, treat this as a hard failure.
		 */
		/* FALL-THROUGH */
	default:
#ifdef HAVE_LDAP_INITIALIZE
		if (inst->is_url) {
			LDAP_ERR_REQ("Bind with %s to %s failed: %s", dn, inst->server, error);
		} else
#endif
		{
			LDAP_ERR_REQ("Bind with %s to %s:%d failed: %s", dn, inst->server,
				     inst->port, error);
		}
		LDAP_EXT_REQ();

		break;
	}

	if (extra) {
		talloc_free(extra);
	}

	return status; /* caller closes the connection */
}


/** Search for something in the LDAP directory
 *
 * Binds as the administrative user and performs a search, dealing with any errors.
 *
 * @param[in] inst rlm_ldap configuration.
 * @param[in] request Current request.
 * @param[in,out] pconn to use. May change as this function calls functions which auto re-connect.
 * @param[in] dn to use as base for the search.
 * @param[in] scope to use (LDAP_SCOPE_BASE, LDAP_SCOPE_ONE, LDAP_SCOPE_SUB).
 * @param[in] filter to use, should be pre-escaped.
 * @param[in] attrs to retrieve.
 * @param[out] result Where to store the result. Must be freed with ldap_msgfree if LDAP_PROC_SUCCESS is returned.
 *	May be NULL in which case result will be automatically freed after use.
 * @return One of the LDAP_PROC_* values.
 */
ldap_rcode_t rlm_ldap_search(ldap_instance_t const *inst, REQUEST *request, ldap_handle_t **pconn,
			     char const *dn, int scope, char const *filter, char const * const *attrs,
			     LDAPMessage **result)
{
	ldap_rcode_t	status;
	LDAPMessage	*our_result = NULL;

	int		msgid;		// Message id returned by
					// ldap_search_ext.

	int		count = 0;	// Number of results we got.

	struct timeval	tv;		// Holds timeout values.

	char const 	*error = NULL;
	char		*extra = NULL;

	rad_assert(*pconn && (*pconn)->handle);

	/*
	 *	OpenLDAP library doesn't declare attrs array as const, but
	 *	it really should be *sigh*.
	 */
	char **search_attrs;
	memcpy(&search_attrs, &attrs, sizeof(attrs));

	/*
	 *	Do all searches as the admin user.
	 */
	if ((*pconn)->rebound) {
		status = rlm_ldap_bind(inst, request, pconn, inst->admin_dn, inst->password, true);
		if (status != LDAP_PROC_SUCCESS) {
			return LDAP_PROC_ERROR;
		}

		rad_assert(*pconn);

		(*pconn)->rebound = false;
	}

	LDAP_DBG_REQ("Performing search in '%s' with filter '%s'", dn, filter);

	/*
	 *	If LDAP search produced an error it should also be logged
	 *	to the ld. result should pick it up without us
	 *	having to pass it explicitly.
	 */
	memset(&tv, 0, sizeof(tv));
	tv.tv_sec = inst->res_timeout;
retry:
	(void) ldap_search_ext((*pconn)->handle, dn, scope, filter, search_attrs, 0, NULL, NULL, &tv, 0, &msgid);

	LDAP_DBG_REQ("Waiting for search result...");
	status = rlm_ldap_result(inst, *pconn, msgid, dn, &our_result, &error, &extra);
	switch (status) {
		case LDAP_PROC_SUCCESS:
			break;
		case LDAP_PROC_RETRY:
			*pconn = fr_connection_reconnect(inst->pool, *pconn);
			if (*pconn) {
				LDAP_DBGW_REQ("Search failed: %s. Got new socket, retrying...", error);

				talloc_free(extra); /* don't leak debug info */

				goto retry;
			}

			status = LDAP_PROC_ERROR;

			/* FALL-THROUGH */
		default:
			LDAP_ERR_REQ("Failed performing search: %s", error);
			if (extra) LDAP_ERR_REQ("%s", extra);

			goto finish;
	}

	count = ldap_count_entries((*pconn)->handle, our_result);
	if (count < 0) {
		LDAP_ERR_REQ("Error counting results: %s", rlm_ldap_error_str(*pconn));
		status = LDAP_PROC_ERROR;

		ldap_msgfree(our_result);
		our_result = NULL;
	} else if (count == 0) {
		LDAP_DBG_REQ("Search returned no results");
		status = LDAP_PROC_NO_RESULT;

		ldap_msgfree(our_result);
		our_result = NULL;
	}

	finish:
	if (extra) {
		talloc_free(extra);
	}

	/*
	 *	We always need to get the result to count entries, but the caller
	 *	may not of requested one. If that's the case, free it, else write
	 *	it to where our caller said.
	 */
	if (!result) {
		if (our_result) {
			ldap_msgfree(our_result);
		}
	} else {
		*result = our_result;
	}

	return status;
}

/** Modify something in the LDAP directory
 *
 * Binds as the administrative user and attempts to modify an LDAP object.
 *
 * @param[in] inst rlm_ldap configuration.
 * @param[in] request Current request.
 * @param[in,out] pconn to use. May change as this function calls functions which auto re-connect.
 * @param[in] dn of the object to modify.
 * @param[in] mods to make, see 'man ldap_modify' for more information.
 * @return One of the LDAP_PROC_* values.
 */
ldap_rcode_t rlm_ldap_modify(ldap_instance_t const *inst, REQUEST *request, ldap_handle_t **pconn,
			     char const *dn, LDAPMod *mods[])
{
	ldap_rcode_t	status;

	int		msgid;		// Message id returned by ldap_search_ext.

	char const 	*error = NULL;
	char		*extra = NULL;

	rad_assert(*pconn && (*pconn)->handle);

	/*
	 *	Perform all modifications as the admin user.
	 */
	if ((*pconn)->rebound) {
		status = rlm_ldap_bind(inst, request, pconn, inst->admin_dn, inst->password, true);
		if (status != LDAP_PROC_SUCCESS) {
			return LDAP_PROC_ERROR;
		}

		rad_assert(*pconn);

		(*pconn)->rebound = false;
	}

	RDEBUG2("Modifying object with DN \"%s\"", dn);
	retry:
	(void) ldap_modify_ext((*pconn)->handle, dn, mods, NULL, NULL, &msgid);

	RDEBUG2("Waiting for modify result...");
	status = rlm_ldap_result(inst, *pconn, msgid, dn, NULL, &error, &extra);
	switch (status) {
		case LDAP_PROC_SUCCESS:
			break;
		case LDAP_PROC_RETRY:
			*pconn = fr_connection_reconnect(inst->pool, *pconn);
			if (*pconn) {
				RWDEBUG("Modify failed: %s. Got new socket, retrying...", error);

				talloc_free(extra); /* don't leak debug info */

				goto retry;
			}

			status = LDAP_PROC_ERROR;

			/* FALL-THROUGH */
		default:
			REDEBUG("Failed modifying object: %s", error);
			REDEBUG("%s", extra);

			goto finish;
	}

	finish:
	if (extra) {
		talloc_free(extra);
	}

	return status;
}

/** Retrieve the DN of a user object
 *
 * Retrieves the DN of a user and adds it to the control list as LDAP-UserDN. Will also retrieve any attributes
 * passed and return the result in *result.
 *
 * This potentially allows for all authorization and authentication checks to be performed in one ldap search
 * operation, which is a big bonus given the number of crappy, slow *cough*AD*cough* LDAP directory servers out there.
 *
 * @param[in] inst rlm_ldap configuration.
 * @param[in] request Current request.
 * @param[in,out] pconn to use. May change as this function calls functions which auto re-connect.
 * @param[in] attrs Additional attributes to retrieve, may be NULL.
 * @param[in] force Query even if the User-DN already exists.
 * @param[out] result Where to write the result, may be NULL in which case result is discarded.
 * @param[out] rcode The status of the operation, one of the RLM_MODULE_* codes.
 * @return The user's DN or NULL on error.
 */
char const *rlm_ldap_find_user(ldap_instance_t const *inst, REQUEST *request, ldap_handle_t **pconn,
			       char const *attrs[], int force, LDAPMessage **result, rlm_rcode_t *rcode)
{
	static char const *tmp_attrs[] = { NULL };

	ldap_rcode_t	status;
	VALUE_PAIR	*vp = NULL;
	LDAPMessage	*tmp_msg = NULL, *entry = NULL;
	int		ldap_errno;
	char		*dn = NULL;
	char	    	filter[LDAP_MAX_FILTER_STR_LEN];
	char	    	base_dn[LDAP_MAX_DN_STR_LEN];

	int freeit = false;					//!< Whether the message should
								//!< be freed after being processed.

	*rcode = RLM_MODULE_FAIL;

	if (!result) {
		result = &tmp_msg;
		freeit = true;
	}
	*result = NULL;

	if (!attrs) {
		memset(&attrs, 0, sizeof(tmp_attrs));
	}

	/*
	 *	If the caller isn't looking for the result we can just return the current userdn value.
	 */
	if (!force) {
		vp = pairfind(request->config_items, PW_LDAP_USERDN, 0, TAG_ANY);
		if (vp) {
			RDEBUG("Using user DN from request \"%s\"", vp->vp_strvalue);
			*rcode = RLM_MODULE_OK;
			return vp->vp_strvalue;
		}
	}

	/*
	 *	Perform all searches as the admin user.
	 */
	if ((*pconn)->rebound) {
		status = rlm_ldap_bind(inst, request, pconn, inst->admin_dn, inst->password, true);
		if (status != LDAP_PROC_SUCCESS) {
			*rcode = RLM_MODULE_FAIL;
			return NULL;
		}

		rad_assert(*pconn);

		(*pconn)->rebound = false;
	}

	if (radius_xlat(filter, sizeof(filter), request, inst->userobj_filter, rlm_ldap_escape_func, NULL) < 0) {
		REDEBUG("Unable to create filter");

		*rcode = RLM_MODULE_INVALID;
		return NULL;
	}

	if (radius_xlat(base_dn, sizeof(base_dn), request, inst->userobj_base_dn, rlm_ldap_escape_func, NULL) < 0) {
		REDEBUG("Unable to create base_dn");

		*rcode = RLM_MODULE_INVALID;
		return NULL;
	}

	status = rlm_ldap_search(inst, request, pconn, base_dn, inst->userobj_scope, filter, attrs, result);
	switch (status) {
		case LDAP_PROC_SUCCESS:
			break;
		case LDAP_PROC_NO_RESULT:
			*rcode = RLM_MODULE_NOTFOUND;
			return NULL;
		default:
			*rcode = RLM_MODULE_FAIL;
			return NULL;
	}

	rad_assert(*pconn);

	entry = ldap_first_entry((*pconn)->handle, *result);
	if (!entry) {
		ldap_get_option((*pconn)->handle, LDAP_OPT_RESULT_CODE, &ldap_errno);
		REDEBUG("Failed retrieving entry: %s",
			ldap_err2string(ldap_errno));

		goto finish;
	}

	dn = ldap_get_dn((*pconn)->handle, entry);
	if (!dn) {
		ldap_get_option((*pconn)->handle, LDAP_OPT_RESULT_CODE, &ldap_errno);

		REDEBUG("Retrieving object DN from entry failed: %s",
			ldap_err2string(ldap_errno));

		goto finish;
	}

	RDEBUG("User object found at DN \"%s\"", dn);
	vp = pairmake(request, &request->config_items, "LDAP-UserDN", dn, T_OP_EQ);
	if (vp) {
		*rcode = RLM_MODULE_OK;
	}

	finish:
	ldap_memfree(dn);

	if ((freeit || (*rcode != RLM_MODULE_OK)) && *result) {
		ldap_msgfree(*result);
		*result = NULL;
	}

	return vp ? vp->vp_strvalue : NULL;
}

/** Check for presence of access attribute in result
 *
 * @param[in] inst rlm_ldap configuration.
 * @param[in] request Current request.
 * @param[in] conn used to retrieve access attributes.
 * @param[in] entry retrieved by rlm_ldap_find_user or rlm_ldap_search.
 * @return RLM_MODULE_USERLOCK if the user was denied access, else RLM_MODULE_OK.
 */
rlm_rcode_t rlm_ldap_check_access(ldap_instance_t const *inst, REQUEST *request,
				  ldap_handle_t const *conn, LDAPMessage *entry)
{
	rlm_rcode_t rcode = RLM_MODULE_OK;
	char **vals = NULL;

	vals = ldap_get_values(conn->handle, entry, inst->userobj_access_attr);
	if (vals) {
		if (inst->access_positive) {
			if (strncmp(vals[0], "false", 5) == 0) {
				RDEBUG("\"%s\" attribute exists but is set to 'false' - user locked out");
				rcode = RLM_MODULE_USERLOCK;
			}
			/* RLM_MODULE_OK set above... */
		} else {
			RDEBUG("\"%s\" attribute exists - user locked out", inst->userobj_access_attr);
			rcode = RLM_MODULE_USERLOCK;
		}

		ldap_value_free(vals);
	} else if (inst->access_positive) {
		RDEBUG("No \"%s\" attribute - user locked out", inst->userobj_access_attr);
		rcode = RLM_MODULE_USERLOCK;
	}

	return rcode;
}

/** Verify we got a password from the search
 *
 * Checks to see if after the LDAP to RADIUS mapping has been completed that a reference password.
 *
 * @param inst rlm_ldap configuration.
 * @param request Current request.
 */
void rlm_ldap_check_reply(ldap_instance_t const *inst, REQUEST *request)
{
       /*
	*	More warning messages for people who can't be bothered to read the documentation.
	*
	*	Expect_password is set when we process the mapping, and is only true if there was a mapping between
	*	an LDAP attribute and a password reference attribute in the control list.
	*/
	if (inst->expect_password && (debug_flag > 1)) {
		if (!pairfind(request->config_items, PW_CLEARTEXT_PASSWORD, 0, TAG_ANY) &&
		    !pairfind(request->config_items, PW_NT_PASSWORD, 0, TAG_ANY) &&
		    !pairfind(request->config_items, PW_USER_PASSWORD, 0, TAG_ANY) &&
		    !pairfind(request->config_items, PW_PASSWORD_WITH_HEADER, 0, TAG_ANY) &&
		    !pairfind(request->config_items, PW_CRYPT_PASSWORD, 0, TAG_ANY)) {
			RWDEBUG("No \"reference\" password added. Ensure the admin user has permission to "
				"read the password attribute");
			RWDEBUG("PAP authentication will *NOT* work with Active Directory (if that is what you "
				"were trying to configure)");
		}
       }
}

#if LDAP_SET_REBIND_PROC_ARGS == 3
/** Callback for OpenLDAP to rebind and chase referrals
 *
 * Called by OpenLDAP when it receives a referral and has to rebind.
 *
 * @param handle to rebind.
 * @param url to bind to.
 * @param request that triggered the rebind.
 * @param msgid that triggered the rebind.
 * @param ctx rlm_ldap configuration.
 */
static int rlm_ldap_rebind(LDAP *handle, LDAP_CONST char *url, UNUSED ber_tag_t request, UNUSED ber_int_t msgid,
			   void *ctx)
{
	ldap_rcode_t status;
	ldap_handle_t *conn = ctx;

	int ldap_errno;

	conn->referred = true;
	conn->rebound = true;	/* not really, but oh well... */
	rad_assert(handle == conn->handle);

	DEBUG("rlm_ldap (%s): Rebinding to URL %s", conn->inst->xlat_name, url);

	status = rlm_ldap_bind(conn->inst, NULL, &conn, conn->inst->admin_dn, conn->inst->password, false);
	if (status != LDAP_PROC_SUCCESS) {
		ldap_get_option(handle, LDAP_OPT_ERROR_NUMBER, &ldap_errno);

		return ldap_errno;
	}


	return LDAP_SUCCESS;
}
#endif

/** Create and return a new connection
 *
 * Create a new ldap connection and allocate memory for a new rlm_handle_t
 *
 * @param instance rlm_ldap instance.
 * @return A new connection handle or NULL on error.
 */
void *mod_conn_create(void *instance)
{
	ldap_rcode_t status;

	int ldap_errno, ldap_version;
	struct timeval tv;

	ldap_instance_t *inst = instance;
	LDAP *handle = NULL;
	ldap_handle_t *conn = NULL;

#ifdef HAVE_LDAP_INITIALIZE
	if (inst->is_url) {
		DEBUG("rlm_ldap (%s): Connecting to %s", inst->xlat_name, inst->server);

		ldap_errno = ldap_initialize(&handle, inst->server);
		if (ldap_errno != LDAP_SUCCESS) {
			LDAP_ERR("ldap_initialize failed: %s", ldap_err2string(ldap_errno));
			goto error;
		}
	} else
#endif
	{
		DEBUG("rlm_ldap (%s): Connecting to %s:%d", inst->xlat_name, inst->server, inst->port);

		handle = ldap_init(inst->server, inst->port);
		if (!handle) {
			LDAP_ERR("ldap_init() failed");
			goto error;
		}
	}

	/*
	 *	We now have a connection structure, but no actual TCP connection.
	 *
	 *	Set a bunch of LDAP options, using common code.
	 */
#define do_ldap_option(_option, _name, _value) \
	if (ldap_set_option(handle, _option, _value) != LDAP_OPT_SUCCESS) { \
		ldap_get_option(handle, LDAP_OPT_ERROR_NUMBER, &ldap_errno); \
		LDAP_ERR("Could not set %s: %s", _name, ldap_err2string(ldap_errno)); \
	}

	if (inst->ldap_debug) {
		do_ldap_option(LDAP_OPT_DEBUG_LEVEL, "ldap_debug", &(inst->ldap_debug));
	}

	/*
	 *	Leave "chase_referrals" unset to use the OpenLDAP default.
	 */
	if (inst->chase_referrals != 2) {
		if (inst->chase_referrals) {
			do_ldap_option(LDAP_OPT_REFERRALS, "chase_referrals", LDAP_OPT_ON);

			if (inst->rebind == true) {
#if LDAP_SET_REBIND_PROC_ARGS == 3
				ldap_set_rebind_proc(handle, rlm_ldap_rebind, inst);
#endif
			}
		} else {
			do_ldap_option(LDAP_OPT_REFERRALS, "chase_referrals", LDAP_OPT_OFF);
		}
	}

	memset(&tv, 0, sizeof(tv));
	tv.tv_sec = inst->net_timeout;

	do_ldap_option(LDAP_OPT_NETWORK_TIMEOUT, "net_timeout", &tv);

	do_ldap_option(LDAP_OPT_TIMELIMIT, "srv_timelimit", &(inst->srv_timelimit));

	ldap_version = LDAP_VERSION3;
	do_ldap_option(LDAP_OPT_PROTOCOL_VERSION, "ldap_version", &ldap_version);

#ifdef LDAP_OPT_X_KEEPALIVE_IDLE
	do_ldap_option(LDAP_OPT_X_KEEPALIVE_IDLE, "keepalive idle", &(inst->keepalive_idle));
#endif

#ifdef LDAP_OPT_X_KEEPALIVE_PROBES
	do_ldap_option(LDAP_OPT_X_KEEPALIVE_PROBES, "keepalive probes", &(inst->keepalive_probes));
#endif

#ifdef LDAP_OPT_X_KEEPALIVE_INTERVAL
	do_ldap_option(LDAP_OPT_X_KEEPALIVE_INTERVAL, "keepalive interval", &(inst->keepalive_interval));
#endif

#ifdef HAVE_LDAP_START_TLS
	/*
	 *	Set all of the TLS options
	 */

#  ifdef LDAP_OPT_X_TLS_NEWCTX
	{
		/* Always use the new TLS configuration context */
		int is_server = 0;
		do_ldap_option(LDAP_OPT_X_TLS_NEWCTX, "new TLS context", &is_server);

	}
#  endif

	if (inst->tls_mode) {
		do_ldap_option(LDAP_OPT_X_TLS, "tls_mode", &(inst->tls_mode));
	}

#  define maybe_ldap_option(_option, _name, _value) \
	if (_value) do_ldap_option(_option, _name, _value)

	maybe_ldap_option(LDAP_OPT_X_TLS_CACERTFILE, "ca_file", inst->tls_ca_file);
	maybe_ldap_option(LDAP_OPT_X_TLS_CACERTDIR, "ca_path", inst->tls_ca_path);


	/*
	 *	Set certificate options
	 */
	maybe_ldap_option(LDAP_OPT_X_TLS_CERTFILE, "certificate_file", inst->tls_certificate_file);
	maybe_ldap_option(LDAP_OPT_X_TLS_KEYFILE, "private_key_file", inst->tls_private_key_file);
	maybe_ldap_option(LDAP_OPT_X_TLS_RANDOM_FILE, "random_file", inst->tls_random_file);

#  ifdef LDAP_OPT_X_TLS_NEVER
	if (inst->tls_require_cert_str) {
		do_ldap_option(LDAP_OPT_X_TLS_REQUIRE_CERT, "require_cert", &inst->tls_require_cert);
	}
#  endif

	/*
	 *	And finally start the TLS code.
	 */
	if (inst->start_tls) {
		if (inst->port == 636) {
			WDEBUG("Told to Start TLS on LDAPS port this will probably fail, please correct the "
			       "configuration");
		}

		if (ldap_start_tls_s(handle, NULL, NULL) != LDAP_SUCCESS) {
			ldap_get_option(handle, LDAP_OPT_ERROR_NUMBER, &ldap_errno);

			LDAP_ERR("Could not start TLS: %s", ldap_err2string(ldap_errno));
			goto error;
		}
	}
#endif /* HAVE_LDAP_START_TLS */

	/*
	 *	Allocate memory for the handle.
	 */
	conn = talloc_zero(instance, ldap_handle_t);
	conn->inst = inst;
	conn->handle = handle;
	conn->rebound = false;
	conn->referred = false;

	status = rlm_ldap_bind(inst, NULL, &conn, inst->admin_dn, inst->password, false);
	if (status != LDAP_PROC_SUCCESS) {
		goto error;
	}

	return conn;

	error:
	if (handle) ldap_unbind_s(handle);
	if (conn) talloc_free(conn);

	return NULL;
}


/** Close and delete a connection
 *
 * Unbinds the LDAP connection, informing the server and freeing any memory, then releases the memory used by the
 * connection handle.
 *
 * @param instance rlm_ldap instance.
 * @param handle to destroy.
 * @return always indicates success.
 */
int mod_conn_delete(UNUSED void *instance, void *handle)
{
	ldap_handle_t *conn = handle;

	ldap_unbind_s(conn->handle);
	talloc_free(conn);

	return 0;
}


/** Gets an LDAP socket from the connection pool
 *
 * Retrieve a socket from the connection pool, or NULL on error (of if no sockets are available).
 *
 * @param inst rlm_ldap configuration.
 * @param request Current request (may be NULL).
 */
ldap_handle_t *rlm_ldap_get_socket(ldap_instance_t const *inst, REQUEST *request)
{
	ldap_handle_t *conn;

	conn = fr_connection_get(inst->pool);
	if (!conn) {
		REDEBUG("All ldap connections are in use");

		return NULL;
	}

	return conn;
}


/** Frees an LDAP socket back to the connection pool
 *
 * If the socket was rebound chasing a referral onto another server then we destroy it.
 * If the socket was rebound to another user on the same server, we let the next caller rebind it.
 *
 * @param inst rlm_ldap configuration.
 * @param conn to release.
 */
void rlm_ldap_release_socket(ldap_instance_t const *inst, ldap_handle_t *conn)
{
	/*
	 *	Could have already been free'd due to a previous error.
	 */
	if (!conn) return;

	/*
	 *	We chased a referral to another server.
	 *
	 *	This connection is no longer part of the pool which is connected to and bound to the configured server.
	 *	Close it.
	 *
	 *	Note that we do NOT close it if it was bound to another user.  Instead, we let the next caller do the
	 *	rebind.
	 */
	if (conn->referred) {
		fr_connection_del(inst->pool, conn);
		return;
	}

	fr_connection_release(inst->pool, conn);
	return;
}
