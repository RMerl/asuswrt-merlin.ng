#include <common.h>

#define BOOT_STATE_VERSION_1 0x01
#define BOOT_STATE_VERSION_2 0x02

#define DO_NOT_RESET_ON_WATCHDOG (1<<22)
#define FLASH_CNTRL_RESET_VAL 0x050b

#ifndef CONFIG_SMC_BASED
#define PCIE_RESET_STATUS       0x10000000
#define SW_RESET_STATUS         0x20000000
#define HW_RESET_STATUS         0x40000000
#define POR_RESET_STATUS        0x80000000
#define RESET_STATUS_MASK       0xF0000000
#else
#define SW_RESET_STATUS         0x1    // bit in reg reset_ctrl_event_history_1
#define RESET_STATUS_MASK       0x1F
#endif


#define BCM_BOOT_REASON_BITS		(0x4)
#define BCM_BOOT_REASON_MASK		(0xF)
#define BCM_BOOT_REASON_REBOOT 		(0x00000000)
#define BCM_BOOT_REASON_ACTIVATE 	(0x00000001)
#define BCM_BOOT_REASON_PANIC 		(0x00000002)
#define BCM_BOOT_REASON_WATCHDOG	(0x00000004)

#define BCM_BOOT_PHASE_BITS		(0x0000004)
#define BCM_BOOT_PHASE_MASK		(0x000000F0)
#define BCM_BOOT_PHASE_UBOOT		(0x00000010)
#define BCM_BOOT_PHASE_LINUX_START	(0x00000020)
#define BCM_BOOT_PHASE_LINUX_RUN	(0x00000030)
#define BCM_BOOT_PHASE_TPL              (0x00000040)
#define BCM_BOOT_PHASE_FB_UBOOT		(0x00000050)
#define BCM_BOOT_PHASE_FB_LINUX_START	(0x00000060)

#define BCM_RESET_REASON_BITS		(BCM_BOOT_REASON_BITS+BCM_BOOT_PHASE_BITS)
#define BCM_RESET_REASON_MASK		(BCM_BOOT_REASON_MASK|BCM_BOOT_PHASE_MASK)
#define BCM_RESET_REASON_FULL_MASK      (0xffff)


#define BOOT_FAILED_COUNT_MASK		0x0c0000
#define BOOT_FAILED_COUNT_BIT_SHIFT	18


#define BOOT_IMAGE_ID_MASK		0x030000
#define BOOT_IMAGE_ID_SHIFT		16

#define MAX_BOOT_FAILED_COUNT		3

int bcmbca_get_reset_status(void);
void bcmbca_set_boot_reason(uint32_t value);
void bcmbca_clear_boot_reason(void);
uint32_t bcmbca_get_boot_reason(void);
void bca_bootstate_probe(void);
uint32_t bcmbca_get_old_boot_reason(void);
void bcmbca_set_boot_failed_count(uint32_t value);
void bcmbca_clear_boot_failed_count(void);
uint32_t bcmbca_get_boot_failed_count(void);
void bcmbca_set_sel_img_id(uint32_t value);
void bcmbca_clear_sel_img_id(void);
uint32_t bcmbca_get_sel_img_id(void);

