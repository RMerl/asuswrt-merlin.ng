/* IMAGE 0 LABELS */
#ifndef IMAGE_0_CODE_ADDRESSES
#define IMAGE_0_CODE_ADDRESSES

#ifndef PC_ADDRESS_INST_IND

#define image_0_start_task_initialization_task        (0x18)
#define image_0_initialization_task        (0x18)
#define image_0_start_task_cpu_tx_wakeup_request        (0x4a8)
#define image_0_cpu_tx_wakeup_request        (0x4a8)
#define image_0_start_task_direct_processing_wakeup_request        (0x198)
#define image_0_direct_processing_wakeup_request        (0x198)
#define image_0_start_task_debug_routine        (0x64)
#define image_0_debug_routine        (0x64)
#define image_0_debug_routine_handler        (0xc)

#else

#define image_0_start_task_initialization_task        (0x6)
#define image_0_initialization_task        (0x6)
#define image_0_start_task_cpu_tx_wakeup_request        (0x12a)
#define image_0_cpu_tx_wakeup_request        (0x12a)
#define image_0_start_task_direct_processing_wakeup_request        (0x66)
#define image_0_direct_processing_wakeup_request        (0x66)
#define image_0_start_task_debug_routine        (0x19)
#define image_0_debug_routine        (0x19)
#define image_0_debug_routine_handler        (0x3)

#endif


#endif

/* COMMON LABELS */
#ifndef COMMON_CODE_ADDRESSES
#define COMMON_CODE_ADDRESSES

#define INVALID_LABEL_ADDRESS 0xFFFFFF

#ifndef PC_ADDRESS_INST_IND


#else


#endif


#endif

