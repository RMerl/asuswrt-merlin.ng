#include <stdio.h>
#include <string.h>
#include "common.h"
#include "../mask.h"
#include "../qrspec.h"
#include "decoder.h"

static char dot[2] = {'_', '#'};
static char *maskPatterns[8] = {
	/* (i + j) mod 2 = 0 */
	"#_#_#_"
	"_#_#_#"
	"#_#_#_"
	"_#_#_#"
	"#_#_#_"
	"_#_#_#",
	/* i mod 2 = 0 */
	"######"
	"______"
	"######"
	"______"
	"######"
	"______",
	/* j mod 3 = 0 */
	"#__#__"
	"#__#__"
	"#__#__"
	"#__#__"
	"#__#__"
	"#__#__",
	/* (i + j) mod 3 = 0 */
	"#__#__"
	"__#__#"
	"_#__#_"
	"#__#__"
	"__#__#"
	"_#__#_",
	/* ((i div 2) + (j div 3)) mod 2 = 0 */
	"###___"
	"###___"
	"___###"
	"___###"
	"###___"
	"###___",
	/* (ij) mod 2 + (ij) mod 3 = 0 */
	"######"
	"#_____"
	"#__#__"
	"#_#_#_"
	"#__#__"
	"#_____",
	/* ((ij) mod 2 + (ij) mod 3) mod 2 = 0 */
	"######"
	"###___"
	"##_##_"
	"#_#_#_"
	"#_##_#"
	"#___##",
	/* ((ij) mod 3 + (i+j) mod 2) mod 2 = 0 */
	"#_#_#_"
	"___###"
	"#___##"
	"_#_#_#"
	"###___"
	"_###__"
};

static void print_mask(int mask)
{
	const unsigned int w = 6;
	unsigned char frame[w * w], *masked, *p;
	int x, y;

	memset(frame, 0, w * w);
	masked = Mask_makeMaskedFrame(w, frame, mask);
	p = masked;
	for(y=0; y<w; y++) {
		for(x=0; x<w; x++) {
			putchar(dot[*p&1]);
			p++;
		}
		printf("\n");
	}
	printf("\n");

	free(masked);
}

static void print_masks(void)
{
	int i;

	puts("\nPrinting mask patterns.");
	for(i=0; i<8; i++) {
		print_mask(i);
	}
}

static int test_mask(int mask)
{
	const int w = 6;
	unsigned char frame[w * w], *masked, *p;
	char *q;
	int x, y;
	int err = 0;

	memset(frame, 0, w * w);
	masked = Mask_makeMaskedFrame(w, frame, mask);
	p = masked;
	q = maskPatterns[mask];
	for(y=0; y<w; y++) {
		for(x=0; x<w; x++) {
			if(dot[*p&1] != *q) {
				err++;
			}
			p++;
			q++;
		}
	}

	free(masked);
	return err;
}

static void test_masks(void)
{
	int i;

	testStart("Mask pattern checks");
	for(i=0; i<8; i++) {
		assert_zero(test_mask(i), "Mask pattern %d incorrect.\n", i);
	}
	testFinish();
}

#define N1 (3)
#define N2 (3)
#define N3 (40)
#define N4 (10)

static void test_eval(void)
{
	unsigned char *frame;
	unsigned int w = 6;
	int demerit;

	frame = (unsigned char *)malloc(w * w);

	testStart("Test mask evaluation (all white)");
	memset(frame, 0, w * w);
	demerit = Mask_evaluateSymbol(w, frame);
	testEndExp(demerit == ((N1 + 1)*w*2 + N2 * (w - 1) * (w - 1)));

	testStart("Test mask evaluation (all black)");
	memset(frame, 1, w * w);
	demerit = Mask_evaluateSymbol(w, frame);
	testEndExp(demerit == ((N1 + 1)*w*2 + N2 * (w - 1) * (w - 1)));

	free(frame);
}

/* .#.#.#.#.#
 * #.#.#.#.#.
 * ..##..##..
 * ##..##..##
 * ...###...#
 * ###...###.
 * ....####..
 * ####....##
 * .....#####
 * #####.....
 */
static void test_eval2(void)
{
	unsigned char *frame;
	unsigned int w = 10;
	int demerit;
	unsigned int x;

	frame = (unsigned char *)malloc(w * w);

	testStart("Test mask evaluation (run length penalty check)");
	for(x=0; x<w; x++) {
		frame[      x] = x & 1;
		frame[w   + x] = (x & 1) ^ 1;
		frame[w*2 + x] = (x / 2) & 1;
		frame[w*3 + x] = ((x / 2) & 1) ^ 1;
		frame[w*4 + x] = (x / 3) & 1;
		frame[w*5 + x] = ((x / 3) & 1) ^ 1;
		frame[w*6 + x] = (x / 4) & 1;
		frame[w*7 + x] = ((x / 4) & 1) ^ 1;
		frame[w*8 + x] = (x / 5) & 1;
		frame[w*9 + x] = ((x / 5) & 1) ^ 1;
	}
	demerit = Mask_evaluateSymbol(w, frame);
	testEndExp(demerit == N1 * 4 + N2 * 4);

	free(frame);
}

static void test_calcN2(void)
{
	unsigned char frame[64];
	int width;
	int demerit;
	int x, y;

	testStart("Test mask evaluation (2x2 block check)");
	width = 4;
	for(y = 0; y < width; y++) {
		for(x = 0; x < width; x++) {
			frame[y * width + x] = ((x & 2) ^ (y & 2)) >> 1;
		}
	}
	demerit = Mask_calcN2(width, frame);
	assert_equal(demerit, N2 * 4, "Calculation of N2 demerit is wrong: %d, expected %d", demerit, N2 * 4);

	width = 4;
	for(y = 0; y < width; y++) {
		for(x = 0; x < width; x++) {
			frame[y * width + x] = (((x + 1) & 2) ^ (y & 2)) >> 1;
		}
	}
	demerit = Mask_calcN2(width, frame);
	assert_equal(demerit, N2 * 2, "Calculation of N2 demerit is wrong: %d, expected %d", demerit, N2 * 2);

	width = 6;
	for(y = 0; y < width; y++) {
		for(x = 0; x < width; x++) {
			frame[y * width + x] = (x / 3) ^ (y / 3);
		}
	}
	demerit = Mask_calcN2(width, frame);
	assert_equal(demerit, N2 * 16, "Calculation of N2 demerit is wrong: %d, expected %d", demerit, N2 * 16);

	testFinish();
}

static void test_eval3(void)
{
	unsigned char *frame;
	int w = 15;
	int demerit;
	int x, y;
	static unsigned char pattern[7][15] = {
		{0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0}, // N3x1
		{1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1}, // N3x1
		{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1}, // N3x1
		{1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0}, // 0
		{1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1}, // N3x2
		{1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0}, // N3 + (N1+1)
		{1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1}  // (N1+1)
	};

	frame = (unsigned char *)malloc(w * w);

	testStart("Test mask evaluation (1:1:3:1:1 check)");

	for(y=0; y<5; y++) {
		for(x=0; x<w; x++) {
			frame[w*y*2     + x] = pattern[y][x];
			frame[w*(y*2+1) + x] = pattern[y][x]^1;
		}
	}
	for(x=0; x<w; x++) {
		frame[w*10 + x] = x & 1;
	}
	for(y=5; y<7; y++) {
		for(x=0; x<w; x++) {
			frame[w*(y*2+1) + x] = pattern[y][x];
			frame[w*(y*2+2) + x] = pattern[y][x]^1;
		}
	}
	/*
	for(y=0; y<w; y++) {
		for(x=0; x<w; x++) {
			printf("%s", frame[w*y+x]?"##":"..");
		}
		printf("\n");
	}
	*/
	demerit = Mask_evaluateSymbol(w, frame);
	testEndExp(demerit == N3 * 10 + (N1 + 1) * 4);

	free(frame);
}

static void test_format(void)
{
	unsigned char *frame, *masked;
	int version, mask, width, dmask;
	QRecLevel level, dlevel;
	QRcode *code;
	int ret;

	testStart("Checking format info.");
	for(version=1; version<=QRSPEC_VERSION_MAX; version++) {
		frame = QRspec_newFrame(version);
		width = QRspec_getWidth(version);
		for(level=QR_ECLEVEL_L; level<=QR_ECLEVEL_H; level++) {
			for(mask=0; mask<8; mask++) {
				masked = Mask_makeMask(width, frame, mask, level);
				code = QRcode_new(version, width, masked);
				ret = QRcode_decodeFormat(code, &dlevel, &dmask);
				assert_zero(ret, "Something wrong in format info.\n");
				assert_equal(dlevel, level, "Decoded level is wrong: %d, expected %d", dlevel, level);
				assert_equal(dmask, mask, "Decoded mask is wrong: %d, expected %d", dlevel, level);
				QRcode_free(code);
			}
		}
		free(frame);
	}
	testFinish();
}

static void test_calcRunLength(void)
{
	int width = 5;
	unsigned char frame[width * width];
	int runLength[width + 1];
	int i, j;
	int length;
	static unsigned char pattern[6][5] = {
		{0, 1, 0, 1, 0},
		{1, 0, 1, 0, 1},
		{0, 0, 0, 0, 0},
		{1, 1, 1, 1, 1},
		{0, 0, 1, 1, 1},
		{1, 1, 0, 0, 0}
	};
	static int expected[6][7] = {
		{ 1, 1, 1, 1, 1, 0, 5},
		{-1, 1, 1, 1, 1, 1, 6},
		{ 5, 0, 0, 0, 0, 0, 1},
		{-1, 5, 0, 0, 0, 0, 2},
		{ 2, 3, 0, 0, 0, 0, 2},
		{-1, 2, 3, 0, 0, 0, 3}
	};

	testStart("Test runlength calc function");
	for(i=0; i<6; i++) {
		length = Mask_calcRunLengthH(width, pattern[i], runLength);
		assert_equal(expected[i][6], length, "Length incorrect: %d, expected %d.\n", length, expected[i][6]);
		assert_zero(memcmp(runLength, expected[i], sizeof(int) * expected[i][6]), "Run length does not match: pattern %d, horizontal access.\n", i);
		for(j=0; j<width; j++) {
			frame[j * width] = pattern[i][j];
		}
		length = Mask_calcRunLengthV(width, frame, runLength);
		assert_equal(expected[i][6], length, "Length incorrect: %d, expected %d.\n", length, expected[i][6]);
		assert_zero(memcmp(runLength, expected[i], sizeof(int) * expected[i][6]), "Run length does not match: pattern %d, vertical access.\n", i);
	}
	testFinish();
}

static void test_calcN1N3(void)
{
	int runLength[26];
	int length;
	int demerit;
	int i;
	static unsigned char pattern[][16] = {
		{1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, N3},
		{0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, N3},
		{1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0},
		{1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, N3},
		{1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, N3},
		{1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, N3 * 2},
	};

	static unsigned char pattern2[][19] = {
		{1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, N3 + N1 + 1},
		{0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, N3 + N1 + 1},
		{1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, N1 + 1},
		{1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, N3 + N1 + 1},
		{1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, N3 + N1 + 1}
	};

	testStart("Test N3 penalty calculation");
	for(i=0; i<6; i++) {
		length = Mask_calcRunLengthH(15, pattern[i], runLength);
		demerit = Mask_calcN1N3(length, runLength);
		assert_equal(pattern[i][15], demerit, "N3 penalty is wrong: %d, expected %d\n", demerit, pattern[i][15]);
	}
	for(i=0; i<5; i++) {
		length = Mask_calcRunLengthH(18, pattern2[i], runLength);
		demerit = Mask_calcN1N3(length, runLength);
		assert_equal(pattern2[i][18], demerit, "N3 penalty is wrong: %d, expected %d\n", demerit, pattern2[i][18]);
	}
	testFinish();
}

int main(int argc, char **argv)
{
	int tests = 9;
	testInit(tests);
	test_masks();
	test_eval();
	test_eval2();
	test_eval3();
	test_format();
	test_calcN2();
	test_calcRunLength();
	test_calcN1N3();
	testReport(tests);

	if(argc > 1) {
		print_masks();
	}

	return 0;
}
