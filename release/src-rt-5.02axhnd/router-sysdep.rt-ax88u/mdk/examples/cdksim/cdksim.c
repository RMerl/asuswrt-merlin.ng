/*
 * $Id: cdksim.c,v 1.11 Broadcom SDK $
 * $Copyright: Copyright 2013 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 *
 * Simple CDK Shell application
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <cdk/cdk_device.h>
#include <cdk/cdk_devlist.h>
#include <cdk/cdk_assert.h>
#include <cdk/cdk_printf.h>
#include <cdk/cdk_readline.h>
#include <cdk/cdk_shell.h>
#include <cdk/cdk_simhook.h>

#include <cdk/arch/xgs_cmds.h>
#include <cdk/shell/chip_cmds.h>

/*******************************************************************************
 *
 * Terminal support
 *
 * The functions below are used by the readline terminal
 * interface, and should work on most POSIX systems.
 *
 ******************************************************************************/

#ifdef SYS_HAS_TTY
#include <termios.h>

/*
 * This function ensures that the TTY returns typed characters
 * immediately and has character echo disabled.
 */
static int
_tty_set(int reset)
{
    static struct termios old, new;
    static int tty_initialized;

    if (reset) {
        /* Restore TTY settings */
        if (tty_initialized) {
            tcsetattr(0, TCSADRAIN, &old);
        }
        return 0;
    }

    if (tcgetattr(0, &old) < 0) {
        perror("tcgetattr");
    } else {
        /* Save terminal settings */
        new = old;
        /* Disable echo and buffering */
        new.c_lflag &= ~(ECHO | ICANON | ISIG);
        new.c_iflag &= ~(ISTRIP | INPCK);
        new.c_cc[VMIN] = 1;
        new.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSADRAIN, &new) < 0) {
            perror("tcsetattr");
        } else {
            tty_initialized = 1;
        }
    }
    return 0;
}

/* Read character from TTY */
static int
_tty_getchar(void)
{
    return getchar();
}

/* Send character to TTY */
static int
_tty_putchar(int ch)
{
    return putchar(ch);
}
#endif

/*******************************************************************************
 *
 * Emulation chip elements
 *
 * The functions below are used for emulating read and write
 * access for XGS devices. All registers and memories will
 * simply return the values written to them, i.e. there is
 * no attempt made to emulate self-clearing bits, etc.
 *
 ******************************************************************************/

typedef struct chelem_s {
    int unit; 
    uint32_t addrx;
    uint32_t addr; 
    uint32_t data[32]; 
} chelem_t; 

#define MAX_CHELEMS 1024*100

static chelem_t chelem[MAX_CHELEMS]; 

static chelem_t *
__find_chelem(int unit, uint32_t addrx, uint32_t addr, chelem_t *chelems)
{
    int i;
    for(i = 0; i < MAX_CHELEMS; i++) {
	if (chelems[i].addrx == addrx &&
            chelems[i].addr == addr &&
            chelems[i].unit == unit) {
	    return chelems + i;
	}
    }
    return NULL;
}

static chelem_t*
__find_or_create_chelem(int unit, uint32_t addrx, uint32_t addr, chelem_t *chelems)
{
    chelem_t *ch; 
    if (!(ch = __find_chelem(unit, addrx, addr, chelems))) {
	ch = __find_chelem(0, 0, 0, chelems); 
	CDK_ASSERT(ch); 
	ch->unit = unit; 
	ch->addrx = addrx; 
	ch->addr = addr; 
    }
    return ch; 
}

static int __read32(void *dvc, uint32_t addr, uint32_t *data)
{
    int unit = *((int*)dvc);
    chelem_t *ch; 
    ch = __find_or_create_chelem(unit, 0, addr, chelem); 
    *data = ch->data[0]; 
    return 0;
}

static int __write32(void *dvc, uint32_t addr, uint32_t data)
{
    int unit = *((int*)dvc);
    chelem_t *ch; 
    ch = __find_or_create_chelem(unit, 0, addr, chelem); 
    ch->data[0] = data;
    return 0; 
}

static int __sim_read(int unit, uint32_t addrx, uint32_t addr,
                      void *vptr, int size)
{
    chelem_t *ch; 
    ch = __find_or_create_chelem(unit, addrx, addr, chelem); 
    CDK_MEMCPY(vptr, ch->data, size);
    return 0;
}

static int __sim_write(int unit, uint32_t addrx, uint32_t addr,
                       void *vptr, int size)
{
    chelem_t *ch; 
    ch = __find_or_create_chelem(unit, addrx, addr, chelem); 
    CDK_MEMCPY(ch->data, vptr, size);
    return 0;
}

/*******************************************************************************
 *
 * Devices structures
 *
 *
 ******************************************************************************/

/*
 * The available devices in the current package configuration can be determined
 * useing the <cdk/cdk_devinfo.h> header
 *
 * We use this header to instantiate a table of all installed devices. 
 * This is a convenience for the application. 
 */

/* Instantiate the table in addition to importing the device information */
#define CDK_DEVINFO_DEFINE
#include <cdk/cdk_devinfo.h>


/*
 * These are the successfully created devices
 */
static struct {
    int unit;
} _sys_devs[CDK_CONFIG_MAX_UNITS];


/*******************************************************************************
 *
 * System interface
 *
 *
 ******************************************************************************/

/*
 * Function:
 *      sys_probe
 * Purpose:
 *      Probe for supported devices
 * Parameters:
 *      None
 * Returns:
 *      Number of CDK devices created, or -1 if an error occurred.
 * Notes:
 *      On a real PCI device with memory mapped registers the 
 *      read32/write32 functions can be omitted if a (logical)
 *      device base address is supplied instead, e.g.:
 *
 *      dv.base_addr = 0x8001000;
 */
int
sys_probe(void) {
    int ndevs = 0;
    int edx;
    cdk_dev_id_t id;
    cdk_dev_vectors_t dv;
   
    /* For each device in the system */
    for (edx = 0; cdk_devinfo_table[edx].name; edx++) {

        /* Set up device ID structure */
        memset(&id, 0, sizeof(id));
        id.vendor_id = cdk_devinfo_table[edx].vendor_id;
        id.device_id = cdk_devinfo_table[edx].device_id;
        id.revision = cdk_devinfo_table[edx].revision_id;

        /* Set up device vectors */
        memset(&dv, 0, sizeof(dv));
        dv.dvc = &_sys_devs[ndevs].unit;
        dv.read32 = __read32;
        dv.write32 = __write32;

        /* Callback context is unit number */
        _sys_devs[ndevs].unit = ndevs;

        /* Create device */
        if (cdk_dev_create(&id, &dv, 0) >= 0) {
            if (++ndevs >= COUNTOF(_sys_devs)) {
                break;
            }
        }
    }

    /* Install generic simulation hooks */
    cdk_simhook_read = __sim_read;
    cdk_simhook_write = __sim_write;

    return ndevs;
}

/*
 * Function:
 *      sys_puts
 * Purpose:
 *      Default console output function
 * Parameters:
 *      str - nul-terminated string to print
 * Returns:
 *      Number of characters written
 */
int
sys_puts(const char *str)
{
    return fputs(str, stdout);
}

/*
 * Function:
 *      sys_gets
 * Purpose:
 *      Default console input function
 * Parameters:
 *      prompt - prompt
 *      buf - (OUT) input string buffer
 *      max - size of input string buffer
 * Returns:
 *      Pointer to input string (in buf)
 */
char *
sys_gets(const char *prompt, char *buf, int max)
{
#ifdef SYS_HAS_TTY
    _tty_set(0);
    cdk_readline(_tty_getchar, _tty_putchar, prompt, buf, max);
    _tty_set(1);
#else
    int len;

    write(0, prompt, strlen(prompt));
    if ((len = read(0, buf, max)) <= 0) {
        buf[0] = 0;
    } else {
        buf[len-1] = 0;
    }
#endif
    return buf;
}

/*
 * Main entry
 */
int main(void)
{
#ifndef USE_SYSTEM_LIBC
    /*
     * Supply a console output function for the printf function
     * in the CDK C library.
     */
    cdk_printhook = sys_puts;
#endif

    printf("\nProbing for devices ...\n");

    /* Probe for CDK devices */
    sys_probe();

    printf("\nStarting CDK Debug Shell ...\n\n");

    /* Initialize Shell */
    cdk_shell_init(); 

    /* Add commands to the CDK shell */
    cdk_shell_add_xgs_core_cmds();

    /* Launch the CDK shell with a prompt and an input function */
    cdk_shell("CDK", sys_gets);

    return 0;
}
