/*
 * Copyright (C) 2004, 2007-2010, 2011-2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ASM_LINKAGE_H
#define __ASM_LINKAGE_H

#ifdef __ASSEMBLY__

#define ASM_NL		 `	/* use '`' to mark new line in macro */

/* annotation for data we want in DCCM - if enabled in .config */
.macro ARCFP_DATA nm
#ifdef CONFIG_ARC_HAS_DCCM
	.section .data.arcfp
#else
	.section .data
#endif
	.global \nm
.endm

/* annotation for data we want in DCCM - if enabled in .config */
.macro ARCFP_CODE
#ifdef CONFIG_ARC_HAS_ICCM
	.section .text.arcfp, "ax",@progbits
#else
	.section .text, "ax",@progbits
#endif
.endm

#else	/* !__ASSEMBLY__ */

#ifdef CONFIG_ARC_HAS_ICCM
#define __arcfp_code __attribute__((__section__(".text.arcfp")))
#else
#define __arcfp_code __attribute__((__section__(".text")))
#endif

#ifdef CONFIG_ARC_HAS_DCCM
#define __arcfp_data __attribute__((__section__(".data.arcfp")))
#else
#define __arcfp_data __attribute__((__section__(".data")))
#endif

#endif /* __ASSEMBLY__ */

#endif
