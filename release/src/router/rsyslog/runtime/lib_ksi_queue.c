#include <malloc.h>
#include <time.h>
#include <errno.h>
#include "lib_ksi_queue.h"

RingBuffer* RingBuffer_new(size_t size) {
	RingBuffer *p = calloc(1, sizeof (RingBuffer));
	if (!p)
		return NULL;

	p->buffer = calloc(size, sizeof (void*));
	p->size = size;
	return p;
}

void RingBuffer_free(RingBuffer* this) {
	if (this->buffer != NULL)
		free(this->buffer);
	free(this);
}

static bool RingBuffer_grow(RingBuffer* this) {
	void **pTmp = calloc(this->size * RB_GROW_FACTOR, sizeof (void*));
	void *pTmpItem = NULL;
	if (!pTmp)
		return false;

	for (size_t i = 0; i < this->size; ++i) {
		RingBuffer_popFront(this, &pTmpItem);
		pTmp[i] = pTmpItem;
	}

	free(this->buffer);
	this->buffer = pTmp;
	this->head = 0;
	this->tail = this->size;
	this->count = this->size;
	this->size = this->size * RB_GROW_FACTOR;
	return true;
}

bool RingBuffer_pushBack(RingBuffer* this, void* item) {

	if (this->size == this->count && !RingBuffer_grow(this))
		return false;

	if(this->size == 0)
		return false;

	this->buffer[this->tail] = item;
	this->tail = (this->tail + 1) % this->size;
	this->count += 1;
	return true;
}

bool RingBuffer_popFront(RingBuffer* this, void** item) {
	if (this->count == 0)
		return false;

	*item = this->buffer[this->head];
	this->buffer[this->head] = NULL;
	this->count -= 1;
	this->head = (this->head + 1) % this->size;
	return true;
}

bool RingBuffer_peekFront(RingBuffer* this, void** item) {
	if (this->count == 0)
		return false;

	*item = this->buffer[this->head];
	return true;
}

size_t RingBuffer_count(RingBuffer* this) {
	return this->count;
}

bool RingBuffer_getItem(RingBuffer* this, size_t index, void** item) {
	if (this->count == 0 || index >= this->count)
		return false;

	*item = this->buffer[(this->head + index) % this->size];
	return true;
}


ProtectedQueue* ProtectedQueue_new(size_t queueSize) {
	ProtectedQueue *p = calloc(1, sizeof (ProtectedQueue));
	if (!p)
		return NULL;

	pthread_mutex_init(&p->mutex, 0);
	p->bStop = false;
	p->workItems = RingBuffer_new(queueSize);
	return p;
}

void ProtectedQueue_free(ProtectedQueue* this) {
	pthread_mutex_destroy(&this->mutex);
	pthread_cond_destroy(&this->condition);
	this->bStop = true;
	RingBuffer_free(this->workItems);
	free(this);
}

/// Signal stop. All threads waiting in FetchItme will be returned false from FetchItem

void ProtectedQueue_stop(ProtectedQueue* this) {
	this->bStop = true;
	pthread_cond_broadcast(&this->condition);
}

/// Atomically adds an item into work item queue and releases a thread waiting
/// in FetchItem

bool ProtectedQueue_addItem(ProtectedQueue* this, void* item) {
	bool ret = false;

	if (this->bStop)
		return false;

	pthread_mutex_lock(&this->mutex);
	if ((ret = RingBuffer_pushBack(this->workItems, item)) == true)
		pthread_cond_signal(&this->condition);
	pthread_mutex_unlock(&this->mutex);
	return ret;
}

bool ProtectedQueue_peekFront(ProtectedQueue* this, void** item) {
	bool ret;
	pthread_mutex_lock(&this->mutex);
	ret = RingBuffer_peekFront(this->workItems, item);
	pthread_mutex_unlock(&this->mutex);
	return ret;
}

bool ProtectedQueue_popFront(ProtectedQueue* this, void** item) {
	bool ret;
	pthread_mutex_lock(&this->mutex);
	ret = RingBuffer_popFront(this->workItems, item);
	pthread_mutex_unlock(&this->mutex);
	return ret;
}

size_t ProtectedQueue_popFrontBatch(ProtectedQueue* this, void** items, size_t bufSize) {
	size_t i;
	pthread_mutex_lock(&this->mutex);
	for (i = 0; RingBuffer_count(this->workItems) > 0 && i < bufSize; ++i)
		RingBuffer_popFront(this->workItems, items[i]);
	pthread_mutex_unlock(&this->mutex);
	return i;
}

bool ProtectedQueue_getItem(ProtectedQueue* this, size_t index, void** item) {
	bool ret=false;
	pthread_mutex_lock(&this->mutex);
	ret=RingBuffer_getItem(this->workItems, index, item);
	pthread_mutex_unlock(&this->mutex);
	return ret;
}

/* Waits for a new work item or timeout (if specified). Returns 0 in case of exit
 * condition, 1 if item became available and ETIMEDOUT in case of timeout. */
int ProtectedQueue_waitForItem(ProtectedQueue* this, void** item, uint64_t timeout) {
	struct timespec ts;
	pthread_mutex_lock(&this->mutex);

	if (timeout > 0) {
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += timeout / 1000LL;
		ts.tv_nsec += (timeout % 1000LL)*1000LL;
	}

	if (timeout) {
		if (pthread_cond_timedwait(&this->condition, &this->mutex, &ts) == ETIMEDOUT) {
			pthread_mutex_unlock(&this->mutex);
			return ETIMEDOUT;
		}
	} else
		pthread_cond_wait(&this->condition, &this->mutex);
	if (this->bStop) {
		pthread_mutex_unlock(&this->mutex);
		return 0;
	}

	if (RingBuffer_count(this->workItems) != 0 && item != NULL)
		RingBuffer_popFront(this->workItems, item);

	pthread_mutex_unlock(&this->mutex);

	return 1;
}

size_t ProtectedQueue_count(ProtectedQueue* this) {
	size_t nCount;
	pthread_mutex_lock(&this->mutex);
	nCount = RingBuffer_count(this->workItems);
	pthread_mutex_unlock(&this->mutex);
	return nCount;
}

void *worker_thread_main(void *arg) {
	int res;
	void* item;
	WorkerThreadContext* tc = (WorkerThreadContext*) arg;

	while (1) {
		item = NULL;
		res = ProtectedQueue_waitForItem(tc->queue, &item, tc->timeout);
		if (tc->queue->bStop)
			return NULL;

		if (res == ETIMEDOUT) {
			if (!tc->timeoutFunc())
				return NULL;
		} else if (item != NULL && !tc->workerFunc(item))
			return NULL;
	}
}
