#include <linux/delay.h>
#include "bbsi.h"
#include "bbsiExecSeq.h"



void bbsiExecuteCommandSequence(int dev, BpCmdElem *prSeq)
{
    if (!prSeq)
    {
        printk("%s: Error , passed a null sequence to execute", __FUNCTION__);
        return;
    }
    
    while (prSeq[0].command != CMD_END)
    {
        switch(prSeq[0].command)
        {
            case CMD_READ:   kerSysBcmSpiSlaveReadReg32(dev, prSeq[0].addr);
                             break;
    
            case CMD_WRITE:  kerSysBcmSpiSlaveWriteReg32(dev, prSeq[0].addr, prSeq[0].value);
                             break;
    
            case CMD_WAIT:   mdelay(prSeq[0].addr); /* Sleep in ms */
                             break;     
                             
            case CMD_END:    break;                                    
        }
    
        prSeq++;
    }

}

