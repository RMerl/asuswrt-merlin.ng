#ifndef DATACHUNK_H
#define DATACHUNK_H

#include "../qrencode.h"

typedef struct _DataChunk {
	QRencodeMode mode;
	int size;
	int bits;
	unsigned char *data;
	struct _DataChunk *next;
} DataChunk;

DataChunk *DataChunk_new(QRencodeMode mode);
void DataChunk_free(DataChunk *chunk);
void DataChunk_freeList(DataChunk *list);
void DataChunk_dumpChunkList(DataChunk *list);
int DataChunk_totalSize(DataChunk *list);
unsigned char *DataChunk_concatChunkList(DataChunk *list, int *retsize);

#endif /* DATACHUNK_H */
