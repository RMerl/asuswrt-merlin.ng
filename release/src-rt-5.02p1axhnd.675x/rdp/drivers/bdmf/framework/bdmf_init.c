/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */


/*******************************************************************
 * bdmf_init.c
 *
 * Broadlight Device Management Framework - init code
 *
 * This file must be given to the linker first!
 *
 * This file is Copyright (c) 2011, Broadlight Communications.
 * This file is licensed under GNU Public License, except that if
 * you have entered in to a signed, written license agreement with
 * Broadlight covering this file, that agreement applies to this
 * file instead of the GNU Public License.
 *
 * This file is free software: you can redistribute and/or modify it
 * under the terms of the GNU Public License, Version 2, as published
 * by the Free Software Foundation, unless a different license
 * applies as provided above.
 *
 * This program is distributed in the hope that it will be useful,
 * but AS-IS and WITHOUT ANY WARRANTY; without even the implied
 * warranties of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * TITLE or NONINFRINGEMENT. Redistribution, except as permitted by
 * the GNU Public License or another license agreement between you
 * and Broadlight, is prohibited.
 *
 * You should have received a copy of the GNU Public License,
 * Version 2 along with this file; if not, see
 * <http://www.gnu.org/licenses>.
 *
 * Author: Igor Ternovsky
 *******************************************************************/

#define DEBUG

#include <bdmf_dev.h>
#include <bdmf_shell.h>
#include <bdmf_shell_server.h>

/*
 * Start type and aggregate type sections.
 */
unsigned long bdmf_type_section_start __attribute__((section("BDMF_init"))) = 0;
unsigned long bdmf_aggr_section_start __attribute__((section("BDMF_aggr_init"))) = 0;
extern unsigned long bdmf_type_section_end;
extern unsigned long bdmf_aggr_section_end;

/* Root object */
struct bdmf_object *bdmf_root_object;

extern bdmfmon_handle_t bdmf_flow_mon_init(bdmfmon_handle_t root_dir);
extern void bdmf_flow_mon_exit(void);
extern bdmfmon_handle_t bdmf_codegen_mon_init(void);
extern bdmfmon_handle_t bdmf_hist_mon_init(void);

/* Register all aggregate types declared types using DECLARE_BDMF_AGGREGATE_TYPE */
int bdmf_register_aggregate_types(const unsigned long *section_start, const unsigned long *section_end)
{
    const unsigned long *bdmf_type_addr;
    int iter, done_in_iter;
    int rc = 0;
    int real_trace_level = bdmf_global_trace_level;
    int repeat_last_iteration = 0;

    iter = 0;
    while (1)
    {
        BDMF_TRACE_INFO("Registering aggregate types: iteration %d\n", iter++);
        done_in_iter = 0;
        for (bdmf_type_addr = section_start; bdmf_type_addr <= section_end; bdmf_type_addr++)
        {
            struct bdmf_aggr_type *at = (struct bdmf_aggr_type *)*bdmf_type_addr;
            BUG_ON(!at);
            /* Skip if already registered */
            if (at->use_count)
                continue;
            /* Suppress error messages if it is not a repeat of last failed iteration */
            if (!repeat_last_iteration)
            	bdmf_global_trace_level = bdmf_trace_level_none;
            rc = bdmf_attr_aggregate_type_register(at);
            bdmf_global_trace_level = real_trace_level;
            BDMF_TRACE_INFO("Registering aggregate type (%d): %-16s : %s\n",
                rc, at->name, at->help?at->help:"");
            if (!rc)
                ++done_in_iter;
        }
        /* Are we done yet ? */
        if (!done_in_iter)
        {
        	/* All good ? - stop */
        	if (!rc)
        		break;
        	/* Was the last iteration has already been repeated with error print ON? */
        	if (repeat_last_iteration)
        		break;
        	/* Repeat the last iteration with error message print ON */
        	repeat_last_iteration = 1;
        }
    }

    return rc;
}

/* Unregister all aggregate types declared types using DECLARE_BDMF_AGGREGATE_TYPE */
int bdmf_unregister_aggregate_types(const unsigned long *section_start, const unsigned long *section_end)
{
    const unsigned long *bdmf_type_addr;
    int rc;

    for (bdmf_type_addr = section_start; bdmf_type_addr <= section_end; bdmf_type_addr++)
    {
        struct bdmf_aggr_type *at = (struct bdmf_aggr_type *)*bdmf_type_addr;
        BUG_ON(!at);
        rc = bdmf_attr_aggregate_type_unregister(at);
        BDMF_TRACE_INFO("Unregistering aggregate type (%d): %-16s : %s\n",
                     rc, at->name, at->help?at->help:"");
    }
    return 0;
}

/* Register all plugins that declared types using DECLARE_BDMF_TYPE */
int bdmf_register_plugins(const unsigned long *section_start, const unsigned long *section_end)
{
    const unsigned long *bdmf_type_addr;
    int iter, done_in_iter;
    int real_trace_level = bdmf_global_trace_level;
    int repeat_last_iteration = 0;
    int rc = 0;

    iter = 0;
    while (1)
    {
        BDMF_TRACE_INFO("Registering plugins: iteration %d\n", iter++);
        done_in_iter = 0;
        for (bdmf_type_addr = section_start; bdmf_type_addr <= section_end; bdmf_type_addr++)
        {
            struct bdmf_type *drv = (struct bdmf_type *)*bdmf_type_addr;
            BUG_ON(!drv);

            /* Skip types that are already registered */
            if (drv->usecount)
                continue;

            /* Suppress error messages if it is not a repeat of last failed iteration */
            if (!repeat_last_iteration)
            	bdmf_global_trace_level = bdmf_trace_level_none;
            rc = bdmf_type_register(drv);
            bdmf_global_trace_level = real_trace_level;
            BDMF_TRACE_INFO("Registering plugin (%d): %-16s : %s\n",
                rc, drv->name, drv->description?drv->description:"");
            if (!rc)
            {
                ++done_in_iter;
                bdmf_trace_level_set(drv, real_trace_level);
            }
        }
        /* Are we done yet ? */
        if (!done_in_iter)
        {
        	/* All good ? - stop */
        	if (!rc)
        		break;
        	/* Was the last iteration has already been repeated with error print ON? */
        	if (repeat_last_iteration)
        		break;
        	/* Repeat the last iteration with error message print ON */
        	repeat_last_iteration = 1;
        }
    }

    return rc;
}

/* Unregister all plugins that declared types using DECLARE_BDMF_TYPE */
int bdmf_unregister_plugins(const unsigned long *section_start, const unsigned long *section_end)
{
    const unsigned long *bdmf_type_addr;

    for (bdmf_type_addr = section_start; bdmf_type_addr <= section_end; bdmf_type_addr++)
    {
        struct bdmf_type *drv = (struct bdmf_type *)*bdmf_type_addr;
        BUG_ON(!drv);
        bdmf_type_unregister(drv);
        BDMF_TRACE_INFO("Unregistering plugin: %-16s : %s\n",
                         drv->name, drv->description?drv->description:"");
    }
    return 0;
}

/* Create root object instance */
static int bdmf_create_root_object(void)
{
    int rc;

    rc = bdmf_new_and_configure(bdmf_root_type, NULL, NULL, &bdmf_root_object);
    if (!rc)
        strcpy(bdmf_root_object->name, "root");
    return rc;
}

/** Initialize Broadlight Device Management Framefork
 *
 *  This function should be called once at init time.
 * \parm[in]    init_config     Initial configuration
 * \return
 *     0    - OK\n
 *    <0    - error
 */
int bdmf_init(const struct bdmf_init_config *init_config)
{
    int rc=0;

#ifdef BDMF_TRANSPORT
    /* Initialize transport library */
    rc = rc ? rc : bdmfts_module_init();
#endif

    rc = rc ? rc : bdmf_area_module_init();
    rc = rc ? rc : bdmf_type_module_init();
#ifdef BDMF_SHELL
    bdmf_flow_mon_init(NULL);
#ifdef BDMF_SHELL_SERVER
    bdmfmons_server_mon_init(NULL);
#endif
#endif
    rc = rc ? rc : bdmf_trace_init();
    if (init_config)
        bdmf_trace_level_set(NULL, init_config->trace_level);
    rc = rc ? rc : bdmf_register_aggregate_types(&bdmf_aggr_section_start + 1,
        &bdmf_aggr_section_end - 1);
    rc = rc ? rc : bdmf_register_plugins(&bdmf_type_section_start + 1,
        &bdmf_type_section_end - 1);
    rc = rc ? rc : bdmf_create_root_object();
#ifdef BDMF_HISTORY
    rc = rc ? rc : bdmf_history_module_init();
#endif
#ifdef BDMF_CODEGEN
    bdmf_codegen_mon_init();
#endif
#if defined(BDMF_HISTORY) && defined(BDMF_SHELL)
    bdmf_hist_mon_init();
#endif
    return rc;
}

void bdmf_exit(void)
{
#ifdef BDMF_SHELL
    bdmf_flow_mon_exit();
#endif
#ifdef BDMF_HISTORY
    bdmf_history_module_exit();
#endif
    bdmf_type_module_exit();
    bdmf_area_module_exit();
}

#ifdef __KERNEL__

module_param(bdmf_global_trace_level, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(bdmf_global_trace_level, "Global trace level: 0=none,1=error,2=info,3=debug");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Igor Ternovsky. (C) Broadcom");

extern int bdmf_chrdev_init(void);
extern void bdmf_chrdev_exit(void);

static int bdmf_module_init(void)
{
    int rc;
    struct bdmf_init_config init_cfg = {
        .trace_level = bdmf_global_trace_level
    };
    rc = bdmf_init(&init_cfg);
    if (rc)
        return -ENOMEM;
    rc = bdmf_chrdev_init();
    if (rc)
        bdmf_exit();
    return rc;
}

static void bdmf_module_exit(void)
{
    bdmf_chrdev_exit();
    bdmf_exit();
}
module_init(bdmf_module_init);
module_exit(bdmf_module_exit);

/*
 * Exports
 */
EXPORT_SYMBOL(bdmf_register_aggregate_types);
EXPORT_SYMBOL(bdmf_register_plugins);
EXPORT_SYMBOL(bdmf_unregister_aggregate_types);
EXPORT_SYMBOL(bdmf_unregister_plugins);

#endif /* #ifdef __KERNEL__ */
