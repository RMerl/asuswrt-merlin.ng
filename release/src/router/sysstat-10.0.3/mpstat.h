/*
 * mpstat: per-processor statistics
 * (C) 2000-2011 by Sebastien Godard (sysstat <at> orange.fr)
 */

#ifndef _MPSTAT_H
#define _MPSTAT_H

/*
 ***************************************************************************
 * mpstat's specific system files.
 ***************************************************************************
 */

#define SOFTIRQS	"/proc/softirqs"

/*
 ***************************************************************************
 * Activities definitions.
 ***************************************************************************
 */

#define M_D_CPU		0x0001
#define M_D_IRQ_SUM	0x0002
#define M_D_IRQ_CPU	0x0004
#define M_D_SOFTIRQS	0x0008

#define DISPLAY_CPU(m)		(((m) & M_D_CPU) == M_D_CPU)
#define DISPLAY_IRQ_SUM(m)	(((m) & M_D_IRQ_SUM) == M_D_IRQ_SUM)
#define DISPLAY_IRQ_CPU(m)	(((m) & M_D_IRQ_CPU) == M_D_IRQ_CPU)
#define DISPLAY_SOFTIRQS(m)	(((m) & M_D_SOFTIRQS) == M_D_SOFTIRQS)

/*
 ***************************************************************************
 * Keywords and constants.
 ***************************************************************************
 */

/* Indicate that option -P has been used */
#define F_P_OPTION	0x01
/* Indicate that stats should be displayed on for online CPU ("-P ON") */
#define F_P_ON		0x02

#define USE_P_OPTION(m)		(((m) & F_P_OPTION) == F_P_OPTION)
#define DISPLAY_ONLINE_CPU(m)	(((m) & F_P_ON) == F_P_ON)

#define K_SUM	"SUM"
#define K_CPU	"CPU"
#define K_SCPU	"SCPU"
#define K_ON	"ON"

#define NR_IRQCPU_PREALLOC	3

#define MAX_IRQ_LEN		16

/*
 ***************************************************************************
 * Structures used to store statistics.
 ***************************************************************************
 */

/*
 * stats_irqcpu->irq:       IRQ#-A
 * stats_irqcpu->interrupt: number of IRQ#-A for proc 0
 * stats_irqcpu->irq:       IRQ#-B
 * stats_irqcpu->interrupt: number of IRQ#-B for proc 0
 * ...
 * stats_irqcpu->irq:       (undef'd)
 * stats_irqcpu->interrupt: number of IRQ#-A for proc 1
 * stats_irqcpu->irq:       (undef'd)
 * stats_irqcpu->interrupt: number of IRQ#-B for proc 1
 * ...
 */
struct stats_irqcpu {
	unsigned int interrupt        __attribute__ ((aligned (4)));
	char         irq_name[MAX_IRQ_LEN];
};

#define STATS_IRQCPU_SIZE      (sizeof(struct stats_irqcpu))

#endif
