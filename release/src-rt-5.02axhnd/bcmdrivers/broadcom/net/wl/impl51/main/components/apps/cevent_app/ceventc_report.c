/*
 * cevent CLI reporting
 *
 *
 * Copyright (C) 2018, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 *
 * $Id:$
 */

#include "ceventc.h"

int
ca_cli_dump_local(ca_cli_t *ctx)
{
	const char * nv_out_file_path = nvram_safe_get("ceventd_out");
	char sys_cmd[256] = "";
	char grep_iface[64] = "";
	char grep_mac[64] = "";

	if (nv_out_file_path && nv_out_file_path[0]) {
		const int pathLen = strnlen(nv_out_file_path, CA_FILE_PATH_LEN);
		if (pathLen > 0 && pathLen < (CA_FILE_PATH_LEN - 4)) { /* -4 to accommodate .bak */
			strncpy(ctx->out_path, nv_out_file_path, CA_FILE_PATH_LEN);
			sprintf(ctx->out_bak_path, "%s.bak", ctx->out_path);

			// snprintf(sys_cmd, sizeof(sys_cmd), "cat %s %s 2>/dev/null",
			//		ctx->out_path, ctx->out_bak_path);
			if (ctx->iface[0]) {
				snprintf(grep_iface, sizeof(grep_iface),
						"grep -i -e %s |", ctx->iface);
			}
			if (ctx->mac[0]) {
				snprintf(grep_mac, sizeof(grep_mac), "grep -i -e %s |", ctx->mac);
			}
			printf("TIME(ms)  TDiff IFace STA_MAC_ADDRESS   "
					"Flags Stts Rsn  Auth AT    \tDir \tEVENT\n");
			snprintf(sys_cmd, sizeof(sys_cmd),
				"bp=0; grep -v TIME.ms %s %s 2>/dev/null | %s %s "
				"while read -r a b c d e f g h i j k rest; do bd=`expr $b - $bp`; "
				"echo -e \"$b  $bd\\t$c  $d $e $f $g $h $i \\t$j \t$k\"; "
				"bp=$b; done",
				ctx->out_path, ctx->out_bak_path, grep_iface, grep_mac);
			system(sys_cmd);
		}
	}

	return BCME_OK;
}
