#ifndef _LINUX_TIMEKEEPING_H
#define _LINUX_TIMEKEEPING_H

/* Included from linux/ktime.h */

void timekeeping_init(void);
extern int timekeeping_suspended;

/*
 * Get and set timeofday
 */
extern void do_gettimeofday(struct timeval *tv);
extern int do_settimeofday64(const struct timespec64 *ts);
extern int do_sys_settimeofday(const struct timespec *tv,
			       const struct timezone *tz);

/*
 * Kernel time accessors
 */
unsigned long get_seconds(void);
struct timespec current_kernel_time(void);
/* does not take xtime_lock */
struct timespec __current_kernel_time(void);

/*
 * timespec based interfaces
 */
struct timespec64 get_monotonic_coarse64(void);
extern void getrawmonotonic64(struct timespec64 *ts);
extern void ktime_get_ts64(struct timespec64 *ts);
extern time64_t ktime_get_seconds(void);
extern time64_t ktime_get_real_seconds(void);

extern int __getnstimeofday64(struct timespec64 *tv);
extern void getnstimeofday64(struct timespec64 *tv);
extern void getboottime64(struct timespec64 *ts);

#if BITS_PER_LONG == 64
/**
 * Deprecated. Use do_settimeofday64().
 */
static inline int do_settimeofday(const struct timespec *ts)
{
	return do_settimeofday64(ts);
}

static inline int __getnstimeofday(struct timespec *ts)
{
	return __getnstimeofday64(ts);
}

static inline void getnstimeofday(struct timespec *ts)
{
	getnstimeofday64(ts);
}

static inline void ktime_get_ts(struct timespec *ts)
{
	ktime_get_ts64(ts);
}

static inline void ktime_get_real_ts(struct timespec *ts)
{
	getnstimeofday64(ts);
}

static inline void getrawmonotonic(struct timespec *ts)
{
	getrawmonotonic64(ts);
}

static inline struct timespec get_monotonic_coarse(void)
{
	return get_monotonic_coarse64();
}

static inline void getboottime(struct timespec *ts)
{
	return getboottime64(ts);
}
#else
/**
 * Deprecated. Use do_settimeofday64().
 */
static inline int do_settimeofday(const struct timespec *ts)
{
	struct timespec64 ts64;

	ts64 = timespec_to_timespec64(*ts);
	return do_settimeofday64(&ts64);
}

static inline int __getnstimeofday(struct timespec *ts)
{
	struct timespec64 ts64;
	int ret = __getnstimeofday64(&ts64);

	*ts = timespec64_to_timespec(ts64);
	return ret;
}

static inline void getnstimeofday(struct timespec *ts)
{
	struct timespec64 ts64;

	getnstimeofday64(&ts64);
	*ts = timespec64_to_timespec(ts64);
}

static inline void ktime_get_ts(struct timespec *ts)
{
	struct timespec64 ts64;

	ktime_get_ts64(&ts64);
	*ts = timespec64_to_timespec(ts64);
}

static inline void ktime_get_real_ts(struct timespec *ts)
{
	struct timespec64 ts64;

	getnstimeofday64(&ts64);
	*ts = timespec64_to_timespec(ts64);
}

static inline void getrawmonotonic(struct timespec *ts)
{
	struct timespec64 ts64;

	getrawmonotonic64(&ts64);
	*ts = timespec64_to_timespec(ts64);
}

static inline struct timespec get_monotonic_coarse(void)
{
	return timespec64_to_timespec(get_monotonic_coarse64());
}

static inline void getboottime(struct timespec *ts)
{
	struct timespec64 ts64;

	getboottime64(&ts64);
	*ts = timespec64_to_timespec(ts64);
}
#endif

#define do_posix_clock_monotonic_gettime(ts) ktime_get_ts(ts)
#define ktime_get_real_ts64(ts)	getnstimeofday64(ts)

/*
 * ktime_t based interfaces
 */

enum tk_offsets {
	TK_OFFS_REAL,
	TK_OFFS_BOOT,
	TK_OFFS_TAI,
	TK_OFFS_MAX,
};

extern ktime_t ktime_get(void);
extern ktime_t ktime_get_with_offset(enum tk_offsets offs);
extern ktime_t ktime_mono_to_any(ktime_t tmono, enum tk_offsets offs);
extern ktime_t ktime_get_raw(void);

/**
 * ktime_get_real - get the real (wall-) time in ktime_t format
 */
static inline ktime_t ktime_get_real(void)
{
	return ktime_get_with_offset(TK_OFFS_REAL);
}

/**
 * ktime_get_boottime - Returns monotonic time since boot in ktime_t format
 *
 * This is similar to CLOCK_MONTONIC/ktime_get, but also includes the
 * time spent in suspend.
 */
static inline ktime_t ktime_get_boottime(void)
{
	return ktime_get_with_offset(TK_OFFS_BOOT);
}

/**
 * ktime_get_clocktai - Returns the TAI time of day in ktime_t format
 */
static inline ktime_t ktime_get_clocktai(void)
{
	return ktime_get_with_offset(TK_OFFS_TAI);
}

/**
 * ktime_mono_to_real - Convert monotonic time to clock realtime
 */
static inline ktime_t ktime_mono_to_real(ktime_t mono)
{
	return ktime_mono_to_any(mono, TK_OFFS_REAL);
}

static inline u64 ktime_get_ns(void)
{
	return ktime_to_ns(ktime_get());
}

static inline u64 ktime_get_real_ns(void)
{
	return ktime_to_ns(ktime_get_real());
}

static inline u64 ktime_get_boot_ns(void)
{
	return ktime_to_ns(ktime_get_boottime());
}

static inline u64 ktime_get_tai_ns(void)
{
	return ktime_to_ns(ktime_get_clocktai());
}

static inline u64 ktime_get_raw_ns(void)
{
	return ktime_to_ns(ktime_get_raw());
}

extern u64 ktime_get_mono_fast_ns(void);
extern u64 ktime_get_raw_fast_ns(void);

/*
 * Timespec interfaces utilizing the ktime based ones
 */
static inline void get_monotonic_boottime(struct timespec *ts)
{
	*ts = ktime_to_timespec(ktime_get_boottime());
}

static inline void get_monotonic_boottime64(struct timespec64 *ts)
{
	*ts = ktime_to_timespec64(ktime_get_boottime());
}

static inline void timekeeping_clocktai(struct timespec *ts)
{
	*ts = ktime_to_timespec(ktime_get_clocktai());
}

/*
 * RTC specific
 */
extern bool timekeeping_rtc_skipsuspend(void);
extern bool timekeeping_rtc_skipresume(void);

extern void timekeeping_inject_sleeptime64(struct timespec64 *delta);

/*
 * PPS accessor
 */
extern void getnstime_raw_and_real(struct timespec *ts_raw,
				   struct timespec *ts_real);

/*
 * Persistent clock related interfaces
 */
extern int persistent_clock_is_local;

extern void read_persistent_clock(struct timespec *ts);
extern void read_persistent_clock64(struct timespec64 *ts);
extern void read_boot_clock(struct timespec *ts);
extern void read_boot_clock64(struct timespec64 *ts);
extern int update_persistent_clock(struct timespec now);
extern int update_persistent_clock64(struct timespec64 now);


#endif
