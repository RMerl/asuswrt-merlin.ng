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

/* ru_custom.c: Customizations for libru in the CPE FW environment. */

#include <access_macros.h>
#include <bdmf_shell.h>
#include "ru.h"
#ifdef USE_BDMF_SHELL
static bdmfmon_handle_t ru_cli_dir;
DEFINE_BDMF_FASTLOCK(ru_fastlock);
bdmf_session_handle ru_session;

#if RU_FIELD_CHECK_ENABLE

/* "check" handler
    BDMFMON_MAKE_ENUM( "enable", "Enable check", bdmfmon_enum_bool_table, 0));
 */
static int _ru_cli_check(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int enable = (int)parm[0].value.number;

    ru_field_bounds_check_enable(enable);

    return BDMF_ERR_OK;
}

#endif

/* "parse" handler
    BDMFMON_MAKE_PARM( "address", "Register address", BDMFMON_PARM_HEX, 0),
    BDMFMON_MAKE_PARM( "value", "Register value", BDMFMON_PARM_HEX, 0) );
 */
static int _ru_cli_parse(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t addr = parm[0].value.number;
    uint32_t value = parm[1].value.number;
    int rc;

    ru_session = session;
    rc = ru_reg_addr_print(addr, value);
    ru_session = NULL;

    return rc ? BDMF_ERR_NOENT : BDMF_ERR_OK;
}

void ru_cli_init(bdmfmon_handle_t driver_dir)
{
    if (ru_cli_dir)
        return; /* Just exit if called not the 1st time */

    ru_cli_dir = bdmfmon_dir_add(driver_dir, "ru", "Register access logging, search, etc.", BDMF_ACCESS_ADMIN, NULL);
    if (!ru_cli_dir)
    {
        bdmf_print("Can't create ru CLI directory\n");
        return;
    }

#if RU_FIELD_CHECK_ENABLE
    BDMFMON_MAKE_CMD(ru_cli_dir, "check", "Enable/disable field boundary check", _ru_cli_check,
        BDMFMON_MAKE_PARM_ENUM( "enable", "Enable check", bdmfmon_enum_bool_table, 0) );
#endif

    BDMFMON_MAKE_CMD(ru_cli_dir, "parse", "Parse register value", _ru_cli_parse,
        BDMFMON_MAKE_PARM( "address", "Register address", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM( "value", "Register value", BDMFMON_PARM_HEX, 0) );
}

void ru_cli_exit(bdmfmon_handle_t driver_dir)
{
    if (ru_cli_dir)
    {
        bdmfmon_token_destroy(ru_cli_dir);
        ru_cli_dir = NULL;
    }
}
#endif /* USE_BDMF_SHELL */