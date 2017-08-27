#include <linux/kernel.h>
#include <board.h>
#include "../../bbsi/bbsi.h"
#include "../../bbsi/bbsiExecSeq.h"

void board_mocaInit(int mocaChipNum);

void board_mocaInit(int mocaChipNum)
{
	int i;
	PBP_MOCA_INFO pMocaInfo;

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

