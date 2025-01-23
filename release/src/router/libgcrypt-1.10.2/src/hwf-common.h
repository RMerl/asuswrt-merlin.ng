/* hwf-common.h - Declarations for hwf-CPU.c modules
 * Copyright (C) 2012  g10 Code GmbH
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HWF_COMMON_H
#define HWF_COMMON_H

unsigned int _gcry_hwf_detect_x86 (void);
unsigned int _gcry_hwf_detect_arm (void);
unsigned int _gcry_hwf_detect_ppc (void);
unsigned int _gcry_hwf_detect_s390x (void);

#endif /*HWF_COMMON_H*/
