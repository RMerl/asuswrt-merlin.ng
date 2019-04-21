#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>

struct nf_conntrack;
struct nf_expect;

int init_log(void);
void dlog(int priority, const char *format, ...);
void dlog_ct(FILE *fd, struct nf_conntrack *ct, unsigned int type);
void dlog_exp(FILE *fd, struct nf_expect *exp, unsigned int type);
void close_log(void);

#endif
