AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	@COVERAGE_CFLAGS@

AM_LDFLAGS = \
	-no-undefined

ipseclib_LTLIBRARIES = libnttfft.la

libnttfft_la_SOURCES = \
	ntt_fft_reduce.h ntt_fft.h ntt_fft.c \
	ntt_fft_params.h ntt_fft_params.c

