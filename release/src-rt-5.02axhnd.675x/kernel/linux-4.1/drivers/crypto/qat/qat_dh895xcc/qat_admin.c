/*
  This file is provided under a dual BSD/GPLv2 license.  When using or
  redistributing this file, you may do so under either license.

  GPL LICENSE SUMMARY
  Copyright(c) 2014 Intel Corporation.
  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  Contact Information:
  qat-linux@intel.com

  BSD LICENSE
  Copyright(c) 2014 Intel Corporation.
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
    * Neither the name of Intel Corporation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <icp_qat_fw_init_admin.h>
#include <adf_accel_devices.h>
#include <adf_common_drv.h>
#include "adf_drv.h"

static struct service_hndl qat_admin;

static int qat_send_admin_cmd(struct adf_accel_dev *accel_dev, int cmd)
{
	struct adf_hw_device_data *hw_device = accel_dev->hw_device;
	struct icp_qat_fw_init_admin_req req;
	struct icp_qat_fw_init_admin_resp resp;
	int i;

	memset(&req, 0, sizeof(struct icp_qat_fw_init_admin_req));
	req.init_admin_cmd_id = cmd;
	for (i = 0; i < hw_device->get_num_aes(hw_device); i++) {
		memset(&resp, 0, sizeof(struct icp_qat_fw_init_admin_resp));
		if (adf_put_admin_msg_sync(accel_dev, i, &req, &resp) ||
		    resp.init_resp_hdr.status)
			return -EFAULT;
	}
	return 0;
}

static int qat_admin_start(struct adf_accel_dev *accel_dev)
{
	return qat_send_admin_cmd(accel_dev, ICP_QAT_FW_INIT_ME);
}

static int qat_admin_event_handler(struct adf_accel_dev *accel_dev,
				   enum adf_event event)
{
	int ret;

	switch (event) {
	case ADF_EVENT_START:
		ret = qat_admin_start(accel_dev);
		break;
	case ADF_EVENT_STOP:
	case ADF_EVENT_INIT:
	case ADF_EVENT_SHUTDOWN:
	default:
		ret = 0;
	}
	return ret;
}

int qat_admin_register(void)
{
	memset(&qat_admin, 0, sizeof(struct service_hndl));
	qat_admin.event_hld = qat_admin_event_handler;
	qat_admin.name = "qat_admin";
	qat_admin.admin = 1;
	return adf_service_register(&qat_admin);
}

int qat_admin_unregister(void)
{
	return adf_service_unregister(&qat_admin);
}
