/* A mutator/crossover for SSH protocol streams.
   Attempts to mutate each SSH packet individually, keeping
   lengths intact.
   It will prepend a SSH-2.0-dbfuzz\r\n version string.

   Linking this file to a binary will make libfuzzer pick up the custom mutator.

   Care is taken to avoid memory allocation which would otherwise
   slow exec/s substantially */

#include "fuzz.h"
#include "dbutil.h"

size_t LLVMFuzzerMutate(uint8_t *Data, size_t Size, size_t MaxSize);

static const char* FIXED_VERSION = "SSH-2.0-dbfuzz\r\n";
static const char* FIXED_IGNORE_MSG = 
        "\x00\x00\x00\x10\x06\x02\x00\x00\x00\x00\x11\x22\x33\x44\x55\x66";
static const unsigned int FIXED_IGNORE_MSG_LEN = 16;
#define MAX_FUZZ_PACKETS 500
/* XXX This might need tuning */
static const size_t MAX_OUT_SIZE = 50000;

/* Splits packets from an input stream buffer "inp".
The initial SSH version identifier is discarded.
If packets are not recognised it will increment until an uint32 of valid
packet length is found. */

/* out_packets an array of num_out_packets*buffer, each of size RECV_MAX_PACKET_LEN */
static void fuzz_get_packets(buffer *inp, buffer **out_packets, unsigned int *num_out_packets) {
    /* Skip any existing banner. Format is
          SSH-protoversion-softwareversion SP comments CR LF
    so we look for SSH-2. then a subsequent LF */
    unsigned char* version = memmem(inp->data, inp->len, "SSH-2.", strlen("SSH-2."));
    if (version) {
        buf_incrpos(inp, version - inp->data);
        unsigned char* newline = memchr(&inp->data[inp->pos], '\n', inp->len - inp->pos);
        if (newline) {
            buf_incrpos(inp, newline - &inp->data[inp->pos]+1);
        } else {
            /* Give up on any version string */
            buf_setpos(inp, 0);
        }
    }

    const unsigned int max_out_packets = *num_out_packets;
    *num_out_packets = 0;
    while (1) {
        if (inp->pos + 4 > inp->len) {
            /* End of input */
            break;
        }

        if (*num_out_packets >= max_out_packets) {
            /* End of output */
            break;
        }

        /* Read packet */
        unsigned int packet_len = buf_getint(inp);
        if (packet_len > RECV_MAX_PACKET_LEN-4) {
            /* Bad length, try skipping a single byte */
            buf_decrpos(inp, 3);
            continue;
        }
        packet_len = MIN(packet_len, inp->len - inp->pos);

        /* Check the packet length makes sense */
        if (packet_len >= MIN_PACKET_LEN-4) {
            /* Copy to output buffer. We're reusing buffers */
            buffer* new_packet = out_packets[*num_out_packets];
            (*num_out_packets)++;
            buf_setlen(new_packet, 0);
            // packet_len doesn't include itself
            buf_putint(new_packet, packet_len);
            buf_putbytes(new_packet, buf_getptr(inp, packet_len), packet_len);
        }
        buf_incrpos(inp, packet_len);
    }
}

/* Mutate a packet buffer in-place.
Returns DROPBEAR_FAILURE if it's too short */
static int buf_llvm_mutate(buffer *buf) {
    int ret;
    /* Position it after packet_length and padding_length */
    const unsigned int offset = 5;
    buf_setpos(buf, 0);
    buf_incrwritepos(buf, offset);
    size_t max_size = buf->size - buf->pos;
    size_t new_size = LLVMFuzzerMutate(buf_getwriteptr(buf, max_size),
        buf->len - buf->pos, max_size);
    size_t new_total = new_size + 1 + 4;
    // Round down to a block size
    new_total = new_total - (new_total % dropbear_nocipher.blocksize);

    if (new_total >= 16) {
        buf_setlen(buf, new_total);
        // Fix up the length fields
        buf_setpos(buf, 0);
        // packet_length doesn't include itself, does include padding_length byte
        buf_putint(buf, new_size+1);
        // always just put minimum padding length = 4
        buf_putbyte(buf, 4);
        ret = DROPBEAR_SUCCESS;
    } else {
        // instead put a fake packet
        buf_setlen(buf, 0);
        buf_putbytes(buf, FIXED_IGNORE_MSG, FIXED_IGNORE_MSG_LEN);
        ret = DROPBEAR_FAILURE;
    }
    return ret;
}


/* Persistent buffers to avoid constant allocations */
static buffer *oup;
static buffer *alloc_packetA;
static buffer *alloc_packetB;
static buffer* packets1[MAX_FUZZ_PACKETS];
static buffer* packets2[MAX_FUZZ_PACKETS];

/* Allocate buffers once at startup.
   'constructor' here so it runs before dbmalloc's interceptor */
static void alloc_static_buffers() __attribute__((constructor));
static void alloc_static_buffers() {

    int i;
    oup = buf_new(MAX_OUT_SIZE);
    alloc_packetA = buf_new(RECV_MAX_PACKET_LEN);
    alloc_packetB = buf_new(RECV_MAX_PACKET_LEN);

    for (i = 0; i < MAX_FUZZ_PACKETS; i++) {
        packets1[i] = buf_new(RECV_MAX_PACKET_LEN);
    }
    for (i = 0; i < MAX_FUZZ_PACKETS; i++) {
        packets2[i] = buf_new(RECV_MAX_PACKET_LEN);
    }
}

size_t LLVMFuzzerCustomMutator(uint8_t *Data, size_t Size,
              size_t MaxSize, unsigned int Seed) {

    buf_setlen(alloc_packetA, 0);
    buf_setlen(alloc_packetB, 0);
    buf_setlen(oup, 0);

    unsigned int i;
    size_t ret_len;
    unsigned short randstate[3] = {0,0,0};
    memcpy(randstate, &Seed, sizeof(Seed));

    // printhex("mutator input", Data, Size);

    /* 0.1% chance straight llvm mutate */
    // if (nrand48(randstate) % 1000 == 0) {
    //     ret_len = LLVMFuzzerMutate(Data, Size, MaxSize);
    //     // printhex("mutator straight llvm", Data, ret_len);
    //     return ret_len;
    // }

    buffer inp_buf = {.data = Data, .size = Size, .len = Size, .pos = 0};
    buffer *inp = &inp_buf;

    /* Parse packets */
    unsigned int num_packets = MAX_FUZZ_PACKETS;
    buffer **packets = packets1;
    fuzz_get_packets(inp, packets, &num_packets);

    if (num_packets == 0) {
        // Make up a packet, writing direct to the buffer
        inp->size = MaxSize;
        buf_setlen(inp, 0);
        buf_putbytes(inp, FIXED_VERSION, strlen(FIXED_VERSION));
        buf_putbytes(inp, FIXED_IGNORE_MSG, FIXED_IGNORE_MSG_LEN);
        // printhex("mutator no input", Data, inp->len);
        return inp->len;
    }

    /* Start output */
    /* Put a new banner to output */
    buf_putbytes(oup, FIXED_VERSION, strlen(FIXED_VERSION));

    /* Iterate output */
    for (i = 0; i < num_packets+1; i++) {
        // These are pointers to output
        buffer *out_packetA = NULL, *out_packetB = NULL;
        buf_setlen(alloc_packetA, 0);
        buf_setlen(alloc_packetB, 0);

        /* 2% chance each */
        const int optA = nrand48(randstate) % 50;
        if (optA == 0) {
            /* Copy another */
            unsigned int other = nrand48(randstate) % num_packets;
            out_packetA = packets[other];
            // printf("copy another %d / %d len %u\n", other, num_packets, out_packetA->len);
        }
        if (optA == 1) {
            /* Mutate another */
            unsigned int other = nrand48(randstate) % num_packets;
            out_packetA = alloc_packetA;
            buffer *from = packets[other];
            buf_putbytes(out_packetA, from->data, from->len);
            if (buf_llvm_mutate(out_packetA) == DROPBEAR_FAILURE) {
                out_packetA = NULL;
            }
            // printf("mutate another %d / %d len %u -> %u\n", other, num_packets, from->len, out_packetA->len);
        }

        if (i < num_packets) {
            int optB = nrand48(randstate) % 100;
            if (optB == 1) {
                /* small chance of drop */
                /* Drop it */
                //printf("%d drop\n", i);
            } else { 
                /* Odds of modification are proportional to packet position.
                First packet has 20% chance, last has 100% chance */
                int optC = nrand48(randstate) % 1000;
                int mutate_cutoff = MAX(200, (1000 * (i+1) / num_packets));
                if (optC < mutate_cutoff) {
                    // // printf("%d mutate\n", i);
                    out_packetB = alloc_packetB;
                    buffer *from = packets[i];
                    buf_putbytes(out_packetB, from->data, from->len);
                    if (buf_llvm_mutate(out_packetB) == DROPBEAR_FAILURE) {
                        out_packetB = from;
                    }
                    // printf("mutate self %d / %d len %u -> %u\n", i, num_packets, from->len, out_packetB->len);
                } else {
                    /* Copy as-is */
                    out_packetB = packets[i];
                    // printf("%d as-is len %u\n", i, out_packetB->len);
                } 
            }
        }

        if (out_packetA && oup->len + out_packetA->len <= oup->size) {
            buf_putbytes(oup, out_packetA->data, out_packetA->len);
        }
        if (out_packetB && oup->len + out_packetB->len <= oup->size) {
            buf_putbytes(oup, out_packetB->data, out_packetB->len);
        }
    }

    ret_len = MIN(MaxSize, oup->len);
    memcpy(Data, oup->data, ret_len);
    // printhex("mutator done", Data, ret_len);
    return ret_len;
}

size_t LLVMFuzzerCustomCrossOver(const uint8_t *Data1, size_t Size1,
                                            const uint8_t *Data2, size_t Size2,
                                            uint8_t *Out, size_t MaxOutSize,
                                            unsigned int Seed) {
    unsigned short randstate[3] = {0,0,0};
    memcpy(randstate, &Seed, sizeof(Seed));

    unsigned int i;
    buffer inp_buf1 = {.data = (void*)Data1, .size = Size1, .len = Size1, .pos = 0};
    buffer *inp1 = &inp_buf1;
    buffer inp_buf2 = {.data = (void*)Data2, .size = Size2, .len = Size2, .pos = 0};
    buffer *inp2 = &inp_buf2;

    unsigned int num_packets1 = MAX_FUZZ_PACKETS;
    fuzz_get_packets(inp1, packets1, &num_packets1);
    unsigned int num_packets2 = MAX_FUZZ_PACKETS;
    fuzz_get_packets(inp2, packets2, &num_packets2);

    // fprintf(stderr, "input 1 %u packets\n", num_packets1);
    // printhex("crossover input1", Data1, Size1);
    // fprintf(stderr, "input 2 %u packets\n", num_packets2);
    // printhex("crossover input2", Data2, Size2);

    buf_setlen(oup, 0);
    /* Put a new banner to output */
    buf_putbytes(oup, FIXED_VERSION, strlen(FIXED_VERSION));

    if (num_packets1 == 0 && num_packets2 == 0) {
        buf_putbytes(oup, FIXED_IGNORE_MSG, FIXED_IGNORE_MSG_LEN);
    } else {
        unsigned int min_out = MIN(num_packets1, num_packets2);
        unsigned int max_out = num_packets1 + num_packets2;
        unsigned int num_out = min_out + nrand48(randstate) % (max_out-min_out+1);

        for (i = 0; i < num_out; i++) {
            unsigned int choose = nrand48(randstate) % (num_packets1 + num_packets2);
            buffer *p = NULL;
            if (choose < num_packets1) {
                p = packets1[choose];
            } else {
                p = packets2[choose-num_packets1];
            }
            if (oup->len + p->len <= oup->size) {
                buf_putbytes(oup, p->data, p->len);
            }
        }
    }

    size_t ret_len = MIN(MaxOutSize, oup->len);
    memcpy(Out, oup->data, ret_len);
    // printhex("crossover output", Out, ret_len);
    return ret_len;
}

