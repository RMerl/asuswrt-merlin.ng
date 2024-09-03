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

/***************************************************************************
 * File Name  : bcm_nvram.h
 *
 * Description: This file contains the definitions and structures for the
 *              Broadcom kernel NVRAM interface
 *
 ***************************************************************************/

#ifndef __BCM_NVRAM_H__
#define __BCM_NVRAM_H__

/* Header file for kernel nvram module */

/* Search the given kernel nvram parameter (name) and returns the value */
extern char *nvram_k_get(char *name);

/* Sets the given value to the kernel nvram parameter (name) */
/* Returns 0 on success, else -ve value on failure */
extern int nvram_k_set(char *name, char *value);

#endif /* __BCM_NVRAM_H__ */
