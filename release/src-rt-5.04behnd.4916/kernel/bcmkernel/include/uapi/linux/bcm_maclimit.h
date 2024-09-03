#ifndef __BCM_MACLIMIT_H_INCLUDED__
#define __BCM_MACLIMIT_H_INCLUDED__
/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
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
struct mac_limit
{
    unsigned int enable;           //dev mac limit enabled
    unsigned int max;              //dev and lower-devs max allow
    unsigned int max_zero_drop;    //max zero value is drop or not
    unsigned int drop_count;       //exceed max drop count
    unsigned int min;              //dev mac learning min commit
    unsigned int reserve;          //reserved for lower-devs' min
    unsigned int learning_count;   //dev and lower-devs learning count
};
#endif