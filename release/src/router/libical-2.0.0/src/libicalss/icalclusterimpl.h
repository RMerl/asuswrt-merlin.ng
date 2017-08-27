/*======================================================================
 FILE: icalfilesetimpl.h
 CREATOR: acampi 13 March 2002

 Copyright (C) 2002 Andrea Campi <a.campi@inet.it>

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

 Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/
======================================================================*/

#ifndef ICALCLUSTERIMPL_H
#define ICALCLUSTERIMPL_H

/* This definition is in its own file so it can be kept out of the
   main header file, but used by "friend classes" like icaldirset*/

#define ICALCLUSTER_ID "clus"

struct icalcluster_impl
{
    char id[5]; /* clus */

    char *key;
    icalcomponent *data;
    int changed;
};

#endif
