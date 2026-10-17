/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _DT_BINDINGS_INTEL_IRQ_H_
#define _DT_BINDINGS_INTEL_IRQ_H_

/* PCI interrupt pin */
#define INTA			1
#define INTB			2
#define INTC			3
#define INTD			4

/* PIRQs */
#define PIRQA			0
#define PIRQB			1
#define PIRQC			2
#define PIRQD			3
#define PIRQE			4
#define PIRQF			5
#define PIRQG			6
#define PIRQH			7

/* PCI bdf encoding */
#ifndef PCI_BDF
#define PCI_BDF(b, d, f)	((b) << 16 | (d) << 11 | (f) << 8)
#endif

#endif /* _DT_BINDINGS_INTEL_IRQ_H_ */
