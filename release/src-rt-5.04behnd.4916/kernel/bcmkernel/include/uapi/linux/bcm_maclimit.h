#ifndef __BCM_MACLIMIT_H_INCLUDED__
#define __BCM_MACLIMIT_H_INCLUDED__
/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

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