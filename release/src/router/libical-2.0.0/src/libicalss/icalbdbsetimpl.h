/*======================================================================
 FILE: icalbdbsetimpl.h

 (C) COPYRIGHT 2001, Critical Path

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

     The LGPL as published by the Free Software Foundation, version
     2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

 Or:

     The Mozilla Public License Version 1.0. You may obtain a copy of
     the License at http://www.mozilla.org/MPL/
======================================================================*/

#ifndef ICALBDBSETIMPL_H
#define ICALBDBSETIMPL_H

#include "icalset.h"
#include <db.h>

/* This definition is in its own file so it can be kept out of the
   main header file, but used by "friend classes" like icaldirset*/

struct icalbdbset_impl
{
    icalset super;        /**< parent class */
    const char *path;
    const char *subdb;
    const char *sindex;
    const char *key;
    void *data;
    int datasize;
    int changed;
    icalcomponent *cluster;
    icalgauge *gauge;
    DB_ENV *dbenv;
    DB *dbp;
    DB *sdbp;
    DBC *dbcp;
};

#endif
