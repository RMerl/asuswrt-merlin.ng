/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * brxre1.h
 *
 * specific parts for B&R KWB Motherboard
 *
 * Copyright (C) 2013 Hannes Schmelzer <oe5hpm@oevsv.at> -
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 */

#ifndef __CONFIG_BRXRE1_H__
#define __CONFIG_BRXRE1_H__

#include <configs/bur_cfg_common.h>
#include <configs/bur_am335x_common.h>
#include <linux/stringify.h>

/* Clock Defines */
#define V_OSCK				26000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

/* Default environment */
#define CFG_EXTRA_ENV_SETTINGS	\
BUR_COMMON_ENV \
"scradr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
"bootaddr=0x80001100\0" \
"bootdev=cpsw(0,0)\0" \
"vx_romfsbase=0x800E0000\0" \
"vx_romfssize=0x20000\0" \
"vx_memtop=0x8FBEF000\0" \
"loadromfs=mmc read ${vx_romfsbase} 700 100\0" \
"loadaddr=0x80100000\0" \
"startvx=run loadromfs; bootvx ${loadaddr}\0" \
"b_break=0\0" \
"b_tgts_std=mmc def net usb0\0" \
"b_tgts_rcy=def net usb0\0" \
"b_tgts_pme=net usb0 mmc\0" \
"b_deftgts=if test ${b_mode} = 12; then setenv b_tgts ${b_tgts_pme};" \
" elif test ${b_mode} = 0; then setenv b_tgts ${b_tgts_rcy};" \
" else setenv b_tgts ${b_tgts_std}; fi\0" \
"b_mmc=load mmc 1 ${loadaddr} arimg && run startvx\0" \
"b_def=mmc read ${loadaddr} 800 8000; run startvx\0" \
"b_net=tftp ${scradr} netscript.img && source ${scradr}\0" \
"b_usb0=usb start && load usb 0 ${scradr} usbscript.img && source ${scradr}\0" \
"b_default=run b_deftgts; for target in ${b_tgts};"\
" do run b_${target}; if test ${b_break} = 1; then; exit; fi; done\0"

/* Environment */

#endif	/* __CONFIG_BRXRE1_H__ */
