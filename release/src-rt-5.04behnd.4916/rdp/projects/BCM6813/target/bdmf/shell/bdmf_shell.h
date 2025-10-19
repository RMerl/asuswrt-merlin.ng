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
 * bdmf_shell.h
 *
 * BL framework - shell API
 *
 *******************************************************************/

#ifndef BDMF_MON_H

#define BDMF_MON_H

#include <bdmf_system.h>
#include <bdmf_session.h>

/** \defgroup bdmf_mon Broadlight Monitor Module (CLI)
 * Broadlight Monitor is used for all configuration and status monitoring.\n
 * It doesn't have built-in scripting capabilities (logical expressions, loops),
 * but can be used in combination with any available scripting language.\n
 * Broadlight Monitor replaces Broadlight Shell and supports the following features:\n
 * - parameter number and type validation (simplifies command handlers development)
 * - parameter value range checking
 * - mandatory and optional parameters
 * - positional and named parameters
 * - parameters with default values
 * - enum parameters can have arbitrary values
 * - automatic command help generation
 * - automatic or user-defined command shortcuts
 * - command handlers return completion status to enable scripting
 * - multiple sessions
 * - session access rights
 * - extendible. Supports user-defined parameter types
 * - relatively low stack usage
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define BDMFMON_MAX_SEARCH_SUBSTR_LENGTH 80

/** Monitor entry handle
 */
typedef struct bdmfmon_entry *bdmfmon_handle_t;

/* if BDMFMON_PARM_USERIO flag is set:
   low_val: t_userscanf_f function
   high_val: t_userprintf_f function
*/

/** Function parameter structure */
typedef struct bdmfmon_cmd_parm bdmfmon_cmd_parm_t;

/** Parameter type */
typedef enum
{
    BDMFMON_PARM_NONE,
    BDMFMON_PARM_DECIMAL,         /**< Decimal number */
    BDMFMON_PARM_DECIMAL64,       /**< Signed 64-bit decimal */
    BDMFMON_PARM_UDECIMAL,        /**< Unsigned decimal number */
    BDMFMON_PARM_UDECIMAL64,      /**< Unsigned 64-bit decimal number */
    BDMFMON_PARM_HEX,             /**< Hexadecimal number */
    BDMFMON_PARM_HEX64,           /**< 64-bit hexadecimal number */
    BDMFMON_PARM_NUMBER,          /**< Decimal number or hex number prefixed by 0x */
    BDMFMON_PARM_NUMBER64,        /**< 64bit decimal number or hex number prefixed by 0x */
    BDMFMON_PARM_UNUMBER,         /**< Unsigned decimal number or hex number prefixed by 0x */
    BDMFMON_PARM_UNUMBER64,       /**< Unsigned 64bit decimal number or hex number prefixed by 0x */
    BDMFMON_PARM_STRING,          /**< String */
    BDMFMON_PARM_ENUM,            /**< Enumeration */
    BDMFMON_PARM_IP,              /**< IP address n.n.n.n */
    BDMFMON_PARM_IPV6,            /**< IPv6 address */
    BDMFMON_PARM_MAC,             /**< MAC address xx:xx:xx:xx:xx:xx */
    BDMFMON_PARM_BUFFER,          /**< Byte array */

    BDMFMON_PARM_USERDEF          /**< User-defined parameter. User must provide scan_cb */
} bdmfmon_parm_type_t;

/** Enum attribute value.
 *
 *  Enum values is an array of bdmfmon_enum_val_t terminated by element with name==NULL
 *
 */
typedef struct bdmfmon_enum_val
{
    const char *name;           /**< Enum symbolic name */
    long val;                   /**< Enum internal value */
    bdmfmon_cmd_parm_t *parms;  /**< Extension parameter table for enum-selector */
} bdmfmon_enum_val_t;
#define BDMFMON_MAX_ENUM_VALUES   128     /**< Max number of enum values */
#define BDMFMON_ENUM_LAST     { NULL, 0}  /**< Last entry in enum table */

/** Boolean values (true/false, yes/no, on/off)
 *
 */
extern bdmfmon_enum_val_t bdmfmon_enum_bool_table[];

/* Monitor data types */
typedef long bdmfmon_number;      /**< Type underlying BDMFMON_PARM_NUMBER, BDMFMON_PARM_DECIMAL */
typedef long bdmfmon_unumber;     /**< Type underlying BDMFMON_PARM_HEX, BDMFMON_PARM_UDECIMAL */
typedef long bdmfmon_number64;    /**< Type underlying BDMFMON_PARM_NUMBER64, BDMFMON_PARM_DECIMAL64 */
typedef long bdmfmon_unumber64;   /**< Type underlying BDMFMON_PARM_HEX64, BDMFMON_PARM_UDECIMAL64 */

/** Parameter value */
typedef union bdmfmon_parm_value
{
    long number;                    /**< Signed number */
    unsigned long unumber;          /**< Unsigned number */
    long long number64;             /**< Signed 64-bit number */
    unsigned long long unumber64;   /**< Unsigned 64-bit number */
    char *string;                   /**< 0-terminated string */
    double d;                       /**< Double-precision floating point number */
    char mac[6];                    /**< MAC address */
}  bdmfmon_parm_value_t;

/** User-defined scan function.
 * The function is used for parsing user-defined parameter types
 * Returns: 0-ok, <=error
 *
 */
typedef int (*bdmfmon_scan_cb_t)(const bdmfmon_cmd_parm_t *parm, bdmfmon_parm_value_t *value,
    const char *string_val);

/** User-defined print function.
 * The function is used for printing user-defined parameter types
 *
 */
typedef void (*bdmfmon_format_cb_t)(const bdmfmon_cmd_parm_t *parm, bdmfmon_parm_value_t value,
    char *buffer, int size);


/** Function parameter structure */
struct bdmfmon_cmd_parm
{
   const char *name;            /**< Parameter name. Shouldn't be allocated on stack! */
   const char *description;     /**< Parameter description. Shouldn't be allocated on stack! */
   bdmfmon_parm_type_t type;       /**< Parameter type */
   uint8_t flags;               /**< Combination of BDMFMON_PARM_xx flags */
#define BDMFMON_PARM_FLAG_OPTIONAL   0x01 /**< Parameter is optional */
#define BDMFMON_PARM_FLAG_DEFVAL     0x02 /**< Default value is set */
#define BDMFMON_PARM_FLAG_RANGE      0x04 /**< Range is set */
#define BDMFMON_PARM_FLAG_ENUM_EOL_SEPARATOR 0x08 /**< Use new line as enum separator */
#define BDMFMON_PARM_FLAG_EOL        0x20 /**< String from the current parser position till EOL */
#define BDMFMON_PARM_FLAG_SELECTOR   0x40 /**< Parameter selects other parameters */
#define BDMFMON_PARM_FLAG_ASSIGNED   0x80 /**< Internal flag: parameter is assigned */

   bdmfmon_number low_val;       /**< Low val for range checking */
   bdmfmon_number hi_val;        /**< Hi val for range checking */
   bdmfmon_parm_value_t value;     /**< Value */
   bdmfmon_enum_val_t *enum_table; /**< Table containing { enum_name, enum_value } pairs */
   bdmfmon_scan_cb_t scan_cb;      /**< User-defined scan function for BDMFMON_PARM_USERDEF parameter type */
   bdmfmon_format_cb_t format_cb;  /**< User-defined format function for BDMFMON_PARM_USERDEF parameter type */
   uint32_t max_array_size;     /**< Max array size for array-parameter */
   uint32_t array_size;         /**< Actual array size for array-parameter */
   bdmfmon_parm_value_t *values;   /**< Array values */
   void *user_data;             /**< User data - passed transparently to command handler */
};

/** Command parameter list terminator */
#define BDMFMON_PARM_LIST_TERMINATOR  { .name=NULL, .type=BDMFMON_PARM_NONE }

/** Helper macro: make simple parameter
 * \param[in] _name     Parameter name
 * \param[in] _descr    Parameter description
 * \param[in] _type     Parameter type
 * \param[in] _flags    Parameter flags
 */
#define BDMFMON_MAKE_PARM(_name, _descr, _type, _flags) \
    { .name=(_name), .description=(_descr), .type=(_type), .flags=(_flags) }

/** Helper macro: make simple parameter
 * \param[in] _name     Parameter name
 * \param[in] _descr    Parameter description
 * \param[in] _type     Parameter type
 * \param[in] _flags    Parameter flags
 * \param[in] _size     Max array size
 * \param[in] _values   Array values buffer
 */
#define BDMFMON_MAKE_PARM_ARRAY(_name, _descr, _type, _flags, _size, _values) \
    { .name=(_name), .description=(_descr), .type=(_type), .flags=(_flags),\
        .max_array_size=(_size), .values=(_values) }

/** Helper macro: make range parameter
 * \param[in] _name     Parameter name
 * \param[in] _descr    Parameter description
 * \param[in] _type     Parameter type
 * \param[in] _flags    Parameter flags
 * \param[in] _min      Min value
 * \param[in] _max      Max value
 */
#define BDMFMON_MAKE_PARM_RANGE(_name, _descr, _type, _flags, _min, _max) \
    { .name=(_name), .description=(_descr), .type=(_type), .flags=(_flags) | BDMFMON_PARM_FLAG_RANGE, \
        .low_val=(_min), .hi_val=(_max) }

/** Helper macro: make parameter with default value
 * \param[in] _name     Parameter name
 * \param[in] _descr    Parameter description
 * \param[in] _type     Parameter type
 * \param[in] _flags    Parameter flags
 * \param[in] _dft      Default value
 */
#define BDMFMON_MAKE_PARM_DEFVAL(_name, _descr, _type, _flags, _dft) \
    { .name=(_name), .description=(_descr), .type=(_type), .flags=(_flags) | BDMFMON_PARM_FLAG_DEFVAL, \
        .value = {_dft} }

/** Helper macro: make range parameter with default value
 * \param[in] _name     Parameter name
 * \param[in] _descr    Parameter description
 * \param[in] _type     Parameter type
 * \param[in] _flags    Parameter flags
 * \param[in] _min      Min value
 * \param[in] _max      Max value
 * \param[in] _dft      Default value
 */
#define BDMFMON_MAKE_PARM_RANGE_DEFVAL(_name, _descr, _type, _flags, _min, _max, _dft) \
    { .name=(_name), .description=(_descr), .type=(_type), \
        .flags=(_flags) | BDMFMON_PARM_FLAG_RANGE | BDMFMON_PARM_FLAG_DEFVAL, \
        .low_val=(_min), .hi_val=(_max), .value = {_dft} }

/** Helper macro: make enum parameter
 * \param[in] _name     Parameter name
 * \param[in] _descr    Parameter description
 * \param[in] _values   Enum values table
 * \param[in] _flags    Parameter flags
 */
#define BDMFMON_MAKE_PARM_ENUM(_name, _descr, _values, _flags) \
    { .name=(_name), .description=(_descr), .type=BDMFMON_PARM_ENUM, .flags=(_flags), .enum_table=(_values)}

/** Helper macro: make enum parameter with default value
 * \param[in] _name     Parameter name
 * \param[in] _descr    Parameter description
 * \param[in] _values   Enum values table
 * \param[in] _flags    Parameter flags
 * \param[in] _dft      Default value
 */
#define BDMFMON_MAKE_PARM_ENUM_DEFVAL(_name, _descr, _values, _flags, _dft) \
    { .name=(_name), .description=(_descr), .type=BDMFMON_PARM_ENUM, .flags=(_flags) | BDMFMON_PARM_FLAG_DEFVAL,\
        .enum_table=(_values), .value={.string=_dft} }

/** Helper macro: make enum-selector parameter
 * \param[in] _name     Parameter name
 * \param[in] _descr    Parameter description
 * \param[in] _values   Selector values table
 * \param[in] _flags    Parameter flags
 */
#define BDMFMON_MAKE_PARM_SELECTOR(_name, _descr, _values, _flags) \
    { .name=(_name), .description=(_descr), .type=BDMFMON_PARM_ENUM, \
        .flags=(_flags) | BDMFMON_PARM_FLAG_SELECTOR | BDMFMON_PARM_FLAG_ENUM_EOL_SEPARATOR, \
        .enum_table=(_values) }

/** Helper macro: make buffer parameter
 * \param[in] _name     Parameter name
 * \param[in] _descr    Parameter description
 * \param[in] _flags    Parameter flags
 * \param[in] _buf      Memory buffer associated with the parameter
 * \param[in] _size     Buffer size
 */
#define BDMFMON_MAKE_PARM_BUFFER(_name, _descr, _flags, _buf, _size) \
    { .name=(_name), .description=(_descr), .type=BDMFMON_PARM_BUFFER, \
        .flags=(_flags), .value.buffer = {.start = _buf, .curr = _buf, .len = _size} }

/** Register command without parameters helper */
#define BDMFMON_MAKE_CMD_NOPARM(dir, cmd, help, cb) \
    bdmfmon_cmd_add(dir, cmd, cb, help, BDMF_ACCESS_ADMIN, NULL, NULL)

/** Register command helper */
#define BDMFMON_MAKE_CMD(dir, cmd, help, cb, parms...)                          \
{                                                                               \
    static bdmfmon_cmd_parm_t cmd_parms[]={                                     \
        parms,                                                                  \
        BDMFMON_PARM_LIST_TERMINATOR                                            \
    };                                                                          \
    bdmfmon_cmd_add(dir, cmd, cb, help, BDMF_ACCESS_ADMIN, NULL, cmd_parms); \
}

/** Optional custom directory handlers */
typedef void (*bdmfmon_dir_enter_leave_cb)(bdmf_session_handle session, bdmfmon_handle_t dir, int is_enter);

/** Optional command or directory help callback
 * \param[in]   session     Session handle
 * \param[in]   h           Command or directory handle
 * \param[in]   parms       Parameter(s) - the rest of the command string.
 *                          Can be used for example to get help on individual parameters
 */
typedef void (*bdmfmon_help_cb_t)(bdmf_session_handle session, bdmfmon_handle_t h, const char *parms);

/** Optional parameter extend callback
 * \param[in]   session         Session handle
 * \param[in]   parm            CLI parameter
 * \param[in]   partial_value   Parameter value entered so far
 * \param[out]  insert_str      String to be inserted in input string
 * \param[in]   insert_size     insert_str buffer size
 * \returns  0=OK or error code
 */
typedef int (*bdmfmon_parm_extend_cb_t)(bdmf_session_handle session, bdmfmon_cmd_parm_t *parm,
    const char *partial_value, char *insert_str, uint32_t insert_size);

/** Extra parameters of monitor directory.
 * See \ref bdmfmon_dir_add
 *
 */
typedef struct bdmfmon_dir_extra_parm
{
    void *user_priv;            /**< private data passed to enter_leave_cb */
    bdmfmon_dir_enter_leave_cb enter_leave_cb; /**< callback function to be called when session enters/leavs the directory */
    bdmfmon_help_cb_t help_cb;     /**< Help function called to print directory help instead of the automatic help */
} bdmfmon_dir_extra_parm_t;


/** Extra parameters of monitor command.
 * See \ref bdmfmon_cmd_add
 *
 */
typedef struct bdmfmon_cmd_extra_parm
{
    bdmfmon_help_cb_t help_cb;                          /**< Optional help callback. Can be used for more sophisticated help, e.g., help for specific parameters */
    uint32_t flags;                                     /**< Command flags */
#define BDMFMON_CMD_FLAG_NO_NAME_PARMS   0x00000001     /**< No named parms. Positional only. Can be useful if parameter value can contain ',' */
    void (*free_parms)(bdmfmon_cmd_parm_t *parms);      /**< Optional user-defined free */
    uint32_t num_parm_extend;                           /**< Number of entries in parm_extend[] array */
    bdmfmon_parm_extend_cb_t parm_extend[];             /**< Optional parameter extend callbacks */
} bdmfmon_cmd_extra_parm_t;


/** Monitor command handler prototype */
typedef int (*bdmfmon_cmd_cb_t)(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);


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
                             const bdmfmon_dir_extra_parm_t *extras);


/** Scan directory tree and look for directory named "name".
 * 
 * \param[in]   parent          Directory sub-tree root. NULL=root
 * \param[in]   name            Name of directory to be found
 * \return      directory handle if found or NULL if not found
 */
bdmfmon_handle_t bdmfmon_dir_find(bdmfmon_handle_t parent, const char *name );


/** Scan directory tree and look for command named "name".
 *
 * \param[in]   parent          Directory sub-tree root. NULL=root
 * \param[in]   name            Name of command to be found
 * \return      command handle if found or NULL if not found
 */
bdmfmon_handle_t bdmfmon_cmd_find(bdmfmon_handle_t parent, const char *name );


/** Get token name
 * \param[in]   token           Directory or command token
 * \return      directory token name
 */
const char *bdmfmon_token_name(bdmfmon_handle_t token);

/** Find the CLI parameter with the specified name (case insensitive).
 * \param[in]   session  CLI session
 * \param[in]   name     Parameter name
 * \return      The CLI parameter that was found, or NULL if not found
 */
bdmfmon_cmd_parm_t *bdmfmon_find_named_parm(bdmf_session_handle session, const char *name);

/** Find the first CLI parameter whose name starts with the specified string (case insensitive).
 * \param[in]   session  CLI session
 * \param[in]   prefix   Parameter name prefix
 * \return      The CLI parameter that was found, or NULL if not found
 */
bdmfmon_cmd_parm_t *bdmfmon_find_parm_by_prefix(bdmf_session_handle session, const char *prefix);

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
int bdmfmon_session_open(const bdmf_session_parm_t *parm, bdmf_session_handle *p_session);

/** Close monitor session.
 * \param[in]   session         Session handle
 */
void bdmfmon_session_close(bdmf_session_handle session );

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
                  const bdmfmon_cmd_extra_parm_t *extras, bdmfmon_cmd_parm_t parms[]);


/** Destroy token (command or directory)
 * \param[in]   token           Directory or command token. NULL=root
 */
void bdmfmon_token_destroy(bdmfmon_handle_t token);

/** Parse and execute input string.
 * input_string can contain multiple commands delimited by ';'
 * 
 * \param[in]   session         Session handle
 * \param[in]   input_string    String to be parsed
 * \return
 *      =0  - OK \n
 *      -EINVAL - parsing error\n
 *      other - return code - as returned from command handler.
 *            It is recommended to return -EINTR to interrupt monitor loop.
 */
int bdmfmon_parse(bdmf_session_handle session, char *input_string);

/** Read input and parse iteratively until EOF or bdmfmon_is_stopped()
 *
 * \param[in]   session         Session handle
 * \return
 *      =0  - OK \n
 */
int bdmfmon_driver(bdmf_session_handle session);

/** Stop monitor driver.
 * The function stops \ref bdmfmon_driver
 * \param[in]   session         Session handle
 */
void bdmfmon_stop(bdmf_session_handle session);

/** Returns 1 if monitor session is stopped
 * \param[in]   session         Session handle
 * \returns 1 if monitor session stopped by bdmfmon_stop()\n
 * 0 otherwise
 */
int bdmfmon_is_stopped(bdmf_session_handle session);

/** Get current directory for the session,
 * \param[in]   session         Session handle
 * \return      The current directory handle
 */
bdmfmon_handle_t bdmfmon_dir_get(bdmf_session_handle session );

/** Set current directory for the session.
 * \param[in]   session         Session handle
 * \param[in]   dir             Directory that should become current
 * \return
 *      =0  - OK
 *      <0  - error
 */
int bdmfmon_dir_set(bdmf_session_handle session, bdmfmon_handle_t dir);

/** Get parameter number given its name.
 * The function is intended for use by command handlers
 * \param[in]       session         Session handle
 * \param[in]       parm_name       Parameter name
 * \return
 *  >=0 - parameter number\n
 *  <0  - parameter with this name doesn't exist
 */
int bdmfmon_parm_number(bdmf_session_handle session, const char *parm_name);

/** Get parameter by name
 * The function is intended for use by command handlers
 * \param[in]       session         Session handle
 * \param[in]       parm_name       Parameter name
 * \return
 * parameter pointer or NULL if not found
 */
bdmfmon_cmd_parm_t *bdmfmon_parm_get(bdmf_session_handle session, const char *parm_name);

/** Check if parameter is set
 * The function is intended for use by command handlers
 * \param[in]       session         Session handle
 * \param[in]       parm_number     Parameter number
 * \return
 * TRUE if parameter is set, FALSE otherwise
 */
int bdmfmon_parm_is_set(bdmf_session_handle session, int parm_number);

/** Check if parameter is set
 * \param[in]       session         Session handle
 * \param[in]       parm_number     Parameter number
 * \return
 *  0 if parameter is set\n
 *  BCM_ERR_NOENT if parameter is not set
 *  BCM_ERR_PARM if parm_number is invalid
 */
int bdmfmon_parm_check(bdmf_session_handle session, int parm_number);


/** Get enum's string value given its internal value
 * \param[in]       table           Enum table
 * \param[in]       value           Internal value
 * \return
 *      enum string value or NULL if internal value is invalid
 */
static inline const char *bdmfmon_enum_stringval(const bdmfmon_enum_val_t table[], long value)
{
    while(table->name)
    {
        if (table->val==value)
            return table->name;
        ++table;
    }
    return NULL;
}


/** Get enum's parameter string value given its internal value
 * \param[in]       session         Session handle
 * \param[in]       parm_number     Parameter number
 * \param[in]       value           Internal value
 * \return
 *      enum string value or NULL if parameter is not enum or
 *      internal value is invalid
 */
const char *bdmfmon_enum_parm_stringval(bdmf_session_handle session, int parm_number, long value);


/** Print CLI parameter value
 * \param[in]       session         Session handle
 * \param[in]       parm            Parameter
 */
void bdmfmon_parm_print(bdmf_session_handle session, const bdmfmon_cmd_parm_t *parm);


/** strncpy flavour that always add 0 terminator
 * \param[in]       dst             Destination string
 * \param[in]       src             Source string
 * \param[in]       dst_size        Destination buffer size
 * \return dst
 */
static inline char *bdmfmon_strncpy(char *dst, const char *src, uint32_t dst_size)
{
    strncpy(dst, src, dst_size-1);
    dst[dst_size-1] = 0;
    return dst;
}


/** strncat flavour that limits size of destination buffer
 * \param[in]       dst             Destination string
 * \param[in]       src             Source string
 * \param[in]       dst_size        Destination buffer size
 * \return dst
 */
static inline char *bdmfmon_strncat(char *dst, const char *src, uint32_t dst_size)
{
    uint32_t dst_len = strlen(dst);
    return strncat(dst, src, dst_size-dst_len-1);
}


/** strdup
 * \param[in]       str             Destination string
 * \return dynamically allocated string replica. Caller must release
 */
char *bdmfmon_strdup(const char *str);

/* Redefine bdmfmon_session_print --> bdmfmon_print */
#define bdmfmon_print bdmf_session_print

#ifdef __cplusplus
}
#endif

/** @} end bcm_cli group */

#endif /* #ifndef BDMF_MON_H */
