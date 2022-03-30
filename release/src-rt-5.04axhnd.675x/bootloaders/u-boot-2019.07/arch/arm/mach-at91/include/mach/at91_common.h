/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 */

#ifndef AT91_COMMON_H
#define AT91_COMMON_H

void at91_can_hw_init(void);
void at91_gmac_hw_init(void);
void at91_macb_hw_init(void);
void at91_mci_hw_init(void);
void at91_serial0_hw_init(void);
void at91_serial1_hw_init(void);
void at91_serial2_hw_init(void);
void at91_seriald_hw_init(void);
void at91_spi0_hw_init(unsigned long cs_mask);
void at91_spi1_hw_init(unsigned long cs_mask);
void at91_udp_hw_init(void);
void at91_uhp_hw_init(void);
void at91_lcd_hw_init(void);
void at91_plla_init(u32 pllar);
void at91_pllb_init(u32 pllar);
void at91_mck_init(u32 mckr);
void at91_mck_init_down(u32 mckr);
void at91_pmc_init(void);
void mem_init(void);
void at91_phy_reset(void);
void at91_sdram_hw_init(void);
void at91_mck_init(u32 mckr);
void at91_spl_board_init(void);
void at91_disable_wdt(void);
void matrix_init(void);
void redirect_int_from_saic_to_aic(void);
void configure_2nd_sram_as_l2_cache(void);

int at91_set_ethaddr(int offset);
int at91_video_show_board_info(void);

#endif /* AT91_COMMON_H */
