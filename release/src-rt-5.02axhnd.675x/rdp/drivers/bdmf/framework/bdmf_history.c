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


#include <bdmf_dev.h>
#include <bdmf_session.h>
#include <bdmf_shell.h>

/* Main hist module's control block */
#define DEC_BUF_LEN     2048

#ifdef __KERNEL__
#define ATOMIC_LEVEL    atomic_t
#define ATOMIC_LEVEL_INC(level)     atomic_inc(&(level))
#define ATOMIC_LEVEL_DEC(level)     atomic_dec(&(level))
#define ATOMIC_LEVEL_READ(level)    atomic_read(&(level))

#else
#define ATOMIC_LEVEL    int
#define ATOMIC_LEVEL_INC(level)     ++(level)
#define ATOMIC_LEVEL_DEC(level)     --(level)
#define ATOMIC_LEVEL_READ(level)    level
#endif


struct hist_dev
{
    bdmf_boolean record_on;
    ATOMIC_LEVEL level;
    /* History buffer */
    uint32_t buf_size;
    uint32_t rec_size;
    char *buffer;
    int overflow;
    /* play-back support */
    bdmf_file fdec;
    char dec_buf[DEC_BUF_LEN];
    int nline;
};
struct hist_dev hist;

#define HIST_MIN_LEFTOVER_SIZE          32

static char *hist_cli_cmd[] = {
    [bdmf_hist_ev_none]              = "*none*",
    [bdmf_hist_ev_new_and_configure] = "/bdmf/new",
    [bdmf_hist_ev_new_and_set]       = "/bdmf/mattr/new",
    [bdmf_hist_ev_destroy]           = "/bdmf/delete",
    [bdmf_hist_ev_link]              = "/bdmf/link",
    [bdmf_hist_ev_unlink]            = "/bdmf/unlink",
    [bdmf_hist_ev_configure]         = "/bdmf/configure",
    [bdmf_hist_ev_mattr_set]         = "/bdmf/mattr/set",
    [bdmf_hist_ev_set_as_num]        = "/bdmf/attr/set",
    [bdmf_hist_ev_set_as_string]     = "/bdmf/attr/set",
    [bdmf_hist_ev_set_as_buf]        = "/bdmf/attr/set",
    [bdmf_hist_ev_add_as_num]        = "/bdmf/attr/add",
    [bdmf_hist_ev_add_as_string]     = "/bdmf/attr/add",
    [bdmf_hist_ev_add_as_buf]        = "/bdmf/attr/add",
    [bdmf_hist_ev_delete]            = "/bdmf/attr/del",
};

static const char *_bdmf_hist_ev_cli_cmd(bdmf_history_event_t ev)
{
    static char *invalid="*invalid*";
    if (ev < sizeof(hist_cli_cmd)/sizeof(char *))
        return hist_cli_cmd[ev];
    return invalid;
}

/*
 * Event recording support
 */

static void _bdmf_hist_overflow(void)
{
    if (!hist.overflow)
    {
        BDMF_TRACE_ERR("BDMF history buffer overflow. Recording stopped.\n");
        hist.overflow = 1;
    }
}

static void _bdmf_hist_vprintf(const char *format, va_list args)
{
    int bytes_left = hist.buf_size - hist.rec_size - 1;
    int size;

    if (hist.overflow || bytes_left < HIST_MIN_LEFTOVER_SIZE) /*check if there is any room for additional writing*/
    {
        _bdmf_hist_overflow();
        return;
    }

    size = vsnprintf(&hist.buffer[hist.rec_size], bytes_left, format, args);
    if (size > bytes_left)
    {
        size = bytes_left;
        _bdmf_hist_overflow();
        return;
    }
    hist.rec_size += size;
}

static void _bdmf_hist_printf(const char *format, ...)
    __attribute__((format(printf, 1, 2)));
static void _bdmf_hist_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    _bdmf_hist_vprintf(format, args);
    va_end(args);
}


#define BDMF_HIST_WRITE_RC(args) \
do { \
    int rc = va_arg(args, int); \
    _bdmf_hist_printf(" ;# %d\n", rc); \
} while(0)


#define HEX(n) ((n > 9) ? 'A' + n - 9 : '0' + n)

#if 0
static void _bdmf_hist_bin_to_hex(char *data, uint32_t size)
{
    int bytes_left = hist.buf_size - hist.rec_size - 1;
    char *pbuf=&hist.buffer[hist.rec_size];
    char c;
    int hi, lo;

    if (!data)
    {
        if (bytes_left < 2)
        {
            _bdmf_hist_overflow();
            return;
        }
        *(pbuf++) = '-';
        goto done;
    }

    if (size > (bytes_left - 1) / 2)
    {
        _bdmf_hist_overflow();
        if (!bytes_left)
            return;
        size = (bytes_left - 1) / 2;
    }
    while(size--)
    {
        c = *(data++);
        hi = c >> 4;
        lo = c & 0xf;
        *(pbuf++) = HEX(hi);
        *(pbuf++) = HEX(lo);
    }
done:
    *pbuf = 0;
    hist.rec_size += (pbuf - &hist.buffer[hist.rec_size]);
}
#endif

static void _bdmf_hist_encode_index(struct bdmf_object *mo, bdmf_attr_id aid, bdmf_index index)
{
    int bytes_left = hist.buf_size - hist.rec_size - 1;
    char *pbuf=&hist.buffer[hist.rec_size];
    struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);

    if (hist.overflow || bytes_left < HIST_MIN_LEFTOVER_SIZE) /*check if there is any room for additional writing*/
    {
        _bdmf_hist_overflow();
        return;
    }
    strcpy(pbuf, "-1");
    bdmf_attr_array_index_to_string(mo, attr, index, pbuf, bytes_left);
    hist.rec_size += strlen(pbuf);
}

static void _bdmf_hist_encode_index_ptr(struct bdmf_object *mo, bdmf_attr_id aid, bdmf_index *pindex)
{
    struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);
    bdmf_index index;
    if (bdmf_attr_type_is_numeric(attr->index_type))
        index = *pindex;
    else
        index = (bdmf_index)pindex;
    _bdmf_hist_encode_index(mo, aid, index);
}

static void _bdmf_hist_encode_num_or_ref(struct bdmf_object *mo, bdmf_attr_id aid, bdmf_number val)
{
    int bytes_left = hist.buf_size - hist.rec_size - 1;
    char *pbuf=&hist.buffer[hist.rec_size];
    struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);
    *pbuf = 0;

    if (hist.overflow || bytes_left < HIST_MIN_LEFTOVER_SIZE) /*check if there is any room for additional writing*/
    {
        _bdmf_hist_overflow();
        return;
    }

    if (attr->type == bdmf_attr_object || attr->type == bdmf_attr_enum || attr->type == bdmf_attr_enum_mask)
    {
        /* Insert space and convert to string value */
        *(pbuf++) = ' '; --bytes_left;
        if (attr->size > sizeof(void *))
            attr->val_to_s(mo, attr, &val, pbuf, bytes_left);
        else
        {
            void *ref_ptr = (void *)((long)val);
            attr->val_to_s(mo, attr, &ref_ptr, pbuf, bytes_left);
        }
        hist.rec_size += strlen(pbuf) + 1;
    }
    else
    {
        _bdmf_hist_printf(" %lld", (long long)val);
    }
}

static void _bdmf_hist_encode_buf(struct bdmf_object *mo, bdmf_attr_id aid, void *buf, uint32_t size)
{
    int bytes_left = hist.buf_size - hist.rec_size - 1;
    char *pbuf=&hist.buffer[hist.rec_size];
    struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);

    if (hist.overflow || bytes_left < HIST_MIN_LEFTOVER_SIZE) /*check if there is any room for additional writing*/
    {
        _bdmf_hist_overflow();
        return;
    }

    if (!buf)
    {
        _bdmf_hist_printf("-");
        return;
    }
    *pbuf = 0;
    attr->val_to_s(mo, attr, buf, pbuf, bytes_left);
    hist.rec_size += strlen(pbuf);
}

/* start:   bdmf_object
 * end:     none
 */
static void _bdmf_hist_destroy(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    if ((point & bdmf_hist_point_start))
    {
        struct bdmf_object *mo = va_arg(args, struct bdmf_object *);
        if (!mo->owner)
            return;
        _bdmf_hist_printf("%s %s\n", _bdmf_hist_ev_cli_cmd(ev), mo->name);
    }
}

/* start:   bdmf_object, bdmf_object
 * end:     rc
 */
static void _bdmf_hist_link_unlink(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    if ((point & bdmf_hist_point_start))
    {
        struct bdmf_object *mo1 = va_arg(args, struct bdmf_object *);
        struct bdmf_object *mo2 = va_arg(args, struct bdmf_object *);
        _bdmf_hist_printf("%s %s %s", _bdmf_hist_ev_cli_cmd(ev), mo1->name, mo2->name);
    }
    if ((point & bdmf_hist_point_end))
        BDMF_HIST_WRITE_RC(args);
}

/* start:   bdmf_object, aid, index, bdmf_number
 * end:     rc [,index]
 */
static void _bdmf_hist_set_as_num(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    if ((point & bdmf_hist_point_start))
    {
        struct bdmf_object *mo = va_arg(args, struct bdmf_object *);
        bdmf_attr_id aid = va_arg(args, int);
        struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);
        bdmf_index index = va_arg(args, int);
        bdmf_number val = va_arg(args, bdmf_number);
        _bdmf_hist_printf("%s %s %s[", _bdmf_hist_ev_cli_cmd(ev), mo->name, attr->name);
        _bdmf_hist_encode_index(mo, aid, index);
        _bdmf_hist_printf("] string ");
        _bdmf_hist_encode_num_or_ref(mo, aid, val);
    }
    if ((point & bdmf_hist_point_end))
        BDMF_HIST_WRITE_RC(args);
}

/* start:   bdmf_object, aid, index ptr, bdmf_number
 * end:     rc [,index]
 */
static void _bdmf_hist_add_as_num(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    struct bdmf_object *mo = va_arg(args, struct bdmf_object *);
    bdmf_attr_id aid = va_arg(args, int);
    struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);
    bdmf_index *pindex = va_arg(args, bdmf_index *);
    bdmf_number val = va_arg(args, bdmf_number);
    int rc = va_arg(args, int);

    BUG_ON((point & bdmf_hist_point_both) != bdmf_hist_point_both);
    _bdmf_hist_printf("%s %s %s[", _bdmf_hist_ev_cli_cmd(ev), mo->name, attr->name);
    _bdmf_hist_encode_index_ptr(mo, aid, pindex);
    _bdmf_hist_printf("] string ");
    _bdmf_hist_encode_num_or_ref(mo, aid, val);
    pindex = va_arg(args, bdmf_index *);
    _bdmf_hist_printf(" ;# %d ", rc);
    _bdmf_hist_encode_index_ptr(mo, aid, pindex);
    _bdmf_hist_printf("\n");
}

/* start:   bdmf_object, aid, index, string
 * end:     rc [,index]
 */
static void _bdmf_hist_set_as_string(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    if ((point & bdmf_hist_point_start))
    {
        struct bdmf_object *mo = va_arg(args, struct bdmf_object *);
        bdmf_attr_id aid = va_arg(args, int);
        struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);
        bdmf_index index = va_arg(args, int);
        char *val = va_arg(args, char *);
        _bdmf_hist_printf("%s %s %s[", _bdmf_hist_ev_cli_cmd(ev), mo->name, attr->name);
        _bdmf_hist_encode_index(mo, aid, index);
        _bdmf_hist_printf("] string %s", val);
    }
    if ((point & bdmf_hist_point_end))
        BDMF_HIST_WRITE_RC(args);
}

/* start:   bdmf_object, aid, index ptr, string
 * end:     rc [,index]
 */
static void _bdmf_hist_add_as_string(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    struct bdmf_object *mo = va_arg(args, struct bdmf_object *);
    bdmf_attr_id aid = va_arg(args, int);
    struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);
    bdmf_index *pindex = va_arg(args, bdmf_index *);
    char *val = va_arg(args, char *);
    int rc = va_arg(args, int);

    BUG_ON((point & bdmf_hist_point_both) != bdmf_hist_point_both);

    _bdmf_hist_printf("%s %s %s", _bdmf_hist_ev_cli_cmd(ev), mo->name, attr->name);
    if (pindex)
    {
        _bdmf_hist_printf("[");
        _bdmf_hist_encode_index_ptr(mo, aid, pindex);
        _bdmf_hist_printf("]");
    }
    _bdmf_hist_printf(" string %s", val);
    _bdmf_hist_printf(" ;# %d ", rc);
    pindex = va_arg(args, bdmf_index *);
    _bdmf_hist_encode_index_ptr(mo, aid, pindex);
    _bdmf_hist_printf("\n");
}

/* start:   bdmf_object, aid, index, data
 * end:     rc [,index]
 */
static void _bdmf_hist_set_as_buf(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    if ((point & bdmf_hist_point_start))
    {
        struct bdmf_object *mo = va_arg(args, struct bdmf_object *);
        bdmf_attr_id aid = va_arg(args, int);
        struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);
        bdmf_index index = va_arg(args, bdmf_index);
        char *data = va_arg(args, char *);
        uint32_t size = va_arg(args, uint32_t);

        _bdmf_hist_printf("%s %s %s[", _bdmf_hist_ev_cli_cmd(ev), mo->name, attr->name);
        _bdmf_hist_encode_index(mo, aid, index);
        _bdmf_hist_printf("] string ");
        _bdmf_hist_encode_buf(mo, aid, data, size);
    }
    if ((point & bdmf_hist_point_end))
    {
        BDMF_HIST_WRITE_RC(args);
    }
}

/* start:   bdmf_object, aid, index ptr, data
 * end:     rc [,index]
 */
static void _bdmf_hist_add_as_buf(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    struct bdmf_object *mo = va_arg(args, struct bdmf_object *);
    bdmf_attr_id aid = va_arg(args, int);
    struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);
    bdmf_index *pindex = va_arg(args, bdmf_index *);
    char *data = va_arg(args, char *);
    uint32_t size = va_arg(args, uint32_t);
    int rc = va_arg(args, int);

    BUG_ON((point & bdmf_hist_point_both) != bdmf_hist_point_both);

    _bdmf_hist_printf("%s %s %s", _bdmf_hist_ev_cli_cmd(ev), mo->name, attr->name);
    if (pindex)
    {
        _bdmf_hist_printf("[");
        _bdmf_hist_encode_index_ptr(mo, aid, pindex);
        _bdmf_hist_printf("]");
    }
    _bdmf_hist_printf(" string ");
    _bdmf_hist_encode_buf(mo, aid, data, size);

    _bdmf_hist_printf(" ;# %d ", rc);
    pindex = va_arg(args, bdmf_index *);
    _bdmf_hist_encode_index_ptr(mo, aid, pindex);
    _bdmf_hist_printf("\n");
}

/* start:   bdmf_object, aid, index
 * end:     rc
 */
static void _bdmf_hist_delete(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    if ((point & bdmf_hist_point_start))
    {
        struct bdmf_object *mo = va_arg(args, struct bdmf_object *);
        bdmf_attr_id aid = va_arg(args, int);
        struct bdmf_attr *attr = bdmf_aid_to_attr(mo->drv, aid);
        bdmf_index index = va_arg(args, int);
        _bdmf_hist_printf("%s %s %s[", _bdmf_hist_ev_cli_cmd(ev), mo->name, attr->name);
        _bdmf_hist_encode_index(mo, aid, index);
        _bdmf_hist_printf("]");
    }
    if ((point & bdmf_hist_point_end))
        BDMF_HIST_WRITE_RC(args);
}

/* start:   bdmf_object, string
 * end:     rc
 */
static void _bdmf_hist_configure(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    if ((point & bdmf_hist_point_start))
    {
        struct bdmf_object *mo = va_arg(args, struct bdmf_object *);
        char *str = va_arg(args, char *);
        /* skip temp object used for aggregate attribute/index translation */
        if (! *mo->name)
            return;
        _bdmf_hist_printf("%s %s %s", _bdmf_hist_ev_cli_cmd(ev), mo->name, str);
    }
    if ((point & bdmf_hist_point_end))
        BDMF_HIST_WRITE_RC(args);
}

/* start:   type, parent, string
 * end:     rc
 */
static void _bdmf_hist_new_and_configure(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    if ((point & bdmf_hist_point_start))
    {
        struct bdmf_type *drv = va_arg(args, struct bdmf_type *);
        struct bdmf_object *owner = va_arg(args, struct bdmf_object *);
        char *str = va_arg(args, char *);
        if (str)
            _bdmf_hist_printf("%s %s/%s", _bdmf_hist_ev_cli_cmd(ev), drv->name, str);
        else
            _bdmf_hist_printf("%s %s", _bdmf_hist_ev_cli_cmd(ev), drv->name);
        if (owner)
            _bdmf_hist_printf(" %s", owner->name);
    }
    if ((point & bdmf_hist_point_end))
    {
        int rc = va_arg(args, int);
        struct bdmf_object *mo = rc ? NULL : va_arg(args, struct bdmf_object *);
        _bdmf_hist_printf(" ;# %d %s\n", rc, mo ? mo->name : "");
    }
}

static void _bdmf_encode_mattr(struct bdmf_object *mo, bdmf_mattr_t *mattr)
{
    bdmf_mattr_entry_t *entry;
    int i, num_entries;
    
    _bdmf_hist_printf("/bdmf/mattr/init %s\n", mo->drv->name);
    num_entries = mattr ? mattr->num_entries : 0;
    for(i=0; i<num_entries; i++)
    {
        entry = &mattr->entries[i];
        /* always encode value as string */
        _bdmf_hist_printf("/bdmf/mattr/add %d %lld string", entry->aid, (long long)entry->index);
        switch(entry->val.val_type)
        {
        case bdmf_attr_number: /**< Numeric attribute */
            _bdmf_hist_encode_num_or_ref(mo, entry->aid, entry->val.x.num);
            break;
        case bdmf_attr_string: /**< 0-terminated string */
            _bdmf_hist_printf(" %s", entry->val.x.s);
            break;
        case bdmf_attr_buffer: /**< Buffer with binary data */
            _bdmf_hist_printf(" ");
            _bdmf_hist_encode_buf(mo, entry->aid, entry->val.x.buf.ptr, entry->val.x.buf.len);
            break;
        default:
            _bdmf_hist_printf(" *invalid_type %d", entry->val.val_type);
            break;
        }
        _bdmf_hist_printf("\n");
    }
}

static void _bdmf_free_mattr(void)
{
    _bdmf_hist_printf(";/bdmf/mattr/free\n");
}

/* start:   object, mattr
 * end:     rc
 */
static void _bdmf_hist_mattr_set(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    if ((point & bdmf_hist_point_start))
    {
        struct bdmf_object *mo = va_arg(args, struct bdmf_object *);
        bdmf_mattr_t *mattr = va_arg(args, bdmf_mattr_t *);
        /* At this point create mattr in cli, apply it and release if necessary */
        _bdmf_encode_mattr(mo, mattr);
        _bdmf_hist_printf("%s %s", _bdmf_hist_ev_cli_cmd(ev), mo->name);
    }
    if ((point & bdmf_hist_point_end))
    {
        int rc = va_arg(args, int);
        if (rc)
            _bdmf_free_mattr();
        _bdmf_hist_printf(" ;# %d\n", rc);
    }
}

/* start:   type, parent, mattr
 * end:     rc, object
 */
static void _bdmf_hist_new_and_set(bdmf_history_event_t ev, bdmf_history_point_t point, va_list args)
{
    static struct bdmf_object mo_tmp = { .name="*new*" };

    if ((point & bdmf_hist_point_start))
    {
        struct bdmf_type *drv = va_arg(args, struct bdmf_type *);
        struct bdmf_object *owner = va_arg(args, struct bdmf_object *);
        bdmf_mattr_t *mattr = va_arg(args, bdmf_mattr_t *);
        mo_tmp.drv = drv;
        /* At this point create mattr in cli, apply it and release if necessary */
        _bdmf_encode_mattr(&mo_tmp, mattr);
        _bdmf_hist_printf("%s %s", _bdmf_hist_ev_cli_cmd(ev), mo_tmp.drv->name);
        if (owner)
            _bdmf_hist_printf(" %s", owner->name);
    }
    if ((point & bdmf_hist_point_end))
    {
        int rc = va_arg(args, int);
        struct bdmf_object *mo = rc ? NULL : va_arg(args, struct bdmf_object *);
        if (rc)
            _bdmf_free_mattr();
        _bdmf_hist_printf(" ;# %d %s\n", rc, mo ? mo->name : "");
    }
}

/* Sometimes bdmf API can call other APIs.
 * Record only the outer call
 */
#define BDMF_HISTORY_HANDLE_POINT(point) \
    do { \
        if ((point & bdmf_hist_point_start)) \
        { \
            ATOMIC_LEVEL_INC(hist.level); \
            if (ATOMIC_LEVEL_READ(hist.level) > 1) \
            { \
                if ((point & bdmf_hist_point_end)) \
                    ATOMIC_LEVEL_DEC(hist.level); \
                return; \
            } \
        } \
        if ((point & bdmf_hist_point_end)) \
        { \
            BUG_ON(ATOMIC_LEVEL_READ(hist.level) <= 0); \
            ATOMIC_LEVEL_DEC(hist.level); \
            if (ATOMIC_LEVEL_READ(hist.level)) \
                return; \
        } \
    } while(0)

/* Record built-in history event */
void bdmf_history_bi_event(bdmf_history_event_t ev, bdmf_history_point_t point, ...)
{
    va_list args;

    BDMF_HISTORY_HANDLE_POINT(point);

    if (!hist.record_on)
        return;

    va_start(args, point);
    switch(ev)
    {
    case bdmf_hist_ev_set_as_num:       /**< bdmf_attrelem_set_as_num() */
        _bdmf_hist_set_as_num(ev, point, args);
        break;

    case bdmf_hist_ev_add_as_num:       /**< bdmf_attrelem_add_as_num() */
        _bdmf_hist_add_as_num(ev, point, args);
        break;

    case bdmf_hist_ev_set_as_buf:       /**< bdmf_attrelem_set_as_buf() */
        _bdmf_hist_set_as_buf(ev, point, args);
        break;

    case bdmf_hist_ev_add_as_buf:       /**< bdmf_attrelem_add_as_buf() */
        _bdmf_hist_add_as_buf(ev, point, args);
        break;

    case bdmf_hist_ev_mattr_set:        /**< bdmf_mattr_set() */
        _bdmf_hist_mattr_set(ev, point, args);
        break;

    case bdmf_hist_ev_set_as_string:    /**< bdmf_attrelem_set_as_string() */
        _bdmf_hist_set_as_string(ev, point, args);
        break;

    case bdmf_hist_ev_add_as_string:    /**< bdmf_attrelem_add_as_string() */
        _bdmf_hist_add_as_string(ev, point, args);
        break;

    case bdmf_hist_ev_delete:           /**< bdmf_attrelem_delete() */
        _bdmf_hist_delete(ev, point, args);
        break;

    case bdmf_hist_ev_new_and_configure: /**< bdmf_new_and_configure() */
        _bdmf_hist_new_and_configure(ev, point, args);
        break;

    case bdmf_hist_ev_new_and_set:       /**< bdmf_new_and_set() */
        _bdmf_hist_new_and_set(ev, point, args);
        break;

    case bdmf_hist_ev_destroy:           /**< bdmf_destroy() */
        _bdmf_hist_destroy(ev, point, args);
        break;

    case bdmf_hist_ev_link:              /**< bdmf_link() */
    case bdmf_hist_ev_unlink:            /**< bdmf_unlink() */
        _bdmf_hist_link_unlink(ev, point, args);
        break;

    case bdmf_hist_ev_configure:         /**< bdmf_configure() */
        _bdmf_hist_configure(ev, point, args);
        break;

    default:
        BDMF_TRACE_ERR("history: unknown event %d\n", ev);
        BUG();
    }
    va_end(args);
}

/* Record custom event
 *
 * History event is represented by a string terminated by "\n" \n
 * event_name event_parameters_separated_by_a_single_space # rc optional_results\\n \n
 *
 * \param[in]   event   Event name - must match event in bdmf_history_event_register()
 * \param[in]   point   Recording point: start of event, end of event or both
 * \param[in]   format  printf-like format
 */
void bdmf_history_event(const char *event, bdmf_history_point_t point, const char *format, ...)
{
    va_list args;

    BDMF_HISTORY_HANDLE_POINT(point);

    if (!hist.record_on)
        return;

    if ((point & bdmf_hist_point_start))
        _bdmf_hist_printf("%s ", event);
    va_start(args, format);
    _bdmf_hist_vprintf(format, args);
    va_end(args);

    return;
}

/*
 * API
 */

/** Init history module
 * \return 0=OK or error code <0
 */
int bdmf_history_module_init(void)
{
    if (hist.buffer)
        return BDMF_ERR_ALREADY;
    return BDMF_ERR_OK;
}

/** Exit history module
 */
void bdmf_history_module_exit(void)
{
    bdmf_history_free();
}

/** Init and start history recording */
int bdmf_history_init(uint32_t size, bdmf_boolean record_on)
{
    if (hist.buffer)
        return BDMF_ERR_ALREADY;
    /* Allocate history structure and buffer */
    hist.buffer = bdmf_calloc(size);
    if (!hist.buffer)
        return BDMF_ERR_NOMEM;
    hist.record_on = record_on;
    hist.buf_size = size;
    return 0;
}

/* Stop history recording */
void bdmf_history_stop(void)
{
    hist.record_on = 0;
    BDMF_TRACE_INFO("history: recording stopped\n");
}

/** Resume history recording */
int bdmf_history_resume(void)
{
    if (!hist.buffer)
        return BDMF_ERR_NOT_SUPPORTED;
    hist.record_on = 1;
    return 0;
}

/** Get history buffer
 * \param[out]  buffer      History buffer pointer
 * \param[out]  size        History buffer size
 * \param[out]  rec_size    Recorded history size
 * \return 0=OK or error code <0
 */
int bdmf_history_get(void **buffer, uint32_t *size, uint32_t *rec_size)
{
    if (!buffer || !size || !rec_size)
        return BDMF_ERR_PARM;
    *buffer = hist.buffer;
    *size = hist.buf_size;
    *rec_size = hist.rec_size;
    return 0;
}

/** Reset history buffer.
 * All recorded history is discarded
 * \return 0=OK or error code <0
 */
void bdmf_history_reset(void)
{
    hist.rec_size = 0;
}

/** Release history buffer.
 * All recorded history is discarded
 * \return 0=OK or error code <0
 */
void bdmf_history_free(void)
{
    if (hist.buffer)
        bdmf_free(hist.buffer);
    memset(&hist, 0, sizeof(hist));
}

/** Save history buffer
 * \param[in]   fname   History file name
 * \return 0=OK or error code <0
 */
int bdmf_history_save(const char *fname)
{
    uint32_t fmode = BDMF_FMODE_WRONLY | BDMF_FMODE_CREATE | BDMF_FMODE_TRUNCATE;
    bdmf_file f;
    int rc;
    if (!hist.buffer)
        return BDMF_ERR_NOT_SUPPORTED;
    if (!fname)
        return BDMF_ERR_PARM;
    f = bdmf_file_open(fname, fmode);
    if (!f)
    {
        BDMF_TRACE_ERR("history: can't open history file %s for writing\n", fname);
        return BDMF_ERR_IO;
    }
    rc = bdmf_file_write(f, hist.buffer, hist.rec_size);
    bdmf_file_close(f);
    return (rc==hist.rec_size) ? 0 : BDMF_ERR_IO;
}


#ifdef BDMF_SHELL

/* Read line from history file */
static char *_bdmf_history_line_read(void)
{
    int i = 0;
    int eof = 0;

    do {
        if (bdmf_file_read(hist.fdec, &hist.dec_buf[i], 1) != 1)
        {
            eof = 1;
            break;
        }
        if (hist.dec_buf[i] == '\n')
            break;
        ++i;
    } while (i < DEC_BUF_LEN - 1);
    hist.dec_buf[i] = 0;

    return (i || !eof) ? hist.dec_buf : NULL;
}

/** Play-back history file */
int bdmf_history_play(bdmf_session_handle session, const char *fname, bdmf_boolean stop_on_mismatch)
{
    uint32_t fmode = BDMF_FMODE_RDONLY;
    char *line;
    char *p_rc;
    int has_expected_rc;
    int expected_cmd_rc, cmd_rc;
    int rc = 0;

    if (!fname)
        return BDMF_ERR_PARM;

    if (hist.fdec)
        return BDMF_ERR_STATE;

    hist.fdec = bdmf_file_open(fname, fmode);
    if (!hist.fdec)
    {
        bdmfmon_print(session, "history: can't open history file %s for reading\n", fname);
        return BDMF_ERR_IO;
    }
    hist.nline = 0;
    while ((line = _bdmf_history_line_read()))
    {
        ++hist.nline;
        /* Find if there is ";# rc */
        p_rc = strstr(line, ";# ");
        if (p_rc)
        {
            p_rc += 3;
            expected_cmd_rc = strtol(p_rc, NULL, 10);
            has_expected_rc = 1;
        }
        else
        {
            has_expected_rc = 0;
        }

        bdmfmon_print(session, "%s\n", line);
        cmd_rc = bdmfmon_parse(session, line);
        if (has_expected_rc && (expected_cmd_rc != cmd_rc))
        {
            bdmfmon_print(session, "#!!! Result mismatch in line %d. Expected %d got %d.",
                hist.nline, expected_cmd_rc, cmd_rc);
            if (stop_on_mismatch)
            {
                bdmfmon_print(session, " Playback stopped\n");
                rc = BDMF_ERR_HIST_RES_MISMATCH;
                break;
            }
            else
            {
                bdmfmon_print(session, "ignored\n");
            }
        }
    }

    bdmf_file_close(hist.fdec);
    hist.fdec = 0;

    return rc;
}

/*
 * CLI
 */

/* Init and start history recording
    BDMFMON_MAKE_PARM("size", "History buffer size", BDMFMON_PARM_NUMBER_, 0),
    BDMFMON_MAKE_PARM_ENUM("record_on", "Record mode", bool_table, "on"),
*/
static int bdmf_mon_hist_init(bdmf_session_handle session,
                               const bdmfmon_cmd_parm_t parm[],  uint16_t n_parms)
{
    uint32_t size = (uint32_t)parm[0].value.number;
    bdmf_boolean record_on = (bdmf_boolean)parm[1].value.number;
    int rc;
    rc = bdmf_history_init(size, record_on);
    return rc;
}

/* Stop history recording
*/
static int bdmf_mon_hist_stop(bdmf_session_handle session,
                               const bdmfmon_cmd_parm_t parm[],  uint16_t n_parms)
{
    bdmf_history_stop();
    return 0;
}

/* Resume history recording
*/
static int bdmf_mon_hist_resume(bdmf_session_handle session,
                               const bdmfmon_cmd_parm_t parm[],  uint16_t n_parms)
{
    int rc;
    rc = bdmf_history_resume();
    return rc;
}

/* Save history to file
    BDMFMON_MAKE_PARM("file", "File name", BDMFMON_PARM_STRING, 0),
*/
static int bdmf_mon_hist_save(bdmf_session_handle session,
                               const bdmfmon_cmd_parm_t parm[],  uint16_t n_parms)
{
    char *fname = (char *)parm[0].value.string;
    return bdmf_history_save(fname);
}

/* Playback history recording
    BDMFMON_MAKE_PARM("file", "File name", BDMFMON_PARM_STRING, 0),
    BDMFMON_MAKE_PARM_ENUM_DEFVAL("stop", "Stop on result mismatch", yes_no_table, 0, "yes"),
*/
static int bdmf_mon_hist_play(bdmf_session_handle session,
                               const bdmfmon_cmd_parm_t parm[],  uint16_t n_parms)
{
    char *fname = (char *)parm[0].value.string;
    bdmf_boolean stop_on_mismatch = (bdmf_boolean)parm[1].value.number;
    return bdmf_history_play(session, fname, stop_on_mismatch);
}

bdmfmon_handle_t bdmf_hist_mon_init(void)
{
    bdmfmon_handle_t bdmf_dir;
    bdmfmon_handle_t hist_dir;

    bdmf_dir=bdmfmon_dir_find(NULL, "bdmf");
    hist_dir = bdmfmon_dir_add(bdmf_dir, "history",
                             "History Recording and Playback",
                             BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_enum_val_t on_off_table[] = {
            { .name="off",   .val=0},
            { .name="on",    .val=1},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_cmd_parm_t parms[]={
            BDMFMON_MAKE_PARM("size", "History buffer size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM_ENUM_DEFVAL("record_on", "Record mode", on_off_table, 0, "on"),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        bdmfmon_cmd_add(hist_dir, "init", bdmf_mon_hist_init,
                      "Init and start history recording",
                      BDMF_ACCESS_ADMIN, NULL, parms);
    }
    {
        bdmfmon_cmd_add(hist_dir, "stop", bdmf_mon_hist_stop,
                      "Stop history recording",
                      BDMF_ACCESS_ADMIN, NULL, NULL);
    }
    {
        bdmfmon_cmd_add(hist_dir, "resume", bdmf_mon_hist_resume,
                      "Resume history recording",
                      BDMF_ACCESS_ADMIN, NULL, NULL);
    }
    {
        static bdmfmon_cmd_parm_t parms[]={
            BDMFMON_MAKE_PARM("file", "File name", BDMFMON_PARM_STRING, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        bdmfmon_cmd_add(hist_dir, "save", bdmf_mon_hist_save,
                      "Save history to file",
                      BDMF_ACCESS_ADMIN, NULL, parms);
    }
    {
        static bdmfmon_enum_val_t yes_no_table[] = {
            { .name="no",   .val=0},
            { .name="yes",  .val=1},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_cmd_parm_t parms[]={
            BDMFMON_MAKE_PARM("file", "File name", BDMFMON_PARM_STRING, 0),
            BDMFMON_MAKE_PARM_ENUM_DEFVAL("stop", "Stop on result mismatch", yes_no_table, 0, "yes"),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        bdmfmon_cmd_add(hist_dir, "play", bdmf_mon_hist_play,
                      "Playback history recording",
                      BDMF_ACCESS_ADMIN, NULL, parms);
    }

    return hist_dir;
}

#endif /* #ifdef BDMF_SHELL */
