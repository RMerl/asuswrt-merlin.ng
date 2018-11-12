/**************************************************************
 * Copyright (C) 2001 Alex Rozin, Optical Access
 *
 *                     All Rights Reserved
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * ALEX ROZIN DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * ALEX ROZIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 ******************************************************************/

#ifndef _MIBGROUP_EVENT_H
#define _MIBGROUP_EVENT_H

    /*
     * function prototypes 
     */
     void            init_event(void);

config_require(util_funcs)

config_require(Rmon/agutil)
config_require(Rmon/rows)

     int
     event_api_send_alarm(u_char is_rising, u_long alarm_index,
                          u_long event_index, oid * alarmed_var,
                          size_t alarmed_var_length, u_long sample_type,
                          u_long value, u_long the_threshold,
                          const char *alarm_descr);

#endif                          /* _MIBGROUP_EVENT_H */
