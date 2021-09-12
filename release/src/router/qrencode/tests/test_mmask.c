#include <stdio.h>
#include <string.h>
#include "common.h"
#include "../mmask.h"
#include "../mqrspec.h"

static char dot[2] = {'_', '#'};
static char *maskPatterns[4] = {
	/* i mod 2 = 0 */
	"######"
	"______"
	"######"
	"______"
	"######"
	"______",
	/* ((i div 2) + (j div 3)) mod 2 = 0 */
	"###___"
	"###___"
	"___###"
	"___###"
	"###___"
	"###___",
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
	const int w = 6;
	unsigned char frame[w * w], *masked, *p;
	int x, y;

	memset(frame, 0, w * w);
	masked = MMask_makeMaskedFrame(w, frame, mask);
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
	for(i=0; i<4; i++) {
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
	masked = MMask_makeMaskedFrame(w, frame, mask);
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
	for(i=0; i<4; i++) {
		assert_zero(test_mask(i), "Mask pattern %d incorrect.\n", i);
	}
	testFinish();
}

static void test_maskEvaluation(void)
{
	static const int w = 11;
	unsigned char pattern[w * w];
	int i, score;

	memset(pattern, 0, w * w);

	testStart("Test mask evaluation");
	score = MMask_evaluateSymbol(w, pattern);
	assert_equal(score, 0, "Mask score caluculation is incorrect. (score=%d (%d expected)\n", score, 0);

	for(i=0; i<w; i++) {
		pattern[(w-1) * w + i] = 1;
	}
	score = MMask_evaluateSymbol(w, pattern);
	assert_equal(score, 16 + w - 1, "Mask score caluculation is incorrect. (score=%d) (%d expected)\n", score, 16 + w - 1);

	for(i=0; i<w; i++) {
		pattern[(w-1) * w + i] = 0;
		pattern[i * w + w - 1] = 1;
	}
	score = MMask_evaluateSymbol(w, pattern);
	assert_equal(score, 16 + w - 1, "Mask score caluculation is incorrect. (score=%d) (%d expected)\n", score, 16 + w - 1);

	for(i=0; i<w; i++) {
		pattern[(w-1) * w + i] = 1;
		pattern[i * w + w - 1] = 1;
	}
	score = MMask_evaluateSymbol(w, pattern);
	assert_equal(score, 16 * (w - 1) + w - 1, "Mask score caluculation is incorrect. (score=%d) (%d expected)\n", score, 16 * (w - 1) + w - 1);

	testFinish();
}

int main(int argc, char **argv)
{
	int tests = 2;
	testInit(tests);
	test_masks();
	test_maskEvaluation();
	testReport(tests);

	if(argc > 1) {
		print_masks();
	}

	return 0;
}
