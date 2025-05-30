# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the drm device driver.  This driver provides support for the
# Direct Rendering Infrastructure (DRI) in XFree86 4.1.0 and higher.

# Add a set of useful warning flags and enable -Werror for CI to prevent
# trivial mistakes from creeping in. We have to do this piecemeal as we reject
# any patch that isn't warning clean, so turning on -Wall -Wextra (or W=1) we
# need to filter out dubious warnings.  Still it is our interest
# to keep running locally with W=1 C=1 until we are completely clean.
#
# Note the danger in using -Wall -Wextra is that when CI updates gcc we
# will most likely get a sudden build breakage... Hopefully we will fix
# new warnings before CI updates!
ifdef BCM_KF # defined(CONFIG_BCM_KF_VLA_REMOVAL_BACKPORT)
subdir-ccflags-y := -Wall -Wextra
else
subdir-ccflags-y := -Wall -Wextra -Wvla
endif
subdir-ccflags-y += $(call cc-disable-warning, unused-parameter)
subdir-ccflags-y += $(call cc-disable-warning, type-limits)
subdir-ccflags-y += $(call cc-disable-warning, missing-field-initializers)
subdir-ccflags-y += $(call cc-disable-warning, implicit-fallthrough)
subdir-ccflags-y += $(call cc-disable-warning, unused-but-set-variable)
# clang warnings
subdir-ccflags-y += $(call cc-disable-warning, sign-compare)
subdir-ccflags-y += $(call cc-disable-warning, sometimes-uninitialized)
subdir-ccflags-y += $(call cc-disable-warning, initializer-overrides)
subdir-ccflags-$(CONFIG_DRM_I915_WERROR) += -Werror

# Fine grained warnings disable
CFLAGS_i915_pci.o = $(call cc-disable-warning, override-init)
CFLAGS_intel_fbdev.o = $(call cc-disable-warning, override-init)

subdir-ccflags-y += \
	$(call as-instr,movntdqa (%eax)$(comma)%xmm0,-DCONFIG_AS_MOVNTDQA)

# Please keep these build lists sorted!

# core driver code
i915-y := i915_drv.o \
	  i915_irq.o \
	  i915_memcpy.o \
	  i915_mm.o \
	  i915_params.o \
	  i915_pci.o \
          i915_suspend.o \
	  i915_syncmap.o \
	  i915_sw_fence.o \
	  i915_sysfs.o \
	  intel_csr.o \
	  intel_device_info.o \
	  intel_pm.o \
	  intel_runtime_pm.o \
	  intel_workarounds.o

i915-$(CONFIG_COMPAT)   += i915_ioc32.o
i915-$(CONFIG_DEBUG_FS) += i915_debugfs.o intel_pipe_crc.o
i915-$(CONFIG_PERF_EVENTS) += i915_pmu.o

# GEM code
i915-y += i915_cmd_parser.o \
	  i915_gem_batch_pool.o \
	  i915_gem_clflush.o \
	  i915_gem_context.o \
	  i915_gem_dmabuf.o \
	  i915_gem_evict.o \
	  i915_gem_execbuffer.o \
	  i915_gem_fence_reg.o \
	  i915_gem_gtt.o \
	  i915_gem_internal.o \
	  i915_gem.o \
	  i915_gem_object.o \
	  i915_gem_render_state.o \
	  i915_gem_shrinker.o \
	  i915_gem_stolen.o \
	  i915_gem_tiling.o \
	  i915_gem_userptr.o \
	  i915_gemfs.o \
	  i915_query.o \
	  i915_request.o \
	  i915_timeline.o \
	  i915_trace_points.o \
	  i915_vma.o \
	  intel_breadcrumbs.o \
	  intel_engine_cs.o \
	  intel_hangcheck.o \
	  intel_lrc.o \
	  intel_mocs.o \
	  intel_ringbuffer.o \
	  intel_uncore.o \
	  intel_wopcm.o

# general-purpose microcontroller (GuC) support
i915-y += intel_uc.o \
	  intel_uc_fw.o \
	  intel_guc.o \
	  intel_guc_ads.o \
	  intel_guc_ct.o \
	  intel_guc_fw.o \
	  intel_guc_log.o \
	  intel_guc_submission.o \
	  intel_huc.o \
	  intel_huc_fw.o

# autogenerated null render state
i915-y += intel_renderstate_gen6.o \
	  intel_renderstate_gen7.o \
	  intel_renderstate_gen8.o \
	  intel_renderstate_gen9.o

# modesetting core code
i915-y += intel_audio.o \
	  intel_atomic.o \
	  intel_atomic_plane.o \
	  intel_bios.o \
	  intel_cdclk.o \
	  intel_color.o \
	  intel_display.o \
	  intel_dpio_phy.o \
	  intel_dpll_mgr.o \
	  intel_fbc.o \
	  intel_fifo_underrun.o \
	  intel_frontbuffer.o \
	  intel_hdcp.o \
	  intel_hotplug.o \
	  intel_modes.o \
	  intel_overlay.o \
	  intel_psr.o \
	  intel_sideband.o \
	  intel_sprite.o
i915-$(CONFIG_ACPI)		+= intel_acpi.o intel_opregion.o
i915-$(CONFIG_DRM_FBDEV_EMULATION)	+= intel_fbdev.o

# modesetting output/encoder code
i915-y += dvo_ch7017.o \
	  dvo_ch7xxx.o \
	  dvo_ivch.o \
	  dvo_ns2501.o \
	  dvo_sil164.o \
	  dvo_tfp410.o \
	  icl_dsi.o \
	  intel_crt.o \
	  intel_ddi.o \
	  intel_dp_aux_backlight.o \
	  intel_dp_link_training.o \
	  intel_dp_mst.o \
	  intel_dp.o \
	  intel_dsi_dcs_backlight.o \
	  intel_dsi_vbt.o \
	  intel_dvo.o \
	  intel_hdmi.o \
	  intel_i2c.o \
	  intel_lspcon.o \
	  intel_lvds.o \
	  intel_panel.o \
	  intel_sdvo.o \
	  intel_tv.o \
	  vlv_dsi.o \
	  vlv_dsi_pll.o

# Post-mortem debug and GPU hang state capture
i915-$(CONFIG_DRM_I915_CAPTURE_ERROR) += i915_gpu_error.o
i915-$(CONFIG_DRM_I915_SELFTEST) += \
	selftests/i915_random.o \
	selftests/i915_selftest.o \
	selftests/igt_flush_test.o

# virtual gpu code
i915-y += i915_vgpu.o

# perf code
i915-y += i915_perf.o \
	  i915_oa_hsw.o \
	  i915_oa_bdw.o \
	  i915_oa_chv.o \
	  i915_oa_sklgt2.o \
	  i915_oa_sklgt3.o \
	  i915_oa_sklgt4.o \
	  i915_oa_bxt.o \
	  i915_oa_kblgt2.o \
	  i915_oa_kblgt3.o \
	  i915_oa_glk.o \
	  i915_oa_cflgt2.o \
	  i915_oa_cflgt3.o \
	  i915_oa_cnl.o \
	  i915_oa_icl.o

ifeq ($(CONFIG_DRM_I915_GVT),y)
i915-y += intel_gvt.o
include $(src)/gvt/Makefile
endif

# LPE Audio for VLV and CHT
i915-y += intel_lpe_audio.o

obj-$(CONFIG_DRM_I915) += i915.o
