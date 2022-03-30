#ifndef _LPORT_PRINT_H_
#define _LPORT_PRINT_H_

#ifdef __UBOOT__
#include <stdio.h>
#define pr_err printf
#define pr_info printf
#define pr_debug(...)
#define RU_PRINT(...) printf(__VA_ARGS__)
#else
#include <linux/printk.h>
#define RU_PRINT(...) pr_info(__VA_ARGS__)
#endif

#endif
