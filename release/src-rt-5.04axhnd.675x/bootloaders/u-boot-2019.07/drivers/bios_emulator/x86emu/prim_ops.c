/****************************************************************************
*
*			Realmode X86 Emulator Library
*
*		Copyright (C) 1991-2004 SciTech Software, Inc.
*		     Copyright (C) David Mosberger-Tang
*		       Copyright (C) 1999 Egbert Eich
*
*  ========================================================================
*
*  Permission to use, copy, modify, distribute, and sell this software and
*  its documentation for any purpose is hereby granted without fee,
*  provided that the above copyright notice appear in all copies and that
*  both that copyright notice and this permission notice appear in
*  supporting documentation, and that the name of the authors not be used
*  in advertising or publicity pertaining to distribution of the software
*  without specific, written prior permission.	The authors makes no
*  representations about the suitability of this software for any purpose.
*  It is provided "as is" without express or implied warranty.
*
*  THE AUTHORS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
*  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
*  EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
*  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
*  USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
*  OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
*  PERFORMANCE OF THIS SOFTWARE.
*
*  ========================================================================
*
* Language:	ANSI C
* Environment:	Any
* Developer:	Kendall Bennett
*
* Description:	This file contains the code to implement the primitive
*		machine operations used by the emulation code in ops.c
*
* Carry Chain Calculation
*
* This represents a somewhat expensive calculation which is
* apparently required to emulate the setting of the OF343364 and AF flag.
* The latter is not so important, but the former is.  The overflow
* flag is the XOR of the top two bits of the carry chain for an
* addition (similar for subtraction).  Since we do not want to
* simulate the addition in a bitwise manner, we try to calculate the
* carry chain given the two operands and the result.
*
* So, given the following table, which represents the addition of two
* bits, we can derive a formula for the carry chain.
*
* a   b	  cin	r     cout
* 0   0	  0	0     0
* 0   0	  1	1     0
* 0   1	  0	1     0
* 0   1	  1	0     1
* 1   0	  0	1     0
* 1   0	  1	0     1
* 1   1	  0	0     1
* 1   1	  1	1     1
*
* Construction of table for cout:
*
* ab
* r  \	00   01	  11  10
* |------------------
* 0  |	 0    1	   1   1
* 1  |	 0    0	   1   0
*
* By inspection, one gets:  cc = ab +  r'(a + b)
*
* That represents alot of operations, but NO CHOICE....
*
* Borrow Chain Calculation.
*
* The following table represents the subtraction of two bits, from
* which we can derive a formula for the borrow chain.
*
* a   b	  bin	r     bout
* 0   0	  0	0     0
* 0   0	  1	1     1
* 0   1	  0	1     1
* 0   1	  1	0     1
* 1   0	  0	1     0
* 1   0	  1	0     0
* 1   1	  0	0     0
* 1   1	  1	1     1
*
* Construction of table for cout:
*
* ab
* r  \	00   01	  11  10
* |------------------
* 0  |	 0    1	   0   0
* 1  |	 1    1	   1   0
*
* By inspection, one gets:  bc = a'b +	r(a' + b)
*
****************************************************************************/

#include <common.h>

#define PRIM_OPS_NO_REDEFINE_ASM
#include "x86emu/x86emui.h"

/*------------------------- Global Variables ------------------------------*/

static u32 x86emu_parity_tab[8] =
{
    0x96696996,
    0x69969669,
    0x69969669,
    0x96696996,
    0x69969669,
    0x96696996,
    0x96696996,
    0x69969669,
};

#define PARITY(x)   (((x86emu_parity_tab[(x) / 32] >> ((x) % 32)) & 1) == 0)
#define XOR2(x)	    (((x) ^ ((x)>>1)) & 0x1)

/*----------------------------- Implementation ----------------------------*/


/*--------- Side effects helper functions -------*/

/****************************************************************************
REMARKS:
implements side efects for byte operations that don't overflow
****************************************************************************/

static void set_parity_flag(u32 res)
{
    CONDITIONAL_SET_FLAG(PARITY(res & 0xFF), F_PF);
}

static void set_szp_flags_8(u8 res)
{
    CONDITIONAL_SET_FLAG(res & 0x80, F_SF);
    CONDITIONAL_SET_FLAG(res == 0, F_ZF);
    set_parity_flag(res);
}

static void set_szp_flags_16(u16 res)
{
    CONDITIONAL_SET_FLAG(res & 0x8000, F_SF);
    CONDITIONAL_SET_FLAG(res == 0, F_ZF);
    set_parity_flag(res);
}

static void set_szp_flags_32(u32 res)
{
    CONDITIONAL_SET_FLAG(res & 0x80000000, F_SF);
    CONDITIONAL_SET_FLAG(res == 0, F_ZF);
    set_parity_flag(res);
}

static void no_carry_byte_side_eff(u8 res)
{
    CLEAR_FLAG(F_OF);
    CLEAR_FLAG(F_CF);
    CLEAR_FLAG(F_AF);
    set_szp_flags_8(res);
}

static void no_carry_word_side_eff(u16 res)
{
    CLEAR_FLAG(F_OF);
    CLEAR_FLAG(F_CF);
    CLEAR_FLAG(F_AF);
    set_szp_flags_16(res);
}

static void no_carry_long_side_eff(u32 res)
{
    CLEAR_FLAG(F_OF);
    CLEAR_FLAG(F_CF);
    CLEAR_FLAG(F_AF);
    set_szp_flags_32(res);
}

static void calc_carry_chain(int bits, u32 d, u32 s, u32 res, int set_carry)
{
    u32 cc;

    cc = (s & d) | ((~res) & (s | d));
    CONDITIONAL_SET_FLAG(XOR2(cc >> (bits - 2)), F_OF);
    CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
    if (set_carry) {
	CONDITIONAL_SET_FLAG(res & (1 << bits), F_CF);
    }
}

static void calc_borrow_chain(int bits, u32 d, u32 s, u32 res, int set_carry)
{
    u32 bc;

    bc = (res & (~d | s)) | (~d & s);
    CONDITIONAL_SET_FLAG(XOR2(bc >> (bits - 2)), F_OF);
    CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
    if (set_carry) {
	CONDITIONAL_SET_FLAG(bc & (1 << (bits - 1)), F_CF);
    }
}

/****************************************************************************
REMARKS:
Implements the AAA instruction and side effects.
****************************************************************************/
u16 aaa_word(u16 d)
{
    u16 res;
    if ((d & 0xf) > 0x9 || ACCESS_FLAG(F_AF)) {
	d += 0x6;
	d += 0x100;
	SET_FLAG(F_AF);
	SET_FLAG(F_CF);
    } else {
	CLEAR_FLAG(F_CF);
	CLEAR_FLAG(F_AF);
    }
    res = (u16)(d & 0xFF0F);
    set_szp_flags_16(res);
    return res;
}

/****************************************************************************
REMARKS:
Implements the AAA instruction and side effects.
****************************************************************************/
u16 aas_word(u16 d)
{
    u16 res;
    if ((d & 0xf) > 0x9 || ACCESS_FLAG(F_AF)) {
	d -= 0x6;
	d -= 0x100;
	SET_FLAG(F_AF);
	SET_FLAG(F_CF);
    } else {
	CLEAR_FLAG(F_CF);
	CLEAR_FLAG(F_AF);
    }
    res = (u16)(d & 0xFF0F);
    set_szp_flags_16(res);
    return res;
}

/****************************************************************************
REMARKS:
Implements the AAD instruction and side effects.
****************************************************************************/
u16 aad_word(u16 d)
{
    u16 l;
    u8 hb, lb;

    hb = (u8)((d >> 8) & 0xff);
    lb = (u8)((d & 0xff));
    l = (u16)((lb + 10 * hb) & 0xFF);

    no_carry_byte_side_eff(l & 0xFF);
    return l;
}

/****************************************************************************
REMARKS:
Implements the AAM instruction and side effects.
****************************************************************************/
u16 aam_word(u8 d)
{
    u16 h, l;

    h = (u16)(d / 10);
    l = (u16)(d % 10);
    l |= (u16)(h << 8);

    no_carry_byte_side_eff(l & 0xFF);
    return l;
}

/****************************************************************************
REMARKS:
Implements the ADC instruction and side effects.
****************************************************************************/
u8 adc_byte(u8 d, u8 s)
{
    u32 res;   /* all operands in native machine order */

    res = d + s;
    if (ACCESS_FLAG(F_CF)) res++;

    set_szp_flags_8(res);
    calc_carry_chain(8,s,d,res,1);

    return (u8)res;
}

/****************************************************************************
REMARKS:
Implements the ADC instruction and side effects.
****************************************************************************/
u16 adc_word(u16 d, u16 s)
{
    u32 res;   /* all operands in native machine order */

    res = d + s;
    if (ACCESS_FLAG(F_CF))
	res++;

    set_szp_flags_16((u16)res);
    calc_carry_chain(16,s,d,res,1);

    return (u16)res;
}

/****************************************************************************
REMARKS:
Implements the ADC instruction and side effects.
****************************************************************************/
u32 adc_long(u32 d, u32 s)
{
    u32 lo;    /* all operands in native machine order */
    u32 hi;
    u32 res;

    lo = (d & 0xFFFF) + (s & 0xFFFF);
    res = d + s;

    if (ACCESS_FLAG(F_CF)) {
	lo++;
	res++;
    }

    hi = (lo >> 16) + (d >> 16) + (s >> 16);

    set_szp_flags_32(res);
    calc_carry_chain(32,s,d,res,0);

    CONDITIONAL_SET_FLAG(hi & 0x10000, F_CF);

    return res;
}

/****************************************************************************
REMARKS:
Implements the ADD instruction and side effects.
****************************************************************************/
u8 add_byte(u8 d, u8 s)
{
    u32 res;   /* all operands in native machine order */

    res = d + s;
    set_szp_flags_8((u8)res);
    calc_carry_chain(8,s,d,res,1);

    return (u8)res;
}

/****************************************************************************
REMARKS:
Implements the ADD instruction and side effects.
****************************************************************************/
u16 add_word(u16 d, u16 s)
{
    u32 res;   /* all operands in native machine order */

    res = d + s;
    set_szp_flags_16((u16)res);
    calc_carry_chain(16,s,d,res,1);

    return (u16)res;
}

/****************************************************************************
REMARKS:
Implements the ADD instruction and side effects.
****************************************************************************/
u32 add_long(u32 d, u32 s)
{
    u32 res;

    res = d + s;
    set_szp_flags_32(res);
    calc_carry_chain(32,s,d,res,0);

    CONDITIONAL_SET_FLAG(res < d || res < s, F_CF);

    return res;
}

/****************************************************************************
REMARKS:
Implements the AND instruction and side effects.
****************************************************************************/
u8 and_byte(u8 d, u8 s)
{
    u8 res;    /* all operands in native machine order */

    res = d & s;

    no_carry_byte_side_eff(res);
    return res;
}

/****************************************************************************
REMARKS:
Implements the AND instruction and side effects.
****************************************************************************/
u16 and_word(u16 d, u16 s)
{
    u16 res;   /* all operands in native machine order */

    res = d & s;

    no_carry_word_side_eff(res);
    return res;
}

/****************************************************************************
REMARKS:
Implements the AND instruction and side effects.
****************************************************************************/
u32 and_long(u32 d, u32 s)
{
    u32 res;   /* all operands in native machine order */

    res = d & s;
    no_carry_long_side_eff(res);
    return res;
}

/****************************************************************************
REMARKS:
Implements the CMP instruction and side effects.
****************************************************************************/
u8 cmp_byte(u8 d, u8 s)
{
    u32 res;   /* all operands in native machine order */

    res = d - s;
    set_szp_flags_8((u8)res);
    calc_borrow_chain(8, d, s, res, 1);

    return d;
}

/****************************************************************************
REMARKS:
Implements the CMP instruction and side effects.
****************************************************************************/
u16 cmp_word(u16 d, u16 s)
{
    u32 res;   /* all operands in native machine order */

    res = d - s;
    set_szp_flags_16((u16)res);
    calc_borrow_chain(16, d, s, res, 1);

    return d;
}

/****************************************************************************
REMARKS:
Implements the CMP instruction and side effects.
****************************************************************************/
u32 cmp_long(u32 d, u32 s)
{
    u32 res;   /* all operands in native machine order */

    res = d - s;
    set_szp_flags_32(res);
    calc_borrow_chain(32, d, s, res, 1);

    return d;
}

/****************************************************************************
REMARKS:
Implements the DAA instruction and side effects.
****************************************************************************/
u8 daa_byte(u8 d)
{
    u32 res = d;
    if ((d & 0xf) > 9 || ACCESS_FLAG(F_AF)) {
	res += 6;
	SET_FLAG(F_AF);
    }
    if (res > 0x9F || ACCESS_FLAG(F_CF)) {
	res += 0x60;
	SET_FLAG(F_CF);
    }
    set_szp_flags_8((u8)res);
    return (u8)res;
}

/****************************************************************************
REMARKS:
Implements the DAS instruction and side effects.
****************************************************************************/
u8 das_byte(u8 d)
{
    if ((d & 0xf) > 9 || ACCESS_FLAG(F_AF)) {
	d -= 6;
	SET_FLAG(F_AF);
    }
    if (d > 0x9F || ACCESS_FLAG(F_CF)) {
	d -= 0x60;
	SET_FLAG(F_CF);
    }
    set_szp_flags_8(d);
    return d;
}

/****************************************************************************
REMARKS:
Implements the DEC instruction and side effects.
****************************************************************************/
u8 dec_byte(u8 d)
{
    u32 res;   /* all operands in native machine order */

    res = d - 1;
    set_szp_flags_8((u8)res);
    calc_borrow_chain(8, d, 1, res, 0);

    return (u8)res;
}

/****************************************************************************
REMARKS:
Implements the DEC instruction and side effects.
****************************************************************************/
u16 dec_word(u16 d)
{
    u32 res;   /* all operands in native machine order */

    res = d - 1;
    set_szp_flags_16((u16)res);
    calc_borrow_chain(16, d, 1, res, 0);

    return (u16)res;
}

/****************************************************************************
REMARKS:
Implements the DEC instruction and side effects.
****************************************************************************/
u32 dec_long(u32 d)
{
    u32 res;   /* all operands in native machine order */

    res = d - 1;

    set_szp_flags_32(res);
    calc_borrow_chain(32, d, 1, res, 0);

    return res;
}

/****************************************************************************
REMARKS:
Implements the INC instruction and side effects.
****************************************************************************/
u8 inc_byte(u8 d)
{
    u32 res;   /* all operands in native machine order */

    res = d + 1;
    set_szp_flags_8((u8)res);
    calc_carry_chain(8, d, 1, res, 0);

    return (u8)res;
}

/****************************************************************************
REMARKS:
Implements the INC instruction and side effects.
****************************************************************************/
u16 inc_word(u16 d)
{
    u32 res;   /* all operands in native machine order */

    res = d + 1;
    set_szp_flags_16((u16)res);
    calc_carry_chain(16, d, 1, res, 0);

    return (u16)res;
}

/****************************************************************************
REMARKS:
Implements the INC instruction and side effects.
****************************************************************************/
u32 inc_long(u32 d)
{
    u32 res;   /* all operands in native machine order */

    res = d + 1;
    set_szp_flags_32(res);
    calc_carry_chain(32, d, 1, res, 0);

    return res;
}

/****************************************************************************
REMARKS:
Implements the OR instruction and side effects.
****************************************************************************/
u8 or_byte(u8 d, u8 s)
{
    u8 res;    /* all operands in native machine order */

    res = d | s;
    no_carry_byte_side_eff(res);

    return res;
}

/****************************************************************************
REMARKS:
Implements the OR instruction and side effects.
****************************************************************************/
u16 or_word(u16 d, u16 s)
{
    u16 res;   /* all operands in native machine order */

    res = d | s;
    no_carry_word_side_eff(res);
    return res;
}

/****************************************************************************
REMARKS:
Implements the OR instruction and side effects.
****************************************************************************/
u32 or_long(u32 d, u32 s)
{
    u32 res;   /* all operands in native machine order */

    res = d | s;
    no_carry_long_side_eff(res);
    return res;
}

/****************************************************************************
REMARKS:
Implements the OR instruction and side effects.
****************************************************************************/
u8 neg_byte(u8 s)
{
    u8 res;

    CONDITIONAL_SET_FLAG(s != 0, F_CF);
    res = (u8)-s;
    set_szp_flags_8(res);
    calc_borrow_chain(8, 0, s, res, 0);

    return res;
}

/****************************************************************************
REMARKS:
Implements the OR instruction and side effects.
****************************************************************************/
u16 neg_word(u16 s)
{
    u16 res;

    CONDITIONAL_SET_FLAG(s != 0, F_CF);
    res = (u16)-s;
    set_szp_flags_16((u16)res);
    calc_borrow_chain(16, 0, s, res, 0);

    return res;
}

/****************************************************************************
REMARKS:
Implements the OR instruction and side effects.
****************************************************************************/
u32 neg_long(u32 s)
{
    u32 res;

    CONDITIONAL_SET_FLAG(s != 0, F_CF);
    res = (u32)-s;
    set_szp_flags_32(res);
    calc_borrow_chain(32, 0, s, res, 0);

    return res;
}

/****************************************************************************
REMARKS:
Implements the NOT instruction and side effects.
****************************************************************************/
u8 not_byte(u8 s)
{
    return ~s;
}

/****************************************************************************
REMARKS:
Implements the NOT instruction and side effects.
****************************************************************************/
u16 not_word(u16 s)
{
    return ~s;
}

/****************************************************************************
REMARKS:
Implements the NOT instruction and side effects.
****************************************************************************/
u32 not_long(u32 s)
{
    return ~s;
}

/****************************************************************************
REMARKS:
Implements the RCL instruction and side effects.
****************************************************************************/
u8 rcl_byte(u8 d, u8 s)
{
    unsigned int res, cnt, mask, cf;

    /* s is the rotate distance.  It varies from 0 - 8. */
    /* have

       CF  B_7 B_6 B_5 B_4 B_3 B_2 B_1 B_0

       want to rotate through the carry by "s" bits.  We could
       loop, but that's inefficient.  So the width is 9,
       and we split into three parts:

       The new carry flag   (was B_n)
       the stuff in B_n-1 .. B_0
       the stuff in B_7 .. B_n+1

       The new rotate is done mod 9, and given this,
       for a rotation of n bits (mod 9) the new carry flag is
       then located n bits from the MSB.  The low part is
       then shifted up cnt bits, and the high part is or'd
       in.  Using CAPS for new values, and lowercase for the
       original values, this can be expressed as:

       IF n > 0
       1) CF <-	 b_(8-n)
       2) B_(7) .. B_(n)  <-  b_(8-(n+1)) .. b_0
       3) B_(n-1) <- cf
       4) B_(n-2) .. B_0 <-  b_7 .. b_(8-(n-1))
     */
    res = d;
    if ((cnt = s % 9) != 0) {
	/* extract the new CARRY FLAG. */
	/* CF <-  b_(8-n)	      */
	cf = (d >> (8 - cnt)) & 0x1;

	/* get the low stuff which rotated
	   into the range B_7 .. B_cnt */
	/* B_(7) .. B_(n)  <-  b_(8-(n+1)) .. b_0  */
	/* note that the right hand side done by the mask */
	res = (d << cnt) & 0xff;

	/* now the high stuff which rotated around
	   into the positions B_cnt-2 .. B_0 */
	/* B_(n-2) .. B_0 <-  b_7 .. b_(8-(n-1)) */
	/* shift it downward, 7-(n-2) = 9-n positions.
	   and mask off the result before or'ing in.
	 */
	mask = (1 << (cnt - 1)) - 1;
	res |= (d >> (9 - cnt)) & mask;

	/* if the carry flag was set, or it in.	 */
	if (ACCESS_FLAG(F_CF)) {     /* carry flag is set */
	    /*	B_(n-1) <- cf */
	    res |= 1 << (cnt - 1);
	}
	/* set the new carry flag, based on the variable "cf" */
	CONDITIONAL_SET_FLAG(cf, F_CF);
	/* OVERFLOW is set *IFF* cnt==1, then it is the
	   xor of CF and the most significant bit.  Blecck. */
	/* parenthesized this expression since it appears to
	   be causing OF to be misset */
	CONDITIONAL_SET_FLAG(cnt == 1 && XOR2(cf + ((res >> 6) & 0x2)),
			     F_OF);

    }
    return (u8)res;
}

/****************************************************************************
REMARKS:
Implements the RCL instruction and side effects.
****************************************************************************/
u16 rcl_word(u16 d, u8 s)
{
    unsigned int res, cnt, mask, cf;

    res = d;
    if ((cnt = s % 17) != 0) {
	cf = (d >> (16 - cnt)) & 0x1;
	res = (d << cnt) & 0xffff;
	mask = (1 << (cnt - 1)) - 1;
	res |= (d >> (17 - cnt)) & mask;
	if (ACCESS_FLAG(F_CF)) {
	    res |= 1 << (cnt - 1);
	}
	CONDITIONAL_SET_FLAG(cf, F_CF);
	CONDITIONAL_SET_FLAG(cnt == 1 && XOR2(cf + ((res >> 14) & 0x2)),
			     F_OF);
    }
    return (u16)res;
}

/****************************************************************************
REMARKS:
Implements the RCL instruction and side effects.
****************************************************************************/
u32 rcl_long(u32 d, u8 s)
{
    u32 res, cnt, mask, cf;

    res = d;
    if ((cnt = s % 33) != 0) {
	cf = (d >> (32 - cnt)) & 0x1;
	res = (d << cnt) & 0xffffffff;
	mask = (1 << (cnt - 1)) - 1;
	res |= (d >> (33 - cnt)) & mask;
	if (ACCESS_FLAG(F_CF)) {     /* carry flag is set */
	    res |= 1 << (cnt - 1);
	}
	CONDITIONAL_SET_FLAG(cf, F_CF);
	CONDITIONAL_SET_FLAG(cnt == 1 && XOR2(cf + ((res >> 30) & 0x2)),
			     F_OF);
    }
    return res;
}

/****************************************************************************
REMARKS:
Implements the RCR instruction and side effects.
****************************************************************************/
u8 rcr_byte(u8 d, u8 s)
{
    u32 res, cnt;
    u32 mask, cf, ocf = 0;

    /* rotate right through carry */
    /*
       s is the rotate distance.  It varies from 0 - 8.
       d is the byte object rotated.

       have

       CF  B_7 B_6 B_5 B_4 B_3 B_2 B_1 B_0

       The new rotate is done mod 9, and given this,
       for a rotation of n bits (mod 9) the new carry flag is
       then located n bits from the LSB.  The low part is
       then shifted up cnt bits, and the high part is or'd
       in.  Using CAPS for new values, and lowercase for the
       original values, this can be expressed as:

       IF n > 0
       1) CF <-	 b_(n-1)
       2) B_(8-(n+1)) .. B_(0)	<-  b_(7) .. b_(n)
       3) B_(8-n) <- cf
       4) B_(7) .. B_(8-(n-1)) <-  b_(n-2) .. b_(0)
     */
    res = d;
    if ((cnt = s % 9) != 0) {
	/* extract the new CARRY FLAG. */
	/* CF <-  b_(n-1)	       */
	if (cnt == 1) {
	    cf = d & 0x1;
	    /* note hackery here.  Access_flag(..) evaluates to either
	       0 if flag not set
	       non-zero if flag is set.
	       doing access_flag(..) != 0 casts that into either
	       0..1 in any representation of the flags register
	       (i.e. packed bit array or unpacked.)
	     */
	    ocf = ACCESS_FLAG(F_CF) != 0;
	} else
	    cf = (d >> (cnt - 1)) & 0x1;

	/* B_(8-(n+1)) .. B_(0)	 <-  b_(7) .. b_n  */
	/* note that the right hand side done by the mask
	   This is effectively done by shifting the
	   object to the right.	 The result must be masked,
	   in case the object came in and was treated
	   as a negative number.  Needed??? */

	mask = (1 << (8 - cnt)) - 1;
	res = (d >> cnt) & mask;

	/* now the high stuff which rotated around
	   into the positions B_cnt-2 .. B_0 */
	/* B_(7) .. B_(8-(n-1)) <-  b_(n-2) .. b_(0) */
	/* shift it downward, 7-(n-2) = 9-n positions.
	   and mask off the result before or'ing in.
	 */
	res |= (d << (9 - cnt));

	/* if the carry flag was set, or it in.	 */
	if (ACCESS_FLAG(F_CF)) {     /* carry flag is set */
	    /*	B_(8-n) <- cf */
	    res |= 1 << (8 - cnt);
	}
	/* set the new carry flag, based on the variable "cf" */
	CONDITIONAL_SET_FLAG(cf, F_CF);
	/* OVERFLOW is set *IFF* cnt==1, then it is the
	   xor of CF and the most significant bit.  Blecck. */
	/* parenthesized... */
	if (cnt == 1) {
	    CONDITIONAL_SET_FLAG(XOR2(ocf + ((d >> 6) & 0x2)),
				 F_OF);
	}
    }
    return (u8)res;
}

/****************************************************************************
REMARKS:
Implements the RCR instruction and side effects.
****************************************************************************/
u16 rcr_word(u16 d, u8 s)
{
    u32 res, cnt;
    u32 mask, cf, ocf = 0;

    /* rotate right through carry */
    res = d;
    if ((cnt = s % 17) != 0) {
	if (cnt == 1) {
	    cf = d & 0x1;
	    ocf = ACCESS_FLAG(F_CF) != 0;
	} else
	    cf = (d >> (cnt - 1)) & 0x1;
	mask = (1 << (16 - cnt)) - 1;
	res = (d >> cnt) & mask;
	res |= (d << (17 - cnt));
	if (ACCESS_FLAG(F_CF)) {
	    res |= 1 << (16 - cnt);
	}
	CONDITIONAL_SET_FLAG(cf, F_CF);
	if (cnt == 1) {
	    CONDITIONAL_SET_FLAG(XOR2(ocf + ((d >> 14) & 0x2)),
				 F_OF);
	}
    }
    return (u16)res;
}

/****************************************************************************
REMARKS:
Implements the RCR instruction and side effects.
****************************************************************************/
u32 rcr_long(u32 d, u8 s)
{
    u32 res, cnt;
    u32 mask, cf, ocf = 0;

    /* rotate right through carry */
    res = d;
    if ((cnt = s % 33) != 0) {
	if (cnt == 1) {
	    cf = d & 0x1;
	    ocf = ACCESS_FLAG(F_CF) != 0;
	} else
	    cf = (d >> (cnt - 1)) & 0x1;
	mask = (1 << (32 - cnt)) - 1;
	res = (d >> cnt) & mask;
	if (cnt != 1)
	    res |= (d << (33 - cnt));
	if (ACCESS_FLAG(F_CF)) {     /* carry flag is set */
	    res |= 1 << (32 - cnt);
	}
	CONDITIONAL_SET_FLAG(cf, F_CF);
	if (cnt == 1) {
	    CONDITIONAL_SET_FLAG(XOR2(ocf + ((d >> 30) & 0x2)),
				 F_OF);
	}
    }
    return res;
}

/****************************************************************************
REMARKS:
Implements the ROL instruction and side effects.
****************************************************************************/
u8 rol_byte(u8 d, u8 s)
{
    unsigned int res, cnt, mask;

    /* rotate left */
    /*
       s is the rotate distance.  It varies from 0 - 8.
       d is the byte object rotated.

       have

       CF  B_7 ... B_0

       The new rotate is done mod 8.
       Much simpler than the "rcl" or "rcr" operations.

       IF n > 0
       1) B_(7) .. B_(n)  <-  b_(8-(n+1)) .. b_(0)
       2) B_(n-1) .. B_(0) <-  b_(7) .. b_(8-n)
     */
    res = d;
    if ((cnt = s % 8) != 0) {
	/* B_(7) .. B_(n)  <-  b_(8-(n+1)) .. b_(0) */
	res = (d << cnt);

	/* B_(n-1) .. B_(0) <-	b_(7) .. b_(8-n) */
	mask = (1 << cnt) - 1;
	res |= (d >> (8 - cnt)) & mask;

	/* set the new carry flag, Note that it is the low order
	   bit of the result!!!				      */
	CONDITIONAL_SET_FLAG(res & 0x1, F_CF);
	/* OVERFLOW is set *IFF* s==1, then it is the
	   xor of CF and the most significant bit.  Blecck. */
	CONDITIONAL_SET_FLAG(s == 1 &&
			     XOR2((res & 0x1) + ((res >> 6) & 0x2)),
			     F_OF);
    } if (s != 0) {
	/* set the new carry flag, Note that it is the low order
	   bit of the result!!!				      */
	CONDITIONAL_SET_FLAG(res & 0x1, F_CF);
    }
    return (u8)res;
}

/****************************************************************************
REMARKS:
Implements the ROL instruction and side effects.
****************************************************************************/
u16 rol_word(u16 d, u8 s)
{
    unsigned int res, cnt, mask;

    res = d;
    if ((cnt = s % 16) != 0) {
	res = (d << cnt);
	mask = (1 << cnt) - 1;
	res |= (d >> (16 - cnt)) & mask;
	CONDITIONAL_SET_FLAG(res & 0x1, F_CF);
	CONDITIONAL_SET_FLAG(s == 1 &&
			     XOR2((res & 0x1) + ((res >> 14) & 0x2)),
			     F_OF);
    } if (s != 0) {
	/* set the new carry flag, Note that it is the low order
	   bit of the result!!!				      */
	CONDITIONAL_SET_FLAG(res & 0x1, F_CF);
    }
    return (u16)res;
}

/****************************************************************************
REMARKS:
Implements the ROL instruction and side effects.
****************************************************************************/
u32 rol_long(u32 d, u8 s)
{
    u32 res, cnt, mask;

    res = d;
    if ((cnt = s % 32) != 0) {
	res = (d << cnt);
	mask = (1 << cnt) - 1;
	res |= (d >> (32 - cnt)) & mask;
	CONDITIONAL_SET_FLAG(res & 0x1, F_CF);
	CONDITIONAL_SET_FLAG(s == 1 &&
			     XOR2((res & 0x1) + ((res >> 30) & 0x2)),
			     F_OF);
    } if (s != 0) {
	/* set the new carry flag, Note that it is the low order
	   bit of the result!!!				      */
	CONDITIONAL_SET_FLAG(res & 0x1, F_CF);
    }
    return res;
}

/****************************************************************************
REMARKS:
Implements the ROR instruction and side effects.
****************************************************************************/
u8 ror_byte(u8 d, u8 s)
{
    unsigned int res, cnt, mask;

    /* rotate right */
    /*
       s is the rotate distance.  It varies from 0 - 8.
       d is the byte object rotated.

       have

       B_7 ... B_0

       The rotate is done mod 8.

       IF n > 0
       1) B_(8-(n+1)) .. B_(0)	<-  b_(7) .. b_(n)
       2) B_(7) .. B_(8-n) <-  b_(n-1) .. b_(0)
     */
    res = d;
    if ((cnt = s % 8) != 0) {		/* not a typo, do nada if cnt==0 */
	/* B_(7) .. B_(8-n) <-	b_(n-1) .. b_(0) */
	res = (d << (8 - cnt));

	/* B_(8-(n+1)) .. B_(0)	 <-  b_(7) .. b_(n) */
	mask = (1 << (8 - cnt)) - 1;
	res |= (d >> (cnt)) & mask;

	/* set the new carry flag, Note that it is the low order
	   bit of the result!!!				      */
	CONDITIONAL_SET_FLAG(res & 0x80, F_CF);
	/* OVERFLOW is set *IFF* s==1, then it is the
	   xor of the two most significant bits.  Blecck. */
	CONDITIONAL_SET_FLAG(s == 1 && XOR2(res >> 6), F_OF);
    } else if (s != 0) {
	/* set the new carry flag, Note that it is the low order
	   bit of the result!!!				      */
	CONDITIONAL_SET_FLAG(res & 0x80, F_CF);
    }
    return (u8)res;
}

/****************************************************************************
REMARKS:
Implements the ROR instruction and side effects.
****************************************************************************/
u16 ror_word(u16 d, u8 s)
{
    unsigned int res, cnt, mask;

    res = d;
    if ((cnt = s % 16) != 0) {
	res = (d << (16 - cnt));
	mask = (1 << (16 - cnt)) - 1;
	res |= (d >> (cnt)) & mask;
	CONDITIONAL_SET_FLAG(res & 0x8000, F_CF);
	CONDITIONAL_SET_FLAG(s == 1 && XOR2(res >> 14), F_OF);
    } else if (s != 0) {
	/* set the new carry flag, Note that it is the low order
	   bit of the result!!!				      */
	CONDITIONAL_SET_FLAG(res & 0x8000, F_CF);
    }
    return (u16)res;
}

/****************************************************************************
REMARKS:
Implements the ROR instruction and side effects.
****************************************************************************/
u32 ror_long(u32 d, u8 s)
{
    u32 res, cnt, mask;

    res = d;
    if ((cnt = s % 32) != 0) {
	res = (d << (32 - cnt));
	mask = (1 << (32 - cnt)) - 1;
	res |= (d >> (cnt)) & mask;
	CONDITIONAL_SET_FLAG(res & 0x80000000, F_CF);
	CONDITIONAL_SET_FLAG(s == 1 && XOR2(res >> 30), F_OF);
    } else if (s != 0) {
	/* set the new carry flag, Note that it is the low order
	   bit of the result!!!				      */
	CONDITIONAL_SET_FLAG(res & 0x80000000, F_CF);
    }
    return res;
}

/****************************************************************************
REMARKS:
Implements the SHL instruction and side effects.
****************************************************************************/
u8 shl_byte(u8 d, u8 s)
{
    unsigned int cnt, res, cf;

    if (s < 8) {
	cnt = s % 8;

	/* last bit shifted out goes into carry flag */
	if (cnt > 0) {
	    res = d << cnt;
	    cf = d & (1 << (8 - cnt));
	    CONDITIONAL_SET_FLAG(cf, F_CF);
	    set_szp_flags_8((u8)res);
	} else {
	    res = (u8) d;
	}

	if (cnt == 1) {
	    /* Needs simplification. */
	    CONDITIONAL_SET_FLAG(
				    (((res & 0x80) == 0x80) ^
				     (ACCESS_FLAG(F_CF) != 0)),
	    /* was (M.x86.R_FLG&F_CF)==F_CF)), */
				    F_OF);
	} else {
	    CLEAR_FLAG(F_OF);
	}
    } else {
	res = 0;
	CONDITIONAL_SET_FLAG((d << (s-1)) & 0x80, F_CF);
	CLEAR_FLAG(F_OF);
	CLEAR_FLAG(F_SF);
	SET_FLAG(F_PF);
	SET_FLAG(F_ZF);
    }
    return (u8)res;
}

/****************************************************************************
REMARKS:
Implements the SHL instruction and side effects.
****************************************************************************/
u16 shl_word(u16 d, u8 s)
{
    unsigned int cnt, res, cf;

    if (s < 16) {
	cnt = s % 16;
	if (cnt > 0) {
	    res = d << cnt;
	    cf = d & (1 << (16 - cnt));
	    CONDITIONAL_SET_FLAG(cf, F_CF);
	    set_szp_flags_16((u16)res);
	} else {
	    res = (u16) d;
	}

	if (cnt == 1) {
	    CONDITIONAL_SET_FLAG(
				    (((res & 0x8000) == 0x8000) ^
				     (ACCESS_FLAG(F_CF) != 0)),
				    F_OF);
	} else {
	    CLEAR_FLAG(F_OF);
	}
    } else {
	res = 0;
	CONDITIONAL_SET_FLAG((d << (s-1)) & 0x8000, F_CF);
	CLEAR_FLAG(F_OF);
	CLEAR_FLAG(F_SF);
	SET_FLAG(F_PF);
	SET_FLAG(F_ZF);
    }
    return (u16)res;
}

/****************************************************************************
REMARKS:
Implements the SHL instruction and side effects.
****************************************************************************/
u32 shl_long(u32 d, u8 s)
{
    unsigned int cnt, res, cf;

    if (s < 32) {
	cnt = s % 32;
	if (cnt > 0) {
	    res = d << cnt;
	    cf = d & (1 << (32 - cnt));
	    CONDITIONAL_SET_FLAG(cf, F_CF);
	    set_szp_flags_32((u32)res);
	} else {
	    res = d;
	}
	if (cnt == 1) {
	    CONDITIONAL_SET_FLAG((((res & 0x80000000) == 0x80000000) ^
				  (ACCESS_FLAG(F_CF) != 0)), F_OF);
	} else {
	    CLEAR_FLAG(F_OF);
	}
    } else {
	res = 0;
	CONDITIONAL_SET_FLAG((d << (s-1)) & 0x80000000, F_CF);
	CLEAR_FLAG(F_OF);
	CLEAR_FLAG(F_SF);
	SET_FLAG(F_PF);
	SET_FLAG(F_ZF);
    }
    return res;
}

/****************************************************************************
REMARKS:
Implements the SHR instruction and side effects.
****************************************************************************/
u8 shr_byte(u8 d, u8 s)
{
    unsigned int cnt, res, cf;

    if (s < 8) {
	cnt = s % 8;
	if (cnt > 0) {
	    cf = d & (1 << (cnt - 1));
	    res = d >> cnt;
	    CONDITIONAL_SET_FLAG(cf, F_CF);
	    set_szp_flags_8((u8)res);
	} else {
	    res = (u8) d;
	}

	if (cnt == 1) {
	    CONDITIONAL_SET_FLAG(XOR2(res >> 6), F_OF);
	} else {
	    CLEAR_FLAG(F_OF);
	}
    } else {
	res = 0;
	CONDITIONAL_SET_FLAG((d >> (s-1)) & 0x1, F_CF);
	CLEAR_FLAG(F_OF);
	CLEAR_FLAG(F_SF);
	SET_FLAG(F_PF);
	SET_FLAG(F_ZF);
    }
    return (u8)res;
}

/****************************************************************************
REMARKS:
Implements the SHR instruction and side effects.
****************************************************************************/
u16 shr_word(u16 d, u8 s)
{
    unsigned int cnt, res, cf;

    if (s < 16) {
	cnt = s % 16;
	if (cnt > 0) {
	    cf = d & (1 << (cnt - 1));
	    res = d >> cnt;
	    CONDITIONAL_SET_FLAG(cf, F_CF);
	    set_szp_flags_16((u16)res);
	} else {
	    res = d;
	}

	if (cnt == 1) {
	    CONDITIONAL_SET_FLAG(XOR2(res >> 14), F_OF);
	} else {
	    CLEAR_FLAG(F_OF);
	}
    } else {
	res = 0;
	CLEAR_FLAG(F_CF);
	CLEAR_FLAG(F_OF);
	SET_FLAG(F_ZF);
	CLEAR_FLAG(F_SF);
	CLEAR_FLAG(F_PF);
    }
    return (u16)res;
}

/****************************************************************************
REMARKS:
Implements the SHR instruction and side effects.
****************************************************************************/
u32 shr_long(u32 d, u8 s)
{
    unsigned int cnt, res, cf;

    if (s < 32) {
	cnt = s % 32;
	if (cnt > 0) {
	    cf = d & (1 << (cnt - 1));
	    res = d >> cnt;
	    CONDITIONAL_SET_FLAG(cf, F_CF);
	    set_szp_flags_32((u32)res);
	} else {
	    res = d;
	}
	if (cnt == 1) {
	    CONDITIONAL_SET_FLAG(XOR2(res >> 30), F_OF);
	} else {
	    CLEAR_FLAG(F_OF);
	}
    } else {
	res = 0;
	CLEAR_FLAG(F_CF);
	CLEAR_FLAG(F_OF);
	SET_FLAG(F_ZF);
	CLEAR_FLAG(F_SF);
	CLEAR_FLAG(F_PF);
    }
    return res;
}

/****************************************************************************
REMARKS:
Implements the SAR instruction and side effects.
****************************************************************************/
u8 sar_byte(u8 d, u8 s)
{
    unsigned int cnt, res, cf, mask, sf;

    res = d;
    sf = d & 0x80;
    cnt = s % 8;
    if (cnt > 0 && cnt < 8) {
	mask = (1 << (8 - cnt)) - 1;
	cf = d & (1 << (cnt - 1));
	res = (d >> cnt) & mask;
	CONDITIONAL_SET_FLAG(cf, F_CF);
	if (sf) {
	    res |= ~mask;
	}
	set_szp_flags_8((u8)res);
    } else if (cnt >= 8) {
	if (sf) {
	    res = 0xff;
	    SET_FLAG(F_CF);
	    CLEAR_FLAG(F_ZF);
	    SET_FLAG(F_SF);
	    SET_FLAG(F_PF);
	} else {
	    res = 0;
	    CLEAR_FLAG(F_CF);
	    SET_FLAG(F_ZF);
	    CLEAR_FLAG(F_SF);
	    CLEAR_FLAG(F_PF);
	}
    }
    return (u8)res;
}

/****************************************************************************
REMARKS:
Implements the SAR instruction and side effects.
****************************************************************************/
u16 sar_word(u16 d, u8 s)
{
    unsigned int cnt, res, cf, mask, sf;

    sf = d & 0x8000;
    cnt = s % 16;
    res = d;
    if (cnt > 0 && cnt < 16) {
	mask = (1 << (16 - cnt)) - 1;
	cf = d & (1 << (cnt - 1));
	res = (d >> cnt) & mask;
	CONDITIONAL_SET_FLAG(cf, F_CF);
	if (sf) {
	    res |= ~mask;
	}
	set_szp_flags_16((u16)res);
    } else if (cnt >= 16) {
	if (sf) {
	    res = 0xffff;
	    SET_FLAG(F_CF);
	    CLEAR_FLAG(F_ZF);
	    SET_FLAG(F_SF);
	    SET_FLAG(F_PF);
	} else {
	    res = 0;
	    CLEAR_FLAG(F_CF);
	    SET_FLAG(F_ZF);
	    CLEAR_FLAG(F_SF);
	    CLEAR_FLAG(F_PF);
	}
    }
    return (u16)res;
}

/****************************************************************************
REMARKS:
Implements the SAR instruction and side effects.
****************************************************************************/
u32 sar_long(u32 d, u8 s)
{
    u32 cnt, res, cf, mask, sf;

    sf = d & 0x80000000;
    cnt = s % 32;
    res = d;
    if (cnt > 0 && cnt < 32) {
	mask = (1 << (32 - cnt)) - 1;
	cf = d & (1 << (cnt - 1));
	res = (d >> cnt) & mask;
	CONDITIONAL_SET_FLAG(cf, F_CF);
	if (sf) {
	    res |= ~mask;
	}
	set_szp_flags_32(res);
    } else if (cnt >= 32) {
	if (sf) {
	    res = 0xffffffff;
	    SET_FLAG(F_CF);
	    CLEAR_FLAG(F_ZF);
	    SET_FLAG(F_SF);
	    SET_FLAG(F_PF);
	} else {
	    res = 0;
	    CLEAR_FLAG(F_CF);
	    SET_FLAG(F_ZF);
	    CLEAR_FLAG(F_SF);
	    CLEAR_FLAG(F_PF);
	}
    }
    return res;
}

/****************************************************************************
REMARKS:
Implements the SHLD instruction and side effects.
****************************************************************************/
u16 shld_word (u16 d, u16 fill, u8 s)
{
    unsigned int cnt, res, cf;

    if (s < 16) {
	cnt = s % 16;
	if (cnt > 0) {
	    res = (d << cnt) | (fill >> (16-cnt));
	    cf = d & (1 << (16 - cnt));
	    CONDITIONAL_SET_FLAG(cf, F_CF);
	    set_szp_flags_16((u16)res);
	} else {
	    res = d;
	}
	if (cnt == 1) {
	    CONDITIONAL_SET_FLAG((((res & 0x8000) == 0x8000) ^
				  (ACCESS_FLAG(F_CF) != 0)), F_OF);
	} else {
	    CLEAR_FLAG(F_OF);
	}
    } else {
	res = 0;
	CONDITIONAL_SET_FLAG((d << (s-1)) & 0x8000, F_CF);
	CLEAR_FLAG(F_OF);
	CLEAR_FLAG(F_SF);
	SET_FLAG(F_PF);
	SET_FLAG(F_ZF);
    }
    return (u16)res;
}

/****************************************************************************
REMARKS:
Implements the SHLD instruction and side effects.
****************************************************************************/
u32 shld_long (u32 d, u32 fill, u8 s)
{
    unsigned int cnt, res, cf;

    if (s < 32) {
	cnt = s % 32;
	if (cnt > 0) {
	    res = (d << cnt) | (fill >> (32-cnt));
	    cf = d & (1 << (32 - cnt));
	    CONDITIONAL_SET_FLAG(cf, F_CF);
	    set_szp_flags_32((u32)res);
	} else {
	    res = d;
	}
	if (cnt == 1) {
	    CONDITIONAL_SET_FLAG((((res & 0x80000000) == 0x80000000) ^
				  (ACCESS_FLAG(F_CF) != 0)), F_OF);
	} else {
	    CLEAR_FLAG(F_OF);
	}
    } else {
	res = 0;
	CONDITIONAL_SET_FLAG((d << (s-1)) & 0x80000000, F_CF);
	CLEAR_FLAG(F_OF);
	CLEAR_FLAG(F_SF);
	SET_FLAG(F_PF);
	SET_FLAG(F_ZF);
    }
    return res;
}

/****************************************************************************
REMARKS:
Implements the SHRD instruction and side effects.
****************************************************************************/
u16 shrd_word (u16 d, u16 fill, u8 s)
{
    unsigned int cnt, res, cf;

    if (s < 16) {
	cnt = s % 16;
	if (cnt > 0) {
	    cf = d & (1 << (cnt - 1));
	    res = (d >> cnt) | (fill << (16 - cnt));
	    CONDITIONAL_SET_FLAG(cf, F_CF);
	    set_szp_flags_16((u16)res);
	} else {
	    res = d;
	}

	if (cnt == 1) {
	    CONDITIONAL_SET_FLAG(XOR2(res >> 14), F_OF);
	} else {
	    CLEAR_FLAG(F_OF);
	}
    } else {
	res = 0;
	CLEAR_FLAG(F_CF);
	CLEAR_FLAG(F_OF);
	SET_FLAG(F_ZF);
	CLEAR_FLAG(F_SF);
	CLEAR_FLAG(F_PF);
    }
    return (u16)res;
}

/****************************************************************************
REMARKS:
Implements the SHRD instruction and side effects.
****************************************************************************/
u32 shrd_long (u32 d, u32 fill, u8 s)
{
    unsigned int cnt, res, cf;

    if (s < 32) {
	cnt = s % 32;
	if (cnt > 0) {
	    cf = d & (1 << (cnt - 1));
	    res = (d >> cnt) | (fill << (32 - cnt));
	    CONDITIONAL_SET_FLAG(cf, F_CF);
	    set_szp_flags_32((u32)res);
	} else {
	    res = d;
	}
	if (cnt == 1) {
	    CONDITIONAL_SET_FLAG(XOR2(res >> 30), F_OF);
	} else {
	    CLEAR_FLAG(F_OF);
	}
    } else {
	res = 0;
	CLEAR_FLAG(F_CF);
	CLEAR_FLAG(F_OF);
	SET_FLAG(F_ZF);
	CLEAR_FLAG(F_SF);
	CLEAR_FLAG(F_PF);
    }
    return res;
}

/****************************************************************************
REMARKS:
Implements the SBB instruction and side effects.
****************************************************************************/
u8 sbb_byte(u8 d, u8 s)
{
    u32 res;   /* all operands in native machine order */
    u32 bc;

    if (ACCESS_FLAG(F_CF))
	res = d - s - 1;
    else
	res = d - s;
    set_szp_flags_8((u8)res);

    /* calculate the borrow chain.  See note at top */
    bc = (res & (~d | s)) | (~d & s);
    CONDITIONAL_SET_FLAG(bc & 0x80, F_CF);
    CONDITIONAL_SET_FLAG(XOR2(bc >> 6), F_OF);
    CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
    return (u8)res;
}

/****************************************************************************
REMARKS:
Implements the SBB instruction and side effects.
****************************************************************************/
u16 sbb_word(u16 d, u16 s)
{
    u32 res;   /* all operands in native machine order */
    u32 bc;

    if (ACCESS_FLAG(F_CF))
	res = d - s - 1;
    else
	res = d - s;
    set_szp_flags_16((u16)res);

    /* calculate the borrow chain.  See note at top */
    bc = (res & (~d | s)) | (~d & s);
    CONDITIONAL_SET_FLAG(bc & 0x8000, F_CF);
    CONDITIONAL_SET_FLAG(XOR2(bc >> 14), F_OF);
    CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
    return (u16)res;
}

/****************************************************************************
REMARKS:
Implements the SBB instruction and side effects.
****************************************************************************/
u32 sbb_long(u32 d, u32 s)
{
    u32 res;   /* all operands in native machine order */
    u32 bc;

    if (ACCESS_FLAG(F_CF))
	res = d - s - 1;
    else
	res = d - s;

    set_szp_flags_32(res);

    /* calculate the borrow chain.  See note at top */
    bc = (res & (~d | s)) | (~d & s);
    CONDITIONAL_SET_FLAG(bc & 0x80000000, F_CF);
    CONDITIONAL_SET_FLAG(XOR2(bc >> 30), F_OF);
    CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
    return res;
}

/****************************************************************************
REMARKS:
Implements the SUB instruction and side effects.
****************************************************************************/
u8 sub_byte(u8 d, u8 s)
{
    u32 res;   /* all operands in native machine order */
    u32 bc;

    res = d - s;
    set_szp_flags_8((u8)res);

    /* calculate the borrow chain.  See note at top */
    bc = (res & (~d | s)) | (~d & s);
    CONDITIONAL_SET_FLAG(bc & 0x80, F_CF);
    CONDITIONAL_SET_FLAG(XOR2(bc >> 6), F_OF);
    CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
    return (u8)res;
}

/****************************************************************************
REMARKS:
Implements the SUB instruction and side effects.
****************************************************************************/
u16 sub_word(u16 d, u16 s)
{
    u32 res;   /* all operands in native machine order */
    u32 bc;

    res = d - s;
    set_szp_flags_16((u16)res);

    /* calculate the borrow chain.  See note at top */
    bc = (res & (~d | s)) | (~d & s);
    CONDITIONAL_SET_FLAG(bc & 0x8000, F_CF);
    CONDITIONAL_SET_FLAG(XOR2(bc >> 14), F_OF);
    CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
    return (u16)res;
}

/****************************************************************************
REMARKS:
Implements the SUB instruction and side effects.
****************************************************************************/
u32 sub_long(u32 d, u32 s)
{
    u32 res;   /* all operands in native machine order */
    u32 bc;

    res = d - s;
    set_szp_flags_32(res);

    /* calculate the borrow chain.  See note at top */
    bc = (res & (~d | s)) | (~d & s);
    CONDITIONAL_SET_FLAG(bc & 0x80000000, F_CF);
    CONDITIONAL_SET_FLAG(XOR2(bc >> 30), F_OF);
    CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
    return res;
}

/****************************************************************************
REMARKS:
Implements the TEST instruction and side effects.
****************************************************************************/
void test_byte(u8 d, u8 s)
{
    u32 res;   /* all operands in native machine order */

    res = d & s;

    CLEAR_FLAG(F_OF);
    set_szp_flags_8((u8)res);
    /* AF == dont care */
    CLEAR_FLAG(F_CF);
}

/****************************************************************************
REMARKS:
Implements the TEST instruction and side effects.
****************************************************************************/
void test_word(u16 d, u16 s)
{
    u32 res;   /* all operands in native machine order */

    res = d & s;

    CLEAR_FLAG(F_OF);
    set_szp_flags_16((u16)res);
    /* AF == dont care */
    CLEAR_FLAG(F_CF);
}

/****************************************************************************
REMARKS:
Implements the TEST instruction and side effects.
****************************************************************************/
void test_long(u32 d, u32 s)
{
    u32 res;   /* all operands in native machine order */

    res = d & s;

    CLEAR_FLAG(F_OF);
    set_szp_flags_32(res);
    /* AF == dont care */
    CLEAR_FLAG(F_CF);
}

/****************************************************************************
REMARKS:
Implements the XOR instruction and side effects.
****************************************************************************/
u8 xor_byte(u8 d, u8 s)
{
    u8 res;    /* all operands in native machine order */

    res = d ^ s;
    no_carry_byte_side_eff(res);
    return res;
}

/****************************************************************************
REMARKS:
Implements the XOR instruction and side effects.
****************************************************************************/
u16 xor_word(u16 d, u16 s)
{
    u16 res;   /* all operands in native machine order */

    res = d ^ s;
    no_carry_word_side_eff(res);
    return res;
}

/****************************************************************************
REMARKS:
Implements the XOR instruction and side effects.
****************************************************************************/
u32 xor_long(u32 d, u32 s)
{
    u32 res;   /* all operands in native machine order */

    res = d ^ s;
    no_carry_long_side_eff(res);
    return res;
}

/****************************************************************************
REMARKS:
Implements the IMUL instruction and side effects.
****************************************************************************/
void imul_byte(u8 s)
{
    s16 res = (s16)((s8)M.x86.R_AL * (s8)s);

    M.x86.R_AX = res;
    if (((M.x86.R_AL & 0x80) == 0 && M.x86.R_AH == 0x00) ||
	((M.x86.R_AL & 0x80) != 0 && M.x86.R_AH == 0xFF)) {
	CLEAR_FLAG(F_CF);
	CLEAR_FLAG(F_OF);
    } else {
	SET_FLAG(F_CF);
	SET_FLAG(F_OF);
    }
}

/****************************************************************************
REMARKS:
Implements the IMUL instruction and side effects.
****************************************************************************/
void imul_word(u16 s)
{
    s32 res = (s16)M.x86.R_AX * (s16)s;

    M.x86.R_AX = (u16)res;
    M.x86.R_DX = (u16)(res >> 16);
    if (((M.x86.R_AX & 0x8000) == 0 && M.x86.R_DX == 0x0000) ||
	((M.x86.R_AX & 0x8000) != 0 && M.x86.R_DX == 0xFFFF)) {
	CLEAR_FLAG(F_CF);
	CLEAR_FLAG(F_OF);
    } else {
	SET_FLAG(F_CF);
	SET_FLAG(F_OF);
    }
}

/****************************************************************************
REMARKS:
Implements the IMUL instruction and side effects.
****************************************************************************/
void imul_long_direct(u32 *res_lo, u32* res_hi,u32 d, u32 s)
{
#ifdef	__HAS_LONG_LONG__
    s64 res = (s32)d * (s32)s;

    *res_lo = (u32)res;
    *res_hi = (u32)(res >> 32);
#else
    u32 d_lo,d_hi,d_sign;
    u32 s_lo,s_hi,s_sign;
    u32 rlo_lo,rlo_hi,rhi_lo;

    if ((d_sign = d & 0x80000000) != 0)
	d = -d;
    d_lo = d & 0xFFFF;
    d_hi = d >> 16;
    if ((s_sign = s & 0x80000000) != 0)
	s = -s;
    s_lo = s & 0xFFFF;
    s_hi = s >> 16;
    rlo_lo = d_lo * s_lo;
    rlo_hi = (d_hi * s_lo + d_lo * s_hi) + (rlo_lo >> 16);
    rhi_lo = d_hi * s_hi + (rlo_hi >> 16);
    *res_lo = (rlo_hi << 16) | (rlo_lo & 0xFFFF);
    *res_hi = rhi_lo;
    if (d_sign != s_sign) {
	d = ~*res_lo;
	s = (((d & 0xFFFF) + 1) >> 16) + (d >> 16);
	*res_lo = ~*res_lo+1;
	*res_hi = ~*res_hi+(s >> 16);
	}
#endif
}

/****************************************************************************
REMARKS:
Implements the IMUL instruction and side effects.
****************************************************************************/
void imul_long(u32 s)
{
    imul_long_direct(&M.x86.R_EAX,&M.x86.R_EDX,M.x86.R_EAX,s);
    if (((M.x86.R_EAX & 0x80000000) == 0 && M.x86.R_EDX == 0x00000000) ||
	((M.x86.R_EAX & 0x80000000) != 0 && M.x86.R_EDX == 0xFFFFFFFF)) {
	CLEAR_FLAG(F_CF);
	CLEAR_FLAG(F_OF);
    } else {
	SET_FLAG(F_CF);
	SET_FLAG(F_OF);
    }
}

/****************************************************************************
REMARKS:
Implements the MUL instruction and side effects.
****************************************************************************/
void mul_byte(u8 s)
{
    u16 res = (u16)(M.x86.R_AL * s);

    M.x86.R_AX = res;
    if (M.x86.R_AH == 0) {
	CLEAR_FLAG(F_CF);
	CLEAR_FLAG(F_OF);
    } else {
	SET_FLAG(F_CF);
	SET_FLAG(F_OF);
    }
}

/****************************************************************************
REMARKS:
Implements the MUL instruction and side effects.
****************************************************************************/
void mul_word(u16 s)
{
    u32 res = M.x86.R_AX * s;

    M.x86.R_AX = (u16)res;
    M.x86.R_DX = (u16)(res >> 16);
    if (M.x86.R_DX == 0) {
	CLEAR_FLAG(F_CF);
	CLEAR_FLAG(F_OF);
    } else {
	SET_FLAG(F_CF);
	SET_FLAG(F_OF);
    }
}

/****************************************************************************
REMARKS:
Implements the MUL instruction and side effects.
****************************************************************************/
void mul_long(u32 s)
{
#ifdef	__HAS_LONG_LONG__
    u64 res = (u32)M.x86.R_EAX * (u32)s;

    M.x86.R_EAX = (u32)res;
    M.x86.R_EDX = (u32)(res >> 32);
#else
    u32 a,a_lo,a_hi;
    u32 s_lo,s_hi;
    u32 rlo_lo,rlo_hi,rhi_lo;

    a = M.x86.R_EAX;
    a_lo = a & 0xFFFF;
    a_hi = a >> 16;
    s_lo = s & 0xFFFF;
    s_hi = s >> 16;
    rlo_lo = a_lo * s_lo;
    rlo_hi = (a_hi * s_lo + a_lo * s_hi) + (rlo_lo >> 16);
    rhi_lo = a_hi * s_hi + (rlo_hi >> 16);
    M.x86.R_EAX = (rlo_hi << 16) | (rlo_lo & 0xFFFF);
    M.x86.R_EDX = rhi_lo;
#endif
    if (M.x86.R_EDX == 0) {
	CLEAR_FLAG(F_CF);
	CLEAR_FLAG(F_OF);
    } else {
	SET_FLAG(F_CF);
	SET_FLAG(F_OF);
    }
}

/****************************************************************************
REMARKS:
Implements the IDIV instruction and side effects.
****************************************************************************/
void idiv_byte(u8 s)
{
    s32 dvd, div, mod;

    dvd = (s16)M.x86.R_AX;
    if (s == 0) {
	x86emu_intr_raise(0);
	return;
    }
    div = dvd / (s8)s;
    mod = dvd % (s8)s;
    if (abs(div) > 0x7f) {
	x86emu_intr_raise(0);
	return;
    }
    M.x86.R_AL = (s8) div;
    M.x86.R_AH = (s8) mod;
}

/****************************************************************************
REMARKS:
Implements the IDIV instruction and side effects.
****************************************************************************/
void idiv_word(u16 s)
{
    s32 dvd, div, mod;

    dvd = (((s32)M.x86.R_DX) << 16) | M.x86.R_AX;
    if (s == 0) {
	x86emu_intr_raise(0);
	return;
    }
    div = dvd / (s16)s;
    mod = dvd % (s16)s;
    if (abs(div) > 0x7fff) {
	x86emu_intr_raise(0);
	return;
    }
    CLEAR_FLAG(F_CF);
    CLEAR_FLAG(F_SF);
    CONDITIONAL_SET_FLAG(div == 0, F_ZF);
    set_parity_flag(mod);

    M.x86.R_AX = (u16)div;
    M.x86.R_DX = (u16)mod;
}

/****************************************************************************
REMARKS:
Implements the IDIV instruction and side effects.
****************************************************************************/
void idiv_long(u32 s)
{
#ifdef	__HAS_LONG_LONG__
    s64 dvd, div, mod;

    dvd = (((s64)M.x86.R_EDX) << 32) | M.x86.R_EAX;
    if (s == 0) {
	x86emu_intr_raise(0);
	return;
    }
    div = dvd / (s32)s;
    mod = dvd % (s32)s;
    if (abs(div) > 0x7fffffff) {
	x86emu_intr_raise(0);
	return;
    }
#else
    s32 div = 0, mod;
    s32 h_dvd = M.x86.R_EDX;
    u32 l_dvd = M.x86.R_EAX;
    u32 abs_s = s & 0x7FFFFFFF;
    u32 abs_h_dvd = h_dvd & 0x7FFFFFFF;
    u32 h_s = abs_s >> 1;
    u32 l_s = abs_s << 31;
    int counter = 31;
    int carry;

    if (s == 0) {
	x86emu_intr_raise(0);
	return;
    }
    do {
	div <<= 1;
	carry = (l_dvd >= l_s) ? 0 : 1;

	if (abs_h_dvd < (h_s + carry)) {
	    h_s >>= 1;
	    l_s = abs_s << (--counter);
	    continue;
	} else {
	    abs_h_dvd -= (h_s + carry);
	    l_dvd = carry ? ((0xFFFFFFFF - l_s) + l_dvd + 1)
		: (l_dvd - l_s);
	    h_s >>= 1;
	    l_s = abs_s << (--counter);
	    div |= 1;
	    continue;
	}

    } while (counter > -1);
    /* overflow */
    if (abs_h_dvd || (l_dvd > abs_s)) {
	x86emu_intr_raise(0);
	return;
    }
    /* sign */
    div |= ((h_dvd & 0x10000000) ^ (s & 0x10000000));
    mod = l_dvd;

#endif
    CLEAR_FLAG(F_CF);
    CLEAR_FLAG(F_AF);
    CLEAR_FLAG(F_SF);
    SET_FLAG(F_ZF);
    set_parity_flag(mod);

    M.x86.R_EAX = (u32)div;
    M.x86.R_EDX = (u32)mod;
}

/****************************************************************************
REMARKS:
Implements the DIV instruction and side effects.
****************************************************************************/
void div_byte(u8 s)
{
    u32 dvd, div, mod;

    dvd = M.x86.R_AX;
    if (s == 0) {
	x86emu_intr_raise(0);
	return;
    }
    div = dvd / (u8)s;
    mod = dvd % (u8)s;
    if (abs(div) > 0xff) {
	x86emu_intr_raise(0);
	return;
    }
    M.x86.R_AL = (u8)div;
    M.x86.R_AH = (u8)mod;
}

/****************************************************************************
REMARKS:
Implements the DIV instruction and side effects.
****************************************************************************/
void div_word(u16 s)
{
    u32 dvd, div, mod;

    dvd = (((u32)M.x86.R_DX) << 16) | M.x86.R_AX;
    if (s == 0) {
	x86emu_intr_raise(0);
	return;
    }
    div = dvd / (u16)s;
    mod = dvd % (u16)s;
    if (abs(div) > 0xffff) {
	x86emu_intr_raise(0);
	return;
    }
    CLEAR_FLAG(F_CF);
    CLEAR_FLAG(F_SF);
    CONDITIONAL_SET_FLAG(div == 0, F_ZF);
    set_parity_flag(mod);

    M.x86.R_AX = (u16)div;
    M.x86.R_DX = (u16)mod;
}

/****************************************************************************
REMARKS:
Implements the DIV instruction and side effects.
****************************************************************************/
void div_long(u32 s)
{
#ifdef	__HAS_LONG_LONG__
    u64 dvd, div, mod;

    dvd = (((u64)M.x86.R_EDX) << 32) | M.x86.R_EAX;
    if (s == 0) {
	x86emu_intr_raise(0);
	return;
    }
    div = dvd / (u32)s;
    mod = dvd % (u32)s;
    if (abs(div) > 0xffffffff) {
	x86emu_intr_raise(0);
	return;
    }
#else
    s32 div = 0, mod;
    s32 h_dvd = M.x86.R_EDX;
    u32 l_dvd = M.x86.R_EAX;

    u32 h_s = s;
    u32 l_s = 0;
    int counter = 32;
    int carry;

    if (s == 0) {
	x86emu_intr_raise(0);
	return;
    }
    do {
	div <<= 1;
	carry = (l_dvd >= l_s) ? 0 : 1;

	if (h_dvd < (h_s + carry)) {
	    h_s >>= 1;
	    l_s = s << (--counter);
	    continue;
	} else {
	    h_dvd -= (h_s + carry);
	    l_dvd = carry ? ((0xFFFFFFFF - l_s) + l_dvd + 1)
		: (l_dvd - l_s);
	    h_s >>= 1;
	    l_s = s << (--counter);
	    div |= 1;
	    continue;
	}

    } while (counter > -1);
    /* overflow */
    if (h_dvd || (l_dvd > s)) {
	x86emu_intr_raise(0);
	return;
    }
    mod = l_dvd;
#endif
    CLEAR_FLAG(F_CF);
    CLEAR_FLAG(F_AF);
    CLEAR_FLAG(F_SF);
    SET_FLAG(F_ZF);
    set_parity_flag(mod);

    M.x86.R_EAX = (u32)div;
    M.x86.R_EDX = (u32)mod;
}

/****************************************************************************
REMARKS:
Implements the IN string instruction and side effects.
****************************************************************************/

static void single_in(int size)
{
    if(size == 1)
	store_data_byte_abs(M.x86.R_ES, M.x86.R_DI,(*sys_inb)(M.x86.R_DX));
    else if (size == 2)
	store_data_word_abs(M.x86.R_ES, M.x86.R_DI,(*sys_inw)(M.x86.R_DX));
    else
	store_data_long_abs(M.x86.R_ES, M.x86.R_DI,(*sys_inl)(M.x86.R_DX));
}

void ins(int size)
{
    int inc = size;

    if (ACCESS_FLAG(F_DF)) {
	inc = -size;
    }
    if (M.x86.mode & (SYSMODE_PREFIX_REPE | SYSMODE_PREFIX_REPNE)) {
	/* dont care whether REPE or REPNE */
	/* in until CX is ZERO. */
	u32 count = ((M.x86.mode & SYSMODE_PREFIX_DATA) ?
		     M.x86.R_ECX : M.x86.R_CX);

	while (count--) {
	  single_in(size);
	  M.x86.R_DI += inc;
	  }
	M.x86.R_CX = 0;
	if (M.x86.mode & SYSMODE_PREFIX_DATA) {
	    M.x86.R_ECX = 0;
	}
	M.x86.mode &= ~(SYSMODE_PREFIX_REPE | SYSMODE_PREFIX_REPNE);
    } else {
	single_in(size);
	M.x86.R_DI += inc;
    }
}

/****************************************************************************
REMARKS:
Implements the OUT string instruction and side effects.
****************************************************************************/

static void single_out(int size)
{
     if(size == 1)
       (*sys_outb)(M.x86.R_DX,fetch_data_byte_abs(M.x86.R_ES, M.x86.R_SI));
     else if (size == 2)
       (*sys_outw)(M.x86.R_DX,fetch_data_word_abs(M.x86.R_ES, M.x86.R_SI));
     else
       (*sys_outl)(M.x86.R_DX,fetch_data_long_abs(M.x86.R_ES, M.x86.R_SI));
}

void outs(int size)
{
    int inc = size;

    if (ACCESS_FLAG(F_DF)) {
	inc = -size;
    }
    if (M.x86.mode & (SYSMODE_PREFIX_REPE | SYSMODE_PREFIX_REPNE)) {
	/* dont care whether REPE or REPNE */
	/* out until CX is ZERO. */
	u32 count = ((M.x86.mode & SYSMODE_PREFIX_DATA) ?
		     M.x86.R_ECX : M.x86.R_CX);
	while (count--) {
	  single_out(size);
	  M.x86.R_SI += inc;
	  }
	M.x86.R_CX = 0;
	if (M.x86.mode & SYSMODE_PREFIX_DATA) {
	    M.x86.R_ECX = 0;
	}
	M.x86.mode &= ~(SYSMODE_PREFIX_REPE | SYSMODE_PREFIX_REPNE);
    } else {
	single_out(size);
	M.x86.R_SI += inc;
    }
}

/****************************************************************************
PARAMETERS:
addr	- Address to fetch word from

REMARKS:
Fetches a word from emulator memory using an absolute address.
****************************************************************************/
u16 mem_access_word(int addr)
{
DB( if (CHECK_MEM_ACCESS())
      x86emu_check_mem_access(addr);)
    return (*sys_rdw)(addr);
}

/****************************************************************************
REMARKS:
Pushes a word onto the stack.

NOTE: Do not inline this, as (*sys_wrX) is already inline!
****************************************************************************/
void push_word(u16 w)
{
DB( if (CHECK_SP_ACCESS())
      x86emu_check_sp_access();)
    M.x86.R_SP -= 2;
    (*sys_wrw)(((u32)M.x86.R_SS << 4)  + M.x86.R_SP, w);
}

/****************************************************************************
REMARKS:
Pushes a long onto the stack.

NOTE: Do not inline this, as (*sys_wrX) is already inline!
****************************************************************************/
void push_long(u32 w)
{
DB( if (CHECK_SP_ACCESS())
      x86emu_check_sp_access();)
    M.x86.R_SP -= 4;
    (*sys_wrl)(((u32)M.x86.R_SS << 4)  + M.x86.R_SP, w);
}

/****************************************************************************
REMARKS:
Pops a word from the stack.

NOTE: Do not inline this, as (*sys_rdX) is already inline!
****************************************************************************/
u16 pop_word(void)
{
    u16 res;

DB( if (CHECK_SP_ACCESS())
      x86emu_check_sp_access();)
    res = (*sys_rdw)(((u32)M.x86.R_SS << 4)  + M.x86.R_SP);
    M.x86.R_SP += 2;
    return res;
}

/****************************************************************************
REMARKS:
Pops a long from the stack.

NOTE: Do not inline this, as (*sys_rdX) is already inline!
****************************************************************************/
u32 pop_long(void)
{
    u32 res;

DB( if (CHECK_SP_ACCESS())
      x86emu_check_sp_access();)
    res = (*sys_rdl)(((u32)M.x86.R_SS << 4)  + M.x86.R_SP);
    M.x86.R_SP += 4;
    return res;
}
