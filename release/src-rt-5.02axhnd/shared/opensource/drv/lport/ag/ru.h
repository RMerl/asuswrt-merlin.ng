/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

:>
*/
#ifndef _RU_H_
#define _RU_H_
/**
 * \brief Register tracking utilities
 *
 * The register track module provides functionality to comprehensively debug
 * register level transactions.  It is a slow interface that should be used in
 * parallel to standard direct read/write transactions.  The module can parse
 * registers into easy read field format.  Registers may be looked up by name
 * or address.
 *
 */
#ifdef __x86_64
#include <stdint.h>
#endif
#include "ru_config.h"
#include "ru_types.h"
#include "ru_chip.h"

//#define RU_TO_BBS /* <- uncomment to enable BBS register access */
#ifdef RU_TO_BBS
#define WRITE32(addr,value) bbs_send(BBS_WRITE, (uint32_t)(addr), value)
#define READ32(addr)        bbs_send(BBS_READ, (uint32_t)(addr), 0)

typedef enum
{
    BBS_READ,
    BBS_WRITE,
    BBS_TERMINATE
} bbs_op_type;

typedef struct
{
    bbs_op_type op;
    uint32_t address;
    uint32_t value;
} bbs_data;

uint32_t bbs_send(bbs_op_type op, uint32_t address,  uint32_t value);
#elif defined(RU_EMULATION)
#define WRITE32(addr,value) ru_emulation_client_send(TYPE_WRITE, (uint32_t)(addr),value)
#define READ32(addr) ru_emulation_client_send(TYPE_READ, (uint32_t)(addr),0)

typedef enum
{
    TYPE_EXIT,
    TYPE_READ,
    TYPE_WRITE,

    TYPE_NUM_OF
} reg_io_type;

typedef struct
{
    uint32_t type;
    uint32_t address;
    uint32_t value;

} reg_io_t;

uint32_t ru_emulation_client_send(reg_io_type io_type, uint32_t address, uint32_t value);
#endif
void *memset_write32 (void *block, int c, uint32_t size);

/******************************************************************************
 * Find by name utilities
 ******************************************************************************/

#define FIELD_SET_(reg,mask,shift,fld) (((reg) & ~(mask)) | ((fld) << (shift)))
#define FIELD_GET_(reg,mask,shift) (((reg) & (mask)) >> (shift))


/**
 * \brief Get block info by name
 *
 * This function fetches a block info structure by the name of the block.  The
 * block name should be in the exact format of the register specification.  If
 * no block is found matching the given name NULL is returned.
 *
 * \param name Block to look up
 *
 * \return
 * Block info structure if found, NULL if block does not exist
 */
extern
const ru_block_rec *ru_block_name_find(const char *name);


/**
 * \brief Get register info by name
 *
 * This function fetches a register info structure given the info of the block
 * and the register name.  Names must exactly match the register specification,
 * the register name should not have the block name prepended.  If no matching
 * register is found for the supplied block NULL is returned.
 *
 * \param blk_inst Block instance
 * \param block Block to search
 * \param name Register name to lookup
 *
 * \return
 * Register info structure if found, NULL if register does not exist
 */
extern
const ru_reg_rec *ru_reg_name_find(ru_block_inst blk_inst,
                                   const ru_block_rec *block,
                                   const char *name);


/**
 * \brief Get field info by name
 *
 * This function fetches a field info structure given the info of the register
 * and the name of the field.  The name should exactly match the register
 * specification, without block or register name prepended.  If no matching
 * field is found for the supplied register NULL is returned.
 *
 * \param reg Register to search
 * \param name Field name to lookup
 *
 * \return
 * Field info structure if found, NULL if field does not exist
 */
extern
const ru_field_rec *ru_field_name_find(const ru_reg_rec *reg, const char *name);


/******************************************************************************
 * Find by register address
 ******************************************************************************/

/**
 * \brief Get a block info by address
 *
 * This function fetches a block record by address.  The address may be anywhere
 * within the register space of the block.  If found the block record is
 * assigned to the supplied container.
 *
 * \param[in] addr Address to lookup
 * \param[out] blk_inst Container for found instance number
 * \param[out] block Container for found block
 *
 * \return
 * 0 if successful, -1 if failed
 */
extern
int ru_block_addr_find(uint32_t addr,
                       ru_block_inst *blk_inst,
                       const ru_block_rec **block);


/**
 * \brief Get block and register info by address
 *
 * This function fetches a block and register record by address.  The found
 * records are assigned to the supplied container.
 *
 * \param[in] addr Address to lookup
 * \param[out] blk_inst Container for found instance number
 * \param[out] block Container for found block
 * \param[out] reg Container for foudn register
 *
 * \return
 * 0 if successful, -1 if failed
 */
extern
int ru_reg_addr_find(uint32_t addr,
                     ru_block_inst *blk_inst,
                     const ru_block_rec **block,
                     const ru_reg_rec **reg);


/******************************************************************************
 * Print parsed register value
 ******************************************************************************/

/**
 * \brief Parse and print a register from records
 *
 * This function parses a register into its individual fields and prints the
 * formatted output to stdout.  The function is supplied the block and register
 * info records for the register to print.
 *
 * \param blk_inst Block instance number
 * \param block Block for regsiter
 * \param reg Register to print
 * \param value Register value
 *
 * \return
 * 0 if successful, -1 if failed
 */
extern
int ru_reg_print(ru_block_inst blk_inst,
                 const ru_block_rec *block,
                 const ru_reg_rec *reg,
                 uint32_t value);


/**
 * \brief Parse and print a register from address
 *
 * This function parses a register into its individual fields and prints the
 * formatted output to stdout.  The function is supplied the address of the
 * register to print.
 *
 * \param addr Register address
 * \param value Register value
 *
 * \return
 * 0 if successful, -1 if failed
 */
extern
int ru_reg_addr_print(uint32_t addr, uint32_t value);


/**
 * \brief Parse and print a register from names
 *
 * This function parses a register into its individual fields and prints the
 * formatted output to stdout.  The function is supplied the names of the block
 * and register to print.
 *
 * \param blk_inst Block instance number
 * \param bname Name of block
 * \param rname Name of register
 * \param value Register value
 *
 * \return
 * 0 if successful, -1 if failed
 */
extern
int ru_reg_name_print(ru_block_inst blk_inst,
                      const char *bname,
                      const char *rname,
                      uint32_t value);


/******************************************************************************
 * Register logging and debugging functions
 ******************************************************************************/

/**
 * \brief Set up a register logging rule
 *
 * This function add a watch rule to a single register access.  Rules are used
 * to conditionally enable logging of register accesses. Registers may be logged
 * on read, write, or both and may be conditionally logged by register contents
 * using the mask, value, and operation.  To create an always true rule set the
 * mask and value to 0 and set the operation to ru_op_equal.
 *
 * \param blk_inst Block instance number
 * \param blk Block record
 * \param reg Register to watch
 * \param type Execute on read/write/both
 * \param op Comparison operator
 * \param mask Mask to apply before comparison
 * \param val Value to compare register against
 *
 * \return
 * None
 */
extern
void ru_reg_rule_set(ru_block_inst blk_inst,
                     const ru_block_rec *blk,
                     const ru_reg_rec *reg,
                     ru_log_type type,
                     ru_log_op op,
                     uint32_t mask,
                     uint32_t val);
#if RU_RULE_POOL_SIZE > 0
#define RU_REG_RULE_SET(i,b,r,t,o,m,v) ru_reg_rule_set(i,&RU_BLK(b),        \
                                                       &RU_REG(b,r),t,o,m,v)
#define RU_REG_RULE_ALWAYS_TRUE(i,b,r, t) ru_reg_rule_set(i,&RU_BLK(b),     \
                                                          &RU_REG(b,r),     \
                                                          t,ru_op_equal,0,0)
#define RU_REG_RULE_ALWAYS_FALSE(i,b,r, t) ru_reg_rule_set(i,&RU_BLK(b),    \
                                                           &RU_REG(b,r),    \
                                                           t,ru_op_not_equal,\
                                                           0,0)
#else
#define RU_REG_RULE_SET(i,b,r,t,o,m,v)
#define RU_REG_RULE_ALWAYS_TRUE(i,b,r, t)
#define RU_REG_RULE_ALWAYS_FALSE(i,b,r, t)
#endif


/**
 * \brief Set up register rules for every register in a block
 *
 * This function applies a register logging rule to every register in a block.
 *
 * \param blk_inst Block instance number
 * \param blk Block to watch
 * \param type Execute on read/write/both
 * \param op Comparison operator
 * \param mask Mask to apply before comparison
 * \param val Value to compare register against
 *
 * \return
 * None
 */
extern
void ru_block_rule_set(ru_block_inst blk_inst,
                       const ru_block_rec *blk,
                       ru_log_type type,
                       ru_log_op op,
                       uint32_t mask,
                       uint32_t val);
#if RU_RULE_POOL_SIZE > 0
#define RU_BLOCK_RULE_SET(i,b,t,o,m,v) ru_block_rule_set(i,&RU_BLK(b),t,o,m,v)
#define RU_BLOCK_RULE_ALWAYS_TRUE(i,b,t) ru_block_rule_set(i,&RU_BLK(b),t,  \
                                                         ru_op_equal,0,0)
#define RU_BLOCK_RULE_ALWAYS_FALSE(i,b,t) ru_block_rule_set(i,&RU_BLK(b),t, \
                                                          ru_op_not_equal,0,0)
#else
#define RU_BLOCK_RULE_SET(i,b,t,o,m,v)
#define RU_BLOCK_RULE_ALWAYS_TRUE(i,b,t)
#define RU_BLOCK_RULE_ALWAYS_FALSE(i,b,t)
#endif


/**
 * \brief Log a register transaction
 *
 * This function saves a register to the register transaction log.  It can be
 * used for logging when RU_EXTERNAL_REGISTER_ADDRESSING is
 * enabled and register access is handled by an 
 * external function. 
 *
 * \param type This transaction is a read or write
 * \param blk_inst Block instance number
 * \param blk Block for register
 * \param reg Register to log
 * \param val Value written to or read from register
 *
 * \return
 * Number of transactions printed
 */
extern
void ru_log_register(ru_log_type type,
                     ru_block_inst blk_inst,
                     const ru_block_rec *blk,
                     const ru_reg_rec *reg,
                     uint32_t val);
#if RU_RULE_POOL_SIZE > 0
#define RU_LOG_REGISTER(t,i,b,r,v) ru_log_register(t,i,&RU_BLK(b),          \
                                                   &RU_REG(b,r),v)
#else
#define RU_LOG_REGISTER(t,i,b,r,v)
#endif


/**
 * \brief Print the full contents of the register log
 *
 * This function dumps all logged register transactions to the CLI.  It returns
 * the number of transactions logged.
 *
 * \return
 * Number of transactions printed
 */
extern
uint32_t ru_log_dump(void);


/**
 * \brief Enable/disable immediate logging
 *
 * This function enables or disables immediate logging.  When enabled all
 * registers that are marked for logging are immediately printed.  If disabled
 * the values are stored to the log to be printed later with ru_log_dump.
 *
 * \return
 * None
 */
extern
void ru_immediate_log_enable(int enable);


/**
 * \brief Enable or disable field bounds checking
 *
 * This function enables or disables the field bounds check down when a register
 * field is set.  The checking will still be compiled in but simply bypassed.
 * To fully bypass bounds checking disable RU_FIELD_CHECK_ENABLE.
 *
 * \param enable Enable or disable bounds checking
 *
 * \return
 * None
 */
extern
void ru_field_bounds_check_enable(int enable);


/******************************************************************************
 * Register access functions
 ******************************************************************************/

/**
 * \brief Write a register
 *
 * This function writes a value to a register.  If debugging is enabled the
 * written value may be logged.
 *
 * \param blk_inst Block instance number
 * \param blk Register block record
 * \param reg Register record
 * \param val Value to write
 *
 * \return
 * 0 if successful
 */
extern
int ru_reg_write(ru_block_inst blk_inst,
                 const ru_block_rec *blk,
                 const ru_reg_rec *reg,
                 uint32_t val);
#if !RU_FUNCTION_REG_ACCESS
#define RU_REG_WRITE(i,b,r,v) WRITE_32(RU_BLK(b).addr[i]+RU_REG_OFFSET(b,r),v)
#else
#define RU_REG_WRITE(i,b,r,v) ru_reg_write(i,&RU_BLK(b),&RU_REG(b,r),v)
#endif


/**
 * \brief Read a register
 *
 * This function reads a register.  If debugging is enabled the value read may
 * be logged.
 *
 * \param blk_inst Block instance number
 * \param blk Register block record
 * \param reg Register record
 * \param val Read back register value
 *
 * \return
 * 0 if successful
 */
extern
uint32_t ru_reg_read(ru_block_inst blk_inst,
                const ru_block_rec *blk,
                const ru_reg_rec *reg);
#if !RU_FUNCTION_REG_ACCESS
#define RU_REG_READ(i,b,r,reg) READ_32(RU_BLK(b).addr[i]+RU_REG_OFFSET(b,r),reg)
#else
#define RU_REG_READ(i,b,r) ru_reg_read(i,&RU_BLK(b),&RU_REG(b,r))
#endif


/**
 * \brief Write a RAM mapped register
 *
 * This function writes a instace of a RAM mapped register (i.e. a RAM that is
 * an indexable table of identical registers).  If debugging is enabled the
 * value written value may be logged.
 *
 * \param blk_inst Block instance number
 * \param ram_addr RAM address, aligned to the size of the register
 * \param blk Register block record
 * \param reg Register record
 * \param val Value to write
 *
 * \return
 * 0 if successful
 */
extern
int ru_reg_ram_write(ru_block_inst blk_inst,
                     ru_ram_addr ram_addr,
                     const ru_block_rec *blk,
                     const ru_reg_rec *reg,
                     uint32_t val);
#if !RU_FUNCTION_REG_ACCESS
#define RU_REG_RAM_WRITE(i,a,b,r,v) \
    WRITE_32(RU_BLK(b).addr[i] + RU_REG_OFFSET(b,r) + (RU_REG(b,r).offset*(a)),v)
#else
#define RU_REG_RAM_WRITE(i,a,b,r,v) \
    ru_reg_ram_write(i,a,&RU_BLK(b),&RU_REG(b,r),v)
#endif


/**
 * \brief Read a RAM mapped register
 *
 * This function reads a instace of a RAM mapped register (i.e. a RAM that is an
 * indexable table of identical registers).  If debugging is enabled the value
 * read may be logged.
 *
 * \param blk_inst Block instance number
 * \param ram_addr RAM address, aligned to the size of the register
 * \param blk Register block record
 * \param reg Register record
 * \param val Read back register value
 *
 * \return
 * 0 if successful
 */
extern
uint32_t ru_reg_ram_read(ru_block_inst blk_inst,
                    ru_ram_addr ram_addr,
                    const ru_block_rec *blk,
                    const ru_reg_rec *reg);
#if !RU_FUNCTION_REG_ACCESS
#define RU_REG_RAM_READ(i,a,b,r, var) \
    READ_32(RU_BLK(b).addr[i] + RU_REG_OFFSET(b,r) + (RU_REG(b,r).offset*(a)), var)
#else
#define RU_REG_RAM_READ(i,a,b,r) \
    ru_reg_ram_read(i,a,&RU_BLK(b),&RU_REG(b,r))
#endif


/**
 * \brief Set a register field
 *
 * This function sets a field in a register.  The value if applied to the
 * supplied old register value and returned.  If debbuging is enabled an
 * assertion will be raised if the field is out of range with range being
 * determined by the width of the field. If the field is out of range it will
 * not be set and the original old value will be returned.
 *
 * \param blk_inst Block instance number
 * \param blk Register block record
 * \param reg Register record
 * \param fld Register field record
 * \param reg_val Old register value to write in to
 * \param fld_val Field value to write
 *
 * \return
 * val with new field value applied
 */
extern
uint32_t ru_field_set(ru_block_inst blk_inst,
                      const ru_block_rec *blk,
                      const ru_reg_rec *reg,
                      const ru_field_rec *fld,
                      uint32_t reg_val,
                      uint32_t fld_val);
#if !RU_FUNCTION_REG_ACCESS
#define RU_FIELD_SET(i,b,r,f,rv,fv)                                         \
    (((rv) & ~RU_FLD_MASK(b,r,f)) | (fv << RU_FLD_SHIFT(b,r,f)))
#else
#define RU_FIELD_SET(i,b,r,f,rv,fv) ru_field_set(i,&RU_BLK(b),              \
                                                 &RU_REG(b,r),              \
                                                 &RU_FLD(b,r,f),            \
                                                 rv, fv)
#endif


/**
 * \brief Get a register field
 *
 * This function gets a field in a register.  The value returned is justified to
 * bit [0]. This function does not support any debug features.
 *
 * \param blk_inst Block instance number
 * \param blk Register block record
 * \param reg Register record
 * \param fld Register field record
 * \param reg_val Old register value to read from
 *
 * \return
 * Bit [0] justified field value
 */
extern
uint32_t ru_field_get(ru_block_inst blk_inst,
                      const ru_block_rec *blk,
                      const ru_reg_rec *reg,
                      const ru_field_rec *fld,
                      uint32_t reg_val);
#if !RU_FUNCTION_REG_ACCESS
#define RU_FIELD_GET(i,b,r,f,rv)                                            \
    (((rv) & RU_FLD_MASK(b,r,f)) >> RU_FLD_SHIFT(b,r,f))
#else
#define RU_FIELD_GET(i,b,r,f,rv) ru_field_get(i,&RU_BLK(b),                 \
                                              &RU_REG(b,r),                 \
                                              &RU_FLD(b,r,f), rv)
#endif


/**
 * \brief Write a field in a register
 *
 * This function performs a read modify write action on a given register setting
 * the desired field in the process. The same logging and field checking applies
 * as ru_reg_write, ru_reg_read, and ru_field_set.  This function should be used
 * when it is desirable to only write a single field in a register.
 *
 * \param blk_inst Block instance number
 * \param blk Register block record
 * \param reg Register record
 * \param fld Register field record
 * \param fld_val Field value to write
 *
 * \return
 * None
 */
extern
void ru_field_write(ru_block_inst blk_inst,
                    const ru_block_rec *blk,
                    const ru_reg_rec *reg,
                    const ru_field_rec *fld,
                    uint32_t fld_val);
#if !RU_FUNCTION_REG_ACCESS
#define RU_FIELD_WRITE(i,b,r,f,fv)                                          \
    do                                                                      \
    {                                                                       \
        uint32_t rv;                                                        \
        rv = RU_REG_READ(i,b,r);                                            \
        rv = RU_FIELD_SET(i,b,r,f,rv,fv);                                   \
        RU_REG_WRITE(i,b,r,v);                                              \
    } while (0)
#else
#define RU_FIELD_WRITE(i,b,r,f,fv) ru_field_write(i,&RU_BLK(b)              \
                                                  &RU_REG(b,r),             \
                                                  &RU_FLD(b,r,f), fv)
#endif


/**
 * \brief Read a field from a register
 *
 * This function reads a register and returns a given field.  The same logging
 * as ru_reg_read applies.  This function should be called when on a single
 * field is required from a register.
 *
 * \param blk_inst Block instance number
 * \param blk Register block record
 * \param reg Register record
 * \param fld Register field record
 *
 * \return
 * Bit [0] justified field value
 */
extern
uint32_t ru_field_read(ru_block_inst blk_inst,
                       const ru_block_rec *blk,
                       const ru_reg_rec *reg,
                       const ru_field_rec *fld);
#if !RU_FUNCTION_REG_ACCESS
#define RU_FIELD_READ(i,b,r,f) RU_FIELD_GET(i,b,r,f,RU_REG_READ(i,b,r))
#else
#define RU_FIELD_READ(i,b,r,f) ru_field_read(i,&RU_BLK(b),                  \
                                             &RU_REG(b,r),                  \
                                             &RU_FLD(b,r,f))
#endif


#endif /* End of file _RU_H_ */
