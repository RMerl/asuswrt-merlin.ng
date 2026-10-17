// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Alexander Dahl <post@lespocky.de>
 */

#include <dm.h>

UCLASS_DRIVER(fpga) = {
	.name	= "fpga",
	.id	= UCLASS_FPGA,
};
