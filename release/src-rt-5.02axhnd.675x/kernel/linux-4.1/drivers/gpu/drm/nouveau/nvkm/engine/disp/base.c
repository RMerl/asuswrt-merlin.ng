/*
 * Copyright 2013 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Ben Skeggs
 */
#include "priv.h"
#include "conn.h"
#include "outp.h"

#include <core/notify.h>
#include <subdev/bios.h>
#include <subdev/bios/dcb.h>

#include <nvif/class.h>
#include <nvif/event.h>
#include <nvif/unpack.h>

int
nvkm_disp_vblank_ctor(struct nvkm_object *object, void *data, u32 size,
		      struct nvkm_notify *notify)
{
	struct nvkm_disp *disp =
		container_of(notify->event, typeof(*disp), vblank);
	union {
		struct nvif_notify_head_req_v0 v0;
	} *req = data;
	int ret;

	if (nvif_unpack(req->v0, 0, 0, false)) {
		notify->size = sizeof(struct nvif_notify_head_rep_v0);
		if (ret = -ENXIO, req->v0.head <= disp->vblank.index_nr) {
			notify->types = 1;
			notify->index = req->v0.head;
			return 0;
		}
	}

	return ret;
}

void
nvkm_disp_vblank(struct nvkm_disp *disp, int head)
{
	struct nvif_notify_head_rep_v0 rep = {};
	nvkm_event_send(&disp->vblank, 1, head, &rep, sizeof(rep));
}

static int
nvkm_disp_hpd_ctor(struct nvkm_object *object, void *data, u32 size,
		   struct nvkm_notify *notify)
{
	struct nvkm_disp *disp =
		container_of(notify->event, typeof(*disp), hpd);
	union {
		struct nvif_notify_conn_req_v0 v0;
	} *req = data;
	struct nvkm_output *outp;
	int ret;

	if (nvif_unpack(req->v0, 0, 0, false)) {
		notify->size = sizeof(struct nvif_notify_conn_rep_v0);
		list_for_each_entry(outp, &disp->outp, head) {
			if (ret = -ENXIO, outp->conn->index == req->v0.conn) {
				if (ret = -ENODEV, outp->conn->hpd.event) {
					notify->types = req->v0.mask;
					notify->index = req->v0.conn;
					ret = 0;
				}
				break;
			}
		}
	}

	return ret;
}

static const struct nvkm_event_func
nvkm_disp_hpd_func = {
	.ctor = nvkm_disp_hpd_ctor
};

int
nvkm_disp_ntfy(struct nvkm_object *object, u32 type, struct nvkm_event **event)
{
	struct nvkm_disp *disp = (void *)object->engine;
	switch (type) {
	case NV04_DISP_NTFY_VBLANK:
		*event = &disp->vblank;
		return 0;
	case NV04_DISP_NTFY_CONN:
		*event = &disp->hpd;
		return 0;
	default:
		break;
	}
	return -EINVAL;
}

int
_nvkm_disp_fini(struct nvkm_object *object, bool suspend)
{
	struct nvkm_disp *disp = (void *)object;
	struct nvkm_output *outp;
	int ret;

	list_for_each_entry(outp, &disp->outp, head) {
		ret = nv_ofuncs(outp)->fini(nv_object(outp), suspend);
		if (ret && suspend)
			goto fail_outp;
	}

	return nvkm_engine_fini(&disp->base, suspend);

fail_outp:
	list_for_each_entry_continue_reverse(outp, &disp->outp, head) {
		nv_ofuncs(outp)->init(nv_object(outp));
	}

	return ret;
}

int
_nvkm_disp_init(struct nvkm_object *object)
{
	struct nvkm_disp *disp = (void *)object;
	struct nvkm_output *outp;
	int ret;

	ret = nvkm_engine_init(&disp->base);
	if (ret)
		return ret;

	list_for_each_entry(outp, &disp->outp, head) {
		ret = nv_ofuncs(outp)->init(nv_object(outp));
		if (ret)
			goto fail_outp;
	}

	return ret;

fail_outp:
	list_for_each_entry_continue_reverse(outp, &disp->outp, head) {
		nv_ofuncs(outp)->fini(nv_object(outp), false);
	}

	return ret;
}

void
_nvkm_disp_dtor(struct nvkm_object *object)
{
	struct nvkm_disp *disp = (void *)object;
	struct nvkm_output *outp, *outt;

	nvkm_event_fini(&disp->vblank);
	nvkm_event_fini(&disp->hpd);

	if (disp->outp.next) {
		list_for_each_entry_safe(outp, outt, &disp->outp, head) {
			nvkm_object_ref(NULL, (struct nvkm_object **)&outp);
		}
	}

	nvkm_engine_destroy(&disp->base);
}

int
nvkm_disp_create_(struct nvkm_object *parent, struct nvkm_object *engine,
		  struct nvkm_oclass *oclass, int heads, const char *intname,
		  const char *extname, int length, void **pobject)
{
	struct nvkm_disp_impl *impl = (void *)oclass;
	struct nvkm_bios *bios = nvkm_bios(parent);
	struct nvkm_disp *disp;
	struct nvkm_oclass **sclass;
	struct nvkm_object *object;
	struct dcb_output dcbE;
	u8  hpd = 0, ver, hdr;
	u32 data;
	int ret, i;

	ret = nvkm_engine_create_(parent, engine, oclass, true, intname,
				  extname, length, pobject);
	disp = *pobject;
	if (ret)
		return ret;

	INIT_LIST_HEAD(&disp->outp);

	/* create output objects for each display path in the vbios */
	i = -1;
	while ((data = dcb_outp_parse(bios, ++i, &ver, &hdr, &dcbE))) {
		if (dcbE.type == DCB_OUTPUT_UNUSED)
			continue;
		if (dcbE.type == DCB_OUTPUT_EOL)
			break;
		data = dcbE.location << 4 | dcbE.type;

		oclass = nvkm_output_oclass;
		sclass = impl->outp;
		while (sclass && sclass[0]) {
			if (sclass[0]->handle == data) {
				oclass = sclass[0];
				break;
			}
			sclass++;
		}

		nvkm_object_ctor(*pobject, NULL, oclass, &dcbE, i, &object);
		hpd = max(hpd, (u8)(dcbE.connector + 1));
	}

	ret = nvkm_event_init(&nvkm_disp_hpd_func, 3, hpd, &disp->hpd);
	if (ret)
		return ret;

	ret = nvkm_event_init(impl->vblank, 1, heads, &disp->vblank);
	if (ret)
		return ret;

	return 0;
}
