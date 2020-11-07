/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <sys/ioctl.h>

#include "bpmctl_common.h"
#include "bpmctl_api.h"


/*
 *------------------------------------------------------------------------------
 * Function Name: bpmctl_open
 * Description  : Opens the BPM device.
 * Returns      : device handle if successsful or -1 if error
 *------------------------------------------------------------------------------
 */
static int bpmctl_open(void)
{
    int nFd = open( BPM_DRV_DEVICE_NAME, O_RDWR );
    if ( nFd == -1 )
    {
        fprintf( stderr, "open <%s> error no %d\n",
                 BPM_DRV_DEVICE_NAME, errno );
        return BPMCTL_ERROR;
    }
    return nFd;
} /* bpmctl_open */

/*
 *------------------------------------------------------------------------------
 * Function Name: bpmctl_ioctl
 * Description  : Ioctls into bpmache driver passing the IOCTL command, and the
 *                bpmache (arg1) and id (arg2) passed as a 16bit tuple.
 *                If arg1 == -1, then arg2 is passed directly.
 *                CAUTION: Display is done in kernel context.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
static int bpmctl_ioctl(bpmctl_ioctl_t ioctl_cmd, void *arg)
{
    int devFd, ret = BPMCTL_ERROR;

    if ( ( devFd = bpmctl_open() ) == BPMCTL_ERROR )
        return BPMCTL_ERROR;

    if ( (ret = ioctl( devFd, ioctl_cmd, (uintptr_t) arg )) == BPMCTL_ERROR )
        fprintf( stderr, "bpmctl_ioctl <%d> error\n", ioctl_cmd );

    close( devFd );
    return ret;
}


int bcm_bpmctl_dump_status( void )
{
    int err = 0;
    bpmctl_data_t ifdata;
    bpmctl_data_t *bpm = &ifdata;

    bpm->subsys = BPMCTL_SUBSYS_STATUS;
    bpm->op = BPMCTL_OP_DUMP;
    if ((err = bpmctl_ioctl(BPMCTL_IOCTL_SYS, (void *) bpm))) {
        if (err == BPMCTL_ERROR)
            fprintf(stderr, "ioctl command return error %d!\n", err);
        goto out;
    }

out:
    return err;
}


int bcm_bpmctl_dump_thresh( void )
{
    int err = 0;
    bpmctl_data_t ifdata;
    bpmctl_data_t *bpm = &ifdata;

    bpm->subsys = BPMCTL_SUBSYS_THRESH;
    bpm->op = BPMCTL_OP_DUMP;
    if ((err = bpmctl_ioctl(BPMCTL_IOCTL_SYS, (void *) bpm))) {
        if (err == BPMCTL_ERROR)
            fprintf(stderr, "ioctl command return error %d!\n", err);
        goto out;
    }

out:
    return err;
}


int bcm_bpmctl_dump_buffers( void )
{
    int err = 0;
    bpmctl_data_t ifdata;
    bpmctl_data_t *bpm = &ifdata;

    bpm->subsys = BPMCTL_SUBSYS_BUFFERS;
    bpm->op = BPMCTL_OP_DUMP;
    if ((err = bpmctl_ioctl(BPMCTL_IOCTL_SYS, (void *) bpm))) {
        if (err == BPMCTL_ERROR)
            fprintf(stderr, "ioctl command return error %d!\n", err);
        goto out;
    }

out:
    return err;
}


#if defined(BPM_TRACK)
int bcm_bpmctl_track( bpmctl_track_t * trk )
{
    int err = 0;
    bpmctl_data_t ifdata;
    bpmctl_data_t *bpm = &ifdata;

    bpm->subsys = BPMCTL_SUBSYS_TRACK;
    bpm->op = BPMCTL_OP_DUMP;
    memcpy(&bpm->track, trk, sizeof(bpmctl_track_t));
    if ((err = bpmctl_ioctl(BPMCTL_IOCTL_SYS, (void *) bpm))) {
        if (err == BPMCTL_ERROR)
            fprintf(stderr, "ioctl command return error %d!\n", err);
        goto out;
    }

out:
    return err;
}
#endif

int bcm_bpmctl_dump_skbuffs( void )
{
    int err = 0;
    bpmctl_data_t ifdata;
    bpmctl_data_t *bpm = &ifdata;

    bpm->subsys = BPMCTL_SUBSYS_SKBUFFS;
    bpm->op = BPMCTL_OP_DUMP;
    if ((err = bpmctl_ioctl(BPMCTL_IOCTL_SYS, (void *) bpm))) {
        if (err == BPMCTL_ERROR)
            fprintf(stderr, "ioctl command return error %d!\n", err);
        goto out;
    }

out:
    return err;
}

