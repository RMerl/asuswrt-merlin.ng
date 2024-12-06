/*
* <:copyright-BRCM:2021:DUAL/GPL:standard
*
*    Copyright (c) 2021 Broadcom
*    All Rights Reserved
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
*
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
*
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
*
* :>
*/


#ifndef _RDPA_BUFFER_H_
#define _RDPA_BUFFER_H_

/** \defgroup rdpa_buffer RDPA Buffer
 *
 * @{
 */


/** Buffer Type */
typedef enum
{
    rdpa_buffer_skb_and_data, /**< Buffer contains the data, followed by a shared info and a SKB header */
    rdpa_buffer_fkb_and_data, /**< Buffer contains a FKB header, followed by the data */
    rdpa_buffer_skb /**< Buffer contains SKB only */
} rdpa_buffer_t;

/** Supported buffers sizes in Ring. According to the buffer types of the dedicated ring, each buffer should have enough
 * room to contain both the data, and the FKB or SKB header and SKB shared info. Typically, 2K size will be used for FKB
 * or 4K for SKB and shared info. Larger size will be used for Jumbo frames, 1K and 512 bytes for optimizations. */
typedef enum
{
    rdpa_buffer_512 = 512, /**< 512 bytes */
    rdpa_buffer_2K = 2048, /**< 2K bytes */
    rdpa_buffer_4K = 4096, /**< 4K bytes */
    rdpa_buffer_8K = 8192, /**< 8K bytes, typically for Jumbo frames */
    rdpa_buffer_10K = 10836, /**< 10K bytes, typically for Jumbo frames */
    rdpa_buffer_16K = 16384 /**< 16K bytes, typically for Jumbo frames */
} rdpa_buffer_size;

#define RDPA_BUFFER_MAX_NUM_OF_RINGS  8

/** RDPA Buffer initial configuration.
 * This is underlying structure of rdpa_buffer_init_cfg aggregate.
 */
typedef struct
{
    rdpa_buffer_t buf_type;     /**< Buffer type */
    uint8_t prio;               /**< Buffer for low or high priority traffic (0 for low, 1 for high) */
    rdpa_buffer_size buf_size;  /**< Max buffer size */
    uint32_t alloc_ring_size;   /**< Size of the pre-allocated buffers ring */
    uint32_t reset_tail_offset; /**< Where to reset from the end of the data. Can serve SKBs and shared data. Optional*/
    uint32_t reset_tail_size;   /**< How much to reset from the end of the data. */
} rdpa_buffer_init_cfg_t;


/** Allocate a single buffer from the buffer alloc ring of given rdpa_buffer object
 *
 * \param[in]   rdpa_buffer_ring    Buffer ring object from which to allocate a buffer. If NULL, buffer is allocated
 *                                  from unreserved common memory pool.
 * \param[out]  buffer_ptr          Pointer to allocated byffer (storing a virtual address).
 * \return 0=OK or int error code\n
*/
int rdpa_buffer_alloc(bdmf_object_handle rdpa_buffer_ring, void **buffer_ptr);

/** Free the buffer given the virtual address of the buffer.
 * \param[in]   buffer_ptr          Pointer to buffer (storing a virtual address).
*/
void rdpa_buffer_free(void *buffer_ptr);

/** Allocate number of buffers from the buffer alloc ring of given rdpa_buffer object
 *
 * \param[in]   rdpa_buffer_ring    Buffer ring object from which to allocate the buffer. If NULL, buffers are allocated
 *                                  from unreserved memory pool.
 * \param[in]   num_of_bufs         Number of buffers to allocate.
 * \param[out]  buffer_head         Pointer to array of allocated buffers (storing a virtual addresses). Array should be
 *                                  pre-allocated in advance.
 *
 * \param[out]  buffer_tail         Pointer to last buffer in the array of the allocated buffers.
 * \return 0=OK or int error code\n
*/
int rdpa_buffer_alloc_mult(bdmf_object_handle rdpa_buffer_ring, uint8_t num_of_bufs, void **buffer_head,
    void **buffer_tail);

/** Free the array of buffers, given the virtual address of the buffer.
 * \param[in]   num_of_bufs         Number of buffers to allocate.
 * \param[out]  buffer_head         Pointer to array of allocated buffers (storing a virtual addresses). Array should be
 *                                  pre-allocated in advance.
*/
void rdpa_buffer_free_mult(uint8_t num_of_bufs, void **buffer_head);

/** @} end of rdpa_buffer Doxygen group */

#endif /* _RDPA_BUFFER_H_ */
