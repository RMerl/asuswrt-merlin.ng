/***************************************************************************
 * Broadcom Corp. Confidential
 * Copyright 2001 Broadcom Corp. All Rights Reserved.
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED
 * SOFTWARE LICENSE AGREEMENT BETWEEN THE USER AND BROADCOM.
 * YOU HAVE NO RIGHT TO USE OR EXPLOIT THIS MATERIAL EXCEPT
 * SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 ***************************************************************************
 * File Name  : ac97drv.h
 *
 * Description: 
 *
 ***************************************************************************/

#if !defined(AC97DRV_H)
#define AC97DRV_H

#if defined(__cplusplus)
extern "C" {
#endif


typedef enum AC97IOCTL_INDEX
{
   AC97IO_INIT_INDEX = 0,
   AC97IO_MAX_INDEX
} AC97IOCTL_INDEX;


/* Defines. */
#define AC97DRV_MAJOR            222 /* arbitrary unused value */

#define AC97IOCTL_INIT \
    _IOWR(AC97DRV_MAJOR, AC97IO_INIT_INDEX, AC97DRV_INIT_PARAM)



#define MAX_AC97DRV_IOCTL_COMMANDS   AC97IO_MAX_INDEX



typedef struct
{
   unsigned int      size;    /* Size of the structure (including the size field) */
   unsigned short    devNum;  /* device number to be initialized */
   unsigned long     data;
} AC97DRV_INIT_PARAM, *PAC97DRV_INIT_PARAM;


AC97DRV_MODE_CTRL_PARAM, *PAC97DRV_MODE_CTRL_PARAM;


#if defined(__cplusplus)
}
#endif

#endif // AC97DRV_H
