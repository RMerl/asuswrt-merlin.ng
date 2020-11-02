/*
 * random.c
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
 * $Id: random.c 470764 2014-04-16 08:40:23Z $
 */
#include <stdio.h>
#if defined(__linux__)
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <fcntl.h>
#include <linux/if_packet.h>
#elif defined(TARGETOS_nucleus)
#include <stdlib.h>
#elif WIN32
#include <stdio.h>
#include <windows.h>
#include <Wincrypt.h>
#endif /* __linux__ */

#include <assert.h>
#include <typedefs.h>
#include <bcmcrypto/bn.h>

#if defined(__linux__)
void linux_random(uint8 *rand, int len);
#elif WIN32
void windows_random(uint8 *rand, int len);
#elif defined(TARGETOS_nucleus)
void generic_random(uint8* rand, int len);
#elif defined(TARGETOS_symbian)
void generic_random(uint8* rand, int len);
#endif /* __linux__ */

void RAND_bytes(unsigned char *buf, int num)
{
#if defined(__linux__)
	linux_random(buf, num);
#elif WIN32
	windows_random(buf, num);
#elif defined(TARGETOS_nucleus)
	generic_random(buf, num);
#elif defined(TARGETOS_symbian)
	generic_random(buf, num);
#endif /* __linux__ */
}

#if defined(__linux__)
void RAND_linux_init()
{
	BN_register_RAND(linux_random);
}

#ifndef	RANDOM_READ_TRY_MAX
#define RANDOM_READ_TRY_MAX	10
#endif // endif

void linux_random(uint8 *rand, int len)
{
	static int dev_random_fd = -1;
	int status;
	int i;

	if (dev_random_fd == -1)
		dev_random_fd = open("/dev/urandom", O_RDONLY|O_NONBLOCK);

	assert(dev_random_fd != -1);

	for (i = 0; i < RANDOM_READ_TRY_MAX; i++) {
		status = read(dev_random_fd, rand, len);
		if (status == -1) {
			if (errno == EINTR)
				continue;

			assert(status != -1);
		}

		return;
	}

	assert(i != RANDOM_READ_TRY_MAX);
}
#elif WIN32
void RAND_windows_init()
{
	BN_register_RAND(windows_random);
}

void windows_random(uint8 *rand, int len)
{
	/* Declare and initialize variables */

	HCRYPTPROV hCryptProv = NULL;
	LPCSTR UserName = "{56E9D11F-76B8-42fa-8645-76980E4E8648}";

	/*
	Attempt to acquire a context and a key
	container. The context will use the default CSP
	for the RSA_FULL provider type. DwFlags is set to 0
	to attempt to open an existing key container.
	*/
	if (CryptAcquireContext(&hCryptProv,
		UserName,
		NULL,
		PROV_RSA_FULL,
		0))
	{
		/* do nothing */
	}
	else
	{
		/*
		An error occurred in acquiring the context. This could mean
		that the key container requested does not exist. In this case,
		the function can be called again to attempt to create a new key
		container. Error codes are defined in winerror.h.
		*/
		if (GetLastError() == NTE_BAD_KEYSET)
		{
			if (!CryptAcquireContext(&hCryptProv,
				UserName,
				NULL,
				PROV_RSA_FULL,
				CRYPT_NEWKEYSET))
			{
				printf("Could not create a new key container.\n");
			}
		}
		else
		{
			printf("A cryptographic service handle could not be acquired.\n");
		}
	}

	if (hCryptProv)
	{
		/* Generate a random initialization vector. */
		if (!CryptGenRandom(hCryptProv, len, rand))
		{
			printf("Error during CryptGenRandom.\n");
		}
		if (!CryptReleaseContext(hCryptProv, 0))
			printf("Failed CryptReleaseContext\n");
	}
	return;
}
#elif TARGETOS_nucleus
void RAND_generic_init()
{
	BN_register_RAND(generic_random);
}
#elif TARGETOS_symbian
void RAND_generic_init()
{
	BN_register_RAND(generic_random);
}
#endif /* __linux__ */

#if (defined(TARGETOS_nucleus) || defined(TARGETOS_symbian))
void
generic_random(uint8 * random, int len)
{
	int tlen = len;
	while (tlen--) {
		*random = (uint8)rand();
		*random++;
	}
	return;
}
#endif /* (TARGETOS_nucleus || TARGETOS_symbian) */
