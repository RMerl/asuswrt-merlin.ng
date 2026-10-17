// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Amarula Solutions
 */

#include <common.h>
#include <serial.h>

#ifndef CONFIG_TPL_BUILD

int spl_start_uboot(void)
{
        /* break into full u-boot on 'c' */
        if (serial_tstc() && serial_getc() == 'c')
                return 1;

        return 0;
}
#endif
