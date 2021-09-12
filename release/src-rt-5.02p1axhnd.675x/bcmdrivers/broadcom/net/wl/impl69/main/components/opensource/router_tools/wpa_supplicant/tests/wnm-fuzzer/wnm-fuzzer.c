/*
 * wpa_supplicant - WNM fuzzer
 * Copyright (c) 2015-2019, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "utils/includes.h"

#include "utils/common.h"
#include "utils/eloop.h"
#include "common/ieee802_11_defs.h"
#include "rsn_supp/wpa.h"
#include "rsn_supp/wpa_i.h"
#include "../../wpa_supplicant/wpa_supplicant_i.h"
#include "../../wpa_supplicant/bss.h"
#include "../../wpa_supplicant/wnm_sta.h"
#include "../../wpa_supplicant/config.h"

struct arg_ctx {
	const char *fname;
	struct wpa_supplicant wpa_s;
	struct wpa_bss bss;
	struct wpa_driver_ops driver;
	struct wpa_sm wpa;
	struct wpa_config conf;
};

static void test_send_wnm(void *eloop_data, void *user_ctx)
{
	struct arg_ctx *ctx = eloop_data;
	char *data;
	size_t len;
	struct ieee80211_mgmt *mgmt;

	wpa_printf(MSG_INFO, "wnm-fuzzer: Send '%s'", ctx->fname);

	data = os_readfile(ctx->fname, &len);
	if (!data) {
		wpa_printf(MSG_ERROR, "Could not read '%s'", ctx->fname);
		goto out;
	}

	wpa_hexdump(MSG_MSGDUMP, "fuzzer - WNM", data, len);

	mgmt = (struct ieee80211_mgmt *) data;
	ieee802_11_rx_wnm_action(&ctx->wpa_s, mgmt, len);

out:
	os_free(data);
	eloop_terminate();
}

static int init_wpa(struct arg_ctx *ctx)
{
	ctx->wpa_s.wpa_state = WPA_COMPLETED;
	os_memcpy(ctx->wpa_s.bssid, "\x02\x00\x00\x00\x03\x00", ETH_ALEN);
	ctx->wpa_s.current_bss = &ctx->bss;
	ctx->wpa_s.driver = &ctx->driver;
	ctx->wpa_s.wpa = &ctx->wpa;
	ctx->wpa_s.conf = &ctx->conf;

	return 0;
}

static void deinit_wpa(struct arg_ctx *ctx)
{
	wnm_deallocate_memory(&ctx->wpa_s);
}

int main(int argc, char *argv[])
{
	struct arg_ctx ctx;
	int ret = -1;

	if (argc < 2) {
		printf("usage: %s <file>\n", argv[0]);
		return -1;
	}

	if (os_program_init())
		return -1;

	wpa_debug_level = 0;
	wpa_debug_show_keys = 1;

	if (eloop_init()) {
		wpa_printf(MSG_ERROR, "Failed to initialize event loop");
		return -1;
	}

	os_memset(&ctx, 0, sizeof(ctx));
	ctx.fname = argv[1];
	if (init_wpa(&ctx))
		goto fail;

	eloop_register_timeout(0, 0, test_send_wnm, &ctx, NULL);

	wpa_printf(MSG_DEBUG, "Starting eloop");
	eloop_run();
	wpa_printf(MSG_DEBUG, "eloop done");
	deinit_wpa(&ctx);

	ret = 0;
fail:
	eloop_destroy();
	os_program_deinit();

	return ret;
}
