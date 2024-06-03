/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2008
 * Ricado Ribalda-Universidad Autonoma de Madrid-ricardo.ribalda@gmail.com
 * This work has been supported by: QTechnology  http://qtec.com/
 * Based on interrupts.c Wolfgang Denk-DENX Software Engineering-wd@denx.de
 */
#ifndef INTERRUPT_H
#define INTERRUPT_H

void external_interrupt(struct pt_regs *regs);

#endif
