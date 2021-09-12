#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "common.h"
#include "../qrinput.h"
#include "../split.h"
#include "../qrspec.h"
#include "decoder.h"

#define MAX_LENGTH 7091
static unsigned char data[MAX_LENGTH];
static unsigned char check[MAX_LENGTH];

static const char *AN = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";

#define drand(__scale__) ((__scale__) * (double)rand() / ((double)RAND_MAX + 1.0))

static int fill8bitString(void)
{
	int len, i;

	len = 1 + (int)drand((MAX_LENGTH - 2));
	for(i=0; i<len; i++) {
		data[i] = (unsigned char)drand(255) + 1;
	}
	data[len] = '\0';

	return len;
}

static int fill8bitData(void)
{
	int len, i;

	len = 1 + (int)drand((MAX_LENGTH - 2));
	for(i=0; i<len; i++) {
		data[i] = (unsigned char)drand(256);
	}
	data[len] = '\0';

	return len;
}

static int fillANData(void)
{
	int len, i;

	len = 1 + (int)drand((MAX_LENGTH - 2));
	for(i=0; i<len; i++) {
		data[i] = AN[(int)drand(45)];
	}
	data[len] = '\0';

	return len;
}

static void test_encode_an(int num)
{
	int ret;
	int len;
	len = fillANData();
	QRcode *qrcode;
	QRdata *qrdata;
	FILE *fp;
	char buf[256];

	qrcode = QRcode_encodeString((char *)data, 0, num % 4, QR_MODE_8, num % 2);
	if(qrcode == NULL) {
		if(errno == ERANGE) return;
		perror("test_encode_an aborted at QRcode_encodeString():");
		printf("Length: %d\n", len);
		printf("Level: %d\n", num % 4);
		return;
	}
	qrdata = QRcode_decode(qrcode);
	if(qrdata == NULL) {
		printf("#%d: Failed to decode this code.\n", num);
		QRcode_free(qrcode);
		return;
	}
	if(qrdata->size != len) {
		printf("#%d: length mismatched (orig: %d, decoded: %d)\n", num, len, qrdata->size);
	}
	ret = memcmp(qrdata->data, data, len);
	if(ret != 0) {
		unsigned char *frame, *p;
		unsigned int x;
		int y,c;
		int dataLength, eccLength;
		QRinput *input;
		QRcode *origcode;
		BitStream *bstream;
		int spec[5];

		printf("#%d: data mismatched.\n", num);
		printf("Version: %d\n", qrcode->version);
		QRspec_getEccSpec(qrcode->version, num%4, spec);
		printf("DataLength: %d\n", QRspec_rsDataLength(spec));
		printf("BlockNum1: %d\n", QRspec_rsBlockNum1(spec));
		printf("BlockNum: %d\n", QRspec_rsBlockNum(spec));
		printf("DataCodes1: %d\n", QRspec_rsDataCodes1(spec));

		snprintf(buf, 256, "monkey-orig-%d.dat", num);
		fp = fopen(buf, "w");
		fputs((char *)data, fp);
		fclose(fp);

		snprintf(buf, 256, "monkey-result-%d.dat", num);
		fp = fopen(buf, "w");
		fputs((char *)qrdata->data, fp);
		fclose(fp);

		snprintf(buf, 256, "monkey-result-unmasked-%d.dat", num);
		fp = fopen(buf, "w");
		frame = QRcode_unmask(qrcode);
		p = frame;
		for(y=0; y<qrcode->width; y++) {
			for(x=0; x<qrcode->width; x++) {
				fputc((*p&1)?'1':'0', fp);
				p++;
			}
			fputc('\n', fp);
		}
		fclose(fp);
		free(frame);

		snprintf(buf, 256, "monkey-orig-unmasked-%d.dat", num);
		fp = fopen(buf, "w");
		input = QRinput_new2(0, num % 4);
		Split_splitStringToQRinput((char *)data, input, QR_MODE_8, num % 2);
		origcode = QRcode_encodeMask(input, -2);
		p = origcode->data;
		for(y=0; y<origcode->width; y++) {
			for(x=0; x<origcode->width; x++) {
				fputc((*p&1)?'1':'0', fp);
				p++;
			}
			fputc('\n', fp);
		}
		fclose(fp);
		QRcode_free(origcode);

		snprintf(buf, 256, "monkey-orig-bits-%d.dat", num);
		fp = fopen(buf, "w");
		bstream = BitStream_new();
		QRinput_mergeBitStream(input, bstream);
		c = 0;
		for(x=0; x<bstream->length; x++) {
			fputc((bstream->data[x]&1)?'1':'0', fp);
			if((x & 7) == 7) {
				fputc(' ', fp);
				c++;
			}
			if((x & 63) == 63) {
				fprintf(fp, "%d\n", c);
			}
		}
		fclose(fp);
		QRinput_free(input);
		BitStream_free(bstream);

		snprintf(buf, 256, "monkey-result-bits-%d.dat", num);
		fp = fopen(buf, "w");
		bstream = QRcode_extractBits(qrcode, &dataLength, &eccLength);
		y = bstream->length;
		p = bstream->data;
		c = 0;
		for(x=0; x<y; x++) {
			fputc((p[x]&1)?'1':'0', fp);
			if((x & 7) == 7) {
				fputc(' ', fp);
				c++;
			}
			if((x & 63) == 63) {
				fprintf(fp, "%d\n", c);
			}
		}
		fclose(fp);
		BitStream_free(bstream);
	}
	QRdata_free(qrdata);
	QRcode_free(qrcode);
}

static void monkey_encode_an(int loop)
{
	int i;

	testStart("Monkey test: QRcode_encodeString() - AlphaNumeric string.");
	srand(0);
	for(i=0; i<loop; i++) {
		test_encode_an(i);
	}
	testEnd(0);
}


static void test_split_an(int num)
{
	QRinput *input;
	QRinput_List *list;
	int len, i, ret;

	len = fillANData();

	input = QRinput_new2(0, QR_ECLEVEL_L);
	if(input == NULL) {
		perror("test_split_an aborted at QRinput_new2():");
		return;
	}
	ret = Split_splitStringToQRinput((char *)data, input, QR_MODE_8, 1);
	if(ret < 0) {
		perror("test_split_an aborted at Split_splitStringToQRinput():");
		QRinput_free(input);
		return;
	}
	list = input->head;
	i = 0;
	while(list != NULL) {
		memcpy(check + i, list->data, list->size);
		i += list->size;
		list = list->next;
	}
	if(i != len) {
		printf("#%d: length is not correct. (%d should be %d)\n", num, i, len);
	}

	check[i] = '\0';
	ret = memcmp(data, check, len);
	if(ret != 0) {
		printf("#%d: data mismatched.\n", num);
		list = input->head;
		i = 0;
		while(list != NULL) {
			ret = memcmp(data + i, list->data, list->size);
			printf("wrong chunk:\n");
			printf(" position: %d\n", i);
			printf(" mode    : %d\n", list->mode);
			printf(" size    : %d\n", list->size);
			printf(" data    : %.*s\n", list->size, list->data);
			i += list->size;
			list = list->next;
		}
		exit(1);
	}
	QRinput_free(input);
}

static void monkey_split_an(int loop)
{
	int i;

	testStart("Monkey test: Split_splitStringToQRinput() - AlphaNumeric string.");
	srand(0);
	for(i=0; i<loop; i++) {
		test_split_an(i);
	}
	testEnd(0);
}

static void test_encode_8(int num)
{
	QRcode *qrcode;
	QRdata *qrdata;
	int len, ret;

	len = fill8bitData();

	qrcode = QRcode_encodeData(len, data, 0, num % 4);
	if(qrcode == NULL) {
		if(errno == ERANGE) return;
		perror("test_encdoe_8 aborted at QRcode_encodeData():");
		return;
	}
	qrdata = QRcode_decode(qrcode);
	if(qrdata == NULL) {
		printf("#%d: Failed to decode this code.\n", num);
		QRcode_free(qrcode);
		return;
	}
	if(qrdata->size != len) {
		printf("#%d: length mismatched (orig: %d, decoded: %d)\n", num, len, qrdata->size);
	}
	ret = memcmp(qrdata->data, data, len);
	if(ret != 0) {
		printf("#%d: data mismatched.\n", num);
	}
	QRdata_free(qrdata);
	QRcode_free(qrcode);
}

static void monkey_encode_8(int loop)
{
	int i;

	testStart("Monkey test: QRcode_encodeData() - 8bit char string.");
	srand(0);
	for(i=0; i<loop; i++) {
		test_encode_8(i);
	}
	testEnd(0);
}

static void test_split_8(int num)
{
	QRinput *input;
	QRinput_List *list;
	int len, i, ret;

	len = fill8bitString();

	input = QRinput_new2(0, QR_ECLEVEL_L);
	if(input == NULL) {
		perror("test_split_8 aborted at QRinput_new2():");
		return;
	}
	ret = Split_splitStringToQRinput((char *)data, input, QR_MODE_8, 1);
	if(ret < 0) {
		perror("test_split_8 aborted at Split_splitStringToQRinput():");
		QRinput_free(input);
		return;
	}
	list = input->head;
	i = 0;
	while(list != NULL) {
		memcpy(check + i, list->data, list->size);
		i += list->size;
		list = list->next;
	}
	if(i != len) {
		printf("#%d: length is not correct. (%d should be %d)\n", num, i, len);
	}

	check[i] = '\0';
	ret = memcmp(data, check, len);
	if(ret != 0) {
		printf("#%d: data mismatched.\n", num);
		list = input->head;
		i = 0;
		while(list != NULL) {
			ret = memcmp(data + i, list->data, list->size);
			printf("wrong chunk:\n");
			printf(" position: %d\n", i);
			printf(" mode    : %d\n", list->mode);
			printf(" size    : %d\n", list->size);
			printf(" data    : %.*s\n", list->size, list->data);
			i += list->size;
			list = list->next;
		}
		exit(1);
	}
	QRinput_free(input);
}

static void monkey_split_8(int loop)
{
	int i;

	testStart("Monkey test: Split_splitStringToQRinput() - 8bit char string.");
	srand(0);
	for(i=0; i<loop; i++) {
		test_split_8(i);
	}
	testEnd(0);
}

static void test_encode_kanji(int num)
{
	QRcode *qrcode;
	QRdata *qrdata;
	int len, ret;

	len = fill8bitString();

	qrcode = QRcode_encodeString((char *)data, 0, num % 4, QR_MODE_8, 1);
	if(qrcode == NULL) {
		if(errno == ERANGE) return;
		perror("test_encdoe_kanji aborted at QRcode_encodeString():");
		return;
	}
	qrdata = QRcode_decode(qrcode);
	if(qrdata == NULL) {
		printf("#%d: Failed to decode this code.\n", num);
		QRcode_free(qrcode);
		return;
	}
	if(qrdata->size != len) {
		printf("#%d: length mismatched (orig: %d, decoded: %d)\n", num, len, qrdata->size);
	}
	ret = memcmp(qrdata->data, data, len);
	if(ret != 0) {
		printf("#%d: data mismatched.\n", num);
	}
	QRdata_free(qrdata);
	QRcode_free(qrcode);
}

static void monkey_encode_kanji(int loop)
{
	int i;

	testStart("Monkey test: QRcode_encodeString() - kanji string.");
	srand(0);
	for(i=0; i<loop; i++) {
		test_encode_kanji(i);
	}
	testEnd(0);
}

static void test_split_kanji(int num)
{
	QRinput *input;
	QRinput_List *list;
	int len, i, ret;

	len = fill8bitString();

	input = QRinput_new2(0, QR_ECLEVEL_L);
	if(input == NULL) {
		perror("test_split_kanji aborted at QRinput_new2():");
		return;
	}
	ret = Split_splitStringToQRinput((char *)data, input, QR_MODE_KANJI, 1);
	if(ret < 0) {
		perror("test_split_kanji aborted at Split_splitStringToQRinput():");
		QRinput_free(input);
		return;
	}
	list = input->head;
	i = 0;
	while(list != NULL) {
		memcpy(check + i, list->data, list->size);
		i += list->size;
		list = list->next;
	}
	if(i != len) {
		printf("#%d: length is not correct. (%d should be %d)\n", num, i, len);
	}

	check[i] = '\0';
	ret = memcmp(data, check, len);
	if(ret != 0) {
		printf("#%d: data mismatched.\n", num);
		list = input->head;
		i = 0;
		while(list != NULL) {
			ret = memcmp(data + i, list->data, list->size);
			printf("wrong chunk:\n");
			printf(" position: %d\n", i);
			printf(" mode    : %d\n", list->mode);
			printf(" size    : %d\n", list->size);
			printf(" data    : %.*s\n", list->size, list->data);
			i += list->size;
			list = list->next;
		}
		exit(1);
	}
	QRinput_free(input);
}

static void monkey_split_kanji(int loop)
{
	int i;

	testStart("Monkey test: Split_splitStringToQRinput() - kanji string.");
	srand(0);
	for(i=0; i<loop; i++) {
		test_split_kanji(i);
	}
	testEnd(0);
}

static void test_split_structure(int num)
{
	QRinput *input;
	QRinput_Struct *s;
	QRcode_List *codes, *list;
	QRinput_InputList *il;
	int version;
	QRecLevel level;
	int c, i, ret;

	version = (int)drand(40) + 1;
	level = (QRecLevel)drand(4);

	fill8bitString();

	input = QRinput_new2(version, level);
	if(input == NULL) {
		perror("test_split_structure aborted at QRinput_new2():");
		return;
	}
	ret = Split_splitStringToQRinput((char *)data, input, QR_MODE_KANJI, 1);
	if(ret < 0) {
		perror("test_split_structure aborted at Split_splitStringToQRinput():");
		QRinput_free(input);
		return;
	}
	s = QRinput_splitQRinputToStruct(input);
	if(s == NULL) {
		if(errno != 0 && errno != ERANGE) {
			perror("test_split_structure aborted at QRinput_splitQRinputToStruct():");
		}
		QRinput_free(input);
		return;
	}
	il = s->head;
	i = 0;
	while(il != NULL) {
		if(il->input->version != version) {
			printf("Test: version %d, level %c\n", version, levelChar[level]);
			printf("wrong version number.\n");
			printQRinputInfo(il->input);
			exit(1);
		}
		i++;
		il = il->next;
	}
	codes = QRcode_encodeInputStructured(s);
	if(codes == NULL) {
		perror("test_split_structure aborted at QRcode_encodeInputStructured():");
		QRinput_free(input);
		QRinput_Struct_free(s);
		return;
	}
	list = codes;
	il = s->head;
	c = 0;
	while(list != NULL) {
		if(list->code->version != version) {
			printf("#%d: data mismatched.\n", num);
			printf("Test: version %d, level %c\n", version, levelChar[level]);
			printf("code #%d\n", c);
			printf("Version mismatch: %d should be %d\n", list->code->version, version);
			printf("max bits: %d\n", QRspec_getDataLength(version, level) * 8 - 20);
			printQRinputInfo(il->input);
			printQRinput(input);
			exit(1);
		}
		list = list->next;
		il = il->next;
		c++;
	}

	QRinput_free(input);
	QRinput_Struct_free(s);
	QRcode_List_free(codes);

	return;
}

static void monkey_split_structure(int loop)
{
	int i;

	testStart("Monkey test: QRinput_splitQRinputToStruct.");
	srand(0);
	for(i=0; i<loop; i++) {
		test_split_structure(i);
	}
	testEnd(0);
}

int main(int argc, char **argv)
{
	int loop = 1000;
	if(argc == 2) {
		loop = atoi(argv[1]);
	}
	int tests = 7;
	testInit(tests);
	monkey_split_an(loop);
	monkey_encode_an(loop);
	monkey_split_8(loop);
	monkey_encode_8(loop);
	monkey_split_kanji(loop);
	monkey_encode_kanji(loop);
	monkey_split_structure(loop);
	testReport(tests);

	return 0;
}
