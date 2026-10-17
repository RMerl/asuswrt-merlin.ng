/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <asm/setjmp.h>

/**
 * struct resume_data - data for resume after interrupt
 */
struct resume_data {
	/** @jump: longjmp buffer */
	jmp_buf jump;
	/** @code: exception code */
	ulong code;
};

/**
 * set_resume() - set longjmp buffer for resuming after exception
 *
 * By calling this function it is possible to use a long jump to catch an
 * exception. The caller sets the long jump buffer with set_resume() and then
 * executes setjmp(). If an exception occurs, the code will return to the
 * setjmp caller(). The exception code will be returned in @data->code.
 *
 * After the critical operation call set_resume(NULL) so that an exception in
 * another part of the code will not accidently invoke the long jump.
 *
 * .. code-block:: c
 *
 *     // This example shows how to use set_resume().
 *
 *     struct resume_data resume;
 *     int ret;
 *
 *     set_resume(&resume);
 *     ret = setjmp(resume.jump);
 *     if (ret) {
 *          printf("An exception %ld occurred\n", resume.code);
 *     } else {
 *          // Do what might raise an exception here.
 *     }
 *     set_resume(NULL);
 *
 * @data:	pointer to structure with longjmp address
 * Return:	0 before an exception, 1 after an exception occurred
 */
void set_resume(struct resume_data *data);
