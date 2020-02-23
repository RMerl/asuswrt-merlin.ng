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
 * bdmf_trace.c
 *
 * Data path builder - built-in trace
 *
 * This file is Copyright (c) 2011, Broadlight Communications.
 * This file is licensed under GNU Public License, except that if
 * you have entered in to a signed, written license agreement with
 * Broadlight covering this file, that agreement applies to this
 * file instead of the GNU Public License.
 *
 * This file is free software: you can redistribute and/or modify it
 * under the terms of the GNU Public License, Version 2, as published
 * by the Free Software Foundation, unless a different license
 * applies as provided above.
 *
 * This program is distributed in the hope that it will be useful,
 * but AS-IS and WITHOUT ANY WARRANTY; without even the implied
 * warranties of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * TITLE or NONINFRINGEMENT. Redistribution, except as permitted by
 * the GNU Public License or another license agreement between you
 * and Broadlight, is prohibited.
 *
 * You should have received a copy of the GNU Public License,
 * Version 2 along with this file; if not, see
 * <http://www.gnu.org/licenses>.
 *
 * Author: Igor Ternovsky
 *******************************************************************/

#include <bdmf_dev.h>

/** Global trace level (\ref bdmf_trace_level_t) */
int bdmf_global_trace_level = bdmf_trace_level_error;

struct bdmf_trace_session
{
    bdmf_session_handle session;
    uint32_t flags;
    DLIST_ENTRY(bdmf_trace_session) list;
};

static DLIST_HEAD(trace_list, bdmf_trace_session) bdmf_trace_list =
        DLIST_HEAD_INITIALIZER(bdmf_trace_list);

/** Initialise tracer.
 * \return
 *     0    - OK\n
 */
int bdmf_trace_init(void)
{
    bdmf_global_trace_level = bdmf_trace_level_error;
    return 0;
}


/* Add trace session.
 * Each trace entry is "printed" to all configured sessions.
 * \param[in]   session     Trace output session
 * \param[in]   flags       Additional output channel parameters. A combination
 *                          of BDMF_TRACE_FLAG_.. constants
 * \return
 *     0    - OK\n
 *     <0   - error code
 */
int bdmf_trace_session_add(bdmf_session_handle session, uint32_t flags)
{
    struct bdmf_trace_session *t;
    t = bdmf_calloc(sizeof(struct bdmf_trace_session));
    if (!t)
        return BDMF_ERR_NOMEM;
    t->session = session;
    t->flags = flags;
    DLIST_INSERT_HEAD(&bdmf_trace_list, t, list);
    return 0;
}


/* Delete trace session.
 * The function deletes output channel created by bdmf_trace_session_add()
 * \param[in]   session     Trace output session
 * \return:\n
 *     0    - OK\n
 *     <0   - error code
 */
int bdmf_trace_session_delete(bdmf_session_handle session)
{
    struct bdmf_trace_session *t, *tn;
    DLIST_FOREACH_SAFE(t, &bdmf_trace_list, list, tn)
    {
        if (t->session == session)
        {
            DLIST_REMOVE(t, list);
            bdmf_free(t);
            return 0;
        }
    }
    return BDMF_ERR_NOENT;
}


/* Get the current trace level
 * \param[in]   drv     Object type or NULL for global level.
 * \return:\n
 *  >=0     - trace level\n
 *  <0      - error code
 */
bdmf_trace_level_t bdmf_trace_level(bdmf_type_handle drv)
{
    if (!drv)
        return bdmf_global_trace_level;
    return drv->trace_level;
}


/* Set trace level
 * \param[in]   drv         Object type handle or NULL for global level.
 *                          Global level is applied as a default to all existing and future
 *                          object types.
 * \param[in]   level       New trace level
 * \return: old trace level
 */
bdmf_trace_level_t bdmf_trace_level_set(bdmf_type_handle drv, bdmf_trace_level_t level)
{
    bdmf_trace_level_t old_level;
    if (!drv)
    {
        struct bdmf_type *drv_next=NULL;
        while((drv_next = bdmf_type_get_next(drv_next)))
            drv_next->trace_level = level;
        old_level = bdmf_global_trace_level;
        bdmf_global_trace_level = level;
        /* replace for all objects */
    }
    else
    {
        old_level = drv->trace_level;
        drv->trace_level = level;
    }
    return old_level;
}


/* Print trace
 * \param[in]   fmt         printf-like format
 */
void bdmf_trace( const char *fmt, ...)
{
    struct bdmf_trace_session *t, *tn;
    va_list ap;
    va_start(ap, fmt);
    bdmf_session_vprint(NULL, fmt, ap);
    va_end(ap);
    DLIST_FOREACH_SAFE(t, &bdmf_trace_list, list, tn)
    {
        va_start(ap, fmt);
        bdmf_session_vprint(t->session, fmt, ap);
        va_end(ap);
    }
}

/*
 * Exports
 */
EXPORT_SYMBOL(bdmf_trace_session_add);
EXPORT_SYMBOL(bdmf_trace_session_delete);
EXPORT_SYMBOL(bdmf_trace_level);
EXPORT_SYMBOL(bdmf_trace_level_set);
EXPORT_SYMBOL(bdmf_trace);
EXPORT_SYMBOL(bdmf_global_trace_level);
