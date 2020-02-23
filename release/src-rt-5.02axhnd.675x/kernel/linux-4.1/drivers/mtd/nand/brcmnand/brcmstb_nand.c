#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
/*
<:copyright-BRCM:2016:GPL/GPL:standard

   Copyright (c) 2016 Broadcom 
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

#include <linux/device.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#include "brcmnand.h"

static const struct of_device_id brcmstb_nand_of_match[] = {
	{ .compatible = "brcm,brcmnand" },
	{},
};
MODULE_DEVICE_TABLE(of, brcmstb_nand_of_match);

static int brcmstb_nand_probe(struct platform_device *pdev)
{
	return brcmnand_probe(pdev, NULL);
}

static struct platform_driver brcmstb_nand_driver = {
	.probe			= brcmstb_nand_probe,
	.remove			= brcmnand_remove,
	.driver = {
		.name		= "brcmstb_nand",
		.pm		= &brcmnand_pm_ops,
		.of_match_table = brcmstb_nand_of_match,
	}
};
module_platform_driver(brcmstb_nand_driver);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Brian Norris");
MODULE_DESCRIPTION("NAND driver for Broadcom STB chips");
#endif
