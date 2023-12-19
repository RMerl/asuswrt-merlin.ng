#pragma once

#include "defines.h"
#include <tcpedit/tcpedit_types.h>
#include <stdint.h>
#include <stdlib.h>

enum {
    FUZZING_DROP_PACKET,
    FUZZING_REDUCE_SIZE,
    FUZZING_CHANGE_START_ZERO,
    FUZZING_CHANGE_START_RANDOM,
    FUZZING_CHANGE_START_FF,
    FUZZING_CHANGE_MID_ZERO,
    FUZZING_CHANGE_MID_RANDOM, /* the default case */
    FUZZING_CHANGE_MID_FF,
    FUZZING_CHANGE_END_ZERO,
    FUZZING_CHANGE_END_RANDOM,
    FUZZING_CHANGE_END_FF,
    FUZZING_TOTAL_ACTION_NUMBER /* always last */
};

/**
 * init fuzz seed and allocate buffer.
 */
void fuzzing_init(uint32_t _fuzz_seed, uint32_t _fuzz_factor);

/*
 * fuzz packet data.
 * only one out of 8 packets are fuzzed.
 * fuzzed packets get one random modification from the enum above.
 * Returns whether the packet has been modified (1, or 0)
 */
int fuzzing(tcpedit_t *tcpedit, struct pcap_pkthdr *pkthdr, u_char **pktdata);
