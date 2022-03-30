/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2003, 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 */

/*
 * ARM PrimeCell UART's (PL010 & PL011)
 * ------------------------------------
 *
 *  Definitions common to both PL010 & PL011
 *
 */

#ifndef __ASSEMBLY__
/*
 * We can use a combined structure for PL010 and PL011, because they overlap
 * only in common registers.
 */
struct pl01x_regs {
	u32	dr;		/* 0x00 Data register */
	u32	ecr;		/* 0x04 Error clear register (Write) */
	u32	pl010_lcrh;	/* 0x08 Line control register, high byte */
	u32	pl010_lcrm;	/* 0x0C Line control register, middle byte */
	u32	pl010_lcrl;	/* 0x10 Line control register, low byte */
	u32	pl010_cr;	/* 0x14 Control register */
	u32	fr;		/* 0x18 Flag register (Read only) */
#ifdef CONFIG_PL011_SERIAL_RLCR
	u32	pl011_rlcr;	/* 0x1c Receive line control register */
#else
	u32	reserved;
#endif
	u32	ilpr;		/* 0x20 IrDA low-power counter register */
	u32	pl011_ibrd;	/* 0x24 Integer baud rate register */
	u32	pl011_fbrd;	/* 0x28 Fractional baud rate register */
	u32	pl011_lcrh;	/* 0x2C Line control register */
	u32	pl011_cr;	/* 0x30 Control register */
};

#ifdef CONFIG_DM_SERIAL

int pl01x_serial_ofdata_to_platdata(struct udevice *dev);
int pl01x_serial_probe(struct udevice *dev);

/* Needed for external pl01x_serial_ops drivers */
int pl01x_serial_putc(struct udevice *dev, const char ch);
int pl01x_serial_pending(struct udevice *dev, bool input);
int pl01x_serial_getc(struct udevice *dev);
int pl01x_serial_setbrg(struct udevice *dev, int baudrate);

struct pl01x_priv {
	struct pl01x_regs *regs;
	enum pl01x_type type;
};

#endif /* CONFIG_DM_SERIAL */
#endif /* !__ASSEMBLY__ */

#define UART_PL01x_RSR_OE               0x08
#define UART_PL01x_RSR_BE               0x04
#define UART_PL01x_RSR_PE               0x02
#define UART_PL01x_RSR_FE               0x01

#define UART_PL01x_FR_TXFE              0x80
#define UART_PL01x_FR_RXFF              0x40
#define UART_PL01x_FR_TXFF              0x20
#define UART_PL01x_FR_RXFE              0x10
#define UART_PL01x_FR_BUSY              0x08
#define UART_PL01x_FR_TMSK              (UART_PL01x_FR_TXFF + UART_PL01x_FR_BUSY)

/*
 *  PL010 definitions
 *
 */
#define UART_PL010_CR_LPE               (1 << 7)
#define UART_PL010_CR_RTIE              (1 << 6)
#define UART_PL010_CR_TIE               (1 << 5)
#define UART_PL010_CR_RIE               (1 << 4)
#define UART_PL010_CR_MSIE              (1 << 3)
#define UART_PL010_CR_IIRLP             (1 << 2)
#define UART_PL010_CR_SIREN             (1 << 1)
#define UART_PL010_CR_UARTEN            (1 << 0)

#define UART_PL010_LCRH_WLEN_8          (3 << 5)
#define UART_PL010_LCRH_WLEN_7          (2 << 5)
#define UART_PL010_LCRH_WLEN_6          (1 << 5)
#define UART_PL010_LCRH_WLEN_5          (0 << 5)
#define UART_PL010_LCRH_FEN             (1 << 4)
#define UART_PL010_LCRH_STP2            (1 << 3)
#define UART_PL010_LCRH_EPS             (1 << 2)
#define UART_PL010_LCRH_PEN             (1 << 1)
#define UART_PL010_LCRH_BRK             (1 << 0)


#define UART_PL010_BAUD_460800            1
#define UART_PL010_BAUD_230400            3
#define UART_PL010_BAUD_115200            7
#define UART_PL010_BAUD_57600             15
#define UART_PL010_BAUD_38400             23
#define UART_PL010_BAUD_19200             47
#define UART_PL010_BAUD_14400             63
#define UART_PL010_BAUD_9600              95
#define UART_PL010_BAUD_4800              191
#define UART_PL010_BAUD_2400              383
#define UART_PL010_BAUD_1200              767
/*
 *  PL011 definitions
 *
 */
#define UART_PL011_LCRH_SPS             (1 << 7)
#define UART_PL011_LCRH_WLEN_8          (3 << 5)
#define UART_PL011_LCRH_WLEN_7          (2 << 5)
#define UART_PL011_LCRH_WLEN_6          (1 << 5)
#define UART_PL011_LCRH_WLEN_5          (0 << 5)
#define UART_PL011_LCRH_FEN             (1 << 4)
#define UART_PL011_LCRH_STP2            (1 << 3)
#define UART_PL011_LCRH_EPS             (1 << 2)
#define UART_PL011_LCRH_PEN             (1 << 1)
#define UART_PL011_LCRH_BRK             (1 << 0)

#define UART_PL011_CR_CTSEN             (1 << 15)
#define UART_PL011_CR_RTSEN             (1 << 14)
#define UART_PL011_CR_OUT2              (1 << 13)
#define UART_PL011_CR_OUT1              (1 << 12)
#define UART_PL011_CR_RTS               (1 << 11)
#define UART_PL011_CR_DTR               (1 << 10)
#define UART_PL011_CR_RXE               (1 << 9)
#define UART_PL011_CR_TXE               (1 << 8)
#define UART_PL011_CR_LPE               (1 << 7)
#define UART_PL011_CR_IIRLP             (1 << 2)
#define UART_PL011_CR_SIREN             (1 << 1)
#define UART_PL011_CR_UARTEN            (1 << 0)

#define UART_PL011_IMSC_OEIM            (1 << 10)
#define UART_PL011_IMSC_BEIM            (1 << 9)
#define UART_PL011_IMSC_PEIM            (1 << 8)
#define UART_PL011_IMSC_FEIM            (1 << 7)
#define UART_PL011_IMSC_RTIM            (1 << 6)
#define UART_PL011_IMSC_TXIM            (1 << 5)
#define UART_PL011_IMSC_RXIM            (1 << 4)
#define UART_PL011_IMSC_DSRMIM          (1 << 3)
#define UART_PL011_IMSC_DCDMIM          (1 << 2)
#define UART_PL011_IMSC_CTSMIM          (1 << 1)
#define UART_PL011_IMSC_RIMIM           (1 << 0)
