/*
 * rdp_drv_parser.c
 * 
 */

#include "rdp_drv_parser.h"
#include "rdd.h"


/*
 * h/w definitions
 */


#define RDP_PARSER_KEY_COMMAND_DWORDS               6

typedef union
{
    uint64_t dword;
    uint32_t word[2];
    struct 
    {
        uint64_t num_of_key_build_dwords :3;
        uint64_t aux_field_offset        :10;
        uint64_t aux_field_en            :1;
        uint64_t generic_field_7_state   :2;
        uint64_t generic_field_6_state   :2;
        uint64_t generic_field_5_state   :2;
        uint64_t generic_field_4_state   :2;
        uint64_t generic_field_3_state   :2;
        uint64_t generic_field_2_state   :2;
        uint64_t generic_field_1_state   :2;
        uint64_t generic_field_0_state   :2;
        uint64_t generic_field_7_cnt     :4;
        uint64_t generic_field_6_cnt     :4;
        uint64_t generic_field_5_cnt     :4;
        uint64_t generic_field_4_cnt     :4;
        uint64_t generic_field_3_cnt     :4;
        uint64_t generic_field_2_cnt     :4;
        uint64_t generic_field_1_cnt     :5;
        uint64_t generic_field_0_cnt     :5;
    } bits;
} key_build_sram_cfg;

/* Raw parser profile config table */
typedef struct 
{
    key_build_sram_cfg cfg;
    uint64_t key_commands[RDP_PARSER_KEY_COMMAND_DWORDS];
} rdp_parser_key_build_table;

void rdp_drv_parser_cfg_key_build_table(int profile_id)
{
    uint32_t *key_build_table_addr;
    rdp_parser_key_build_table key_build_table;
    int i;
    /* currently dummy function until generate TCAM driver to support key build*/

    memset(&key_build_table, 0, sizeof(rdp_parser_key_build_table));
    key_build_table.cfg.bits.num_of_key_build_dwords = 4;


    key_build_table_addr = RDD_PARSER_KEY_BUILD_TABLE_ADDRESS_ARR;

    /* Copy commands first then configuration */
    for (i = 0; i < 6; i++)
    {
        RDD_PARSER_KEY_BUILD_ENTRY_KEY_CMDS_WRITE_G(key_build_table.key_commands[i] >> 32, 
            key_build_table_addr, profile_id, i*2);
        RDD_PARSER_KEY_BUILD_ENTRY_KEY_CMDS_WRITE_G(key_build_table.key_commands[i] & 0xffffffff, 
            key_build_table_addr, profile_id, i*2+1);
    }
    /* sram_cfg is word-swapped */
    RDD_PARSER_KEY_BUILD_ENTRY_CONFIG_WRITE_G(key_build_table.cfg.word[1], 
        key_build_table_addr, profile_id, 0);
    RDD_PARSER_KEY_BUILD_ENTRY_CONFIG_WRITE_G(key_build_table.cfg.word[0], 
        key_build_table_addr, profile_id, 1);
}
