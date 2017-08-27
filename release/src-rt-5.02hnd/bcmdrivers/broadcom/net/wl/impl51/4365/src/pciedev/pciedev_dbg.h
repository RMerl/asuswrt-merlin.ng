/*
 * Initialization and support routines for self-booting compressed image.
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: rtecdc.c 405571 2013-06-03 20:03:49Z $
 */
#ifndef	_pciiedev_dbg_h_
#define	_pciiedev_dbg_h_

#include <typedefs.h>
#include <osl.h>

#define PCI_ERROR_VAL	0x00000001	/* Error level tracing */
#define PCI_TRACE_VAL	0x00000002	/* Function level tracing */
#define PCI_PRHDRS_VAL	0x00000004	/* print packet headers */
#define PCI_PRPKT_VAL	0x00000008	/* print packet contents */
#define PCI_INFORM_VAL	0x00000010	/* debug level tracing */

/* pci_msg_level is a bitvector with defs in wlioctl.h */
#ifdef	BCMDBG
extern int pci_msg_level;

#ifdef MSGTRACE
#define HBUS_TRACE msgtrace_hbus_trace = TRUE
#else
#define HBUS_TRACE
#endif

#define	PCI_ERROR(args)		do {if (pci_msg_level & PCI_ERROR_VAL) printf args;} while (0)
#define	PCI_TRACE(args)		do {if (pci_msg_level & PCI_TRACE_VAL) \
					{HBUS_TRACE; printf args;}} while (0)
#define PCI_PRHDRS(i, p, f, t, r, l) do {if (pci_msg_level & PCI_PRHDRS_VAL)  \
					{HBUS_TRACE; wlc_dumphdrs(i, p, f, t, r, l);}} while (0)
#define	PCI_PRPKT(m, b, n)	do {if (pci_msg_level & PCI_PRPKT_VAL) \
					{HBUS_TRACE; prhex(m, b, n);}} while (0)
#define	PCI_INFORM(args)		do {if (pci_msg_level & PCI_INFORM_VAL) \
					{HBUS_TRACE; printf args;}} while (0)
#else	/* BCMDBG */

#if defined(EVENT_PCI_ERROR) && defined(EVENT_LOG_COMPILE)
#define PCI_ERROR_TEMP(fmts, ...) EVENT_LOG_COMPACT(EVENT_LOG_TAG_PCI_ERROR, fmts , ## __VA_ARGS__)
#define PCI_ERROR(fmts) PCI_ERROR_TEMP fmts
#else
#ifdef BCMDBG_ERR
#define	PCI_ERROR(args)		printf args
#else
#define	PCI_ERROR(args)
#endif /* BCMDBG_ERR */
#endif /* defined(EVENT_PCI_ERROR) && defined(EVENT_LOG_COMPILE) */

#if defined(EVENT_PCI_TRACE) && defined(EVENT_LOG_COMPILE)
#define	PCI_TRACE_TEMP(fmts, ...) EVENT_LOG_COMPACT(EVENT_LOG_TAG_PCI_TRACE, fmts , ## __VA_ARGS__)
#define	PCI_TRACE(fmts)	PCI_TRACE_TEMP fmts
#else
#define	PCI_TRACE(fmts)
#endif /* defined (EVENT_PCI_TRACE) && defined(EVENT_LOG_COMPILE) */
#define	PCI_PRHDRS(i, s, h, p, n, l)
#define	PCI_PRPKT(m, b, n)
#define	PCI_INFORM(args)
#endif /* !BCMDBG */
#endif /* _pciiedev_dbg_h_ */
