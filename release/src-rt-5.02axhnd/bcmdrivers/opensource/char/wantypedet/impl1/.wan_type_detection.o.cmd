cmd_../../bcmdrivers/opensource/char/wantypedet/bcm94908/wan_type_detection.o := /opt/toolchains/crosstools-aarch64-gcc-5.5-linux-4.1-glibc-2.26-binutils-2.28.1/usr/bin/aarch64-buildroot-linux-gnu-gcc -Wp,-MD,../../bcmdrivers/opensource/char/wantypedet/bcm94908/.wan_type_detection.o.d  -nostdinc -isystem /home/merlin/am-toolchains/brcm-arm-hnd/crosstools-aarch64-gcc-5.5-linux-4.1-glibc-2.26-binutils-2.28.1/bin/../lib/gcc/aarch64-buildroot-linux-gnu/5.5.0/include -I./arch/arm64/include -Iarch/arm64/include/generated/uapi -Iarch/arm64/include/generated  -Iinclude -I./arch/arm64/include/uapi -Iarch/arm64/include/generated/uapi -I./include/uapi -Iinclude/generated/uapi -include ./include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 -DRTAX88U -DEXT_BCM53134 -mgeneral-regs-only -fno-delete-null-pointer-checks -fno-PIE -O2 --param=allow-store-data-races=0 -DCC_HAVE_ASM_GOTO -Wframe-larger-than=2048 -fno-stack-protector -Wno-unused-but-set-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-var-tracking-assignments -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fno-stack-check -fconserve-stack -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -DBCA_HNDROUTER @/home/merlin/amng-ax-build/release/src-rt-5.02axhnd/kernel/linux-4.1/rdp_flags.txt -Werror -Wall  -DMODULE -mcmodel=large  -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(wan_type_detection)"  -D"KBUILD_MODNAME=KBUILD_STR(wantypedet)" -c -o ../../bcmdrivers/opensource/char/wantypedet/bcm94908/wan_type_detection.o ../../bcmdrivers/opensource/char/wantypedet/bcm94908/wan_type_detection.c

source_../../bcmdrivers/opensource/char/wantypedet/bcm94908/wan_type_detection.o := ../../bcmdrivers/opensource/char/wantypedet/bcm94908/wan_type_detection.c

deps_../../bcmdrivers/opensource/char/wantypedet/bcm94908/wan_type_detection.o := \
    $(wildcard include/config/bcm/pon/wan/type/auto/detect.h) \
    $(wildcard include/config/bcm96858.h) \
    $(wildcard include/config/reg/address.h) \
    $(wildcard include/config/lof/params/reg/address.h) \
    $(wildcard include/config/bit/rx/disable.h) \
    $(wildcard include/config/lof/delta/shift.h) \
    $(wildcard include/config/lof/delta.h) \
    $(wildcard include/config/lof/alpha/shift.h) \
    $(wildcard include/config/lof/alpha.h) \
    $(wildcard include/config/lof/params/clear/mask.h) \
    $(wildcard include/config/bcm96856.h) \
  include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  include/linux/compiler.h \
    $(wildcard include/config/sparse/rcu/pointer.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
    $(wildcard include/config/bcm/kf/buzzz.h) \
    $(wildcard include/config/buzzz/func.h) \
    $(wildcard include/config/kprobes.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
    $(wildcard include/config/gcov/kernel.h) \
    $(wildcard include/config/arch/use/builtin/bswap.h) \
  include/uapi/linux/types.h \
  arch/arm64/include/generated/asm/types.h \
  include/uapi/asm-generic/types.h \
  include/asm-generic/int-ll64.h \
  include/uapi/asm-generic/int-ll64.h \
  arch/arm64/include/uapi/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
    $(wildcard include/config/64bit.h) \
  include/uapi/asm-generic/bitsperlong.h \
  include/uapi/linux/posix_types.h \
  include/linux/stddef.h \
  include/uapi/linux/stddef.h \
  arch/arm64/include/uapi/asm/posix_types.h \
  include/uapi/asm-generic/posix_types.h \
  include/linux/types.h \
    $(wildcard include/config/have/uid16.h) \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/arch/dma/addr/t/64bit.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/bcm/kf/unaligned/exception.h) \
    $(wildcard include/config/mips/bcm963xx.h) \
  /home/merlin/am-toolchains/brcm-arm-hnd/crosstools-aarch64-gcc-5.5-linux-4.1-glibc-2.26-binutils-2.28.1/lib/gcc/aarch64-buildroot-linux-gnu/5.5.0/include/stdarg.h \
  include/uapi/linux/string.h \
  arch/arm64/include/asm/string.h \
  include/linux/delay.h \
  include/linux/kernel.h \
    $(wildcard include/config/bcm/kf/optee/414/backports.h) \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/atomic/sleep.h) \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/panic/timeout.h) \
    $(wildcard include/config/ring/buffer.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
  include/linux/linkage.h \
  include/linux/stringify.h \
  include/linux/export.h \
    $(wildcard include/config/have/underscore/symbol/prefix.h) \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/unused/symbols.h) \
  arch/arm64/include/asm/linkage.h \
  include/linux/bitops.h \
  arch/arm64/include/asm/bitops.h \
  arch/arm64/include/asm/barrier.h \
    $(wildcard include/config/smp.h) \
  include/asm-generic/bitops/builtin-__ffs.h \
  include/asm-generic/bitops/builtin-ffs.h \
  include/asm-generic/bitops/builtin-__fls.h \
  include/asm-generic/bitops/builtin-fls.h \
  include/asm-generic/bitops/ffz.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/find.h \
    $(wildcard include/config/generic/find/first/bit.h) \
  include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h \
  include/asm-generic/bitops/arch_hweight.h \
  include/asm-generic/bitops/const_hweight.h \
  include/asm-generic/bitops/lock.h \
  include/asm-generic/bitops/non-atomic.h \
  include/asm-generic/bitops/le.h \
  arch/arm64/include/uapi/asm/byteorder.h \
  include/linux/byteorder/little_endian.h \
  include/uapi/linux/byteorder/little_endian.h \
  include/linux/swab.h \
  include/uapi/linux/swab.h \
  arch/arm64/include/generated/asm/swab.h \
  include/uapi/asm-generic/swab.h \
  include/linux/byteorder/generic.h \
  include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  include/linux/typecheck.h \
  include/linux/printk.h \
    $(wildcard include/config/message/loglevel/default.h) \
    $(wildcard include/config/early/printk.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/bcm/kf/extra/debug.h) \
    $(wildcard include/config/dynamic/debug.h) \
  include/linux/init.h \
    $(wildcard include/config/broken/rodata.h) \
    $(wildcard include/config/lto.h) \
  include/linux/kern_levels.h \
  include/linux/cache.h \
    $(wildcard include/config/arch/has/cache/line/size.h) \
  include/uapi/linux/kernel.h \
  include/uapi/linux/sysinfo.h \
  arch/arm64/include/asm/cache.h \
  arch/arm64/include/asm/cachetype.h \
  arch/arm64/include/asm/cputype.h \
    $(wildcard include/config/bcm/kf/arm64/bcm963xx.h) \
  include/linux/dynamic_debug.h \
  include/linux/errno.h \
  include/uapi/linux/errno.h \
  arch/arm64/include/generated/asm/errno.h \
  include/uapi/asm-generic/errno.h \
  include/uapi/asm-generic/errno-base.h \
  arch/arm64/include/generated/asm/delay.h \
  include/asm-generic/delay.h \
  include/linux/bcm_log.h \
    $(wildcard include/config/bcm/kf/log.h) \
    $(wildcard include/config/brcm/colorize/prints.h) \
    $(wildcard include/config/bcm/log.h) \
  include/linux/bcm_log_mod.h \
    $(wildcard include/config/bcm/gmac.h) \
  include/uapi/linux/bcm_colors.h \
    $(wildcard include/config/bcm/kf/blog.h) \
    $(wildcard include/config/bcm/in/kernel.h) \
    $(wildcard include/config/bcm/colorize/prints.h) \
  /home/merlin/amng-ax-build/release/src-rt-5.02axhnd/bcmdrivers/opensource/include/bcm963xx/bcm_map_part.h \
    $(wildcard include/config/bcm963268.h) \
    $(wildcard include/config/bcm96838.h) \
    $(wildcard include/config/bcm963138.h) \
    $(wildcard include/config/bcm960333.h) \
    $(wildcard include/config/bcm963381.h) \
    $(wildcard include/config/bcm963148.h) \
    $(wildcard include/config/bcm96848.h) \
    $(wildcard include/config/bcm94908.h) \
    $(wildcard include/config/bcm947189.h) \
    $(wildcard include/config/bcm96836.h) \
    $(wildcard include/config/bcm963158.h) \
    $(wildcard include/config/bcm96846.h) \
  /home/merlin/amng-ax-build/release/src-rt-5.02axhnd/shared/opensource/include/bcm963xx/4908_map_part.h \
    $(wildcard include/config/lock.h) \
    $(wildcard include/config/arm.h) \
    $(wildcard include/config/arm64.h) \
    $(wildcard include/config/3x2/crossbar/support.h) \
  /home/merlin/amng-ax-build/release/src-rt-5.02axhnd/shared/opensource/include/bcm963xx/bcmtypes.h \
  /home/merlin/amng-ax-build/release/src-rt-5.02axhnd/shared/opensource/include/bcm963xx/bcm_io_map.h \
  /home/merlin/amng-ax-build/release/src-rt-5.02axhnd/shared/opensource/include/bcm963xx/8486x_map_part.h \
  /home/merlin/amng-ax-build/release/src-rt-5.02axhnd/bcmdrivers/opensource/include/bcm963xx/board.h \
    $(wildcard include/config/bcm/pon.h) \
    $(wildcard include/config/bcm/avs/pwrsave.h) \
    $(wildcard include/config/bcm/moca/avs.h) \
    $(wildcard include/config/bcm/watchdog/timer.h) \
    $(wildcard include/config/bcm/ddr/self/refresh/pwrsave.h) \
    $(wildcard include/config/bcm/pwrmngt/ddr/sr/api.h) \
    $(wildcard include/config/bcm/adsl.h) \
    $(wildcard include/config/bcm/rdpa.h) \
    $(wildcard include/config/bcm/dhd/runner.h) \
    $(wildcard include/config/bcm/optee.h) \
    $(wildcard include/config/bcm/b15/mega/barrier.h) \
  /home/merlin/amng-ax-build/release/src-rt-5.02axhnd/shared/opensource/include/bcm963xx/bcm_hwdefs.h \
    $(wildcard include/config/brcm/ikos.h) \
    $(wildcard include/config/bcm63138/sim.h) \
    $(wildcard include/config/bcm63148/sim.h) \
    $(wildcard include/config/bcm9647189.h) \
    $(wildcard include/config/bcm/jumbo/frame.h) \
    $(wildcard include/config/bcm/rdpa/mcast.h) \
    $(wildcard include/config/bcm/pon/xrdp.h) \
  /home/merlin/amng-ax-build/release/src-rt-5.02axhnd/shared/opensource/include/bcm963xx/bcmTag.h \
  /home/merlin/amng-ax-build/release/src-rt-5.02axhnd/shared/opensource/include/bcm963xx/boardparms.h \
    $(wildcard include/config/mdio.h) \
    $(wildcard include/config/mdio/pseudo/phy.h) \
    $(wildcard include/config/spi/ssb/0.h) \
    $(wildcard include/config/spi/ssb/1.h) \
    $(wildcard include/config/spi/ssb/2.h) \
    $(wildcard include/config/spi/ssb/3.h) \
    $(wildcard include/config/mmap.h) \
    $(wildcard include/config/gpio/mdio.h) \
    $(wildcard include/config/hs/spi/ssb/0.h) \
    $(wildcard include/config/hs/spi/ssb/1.h) \
    $(wildcard include/config/hs/spi/ssb/2.h) \
    $(wildcard include/config/hs/spi/ssb/3.h) \
    $(wildcard include/config/hs/spi/ssb/4.h) \
    $(wildcard include/config/hs/spi/ssb/5.h) \
    $(wildcard include/config/hs/spi/ssb/6.h) \
    $(wildcard include/config/hs/spi/ssb/7.h) \
    $(wildcard include/config/bp/phys/intf.h) \
    $(wildcard include/config/mask.h) \
    $(wildcard include/config/shift.h) \
    $(wildcard include/config/none.h) \
    $(wildcard include/config/1.h) \
    $(wildcard include/config/2.h) \
    $(wildcard include/config/custom.h) \
    $(wildcard include/config/debug.h) \
    $(wildcard include/config/override.h) \
    $(wildcard include/config/new/leds.h) \
  /home/merlin/amng-ax-build/release/src-rt-5.02axhnd/shared/opensource/include/bcm963xx/bp_defs.h \
  /home/merlin/amng-ax-build/release/src-rt-5.02axhnd/shared/opensource/include/bcm963xx/flash_common.h \
  /home/merlin/amng-ax-build/release/src-rt-5.02axhnd/bcmdrivers/opensource/include/bcm963xx/wan_drv.h \
    $(wildcard include/config/bcm/xrdp.h) \
    $(wildcard include/config/bcm/ngpon.h) \
  /home/merlin/amng-ax-build/release/src-rt-5.02axhnd/rdp/projects/WL4908/target/bdmf/framework/bdmf_data_types.h \
  /home/merlin/amng-ax-build/release/src-rt-5.02axhnd/bcmdrivers/opensource/include/bcm963xx/opticaldet.h \
  /home/merlin/amng-ax-build/release/src-rt-5.02axhnd/bcmdrivers/opensource/include/bcm963xx/wan_types.h \
  /home/merlin/amng-ax-build/release/src-rt-5.02axhnd/rdp/projects/WL4908/target/rdpa_gpl/rdpa_types.h \

../../bcmdrivers/opensource/char/wantypedet/bcm94908/wan_type_detection.o: $(deps_../../bcmdrivers/opensource/char/wantypedet/bcm94908/wan_type_detection.o)

$(deps_../../bcmdrivers/opensource/char/wantypedet/bcm94908/wan_type_detection.o):
