/*
 * Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

#include <net-snmp/net-snmp-config.h>

#define SYSLOG_NAMES

#include <limits.h>
#include <stdio.h>
#ifndef HAVE_PRIORITYNAMES
#include <errno.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/types.h>
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#include <stdarg.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#ifdef HAVE_PRIORITYNAMES
#include <sys/syslog.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/library/snmp_debug.h>        /* For this file's "internal" definitions */
#include <net-snmp/config_api.h>
#include <net-snmp/utilities.h>

#include <net-snmp/library/mib.h>
#include <net-snmp/library/snmp_api.h>
#include <net-snmp/library/snmp_assert.h>

#define SNMP_DEBUG_DISABLED           0
#define SNMP_DEBUG_ACTIVE             1
#define SNMP_DEBUG_EXCLUDED           2

#ifndef NETSNMP_NO_DEBUGGING

static int      dodebug = NETSNMP_ALWAYS_DEBUG;
int             debug_num_tokens = 0;
static int      debug_print_everything = 0;
#ifndef NETSNMP_DISABLE_DYNAMIC_LOG_LEVEL
static int      debug_log_level = LOG_DEBUG;
#else
#define debug_log_level LOG_DEBUG
#endif /* NETSNMP_DISABLE_DYNAMIC_LOG_LEVEL */

netsnmp_token_descr dbg_tokens[MAX_DEBUG_TOKENS];

/*
 * Number of spaces to indent debug output. Valid range is [0,INT_MAX]
 */
static int debugindent = 0;

int
debug_indent_get(void)
{
    return debugindent;
}

const char*
debug_indent(void)
{
#define SPACES "                                        " \
               "                                        "
    if ((sizeof(SPACES) - 1) < (unsigned int)debugindent) {
        snmp_log(LOG_ERR, "Too deep indentation for debug_indent. "
                 "Consider using \"%%*s\", debug_indent_get(), \"\" instead.");
        return SPACES;
    }
    return SPACES + sizeof(SPACES) - 1 - debugindent;
#undef SPACES
}

void
debug_indent_add(int amount)
{
    if (-debugindent <= amount && amount <= INT_MAX - debugindent)
	debugindent += amount;
    netsnmp_assert( debugindent >= 0 ); /* no negative indents */
}

NETSNMP_IMPORT void
debug_config_register_tokens(const char *configtoken, char *tokens);

void
debug_indent_reset(void)
{
    if (debugindent != 0)
        DEBUGMSGTL(("dump_indent","indent reset from %d\n", debugindent));
    debugindent = 0;
}

void
debug_config_register_tokens(const char *configtoken, char *tokens)
{
    debug_register_tokens(tokens);
}

NETSNMP_IMPORT void
debug_config_turn_on_debugging(const char *configtoken, char *line);

void
debug_config_turn_on_debugging(const char *configtoken, char *line)
{
    snmp_set_do_debugging(atoi(line));
}

#ifndef NETSNMP_DISABLE_DYNAMIC_LOG_LEVEL

void
netsnmp_set_debug_log_level(int val)
{
    if (val < LOG_EMERG)
        val = LOG_EMERG;
    else if (val > LOG_DEBUG)
        val = LOG_DEBUG;
    debug_log_level = val;
}

int
netsnmp_get_debug_log_level(void)
{
    return debug_log_level;
}

static void
debug_config_debug_log_level(const char *configtoken, char *line)
{
#if !HAVE_PRIORITYNAMES
    static const struct strval_s {
        const char *c_name;
        int         c_val;
    } prioritynames[] = {
        { "alert", LOG_ALERT },
        { "crit", LOG_CRIT },
        { "debug", LOG_DEBUG },
        { "emerg", LOG_EMERG },
        { "err", LOG_ERR },
        { "info", LOG_INFO },
        { "notice", LOG_NOTICE },
        { "warning", LOG_WARNING },
        { NULL, 0 }
    };
#endif
    int i = 0, len_l, len_p;

    len_l = strlen(line);
    for(;prioritynames[i].c_name;++i) {
        len_p = strlen(prioritynames[i].c_name);
        if ((len_p != len_l) ||
            (strcasecmp(line,prioritynames[i].c_name) != 0))
            continue;
        netsnmp_set_debug_log_level(prioritynames[i].c_val);
        return;
    }
    config_perror("unknown debug log level, using debug");
    netsnmp_set_debug_log_level(LOG_DEBUG);
}
#endif /* NETSNMP_DISABLE_DYNAMIC_LOG_LEVEL */

void
debug_register_tokens(const char *tokens)
{
    char           *newp, *cp;
    char           *st = NULL;
    int             status;

    if (tokens == NULL || *tokens == 0)
        return;

    newp = strdup(tokens);      /* strtok_r messes it up */
    if (!newp)
        return;
    cp = strtok_r(newp, DEBUG_TOKEN_DELIMITER, &st);
    while (cp) {
        if (strlen(cp) < MAX_DEBUG_TOKEN_LEN) {
            if (strcasecmp(cp, DEBUG_ALWAYS_TOKEN) == 0) {
                debug_print_everything = 1;
            } else if (debug_num_tokens < MAX_DEBUG_TOKENS) {
                if ('-' == *cp) {
                    ++cp;
                    status = SNMP_DEBUG_EXCLUDED;
                }
                else
                    status = SNMP_DEBUG_ACTIVE;
                dbg_tokens[debug_num_tokens].token_name = strdup(cp);
                dbg_tokens[debug_num_tokens++].enabled  = status;
                snmp_log(LOG_NOTICE, "registered debug token %s, %d\n", cp, status);
            } else {
                snmp_log(LOG_NOTICE, "Unable to register debug token %s\n", cp);
            }
        } else {
            snmp_log(LOG_NOTICE, "Debug token %s over length\n", cp);
        }
        cp = strtok_r(NULL, DEBUG_TOKEN_DELIMITER, &st);
    }
    free(newp);
}

/*
 * Print all registered tokens along with their current status
 */
void
debug_print_registered_tokens(void) {
    int i;

    snmp_log(LOG_INFO, "%d tokens registered :\n", debug_num_tokens);
    for (i=0; i<debug_num_tokens; i++) {
        snmp_log( LOG_INFO, "%d) %s : %d\n",
                 i, dbg_tokens [i].token_name, dbg_tokens [i].enabled);
    }
}


/*
 * Enable logs on a given token
 */
int
debug_enable_token_logs (const char *token) {
    int i;

    /* debugging flag is on or off */
    if (!dodebug)
        return SNMPERR_GENERR;

    if (debug_num_tokens == 0 || debug_print_everything) {
        /* no tokens specified, print everything */
        return SNMPERR_SUCCESS;
    } else {
        for(i=0; i < debug_num_tokens; i++) {
            if (dbg_tokens[i].token_name &&
                strncmp(dbg_tokens[i].token_name, token,
                        strlen(dbg_tokens[i].token_name)) == 0) {
                dbg_tokens[i].enabled = SNMP_DEBUG_ACTIVE;
                return SNMPERR_SUCCESS;
            }
        }
    }
    return SNMPERR_GENERR;
}

/*
 * Diable logs on a given token
 */
int
debug_disable_token_logs (const char *token) {
    int i;

    /* debugging flag is on or off */
    if (!dodebug)
        return SNMPERR_GENERR;

    if (debug_num_tokens == 0 || debug_print_everything) {
        /* no tokens specified, print everything */
        return SNMPERR_SUCCESS;
    } else {
        for(i=0; i < debug_num_tokens; i++) {
            if (strncmp(dbg_tokens[i].token_name, token, 
                  strlen(dbg_tokens[i].token_name)) == 0) {
                dbg_tokens[i].enabled = SNMP_DEBUG_DISABLED;
                return SNMPERR_SUCCESS;
            }
        }
    }
    return SNMPERR_GENERR;
}

/*
 * debug_is_token_registered(char *TOKEN):
 *
 * returns SNMPERR_SUCCESS
 * or SNMPERR_GENERR
 *
 * if TOKEN has been registered and debugging support is turned on.
 */
int
debug_is_token_registered(const char *token)
{
    int             i, rc;

    /*
     * debugging flag is on or off
     */
    if (!dodebug)
        return SNMPERR_GENERR;

    if (debug_num_tokens == 0 || debug_print_everything) {
        /*
         * no tokens specified, print everything
         */
        return SNMPERR_SUCCESS;
    }
    else
        rc = SNMPERR_GENERR; /* ! found = err */

    for (i = 0; i < debug_num_tokens; i++) {
        if (SNMP_DEBUG_DISABLED == dbg_tokens[i].enabled)
            continue;
        if (dbg_tokens[i].token_name &&
            strncmp(dbg_tokens[i].token_name, token,
                    strlen(dbg_tokens[i].token_name)) == 0) {
            if (SNMP_DEBUG_ACTIVE == dbg_tokens[i].enabled)
                return SNMPERR_SUCCESS; /* active */
            else
                return SNMPERR_GENERR; /* excluded */
        }
    }
    return rc;
}

void
debugmsg(const char *token, const char *format, ...)
{
    if (debug_is_token_registered(token) == SNMPERR_SUCCESS) {
	va_list         debugargs;

	va_start(debugargs, format);
	snmp_vlog(debug_log_level, format, debugargs);
	va_end(debugargs);
    }
}

void
debugmsg_oid(const char *token, const oid * theoid, size_t len)
{
    u_char         *buf = NULL;
    size_t          buf_len = 0, out_len = 0;

    if (sprint_realloc_objid(&buf, &buf_len, &out_len, 1, theoid, len)) {
        if (buf != NULL) {
            debugmsg(token, "%s", buf);
        }
    } else {
        if (buf != NULL) {
            debugmsg(token, "%s [TRUNCATED]", buf);
        }
    }

    if (buf != NULL) {
        free(buf);
    }
}

void
debugmsg_suboid(const char *token, const oid * theoid, size_t len)
{
    u_char         *buf = NULL;
    size_t          buf_len = 0, out_len = 0;
    int             buf_overflow = 0;

    netsnmp_sprint_realloc_objid(&buf, &buf_len, &out_len, 1,
                                 &buf_overflow, theoid, len);
    if(buf_overflow) {
        if (buf != NULL) {
            debugmsg(token, "%s [TRUNCATED]", buf);
        }
    } else {
        if (buf != NULL) {
            debugmsg(token, "%s", buf);
        }
    }

    if (buf != NULL) {
        free(buf);
    }
}

void
debugmsg_var(const char *token, netsnmp_variable_list * var)
{
    u_char         *buf = NULL;
    size_t          buf_len = 0, out_len = 0;

    if (var == NULL || token == NULL) {
        return;
    }

    if (sprint_realloc_variable(&buf, &buf_len, &out_len, 1,
                                var->name, var->name_length, var)) {
        if (buf != NULL) {
            debugmsg(token, "%s", buf);
        }
    } else {
        if (buf != NULL) {
            debugmsg(token, "%s [TRUNCATED]", buf);
        }
    }

    if (buf != NULL) {
        free(buf);
    }
}

void
debugmsg_oidrange(const char *token, const oid * theoid, size_t len,
                  size_t var_subid, oid range_ubound)
{
    u_char         *buf = NULL;
    size_t          buf_len = 0, out_len = 0, i = 0;
    int             rc = 0;

    if (var_subid == 0) {
        rc = sprint_realloc_objid(&buf, &buf_len, &out_len, 1, theoid,
                                  len);
    } else {
        char            tmpbuf[128];
        /* XXX - ? check for 0 == var_subid -1 ? */
        rc = sprint_realloc_objid(&buf, &buf_len, &out_len, 1, theoid,
                                  var_subid-1);  /* Adjust for C's 0-based array indexing */
        if (rc) {
            sprintf(tmpbuf, ".%" NETSNMP_PRIo "u--%" NETSNMP_PRIo "u",
                    theoid[var_subid - 1], range_ubound);
            rc = snmp_cstrcat(&buf, &buf_len, &out_len, 1, tmpbuf);
            if (rc) {
                for (i = var_subid; i < len; i++) {
                    sprintf(tmpbuf, ".%" NETSNMP_PRIo "u", theoid[i]);
                    if (!snmp_cstrcat(&buf, &buf_len, &out_len, 1, tmpbuf)) {
                        break;
                    }
                }
            }
        }
    }


    if (buf != NULL) {
        debugmsg(token, "%s%s", buf, rc ? "" : " [TRUNCATED]");
        free(buf);
    }
}

void
debugmsg_hex(const char *token, const u_char * thedata, size_t len)
{
    u_char         *buf = NULL;
    size_t          buf_len = 0, out_len = 0;

    if (sprint_realloc_hexstring
        (&buf, &buf_len, &out_len, 1, thedata, len)) {
        if (buf != NULL) {
            debugmsg(token, "%s", buf);
        }
    } else {
        if (buf != NULL) {
            debugmsg(token, "%s [TRUNCATED]", buf);
        }
    }

    if (buf != NULL) {
        free(buf);
    }
}

void
debugmsg_hextli(const char *token, const u_char * thedata, size_t len)
{
    char            buf[SPRINT_MAX_LEN], token2[SPRINT_MAX_LEN];
    u_char         *b3 = NULL;
    size_t          b3_len = 0, o3_len = 0;
    int             incr;
    sprintf(token2, "dumpx_%s", token);

    /*
     * XX tracing lines removed from this function DEBUGTRACE; 
     */
    DEBUGIF(token2) {
        for (incr = 16; len > 0; len -= incr, thedata += incr) {
            if ((int) len < incr) {
                incr = len;
            }
            /*
             * XXnext two lines were DEBUGPRINTINDENT(token);
             */
            sprintf(buf, "dumpx%s", token);
            debugmsg(buf, "%s: %*s", token2, debug_indent_get(), "");
            if (sprint_realloc_hexstring
                (&b3, &b3_len, &o3_len, 1, thedata, incr)) {
                if (b3 != NULL) {
                    debugmsg(token2, "%s", b3);
                }
            } else {
                if (b3 != NULL) {
                    debugmsg(token2, "%s [TRUNCATED]", b3);
                }
            }
            o3_len = 0;
        }
    }
    if (b3 != NULL) {
        free(b3);
    }
}

void
debugmsgtoken(const char *token, const char *format, ...)
{
    va_list         debugargs;

    va_start(debugargs, format);
    debugmsg(token, "%s: ", token);
    va_end(debugargs);
}

void
debug_combo_nc(const char *token, const char *format, ...)
{
    va_list         debugargs;

    va_start(debugargs, format);
    snmp_log(debug_log_level, "%s: ", token);
    snmp_vlog(debug_log_level, format, debugargs);
    va_end(debugargs);
}

/*
 * for speed, these shouldn't be in default_storage space 
 */
void
snmp_set_do_debugging(int val)
{
    dodebug = val;
}

int
snmp_get_do_debugging(void)
{
    return dodebug;
}

void
snmp_debug_shutdown(void)
{
    int i;

    for (i = 0; i < debug_num_tokens; i++)
       SNMP_FREE(dbg_tokens[i].token_name);
}

#else /* ! NETSNMP_NO_DEBUGGING */

int debug_indent_get(void) { return 0; }

const char* debug_indent(void) { return ""; }

void debug_indent_add(int amount)
{ }

NETSNMP_IMPORT void
debug_config_register_tokens(const char *configtoken, char *tokens);

void
debug_indent_reset(void)
{ }

void
debug_config_register_tokens(const char *configtoken, char *tokens)
{ }

#ifndef NETSNMP_DISABLE_DYNAMIC_LOG_LEVEL
static void
debug_config_debug_log_level(const char *configtoken NETSNMP_ATTRIBUTE_UNUSED,
                             char *tokens NETSNMP_ATTRIBUTE_UNUSED)
{ }

NETSNMP_IMPORT void
netsnmp_set_debug_log_level(int val NETSNMP_ATTRIBUTE_UNUSED)
{ }

NETSNMP_IMPORT int
netsnmp_get_debug_log_level(void)
{
    return 0;
}
#endif /* NETSNMP_DISABLE_DYNAMIC_LOG_LEVEL */

NETSNMP_IMPORT void
debug_config_turn_on_debugging(const char *configtoken, char *line);

void
debug_config_turn_on_debugging(const char *configtokenu, char *line)
{ }

void
debug_register_tokens(const char *tokens)
{ }

void
debug_print_registered_tokens(void)
{ }


int
debug_enable_token_logs (const char *token)
{
    return SNMPERR_GENERR;
}

int
debug_disable_token_logs (const char *token)
{
    return SNMPERR_GENERR;
}

int
debug_is_token_registered(const char *token)
{
    return SNMPERR_GENERR;
}

void
debugmsg(const char *token, const char *format, ...)
{ }

void debugmsg_oid(const char *token, const oid * theoid, size_t len)
{ }

void
debugmsg_suboid(const char *token, const oid * theoid, size_t len)
{ }

void
debugmsg_var(const char *token, netsnmp_variable_list * var)
{ }

void
debugmsg_oidrange(const char *token, const oid * theoid, size_t len,
                  size_t var_subid, oid range_ubound)
{ }

void
debugmsg_hex(const char *token, const u_char * thedata, size_t len)
{ }

void
debugmsg_hextli(const char *token, const u_char * thedata, size_t len)
{ }

void
debugmsgtoken(const char *token, const char *format, ...)
{ }

void
debug_combo_nc(const char *token, const char *format, ...)
{ }

void
snmp_set_do_debugging(int val)
{ }

int
snmp_get_do_debugging(void)
{
    return 0;
}

void
snmp_debug_shutdown(void)
{  }

#endif /* NETSNMP_NO_DEBUGGING */

void
snmp_debug_init(void)
{
    register_prenetsnmp_mib_handler("snmp", "doDebugging",
                                    debug_config_turn_on_debugging, NULL,
                                    "(1|0)");
    register_prenetsnmp_mib_handler("snmp", "debugTokens",
                                    debug_config_register_tokens, NULL,
                                    "token[,token...]");
#ifndef NETSNMP_DISABLE_DYNAMIC_LOG_LEVEL
    register_prenetsnmp_mib_handler("snmp", "debugLogLevel",
                                    debug_config_debug_log_level, NULL,
                                    "(emerg|alert|crit|err|warning|notice|info|debug)");
#endif /* NETSNMP_DISABLE_DYNAMIC_LOG_LEVEL */
}

