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

#define OF_NODE_SIZE_CELLS_DEFAULT 0x1
#define OF_NODE_ADDR_CELLS_DEFAULT 0x2

typedef enum DTB_PREPARE_OPT {
   DTB_PREPARE_FDT_DEF,
   DTB_PREPARE_FDT,
   DTB_PREPARE_NONE
} dtb_prepare_opt_t;

void dtb_init(void);
int dtb_done(void);
dtb_prepare_opt_t dtb_get_prepare_status(void);
int dtb_set_memory_size(uint64_t memsize);
int dtb_set_memory_addr(uint64_t memaddr);
int dtb_set_cmdline(char* cmdline);
int dtb_set_reserved_memory(char* name, uint64_t addr, uint64_t size);
int dtb_get_reserved_memory(char* name, uint64_t* addr, uint64_t* size);
int dtb_del_reserved_memory(char* name);
int dtb_get_cpu_rel_addr(uint64_t* rel_addr, int cpu);
int dtb_set_blparms(const char* parms, unsigned int size);
int dtb_getprop_reg(const char* node_path_par, const char *node_path,
                    uint64_t* addr, uint64_t* size);
int dtb_set_chosen_initrd(uint64_t initrd_addr, uint64_t initrd_size);
int dtb_get_fdt_size(void);
const void* dtb_get_fdt(void);

int dtb_prepare(enum DTB_PREPARE_OPT prep_opt, const unsigned char* img, int size);
#if defined (_BCM96858_) || defined (_BCM94908_) || defined (_BCM968360_)
int dtb_set_nr_cpus(void);
#endif

#endif
