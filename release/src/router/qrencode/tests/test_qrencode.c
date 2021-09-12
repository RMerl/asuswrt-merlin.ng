#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "common.h"
#include "../qrencode_inner.h"
#include "../qrspec.h"
#include "../mqrspec.h"
#include "../qrinput.h"
#include "../mask.h"
#include "../rsecc.h"
#include "../split.h"
#include "decoder.h"

static const char decodeAnTable[45] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z', ' ', '$', '%', '*',
	'+', '-', '.', '/', ':'
};

typedef struct {
	char *str;
	int version;
	QRecLevel level;
	QRencodeMode hint;
	int casesensitive;
} TestString;
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))

#define drand(__scale__) ((__scale__) * (double)rand() / ((double)RAND_MAX + 1.0))

static void test_qrraw_new(void)
{
	int i;
	QRinput *stream;
	char num[9] = "01234567";
	QRRawCode *raw;

	testStart("Test QRraw_new()");
	stream = QRinput_new();
	QRinput_setVersion(stream, 10);
	QRinput_setErrorCorrectionLevel(stream, QR_ECLEVEL_Q);
	QRinput_append(stream, QR_MODE_NUM, 8, (unsigned char *)num);

	raw = QRraw_new(stream);
	assert_nonnull(raw, "Failed QRraw_new().\n");
	assert_zero(raw->count, "QRraw.count = %d != 0\n", raw->count);
	assert_equal(raw->version, 10, "QRraw.version was not as expected. (%d)\n", raw->version);
	assert_equal(raw->dataLength, 19 * 6 + 20 * 2, "QRraw.dataLength was not as expected.\n");
	assert_equal(raw->eccLength, 24 * 8, "QRraw.eccLength was not as expected.\n");
	assert_equal(raw->b1, 6, "QRraw.b1 was not as expected.\n");
	assert_equal(raw->blocks, 8, "QRraw.blocks was not as expected.\n");

	for(i=0; i<raw->b1; i++) {
		assert_equal(raw->rsblock[i].dataLength, 19, "QRraw.rsblock[%d].dataLength was not as expected. (%d)\n", i, raw->rsblock[i].dataLength);
	}
	for(i=raw->b1; i<raw->blocks; i++) {
		assert_equal(raw->rsblock[i].dataLength, 20, "QRraw.rsblock[%d].dataLength was not as expected. (%d)\n", i, raw->rsblock[i].dataLength);
	}
	for(i=0; i<raw->blocks; i++) {
		assert_equal(raw->rsblock[i].eccLength, 24, "QRraw.rsblock[%d].eccLength was not as expected. (%d)\n", i, raw->rsblock[i].eccLength);
	}

	QRinput_free(stream);
	QRraw_free(raw);
	testFinish();
}

static void test_iterate()
{
	int i;
	QRinput *stream;
	char num[9] = "01234567";
	unsigned char *data;
	QRRawCode *raw;
	int err = 0;

	testStart("Test getCode (1-L)");
	stream = QRinput_new();
	QRinput_setVersion(stream, 1);
	QRinput_setErrorCorrectionLevel(stream, QR_ECLEVEL_L);
	QRinput_append(stream, QR_MODE_NUM, 8, (unsigned char *)num);

	raw = QRraw_new(stream);
	data = raw->datacode;
	for(i=0; i<raw->dataLength; i++) {
		if(data[i] != QRraw_getCode(raw)) {
			err++;
		}
	}

	QRinput_free(stream);
	QRraw_free(raw);
	testEnd(err);
}

static void test_iterate2()
{
	int i;
	QRinput *stream;
	char num[9] = "01234567";
	QRRawCode *raw;
	int err = 0;
	unsigned char correct[] = {
	0x10, 0x11, 0xec, 0xec, 0x20, 0xec, 0x11, 0x11,
	0x0c, 0x11, 0xec, 0xec, 0x56, 0xec, 0x11, 0x11,
	0x61, 0x11, 0xec, 0xec, 0x80, 0xec, 0x11, 0x11,
	0xec, 0x11, 0xec, 0xec, 0x11, 0xec, 0x11, 0x11,
	0xec, 0x11, 0xec, 0xec, 0x11, 0xec, 0x11, 0x11,
	0xec, 0x11, 0xec, 0xec, 0x11, 0x11,
	0x5c, 0xde, 0x68, 0x68, 0x4d, 0xb3, 0xdb, 0xdb,
	0xd5, 0x14, 0xe1, 0xe1, 0x5b, 0x2a, 0x1f, 0x1f,
	0x49, 0xc4, 0x78, 0x78, 0xf7, 0xe0, 0x5b, 0x5b,
	0xc3, 0xa7, 0xc1, 0xc1, 0x5d, 0x9a, 0xea, 0xea,
	0x48, 0xad, 0x9d, 0x9d, 0x58, 0xb3, 0x3f, 0x3f,
	0x10, 0xdb, 0xbf, 0xbf, 0xeb, 0xec, 0x05, 0x05,
	0x98, 0x35, 0x83, 0x83, 0xa9, 0x95, 0xa6, 0xa6,
	0xea, 0x7b, 0x8d, 0x8d, 0x04, 0x3c, 0x08, 0x08,
	0x64, 0xce, 0x3e, 0x3e, 0x4d, 0x9b, 0x30, 0x30,
	0x4e, 0x65, 0xd6, 0xd6, 0xe4, 0x53, 0x2c, 0x2c,
	0x46, 0x1d, 0x2e, 0x2e, 0x29, 0x16, 0x27, 0x27
	};

	testStart("Test getCode (5-H)");
	stream = QRinput_new();
	QRinput_setVersion(stream, 5);
	QRinput_setErrorCorrectionLevel(stream, QR_ECLEVEL_H);
	QRinput_append(stream, QR_MODE_NUM, 8, (unsigned char *)num);

	raw = QRraw_new(stream);
	for(i=0; i<raw->dataLength; i++) {
		if(correct[i] != QRraw_getCode(raw)) {
			err++;
		}
	}

	QRinput_free(stream);
	QRraw_free(raw);
	testEnd(err);
}

static void print_filler(void)
{
	int width;
	int version = 7;
	unsigned char *frame;

	puts("\nPrinting debug info of FrameFiller.");
	width = QRspec_getWidth(version);
	frame = FrameFiller_test(version);
	if(frame == NULL) abort();

	printFrame(width, frame);
	free(frame);
}

static void test_filler(void)
{
	unsigned char *frame;
	int i, j, w, e, length;

	testStart("Frame filler test");
	for(i=1; i<=QRSPEC_VERSION_MAX; i++) {
		length = QRspec_getDataLength(i, QR_ECLEVEL_L) * 8
		       + QRspec_getECCLength(i, QR_ECLEVEL_L) * 8
			   + QRspec_getRemainder(i);
		frame = FrameFiller_test(i);
		if(frame == NULL) {
			assert_nonnull(frame, "Something wrong in version %d\n", i);
		} else {
			w = QRspec_getWidth(i);
			e = 0;
			for(j=0; j<w*w; j++) {
				if(frame[j] == 0) e++;
			}
			assert_zero(e, "Not filled bit is found. (%d,%d)\n", j%w,j/w);
			e = w * (w - 9 - ((i > 6)?3:0));
			assert_equal(frame[e], (unsigned char)((length - 1) & 127) | 0x80,
			"Number of cell does not match.\n");
			free(frame);
		}
	}
	testFinish();
}

static void print_fillerMQR(void)
{
	int width;
	int version = 3;
	unsigned char *frame;

	puts("\nPrinting debug info of FrameFiller for Micro QR.");
	for(version = 1; version <= MQRSPEC_VERSION_MAX; version++) {
		width = MQRspec_getWidth(version);
		frame = FrameFiller_testMQR(version);
		if(frame == NULL) abort();

		printFrame(width, frame);
	}
}

static void test_fillerMQR(void)
{
	unsigned char *frame;
	int i, j, w, e, length;

	testStart("Micro QR Code Frame filler test");
	for(i=1; i<=MQRSPEC_VERSION_MAX; i++) {
		length = MQRspec_getDataLengthBit(i, QR_ECLEVEL_L)
		       + MQRspec_getECCLength(i, QR_ECLEVEL_L) * 8;
		frame = FrameFiller_testMQR(i);
		if(frame == NULL) {
			assert_nonnull(frame, "Something wrong in version %d\n", i);
		} else {
			w = MQRspec_getWidth(i);
			e = 0;
			for(j=0; j<w*w; j++) {
				if(frame[j] == 0) e++;
			}
			assert_zero(e, "Not filled bit is found. (%d,%d)\n", j%w,j/w);
			if(i & 1) {
				e = w * 9 + 1;
			} else {
				e = w * (w - 1) + 1;
			}
			assert_equal(frame[e], (unsigned char)((length - 1) & 127) | 0x80,
			"Number of cell does not match in version %d.\n", i);
			free(frame);
		}
	}
	testFinish();
}

static void test_format(void)
{
	unsigned char *frame;
	unsigned int format;
	int width;
	int i;
	unsigned int decode;
	int blacks, b1 = 0, b2 = 0;

	testStart("Test format information(level L,mask 0)");
	width = QRspec_getWidth(1);
	frame = QRspec_newFrame(1);
	if(frame == NULL) goto ABORT;
	format = QRspec_getFormatInfo(1, QR_ECLEVEL_L);
	blacks = Mask_writeFormatInformation(width, frame, 1, QR_ECLEVEL_L);
	decode = 0;
	for(i=0; i<15; i++) {
		if((1<<i) & format) b2 += 2;
	}
	for(i=0; i<8; i++) {
		decode = decode << 1;
		decode |= frame[width * 8 + i + (i > 5)] & 1;
		if(decode & 1) b1++;
	}
	for(i=0; i<7; i++) {
		decode = decode << 1;
		decode |= frame[width * ((6 - i) + (i < 1)) + 8] & 1;
		if(decode & 1) b1++;
	}
	if(decode != format) {
		printf("Upper-left format information is invalid.\n");
		printf("%08x, %08x\n", format, decode);
		testEnd(1);
		return;
	}
	decode = 0;
	for(i=0; i<7; i++) {
		decode = decode << 1;
		decode |= frame[width * (width - 1 - i) + 8] & 1;
		if(decode & 1) b1++;
	}
	for(i=0; i<8; i++) {
		decode = decode << 1;
		decode |= frame[width * 8 + width - 8 + i] & 1;
		if(decode & 1) b1++;
	}
	if(decode != format) {
		printf("Bottom and right format information is invalid.\n");
		printf("%08x, %08x\n", format, decode);
		testEnd(1);
		return;
	}

	if(b2 != blacks || b1 != b2) {
		printf("Number of dark modules is incorrect.\n");
		printf("Return value: %d, dark modules in frame: %d, should be: %d\n", blacks, b1, b2);
		testEnd(1);
		return;
	}

	free(frame);

ABORT:
	testEnd(0);
}

unsigned int m1pat[8][21] = {
	{0x1fc77f, 0x105c41, 0x174c5d, 0x174b5d, 0x175b5d, 0x104241, 0x1fd57f,
 	 0x000000, 0x154512, 0x1a16a2, 0x0376ee, 0x19abb2, 0x04eee1, 0x001442,
	 0x1fc111, 0x10444b, 0x175d5d, 0x174aae, 0x175ae5, 0x1043b8, 0x1fd2e5},
	{0x1fdd7f, 0x104641, 0x17565d, 0x17415d, 0x17415d, 0x105841, 0x1fd57f,
	 0x000a00, 0x146f25, 0x10bc08, 0x09dc44, 0x130118, 0x0e444b, 0x001ee8,
	 0x1fdbbb, 0x104ee1, 0x1747f7, 0x174004, 0x17504f, 0x104912, 0x1fd84f},
	{0x1fcb7f, 0x104f41, 0x17505d, 0x17585d, 0x17575d, 0x105141, 0x1fd57f,
	 0x001300, 0x17c97c, 0x02b52c, 0x046a9f, 0x01083c, 0x03f290, 0x0017cc,
	 0x1fcd60, 0x1057c5, 0x17512c, 0x175920, 0x175694, 0x104036, 0x1fde94},
	{0x1fdb7f, 0x105441, 0x174d5d, 0x17585d, 0x174c5d, 0x104c41, 0x1fd57f,
	 0x001800, 0x16e44b, 0x02b52c, 0x12f1f2, 0x1a258a, 0x03f290, 0x001ca1,
	 0x1fd0d6, 0x1057c5, 0x174a41, 0x175496, 0x175694, 0x104b5b, 0x1fd322},
	{0x1fd37f, 0x104741, 0x17475d, 0x175f5d, 0x175f5d, 0x105941, 0x1fd57f,
	 0x001400, 0x1171f9, 0x0c8dcf, 0x15ed83, 0x108f20, 0x0dca73, 0x001f2f,
	 0x1fda7c, 0x1040d9, 0x1759cf, 0x1741c3, 0x174188, 0x10472a, 0x1fd677},
	{0x1fcd7f, 0x105741, 0x17505d, 0x17545d, 0x17475d, 0x104941, 0x1fd57f,
	 0x001b00, 0x1059ce, 0x05a95d, 0x046a9f, 0x03001c, 0x0e444b, 0x001fec,
	 0x1fcd60, 0x104bb4, 0x17412c, 0x174100, 0x17404f, 0x104816, 0x1fde94},
	{0x1fdd7f, 0x105741, 0x17545d, 0x17445d, 0x17555d, 0x104f41, 0x1fd57f,
	 0x000b00, 0x13fd97, 0x05a95d, 0x00f8d6, 0x028604, 0x0e444b, 0x001f2f,
	 0x1fd9f2, 0x105bb4, 0x175365, 0x175718, 0x17404f, 0x1048d5, 0x1fda06},
	{0x1fc77f, 0x104841, 0x174e5d, 0x174b5d, 0x174f5d, 0x105041, 0x1fd57f,
	 0x000400, 0x12d7a0, 0x1a16a2, 0x0a527c, 0x1d39fb, 0x04eee1, 0x0010d0,
	 0x1fc358, 0x10544b, 0x1749cf, 0x1758e7, 0x174ae5, 0x10472a, 0x1fd0ac}
};

static void test_encode(void)
{
	QRinput *stream;
	char num[9] = "01234567";
	unsigned char *frame;
	int err = 0;
	int x, y, w;
	int mask;
	QRcode *qrcode;

	testStart("Test encode (1-M)");
	stream = QRinput_new();
	if(stream == NULL) goto ABORT;
	QRinput_append(stream, QR_MODE_NUM, 8, (unsigned char *)num);
	for(mask=0; mask<8; mask++) {
		QRinput_setVersion(stream, 1);
		QRinput_setErrorCorrectionLevel(stream, QR_ECLEVEL_M);
		qrcode = QRcode_encodeMask(stream, mask);
		if(qrcode == NULL) goto ABORT;
		w = qrcode->width;
		frame = qrcode->data;
		for(y=0; y<w; y++) {
			for(x=0; x<w; x++) {
				if(((m1pat[mask][y] >> (20-x)) & 1) != (frame[y*w+x]&1)) {
					printf("Diff in mask=%d (%d,%d)\n", mask, x, y);
					err++;
				}
			}
		}
		QRcode_free(qrcode);
	}
	QRinput_free(stream);
ABORT:
	testEnd(err);
}

static void test_encode2(void)
{
	QRcode *qrcode;

	testStart("Test encode (2-H) (no padding test)");
	qrcode = QRcode_encodeString("abcdefghijk123456789012", 0, QR_ECLEVEL_H, QR_MODE_8, 0);
	testEndExp(qrcode->version == 2);
	QRcode_free(qrcode);
}

static void test_encode3(void)
{
	QRcode *code1, *code2;
	QRinput *input;

	testStart("Compare encodeString and encodeInput");
	code1 = QRcode_encodeString("0123456", 0, QR_ECLEVEL_L, QR_MODE_8, 0);
	input = QRinput_new2(0, QR_ECLEVEL_L);
	QRinput_append(input, QR_MODE_NUM, 7, (unsigned char *)"0123456");
	code2 = QRcode_encodeInput(input);
	testEnd(memcmp(code1->data, code2->data, code1->width * code1->width));

	QRcode_free(code1);
	QRcode_free(code2);
	QRinput_free(input);
}

static void test_encodeNull(void)
{
	QRcode *qrcode;

	testStart("Test encode NULL.");
	qrcode = QRcode_encodeString(NULL, 0, QR_ECLEVEL_H, QR_MODE_8, 0);
	assert_null(qrcode, "QRcode_encodeString() returned something.\n");
	testFinish();
	if(qrcode != NULL) QRcode_free(qrcode);
}


static void test_encodeEmpty(void)
{
	QRcode *qrcode;

	testStart("Test encode an empty string.");
	qrcode = QRcode_encodeString("", 0, QR_ECLEVEL_H, QR_MODE_8, 0);
	assert_null(qrcode, "QRcode_encodeString() returned something.\n");
	testFinish();
	if(qrcode != NULL) QRcode_free(qrcode);
}

static void test_encodeNull8(void)
{
	QRcode *qrcode;

	testStart("Test encode NULL.");
	qrcode = QRcode_encodeString8bit(NULL, 0, QR_ECLEVEL_H);
	assert_null(qrcode, "QRcode_encodeString8bit() returned something.\n");
	testFinish();
	if(qrcode != NULL) QRcode_free(qrcode);
}


static void test_encodeEmpty8(void)
{
	QRcode *qrcode;

	testStart("Test encode an empty string.");
	qrcode = QRcode_encodeString8bit("", 0, QR_ECLEVEL_H);
	assert_null(qrcode, "QRcode_encodeString8bit() returned something.\n");
	testFinish();
	if(qrcode != NULL) QRcode_free(qrcode);
}

static void test_encodeLongData(void)
{
	QRinput *stream;
	unsigned char data[7090];
	int maxlength[4][4] = {{7089,5596,3993,3057},
						   {4296,3391,2420,1852},
						   {2953,2331,1663,1273},
						   {1817*2,1435*2,1024*2, 784*2}};
	int i, l, len, ret;
	QRcode *qrcode;

	testStart("Encoding long data.");

	for(i=QR_MODE_NUM; i<=QR_MODE_KANJI; i++) {
		if(i != QR_MODE_KANJI) {
			memset(data, '0', maxlength[i][0] + 1);
		} else {
			for(l=0; l<=maxlength[i][0]/2+1; l++) {
				data[l*2] = 0x93; data[l*2+1] = 0x5f;
			}
		}
		for(l=QR_ECLEVEL_L; l<=QR_ECLEVEL_H; l++) {
			stream = QRinput_new2(0, l);
			ret = QRinput_append(stream, i, maxlength[i][l], data);
			assert_zero(ret, "Failed to add %d-byte %s to a QRinput\n", maxlength[i][l], modeStr[i]);
			qrcode = QRcode_encodeInput(stream);
			assert_nonnull(qrcode, "(QRcode_encodeInput) failed to encode %d-byte %s in level %d.\n", maxlength[i][l], modeStr[i], l);
			if(qrcode != NULL) {
				QRcode_free(qrcode);
			}
			QRinput_free(stream);

			stream = QRinput_new2(0, l);
			len = maxlength[i][l];
			if(i == QR_MODE_KANJI) {
				len += 2;
			} else {
				len += 1;
			}
			ret = QRinput_append(stream, i, len, data);
			if(ret == 0) {
				qrcode = QRcode_encodeInput(stream);
				assert_null(qrcode, "(QRcode_encodeInput) incorrectly succeeded to encode %d-byte %s in level %d.\n", len, modeStr[i], l);
				if(qrcode != NULL) {
					printf("version: %d\n", qrcode->version);
					QRcode_free(qrcode);
				}
			}
			QRinput_free(stream);
		}
	}

	testFinish();
}

static void test_encodeVer26Num(void)
{
	char data[3284];
	QRcode *qrcode;

	testStart("Encoding 3283 digits number. (issue #160)");

	memset(data, '0', 3283);
	data[3283] = '\0';
	qrcode = QRcode_encodeString(data, 0, QR_ECLEVEL_L, QR_MODE_8, 0);
	assert_nonnull(qrcode, "(QRcode_encodeString) failed to encode 3283 digits number in level L.\n");
	assert_equal(qrcode->version, 26, "version number is %d (26 expected)\n", qrcode->version);
	if(qrcode != NULL) {
		QRcode_free(qrcode);
	}

	QRinput *input;
	QRinput_List *list;

	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput(data, input, QR_MODE_8, 0);
	list = input->head;
	assert_equal(list->size, 3283, "chunk size is wrong. (%d, 3283 expected)\n", list->size);
	QRinput_free(input);

	testFinish();
}

static void test_01234567(void)
{
	QRinput *stream;
	char num[9] = "01234567";
	int i, err = 0;
	QRcode *qrcode;
	unsigned char correct[] = {
0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc0, 0x84, 0x03, 0x02, 0x03, 0x03, 0xc0, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1,
0xc1, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc1, 0xc0, 0x84, 0x03, 0x03, 0x03, 0x03, 0xc0, 0xc1, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc1,
0xc1, 0xc0, 0xc1, 0xc1, 0xc1, 0xc0, 0xc1, 0xc0, 0x85, 0x02, 0x02, 0x02, 0x02, 0xc0, 0xc1, 0xc0, 0xc1, 0xc1, 0xc1, 0xc0, 0xc1,
0xc1, 0xc0, 0xc1, 0xc1, 0xc1, 0xc0, 0xc1, 0xc0, 0x85, 0x03, 0x02, 0x02, 0x02, 0xc0, 0xc1, 0xc0, 0xc1, 0xc1, 0xc1, 0xc0, 0xc1,
0xc1, 0xc0, 0xc1, 0xc1, 0xc1, 0xc0, 0xc1, 0xc0, 0x85, 0x02, 0x03, 0x01, 0x01, 0xc0, 0xc1, 0xc0, 0xc1, 0xc1, 0xc1, 0xc0, 0xc1,
0xc1, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc1, 0xc0, 0x85, 0x02, 0x02, 0x00, 0x01, 0xc0, 0xc1, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc1,
0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc0, 0x91, 0x90, 0x91, 0x90, 0x91, 0xc0, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1,
0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x85, 0x02, 0x02, 0x01, 0x01, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
0x85, 0x84, 0x85, 0x85, 0x85, 0x85, 0x91, 0x84, 0x84, 0x03, 0x02, 0x00, 0x01, 0x84, 0x85, 0x85, 0x85, 0x85, 0x85, 0x84, 0x84,
0x02, 0x02, 0x02, 0x03, 0x02, 0x03, 0x90, 0x03, 0x03, 0x02, 0x03, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00,
0x02, 0x02, 0x03, 0x02, 0x02, 0x02, 0x91, 0x03, 0x02, 0x03, 0x02, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01,
0x02, 0x02, 0x02, 0x02, 0x03, 0x02, 0x90, 0x02, 0x02, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x91, 0x03, 0x03, 0x02, 0x02, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x81, 0x02, 0x03, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00,
0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc0, 0x84, 0x03, 0x03, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
0xc1, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc1, 0xc0, 0x85, 0x02, 0x03, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
0xc1, 0xc0, 0xc1, 0xc1, 0xc1, 0xc0, 0xc1, 0xc0, 0x85, 0x02, 0x02, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00,
0xc1, 0xc0, 0xc1, 0xc1, 0xc1, 0xc0, 0xc1, 0xc0, 0x85, 0x03, 0x02, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
0xc1, 0xc0, 0xc1, 0xc1, 0xc1, 0xc0, 0xc1, 0xc0, 0x85, 0x02, 0x03, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00,
0xc1, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc1, 0xc0, 0x84, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01, 0x00,
0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc0, 0x85, 0x03, 0x03, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00};

	testStart("Encode 01234567 in 1-M");
	stream = QRinput_new2(1, QR_ECLEVEL_M);
	QRinput_append(stream, QR_MODE_NUM, 8, (unsigned char *)num);
	qrcode = QRcode_encodeInput(stream);
	for(i=0; i<qrcode->width * qrcode->width; i++) {
		if(qrcode->data[i] != correct[i]) {
			err++;
		}
	}
	testEnd(err);
	QRinput_free(stream);
	QRcode_free(qrcode);
}

static void print_01234567(void)
{
	QRinput *stream;
	char num[9] = "01234567";
	QRcode *qrcode;

	puts("\nPrinting QR code of '01234567'.");
	stream = QRinput_new2(1, QR_ECLEVEL_M);
	QRinput_append(stream, QR_MODE_NUM, 8, (unsigned char *)num);
	qrcode = QRcode_encodeInput(stream);
	printQRcode(qrcode);
	QRinput_free(stream);
	QRcode_free(qrcode);
}

static void test_invalid_input(void)
{
	QRinput *input;
	QRcode *code;

	testStart("Testing invalid input.");
	input = QRinput_new();
	QRinput_append(input, QR_MODE_AN, 5, (unsigned char *)"TEST1");
	input->version = -1;
	input->level = QR_ECLEVEL_L;
	code = QRcode_encodeInput(input);
	assert_null(code, "invalid version(-1)  was not checked.\n");
	if(code != NULL) QRcode_free(code);

	input->version = 41;
	input->level = QR_ECLEVEL_L;
	code = QRcode_encodeInput(input);
	assert_null(code, "invalid version(41) access was not checked.\n");
	if(code != NULL) QRcode_free(code);

	input->version = 1;
	input->level = (QRecLevel)(QR_ECLEVEL_H + 1);
	code = QRcode_encodeInput(input);
	assert_null(code, "invalid level(H+1) access was not checked.\n");
	if(code != NULL) QRcode_free(code);

	input->version = 1;
	input->level = (QRecLevel)-1;
	code = QRcode_encodeInput(input);
	assert_null(code, "invalid level(-1) access was not checked.\n");
	if(code != NULL) QRcode_free(code);

	QRinput_free(input);

	testFinish();
}

static void test_struct_semilong(void)
{
	QRcode_List *codes, *list;
	const char *str = "asdfasdfasdfasdfasdfASDFASDASDFASDFAsdfasdfasdfasdASDFASDFADSADadsfasdf";
	int num, size;

	testStart("Testing semi-long structured-append symbols");
	codes = QRcode_encodeString8bitStructured(str, 1, QR_ECLEVEL_L);
	list = codes;
	num = 0;
	while(list != NULL) {
		num++;
		assert_equal(list->code->version, 1, "version number is %d (1 expected)\n", list->code->version);
		list = list->next;
	}
	size = QRcode_List_size(codes);
	assert_equal(num, size, "QRcode_List_size returns wrong size?");
	QRcode_List_free(codes);

	codes = QRcode_encodeStringStructured(str, 1, QR_ECLEVEL_L, QR_MODE_8, 1);
	list = codes;
	num = 0;
	while(list != NULL) {
		num++;
		assert_equal(list->code->version, 1, "version number is %d (1 expected)\n", list->code->version);
		list = list->next;
	}
	size = QRcode_List_size(codes);
	assert_equal(num, size, "QRcode_List_size returns wrong size?");
	QRcode_List_free(codes);

	testFinish();
}

static void test_struct_example(void)
{
	QRcode_List *codes, *list;
	const char *str = "an example of four Structured Append symbols,";
	int num;

	testStart("Testing the example of structured-append symbols");
	codes = QRcode_encodeString8bitStructured(str, 1, QR_ECLEVEL_M);
	list = codes;
	num = 0;
	while(list != NULL) {
		num++;
		assert_equal(list->code->version, 1, "version number is %d (1 expected)\n", list->code->version);
		list = list->next;
	}
	assert_equal(num, 4, "number of symbols is %d (4 expected).", num);
	testFinish();
	QRcode_List_free(codes);
}

static void test_null_free(void)
{
	testStart("Testing free NULL pointers");
	assert_nothing(QRcode_free(NULL), "Check QRcode_free(NULL).\n");
	assert_nothing(QRcode_List_free(NULL), "Check QRcode_List_free(NULL).\n");
	assert_nothing(QRraw_free(NULL), "Check QRraw_free(NULL).\n");
	testFinish();
}

static void test_encodeTooLongMQR(void)
{
	QRcode *code;
	char *data[] = {"012345", "ABC0EFG", "0123456789", "0123456789ABCDEFG"};

	testStart("Encode too large data for MQR.");

	code = QRcode_encodeStringMQR(data[0], 1, QR_ECLEVEL_L, QR_MODE_8, 0);
	assert_nonnull(code, "6 byte length numeric string should be accepted to version 2 or larger.\n");
	assert_equal(code->version, 2, "6 byte length numeric string should be accepted to version 2.\n");
	code = QRcode_encodeStringMQR(data[1], 2, QR_ECLEVEL_L, QR_MODE_8, 0);
	assert_nonnull(code, "7 byte length alphanumeric string should be accepted to version 3 or larger.\n");
	assert_equal(code->version, 3, "7 byte length alphanumeric string should be accepted to version 3.\n");
	code = QRcode_encodeString8bitMQR(data[2], 3, QR_ECLEVEL_L);
	assert_nonnull(code, "9 byte length 8bit string should be accepted to version 4.\n");
	assert_equal(code->version, 4, "9 byte length 8bit string should be accepted to version 4.\n");
	code = QRcode_encodeString8bitMQR(data[3], 4, QR_ECLEVEL_L);
	assert_null(code, "16 byte length 8bit string was accepted to version 4.\n");
	assert_equal(errno, ERANGE, "errno != ERANGE\n");
	testFinish();

	if(code != NULL) {
		printQRcode(code);
		QRcode_free(code);
	}
}

static void test_mqrraw_new(void)
{
	QRinput *stream;
	char *num = "01234";
	unsigned char datacode[] = {0xa0, 0x62, 0x20};
	MQRRawCode *raw;

	testStart("Test MQRRaw_new()");
	stream = QRinput_newMQR(1, QR_ECLEVEL_L);
	QRinput_append(stream, QR_MODE_NUM, 5, (unsigned char *)num);

	raw = MQRraw_new(stream);
	assert_nonnull(raw, "Failed MQRraw_new().\n");
	assert_zero(raw->count, "MQRraw.count = %d != 0\n", raw->count);
	assert_equal(raw->version, 1, "MQRraw.version was not as expected. (%d)\n", raw->version);
	assert_equal(raw->dataLength, 3, "MQRraw.dataLength was not as expected.\n");
	assert_equal(raw->eccLength, 2, "MQRraw.eccLength was not as expected.\n");
	assert_zero(memcmp(raw->datacode, datacode, 3), "Datacode doesn't match.\n");


	QRinput_free(stream);
	MQRraw_free(raw);
	testFinish();
}

static void test_encodeData(void)
{
	QRcode *qrcode;

	testStart("Test QRencode_encodeData.");
	qrcode = QRcode_encodeData(0, NULL, 0, QR_ECLEVEL_H);
	assert_null(qrcode, "QRcode_encodeData(NULL, 0) returned something.\n");
	if(qrcode != NULL) QRcode_free(qrcode);

	qrcode = QRcode_encodeData(10, (unsigned char*)"test\0\0test", 0, QR_ECLEVEL_H);
	assert_nonnull(qrcode, "QRcode_encodeData() failed.\n");
	if(qrcode != NULL) QRcode_free(qrcode);

	testFinish();
}

static void test_formatInfo(void)
{
	QRcode *qrcode;
	QRecLevel level;
	int mask;
	int ret;

	testStart("Test format info in QR code.");
	qrcode = QRcode_encodeString("AC-42", 1, QR_ECLEVEL_H, QR_MODE_8, 1);
	ret = QRcode_decodeFormat(qrcode, &level, &mask);
	assert_zero(ret, "Failed to decode.\n");
	assert_equal(level, QR_ECLEVEL_H, "Decoded format is wrong.\n");

	if(qrcode != NULL) QRcode_free(qrcode);

	testFinish();
}

static void test_formatInfoMQR(void)
{
	QRcode *qrcode;
	QRecLevel level;
	int version, mask;
	int i, ret;

	testStart("Test format info in Micro QR code.");
	for(i=0; i<8; i++) {
		qrcode = QRcode_encodeStringMQR("1",
										MQRformat[i].version,
										MQRformat[i].level,
										QR_MODE_8, 1);
		ret = QRcode_decodeFormatMQR(qrcode, &version, &level, &mask);
		assert_zero(ret, "Failed to decode.\n");
		assert_equal(MQRformat[i].version, version, "Decoded verion is wrong.\n");
		assert_equal(MQRformat[i].level, level, "Decoded level is wrong.\n");
		QRcode_free(qrcode);
	}

	testFinish();
}

static void test_decodeSimple(void)
{
	char *str = "AC-42";
	QRcode *qrcode;
	QRdata *qrdata;

	testStart("Test code words.");
	qrcode = QRcode_encodeString(str, 1, QR_ECLEVEL_H, QR_MODE_8, 1);
	qrdata = QRcode_decode(qrcode);

	assert_nonnull(qrdata, "Failed to decode.\n");
	if(qrdata != NULL) {
		assert_equal(strlen(str), qrdata->size, "Lengths of input/output mismatched: %d, expected %d.\n", qrdata->size, (int)strlen(str));
		assert_zero(strncmp(str, (char *)(qrdata->data), qrdata->size), "Decoded data %s is different from the original %s\n", qrdata->data, str);
	}
	if(qrdata != NULL) QRdata_free(qrdata);
	if(qrcode != NULL) QRcode_free(qrcode);

	testFinish();
}


static void test_decodeLong(void)
{
	char *str = "12345678901234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ?????????????";
	QRcode *qrcode;
	QRdata *qrdata;

	testStart("Test code words (long, splitted).");
	qrcode = QRcode_encodeString(str, 0, QR_ECLEVEL_H, QR_MODE_8, 1);
	qrdata = QRcode_decode(qrcode);

	assert_nonnull(qrdata, "Failed to decode.\n");
	if(qrdata != NULL) {
		assert_equal(strlen(str), qrdata->size, "Lengths of input/output mismatched.\n");
		assert_zero(strncmp(str, (char *)(qrdata->data), qrdata->size), "Decoded data %s is different from the original %s\n", qrdata->data, str);
	}
	if(qrdata != NULL) QRdata_free(qrdata);
	if(qrcode != NULL) QRcode_free(qrcode);

	testFinish();
}

static void test_decodeVeryLong(void)
{
	char str[4000];
	int i;
	QRcode *qrcode;
	QRdata *qrdata;

	testStart("Test code words (very long string).");

	for(i=0; i<3999; i++) {
		str[i] = decodeAnTable[(int)drand(45)];
	}
	str[3999] = '\0';

	qrcode = QRcode_encodeString(str, 0, QR_ECLEVEL_L, QR_MODE_8, 0);
	qrdata = QRcode_decode(qrcode);

	assert_nonnull(qrdata, "Failed to decode.\n");
	if(qrdata != NULL) {
		assert_equal(strlen(str), qrdata->size, "Lengths of input/output mismatched.\n");
		assert_zero(strncmp(str, (char *)(qrdata->data), qrdata->size), "Decoded data %s is different from the original %s\n", qrdata->data, str);
	}
	if(qrdata != NULL) QRdata_free(qrdata);
	if(qrcode != NULL) QRcode_free(qrcode);

	testFinish();
}

static void test_decodeShortMQR(void)
{
	char str[]="55";
	QRcode *qrcode;
	QRdata *qrdata;
	int i;

	testStart("Test code words (MQR).");
	for(i=0; i<8; i++) {
		qrcode = QRcode_encodeStringMQR(str,
										MQRformat[i].version,
										MQRformat[i].level,
										QR_MODE_8, 1);
		qrdata = QRcode_decodeMQR(qrcode);

		assert_nonnull(qrdata, "Failed to decode.\n");
		assert_zero(strcmp((char *)qrdata->data, str), "Decoded data (%s) mismatched (%s)\n", (char *)qrdata->data, str);
		if(qrdata != NULL) QRdata_free(qrdata);
		if(qrcode != NULL) QRcode_free(qrcode);
	}

	testFinish();
}

static void test_oddBitCalcMQR(void)
{
	/* test issue #25 (odd bits calculation bug) */
	/* test pattern contributed by vlad417 */
	TestString tests[] = {
		{"46194", 1, QR_ECLEVEL_L, QR_MODE_8, 1},
		{"WBA5Y47YPQQ", 3, QR_ECLEVEL_L, QR_MODE_8, 1}
	};
	QRcode *qrcode;
	QRdata *qrdata;
	int i;

	testStart("Odd bits calculation bug checking (MQR).");

	for(i=0; i<_countof(tests); i++) {
		qrcode = QRcode_encodeStringMQR(tests[i].str,
										tests[i].version,
										tests[i].level,
										tests[i].hint,
										tests[i].casesensitive);
		assert_nonnull(qrcode, "Failed to encode: %s\n", tests[i].str);
		if(qrcode == NULL) continue;
		qrdata = QRcode_decodeMQR(qrcode);
		assert_nonnull(qrdata, "Failed to decode.\n");
		assert_zero(strcmp((char *)qrdata->data, tests[i].str), "Decoded data (%s) mismatched (%s)\n", (char *)qrdata->data, tests[i].str);
		if(qrdata != NULL) QRdata_free(qrdata);
		QRcode_free(qrcode);
	}

	testFinish();
}

static void test_invalid_inputMQR(void)
{
	QRinput *input;
	QRcode *code;

	testStart("Testing invalid input (MQR).");
	input = QRinput_newMQR(1, QR_ECLEVEL_L);
	QRinput_append(input, QR_MODE_AN, 5, (unsigned char *)"TEST1");
	input->version = -1;
	input->level = QR_ECLEVEL_L;
	code = QRcode_encodeInput(input);
	assert_null(code, "invalid version(-1)  was not checked.\n");
	if(code != NULL) QRcode_free(code);

	input->version = 5;
	input->level = QR_ECLEVEL_L;
	code = QRcode_encodeInput(input);
	assert_null(code, "invalid version(5) access was not checked.\n");
	if(code != NULL) QRcode_free(code);

	input->version = 1;
	input->level = (QRecLevel)(QR_ECLEVEL_H);
	code = QRcode_encodeInput(input);
	assert_null(code, "invalid level(H) access was not checked.\n");
	if(code != NULL) QRcode_free(code);

	input->version = 1;
	input->level = (QRecLevel)-1;
	code = QRcode_encodeInput(input);
	assert_null(code, "invalid level(-1) access was not checked.\n");
	if(code != NULL) QRcode_free(code);

	QRinput_free(input);

	testFinish();
}

static void test_mqrencode(void)
{
	char *str = "MICROQR";
	char pattern[] = {
		"#######_#_#_#_#"
		"#_____#_#__####"
		"#_###_#_#_####_"
		"#_###_#_#__##_#"
		"#_###_#___#__##"
		"#_____#____#_#_"
		"#######__##_#_#"
		"_________#__#__"
		"#___#__####_#_#"
		"_#######_#_##_#"
		"##___#_#____#__"
		"_##_#_####____#"
		"#__###___#__##_"
		"_###_#_###_#_#_"
		"##____####_###_"
	};
	QRcode qrcode;
	QRdata *qrdata;
	unsigned char *frame;
	int i;

	testStart("Encoding test (MQR).");

	qrcode.width = 15;
	qrcode.version = 3;

	frame = MQRspec_newFrame(qrcode.version);
	for(i=0; i<225; i++) {
		frame[i] ^= (pattern[i] == '#')?1:0;
	}

	qrcode.data = frame;
	qrdata = QRcode_decodeMQR(&qrcode);
	assert_equal(qrdata->version, 3, "Format info decoder returns wrong version number: %d (%d expected)\n", qrdata->version, 3);
	assert_equal(qrdata->level, 1, "Format info decoder returns wrong level: %d (%d expected)\n", qrdata->level, 1);
	assert_zero(strcmp((char *)qrdata->data, str), "Decoded data (%s) mismatched (%s)\n", (char *)qrdata->data, str);

	QRdata_free(qrdata);
	free(frame);

	testFinish();
}

static void test_apiversion(void)
{
	int major_version, minor_version, micro_version;
	char *str, *str2;

	testStart("API Version check");
	QRcode_APIVersion(&major_version, &minor_version, &micro_version);
	assert_equal(major_version, MAJOR_VERSION, "Major version number mismatched: %d (%d expected)\n", major_version, MAJOR_VERSION);
	assert_equal(minor_version, MINOR_VERSION, "Minor version number mismatched: %d (%d expected)\n", minor_version, MINOR_VERSION);
	assert_equal(micro_version, MICRO_VERSION, "Micro version number mismatched: %d (%d expected)\n", micro_version, MICRO_VERSION);
	str = QRcode_APIVersionString();
	str2 = QRcode_APIVersionString();
	assert_zero(strcmp(VERSION, str), "Version string mismatched: %s (%s expected)\n", str, VERSION);
	assert_equal(str, str2, "Version strings are not identical.");
	testFinish();
}

int main(int argc, char **argv)
{
	int tests = 33;
	testInit(tests);
	test_iterate();
	test_iterate2();
	test_filler();
	test_format();
	test_encode();
	test_encode2();
	test_encode3();
	test_encodeNull();
	test_encodeEmpty();
	test_encodeNull8();
	test_encodeEmpty8();
	test_encodeLongData();
	test_encodeVer26Num();
	test_01234567();
	test_invalid_input();
	test_struct_example();
	test_struct_semilong();
	test_null_free();
	test_qrraw_new();
	test_mqrraw_new();
	test_encodeData();
	test_formatInfo();
	test_decodeSimple();
	test_decodeLong();
	test_decodeVeryLong();
	test_fillerMQR();
	test_formatInfoMQR();
	test_encodeTooLongMQR();
	test_decodeShortMQR();
	test_oddBitCalcMQR();
	test_invalid_inputMQR();
	test_mqrencode();
	test_apiversion();
	testReport(tests);

	if(argc > 1) {
		print_filler();
		print_01234567();
		print_fillerMQR();
	}

	return 0;
}
