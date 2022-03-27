/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2003-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

enum {					/**** Backend exit codes ****/
	CUPS_BACKEND_OK = 0,		/* Job completed successfully */
	CUPS_BACKEND_FAILED = 1,	/* Job failed, use error-policy */
	CUPS_BACKEND_AUTH_REQUIRED = 2,	/* Job failed, authentication required */
	CUPS_BACKEND_HOLD = 3,		/* Job failed, hold job */
	CUPS_BACKEND_STOP = 4,		/* Job failed, stop queue */
	CUPS_BACKEND_CANCEL = 5,	/* Job failed, cancel job */
	CUPS_BACKEND_RETRY = 6,		/* Failure requires us to retry (BlueZ specific) */
};

int sdp_search_spp(sdp_session_t *sdp, uint8_t *channel);
int sdp_search_hcrp(sdp_session_t *sdp, unsigned short *ctrl_psm, unsigned short *data_psm);

int spp_print(bdaddr_t *src, bdaddr_t *dst, uint8_t channel, int fd, int copies, const char *cups_class);
int hcrp_print(bdaddr_t *src, bdaddr_t *dst, unsigned short ctrl_psm, unsigned short data_psm, int fd, int copies, const char *cups_class);
