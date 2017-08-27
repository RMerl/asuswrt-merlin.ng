/*
 * $Id$
 *
 * Passcode verification function (otpd client) for rlm_otp.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 *
 * Copyright 2006,2007 TRI-D Systems, Inc.
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>

#include "extern.h"
#include "otp.h"
#include "otp_pw_valid.h"

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#include <sys/un.h>


/* transform otpd return codes into rlm return codes */
static int otprc2rlmrc(int rc)
{
	switch (rc) {
	case OTP_RC_OK:			return RLM_MODULE_OK;
	case OTP_RC_USER_UNKNOWN:	return RLM_MODULE_REJECT;
	case OTP_RC_AUTHINFO_UNAVAIL:	return RLM_MODULE_REJECT;
	case OTP_RC_AUTH_ERR:		return RLM_MODULE_REJECT;
	case OTP_RC_MAXTRIES:		return RLM_MODULE_USERLOCK;
	case OTP_RC_NEXTPASSCODE:	return RLM_MODULE_USERLOCK;
	case OTP_RC_IPIN:		return RLM_MODULE_REJECT;
	case OTP_RC_SERVICE_ERR:	return RLM_MODULE_FAIL;
	default:			return RLM_MODULE_FAIL;
	}
}

static otp_fd_t *otp_fd_head;
static pthread_mutex_t otp_fd_head_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Test for passcode validity by asking otpd.
 *
 * If challenge is supplied, it is used to generate the card response
 * against which the passcode will be compared.  If challenge is not
 * supplied, or if the comparison fails, synchronous responses are
 * generated and tested.  NOTE: for async authentications, sync mode
 * responses are still considered valid!  (Assuming module configuration
 * allows sync mode.)
 *
 * Returns one of the RLM_MODULE_* codes.  passcode is filled in.
 * NB: The returned passcode will contain the PIN!  DO NOT LOG!
 */
int otp_pw_valid(REQUEST *request, int pwe, char const *challenge,
		 rlm_otp_t const *opt,
		 char passcode[OTP_MAX_PASSCODE_LEN + 1])
{
	otp_request_t	otp_request;
	otp_reply_t	otp_reply;
	VALUE_PAIR	*cvp, *rvp;
	char const	*username = request->username->vp_strvalue;
	int		rc;

	if (request->username->length > OTP_MAX_USERNAME_LEN) {
		AUTH("rlm_otp: username [%s] too long", username);
		return RLM_MODULE_REJECT;
	}

	/* we already know challenge is short enough */
	otp_request.version = 2;

	strcpy(otp_request.username, username);
	strcpy(otp_request.challenge, challenge);

	otp_request.pwe.pwe = pwe;

	/*
	 *	otp_pwe_present() (done by caller) guarantees that both of
	 *	these exist
	 */
	cvp = pairfind(request->packet->vps, pwattr[pwe - 1]->attr,
		       pwattr[pwe - 1]->vendor, TAG_ANY);

	rvp = pairfind(request->packet->vps, pwattr[pwe]->attr,
		       pwattr[pwe]->vendor, TAG_ANY);

	/* this is just to quiet Coverity */
	if (!rvp || !cvp) {
		return RLM_MODULE_REJECT;
	}

	/*
	 *	Validate available vps based on pwe type.
	 *	Unfortunately (?) otpd must do this also.
	 */
	switch (otp_request.pwe.pwe) {
	case PWE_PAP:
		if (rvp->length >= sizeof(otp_request.pwe.u.pap.passcode)) {
			AUTH("rlm_otp: passcode for [%s] too long",
			       username);

			return RLM_MODULE_REJECT;
		}

		(void) strcpy(otp_request.pwe.u.pap.passcode, rvp->vp_strvalue);
		break;

	case PWE_CHAP:
		if (cvp->length > 16) {
			AUTH("rlm_otp: CHAP challenge for [%s] "
			       "too long", username);

			return RLM_MODULE_INVALID;
		}

		if (rvp->length != 17) {
			AUTH("rlm_otp: CHAP response for [%s] "
			      "wrong size", username);

			return RLM_MODULE_INVALID;
		}

		(void) memcpy(otp_request.pwe.u.chap.challenge, cvp->vp_octets,
			      cvp->length);

		otp_request.pwe.u.chap.clen = cvp->length;
		(void) memcpy(otp_request.pwe.u.chap.response, rvp->vp_octets,
			      rvp->length);

		otp_request.pwe.u.chap.rlen = rvp->length;
		break;

	case PWE_MSCHAP:
		if (cvp->length != 8) {
			AUTH("rlm_otp: MS-CHAP challenge for "
			       "[%s] wrong size", username);

			return RLM_MODULE_INVALID;
		}

		if (rvp->length != 50) {
			AUTH("rlm_otp: MS-CHAP response for [%s] "
			       "wrong size", username);

			return RLM_MODULE_INVALID;
		}
		(void) memcpy(otp_request.pwe.u.chap.challenge,
			      cvp->vp_octets, cvp->length);

		otp_request.pwe.u.chap.clen = cvp->length;

		(void) memcpy(otp_request.pwe.u.chap.response,
			      rvp->vp_octets, rvp->length);

		otp_request.pwe.u.chap.rlen = rvp->length;
		break;

	case PWE_MSCHAP2:
		if (cvp->length != 16) {
			AUTH("rlm_otp: MS-CHAP2 challenge for "
				      "[%s] wrong size", username);

			return RLM_MODULE_INVALID;
		}

		if (rvp->length != 50) {
			AUTH("rlm_otp: MS-CHAP2 response for [%s] "
			       "wrong size", username);

			return RLM_MODULE_INVALID;
		}

		(void) memcpy(otp_request.pwe.u.chap.challenge, cvp->vp_octets,
			      cvp->length);

		otp_request.pwe.u.chap.clen = cvp->length;

		(void) memcpy(otp_request.pwe.u.chap.response, rvp->vp_octets,
			      rvp->length);
		otp_request.pwe.u.chap.rlen = rvp->length;
	break;
	} /* switch (otp_request.pwe.pwe) */

	/*
	 *	last byte must also be a terminator so otpd can verify
	 *	length easily.
	 */
	otp_request.username[OTP_MAX_USERNAME_LEN] = '\0';
	otp_request.challenge[OTP_MAX_CHALLENGE_LEN] = '\0';

	if (otp_request.pwe.pwe == PWE_PAP) {
		otp_request.pwe.u.pap.passcode[OTP_MAX_PASSCODE_LEN] = '\0';
	}

	otp_request.allow_sync = opt->allow_sync;
	otp_request.allow_async = opt->allow_async;
	otp_request.challenge_delay = opt->challenge_delay;
	otp_request.resync = 1;


	rc = otp_verify(opt, &otp_request, &otp_reply);
	if (rc == OTP_RC_OK) {
		(void) strcpy(passcode, otp_reply.passcode);
	}

	return otprc2rlmrc(rc);
}

/*
 * Verify an otp by asking otpd.
 * Returns an OTP_* code, or -1 on system failure.
 * Fills in reply.
 */
static int otp_verify(rlm_otp_t const *opt,
		      otp_request_t const *request, otp_reply_t *reply)
{
	otp_fd_t *fdp;
	int rc;
	int tryagain = 2;

	retry:
	if (!tryagain--) {
		return -1;
	}

	fdp = otp_getfd(opt);
	if (!fdp || fdp->fd == -1) {
		return -1;
	}

	rc = otp_write(fdp, (char const *) request, sizeof(*request));
	if (rc != sizeof(*request)) {
		if (rc == 0) {
			goto retry;	/* otpd disconnect */	/*TODO: pause */
		} else {
			return -1;
		}
	}

	rc = otp_read(fdp, (char *) reply, sizeof(*reply));
	if (rc != sizeof(*reply)) {
		if (rc == 0) {
			goto retry;	/* otpd disconnect */	/*TODO: pause */
		} else {
			return -1;
		}
	}

	/* validate the reply */
	if (reply->version != 1) {
		AUTH("rlm_otp: otpd reply for [%s] invalid "
		       "(version %d != 1)", request->username, reply->version);

		otp_putfd(fdp, 1);
		return -1;
	}

	if (reply->passcode[OTP_MAX_PASSCODE_LEN] != '\0') {
		AUTH("rlm_otp: otpd reply for [%s] invalid "
		       "(passcode)", request->username);

		otp_putfd(fdp, 1);
		return -1;
	}

	otp_putfd(fdp, 0);
	return reply->rc;
}

/*
 * Full read with logging, and close on failure.
 * Returns nread on success, 0 on EOF, -1 on other failures.
 */
static int
otp_read(otp_fd_t *fdp, char *buf, size_t len)
{
	ssize_t n;
	size_t nread = 0;	/* bytes read into buf */

	while (nread < len) {
		n = read(fdp->fd, &buf[nread], len - nread);
		if (n == -1) {
			if (errno == EINTR) {
				continue;
			} else {
				ERROR("rlm_otp: %s: read from otpd: %s",
		     		       __func__, strerror(errno));
				otp_putfd(fdp, 1);

				return -1;
			}
		}

		if (!n) {
			ERROR("rlm_otp: %s: otpd disconnect", __func__);
			otp_putfd(fdp, 1);

			return 0;
		}

		nread += n;
	} /* while (more to read) */

	return nread;
}

/*
 *	Full write with logging, and close on failure.
 *	Returns 0 on success, errno on failure.
 */
static int otp_write(otp_fd_t *fdp, char const *buf, size_t len)
{
	size_t nleft = len;
	ssize_t nwrote;

  	while (nleft) {
		nwrote = write(fdp->fd, &buf[len - nleft], nleft);
		if (nwrote == -1) {
			if (errno == EINTR) {
				continue;
			} else {
				ERROR("rlm_otp: %s: write to otpd: %s",
				       __func__, strerror(errno));

				otp_putfd(fdp, 1);
				return errno;

			}
		}

		nleft -= nwrote;
	}

	return 0;
}

/* connect to otpd and return fd */
static int otp_connect(char const *path)
{
	int fd;
	struct sockaddr_un sa;
	size_t sp_len;		/* sun_path length (strlen) */

	/* setup for unix domain socket */
	sp_len = strlen(path);
	if (sp_len > sizeof(sa.sun_path) - 1) {
		ERROR("rlm_otp: %s: rendezvous point name too long",
		       __func__);

		return -1;
	}
	sa.sun_family = AF_UNIX;
	(void) strcpy(sa.sun_path, path);

	/* connect to otpd */
	fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (fd == -1) {
		ERROR("rlm_otp: %s: socket: %s", __func__,
		       strerror(errno));

		return -1;
	}
	if (connect(fd, (struct sockaddr *) &sa,
	      sizeof(sa.sun_family) + sp_len) == -1) {

		ERROR("rlm_otp: %s: connect(%s): %s",
		       __func__, path, strerror(errno));

		(void) close(fd);

		return -1;
	}

	return fd;
}

/*
 * Retrieve an fd (from pool) to use for otpd connection.
 * It'd be simpler to use TLS but FR can have lots of threads
 * and we don't want to waste fd's that way.
 * We can't have a global fd because we'd then be pipelining
 * requests to otpd and we have no way to demultiplex
 * the responses.
 */
static otp_fd_t * otp_getfd(rlm_otp_t const *opt)
{
	int rc;
	otp_fd_t *fdp;

	/* walk the connection pool looking for an available fd */
	for (fdp = otp_fd_head; fdp; fdp = fdp->next) {
		rc = otp_pthread_mutex_trylock(&fdp->mutex);
		if (!rc) {
			if (!strcmp(fdp->path, opt->otpd_rp)) {	/* could just use == */
				break;
			}
		}
	}

	if (!fdp) {
		/* no fd was available, add a new one */
		fdp = rad_malloc(sizeof(*fdp));
		otp_pthread_mutex_init(&fdp->mutex, NULL);
		otp_pthread_mutex_lock(&fdp->mutex);

		/* insert new fd at head */
		otp_pthread_mutex_lock(&otp_fd_head_mutex);
		fdp->next = otp_fd_head;
		otp_fd_head = fdp;
		otp_pthread_mutex_unlock(&otp_fd_head_mutex);

		/* initialize */
		fdp->path = opt->otpd_rp;
		fdp->fd = -1;
	}

	/* establish connection */
	if (fdp->fd == -1) {
		fdp->fd = otp_connect(fdp->path);
	}

	return fdp;
}

/* release fd, and optionally disconnect from otpd */
static void otp_putfd(otp_fd_t *fdp, int disconnect)
{
	if (disconnect) {
		(void) close(fdp->fd);
		fdp->fd = -1;
	}

	/* make connection available to another thread */
	otp_pthread_mutex_unlock(&fdp->mutex);
}
