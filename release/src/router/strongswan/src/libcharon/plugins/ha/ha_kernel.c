/*
 * Copyright (C) 2009-2011 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "ha_kernel.h"

typedef uint32_t u32;
typedef uint8_t u8;

#include <sys/utsname.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define CLUSTERIP_DIR "/proc/net/ipt_CLUSTERIP"

/**
 * Versions of jhash used in the Linux kernel
 */
typedef enum {
	/* old variant, http://burtleburtle.net/bob/c/lookup2.c */
	JHASH_LOOKUP2,
	/* new variant, http://burtleburtle.net/bob/c/lookup3.c, since 2.6.37 */
	JHASH_LOOKUP3,
	/* variant with different init values, since 4.1 */
	JHASH_LOOKUP3_1,
} jhash_version_t;

typedef struct private_ha_kernel_t private_ha_kernel_t;

/**
 * Private data of an ha_kernel_t object.
 */
struct private_ha_kernel_t {

	/**
	 * Public ha_kernel_t interface.
	 */
	ha_kernel_t public;

	/**
	 * Total number of ClusterIP segments
	 */
	u_int count;

	/**
	 * jhash version the kernel uses
	 */
	jhash_version_t version;
};

/**
 * Get the jhash version based on the uname().release
 */
static jhash_version_t get_jhash_version()
{
	struct utsname utsname;
	int a, b, c;

	if (uname(&utsname) == 0)
	{
		switch (sscanf(utsname.release, "%d.%d.%d", &a, &b, &c))
		{
			case 3:
				if (a == 2 && b == 6)
				{
					if (c < 37)
					{
						DBG1(DBG_CFG, "detected Linux %d.%d.%d, using old "
							 "jhash", a, b, c);
						return JHASH_LOOKUP2;
					}
					DBG1(DBG_CFG, "detected Linux %d.%d.%d, using new "
						 "jhash", a, b, c);
					return JHASH_LOOKUP3;
				}
				/* FALL */
			case 2:
				if (a < 4 || (a == 4 && b == 0))
				{
					DBG1(DBG_CFG, "detected Linux %d.%d, using new jhash",
						 a, b);
					return JHASH_LOOKUP3;
				}
				DBG1(DBG_CFG, "detected Linux %d.%d, using new jhash with "
					 "updated init values", a, b);
				return JHASH_LOOKUP3_1;
			default:
				break;
		}
	}
	DBG1(DBG_CFG, "detecting Linux version failed, using new jhash");
	return JHASH_LOOKUP3;
}

/**
 * Rotate 32 bit word x by k bits
 */
#define jhash_rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))

/**
 * jhash algorithm of two words, as used in kernel (using 0 as initval)
 */
static uint32_t jhash(jhash_version_t version, uint32_t a, uint32_t b)
{
	uint32_t c = 0;

	switch (version)
	{
		case JHASH_LOOKUP2:
			a += 0x9e3779b9;
			b += 0x9e3779b9;

			a -= b; a -= c; a ^= (c >> 13);
			b -= c; b -= a; b ^= (a <<  8);
			c -= a; c -= b; c ^= (b >> 13);
			a -= b; a -= c; a ^= (c >> 12);
			b -= c; b -= a; b ^= (a << 16);
			c -= a; c -= b; c ^= (b >>  5);
			a -= b; a -= c; a ^= (c >>  3);
			b -= c; b -= a; b ^= (a << 10);
			c -= a; c -= b; c ^= (b >> 15);
			break;
		case JHASH_LOOKUP3_1:
			/* changed with 4.1: # of 32-bit words shifted by 2 and c is
			 * initialized. we only use the two word variant with SPIs, so it's
			 * unlikely that b is 0 in that case */
			c += ((b ? 2 : 1) << 2) + 0xdeadbeef;
			a += ((b ? 2 : 1) << 2);
			b += ((b ? 2 : 1) << 2);
			/* FALL */
		case JHASH_LOOKUP3:
			a += 0xdeadbeef;
			b += 0xdeadbeef;

			c ^= b; c -= jhash_rot(b, 14);
			a ^= c; a -= jhash_rot(c, 11);
			b ^= a; b -= jhash_rot(a, 25);
			c ^= b; c -= jhash_rot(b, 16);
			a ^= c; a -= jhash_rot(c,  4);
			b ^= a; b -= jhash_rot(a, 14);
			c ^= b; c -= jhash_rot(b, 24);
			break;
	}
	return c;
}

/**
 * Segmentate a calculated hash
 */
static u_int hash2segment(private_ha_kernel_t *this, uint64_t hash)
{
	return ((hash * this->count) >> 32) + 1;
}

/**
 * Get a host as an integer for hashing
 */
static uint32_t host2int(host_t *host)
{
	if (host->get_family(host) == AF_INET)
	{
		return *(uint32_t*)host->get_address(host).ptr;
	}
	return 0;
}

METHOD(ha_kernel_t, get_segment, u_int,
	private_ha_kernel_t *this, host_t *host)
{
	unsigned long hash;
	uint32_t addr;

	addr = host2int(host);
	hash = jhash(this->version, ntohl(addr), 0);

	return hash2segment(this, hash);
}

METHOD(ha_kernel_t, get_segment_spi, u_int,
	private_ha_kernel_t *this, host_t *host, uint32_t spi)
{
	unsigned long hash;
	uint32_t addr;

	addr = host2int(host);
	hash = jhash(this->version, ntohl(addr), ntohl(spi));

	return hash2segment(this, hash);
}

METHOD(ha_kernel_t, get_segment_int, u_int,
	private_ha_kernel_t *this, int n)
{
	unsigned long hash;

	hash = jhash(this->version, ntohl(n), 0);

	return hash2segment(this, hash);
}

/**
 * Activate/Deactivate a segment for a given clusterip file
 */
static void enable_disable(private_ha_kernel_t *this, u_int segment,
						   char *file, bool enable)
{
	char cmd[8];
	int fd;

	snprintf(cmd, sizeof(cmd), "%c%d\n", enable ? '+' : '-', segment);

	fd = open(file, O_WRONLY);
	if (fd == -1)
	{
		DBG1(DBG_CFG, "opening CLUSTERIP file '%s' failed: %s",
			 file, strerror(errno));
		return;
	}
	if (write(fd, cmd, strlen(cmd)) == -1)
	{
		DBG1(DBG_CFG, "writing to CLUSTERIP file '%s' failed: %s",
			 file, strerror(errno));
	}
	close(fd);
}

/**
 * Get the currently active segments in the kernel for a clusterip file
 */
static segment_mask_t get_active(private_ha_kernel_t *this, char *file)
{
	char buf[256];
	segment_mask_t mask = 0;
	ssize_t len;
	int fd;

	fd = open(file, O_RDONLY);
	if (fd == -1)
	{
		DBG1(DBG_CFG, "opening CLUSTERIP file '%s' failed: %s",
			 file, strerror(errno));
		return 0;
	}
	len = read(fd, buf, sizeof(buf)-1);
	close(fd);
	if (len == -1)
	{
		DBG1(DBG_CFG, "reading from CLUSTERIP file '%s' failed: %s",
			 file, strerror(errno));
	}
	else
	{
		enumerator_t *enumerator;
		u_int segment;
		char *token;

		buf[len] = '\0';
		enumerator = enumerator_create_token(buf, ",", " ");
		while (enumerator->enumerate(enumerator, &token))
		{
			segment = atoi(token);
			if (segment)
			{
				mask |= SEGMENTS_BIT(segment);
			}
		}
		enumerator->destroy(enumerator);
	}
	return mask;
}

METHOD(ha_kernel_t, activate, void,
	private_ha_kernel_t *this, u_int segment)
{
	enumerator_t *enumerator;
	char *file;

	enumerator = enumerator_create_directory(CLUSTERIP_DIR);
	if (enumerator)
	{
		while (enumerator->enumerate(enumerator, NULL, &file, NULL))
		{
			enable_disable(this, segment, file, TRUE);
		}
		enumerator->destroy(enumerator);
	}
}

METHOD(ha_kernel_t, deactivate, void,
	private_ha_kernel_t *this, u_int segment)
{
	enumerator_t *enumerator;
	char *file;

	enumerator = enumerator_create_directory(CLUSTERIP_DIR);
	if (enumerator)
	{
		while (enumerator->enumerate(enumerator, NULL, &file, NULL))
		{
			enable_disable(this, segment, file, FALSE);
		}
		enumerator->destroy(enumerator);
	}
}

/**
 * Disable all not-yet disabled segments on all clusterip addresses
 */
static void disable_all(private_ha_kernel_t *this)
{
	enumerator_t *enumerator;
	segment_mask_t active;
	char *file;
	int i;

	enumerator = enumerator_create_directory(CLUSTERIP_DIR);
	if (enumerator)
	{
		while (enumerator->enumerate(enumerator, NULL, &file, NULL))
		{
			if (chown(file, lib->caps->get_uid(lib->caps),
					  lib->caps->get_gid(lib->caps)) != 0)
			{
				DBG1(DBG_CFG, "changing ClusterIP permissions failed: %s",
					 strerror(errno));
			}
			active = get_active(this, file);
			for (i = 1; i <= this->count; i++)
			{
				if (active & SEGMENTS_BIT(i))
				{
					enable_disable(this, i, file, FALSE);
				}
			}
		}
		enumerator->destroy(enumerator);
	}
}

METHOD(ha_kernel_t, destroy, void,
	private_ha_kernel_t *this)
{
	free(this);
}

/**
 * See header
 */
ha_kernel_t *ha_kernel_create(u_int count)
{
	private_ha_kernel_t *this;

	INIT(this,
		.public = {
			.get_segment = _get_segment,
			.get_segment_spi = _get_segment_spi,
			.get_segment_int = _get_segment_int,
			.activate = _activate,
			.deactivate = _deactivate,
			.destroy = _destroy,
		},
		.version = get_jhash_version(),
		.count = count,
	);

	disable_all(this);

	return &this->public;
}

