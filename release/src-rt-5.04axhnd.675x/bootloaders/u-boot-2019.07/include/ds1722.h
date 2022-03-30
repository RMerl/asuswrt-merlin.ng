/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _DS1722_H_
#define _DS1722_H_

#define DS1722_RESOLUTION_8BIT	0x0
#define DS1722_RESOLUTION_9BIT	0x1
#define DS1722_RESOLUTION_10BIT	0x2
#define DS1722_RESOLUTION_11BIT	0x3
#define DS1722_RESOLUTION_12BIT	0x4

int ds1722_probe(int dev);

#endif /* _DS1722_H_ */
