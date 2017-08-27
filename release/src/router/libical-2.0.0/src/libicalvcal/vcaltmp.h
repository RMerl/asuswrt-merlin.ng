/***************************************************************************
(C) Copyright 1996 Apple Computer, Inc., AT&T Corp., International
Business Machines Corporation and Siemens Rolm Communications Inc.

For purposes of this license notice, the term Licensors shall mean,
collectively, Apple Computer, Inc., AT&T Corp., International
Business Machines Corporation and Siemens Rolm Communications Inc.
The term Licensor shall mean any of the Licensors.

Subject to acceptance of the following conditions, permission is hereby
granted by Licensors without the need for written agreement and without
license or royalty fees, to use, copy, modify and distribute this
software for any purpose.

The above copyright notice and the following four paragraphs must be
reproduced in all copies of this software and any software including
this software.

THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS AND NO LICENSOR SHALL HAVE
ANY OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS OR
MODIFICATIONS.

IN NO EVENT SHALL ANY LICENSOR BE LIABLE TO ANY PARTY FOR DIRECT,
INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOST PROFITS ARISING OUT
OF THE USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.

EACH LICENSOR SPECIFICALLY DISCLAIMS ANY WARRANTIES, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO ANY WARRANTY OF NONINFRINGEMENT OR THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE.

The software is provided with RESTRICTED RIGHTS.  Use, duplication, or
disclosure by the government are subject to restrictions set forth in
DFARS 252.227-7013 or 48 CFR 52.227-19, as applicable.

***************************************************************************/

#ifndef VCALTMP_H
#define VCALTMP_H

#include "libical_vcal_export.h"
#include "vcc.h"

#if defined(__CPLUSPLUS__) || defined(__cplusplus)
extern "C"
{
#endif

    LIBICAL_VCAL_EXPORT VObject *vcsCreateVCal(char *date_created,
                                               char *location,
                                               char *product_id, char *time_zone, char *version);

    LIBICAL_VCAL_EXPORT VObject *vcsAddEvent(VObject *vcal,
                                             char *start_date_time,
                                             char *end_date_time,
                                             char *description,
                                             char *summary,
                                             char *categories,
                                             char *classification,
                                             char *status, char *transparency, char *uid,
                                             char *url);

    LIBICAL_VCAL_EXPORT VObject *vcsAddTodo(VObject *vcal,
                                            char *start_date_time,
                                            char *due_date_time,
                                            char *date_time_complete,
                                            char *description,
                                            char *summary,
                                            char *priority,
                                            char *classification, char *status, char *uid,
                                            char *url);

    LIBICAL_VCAL_EXPORT VObject *vcsAddAAlarm(VObject *vevent,
                                              char *run_time,
                                              char *snooze_time,
                                              char *repeat_count, char *audio_content);

    LIBICAL_VCAL_EXPORT VObject *vcsAddMAlarm(VObject *vevent,
                                              char *run_time,
                                              char *snooze_time,
                                              char *repeat_count, char *email_address, char *note);

    LIBICAL_VCAL_EXPORT VObject *vcsAddDAlarm(VObject *vevent,
                                              char *run_time,
                                              char *snooze_time,
                                              char *repeat_count, char *display_string);

    LIBICAL_VCAL_EXPORT VObject *vcsAddPAlarm(VObject *vevent,
                                              char *run_time,
                                              char *snooze_time,
                                              char *repeat_count, char *procedure_name);

#if defined(__CPLUSPLUS__) || defined(__cplusplus)
}

#endif

#endif /* VCALTMP_H */
