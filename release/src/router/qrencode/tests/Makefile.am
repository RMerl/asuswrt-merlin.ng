AM_CFLAGS =
AM_LDFLAGS =

if HAVE_SDL
sdlPROGRAMS = view_qrcode
AM_CFLAGS += $(SDL_CFLAGS)
AM_LDFLAGS += $(SDL_LIBS)
endif

if HAVE_PNG
pngPROGRAMS = create_frame_pattern create_mqr_frame_pattern
endif

check_PROGRAMS = test_qrinput test_bitstream test_estimatebit test_qrspec \
				 test_rs test_qrencode test_mqrspec test_split test_mask \
				 test_mmask test_split_urls test_monkey
noinst_PROGRAMS = $(pngPROGRAMS) $(sdlPROGRAMS) $(check_PROGRAMS) prof_qrencode
noinst_LIBRARIES = libdecoder.a
DECODER_LIBS = libdecoder.a $(LIBICONV)
noinst_HEADERS = common.h rscode.h
if HAVE_LIBPTHREAD
noinst_PROGRAMS += pthread_qrencode
endif

TESTS = $(check_PROGRAMS)
EXTRA_DIST = test_all.sh test_basic.sh test_configure.sh frame URI_testset.inc CMakeLists.txt

libdecoder_a_SOURCES = decoder.c decoder.h datachunk.c datachunk.h rsecc_decoder.c rsecc_decoder.h

test_qrinput_SOURCES = test_qrinput.c common.c
test_qrinput_LDADD = $(DECODER_LIBS) ../libqrencode.la

test_bitstream_SOURCES = test_bitstream.c common.c
test_bitstream_LDADD = ../libqrencode.la

test_estimatebit_SOURCES = test_estimatebit.c common.c
test_estimatebit_LDADD = ../libqrencode.la

test_qrspec_SOURCES = test_qrspec.c common.c
test_qrspec_CPPFLAGS = -DSRCDIR=\"$(srcdir)/\"
test_qrspec_LDADD = $(DECODER_LIBS) ../libqrencode.la

test_mqrspec_SOURCES = test_mqrspec.c common.c
test_mqrspec_LDADD = $(DECODER_LIBS) ../libqrencode.la

test_rs_SOURCES = test_rs.c rscode.c common.c
test_rs_LDADD = $(DECODER_LIBS) ../libqrencode.la

test_qrencode_SOURCES = test_qrencode.c common.c
test_qrencode_LDADD = $(DECODER_LIBS) ../libqrencode.la

test_split_SOURCES = test_split.c common.c
test_split_LDADD = ../libqrencode.la

test_split_urls_SOURCES = test_split_urls.c common.c
test_split_urls_LDADD = $(DECODER_LIBS) ../libqrencode.la

test_monkey_SOURCES = test_monkey.c common.c
test_monkey_LDADD = $(DECODER_LIBS) ../libqrencode.la

test_mask_SOURCES = test_mask.c common.c
test_mask_LDADD = $(DECODER_LIBS) ../libqrencode.la

test_mmask_SOURCES = test_mmask.c common.c
test_mmask_LDADD = $(DECODER_LIBS) ../libqrencode.la

prof_qrencode_SOURCES = prof_qrencode.c common.c
prof_qrencode_LDADD = ../libqrencode.la

pthread_qrencode_SOURCES = pthread_qrencode.c common.c
pthread_qrencode_LDADD = ../libqrencode.la

if HAVE_PNG
create_frame_pattern_SOURCES = create_frame_pattern.c common.c
create_frame_pattern_CFLAGS = $(png_CFLAGS) $(AM_CFLAGS)
create_frame_pattern_LDADD = ../libqrencode.la $(png_LIBS)

create_mqr_frame_pattern_SOURCES = create_mqr_frame_pattern.c common.c
create_mqr_frame_pattern_CFLAGS = $(png_CFLAGS) $(AM_CFLAGS)
create_mqr_frame_pattern_LDADD = ../libqrencode.la $(png_LIBS)
endif

if HAVE_SDL
view_qrcode_SOURCES = view_qrcode.c common.c
view_qrcode_LDADD = ../libqrencode.la
endif
