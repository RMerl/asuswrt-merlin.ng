// SPDX-License-Identifier: GPL-2.0+
/*
 * position independent shared object relocator
 *
 * Copyright (c) 2019 Heinrich Schuchardt
 */

#include <host_arch.h>

#if HOST_ARCH == HOST_ARCH_X86_64
#include "../../../arch/x86/lib/reloc_x86_64_efi.c"
#endif

#if HOST_ARCH == HOST_ARCH_X86
#include "../../../arch/x86/lib/reloc_ia32_efi.c"
#endif

#if HOST_ARCH == HOST_ARCH_AARCH64
#include "../../../arch/arm/lib/reloc_aarch64_efi.c"
#endif

#if HOST_ARCH == HOST_ARCH_ARM
#include "../../../arch/arm/lib/reloc_arm_efi.c"
#endif

#if HOST_ARCH == HOST_ARCH_RISCV32
#include "../../../arch/riscv/lib/reloc_riscv_efi.c"
#endif

#if HOST_ARCH == HOST_ARCH_RISCV64
#include "../../../arch/riscv/lib/reloc_riscv_efi.c"
#endif
