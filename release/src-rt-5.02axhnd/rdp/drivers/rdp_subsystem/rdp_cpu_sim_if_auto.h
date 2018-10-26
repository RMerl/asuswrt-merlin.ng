#ifndef _RDP_CPU_SIM_IF_AUTO_H_
#define _RDP_CPU_SIM_IF_AUTO_H_

/* Rdpa Simulator (Sw) <--> Runner Simulator (Hw) interface */

#define MAX_HOST_DATA_PASSED	2048
#define MAX_PKT_CFG_PASSED      3000
#pragma pack(push, 1)

/* Messages from Rdpa Simulator (Sw) to Runner Simulator (Hw) */
typedef enum 
{
    SW2HW_MSG_NONE          = 0,
    SW2HW_MSG_FPM_ALLOC	    = 1,
    SW2HW_MSG_FPM_FREE      = 2,
    SW2HW_MSG_QM_REQUEST    = 3,
    SW2HW_MSG_DDR_READ      = 4,
    SW2HW_MSG_DDR_WRTIE	    = 5,
    SW2HW_MSG_MEM_READ      = 6,
    SW2HW_MSG_MEM_WRITE     = 7,
    /* notification for the last message in a conversation/method to be done in zero-time */
    SW2HW_MSG_END_OF_METHOD	= 8,
    SW2HW_MSG_WAKEUP_RUNNER = 9,
    SW2HW_MSG_COUNTER_READ  = 10
} sw2hw_msg_t;

/* Messages from Runner Simulator (Hw)  to Rdpa Simulator (Sw) */
typedef enum 
{
    HW2SW_MSG_NONE                  = 0,
    HW2SW_MSG_SEND_PACKET           = 1, /* insert cpu tx packet */
    HW2SW_MSG_INTERRUPT	            = 2,
    HW2SW_MSG_END_OF_SIMULATION	    = 3, /* for various checks */
    HW2SW_MSG_FPM_ALLOC_RESPONSE    = 4,
    HW2SW_MSG_DDR_READ_RESPONSE     = 5,
    HW2SW_MSG_MEM_READ_RESPONSE     = 6,
    HW2SW_MSG_COUNTER_READ_RESPONSE = 7
} hw2sw_msg_t;

typedef struct
{
    sw2hw_msg_t type;
    union
    {
        struct
        {
            uint32_t pool_id;
        } fpm_alloc;

        struct
        {
            uint32_t buffer_num;
        } fpm_free;

        struct
        {
            uint32_t queue_id;
            uint8_t pd[16];
        } qm_insert_request;

        struct
        {
            uint8_t finish;
        } end_of_method;

        struct
        {
            uint32_t size;
            uint64_t address;
            uint8_t  data[MAX_HOST_DATA_PASSED];
        } ddr_write;

        struct
        {
            uint32_t size;
            uint64_t address;
        } ddr_read;

        struct
        {
            uint32_t size;
            uint32_t address;
            uint8_t  data[MAX_HOST_DATA_PASSED];
        } mem_write;

        struct
        {
            uint32_t size;
            uint32_t address;
        } mem_read;

        struct
        {
            uint32_t runner_id;
            uint32_t task_id;
        } wakeup;
        
        struct
        {
            uint32_t group_id;
            uint32_t start_counter;
            uint32_t count;
        } counter_read;
    };
} sw2hw_msg;

typedef struct
{
    hw2sw_msg_t type;
    union
    {
        struct
        {
            char pkt_cfg[MAX_PKT_CFG_PASSED];
        } send_packet;

        struct
        {
            uint32_t  interrupt_id;
        } interrupt;

        struct
        {
            uint8_t  finish;
        } end_of_simulation;

        struct
        {
            uint32_t size;
            uint8_t  data[MAX_HOST_DATA_PASSED];
        } read_response;

        struct
        {
            uint32_t buffer_num;
        } fpm_alloc_response;

        struct
        {
            uint32_t size;
            uint8_t  cntr_double;
            uint8_t  cn0_bytes;            
            uint8_t  data[MAX_HOST_DATA_PASSED];
        } counter_read_response;

    };
} hw2sw_msg;

/* end packing */
#pragma pack(pop)
#endif
