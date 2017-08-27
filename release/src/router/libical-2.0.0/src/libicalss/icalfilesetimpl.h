/*======================================================================
 FILE: icalfilesetimpl.h
 CREATOR: eric 23 December 1999

 (C) COPYRIGHT 2000, Eric Busboom <eric@softwarestudio.org>
     http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

 Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

 The Original Code is eric. The Initial Developer of the Original
 Code is Eric Busboom
======================================================================*/

#ifndef ICALFILESETIMPL_H
#define ICALFILESETIMPL_H

#include "icalfileset.h"

struct icalfileset_impl
{
    icalset super;              /**< parent class */
    char *path;                 /**< pathname of file */
    icalfileset_options options;/**< copy of options passed to icalset_new() */

    icalcomponent *cluster;     /**< cluster containing data */
    icalgauge *gauge;           /**< gauge for filtering out data */
    int changed;                /**< boolean flag, 1 if data has changed */
    int fd;                     /**< file descriptor */
};

#endif
