/* SPDX-License-Identifier: GPL-2.0+ */

/*
 * MPC8xx I/O port pin manipulation functions
 * Roughly based on iopin_8260.h
 */

#ifndef _ASM_IOPIN_8XX_H_
#define _ASM_IOPIN_8XX_H_

#include <linux/types.h>
#include <asm/immap_8xx.h>
#include <asm/io.h>

#ifdef __KERNEL__

typedef struct {
	u_char port:2;	/* port number (A=0, B=1, C=2, D=3) */
	u_char pin:5;	/* port pin (0-31) */
	u_char flag:1;	/* for whatever */
} iopin_t;

#define IOPIN_PORTA	0
#define IOPIN_PORTB	1
#define IOPIN_PORTC	2
#define IOPIN_PORTD	3

static inline void iopin_set_high(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTA) {
		ushort __iomem *datp = &immap->im_ioport.iop_padat;

		setbits_be16(datp, 1 << (15 - iopin->pin));
	} else if (iopin->port == IOPIN_PORTB) {
		uint __iomem *datp = &immap->im_cpm.cp_pbdat;

		setbits_be32(datp, 1 << (31 - iopin->pin));
	} else if (iopin->port == IOPIN_PORTC) {
		ushort __iomem *datp = &immap->im_ioport.iop_pcdat;

		setbits_be16(datp, 1 << (15 - iopin->pin));
	} else if (iopin->port == IOPIN_PORTD) {
		ushort __iomem *datp = &immap->im_ioport.iop_pddat;

		setbits_be16(datp, 1 << (15 - iopin->pin));
	}
}

static inline void iopin_set_low(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTA) {
		ushort __iomem *datp = &immap->im_ioport.iop_padat;

		clrbits_be16(datp, 1 << (15 - iopin->pin));
	} else if (iopin->port == IOPIN_PORTB) {
		uint __iomem *datp = &immap->im_cpm.cp_pbdat;

		clrbits_be32(datp, 1 << (31 - iopin->pin));
	} else if (iopin->port == IOPIN_PORTC) {
		ushort __iomem *datp = &immap->im_ioport.iop_pcdat;

		clrbits_be16(datp, 1 << (15 - iopin->pin));
	} else if (iopin->port == IOPIN_PORTD) {
		ushort __iomem *datp = &immap->im_ioport.iop_pddat;

		clrbits_be16(datp, 1 << (15 - iopin->pin));
	}
}

static inline uint iopin_is_high(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTA) {
		ushort __iomem *datp = &immap->im_ioport.iop_padat;

		return (in_be16(datp) >> (15 - iopin->pin)) & 1;
	} else if (iopin->port == IOPIN_PORTB) {
		uint __iomem *datp = &immap->im_cpm.cp_pbdat;

		return (in_be32(datp) >> (31 - iopin->pin)) & 1;
	} else if (iopin->port == IOPIN_PORTC) {
		ushort __iomem *datp = &immap->im_ioport.iop_pcdat;

		return (in_be16(datp) >> (15 - iopin->pin)) & 1;
	} else if (iopin->port == IOPIN_PORTD) {
		ushort __iomem *datp = &immap->im_ioport.iop_pddat;

		return (in_be16(datp) >> (15 - iopin->pin)) & 1;
	}
	return 0;
}

static inline uint iopin_is_low(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTA) {
		ushort __iomem *datp = &immap->im_ioport.iop_padat;

		return ((in_be16(datp) >> (15 - iopin->pin)) & 1) ^ 1;
	} else if (iopin->port == IOPIN_PORTB) {
		uint __iomem *datp = &immap->im_cpm.cp_pbdat;

		return ((in_be32(datp) >> (31 - iopin->pin)) & 1) ^ 1;
	} else if (iopin->port == IOPIN_PORTC) {
		ushort __iomem *datp = &immap->im_ioport.iop_pcdat;

		return ((in_be16(datp) >> (15 - iopin->pin)) & 1) ^ 1;
	} else if (iopin->port == IOPIN_PORTD) {
		ushort __iomem *datp = &immap->im_ioport.iop_pddat;

		return ((in_be16(datp) >> (15 - iopin->pin)) & 1) ^ 1;
	}
	return 0;
}

static inline void iopin_set_out(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTA) {
		ushort __iomem *dirp = &immap->im_ioport.iop_padir;

		setbits_be16(dirp, 1 << (15 - iopin->pin));
	} else if (iopin->port == IOPIN_PORTB) {
		uint __iomem *dirp = &immap->im_cpm.cp_pbdir;

		setbits_be32(dirp, 1 << (31 - iopin->pin));
	} else if (iopin->port == IOPIN_PORTC) {
		ushort __iomem *dirp = &immap->im_ioport.iop_pcdir;

		setbits_be16(dirp, 1 << (15 - iopin->pin));
	} else if (iopin->port == IOPIN_PORTD) {
		ushort __iomem *dirp = &immap->im_ioport.iop_pddir;

		setbits_be16(dirp, 1 << (15 - iopin->pin));
	}
}

static inline void iopin_set_in(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTA) {
		ushort __iomem *dirp = &immap->im_ioport.iop_padir;

		clrbits_be16(dirp, 1 << (15 - iopin->pin));
	} else if (iopin->port == IOPIN_PORTB) {
		uint __iomem *dirp = &immap->im_cpm.cp_pbdir;

		clrbits_be32(dirp, 1 << (31 - iopin->pin));
	} else if (iopin->port == IOPIN_PORTC) {
		ushort __iomem *dirp = &immap->im_ioport.iop_pcdir;

		clrbits_be16(dirp, 1 << (15 - iopin->pin));
	} else if (iopin->port == IOPIN_PORTD) {
		ushort __iomem *dirp = &immap->im_ioport.iop_pddir;

		clrbits_be16(dirp, 1 << (15 - iopin->pin));
	}
}

static inline uint iopin_is_out(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTA) {
		ushort __iomem *dirp = &immap->im_ioport.iop_padir;

		return (in_be16(dirp) >> (15 - iopin->pin)) & 1;
	} else if (iopin->port == IOPIN_PORTB) {
		uint __iomem *dirp = &immap->im_cpm.cp_pbdir;

		return (in_be32(dirp) >> (31 - iopin->pin)) & 1;
	} else if (iopin->port == IOPIN_PORTC) {
		ushort __iomem *dirp = &immap->im_ioport.iop_pcdir;

		return (in_be16(dirp) >> (15 - iopin->pin)) & 1;
	} else if (iopin->port == IOPIN_PORTD) {
		ushort __iomem *dirp = &immap->im_ioport.iop_pddir;

		return (in_be16(dirp) >> (15 - iopin->pin)) & 1;
	}
	return 0;
}

static inline uint iopin_is_in(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTA) {
		ushort __iomem *dirp = &immap->im_ioport.iop_padir;

		return ((in_be16(dirp) >> (15 - iopin->pin)) & 1) ^ 1;
	} else if (iopin->port == IOPIN_PORTB) {
		uint __iomem *dirp = &immap->im_cpm.cp_pbdir;

		return ((in_be32(dirp) >> (31 - iopin->pin)) & 1) ^ 1;
	} else if (iopin->port == IOPIN_PORTC) {
		ushort __iomem *dirp = &immap->im_ioport.iop_pcdir;

		return ((in_be16(dirp) >> (15 - iopin->pin)) & 1) ^ 1;
	} else if (iopin->port == IOPIN_PORTD) {
		ushort __iomem *dirp = &immap->im_ioport.iop_pddir;

		return ((in_be16(dirp) >> (15 - iopin->pin)) & 1) ^ 1;
	}
	return 0;
}

static inline void iopin_set_odr(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTA) {
		ushort __iomem *odrp = &immap->im_ioport.iop_paodr;

		setbits_be16(odrp, 1 << (15 - iopin->pin));
	} else if (iopin->port == IOPIN_PORTB) {
		ushort __iomem *odrp = &immap->im_cpm.cp_pbodr;

		setbits_be16(odrp, 1 << (31 - iopin->pin));
	}
}

static inline void iopin_set_act(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTA) {
		ushort __iomem *odrp = &immap->im_ioport.iop_paodr;

		clrbits_be16(odrp, 1 << (15 - iopin->pin));
	} else if (iopin->port == IOPIN_PORTB) {
		ushort __iomem *odrp = &immap->im_cpm.cp_pbodr;

		clrbits_be16(odrp, 1 << (31 - iopin->pin));
	}
}

static inline uint iopin_is_odr(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTA) {
		ushort __iomem *odrp = &immap->im_ioport.iop_paodr;

		return (in_be16(odrp) >> (15 - iopin->pin)) & 1;
	} else if (iopin->port == IOPIN_PORTB) {
		ushort __iomem *odrp = &immap->im_cpm.cp_pbodr;

		return (in_be16(odrp) >> (31 - iopin->pin)) & 1;
	}
	return 0;
}

static inline uint iopin_is_act(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTA) {
		ushort __iomem *odrp = &immap->im_ioport.iop_paodr;

		return ((in_be16(odrp) >> (15 - iopin->pin)) & 1) ^ 1;
	} else if (iopin->port == IOPIN_PORTB) {
		ushort __iomem *odrp = &immap->im_cpm.cp_pbodr;

		return ((in_be16(odrp) >> (31 - iopin->pin)) & 1) ^ 1;
	}
	return 0;
}

static inline void iopin_set_ded(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTA) {
		ushort __iomem *parp = &immap->im_ioport.iop_papar;

		setbits_be16(parp, 1 << (15 - iopin->pin));
	} else if (iopin->port == IOPIN_PORTB) {
		uint __iomem *parp = &immap->im_cpm.cp_pbpar;

		setbits_be32(parp, 1 << (31 - iopin->pin));
	} else if (iopin->port == IOPIN_PORTC) {
		ushort __iomem *parp = &immap->im_ioport.iop_pcpar;

		setbits_be16(parp, 1 << (15 - iopin->pin));
	} else if (iopin->port == IOPIN_PORTD) {
		ushort __iomem *parp = &immap->im_ioport.iop_pdpar;

		setbits_be16(parp, 1 << (15 - iopin->pin));
	}
}

static inline void iopin_set_gen(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTA) {
		ushort __iomem *parp = &immap->im_ioport.iop_papar;

		clrbits_be16(parp, 1 << (15 - iopin->pin));
	} else if (iopin->port == IOPIN_PORTB) {
		uint __iomem *parp = &immap->im_cpm.cp_pbpar;

		clrbits_be32(parp, 1 << (31 - iopin->pin));
	} else if (iopin->port == IOPIN_PORTC) {
		ushort __iomem *parp = &immap->im_ioport.iop_pcpar;

		clrbits_be16(parp, 1 << (15 - iopin->pin));
	} else if (iopin->port == IOPIN_PORTD) {
		ushort __iomem *parp = &immap->im_ioport.iop_pdpar;

		clrbits_be16(parp, 1 << (15 - iopin->pin));
	}
}

static inline uint iopin_is_ded(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTA) {
		ushort __iomem *parp = &immap->im_ioport.iop_papar;

		return (in_be16(parp) >> (15 - iopin->pin)) & 1;
	} else if (iopin->port == IOPIN_PORTB) {
		uint __iomem *parp = &immap->im_cpm.cp_pbpar;

		return (in_be32(parp) >> (31 - iopin->pin)) & 1;
	} else if (iopin->port == IOPIN_PORTC) {
		ushort __iomem *parp = &immap->im_ioport.iop_pcpar;

		return (in_be16(parp) >> (15 - iopin->pin)) & 1;
	} else if (iopin->port == IOPIN_PORTD) {
		ushort __iomem *parp = &immap->im_ioport.iop_pdpar;

		return (in_be16(parp) >> (15 - iopin->pin)) & 1;
	}
	return 0;
}

static inline uint iopin_is_gen(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTA) {
		ushort __iomem *parp = &immap->im_ioport.iop_papar;

		return ((in_be16(parp) >> (15 - iopin->pin)) & 1) ^ 1;
	} else if (iopin->port == IOPIN_PORTB) {
		uint __iomem *parp = &immap->im_cpm.cp_pbpar;

		return ((in_be32(parp) >> (31 - iopin->pin)) & 1) ^ 1;
	} else if (iopin->port == IOPIN_PORTC) {
		ushort __iomem *parp = &immap->im_ioport.iop_pcpar;

		return ((in_be16(parp) >> (15 - iopin->pin)) & 1) ^ 1;
	} else if (iopin->port == IOPIN_PORTD) {
		ushort __iomem *parp = &immap->im_ioport.iop_pdpar;

		return ((in_be16(parp) >> (15 - iopin->pin)) & 1) ^ 1;
	}
	return 0;
}

static inline void iopin_set_opt2(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTC) {
		ushort __iomem *sorp = &immap->im_ioport.iop_pcso;

		setbits_be16(sorp, 1 << (15 - iopin->pin));
	}
}

static inline void iopin_set_opt1(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTC) {
		ushort __iomem *sorp = &immap->im_ioport.iop_pcso;

		clrbits_be16(sorp, 1 << (15 - iopin->pin));
	}
}

static inline uint iopin_is_opt2(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTC) {
		ushort __iomem *sorp = &immap->im_ioport.iop_pcso;

		return (in_be16(sorp) >> (15 - iopin->pin)) & 1;
	}
	return 0;
}

static inline uint iopin_is_opt1(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTC) {
		ushort __iomem *sorp = &immap->im_ioport.iop_pcso;

		return ((in_be16(sorp) >> (15 - iopin->pin)) & 1) ^ 1;
	}
	return 0;
}

static inline void iopin_set_falledge(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTC) {
		ushort __iomem *intp = &immap->im_ioport.iop_pcint;

		setbits_be16(intp, 1 << (15 - iopin->pin));
	}
}

static inline void iopin_set_anyedge(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTC) {
		ushort __iomem *intp = &immap->im_ioport.iop_pcint;

		clrbits_be16(intp, 1 << (15 - iopin->pin));
	}
}

static inline uint iopin_is_falledge(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTC) {
		ushort __iomem *intp = &immap->im_ioport.iop_pcint;

		return (in_be16(intp) >> (15 - iopin->pin)) & 1;
	}
	return 0;
}

static inline uint iopin_is_anyedge(iopin_t *iopin)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	if (iopin->port == IOPIN_PORTC) {
		ushort __iomem *intp = &immap->im_ioport.iop_pcint;

		return ((in_be16(intp) >> (15 - iopin->pin)) & 1) ^ 1;
	}
	return 0;
}

#endif /* __KERNEL__ */

#endif /* _ASM_IOPIN_8XX_H_ */
