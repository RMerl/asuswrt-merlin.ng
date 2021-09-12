#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "common.h"
#include "../qrspec.h"
#include "../qrinput.h"
#include "../mask.h"
#include "../split.h"
#include "../bitstream.h"

static int inputTest(QRinput_List *list, const char *fmt, ...)
{
	va_list ap;
	int size;
	QRencodeMode mode;
	int i, err = 0;

	va_start(ap, fmt);
	i = 1;
	while(*fmt) {
		if(list == NULL) {
			err = 1;
			break;
		}
		size = va_arg(ap, int);
		if(list->size != size) {
			err = 1;
			break;
		}

		switch(*fmt++) {
		case 'n':
			mode = QR_MODE_NUM;
			break;
		case 'a':
			mode = QR_MODE_AN;
			break;
		case 'k':
			mode = QR_MODE_KANJI;
			break;
		case '8':
			mode = QR_MODE_8;
			break;
		default:
			return -1;
			break;
		}
		if(list->mode != mode) {
			err = 1;
			break;
		}
		list = list->next;
		i++;
	}
	va_end(ap);
	if(list != NULL) {
		err = 1;
	}
	if(err) {
		return -i;
	}
	return 0;
}

static int inputSize(QRinput *input)
{
	BitStream *bstream;
	int size;

	bstream = BitStream_new();
	QRinput_mergeBitStream(input, bstream);
	size = BitStream_size(bstream);
	BitStream_free(bstream);

	return size;
}

static void test_split1(void)
{
	QRinput *input;
	BitStream *bstream;

	testStart("Split test: null string");
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput("", input, QR_MODE_8, 0);
	bstream = BitStream_new();
	QRinput_mergeBitStream(input, bstream);
	testEndExp(BitStream_size(bstream) == 0);
	QRinput_free(input);
	BitStream_free(bstream);
}

static void test_split2(void)
{
	QRinput *input;
	QRinput_List *list;
	int err = 0;

	testStart("Split test: single typed strings (num)");
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput("0123", input, QR_MODE_8, 0);
	list = input->head;
	if(inputTest(list, "n", 4)) {
		err++;
	}
	testEnd(err);
	QRinput_free(input);

	err = 0;
	testStart("Split test: single typed strings (num2)");
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput("12345678901234567890", input, QR_MODE_KANJI, 0);
	list = input->head;
	if(inputTest(list, "n", 20)) {
		err++;
	}
	testEnd(err);
	QRinput_free(input);
}

static void test_split3(void)
{
	QRinput *input;
	QRinput_List *list;
	int err = 0;

	testStart("Split test: single typed strings (an)");
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput("ab:-E", input, QR_MODE_8, 0);
	list = input->head;
	if(inputTest(list, "a", 5)) {
		printQRinputInfo(input);
		err++;
	}
	testEnd(err);
	QRinput_free(input);

	err = 0;
	testStart("Split test: num + an");
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput("0123abcde", input, QR_MODE_KANJI, 0);
	list = input->head;
	if(inputTest(list, "a", 9)) {
		err++;
	}
	testEnd(err);
	QRinput_free(input);

	err = 0;
	testStart("Split test: an + num + an");
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput("Ab345fg", input, QR_MODE_KANJI, 0);
	list = input->head;
	if(inputTest(list, "a", 7)) {
		err++;
	}
	testEnd(err);
	QRinput_free(input);
}

static void test_split4(void)
{
	QRinput *input;
	QRinput *i1, *i2;
	int s1, s2, size;
#define CHUNKA "ABCDEFGHIJK"
#define CHUNKB "123456"
#define CHUNKC "1234567"

	testStart("Split test: an and num entries");
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput(CHUNKA/**/CHUNKB, input, QR_MODE_8, 0);
	i1 = QRinput_new();
	QRinput_append(i1, QR_MODE_AN, 17, (unsigned char *)CHUNKA/**/CHUNKB);
	i2 = QRinput_new();
	QRinput_append(i2, QR_MODE_AN, 11, (unsigned char *)CHUNKA);
	QRinput_append(i2, QR_MODE_NUM, 6, (unsigned char *)CHUNKB);

	size = inputSize(input);
	s1 = inputSize(i1);
	s2 = inputSize(i2);
	testEndExp(size == ((s1 < s2)?s1:s2));
	QRinput_free(input);
	QRinput_free(i1);
	QRinput_free(i2);

	testStart("Split test: num and an entries");
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput(CHUNKB/**/CHUNKA, input, QR_MODE_8, 0);
	i1 = QRinput_new();
	QRinput_append(i1, QR_MODE_AN, 17, (unsigned char *)CHUNKB/**/CHUNKA);
	i2 = QRinput_new();
	QRinput_append(i2, QR_MODE_NUM, 6, (unsigned char *)CHUNKB);
	QRinput_append(i2, QR_MODE_AN, 11, (unsigned char *)CHUNKA);

	size = inputSize(input);
	s1 = inputSize(i1);
	s2 = inputSize(i2);
	testEndExp(size == ((s1 < s2)?s1:s2));
	QRinput_free(input);
	QRinput_free(i1);
	QRinput_free(i2);

	testStart("Split test: num and an entries (should be splitted)");
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput(CHUNKC/**/CHUNKA, input, QR_MODE_8, 0);
	i1 = QRinput_new();
	QRinput_append(i1, QR_MODE_AN, 18, (unsigned char *)CHUNKC/**/CHUNKA);
	i2 = QRinput_new();
	QRinput_append(i2, QR_MODE_NUM, 7, (unsigned char *)CHUNKC);
	QRinput_append(i2, QR_MODE_AN, 11, (unsigned char *)CHUNKA);

	size = inputSize(input);
	s1 = inputSize(i1);
	s2 = inputSize(i2);
	testEndExp(size == ((s1 < s2)?s1:s2));
	QRinput_free(input);
	QRinput_free(i1);
	QRinput_free(i2);
}

static void test_split5(void)
{
	QRinput *input;
	QRinput_List *list;
	int err = 0;

	testStart("Split test: bit, an, bit, num");
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput("\x82\xd9""abcdeabcdea\x82\xb0""123456", input, QR_MODE_8, 0);
	list = input->head;
	if(inputTest(list, "8a8n", 2, 11, 2, 6)) {
		err++;
	}
	testEnd(err);
	QRinput_free(input);
}

static void test_split6(void)
{
	QRinput *input;
	QRinput_List *list;
	int err = 0;

	testStart("Split test: kanji, an, kanji, num");
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput("\x82\xd9""abcdeabcdea\x82\xb0""123456", input, QR_MODE_KANJI, 0);
	list = input->head;
	if(inputTest(list, "kakn", 2, 11, 2, 6)) {
		printQRinputInfo(input);
		err++;
	}
	testEnd(err);
	QRinput_free(input);
}

static void test_split7(void)
{
	QRinput *input;
	QRinput_List *list;
	int err = 0;

	testStart("Split test: an and num as bits");
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput("\x82\xd9""abcde\x82\xb0""12345", input, QR_MODE_8, 0);
	list = input->head;
	if(inputTest(list, "8n", 9, 5)) {
		err++;
	}
	testEnd(err);
	QRinput_free(input);
}

static void test_split8(void)
{
	QRinput *input;
	QRinput_List *list;
	int err = 0;

	testStart("Split test: terminated with a half of kanji code");
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput("\x82\xd9""abcdefgh\x82", input, QR_MODE_KANJI, 0);
	list = input->head;
	if(inputTest(list, "ka8", 2, 8, 1)) {
		err++;
	}
	testEnd(err);
	QRinput_free(input);
}

static void test_split3c(void)
{
	QRinput *input;
	QRinput_List *list;
	int err = 0;

	testStart("Split test: single typed strings (an, case-sensitive)");
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput("ab:-E", input, QR_MODE_8, 1);
	list = input->head;
	if(inputTest(list, "8", 5)) {
		err++;
	}
	testEnd(err);
	QRinput_free(input);

	err = 0;
	testStart("Split test: num + an");
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput("0123abcde", input, QR_MODE_KANJI, 1);
	list = input->head;
	if(inputTest(list, "n8", 4, 5)) {
		err++;
	}
	testEnd(err);
	QRinput_free(input);

	err = 0;
	testStart("Split test: an + num + an");
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput("Ab345fg", input, QR_MODE_KANJI, 1);
	list = input->head;
	if(inputTest(list, "8", 7)) {
		printQRinputInfo(input);
		err++;
	}
	testEnd(err);
	QRinput_free(input);
}

static void test_toupper(void)
{
	QRinput *input;
	QRinput_List *list;
	int err = 0;

	testStart("Split test: check dupAndToUpper (lower->upper)");
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput("abcde", input, QR_MODE_8, 0);
	list = input->head;
	if(inputTest(list, "a", 5)) {
		err++;
	}
	if(strncmp((char *)list->data, "ABCDE", list->size)) {
		err++;
	}
	testEnd(err);
	QRinput_free(input);

	err = 0;
	testStart("Split test: check dupAndToUpper (kanji)");
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput("\x83n\x83q\x83t\x83w\x83z", input, QR_MODE_KANJI, 0);
	list = input->head;
	if(inputTest(list, "k", 10)) {
		printQRinputInfo(input);
		err++;
	}
	if(strncmp((char *)list->data, "\x83n\x83q\x83t\x83w\x83z", list->size)) {
		err++;
	}
	testEnd(err);
	QRinput_free(input);

	err = 0;
	testStart("Split test: check dupAndToUpper (8bit)");
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput("\x83n\x83q\x83t\x83w\x83z", input, QR_MODE_8, 0);
	list = input->head;
	if(inputTest(list, "8", 10)) {
		printQRinputInfo(input);
		err++;
	}
	if(strncmp((char *)list->data, "\x83N\x83Q\x83T\x83W\x83Z", list->size)) {
		err++;
	}
	testEnd(err);
	QRinput_free(input);
}

static void test_splitNum8(void)
{
	QRinput *input;
	QRinput_List *list;
	int err = 0;

	testStart("Split test: num and 8bit to 8bit");
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput("1abcdefg", input, QR_MODE_8, 1);
	list = input->head;
	if(inputTest(list, "8", 8)) {
		err++;
		printQRinputInfo(input);
	}
	testEnd(err);
	QRinput_free(input);
}

static void test_splitAnNAn(void)
{
	QRinput *input1, *input2, *input3;
	int s1, s2, s3;
	char *strall = "326A80A9C5004C0875571F8B71C311F2F86";
	char *str1 = "326A80A9C5004C";
	char *str2 = "0875571";
	char *str3 = "F8B71C311F2F86";

	testStart("Split test: An-N-An switching cost test");
	input1 = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput(strall, input1, QR_MODE_8, 0);

	input2 = QRinput_new();
	QRinput_append(input2, QR_MODE_AN, 35, (unsigned char *)strall);

	input3 = QRinput_new();
	QRinput_append(input3, QR_MODE_AN, 14, (unsigned char *)str1);
	QRinput_append(input3, QR_MODE_NUM, 7, (unsigned char *)str2);
	QRinput_append(input3, QR_MODE_AN, 14, (unsigned char *)str3);

	s1 = inputSize(input1);
	s2 = inputSize(input2);
	s3 = inputSize(input3);

	assert_equal(s1, s2, "Incorrect split");
	assert_exp(s2 < s3, "Incorrect estimation");
	testFinish();
	QRinput_free(input1);
	QRinput_free(input2);
	QRinput_free(input3);
}

static void test_splitAn8An(void)
{
	QRinput *input1, *input2, *input3;
	int s1, s2, s3;
	char *strall = "ABCDabcdefABCD";
	char *str1 = "ABCD";
	char *str2 = "abcdef";
	char *str3 = "ABCD";

	testStart("Split test: An-8-An switching cost test");
	input1 = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput(strall, input1, QR_MODE_8, 1);

	input2 = QRinput_new();
	QRinput_append(input2, QR_MODE_8, 14, (unsigned char *)strall);

	input3 = QRinput_new();
	QRinput_append(input3, QR_MODE_AN,  4, (unsigned char *)str1);
	QRinput_append(input3, QR_MODE_8, 6, (unsigned char *)str2);
	QRinput_append(input3, QR_MODE_AN,  4, (unsigned char *)str3);

	s1 = inputSize(input1);
	s2 = inputSize(input2);
	s3 = inputSize(input3);

	assert_equal(s1, s2, "Incorrect split");
	assert_exp(s2 < s3, "Incorrect estimation");
	testFinish();
	QRinput_free(input1);
	QRinput_free(input2);
	QRinput_free(input3);
}

static void test_split8An8(void)
{
	QRinput *input1, *input2, *input3;
	int s1, s2, s3;
	char *strall = "abcABCDEFGHabc";
	char *str1 = "abc";
	char *str2 = "ABCDEFGH";
	char *str3 = "abc";

	testStart("Split test: 8-An-8 switching cost test");
	input1 = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput(strall, input1, QR_MODE_8, 1);

	input2 = QRinput_new();
	QRinput_append(input2, QR_MODE_8, 14, (unsigned char *)strall);

	input3 = QRinput_new();
	QRinput_append(input3, QR_MODE_8,  3, (unsigned char *)str1);
	QRinput_append(input3, QR_MODE_AN, 8, (unsigned char *)str2);
	QRinput_append(input3, QR_MODE_8,  3, (unsigned char *)str3);

	s1 = inputSize(input1);
	s2 = inputSize(input2);
	s3 = inputSize(input3);

	assert_equal(s1, s2, "Incorrect split");
	assert_exp(s2 < s3, "Incorrect estimation");
	testFinish();
	QRinput_free(input1);
	QRinput_free(input2);
	QRinput_free(input3);
}

static void test_split8N8(void)
{
	QRinput *input1, *input2, *input3;
	int s1, s2, s3;
	char *strall = "abc1234abc";
	char *str1 = "abc";
	char *str2 = "1234";
	char *str3 = "abc";

	testStart("Split test: 8-N-8 switching cost test");
	input1 = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput(strall, input1, QR_MODE_8, 1);

	input2 = QRinput_new();
	QRinput_append(input2, QR_MODE_8, 10, (unsigned char *)strall);

	input3 = QRinput_new();
	QRinput_append(input3, QR_MODE_8,   3, (unsigned char *)str1);
	QRinput_append(input3, QR_MODE_NUM, 4, (unsigned char *)str2);
	QRinput_append(input3, QR_MODE_8,   3, (unsigned char *)str3);

	s1 = inputSize(input1);
	s2 = inputSize(input2);
	s3 = inputSize(input3);

	assert_equal(s1, s2, "Incorrect split");
	assert_exp(s2 < s3, "Incorrect estimation");
	testFinish();
	QRinput_free(input1);
	QRinput_free(input2);
	QRinput_free(input3);
}

int main()
{
	int tests = 24;
	testInit(tests);
	test_split1();
	test_split2();
	test_split3();
	test_split4();
	test_split5();
	test_split6();
	test_split7();
	test_split8();
	test_split3c();
	test_toupper();
	test_splitNum8();
	test_splitAnNAn();
	test_splitAn8An();
	test_split8An8();
	test_split8N8();
	testReport(tests);

	return 0;
}
