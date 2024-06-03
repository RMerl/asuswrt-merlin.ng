/*
 * <:copyright-BRCM:2015:GPL/GPL:standard
 * 
 *    Copyright (c) 2015 Broadcom 
 *    All Rights Reserved
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
 */

#include <linux/version.h>
#include <linux/slab.h>

/* The location folder of the semaphore.h file changed at Kernel version 2.6.26 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
#include <linux/semaphore.h>
#else
#include <asm/semaphore.h>
#endif

#include "target.h"
#include "gki_int.h"
#include "bt_types.h"


volatile unsigned int _gki_lock_nesting = 0;
volatile pid_t LockThread = 0xffff;    // thread currently holding the spinlock
DEFINE_SPINLOCK(Lock);
volatile unsigned long Kirql; // saved execution priority

/* Define the structure that holds the GKI variables
*/
#ifndef _BT_DYNAMIC_MEMORY
tGKI_CB gki_cb = {0};
#else
tGKI_CB *gp_gki_cb = NULL;
#endif

void LogMsg(int TraceMask, const char *format, ...)
{
    char        temp1[400];
    char        *temp = temp1;
    va_list     Next;
    int         len;

    // Go ahead and trace...
    va_start(Next, format);
    len = _vsnprintf(temp, 380, format, Next);
    va_end(Next);


    if ((len < 0) || (len > 380))
    {
        len = strlen(temp);
    }

    if (len > 380)
    {
        temp[380] = 0;
        len = 380;

    }

    if (temp[len - 1] >= ' ')
    {
        temp[len] = '\n';
        temp[len + 1] = '\0';
    }


    printk("%s", temp);
    return;
}

/*******************************************************************************
**
** Function         GKI_init
**
** Description      This function is called once at startup to initialize
**                  all the timer structures.
**
** Returns          void
**
*******************************************************************************/
void GKI_init(void)
{
    memset(&gki_cb, 0, sizeof(gki_cb));

    gki_buffer_init();

    spin_lock_init(&Lock);
    gki_cb.IsRunning = TRUE;
}


/*******************************************************************************
**
** Function         GKI_shutdown
**
** Description      This function is called to shut down GKI
**
** Returns          void
**
*******************************************************************************/
void GKI_shutdown(void)
{
    int i;

     if (!gki_cb.IsRunning)
        return;

    gki_cb.IsRunning = FALSE;

#if (GKI_USE_DYNAMIC_BUFFERS == TRUE)

    for (i = 0; i < GKI_NUM_TOTAL_BUF_POOLS; i++)
    {
        if (gki_cb.pool_buf_size[i])
            GKI_delete_pool((UINT8)i);
    }

#endif

}

/*******************************************************************************
**
** Function         GKI_enable
**
** Description      This function enables interrupts.
**
** Returns          void
**
*******************************************************************************/
void GKI_enable(void)
{
    spin_unlock_irqrestore(&Lock, Kirql);
}

/*******************************************************************************
**
** Function         GKI_disable
**
** Description      This function disables interrupts.
**
** Returns          void
**
*******************************************************************************/
void GKI_disable(void)
{
    volatile unsigned long tmp_irql;

    spin_lock_irqsave(&Lock, tmp_irql);

    // OK, we've acquired the spinlock.  It means that
    // the other guy has released it already and we can
    // safely overwrite his saved irql...
    Kirql = tmp_irql;
}


/*******************************************************************************
**
** Function         GKI_exception
**
** Description      This function throws an exception.
**                  This is normally only called for a non recoverable error.
**
** Parameters:      code    -  (input) The code for the error
**                  msg     -  (input) The message that has to be logged
**
** Returns          void
**
*******************************************************************************/
void GKI_exception(UINT16 code, const char *msg, ...)
{
    char        buff[MAX_EXCEPTION_MSGLEN];
    va_list     Next;
    int         len;

    va_start(Next, msg);
    len = vsnprintf(buff, MAX_EXCEPTION_MSGLEN - 1, msg, Next);
    va_end(Next);

    LogMsg(-1, "GKI_exception: Entry Code %d Message %s\n", code, buff);
    GKI_disable();

    if (gki_cb.ExceptionCnt < MAX_EXCEPTION)
    {
        EXCEPTION_T *pExp;

        pExp =  &gki_cb.Exception[gki_cb.ExceptionCnt++];
        pExp->type = code;
        pExp->taskid = 0;
        strncpy((INT8 *)pExp->msg, msg, MAX_EXCEPTION_MSGLEN - 1);
    }

    GKI_enable();

    LogMsg(-1, "GKI_Exception called with code: %d", code);

    return;
}


/*******************************************************************************
**
** Function         gki_reserve_os_memory
**
** Description      This function allocates memory
**
** Parameters:      size -  (input) The size of the memory that has to be
**                  allocated
**
** Returns          the address of the memory allocated, or NULL if failed
**
** NOTE             This function is NOT called by the Widcomm stack and
**                  profiles. It is only called from within GKI if dynamic
**                  buffer pools are used.
**
*******************************************************************************/
void *gki_reserve_os_memory(UINT32 size)
{
    void *p;
    p = kmalloc(size, GFP_ATOMIC);
    if (p == NULL)
        return NULL;
    memset(p, 0x00, sizeof(*p));
    return p;
}

/*******************************************************************************
**
** Function         gki_release_os_memory
**
** Description      This function frees memory
**
** Parameters:      size -  (input) The address of the memory that has to be
**                  freed
**
** Returns          void
**
** NOTE             This function is NOT called by the Widcomm stack and
**                  profiles. It is only called from within GKI if dynamic
**                  buffer pools are used.
**
*******************************************************************************/
void gki_release_os_memory(void *p_mem)
{
    kfree(p_mem);

    return;
}


