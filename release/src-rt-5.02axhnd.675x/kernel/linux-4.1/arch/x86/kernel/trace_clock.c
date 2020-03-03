/*
 * X86 trace clocks
 */
#include <asm/trace_clock.h>
#include <asm/barrier.h>
#include <asm/msr.h>

/*
 * trace_clock_x86_tsc(): A clock that is just the cycle counter.
 *
 * Unlike the other clocks, this is not in nanoseconds.
 */
u64 notrace trace_clock_x86_tsc(void)
{
	u64 ret;

	rdtsc_barrier();
	rdtscll(ret);

	return ret;
}
