/*
 * Boot a Marvell SoC, with Xmodem over UART0.
 *  supports Kirkwood, Dove, Avanta, Armada 370, Armada XP, Armada 375,
 *           Armada 38x and Armada 39x.
 *
 * (c) 2012 Daniel Stodden <daniel.stodden@gmail.com>
 * (c) 2021 Pali Rohár <pali@kernel.org>
 * (c) 2021 Marek Behún <kabel@kernel.org>
 *
 * References:
 * - "88F6180, 88F6190, 88F6192, and 88F6281: Integrated Controller: Functional
 *   Specifications" December 2, 2008. Chapter 24.2 "BootROM Firmware".
 *   https://web.archive.org/web/20130730091033/https://www.marvell.com/embedded-processors/kirkwood/assets/FS_88F6180_9x_6281_OpenSource.pdf
 * - "88AP510: High-Performance SoC with Integrated CPU, 2D/3D Graphics
 *   Processor, and High-Definition Video Decoder: Functional Specifications"
 *   August 3, 2011. Chapter 5 "BootROM Firmware"
 *   https://web.archive.org/web/20120130172443/https://www.marvell.com/application-processors/armada-500/assets/Armada-510-Functional-Spec.pdf
 * - "88F6665, 88F6660, 88F6658, 88F6655, 88F6655F, 88F6650, 88F6650F, 88F6610,
 *   and 88F6610F Avanta LP Family Integrated Single/Dual CPU Ecosystem for
 *   Gateway (GW), Home Gateway Unit (HGU), and Single Family Unit (SFU)
 *   Functional Specifications" Doc. No. MV-S108952-00, Rev. A. November 7, 2013.
 *   Chapter 7 "Boot Flow"
 *   CONFIDENTIAL, no public documentation available
 * - "88F6710, 88F6707, and 88F6W11: ARMADA(R) 370 SoC: Functional Specifications"
 *   May 26, 2014. Chapter 6 "BootROM Firmware".
 *   https://web.archive.org/web/20140617183701/https://www.marvell.com/embedded-processors/armada-300/assets/ARMADA370-FunctionalSpec-datasheet.pdf
 * - "MV78230, MV78260, and MV78460: ARMADA(R) XP Family of Highly Integrated
 *   Multi-Core ARMv7 Based SoC Processors: Functional Specifications"
 *   May 29, 2014. Chapter 6 "BootROM Firmware".
 *   https://web.archive.org/web/20180829171131/https://www.marvell.com/embedded-processors/armada-xp/assets/ARMADA-XP-Functional-SpecDatasheet.pdf
 * - "BobCat2 Control and Management Subsystem Functional Specifications"
 *   Doc. No. MV-S109400-00, Rev. A. December 4, 2014.
 *   Chapter 1.6 BootROM Firmware
 *   CONFIDENTIAL, no public documentation available
 * - "AlleyCat3 and PONCat3 Highly Integrated 1/10 Gigabit Ethernet Switch
 *   Control and Management Subsystem: Functional Specifications"
 *   Doc. No. MV-S109693-00, Rev. A. May 20, 2014.
 *   Chapter 1.6 BootROM Firmware
 *   CONFIDENTIAL, no public documentation available
 * - "ARMADA(R) 375 Value-Performance Dual Core CPU System on Chip: Functional
 *   Specifications" Doc. No. MV-S109377-00, Rev. A. September 18, 2013.
 *   Chapter 7 "Boot Sequence"
 *   CONFIDENTIAL, no public documentation available
 * - "88F6810, 88F6811, 88F6821, 88F6W21, 88F6820, and 88F6828: ARMADA(R) 38x
 *   Family High-Performance Single/Dual CPU System on Chip: Functional
 *   Specifications" Doc. No. MV-S109094-00, Rev. C. August 2, 2015.
 *   Chapter 7 "Boot Flow"
 *   CONFIDENTIAL, no public documentation available
 * - "88F6920, 88F6925 and 88F6928: ARMADA(R) 39x High-Performance Dual Core CPU
 *   System on Chip Functional Specifications" Doc. No. MV-S109896-00, Rev. B.
 *   December 22, 2015. Chapter 7 "Boot Flow"
 *   CONFIDENTIAL, no public documentation available
 * - "Marvell boot image parser", Marvell U-Boot 2013.01, version 18.06. September 17, 2015.
 *   https://github.com/MarvellEmbeddedProcessors/u-boot-marvell/blob/u-boot-2013.01-armada-18.06/tools/marvell/doimage_mv/hdrparser.c
 * - "Marvell doimage Tool", Marvell U-Boot 2013.01, version 18.06. August 30, 2015.
 *   https://github.com/MarvellEmbeddedProcessors/u-boot-marvell/blob/u-boot-2013.01-armada-18.06/tools/marvell/doimage_mv/doimage.c
 *
 * Storage location / offset of different image types:
 * - IBR_HDR_SPI_ID (0x5A):
 *   SPI image can be stored at any 2 MB aligned offset in the first 16 MB of
 *   SPI-NOR or parallel-NOR. Despite the type name it really can be stored on
 *   parallel-NOR and cannot be stored on other SPI devices, like SPI-NAND.
 *   So it should have been named NOR image, not SPI image. This image type
 *   supports XIP - Execute In Place directly from NOR memory. Destination
 *   address of the XIP image is set to 0xFFFFFFFF and execute address to the
 *   absolute offset in bytes from the beginning of NOR memory.
 *
 * - IBR_HDR_NAND_ID (0x8B):
 *   NAND image can be stored either at any 2 MB aligned offset in the first
 *   16 MB of SPI-NAND or at any blocksize aligned offset in the first 64 MB
 *   of parallel-NAND.
 *
 * - IBR_HDR_PEX_ID (0x9C):
 *   PEX image is used for booting from PCI Express device. Source address
 *   stored in image is ignored by BootROM. It is not the BootROM who parses
 *   or loads data part of the PEX image. BootROM just configures SoC to the
 *   PCIe endpoint mode and let the PCIe device on the other end of the PCIe
 *   link (which must be in Root Complex mode) to load kwbimage into SoC's
 *   memory and tell BootROM physical address.
 *
 * - IBR_HDR_UART_ID (0x69):
 *   UART image can be transfered via xmodem protocol over first UART.
 *   Unlike all other image types, header size stored in the image must be
 *   multiply of the 128 bytes (for all other image types it can be any size)
 *   and data part of the image does not have to contain 32-bit checksum
 *   (all other image types must have valid 32-bit checksum in its data part).
 *   And data size stored in the image is ignored. A38x BootROM determinates
 *   size of the data part implicitly by the end of the xmodem transfer.
 *   A38x BootROM has a bug which cause that BootROM loads data part of UART
 *   image into RAM target address increased by one byte when source address
 *   and header size stored in the image header are not same. So UART image
 *   should be constructed in a way that there is no gap between header and
 *   data part.
 *
 * - IBR_HDR_I2C_ID (0x4D):
 *   It is unknown for what kind of storage is used this image. It is not
 *   specified in any document from References section.
 *
 * - IBR_HDR_SATA_ID (0x78):
 *   SATA image can be stored at sector 1 (after the MBR table), sector 34
 *   (after the GPT table) or at any next sector which is aligned to 2 MB and
 *   is in the first 16 MB of SATA disk. Note that source address in SATA image
 *   is stored in sector unit and not in bytes like for any other images.
 *   Unfortunately sector size is disk specific, in most cases it is 512 bytes
 *   but there are also Native 4K SATA disks which have 4096 bytes long sectors.
 *
 * - IBR_HDR_SDIO_ID (0xAE):
 *   SDIO image can be stored on different medias:
 *   - SD(SC) card
 *   - SDHC/SDXC card
 *   - eMMC HW boot partition
 *   - eMMC user data partition / MMC card
 *   It cannot be stored on SDIO card despite the image name.
 *
 *   For SD(SC)/SDHC/SDXC cards, image can be stored at the same locations as
 *   the SATA image (sector 1, sector 34 or any 2 MB aligned sector) but within
 *   the first 64 MB. SDHC and SDXC cards have fixed 512 bytes long sector size.
 *   Old SD(SC) cards unfortunately can have also different sector sizes, mostly
 *   1024 bytes long sector sizes and also can be changed at runtime.
 *
 *   For MMC-compatible devices, image can be stored at offset 0 or at offset
 *   2 MB. If MMC device supports HW boot partitions then image must be stored
 *   on the HW partition as is configured in the EXT_CSC register (it can be
 *   either boot or user data).
 *
 *   Note that source address for SDIO image is stored in byte unit, like for
 *   any other images (except SATA). Marvell Functional Specifications for
 *   A38x and A39x SoCs say that source address is in sector units, but this
 *   is purely incorrect information. A385 BootROM really expects source address
 *   for SDIO images in bytes and also Marvell tools generate SDIO image with
 *   source address in byte units.
 */

#include "kwbimage.h"
#include "mkimage.h"
#include "version.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <image.h>
#include <libgen.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <sys/stat.h>
#include <pthread.h>

#ifdef __linux__
#include "termios_linux.h"
#else
#include <termios.h>
#endif

/*
 * These functions are in <term.h> header file, but this header file conflicts
 * with "termios_linux.h" header file. So declare these functions manually.
 */
extern int setupterm(const char *, int, int *);
extern char *tigetstr(const char *);

/*
 * Marvell BootROM UART Sensing
 */

static unsigned char kwboot_msg_boot[] = {
	0xBB, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77
};

static unsigned char kwboot_msg_debug[] = {
	0xDD, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77
};

/* Defines known to work on Kirkwood */
#define KWBOOT_MSG_RSP_TIMEO	50 /* ms */

/* Defines known to work on Armada XP */
#define KWBOOT_MSG_RSP_TIMEO_AXP	10 /* ms */

/*
 * Xmodem Transfers
 */

#define SOH	1	/* sender start of block header */
#define EOT	4	/* sender end of block transfer */
#define ACK	6	/* target block ack */
#define NAK	21	/* target block negative ack */

#define KWBOOT_XM_BLKSZ	128 /* xmodem block size */

struct kwboot_block {
	uint8_t soh;
	uint8_t pnum;
	uint8_t _pnum;
	uint8_t data[KWBOOT_XM_BLKSZ];
	uint8_t csum;
} __packed;

#define KWBOOT_BLK_RSP_TIMEO 2000 /* ms */
#define KWBOOT_HDR_RSP_TIMEO 10000 /* ms */

/* ARM code to change baudrate */
static unsigned char kwboot_baud_code[] = {
				/* ; #define UART_BASE 0xd0012000             */
				/* ; #define DLL       0x00                   */
				/* ; #define DLH       0x04                   */
				/* ; #define LCR       0x0c                   */
				/* ; #define   DLAB    0x80                   */
				/* ; #define LSR       0x14                   */
				/* ; #define   TEMT    0x40                   */
				/* ; #define DIV_ROUND(a, b) ((a + b/2) / b)  */
				/* ;                                          */
				/* ; u32 set_baudrate(u32 old_b, u32 new_b) { */
				/* ;   while                                  */
				/* ;      (!(readl(UART_BASE + LSR) & TEMT)); */
				/* ;   u32 lcr = readl(UART_BASE + LCR);      */
				/* ;   writel(UART_BASE + LCR, lcr | DLAB);   */
				/* ;   u8 old_dll = readl(UART_BASE + DLL);   */
				/* ;   u8 old_dlh = readl(UART_BASE + DLH);   */
				/* ;   u16 old_dl = old_dll | (old_dlh << 8); */
				/* ;   u32 clk = old_b * old_dl;              */
				/* ;   u16 new_dl = DIV_ROUND(clk, new_b);    */
				/* ;   u8 new_dll = new_dl & 0xff;            */
				/* ;   u8 new_dlh = (new_dl >> 8) & 0xff;     */
				/* ;   writel(UART_BASE + DLL, new_dll);      */
				/* ;   writel(UART_BASE + DLH, new_dlh);      */
				/* ;   writel(UART_BASE + LCR, lcr & ~DLAB);  */
				/* ;   msleep(5);                             */
				/* ;   return 0;                              */
				/* ; }                                        */

				/*  ; r0 = UART_BASE                          */
	0x0d, 0x02, 0xa0, 0xe3, /* mov   r0, #0xd0000000                      */
	0x12, 0x0a, 0x80, 0xe3, /* orr   r0, r0, #0x12000                     */

				/*  ; Wait until Transmitter FIFO is Empty    */
				/* .Lloop_txempty:                            */
				/*  ; r1 = UART_BASE[LSR] & TEMT              */
	0x14, 0x10, 0x90, 0xe5, /* ldr   r1, [r0, #0x14]                      */
	0x40, 0x00, 0x11, 0xe3, /* tst   r1, #0x40                            */
	0xfc, 0xff, 0xff, 0x0a, /* beq   .Lloop_txempty                       */

				/*  ; Set Divisor Latch Access Bit            */
				/*  ; UART_BASE[LCR] |= DLAB                  */
	0x0c, 0x10, 0x90, 0xe5, /* ldr   r1, [r0, #0x0c]                      */
	0x80, 0x10, 0x81, 0xe3, /* orr   r1, r1, #0x80                        */
	0x0c, 0x10, 0x80, 0xe5, /* str   r1, [r0, #0x0c]                      */

				/*  ; Read current Divisor Latch              */
				/*  ; r1 = UART_BASE[DLH]<<8 | UART_BASE[DLL] */
	0x00, 0x10, 0x90, 0xe5, /* ldr   r1, [r0, #0x00]                      */
	0xff, 0x10, 0x01, 0xe2, /* and   r1, r1, #0xff                        */
	0x01, 0x20, 0xa0, 0xe1, /* mov   r2, r1                               */
	0x04, 0x10, 0x90, 0xe5, /* ldr   r1, [r0, #0x04]                      */
	0xff, 0x10, 0x01, 0xe2, /* and   r1, r1, #0xff                        */
	0x41, 0x14, 0xa0, 0xe1, /* asr   r1, r1, #8                           */
	0x02, 0x10, 0x81, 0xe1, /* orr   r1, r1, r2                           */

				/*  ; Read old baudrate value                 */
				/*  ; r2 = old_baudrate                       */
	0x74, 0x20, 0x9f, 0xe5, /* ldr   r2, old_baudrate                     */

				/*  ; Calculate base clock                    */
				/*  ; r1 = r2 * r1                            */
	0x92, 0x01, 0x01, 0xe0, /* mul   r1, r2, r1                           */

				/*  ; Read new baudrate value                 */
				/*  ; r2 = new_baudrate                       */
	0x70, 0x20, 0x9f, 0xe5, /* ldr   r2, new_baudrate                     */

				/*  ; Calculate new Divisor Latch             */
				/*  ; r1 = DIV_ROUND(r1, r2) =                */
				/*  ;    = (r1 + r2/2) / r2                   */
	0xa2, 0x10, 0x81, 0xe0, /* add   r1, r1, r2, lsr #1                   */
	0x02, 0x40, 0xa0, 0xe1, /* mov   r4, r2                               */
	0xa1, 0x00, 0x54, 0xe1, /* cmp   r4, r1, lsr #1                       */
				/* .Lloop_div1:                               */
	0x84, 0x40, 0xa0, 0x91, /* movls r4, r4, lsl #1                       */
	0xa1, 0x00, 0x54, 0xe1, /* cmp   r4, r1, lsr #1                       */
	0xfc, 0xff, 0xff, 0x9a, /* bls   .Lloop_div1                          */
	0x00, 0x30, 0xa0, 0xe3, /* mov   r3, #0                               */
				/* .Lloop_div2:                               */
	0x04, 0x00, 0x51, 0xe1, /* cmp   r1, r4                               */
	0x04, 0x10, 0x41, 0x20, /* subhs r1, r1, r4                           */
	0x03, 0x30, 0xa3, 0xe0, /* adc   r3, r3, r3                           */
	0xa4, 0x40, 0xa0, 0xe1, /* mov   r4, r4, lsr #1                       */
	0x02, 0x00, 0x54, 0xe1, /* cmp   r4, r2                               */
	0xf9, 0xff, 0xff, 0x2a, /* bhs   .Lloop_div2                          */
	0x03, 0x10, 0xa0, 0xe1, /* mov   r1, r3                               */

				/*  ; Set new Divisor Latch Low               */
				/*  ; UART_BASE[DLL] = r1 & 0xff              */
	0x01, 0x20, 0xa0, 0xe1, /* mov   r2, r1                               */
	0xff, 0x20, 0x02, 0xe2, /* and   r2, r2, #0xff                        */
	0x00, 0x20, 0x80, 0xe5, /* str   r2, [r0, #0x00]                      */

				/*  ; Set new Divisor Latch High              */
				/*  ; UART_BASE[DLH] = r1>>8 & 0xff           */
	0x41, 0x24, 0xa0, 0xe1, /* asr   r2, r1, #8                           */
	0xff, 0x20, 0x02, 0xe2, /* and   r2, r2, #0xff                        */
	0x04, 0x20, 0x80, 0xe5, /* str   r2, [r0, #0x04]                      */

				/*  ; Clear Divisor Latch Access Bit          */
				/*  ; UART_BASE[LCR] &= ~DLAB                 */
	0x0c, 0x10, 0x90, 0xe5, /* ldr   r1, [r0, #0x0c]                      */
	0x80, 0x10, 0xc1, 0xe3, /* bic   r1, r1, #0x80                        */
	0x0c, 0x10, 0x80, 0xe5, /* str   r1, [r0, #0x0c]                      */

				/*  ; Loop 0x2dc000 (2998272) cycles          */
				/*  ; which is about 5ms on 1200 MHz CPU      */
				/*  ; r1 = 0x2dc000                           */
	0xb7, 0x19, 0xa0, 0xe3, /* mov   r1, #0x2dc000                        */
				/* .Lloop_sleep:                              */
	0x01, 0x10, 0x41, 0xe2, /* sub   r1, r1, #1                           */
	0x00, 0x00, 0x51, 0xe3, /* cmp   r1, #0                               */
	0xfc, 0xff, 0xff, 0x1a, /* bne   .Lloop_sleep                         */

				/*  ; Jump to the end of execution            */
	0x01, 0x00, 0x00, 0xea, /* b     end                                  */

				/*  ; Placeholder for old baudrate value      */
				/* old_baudrate:                              */
	0x00, 0x00, 0x00, 0x00, /* .word 0                                    */

				/*  ; Placeholder for new baudrate value      */
				/* new_baudrate:                              */
	0x00, 0x00, 0x00, 0x00, /* .word 0                                    */

				/* end:                                       */
};

/* ARM code from binary header executed by BootROM before changing baudrate */
static unsigned char kwboot_baud_code_binhdr_pre[] = {
				/* ; #define UART_BASE 0xd0012000             */
				/* ; #define THR       0x00                   */
				/* ; #define LSR       0x14                   */
				/* ; #define   THRE    0x20                   */
				/* ;                                          */
				/* ; void send_preamble(void) {               */
				/* ;   const u8 *str = "$baudratechange";     */
				/* ;   u8 c;                                  */
				/* ;   do {                                   */
				/* ;       while                              */
				/* ;       ((readl(UART_BASE + LSR) & THRE)); */
				/* ;       c = *str++;                        */
				/* ;       writel(UART_BASE + THR, c);        */
				/* ;   } while (c);                           */
				/* ; }                                        */

				/*  ; Preserve registers for BootROM          */
	0xfe, 0x5f, 0x2d, 0xe9, /* push  { r1 - r12, lr }                     */

				/*  ; r0 = UART_BASE                          */
	0x0d, 0x02, 0xa0, 0xe3, /* mov   r0, #0xd0000000                      */
	0x12, 0x0a, 0x80, 0xe3, /* orr   r0, r0, #0x12000                     */

				/*  ; r2 = address of preamble string         */
	0x00, 0x20, 0x8f, 0xe2, /* adr   r2, .Lstr_preamble                   */

				/*  ; Skip preamble data section              */
	0x03, 0x00, 0x00, 0xea, /* b     .Lloop_preamble                      */

				/*  ; Preamble string                         */
				/* .Lstr_preamble:                            */
	0x24, 0x62, 0x61, 0x75, /* .asciz "$baudratechange"                   */
	0x64, 0x72, 0x61, 0x74,
	0x65, 0x63, 0x68, 0x61,
	0x6e, 0x67, 0x65, 0x00,

				/*  ; Send preamble string over UART          */
				/* .Lloop_preamble:                           */
				/*                                            */
				/*  ; Wait until Transmitter Holding is Empty */
				/* .Lloop_thre:                               */
				/*  ; r1 = UART_BASE[LSR] & THRE              */
	0x14, 0x10, 0x90, 0xe5, /* ldr   r1, [r0, #0x14]                      */
	0x20, 0x00, 0x11, 0xe3, /* tst   r1, #0x20                            */
	0xfc, 0xff, 0xff, 0x0a, /* beq   .Lloop_thre                          */

				/*  ; Put character into Transmitter FIFO     */
				/*  ; r1 = *r2++                              */
	0x01, 0x10, 0xd2, 0xe4, /* ldrb  r1, [r2], #1                         */
				/*  ; UART_BASE[THR] = r1                     */
	0x00, 0x10, 0x80, 0xe5, /* str   r1, [r0, #0x0]                       */

				/*  ; Loop until end of preamble string       */
	0x00, 0x00, 0x51, 0xe3, /* cmp   r1, #0                               */
	0xf8, 0xff, 0xff, 0x1a, /* bne   .Lloop_preamble                      */
};

/* ARM code for returning from binary header back to BootROM */
static unsigned char kwboot_baud_code_binhdr_post[] = {
				/*  ; Return 0 - no error                     */
	0x00, 0x00, 0xa0, 0xe3, /* mov   r0, #0                               */
	0xfe, 0x9f, 0xbd, 0xe8, /* pop   { r1 - r12, pc }                     */
};

/* ARM code for jumping to the original image exec_addr */
static unsigned char kwboot_baud_code_data_jump[] = {
	0x04, 0xf0, 0x1f, 0xe5, /* ldr   pc, exec_addr                        */
				/*  ; Placeholder for exec_addr               */
				/* exec_addr:                                 */
	0x00, 0x00, 0x00, 0x00, /* .word 0                                    */
};

static const char kwb_baud_magic[16] = "$baudratechange";

static int kwboot_verbose;

static int msg_rsp_timeo = KWBOOT_MSG_RSP_TIMEO;
static int blk_rsp_timeo = KWBOOT_BLK_RSP_TIMEO;

static ssize_t
kwboot_write(int fd, const char *buf, size_t len)
{
	ssize_t tot = 0;

	while (tot < len) {
		ssize_t wr = write(fd, buf + tot, len - tot);

		if (wr < 0 && errno == EINTR)
			continue;
		else if (wr < 0)
			return wr;

		tot += wr;
	}

	return tot;
}

static void
kwboot_printv(const char *fmt, ...)
{
	va_list ap;

	if (kwboot_verbose) {
		va_start(ap, fmt);
		vprintf(fmt, ap);
		va_end(ap);
		fflush(stdout);
	}
}

static void
__spinner(void)
{
	const char seq[] = { '-', '\\', '|', '/' };
	const int div = 8;
	static int state, bs;

	if (state % div == 0) {
		fputc(bs, stdout);
		fputc(seq[state / div % sizeof(seq)], stdout);
		fflush(stdout);
	}

	bs = '\b';
	state++;
}

static void
kwboot_spinner(void)
{
	if (kwboot_verbose)
		__spinner();
}

static void
__progress(int pct, char c)
{
	const int width = 70;
	static const char *nl = "";
	static int pos;

	if (pos % width == 0)
		printf("%s%3d %% [", nl, pct);

	fputc(c, stdout);

	nl = "]\n";
	pos = (pos + 1) % width;

	if (pct == 100) {
		while (pos && pos++ < width)
			fputc(' ', stdout);
		fputs(nl, stdout);
		nl = "";
		pos = 0;
	}

	fflush(stdout);

}

static void
kwboot_progress(int _pct, char c)
{
	static int pct;

	if (_pct != -1)
		pct = _pct;

	if (kwboot_verbose)
		__progress(pct, c);

	if (pct == 100)
		pct = 0;
}

static int
kwboot_tty_recv(int fd, void *buf, size_t len, int timeo)
{
	int rc, nfds;
	fd_set rfds;
	struct timeval tv;
	ssize_t n;

	rc = -1;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	tv.tv_sec = 0;
	tv.tv_usec = timeo * 1000;
	if (tv.tv_usec > 1000000) {
		tv.tv_sec += tv.tv_usec / 1000000;
		tv.tv_usec %= 1000000;
	}

	do {
		nfds = select(fd + 1, &rfds, NULL, NULL, &tv);
		if (nfds < 0 && errno == EINTR)
			continue;
		else if (nfds < 0)
			goto out;
		else if (!nfds) {
			errno = ETIMEDOUT;
			goto out;
		}

		n = read(fd, buf, len);
		if (n < 0 && errno == EINTR)
			continue;
		else if (n <= 0)
			goto out;

		buf = (char *)buf + n;
		len -= n;
	} while (len > 0);

	rc = 0;
out:
	return rc;
}

static int
kwboot_tty_send(int fd, const void *buf, size_t len, int nodrain)
{
	if (!buf)
		return 0;

	if (kwboot_write(fd, buf, len) < 0)
		return -1;

	if (nodrain)
		return 0;

	return tcdrain(fd);
}

static int
kwboot_tty_send_char(int fd, unsigned char c)
{
	return kwboot_tty_send(fd, &c, 1, 0);
}

static speed_t
kwboot_tty_baudrate_to_speed(int baudrate)
{
	switch (baudrate) {
#ifdef B4000000
	case 4000000:
		return B4000000;
#endif
#ifdef B3500000
	case 3500000:
		return B3500000;
#endif
#ifdef B3000000
	case 3000000:
		return B3000000;
#endif
#ifdef B2500000
	case 2500000:
		return B2500000;
#endif
#ifdef B2000000
	case 2000000:
		return B2000000;
#endif
#ifdef B1500000
	case 1500000:
		return B1500000;
#endif
#ifdef B1152000
	case 1152000:
		return B1152000;
#endif
#ifdef B1000000
	case 1000000:
		return B1000000;
#endif
#ifdef B921600
	case 921600:
		return B921600;
#endif
#ifdef B614400
	case 614400:
		return B614400;
#endif
#ifdef B576000
	case 576000:
		return B576000;
#endif
#ifdef B500000
	case 500000:
		return B500000;
#endif
#ifdef B460800
	case 460800:
		return B460800;
#endif
#ifdef B307200
	case 307200:
		return B307200;
#endif
#ifdef B230400
	case 230400:
		return B230400;
#endif
#ifdef B153600
	case 153600:
		return B153600;
#endif
#ifdef B115200
	case 115200:
		return B115200;
#endif
#ifdef B76800
	case 76800:
		return B76800;
#endif
#ifdef B57600
	case 57600:
		return B57600;
#endif
#ifdef B38400
	case 38400:
		return B38400;
#endif
#ifdef B19200
	case 19200:
		return B19200;
#endif
#ifdef B9600
	case 9600:
		return B9600;
#endif
#ifdef B4800
	case 4800:
		return B4800;
#endif
#ifdef B2400
	case 2400:
		return B2400;
#endif
#ifdef B1800
	case 1800:
		return B1800;
#endif
#ifdef B1200
	case 1200:
		return B1200;
#endif
#ifdef B600
	case 600:
		return B600;
#endif
#ifdef B300
	case 300:
		return B300;
#endif
#ifdef B200
	case 200:
		return B200;
#endif
#ifdef B150
	case 150:
		return B150;
#endif
#ifdef B134
	case 134:
		return B134;
#endif
#ifdef B110
	case 110:
		return B110;
#endif
#ifdef B75
	case 75:
		return B75;
#endif
#ifdef B50
	case 50:
		return B50;
#endif
	default:
#ifdef BOTHER
		return BOTHER;
#else
		return B0;
#endif
	}
}

static int
_is_within_tolerance(int value, int reference, int tolerance)
{
	return 100 * value >= reference * (100 - tolerance) &&
	       100 * value <= reference * (100 + tolerance);
}

static int
kwboot_tty_change_baudrate(int fd, int baudrate)
{
	struct termios tio;
	speed_t speed;
	int rc;

	rc = tcgetattr(fd, &tio);
	if (rc)
		return rc;

	speed = kwboot_tty_baudrate_to_speed(baudrate);
	if (speed == B0) {
		errno = EINVAL;
		return -1;
	}

#ifdef BOTHER
	if (speed == BOTHER)
		tio.c_ospeed = tio.c_ispeed = baudrate;
#endif

	rc = cfsetospeed(&tio, speed);
	if (rc)
		return rc;

	rc = cfsetispeed(&tio, speed);
	if (rc)
		return rc;

	rc = tcsetattr(fd, TCSANOW, &tio);
	if (rc)
		return rc;

	rc = tcgetattr(fd, &tio);
	if (rc)
		return rc;

	if (cfgetospeed(&tio) != speed || cfgetispeed(&tio) != speed)
		goto baud_fail;

#ifdef BOTHER
	/*
	 * Check whether set baudrate is within 3% tolerance.
	 * If BOTHER is defined, Linux always fills out c_ospeed / c_ispeed
	 * with real values.
	 */
	if (!_is_within_tolerance(tio.c_ospeed, baudrate, 3))
		goto baud_fail;

	if (!_is_within_tolerance(tio.c_ispeed, baudrate, 3))
		goto baud_fail;
#endif

	return 0;

baud_fail:
	fprintf(stderr, "Could not set baudrate to requested value\n");
	errno = EINVAL;
	return -1;
}

static int
kwboot_open_tty(const char *path, int baudrate)
{
	int rc, fd, flags;
	struct termios tio;

	rc = -1;

	fd = open(path, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd < 0)
		goto out;

	rc = tcgetattr(fd, &tio);
	if (rc)
		goto out;

	cfmakeraw(&tio);
	tio.c_cflag |= CREAD | CLOCAL;
	tio.c_cflag &= ~(CSTOPB | HUPCL | CRTSCTS);
	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 0;

	rc = tcsetattr(fd, TCSANOW, &tio);
	if (rc)
		goto out;

	flags = fcntl(fd, F_GETFL);
	if (flags < 0)
		goto out;

	rc = fcntl(fd, F_SETFL, flags & ~O_NDELAY);
	if (rc)
		goto out;

	rc = kwboot_tty_change_baudrate(fd, baudrate);
	if (rc)
		goto out;

	rc = fd;
out:
	if (rc < 0) {
		if (fd >= 0)
			close(fd);
	}

	return rc;
}

static void *
kwboot_msg_write_handler(void *arg)
{
	int tty = *(int *)((void **)arg)[0];
	const void *msg = ((void **)arg)[1];
	int rsp_timeo = msg_rsp_timeo;
	int i, dummy_oldtype;

	/* allow to cancel this thread at any time */
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &dummy_oldtype);

	while (1) {
		/* write 128 samples of message pattern into the output queue without waiting */
		for (i = 0; i < 128; i++) {
			if (kwboot_tty_send(tty, msg, 8, 1) < 0) {
				perror("\nFailed to send message pattern");
				exit(1);
			}
		}
		/* wait until output queue is transmitted and then make pause */
		if (tcdrain(tty) < 0) {
			perror("\nFailed to send message pattern");
			exit(1);
		}
		/* BootROM requires pause on UART after it detects message pattern */
		usleep(rsp_timeo * 1000);
	}
}

static int
kwboot_msg_start_thread(pthread_t *thread, int *tty, void *msg)
{
	void *arg[2];
	int rc;

	arg[0] = tty;
	arg[1] = msg;
	rc = pthread_create(thread, NULL, kwboot_msg_write_handler, arg);
	if (rc) {
		errno = rc;
		return -1;
	}

	return 0;
}

static int
kwboot_msg_stop_thread(pthread_t thread)
{
	int rc;

	rc = pthread_cancel(thread);
	if (rc) {
		errno = rc;
		return -1;
	}

	rc = pthread_join(thread, NULL);
	if (rc) {
		errno = rc;
		return -1;
	}

	return 0;
}

static int
kwboot_bootmsg(int tty)
{
	struct kwboot_block block;
	pthread_t write_thread;
	int rc, err;
	char c;

	/* flush input and output queue */
	tcflush(tty, TCIOFLUSH);

	rc = kwboot_msg_start_thread(&write_thread, &tty, kwboot_msg_boot);
	if (rc) {
		perror("Failed to start write thread");
		return rc;
	}

	kwboot_printv("Sending boot message. Please reboot the target...");

	err = 0;
	while (1) {
		kwboot_spinner();

		rc = kwboot_tty_recv(tty, &c, 1, msg_rsp_timeo);
		if (rc && errno == ETIMEDOUT) {
			continue;
		} else if (rc) {
			err = errno;
			break;
		}

		if (c == NAK)
			break;
	}

	kwboot_printv("\n");

	rc = kwboot_msg_stop_thread(write_thread);
	if (rc) {
		perror("Failed to stop write thread");
		return rc;
	}

	if (err) {
		errno = err;
		perror("Failed to read response for boot message pattern");
		return -1;
	}

	/*
	 * At this stage we have sent more boot message patterns and BootROM
	 * (at least on Armada XP and 385) started interpreting sent bytes as
	 * part of xmodem packets. If BootROM is expecting SOH byte as start of
	 * a xmodem packet and it receives byte 0xff, then it throws it away and
	 * sends a NAK reply to host. If BootROM does not receive any byte for
	 * 2s when expecting some continuation of the xmodem packet, it throws
	 * away the partially received xmodem data and sends NAK reply to host.
	 *
	 * Therefore for starting xmodem transfer we have two options: Either
	 * wait 2s or send 132 0xff bytes (which is the size of xmodem packet)
	 * to ensure that BootROM throws away any partially received data.
	 */

	/* flush output queue with remaining boot message patterns */
	rc = tcflush(tty, TCOFLUSH);
	if (rc) {
		perror("Failed to flush output queue");
		return rc;
	}

	/* send one xmodem packet with 0xff bytes to force BootROM to re-sync */
	memset(&block, 0xff, sizeof(block));
	rc = kwboot_tty_send(tty, &block, sizeof(block), 0);
	if (rc) {
		perror("Failed to send sync sequence");
		return rc;
	}

	/*
	 * Sending 132 bytes via 115200B/8-N-1 takes 11.45 ms, reading 132 bytes
	 * takes 11.45 ms, so waiting for 30 ms should be enough.
	 */
	usleep(30 * 1000);

	/* flush remaining NAK replies from input queue */
	rc = tcflush(tty, TCIFLUSH);
	if (rc) {
		perror("Failed to flush input queue");
		return rc;
	}

	return 0;
}

static int
kwboot_debugmsg(int tty)
{
	unsigned char buf[8192];
	pthread_t write_thread;
	int rc, err, i, pos;
	size_t off;

	/* flush input and output queue */
	tcflush(tty, TCIOFLUSH);

	rc = kwboot_msg_start_thread(&write_thread, &tty, kwboot_msg_debug);
	if (rc) {
		perror("Failed to start write thread");
		return rc;
	}

	kwboot_printv("Sending debug message. Please reboot the target...");
	kwboot_spinner();

	err = 0;
	off = 0;
	while (1) {
		/* Read immediately all bytes in queue without waiting */
		rc = read(tty, buf + off, sizeof(buf) - off);
		if ((rc < 0 && errno == EINTR) || rc == 0) {
			continue;
		} else if (rc < 0) {
			err = errno;
			break;
		}
		off += rc - 1;

		kwboot_spinner();

		/*
		 * Check if we received at least 4 debug message patterns
		 * (console echo from BootROM) in cyclic buffer
		 */

		for (pos = 0; pos < sizeof(kwboot_msg_debug); pos++)
			if (buf[off] == kwboot_msg_debug[(pos + off) % sizeof(kwboot_msg_debug)])
				break;

		for (i = off; i >= 0; i--)
			if (buf[i] != kwboot_msg_debug[(pos + i) % sizeof(kwboot_msg_debug)])
				break;

		off -= i;

		if (off >= 4 * sizeof(kwboot_msg_debug))
			break;

		/* If not move valid suffix from end of the buffer to the beginning of buffer */
		memmove(buf, buf + i + 1, off);
	}

	kwboot_printv("\n");

	rc = kwboot_msg_stop_thread(write_thread);
	if (rc) {
		perror("Failed to stop write thread");
		return rc;
	}

	if (err) {
		errno = err;
		perror("Failed to read response for debug message pattern");
		return -1;
	}

	/* flush output queue with remaining debug message patterns */
	rc = tcflush(tty, TCOFLUSH);
	if (rc) {
		perror("Failed to flush output queue");
		return rc;
	}

	kwboot_printv("Clearing input buffer...\n");

	/*
	 * Wait until BootROM transmit all remaining echo characters.
	 * Experimentally it was measured that for Armada 385 BootROM
	 * it is required to wait at least 0.415s. So wait 0.5s.
	 */
	usleep(500 * 1000);

	/*
	 * In off variable is stored number of characters received after the
	 * successful detection of echo reply. So these characters are console
	 * echo for other following debug message patterns. BootROM may have in
	 * its output queue other echo characters which were being transmitting
	 * before above sleep call. So read remaining number of echo characters
	 * sent by the BootROM now.
	 */
	while ((rc = kwboot_tty_recv(tty, &buf[0], 1, 0)) == 0)
		off++;
	if (errno != ETIMEDOUT) {
		perror("Failed to read response");
		return rc;
	}

	/*
	 * Clear every echo character set by the BootROM by backspace byte.
	 * This is required prior writing any command to the BootROM debug
	 * because BootROM command line buffer has limited size. If length
	 * of the command is larger than buffer size then it looks like
	 * that Armada 385 BootROM crashes after sending ENTER. So erase it.
	 * Experimentally it was measured that for Armada 385 BootROM it is
	 * required to send at least 3 backspace bytes for one echo character.
	 * This is unknown why. But lets do it.
	 */
	off *= 3;
	memset(buf, '\x08', sizeof(buf));
	while (off > sizeof(buf)) {
		rc = kwboot_tty_send(tty, buf, sizeof(buf), 1);
		if (rc) {
			perror("Failed to send clear sequence");
			return rc;
		}
		off -= sizeof(buf);
	}
	rc = kwboot_tty_send(tty, buf, off, 0);
	if (rc) {
		perror("Failed to send clear sequence");
		return rc;
	}

	usleep(msg_rsp_timeo * 1000);
	rc = tcflush(tty, TCIFLUSH);
	if (rc) {
		perror("Failed to flush input queue");
		return rc;
	}

	return 0;
}

static size_t
kwboot_xm_makeblock(struct kwboot_block *block, const void *data,
		    size_t size, int pnum)
{
	size_t i, n;

	block->soh = SOH;
	block->pnum = pnum;
	block->_pnum = ~block->pnum;

	n = size < KWBOOT_XM_BLKSZ ? size : KWBOOT_XM_BLKSZ;
	memcpy(&block->data[0], data, n);
	memset(&block->data[n], 0, KWBOOT_XM_BLKSZ - n);

	block->csum = 0;
	for (i = 0; i < n; i++)
		block->csum += block->data[i];

	return n;
}

static uint64_t
_now(void)
{
	struct timespec ts;

	if (clock_gettime(CLOCK_MONOTONIC, &ts)) {
		static int err_print;

		if (!err_print) {
			perror("clock_gettime() does not work");
			err_print = 1;
		}

		/* this will just make the timeout not work */
		return -1ULL;
	}

	return ts.tv_sec * 1000ULL + (ts.tv_nsec + 500000) / 1000000;
}

static int
_is_xm_reply(char c)
{
	return c == ACK || c == NAK;
}

static int
_xm_reply_to_error(int c)
{
	int rc = -1;

	switch (c) {
	case ACK:
		rc = 0;
		break;
	case NAK:
		errno = EBADMSG;
		break;
	default:
		errno = EPROTO;
		break;
	}

	return rc;
}

static int
kwboot_baud_magic_handle(int fd, char c, int baudrate)
{
	static size_t rcv_len;

	if (rcv_len < sizeof(kwb_baud_magic)) {
		/* try to recognize whole magic word */
		if (c == kwb_baud_magic[rcv_len]) {
			rcv_len++;
		} else {
			printf("%.*s%c", (int)rcv_len, kwb_baud_magic, c);
			fflush(stdout);
			rcv_len = 0;
		}
	}

	if (rcv_len == sizeof(kwb_baud_magic)) {
		/* magic word received */
		kwboot_printv("\nChanging baudrate to %d Bd\n", baudrate);

		return kwboot_tty_change_baudrate(fd, baudrate) ? : 1;
	} else {
		return 0;
	}
}

static int
kwboot_xm_recv_reply(int fd, char *c, int stop_on_non_xm,
		     int ignore_nak_reply,
		     int allow_non_xm, int *non_xm_print,
		     int baudrate, int *baud_changed)
{
	int timeout = allow_non_xm ? KWBOOT_HDR_RSP_TIMEO : blk_rsp_timeo;
	uint64_t recv_until = _now() + timeout;
	int rc;

	while (1) {
		rc = kwboot_tty_recv(fd, c, 1, timeout);
		if (rc) {
			if (errno != ETIMEDOUT)
				return rc;
			else if (allow_non_xm && *non_xm_print)
				return -1;
			else
				*c = NAK;
		}

		/* If received xmodem reply, end. */
		if (_is_xm_reply(*c)) {
			if (*c == NAK && ignore_nak_reply) {
				timeout = recv_until - _now();
				if (timeout >= 0)
					continue;
			}
			break;
		}

		/*
		 * If receiving/printing non-xmodem text output is allowed and
		 * such a byte was received, we want to increase receiving time
		 * and either:
		 * - print the byte, if it is not part of baudrate change magic
		 *   sequence while baudrate change was requested (-B option)
		 * - change baudrate
		 * Otherwise decrease timeout by time elapsed.
		 */
		if (allow_non_xm) {
			recv_until = _now() + timeout;

			if (baudrate && !*baud_changed) {
				rc = kwboot_baud_magic_handle(fd, *c, baudrate);
				if (rc == 1)
					*baud_changed = 1;
				else if (!rc)
					*non_xm_print = 1;
				else
					return rc;
			} else if (!baudrate || !*baud_changed) {
				putchar(*c);
				fflush(stdout);
				*non_xm_print = 1;
			}
		} else {
			if (stop_on_non_xm)
				break;
			timeout = recv_until - _now();
			if (timeout < 0) {
				errno = ETIMEDOUT;
				return -1;
			}
		}
	}

	return 0;
}

static int
kwboot_xm_sendblock(int fd, struct kwboot_block *block, int allow_non_xm,
		    int *done_print, int baudrate, int allow_retries)
{
	int non_xm_print, baud_changed;
	int rc, err, retries;
	char c;

	*done_print = 0;
	non_xm_print = 0;
	baud_changed = 0;

	retries = 0;
	do {
		rc = kwboot_tty_send(fd, block, sizeof(*block), 1);
		if (rc)
			goto err;

		if (allow_non_xm && !*done_print) {
			kwboot_progress(100, '.');
			kwboot_printv("Done\n");
			*done_print = 1;
		}

		rc = kwboot_xm_recv_reply(fd, &c, retries < 3,
					  retries > 8,
					  allow_non_xm, &non_xm_print,
					  baudrate, &baud_changed);
		if (rc)
			goto err;

		if (!allow_non_xm && c != ACK) {
			if (c == NAK && allow_retries && retries + 1 < 16)
				kwboot_progress(-1, '+');
			else
				kwboot_progress(-1, 'E');
		}
	} while (c == NAK && allow_retries && retries++ < 16);

	if (non_xm_print)
		kwboot_printv("\n");

	if (allow_non_xm && baudrate && !baud_changed) {
		fprintf(stderr, "Baudrate was not changed\n");
		errno = EPROTO;
		return -1;
	}

	return _xm_reply_to_error(c);
err:
	err = errno;
	kwboot_printv("\n");
	errno = err;
	return rc;
}

static int
kwboot_xm_finish(int fd)
{
	int rc, retries;
	char c;

	kwboot_printv("Finishing transfer\n");

	retries = 0;
	do {
		rc = kwboot_tty_send_char(fd, EOT);
		if (rc)
			return rc;

		rc = kwboot_xm_recv_reply(fd, &c, retries < 3,
					  retries > 8,
					  0, NULL, 0, NULL);
		if (rc)
			return rc;
	} while (c == NAK && retries++ < 16);

	return _xm_reply_to_error(c);
}

static int
kwboot_xmodem_one(int tty, int *pnum, int header, const uint8_t *data,
		  size_t size, int baudrate)
{
	int done_print = 0;
	size_t sent, left;
	int rc;

	kwboot_printv("Sending boot image %s (%zu bytes)...\n",
		      header ? "header" : "data", size);

	left = size;
	sent = 0;

	while (sent < size) {
		struct kwboot_block block;
		int last_block;
		size_t blksz;

		blksz = kwboot_xm_makeblock(&block, data, left, (*pnum)++);
		data += blksz;

		last_block = (left <= blksz);

		/*
		 * Handling of repeated xmodem packets is completely broken in
		 * Armada 385 BootROM - it completely ignores xmodem packet
		 * numbers, they are only used for checksum verification.
		 * BootROM can handle a retry of the xmodem packet only during
		 * the transmission of kwbimage header and only if BootROM
		 * itself sent NAK response to previous attempt (it does it on
		 * checksum failure). During the transmission of kwbimage data
		 * part, BootROM always expects next xmodem packet, even if it
		 * sent NAK to previous attempt - there is absolutely no way to
		 * repair incorrectly transmitted xmodem packet during kwbimage
		 * data part upload. Also, if kwboot receives non-ACK/NAK
		 * response (meaning that original BootROM response was damaged
		 * on UART) there is no way to detect if BootROM accepted xmodem
		 * packet or not and no way to check if kwboot could repeat the
		 * packet or not.
		 *
		 * Stop transfer and return failure if kwboot receives unknown
		 * reply if non-xmodem reply is not allowed (for all xmodem
		 * packets except the last header packet) or when non-ACK reply
		 * is received during data part transfer.
		 */
		rc = kwboot_xm_sendblock(tty, &block, header && last_block,
					 &done_print, baudrate, header);
		if (rc)
			goto out;

		sent += blksz;
		left -= blksz;

		if (!done_print)
			kwboot_progress(sent * 100 / size, '.');
	}

	if (!done_print)
		kwboot_printv("Done\n");

	return 0;
out:
	kwboot_printv("\n");
	return rc;
}

static int
kwboot_xmodem(int tty, const void *_img, size_t size, int baudrate)
{
	const uint8_t *img = _img;
	int rc, pnum;
	size_t hdrsz;

	hdrsz = kwbheader_size(img);

	/*
	 * If header size is not aligned to xmodem block size (which applies
	 * for all images in kwbimage v0 format) then we have to ensure that
	 * the last xmodem block of header contains beginning of the data
	 * followed by the header. So align header size to xmodem block size.
	 */
	hdrsz += (KWBOOT_XM_BLKSZ - hdrsz % KWBOOT_XM_BLKSZ) % KWBOOT_XM_BLKSZ;
	if (hdrsz > size)
		hdrsz = size;

	pnum = 1;

	rc = kwboot_xmodem_one(tty, &pnum, 1, img, hdrsz, baudrate);
	if (rc)
		return rc;

	/*
	 * If we have already sent image data as a part of the last
	 * xmodem header block then we have nothing more to send.
	 */
	if (hdrsz < size) {
		img += hdrsz;
		size -= hdrsz;
		rc = kwboot_xmodem_one(tty, &pnum, 0, img, size, 0);
		if (rc)
			return rc;
	}

	rc = kwboot_xm_finish(tty);
	if (rc)
		return rc;

	if (baudrate) {
		kwboot_printv("\nChanging baudrate back to 115200 Bd\n\n");
		rc = kwboot_tty_change_baudrate(tty, 115200);
		if (rc)
			return rc;
	}

	return 0;
}

static int
kwboot_term_pipe(int in, int out, const char *quit, int *s, const char *kbs, int *k)
{
	char buf[128];
	ssize_t nin, noff;

	nin = read(in, buf, sizeof(buf));
	if (nin <= 0)
		return -1;

	noff = 0;

	if (quit || kbs) {
		int i;

		for (i = 0; i < nin; i++) {
			if ((quit || kbs) &&
			    (!quit || buf[i] != quit[*s]) &&
			    (!kbs || buf[i] != kbs[*k])) {
				const char *prefix;
				int plen;

				if (quit && kbs) {
					prefix = (*s >= *k) ? quit : kbs;
					plen = (*s >= *k) ? *s : *k;
				} else if (quit) {
					prefix = quit;
					plen = *s;
				} else {
					prefix = kbs;
					plen = *k;
				}

				if (plen > i && kwboot_write(out, prefix, plen - i) < 0)
					return -1;
			}

			if (quit && buf[i] == quit[*s]) {
				(*s)++;
				if (!quit[*s]) {
					nin = (i > *s) ? (i - *s) : 0;
					break;
				}
			} else if (quit) {
				*s = 0;
			}

			if (kbs && buf[i] == kbs[*k]) {
				(*k)++;
				if (!kbs[*k]) {
					if (i > *k + noff &&
					    kwboot_write(out, buf + noff, i - *k - noff) < 0)
						return -1;
					/*
					 * Replace backspace key by '\b' (0x08)
					 * byte which is the only recognized
					 * backspace byte by Marvell BootROM.
					 */
					if (write(out, "\x08", 1) < 0)
						return -1;
					noff = i + 1;
					*k = 0;
				}
			} else if (kbs) {
				*k = 0;
			}
		}

		if (i == nin) {
			i = 0;
			if (quit && i < *s)
				i = *s;
			if (kbs && i < *k)
				i = *k;
			nin -= (nin > i) ? i : nin;
		}
	}

	if (nin > noff && kwboot_write(out, buf + noff, nin - noff) < 0)
		return -1;

	return 0;
}

static int
kwboot_terminal(int tty)
{
	int rc, in, s, k;
	const char *kbs = NULL;
	const char *quit = "\34c";
	struct termios otio, tio;

	rc = -1;

	in = STDIN_FILENO;
	if (isatty(in)) {
		rc = tcgetattr(in, &otio);
		if (!rc) {
			tio = otio;
			cfmakeraw(&tio);
			rc = tcsetattr(in, TCSANOW, &tio);
		}
		if (rc) {
			perror("tcsetattr");
			goto out;
		}

		/*
		 * Get sequence for backspace key used by the current
		 * terminal. Every occurrence of this sequence will be
		 * replaced by '\b' byte which is the only recognized
		 * backspace byte by Marvell BootROM.
		 *
		 * Note that we cannot read this sequence from termios
		 * c_cc[VERASE] as VERASE is valid only when ICANON is
		 * set in termios c_lflag, which is not case for us.
		 *
		 * Also most terminals do not set termios c_cc[VERASE]
		 * as c_cc[VERASE] can specify only one-byte sequence
		 * and instead let applications to read (possible
		 * multi-byte) sequence for backspace key from "kbs"
		 * terminfo database based on $TERM env variable.
		 *
		 * So read "kbs" from terminfo database via tigetstr()
		 * call after successful setupterm(). Most terminals
		 * use byte 0x7F for backspace key, so replacement with
		 * '\b' is required.
		 */
		if (setupterm(NULL, STDOUT_FILENO, &rc) == 0) {
			kbs = tigetstr("kbs");
			if (kbs == (char *)-1)
				kbs = NULL;
		}

		kwboot_printv("[Type Ctrl-%c + %c to quit]\r\n",
			      quit[0] | 0100, quit[1]);
	} else
		in = -1;

	rc = 0;
	s = 0;
	k = 0;

	do {
		fd_set rfds;
		int nfds = 0;

		FD_ZERO(&rfds);
		FD_SET(tty, &rfds);
		nfds = nfds < tty ? tty : nfds;

		if (in >= 0) {
			FD_SET(in, &rfds);
			nfds = nfds < in ? in : nfds;
		}

		nfds = select(nfds + 1, &rfds, NULL, NULL, NULL);
		if (nfds < 0)
			break;

		if (FD_ISSET(tty, &rfds)) {
			rc = kwboot_term_pipe(tty, STDOUT_FILENO, NULL, NULL, NULL, NULL);
			if (rc)
				break;
		}

		if (in >= 0 && FD_ISSET(in, &rfds)) {
			rc = kwboot_term_pipe(in, tty, quit, &s, kbs, &k);
			if (rc)
				break;
		}
	} while (quit[s] != 0);

	if (in >= 0)
		tcsetattr(in, TCSANOW, &otio);
	printf("\n");
out:
	return rc;
}

static void *
kwboot_read_image(const char *path, size_t *size, size_t reserve)
{
	int rc, fd;
	void *img;
	off_t len;
	off_t tot;

	rc = -1;
	img = NULL;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		goto out;

	len = lseek(fd, 0, SEEK_END);
	if (len == (off_t)-1)
		goto out;

	if (lseek(fd, 0, SEEK_SET) == (off_t)-1)
		goto out;

	img = malloc(len + reserve);
	if (!img)
		goto out;

	tot = 0;
	while (tot < len) {
		ssize_t rd = read(fd, img + tot, len - tot);

		if (rd < 0)
			goto out;

		tot += rd;

		if (!rd && tot < len) {
			errno = EIO;
			goto out;
		}
	}

	rc = 0;
	*size = len;
out:
	if (rc && img) {
		free(img);
		img = NULL;
	}
	if (fd >= 0)
		close(fd);

	return img;
}

static uint8_t
kwboot_hdr_csum8(const void *hdr)
{
	const uint8_t *data = hdr;
	uint8_t csum;
	size_t size;

	size = kwbheader_size_for_csum(hdr);

	for (csum = 0; size-- > 0; data++)
		csum += *data;

	return csum;
}

static uint32_t *
kwboot_img_csum32_ptr(void *img)
{
	struct main_hdr_v1 *hdr = img;
	uint32_t datasz;

	datasz = le32_to_cpu(hdr->blocksize) - sizeof(uint32_t);

	return img + le32_to_cpu(hdr->srcaddr) + datasz;
}

static uint32_t
kwboot_img_csum32(const void *img)
{
	const struct main_hdr_v1 *hdr = img;
	uint32_t datasz, csum = 0;
	const uint32_t *data;

	datasz = le32_to_cpu(hdr->blocksize) - sizeof(csum);
	if (datasz % sizeof(uint32_t))
		return 0;

	data = img + le32_to_cpu(hdr->srcaddr);
	while (datasz > 0) {
		csum += le32_to_cpu(*data++);
		datasz -= 4;
	}

	return cpu_to_le32(csum);
}

static int
kwboot_img_is_secure(void *img)
{
	struct opt_hdr_v1 *ohdr;

	for_each_opt_hdr_v1 (ohdr, img)
		if (ohdr->headertype == OPT_HDR_V1_SECURE_TYPE)
			return 1;

	return 0;
}

static int
kwboot_img_has_ddr_init(void *img)
{
	const struct register_set_hdr_v1 *rhdr;
	const struct main_hdr_v0 *hdr0;
	struct opt_hdr_v1 *ohdr;
	u32 ohdrsz;
	int last;

	/*
	 * kwbimage v0 image headers contain DDR init code either in
	 * extension header or in binary code header.
	 */
	if (kwbimage_version(img) == 0) {
		hdr0 = img;
		return hdr0->ext || hdr0->bin;
	}

	/*
	 * kwbimage v1 image headers contain DDR init code either in binary
	 * code header or in a register set list header with SDRAM_SETUP.
	 */
	for_each_opt_hdr_v1 (ohdr, img) {
		if (ohdr->headertype == OPT_HDR_V1_BINARY_TYPE)
			return 1;
		if (ohdr->headertype == OPT_HDR_V1_REGISTER_TYPE) {
			rhdr = (const struct register_set_hdr_v1 *)ohdr;
			ohdrsz = opt_hdr_v1_size(ohdr);
			if (ohdrsz >= sizeof(*ohdr) + sizeof(rhdr->data[0].last_entry)) {
				ohdrsz -= sizeof(*ohdr) + sizeof(rhdr->data[0].last_entry);
				last = ohdrsz / sizeof(rhdr->data[0].entry);
				if (rhdr->data[last].last_entry.delay ==
				    REGISTER_SET_HDR_OPT_DELAY_SDRAM_SETUP)
					return 1;
			}
		}
	}

	return 0;
}

static void *
kwboot_img_grow_data_right(void *img, size_t *size, size_t grow)
{
	struct main_hdr_v1 *hdr = img;
	void *result;

	/*
	 * 32-bit checksum comes after end of image code, so we will be putting
	 * new code there. So we get this pointer and then increase data size
	 * (since increasing data size changes kwboot_img_csum32_ptr() return
	 *  value).
	 */
	result = kwboot_img_csum32_ptr(img);
	hdr->blocksize = cpu_to_le32(le32_to_cpu(hdr->blocksize) + grow);
	*size += grow;

	return result;
}

static void
kwboot_img_grow_hdr(void *img, size_t *size, size_t grow)
{
	uint32_t hdrsz, datasz, srcaddr;
	struct main_hdr_v1 *hdr = img;
	struct opt_hdr_v1 *ohdr;
	uint8_t *data;

	srcaddr = le32_to_cpu(hdr->srcaddr);

	/* calculate real used space in kwbimage header */
	if (kwbimage_version(img) == 0) {
		hdrsz = kwbheader_size(img);
	} else {
		hdrsz = sizeof(*hdr);
		for_each_opt_hdr_v1 (ohdr, hdr)
			hdrsz += opt_hdr_v1_size(ohdr);
	}

	data = (uint8_t *)img + srcaddr;
	datasz = *size - srcaddr;

	/* only move data if there is not enough space */
	if (hdrsz + grow > srcaddr) {
		size_t need = hdrsz + grow - srcaddr;

		/* move data by enough bytes */
		memmove(data + need, data, datasz);

		hdr->srcaddr = cpu_to_le32(srcaddr + need);
		*size += need;
	}

	if (kwbimage_version(img) == 1) {
		hdrsz += grow;
		if (hdrsz > kwbheader_size(img)) {
			hdr->headersz_msb = hdrsz >> 16;
			hdr->headersz_lsb = cpu_to_le16(hdrsz & 0xffff);
		}
	}
}

static void *
kwboot_add_bin_ohdr_v1(void *img, size_t *size, uint32_t binsz)
{
	struct main_hdr_v1 *hdr = img;
	struct opt_hdr_v1 *ohdr;
	uint32_t num_args;
	uint32_t offset;
	uint32_t ohdrsz;
	uint8_t *prev_ext;

	if (hdr->ext) {
		for_each_opt_hdr_v1 (ohdr, img)
			if (opt_hdr_v1_next(ohdr) == NULL)
				break;

		prev_ext = opt_hdr_v1_ext(ohdr);
		ohdr = _opt_hdr_v1_next(ohdr);
	} else {
		ohdr = (void *)(hdr + 1);
		prev_ext = &hdr->ext;
	}

	/*
	 * ARM executable code inside the BIN header on some mvebu platforms
	 * (e.g. A370, AXP) must always be aligned with the 128-bit boundary.
	 * This requirement can be met by inserting dummy arguments into
	 * BIN header, if needed.
	 */
	offset = &ohdr->data[4] - (char *)img;
	num_args = ((16 - offset % 16) % 16) / sizeof(uint32_t);

	ohdrsz = sizeof(*ohdr) + 4 + 4 * num_args + binsz + 4;
	kwboot_img_grow_hdr(hdr, size, ohdrsz);

	*prev_ext = 1;

	ohdr->headertype = OPT_HDR_V1_BINARY_TYPE;
	ohdr->headersz_msb = ohdrsz >> 16;
	ohdr->headersz_lsb = cpu_to_le16(ohdrsz & 0xffff);

	memset(&ohdr->data[0], 0, ohdrsz - sizeof(*ohdr));
	*(uint32_t *)&ohdr->data[0] = cpu_to_le32(num_args);

	return &ohdr->data[4 + 4 * num_args];
}

static void
_inject_baudrate_change_code(void *img, size_t *size, int for_data,
			     int old_baud, int new_baud)
{
	struct main_hdr_v1 *hdr = img;
	uint32_t orig_datasz;
	uint32_t codesz;
	uint8_t *code;

	if (for_data) {
		orig_datasz = le32_to_cpu(hdr->blocksize) - sizeof(uint32_t);

		codesz = sizeof(kwboot_baud_code) +
			 sizeof(kwboot_baud_code_data_jump);
		code = kwboot_img_grow_data_right(img, size, codesz);
	} else {
		codesz = sizeof(kwboot_baud_code_binhdr_pre) +
			 sizeof(kwboot_baud_code) +
			 sizeof(kwboot_baud_code_binhdr_post);
		code = kwboot_add_bin_ohdr_v1(img, size, codesz);

		codesz = sizeof(kwboot_baud_code_binhdr_pre);
		memcpy(code, kwboot_baud_code_binhdr_pre, codesz);
		code += codesz;
	}

	codesz = sizeof(kwboot_baud_code) - 2 * sizeof(uint32_t);
	memcpy(code, kwboot_baud_code, codesz);
	code += codesz;
	*(uint32_t *)code = cpu_to_le32(old_baud);
	code += sizeof(uint32_t);
	*(uint32_t *)code = cpu_to_le32(new_baud);
	code += sizeof(uint32_t);

	if (for_data) {
		codesz = sizeof(kwboot_baud_code_data_jump) - sizeof(uint32_t);
		memcpy(code, kwboot_baud_code_data_jump, codesz);
		code += codesz;
		*(uint32_t *)code = hdr->execaddr;
		code += sizeof(uint32_t);
		hdr->execaddr = cpu_to_le32(le32_to_cpu(hdr->destaddr) + orig_datasz);
	} else {
		codesz = sizeof(kwboot_baud_code_binhdr_post);
		memcpy(code, kwboot_baud_code_binhdr_post, codesz);
		code += codesz;
	}
}

static int
kwboot_img_guess_sata_blksz(void *img, uint32_t blkoff, uint32_t data_size, size_t total_size)
{
	uint32_t sum, *ptr, *end;
	int blksz;

	/*
	 * Try all possible sector sizes which are power of two,
	 * at least 512 bytes and up to the 32 kB.
	 */
	for (blksz = 512; blksz < 0x10000; blksz *= 2) {
		if (blkoff * blksz > total_size ||
		    blkoff * blksz + data_size > total_size ||
		    data_size % 4)
			break;

		/*
		 * Calculate data checksum and if it matches
		 * then tried blksz should be correct.
		 */
		ptr = img + blkoff * blksz;
		end = (void *)ptr + data_size - 4;
		for (sum = 0; ptr < end; ptr++)
			sum += *ptr;

		if (sum == *end)
			return blksz;
	}

	/* Fallback to 512 bytes */
	return 512;
}

static const char *
kwboot_img_type(uint8_t blockid)
{
	switch (blockid) {
	case IBR_HDR_I2C_ID: return "I2C";
	case IBR_HDR_SPI_ID: return "SPI";
	case IBR_HDR_NAND_ID: return "NAND";
	case IBR_HDR_SATA_ID: return "SATA";
	case IBR_HDR_PEX_ID: return "PEX";
	case IBR_HDR_UART_ID: return "UART";
	case IBR_HDR_SDIO_ID: return "SDIO";
	default: return "unknown";
	}
}

static int
kwboot_img_patch(void *img, size_t *size, int baudrate)
{
	struct main_hdr_v1 *hdr;
	struct opt_hdr_v1 *ohdr;
	uint32_t srcaddr;
	uint8_t csum;
	size_t hdrsz;
	int image_ver;
	int is_secure;

	hdr = img;

	if (*size < sizeof(struct main_hdr_v1)) {
		fprintf(stderr, "Invalid image header size\n");
		goto err;
	}

	image_ver = kwbimage_version(img);
	if (image_ver != 0 && image_ver != 1) {
		fprintf(stderr, "Invalid image header version\n");
		goto err;
	}

	hdrsz = kwbheader_size(hdr);

	if (*size < hdrsz) {
		fprintf(stderr, "Invalid image header size\n");
		goto err;
	}

	kwboot_printv("Detected kwbimage v%d with %s boot signature\n", image_ver, kwboot_img_type(hdr->blockid));

	csum = kwboot_hdr_csum8(hdr) - hdr->checksum;
	if (csum != hdr->checksum) {
		fprintf(stderr, "Image has invalid header checksum stored in image header\n");
		goto err;
	}

	srcaddr = le32_to_cpu(hdr->srcaddr);

	switch (hdr->blockid) {
	case IBR_HDR_SATA_ID:
		hdr->srcaddr = cpu_to_le32(srcaddr * kwboot_img_guess_sata_blksz(img, srcaddr, le32_to_cpu(hdr->blocksize), *size));
		break;

	case IBR_HDR_PEX_ID:
		if (srcaddr == 0xFFFFFFFF)
			hdr->srcaddr = cpu_to_le32(hdrsz);
		break;

	case IBR_HDR_SPI_ID:
		if (hdr->destaddr == cpu_to_le32(0xFFFFFFFF)) {
			kwboot_printv("Patching destination and execution addresses from SPI/NOR XIP area to DDR area 0x00800000\n");
			hdr->destaddr = cpu_to_le32(0x00800000 + le32_to_cpu(hdr->srcaddr));
			hdr->execaddr = cpu_to_le32(0x00800000 + le32_to_cpu(hdr->execaddr));
		}
		break;
	}

	if (hdrsz > le32_to_cpu(hdr->srcaddr)) {
		fprintf(stderr, "Image has invalid data offset stored in image header\n");
		goto err;
	}

	if (*size < le32_to_cpu(hdr->srcaddr) + le32_to_cpu(hdr->blocksize)) {
		fprintf(stderr, "Image has invalid data size stored in image header\n");
		goto err;
	}

	for_each_opt_hdr_v1 (ohdr, hdr) {
		if (!opt_hdr_v1_valid_size(ohdr, (const uint8_t *)hdr + hdrsz)) {
			fprintf(stderr, "Invalid optional image header\n");
			goto err;
		}
	}

	/*
	 * The 32-bit data checksum is optional for UART image. If it is not
	 * present (checksum detected as invalid) then grow data part of the
	 * image for the checksum, so it can be inserted there.
	 */
	if (kwboot_img_csum32(img) != *kwboot_img_csum32_ptr(img)) {
		if (hdr->blockid != IBR_HDR_UART_ID) {
			fprintf(stderr, "Image has invalid data checksum\n");
			goto err;
		}
		kwboot_img_grow_data_right(img, size, sizeof(uint32_t));
		/* Update the 32-bit data checksum */
		*kwboot_img_csum32_ptr(img) = kwboot_img_csum32(img);
	}

	if (!kwboot_img_has_ddr_init(img) &&
	    (le32_to_cpu(hdr->destaddr) < 0x40000000 ||
	     le32_to_cpu(hdr->destaddr) + le32_to_cpu(hdr->blocksize) > 0x40034000)) {
		fprintf(stderr, "Image does not contain DDR init code needed for UART booting\n");
		goto err;
	}

	is_secure = kwboot_img_is_secure(img);

	if (hdr->blockid != IBR_HDR_UART_ID) {
		if (is_secure) {
			fprintf(stderr,
				"Image has secure header with signature for non-UART booting\n");
			goto err;
		}

		kwboot_printv("Patching image boot signature to UART\n");
		hdr->blockid = IBR_HDR_UART_ID;
	}

	if (!is_secure) {
		if (image_ver == 1) {
			/*
			 * Tell BootROM to send BootROM messages to UART port
			 * number 0 (used also for UART booting) with default
			 * baudrate (which should be 115200) and do not touch
			 * UART MPP configuration.
			 */
			hdr->flags |= 0x1;
			hdr->options &= ~0x1F;
			hdr->options |= MAIN_HDR_V1_OPT_BAUD_DEFAULT;
			hdr->options |= 0 << 3;
		}
		if (image_ver == 0)
			((struct main_hdr_v0 *)img)->nandeccmode = IBR_HDR_ECC_DISABLED;
		hdr->nandpagesize = 0;
	}

	if (baudrate) {
		if (image_ver == 0) {
			fprintf(stderr,
				"Cannot inject code for changing baudrate into v0 image header\n");
			goto err;
		}

		if (is_secure) {
			fprintf(stderr,
				"Cannot inject code for changing baudrate into image with secure header\n");
			goto err;
		}

		/*
		 * First inject code that changes the baudrate from the default
		 * value of 115200 Bd to requested value. This code is inserted
		 * as a new opt hdr, so it is executed by BootROM after the
		 * header part is received.
		 */
		kwboot_printv("Injecting binary header code for changing baudrate to %d Bd\n",
			      baudrate);
		_inject_baudrate_change_code(img, size, 0, 115200, baudrate);

		/*
		 * Now inject code that changes the baudrate back to 115200 Bd.
		 * This code is appended after the data part of the image, and
		 * execaddr is changed so that it is executed before U-Boot
		 * proper.
		 */
		kwboot_printv("Injecting code for changing baudrate back\n");
		_inject_baudrate_change_code(img, size, 1, baudrate, 115200);

		/* Update the 32-bit data checksum */
		*kwboot_img_csum32_ptr(img) = kwboot_img_csum32(img);

		/* recompute header size */
		hdrsz = kwbheader_size(hdr);
	}

	if (hdrsz % KWBOOT_XM_BLKSZ) {
		size_t grow = KWBOOT_XM_BLKSZ - hdrsz % KWBOOT_XM_BLKSZ;

		if (is_secure) {
			fprintf(stderr, "Cannot align image with secure header\n");
			goto err;
		}

		kwboot_printv("Aligning image header to Xmodem block size\n");
		kwboot_img_grow_hdr(img, size, grow);
		hdrsz += grow;

		/*
		 * kwbimage v1 contains header size field and for UART type it
		 * must be set to the aligned xmodem header size because BootROM
		 * rounds header size down to xmodem block size.
		 */
		if (kwbimage_version(img) == 1) {
			hdr->headersz_msb = hdrsz >> 16;
			hdr->headersz_lsb = cpu_to_le16(hdrsz & 0xffff);
		}
	}

	/* Header size and source address must be same for UART type due to A38x BootROM bug */
	if (hdrsz != le32_to_cpu(hdr->srcaddr)) {
		if (is_secure) {
			fprintf(stderr, "Cannot align image with secure header\n");
			goto err;
		}

		kwboot_printv("Removing gap between image header and data\n");
		memmove(img + hdrsz, img + le32_to_cpu(hdr->srcaddr), le32_to_cpu(hdr->blocksize));
		hdr->srcaddr = cpu_to_le32(hdrsz);
	}

	hdr->checksum = kwboot_hdr_csum8(hdr) - csum;

	*size = le32_to_cpu(hdr->srcaddr) + le32_to_cpu(hdr->blocksize);
	return 0;
err:
	errno = EINVAL;
	return -1;
}

static void
kwboot_usage(FILE *stream, char *progname)
{
	fprintf(stream,
		"Usage: %s [OPTIONS] [-b <image> | -D <image> | -b | -d ] [-B <baud> ] [-t] <TTY>\n",
		progname);
	fprintf(stream, "\n");
	fprintf(stream,
		"  -b <image>: boot <image> with preamble (Kirkwood, Avanta, Armada 370/XP/375/38x/39x)\n");
	fprintf(stream,
		"  -D <image>: boot <image> without preamble (Dove)\n");
	fprintf(stream, "  -b: enter xmodem boot mode\n");
	fprintf(stream, "  -d: enter console debug mode\n");
	fprintf(stream, "  -a: use timings for Armada XP\n");
	fprintf(stream, "  -s <resp-timeo>: use specific response-timeout\n");
	fprintf(stream,
		"  -o <block-timeo>: use specific xmodem block timeout\n");
	fprintf(stream, "\n");
	fprintf(stream, "  -t: mini terminal\n");
	fprintf(stream, "\n");
	fprintf(stream, "  -B <baud>: set baud rate\n");
	fprintf(stream, "\n");
}

int
main(int argc, char **argv)
{
	const char *ttypath, *imgpath;
	int rv, rc, tty, term;
	int bootmsg;
	int debugmsg;
	void *img;
	size_t size;
	size_t after_img_rsv;
	int baudrate;
	int prev_optind;
	int c;

	rv = 1;
	tty = -1;
	bootmsg = 0;
	debugmsg = 0;
	imgpath = NULL;
	img = NULL;
	term = 0;
	size = 0;
	after_img_rsv = KWBOOT_XM_BLKSZ;
	baudrate = 115200;

	printf("kwboot version %s\n", PLAIN_VERSION);

	kwboot_verbose = isatty(STDOUT_FILENO);

	do {
		prev_optind = optind;
		c = getopt(argc, argv, "hbptaB:dD:q:s:o:");
		if (c < 0)
			break;

		switch (c) {
		case 'b':
			if (imgpath || bootmsg || debugmsg)
				goto usage;
			bootmsg = 1;
			if (prev_optind == optind)
				goto usage;
			/* Option -b could have optional argument which specify image path */
			if (optind < argc && argv[optind] && argv[optind][0] != '-')
				imgpath = argv[optind++];
			break;

		case 'D':
			if (imgpath || bootmsg || debugmsg)
				goto usage;
			bootmsg = 0;
			imgpath = optarg;
			break;

		case 'd':
			if (imgpath || bootmsg || debugmsg)
				goto usage;
			debugmsg = 1;
			break;

		case 'p':
			/* nop, for backward compatibility */
			break;

		case 't':
			term = 1;
			break;

		case 'a':
			msg_rsp_timeo = KWBOOT_MSG_RSP_TIMEO_AXP;
			break;

		case 'q':
			/* nop, for backward compatibility */
			break;

		case 's':
			msg_rsp_timeo = atoi(optarg);
			break;

		case 'o':
			blk_rsp_timeo = atoi(optarg);
			break;

		case 'B':
			baudrate = atoi(optarg);
			break;

		case 'h':
			rv = 0;
		default:
			goto usage;
		}
	} while (1);

	if (!bootmsg && !term && !debugmsg && !imgpath)
		goto usage;

	/*
	 * If there is no remaining argument but optional imgpath was parsed
	 * then it means that optional imgpath was eaten by getopt parser.
	 * Reassing imgpath to required ttypath argument.
	 */
	if (optind == argc && imgpath) {
		ttypath = imgpath;
		imgpath = NULL;
	} else if (optind + 1 == argc) {
		ttypath = argv[optind];
	} else {
		goto usage;
	}

	/* boot and debug message use baudrate 115200 */
	if (((bootmsg && !imgpath) || debugmsg) && baudrate != 115200) {
		fprintf(stderr, "Baudrate other than 115200 cannot be used for this operation.\n");
		goto usage;
	}

	tty = kwboot_open_tty(ttypath, baudrate);
	if (tty < 0) {
		perror(ttypath);
		goto out;
	}

	/*
	 * initial baudrate for image transfer is always 115200,
	 * the change to different baudrate is done only after the header is sent
	 */
	if (imgpath && baudrate != 115200) {
		rc = kwboot_tty_change_baudrate(tty, 115200);
		if (rc) {
			perror(ttypath);
			goto out;
		}
	}

	if (baudrate == 115200)
		/* do not change baudrate during Xmodem to the same value */
		baudrate = 0;
	else
		/* ensure we have enough space for baudrate change code */
		after_img_rsv += sizeof(struct opt_hdr_v1) + 8 + 16 +
				 sizeof(kwboot_baud_code_binhdr_pre) +
				 sizeof(kwboot_baud_code) +
				 sizeof(kwboot_baud_code_binhdr_post) +
				 KWBOOT_XM_BLKSZ +
				 sizeof(kwboot_baud_code) +
				 sizeof(kwboot_baud_code_data_jump) +
				 sizeof(uint32_t) +
				 KWBOOT_XM_BLKSZ;

	if (imgpath) {
		img = kwboot_read_image(imgpath, &size, after_img_rsv);
		if (!img) {
			perror(imgpath);
			goto out;
		}

		rc = kwboot_img_patch(img, &size, baudrate);
		if (rc) {
			fprintf(stderr, "%s: Invalid image.\n", imgpath);
			goto out;
		}
	}

	if (debugmsg) {
		rc = kwboot_debugmsg(tty);
		if (rc)
			goto out;
	} else if (bootmsg) {
		rc = kwboot_bootmsg(tty);
		if (rc)
			goto out;
	}

	if (img) {
		rc = kwboot_xmodem(tty, img, size, baudrate);
		if (rc) {
			perror("xmodem");
			goto out;
		}
	}

	if (term) {
		rc = kwboot_terminal(tty);
		if (rc && !(errno == EINTR)) {
			perror("terminal");
			goto out;
		}
	}

	rv = 0;
out:
	if (tty >= 0)
		close(tty);

	if (img)
		free(img);

	return rv;

usage:
	kwboot_usage(rv ? stderr : stdout, basename(argv[0]));
	goto out;
}
