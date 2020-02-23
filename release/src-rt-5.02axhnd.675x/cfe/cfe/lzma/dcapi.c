#include "LzmaDecode.h"
#include "bcm63xx_util.h"
#include "lib_malloc.h"

#define LZMA_ORIGSIZE_SIZE 8

int decompressLZMA(unsigned char *in, unsigned insize, unsigned char *out, unsigned outsize);
unsigned int LzmaGetUncompSize(unsigned char *in);

/* return the uncompress data size or 0 if image corrupted */
unsigned int LzmaGetUncompSize(unsigned char *in)
{
    unsigned origsize = 0;

    in += LZMA_PROPERTIES_SIZE;
    if (in[4]==0 && in[5]==0 && in[6]==0 && in[7]==0) { // uncompressed size < 4GB (should be)
        origsize = in[0] | (in[1] << 8) | (in[2] << 16) | (in[3] << 24);
    }
   
    return origsize;
}

// Call ANSI C LZMA decoder to decompress a LZMA block
int decompressLZMA(unsigned char *in, unsigned insize, unsigned char *out, unsigned outsize)
{

    SizeT inProcessed, outProcessed;
    unsigned origsize;
    int ret,len;
    CLzmaDecoderState state;

    ret = LzmaDecodeProperties(&state.Properties, in, LZMA_PROPERTIES_SIZE);
    if (ret != LZMA_RESULT_OK) {
        return ret;
    }
    in += LZMA_PROPERTIES_SIZE;
    len = LzmaGetNumProbs(&state.Properties) * sizeof(CProb);
    state.Probs = (CProb *)KMALLOC(len, 0);
    if (!state.Probs) {
        return 1001;
    }
    memset(state.Probs, 0, len);
    if (in[4]==0 && in[5]==0 && in[6]==0 && in[7]==0) { // uncompressed size < 4GB (should be)
        origsize = in[0] | (in[1] << 8) | (in[2] << 16) | (in[3] << 24);
        in += LZMA_ORIGSIZE_SIZE;
        if (origsize <= outsize) {
              ret = LzmaDecode(&state, in, insize, &inProcessed, out, origsize, &outProcessed);
        }
        else {
            // output buffer too small
            ret = 1000;
        }
    }
    else { // uncompressed size > 4GB, old lzma format or corrupted image, assume old format here
        printf("LZMA: Prossible old LZMA format, trying to decompress..\n");
        LzmaDecode(&state, in, insize, &inProcessed, out, outsize, &outProcessed);
        ret = 0; // It would return an error code as the output buffer size doesn't match. We need to ignore the code.
    }

    KFREE(state.Probs);
    
    return ret;
}

