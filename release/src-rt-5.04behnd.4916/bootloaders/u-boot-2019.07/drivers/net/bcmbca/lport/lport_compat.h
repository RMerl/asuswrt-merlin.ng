#ifndef _LPORT_DELAY_H_
#define _LPORT_DELAY_H_

#ifdef  __UBOOT__
#define EXPORT_SYMBOL(...)
#define UDELAY(_a) udelay(_a)
#include <linux/delay.h>
#include <linux/string.h>
#else
#define UDELAY(_a) udelay(_a)
#include <asm/delay.h>
#include "lport_intr.h"
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/completion.h>
#include <linux/jiffies.h>
#include <linux/string.h>
#endif

#endif
