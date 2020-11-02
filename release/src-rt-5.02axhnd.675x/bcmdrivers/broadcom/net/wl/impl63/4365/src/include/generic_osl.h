/*
 * Generic OS Support Layer
 *
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
 * $Id: generic_osl.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef generic_osl_h
#define generic_osl_h

#ifdef __cplusplus
extern "C" {
#endif // endif

#include <typedefs.h>
#include <lbuf.h>
#include <string.h>
#include <stdio.h>

/* --------------------------------------------------------------------------
 *  ASSERT
 */

#ifdef BCMDBG_ASSERT
extern void osl_assert(char *exp, char *file, int line);
#define ASSERT(exp) \
		do { if (!(exp)) osl_assert(#exp, __FILE__, __LINE__); } while (0)
#else
#define	ASSERT(exp)
#endif // endif

/* Helper macros for unsupported functionality. */
static INLINE int
NU_OSL_STUB_INT(int ret)
{
	ASSERT(0);
	return (ret);
}

static INLINE void*
NU_OSL_STUB_VOID_PTR(void* ret)
{
	ASSERT(0);
	return (ret);
}

/* --------------------------------------------------------------------------
 * Network interface packet buffer macros.
 *
 *  This needs to be included after the ASSERT macros because it includes
 * static include functions that use the ASSERT macro.
 */
#include <pkt_lbuf.h>

/* --------------------------------------------------------------------------
 * Printf
 */

/* Logging functions used by the Mobile Communications BSP. */
#ifdef BWL_MOBCOM_DBGPRINTF
#include <stdio.h>
int fprintf_bsp(FILE *stream, const char *fmt, ...);
int printf_bsp(const char *fmt, ...);
#define fprintf fprintf_bsp
#define printf printf_bsp
#define fputs(str, stream) 		printf("%s", str)
#define fflush(stream)
#endif   /* BWL_MOBCOM_DBGPRINTF */

/* --------------------------------------------------------------------------
 * Malloc
 */

#ifdef BCMDBG_MEM

#define	MALLOC(osh, size)	osl_debug_malloc((osh), (size), __LINE__, __FILE__)
void* osl_debug_malloc(osl_t *osh, uint size, int line, char* file);

#define	MALLOCZ(osh, size)	osl_debug_mallocz((osh), (size), __LINE__, __FILE__)
void* osl_debug_mallocz(osl_t *osh, uint size, int line, char* file);

#define	MFREE(osh, addr, size)	osl_debug_mfree((osh), (addr), (size), __LINE__, __FILE__)
void osl_debug_mfree(osl_t *osh, void *addr, uint size, int line, char* file);

#else    /* BCMDBG_MEM */

#define	MALLOC(osh, size)	osl_malloc((osh), (size))
#define	MALLOCZ(osh, size)	osl_mallocz((osh), (size))
#define	MFREE(osh, addr, size)	osl_mfree((osh), (addr), (size))

#endif   /* BCMDBG_MEM */

void* osl_malloc(osl_t *osh, uint size);
void* osl_mallocz(osl_t *osh, uint size);
void osl_mfree(osl_t *osh, void *addr, uint size);

#define	MALLOCED(osh)		osl_malloced((osh))
uint osl_malloced(osl_t *osh);

/* osl_malloc_std() / osl_free_std() will be used by osl_malloc() / osl_free()
 * respectively to perform the actually memory allocation and de-allocation.
 * Users can select to simply map these to the standard malloc/free routines
 * provided by the C run-time library, by defining BWL_OSL_USE_STDLIB_MALLOC.
 * Alternatively, users can implement their own version of osl_malloc_std() and
 * osl_free_std() if they wish to use a custom memory manager.
 */
#ifdef BWL_OSL_USE_STDLIB_MALLOC
#define osl_malloc_std	malloc
#define osl_free_std	free
#else
void* osl_malloc_std(size_t size);
void osl_free_std(void *ptr);
#endif   /* BWL_USE_STDLIB_MALLOC */

/* --------------------------------------------------------------------------
** OS abstraction APIs.
*/

typedef struct osl_pubinfo {
	unsigned int pktalloced;	/* Number of allocated packet buffers */

} osl_pubinfo_t;

/* microsecond delay */
#define	OSL_DELAY(usec)		osl_delay(usec)
extern void osl_delay(uint usec);

/* map from internal BRCM error code to OS error code. */
#define OSL_ERROR(bcmerror) ((bcmerror) < 0 ? -1 : 0)

/****************************************************************************
* Function:   osl_attach
*
* Purpose:    Init operating system abstraction layer.
*
* Parameters: None.
*
* Returns:    Operating system context.
*****************************************************************************
*/
osl_t* osl_attach(void);

/****************************************************************************
* Function:   osl_detach
*
* Purpose:    De-init the operating system abstraction layer.
*
* Parameters: osl (mod) Operating system context.
*
* Returns:    Nothing.
*****************************************************************************
*/
void osl_detach(osl_t *osh);

/* --------------------------------------------------------------------------
 * Hardware/architecture APIs.
 */

/* NOT SUPPORTED. */
#define	OSL_PCMCIA_READ_ATTR(osh, offset, buf, size) 	\
	({ \
	 BCM_REFERENCE(osh); \
	 BCM_REFERENCE(buf); \
	 NU_OSL_STUB_INT(0); \
	 })
#define	OSL_PCMCIA_WRITE_ATTR(osh, offset, buf, size) 	\
	({ \
	 BCM_REFERENCE(osh); \
	 BCM_REFERENCE(buf); \
	 NU_OSL_STUB_INT(0); \
	 })
#define OSL_PCI_READ_CONFIG(osh, offset, size)		({BCM_REFERENCE(osh); NU_OSL_STUB_INT(0);})
#define OSL_PCI_WRITE_CONFIG(osh, offset, size, val)	\
	({ \
	 BCM_REFERENCE(osh); \
	 BCM_REFERENCE(val); \
	 NU_OSL_STUB_INT(0); \
	 })
#define OSL_PCI_BUS(osh)				({BCM_REFERENCE(osh); NU_OSL_STUB_INT(0);})
#define OSL_PCI_SLOT(osh)				({BCM_REFERENCE(osh); NU_OSL_STUB_INT(0);})
#define OSL_PCIE_DOMAIN(osh)				({BCM_REFERENCE(osh); NU_OSL_STUB_INT(0);})
#define OSL_PCIE_BUS(osh)				({BCM_REFERENCE(osh); NU_OSL_STUB_INT(0);})

/* Map/unmap physical to virtual - NOT SUPPORTED. */
#define REG_MAP(pa, size)	({BCM_REFERENCE(pa); NU_OSL_STUB_VOID_PTR(NULL);})
#define REG_UNMAP(va)		({BCM_REFERENCE(va); NU_OSL_STUB_VOID_PTR(NULL);})

/* --------------------------------------------------------------------------
 * Register access macros.
 */
#if defined(BCMSDIO)
#include <bcmsdh.h>
#define R_REG(osh, r) \
	({ \
	 BCM_REFERENCE(osh); \
	 sizeof(*(r)) == sizeof(uint8) ? \
		(uint8)(bcmsdh_reg_read(NULL, (uint32)r, sizeof(*(r))) & 0xff) : \
	 sizeof(*(r)) == sizeof(uint16) ? \
		(uint16)(bcmsdh_reg_read(NULL, (uint32)r, sizeof(*(r))) & 0xffff) : \
	 bcmsdh_reg_read(NULL, (uint32)r, sizeof(*(r))); \
	 })
#define	W_REG(osh, r, v) \
	({ \
	 BCM_REFERENCE(osh); \
	 bcmsdh_reg_write(NULL, (uint32)r, sizeof(*(r)), (v)); \
	 })
#endif   /* BCMSDIO */

/* --------------------------------------------------------------------------
** Map bcopy to memcpy.
*/

#ifdef BWL_MAP_BCOPY_TO_MEMCPY
/* bcopy, bcmp, and bzero */
#define bcopy(src, dst, len)	memcpy((dst), (src), (len))
#define bcmp(b1, b2, len)	memcmp((b1), (b2), (len))
#define bzero(b, len)		memset((b), 0, (len))
#endif   /* BWL_MAP_BCOPY_TO_MEMCPY */

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
	}
#endif // endif

#endif  /* generic_osl_h  */
