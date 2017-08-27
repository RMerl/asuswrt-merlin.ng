/*
 * Broadcom Home Gateway Reference Design
 * Broadcom WiFi Insight Webpage functions
 *
 * Copyright (C) 2016, Broadcom. All Rights Reserved.
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
 * $Id: vis_gui.h 546735 2015-04-06 11:34:36Z $
 */

#ifndef _VIS_GUI_H_
#define _VIS_GUI_H_

extern void vis_do_json_set(const char *url, FILE *stream, int len, const char *boundary);
extern void vis_do_json_get(char *url, FILE *stream);
extern void vis_do_visdbdwnld_cgi(char *url, FILE *stream);

#endif /* _VIS_GUI_H_ */
