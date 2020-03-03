#ifndef __LINUX_IIO_KFIFO_BUF_H__
#define __LINUX_IIO_KFIFO_BUF_H__

#include <linux/kfifo.h>
#include <linux/iio/iio.h>
#include <linux/iio/buffer.h>

struct iio_buffer *iio_kfifo_allocate(void);
void iio_kfifo_free(struct iio_buffer *r);

struct iio_buffer *devm_iio_kfifo_allocate(struct device *dev);
void devm_iio_kfifo_free(struct device *dev, struct iio_buffer *r);

#endif
