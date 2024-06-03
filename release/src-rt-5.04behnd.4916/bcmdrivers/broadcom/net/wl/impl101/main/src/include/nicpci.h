/*
 * BCM43XX PCI/E core sw API definitions.
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: nicpci.h 821234 2023-02-06 14:16:52Z $
 */

#ifndef	_NICPCI_H
#define	_NICPCI_H

#if (defined(BCMBUSTYPE) && (BCMBUSTYPE == SI_BUS))
#define pcicore_find_pci_capability(a, b, c, d) (0)
#define pcie_readreg(a, b, c, d)	(0)
#define pcie_writereg(a, b, c, d, e)	(0)

#define pcie_clkreq(a, b, c)		(0)
#define pcie_lcreg(a, b, c)		(0)
#define pcie_set_error_injection(a, b)	do { } while (0)
#define pcie_ltrenable(a, b, c)		(0)
#define pcie_obffenable(a, b, c)	(0)
#define pcie_ltr_reg(a, b, c, d)	(0)

#define pcicore_init(a, b, c)		(0x0dadbeef)
#define pcicore_deinit(a)		do { } while (0)
#define pcicore_attach(a, b, c)		do { } while (0)
#define pcicore_up(a, b)		do { } while (0)
#define pcicore_down(a, b)		do { } while (0)

#define pcicore_pcieserdesreg(a, b, c, d, e) (0)
#define pcicore_pciereg(a, b, c, d, e)	(0)

#if defined(WLTEST) || defined(BCMDBG)
#define pcicore_dump_pcieregs(a, b)	(0)
#define pcicore_dump_pcieinfo(a, b)	(0)
#endif

#ifdef BCMDBG
#define pcie_lcreg(a, b, c)		(0)
#define pcicore_dump(a, b)		do { } while (0)
#endif

#define pcie_set_request_size(pch, size) do { } while (0)
#define pcie_get_request_size(pch)	(0)
#define pcie_set_maxpayload_size(pch, size) do { } while (0)
#define pcie_get_maxpayload_size(pch)	(0)
#define pcie_get_ssid(a)		(0)
#define pcie_get_bar0(a)		(0)
#define pcie_get_link_speed(a)		(0)
#else
struct sbpcieregs;

extern uint8 pcicore_find_pci_capability(osl_t *osh, uint8 req_cap_id,
                                         uchar *buf, uint32 *buflen);
extern uint pcie_readreg(si_t *sih, volatile struct sbpcieregs *pcieregs,
                         uint addrtype, uint offset);
extern uint pcie_writereg(si_t *sih, volatile struct sbpcieregs *pcieregs,
                          uint addrtype, uint offset, uint val);

extern uint8 pcie_clkreq(void *pch, uint32 mask, uint32 val);
extern uint32 pcie_lcreg(void *pch, uint32 mask, uint32 val);
extern void pcie_set_error_injection(void *pch, uint32 mode);
extern uint8 pcie_ltrenable(void *pch, uint32 mask, uint32 val);
extern uint8 pcie_obffenable(void *pch, uint32 mask, uint32 val);
extern uint32 pcie_ltr_reg(void *pch, uint32 reg, uint32 mask, uint32 val);

extern void *pcicore_init(si_t *sih, osl_t *osh, volatile void *regs);
extern void pcicore_deinit(void *pch);
extern void pcicore_attach(void *pch, char *pvars, int state);
extern void pcicore_up(void *pch, int state);
extern void pcicore_down(void *pch, int state);

extern uint32 pcicore_pcieserdesreg(void *pch, uint32 mdioslave, uint32 offset,
                                    uint32 mask, uint32 val);

extern uint32 pcicore_pciereg(void *pch, uint32 offset, uint32 mask, uint32 val, uint type);

#if defined(WLTEST) || defined(BCMDBG)
extern int pcicore_dump_pcieinfo(void *pch, struct bcmstrbuf *b);
extern int pcicore_dump_pcieregs(void *pch, struct bcmstrbuf *b);
#endif

#ifdef BCMDBG
extern void pcicore_dump(void *pch, struct bcmstrbuf *b);
#endif

extern void pcie_set_request_size(void *pch, uint16 size);
extern uint16 pcie_get_request_size(void *pch);
extern void pcie_set_maxpayload_size(void *pch, uint16 size);
extern uint16 pcie_get_maxpayload_size(void *pch);
extern uint16 pcie_get_ssid(void *pch);
extern uint32 pcie_get_bar0(void *pch);
extern uint32 pcie_get_link_speed(void* pch);
#endif

#endif	/* _NICPCI_H */
