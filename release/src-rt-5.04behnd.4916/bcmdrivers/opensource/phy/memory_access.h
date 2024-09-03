/*
    Copyright 2007-2015 Broadcom Corporation

    <:label-BRCM:2015:DUAL/GPL:standard
    
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

#ifndef _MEMORY_ACCESS_H_
#define _MEMORY_ACCESS_H_

#define WRITE_32(a, r)  (*(volatile uint32_t*)(a) = *(uint32_t*)&(r))
#define READ_32(a, r)   (*(volatile uint32_t*)&(r) = *(volatile uint32_t*)(a))
#define WRITE_64(a, r)  (*(volatile uint64_t*)(a) = *(uint64_t*)&(r))
#define READ_64(a, r)   (*(volatile uint64_t*)&(r) = *(volatile uint64_t*)(a))

#define VAL32(_a)       ( *(volatile uint32_t*)(_a))
#define VAL64(_a)       ( *(volatile uint64_t*)(_a))

#endif
