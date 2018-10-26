/***************************************************************************
 <:copyright-BRCM:2018:DUAL/GPL:standard
 
    Copyright (c) 2018 Broadcom 
    All Rights Reserved
 
 Unless you and Broadcom execute a separate written software license
 agreement governing use of this software, this software is licensed
 to you under the terms of the GNU General Public License version 2
 (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 with the following added to such license:
 
    As a special exception, the copyright holders of this software give
    you permission to link this software with independent modules, and
    to copy and distribute the resulting executable under terms of your
    choice, provided that you also meet, for each linked independent
    module, the terms and conditions of the license of that module.
    An independent module is a module which is not derived from this
    software.  The special exception does not apply to any modifications
    of the software.
 
 Not withstanding the above, under no circumstances may you combine
 this software in any way with any other Broadcom software provided
 under a license other than the GPL, without Broadcom's express prior
 written consent.
 
:> 
 ***************************************************************************/

#include "cfe.h"
#include "lib_types.h"
#include "lib_malloc.h"
#include "lib_printf.h"
#include "cfe_iocb.h"
#include "cfe_device.h"
#include "cfe_ioctl.h"
#include "bsp_config.h"

#include "bcm_hwdefs.h"
#include "bcm_map.h"

static void pl011_uart_probe(cfe_driver_t *drv,
                               unsigned long probe_a, unsigned long probe_b,
                               void *probe_ptr);

static int pl011_uart_open(cfe_devctx_t *ctx);
static int pl011_uart_read(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int pl011_uart_inpstat(cfe_devctx_t *ctx,iocb_inpstat_t *inpstat);
static int pl011_uart_write(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int pl011_uart_ioctl(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int pl011_uart_close(cfe_devctx_t *ctx);

const static cfe_devdisp_t pl011_uart_dispatch = {
    pl011_uart_open,
    pl011_uart_read,
    pl011_uart_inpstat,
    pl011_uart_write,
    pl011_uart_ioctl,
    pl011_uart_close,
    NULL,
    NULL
};


const cfe_driver_t pl011_uart = {
    "ARM pl011 UART",
    "uart",
    CFE_DEV_SERIAL,
    &pl011_uart_dispatch,
    pl011_uart_probe
};


typedef struct pl011_uart_s {
	int      baudrate;
} pl011_uart_t;


static void pl011_set_baudrate( pl011_uart_t * softc )
{
	uint32_t baudwdi, baudwdf, temp;

	/* 
	# (FPERIPH/4) #50Mhz clock for MAINUART CLK
	# Calc ibrd (integer part of the baudrate for 115200
	# Pls. follow Tech. Ref. manual for ARM uart
	# 50Mhz/(115200*16)=27.127
	# IBRD=27 FBRD=8 {integer((0.127*64)+0.5)=>8.628)=>8}
	# Generated divider will be: 27+8/64=27.125; Error estimate: 0.006%
	# IBRD = (FPERIPH/4)/(BAUD*16) = FPERIPH/BAUD/64 = (FPERIPH/BAUD)>>6
	# FBRD = (FPERIPH/BAUD) & 0x3f
	*/

	temp = (FPERIPH / softc->baudrate);
	baudwdi = temp >> 6;
	baudwdf = temp & 0x3f;

	ARM_UART->ibrd = baudwdi;
	ARM_UART->fbrd = baudwdf;
}


static void pl011_uart_probe(cfe_driver_t *drv,
                               unsigned long probe_a, unsigned long probe_b,
                               void *probe_ptr)
{
	pl011_uart_t * softc;
	char descr[80];

	/* enable the transmitter interrupt? */

    /*
     * probe_a is the DUART base address.
     * probe_b is the channel-number-within-duart (0 or 1)
     * probe_ptr is unused.
     */
    softc = (pl011_uart_t *) KMALLOC(sizeof(pl011_uart_t),0);
    if (softc) {
		xsprintf( descr, "%s channel %d", drv->drv_description, probe_b );
		cfe_attach( drv, softc, NULL, descr );
	}
}

static int pl011_uart_open(cfe_devctx_t *ctx)
{
	pl011_uart_t * softc = ctx->dev_softc;

	/* Enable the UART clock */
	softc->baudrate = CFG_SERIAL_BAUD_RATE;
	pl011_set_baudrate( softc );
	ARM_UART->lcr_h = LCR_H_FEN | LCR_H_WLEN_8BIT; // 8 Bits/1 Stop
	ARM_UART->imsc  = 0;
	ARM_UART->cr  = CR_TXE | CR_RXE | CR_EN;

	return 0;
}


static int pl011_uart_read(cfe_devctx_t *ctx,iocb_buffer_t *buffer)
{
	unsigned char * bptr;
	int blen;

	bptr = buffer->buf_ptr;
	blen = buffer->buf_length;

	/* no error checking yet */
	while ((blen > 0) && !(ARM_UART->fr&FR_RXFE)) {
		*bptr++ = (unsigned char)ARM_UART->dr;
		blen--;
	}

	buffer->buf_retlen = buffer->buf_length - blen;
	return 0;
}


static int pl011_uart_inpstat(cfe_devctx_t *ctx,iocb_inpstat_t *inpstat)
{
	inpstat->inp_status = (ARM_UART->fr & FR_RXFE) ? 0 : 1;

        return 0;
}


static int pl011_uart_write(cfe_devctx_t *ctx,iocb_buffer_t *buffer)
{
	unsigned char * bptr;
	int blen;

	bptr = buffer->buf_ptr;
	blen = buffer->buf_length;

	while((blen > 0) && !(ARM_UART->fr & FR_TXFF)) {
		ARM_UART->dr = *bptr;
		bptr++;
		blen--;
	}

	buffer->buf_retlen = buffer->buf_length - blen;
	return 0;
}


static int pl011_uart_ioctl(cfe_devctx_t *ctx,iocb_buffer_t *buffer)
{
	pl011_uart_t * softc = ctx->dev_softc;
	unsigned int * info = (unsigned int *) buffer->buf_ptr;

	switch ((int)buffer->buf_ioctlcmd) {
	case IOCTL_SERIAL_GETSPEED:
	    *info = softc->baudrate;
	    break;
	case IOCTL_SERIAL_SETSPEED:
	    softc->baudrate = *info;
	    pl011_set_baudrate( softc );
	    break;
	case IOCTL_SERIAL_GETFLOW:
	    *info = SERIAL_FLOW_NONE;
	    break;
	case IOCTL_SERIAL_SETFLOW:
	    break;
	default:
	    return -1;
	}

	return 0;
}


static int pl011_uart_close(cfe_devctx_t *ctx)
{
	/* Turn off the UART clock. */
	return 0;
}
