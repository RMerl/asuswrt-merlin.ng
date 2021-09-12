#include <stdio.h>
#include <string.h>
#include "common.h"
#include "../bitstream.h"

static void test_null(void)
{
	BitStream *bstream;

	testStart("Empty stream");
	bstream = BitStream_new();
	assert_zero(BitStream_size(bstream), "Size of empty BitStream is not 0.\n");
	assert_null(BitStream_toByte(bstream), "BitStream_toByte returned non-NULL.\n");
	assert_nothing(BitStream_free(NULL), "Checking BitStream_free(NULL).\n");
	testFinish();

	BitStream_free(bstream);
}

static void test_num(void)
{
	BitStream *bstream;
	unsigned int data = 0x13579bdf;
	char correct[] = "0010011010101111001101111011111";

	testStart("New from num");
	bstream = BitStream_new();
	BitStream_appendNum(bstream, 31, data);
	testEnd(cmpBin(correct, bstream));

	BitStream_free(bstream);
}

static void test_bytes(void)
{
	BitStream *bstream;
	unsigned char data[1] = {0x3a};
	char correct[] = "00111010";

	testStart("New from bytes");
	bstream = BitStream_new();
	BitStream_appendBytes(bstream, 1, data);
	testEnd(cmpBin(correct, bstream));
	BitStream_free(bstream);
}

static void test_appendNum(void)
{
	BitStream *bstream;
	char correct[] = "10001010 11111111 11111111 00010010001101000101011001111000";

	testStart("Append Num");
	bstream = BitStream_new();

	BitStream_appendNum(bstream,  8, 0x0000008a);
	assert_zero(ncmpBin(correct, bstream, 8), "Internal data is incorrect.\n");

	BitStream_appendNum(bstream, 16, 0x0000ffff);
	assert_zero(ncmpBin(correct, bstream, 24), "Internal data is incorrect.\n");

	BitStream_appendNum(bstream, 32, 0x12345678);

	assert_zero(cmpBin(correct, bstream), "Internal data is incorrect.\n");
	testFinish();

	BitStream_free(bstream);
}

static void test_appendBytes(void)
{
	BitStream *bstream;
	unsigned char data[8];
	char correct[] = "10001010111111111111111100010010001101000101011001111000";

	testStart("Append Bytes");
	bstream = BitStream_new();

	data[0] = 0x8a;
	BitStream_appendBytes(bstream,  1, data);
	assert_zero(ncmpBin(correct, bstream, 8), "Internal data is incorrect.");

	data[0] = 0xff;
	data[1] = 0xff;
	BitStream_appendBytes(bstream, 2, data);
	assert_zero(ncmpBin(correct, bstream, 24), "Internal data is incorrect.\n");

	data[0] = 0x12;
	data[1] = 0x34;
	data[2] = 0x56;
	data[3] = 0x78;
	BitStream_appendBytes(bstream, 4, data);

	assert_zero(cmpBin(correct, bstream), "Internal data is incorrect.\n");
	testFinish();

	BitStream_free(bstream);
}

static void test_toByte(void)
{
	BitStream *bstream;
	unsigned char correct[] = {
		0x8a, 0xff, 0xff, 0x12, 0x34, 0x56, 0x78
	};
	unsigned char *result;

	testStart("Convert to a byte array");
	bstream = BitStream_new();

	BitStream_appendBytes(bstream, 1, &correct[0]);
	BitStream_appendBytes(bstream, 2, &correct[1]);
	BitStream_appendBytes(bstream, 4, &correct[3]);

	result = BitStream_toByte(bstream);
	testEnd(memcmp(correct, result, 7));

	BitStream_free(bstream);
	free(result);
}

static void test_toByte_4bitpadding(void)
{
	BitStream *bstream;
	unsigned char *result;

	testStart("Convert to a byte array");

	bstream = BitStream_new();
	BitStream_appendNum(bstream, 4, 0xb);
	result = BitStream_toByte(bstream);
	assert_equal(result[0], 0xb0, "incorrect paddings\n");
	BitStream_free(bstream);
	free(result);

	bstream = BitStream_new();
	BitStream_appendNum(bstream, 12, 0x335);
	result = BitStream_toByte(bstream);
	assert_equal(result[0], 0x33, "incorrect paddings\n");
	assert_equal(result[1], 0x50, "incorrect paddings\n");
	BitStream_free(bstream);
	free(result);

	testFinish();

}

static void test_size(void)
{
	BitStream *bstream;

	testStart("size check");
	bstream = BitStream_new();
	assert_equal(BitStream_size(bstream), 0, "Initialized BitStream is not 0 length");
	BitStream_appendNum(bstream, 1, 0);
	assert_equal(BitStream_size(bstream), 1, "Size incorrect. (first append)");
	BitStream_appendNum(bstream, 2, 0);
	assert_equal(BitStream_size(bstream), 3, "Size incorrect. (second append)");
	testFinish();

	BitStream_free(bstream);
}

static void test_append(void)
{
	BitStream *bs1, *bs2;
	char c1[] = "00";
	char c2[] = "0011";
	char c3[] = "01111111111111111";
	char c4[] = "001101111111111111111";
	char c5[] = "0011011111111111111111111111111111";
	int ret;

	testStart("Append two BitStreams");

	bs1 = BitStream_new();
	bs2 = BitStream_new();
	ret = BitStream_appendNum(bs1, 1, 0);
	ret = BitStream_appendNum(bs2, 1, 0);

	ret = BitStream_append(bs1, bs2);
	assert_zero(ret, "Failed to append.");
	assert_zero(cmpBin(c1, bs1), "Internal data is incorrect.");

	ret = BitStream_appendNum(bs1, 2, 3);
	assert_zero(ret, "Failed to append.");
	assert_zero(cmpBin(c2, bs1), "Internal data is incorrect.");

	ret = BitStream_appendNum(bs2, 16, 65535);
	assert_zero(ret, "Failed to append.");
	assert_zero(cmpBin(c3, bs2), "Internal data is incorrect.");

	ret = BitStream_append(bs1, bs2);
	assert_zero(ret, "Failed to append.");
	assert_zero(cmpBin(c4, bs1), "Internal data is incorrect.");

	ret = BitStream_appendNum(bs1, 13, 16383);
	assert_zero(ret, "Failed to append.");
	assert_zero(cmpBin(c5, bs1), "Internal data is incorrect.");

	testFinish();

	BitStream_free(bs1);
	BitStream_free(bs2);
}

static void test_newWithBits(void)
{
	BitStream *bstream;
	unsigned char data[4] = {0, 1, 0, 1};

	testStart("New with bits");

	bstream = BitStream_newWithBits(4, data);
	assert_equal(bstream->length, 4, "Internal bit length is incorrect.\n");
	assert_equal(bstream->datasize, 4, "Internal buffer size is incorrect.\n");
	assert_zero(cmpBin("0101", bstream), "Internal data is incorrect.\n");

	testFinish();

	BitStream_free(bstream);
}

static void test_newWithBits_size0(void)
{
	BitStream *bstream;

	testStart("New with bits (size = 0)");

	bstream = BitStream_newWithBits(0, NULL);
	assert_equal(bstream->length, 0, "Internal bit length is incorrect.\n");
	assert_nonzero(bstream->datasize, "Internal buffer size is incorrect.\n");
	assert_nonnull(bstream->data, "Internal buffer not allocated.\n");

	testFinish();

	BitStream_free(bstream);
}

int main()
{
	int tests = 11;
	testInit(tests);
	test_null();
	test_num();
	test_bytes();
	test_appendNum();
	test_appendBytes();
	test_toByte();
	test_toByte_4bitpadding();
	test_size();
	test_append();
	test_newWithBits();
	test_newWithBits_size0();
	testReport(tests);

	return 0;
}
