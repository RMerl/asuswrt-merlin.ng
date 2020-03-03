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
 * bdmf_session.h
 *
 * BL framework - management session
 *
 *******************************************************************/

#ifndef BDMF_SESSION_H

#define BDMF_SESSION_H

#include <bdmf_system.h>
#include <stdarg.h>

/** \defgroup bdmf_session Management Session Control
 *
 * APIs in this header file allow to create/destroy management sessions.
 * Management session is characterized by its access level and also
 * input/output functions.
 * Management sessions allow managed entities in the system to communicate
 * with local or remote managers (e.g., local or remote shell or NMS)
 * @{
 */

/** Access rights */
typedef enum
{
    BDMF_ACCESS_GUEST,     /**< Guest. Doesn't have access to commands and directories registered with ADMIN rights */
    BDMF_ACCESS_ADMIN,     /**< Administrator: full access */
    BDMF_ACCESS_DEBUG,     /**< Administrator: full access + extended debug features */
} bdmf_access_right_t;

/** Line edit mode */
typedef enum
{
    BDMF_LINE_EDIT_DEFAULT,/**< Enable line editing and history if CONFIG_EDITLINE is defined, disable otherwise */
    BDMF_LINE_EDIT_ENABLE, /**< Enable line editing. Requires CONFIG_EDITLINE define and libedit-dev library */
    BDMF_LINE_EDIT_DISABLE,/**< Disable line editing and history */
} bdmf_line_edit_mode_t;

/** Hex dump format */
typedef enum
{
    BDMF_HEX_DUMP_FORMAT_DEFAULT,/**< Default format of hex dump */
    BDMF_HEX_DUMP_FORMAT_BYTE, /**< Hex dump format of separate bytes */
} bdmf_hex_dump_format_t;

/** Management session handle
 */
typedef struct bdmf_session bdmf_session;

/** Management session handle
 */
typedef struct bdmf_session *bdmf_session_handle;

/** Session parameters structure.
 * See \ref bdmf_session_open
 */
typedef struct bdmf_session_parm
{
    const char *name;       /**< Session name */
    void *user_priv;        /**< Private user's data */

    /** Session's output function. NULL=use write(stdout)
     * returns the number of bytes written or <0 if error
     */
    int (*write)(bdmf_session_handle session, const char *buf, uint32_t size);

    /** Session line input function. NULL=use default(stdin[+line edit) */
    char *(*gets)(bdmf_session_handle session, char *buf, uint32_t size);

    /** Access rights */
    bdmf_access_right_t access_right;

    /** Line editing mode */
    bdmf_line_edit_mode_t line_edit_mode;

    /** Extra data size to be allocated along with session control block.
     * The extra data is accessible using bdmf_session_data().
     */
    uint32_t extra_size;

    /** Hex dump format */
    bdmf_hex_dump_format_t hex_dump_format;
} bdmf_session_parm_t;


/** Open management session
 *
 * Monitor supports multiple simultaneous sessions with different
 * access rights.
 *
 * \param[in]   parm        Session parameters. Must not be allocated on the stack.
 * \param[out]  p_session   Session handle
 * \return
 *      0   =OK\n
 *      <0  =error code
 */
int bdmf_session_open(const bdmf_session_parm_t *parm, bdmf_session_handle *p_session);


/** Close management session.
 * \param[in]   session         Session handle
 */
void bdmf_session_close(bdmf_session_handle session);


/** Write function.
 * Write buffer to the current session.
 * \param[in]   session         Session handle. NULL=use stdout
 * \param[in]   buf             output buffer
 * \param[in]   size            number of bytes to be written
 * \return
 *  >=0 - number of bytes written\n
 *  <0  - output error
 */
int bdmf_session_write(bdmf_session_handle session, const char *buf, uint32_t size);


/** Read line
 * \param[in]       session         Session handle. NULL=use default
 * \param[in,out]   buf             input buffer
 * \param[in]       size            buf size
 * \return
 *      buf if successful
 *      NULL if EOF or error
 */
char *bdmf_session_gets(bdmf_session_handle session, char *buf, uint32_t size);


/** Print function.
 * Prints in the context of current session.
 * \param[in]   session         Session handle. NULL=use stdout
 * \param[in]   format          print format - as in printf
 */
void bdmf_session_print(bdmf_session_handle session, const char *format, ...)
#ifndef BDMF_SESSION_DISABLE_FORMAT_CHECK
__attribute__((format(printf, 2, 3)))
#endif
;


/** Print function.
 * Prints in the context of current session.
 * \param[in]   session         Session handle. NULL=use stdout
 * \param[in]   format          print format - as in printf
 * \param[in]   ap              parameters list. Undefined after the call
 */
void bdmf_session_vprint(bdmf_session_handle session, const char *format, va_list ap);

/** Print buffer in hexadecimal format
 * \param[in]   session         Session handle. NULL=use stdout
 * \param[in]   buffer          Buffer address
 * \param[in]   offset          Start offset in the buffer
 * \param[in]   count           Number of bytes to dump
 */
void bdmf_session_hexdump(bdmf_session_handle session, void *buffer, uint32_t offset, uint32_t count);

/** Get extra data associated with the session
 * \param[in]       session         Session handle. NULL=default session
 * \return extra_data pointer or NULL if there is no extra data
 */
void *bdmf_session_data(bdmf_session_handle session);


/** Get user_priv provided in session parameters when it was registered
 * \param[in]       session         Session handle. NULL=default session
 * \return usr_priv value
 */
void *bdmf_session_user_priv(bdmf_session_handle session);


/** Set user_priv provided in session parameters when it was registered
 * \param[in]       session         Session handle. NULL=default session
 * \param[in]       usr_priv        user_priv value
 * \return old usr_priv value
 */
void *bdmf_session_user_priv_set(bdmf_session_handle session, void *user_priv);


/** Get session name
 * \param[in]       session         Session handle. NULL=use stdin
 * \return session name
 */
const char *bdmf_session_name(bdmf_session_handle session);


/** Get session access rights
 * \param[in]       session         Session handle. NULL=default debug session
 * \return session access right
 */
bdmf_access_right_t bdmf_session_access_right(bdmf_session_handle session);

/** @} end of bdmf_session group */

/** Context extension
 *
 * - if no command - display list of command or extend command
 * - if prev char is " "
 *      - if positional and next parm is enum - show/extends list of matching values
 *      - else - show/extend list of unused parameters
 *   else
 *      - if entering value and enum - show/extends list of matching values
 *      - else - show/extend list of matching unused parameters
 *
 * \param[in]       session         Session handle
 * \param[in]       input_string    String to be parsed
 * \param[out]      insert_str      String to insert at cursor position
 * \param[in]       insert_size     Insert buffer size
 * \return
 *      =0  - OK \n
 *      -EINVAL - parsing error\n
 */
int bdmf_extend(bdmf_session_handle session, char *input_string,
    char *insert_str, uint32_t insert_size);


#ifdef BDMF_INTERNAL

#define BDMF_SESSION_OUTBUF_LEN   2048

/* editline functionality */
/* If libedit is included - it takes precedence */
#ifdef CONFIG_LIBEDIT
#include <histedit.h>
#undef CONFIG_LINENOISE
#endif /* #ifdef CONFIG_LIBEDIT */

#ifdef CONFIG_LINENOISE
#include <linenoise.h>
#endif

/* Management session structure */
struct bdmf_session
{
    bdmf_session *next;
    bdmf_session_parm_t parms;
    uint32_t magic;
#define BDMF_SESSION_MAGIC            (('s'<<24)|('e'<<16)|('s'<<8)|'s')
#define BDMF_SESSION_MAGIC_DEL        (('s'<<24)|('e'<<16)|('s'<<8)|'~')

    /* Line editing and history support */
#ifdef CONFIG_LIBEDIT
    EditLine *el;
    History *history;
    HistEvent histevent;
#endif
#ifdef CONFIG_LINENOISE
    linenoiseSession *ln_session;
#endif
    char outbuf[BDMF_SESSION_OUTBUF_LEN];

    /* Followed by session data */
};
#endif

#endif /* #ifndef BDMF_SESSION_H */
