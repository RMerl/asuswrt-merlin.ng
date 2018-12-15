#ifndef INCLUDED_LIBRSKSI_QUEUE_H
#define INCLUDED_LIBRSKSI_QUEUE_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#define RB_GROW_FACTOR 2

typedef struct RingBuffer_st {
	void **buffer;
	size_t size;
	size_t count;
	size_t head;
	size_t tail;
} RingBuffer;

RingBuffer* RingBuffer_new(size_t size);
void RingBuffer_free(RingBuffer* this);
bool RingBuffer_pushBack(RingBuffer* this, void* item);
bool RingBuffer_popFront(RingBuffer* this, void** item);
bool RingBuffer_peekFront(RingBuffer* this, void** item);
bool RingBuffer_getItem(RingBuffer* this, size_t index, void** item);
size_t RingBuffer_count(RingBuffer* this);

typedef struct ProtectedQueue_st {
	bool bStop;
	RingBuffer *workItems;
	pthread_mutex_t mutex;
	pthread_cond_t condition;
} ProtectedQueue;

ProtectedQueue* ProtectedQueue_new(size_t queueSize);
void ProtectedQueue_free(ProtectedQueue* this);
void ProtectedQueue_stop(ProtectedQueue* this);
bool ProtectedQueue_addItem(ProtectedQueue* this, void* item);
bool ProtectedQueue_peekFront(ProtectedQueue* this, void** item);
bool ProtectedQueue_popFront(ProtectedQueue* this, void** item);
size_t ProtectedQueue_popFrontBatch(ProtectedQueue* this, void** items, size_t bufSize);
int ProtectedQueue_waitForItem(ProtectedQueue* this, void** item, uint64_t timeout);
size_t ProtectedQueue_count(ProtectedQueue* this);
bool ProtectedQueue_getItem(ProtectedQueue* this, size_t index, void** item);

typedef struct WorkerThreadContext_st {
	bool (*workerFunc)(void*);
	bool (*timeoutFunc)(void);
	ProtectedQueue* queue;
	unsigned timeout;
} WorkerThreadContext;

void *worker_thread_main(void *arg);

#endif //INCLUDED_LIBRSKSI_QUEUE_H
