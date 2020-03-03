/*
 * This program is used to generate definitions needed by
 * assembly language modules.
 *
 */

#include <linux/kbuild.h>
#include <linux/thread_info.h>

int main(void)
{
	DEFINE(THREAD_INFO_SIZE, sizeof(struct thread_info));
	return 0;
}
