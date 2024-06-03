// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <dt-structs.h>

static int sandbox_spl_probe(struct udevice *dev)
{
	struct dtd_sandbox_spl_test *plat = dev_get_platdata(dev);
	int i;

	printf("of-platdata probe:\n");
	printf("bool %d\n", plat->boolval);

	printf("byte %02x\n", plat->byteval);
	printf("bytearray");
	for (i = 0; i < sizeof(plat->bytearray); i++)
		printf(" %02x", plat->bytearray[i]);
	printf("\n");

	printf("int %d\n", plat->intval);
	printf("intarray");
	for (i = 0; i < ARRAY_SIZE(plat->intarray); i++)
		printf(" %d", plat->intarray[i]);
	printf("\n");

	printf("longbytearray");
	for (i = 0; i < sizeof(plat->longbytearray); i++)
		printf(" %02x", plat->longbytearray[i]);
	printf("\n");

	printf("string %s\n", plat->stringval);
	printf("stringarray");
	for (i = 0; i < ARRAY_SIZE(plat->stringarray); i++)
		printf(" \"%s\"", plat->stringarray[i]);
	printf("\n");

	return 0;
}

U_BOOT_DRIVER(sandbox_spl_test) = {
	.name	= "sandbox_spl_test",
	.id	= UCLASS_MISC,
	.flags	= DM_FLAG_PRE_RELOC,
	.probe	= sandbox_spl_probe,
};
