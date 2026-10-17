/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _BOARD_CSSI_COMMON_H
#define _BOARD_CSSI_COMMON_H

int read_eeprom(u8 *din, int len);
int ft_board_setup_common(void *blob);
void ft_board_setup_phy3(void);
int checkboard_common(void);
void misc_init_r_common(void);
void iop_setup_common(void);
void iop_setup_mcr(void);
void iop_setup_miae(void);

#endif /* _BOARD_CSSI_COMMON_H */
