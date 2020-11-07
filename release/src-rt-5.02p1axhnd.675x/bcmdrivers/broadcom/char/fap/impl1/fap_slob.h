#ifndef _FAP_SLOB_H_INCLUDED_
#define _FAP_SLOB_H_INCLUDED_

#ifdef DYN_MEM_TEST_APP
#include "test.h"
#endif

#include "fap_dynmem.h"

void
fapslob_init(void);

fapDm_BlockId
fapslob_alloc ( uint32 fapIdx, uint32 size, fapDm_RegionOrder order);

void
fapslob_free ( uint32 fapIdx, fapDm_BlockId blockId );


void
fapslob_shrink (    uint32         fapIdx,
                    fapDm_BlockId  blockId,
                    uint32         newSize );


//internal defs:  these need to be exported to fap_memory.h

typedef struct fapSlob_BlockHeader {
    struct fapSlob_BlockHeader    * next;
    struct fapSlob_BlockHeader    * prev;
    struct fapSlob_BlockHeader    * nextFree;
    struct fapSlob_BlockHeader    * prevFree;
    uint32                          offset;       // note: if we assume block <= 64k, this could be uint16
    uint16                          size;
    uint8                           isFree;
    uint8                           sludge;       // number of bytes of sludge at end of allocation
} fapSlob_BlockHeader;


typedef struct {
    fapDm_RegionIdx  regionIdx;         
    char            name[6];
    uint32          freeBytes;
    uint32          size;
    uint32          maxBlocks;
    char *          hostAddr;

    fapSlob_BlockHeader  *  firstBlock;
    fapSlob_BlockHeader  *  nextFreeBlock;
    fapSlob_BlockHeader  *  unusedBlocks;
    fapSlob_BlockHeader  *  blockArray;
} fapslob_Region;




/* INLINE FUNCTIONS: */



#endif
