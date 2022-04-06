#include "fuzz.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
	return fuzz_run_server(Data, Size, 1, 1);
}

