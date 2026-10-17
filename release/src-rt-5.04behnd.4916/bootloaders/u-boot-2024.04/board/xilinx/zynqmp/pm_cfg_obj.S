/* SPDX-License-Identifier: GPL-2.0+ */

.section .rodata

.global zynqmp_pm_cfg_obj
.type   zynqmp_pm_cfg_obj, @object
.global zynqmp_pm_cfg_obj_size
.type   zynqmp_pm_cfg_obj_size, @object

zynqmp_pm_cfg_obj:
.align 4
.incbin CONFIG_ZYNQMP_SPL_PM_CFG_OBJ_FILE

zynqmp_pm_cfg_obj_end:

zynqmp_pm_cfg_obj_size:
.int zynqmp_pm_cfg_obj_end - zynqmp_pm_cfg_obj
