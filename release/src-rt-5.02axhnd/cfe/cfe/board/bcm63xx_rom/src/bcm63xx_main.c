/*
<:label-BRCM:2011:NONE:standard

     :> 
*/

/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  Main Module              File: bcm63xx_main.c       
    *  
    *  This module contains the main "C" routine for CFE bootstrap loader 
    *  and decompressor to decompress the real CFE to ram and jump over.
    *
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *  Revised: seanl
    *  
    *********************************************************************  
    *
    <:label-BRCM:2012:proprietary:standard
    *
    *  Copyright 2000,2001,2002,2003
    *  Broadcom. All rights reserved.
    *  
    *  This software is furnished under license and may be used and 
    *  copied only in accordance with the following terms and 
    *  conditions.  Subject to these conditions, you may download, 
    *  copy, install, use, modify and distribute modified or unmodified 
    *  copies of this software in source and/or binary form.  No title 
    *  or ownership is transferred hereby.
    *  
    *  1) Any source code used, modified or distributed must reproduce 
    *     and retain this copyright notice and list of conditions 
    *     as they appear in the source file.
    *  
    *  2) No right is granted to use any trade name, trademark, or 
    *     logo of Broadcom Corporation.  The "Broadcom Corporation" 
    *     name may not be used to endorse or promote products derived 
    *     from this software without the prior written permission of 
    *     Broadcom Corporation.
    *  
    *  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
    *     IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
    *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
    *     PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
    *     SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
    *     PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
    *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
    *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    *     GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    *     BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
    *     OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
    *     TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
    *     THE POSSIBILITY OF SUCH DAMAGE.
    :> 
    ********************************************************************* */
#include "rom_main.h"
#include "bcm63xx_utils.h"

void cfe_main(int,int);
/*  *********************************************************************
    *  cfe_main(a,b)
    *  
    *  It's gotta start somewhere.
    *  Input parameters: 
    *      a,b - not used
    *      
    *  Return value:
    *      does not return
    ********************************************************************* */
void cfe_main(int a,int b)
{   
    bootInit();

#if (BOOT_PRE_CFE==1)
    bootPreCfeImage();
#else

#if (INC_MEMSYS_INIT==1) && !defined(IKOS_NO_DDRINIT) && !defined(IKOS_SMPL_DDRINIT)
    ddrInit();
#endif

#if defined(CFE_ROM_STOP) || (CFG_BOOT_PSRAM==1)
    stopCfeRom();
#endif

#if defined(IKOS_BD_LINUX_ROM)
    launchLinux();
#endif

    boot_media();

#endif
}
