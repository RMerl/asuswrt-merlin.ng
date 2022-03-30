/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB <daniel@omicron.se>.
 */

#ifndef __ASM_IC_ALI512X_H_
#define __ASM_IC_ALI512X_H_

# define ALI_INDEX    0x3f0
# define ALI_DATA     0x3f1

# define ALI_ENABLED  1
# define ALI_DISABLED 0

# define ALI_UART1    0
# define ALI_UART2    1

/* setup functions */
void ali512x_init(void);
void ali512x_set_fdc(int enabled, u16 io, u8 irq, u8 dma_channel);
void ali512x_set_pp(int enabled, u16 io, u8 irq, u8 dma_channel);
void ali512x_set_uart(int enabled, int index, u16 io, u8 irq);
void ali512x_set_rtc(int enabled, u16 io, u8 irq);
void ali512x_set_kbc(int enabled, u8 kbc_irq, u8 mouse_irq);
void ali512x_set_cio(int enabled);


/* common I/O functions */
void ali512x_cio_function(int pin, int special, int inv, int input);
void ali512x_cio_out(int pin, int value);
int ali512x_cio_in(int pin);

/* misc features */
void ali512x_set_uart2_irda(int enabled);

#endif
