/*
** video easylogo
** ==============
** (C) 2000 by Paolo Scaffardi (arsenio@tin.it)
** AIRVENT SAM s.p.a - RIMINI(ITALY)
**
** This utility is still under construction!
*/

#ifndef _EASYLOGO_H_
#define _EASYLOGO_H_

#if 0
#define ENABLE_ASCII_BANNERS
#endif

typedef struct {
	unsigned char	*data;
	int		width;
	int		height;
	int		bpp;
	int		pixel_size;
	int		size;
} fastimage_t ;

#endif	/* _EASYLOGO_H_ */
