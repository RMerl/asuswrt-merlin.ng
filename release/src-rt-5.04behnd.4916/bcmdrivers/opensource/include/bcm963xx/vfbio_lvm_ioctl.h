/*
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom 
   All Rights Reserved

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
/*******************************************************************************
 *
 * vfbio_lvm_ioctl.h    VFBIO LVM user interface - ioctl access
 * June 7, 2022
 * Igor Ternovsky
 *
 *******************************************************************************/
#ifndef _VFBIO_LVM_IOCTL_H_
#define _VFBIO_LVM_IOCTL_H_

#ifdef __KERNEL__
#include <linux/ioctl.h>
#else
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
#include <vfbio_lvm.h>

#define VFBIO_CLASS_NAME    "vfbio-lvm"
#define VFBIO_DEV_NAME      VFBIO_CLASS_NAME "0"

typedef struct
{
    int rc;

    struct
    {
        int lun_id;
        int lun_flags;
        char lun_name[VFBIO_LVM_MAX_NAME_SIZE];
        uint64_t size; /* in bytes*/
    } create;

    struct
    {
        int lun_id;
    } destroy;

    struct
    {
        int lun_id;
        uint64_t size; /* in bytes */
    } resize;

    struct
    {
        int num_luns;
        vfbio_lun_id_name id_name_pairs[VFBIO_LVM_MAX_ID_NAME_PAIRS];
    } rename;

    struct
    {
        int lun_id;
        int read_only;
    } chmod;

    struct
    {
        int lun_id;
        struct vfbio_lun_descr info;
    } lun_info;

    struct
    {
        uint64_t total_size;
        uint64_t free_size;
    } device_info;

    struct
    {
        int lun_id;
        char lun_name[VFBIO_LVM_MAX_NAME_SIZE];
    } get_id;

    struct
    {
        int lun_id;
    } get_next;

    struct
    {
        int lun_id;
        uint32_t size;
        void *data;
    } write;

} vfbio_lvm_ioctl_param;

#define VFBIO_LVM_IOCTL_MAGIC 'V'

#define VFBIO_LVM_IOCTL_OP_CREATE       _IOW(VFBIO_LVM_IOCTL_MAGIC, 1, vfbio_lvm_ioctl_param)
#define VFBIO_LVM_IOCTL_OP_DELETE       _IOW(VFBIO_LVM_IOCTL_MAGIC, 2, vfbio_lvm_ioctl_param)
#define VFBIO_LVM_IOCTL_OP_RESIZE       _IOW(VFBIO_LVM_IOCTL_MAGIC, 3, vfbio_lvm_ioctl_param)
#define VFBIO_LVM_IOCTL_OP_RENAME       _IOW(VFBIO_LVM_IOCTL_MAGIC, 4, vfbio_lvm_ioctl_param)
#define VFBIO_LVM_IOCTL_OP_CHMOD        _IOW(VFBIO_LVM_IOCTL_MAGIC, 5, vfbio_lvm_ioctl_param)
#define VFBIO_LVM_IOCTL_OP_VOLUME_INFO  _IOW(VFBIO_LVM_IOCTL_MAGIC, 6, vfbio_lvm_ioctl_param)
#define VFBIO_LVM_IOCTL_OP_DEVICE_INFO  _IOW(VFBIO_LVM_IOCTL_MAGIC, 7, vfbio_lvm_ioctl_param)
#define VFBIO_LVM_IOCTL_OP_GET_ID       _IOW(VFBIO_LVM_IOCTL_MAGIC, 8, vfbio_lvm_ioctl_param)
#define VFBIO_LVM_IOCTL_OP_GET_NEXT     _IOW(VFBIO_LVM_IOCTL_MAGIC, 9, vfbio_lvm_ioctl_param)
#define VFBIO_LVM_IOCTL_OP_WRITE        _IOW(VFBIO_LVM_IOCTL_MAGIC,10, vfbio_lvm_ioctl_param)

#endif
