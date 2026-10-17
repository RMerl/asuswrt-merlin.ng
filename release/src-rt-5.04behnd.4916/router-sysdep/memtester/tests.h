/*
 * Very simple yet very effective memory tester.
 * Originally by Simon Kirby <sim@stormix.com> <sim@neato.org>
 * Version 2 by Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Version 3 not publicly released.
 * Version 4 rewrite:
 * Copyright (C) 2004-2020 Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Licensed under the terms of the GNU General Public License version 2 (only).
 * See the file COPYING for details.
 *
 * This file contains the declarations for the functions for the actual tests,
 * called from the main routine in memtester.c.  See other comments in that 
 * file.
 *
 */

/* Function declaration. */

int test_stuck_address(unsigned long volatile *bufa, size_t count);
int test_random_value(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_xor_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_sub_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_mul_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_div_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_or_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_and_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_seqinc_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_solidbits_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_checkerboard_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_blockseq_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_walkbits0_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_walkbits1_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_bitspread_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_bitflip_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
#ifdef TEST_NARROW_WRITES    
int test_8bit_wide_random(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_16bit_wide_random(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
#endif

