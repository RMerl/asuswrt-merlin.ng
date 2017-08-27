/*======================================================================
 FILE: icalparameterimpl.h
 CREATOR: eric 09 May 1999

 (C) COPYRIGHT 2000, Eric Busboom <eric@softwarestudio.org>
     http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

 Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

 The original code is icalderivedparameters.{c,h}

 Contributions from:
   Graham Davison (g.m.davison@computer.org)
======================================================================*/

#ifndef ICALPARAMETERIMPL_H
#define ICALPARAMETERIMPL_H

#include "icalproperty.h"

struct icalparameter_impl
{
    icalparameter_kind kind;
    char id[5];
    int size;
    const char *string;
    const char *x_name;
    icalproperty *parent;

    int data;
};

#endif /*ICALPARAMETER_IMPL */
