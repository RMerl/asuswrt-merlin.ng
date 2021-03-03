/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _TC_RED_H_
#define _TC_RED_H_ 1

int tc_red_eval_P(unsigned qmin, unsigned qmax, double prob);
int tc_red_eval_ewma(unsigned qmin, unsigned burst, unsigned avpkt);
int tc_red_eval_idle_damping(int wlog, unsigned avpkt, unsigned bandwidth,
			     __u8 *sbuf);
void tc_red_print_flags(__u32 flags);

#endif
