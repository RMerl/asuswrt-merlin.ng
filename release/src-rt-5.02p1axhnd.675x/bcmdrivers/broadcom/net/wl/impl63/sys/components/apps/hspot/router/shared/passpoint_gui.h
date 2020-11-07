/*
 * Broadcom Home Gateway Reference Design
 * Broadcom Passpoint Webpage functions
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: passpoint_gui.h 490618 2014-07-11 11:23:19Z $
 */

#ifndef _PASSPOINT_GUI_H_
#define _PASSPOINT_GUI_H_

/* ====================== MIME HANDLER FUNCTIONS =========================== */
extern void do_uploadIcons_post(char *url, FILE *stream, int len, char *boundary);
extern void do_uploadIcons_cgi(char *url, FILE *stream);
extern void do_passpoint_asp(char *url, FILE *stream);
/* ====================== MIME HANDLER FUNCTIONS =========================== */

/* ================== WEBUI CONTROL ONCHANGE FUNCTIONS ===================== */
extern int ej_authid_change(int eid, webs_t wp, int argc, char_t** argv);
extern int ej_vanuegrp_change(int eid, webs_t wp, int argc, char_t** argv);
extern int ej_icon_change(int eid, webs_t wp, int argc, char_t **argv);
/* ================== WEBUI CONTROL ONCHANGE FUNCTIONS ===================== */

/* ======================= POPUP PRINT FUNCTIONS =========================== */
extern int ej_print_popup_realm(int eid, webs_t wp, int argc, char **argv);
extern int ej_print_popup_osup(int eid, webs_t wp, int argc, char_t **argv);
/* ======================= POPUP PRINT FUNCTIONS =========================== */

/* ========================== PRINT FUNCTIONS ============================== */
extern int ej_print_wl_netauthlist(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_print_wl_realmlist(int eid, webs_t wp, int argc, char **argv);
extern int ej_print_wl_venuegrp_type(int eid, webs_t wp, int argc, char_t** argv);
extern int ej_print_wl_venuelist(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_print_wl_ouilist(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_print_wl_3gpplist(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_print_wl_qosmapie(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_print_wl_wanmetrics(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_print_wl_oplist(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_print_wl_homeqlist(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_print_wl_concaplist(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_print_wl_osuplist(int eid, webs_t wp, int argc, char_t **argv);
extern int ej_print_iconlist(int eid, webs_t wp, int argc, char_t **argv);
/* ========================== PRINT FUNCTIONS ============================== */

/* ======================== VALIDATE FUNCTIONS ============================= */
extern void validate_wl_hsflag(webs_t wp, char *value, struct variable *v, char *varname);
extern void validate_wl_netauthlist(webs_t wp, char *value, struct variable *v, char *varname);
extern void validate_wl_realmlist(webs_t wp, char *value, struct variable *v, char *varname);
extern void validate_wl_venuelist(webs_t wp, char *value, struct variable *v, char *varname);
extern void validate_wl_ouilist(webs_t wp, char *value, struct variable *v, char *varname);
extern void validate_wl_3gpplist(webs_t wp, char *value, struct variable *v, char *varname);
extern void validate_wl_qosmapie(webs_t wp, char *value, struct variable *v, char *varname);
extern void validate_wl_wanmetrics(webs_t wp, char *value, struct variable *v, char *varname);
extern void validate_wl_oplist(webs_t wp, char *value, struct variable *v, char *varname);
extern void validate_wl_homeqlist(webs_t wp, char *value, struct variable *v, char *varname);
extern void validate_wl_concaplist(webs_t wp, char *value, struct variable *v, char *varname);
extern void validate_wl_osuplist(webs_t wp, char *value, struct variable *v, char *varname);
/* ======================== VALIDATE FUNCTIONS ============================= */

#endif /* _PASSPOINT_GUI_H_ */
