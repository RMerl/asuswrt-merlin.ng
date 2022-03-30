// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2009 Extreme Engineering Solutions, Inc.
 */

#include <common.h>
#include "fsl_8xxx_misc.h"

int checkboard(void)
{
	char name[] = CONFIG_SYS_BOARD_NAME;
	char buf[64];
	char *s;
	int i;

#ifdef CONFIG_SYS_FORM_CUSTOM
	s = "Custom";
#elif CONFIG_SYS_FORM_6U_CPCI
	s = "6U CompactPCI";
#elif CONFIG_SYS_FORM_ATCA_PMC
	s = "ATCA w/PMC";
#elif CONFIG_SYS_FORM_ATCA_AMC
	s = "ATCA w/AMC";
#elif CONFIG_SYS_FORM_VME
	s = "VME";
#elif CONFIG_SYS_FORM_6U_VPX
	s = "6U VPX";
#elif CONFIG_SYS_FORM_PMC
	s = "PMC";
#elif CONFIG_SYS_FORM_PCI
	s = "PCI";
#elif CONFIG_SYS_FORM_3U_CPCI
	s = "3U CompactPCI";
#elif CONFIG_SYS_FORM_AMC
	s = "AdvancedMC";
#elif CONFIG_SYS_FORM_XMC
	s = "XMC";
#elif CONFIG_SYS_FORM_PMC_XMC
	s = "PMC/XMC";
#elif CONFIG_SYS_FORM_PCI_EXPRESS
	s = "PCI Express";
#elif CONFIG_SYS_FORM_3U_VPX
	s = "3U VPX";
#else
#error "Form factor not defined"
#endif

	name[strlen(name) - 1] += get_board_derivative();
	printf("Board: X-ES %s %s SBC\n", name, s);

	/* Display board specific information */
	puts("       ");
	i = env_get_f("board_rev", buf, sizeof(buf));
	if (i > 0)
		printf("Rev %s, ", buf);
	i = env_get_f("serial#", buf, sizeof(buf));
	if (i > 0)
		printf("Serial# %s, ", buf);
	i = env_get_f("board_cfg", buf, sizeof(buf));
	if (i > 0)
		printf("Cfg %s", buf);
	puts("\n");

	return 0;
}
