/*
 * Copyright © 2016 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * A copy of the GPL is available at
 * http://www.broadcom.com/licenses/GPLv2.php or from the Free Software
 * Foundation at https://www.gnu.org/licenses/ .
 */

#ifndef _BRCMSTB_NEXUS_VERSION_CHECK_H_
#define _BRCMSTB_NEXUS_VERSION_CHECK_H_

#if (!defined(NEXUS_VERSION_MAJOR) || !defined(NEXUS_VERSION_MINOR))
#error NEXUS_VERSION_MAJOR and NEXUS_VERSION_MINOR must be defined!!!
#else

/* Check known version incompatibility rules */

/*
 * Redefine this rule when an incompatible change is made to a Linux release.
 *
 *  Placeholder template - older releases of Nexus do not include this file.
 *
 */
#define BRCM_INVALID_NEXUS_MAJOR	15
#define BRCM_INVALID_NEXUS_MINOR	4
#if ((NEXUS_VERSION_MAJOR) < BRCM_INVALID_NEXUS_MAJOR ) || \
     (((NEXUS_VERSION_MAJOR) == BRCM_INVALID_NEXUS_MAJOR) && \
      ((NEXUS_VERSION_MINOR) <= BRCM_INVALID_NEXUS_MINOR))
#error SWLINUX-3844: Nexus releases prior to 16.1 do not include this file!!!
#endif	/* NEXUS_VERSION incompatibility check */

#endif	/* NEXUS_VERSIONs must be defined */

#endif /* _BRCMSTB_NEXUS_VERSION_CHECK_H_ */
