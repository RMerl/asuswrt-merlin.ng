#ifndef __FAP4KE_COLORS_H_INCLUDED__
#define __FAP4KE_COLORS_H_INCLUDED__

/*
 <:copyright-BRCM:2009:DUAL/GPL:standard
 
    Copyright (c) 2009 Broadcom 
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
/*
 *--------------------------------------------------------------------------
 * Color encodings for console printing:
 *
 * This feature is controlled from top level make menuconfig, under
 * Debug Selection>Enable Colorized Prints
 *
 * You may select a color specific to your subsystem by:
 *  #define CLRsys CLRg
 *
 * Usage:  PRINT(CLRr "format" CLRNL);
 *--------------------------------------------------------------------------
 */

#ifdef CONFIG_BCM_COLORIZE_PRINTS
#define BCMCOLOR(clr_code)     clr_code
#else
#define BCMCOLOR(clr_code)
#endif

/* White background */
#define CLRr             BCMCOLOR("\e[0;31m")       /* red              */
#define CLRg             BCMCOLOR("\e[0;32m")       /* green            */
#define CLRy             BCMCOLOR("\e[0;33m")       /* yellow           */
#define CLRb             BCMCOLOR("\e[0;34m")       /* blue             */
#define CLRm             BCMCOLOR("\e[0;35m")       /* magenta          */
#define CLRc             BCMCOLOR("\e[0;36m")       /* cyan             */

/* blacK "inverted" background */
#define CLRrk            BCMCOLOR("\e[0;31;40m")    /* red     on blacK */
#define CLRgk            BCMCOLOR("\e[0;32;40m")    /* green   on blacK */
#define CLRyk            BCMCOLOR("\e[0;33;40m")    /* yellow  on blacK */
#define CLRmk            BCMCOLOR("\e[0;35;40m")    /* magenta on blacK */
#define CLRck            BCMCOLOR("\e[0;36;40m")    /* cyan    on blacK */
#define CLRwk            BCMCOLOR("\e[0;37;40m")    /* whilte  on blacK */

/* Colored background */
#define CLRcb            BCMCOLOR("\e[0;36;44m")    /* cyan    on blue  */
#define CLRyr            BCMCOLOR("\e[0;33;41m")    /* yellow  on red   */
#define CLRym            BCMCOLOR("\e[0;33;45m")    /* yellow  on magen */

/* Generic foreground colors */
#define CLRhigh          CLRm                    /* Highlight color  */
#define CLRbold          CLRcb                   /* Bold      color  */
#define CLRbold2         CLRym                   /* Bold2     color  */
#define CLRerr           CLRyr                   /* Error     color  */
#define CLRnorm          BCMCOLOR("\e[0m")       /* Normal    color  */
#define CLRnl            CLRnorm "\n"            /* Normal + newline */

/* Each subsystem may define CLRsys */

#endif /* __FAP4KE_COLORS_H_INCLUDED__ */
