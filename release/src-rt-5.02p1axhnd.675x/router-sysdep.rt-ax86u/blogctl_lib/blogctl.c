/***********************************************************************
 *
 *  Copyright (c) 2006-2010  Broadcom Corporation
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

/***************************************************************************
 * File Name  : blogctl_api.c
 * Description: Linux command line utility that controls the Broadcom blog
 ***************************************************************************/

/*** Includes. ***/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include <blog_ioctl.h>
#include <blogctl_api.h>

static int  blog_dev_open(void);
static int  blog_dev_ioctl(blog_ioctl_t ioctl, int arg1, int arg2);

/*
 *------------------------------------------------------------------------------
 * Function Name: blogctl_get_stats
 * Description  : Get blog stats
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int blogctl_get_stats(void)
{
    return blog_dev_ioctl( BLOG_IOCTL_GET_STATS, 0, 0 );
}

/*
 *------------------------------------------------------------------------------
 * Function Name: blogctl_reset_stats
 * Description  : Reset blog stats
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int blogctl_reset_stats(void)
{
    return blog_dev_ioctl( BLOG_IOCTL_RESET_STATS, 0, 0 );
}

/*
 *------------------------------------------------------------------------------
 * Function Name: blogctl_config
 * Description  : Configures the flow cache parameters.
 * Parameters   :
 *       option : one of the option to be configured.
 *         arg1 : parameter value
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int blogctl_config(int option, int arg1)
{
    int ret = BLOG_ERROR;

    switch (option)
    {
        case BLOG_CONFIG_OPT_DUMP_BLOG:
            ret = blog_dev_ioctl( BLOG_IOCTL_DUMP_BLOG, -1, arg1 );
            break;

        default:
            fprintf( stderr, "invalid config option <%d>\n", option );
            ret = BLOG_ERROR;
    }

    return ret;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: blog_dev_open
 * Description  : Opens the flow cache device.
 * Returns      : device handle if successsful or -1 if error
 *------------------------------------------------------------------------------
 */
static int blog_dev_open(void)
{
    int fd = open( BLOG_DRV_DEVICE_NAME, O_RDWR );
    if ( fd == -1 )
    {
        fprintf( stderr, "open <%s> error no %d\n",
                 BLOG_DRV_DEVICE_NAME, errno );
        return BLOG_ERROR;
    }
    return fd;
} /* blog_dev_open */

/*
 *------------------------------------------------------------------------------
 * Function Name: blog_dev_ioctl
 * Description  : Ioctls into fcache driver passing the IOCTL command, and the
 *                fcache (arg1) and id (arg2) passed as a 16bit tuple.
 *                If arg1 == -1, then arg2 is passed directly.
 *                CAUTION: Display is done in kernel context.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
static int blog_dev_ioctl(blog_ioctl_t ioctl_cmd, int arg1, int arg2)
{
    int dev_fd, arg = 0, ret = BLOG_ERROR;

    if ( arg1 == -1 )
        arg = arg2;
    else
        arg = ( ( (arg1 & 0xFF) << 8) | (arg2 & 0xFF) ) ;

    if ( ( dev_fd = blog_dev_open() ) == BLOG_ERROR )
        return BLOG_ERROR;

    if ( (ret = ioctl( dev_fd, ioctl_cmd, arg )) == BLOG_ERROR )
        fprintf( stderr, "blog_dev_ioctl <%d> error\n", ioctl_cmd );

    close( dev_fd );
    return ret;
}


