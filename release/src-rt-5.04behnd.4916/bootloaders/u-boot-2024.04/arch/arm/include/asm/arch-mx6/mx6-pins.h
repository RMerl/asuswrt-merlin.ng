/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Boundary Devices Inc.
 */
#ifndef __ASM_ARCH_MX6_PINS_H__
#define __ASM_ARCH_MX6_PINS_H__

#include <asm/mach-imx/iomux-v3.h>

#define MX6_PAD_DECLARE(prefix, name, pco, mc, mm, sio, si, pc) \
	prefix##name = IOMUX_PAD(pco, mc, mm, sio, si, pc)

#ifdef CONFIG_MX6QDL
enum {
#define MX6_PAD_DECL(name, pco, mc, mm, sio, si, pc) \
	MX6_PAD_DECLARE(MX6Q_PAD_,name, pco, mc, mm, sio, si, pc),
#include "mx6q_pins.h"
#undef MX6_PAD_DECL
#define MX6_PAD_DECL(name, pco, mc, mm, sio, si, pc) \
	MX6_PAD_DECLARE(MX6DL_PAD_,name, pco, mc, mm, sio, si, pc),
#include "mx6dl_pins.h"
};
#elif defined(CONFIG_MX6Q)
enum {
#define MX6_PAD_DECL(name, pco, mc, mm, sio, si, pc) \
	MX6_PAD_DECLARE(MX6_PAD_,name, pco, mc, mm, sio, si, pc),
#include "mx6q_pins.h"
};
#elif defined(CONFIG_MX6DL) || defined(CONFIG_MX6S)
enum {
#define MX6_PAD_DECL(name, pco, mc, mm, sio, si, pc) \
	MX6_PAD_DECLARE(MX6_PAD_,name, pco, mc, mm, sio, si, pc),
#include "mx6dl_pins.h"
};
#elif defined(CONFIG_MX6SLL)
#include "mx6sll_pins.h"
#elif defined(CONFIG_MX6SL)
#include "mx6sl_pins.h"
#elif defined(CONFIG_MX6SX)
#include "mx6sx_pins.h"
#elif defined(CONFIG_MX6ULL)
#include "mx6ull_pins.h"
#elif defined(CONFIG_MX6UL)
#include "mx6ul_pins.h"
#else
#error "Please select cpu"
#endif	/* CONFIG_MX6Q */

#endif	/*__ASM_ARCH_MX6_PINS_H__ */
