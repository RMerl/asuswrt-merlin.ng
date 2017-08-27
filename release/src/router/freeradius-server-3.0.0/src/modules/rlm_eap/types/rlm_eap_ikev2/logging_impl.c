/*
 *  logging_impl.c - logging callback for lib-eapikev2
 *
 *  This file is part of rlm_eap_ikev2 freeRADIUS module which implements
 *  EAP-IKEv2 protocol functionality.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Copyright (C) 2005-2006 Krzysztof Rzecki <krzysztof.rzecki@ccns.pl>
 *  Copyright (C) 2005-2006 Rafal Mijal <rafal.mijal@ccns.pl>
 *  Copyright (C) 2005-2006 Piotr Marnik <piotr.marnik@ccns.pl>
 *  Copyright (C) 2005-2006 Pawel Matejski <pawel.matejski@ccns.pl>
 *
 */

#include <freeradius-devel/radiusd.h>
#include "logging_impl.h"
#include <stdio.h>
#include <stdlib.h>
#include <EAPIKEv2/logging.h>

void vxlogf(int iklevel, char const * fmt,va_list ap)
{
    int level;
    switch(iklevel) {
	case I2L_DBG:
	    level=L_DBG;
	    break;
	case I2L_INFO:
	    level=L_INFO;
	    break;
	case I2L_ERR:
	    level=L_ERR;
	    break;
	default:
	level=L_DBG;

    }
    vradlog(level, fmt, ap);
}
