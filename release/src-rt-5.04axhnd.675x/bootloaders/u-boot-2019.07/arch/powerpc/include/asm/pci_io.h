/* originally from linux source (asm-ppc/io.h).
 * Sanity added by Rob Taylor, Flying Pig Systems, 2000
 */
#ifndef _PCI_IO_H_
#define _PCI_IO_H_

#include "io.h"


#define pci_read_le16(addr, dest) \
    __asm__ __volatile__("lhbrx %0,0,%1" : "=r" (dest) : \
		  "r" (addr), "m" (*addr));

#define pci_write_le16(addr, val) \
    __asm__ __volatile__("sthbrx %1,0,%2" : "=m" (*addr) : \
		  "r" (val), "r" (addr));


#define pci_read_le32(addr, dest) \
    __asm__ __volatile__("lwbrx %0,0,%1" : "=r" (dest) : \
		 "r" (addr), "m" (*addr));

#define pci_write_le32(addr, val) \
__asm__ __volatile__("stwbrx %1,0,%2" : "=m" (*addr) : \
		 "r" (val), "r" (addr));

#define pci_readb(addr,b) ((b) = *(volatile u8 *) (addr))
#define pci_writeb(b,addr) ((*(volatile u8 *) (addr)) = (b))

#if !defined(__BIG_ENDIAN)
#define pci_readw(addr,b) ((b) = *(volatile u16 *) (addr))
#define pci_readl(addr,b) ((b) = *(volatile u32 *) (addr))
#define pci_writew(b,addr) ((*(volatile u16 *) (addr)) = (b))
#define pci_writel(b,addr) ((*(volatile u32 *) (addr)) = (b))
#else
#define pci_readw(addr,b) pci_read_le16((volatile u16 *)(addr),(b))
#define pci_readl(addr,b) pci_read_le32((volatile u32 *)(addr),(b))
#define pci_writew(b,addr) pci_write_le16((volatile u16 *)(addr),(b))
#define pci_writel(b,addr) pci_write_le32((volatile u32 *)(addr),(b))
#endif


#endif /* _PCI_IO_H_ */
