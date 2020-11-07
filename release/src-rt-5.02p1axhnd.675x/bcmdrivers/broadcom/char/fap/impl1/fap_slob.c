

/* TBD: add multithreaded support */

#ifdef DYN_MEM_TEST_APP
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "fap_dynmem_host.h"
#include "fap_slob.h"
#include <linux/version.h>

#define MIN_FREE_SIZE   32


#define regions(fapIdx) regionsDef[fapIdx]
fapslob_Region regionsDef[NUM_FAPS][FAP_DM_REGION_MAX] = {{{0}}};
fapDm_RegionIdx regionOrders[FAP_DM_REGION_ORDER_MAX][FAP_DM_REGION_MAX+1];

#else /* DYN_MEM_TEST_APP */

#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/version.h>

#include "fap_hw.h"
#include "fap_dynmem_host.h"
#include "fap_slob.h"
#include "fap4ke_memory.h"
#include <linux/jiffies.h>

#if 0
#define ASSERT(x)      do { if (unlikely(!(x))) { printk ("ERROR %s.%d   %s failed\n\n", __func__, __LINE__, #x); } } while (0)
#define DEBUG(x,...)   
#define ERROR(x,...)   printk("ERR [%s.%d]: " x "\n", __func__, __LINE__, ##__VA_ARGS__)
#else
#include <linux/bcm_log.h>
#define ASSERT(x)      do { if (unlikely(!(x))) { printk ("ERROR %s.%d   %s failed\n\n", __func__, __LINE__, #x); } } while (0)
#define DEBUG(x,...)   BCM_LOG_DEBUG( BCM_LOG_ID_FAP, "[%s.%d]: " x, __func__, __LINE__, ##__VA_ARGS__)
#define ERROR(x,...)   BCM_LOG_ERROR( BCM_LOG_ID_FAP, "[%s.%d]: " x, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)
static DEFINE_SPINLOCK(fapDm_lock_g);
#else
static spinlock_t fapDm_lock_g = SPIN_LOCK_UNLOCKED;
#endif

#define FAP_DM_LOCK() spin_lock_bh( &fapDm_lock_g )
#define FAP_DM_UNLOCK() spin_unlock_bh( &fapDm_lock_g )

#define malloc(size) kmalloc(size, GFP_KERNEL)

#define MIN_FREE_SIZE 32

#define regions(fapIdx) pHostFapSdram(fapIdx)->dynMem.regions
#define regionOrders    pHostFapSdram(0)->dynMem.regionOrders


#define DSP_MAX_BLOCKS  ((FAP_DM_DSP_SIZE / 32)+1)
#define PSM_MAX_BLOCKS  ((FAP_DM_PSM_SIZE / 32)+1)
#define QSM_MAX_BLOCKS  ((FAP_DM_QSM_SIZE / 32)+1)
#define HP_MAX_BLOCKS   (((FAP_DM_HP_SIZE) / 32)+1)

#endif /* DYN_MEM_TEST_APP */


#ifndef _arrayIdxOf
#define _arrayIdxOf(array, ptr)  ((uint16)( ((char *)ptr - (char *)array) / sizeof(array[0]) ))
#endif

#if 0
#define CHECK_BLOCKID(fapIdx, blockId)    do { \
                                                fapDm_BlockInfo info;  \
                                                info.id = blockId; \
                                                if(blockId != FAP_DM_INVALID_BLOCK_ID) { \
                                                    ASSERT(info.regionIdx < FAP_DM_REGION_MAX); \
                                                    ASSERT(info.blockIdx < regions(fapIdx)[info.regionIdx].maxBlocks); \
                                                    ASSERT(info.offset < regions(fapIdx)[info.regionIdx].size); \
                                                } \
                                            } while(0)
#else
#define CHECK_BLOCKID(fapIdx, blockId)
#endif

//#define SCRIBBLE_PROTECTION

void pushUnused( fapslob_Region * region, fapSlob_BlockHeader * bh)
{
    if (region->unusedBlocks == NULL)
    {
        bh->next = bh;
        bh->prev = bh;
        region->unusedBlocks = bh;
    }
    else
    {
        bh->prev = region->unusedBlocks->prev;
        bh->next = region->unusedBlocks;

        bh->prev->next = bh;
        bh->next->prev = bh;
    }
}

fapSlob_BlockHeader * popUnused(fapslob_Region * region)
{
    fapSlob_BlockHeader * bh;
    
    bh = region->unusedBlocks;
    
    if (bh == NULL)
        return NULL;
    
    if (bh->next == bh)
    {
        region->unusedBlocks = NULL;
        return bh;
    }
    else
    {
        region->unusedBlocks = bh->next;
        region->unusedBlocks->prev = bh->prev;
        region->unusedBlocks->prev->next = region->unusedBlocks;
        
        return bh;
    }
}


void
fapslob_initRegion(     uint32              fapIdx,
                        fapDm_RegionIdx     regionIdx,                        
                        const char *        name,
                        uint32              size,           // in bytes
                        uint32              maxBlocks )
{
    fapslob_Region * region;
    int i;
    fapSlob_BlockHeader * bh;
        
    ASSERT(regionIdx >= FAP_DM_REGION_DSP && regionIdx < FAP_DM_REGION_MAX);
    ASSERT(size < (1 << 20));
    ASSERT(maxBlocks > 0);

    DEBUG("Initializing Region %s for FAP%d: size = %d, maxBlocks=%d", name, fapIdx, size, maxBlocks);

    region = &regions(fapIdx)[regionIdx];
    //memset(region, 0, sizeof(region));  should already be 0...
    region->regionIdx = regionIdx;

    strcpy(region->name, name);
    region->size = size;
    region->freeBytes = size;
    region->maxBlocks = maxBlocks;
    region->hostAddr = fapDm_getHostBaseAddrFromRegion(fapIdx, regionIdx);

    DEBUG("region(%s)->hostAddr=%p", name, region->hostAddr);

    // Temporary code for debugging...
    if (region->hostAddr)
    {
        memset(region->hostAddr, 0, size);
    }
    
    region->blockArray = kmalloc(sizeof(fapSlob_BlockHeader) * maxBlocks, GFP_ATOMIC);
    if (region->blockArray == NULL)
    {
        DEBUG("malloc failed");
        ASSERT(FALSE);
        return;
    }
    for (i = 0; i < maxBlocks-1; i++)
    {
        region->blockArray[i].next = &(region->blockArray[ ((i+1)!=(maxBlocks-1)) ? (i+1) : 0]);
        region->blockArray[i].next->prev = &region->blockArray[i];
    }

    region->unusedBlocks = maxBlocks > 1 ? &region->blockArray[0] : NULL;

    bh = &region->blockArray[maxBlocks-1];

    bh->isFree = TRUE;
    bh->offset = 0;
    bh->size = size;
    bh->next = bh;
    bh->prev = bh;
    bh->nextFree = bh;
    bh->prevFree = bh;

    region->firstBlock = bh;
    region->nextFreeBlock = bh;    
}

#define CHECK_BH(region, bh)   do { \
                                    ASSERT(region->regionIdx < FAP_DM_REGION_MAX); \
                                    if (!(bh >= region->blockArray)) \
                                    { \
                                           ERROR("[%s.%d]: (bh(%p) >= region(%s)->blockArray(%p)) failed\n", __func__, __LINE__, bh, region->name, region->blockArray); \
                                    } \
                                    ASSERT(bh < region->blockArray + region->maxBlocks); \
                               } while(0)
                                       

/* NOTE: this is a 'remote' slob, where the manager is not assumed to have access to the
   memory it is managing...  Therefore, the memory block inforamtion must be kept seperate. */

fapSlob_BlockHeader *fapslob_regionAlloc ( fapslob_Region * region, uint32 size )
{
    
    fapSlob_BlockHeader *bh;
    fapSlob_BlockHeader *sbh;        //starting block header;
    fapSlob_BlockHeader *nbh;        //new block header;

    size = (size + 3) & (~3);
    ASSERT( region->nextFreeBlock ? (region->nextFreeBlock->nextFree->prevFree == region->nextFreeBlock):(region->freeBytes==0));
    ASSERT( region->nextFreeBlock ? (region->nextFreeBlock->nextFree->prevFree == region->nextFreeBlock):(region->freeBytes==0));
    
    sbh = region->nextFreeBlock;
    bh = sbh;
    
    if (bh == NULL)
    {
        return NULL;
    }

    CHECK_BH(region, bh);
    ASSERT(bh->isFree);
    
    while (bh->size < size)
    {
        bh = bh->nextFree;
        if (bh == sbh)
        {
            return NULL;
        }
        CHECK_BH(region, bh);
        ASSERT(bh->isFree);
    }

    // found a match
    if (bh->size - size < MIN_FREE_SIZE)
    {
        if (bh->nextFree == bh)
        {
            // last free block...
            region->nextFreeBlock = NULL;
        }
        else
        {
            region->nextFreeBlock = bh->nextFree;
            bh->prevFree->nextFree = bh->nextFree;
            bh->nextFree->prevFree = bh->prevFree;
            {
                ASSERT(region->nextFreeBlock->nextFree->prevFree == region->nextFreeBlock);
                ASSERT(region->nextFreeBlock->prevFree->nextFree == region->nextFreeBlock);
            }
        }
        region->freeBytes -= bh->size;
        bh->isFree = 0;
        bh->sludge = bh->size - size;
        ASSERT(region->nextFreeBlock?region->nextFreeBlock->nextFree->prevFree == region->nextFreeBlock:1);
        ASSERT(region->nextFreeBlock?region->nextFreeBlock->nextFree->prevFree == region->nextFreeBlock:1);
        return bh;
    }
    else
    {
        // only using part of the free block.  Using the front part of the free block!
        // getting a new unused block to put data in:
        nbh = popUnused(region);
        CHECK_BH(region, nbh);
        
        if (nbh == NULL)
        {
            ASSERT(FALSE);
            return NULL;
        }
        
        region->freeBytes -= size;
        
        nbh->offset = bh->offset;
        nbh->size = size;
        nbh->isFree = 0;
        nbh->sludge = 0;
        nbh->next = bh;
        nbh->prev = bh->prev;
        nbh->prev->next = nbh;
        bh->prev = nbh;

        bh->size -= size;
        bh->offset += size;

        if (region->firstBlock == bh)
            region->firstBlock = nbh;

        ASSERT(region->nextFreeBlock?region->nextFreeBlock->nextFree->prevFree == region->nextFreeBlock:1);
        ASSERT(region->nextFreeBlock?region->nextFreeBlock->nextFree->prevFree == region->nextFreeBlock:1);

        return nbh;
    }
    
}


void
fapslob_regionFree(fapslob_Region * region, uint16 idx)
{
    bool isPrevFree;
    bool isNextFree;
    fapSlob_BlockHeader *bh;

    ASSERT(idx < region->size);
    ASSERT(region->blockArray);

    bh = &region->blockArray[idx];
    
    ASSERT(bh);
    ASSERT(bh->isFree == 0);

    region->freeBytes += bh->size;
    isPrevFree = (bh->offset > bh->prev->offset && bh->prev->isFree);
    isNextFree = (bh->offset < bh->next->offset && bh->next->isFree);

    if (isPrevFree && isNextFree)
    {
        fapSlob_BlockHeader *prev;
        fapSlob_BlockHeader *next;

        DEBUG("prev and next free");
        
        prev = bh->prev;
        next = bh->next;

        prev->nextFree = next->nextFree;
        next->nextFree->prevFree = prev;

        prev->next = next->next;
        prev->next->prev = prev;
        
        prev->size += bh->size + next->size;

        ASSERT(prev->nextFree != bh && prev->prevFree != next);
        ASSERT(prev->next != bh && prev->prev != next);       

        pushUnused(region, bh);

        if (region->nextFreeBlock == next || region->nextFreeBlock == prev)
        {
            region->nextFreeBlock = prev->nextFree;
        }
        ASSERT(region->nextFreeBlock?region->nextFreeBlock->nextFree->prevFree == region->nextFreeBlock:region->freeBytes==0);
        ASSERT(region->nextFreeBlock?region->nextFreeBlock->nextFree->prevFree == region->nextFreeBlock:region->freeBytes==0);
        pushUnused(region, next);
        
    }
    else if (isPrevFree)
    {
        fapSlob_BlockHeader *prev;
        
        DEBUG("prev free");
        
        prev = bh->prev;

        prev->next = bh->next;
        prev->next->prev = prev;
        prev->size += bh->size;
        
        ASSERT(region->nextFreeBlock?region->nextFreeBlock->nextFree->prevFree == region->nextFreeBlock:1);
        ASSERT(region->nextFreeBlock?region->nextFreeBlock->nextFree->prevFree == region->nextFreeBlock:1);

        ASSERT(prev->nextFree != bh);
        ASSERT(prev->prevFree != bh);
        ASSERT(prev->next != bh);
        ASSERT(prev->prev != bh);

        pushUnused(region, bh);
    }
    else if (isNextFree)
    {
        fapSlob_BlockHeader *next;
        
        DEBUG("next free");

        bh->isFree = 1;
        
        next = bh->next;

        bh->next = next->next;
        bh->next->prev = bh;
        if (bh->prev == next)
        {
            bh->prev = bh;
        }

        if (next->nextFree == next)
        {
            ASSERT(next->prevFree == next);
            ASSERT(region->nextFreeBlock == next);           
            bh->nextFree=bh;
            bh->prevFree=bh;
        }
        else
        {
            bh->nextFree = next->nextFree;
            bh->prevFree = next->prevFree;
            bh->nextFree->prevFree = bh;
            bh->prevFree->nextFree = bh;
        }
        ASSERT(bh->nextFree != next);
        ASSERT(bh->prevFree != next);

        bh->size += next->size;

        if (region->nextFreeBlock == next)
        {
            region->nextFreeBlock = bh->nextFree;
        }
        ASSERT(region->nextFreeBlock?region->nextFreeBlock->nextFree->prevFree == region->nextFreeBlock:region->freeBytes==0);
        ASSERT(region->nextFreeBlock?region->nextFreeBlock->nextFree->prevFree == region->nextFreeBlock:region->freeBytes==0);
        pushUnused(region, next);
    }
    else
    {
        // TBD: reclaim sludge from previous block if applicable...
        fapSlob_BlockHeader * ibh;  // itterator...

        DEBUG("next and prev not free");

        bh->isFree = 1;

        for (ibh = bh->next; ibh != bh; ibh = ibh->next)
        {
            if (ibh->isFree)
                break;
        }

        if (ibh == bh)
        {
            // this is the only free block
            region->nextFreeBlock = bh;
            bh->nextFree = bh;
            bh->prevFree = bh;
            
            ASSERT(region->nextFreeBlock->nextFree->prevFree == region->nextFreeBlock);
            ASSERT(region->nextFreeBlock->prevFree->nextFree == region->nextFreeBlock);
            ASSERT(bh->size == region->freeBytes);
        }
        else
        {
            bh->nextFree = ibh;
            bh->prevFree = ibh->prevFree;

            bh->nextFree->prevFree = bh;
            bh->prevFree->nextFree = bh;
        }        
    }


    
    ASSERT(region->nextFreeBlock?region->nextFreeBlock->nextFree->prevFree == region->nextFreeBlock:region->freeBytes==0);
    ASSERT(region->nextFreeBlock?region->nextFreeBlock->nextFree->prevFree == region->nextFreeBlock:region->freeBytes==0);

}


void
fapslob_init(void)
{
    int fapIdx;
    //fap4keDspram_global_t temp;

    
    FAP_DM_LOCK();

#if !defined DYN_MEM_TEST_APP
    {
        uint32 flowId;
        for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
        {
            for (flowId = 0; flowId < FAP4KE_PKT_MAX_FLOWS; flowId++)
            {
                fapDm_setFlowBlockId(fapIdx, flowId, FAP_DM_INVALID_BLOCK_ID);
                fapDm_setCmdListBlockId(fapIdx, flowId, FAP_DM_INVALID_BLOCK_ID);                
            }
        }
    }
#endif
  

    memset(regionOrders, 0, sizeof(regionOrders));

#if !defined(SCRIBBLE_PROTECTION)
    regionOrders[FAP_DM_REGION_ORDER_DSP_PSM_QSM][0] = FAP_DM_REGION_DSP;
    regionOrders[FAP_DM_REGION_ORDER_DSP_PSM_QSM][1] = FAP_DM_REGION_PSM;
    regionOrders[FAP_DM_REGION_ORDER_DSP_PSM_QSM][2] = FAP_DM_REGION_QSM;
    regionOrders[FAP_DM_REGION_ORDER_DSP_PSM_QSM][3] = FAP_DM_REGION_MAX;

    regionOrders[FAP_DM_REGION_ORDER_DSP_PSM_QSM_HP][0] = FAP_DM_REGION_DSP;
    regionOrders[FAP_DM_REGION_ORDER_DSP_PSM_QSM_HP][1] = FAP_DM_REGION_PSM;
    regionOrders[FAP_DM_REGION_ORDER_DSP_PSM_QSM_HP][2] = FAP_DM_REGION_QSM;
    regionOrders[FAP_DM_REGION_ORDER_DSP_PSM_QSM_HP][3] = FAP_DM_REGION_HP;
    regionOrders[FAP_DM_REGION_ORDER_DSP_PSM_QSM_HP][4] = FAP_DM_REGION_MAX;
    
#else
    // with scribble protection enabled: make sure that first few flows
    // are allocated to PSM where scribbles can be checked.
    regionOrders[FAP_DM_REGION_ORDER_DSP_PSM_QSM][0] = FAP_DM_REGION_PSM;
    regionOrders[FAP_DM_REGION_ORDER_DSP_PSM_QSM][1] = FAP_DM_REGION_DSP;
    regionOrders[FAP_DM_REGION_ORDER_DSP_PSM_QSM][2] = FAP_DM_REGION_QSM;
    regionOrders[FAP_DM_REGION_ORDER_DSP_PSM_QSM][3] = FAP_DM_REGION_MAX;
    
    regionOrders[FAP_DM_REGION_ORDER_DSP_PSM_QSM_HP][0] = FAP_DM_REGION_PSM;
    regionOrders[FAP_DM_REGION_ORDER_DSP_PSM_QSM_HP][1] = FAP_DM_REGION_DSP;
    regionOrders[FAP_DM_REGION_ORDER_DSP_PSM_QSM_HP][2] = FAP_DM_REGION_QSM;
    regionOrders[FAP_DM_REGION_ORDER_DSP_PSM_QSM_HP][3] = FAP_DM_REGION_HP;
    regionOrders[FAP_DM_REGION_ORDER_DSP_PSM_QSM_HP][4] = FAP_DM_REGION_MAX;
#endif
    
    regionOrders[FAP_DM_REGION_ORDER_QSM_PSM][0] = FAP_DM_REGION_QSM;
    regionOrders[FAP_DM_REGION_ORDER_QSM_PSM][1] = FAP_DM_REGION_PSM;
    regionOrders[FAP_DM_REGION_ORDER_QSM_PSM][2] = FAP_DM_REGION_MAX;
    
    regionOrders[FAP_DM_REGION_ORDER_QSM_PSM_HP][0] = FAP_DM_REGION_QSM;
    regionOrders[FAP_DM_REGION_ORDER_QSM_PSM_HP][1] = FAP_DM_REGION_PSM;
    regionOrders[FAP_DM_REGION_ORDER_QSM_PSM_HP][2] = FAP_DM_REGION_HP;
    regionOrders[FAP_DM_REGION_ORDER_QSM_PSM_HP][3] = FAP_DM_REGION_MAX;
    
    regionOrders[FAP_DM_REGION_ORDER_QSM][0] = FAP_DM_REGION_QSM;
    regionOrders[FAP_DM_REGION_ORDER_QSM][1] = FAP_DM_REGION_MAX;
    
    
    for (fapIdx = 0; fapIdx < NUM_FAPS; fapIdx++)
    {   
        fapslob_initRegion( fapIdx, FAP_DM_REGION_DSP, "DSP", FAP_DM_DSP_SIZE, DSP_MAX_BLOCKS );
        fapslob_initRegion( fapIdx, FAP_DM_REGION_PSM, "PSM", FAP_DM_PSM_SIZE, PSM_MAX_BLOCKS );
        fapslob_initRegion( fapIdx, FAP_DM_REGION_QSM, "QSM", FAP_DM_QSM_SIZE, QSM_MAX_BLOCKS );
        fapslob_initRegion( fapIdx, FAP_DM_REGION_HP, "HP", FAP_DM_HP_SIZE, HP_MAX_BLOCKS );        
    }
    
    FAP_DM_UNLOCK();
}

inline fapDm_BlockId
fapslob_alloc ( uint32 fapIdx, uint32 size, fapDm_RegionOrder order)
{
    uint32 allocSize = (size + 3) & (~0x3);
    uint32 orderIdx;
    fapslob_Region * region;
    fapSlob_BlockHeader *bh;
    fapDm_BlockInfo blockInfo;

    
    FAP_DM_LOCK();

    DEBUG("fapIdx=%d, size=%d, order=%d", fapIdx, size, order);    

    for (orderIdx = 0; regionOrders[order][orderIdx] != FAP_DM_REGION_MAX; orderIdx++)
    {
        region = &regions(fapIdx)[regionOrders[order][orderIdx]];
        if (region->freeBytes < allocSize)
        {
            DEBUG("not enough free space in %s, continuing (%d/%d)", region->name, allocSize, region->freeBytes);
            continue;
        }
#if defined(SCRIBBLE_PROTECTION)
        if (region->hostAddr)
        {
            allocSize += 8;
        }
#endif
        bh = fapslob_regionAlloc(region, allocSize);

        if (bh == NULL)
        {
            DEBUG("could not find a block of size %d (%d) in region %s, continuing", size, allocSize, region->name);
            continue;
        }

#if defined(SCRIBBLE_PROTECTION)
        if (region->hostAddr)
        {
            blockInfo.offset = bh->offset+4;
            blockInfo.regionIdx = region->regionIdx;
            blockInfo.blockIdx = _arrayIdxOf(region->blockArray, bh);
            
            DEBUG("writing scribble to memory location (region <%s>  addrs <%p %p> val <0x%08lx>)", 
                  region->name, 
                  (char *)region->hostAddr + bh->offset,
                  (char *)region->hostAddr + bh->offset + bh->size - 4,
                  (uint32)blockInfo.id);
            
            *((uint32 *)((char *)region->hostAddr + bh->offset)) = (uint32)blockInfo.id;
            *((uint32 *)((char *)region->hostAddr + bh->offset + bh->size - 4)) = (uint32)blockInfo.id;
        }
        else
#endif
        {
            blockInfo.offset = bh->offset;
            blockInfo.regionIdx = region->regionIdx;
            blockInfo.blockIdx = _arrayIdxOf(region->blockArray, bh); 
        }
        DEBUG("Done: offset=%d, regionIdx=%d, blockIdx=%d, region->blockArray[%d]=%p, region->freeBytes=%d",
            blockInfo.offset, 
            blockInfo.regionIdx, 
            blockInfo.blockIdx, 
            _arrayIdxOf(region->blockArray, bh), bh, region->freeBytes);
        FAP_DM_UNLOCK();
        return blockInfo.id;
        
    }

    
    FAP_DM_UNLOCK();
    
    return FAP_DM_INVALID_BLOCK_ID;
}

inline void
fapslob_free ( uint32 fapIdx, fapDm_BlockId blockId )
{
    fapslob_Region * region;
    fapSlob_BlockHeader *bh;
    fapDm_BlockInfo blockInfo;

    FAP_DM_LOCK();

    CHECK_BLOCKID(fapIdx, blockId);
    
    blockInfo.id = blockId;
    region = &regions(fapIdx)[blockInfo.regionIdx];

    ASSERT(blockInfo.regionIdx >= FAP_DM_REGION_DSP && blockInfo.regionIdx < FAP_DM_REGION_MAX);
    ASSERT(blockInfo.regionIdx < region->maxBlocks);
    
    bh = &region->blockArray[blockInfo.blockIdx];

    DEBUG("freeing <%s> <%p>", region->name, bh);

    ASSERT(bh->offset < region->size);
    ASSERT(blockInfo.blockIdx == _arrayIdxOf(region->blockArray, bh));
    ASSERT(region != NULL && bh != NULL);
    ASSERT(bh->isFree == 0);
    
#if defined(SCRIBBLE_PROTECTION)
    if (region->hostAddr != NULL )
    {
        if (   *(uint32 *)(region->hostAddr + blockInfo.offset - 4) != (uint32)blockId
            || *(uint32 *)(region->hostAddr + blockInfo.offset + (bh->size-8)) != (uint32)blockId )
        {
            ERROR("ERROR -- memory scribble detected! (%s) %p=%08lx, %p=%08lx", 
                region->name,
                region->hostAddr + blockInfo.offset - 4,
                *(uint32 *)(region->hostAddr + blockInfo.offset - 4),
                region->hostAddr + blockInfo.offset + (bh->size-8),
                *(uint32 *)(region->hostAddr + blockInfo.offset + (bh->size-8)));
            ASSERT(FALSE);
        }
        else
        {
            DEBUG("blockId<%08lx> Memory Scribble checked out", blockId);
        }
    }
#endif
    fapslob_regionFree(region, blockInfo.blockIdx);

    FAP_DM_UNLOCK();
}

void
fapslob_shrink ( uint32            fapIdx,
                 fapDm_BlockId     blockId,
                 uint32            newSize )
{
    
    FAP_DM_LOCK();
    CHECK_BLOCKID(fapIdx, blockId);
    /* TBD */
    ASSERT(FALSE);
    
    FAP_DM_UNLOCK();
}


#ifdef DYN_MEM_TEST_APP
    #define PRINT   printf
#else
    #define PRINT   printk
#endif



void 
fapslob_dump(uint32 fapIdx, fapDm_RegionIdx regionIdx, fapDm_DebugDumpType dumpType)
{
    //fapDm_RegionIdx             regionIdx;
    fapDm_RegionIdx             startRegionIdx;
    fapDm_RegionIdx             endRegionIdx;
    fapslob_Region            * region;
    fapSlob_BlockHeader       * bh;
    fapSlob_BlockHeader       * ph;
    uint32                      totalBytes;
    uint32                      freeBytes;
    int                         isShort;
    int                         isLong;

    isShort = ((dumpType == FAP_DM_DBG_DUMP_TYPE_SHORT) || (dumpType == FAP_DM_DBG_DUMP_TYPE_LONG));
    isLong = (dumpType == FAP_DM_DBG_DUMP_TYPE_LONG);

    FAP_DM_LOCK();

    if (regionIdx == FAP_DM_REGION_MAX)
    {
        startRegionIdx = FAP_DM_REGION_DSP;
        endRegionIdx = FAP_DM_REGION_MAX-1;
    }
    else
    {
        startRegionIdx = regionIdx;
        endRegionIdx = regionIdx;
    }

    for(regionIdx = startRegionIdx; regionIdx <= endRegionIdx; regionIdx++)
    {
        region = &regions(fapIdx)[regionIdx];

        if (region->regionIdx != regionIdx)
        {
            if(isShort) PRINT("   .\n   .\n");
            if(isShort) PRINT("[===========================================]\n");
            if(isShort) PRINT("[ Region %2d: %s\n", regionIdx, region->name);
            if(isShort) PRINT("[ UNINITIALIZED\n");
            if(isShort) PRINT("[============================]\n");
            continue;
        }

        if(isShort) PRINT("   .\n   .\n");
        if(isShort) PRINT("[===========================================]\n");
        if(isShort) PRINT("[ Region %2d: %s\n", regionIdx, region->name);
        if(isShort) PRINT("[  host address        %p\n", region->hostAddr);
        if(isShort) PRINT("[  size:               %u\n", region->size);
        if(isShort) PRINT("[  free:               %u\n", region->freeBytes);
        if(isShort) PRINT("[  maxBlocks:          %u\n", region->maxBlocks);
        if(isShort) PRINT("[  first block:        %p\n", region->firstBlock);
        if(isShort) PRINT("[  next free block:    %p\n", region->nextFreeBlock);
        if(isShort) PRINT("[=================================\n");

        bh = region->firstBlock;
        totalBytes = 0;
        freeBytes = 0;
        ph = NULL;
        
        do
        {
            if(isLong) PRINT("[ [-- %p ---------------------]\n", bh);
            totalBytes += bh->size;
            if (bh->isFree)
            {
                freeBytes += bh->size;
                if(isLong) PRINT("[    start:    %u\n", bh->offset);
                if(isLong) PRINT("[    size:     %u\n", bh->size);
                if(isLong) PRINT("[    next:     %p, prev     %p\n", bh->next, bh->prev);
                if(isLong) PRINT("[    nextFree: %p, prevFree %p\n", bh->nextFree, bh->prevFree);
                ASSERT( bh->nextFree->prevFree == bh );
                ASSERT( bh->prevFree->nextFree == bh );
            }
            else
            {
                if(isLong) PRINT("[  | start:    %u\n", bh->offset );
                if(isLong) PRINT("[  | size:     %u\n", bh->size );
                if(isLong) PRINT("[  | next:     %p, prev     %p\n", bh->next, bh->prev);
                if(isLong) PRINT("[  | sludge:   %d \n", bh->sludge);

#if defined(SCRIBBLE_PROTECTION)
                if (region->hostAddr)
                {
                    fapDm_BlockId * blockId1;
                    fapDm_BlockId * blockId2;
                    
                    blockId1 = (fapDm_BlockId *)(region->hostAddr + bh->offset);
                    blockId2 = (fapDm_BlockId *)(region->hostAddr + bh->offset + bh->size - 4);
                    if (    (*blockId1 != *blockId2) 
                         || fapDm_getRegionFromBlockId(fapIdx, *blockId1) != regionIdx
                         || fapDm_getOffsetFromBlockId(fapIdx, *blockId1) != bh->offset+4 )
                    {
                        ERROR("Corrupted scribbles: <%p:%08lx / %p:%08lx>", blockId1, (uint32)*blockId1, blockId2, (uint32)*blockId2 );
                        ASSERT(FALSE);
                    }
                    else
                    {
                        PRINT("[  | (scribble check passed)\n ");
                    }
                }
#endif


            }

            ASSERT(bh->next->offset <= bh->offset || bh->next->offset == bh->offset + bh->size);
            if (ph != NULL)
            {
                ASSERT(ph == bh->prev);
                ASSERT(ph->next == bh);
            }
            ph = bh;
            
            bh = bh->next;
        } while (bh != region->firstBlock);  

        if (region->nextFreeBlock == NULL)
            ASSERT(region->freeBytes == 0);
        else
        {
            int freeBytes2 = 0 ;
            bh = region->nextFreeBlock;
            do
            {
                ASSERT(bh->isFree == 1);
                ASSERT(bh->nextFree->prevFree == bh);
                ASSERT(bh->prevFree->nextFree == bh);
                freeBytes2 += bh->size;
                bh = bh->nextFree;
            }
            while (bh != region->nextFreeBlock);
            ASSERT(freeBytes2 == freeBytes);
        }
        
        if(isLong) PRINT("[================================= (%u / %u)\n", totalBytes, freeBytes);
        ASSERT(region->firstBlock->prev->next == region->firstBlock);
        ASSERT(region->firstBlock->next->prev == region->firstBlock);
        ASSERT(region->size == totalBytes);
        ASSERT(region->freeBytes == freeBytes);
    }  
    
    FAP_DM_UNLOCK();
}

void  fapDm_debug(  uint32            fapIdx,
                    fapDm_DebugType   type,
                    uint32            val2 )
{

    
    fapDm_DebugDumpType dumpType;

    dumpType = (type & FAP_DM_DBG_DUMP_REGION_SHORT_MASK) ? FAP_DM_DBG_DUMP_TYPE_SHORT : FAP_DM_DBG_DUMP_TYPE_LONG;
    type &= ~FAP_DM_DBG_DUMP_REGION_SHORT_MASK;
    
    switch(type)
    {
        case FAP_DM_DBG_DUMP_ALL_REGIONS:            
            fapslob_dump( fapIdx, FAP_DM_REGION_DSP, dumpType);
            fapslob_dump( fapIdx, FAP_DM_REGION_PSM, dumpType);
            fapslob_dump( fapIdx, FAP_DM_REGION_QSM, dumpType);
            break;
        case FAP_DM_DBG_DUMP_REGION:
            fapslob_dump( fapIdx, val2, dumpType);
            break;
        default:
            PRINT("ERROR: FAP SLOB does not support this debug method (%d)\n", type);
            break;
    }
}


void fapDm_init( void )
{
    fapslob_init();
}


fapDm_BlockId  fapDm_alloc(  uint32                 fapIdx,
                                           uint32                 size, 
                                           fapDm_RegionOrder      order
                                         )
{
    return fapslob_alloc( fapIdx, size, order);
}


void  fapDm_free(           uint32          fapIdx, 
                                          fapDm_BlockId   blockId)
{
    fapslob_free( fapIdx, blockId );
}


void  fapDm_shrink( uint32          fapIdx,
                                fapDm_BlockId   blockId,
                                uint32          newSize )
{
    fapslob_shrink( fapIdx, blockId, newSize);
}



