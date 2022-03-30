/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef _SETJMP_H_
#define _SETJMP_H_

struct jmp_buf_data {
	/*
	 * We're not sure how long this should be:
	 *
	 *   amd64: 200 bytes
	 *   arm64: 392 bytes
	 *   armhf: 392 bytes
	 *
	 * So allow space for all of those, plus some extra.
	 * We don't need to worry about 16-byte alignment, since this does not
	 * run on Windows.
	 */
	ulong data[128];
};

typedef struct jmp_buf_data jmp_buf[1];

/*
 * We have to directly link with the system versions of
 * setjmp/longjmp, because setjmp must not return as otherwise
 * the stack may become invalid.
 */
int setjmp(jmp_buf jmp);
__noreturn void longjmp(jmp_buf jmp, int ret);

#endif /* _SETJMP_H_ */
