#include <stdio.h>
#include <string.h>
#include "common.h"
#include "../qrencode_inner.h"
#include "../qrspec.h"
#include "../mqrspec.h"
#include "../qrinput.h"
#include "../rsecc.h"
#include "decoder.h"
#include "rsecc_decoder.h"
#include "rscode.h"

/* See pp. 73 of JIS X0510:2004 */
void test_rscodeexample(void)
{
	QRinput *stream;
	QRRawCode *code;
	static const char str[9] = "01234567";
	static unsigned char correct[26] = {
		0x10, 0x20, 0x0c, 0x56, 0x61, 0x80, 0xec, 0x11, 0xec, 0x11, 0xec, 0x11,
		0xec, 0x11, 0xec, 0x11, 0xa5, 0x24, 0xd4, 0xc1, 0xed, 0x36, 0xc7, 0x87,
		0x2c, 0x55};

	testStart("RS ecc test");
	stream = QRinput_new();
	QRinput_append(stream, QR_MODE_NUM, 8, (unsigned char *)str);
	QRinput_setErrorCorrectionLevel(stream, QR_ECLEVEL_M);
	code = QRraw_new(stream);

	testEnd(memcmp(correct + 16, code->rsblock[0].ecc, 10));
	QRinput_free(stream);
	QRraw_free(code);
}

static void compareRS(unsigned char data[])
{
	int i, j;
	RS *rs;
	int spec[5];
	int dl, el;
	unsigned char ecc_expected[256], ecc_rscodec[256];

	for(i = 1; i <= QRSPEC_VERSION_MAX; i++) {
		for(j = QR_ECLEVEL_L; j <= QR_ECLEVEL_H; j++) {
			QRspec_getEccSpec(i, (QRecLevel)j, spec);
			dl = QRspec_rsDataCodes1(spec);
			el = QRspec_rsEccCodes1(spec);
			rs = init_rs(8, 0x11d, 0, 1, el, 255 - dl - el);
			RSECC_encode(dl, el, data, ecc_rscodec);
			encode_rs_char(rs, data, ecc_expected);
			assert_zero(memcmp(ecc_expected, ecc_rscodec, el), "Invalid ECC found: length %d.\n", el);
			assert_zero(RSECC_decoder_checkSyndrome(dl, data, el, ecc_rscodec), "ECC error found.");
			free_rs_char(rs);


			dl = QRspec_rsDataCodes2(spec);
			el = QRspec_rsEccCodes2(spec);
			if(dl != 0) {
				rs = init_rs(8, 0x11d, 0, 1, el, 255 - dl - el);
				RSECC_encode(dl, el, data, ecc_rscodec);
				encode_rs_char(rs, data, ecc_expected);
				assert_zero(memcmp(ecc_expected, ecc_rscodec, el), "Invalid ECC found: length %d.\n", el);
				assert_zero(RSECC_decoder_checkSyndrome(dl, data, el, ecc_rscodec), "ECC error found.");
				free_rs_char(rs);
			}
		}
	}
}

static void compareRSMQR(unsigned char data[])
{
	int i, j;
	RS *rs;
	int dl, el;
	unsigned char ecc_expected[256], ecc_rscodec[256];

	for(i = 1; i <= MQRSPEC_VERSION_MAX; i++) {
		for(j = QR_ECLEVEL_L; j <= QR_ECLEVEL_Q; j++) {
			dl = MQRspec_getDataLength(i, (QRecLevel)j);
			el = MQRspec_getECCLength(i, (QRecLevel)j);
			if(dl != 0) {
				rs = init_rs(8, 0x11d, 0, 1, el, 255 - dl - el);
				RSECC_encode(dl, el, data, ecc_rscodec);
				encode_rs_char(rs, data, ecc_expected);
				assert_zero(memcmp(ecc_expected, ecc_rscodec, el), "Invalid ECC found: length %d.\n", el);
				assert_zero(RSECC_decoder_checkSyndrome(dl, data, el, ecc_rscodec), "ECC error found.");
				free_rs_char(rs);
			}
		}
	}
}

void test_allQRSizeAndECCLevel(void)
{
	int i;
	unsigned char data[256];

	testStart("Comparing with KA9Q's code: all QR Code sizes and ECC levels");
	memset(data, 0, 256);
	compareRS(data);
	compareRSMQR(data);
	memset(data, 0xaa, 256);
	compareRS(data);
	compareRSMQR(data);
	memset(data, 0xff, 256);
	compareRS(data);
	compareRSMQR(data);
	for(i=0; i<256; i++) {
		data[i] = i;
	}
	compareRS(data);
	compareRSMQR(data);
	testFinish();
}

int main()
{
	RSECC_decoder_init();
	int tests = 2;
	testInit(tests);
	test_rscodeexample();
	test_allQRSizeAndECCLevel();
	testReport(tests);

	return 0;
}
