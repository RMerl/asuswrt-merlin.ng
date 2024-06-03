/***********************************************************************
 *
 * Copyright (c) 2019  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2019:DUAL/GPL:standard
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
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
