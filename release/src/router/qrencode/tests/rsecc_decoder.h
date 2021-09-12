#ifndef RSECC_DECODER_H
#define RSECC_DECODER_H

void RSECC_decoder_init();
int RSECC_decoder_checkSyndrome(int dl, unsigned char *data, int el, unsigned char *ecc);

#endif /* RSECC_DECODER_H */
