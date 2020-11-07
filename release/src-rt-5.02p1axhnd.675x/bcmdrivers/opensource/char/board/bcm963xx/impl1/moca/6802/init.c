#include <linux/kernel.h>
#include <board.h>

#include "../../bbsi/bbsi.h"
#include "../../bbsi/bbsiExecSeq.h"
#include "../board_moca.h"

void board_mocaInit(void)
{
    int i, mocaChipNum;
    PBP_MOCA_INFO pMocaInfo;

    mocaChipNum =  board_mocaPreInit();

    for( i = 0; i < mocaChipNum; i++ )
    {
        printk("Initializing the 6802 moca board %d\n", i);
        pMocaInfo = boardGetMocaInfo(i);
        if(pMocaInfo)
        {
            kerSysBcmSpiSlaveInit(i);
    	    bbsiExecuteCommandSequence(i, pMocaInfo->initCmd);
        }
    }
}

