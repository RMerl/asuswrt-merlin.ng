/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */


/*******************************************************************
 * bdmf_mon.c
 *
 * BL framework - monitor
 *
 *******************************************************************/
#include <stdarg.h>
#include <bdmf_system.h>
#define BDMF_INTERNAL
#include <bdmf_shell.h>

#define BDMFMON_INBUF_LEN          2048
#define BDMFMON_MAX_QUAL_NAME_LENGTH 256
#define BDMFMON_MAX_PARMS          128
#define BDMFMON_UP_STR             ".."
#define BDMFMON_ROOT_STR           "/"
#define BDMFMON_COMMENT_CHAR       '#'
#define BDMFMON_HELP_CHAR          '?'
#define BDMFMON_ARRAY_DELIM_CHAR   ','
#define BDMFMON_ROOT_HELP          "root directory"
#define BDMFMON_MAX_PARM_VAL_LEN   256
#define BDMFMON_ARRAY_NO_VALUE     "-"
#define BDMFMON_HELP_BUFFER_SIZE   16384

#define BDMFMON_EQUAL_CHAR         ':'
#define BDMFMON_EQUAL_CHAR_STR     ":"


typedef enum { BDMFMON_ENTRY_DIR, BDMFMON_ENTRY_CMD } bdmfmon_entry_selector_t;

/* External table - boolean values */
bdmfmon_enum_val_t bdmfmon_enum_bool_table[] = {
    { .name="true", .val=1},
    { .name="yes", .val=1},
    { .name="on", .val=1},
    { .name="false", .val=0},
    { .name="no", .val=0},
    { .name="off", .val=0},
    BDMFMON_ENUM_LAST
};

/* Monitor token structure */
struct bdmfmon_entry
{
    struct bdmfmon_entry  *next;
    char *name;                                  /* Command/directory name */
    char *help;                                  /* Command/directory help */
    bdmfmon_entry_selector_t sel;                /* Entry selector */
    char *alias;                                 /* Alias */
    uint16_t alias_len;                          /* Alias length */
    struct bdmfmon_entry *parent;                /* Parent directory */
    bdmf_access_right_t access_right;

    union {
        struct
        {
            struct bdmfmon_entry *first;            /* First entry in directory */
            const bdmfmon_dir_extra_parm_t *extras; /* Optional extras */
        } dir;
        struct
        {
            bdmfmon_cmd_cb_t cmd_cb;                /* Command callback */
            bdmfmon_cmd_parm_t *parms;              /* Command parameters */
            const bdmfmon_cmd_extra_parm_t *extras; /* Optional extras */
            uint16_t num_parms;
        } cmd;
    } u;
};


/* Token types */
typedef enum
{
    BDMFMON_TOKEN_EMPTY,
    BDMFMON_TOKEN_UP,
    BDMFMON_TOKEN_ROOT,
    BDMFMON_TOKEN_BREAK,
    BDMFMON_TOKEN_HELP,
    BDMFMON_TOKEN_NAME,
    BDMFMON_TOKEN_VALUE,
} bdmfmon_token_type_t;


/* CLI session data */
typedef struct bdmfmon_session_data
{
    bdmfmon_handle_t curdir;
    bdmfmon_handle_t curcmd;
    bdmfmon_cmd_parm_t cmd_parms[BDMFMON_MAX_PARMS];
    bdmf_session_handle session;
    uint16_t num_parms;
    char *p_inbuf;
    int stop_monitor;
    char inbuf[BDMFMON_INBUF_LEN];
} bdmfmon_session_extras_t;

/* Name, value pairs */
typedef struct bdmfmon_name_value
{
    bdmfmon_token_type_t type;
    const char *name;
    const char *value;
} bdmfmon_name_value_t;

/* help session control block */
typedef struct bdmfmon_help_scratchpad
{
    uint32_t pos;
    /* Followed by data */
} bdmfmon_help_scratchpad_t;

static bdmfmon_handle_t          bdmfmon_root_dir;
static bdmfmon_session_extras_t  *bdmfmon_root_session;

#define BDMFMON_MIN_NAME_LENGTH_FOR_ALIAS   3
#define BDMFMON_ROOT_NAME       "/"

/* Internal functions */
static void        _bdmfmon_alloc_root(const bdmf_session_parm_t *parm);
static void        _bdmfmon_display_dir(bdmfmon_session_extras_t *mon_session, bdmfmon_handle_t p_dir );
static bdmfmon_token_type_t _bdmfmon_get_word(bdmfmon_session_extras_t *session, char **inbuf, char **p_word);
static bdmfmon_token_type_t _bdmfmon_analyze_token( const char *name );
static int         _bdmfmon_parse_parms( bdmfmon_session_extras_t *mon_session, bdmfmon_handle_t p_token,
    bdmfmon_name_value_t *pairs, int npairs);
static int _bdmfmon_extend_parms( bdmfmon_session_extras_t *mon_session, bdmfmon_name_value_t *pairs,
    int npairs, int last_is_space, char *insert_str, uint32_t insert_size);
static bdmfmon_handle_t _bdmfmon_search_token( bdmfmon_handle_t p_dir, const char *name );
static void        _bdmfmon_help_dir( bdmfmon_session_extras_t *mon_session, bdmfmon_handle_t p_dir );
static void        _bdmfmon_help_entry(bdmfmon_session_extras_t *mon_session, bdmfmon_handle_t p_token,
    bdmfmon_name_value_t *pairs, int npairs, int suppress_err_print);
static void        _bdmfmon_help_populated_cmd(bdmfmon_session_extras_t *mon_session, bdmfmon_handle_t p_token,
    const char *partial_match, int suppress_assigned);
static void        _bdmfmon_choose_alias( bdmfmon_handle_t p_dir, bdmfmon_handle_t p_new_token );
static bdmfmon_cmd_parm_t *_bdmfmon_find_named_parm(bdmfmon_session_extras_t *mon_session, const char *name);
static bdmfmon_cmd_parm_t *_bdmfmon_find_parm_by_prefix(bdmfmon_session_extras_t *mon_session, const char *prefix);
static char       *_bdmfmon_strlwr( char *s );
static int         _bdmfmon_stricmp( const char *s1, const char *s2, int len );
static int _bdmfmon_dft_scan_cb(const bdmfmon_cmd_parm_t *parm, bdmfmon_parm_value_t *value, const char *string_val);
static const char *_bdmfmon_get_type_name(const bdmfmon_cmd_parm_t *parm);
static void        _bdmfmon_dft_format_cb(const bdmfmon_cmd_parm_t *parm, bdmfmon_parm_value_t value, char *buffer, int size);
static int _bdmfmon_enum_scan_cb(const bdmfmon_cmd_parm_t *parm, bdmfmon_parm_value_t *value, const char *string_val);
static void        _bdmfmon_enum_format_cb(const bdmfmon_cmd_parm_t *parm, bdmfmon_parm_value_t value, char *buffer, int size);
static const char *_bdmfmon_qualified_name( bdmfmon_handle_t token, char *buffer, int size);
static int _bdmfmon_split(bdmfmon_session_extras_t *mon_session, bdmfmon_name_value_t **pairs, int *npairs);
static void        _bdmfmon_assign_callbacks(bdmfmon_cmd_parm_t *parm);

static inline bdmfmon_session_extras_t *_bdmfmon_session_data(bdmf_session_handle session)
{
    if (!session)
        return bdmfmon_root_session;
    return bdmf_session_data(session);
}

/** Add subdirectory to the parent directory
 *
 * \param[in]   parent          Parent directory handle. NULL=root
 * \param[in]   name            Directory name
 * \param[in]   help            Help string
 * \param[in]   access_right    Access rights
 * \param[in]   extras          Optional directory descriptor. Mustn't be allocated on the stack.
 * \return      new directory handle or NULL in case of failure
 */
bdmfmon_handle_t bdmfmon_dir_add(bdmfmon_handle_t parent, const char *name,
                             const char *help, bdmf_access_right_t access_right,
                             const bdmfmon_dir_extra_parm_t *extras)
{
    bdmfmon_handle_t p_dir;
    bdmfmon_handle_t *p_e;

    assert(name);
    assert(help);
    if (!name || !help)
        return NULL;

    if (!bdmfmon_root_dir)
    {
        _bdmfmon_alloc_root(NULL);
        if (!bdmfmon_root_dir)
            return NULL;
    }

    if (!parent)
        parent = bdmfmon_root_dir;

    p_dir=(bdmfmon_handle_t )bdmf_calloc(sizeof(struct bdmfmon_entry) + strlen(name) + strlen(help) + 2 );
    if ( !p_dir )
        return NULL;

    p_dir->name = (char *)(p_dir + 1);
    strcpy( p_dir->name, name);
    p_dir->help = p_dir->name + strlen(name) + 1;
    strcpy(p_dir->help, help);
    p_dir->sel = BDMFMON_ENTRY_DIR;
    _bdmfmon_choose_alias( parent, p_dir );
    p_dir->access_right = access_right;
    p_dir->u.dir.extras = extras;

    /* Add new directory to the parent's list */
    p_dir->parent = parent;
    p_e = &(parent->u.dir.first);
    while (*p_e)
        p_e = &((*p_e)->next);
    *p_e = p_dir;

    return p_dir;
}

static bdmfmon_handle_t  find_entry_in_dir( bdmfmon_handle_t dir, const char *name,
        bdmfmon_entry_selector_t type, uint16_t recursive_search)
{
    bdmfmon_handle_t p1, p;

    if ( !dir )
    {
        dir = bdmfmon_root_dir;
        if (!dir)
            return NULL;
    }
    p = dir->u.dir.first;
    while (p)
    {
        if ( !_bdmfmon_stricmp(p->name, name, -1) && type == p->sel )
            return p;
        if ( recursive_search && p->sel == BDMFMON_ENTRY_DIR )
        {
            p1 = find_entry_in_dir(p, name , type, 1 );
            if ( p1 )
                return p1;
        }
        p = p->next;
    }
    return NULL;
}


/* Scan directory tree and look for directory with name starts from
 * root directory with name root_name
 */
bdmfmon_handle_t bdmfmon_dir_find(bdmfmon_handle_t parent, const char  *name)
{
    if ( !parent )
        parent = bdmfmon_root_dir;
    return find_entry_in_dir(parent, name, BDMFMON_ENTRY_DIR, 0 );
}


/* Scan directory tree and look for command named "name". */
bdmfmon_handle_t bdmfmon_cmd_find(bdmfmon_handle_t parent, const char *name )
{
    if ( !parent )
        parent = bdmfmon_root_dir;
    return find_entry_in_dir(parent, name, BDMFMON_ENTRY_CMD, 0 );
}


/** Add CLI command
 *
 * \param[in]   dir             Handle of directory to add command to. NULL=root
 * \param[in]   name            Command name
 * \param[in]   cmd_cb          Command handler
 * \param[in]   help            Help string
 * \param[in]   access_right    Access rights
 * \param[in]   extras          Optional extras
 * \param[in]   parms           Optional parameters array. Must not be allocated on the stack!
 *                              If parms!=NULL, the last parameter in the array must have name==NULL.
 * \return
 *      0   =OK\n
 *      <0  =error code
 */
int bdmfmon_cmd_add(bdmfmon_handle_t dir, const char *name, bdmfmon_cmd_cb_t cmd_cb,
                  const char *help, bdmf_access_right_t access_right,
                  const bdmfmon_cmd_extra_parm_t *extras, bdmfmon_cmd_parm_t parms[])
{
    bdmfmon_handle_t p_token;
    bdmfmon_handle_t *p_e;
    uint16_t       i;
    bdmfmon_cmd_parm_t *parm = parms;

    assert(name);
    assert(help);
    assert(cmd_cb);
    if (!name || !cmd_cb || !help)
        return BDMF_ERR_PARM;

    if (!bdmfmon_root_dir)
    {
        _bdmfmon_alloc_root(NULL);
        if (!bdmfmon_root_dir)
            return BDMF_ERR_NOMEM;
    }

    if (!dir)
        dir = bdmfmon_root_dir;

    p_token=(bdmfmon_handle_t )bdmf_calloc( sizeof(struct bdmfmon_entry) + strlen(name) + strlen(help) + 2 );
    if ( !p_token )
        return BDMF_ERR_NOMEM;

    /* Copy name */
    p_token->name = (char *)(p_token + 1);
    strcpy( p_token->name, name );
    p_token->help = p_token->name + strlen(name) + 1;
    strcpy(p_token->help, help);
    p_token->sel = BDMFMON_ENTRY_CMD;
    p_token->u.cmd.cmd_cb = cmd_cb;
    p_token->u.cmd.parms = parms;
    p_token->u.cmd.extras = extras;
    p_token->access_right = access_right;

    /* Convert name to lower case and choose alias */
    _bdmfmon_choose_alias(dir, p_token );


    /* Check parameters */
    for (i = 0; i < BDMFMON_MAX_PARMS && parms && parms[i].name; i++)
    {
        parm = &parms[i];
        /* Pointer parameter must have an address */
        if ((parm->type==BDMFMON_PARM_USERDEF) && !parm->scan_cb)
        {
            bdmf_print("MON: %s> scan_cb callback must be set for user-defined parameter %s\n", name, parm->name);
            goto cmd_add_error;
        }
        if (parm->type==BDMFMON_PARM_ENUM)
        {
            if (!parm->enum_table)
            {
                bdmf_print("MON: %s> value table must be set in low_val for enum parameter %s\n", name, parm->name);
                goto cmd_add_error;
            }

            /* Check default value if any */
            if ((parm->flags & BDMFMON_PARM_FLAG_DEFVAL))
            {
                if (_bdmfmon_enum_scan_cb(parm, &parm->value, parm->value.string) < 0)
                {
                    bdmf_print("MON: %s> default value %s doesn't match any value of enum parameter %s\n", name, parm->value.string, parm->name);
                    goto cmd_add_error;
                }
            }
            else if ((parm->flags & BDMFMON_PARM_FLAG_OPTIONAL))
            {
                /* Optional enum parameters are initialized by their 1st value by default.
                 * All other parameters are initialized to 0.
                 */
                bdmfmon_enum_val_t *values=parm->enum_table;
                parm->value.number = values[0].val;
            }
        }
        if (parm->max_array_size)
        {
            if (!parm->values)
            {
                bdmf_print("MON: %s> parm->values must be set for parameter-array %s\n", name, parm->name);
                goto cmd_add_error;
            }
        }
        _bdmfmon_assign_callbacks(parm);
    }
    if ((i == BDMFMON_MAX_PARMS) && parms[i].name[0])
    {
        bdmf_print("MON: %s> too many parameters\n", name);
        goto cmd_add_error;
    }
    p_token->u.cmd.num_parms = i;

    /* Add token to the directory */
    p_token->parent = dir;
    p_e = &(dir->u.dir.first);
    while (*p_e)
        p_e = &((*p_e)->next);
    *p_e = p_token;

    return 0;

cmd_add_error:
    bdmf_free( p_token );
    return BDMF_ERR_PARM;
}


/** Destroy token (command or directory)
 * \param[in]   token           Directory or command token. NULL=root
 */
void bdmfmon_token_destroy(bdmfmon_handle_t token)
{
    if (!token)
    {
        if (!bdmfmon_root_dir)
            return;
        token = bdmfmon_root_dir;
    }
    /* Remove from parent's list */
    if (token->parent)
    {
        bdmfmon_handle_t *p_e;
        p_e = &(token->parent->u.dir.first);
        while (*p_e)
        {
            if (*p_e == token)
            {
                *p_e = token->next;
                break;
            }
            p_e = &((*p_e)->next);
        }
    }

    /* Remove all directory entries */
    if (token->sel == BDMFMON_ENTRY_DIR)
    {
        bdmfmon_handle_t e = token->u.dir.first;
        while((e = token->u.dir.first))
            bdmfmon_token_destroy(e);
    }
    else if (token->u.cmd.extras && token->u.cmd.extras->free_parms)
        token->u.cmd.extras->free_parms(token->u.cmd.parms);

    /* Release the token */
    bdmf_free(token);

    if (token == bdmfmon_root_dir)
    {
        bdmfmon_root_dir = NULL;
        if (bdmfmon_root_session)
        {
            bdmf_session_close(bdmfmon_root_session->session);
            bdmfmon_root_session = NULL;
        }
    }
}

/** Open monitor session
 *
 * Monitor supports multiple simultaneous sessions with different
 * access rights.
 * Note that there already is a default session with full administrative rights,
 * that takes input from stdin and outputs to stdout.
 * \param[in]   parm        Session parameters. Must not be allocated on the stack.
 * \param[out]  p_session   Session handle
 * \return
 *      0   =OK\n
 *      <0  =error code
 */
int bdmfmon_session_open(const bdmf_session_parm_t *parm, bdmf_session_handle *p_session)
{
    bdmf_session_handle session;
    bdmfmon_session_extras_t *mon_session;
    bdmf_session_parm_t session_parms;
    int rc;

    assert(p_session);
    if (!p_session)
        return BDMF_ERR_PARM;
    
    if (!bdmfmon_root_dir)
    {
        _bdmfmon_alloc_root(parm);
        if (!bdmfmon_root_dir)
            return BDMF_ERR_NOMEM;
    }
    if (parm)
        session_parms = *parm;
    else
    {
        memset(&session_parms, 0, sizeof(session_parms));
        session_parms.name = "unnamed";
    }

    /* Open comm session */
    session_parms.extra_size = sizeof(bdmfmon_session_extras_t);
    rc = bdmf_session_open(&session_parms, &session);
    if (rc)
        return rc;
    mon_session = _bdmfmon_session_data(session);
    mon_session->curdir = bdmfmon_root_dir;
    mon_session->session = session;

    *p_session = session;

    return 0;
}

/** Close monitor session.
 * \param[in]   session         Session handle
 */
void bdmfmon_session_close(bdmf_session_handle session )
{
    bdmf_session_close(session);
}

#define BDMFMON_PARSE_RETURN(ret) \
    do { \
        rc = ret;   \
        goto bdmfmon_parse_out; \
    } while (0)

/* Parse a single command. Stop on ';' or EOL */
static int bdmfmon_parse_command(bdmf_session_handle session)
{
    bdmfmon_session_extras_t *mon_session=_bdmfmon_session_data(session);
    bdmfmon_handle_t  p_token;
    bdmfmon_name_value_t *pairs;
    int stop_parsing = 0;
    int npairs;
    int i;
    int rc = BDMF_ERR_OK;

    /* Split string to name/value pairs */
    rc = _bdmfmon_split(mon_session, &pairs, &npairs);
    if (rc)
    {
        if (rc == BDMF_ERR_NOENT)
        {
            return BDMF_ERR_OK;
        }
        return rc;
    }

    /* Interpret empty string as "display directory" */
    if ( !npairs )
    {
        _bdmfmon_display_dir(mon_session, mon_session->curdir );
        BDMFMON_PARSE_RETURN(0);
    }


    /* Identify parameters */
    for (i=0; i<npairs && !rc && !stop_parsing; i++)
    {
        switch (pairs[i].type)
        {
        case BDMFMON_TOKEN_NAME:
        case BDMFMON_TOKEN_VALUE:
            /* Identify command. The 1st pair can't contain name, only value */
            if (pairs[i].name)
            {
                bdmf_session_print(session, "**ERR: %s is unexpected\n", pairs[i].name);
                BDMFMON_PARSE_RETURN(BDMF_ERR_PARM);
            }
            p_token = _bdmfmon_search_token(mon_session->curdir, pairs[i].value);
            if (p_token == NULL)
            {
                bdmf_session_print(session, "**ERR: %s is unexpected\n", pairs[i].value);
                BDMFMON_PARSE_RETURN(BDMF_ERR_PARM);
            }
            /* Directory or command ? */
            if (p_token->sel == BDMFMON_ENTRY_DIR)
            {
                mon_session->curdir = p_token;
                _bdmfmon_display_dir(mon_session, mon_session->curdir );
            }
            else
            {
                /* Function token */
                mon_session->curcmd = p_token;
                if (_bdmfmon_parse_parms(mon_session, p_token, &pairs[i+1], npairs-i-1) < 0)
                {
                    _bdmfmon_help_entry(mon_session, p_token, &pairs[i+1], npairs-i-1, 1);
                    rc = BDMF_ERR_PARM;
                }
                else
                {
                    rc = p_token->u.cmd.cmd_cb(session, mon_session->cmd_parms, npairs-i-1 );
                    if (rc)
                    {
                        char buffer[BDMFMON_MAX_QUAL_NAME_LENGTH];
                        bdmf_session_print(session, "MON: %s> failed with error code %s(%d)\n",
                            _bdmfmon_qualified_name(p_token, buffer, sizeof(buffer)),
                                         bdmf_strerror(rc), rc);
                    }
                }
                stop_parsing = 1;
            }
            break;

        case BDMFMON_TOKEN_UP: /* Go to upper directory */
            if (mon_session->curdir->parent)
                mon_session->curdir = mon_session->curdir->parent;
            _bdmfmon_display_dir(mon_session, mon_session->curdir );
            break;

        case BDMFMON_TOKEN_ROOT: /* Go to the root directory */
            mon_session->curdir = bdmfmon_root_dir;
            _bdmfmon_display_dir(mon_session, mon_session->curdir );
            break;

        case BDMFMON_TOKEN_HELP: /* Display help */
            if (i < npairs-1 &&
                ((p_token = _bdmfmon_search_token( mon_session->curdir, pairs[i+1].value)) != NULL ))
            {
                _bdmfmon_help_entry(mon_session, p_token, &pairs[i+2], npairs-i-2, 0);
            }
            else
            {
                _bdmfmon_help_dir(mon_session, mon_session->curdir);
            }
            stop_parsing = 1;
            break;

        default:
            stop_parsing = 1;
            break;
        }
    }

bdmfmon_parse_out:
    if (pairs)
        bdmf_free(pairs);
    return rc;

}

/** Context extension */
int bdmf_extend(bdmf_session_handle session, char *input_str, char *insert_str, uint32_t insert_size)
{
    bdmfmon_session_extras_t *mon_session=_bdmfmon_session_data(session);
    bdmfmon_handle_t p_token;
    bdmfmon_name_value_t *pairs;
    int last_is_space;
    int npairs;
    int rc = BDMF_ERR_OK;

    if (!mon_session || !mon_session->curdir || !input_str)
        return BDMF_ERR_PARM;

    insert_str[0] = 0;
    mon_session->p_inbuf = input_str;

    last_is_space = strlen(input_str) && (input_str[strlen(input_str) - 1] == ' ');

    /* Split string to name/value pairs */
    rc = _bdmfmon_split(mon_session, &pairs, &npairs);
    if (rc)
        return rc;

    /* empty list - display list of commands */
    if ( !npairs )
    {
        _bdmfmon_display_dir(mon_session, mon_session->curdir );
        BDMFMON_PARSE_RETURN(0);
    }

    /* Identify parameters */
    switch (pairs[0].type)
    {
    case BDMFMON_TOKEN_NAME:
    case BDMFMON_TOKEN_VALUE:
        /* Identify command. The 1st pair can't contain name, only value */
        if (pairs[0].name ||
            !(p_token = _bdmfmon_search_token(mon_session->curdir, pairs[0].value)))
        {
            _bdmfmon_display_dir(mon_session, mon_session->curdir );
            BDMFMON_PARSE_RETURN(BDMF_ERR_PARM);
        }

        /* Directory or command ? */
        if (p_token->sel != BDMFMON_ENTRY_CMD)
            BDMFMON_PARSE_RETURN(BDMF_ERR_OK);

        /* Function token */
        mon_session->curcmd = p_token;
        rc = _bdmfmon_extend_parms(mon_session, &pairs[1], npairs-1, last_is_space, insert_str, insert_size);
        break;

    default:
        break;
    }

bdmfmon_parse_out:
    bdmf_free(pairs);
    return rc;
}

/** Parse and execute input string.
 * input_string can contain multiple commands delimited by ';'
 *
 * \param[in]   session         Session handle
 * \param[in]   input_string    String to be parsed. May consist of multiple ';'-delimited commands
 * \return
 *      =0  - OK \n
 *      BDMF_ERR_PARM - parsing error\n
 *      other - return code - as returned from command handler.
 *            It is recommended to return -EINTR to interrupt monitor loop.
 */
int bdmfmon_parse(bdmf_session_handle session, char* input_string)
{
    bdmfmon_session_extras_t *mon_session=_bdmfmon_session_data(session);
    uint32_t input_len;
    int rc = 0;

    if (!mon_session || !mon_session->curdir || !input_string)
        return BDMF_ERR_PARM;
    input_len = strlen(input_string);
    if (!input_len)
        return 0;

    /* cut CR, LF if any */
    while (input_len && (input_string[input_len-1]=='\n' || input_string[input_len-1]=='\r'))
        input_string[--input_len]=0;

    mon_session->p_inbuf = input_string;
    mon_session->stop_monitor = 0;

    do {
        rc = bdmfmon_parse_command(session);
    } while (mon_session->p_inbuf && mon_session->p_inbuf[0] && !mon_session->stop_monitor && !rc);

    return rc;
}

/** Read input and parse iteratively until EOF or bdmfmon_is_stopped()
 *
 * \param[in]   session         Session handle
 * \return
 *      =0  - OK \n
 */
int bdmfmon_driver(bdmf_session_handle session)
{
    bdmfmon_session_extras_t *mon_session=_bdmfmon_session_data(session);

    while(!bdmfmon_is_stopped(session) &&
          bdmf_session_gets(session, mon_session->inbuf, sizeof(mon_session->inbuf)-1))
    {
        bdmfmon_parse(session, mon_session->inbuf);
    }

    return BDMF_ERR_OK;
}

/* Stop monitor driver */
void bdmfmon_stop( bdmf_session_handle session )
{
    bdmfmon_session_extras_t *mon_session=_bdmfmon_session_data(session);
    assert(mon_session);
    mon_session->stop_monitor = 1;
}

/** Returns 1 if monitor session is stopped
 * \param[in]   session         Session handle
 * \returns 1 if monitor session stopped by bdmfmon_stop()\n
 * 0 otherwise
 */
int bdmfmon_is_stopped(bdmf_session_handle session)
{
    bdmfmon_session_extras_t *mon_session=_bdmfmon_session_data(session);
    return mon_session->stop_monitor;
}


/** Get parameter number given its name.
 * The function is intended for use by command handlers
 * \param[in]       session         Session handle
 * \param[in,out]   parm_name       Parameter name
 * \return
 *  >=0 - parameter number\n
 *  <0  - parameter with this name doesn't exist
 */
int bdmfmon_parm_number(bdmf_session_handle session, const char *parm_name)
{
    bdmfmon_session_extras_t *mon_session=_bdmfmon_session_data(session);
    int i;
    if ( !parm_name || !mon_session || !mon_session->curcmd)
        return BDMF_ERR_PARM;
    for(i=0;
        mon_session->cmd_parms[i].name &&
            _bdmfmon_stricmp( parm_name, mon_session->cmd_parms[i].name, -1);
        i++)
        ;
    if (!mon_session->cmd_parms[i].name)
        return BDMF_ERR_PARM;
    return i;
}


/** Get parameter by name
 * The function is intended for use by command handlers
 * \param[in]       session         Session handle
 * \param[in]       parm_name       Parameter name
 * \return
 * parameter pointer or NULL if not found
 */
bdmfmon_cmd_parm_t *bdmfmon_parm_get(bdmf_session_handle session, const char *parm_name)
{
    bdmfmon_session_extras_t *mon_session=_bdmfmon_session_data(session);
    int nparm = bdmfmon_parm_number(session, parm_name);
    if (nparm < 0)
    {
        return NULL;
    }
    return &mon_session->cmd_parms[nparm];
}


/** Check if parameter is set
 * \param[in]       session         Session handle
 * \param[in]       parm_number     Parameter number
 * \return
 *  1 if parameter is set\n
 *  0 if parameter is not set or parm_number is invalid
 */
int bdmfmon_parm_check(bdmf_session_handle session, int parm_number)
{
    bdmfmon_session_extras_t *mon_session=_bdmfmon_session_data(session);

    if (parm_number < 0 || !mon_session || !mon_session->curcmd)
        return BDMF_ERR_PARM;
    if (parm_number >= mon_session->num_parms)
        return BDMF_ERR_PARM;
    if (!(mon_session->cmd_parms[parm_number].flags & BDMFMON_PARM_FLAG_ASSIGNED))
        return BDMF_ERR_NOENT;
    return BDMF_ERR_OK;
}


/** Get enum's string value given its internal value
 * \param[in]       session         Session handle
 * \param[in]       parm_number     Parameter number
 * \param[in]       value           Internal value
 * \return
 *      enum string value or NULL if parameter is not enum or
 *      internal value is invalid
 */
const char *bdmfmon_enum_parm_stringval(bdmf_session_handle session, int parm_number, long value)
{
    bdmfmon_session_extras_t *mon_session=_bdmfmon_session_data(session);
    int i;
    bdmfmon_enum_val_t *values;
    if (parm_number < 0 || !mon_session || !mon_session->curcmd)
        return NULL;
    for(i=0; i<parm_number && mon_session->cmd_parms[i].name; i++)
        ;
    if (i < parm_number)
        return NULL;
    if (mon_session->cmd_parms[parm_number].type != BDMFMON_PARM_ENUM)
        return NULL;
    values = mon_session->cmd_parms[parm_number].enum_table;
    return bdmfmon_enum_stringval(values, value);
}


/* Get current directory handle */
bdmfmon_handle_t bdmfmon_dir_get(bdmf_session_handle session)
{
    bdmfmon_session_extras_t *mon_session=_bdmfmon_session_data(session);
    if (!mon_session)
        return NULL;
    return mon_session->curdir;
}

/* Set current directory */
int bdmfmon_dir_set(bdmf_session_handle session, bdmfmon_handle_t dir)
{
    bdmfmon_session_extras_t *mon_session=_bdmfmon_session_data(session);
    assert(mon_session);
    if (!mon_session)
        return BDMF_ERR_PARM;
    /* Check access rights */
    if (!dir)
        dir = bdmfmon_root_dir;
    if (dir->access_right > bdmf_session_access_right(mon_session->session))
        return BDMF_ERR_PERM;
    mon_session->curdir = dir;
    return 0;
}

/** Get token name
 * \param[in]   token           Directory or command token
 * \return      directory token name
 */
const char *bdmfmon_token_name(bdmfmon_handle_t token)
{
    if (!token)
        return NULL;
    return token->name;
}

bdmfmon_cmd_parm_t *bdmfmon_find_named_parm(bdmf_session_handle session, const char *name)
{
    bdmfmon_cmd_parm_t * cmd_parm;

    if ( !session || !name || *name=='\0')
        return NULL;

    cmd_parm = _bdmfmon_find_named_parm(_bdmfmon_session_data(session), name);
    if(cmd_parm && (cmd_parm->flags & BDMFMON_PARM_FLAG_ASSIGNED))
    {
        return cmd_parm;
    }

    return NULL;
}

bdmfmon_cmd_parm_t *bdmfmon_find_parm_by_prefix(bdmf_session_handle session, const char *prefix)
{
    bdmfmon_cmd_parm_t *cmd_parm;

    if (!session || !prefix)
        return NULL;

    cmd_parm = _bdmfmon_find_parm_by_prefix(_bdmfmon_session_data(session), prefix);
    if (cmd_parm && (cmd_parm->flags & BDMFMON_PARM_FLAG_ASSIGNED))
    {
        return cmd_parm;
    }

    return NULL;
}

/** Print CLI parameter
 * \param[in]       session         Session handle
 * \param[in]       parm            Parameter
 */
void bdmfmon_parm_print(bdmf_session_handle session, const bdmfmon_cmd_parm_t *parm)
{
    char buf[BDMFMON_MAX_PARM_VAL_LEN] = "";
    parm->format_cb(parm, parm->value, buf, sizeof(buf));
    bdmfmon_print(session, "%s\n", buf);
}

/** Check if parameter is set
 * \param[in]       session         Session handle
 * \param[in]       parm_number     Parameter number
 * \return
 *  1 if parameter is set\n
 *  0 if parameter is not set or parm_number is invalid
 */
int bdmfmon_parm_is_set(bdmf_session_handle session, int parm_number)
{
    bdmfmon_session_extras_t *mon_session=_bdmfmon_session_data(session);
    int i;
    if (parm_number < 0 || !mon_session || !mon_session->curcmd)
        return 0;
    for(i=0; i<parm_number && mon_session->cmd_parms[i].name; i++)
        ;
    if (i < parm_number)
        return 0;
    return ((mon_session->cmd_parms[parm_number].flags & BDMFMON_PARM_FLAG_ASSIGNED)!=0);
}

/*********************************************************/
/* Internal functions                                    */
/*********************************************************/

static int _bdmfmon_help_session_write(bdmf_session_handle session, const char *buf, uint32_t size)
{
    bdmfmon_help_scratchpad_t *scratchpad = (bdmfmon_help_scratchpad_t *)bdmf_session_data(session);
    char *scratch_buf = (char *)((long)scratchpad + sizeof(bdmfmon_help_scratchpad_t));

    if (scratchpad->pos + size >= BDMFMON_HELP_BUFFER_SIZE)
    {
        bdmf_session_handle main_session = bdmf_session_user_priv(session);
        bdmf_session_print(main_session, "%s", scratch_buf);
        scratchpad->pos = 0;
    }
    memcpy(&scratch_buf[scratchpad->pos], buf, size);
    scratchpad->pos += size;
    return size;
}

static bdmf_session_handle _bdmfmon_help_session_open(bdmf_session_handle main_session)
{
    bdmf_session_parm_t help_session_parm =
    {
        .extra_size = BDMFMON_HELP_BUFFER_SIZE + sizeof(bdmfmon_help_scratchpad_t),
        .access_right = BDMF_ACCESS_ADMIN, 
        .write = _bdmfmon_help_session_write,
        .user_priv = main_session
    };
    bdmf_session_handle help_session;
    bdmfmon_help_scratchpad_t *scratchpad;
    int err;

    err = bdmf_session_open(&help_session_parm, &help_session);
    if (err)
    {
        bdmf_session_print(main_session, "CLI: can't create help session. Error %s\n", bdmf_strerror(err));
        return NULL;
    }
    scratchpad = bdmf_session_data(help_session);
    scratchpad->pos = 0;

    return help_session;
}

static void _bdmfmon_help_session_print_and_close(bdmf_session_handle main_session, bdmf_session_handle help_session)
{
    bdmfmon_help_scratchpad_t *scratchpad = (bdmfmon_help_scratchpad_t *)bdmf_session_data(help_session);
    char *scratch_buf = (char *)((long)scratchpad + sizeof(bdmfmon_help_scratchpad_t));

    bdmf_session_print(main_session, "%s", scratch_buf);
    bdmf_session_close(help_session);
}

#ifdef CONFIG_LINENOISE
static int _bdmfmon_line_edit_cmd(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    if (n_parms > 0)
    {
        if ((parm[0].flags & BDMFMON_PARM_FLAG_ASSIGNED))
            linenoiseSetDumbTerminal(session->ln_session, ! parm[0].value.number);
        if ((parm[1].flags & BDMFMON_PARM_FLAG_ASSIGNED))
            linenoiseSetMultiLine(session->ln_session, parm[1].value.number);
    }
    else
    {
        int dumb = linenoiseGetDumbTerminal(session->ln_session);
        int multiline = linenoiseGetMultiLine(session->ln_session);
        bdmf_session_print(session, "Line editing: %s  Multiline: %s\n",
            dumb ? "off" : "on", multiline ? "on" : "off");
    }
    return BDMF_ERR_OK;
}
#endif

/* Allocate root directory and default session */
static void _bdmfmon_alloc_root(const bdmf_session_parm_t *first_session_parm)
{
    bdmf_session_parm_t session_parms;
    bdmf_session_handle session;
    int rc;
    
    /* The very first call. Allocate root structure */
    if ((bdmfmon_root_dir=(bdmfmon_handle_t )bdmf_calloc(sizeof(struct bdmfmon_entry) + strlen(BDMFMON_ROOT_HELP) + 2 )) == NULL)
        return;
    bdmfmon_root_dir->name = (char *)(bdmfmon_root_dir + 1);
    bdmfmon_root_dir->help = (char *)(bdmfmon_root_dir->name + 1);
    strcpy(bdmfmon_root_dir->help, BDMFMON_ROOT_HELP);
    bdmfmon_root_dir->sel = BDMFMON_ENTRY_DIR;
    bdmfmon_root_dir->access_right = BDMF_ACCESS_GUEST;

    memset(&session_parms, 0, sizeof(session_parms));
    session_parms.access_right = BDMF_ACCESS_ADMIN;
    session_parms.extra_size = sizeof(bdmfmon_session_extras_t);
    session_parms.name = "monroot";
    if (first_session_parm)
    {
        session_parms.line_edit_mode = first_session_parm->line_edit_mode;
    }
    rc = bdmfmon_session_open(&session_parms, &session);
    if (rc)
    {
        bdmf_free(bdmfmon_root_dir);
        bdmfmon_root_dir = NULL;
        bdmfmon_root_session = NULL;
        return;
    }
    bdmfmon_root_session = _bdmfmon_session_data(session);
    bdmfmon_root_session->session = session;
    bdmfmon_root_session->curdir = bdmfmon_root_dir;

    /* Add command to disable/enable line editing */
#ifdef CONFIG_LINENOISE
    if (session_parms.line_edit_mode != BDMF_LINE_EDIT_DISABLE)
    {
        BDMFMON_MAKE_CMD(NULL, "~", "Enable/disable/query line editing", _bdmfmon_line_edit_cmd,
            BDMFMON_MAKE_PARM_ENUM("enable", "Enable line editing", bdmfmon_enum_bool_table, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM_ENUM("multiline", "Enable multiline mode", bdmfmon_enum_bool_table,
                BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif
}

/* Display directory */
static void _bdmfmon_display_dir(bdmfmon_session_extras_t *mon_session, bdmfmon_handle_t p_dir)
{
    bdmf_session_handle session = mon_session->session;
    bdmfmon_handle_t p_token;
    bdmfmon_handle_t prev=NULL;
    bdmf_session_handle help_session = _bdmfmon_help_session_open(session);

    if (!help_session)
        return;

    bdmf_session_print(help_session, "%s%s> ", (p_dir==bdmfmon_root_dir)?"":".../", p_dir->name );
    p_token = p_dir->u.dir.first;
    while ( p_token )
    {
        if (p_token->access_right <= bdmf_session_access_right(session))
        {
            if (prev)
                bdmf_session_print(help_session, ", ");
            bdmf_session_print(help_session, "%s", p_token->name );
            if (p_token->sel == BDMFMON_ENTRY_DIR )
                bdmf_session_print(help_session, "/");
            prev = p_token;
        }
        p_token = p_token->next;
    }
    bdmf_session_print(help_session, "\n");
    _bdmfmon_help_session_print_and_close(session, help_session);
}


/* Is character that can be used in a single token ? */
static inline int _bdmfmon_is_special_char(char c)
{
    if (!c)
        return 0;
    return (c == BDMFMON_HELP_CHAR || c == BDMFMON_COMMENT_CHAR || c == BDMFMON_EQUAL_CHAR);
}

/* Make a preliminary analizis of <name> token.
 *   Returns a token type (Empty, Up, Root, Break, Name)
 */
static bdmfmon_token_type_t _bdmfmon_analyze_token( const char *name )
{
    if (!name[0] || name[0]==';')
        return BDMFMON_TOKEN_EMPTY;

    if (*name == BDMFMON_COMMENT_CHAR)
        return BDMFMON_TOKEN_BREAK;

    if (!strcmp(name, BDMFMON_UP_STR))
        return BDMFMON_TOKEN_UP;

    if (!strcmp(name, BDMFMON_ROOT_STR))
        return BDMFMON_TOKEN_ROOT;

    if (*name == BDMFMON_HELP_CHAR)
        return BDMFMON_TOKEN_HELP;

    return BDMFMON_TOKEN_VALUE;

}


/* isspace wrapper */
static inline int _bdmfmon_isspace(char c)
{
    return isspace((int)c);
}

/* Cut the first word from <p_inbuf>.
 * - Return pointer to start of the word in p_word
 * - 0 terminator is inserted in the end of the word
 * - session->p_inbuf is updated to point after the word
 * Returns token type
 */
static bdmfmon_token_type_t _bdmfmon_get_word(bdmfmon_session_extras_t *mon_session, char **buf, char **p_word)
{
    bdmfmon_token_type_t token_type;
    char *p_inbuf = *buf;
    char next_char = 0;

    /* Skip leading blanks */
    while (*p_inbuf && (_bdmfmon_isspace(*p_inbuf) || (*p_inbuf==',')))
        ++p_inbuf;

    *buf = p_inbuf;
    if (! *p_inbuf)
        return BDMFMON_TOKEN_EMPTY;
    if (*p_inbuf == ';')
    {
        *p_inbuf = 0;
        *buf = ++p_inbuf;
        return BDMFMON_TOKEN_EMPTY;
    }

    /* Quoted string ? */
    if (*p_inbuf == '"')
    {
        *p_word = ++p_inbuf;
        while ( *p_inbuf && *p_inbuf!='"' )
            ++p_inbuf;
        if (*p_inbuf != '"')
        {
            bdmf_session_print(mon_session->session, "MON: unterminated string %s\n", *p_word);
            return BDMFMON_TOKEN_EMPTY;
        }
        if (*p_inbuf)
            *(p_inbuf++) = 0;
    }
    else
    {
        *p_word = p_inbuf;
        if (!_bdmfmon_is_special_char(*p_inbuf))
        {
            do ++p_inbuf;
            while (*p_inbuf && (isalnum(*p_inbuf) || *p_inbuf == '_'));

            next_char = *p_inbuf;
            if (next_char == BDMFMON_EQUAL_CHAR)
            {
                *(p_inbuf++) = 0;
            }
            else
            {
                /* It is not named parameter. Keep going until the next space or EOL */
                while (*p_inbuf && !_bdmfmon_isspace(*p_inbuf))
                    ++p_inbuf;
            }

            /* Skip trailing spaces */
            while (*p_inbuf && _bdmfmon_isspace(*p_inbuf))
                *(p_inbuf++) = 0;

        }
        else
        {
            ++p_inbuf;
        }
    }
    *buf = p_inbuf;
    token_type   = _bdmfmon_analyze_token( *p_word );
    if (token_type == BDMFMON_TOKEN_VALUE && next_char == BDMFMON_EQUAL_CHAR)
        token_type = BDMFMON_TOKEN_NAME;
    return token_type;
}

/* Split string to [name=]value pairs */
static int _bdmfmon_split(bdmfmon_session_extras_t *mon_session, bdmfmon_name_value_t **p_pairs, int *p_npairs)
{
    bdmfmon_name_value_t *pairs;
    char *tmp_buf, *tmp_buf_org;
    char *word;
    bdmfmon_token_type_t token_type, prev_type=BDMFMON_TOKEN_EMPTY;
    int n = 0;

    /* Make a copy of input buffer */
    tmp_buf_org = tmp_buf = bdmf_alloc(strlen(mon_session->p_inbuf) + 1);
    if (!tmp_buf)
        return BDMF_ERR_NOMEM;
    strcpy(tmp_buf, mon_session->p_inbuf);

    /* Calculate number of pairs first */
    token_type = _bdmfmon_get_word(mon_session, &tmp_buf, &word);
    while (token_type != BDMFMON_TOKEN_EMPTY && token_type != BDMFMON_TOKEN_BREAK)
    {
        /* Skip =value */
        if (!(prev_type == BDMFMON_TOKEN_NAME && token_type == BDMFMON_TOKEN_VALUE))
            ++n;
        prev_type = token_type;
        token_type = _bdmfmon_get_word(mon_session, &tmp_buf, &word);
    }
    bdmf_free(tmp_buf_org);
    *p_npairs = n;
    if (!n)
    {
        *p_pairs = NULL;
        /* Cut input string in order to prevent infinite loop in the parser if the string
         * is not empty (e.g., contains spaces) */
        *mon_session->p_inbuf = 0;
        if (token_type == BDMFMON_TOKEN_BREAK)
        {
            return BDMF_ERR_NOENT;
        }
        return 0;
    }

    *p_pairs = pairs = bdmf_calloc(n * sizeof(bdmfmon_name_value_t));
    if (! pairs)
        return BDMF_ERR_NOMEM;

    /* Now scan the original string and set names and values */
    token_type = _bdmfmon_get_word(mon_session, &mon_session->p_inbuf, &word);
    prev_type=BDMFMON_TOKEN_EMPTY;
    --pairs; /* it is going to be pre-incremented */
    while (token_type != BDMFMON_TOKEN_EMPTY && token_type != BDMFMON_TOKEN_BREAK)
    {
        if (!(prev_type == BDMFMON_TOKEN_NAME && token_type == BDMFMON_TOKEN_VALUE))
            ++pairs;
        pairs->type = token_type;
        if (token_type == BDMFMON_TOKEN_NAME)
        {
            pairs->name = word;
        }
        else
        {
            pairs->value = word;
        }
        prev_type = token_type;
        token_type = _bdmfmon_get_word(mon_session, &mon_session->p_inbuf, &word);
    }
    return 0;
}

/* Find parameter by name */
static bdmfmon_cmd_parm_t *_bdmfmon_find_named_parm(bdmfmon_session_extras_t *mon_session, const char *name)
{
    bdmfmon_cmd_parm_t *cmd_parm = mon_session->cmd_parms;

    while(cmd_parm->name)
    {
        if (!_bdmfmon_stricmp(name, cmd_parm->name, -1))
            break;
        ++cmd_parm;
    }

    if (!cmd_parm->name)
        return NULL;

    return cmd_parm;
}

/* Find parameter by prefix */
static bdmfmon_cmd_parm_t *_bdmfmon_find_parm_by_prefix(bdmfmon_session_extras_t *mon_session, const char *prefix)
{
    bdmfmon_cmd_parm_t *cmd_parm = mon_session->cmd_parms;

    while(cmd_parm->name)
    {
        if (!_bdmfmon_stricmp(prefix, cmd_parm->name, strlen(prefix)))
            break;
        ++cmd_parm;
    }

    if (!cmd_parm->name)
        return NULL;

    return cmd_parm;
}

/* Extend session parameter table based on selector value */
static int _bdmfmon_extend_parm_table(bdmfmon_session_extras_t *mon_session,
    bdmfmon_cmd_parm_t *selector, const char *value)
{
    bdmfmon_enum_val_t *values=selector->enum_table;
    bdmfmon_cmd_parm_t *parms;
    bdmfmon_cmd_parm_t *session_parm;
    int nparms;

    while (values->name)
    {
        if (!_bdmfmon_stricmp(values->name, value, -1))
            break;
        ++values;
    }
    if (!values->name)
        return BDMF_ERR_INTERNAL;

    /* Calculate number of parameters in selected table */
    parms = values->parms;
    while(parms && parms->name)
    {
        ++parms;
    }
    nparms = parms - values->parms;

    if (mon_session->num_parms + nparms >= BDMFMON_MAX_PARMS)
    {
        bdmf_session_print(mon_session->session, "MON: %s> Can's process selector %s. Too many parameters\n",
            mon_session->curcmd->name, selector->name);
        return BDMF_ERR_OVERFLOW;
    }

    /* Shift session parameters making room for the new table */
    if (selector != &mon_session->cmd_parms[mon_session->num_parms-1])
    {
        memmove(selector + nparms + 1, selector + 1,
            (&mon_session->cmd_parms[mon_session->num_parms-1] - selector) * sizeof(bdmfmon_cmd_parm_t));
    }

    /* Finally insert selector's table */
    parms = values->parms;
    session_parm = selector+1;
    while (parms && parms->name)
    {
        *session_parm = *parms;
        _bdmfmon_assign_callbacks(session_parm);
        ++parms;
        ++session_parm;
    }
    mon_session->num_parms += nparms;

    return BDMF_ERR_OK;
}

/* Parse a single parameter value (scalar value or array element) */
static int _bdmfmon_parse_1value(bdmfmon_session_extras_t *mon_session, bdmfmon_handle_t cmd,
    bdmfmon_cmd_parm_t *parm, bdmfmon_parm_value_t *value, const char *string_value,
    int val_len, int suppress_err_print)
{
    int rc;

    if (val_len >= 0)
    {
        /* We are dealing with array element. string_value is comma rather than
         * 0-terminated. Copy it aside.
         */
        char val_copy[val_len + 1];
        strncpy(val_copy, string_value, val_len);
        val_copy[val_len] = 0;
        rc = parm->scan_cb(parm, value, val_copy);
    }
    else
    {
        rc = parm->scan_cb(parm, value, string_value);
    }
    if (rc)
    {
        if (!suppress_err_print)
        {
            bdmf_session_print(mon_session->session, "MON: %s> <%s>: value %s is invalid\n",
                cmd->name, parm->name, string_value);
        }
    }
    return rc;
}


/* Parse parameter value, including array value (comma-delimited list of element values) */
static int _bdmfmon_parse_value(bdmfmon_session_extras_t *mon_session, bdmfmon_handle_t cmd,
    bdmfmon_cmd_parm_t *parm, const char *string_value, int suppress_err_print)
{
    int rc = BDMF_ERR_OK;

    if (parm->max_array_size)
    {
        uint32_t i = 0;

        /* array element values are comma-delimited */
        for (i = 0; i < parm->max_array_size && string_value && *string_value && !rc; i++)
        {
            const char *pcomma;
            int val_len;

            pcomma = strchr(string_value, BDMFMON_ARRAY_DELIM_CHAR);
            if (pcomma)
            {
                val_len = pcomma - string_value;
            }
            else
            {
                val_len = -1; /* to the end of string */
            }
            /* No value ? */
            if (!_bdmfmon_stricmp(string_value, BDMFMON_ARRAY_NO_VALUE, val_len))
            {
                string_value += strlen(BDMFMON_ARRAY_NO_VALUE);
                break;
            }
            rc = _bdmfmon_parse_1value(mon_session, cmd,
                parm, &parm->values[i], string_value, val_len, suppress_err_print);
            string_value = pcomma ? pcomma + 1 : NULL;
        }
        /* If all parsed values were ok, but we have more values than array size - it is an error */
        if (string_value && *string_value && !rc)
        {
            rc = BDMF_ERR_TOO_MANY;
            if (!suppress_err_print)
            {
                bdmf_session_print(mon_session->session, "MON: %s> <%s>: too many values. %s is invalid\n",
                    cmd->name, parm->name, string_value);
            }
        }
        parm->array_size = i;
    }
    else
    {
        rc = _bdmfmon_parse_1value(mon_session, cmd,
            parm, &parm->value, string_value, -1, suppress_err_print);
    }

    return rc;
}

/* Populate session parameters. Apply selectors */
static int _bdmfmon_populate_parms(bdmfmon_session_extras_t *mon_session, bdmfmon_handle_t cmd,
    bdmfmon_name_value_t *pairs, int npairs, int suppress_err_print, int *last)
{
    const char *parm_value;
    int positional=1;
    bdmfmon_cmd_parm_t *parms=mon_session->cmd_parms;
    bdmfmon_cmd_parm_t *cur_parm;
    int rc;
    int i;
    
    /* Mark all parameters as don't having an explicit value */
    memset(&parms[0], 0, sizeof(mon_session->cmd_parms));
    memcpy(&parms[0], cmd->u.cmd.parms, sizeof(bdmfmon_cmd_parm_t)*cmd->u.cmd.num_parms);
    /* Clear array buffers */
    for (i = 0; i < cmd->u.cmd.num_parms; i++)
    {
        if (parms[i].max_array_size)
        {
            BUG_ON(!parms[i].values);
            memset(parms[i].values, 0, sizeof(bdmfmon_parm_value_t) * parms[i].max_array_size);
        }
    }
    mon_session->curcmd = cmd;
    mon_session->num_parms = cmd->u.cmd.num_parms;
    if (last)
        *last = 0;
    /* Build a format string */
    for (i=0; i<npairs && pairs[i].type != BDMFMON_TOKEN_BREAK; i++)
    {
        parm_value = pairs[i].value;
        if (last)
            *last = i;
        cur_parm = NULL;
        /* Named parameter ? */
        if (pairs[i].name)
        {
            if ((cmd->u.cmd.extras && (cmd->u.cmd.extras->flags & BDMFMON_CMD_FLAG_NO_NAME_PARMS)))
            {
                if (!suppress_err_print)
                {
                    bdmf_session_print(mon_session->session, "MON: %s> Doesn't support named parameters. %s is unexpected\n",
                        cmd->name, pairs[i].name);
                }
                return BDMF_ERR_PARM;
            }
            positional = 0; /* No more positional parameters */
            /* Check name */
            cur_parm = _bdmfmon_find_named_parm(mon_session, pairs[i].name);
            if (!cur_parm)
            {
                if (!suppress_err_print)
                {
                    bdmf_session_print(mon_session->session, "MON: %s> parameter <%s> doesn't exist\n",
                        cmd->name, pairs[i].name);
                }
                return BDMF_ERR_PARM;
            }
            if (!parm_value)
            {
                if (!suppress_err_print)
                {
                    bdmf_session_print(mon_session->session, "MON: %s> <%s>: value is missing\n",
                        cmd->name, cur_parm->name);
                }
                return BDMF_ERR_PARM;
            }
        }
        else
        {
            /* it can still be named ENUM parameter (without =value). In this case the 1st
             * enum value is assumed. Check it
             */
            if (parm_value && (cur_parm = _bdmfmon_find_named_parm(mon_session, parm_value)) &&
                (cur_parm->type == BDMFMON_PARM_ENUM))
            {
                pairs[i].name = parm_value;
                pairs[i].value = parm_value = cur_parm->enum_table->name;
                positional = 0; /* No more positional parameters */
            }
            else
            {
                if (!positional)
                {
                    if (!suppress_err_print)
                        bdmf_session_print(mon_session->session, "MON: %s> Expected named parameter. Got %s\n", cmd->name, parm_value);
                    return BDMF_ERR_PARM;
                }
                cur_parm = &parms[i];
            }
            if (!cur_parm->name)
            {
                if (!suppress_err_print)
                    bdmf_session_print(mon_session->session, "MON: %s> Too many parameters. %s is unexpected\n", cmd->name, parm_value);
                return BDMF_ERR_PARM;
            }
        }

        if (cur_parm->flags & BDMFMON_PARM_FLAG_ASSIGNED)
        {
            if (!suppress_err_print)
            {
                bdmf_session_print(mon_session->session, "MON: %s> Attempt to assign parameter %s more than once\n",
                    cmd->name, cur_parm->name);
            }
            return BDMF_ERR_PARM;
        }

        if (cur_parm->type == BDMFMON_PARM_STRING)
            cur_parm->value.string = (char *)(long)parm_value;
        else
        {
            rc = _bdmfmon_parse_value(mon_session, cmd, cur_parm, parm_value, suppress_err_print);
            if (rc)
                return rc;

            /* For parameter-selector extend list of parameters accordingly */
            if ((cur_parm->flags & BDMFMON_PARM_FLAG_SELECTOR))
            {
                rc = _bdmfmon_extend_parm_table(mon_session, cur_parm, parm_value);
                if (rc)
                    return rc;
            }
        }
        cur_parm->flags |= BDMFMON_PARM_FLAG_ASSIGNED;
    }
    return BDMF_ERR_OK;
}

/* Parse p_inbuf string based on parameter descriptions in <p_token>.
 *   Fill parameter values in <p_token>.
 *   Returns the number of parameters filled or BDMF_ERR_PARM
 *   To Do: add a option of one-by-one user input of missing parameters.
 */
static int _bdmfmon_parse_parms( bdmfmon_session_extras_t *mon_session, bdmfmon_handle_t cmd, bdmfmon_name_value_t *pairs, int npairs)
{
    bdmfmon_cmd_parm_t *parms=mon_session->cmd_parms;
    int rc;
    int i;

    /* Populate parameter table */
    rc = _bdmfmon_populate_parms(mon_session, cmd, pairs, npairs, 0, NULL);
    if (rc)
        return rc;

    /* Make sure that parameters are OK. Check range, process default values */
    for (i=0; i<mon_session->num_parms; i++)
    {
        bdmfmon_cmd_parm_t *cur_parm = &parms[i];

        if (!(cur_parm->flags & BDMFMON_PARM_FLAG_ASSIGNED))
        {
            if ((cur_parm->flags & BDMFMON_PARM_FLAG_DEFVAL))
            {
                cur_parm->flags |= BDMFMON_PARM_FLAG_ASSIGNED;
            }
            else if (!(cur_parm->flags & BDMFMON_PARM_FLAG_OPTIONAL) )
            {
                /* Mandatory parameter missing */
                bdmf_session_print(mon_session->session, "MON: %s> Mandatory parameter <%s> is missing\n", cmd->name, parms[i].name);
                return BDMF_ERR_PARM;
            }
        }

        /* Check value */
        if ((cur_parm->flags & BDMFMON_PARM_FLAG_RANGE))
        {
            if ((cur_parm->flags & BDMFMON_PARM_FLAG_ASSIGNED) &&
                ((cur_parm->value.number < cur_parm->low_val) ||
                 (cur_parm->value.number > cur_parm->hi_val)) )
            {
                bdmf_session_print(mon_session->session, "MON: %s> <%s>: %ld out of range (%ld, %ld)\n",
                    cmd->name, cur_parm->name, cur_parm->value.number, cur_parm->low_val, cur_parm->hi_val);
                return BDMF_ERR_PARM;
            }
        }
    }

    return 0;
}

/* insert value skipping partial match that is already present */
static void _bdmfmon_insert(const char *partial_match, const char *insert_val1,
    const char *insert_val2, char *insert_str, uint32_t insert_size)
{
    if (partial_match)
        insert_val1 += strlen(partial_match);
    bdmfmon_strncpy(insert_str, insert_val1, insert_size);
    if (insert_val2)
        bdmfmon_strncat(insert_str, insert_val2, insert_size);
}

static void _bdmfmon_update_longest_match(char *longest_match, const char *name)
{
    uint32_t nlen = strlen(name);
    uint32_t lmlen = strlen(longest_match);

    if (nlen < lmlen)
    {
        lmlen = nlen;
    }
    while (lmlen && memcmp(longest_match, name, lmlen))
    {
        --lmlen;
    }
    longest_match[lmlen] = 0;
}


/* extend value.
 * If !enum - do nothing
 * If more than 1 matching value - display them
 * If no matching value - do nothing
 * If 1 matching value - insert
 */
static void _bdmfmon_extend_value(bdmfmon_session_extras_t *mon_session, bdmfmon_cmd_parm_t *parm,
    const char *partial_value, char *insert_str, uint32_t insert_size)
{
    int nmatch = 0;
    bdmfmon_enum_val_t *vals = parm->enum_table;
    char longest_match[BDMFMON_MAX_SEARCH_SUBSTR_LENGTH]="";

    if (parm->type != BDMFMON_PARM_ENUM || !vals)
    {
        if (mon_session->curcmd->u.cmd.extras && mon_session->curcmd->u.cmd.extras->num_parm_extend)
        {
            uint32_t np = parm - &mon_session->cmd_parms[0];
            if (np < mon_session->curcmd->u.cmd.extras->num_parm_extend && mon_session->curcmd->u.cmd.extras->parm_extend[np] != NULL)
                mon_session->curcmd->u.cmd.extras->parm_extend[np](mon_session->session, parm, partial_value, insert_str, insert_size);
        }
        return;
    }

    while (vals->name)
    {
        if (!partial_value || !strncmp(vals->name, partial_value, strlen(partial_value)))
        {
            if (!nmatch)
            {
                bdmfmon_strncpy(longest_match, vals->name, sizeof(longest_match));
            }
            else
            {
                _bdmfmon_update_longest_match(longest_match, vals->name);
            }
            ++nmatch;
        }
        ++vals;
    }
    if (!nmatch)
        return;
    if (nmatch == 1)
    {
        _bdmfmon_insert(partial_value, longest_match, " ", insert_str, insert_size);
        return;
    }
    /* display all matching values */
    _bdmfmon_insert(partial_value, longest_match, "", insert_str, insert_size);
    bdmf_session_print(mon_session->session, "\n");
    vals = parm->enum_table;
    while (vals->name)
    {
        if (!partial_value || !strncmp(vals->name, partial_value, strlen(partial_value)))
            bdmf_session_print(mon_session->session, " %s", vals->name);
        ++vals;
    }
    bdmf_session_print(mon_session->session, "\n");
}

/* calculate number of matching parameter names */
static int _bdmfmon_num_matching_names(bdmfmon_session_extras_t *mon_session, const char *partial_value, int *first_match)
{
    int i;
    int nmatch = 0;

    *first_match = -1;
    for (i = 0; i < mon_session->num_parms; i++)
    {
        uint32_t flags = mon_session->cmd_parms[i].flags;
        if ((flags & BDMFMON_PARM_FLAG_ASSIGNED))
            continue;
        if (partial_value && strncmp(mon_session->cmd_parms[i].name, partial_value, strlen(partial_value)))
            continue;
        if (*first_match == -1)
            *first_match = i;
        ++nmatch;
    }
    return nmatch;
}

/* calculate longest matching string.
 * returns number of matching parameters
 */
static int _bdmfmon_longest_match(bdmfmon_session_extras_t *mon_session, const char *partial_value,
    char *longest_match, uint32_t longest_match_size, int *first_match)
{
    int nmatch0 = _bdmfmon_num_matching_names(mon_session, partial_value, first_match);
    int nmatch;
    const char *match_name;

    if (!nmatch0)
        return nmatch0;
    match_name = mon_session->cmd_parms[*first_match].name;
    if (nmatch0 == 1)
    {
        bdmfmon_strncpy(longest_match, match_name, longest_match_size);
        return nmatch0;
    }
    bdmfmon_strncpy(longest_match, match_name, longest_match_size);
    nmatch = _bdmfmon_num_matching_names(mon_session, longest_match, first_match);
    while (nmatch != nmatch0)
    {
        longest_match[strlen(longest_match)-1] = 0;
        nmatch = _bdmfmon_num_matching_names(mon_session, longest_match, first_match);
    }
    return nmatch0;
}

/* display/insert unset matching names
 * If more than 1 matching value - display them
 * If no matching value - do nothing
 * If 1 matching value - insert
 */
static void _bdmfmon_extend_name(bdmfmon_session_extras_t *mon_session, const char *partial_value,
    char *insert_str, uint32_t insert_size)
{
    char longest_match[BDMFMON_MAX_SEARCH_SUBSTR_LENGTH]="";
    int first_match;
    int nmatch = _bdmfmon_longest_match(mon_session, partial_value, longest_match,
        sizeof(longest_match), &first_match);

    if (!nmatch)
        return;
    if (!partial_value || strcmp(partial_value, longest_match))
        _bdmfmon_insert(partial_value, longest_match, (nmatch == 1) ? BDMFMON_EQUAL_CHAR_STR : "", insert_str, insert_size);
    else
        _bdmfmon_help_populated_cmd(mon_session, mon_session->curcmd, partial_value, 1);
}

static int _bdmfmon_extend_parms( bdmfmon_session_extras_t *mon_session, bdmfmon_name_value_t *pairs,
    int npairs, int last_is_space, char *insert_str, uint32_t insert_size)
{
    int rc;
    int last = 0;
    bdmfmon_cmd_parm_t *help_parm = NULL;
    int i;

    rc = _bdmfmon_populate_parms(mon_session, mon_session->curcmd, pairs, npairs, 1, &last);
    /* If the last populated parameter has "extend" callback, call it. Perhaps the parameter
     * is not quite finished yet
     */
    if (!rc &&
        mon_session->curcmd->u.cmd.extras &&
        mon_session->curcmd->u.cmd.extras->num_parm_extend >= last &&
        mon_session->curcmd->u.cmd.extras->parm_extend[last])
    {
        rc = mon_session->curcmd->u.cmd.extras->parm_extend[last](
            mon_session->session, &mon_session->cmd_parms[last],
            npairs ? pairs[last].value : NULL, insert_str, insert_size);
        /* If not fully extended yet - return here */
        if (rc)
            return 0;
    }
    if (!rc)
    {
        /* So far so good */
        /* If there is unset mandatory parameter - insert its name.
         * Otherwise, display list of unset parameters
         */
        /* Find mandatory parameter that is still unassigned */
        for (i = 0; i < mon_session->num_parms; i++)
        {
            uint32_t flags = mon_session->cmd_parms[i].flags;
            if (!(flags & (BDMFMON_PARM_FLAG_OPTIONAL | BDMFMON_PARM_FLAG_DEFVAL | BDMFMON_PARM_FLAG_ASSIGNED)))
            {
                help_parm = &mon_session->cmd_parms[i];
                break;
            }
        }
        if (help_parm)
        {
            if (!last_is_space)
                bdmfmon_strncat(insert_str, " ", insert_size);
            bdmfmon_strncat(insert_str, help_parm->name, insert_size);
            bdmfmon_strncat(insert_str, BDMFMON_EQUAL_CHAR_STR, insert_size);
        }
        else if (mon_session->num_parms && last < mon_session->num_parms - 1)
            _bdmfmon_help_populated_cmd(mon_session, mon_session->curcmd, NULL, 1);
        return BDMF_ERR_OK;
    }

    /* Parsing failed. See what stopped at */
    if (last < mon_session->num_parms)
    {
        bdmfmon_name_value_t *last_pair;

        last_pair = &pairs[last];
        if (last_pair->name)
        {
            /* Try to identify by name */
            help_parm = _bdmfmon_find_named_parm(mon_session, last_pair->name ? last_pair->name : last_pair->value);
        }
        if (help_parm)
        {
            /* Looking for values */
            _bdmfmon_extend_value(mon_session, help_parm, last_pair->value, insert_str, insert_size);
        }
        else
        {
            /* Looking for partial name */
            _bdmfmon_extend_name(mon_session, last_pair->name ? last_pair->name : last_pair->value,
                insert_str, insert_size);
        }
    }

    return 0;
}

/* Identify token in the given directory */
static bdmfmon_handle_t _bdmfmon_search_token1( bdmfmon_handle_t p_dir, const char **p_name, int name_len )
{
    bdmfmon_handle_t p_token = NULL;
    const char *name = *p_name;
    bdmfmon_token_type_t type=_bdmfmon_analyze_token(name);

    /* Name can be qualified */
    if (type == BDMFMON_TOKEN_VALUE && !strncmp(name, BDMFMON_UP_STR, name_len))
        type = BDMFMON_TOKEN_UP;

    switch(type)
    {
        case BDMFMON_TOKEN_ROOT:
            p_token = bdmfmon_root_dir;
            *p_name = name + strlen(BDMFMON_ROOT_STR);
            break;
        case BDMFMON_TOKEN_UP:
            if (p_dir->parent)
                p_token = p_dir->parent;
            else
                p_token = p_dir;
            *p_name = name + strlen(BDMFMON_UP_STR) + 1;
            break;
        case BDMFMON_TOKEN_NAME:
        case BDMFMON_TOKEN_VALUE:
            /* Check alias */
            p_token = p_dir->u.dir.first;
            while ( p_token )
            {
                if (p_token->alias &&
                        (name_len == p_token->alias_len) &&
                        !_bdmfmon_stricmp( p_token->alias, name, p_token->alias_len) )
                    break;
                p_token = p_token->next;
            }
            if (!p_token)
            {
                bdmfmon_handle_t partial_match = NULL;
                /* Check name */
                p_token = p_dir->u.dir.first;
                while ( p_token )
                {
                    if (!_bdmfmon_stricmp( p_token->name, name, name_len ) )
                    {
                        if (name_len == strlen(p_token->name))
                            break;
                        if (!partial_match)
                            partial_match = p_token;
                    }
                    p_token = p_token->next;
                }
                if (!p_token)
                    p_token = partial_match;
            }
            *p_name = name + name_len + 1;
            break;
        default:
            break;
    }

    return p_token;
}


/* Search a token by name in the current directory.
 * The name can be qualified (contain path)
 */
static bdmfmon_handle_t _bdmfmon_search_token( bdmfmon_handle_t p_dir, const char *name )
{
    bdmfmon_handle_t p_token;
    const char *name0 = name;
    const char *p_slash;

    if (!name[0])
        return p_dir;
    
    /* Check if name is qualified */
    do
    {
        p_slash = strchr(name, '/');
        if (p_slash)
        {
            if (p_slash == name0)
            {
                p_dir = p_token = bdmfmon_root_dir;
                name = p_slash + 1;
            }
            else
            {
                p_token = _bdmfmon_search_token1(p_dir, &name, p_slash - name);
                if (p_token && (p_token->sel == BDMFMON_ENTRY_DIR))
                    p_dir = p_token;
            }
        }
        else
        {
            p_token = _bdmfmon_search_token1(p_dir, &name, strlen(name));
        }
    } while (p_slash && p_token && *name);

    return p_token;
}



/* Display help for each entry in the current directory */
static void  _bdmfmon_help_dir(bdmfmon_session_extras_t *mon_session, bdmfmon_handle_t p_dir)
{
    bdmf_session_handle help_session = _bdmfmon_help_session_open(mon_session->session);
    bdmfmon_handle_t p_token;
    char buffer[BDMFMON_MAX_QUAL_NAME_LENGTH];

    _bdmfmon_qualified_name(p_dir, buffer, sizeof(buffer));
    bdmf_session_print(help_session, "Directory %s/ - %s\n", buffer, p_dir->help);
    bdmf_session_print(help_session, "Commands:\n");
    
    p_token = p_dir->u.dir.first;
    while ( p_token )
    {
        if (bdmf_session_access_right(help_session) >= p_token->access_right)
        {
            if (p_token->sel == BDMFMON_ENTRY_DIR)
                bdmf_session_print(help_session, "\t%s/:  %s directory\n", p_token->name, p_token->help );
            else
            {
                char *peol = strchr(p_token->help, '\n');
                int help_len = peol ? peol - p_token->help : (int)strlen(p_token->help);
                bdmf_session_print(help_session, "\t%s(%d parms): %.*s\n",
                            p_token->name, p_token->u.cmd.num_parms, help_len, p_token->help );
            }
        }
        p_token = p_token->next;
    }
    bdmf_session_print(help_session, "Type ? <name> for command help, \"/\"-root, \"..\"-upper\n" );
    _bdmfmon_help_session_print_and_close(mon_session->session, help_session);
}


/* Display help a token */
static void _bdmfmon_help_populated_cmd(bdmfmon_session_extras_t *mon_session, bdmfmon_handle_t p_token,
    const char *partial_match, int suppress_assigned)
{
    char tmp[80];
    char bra, ket;
    uint16_t i;

    if (suppress_assigned)
        bdmf_session_print(mon_session->session, "\n");
    for ( i=0; i<mon_session->num_parms; i++ )
    {
        bdmfmon_cmd_parm_t *cur_parm = &mon_session->cmd_parms[i];
        if (suppress_assigned && (cur_parm->flags & BDMFMON_PARM_FLAG_ASSIGNED))
            continue;
        if (partial_match && memcmp(partial_match, cur_parm->name, strlen(partial_match)))
            continue;

        if ((cur_parm->flags & BDMFMON_PARM_FLAG_OPTIONAL))
        {
            bra = '[';
            ket=']';
        }
        else
        {
            bra = '<';
            ket='>';
        }
        bdmf_session_print(mon_session->session, "\t%c%s(%s)", bra, cur_parm->name, _bdmfmon_get_type_name(cur_parm) );
        if (cur_parm->max_array_size || cur_parm->type == BDMFMON_PARM_BUFFER)
        {
            uint32_t num_entries = cur_parm->max_array_size;
            bdmf_session_print(mon_session->session, "[%u]", num_entries);
        }
        if (cur_parm->type == BDMFMON_PARM_ENUM)
        {
            bdmfmon_enum_val_t *values=cur_parm->enum_table;
            bdmf_session_print(mon_session->session, " {");
            if (cur_parm->flags & BDMFMON_PARM_FLAG_ENUM_EOL_SEPARATOR)
                bdmf_session_print(mon_session->session, "\n\t");

            while(values->name)
            {
                if (values!=cur_parm->enum_table)
                {
                    bdmf_session_print(mon_session->session, ", ");
                    if (cur_parm->flags & BDMFMON_PARM_FLAG_ENUM_EOL_SEPARATOR)
                        bdmf_session_print(mon_session->session, "\n\t");
                }
                bdmf_session_print(mon_session->session, "%s", values->name);
                ++values;
            }
            if (cur_parm->flags & BDMFMON_PARM_FLAG_ENUM_EOL_SEPARATOR)
                bdmf_session_print(mon_session->session, "\n");
            bdmf_session_print(mon_session->session, "}");
        }
        if ((cur_parm->flags & BDMFMON_PARM_FLAG_DEFVAL))
        {
            bdmf_session_print(mon_session->session, BDMFMON_EQUAL_CHAR_STR);
            cur_parm->format_cb(cur_parm, cur_parm->value, tmp, sizeof(tmp));
            bdmf_session_print(mon_session->session, "%s", tmp);
        }
        if ((cur_parm->flags & BDMFMON_PARM_FLAG_RANGE))
        {
            bdmfmon_parm_value_t low_val = { .number = cur_parm->low_val };
            bdmfmon_parm_value_t hi_val = { .number = cur_parm->hi_val };

            bdmf_session_print(mon_session->session, " (");
            cur_parm->format_cb(cur_parm, low_val, tmp, sizeof(tmp));
            bdmf_session_print(mon_session->session, "%s..", tmp);
            cur_parm->format_cb(cur_parm, hi_val, tmp, sizeof(tmp));
            bdmf_session_print(mon_session->session, "%s)", tmp);
        }
        bdmf_session_print(mon_session->session, "%c ", ket);
        bdmf_session_print(mon_session->session, "- %s\n", cur_parm->description);
    }

    /* Print extra help if command has unresolved selector */
    if (mon_session->num_parms &&
        (mon_session->cmd_parms[mon_session->num_parms-1].flags & BDMFMON_PARM_FLAG_SELECTOR) &&
        !(mon_session->cmd_parms[mon_session->num_parms-1].flags & BDMFMON_PARM_FLAG_ASSIGNED))
    {
        const char *sel_name = mon_session->cmd_parms[mon_session->num_parms-1].name;
        bdmf_session_print(mon_session->session, "Add %s=%s_value to see %s-specific parameters\n",
            sel_name, sel_name, sel_name);
    }
    bdmf_session_print(mon_session->session, "\n");
}


/* Display help a token */
static void _bdmfmon_help_entry(bdmfmon_session_extras_t *mon_session, bdmfmon_handle_t p_token,
    bdmfmon_name_value_t *pairs, int npairs, int suppress_err_print)
{
    char buffer[BDMFMON_MAX_QUAL_NAME_LENGTH];

    if (p_token->sel == BDMFMON_ENTRY_DIR)
    {
        _bdmfmon_help_dir(mon_session, p_token);
        return;
    }

    /* Populate parameter table */
    _bdmfmon_populate_parms(mon_session, p_token, pairs, npairs, suppress_err_print, NULL);

    _bdmfmon_qualified_name(p_token, buffer, sizeof(buffer));
    bdmf_session_print(mon_session->session, "%s: \t%s\n", buffer, p_token->help );
    if (p_token->u.cmd.num_parms)
        bdmf_session_print(mon_session->session, "Parameters:\n");
    _bdmfmon_help_populated_cmd(mon_session, p_token, NULL, 0);
}


/* Choose unique alias for <name> in <p_dir> */
/* Currently only single-character aliases are supported */
static void __bdmfmon_chooseAlias(bdmfmon_handle_t p_dir, bdmfmon_handle_t p_new_token, int from)
{
    bdmfmon_handle_t p_token;
    int         i;
    char        c;

    _bdmfmon_strlwr( p_new_token->name );
    i = from;
    while ( p_new_token->name[i] )
    {
        c = p_new_token->name[i];
        p_token = p_dir->u.dir.first;

        while ( p_token )
        {
            if (p_token->alias &&
                    (tolower( *p_token->alias ) == c) )
                break;
            if (strlen(p_token->name)<=2 && tolower(p_token->name[0])==c)
                break;
            p_token = p_token->next;
        }
        if (p_token)
            ++i;
        else
        {
            p_new_token->name[i] = toupper( c );
            p_new_token->alias   = &p_new_token->name[i];
            p_new_token->alias_len = 1;
            break;
        }
    }
}

/* isupper wrapper */
static inline int _bdmfmon_isupper(char c)
{
    return isupper((int)c);
}

static void _bdmfmon_choose_alias(bdmfmon_handle_t p_dir, bdmfmon_handle_t p_new_token)
{
    int i=0;
    p_new_token->alias_len = 0;
    p_new_token->alias = NULL;
    /* Don't try to alias something short */
    if (strlen(p_new_token->name) < BDMFMON_MIN_NAME_LENGTH_FOR_ALIAS)
        return;
    /* Try pre-set alias 1st */
    while ( p_new_token->name[i] )
    {
        if (_bdmfmon_isupper(p_new_token->name[i]))
            break;
        i++;
    }
    if (p_new_token->name[i])
        __bdmfmon_chooseAlias(p_dir, p_new_token, i);
    if (p_new_token->alias != &p_new_token->name[i])
        __bdmfmon_chooseAlias(p_dir, p_new_token, 0);
}


/* Convert string s to lower case. Return pointer to s */
static char  * _bdmfmon_strlwr( char *s )
{
    char  *s0=s;

    while ( *s )
    {
        *s = tolower( *s );
        ++s;
    }

    return s0;
}


/* Compare strings case incensitive */
static int _bdmfmon_stricmp(const char *s1, const char *s2, int len)
{
    int  i;

    for ( i=0; (i<len || len<0); i++ )
    {
        if (tolower( s1[i])  != tolower( s2[i] ))
            return 1;
        if (!s1[i])
            break;
    }

    return 0;
}

static const char *_bdmfmon_get_type_name(const bdmfmon_cmd_parm_t *parm)
{
    bdmfmon_parm_type_t type = parm->type;
    static const char *type_name[] = {
        [BDMFMON_PARM_DECIMAL]    = "decimal",
        [BDMFMON_PARM_DECIMAL64]  = "decimal64",
        [BDMFMON_PARM_UDECIMAL]   = "udecimal",
        [BDMFMON_PARM_UDECIMAL64] = "udecimal64",
        [BDMFMON_PARM_HEX]        = "hex",
        [BDMFMON_PARM_HEX64]      = "hex64",
        [BDMFMON_PARM_NUMBER]     = "number",
        [BDMFMON_PARM_NUMBER64]   = "number64",
        [BDMFMON_PARM_UNUMBER]    = "unumber",
        [BDMFMON_PARM_UNUMBER64]  = "unumber64",
        [BDMFMON_PARM_ENUM]       = "enum",
        [BDMFMON_PARM_STRING]     = "string",
        [BDMFMON_PARM_IP]         = "IP",
        [BDMFMON_PARM_IPV6]       = "IPv6",
        [BDMFMON_PARM_MAC]        = "MAC",
        [BDMFMON_PARM_BUFFER]     = "buffer",
        [BDMFMON_PARM_USERDEF]    = "userdef",
    };
    static const char *undefined = "undefined";
    static const char *selector = "selector";
    if (type > BDMFMON_PARM_USERDEF || !type_name[type])
        return undefined;
    if (type == BDMFMON_PARM_ENUM && (parm->flags & BDMFMON_PARM_FLAG_SELECTOR))
        return selector;
    return type_name[type];
}

/* Assign default callbacks */
static void _bdmfmon_assign_callbacks(bdmfmon_cmd_parm_t *parm)
{
    if (parm->type == BDMFMON_PARM_ENUM)
    {
        parm->scan_cb = _bdmfmon_enum_scan_cb;
        parm->format_cb = _bdmfmon_enum_format_cb;
    }
    else
    {
        if (!parm->scan_cb)
            parm->scan_cb = _bdmfmon_dft_scan_cb;
        if (!parm->format_cb)
            parm->format_cb = _bdmfmon_dft_format_cb;
    }
}

/* Default function for string->value conversion.
 * Returns 0 if OK
 */
static int _bdmfmon_dft_scan_cb(const bdmfmon_cmd_parm_t *parm, bdmfmon_parm_value_t *value, const char *string_val)
{
    char *p_end = NULL;
    int n;

    if (parm->type == BDMFMON_PARM_UDECIMAL ||
        parm->type == BDMFMON_PARM_UDECIMAL64 ||
        parm->type == BDMFMON_PARM_UNUMBER ||
        parm->type == BDMFMON_PARM_UNUMBER64)
    {
        /* strtoul returns OK even when parsing a negative number */
        if (string_val[0] == '-')
        {
            return BDMF_ERR_PARM;
        }
    }

    switch(parm->type)
    {
        case BDMFMON_PARM_DECIMAL:
            value->number = strtol(string_val, &p_end, 10);
            break;
        case BDMFMON_PARM_UDECIMAL:
            value->unumber = strtoul(string_val, &p_end, 10);
            break;
        case BDMFMON_PARM_DECIMAL64:
            value->number64 = strtoll(string_val, &p_end, 10);
            break;
        case BDMFMON_PARM_UDECIMAL64:
            value->unumber64 = strtoull(string_val, &p_end, 10);
            break;
        case BDMFMON_PARM_HEX:
            value->unumber = strtoul(string_val, &p_end, 16);
            break;
        case BDMFMON_PARM_HEX64:
            value->unumber64 = strtoull(string_val, &p_end, 16);
            break;
        case BDMFMON_PARM_NUMBER:
            value->number = strtol(string_val, &p_end, 0);
            break;
        case BDMFMON_PARM_UNUMBER:
            value->unumber = strtoul(string_val, &p_end, 0);
            break;
        case BDMFMON_PARM_NUMBER64:
            value->number64 = strtoll(string_val, &p_end, 0);
            break;
        case BDMFMON_PARM_UNUMBER64:
            value->unumber64 = strtoull(string_val, &p_end, 0);
            break;
        case BDMFMON_PARM_MAC:
        {
            unsigned m0, m1, m2, m3, m4, m5;
            n = sscanf(string_val, "%02x:%02x:%02x:%02x:%02x:%02x",
                &m0, &m1, &m2, &m3, &m4, &m5);
            if (n != 6)
                return BDMF_ERR_PARM;
            if (m0 > 255 || m1 > 255 || m2 > 255 || m3 > 255 || m4 > 255 || m5 > 255)
                return BDMF_ERR_PARM;
            value->mac[0] = m0;
            value->mac[1] = m1;
            value->mac[2] = m2;
            value->mac[3] = m3;
            value->mac[4] = m4;
            value->mac[5] = m5;
            break;
        }
        case BDMFMON_PARM_IP:
        {
            int n1, n2, n3, n4;
            n = sscanf(string_val, "%d.%d.%d.%d", &n1, &n2, &n3, &n4);
            if (n != 4)
                return BDMF_ERR_PARM;
            if ((unsigned)n1 > 255 || (unsigned)n2 > 255 || (unsigned)n3 > 255 || (unsigned)n4 > 255)
                return BDMF_ERR_PARM;
            value->unumber = (n1 << 24) | (n2 << 16) | (n3 << 8) | n4;
            break;
        }

        default:
            return BDMF_ERR_PARM;
    }
    if (p_end && *p_end)
        return BDMF_ERR_PARM;
    return BDMF_ERR_OK;
}

static void _bdmfmon_dft_format_cb(const bdmfmon_cmd_parm_t *parm, bdmfmon_parm_value_t value, char *buffer, int size)
{
    switch(parm->type)
    {
        case BDMFMON_PARM_DECIMAL:
            snprintf(buffer, size, "%ld", value.number);
            break;
        case BDMFMON_PARM_UDECIMAL:
            snprintf(buffer, size, "%lu", value.unumber);
            break;
        case BDMFMON_PARM_DECIMAL64:
            snprintf(buffer, size, "%lld", value.number64);
            break;
        case BDMFMON_PARM_UDECIMAL64:
            snprintf(buffer, size, "%llu", value.unumber64);
            break;
        case BDMFMON_PARM_HEX:
            snprintf(buffer, size, "0x%lx", value.unumber);
            break;
        case BDMFMON_PARM_HEX64:
            snprintf(buffer, size, "0x%llx", value.unumber64);
            break;
        case BDMFMON_PARM_NUMBER:
            snprintf(buffer, size, "%ld", value.number);
            break;
        case BDMFMON_PARM_NUMBER64:
            snprintf(buffer, size, "%lld", value.number64);
            break;
        case BDMFMON_PARM_UNUMBER:
            snprintf(buffer, size, "%lu", value.unumber);
            break;
        case BDMFMON_PARM_UNUMBER64:
            snprintf(buffer, size, "%llu", value.unumber64);
            break;
        case BDMFMON_PARM_STRING:
            snprintf(buffer, size, "%s", value.string);
            break;
        case BDMFMON_PARM_MAC:
            snprintf(buffer, size, "%02x:%02x:%02x:%02x:%02x:%02x",
                parm->value.mac[0], parm->value.mac[1], parm->value.mac[2],
                parm->value.mac[3], parm->value.mac[4], parm->value.mac[5]);
            break;
        case BDMFMON_PARM_IP:
            snprintf(buffer, size, "%d.%d.%d.%d",
                (int)((parm->value.unumber >> 24) & 0xff), (int)((parm->value.unumber >> 16) & 0xff),
                (int)((parm->value.unumber >> 8) & 0xff), (int)(parm->value.unumber & 0xff));
            break;

        default:
            bdmfmon_strncpy(buffer, "*unknown*", size);
    }
}

static int _bdmfmon_enum_scan_cb(const bdmfmon_cmd_parm_t *parm, bdmfmon_parm_value_t *value, const char *string_val)
{
    bdmfmon_enum_val_t *values=parm->enum_table;
    while(values->name)
    {
        if (!_bdmfmon_stricmp(values->name, string_val, -1))
        {
            value->number = values->val;
            return BDMF_ERR_OK;
        }
        ++values;
    }
    return BDMF_ERR_PARM;
}

static void _bdmfmon_enum_format_cb(const bdmfmon_cmd_parm_t *parm, bdmfmon_parm_value_t value, char *buffer, int size)
{
    bdmfmon_enum_val_t *values=parm->enum_table;
    while(values->name)
    {
        if (values->val == value.number)
            break;
        ++values;
    }
    if (values->name)
        strncpy(buffer, values->name, size);
    else
        strncpy(buffer, "*invalid*", size);
}

static const char *_bdmfmon_qualified_name(bdmfmon_handle_t token, char *buffer, int size )
{
    bdmfmon_handle_t parent = token->parent;
    char qual_name[BDMFMON_MAX_QUAL_NAME_LENGTH];
    *buffer=0;
    while(parent)
    {
        strncpy(qual_name, parent->name, sizeof(qual_name));
        if (parent->parent)
            bdmfmon_strncat(qual_name, "/", sizeof(qual_name));
        bdmfmon_strncat(qual_name, buffer, sizeof(qual_name));
        strncpy(buffer, qual_name, size);
        parent = parent->parent;
    }
    size -= strlen(buffer);
    bdmfmon_strncat(buffer, token->name, size);
    return buffer;
}


/** strdup
 * \param[in]       str             Destination string
 * \return dynamically allocated string replica. Caller must release
 */
char *bdmfmon_strdup(const char *str)
{
    char *dst;
    if (!str)
        return NULL;
    dst = bdmf_alloc(strlen(str) + 1);
    if (!dst)
        return NULL;
    strcpy(dst, str);
    return dst;
}

/*
 * Exports
 */
EXPORT_SYMBOL(bdmfmon_dir_add);
EXPORT_SYMBOL(bdmfmon_dir_find);
EXPORT_SYMBOL(bdmfmon_token_name);
EXPORT_SYMBOL(bdmfmon_cmd_add);
EXPORT_SYMBOL(bdmfmon_session_open);
EXPORT_SYMBOL(bdmfmon_session_close);
EXPORT_SYMBOL(bdmfmon_parse);
EXPORT_SYMBOL(bdmfmon_stop);
EXPORT_SYMBOL(bdmfmon_is_stopped);
EXPORT_SYMBOL(bdmfmon_dir_get);
EXPORT_SYMBOL(bdmfmon_dir_set);
EXPORT_SYMBOL(bdmfmon_parm_number);
EXPORT_SYMBOL(bdmfmon_parm_is_set);
EXPORT_SYMBOL(bdmfmon_enum_parm_stringval);
EXPORT_SYMBOL(bdmfmon_token_destroy);
EXPORT_SYMBOL(bdmfmon_enum_bool_table);
EXPORT_SYMBOL(bdmfmon_find_named_parm);
