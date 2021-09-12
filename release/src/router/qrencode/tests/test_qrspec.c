#include <stdio.h>
#include <string.h>
#include "common.h"
#include "../qrspec.h"
#include "../qrencode_inner.h"
#include "decoder.h"

#ifndef SRCDIR
#	define SRCDIR
#endif

static void print_eccTable(void)
{
	int i, j;
	int ecc;
	int data;
	int spec[5];

	puts("\nPrinting ECC table.\n");
	for(i=1; i<=QRSPEC_VERSION_MAX; i++) {
		printf("Version %2d\n", i);
		for(j=0; j<4; j++) {
			QRspec_getEccSpec(i, (QRecLevel)j, spec);
			data = QRspec_rsBlockNum1(spec) * QRspec_rsDataCodes1(spec)
			     + QRspec_rsBlockNum2(spec) * QRspec_rsDataCodes2(spec);
			ecc  = QRspec_rsBlockNum1(spec) * QRspec_rsEccCodes1(spec)
			     + QRspec_rsBlockNum2(spec) * QRspec_rsEccCodes2(spec);
			printf("%3d\t", data);
			printf("%3d\t", ecc);
			printf("%2d\t", QRspec_rsBlockNum1(spec));
			printf("(%3d, %3d, %3d)\n",
				   QRspec_rsDataCodes1(spec) + QRspec_rsEccCodes1(spec),
				   QRspec_rsDataCodes1(spec),
				   QRspec_rsEccCodes1(spec));
			if(QRspec_rsBlockNum2(spec) > 0) {
				printf("\t%2d\t", QRspec_rsBlockNum2(spec));
				printf("(%3d, %3d, %3d)\n",
					   QRspec_rsDataCodes2(spec) + QRspec_rsEccCodes2(spec),
					   QRspec_rsDataCodes2(spec),
					   QRspec_rsEccCodes2(spec));
			}
		}
	}
}

static void test_eccTable(void)
{
	int i, j;
	int ecc;
	int data;
	int err = 0;
	int spec[5];

	testStart("Checking ECC table.");
	for(i=1; i<=QRSPEC_VERSION_MAX; i++) {
		for(j=0; j<4; j++) {
			QRspec_getEccSpec(i, (QRecLevel)j, spec);
			data = QRspec_rsBlockNum1(spec) * QRspec_rsDataCodes1(spec)
			     + QRspec_rsBlockNum2(spec) * QRspec_rsDataCodes2(spec);
			ecc  = QRspec_rsBlockNum1(spec) * QRspec_rsEccCodes1(spec)
			     + QRspec_rsBlockNum2(spec) * QRspec_rsEccCodes2(spec);
			if(data + ecc != QRspec_getDataLength(i, (QRecLevel)j) + QRspec_getECCLength(i, (QRecLevel)j)) {
				printf("Error in version %d, level %d: invalid size\n", i, j);
				printf("%d %d %d %d %d %d\n", spec[0], spec[1], spec[2], spec[3], spec[4], spec[2]);
				err++;
			}
			if(ecc != QRspec_getECCLength(i, (QRecLevel)j)) {
				printf("Error in version %d, level %d: invalid data\n", i, j);
				printf("%d %d %d %d %d %d\n", spec[0], spec[1], spec[2], spec[3], spec[4], spec[2]);
				err++;
			}
		}
	}
	testEnd(err);
}

static void test_eccTable2(void)
{
	int i;
	int spec[5];

	const int correct[7][6] = {
		{ 8,  1, 0,  2, 60, 38},
		{ 8,  1, 1,  2, 61, 39},
		{24,  2, 0, 11, 54, 24},
		{24,  2, 1, 16, 55, 25},
		{32,  0, 0, 17, 145, 115},
		{40,  3, 0, 20, 45, 15},
		{40,  3, 1, 61, 46, 16},
	};

	testStart("Checking ECC table(2)");
	for(i=0; i<7; i++) {
		QRspec_getEccSpec(correct[i][0], (QRecLevel)correct[i][1], spec);
		if(correct[i][2] == 0) {
			assert_equal(QRspec_rsBlockNum1(spec), correct[i][3],
				"Error in version %d, level %d. rsBlockNum1 was %d, expected %d.\n",
				correct[i][0], correct[i][1],
				QRspec_rsBlockNum1(spec), correct[i][3]);
			assert_equal(QRspec_rsDataCodes1(spec) + QRspec_rsEccCodes1(spec), correct[i][4],
				"Error in version %d, level %d. rsDataCodes1 + rsEccCodes1 was %d, expected %d.\n",
				correct[i][0], correct[i][1],
				QRspec_rsDataCodes1(spec) + QRspec_rsEccCodes1(spec), correct[i][4]);
			assert_equal(QRspec_rsDataCodes1(spec), correct[i][5],
				"Error in version %d, level %d. rsDataCodes1 was %d, expected %d.\n",
				correct[i][0], correct[i][1],
				QRspec_rsDataCodes1(spec), correct[i][5]);
		} else {
			assert_equal(QRspec_rsBlockNum2(spec), correct[i][3],
				"Error in version %d, level %d. rsBlockNum2 was %d, expected %d.\n",
				correct[i][0], correct[i][1],
				QRspec_rsBlockNum2(spec), correct[i][3]);
			assert_equal(QRspec_rsDataCodes2(spec) + QRspec_rsEccCodes2(spec), correct[i][4],
				"Error in version %d, level %d. rsDataCodes2 + rsEccCodes2 was %d, expected %d.\n",
				correct[i][0], correct[i][1],
				QRspec_rsDataCodes2(spec) + QRspec_rsEccCodes2(spec), correct[i][4]);
			assert_equal(QRspec_rsDataCodes2(spec), correct[i][5],
				"Error in version %d, level %d. rsDataCodes2 was %d, expected %d.\n",
				correct[i][0], correct[i][1],
				QRspec_rsDataCodes2(spec), correct[i][5]);
		}
	}
	testFinish();
}

static void test_newframe(void)
{
	unsigned char buf[QRSPEC_WIDTH_MAX * QRSPEC_WIDTH_MAX];
	int i, width;
	size_t len;
	FILE *fp;
	unsigned char *frame;
	QRcode *qrcode;
	int version;

	testStart("Checking newly created frame.");
	fp = fopen(SRCDIR "frame", "rb");
	if(fp == NULL) {
		perror("Failed to open \"" SRCDIR "frame\":");
		abort();
	}
	for(i=1; i<=QRSPEC_VERSION_MAX; i++) {
		frame = QRspec_newFrame(i);
		width = QRspec_getWidth(i);
		len = fread(buf, 1, width * width, fp);
		if((int)len != width * width) {
			perror("Failed to read the pattern file:");
			abort();
		}
		assert_zero(memcmp(frame, buf, len), "frame pattern mismatch (version %d)\n", i);
		qrcode = QRcode_new(i, width, frame);
		version = QRcode_decodeVersion(qrcode);
		assert_equal(version, i, "Decoded version number is wrong: %d, expected %d.\n", version, i);
		QRcode_free(qrcode);
	}

	testFinish();
	fclose(fp);
}

static void test_newframe_invalid(void)
{
	unsigned char *frame;

	testStart("Checking QRspec_newFrame with invalid version.");
	frame = QRspec_newFrame(0);
	assert_null(frame, "QRspec_newFrame(0) returns non-NULL.");
	frame = QRspec_newFrame(QRSPEC_VERSION_MAX+1);
	assert_null(frame, "QRspec_newFrame(0) returns non-NULL.");
	testFinish();
}

#if 0
/* This test is used to check positions of alignment pattern. See Appendix E
 * (p.71) of JIS X0510:2004 and compare to the output. Before comment out
 * this test, change the value of the pattern marker's center dot from 0xa1
 * to 0xb1 (QRspec_putAlignmentMarker() : finder).
 */
static void test_alignment(void)
{
	unsigned char *frame;
	int i, x, y, width, c;

	testStart("Checking alignment pattern.");
	for(i=2; i<=QRSPEC_VERSION_MAX; i++) {
		printf("%2d", i);
		frame = QRspec_newFrame(i);
		width = QRspec_getWidth(i);
		c = 0;
		for(x=0; x<width * width; x++) {
			if(frame[x] == 0xb1) {
				c++;
			}
		}
		printf("|%2d|   6", c);
		y = width - 7;
		for(x=0; x < width; x++) {
			if(frame[y * width + x] == 0xb1) {
				printf(", %3d", x);
			}
		}
		printf("\n");
		free(frame);
	}
	testFinish();
}
#endif

static void test_verpat(void)
{
	int version;
	unsigned int pattern;
	int err = 0;
	unsigned int data;
	unsigned int code;
	int i, c;
	unsigned int mask;

	testStart("Checking version pattern.");
	for(version=7; version <= QRSPEC_VERSION_MAX; version++) {
		pattern = QRspec_getVersionPattern(version);
		if((pattern >> 12) != (unsigned int)version) {
			printf("Error in version %d.\n", version);
			err++;
			continue;
		}
		mask = 0x40;
		for(i=0; mask != 0; i++) {
			if(version & mask) break;
			mask = mask >> 1;
		}
		c = 6 - i;
		data = version << 12;
		code = 0x1f25 << c;
		mask = 0x40000 >> (6 - c);
		for(i=0; i<=c; i++) {
			if(mask & data) {
				data ^= code;
			}
			code = code >> 1;
			mask = mask >> 1;
		}
		data = (version << 12) | (data & 0xfff);
		if(data != pattern) {
			printf("Error in version %d\n", version);
			err++;
		}
	}
	testEnd(err);
}

/* See Table 22 (p.45) and Appendix C (p. 65) of JIS X0510:2004 */
static unsigned int levelIndicator[4] = {1, 0, 3, 2};
static unsigned int calcFormatInfo(int mask, QRecLevel level)
{
	unsigned int data, ecc, b, code;
	int i, c;

	data = (levelIndicator[level] << 13) | (mask << 10);
	ecc = data;
	b = 1 << 14;
	for(i=0; b != 0; i++) {
		if(ecc & b) break;
		b = b >> 1;
	}
	c = 4 - i;
	code = 0x537 << c ; //10100110111
	b = 1 << (10 + c);
	for(i=0; i<=c; i++) {
		if(b & ecc) {
			ecc ^= code;
		}
		code = code >> 1;
		b = b >> 1;
	}

	return (data | ecc) ^ 0x5412;
}

static void test_format(void)
{
	unsigned int format;
	int i, j;
	int err = 0;

	testStart("Format info test");
	for(i=0; i<4; i++) {
		for(j=0; j<8; j++) {
			format = calcFormatInfo(j, (QRecLevel)i);
//			printf("0x%04x, ", format);
			if(format != QRspec_getFormatInfo(j, (QRecLevel)i)) {
				printf("Level %d, mask %x\n", i, j);
				err++;
			}
		}
//		printf("\n");
	}
	testEnd(err);
}

int main(int argc, char **argv)
{
	int tests = 6;
	testInit(tests);
	test_eccTable();
	test_eccTable2();
	test_newframe();
	test_newframe_invalid();
	//test_alignment();
	test_verpat();
	test_format();
	testReport(tests);

	if(argc > 1) {
		print_eccTable();
	}

	return 0;
}
