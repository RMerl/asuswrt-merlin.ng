#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "common.h"
#include "../qrinput.h"
#include "../qrencode_inner.h"
#include "../split.h"
#include "decoder.h"

/* taken from the Table 7 of JIS X0510:2018 (pp. 31-34) */
static int maxCharacterLengths[40][4] = {
	{  41,   25,   17,   10}, /* ver  1 */
	{  77,   47,   32,   20}, /* ver  2 */
	{ 127,   77,   53,   32}, /* ver  3 */
	{ 187,  114,   78,   48}, /* ver  4 */
	{ 255,  154,  106,   65}, /* ver  5 */
	{ 322,  195,  134,   82}, /* ver  6 */
	{ 370,  224,  154,   95}, /* ver  7 */
	{ 461,  279,  192,  118}, /* ver  8 */
	{ 552,  335,  230,  141}, /* ver  9 */
	{ 652,  395,  271,  167}, /* ver 10 */
	{ 772,  468,  321,  198}, /* ver 11 */
	{ 883,  535,  367,  226}, /* ver 12 */
	{1022,  619,  425,  262}, /* ver 13 */
	{1101,  667,  458,  282}, /* ver 14 */
	{1250,  758,  520,  320}, /* ver 15 */
	{1408,  854,  586,  361}, /* ver 16 */
	{1548,  938,  644,  397}, /* ver 17 */
	{1725, 1046,  718,  442}, /* ver 18 */
	{1903, 1153,  792,  488}, /* ver 19 */
	{2061, 1249,  858,  528}, /* ver 20 */
	{2232, 1352,  929,  572}, /* ver 21 */
	{2409, 1460, 1003,  618}, /* ver 22 */
	{2620, 1588, 1091,  672}, /* ver 23 */
	{2812, 1704, 1171,  721}, /* ver 24 */
	{3057, 1853, 1273,  784}, /* ver 25 */
	{3283, 1990, 1367,  842}, /* ver 26 */
	{3517, 2132, 1465,  902}, /* ver 27 */
	{3669, 2223, 1528,  940}, /* ver 28 */
	{3909, 2369, 1628, 1002}, /* ver 29 */
	{4158, 2520, 1732, 1066}, /* ver 30 */
	{4417, 2677, 1840, 1132}, /* ver 31 */
	{4686, 2840, 1952, 1201}, /* ver 32 */
	{4965, 3009, 2068, 1273}, /* ver 33 */
	{5253, 3183, 2188, 1347}, /* ver 34 */
	{5529, 3351, 2303, 1417}, /* ver 35 */
	{5836, 3537, 2431, 1496}, /* ver 36 */
	{6153, 3729, 2563, 1577}, /* ver 37 */
	{6479, 3927, 2699, 1661}, /* ver 38 */
	{6743, 4087, 2809, 1729}, /* ver 39 */
	{7089, 4296, 2953, 1817}  /* ver 40 */
};

static int encodeAndCheckBStream(int mqr, int version, QRecLevel level, QRencodeMode mode, char *data, char *correct)
{
	QRinput *input;
	BitStream *bstream;
	int ret;

	if(mqr) {
		input = QRinput_newMQR(version, level);
	} else {
		input = QRinput_new2(version, level);
	}
	QRinput_append(input, mode, strlen(data), (unsigned char *)data);
	bstream = BitStream_new();
	QRinput_getBitStream(input, bstream);
	ret = cmpBin(correct, bstream);
	if(ret) {
		printf("result : ");
		printBstream(bstream);
		printf("correct: %s\n", correct);
	}
	QRinput_free(input);
	BitStream_free(bstream);

	return ret;
}

static int mergeAndCheckBStream(int mqr, QRencodeMode mode, char *data, char *correct)
{
	QRinput *input;
	BitStream *bstream;
	int ret;

	if(mqr) {
		input = QRinput_newMQR(1, QR_ECLEVEL_L);
	} else {
		input = QRinput_new();
	}
	QRinput_append(input, mode, strlen(data), (unsigned char *)data);
	bstream = BitStream_new();
	QRinput_mergeBitStream(input, bstream);
	ret = cmpBin(correct, bstream);

	QRinput_free(input);
	BitStream_free(bstream);

	return ret;
}

static void test_encodeKanji(void)
{
	char str[5]= {0x93, 0x5f, 0xe4, 0xaa, 0x00};
	char *correct = "10000000001001101100111111101010101010";

	testStart("Encoding kanji stream.");
	testEnd(mergeAndCheckBStream(0, QR_MODE_KANJI, str, correct));
}

static void test_encode8(void)
{
	char str[] = "AC-42";
	char correct[] = "0100000001010100000101000011001011010011010000110010";

	testStart("Encoding 8bit stream.");
	testEnd(mergeAndCheckBStream(0, QR_MODE_8, str, correct));
}

static void test_encode8_versionup(void)
{
	QRinput *stream;
	BitStream *bstream;
	char *str;
	int version;

	testStart("Encoding 8bit stream. (auto-version up test)");
	str = (char *)malloc(2900);
	memset(str, 0xff, 2900);
	stream = QRinput_new();
	bstream = BitStream_new();
	QRinput_append(stream, QR_MODE_8, 2900, (unsigned char *)str);
	QRinput_mergeBitStream(stream, bstream);
	version = QRinput_getVersion(stream);
	assert_equal(version, 40, "Version is %d (40 expected).\n", version);
	testFinish();
	QRinput_free(stream);
	BitStream_free(bstream);
	free(str);
}

static void test_encodeAn(void)
{
	char *str = "AC-42";
	char correct[] = "00100000001010011100111011100111001000010";

	testStart("Encoding alphabet-numeric stream.");
	testEnd(mergeAndCheckBStream(0, QR_MODE_AN, str, correct));
}

static void test_encodeAn2(void)
{
	QRinput *stream;
	char str[] = "!,;$%";
	int ret;

	testStart("Encoding INVALID alphabet-numeric stream.");
	stream = QRinput_new();
	ret = QRinput_append(stream, QR_MODE_AN, 5, (unsigned char *)str);
	testEnd(!ret);
	QRinput_free(stream);
}

static void test_encodeNumeric(void)
{
	char *str = "01234567";
	char correct[] = "00010000001000000000110001010110011000011";

	testStart("Encoding numeric stream. (8 digits)");
	testEnd(mergeAndCheckBStream(0, QR_MODE_NUM, str, correct));
}

static void test_encodeNumeric_versionup(void)
{
	QRinput *stream;
	BitStream *bstream;
	char *str;
	int version;

	testStart("Encoding numeric stream. (auto-version up test)");
	str = (char *)malloc(1050);
	memset(str, '1', 1050);
	stream = QRinput_new2(0, QR_ECLEVEL_L);
	bstream = BitStream_new();
	QRinput_append(stream, QR_MODE_NUM, 1050, (unsigned char *)str);
	QRinput_mergeBitStream(stream, bstream);
	version = QRinput_getVersion(stream);
	assert_equal(version, 14, "Version is %d (14 expected).", version);
	testFinish();
	QRinput_free(stream);
	BitStream_free(bstream);
	free(str);
}

static void test_encodeNumericPadded(void)
{
	char *str = "01234567";
	char *correct;
	char *correctHead = "000100000010000000001100010101100110000110000000";
	int i, ret;

	testStart("Encoding numeric stream. (8 digits)(padded)");
	correct = (char *)malloc(19 * 8 + 1);
	correct[0] = '\0';
	strcat(correct, correctHead);
	for(i=0; i<13; i++) {
		strcat(correct, (i&1)?"00010001":"11101100");
	}
	ret = encodeAndCheckBStream(0, 0, QR_ECLEVEL_L, QR_MODE_NUM, str, correct);
	testEnd(ret);

	free(correct);
}

static void test_encodeNumericPadded2(void)
{
	char *str = "0123456";
	char *correct;
	char *correctHead = "000100000001110000001100010101100101100000000000";
	int i, ret;

	testStart("Encoding numeric stream. (7 digits)(padded)");
	correct = (char *)malloc(19 * 8 + 1);
	correct[0] = '\0';
	strcat(correct, correctHead);
	for(i=0; i<13; i++) {
		strcat(correct, (i&1)?"00010001":"11101100");
	}
	ret = encodeAndCheckBStream(0, 0, QR_ECLEVEL_L, QR_MODE_NUM, str, correct);
	testEnd(ret);

	free(correct);
}

static void test_padding(void)
{
	QRinput *input;
	BitStream *bstream;
	int i, size;
	char data[] = "0123456789ABCDeFG";
	unsigned char c;

	testStart("Padding bit check. (less than 5 bits)");
	input = QRinput_new2(1, QR_ECLEVEL_L);
	QRinput_append(input, QR_MODE_8, 17, (unsigned char *)data);
	bstream = BitStream_new();
	QRinput_getBitStream(input, bstream);
	size = BitStream_size(bstream);
	assert_equal(size, 152, "# of bit is incorrect (%d != 152).\n", size);
	c = 0;
	for(i=0; i<4; i++) {
		c += bstream->data[size - i - 1];
	}
	assert_zero(c, "Padding bits are not zero.");
	testFinish();

	QRinput_free(input);
	BitStream_free(bstream);
}

static void test_padding2(void)
{
	QRinput *input;
	BitStream *bstream;
	int i, size, ret;
	char data[] = "0123456789ABCDeF";
	char correct[153];
	unsigned char c;

	testStart("Padding bit check. (1 or 2 padding bytes)");

	/* 16 byte data (4 bit terminator and 1 byte padding) */
	memset(correct, 0, 153);
	memcpy(correct, "010000010000", 12);
	for(size=0; size<16; size++) {
		c = 0x80;
		for(i=0; i<8; i++) {
			correct[size * 8 + i + 12] = (data[size]&c)?'1':'0';
			c = c >> 1;
		}
	}
	memcpy(correct + 140, "000011101100", 12);

	input = QRinput_new2(1, QR_ECLEVEL_L);
	QRinput_append(input, QR_MODE_8, 16, (unsigned char *)data);
	bstream = BitStream_new();
	QRinput_getBitStream(input, bstream);
	size = BitStream_size(bstream);
	assert_equal(size, 152, "16byte: # of bit is incorrect (%d != 152).\n", size);
	ret = ncmpBin(correct, bstream, 152);
	assert_zero(ret, "Padding bits incorrect.\n");

	QRinput_free(input);
	BitStream_free(bstream);

	/* 15 byte data (4 bit terminator and 2 byte paddings) */

	memcpy(correct, "010000001111", 12);
	memcpy(correct + 132, "00001110110000010001", 20);

	input = QRinput_new2(1, QR_ECLEVEL_L);
	QRinput_append(input, QR_MODE_8, 15, (unsigned char *)data);
	bstream = BitStream_new();
	QRinput_getBitStream(input, bstream);
	size = BitStream_size(bstream);
	assert_equal(size, 152, "15byte: # of bit is incorrect (%d != 152).\n", size);
	ret = ncmpBin(correct, bstream, 152);
	assert_zero(ret, "Padding bits incorrect.\n");

	testFinish();

	QRinput_free(input);
	BitStream_free(bstream);
}

static void test_encodeNumeric2(void)
{
	char *str = "0123456789012345";
	char *correct = "00010000010000000000110001010110011010100110111000010100111010100101";

	testStart("Encoding numeric stream. (16 digits)");
	testEnd(mergeAndCheckBStream(0, QR_MODE_NUM, str, correct));
}

static void test_encodeNumeric3(void)
{
	char *str = "0123456";
	char *correct = "0001 0000000111 0000001100 0101011001 0110";

	testStart("Encoding numeric stream. (7 digits)");
	testEnd(mergeAndCheckBStream(0, QR_MODE_NUM, str, correct));
}

static void test_encodeAnNum(void)
{
	QRinput *input;
	BitStream *bstream;

	testStart("Bit length check of alpha-numeric stream. (11 + 12)");
	input = QRinput_new();
	QRinput_append(input, QR_MODE_AN, 11, (unsigned char *)"ABCDEFGHIJK");
	QRinput_append(input, QR_MODE_NUM, 12, (unsigned char *)"123456789012");
	bstream = BitStream_new();
	QRinput_mergeBitStream(input, bstream);
	testEndExp(BitStream_size(bstream) == 128);
	QRinput_free(input);
	BitStream_free(bstream);

	testStart("Bit length check of alphabet stream. (23)");
	input = QRinput_new();
	QRinput_append(input, QR_MODE_AN, 23, (unsigned char *)"ABCDEFGHIJK123456789012");
	bstream = BitStream_new();
	QRinput_mergeBitStream(input, bstream);
	testEndExp(BitStream_size(bstream) == 140);
	QRinput_free(input);
	BitStream_free(bstream);
}

static void test_struct_listop(void)
{
	QRinput_Struct *s;
	QRinput *inputs[5];
	QRinput_InputList *l;
	int i, ret;

	testStart("QRinput_Struct list operation test.");
	s = QRinput_Struct_new();
	QRinput_Struct_setParity(s, 10);
	assert_nonnull(s, "QRinput_Struct_new() failed.");
	assert_equal(s->parity, 10, "QRinput_Struct_setParity() failed.");

	for(i=0; i<5; i++) {
		inputs[i] = QRinput_new();
		QRinput_append(inputs[i], QR_MODE_AN, 5, (unsigned char *)"ABCDE");
		ret = QRinput_Struct_appendInput(s, inputs[i]);
	}
	assert_equal(ret, 5, "QRinput_Struct_appendInput() returns wrong num?");
	assert_equal(s->size, 5, "QRiput_Struct.size counts wrong number.");

	l = s->head;
	i = 0;
	while(l != NULL) {
		assert_equal(l->input, inputs[i], "QRinput_Struct input list order would be wrong?");
		l = l->next;
		i++;
	}

	QRinput_Struct_free(s);
	testFinish();
}

static void test_insertStructuredAppendHeader(void)
{
	QRinput *stream;
	char correct[] = "0011000011111010010101000000000101000001";
	BitStream *bstream;
	int ret;

	testStart("Insert a structured-append header");
	stream = QRinput_new();
	bstream = BitStream_new();
	QRinput_append(stream, QR_MODE_8, 1, (unsigned char *)"A");
	ret = QRinput_insertStructuredAppendHeader(stream, 16, 1, 0xa5);
	assert_zero(ret, "QRinput_insertStructuredAppendHeader() returns nonzero.\n");
	QRinput_mergeBitStream(stream, bstream);
	assert_nonnull(bstream->data, "Bstream->data is null.");
	assert_zero(cmpBin(correct, bstream), "bitstream is wrong.");
	testFinish();

	QRinput_free(stream);
	BitStream_free(bstream);
}

static void test_insertStructuredAppendHeader_error(void)
{
	QRinput *stream;
	int ret;

	testStart("Insert a structured-append header (errors expected)");
	stream = QRinput_new();
	QRinput_append(stream, QR_MODE_8, 1, (unsigned char *)"A");
	ret = QRinput_insertStructuredAppendHeader(stream, 17, 1, 0xa5);
	assert_equal(-1, ret, "QRinput_insertStructuredAppendHeader() returns 0.");
	assert_equal(EINVAL, errno, "errno is not set correctly (%d returned).", errno);
	ret = QRinput_insertStructuredAppendHeader(stream, 16, 17, 0xa5);
	assert_equal(-1, ret, "QRinput_insertStructuredAppendHeader() returns 0.");
	assert_equal(EINVAL, errno, "errno is not set correctly (%d returned).", errno);
	ret = QRinput_insertStructuredAppendHeader(stream, 16, 0, 0xa5);
	assert_equal(-1, ret, "QRinput_insertStructuredAppendHeader() returns 0.");
	assert_equal(EINVAL, errno, "errno is not set correctly (%d returned).", errno);
	testFinish();

	QRinput_free(stream);
}

static void test_struct_insertStructuredAppendHeaders(void)
{
	QRinput *input;
	QRinput_Struct *s;
	QRinput_InputList *p;
	int i;

	testStart("Insert structured-append headers to a QRinput_Struct.");
	s = QRinput_Struct_new();
	for(i=0; i<10; i++) {
		input = QRinput_new();
		QRinput_append(input, QR_MODE_8, 1, (unsigned char *)"A");
		QRinput_Struct_appendInput(s, input);
	}
	QRinput_Struct_insertStructuredAppendHeaders(s);
	p = s->head;
	i = 1;
	while(p != NULL) {
		assert_equal(p->input->head->mode, QR_MODE_STRUCTURE, "a structured-append header is not inserted.");
		assert_equal(p->input->head->data[0], 10, "size of the structured-header is wrong: #%d, %d should be %d\n", i, p->input->head->data[0], 10);
		assert_equal(p->input->head->data[1], i, "index of the structured-header is wrong: #%d, %d should be %d\n", i, p->input->head->data[1], i);
		assert_equal(p->input->head->data[2], 0, "parity of the structured-header is wrong: #%d\n", i);
		p = p->next;
		i++;
	}
	testFinish();
	QRinput_Struct_free(s);
}

static int check_lengthOfCode(QRencodeMode mode, char *data, int size, int version)
{
	QRinput *input;
	BitStream *b;
	size_t bits;
	int bytes;

	input = QRinput_new();
	QRinput_setVersion(input, version);
	QRinput_append(input, mode, size, (unsigned char *)data);
	b = BitStream_new();
	QRinput_mergeBitStream(input, b);
	bits = BitStream_size(b);
	bytes = QRinput_lengthOfCode(mode, version, bits);
	QRinput_free(input);
	BitStream_free(b);

	return bytes;
}

static void test_lengthOfCode_num(void)
{
	int i, bytes;
	char *data;

	data = (char *)malloc(8000);
	for(i=0; i<8000; i++) {
		data[i] = '0' + i % 10;
	}

	testStart("Checking length of code (numeric)");
	for(i=1; i<=9; i++) {
		bytes = check_lengthOfCode(QR_MODE_NUM, data, i, 1);
		assert_equal(i, bytes, "lengthOfCode failed. (QR_MODE_NUM, version:1, size:%d)\n", i);
	}
	for(i=1023; i<=1025; i++) {
		bytes = check_lengthOfCode(QR_MODE_NUM, data, i, 1);
		assert_equal(1023, bytes, "lengthOfCode failed. (QR_MODE_NUM, version:1, size:%d)\n", i);
	}
	testFinish();
	free(data);
}

static void test_lengthOfCode_kanji(void)
{
	int i, bytes;
	unsigned char str[12]= {0x93, 0x5f, 0xe4, 0xaa, 0x81, 0x40, 0x9f, 0xfc, 0xe0, 0x40, 0xeb, 0xbf};

	testStart("Checking length of code (kanji)");
	for(i=2; i<=12; i+=2) {
		bytes = check_lengthOfCode(QR_MODE_KANJI, (char *)str, i, 1);
		assert_equal(i, bytes, "lengthOfCode failed. (QR_MODE_KANJI, version:1, size:%d)\n", i);
	}
	testFinish();
}

static void test_struct_split_example(void)
{
	QRinput *input;
	QRinput_Struct *s;
	QRinput_InputList *e;
	QRinput_List *l;
	const char *str[4] = { "an example ", "of four Str", "uctured Appe", "nd symbols,"};
	int i;
	BitStream *bstream;

	testStart("Testing the example of structured-append symbols");
	s = QRinput_Struct_new();
	for(i=0; i<4; i++) {
		input = QRinput_new2(1, QR_ECLEVEL_M);
		QRinput_append(input, QR_MODE_8, strlen(str[i]), (unsigned char *)str[i]);
		QRinput_Struct_appendInput(s, input);
	}
	QRinput_Struct_insertStructuredAppendHeaders(s);
	e = s->head;
	i = 0;
	while(e != NULL) {
		bstream = BitStream_new();
		QRinput_mergeBitStream(e->input, bstream);
		BitStream_free(bstream);
		l = e->input->head->next;
		assert_equal(l->mode, QR_MODE_8, "#%d: wrong mode (%d).\n", i, l->mode);
		assert_equal(e->input->level, QR_ECLEVEL_M, "#%d: wrong level (%d).\n", i, e->input->level);

		e = e->next;
		i++;
	}
	testFinish();
	QRinput_Struct_free(s);
}

static void test_struct_split_tooLarge(void)
{
	QRinput *input;
	QRinput_Struct *s;
	char *str;
	int errsv;

	testStart("Testing structured-append symbols. (too large data)");
	str = (char *)malloc(128);
	memset(str, 'a', 128);
	input = QRinput_new2(1, QR_ECLEVEL_H);
	QRinput_append(input, QR_MODE_8, 128, (unsigned char *)str);
	s = QRinput_splitQRinputToStruct(input);
	errsv = errno;
	assert_null(s, "returns non-null.");
	assert_equal(errsv, ERANGE, "did not return ERANGE.");
	testFinish();
	if(s != NULL) QRinput_Struct_free(s);
	QRinput_free(input);
	free(str);
}

static void test_struct_split_invalidVersion(void)
{
	QRinput *input;
	QRinput_Struct *s;
	char *str;
	int errsv;

	testStart("Testing structured-append symbols. (invalid version 0)");
	str = (char *)malloc(128);
	memset(str, 'a', 128);
	input = QRinput_new2(0, QR_ECLEVEL_H);
	QRinput_append(input, QR_MODE_8, 128, (unsigned char *)str);
	s = QRinput_splitQRinputToStruct(input);
	errsv = errno;
	assert_null(s, "returns non-null.");
	assert_equal(errsv, ERANGE, "did not return ERANGE.");
	testFinish();
	if(s != NULL) QRinput_Struct_free(s);
	QRinput_free(input);
	free(str);
}

static void test_struct_singlestructure(void)
{
	QRinput *input;
	QRinput_Struct *s;
	char *str = "TEST";

	testStart("Testing structured-append symbols. (single structure)");
	input = QRinput_new2(10, QR_ECLEVEL_H);
	QRinput_append(input, QR_MODE_AN, strlen(str), (unsigned char *)str);
	s = QRinput_splitQRinputToStruct(input);
	assert_nonnull(s, "must return a code.");
	assert_equal(s->size, 1, "size must be 1, but %d returned.", s->size);
	if(s->size != 1) {
		printQRinputStruct(s);
	}
	testFinish();
	if(s != NULL) QRinput_Struct_free(s);
	QRinput_free(input);
}

static void test_splitentry(void)
{
	QRinput *i1, *i2;
	QRinput_List *e;
	const char *str = "abcdefghij";
	int size1, size2, i;
	unsigned char *d1, *d2;

	testStart("Testing QRinput_splitEntry. (next == NULL)");
	i1 = QRinput_new();
	QRinput_append(i1, QR_MODE_8, strlen(str), (unsigned char *)str);

	i2 = QRinput_dup(i1);
	e = i2->head;
	QRinput_splitEntry(e, 4);

	size1 = size2 = 0;
	e = i1->head;
	while(e != NULL) {
		size1 += e->size;
		e = e->next;
	}
	e = i2->head;
	while(e != NULL) {
		size2 += e->size;
		e = e->next;
	}

	d1 = (unsigned char *)malloc(size1);
	e = i1->head;
	i = 0;
	while(e != NULL) {
		memcpy(&d1[i], e->data, e->size);
		i += e->size;
		e = e->next;
	}
	d2 = (unsigned char *)malloc(size2);
	e = i2->head;
	i = 0;
	while(e != NULL) {
		memcpy(&d2[i], e->data, e->size);
		i += e->size;
		e = e->next;
	}

	assert_equal(size1, size2, "sizes are different. (%d:%d)\n", size1, size2);
	assert_equal(i2->head->size, 4, "split failed (first half)");
	assert_equal(i2->head->next->size, 6, "split failed(second half)");
	assert_zero(memcmp(d1, d2, size1), "strings are different.");
	QRinput_free(i1);
	QRinput_free(i2);
	free(d1);
	free(d2);

	testFinish();
}

static void test_splitentry2(void)
{
	QRinput *i1, *i2;
	QRinput_List *e;
	const char *str = "abcdefghij";
	int size1, size2, i;
	unsigned char *d1, *d2;

	testStart("Testing QRinput_splitEntry. (next != NULL)");
	i1 = QRinput_new();
	QRinput_append(i1, QR_MODE_8, strlen(str), (unsigned char *)str);
	QRinput_append(i1, QR_MODE_8, strlen(str), (unsigned char *)str);

	i2 = QRinput_dup(i1);
	e = i2->head;
	QRinput_splitEntry(e, 4);

	size1 = size2 = 0;
	e = i1->head;
	while(e != NULL) {
		size1 += e->size;
		e = e->next;
	}
	e = i2->head;
	while(e != NULL) {
		size2 += e->size;
		e = e->next;
	}

	d1 = (unsigned char *)malloc(size1);
	e = i1->head;
	i = 0;
	while(e != NULL) {
		memcpy(&d1[i], e->data, e->size);
		i += e->size;
		e = e->next;
	}
	d2 = (unsigned char *)malloc(size2);
	e = i2->head;
	i = 0;
	while(e != NULL) {
		memcpy(&d2[i], e->data, e->size);
		i += e->size;
		e = e->next;
	}

	assert_equal(size1, size2, "sizes are different. (%d:%d)\n", size1, size2);
	assert_equal(i2->head->size, 4, "split failed (first half)");
	assert_equal(i2->head->next->size, 6, "split failed(second half)");
	assert_zero(memcmp(d1, d2, size1), "strings are different.");
	QRinput_free(i1);
	QRinput_free(i2);
	free(d1);
	free(d2);

	testFinish();
}

static void test_splitentry3(void)
{
	QRinput *input;
	QRinput_Struct *s;
	QRinput_List *e00, *e01, *e10, *e11;
	QRinput_InputList *list;
	const char *str = "abcdefghijklmno";

	testStart("Testing QRinput_splitEntry. (does not split an entry)");
	/* version 1 symbol contains 152 bit (19 byte) data.
	 * 20 bits for a structured-append header, so 132 bits can be used.
	 * 15 bytes of 8-bit data is suitable for the symbol.
	 * (mode(4) + length(8) + data(120) == 132.)
	 */
	input = QRinput_new2(1, QR_ECLEVEL_L);
	QRinput_append(input, QR_MODE_8, strlen(str), (unsigned char *)str);
	QRinput_append(input, QR_MODE_8, strlen(str), (unsigned char *)str);
	s = QRinput_splitQRinputToStruct(input);
	list = s->head;
	e00 = list->input->head;
	e01 = e00->next;
	list = list->next;
	e10 = list->input->head;
	e11 = e10->next;

	assert_equal(e00->mode, QR_MODE_STRUCTURE, "Structure header is missing?");
	assert_equal(e01->mode, QR_MODE_8, "no data?!");
	assert_null(e01->next, "Input list is not terminated!\n");
	assert_equal(e10->mode, QR_MODE_STRUCTURE, "Structure header is missing?");
	assert_equal(e11->mode, QR_MODE_8, "no data?!");
	assert_null(e11->next, "Input list is not terminated!\n");

	QRinput_free(input);
	QRinput_Struct_free(s);
	testFinish();
}

static void test_parity(void)
{
	QRinput *input;
	QRinput_Struct *s;
	const char *text = "an example of four Structured Append symbols,";
	const char *str[4] = {
		"an example ",
		"of four Str",
		"uctured Appe",
		"nd symbols,"};
	unsigned char p1, p2;
	int i, len;

	testStart("Testing parity calc.");
	s = QRinput_Struct_new();
	for(i=0; i<4; i++) {
		input = QRinput_new2(1, QR_ECLEVEL_M);
		QRinput_append(input, QR_MODE_8, strlen(str[i]), (unsigned char *)str[i]);
		QRinput_Struct_appendInput(s, input);
	}
	QRinput_Struct_insertStructuredAppendHeaders(s);
	p1 = s->parity;

	p2 = 0;
	len = strlen(text);
	for(i=0; i<len; i++) {
		p2 ^= text[i];
	}
	assert_equal(p1, p2, "Parity numbers didn't match. (%02x should be %02x).\n", p1, p2);
	testFinish();
	QRinput_Struct_free(s);
}

static void test_parity2(void)
{
	QRinput *input;
	QRinput_Struct *s;
	const char *text = "an example of four Structured Append symbols,";
	unsigned char p1, p2;
	int i, len;

	testStart("Testing parity calc.(split)");
	input = QRinput_new2(1, QR_ECLEVEL_L);
	QRinput_append(input, QR_MODE_8, strlen(text), (unsigned char *)text);
	s = QRinput_splitQRinputToStruct(input);
	p1 = s->parity;

	p2 = 0;
	len = strlen(text);
	for(i=0; i<len; i++) {
		p2 ^= text[i];
	}
	assert_equal(p1, p2, "Parity numbers didn't match. (%02x should be %02x).\n", p1, p2);
	testFinish();
	QRinput_free(input);
	QRinput_Struct_free(s);
}

static void test_null_free(void)
{
	testStart("Testing free NULL pointers");
	assert_nothing(QRinput_free(NULL), "Check QRinput_free(NULL).\n");
	assert_nothing(QRinput_Struct_free(NULL), "Check QRinput_Struct_free(NULL).\n");
	testFinish();
}

static void fillCharacter(char *dest, char ch, int size)
{
	memset(dest, ch, size);
	dest[size] = '\0';
}

static void checkEstimatedVersion(int ver, int mode)
{
	int estimatedVersion;
	char data[7200];
	QRinput *input;
	QRencodeMode hint;
	int size1, size2;
	static char *modeStr[4] = {"numeric", "alphanumeric", "8 bit data", "kanji"};
	static char ch[4] = {'0', 'A', 'a', '\x92'};

	if(mode == QR_MODE_KANJI) {
		hint = QR_MODE_KANJI;
		size1 = maxCharacterLengths[ver - 1][mode] * 2;
		size2 = size1 + 2;
	} else {
		hint = QR_MODE_8;
		size1 = maxCharacterLengths[ver - 1][mode];
		size2 = size1 + 1;
	}

	fillCharacter(data, ch[mode], size1);
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput(data, input, hint, 1);
	estimatedVersion = QRinput_estimateVersion(input);
	assert_equal(estimatedVersion, ver, "Estimated version %d is not equal to the expected version %d for %d %s sequence.\n", estimatedVersion, ver, maxCharacterLengths[ver - 1][mode], modeStr[mode]);
	QRinput_free(input);

	fillCharacter(data, ch[mode], size2);
	input = QRinput_new2(0, QR_ECLEVEL_L);
	Split_splitStringToQRinput(data, input, hint, 1);
	estimatedVersion = QRinput_estimateVersion(input);
	assert_equal(estimatedVersion, ver + 1, "Estimated version %d is not equal to the expected version %d for %d %s sequence.\n", estimatedVersion, ver, maxCharacterLengths[ver - 1][mode] + 1, modeStr[mode]);
	QRinput_free(input);
}

static void test_estimateVersionBoundaryCheck(void)
{
	int ver;
	testStart("Boundary check of estimateVersion");
	for(ver = 1; ver < QRSPEC_VERSION_MAX; ver++) {
		checkEstimatedVersion(ver, QR_MODE_NUM);
		checkEstimatedVersion(ver, QR_MODE_AN);
		checkEstimatedVersion(ver, QR_MODE_8);
		checkEstimatedVersion(ver, QR_MODE_KANJI);
	}
	testFinish();
}

static void test_QRinput_new_invalid(void)
{
	testStart("Invalid input to QRinput_new2()");
	QRinput *input;

	input = QRinput_new2(-1, QR_ECLEVEL_H);
	assert_null(input, "QRinput_new2() returns non-null for invalid version (-1).\n");
	assert_equal(errno, EINVAL, "Error code is not EINVAL.\n");
	input = QRinput_new2(41, QR_ECLEVEL_H);
	assert_null(input, "QRinput_new2() returns non-null for invalid version (41).\n");
	assert_equal(errno, EINVAL, "Error code is not EINVAL.\n");
	input = QRinput_new2(1, -1);
	assert_null(input, "QRinput_new2() returns non-null for invalid level (-1).\n");
	assert_equal(errno, EINVAL, "Error code is not EINVAL.\n");
	input = QRinput_new2(1, 5);
	assert_null(input, "QRinput_new2() returns non-null for invalid level (5).\n");
	assert_equal(errno, EINVAL, "Error code is not EINVAL.\n");
	testFinish();
}

static void test_QRinput_getErrorCorrectionLevel(void)
{
	testStart("Invalid input to QRinput_getErrorCorrectionLevel()");
	QRinput *input;
	QRecLevel level;

	input = QRinput_new2(1, QR_ECLEVEL_H);
	level = QRinput_getErrorCorrectionLevel(input);
	assert_equal(level, QR_ECLEVEL_H, "QRinput_getErrorCorrectionLevel() fails to return expected level.\n");
	testFinish();
}

static void test_mqr_new(void)
{
	QRinput *input;
	testStart("Testing QRinput_newMQR().");

	input = QRinput_newMQR(0, QR_ECLEVEL_L);
	assert_null(input, "Version 0 passed.\n");
	QRinput_free(input);

	input = QRinput_newMQR(5, QR_ECLEVEL_L);
	assert_null(input, "Version 5 passed.\n");
	QRinput_free(input);

	input = QRinput_newMQR(1, QR_ECLEVEL_M);
	assert_null(input, "Invalid ECLEVEL passed.\n");
	QRinput_free(input);

	input = QRinput_newMQR(1, QR_ECLEVEL_L);
	assert_equal(input->version, 1, "QRinput.version was not as expected.\n");
	assert_equal(input->level, QR_ECLEVEL_L, "QRinput.version was not as expected.\n");
	QRinput_free(input);

	testFinish();
}

static void test_mqr_setversion(void)
{
	QRinput *input;
	int ret;
	testStart("Testing QRinput_setVersion() for MQR.");

	input = QRinput_newMQR(1, QR_ECLEVEL_L);
	ret = QRinput_setVersion(input, 2);
	assert_exp((ret < 0), "QRinput_setVersion should be denied.\n");
	QRinput_free(input);

	testFinish();
}

static void test_mqr_setlevel(void)
{
	QRinput *input;
	int ret;
	testStart("Testing QRinput_setErrorCorrectionLevel() for MQR.");

	input = QRinput_newMQR(1, QR_ECLEVEL_L);
	ret = QRinput_setErrorCorrectionLevel(input, QR_ECLEVEL_M);
	assert_exp((ret < 0), "QRinput_setErrorCorrectionLevel should be denied.\n");
	QRinput_free(input);

	testFinish();
}

static void test_paddingMQR(void)
{
	char *dataM1[] = {"65", "513", "5139", "51365"};
	char *correctM1[] = {"01010000010000000000",
						 "01110000000010000000",
						 "10010000000011001000",
						 "10110000000011000001"};
	char *dataM2[] = {"513513", "51351365"};
	char *correctM2[] = {"0 0110 1000000001 1000000001 0000000",
						 "0 1000 1000000001 1000000001 1000001"};
	int i, ret;

	testStart("Padding bit check of MQR. (only 0 padding)");
	for(i=0; i<4; i++) {
		ret = encodeAndCheckBStream(1, 1, QR_ECLEVEL_L, QR_MODE_NUM, dataM1[i], correctM1[i]);
		assert_zero(ret, "Number %s incorrectly encoded.\n", dataM1[i]);
	}
	for(i=0; i<2; i++) {
		ret = encodeAndCheckBStream(1, 2, QR_ECLEVEL_M, QR_MODE_NUM, dataM2[i], correctM2[i]);
		assert_zero(ret, "Number %s incorrectly encoded.\n", dataM2[i]);
	}
	testFinish();
}

static void test_padding2MQR(void)
{
	char *data[] = {"9", "513513", "513", "513"};
	int ver[] = {1, 2, 2, 3};
	char *correct[] = {"00110010 00000000 0000",
					   "0 0110 1000000001 1000000001 0000000 11101100",
					   "0 0011 1000000001 000000000 11101100 00010001",
					   "00 00011 1000000001 0000000 11101100 00010001 11101100 00010001 11101100 00010001 11101100 0000"
					   };
	int i, ret;

	testStart("Padding bit check. (1 or 2 padding bytes)");

	for(i=0; i<4; i++) {
		ret = encodeAndCheckBStream(1, ver[i], QR_ECLEVEL_L, QR_MODE_NUM, data[i], correct[i]);
		assert_zero(ret, "Number %s incorrectly encoded.\n", data[i]);
	}
	testFinish();
}

static void test_textMQR(void)
{
	int version = 3;
	QRecLevel level = QR_ECLEVEL_M;
	char *str = "MICROQR";
	char *correct = {"01 0111 01111110000 01000110111 10001010010 011011 0000000 0000 11101100 0000"};
	int ret;

	testStart("Text encoding (Micro QR)");
	ret = encodeAndCheckBStream(1, version, level, QR_MODE_AN, str, correct);
	assert_zero(ret, "AlphaNumeric string '%s' incorrectly encoded.\n", str);
	testFinish();
}

static void test_ECIinvalid(void)
{
	QRinput *stream;
	int ret;

	testStart("Appending invalid ECI header");
	stream = QRinput_new();
	ret = QRinput_appendECIheader(stream, 999999);
	assert_zero(ret, "Valid ECI header rejected.");
	ret = QRinput_appendECIheader(stream, 1000000);
	assert_nonzero(ret, "Invalid ECI header accepted.");
	QRinput_free(stream);
	testFinish();
}

static void test_encodeECI(void)
{
	QRinput *input;
	BitStream *bstream;
	unsigned char str[] = {0xa1, 0xa2, 0xa3, 0xa4, 0xa5};
	char *correct = "0111 00001001 0100 00000101 10100001 10100010 10100011 10100100 10100101";
	int ret;

	testStart("Encoding characters with ECI header.");
	input = QRinput_new();
	ret = QRinput_appendECIheader(input, 9);
	assert_zero(ret, "Valid ECI header rejected.\n");

	ret = QRinput_append(input, QR_MODE_8, 5, str);
	assert_zero(ret, "Failed to append characters.\n");
	bstream = BitStream_new();
	QRinput_mergeBitStream(input, bstream);
	assert_nonnull(bstream, "Failed to merge.\n");
	if(bstream != NULL) {
		ret = ncmpBin(correct, bstream, 64);
		assert_zero(ret, "Encodation of ECI header was invalid.\n");
		BitStream_free(bstream);
	}
	QRinput_free(input);
	testFinish();
}

int main()
{
	int tests = 42;
	testInit(tests);

	test_encodeNumeric();
	test_encodeNumeric2();
	test_encodeNumeric3();
	test_encodeNumeric_versionup();
	test_encode8();
	test_encode8_versionup();
	test_encodeAn();
	test_encodeAn2();
	test_encodeKanji();
	test_encodeNumericPadded();
	test_encodeNumericPadded2();
	test_encodeAnNum();
	test_padding();
	test_padding2();
	test_struct_listop();
	test_insertStructuredAppendHeader();
	test_insertStructuredAppendHeader_error();
	test_struct_insertStructuredAppendHeaders();
	test_lengthOfCode_num();
	test_lengthOfCode_kanji();
	test_splitentry();
	test_splitentry2();
	test_splitentry3();
	test_struct_split_example();
	test_struct_split_tooLarge();
	test_struct_split_invalidVersion();
	test_struct_singlestructure();
	test_parity();
	test_parity2();
	test_null_free();
	test_estimateVersionBoundaryCheck();
	test_QRinput_new_invalid();
	test_QRinput_getErrorCorrectionLevel();

	test_mqr_new();
	test_mqr_setversion();
	test_mqr_setlevel();
	test_paddingMQR();
	test_padding2MQR();
	test_textMQR();

	test_ECIinvalid();
	test_encodeECI();

	testReport(tests);

	return 0;
}
