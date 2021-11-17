/*
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

#ifndef __BCM_MAP_PART_H
#define __BCM_MAP_PART_H

#if defined(CONFIG_BCM96318)
#include <6318_map_part.h>
#endif
#if defined(CONFIG_BCM963268)
#include <63268_map_part.h>
#endif
#if defined(CONFIG_BCM96328)
#include <6328_map_part.h>
#endif
#if defined(CONFIG_BCM96362)
#include <6362_map_part.h>
#endif
#if defined(CONFIG_BCM96838)
#include <6838_map_part.h>
#endif
#if defined(CONFIG_BCM963138)
#include <63138_map_part.h>
#endif
#if defined(CONFIG_BCM960333)
#include <60333_map_part.h>
#endif
#if defined(CONFIG_BCM963381)
#include <63381_map_part.h>
#endif
#if defined(CONFIG_BCM963148)
#include <63148_map_part.h>
#endif
#if defined(CONFIG_BCM96848)
#include <6848_map_part.h>
#endif
#if defined(CONFIG_BCM94908)
#include <4908_map_part.h>
#include <8486x_map_part.h>
#endif
#if defined(CONFIG_BCM96858)
#include <6858_map_part.h>
#endif
#if defined(CONFIG_BCM947189)
#include <47189_map_part.h>
#endif
#if defined(CONFIG_BCM968360)
#include <68360_map_part.h>
#endif
#endif

