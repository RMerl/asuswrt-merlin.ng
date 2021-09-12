/*
 * common part of test units.
 */

#ifndef COMMON_H__
#define COMMON_H__

#include <stdlib.h>
#if HAVE_CONFIG_H
#include "../config.h"
#endif
#include "../qrencode.h"
#include "../qrinput.h"
#include "../bitstream.h"
#include "../qrencode_inner.h"

extern int assertionFailed;
extern int assertionNum;
extern const char levelChar[4];
extern const char *modeStr[5];

void testInit(int tests);
#define testStart(__arg__) (testStartReal(__func__, __arg__))
#define testEndExp(__arg__) (testEnd(!(__arg__)))
void testStartReal(const char *func, const char *name);
void testEnd(int result);
void testFinish(void);
void testReport(int tests);

#define assert_exp(__exp__, ...) \
{assertionNum++;if(!(__exp__)) {assertionFailed++; printf(__VA_ARGS__);}}

#define assert_zero(__exp__, ...) assert_exp((__exp__) == 0, __VA_ARGS__)
#define assert_nonzero(__exp__, ...) assert_exp((__exp__) != 0, __VA_ARGS__)
#define assert_null(__ptr__, ...) assert_exp((__ptr__) == NULL, __VA_ARGS__)
#define assert_nonnull(__ptr__, ...) assert_exp((__ptr__) != NULL, __VA_ARGS__)
#define assert_equal(__e1__, __e2__, ...) assert_exp((__e1__) == (__e2__), __VA_ARGS__)
#define assert_notequal(__e1__, __e2__, ...) assert_exp((__e1__) != (__e2__), __VA_ARGS__)
#define assert_nothing(__exp__, ...) {printf(__VA_ARGS__); __exp__;}

int ncmpBin(char *correct, BitStream *bstream, size_t len);
int cmpBin(char *correct, BitStream *bstream);

void printFrame(int width, unsigned char *frame);
void printQRcode(QRcode *code);
void printQRRawCodeFromQRinput(QRinput *input);
void printQRinput(QRinput *input);
void printQRinputInfo(QRinput *input);
void printQRinputStruct(QRinput_Struct *s);

void printBinary(unsigned char *data, int length);
void printBstream(BitStream *bstream);

void show_QRcode(QRcode *qrcode);

#endif /* COMMON_H__ */
