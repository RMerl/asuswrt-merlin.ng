/*======================================================================
 FILE: icalmessage.h
 CREATOR: eric 07 Nov 2000

 (C) COPYRIGHT 2000, Eric Busboom <eric@softwarestudio.org>

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

  Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/
 =========================================================================*/

#ifndef ICALMESSAGE_H
#define ICALMESSAGE_H

#include "libical_icalss_export.h"
#include "icalcomponent.h"

LIBICAL_ICALSS_EXPORT icalcomponent *icalmessage_new_accept_reply(icalcomponent *c,
                                                                  const char *user,
                                                                  const char *msg);

LIBICAL_ICALSS_EXPORT icalcomponent *icalmessage_new_decline_reply(icalcomponent *c,
                                                                   const char *user,
                                                                   const char *msg);

/* New is modified version of old */
LIBICAL_ICALSS_EXPORT icalcomponent *icalmessage_new_counterpropose_reply(icalcomponent *oldc,
                                                                          icalcomponent *newc,
                                                                          const char *user,
                                                                          const char *msg);

LIBICAL_ICALSS_EXPORT icalcomponent *icalmessage_new_delegate_reply(icalcomponent *c,
                                                                    const char *user,
                                                                    const char *delegatee,
                                                                    const char *msg);

LIBICAL_ICALSS_EXPORT icalcomponent *icalmessage_new_cancel_event(icalcomponent *c,
                                                                  const char *user,
                                                                  const char *msg);

LIBICAL_ICALSS_EXPORT icalcomponent *icalmessage_new_cancel_instance(icalcomponent *c,
                                                                     const char *user,
                                                                     const char *msg);

LIBICAL_ICALSS_EXPORT icalcomponent *icalmessage_new_cancel_all(icalcomponent *c,
                                                                const char *user, const char *msg);

LIBICAL_ICALSS_EXPORT icalcomponent *icalmessage_new_error_reply(icalcomponent *c,
                                                                 const char *user,
                                                                 const char *msg,
                                                                 const char *debug,
                                                                 icalrequeststatus rs);

#endif /* ICALMESSAGE_H */
