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
 * bdmf_errno.c
 *
 * BL framework - error code helpers
 *
 *******************************************************************/
#include <bdmf_system.h>
#include <bdmf_errno.h>

struct bdmf_errno_range
{
    int from;
    int to;
    const char *(*p_strerr)(int err);
    struct bdmf_errno_range *next;
};

static const char *bdmf_errno_generic_strerr(int err);

static struct bdmf_errno_range bdmf_errno_ranges = 
    { BDMF_ERR_LAST, BDMF_ERR_OK, bdmf_errno_generic_strerr, NULL };


/** Register error code range
 *
 * \param[in]   from        From number. Must be negative
 * \param[in]   to          To number. Must be negative >from
 * \param[in]   p_strerr    Callback that returns error string
 *
 * \returns  BDMF_ERR_OK      - OK\n
 *           BDMF_ERR_PARM    - error in parameters\n
 *           BDMF_ERR_NOMEM   - no memory
 *           BDMF_ERR_ALREADY - ( from, to ) range overlaps with existing range
 */
bdmf_error_t bdmf_error_range_register(int from, int to,
                                   const char *(*p_strerr)(int err))
{
    struct bdmf_errno_range *range=&bdmf_errno_ranges;
    struct bdmf_errno_range *last_range;
    struct bdmf_errno_range *new_range;
    
    if (from>=0 || to>=0 || !p_strerr)
        return -BDMF_ERR_PARM;
    
    /* Reorder from and to if necessary */
    if (to < from)
    {
        int tmp=to;
        to = from;
        from = tmp;
    }
    
    /* Make sure that new range doesn't overlap with existing ranges */
    do
    {
        if ((from>=range->from && from<=range->to) ||
            (to>=range->from && to<=range->to)     ||
            (from<range->from && to>range->to))
        {
            return BDMF_ERR_ALREADY;
        }
        last_range = range;
        range = range->next;
    } while(range);

    /* Register new range */
    new_range = bdmf_calloc(sizeof(struct bdmf_errno_range));
    if (!new_range)
        return BDMF_ERR_NOMEM;
    new_range->from = from;
    new_range->to = to;
    new_range->p_strerr = p_strerr;
    last_range->next = new_range;
    return BDMF_ERR_OK;
}


/** Unregister error code range
 *
 * \param[in]   from        From number. Must be negative
 * \param[in]   to          To number. Must be negative >from
 * \param[in]   p_strerr    Callback that returns error string
 *
 * \returns  BDMF_ERR_OK      - OK\n
 *           BDMF_ERR_PARM    - error in parameters\n
 *           BDMF_ERR_NOENT   - ( from, to ) range is not registered
 */
bdmf_error_t bdmf_error_range_unregister(int from, int to)
{
    struct bdmf_errno_range *prev_range = &bdmf_errno_ranges;
    struct bdmf_errno_range *range=prev_range->next;
    while(range && (range->from!=from || range->to!=to))
    {
        prev_range = range;
        range = range->next;
    }
    if (!range)
        return BDMF_ERR_NOENT;
    prev_range->next = range->next;
    bdmf_free(range);
    return BDMF_ERR_OK;
}


/** Convert error code to error string
 *
 * \param[in]   err         Error code. One of bdmf_error_t constants or additional
 *                          codes registered using bdmf_error_register()
 * \returns Error string
 */
const char *bdmf_strerror(int err)
{
    static char *unknown="Unknown error code";
    struct bdmf_errno_range *range=&bdmf_errno_ranges;
    const char *msg=NULL;
    do
    {
        if (err>=range->from && err<=range->to)
            break;
        range = range->next;
    } while(range);
    if (range)
        msg = range->p_strerr(err);
    if (!msg)
        msg = unknown;
    return msg;
}



static const char *bdmf_errno_generic_strerr(int err)
{
    static char *generic_error_codes[] = {
        [-BDMF_ERR_OK]            = "OK",
        [-BDMF_ERR_PARM]          = "Error in parameters",
        [-BDMF_ERR_NOMEM]         = "No memory",
        [-BDMF_ERR_NORES]         = "No resources",
        [-BDMF_ERR_INTERNAL]      = "Internal error",
        [-BDMF_ERR_NOENT]         = "Entry doesn't exist",
        [-BDMF_ERR_NODEV]         = "Device doesn't exist",
        [-BDMF_ERR_ALREADY]       = "Entry already exists",
        [-BDMF_ERR_RANGE]         = "Out of range",
        [-BDMF_ERR_PERM]          = "No permission to perform an operation",
        [-BDMF_ERR_NOT_SUPPORTED]  = "Operation is not supported",
        [-BDMF_ERR_PARSE]         = "Parsing error",
        [-BDMF_ERR_INVALID_OP]    = "Invalid operation",
        [-BDMF_ERR_IO]            = "I/O error",
        [-BDMF_ERR_STATE]         = "Object is in bad state",
        [-BDMF_ERR_DELETED]       = "Object is deleted",
        [-BDMF_ERR_TOO_MANY]      = "Too many objects",
        [-BDMF_ERR_NOT_LINKED]    = "Objects are not linked",
        [-BDMF_ERR_OVERFLOW]        = "Buffer overflow",
        [-BDMF_ERR_COMM_FAIL]     = "Communication failure",
        [-BDMF_ERR_NOT_CONNECTED] = "No connection with the target system",
        [-BDMF_ERR_SYSCALL_ERR]   = "System call returned error",
        [-BDMF_ERR_MSG_ERROR]     = "Received message is insane",
        [-BDMF_ERR_TOO_MANY_REQS] = "Too many outstanding requests",
        [-BDMF_ERR_NO_MSG_SERVER] = "Remote delivery error. No message server",
        [-BDMF_ERR_NO_LOCAL_SUBS] = "Local subsystem is not set",
        [-BDMF_ERR_NO_SUBS]       = "Subsystem is not recognised",
        [-BDMF_ERR_NO_MORE]       = "No more entries",
        [-BDMF_ERR_INTR]          = "Interrupted",
        [-BDMF_ERR_HIST_RES_MISMATCH] = "History result mismatch",
        [-BDMF_ERR_MORE]          = "More work to do"
    };
    err = -err;
    if (err<0 || err>=sizeof(generic_error_codes)/sizeof(generic_error_codes[0]))
        return NULL;
    return generic_error_codes[err];
}

/*
 * Exports
 */
EXPORT_SYMBOL(bdmf_error_range_register);
EXPORT_SYMBOL(bdmf_error_range_unregister);
EXPORT_SYMBOL(bdmf_strerror);
