/***********************************************************************
 *
 * Copyright (c) 2019  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2019:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 *
 ************************************************************************/

#ifndef _LOG_COLORS_H_
#define _LOG_COLORS_H_

/*
 *------------------------------------------------------------------------------
 * Color encodings for console printing:
 *
 * To enable  color coded console printing: #define COLOR(clr_code)  clr_code
 * To disable color coded console printing: #define COLOR(clr_code)
 *
 * You may select a color specific to your subsystem by:
 *  #define CLRsys CLRg
 *
 *------------------------------------------------------------------------------
 */

/* Defines the supported funtionality */
#define LOG_COLOR_SUPPORTED

#if defined(LOG_COLOR_SUPPORTED)
#define COLOR(clr_code)     clr_code
#else
#define COLOR(clr_code)
#endif

/* White background */
#define CLRr                COLOR("\e[0;31m")       /* red              */
#define CLRg                COLOR("\e[0;32m")       /* green            */
#define CLRy                COLOR("\e[0;33m")       /* yellow           */
#define CLRb                COLOR("\e[0;34m")       /* blue             */
#define CLRm                COLOR("\e[0;35m")       /* magenta          */
#define CLRc                COLOR("\e[0;36m")       /* cyan             */

/* blacK "inverted" background */
#define CLRrk               COLOR("\e[0;31;40m")    /* red     on blacK */
#define CLRgk               COLOR("\e[0;32;40m")    /* green   on blacK */
#define CLRyk               COLOR("\e[0;33;40m")    /* yellow  on blacK */
#define CLRmk               COLOR("\e[0;35;40m")    /* magenta on blacK */
#define CLRck               COLOR("\e[0;36;40m")    /* cyan    on blacK */
#define CLRwk               COLOR("\e[0;37;40m")    /* white   on blacK */

/* Colored background */
#define CLRcb               COLOR("\e[0;36;44m")    /* cyan    on blue  */
#define CLRyr               COLOR("\e[0;33;41m")    /* yellow  on red   */
#define CLRym               COLOR("\e[0;33;45m")    /* yellow  on magen */

/* Generic foreground colors */
#define CLRhigh             CLRm                    /* Highlight color  */
#define CLRbold             CLRcb                   /* Bold      color  */
#define CLRbold2            CLRym                   /* Bold2     color  */
#define CLRerr              CLRwk                   /* Error     color  */
#define CLRnorm             COLOR("\e[0m")          /* Normal    color  */
#define CLRnl               CLRnorm "\n"            /* Normal + newline */

#endif
