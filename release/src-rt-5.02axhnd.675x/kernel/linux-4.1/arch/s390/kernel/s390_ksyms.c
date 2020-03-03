#include <linux/module.h>
#include <linux/kvm_host.h>
#include <asm/ftrace.h>

#ifdef CONFIG_FUNCTION_TRACER
EXPORT_SYMBOL(_mcount);
#endif
#if IS_ENABLED(CONFIG_KVM)
EXPORT_SYMBOL(sie64a);
EXPORT_SYMBOL(sie_exit);
#endif
EXPORT_SYMBOL(memcpy);
EXPORT_SYMBOL(memset);
