/******************************************************************************

   Copyright 2023 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "host_adapt.h"

#include "host_api_impl.h"


#define CTRL_BUSY_MASK		BIT(15)
#define CTRL_CMD_MASK		(BIT(15) - 1)

#define MAX_BUSY_LOOP		1000	/* roughly 10ms */

#define THR_RST_DATA		5

#if defined(ENABLE_GETSET_OPT) && ENABLE_GETSET_OPT
static struct {
	uint16_t ctrl;
	int16_t ret;
	mmd_api_data_t data;
} shadow = {
	.ctrl = ~0,
	.ret = -1,
	.data = {{0}}
};
#endif

static int __wait_ctrl_busy(const GSW_Device_t *dev)
{
	int ret, i;

	for (i = 0; i < MAX_BUSY_LOOP; i++) {
		ret = gsw_read(dev, GSW_MMD_REG_CTRL);
		if (ret < 0)
			return ret;

		if (!(ret & CTRL_BUSY_MASK))
			return 0;

		dev->usleep(10);
	}

	return -ETIMEDOUT;
}

static int __gsw_rst_data(const GSW_Device_t *dev)
{
	int ret;

	ret = gsw_write(dev, GSW_MMD_REG_LEN_RET, 0);
	if (ret < 0)
		return ret;

	ret = gsw_write(dev, GSW_MMD_REG_CTRL,
			MMD_API_RST_DATA | CTRL_BUSY_MASK);
	if (ret < 0)
		return ret;

	return __wait_ctrl_busy(dev);
}

static int __gsw_set_data(const GSW_Device_t *dev, uint16_t words)
{
	int ret;
	uint16_t cmd;

	ret = gsw_write(dev, GSW_MMD_REG_LEN_RET,
			GSW_MMD_REG_DATA_MAX_SIZE * sizeof(uint16_t));
	if (ret < 0)
		return ret;

	cmd = words / GSW_MMD_REG_DATA_MAX_SIZE - 1;
	assert(cmd < 2);
	cmd += MMD_API_SET_DATA_0;
	ret = gsw_write(dev, GSW_MMD_REG_CTRL,
			cmd | CTRL_BUSY_MASK);
	if (ret < 0)
		return ret;

	return __wait_ctrl_busy(dev);
}

static int __gsw_get_data(const GSW_Device_t *dev, uint16_t words)
{
	int ret;
	uint16_t cmd;

	ret = gsw_write(dev, GSW_MMD_REG_LEN_RET,
			GSW_MMD_REG_DATA_MAX_SIZE * sizeof(uint16_t));
	if (ret < 0)
		return ret;

	cmd = words / GSW_MMD_REG_DATA_MAX_SIZE;
	assert(cmd > 0 && cmd < 3);
	cmd += MMD_API_GET_DATA_0;
	ret = gsw_write(dev, GSW_MMD_REG_CTRL,
			cmd | CTRL_BUSY_MASK);
	if (ret < 0)
		return ret;

	return __wait_ctrl_busy(dev);
}

static int __gsw_send_cmd(const GSW_Device_t *dev, uint16_t cmd, uint16_t size,
			  int16_t *presult)
{
	int ret;

	ret = gsw_write(dev, GSW_MMD_REG_LEN_RET, size);
	if (ret < 0)
		return ret;

	ret = gsw_write(dev, GSW_MMD_REG_CTRL,
			cmd | CTRL_BUSY_MASK);
	if (ret < 0)
		return ret;

	ret = __wait_ctrl_busy(dev);
	if (ret < 0)
		return ret;

	ret = gsw_read(dev, GSW_MMD_REG_LEN_RET);
	if (ret < 0)
		return ret;

	*presult = ret;
	return 0;
}

static bool __gsw_cmd_r_valid(uint16_t cmd_r)
{
#if defined(ENABLE_GETSET_OPT) && ENABLE_GETSET_OPT
	return (shadow.ctrl == cmd_r && shadow.ret >= 0) ? true : false;
#else
	return false;
#endif
}

/* This is usually used to implement CFG_SET command.
 * With previous CFG_GET command executed properly, the retrieved data
 * are shadowed in local structure. WSP FW has a set of shadow too,
 * so that only the difference to be sent over SMDIO.
 */
static int __gsw_api_wrap_cmd_r(const GSW_Device_t *dev, uint16_t cmd, void *pdata,
				uint16_t size, uint16_t r_size)
{
#if defined(ENABLE_GETSET_OPT) && ENABLE_GETSET_OPT
	int ret;

	uint16_t max, i;
	uint16_t *data;
	int16_t result = 0;

	max = (size + 1) / 2;
	data = pdata;

	ret = __wait_ctrl_busy(dev);
	if (ret < 0)
		return ret;

	for (uint16_t i = 0; i < max; i++) {
		uint16_t off = i % GSW_MMD_REG_DATA_MAX_SIZE;

		if (i && off == 0) {
			/* Send command to set data when every
			 * GSW_MMD_REG_DATA_MAX_SIZE of WORDs are written
			 * and reload next batch of data from last CFG_GET.
			 */
			ret = __gsw_set_data(dev, i);
			if (ret < 0)
				return ret;
		}

		if (data[i] == shadow.data.data[i])
			continue;

		gsw_write(dev, GSW_MMD_REG_DATA_FIRST + off,
			  sys_le16_to_cpu(data[i]));
	}

	ret = __gsw_send_cmd(dev, cmd, size, &result);
	if (ret < 0)
		return ret;

	if (result < 0)
		return result;

	max = (r_size + 1) / 2;
	for (i = 0; i < max; i++) {
		uint16_t off = i % GSW_MMD_REG_DATA_MAX_SIZE;

		if (i && off == 0) {
			/* Send command to fetch next batch of data
			 * when every GSW_MMD_REG_DATA_MAX_SIZE of WORDs
			 * are read.
			 */
			ret = __gsw_get_data(dev, i);
			if (ret < 0)
				return ret;
		}

		ret = gsw_read(dev, GSW_MMD_REG_DATA_FIRST + off);
		if (ret < 0)
			return ret;

		if ((i * 2 + 1) == r_size) {
			/* Special handling for last BYTE
			 * if it's not WORD aligned.
			 */
			*(uint8_t *)&data[i] = ret & 0xFF;
		} else {
			data[i] = sys_cpu_to_le16((uint16_t)ret);
		}
	}

	shadow.data.data[max] = 0;
	memcpy(shadow.data.data, data, r_size);

	return result;
#else /* defined(ENABLE_GETSET_OPT) && ENABLE_GETSET_OPT */
	ARG_UNUSED(dev);
	ARG_UNUSED(cmd);
	ARG_UNUSED(pdata);
	ARG_UNUSED(size);
	return -ENOTSUP;
#endif /* defined(ENABLE_GETSET_OPT) && ENABLE_GETSET_OPT */
}

int gsw_api_wrap(const GSW_Device_t *dev, uint16_t cmd, void *pdata,
		 uint16_t size, uint16_t cmd_r, uint16_t r_size)
{
	int ret;
	uint16_t max, i, cnt;
	uint16_t *data;
	int16_t result = 0;

	if (!dev || (!pdata && size))
		return -EINVAL;

	assert(size <= sizeof(mmd_api_data_t));
	assert(r_size <= size);

	dev->lock(dev->lock_data);

	if (__gsw_cmd_r_valid(cmd_r)) {
		/* Special handling for GET and SET command pair. */
		ret = __gsw_api_wrap_cmd_r(dev, cmd, pdata, size, r_size);
		goto EXIT;
	}

	max = (size + 1) / 2;
	data = pdata;

	/* Check whether it's worth to issue RST_DATA command. */
	for (i = cnt = 0; i < max && cnt < THR_RST_DATA; i++) {
		if (!data[i])
			cnt++;
	}

	ret = __wait_ctrl_busy(dev);
	if (ret < 0)
		goto EXIT;

	if (cnt >= THR_RST_DATA) {
		/* Issue RST_DATA commdand. */
		ret = __gsw_rst_data(dev);
		if (ret < 0)
			goto EXIT;

		for (uint16_t i = 0, cnt = 0; i < max; i++) {
			uint16_t off = i % GSW_MMD_REG_DATA_MAX_SIZE;

			if (i && off == 0) {
				uint16_t cnt_old = cnt;

				cnt = 0;

				/* No actual data was written. */
				if (!cnt_old)
					continue;

				/* Send command to set data when every
				 * GSW_MMD_REG_DATA_MAX_SIZE of WORDs are written
				 * and clear the MMD registe space.
				 */
				ret = __gsw_set_data(dev, i);
				if (ret < 0)
					goto EXIT;
			}

			/* Skip '0' data. */
			if (!data[i])
				continue;

			gsw_write(dev, GSW_MMD_REG_DATA_FIRST + off,
				  sys_le16_to_cpu(data[i]));
			cnt++;
		}
	} else {
		for (i = 0; i < max; i++) {
			uint16_t off = i % GSW_MMD_REG_DATA_MAX_SIZE;

			if (i && off == 0) {
				/* Send command to set data when every
				 * GSW_MMD_REG_DATA_MAX_SIZE of WORDs are written.
				 */
				ret = __gsw_set_data(dev, i);
				if (ret < 0)
					goto EXIT;
			}

			gsw_write(dev, GSW_MMD_REG_DATA_FIRST + off,
				  sys_le16_to_cpu(data[i]));
		}
	}

	ret = __gsw_send_cmd(dev, cmd, size, &result);
	if (ret < 0)
		goto EXIT;

	if (result < 0) {
		ret = result;
		goto EXIT;
	}

	max = (r_size + 1) / 2;
	for (i = 0; i < max; i++) {
		uint16_t off = i % GSW_MMD_REG_DATA_MAX_SIZE;

		if (i && off == 0) {
			/* Send command to fetch next batch of data
			 * when every GSW_MMD_REG_DATA_MAX_SIZE of WORDs
			 * are read.
			 */
			ret = __gsw_get_data(dev, i);
			if (ret < 0)
				goto EXIT;
		}

		ret = gsw_read(dev, GSW_MMD_REG_DATA_FIRST + off);
		if (ret < 0)
			goto EXIT;

		if ((i * 2 + 1) == r_size) {
			/* Special handling for last BYTE
			 * if it's not WORD aligned.
			 */
			*(uint8_t *)&data[i] = ret & 0xFF;
		} else {
			data[i] = sys_cpu_to_le16((uint16_t)ret);
		}
	}

#if defined(ENABLE_GETSET_OPT) && ENABLE_GETSET_OPT
	shadow.data.data[max] = 0;
	memcpy(shadow.data.data, data, r_size);
#endif

	ret = result;

EXIT:
#if defined(ENABLE_GETSET_OPT) && ENABLE_GETSET_OPT
	shadow.ctrl = cmd;
	shadow.ret = ret;
#endif
	dev->unlock(dev->lock_data);
	return ret;
}
