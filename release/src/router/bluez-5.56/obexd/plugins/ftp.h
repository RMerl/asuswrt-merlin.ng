/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

void *ftp_connect(struct obex_session *os, int *err);
int ftp_get(struct obex_session *os, void *user_data);
int ftp_chkput(struct obex_session *os, void *user_data);
int ftp_put(struct obex_session *os, void *user_data);
int ftp_setpath(struct obex_session *os, void *user_data);
void ftp_disconnect(struct obex_session *os, void *user_data);
int ftp_action(struct obex_session *os, void *user_data);
