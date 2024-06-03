/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
 */

/* Define MUSB_DEBUG before including this file to get debug macros */
#ifdef MUSB_DEBUG

#define MUSB_FLAGS_PRINT(v, x, y)		\
  if (((v) & MUSB_##x##_##y))			\
		serial_printf("\t\t"#y"\n")

static inline void musb_print_pwr(u8 b)
{
	serial_printf("\tpower   0x%2.2x\n", b);
	MUSB_FLAGS_PRINT(b, POWER, ISOUPDATE);
	MUSB_FLAGS_PRINT(b, POWER, SOFTCONN);
	MUSB_FLAGS_PRINT(b, POWER, HSENAB);
	MUSB_FLAGS_PRINT(b, POWER, HSMODE);
	MUSB_FLAGS_PRINT(b, POWER, RESET);
	MUSB_FLAGS_PRINT(b, POWER, RESUME);
	MUSB_FLAGS_PRINT(b, POWER, SUSPENDM);
	MUSB_FLAGS_PRINT(b, POWER, ENSUSPEND);
}

static inline void musb_print_csr0(u16 w)
{
	serial_printf("\tcsr0    0x%4.4x\n", w);
	MUSB_FLAGS_PRINT(w, CSR0, FLUSHFIFO);
	MUSB_FLAGS_PRINT(w, CSR0_P, SVDSETUPEND);
	MUSB_FLAGS_PRINT(w, CSR0_P, SVDRXPKTRDY);
	MUSB_FLAGS_PRINT(w, CSR0_P, SENDSTALL);
	MUSB_FLAGS_PRINT(w, CSR0_P, SETUPEND);
	MUSB_FLAGS_PRINT(w, CSR0_P, DATAEND);
	MUSB_FLAGS_PRINT(w, CSR0_P, SENTSTALL);
	MUSB_FLAGS_PRINT(w, CSR0, TXPKTRDY);
	MUSB_FLAGS_PRINT(w, CSR0, RXPKTRDY);
}

static inline void musb_print_intrusb(u8 b)
{
	serial_printf("\tintrusb 0x%2.2x\n", b);
	MUSB_FLAGS_PRINT(b, INTR, VBUSERROR);
	MUSB_FLAGS_PRINT(b, INTR, SESSREQ);
	MUSB_FLAGS_PRINT(b, INTR, DISCONNECT);
	MUSB_FLAGS_PRINT(b, INTR, CONNECT);
	MUSB_FLAGS_PRINT(b, INTR, SOF);
	MUSB_FLAGS_PRINT(b, INTR, RESUME);
	MUSB_FLAGS_PRINT(b, INTR, SUSPEND);

	if (b & MUSB_INTR_BABBLE)
		serial_printf("\t\tMUSB_INTR_RESET or MUSB_INTR_BABBLE\n");

}

static inline void musb_print_intrtx(u16 w)
{
	serial_printf("\tintrtx 0x%4.4x\n", w);
}

static inline void musb_print_intrrx(u16 w)
{
	serial_printf("\tintrx 0x%4.4x\n", w);
}

static inline void musb_print_devctl(u8 b)
{
	serial_printf("\tdevctl  0x%2.2x\n", b);
	if (b & MUSB_DEVCTL_BDEVICE)
		serial_printf("\t\tB device\n");
	else
		serial_printf("\t\tA device\n");
	if (b & MUSB_DEVCTL_FSDEV)
		serial_printf("\t\tFast Device -(host mode)\n");
	if (b & MUSB_DEVCTL_LSDEV)
		serial_printf("\t\tSlow Device -(host mode)\n");
	if (b & MUSB_DEVCTL_HM)
		serial_printf("\t\tHost mode\n");
	else
		serial_printf("\t\tPeripherial mode\n");
	if (b & MUSB_DEVCTL_HR)
		serial_printf("\t\tHost request started(B device)\n");
	else
		serial_printf("\t\tHost request finished(B device)\n");
	if (b & MUSB_DEVCTL_BDEVICE) {
		if (b & MUSB_DEVCTL_SESSION)
			serial_printf("\t\tStart of session(B device)\n");
		else
			serial_printf("\t\tEnd of session(B device)\n");
	} else {
		if (b & MUSB_DEVCTL_SESSION)
			serial_printf("\t\tStart of session(A device)\n");
		else
			serial_printf("\t\tEnd of session(A device)\n");
	}
}

static inline void musb_print_config(u8 b)
{
	serial_printf("\tconfig 0x%2.2x\n", b);
	if (b & MUSB_CONFIGDATA_MPRXE)
		serial_printf("\t\tAuto combine rx bulk packets\n");
	if (b & MUSB_CONFIGDATA_MPTXE)
		serial_printf("\t\tAuto split tx bulk packets\n");
	if (b & MUSB_CONFIGDATA_BIGENDIAN)
		serial_printf("\t\tBig Endian ordering\n");
	else
		serial_printf("\t\tLittle Endian ordering\n");
	if (b & MUSB_CONFIGDATA_HBRXE)
		serial_printf("\t\tHigh speed rx iso endpoint\n");
	if (b & MUSB_CONFIGDATA_HBTXE)
		serial_printf("\t\tHigh speed tx iso endpoint\n");
	if (b & MUSB_CONFIGDATA_DYNFIFO)
		serial_printf("\t\tDynamic fifo sizing\n");
	if (b & MUSB_CONFIGDATA_SOFTCONE)
		serial_printf("\t\tSoft Connect\n");
	if (b & MUSB_CONFIGDATA_UTMIDW)
		serial_printf("\t\t16 bit data width\n");
	else
		serial_printf("\t\t8 bit data width\n");
}

static inline void musb_print_rxmaxp(u16 w)
{
	serial_printf("\trxmaxp  0x%4.4x\n", w);
}

static inline void musb_print_rxcsr(u16 w)
{
	serial_printf("\trxcsr   0x%4.4x\n", w);
	MUSB_FLAGS_PRINT(w, RXCSR, AUTOCLEAR);
	MUSB_FLAGS_PRINT(w, RXCSR, DMAENAB);
	MUSB_FLAGS_PRINT(w, RXCSR, DISNYET);
	MUSB_FLAGS_PRINT(w, RXCSR, PID_ERR);
	MUSB_FLAGS_PRINT(w, RXCSR, DMAMODE);
	MUSB_FLAGS_PRINT(w, RXCSR, CLRDATATOG);
	MUSB_FLAGS_PRINT(w, RXCSR, FLUSHFIFO);
	MUSB_FLAGS_PRINT(w, RXCSR, DATAERROR);
	MUSB_FLAGS_PRINT(w, RXCSR, FIFOFULL);
	MUSB_FLAGS_PRINT(w, RXCSR, RXPKTRDY);
	MUSB_FLAGS_PRINT(w, RXCSR_P, SENTSTALL);
	MUSB_FLAGS_PRINT(w, RXCSR_P, SENDSTALL);
	MUSB_FLAGS_PRINT(w, RXCSR_P, OVERRUN);

	if (w & MUSB_RXCSR_P_ISO)
		serial_printf("\t\tiso mode\n");
	else
		serial_printf("\t\tbulk mode\n");

}

static inline void musb_print_txmaxp(u16 w)
{
	serial_printf("\ttxmaxp  0x%4.4x\n", w);
}

static inline void musb_print_txcsr(u16 w)
{
	serial_printf("\ttxcsr   0x%4.4x\n", w);
	MUSB_FLAGS_PRINT(w, TXCSR, TXPKTRDY);
	MUSB_FLAGS_PRINT(w, TXCSR, FIFONOTEMPTY);
	MUSB_FLAGS_PRINT(w, TXCSR, FLUSHFIFO);
	MUSB_FLAGS_PRINT(w, TXCSR, CLRDATATOG);
	MUSB_FLAGS_PRINT(w, TXCSR_P, UNDERRUN);
	MUSB_FLAGS_PRINT(w, TXCSR_P, SENTSTALL);
	MUSB_FLAGS_PRINT(w, TXCSR_P, SENDSTALL);

	if (w & MUSB_TXCSR_MODE)
		serial_printf("\t\tTX mode\n");
	else
		serial_printf("\t\tRX mode\n");
}

#else

/* stubs */

#define musb_print_pwr(b)
#define musb_print_csr0(w)
#define musb_print_intrusb(b)
#define musb_print_intrtx(w)
#define musb_print_intrrx(w)
#define musb_print_devctl(b)
#define musb_print_config(b)
#define musb_print_rxmaxp(w)
#define musb_print_rxcsr(w)
#define musb_print_txmaxp(w)
#define musb_print_txcsr(w)

#endif /* MUSB_DEBUG */
