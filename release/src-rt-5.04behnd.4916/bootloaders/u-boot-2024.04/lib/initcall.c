// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 The Chromium OS Authors.
 */

#include <efi.h>
#include <initcall.h>
#include <log.h>
#include <relocate.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static ulong calc_reloc_ofs(void)
{
#ifdef CONFIG_EFI_APP
	return (ulong)image_base;
#endif
	/*
	 * Sandbox is relocated by the OS, so symbols always appear at
	 * the relocated address.
	 */
	if (IS_ENABLED(CONFIG_SANDBOX) || (gd->flags & GD_FLG_RELOC))
		return gd->reloc_off;

	return 0;
}

/**
 * initcall_is_event() - Get the event number for an initcall
 *
 * func: Function pointer to check
 * Return: Event number, if this is an event, else 0
 */
static int initcall_is_event(init_fnc_t func)
{
	ulong val = (ulong)func;

	if ((val & INITCALL_IS_EVENT) == INITCALL_IS_EVENT)
		return val & INITCALL_EVENT_TYPE;

	return 0;
}

/*
 * To enable debugging. add #define DEBUG at the top of the including file.
 *
 * To find a symbol, use grep on u-boot.map
 */
int initcall_run_list(const init_fnc_t init_sequence[])
{
	ulong reloc_ofs = calc_reloc_ofs();
	const init_fnc_t *ptr;
	enum event_t type;
	init_fnc_t func;
	int ret = 0;

	for (ptr = init_sequence; func = *ptr, !ret && func; ptr++) {
		type = initcall_is_event(func);

		if (type) {
			if (!CONFIG_IS_ENABLED(EVENT))
				continue;
			debug("initcall: event %d/%s\n", type,
			      event_type_name(type));
		} else if (reloc_ofs) {
			debug("initcall: %p (relocated to %p)\n",
			      (char *)func - reloc_ofs, (char *)func);
		} else {
			debug("initcall: %p\n", (char *)func - reloc_ofs);
		}

		ret = type ? event_notify_null(type) : func();
	}

	if (ret) {
		if (CONFIG_IS_ENABLED(EVENT)) {
			char buf[60];

			/* don't worry about buf size as we are dying here */
			if (type) {
				sprintf(buf, "event %d/%s", type,
					event_type_name(type));
			} else {
				sprintf(buf, "call %p", func);
			}

			printf("initcall failed at %s (err=%dE)\n", buf, ret);
		} else {
			printf("initcall failed at call %p (err=%d)\n",
			       (char *)func - reloc_ofs, ret);
		}

		return ret;
	}

	return 0;
}
