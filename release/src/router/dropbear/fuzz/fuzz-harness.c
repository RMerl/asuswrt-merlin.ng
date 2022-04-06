#include "includes.h"
#include "buffer.h"
#include "dbutil.h"

extern int LLVMFuzzerTestOneInput(const unsigned char *data, size_t size);

int main(int argc, char ** argv) {
    int i;
    buffer *input = buf_new(100000);
    int quiet = 0;

    for (i = 1; i < argc; i++) {
#if DEBUG_TRACE
        if (strcmp(argv[i], "-v") == 0) {
            debug_trace++;
            fprintf(stderr, "debug level -> %d\n", debug_trace);
        }
#endif
        if (strcmp(argv[i], "-q") == 0) {
            printf("Running quiet\n");
            quiet = 1;
        }
    }

    int old_fuzz_wrapfds = 0;
    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            /* ignore arguments */
            continue;
        }

        char* fn = argv[i];
        buf_setlen(input, 0);
        buf_readfile(input, fn);
        buf_setpos(input, 0);

		/* Run twice to catch problems with statefulness */
        fuzz.wrapfds = old_fuzz_wrapfds;
        if (!quiet) {
            printf("Running %s once \n", fn);
        }
        LLVMFuzzerTestOneInput(input->data, input->len);
        if (!quiet) {
            printf("Running %s twice \n", fn);
        }
        LLVMFuzzerTestOneInput(input->data, input->len);
        if (!quiet) {
            printf("Done %s\n", fn);
        }

        /* Disable wrapfd so it won't interfere with buf_readfile() above */
        old_fuzz_wrapfds = fuzz.wrapfds;
        fuzz.wrapfds = 0;
    }

    printf("Finished\n");

    return 0;
}

// Just to let it link
size_t LLVMFuzzerMutate(uint8_t *UNUSED(Data), size_t UNUSED(Size), size_t UNUSED(MaxSize)) {
    printf("standalone fuzzer harness shouldn't call LLVMFuzzerMutate");
    abort();
    return 0;
}
