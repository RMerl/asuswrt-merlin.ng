/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2021 Google LLC
 */

#ifndef __SANDBOX_IRQ_H
#define __SANDBOX_IRQ_H

/**
 * struct sandbox_irq_priv - private data for this driver
 *
 * @count: Counts the number calls to the read_and_clear() method
 * @pending: true if an interrupt is pending, else false
 */
struct sandbox_irq_priv {
	int count;
	bool pending;
};

#endif /* __SANDBOX_IRQ_H */
