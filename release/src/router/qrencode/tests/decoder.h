#ifndef DECODER_H
#define DECODER_H

#include "../qrencode.h"
#include "datachunk.h"

typedef struct {
	unsigned char *data;
	int size;
	int mqr;
	int version;
	QRecLevel level;
	DataChunk *chunks, *last;
	int eccResult;
} QRdata;

struct FormatInfo {
	int version;
	QRecLevel level;
};

extern struct FormatInfo MQRformat[];

QRdata *QRdata_new(void);
QRdata *QRdata_newMQR(void);
int QRdata_decodeBitStream(QRdata *qrdata, BitStream *bstream);
void QRdata_dump(QRdata *data);
void QRdata_free(QRdata *data);

int QRcode_decodeVersion(QRcode *code);
int QRcode_decodeFormat(QRcode *code, QRecLevel *level, int *mask);
unsigned char *QRcode_unmask(QRcode *code);
BitStream *QRcode_extractBits(QRcode *code, int *dataLength, int *eccLength);
QRdata *QRcode_decodeBits(QRcode *code);
QRdata *QRcode_decode(QRcode *code);

int QRcode_decodeFormatMQR(QRcode *code, int *vesion, QRecLevel *level, int *mask);
unsigned char *QRcode_unmaskMQR(QRcode *code);
BitStream *QRcode_extractBitsMQR(QRcode *code, int *dataLength, int *eccLength, int *version, QRecLevel *level);
QRdata *QRcode_decodeBitsMQR(QRcode *code);
QRdata *QRcode_decodeMQR(QRcode *code);

#endif /* DECODER_H */
