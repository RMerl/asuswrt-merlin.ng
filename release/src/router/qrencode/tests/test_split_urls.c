#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "common.h"
#include "../qrinput.h"
#include "../qrencode_inner.h"
#include "../split.h"
#include "decoder.h"

#include "URI_testset.inc"

#if 0
static void encodeURLandPrint(char *url) {
	QRinput *input;
	BitStream *bstream;

	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput(url, input, QR_MODE_8, 1);
	bstream = BitStream_new();
	QRinput_mergeBitStream(input, bstream);

	printf("{%zu,\"%s\"},\n", BitStream_size(bstream), url);

	QRinput_free(input);
	BitStream_free(bstream);
}

static void print_currentBitLength() {
	struct TestSet *ts = testset;

	puts("struct TestSet {\n\tint expected_length;\n\tchar *url;\n};");
	puts("\nstruct TestSet testset[] = {");

	while(ts->url != NULL) {
		encodeURLandPrint(ts->url);
		ts++;
	}

	puts("{0,NULL}\n};");
}
#endif

static int encodeURLandCompare(char *url, size_t expected_length) {
	QRinput *input;
	BitStream *bstream;
	int ret = 0;

	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput(url, input, QR_MODE_8, 1);
	bstream = BitStream_new();
	QRinput_mergeBitStream(input, bstream);

	size_t length = BitStream_size(bstream);
	if(length > expected_length) {
		printf("The length of the encode stream is longer than expected: %zu over %zu\n", length, expected_length);
		printQRinput(input);

		ret = 1;
	} else if(length < expected_length) {
		printf("The length of the encode stream is shorter than expected: %zu under %zu\n", length, expected_length);
		printQRinput(input);

		ret = 1;
	}

	QRinput_free(input);
	BitStream_free(bstream);

	return ret;
}

static void test_bitstream_length() {
	struct TestSet *ts = testset;
	int err = 0;

	testStart("Split_URL test: compare bitstream length");
	while(ts->url != NULL) {
		err += encodeURLandCompare(ts->url, ts->expected_length);
		ts++;
	}
	testEnd(err);
}

int main()
{
	int tests = 1;
	testInit(tests);
	test_bitstream_length();
	testReport(tests);

	return 0;
}
