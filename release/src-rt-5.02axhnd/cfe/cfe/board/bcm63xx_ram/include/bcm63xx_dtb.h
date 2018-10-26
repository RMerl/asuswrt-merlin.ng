/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
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
#if !defined(__BCM63XX_DTB_H__)
#define __BCM63XX_DTB_H__

#include "lib_types.h"
#include "libfdt_env.h"
#include "fdt.h"
#include "libfdt.h"

#define MAX_RESERVED_MEM_NUM         16
#define BRCM_BLPARMS_PROP            "brcm_blparms"
#define BRCM_NVRAM_PROP              "brcm_nvram"
#define BRCM_CFEVER_PROP             "brcm_cfever"
#define BRCM_ROOTFS_SHA256_PROP      "brcm_rootfs_sha256"
#define BRCM_ROOTFS_IMGLEN_PROP      "brcm_rootfs_imglen"

#define OF_NODE_SIZE_CELLS_DEFAULT 0x1
#define OF_NODE_ADDR_CELLS_DEFAULT 0x2

#define MAX_NUM_DTB                2
#define DTB_ID_CFE                 0
#define DTB_ID_OPTEE               1

typedef enum DTB_PREPARE_OPT {
   DTB_PREPARE_NONE = 0,
   DTB_PREPARE_INIT,
   DTB_PREPARE_FDT_DEF,
   DTB_PREPARE_FDT,
   DTB_PREPARE_DONE
} dtb_prepare_opt_t;

typedef struct _dtb_ctx {
    void* dtb_ptr;
    int total_len;
    int dtb_size;
    dtb_prepare_opt_t status;
} dtb_ctx_t;

int dtb_init(int dtb_id, void* dtb_addr, int len);
int dtb_done(int dtb_id);
dtb_prepare_opt_t dtb_get_prepare_status(int dtb_id);
int dtb_set_memory(int dtb_id, uint64_t memsize);
int dtb_set_cmdline(int dtb_id, char* cmdline);
int dtb_set_reserved_memory(int dtb_id, char* name, uint64_t addr, uint64_t size);
int dtb_get_reserved_memory(int dtb_id, char* name, uint64_t* addr, uint64_t* size);
int dtb_del_reserved_memory(int dtb_id, char* name);
int dtb_get_cpu_rel_addr(int dtb_id, uint64_t* rel_addr, int cpu);
int dtb_set_blparms(int dtb_id, const char* parms, unsigned int size);
int dtb_append_data_blob(int dtb_id, const char* blob_name, const char * blob_data, 
                         unsigned int blob_size );
int dtb_getprop_reg(int dtb_id, const char* node_path_par, const char *node_path,
                    uint64_t* addr, uint64_t* size);
int dtb_set_chosen_initrd(int dtb_id, uint64_t initrd_addr, uint64_t initrd_size);
int dtb_set_chosen_root(int dtb_id, char * root_device_full_pathname );
int dtb_preappend_bootargs(int dtb_id, char * tail_bootargs );
int dtb_get_fdt_size(int dtb_id);
const void* dtb_get_fdt(int dtb_id);
const void *dtb_get_prop(int dtb_id, const char *node_path,
                         const char *property, int *len);
int dtb_prepare(int dtb_id, enum DTB_PREPARE_OPT prep_opt, const char* dtb_name, 
                const unsigned char* img, int size);
#if defined (_BCM96858_) || defined (_BCM94908_) || defined (_BCM963158_) || defined (_BCM96846_) || defined (_BCM96856_) || defined (_BCM963138_)
int dtb_set_nr_cpus(int dtb_id);
#endif
#endif
