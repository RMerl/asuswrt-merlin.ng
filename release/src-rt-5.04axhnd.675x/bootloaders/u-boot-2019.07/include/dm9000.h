/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * NOTE:        DAVICOM DM9000 ethernet driver interface
 *
 * Authors:     Remy Bohmer <linux@bohmer.net>
 */
#ifndef __DM9000_H__
#define __DM9000_H__

/******************  function prototypes **********************/
#if !defined(CONFIG_DM9000_NO_SROM)
void dm9000_write_srom_word(int offset, u16 val);
void dm9000_read_srom_word(int offset, u8 *to);
#endif

#endif /* __DM9000_H__ */
