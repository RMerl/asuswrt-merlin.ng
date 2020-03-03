/*
 * Copyright (c) 2014, NVIDIA CORPORATION. All rights reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef __NOUVEAU_PLATFORM_H__
#define __NOUVEAU_PLATFORM_H__

#include "core/device.h"
#include "core/mm.h"

struct reset_control;
struct clk;
struct regulator;
struct iommu_domain;
struct platform_driver;

struct nouveau_platform_gpu {
	struct reset_control *rst;
	struct clk *clk;
	struct clk *clk_pwr;

	struct regulator *vdd;

	struct {
		/*
		 * Protects accesses to mm from subsystems
		 */
		struct mutex mutex;

		struct nvkm_mm _mm;
		/*
		 * Just points to _mm. We need this to avoid embedding
		 * struct nvkm_mm in os.h
		 */
		struct nvkm_mm *mm;
		struct iommu_domain *domain;
		unsigned long pgshift;
	} iommu;
};

struct nouveau_platform_device {
	struct nvkm_device device;

	struct nouveau_platform_gpu *gpu;

	int gpu_speedo;
};

#define nv_device_to_platform(d)                                               \
	container_of(d, struct nouveau_platform_device, device)

extern struct platform_driver nouveau_platform_driver;

#endif
