/*
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom
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
 ****************************************************************************
 * Logical Volume Manegement user space API implementation
 *
 * Author: Igor Ternovsky <igor.ternovsky@broadcom.com>
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

#include <vfbio_lvm_ioctl.h>

#define VFBIO_CHARDEV_NAME  "/dev/" VFBIO_DEV_NAME

static int vfbio_ioctl_request(uint32_t cmd, vfbio_lvm_ioctl_param *params)
{
    int fd;
    int rc;

    fd = open(VFBIO_CHARDEV_NAME, O_RDWR);
    if (fd < 0)
    {
        fprintf(stderr, "Can't open file %s\n", VFBIO_CHARDEV_NAME);
        return -errno;
    }
    rc = ioctl(fd, cmd, params);
    close(fd);
    if (rc < 0)
    {
        fprintf(stderr, "ioctl request failed with error %s\n", strerror(errno));
        return -errno;
    }
    return rc ? rc : params->rc;
}

/* Create a new dynamic lun */
int vfbio_lun_create(const char *name, uint64_t size, int *lun_id)
{
    vfbio_lvm_ioctl_param params = {
        .create.lun_id = *lun_id,
        .create.size = size
    };
    int rc;
    if (name)
        strncpy(params.create.lun_name, name, sizeof(params.create.lun_name) - 1);
    rc = vfbio_ioctl_request(VFBIO_LVM_IOCTL_OP_CREATE, &params);
    if (!rc)
        *lun_id = params.create.lun_id;
    return rc;
}

/* Delete dynamic lun created by vfbio_lun_create */
int vfbio_lun_delete(int lun_id)
{
    vfbio_lvm_ioctl_param params = {
        .destroy.lun_id = lun_id,
    };
    int rc;

    rc = vfbio_ioctl_request(VFBIO_LVM_IOCTL_OP_DELETE, &params);
    return rc;
}

/* Resize dynamic lun */
int vfbio_lun_resize(int lun_id, uint64_t size)
{
    vfbio_lvm_ioctl_param params = {
        .resize.lun_id = lun_id,
        .resize.size = size
    };
    int rc;

    rc = vfbio_ioctl_request(VFBIO_LVM_IOCTL_OP_RESIZE, &params);

    return rc;
}

/* Rename 1 or multiple luns atomically (all or nothing) */
int vfbio_lun_rename(uint8_t num_luns, struct vfbio_lun_id_name id_name[])
{
    vfbio_lvm_ioctl_param params = {
        .rename.num_luns = num_luns,
    };
    int rc;

    if (num_luns > VFBIO_LVM_MAX_ID_NAME_PAIRS)
    {
        fprintf(stderr, "Can't rename more than %d luns at once\n", VFBIO_LVM_MAX_ID_NAME_PAIRS);
        return -ENOTSUP;
    }
    for (unsigned i = 0; i < num_luns; i++)
    {
        params.rename.id_name_pairs[i] = id_name[i];
    }

    rc = vfbio_ioctl_request(VFBIO_LVM_IOCTL_OP_RENAME, &params);

    return rc;
}

/* Set lun access type (read-write or read-only) */
int vfbio_lun_chmod(int lun_id, int read_only)
{
    vfbio_lvm_ioctl_param params = {
        .chmod.lun_id = lun_id,
        .chmod.read_only = read_only
    };
    int rc;

    rc = vfbio_ioctl_request(VFBIO_LVM_IOCTL_OP_CHMOD, &params);

    return rc;
}

/* Get lun info */
int vfbio_lun_get_info(int lun_id, struct vfbio_lun_descr *lun_descr)
{
    vfbio_lvm_ioctl_param params = {
        .lun_info.lun_id = lun_id,
    };
    int rc;

    if (!lun_descr)
        return -EINVAL;
    rc = vfbio_ioctl_request(VFBIO_LVM_IOCTL_OP_VOLUME_INFO, &params);
    if (!rc)
        *lun_descr = params.lun_info.info;

    return rc;
}

/* Get device info */
int vfbio_device_get_info(uint64_t *total_size, uint64_t *free_size)
{
    vfbio_lvm_ioctl_param params = {};
    int rc;

    if (!total_size || !free_size)
        return -EINVAL;
    rc = vfbio_ioctl_request(VFBIO_LVM_IOCTL_OP_DEVICE_INFO, &params);
    if (!rc)
    {
        *total_size = params.device_info.total_size;
        *free_size = params.device_info.free_size;
    }

    return rc;
}

/* Get lun id by name */
int vfbio_lun_get_id(const char *name, int *id)
{
    vfbio_lvm_ioctl_param params = {};
    int rc;

    if (!name || !id)
        return -EINVAL;
    strncpy(params.get_id.lun_name, name, sizeof(params.get_id.lun_name));
    rc = vfbio_ioctl_request(VFBIO_LVM_IOCTL_OP_GET_ID, &params);
    if (!rc)
    {
        *id = params.get_id.lun_id;
    }

    return rc;
}

/* Volume iterator. */
int vfbio_lun_get_next(int prev, int *lun_id)
{
    vfbio_lvm_ioctl_param params = {};
    int rc;

    if (!lun_id)
        return -EINVAL;
    params.get_next.lun_id = prev;
    rc = vfbio_ioctl_request(VFBIO_LVM_IOCTL_OP_GET_NEXT, &params);
    if (!rc)
    {
        *lun_id = params.get_next.lun_id;
    }

    return rc;
}

int vfbio_lun_write(int lun_id, void *data, uint32_t size)
{
    vfbio_lvm_ioctl_param params = {};
    int rc;

    params.write.lun_id = lun_id;
    params.write.size = size;
    params.write.data = data;
    rc = vfbio_ioctl_request(VFBIO_LVM_IOCTL_OP_WRITE, &params);

    return rc;
}

/* Get error text */
const char *vfbio_error_str(int err)
{
    const char *err_text;
    if (err > VFBIO_ERROR__FIRST)
        return strerror((err < 0) ? -err : err);
    switch(err)
    {
        case VFBIO_ERROR_OK:
            err_text = "OK";
            break;
        case VFBIO_ERROR_PARMS:
            err_text = "Error in parameters";
            break;
        case VFBIO_ERROR_LUN_INVALID:
            err_text = "Invalid LUN index or name";
            break;
        case VFBIO_ERROR_LUN_NOT_OPENED:
            err_text = "LUN is not opened";
            break;
        case VFBIO_ERROR_LUN_IDX_EXISTS:
            err_text = "LUN with such index already exists";
            break;
        case VFBIO_ERROR_LUN_NAME_EXISTS:
            err_text = "LUN with such name already exists";
            break;
        case VFBIO_ERROR_LUN_IS_NOT_DYNAMIC:
            err_text = "LUN is not dynamic";
            break;
        case VFBIO_ERROR_NO_MORE_LUNS:
            err_text = "No more LUNs";
            break;
        case VFBIO_ERROR_TRANSLATION:
            err_text = "Address translation error";
            break;
        case VFBIO_ERROR_NO_MEM:
            err_text = "Dynamic memory allocation failed";
            break;
        case VFBIO_ERROR_NO_ROOM:
            err_text = "No room on flash";
            break;
        case VFBIO_ERROR_NO_WRITE_PERMISISON:
            err_text = "No write permission";
            break;
        case VFBIO_ERROR_NO_READ_PERMISISON:
            err_text = "No read permission";
            break;
        case VFBIO_ERROR_INTERNAL:
            err_text = "Internal error";
            break;
        case VFBIO_ERROR_INVALID_DEVICE:
            err_text = "Invalid device index";
            break;
        case VFBIO_ERROR_INVALID_DEVICE_PAGE:
            err_text = "Invalid device page";
            break;
        case VFBIO_ERROR_NO_LVM_LUN:
            err_text = "No LUN for for saving dynamic configuration";
            break;
        case VFBIO_ERROR_CANT_SAVE_CONFIG:
            err_text = "Can't save dynamic LUN configuration";
            break;
        case VFBIO_ERROR_CANT_LOAD_CONFIG:
            err_text = "Can't load dynamic LUN configuration";
            break;
        case VFBIO_ERROR_LVM_IS_NOT_SUPPORTED:
            err_text = "Logical LUN management is not supported";
            break;
        default:
            err_text = "Unknown error";
            break;
    }
    return err_text;
}
