/*
 * Broadcom 802.11abg Networking Device Driver
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: wl_ate.c kshaha $
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */

#include "wl_rte.h"
#include "wl_ate.h"
#include <wlc_phy_hal.h>
#include <sbchipc.h>

/* Macros */
#define ATE_CMD_STR_LEN_MAX	100
#define WLC_ATE_CMD_PARAMS_MAX 20
#define ATE_CMDS_NUM_MAX 768

/* External references referred in this file */
extern hnd_dev_t *dev_list;

/* Locally used variables */
static const char rstr_ate_cmd_def_chan[] = "ate_def_chan";
static const char rstr_ate_cmd_gpio_ip[] = "ate_gpio_ip";
static const char rstr_ate_cmd_gpio_op[] = "ate_gpio_op";
static uint32 ate_buffer_regval_size = 0;

/* ATE global data structure */
ate_params_t ate_params;
char ate_buffer_sc[ATE_SAMPLE_COLLECT_BUFFER_SIZE] __attribute__ ((aligned (32)));
uint32 ate_buffer_sc_size = 0;
ate_buffer_regval_t ate_buffer_regval; /* Buffer for storing various register values */
uint16 otp_regval[ATE_OTP_REGVAL_SIZE];

/* Locally used ATE functions */
static int wl_ate_cmd_preproc(char **argv, char *cmd_str);
static bool wlc_ate_capable_chip(uint32 devid);
static void wl_ate_sc_init(wl_info_t *wl, hnd_dev_t *dev);
static void wl_ate_gpio_init(wlc_info_t *wlc, si_t *sih, hnd_dev_t *dev);

static void
wl_ate_sc_init(wl_info_t *wl, hnd_dev_t *dev)
{
	/* Initialize the sample buffer */
	bzero(ate_buffer_sc, ATE_SAMPLE_COLLECT_BUFFER_SIZE);
	printf("ATE: Sample Collect Buffer Init Done\n");
}

static void
wl_ate_gpio_init(wlc_info_t *wlc, si_t *sih, hnd_dev_t *dev)
{
	/* Configure the GPIOs IN */
	if (getvar(wlc->pub->vars, rstr_ate_cmd_gpio_ip)) {
		ate_params.gpio_input = (uint8)getintvar(NULL, rstr_ate_cmd_gpio_ip);

#if ATE_DEBUG
		void *handle;

		/* Take over gpio control from cc */
		si_gpiocontrol(sih, (1 << ate_params.gpio_input),
			(1 << ate_params.gpio_input), GPIO_DRV_PRIORITY);

		/* GCI related GPIO programming */
		si_ccreg(sih, CC_GCI_INDIRECT_ADDR_REG, ~0, 0); /* wl ccreg  0 */
		si_ccreg(sih, CC_GCI_CHIP_CTRL_REG, ~0, 0x11111111); /* wl ccreg 0xe00 0x11111111 */
		si_ccreg(sih, CC_GCI_INDIRECT_ADDR_REG, ~0, 1); /* wl ccreg 0xc40 1 */
		si_ccreg(sih, CC_GCI_CHIP_CTRL_REG, ~0, 0x11111111); /* wl ccreg 0xe00 0x11111111 */
		si_ccreg(sih, CC_GCI_INDIRECT_ADDR_REG, ~0, 6); /* wl ccreg 0xc40 6 */
		si_ccreg(sih, CC_GCI_CHIP_CTRL_REG, ~0, 0x41b3); /* wl ccreg 0xe00 0x41b3 */

		/* Disable the PHY GPIOs */
		wlc_phy_gpiosel_disable(WLC_PI(wlc));

		/* Register the GPIO interrupt handler (FALSE = edge-detect). */
		handle = si_gpio_handler_register(sih, (1 << ate_params.gpio_input), FALSE,
			wl_ate_gpio_interrupt, (void *) &ate_params);

		/* make polarity opposite of the current value */
		si_gpiointpolarity(sih, (1 << ate_params.gpio_input),
			(si_gpioin(sih) & (1 << ate_params.gpio_input)), 0);
#endif /* ATE_DEBUG */

		/* Enable the GPIO INT specified */
		si_gpiointmask(sih, (1 << ate_params.gpio_input),
			(1 << ate_params.gpio_input), 0);

		printf("ATE: GPIO I/P %d init done\n", ate_params.gpio_input);
	}

	/* Configure the GPIO OUT */
	if (getvar(wlc->pub->vars, rstr_ate_cmd_gpio_op)) {
		ate_params.gpio_output = (uint8)getintvar(NULL, rstr_ate_cmd_gpio_op);

#if ATE_DEBUG
		/* GCI related GPIO programming */
		si_ccreg(sih, CC_GCI_INDIRECT_ADDR_REG, ~0, 0);
		si_ccreg(sih, CC_GCI_CHIP_CTRL_REG, ~0, 0x11111111);
		si_ccreg(sih, CC_GCI_INDIRECT_ADDR_REG, ~0, 1);
		si_ccreg(sih, CC_GCI_CHIP_CTRL_REG, ~0, 0x11111111);
		si_ccreg(sih, CC_GCI_INDIRECT_ADDR_REG, ~0, 6);
		si_ccreg(sih, CC_GCI_CHIP_CTRL_REG, ~0, 0x41b3);

		/* Disable the PHY GPIOs */
		wlc_ate_gpiosel_disable(WLC_PI(wlc));

		/* Take over gpio control from cc and enable it for output */
		si_gpiocontrol(sih, (1 << ate_params.gpio_output), 0, GPIO_DRV_PRIORITY);
#endif	/* ATE_DEBUG */

		/* Enable the GPIO OUTPUT specified */
		si_gpioouten(sih, 1 << ate_params.gpio_output,
			1 << ate_params.gpio_output, GPIO_HI_PRIORITY);

#if ATE_DEBUG
		/* Usage of GPIO output */
		/* Writing to the GPIO now */
		si_gpioout(sih, 1 << ate_params.gpio_output,
			1 << ate_params.gpio_output, GPIO_HI_PRIORITY);
#endif /* ATE_DEBUG */

		printf("ATE: GPIO O/P %d init done\n", ate_params.gpio_output);
	}

	return;
}

static int
wl_ate_cmd_preproc(char **argv, char *cmd_str)
{
	int param_count;
	char *array_ptr = cmd_str;

	ASSERT(strlen(cmd_str) <= ATE_CMD_STR_LEN_MAX);

	for (param_count = 0; param_count < WLC_ATE_CMD_PARAMS_MAX; param_count++)
		argv[param_count] = 0;

	for (param_count = 0; *array_ptr != '\0'; param_count++) {
		argv[param_count] = array_ptr;
		for (; (*array_ptr != '\0') && (*array_ptr != ' '); array_ptr++);
			if (*array_ptr == '\0')
				break;
			*array_ptr = '\0';
			array_ptr++;
	}

	return param_count + 1;
}

static bool
wlc_ate_capable_chip(uint32 devid)
{
	bool retval = FALSE;
	switch (devid) {
		case BCM4336_D11N_ID:
		case BCM4330_D11N_ID:
		case BCM4330_D11N2G_ID:
		case BCM4334_D11N_ID:
		case BCM4334_D11N2G_ID:
		case BCM4334_D11N5G_ID:
		case BCM4345_D11AC2G_ID:
		case BCM4345_D11AC5G_ID:
		case BCM4345_D11AC_ID:
		case BCM4349_D11AC_ID:
		case BCM4365_D11AC5G_ID:
		case BCM4365_D11AC_ID:
		case BCM4365_D11AC2G_ID:
		case BCM4366_D11AC5G_ID:
		case BCM4366_D11AC_ID:
		case BCM4366_D11AC2G_ID:
			retval = TRUE;
			break;
		default:
			retval = FALSE;
	}
	return retval;
}

void
wl_ate_cmd_proc(void)
{
	char *ate_str = NULL;
	char *argv[WLC_ATE_CMD_PARAMS_MAX];
	uint8 argc = 0;
	char ate_cmd_str[10] = "ate_cmd";
	uint8 ate_cmd_str_len = strlen(ate_cmd_str);
	char ate_cmd_num[3];
	wl_info_t *wl = NULL;
	wlc_info_t *wlc = NULL;
	uint32 gpioout, gpioinput;
	uint32 mva;

	if ((ate_params.ate_cmd_done == TRUE) ||	/* All commands executed */
		(ate_params.cmd_proceed == FALSE))		/* Waiting for GPIO trigger */
		return;

	ASSERT(ate_params.wl);
	wl = ate_params.wl;
	wlc = wl->wlc;

	if (ate_params.cmd_idx == 0) {
		printf("\nATE CMD : START!!!\n");
		/* Be prepared for a Wait for INT ATE command */
		ate_params.cmd_proceed = FALSE;
	}

	do {
		sprintf(ate_cmd_num, "%02X", ate_params.cmd_idx);
		ate_cmd_str[ate_cmd_str_len] = '\0';
		strcat(ate_cmd_str, ate_cmd_num);
		ate_str = getvar(wlc->pub->vars, ate_cmd_str);

		if (ate_str) {
			printf("ATE CMD%02X: %s : ", ate_params.cmd_idx, ate_str);
			argc = wl_ate_cmd_preproc(argv, ate_str);

			if (strcmp(argv[0], "ate_cmd_wait_gpio_rising_edge") == 0) {
				/* Execute the ATE command */
				if (ate_params.cmd_proceed == TRUE) {
					ate_params.cmd_proceed = FALSE;
					/* Proceed with the next ATE command */
					ate_params.cmd_idx++;
					printf("\n");
					continue;
				} else {
					printf("intmask 0x%08x\n",
						si_gpio_intmask_enable(ate_params.sih));
					return;
				}
			} else if (strcmp(argv[0], "ate_cmd_write_gpio") == 0) {
				/* Write to the assigned GPIO */
				si_gpioout(ate_params.sih, 1 << ate_params.gpio_output,
					(strcmp(argv[1], "0")) << ate_params.gpio_output,
					GPIO_HI_PRIORITY);

				/* Proceed with the next ATE command */
				ate_params.cmd_idx++;
				continue;
			} else if (strcmp(argv[0], "ate_cmd_enable_gpio") == 0) {
				wl_ate_gpio_init(wlc, ate_params.sih, NULL);
				printf("intmask 0x%08x\n", si_gpio_intmask_enable(ate_params.sih));
				/* Proceed with the next ATE command */
				ate_params.cmd_idx++;
				continue;
			} else if (strcmp(argv[0], "ate_cmd_disable_gpio") == 0) {
				gpioout = si_gpioouten(ate_params.sih, 1 << ate_params.gpio_output,
						0, GPIO_HI_PRIORITY);
				gpioinput = si_gpiointmask(ate_params.sih,
						(1 << ate_params.gpio_input), 0, 0);
				printf("gpioout = 0x%08x gpioinput = 0x%08x\n", gpioout, gpioinput);
				/* Proceed with the next ATE command */
				ate_params.cmd_idx++;
				continue;
			} else if (strcmp(argv[0], "ate_cmd_flush_dcache_all") == 0) {
				cpu_flush_cache_all();
				printf("cpu flush all dcache\n");
				/* Proceed with the next ATE command */
				ate_params.cmd_idx++;
				continue;
			} else if (strcmp(argv[0], "ate_cmd_flush_dcache_scbuf") == 0) {
				/* flush cache (clean and invalidation) by MVA to PoC */
				for (mva = (uint32)ate_buffer_sc;
					mva < (uint32)ate_buffer_sc + ate_buffer_sc_size;
					mva = mva + 64) {
					asm volatile("mcr p15, 0, %0, c7, c14, 1" : : "r" (mva));
				}
				mva = (uint32)&ate_buffer_sc_size;
				asm volatile("mcr p15, 0, %0, c7, c14, 1" : : "r" (mva));
				printf("cpu flush dcache for ate_buffer_sc\n");
				/* Proceed with the next ATE command */
				ate_params.cmd_idx++;
				continue;
			} else if (strcmp(argv[0], "ate_cmd_flush_dcache_regval") == 0) {
				for (mva = (uint32)&ate_buffer_regval;
					mva < (uint32)&ate_buffer_regval + ate_buffer_regval_size;
					mva = mva + 64) {
					asm volatile("mcr p15, 0, %0, c7, c14, 1" : : "r" (mva));
				}
				mva = (uint32)&ate_buffer_regval_size;
				asm volatile("mcr p15, 0, %0, c7, c14, 1" : : "r" (mva));
				printf("cpu flush dcache for regval\n");
				/* Proceed with the next ATE command */
				ate_params.cmd_idx++;
				continue;
			} else {
				if (argc > 1) {
					/* Execute the ATE wl command */
					do_wl_cmd((uint32)wlc->wl, argc, argv);
				} else {
					printf("ATE Command: Invalid command : %s. "
						"Num of params : %d\n", ate_str, argc);
				}
				ate_params.cmd_idx++;
			}
		} else {
			ate_params.cmd_idx++;
		}
	} while (ate_params.cmd_idx < ATE_CMDS_NUM_MAX);

	printf("ATE CMD : END!!!\n");

	/* All ATE commands done, update the variables accordingly */
	ate_params.cmd_idx = 0;
	ate_params.ate_cmd_done = TRUE;

	return;
}

void
wl_ate_init(si_t *sih, hnd_dev_t *bcmwl)
{
	hnd_dev_t *dev = bcmwl;
	wl_info_t *wl = NULL;
	wlc_info_t *wlc = NULL;

	/* Validate chip - ATE commands supported for some chips only */
	while (dev) {
		if (wlc_ate_capable_chip(dev->devid))
			break;
		dev = dev->next;
	}
	if (!dev) {
		printf("ATE: This chip is NOT supported for ATE operations!!!\n");
		ASSERT(FALSE);
		return;
	}
	wl = ate_params.wl = dev->softc;
	wlc = wl->wlc;
	ate_params.sih = sih;

	/* Init ATE params */
	ate_params.cmd_proceed = TRUE;
	ate_params.ate_cmd_done = FALSE;
	ate_params.cmd_idx = 0;
	ate_params.gpio_input = 0xFF;
	ate_params.gpio_output = 0xFF;

	/* Init the GPIOs, if needed */
	wl_ate_gpio_init(wlc, sih, dev);

	/* Configure the default channel */
	if (getvar(wlc->pub->vars, rstr_ate_cmd_def_chan)) {
		wlc->default_bss->chanspec =
			CH20MHZ_CHSPEC((uint8)getintvar(NULL, rstr_ate_cmd_def_chan));
	}

	/* Init the sample collect related params */
	wl_ate_sc_init(wl, dev);

	/* Initialize the Regval buffer related params */
	bzero((char *) &ate_buffer_regval, sizeof(ate_buffer_regval_t));
	ate_buffer_regval_size = sizeof(ate_buffer_regval_t);
	bzero((char *)otp_regval, ATE_OTP_REGVAL_SIZE * sizeof(uint16));
	/* Done with init */
	printf("ATE: Init done\n");
}
