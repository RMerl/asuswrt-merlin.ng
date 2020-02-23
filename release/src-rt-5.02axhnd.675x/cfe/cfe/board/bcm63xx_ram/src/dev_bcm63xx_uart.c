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

static void bcm63xx_uart_probe(cfe_driver_t *drv,
                               unsigned long probe_a, unsigned long probe_b,
                               void *probe_ptr);

static int bcm63xx_uart_open(cfe_devctx_t *ctx);
static int bcm63xx_uart_read(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int bcm63xx_uart_inpstat(cfe_devctx_t *ctx,iocb_inpstat_t *inpstat);
static int bcm63xx_uart_write(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int bcm63xx_uart_ioctl(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int bcm63xx_uart_close(cfe_devctx_t *ctx);

const static cfe_devdisp_t bcm63xx_uart_dispatch = {
    bcm63xx_uart_open,
    bcm63xx_uart_read,
    bcm63xx_uart_inpstat,
    bcm63xx_uart_write,
    bcm63xx_uart_ioctl,
    bcm63xx_uart_close,
    NULL,
    NULL
};


const cfe_driver_t bcm63xx_uart = {
    "BCM63xx DUART",
    "uart",
    CFE_DEV_SERIAL,
    &bcm63xx_uart_dispatch,
    bcm63xx_uart_probe
};


typedef struct bcm63xx_uart_s {
	int      baudrate;
} bcm63xx_uart_t;


static void bcm63xx_set_baudrate( bcm63xx_uart_t * softc )
{
#if defined(_BCM947189_)
    UART->data = 0x16;
    UART->imr = 1;
#else
    uint32_t baudwd;

    baudwd = (FPERIPH / softc->baudrate) / 16;
    if( baudwd & 0x1 ) {
        baudwd = baudwd / 2;
    } else {
        baudwd = baudwd / 2 - 1;
    }
    UART->baudword = baudwd;
#endif
}


static void bcm63xx_uart_probe(cfe_driver_t *drv,
                               unsigned long probe_a, unsigned long probe_b,
                               void *probe_ptr)
{
	bcm63xx_uart_t * softc;
	char descr[80];

	/* enable the transmitter interrupt? */

    /*
     * probe_a is the DUART base address.
     * probe_b is the channel-number-within-duart (0 or 1)
     * probe_ptr is unused.
     */
    softc = (bcm63xx_uart_t *) KMALLOC(sizeof(bcm63xx_uart_t),0);
    if (softc) {
		xsprintf( descr, "%s channel %d", drv->drv_description, probe_b );
		cfe_attach( drv, softc, NULL, descr );
	}
}

static int bcm63xx_uart_open(cfe_devctx_t *ctx)
{
	bcm63xx_uart_t * softc = ctx->dev_softc;

#if defined(_BCM947189_)
	softc->baudrate = CFG_SERIAL_BAUD_RATE;
	bcm63xx_set_baudrate( softc );
	/* 8 bits per byte */
	UART->lcr = 3;
	/* DTR | RTS | IENABLE */
	UART->mcr = MCR_DTR | MCR_RTS | MCR_IENABLE;// 0xB;
	/* IER */
	UART->imr = IER_ERXRDY;
	/* Enable FIFO */
	UART->fcr = FIFO_ENABLE;
	/* ENABLE | RCV_RST | XMT_RST | TRIGGER_1 */
	UART->fcr = FIFO_ENABLE | FIFO_RCV_RST | FIFO_XMT_RST | FIFO_TRIGGER_1; //7;
#else
	/* Enable the UART clock */
	softc->baudrate = CFG_SERIAL_BAUD_RATE;
	bcm63xx_set_baudrate( softc );
	UART->control = BRGEN | TXEN | RXEN;
	UART->config = BITS8SYM | ONESTOP;
	UART->fifoctl = RSTTXFIFOS | RSTRXFIFOS;
	UART->intMask  = 0;
#endif
	return 0;
}


static int bcm63xx_uart_read(cfe_devctx_t *ctx,iocb_buffer_t *buffer)
{
	unsigned char * bptr;
	int blen;

#if defined(_BCM947189_)
	bptr = buffer->buf_ptr;
	blen = buffer->buf_length;

	while ((blen > 0) && (UART->lsr & LSR_RXRDY)) {
		*bptr++ = UART->data & 0xFF;
		blen--;
	}
#else
	uint32_t status;
	char inval;

	bptr = buffer->buf_ptr;
	blen = buffer->buf_length;

    while (blen > 0) {
		status = UART->intStatus;
        if(status & (RXOVFERR | RXPARERR | RXFRAMERR | RXBRK)) {
            /* RX over flow */
            if(status & RXOVFERR) {
                /* reset RX FIFO to clr interrupt */
                UART->fifoctl |= RSTRXFIFOS;
            }

            /* other errors just read the bad character to clear the bit */
            inval = UART->Data;
            inval++;  /*dummy increase to supress not used variable warning*/
        }
        else if(status & RXFIFONE) {
			*bptr++ = UART->Data;
			blen--;
        }
        else
            break;
    }
#endif
	buffer->buf_retlen = buffer->buf_length - blen;
	return 0;
}


static int bcm63xx_uart_inpstat(cfe_devctx_t *ctx,iocb_inpstat_t *inpstat)
{
#if defined(_BCM947189_)
	inpstat->inp_status = (UART->lsr & LSR_RXRDY) ? 1 : 0;
#else
	inpstat->inp_status = UART->intStatus & RXFIFONE;
#endif
        return 0;
}


static int bcm63xx_uart_write(cfe_devctx_t *ctx,iocb_buffer_t *buffer)
{
	unsigned char * bptr;
	int blen;

#if defined(_BCM947189_)
	bptr = buffer->buf_ptr;
	blen = buffer->buf_length;

	while((blen > 0) && (UART->lsr & LSR_TXRDY)) {
		UART->data = *bptr;
		bptr++;
		blen--;
	}
#else
	uint32_t status;

	bptr = buffer->buf_ptr;
	blen = buffer->buf_length;

	status = 0;
	while( (blen > 0) && !status ) {
		/* Wait for the buffer to empty before we write the next character */
		/* FIXME - The serial port should be able to accept more than one  */
		/*         character at a time.  Why doesn't it work though?       */
		do {
			status = UART->intStatus & TXFIFOEMT;
		} while( !status );
		UART->Data = *bptr;
		bptr++;
		blen--;

		status  = UART->intStatus & (TXOVFERR|TXUNDERR);
	}

	if( status ) {
		/* Reset TX FIFO */
        UART->fifoctl |= RSTTXFIFOS;
		blen++;
	}
#endif
	buffer->buf_retlen = buffer->buf_length - blen;
	return 0;
}


static int bcm63xx_uart_ioctl(cfe_devctx_t *ctx,iocb_buffer_t *buffer)
{
#if defined(_BCM947189_)
#else
	bcm63xx_uart_t * softc = ctx->dev_softc;
	unsigned int * info = (unsigned int *) buffer->buf_ptr;

    switch ((int)buffer->buf_ioctlcmd) {
	case IOCTL_SERIAL_GETSPEED:
	    *info = softc->baudrate;
	    break;
	case IOCTL_SERIAL_SETSPEED:
	    softc->baudrate = *info;
		bcm63xx_set_baudrate( softc );
	    break;
	case IOCTL_SERIAL_GETFLOW:
    	*info = SERIAL_FLOW_NONE;
	    break;
	case IOCTL_SERIAL_SETFLOW:
	    break;
	default:
	    return -1;
	}
#endif
	return 0;
}


static int bcm63xx_uart_close(cfe_devctx_t *ctx)
{
	/* Turn off the UART clock. */
	return 0;
}
