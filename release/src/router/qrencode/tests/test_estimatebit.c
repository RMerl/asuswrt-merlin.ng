#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "../qrinput.h"

static QRinput *gstream;

static void test_numbit(void)
{
	QRinput *stream;
	char num[9]="01234567";
	int bits;

	testStart("Estimation of Numeric stream (8 digits)");
	stream = QRinput_new();
	QRinput_append(stream, QR_MODE_NUM, 8, (unsigned char *)num);
	bits = QRinput_estimateBitStreamSize(stream, 0);
	testEndExp(bits == 41);

	QRinput_append(gstream, QR_MODE_NUM, 8, (unsigned char *)num);
	QRinput_free(stream);
}

static void test_numbit2(void)
{
	QRinput *stream;
	char num[17]="0123456789012345";
	int bits;

	testStart("Estimation of Numeric stream (16 digits)");
	stream = QRinput_new();
	QRinput_append(stream, QR_MODE_NUM, 16, (unsigned char *)num);
	bits = QRinput_estimateBitStreamSize(stream, 0);
	testEndExp(bits == 68);

	QRinput_append(gstream, QR_MODE_NUM, 16, (unsigned char *)num);
	QRinput_free(stream);
}

static void test_numbit3(void)
{
	QRinput *stream;
	char *num;
	int bits;

	testStart("Estimation of Numeric stream (400 digits)");
	stream = QRinput_new();
	num = (char *)malloc(401);
	memset(num, '1', 400);
	num[400] = '\0';
	QRinput_append(stream, QR_MODE_NUM, 400, (unsigned char *)num);
	bits = QRinput_estimateBitStreamSize(stream, 0);
	/* 4 + 10 + 133*10 + 4 = 1348 */
	testEndExp(bits == 1348);

	QRinput_append(gstream, QR_MODE_NUM, 400, (unsigned char *)num);
	QRinput_free(stream);
	free(num);
}

static void test_an(void)
{
	QRinput *stream;
	char str[6]="AC-42";
	int bits;

	testStart("Estimation of Alphabet-Numeric stream (5 chars)");
	stream = QRinput_new();
	QRinput_append(stream, QR_MODE_AN, 5, (unsigned char *)str);
	bits = QRinput_estimateBitStreamSize(stream, 0);
	testEndExp(bits == 41);

	QRinput_append(gstream, QR_MODE_AN, 5, (unsigned char *)str);
	QRinput_free(stream);
}

static void test_8(void)
{
	QRinput *stream;
	char str[9]="12345678";
	int bits;

	testStart("Estimation of 8 bit data stream (8 bytes)");
	stream = QRinput_new();
	QRinput_append(stream, QR_MODE_8, 8, (unsigned char *)str);
	bits = QRinput_estimateBitStreamSize(stream, 0);
	testEndExp(bits == 76);

	QRinput_append(gstream, QR_MODE_8, 8, (unsigned char *)str);
	QRinput_free(stream);
}

static void test_structure(void)
{
	QRinput *stream;
	int bits;

	testStart("Estimation of a structure-append header");
	stream = QRinput_new();
	QRinput_insertStructuredAppendHeader(stream, 10, 1, 0);
	bits = QRinput_estimateBitStreamSize(stream, 1);
	testEndExp(bits == 20);

	QRinput_insertStructuredAppendHeader(gstream, 10, 1, 0);
	QRinput_free(stream);
}

static void test_kanji(void)
{
	int res;

	QRinput *stream;
	unsigned char str[4]= {0x93, 0x5f,0xe4, 0xaa};
	int bits;

	testStart("Estimation of Kanji stream (2 chars)");
	stream = QRinput_new();
	res = QRinput_append(stream, QR_MODE_KANJI, 4, (unsigned char *)str);
	if(res < 0) {
		printf("Failed to add.\n");
		testEnd(1);
	} else {
		bits = QRinput_estimateBitStreamSize(stream, 0);
		testEndExp(bits == 38);
		QRinput_append(gstream, QR_MODE_KANJI, 4, (unsigned char *)str);
	}

	QRinput_free(stream);
}

static void test_mix(void)
{
	int bits;

	testStart("Estimation of Mixed stream");
	bits = QRinput_estimateBitStreamSize(gstream, 0);
	testEndExp(bits == (41 + 68 + 1348 + 41 + 76 + 38 + 20));
	QRinput_free(gstream);
}

/* Taken from JISX 0510:2018, p.23 */
static void test_numbit1_mqr(void)
{
	QRinput *stream;
	char *str = "0123456789012345";
	int bits;

	testStart("Estimation of Numeric stream for Micro QR Code (16 digits)");
	stream = QRinput_newMQR(3, QR_ECLEVEL_M);
	QRinput_append(stream, QR_MODE_NUM, 16, (const unsigned char *)str);
	bits = QRinput_estimateBitStreamSize(stream, QRinput_getVersion(stream));
	assert_equal(bits, 61, "Estimated bit length is wrong: %d, expected: %d.\n", bits, 61);
	QRinput_free(stream);

	stream = QRinput_newMQR(4, QR_ECLEVEL_M);
	QRinput_append(stream, QR_MODE_NUM, 16, (const unsigned char *)str);
	bits = QRinput_estimateBitStreamSize(stream, QRinput_getVersion(stream));
	assert_equal(bits, 63, "Estimated bit length is wrong: %d, expected: %d.\n", bits, 63);
	QRinput_free(stream);

	testFinish();
}

int main()
{
	gstream = QRinput_new();

	int tests = 9;
	testInit(tests);
	test_numbit();
	test_numbit2();
	test_numbit3();
	test_an();
	test_8();
	test_kanji();
	test_structure();
	test_mix();
	test_numbit1_mqr();
	testReport(tests);

	return 0;
}
