// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 *
 * Copyright (C) 2011 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */
#ifndef CONFIG_TIMER
#include <common.h>
#include <asm/io.h>
#include <faraday/fttmr010.h>
#error "AE3XX timer only support DM flow"
#endif /* CONFIG_TIMER */
