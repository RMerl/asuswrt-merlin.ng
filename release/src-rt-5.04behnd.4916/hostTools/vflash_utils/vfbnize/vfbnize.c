/*
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>

/*
 * Generate vfbio images.
 */

#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <iniparser/iniparser.h>

#define VFBNIZE_NAME                    "vfbnize"
#define VFBNIZE_VERSION                 1
#define VFBIO_PAGE_SIZE                 4096
#define VFBIO_MAX_LUNS                  16    /* Up to 16 dynamic LUNs */
#define VFBIO_MAX_NAME_LEN              16
#define VFBIO_DEFAULT_DEV_ID            1
#define VFBIO_LVMCTRL_LUN_SIZE          32*1024
#define VFBNIZE_DEFAULT_OUTPUT_NAME     "dyn_lun_flash_image.bin"

#define VFBIO_LVM_HEADER_MAGIC      (('V'<<24) | ('L' << 16) | ('V' << 8) | 'M')
#define VFBIO_LVM_HEADER_VERSION    1

#define VF_LUN_FLAG_VALID        0x00000001
#define VF_LUN_FLAG_DYNAMIC      0x00000002
#define VF_LUN_FLAG_READ_ONLY    0x00000010

static const struct option long_options[] = {
    { .name = "device",         .has_arg = 1, .flag = NULL, .val = 'd' },
    { .name = "start",          .has_arg = 1, .flag = NULL, .val = 's' },
    { .name = "output",         .has_arg = 1, .flag = NULL, .val = 'o' },
    { .name = "verbose",        .has_arg = 0, .flag = NULL, .val = 'v' },
    { .name = "help",           .has_arg = 0, .flag = NULL, .val = 'h' },
    { .name = "version",        .has_arg = 0, .flag = NULL, .val = 'V' },
    { NULL, 0, NULL, 0}
};

struct args {
    const char *f_in;
    const char *f_out;
    const char *start_offset_str;
    int out_fd;
    int verbose;
    int lun_dev_id;
    dictionary *dict;
};

struct lun_segment {
    uint32_t start_page; /* Start offset */
    uint32_t num_pages;  /* Size in pages */
};

struct lun {
    int lun_id;             /* LUN id */
    const char *image;      /* Image file name */
    size_t image_size;      /* Image size (bytes) */
    uint32_t num_pages;     /* LUN size (pages) */
    int num_segs;           /* Number of segments in 'seg' array */
    uint32_t flags;         /* LUN flags */
    char name[VFBIO_MAX_NAME_LEN];
    struct lun_segment segs[2]; /* Up to 2 segments per LUN: image data, free space */
};

/* LVMCTRL LUN structure
   - 'struct lvmlun_hdr' (16 bytes)
   - for each LUN
   -    'struct lun_hdr'
   -    lun->num_segs * 'struct lun_segment_hdr'
   - crc32 over entire buffer including header
*/
struct lvmlun_hdr {
    uint32_t magic;         /* VLVM */
    uint32_t version;
    uint32_t id;            /* 1 */
    uint32_t config_size;   /* config size without header and footer */
};

struct lun_hdr {
    uint16_t num_segs;      /* Number of segments */
    uint8_t lun_id;
    uint8_t flags;
    char name[VFBIO_MAX_NAME_LEN];
};

struct lun_segment_hdr {
    uint32_t dev_index;
    uint32_t start_page;
    uint32_t num_pages;
};

static struct args cmd_args;
static uint32_t used_lun_id_mask;
static struct lun luns[VFBIO_MAX_LUNS];
static int num_luns;
static int num_segments;
static uint32_t start_page;

#define PRINT_VERBOSE(fmt, args...) \
    do { \
        if (cmd_args.verbose) \
            printf(fmt, ##args); \
    } while(0)

#define PRINT_ERR_EXIT(fmt, args...) \
    do { \
        fprintf(stderr, fmt, ##args); \
        exit(EXIT_FAILURE); \
    } while(0)

#define PRINT_ERR(fmt, args...) \
    (fprintf(stdout, "\n"), fprintf(stderr, fmt, ##args), -1)

#define ROUND_TO_BLOCK_SIZE(size)  \
    (((size) + VFBIO_PAGE_SIZE - 1) & ~(VFBIO_PAGE_SIZE - 1))

#define SIZE_IN_PAGES(size)  \
    (ROUND_TO_BLOCK_SIZE(size) / VFBIO_PAGE_SIZE)

static int print_help(void)
{
    printf(
        "Usage: " VFBNIZE_NAME " [options] <ini-file>\n\n"
        "Generate VFBIO images. An VFBIO image may contain one or more dynamic LUNs\n"
        "which have to be defined in the input configuration file.\n\n"
        "-o, --output=<file name>     output file name. dyn_lun_flash_image.bin if not set\n"
        "-d, --device=<number>        LUN device index. 1 if not set\n"
        "-s, --start=<number>         Start offset on the device. 0 if not set.\n"
        "                             decimal|0xhex; bytes, KiB, MiB; GiB\n"
        "-v, --verbose                be verbose\n"
        "-h, --help                   print help message\n"
        "-V, --version                print program version\n\n"
        "See configuration file example in vfbnize-ini-example.ini.\n\n");
    return -1;
}

static int parse_opt(int argc, char * const argv[])
{
    while (1) {
        int key;

        key = getopt_long(argc, argv, "o:d:s:vhV", long_options, NULL);
        if (key == -1)
            break;

        switch (key) {
        case 'o':
            cmd_args.f_out = optarg;
            break;

        case 'd':
            cmd_args.lun_dev_id = atoi(optarg);
            break;

        case 's':
            cmd_args.start_offset_str = optarg;
            break;

        case 'v':
            cmd_args.verbose = 1;
            break;

        case 'h':
            print_help();
            exit(EXIT_SUCCESS);

        case 'V':
            printf("%s version %d\n", argv[0], VFBNIZE_VERSION);
            exit(EXIT_SUCCESS);

        default:
            print_help();
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
        PRINT_ERR_EXIT("Input configuration file was not specified (use -h for help)\n");

    if (optind != argc - 1)
        PRINT_ERR_EXIT("More then one configuration file was specified (use -h for help)\n");

    cmd_args.out_fd = open(cmd_args.f_out, O_CREAT | O_TRUNC | O_WRONLY,
                S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (cmd_args.out_fd == -1)
        return PRINT_ERR("cannot open file \"%s\"", cmd_args.f_out);

    cmd_args.f_in = argv[optind];

    return 0;
}

/* Parse size in format: <number>[KiB|MiB|GiB] */
static ssize_t parse_size(const char *size_str)
{
    char *end_ptr = NULL;
    ssize_t size = strtol(size_str, &end_ptr, 0);
    int multiplier = 1;

    if (end_ptr && *end_ptr) {
        if (!strcasecmp(end_ptr, "kib"))
            multiplier = 1024;
        else if (!strcasecmp(end_ptr, "mib"))
            multiplier = 1024 * 1024;
        else if (!strcasecmp(end_ptr, "gib"))
            multiplier = 1024 * 1024 * 1024;
        else
            multiplier = -1;
    }
    return size * multiplier;
}

static int read_section(const char *sname)
{
    struct lun lun = {
        .flags = VF_LUN_FLAG_VALID | VF_LUN_FLAG_DYNAMIC
    };
    char buf[256];
    const char *p;
    uint32_t image_size_in_pages = 0;
    int i;

    PRINT_VERBOSE("Parsing section \"%s\"\n", sname);

    /* Fetch the name of the volume image file */
	sprintf(buf, "%s:image", sname);
    p = iniparser_getstring(cmd_args.dict, buf, NULL);
    if (p) {
        struct stat st;
        lun.image = p;
        if (stat(p, &st))
            return PRINT_ERR("cannot stat \"%s\"\n", p);
        if (st.st_size == 0)
            return PRINT_ERR("file \"%s\" is empty\n", p);
        lun.image_size = st.st_size;
        PRINT_VERBOSE("\timage=\"%s\" size=%ld bytes\n", p, lun.image_size);
        image_size_in_pages = SIZE_IN_PAGES(lun.image_size);
    }

    /* Fetch volume id */
	sprintf(buf, "%s:lun_id", sname);
    lun.lun_id = iniparser_getint(cmd_args.dict, buf, -1);
    if (lun.lun_id == -1) {
        /* LUN id is not specified. Try to autoassign */
        lun.lun_id = ffs(used_lun_id_mask);
        if (!lun.lun_id || lun.lun_id >= VFBIO_MAX_LUNS)
            return PRINT_ERR("Unable to auto-assign lun_id\n");
        --lun.lun_id;
        PRINT_VERBOSE("\tlun_id=%d (auto-assigned)\n", lun.lun_id);
    }
    else {
        if (lun.lun_id >= VFBIO_MAX_LUNS)
            return PRINT_ERR("lun_id %d is out of range 0..%d\n", lun.lun_id, VFBIO_MAX_LUNS - 1);
        PRINT_VERBOSE("\tlun_id=%d\n", lun.lun_id);
    }
    /* Make sure it is not in use yet */
    if ((used_lun_id_mask & (1 << lun.lun_id)) != 0)
        return PRINT_ERR("lun_id %d is not unique\n", lun.lun_id);
    used_lun_id_mask |= (1 << lun.lun_id);

    /* Fetch LUN size */
    sprintf(buf, "%s:lun_size", sname);
    p = iniparser_getstring(cmd_args.dict, buf, NULL);
    if (p) {
        ssize_t size = parse_size(p);
        if (size < 0)
            return PRINT_ERR("lun_size \"%s\" is invalid\n", p);
        lun.num_pages = SIZE_IN_PAGES(size);
        PRINT_VERBOSE("\tlun_size=%lld bytes\n", (long long)lun.num_pages * VFBIO_PAGE_SIZE);
    }

    /* Fetch LUN name */
    sprintf(buf, "%s:lun_name", sname);
    p = iniparser_getstring(cmd_args.dict, buf, NULL);
    if (p == NULL)
        return PRINT_ERR("lun_name must be specified\n");
    if (strlen(p) > VFBIO_MAX_NAME_LEN)
        return PRINT_ERR("lun_name \"%s\" is too long\n", p);
    /* Make sure that LUN name is unique */
    for (i = 0; i < VFBIO_MAX_LUNS; i++) {
        if (luns[i].num_pages && !strncmp(luns[i].name, p, VFBIO_MAX_NAME_LEN))
            return PRINT_ERR("lun_name \"%s\" is not unique\n", p);
    }
    strncpy(lun.name, p, VFBIO_MAX_NAME_LEN);
    PRINT_VERBOSE("\tlun_name=%s\n", lun.name);

    /* Fetch read-only flag, if any */
    sprintf(buf, "%s:read_only", sname);
    p = iniparser_getstring(cmd_args.dict, buf, NULL);
    if (p != NULL) {
        if (!strcasecmp(p, "yes"))
            lun.flags |= VF_LUN_FLAG_READ_ONLY;
        else if (strcasecmp(p, "no"))
            return PRINT_ERR("Expected read_only=yes|no in section \"%s\" is not unique\n", sname);
        PRINT_VERBOSE("\tread_only=%s\n", p);
    }

    /* Final validation */
    if (!image_size_in_pages && !lun.num_pages)
        return PRINT_ERR("Neither image file nor LUN size are specified\n");

    if (lun.num_pages && image_size_in_pages > lun.num_pages) {
        return PRINT_ERR("lun_size %lld is less than image \"%s\" size %lld\n",
            (long long)lun.num_pages * VFBIO_PAGE_SIZE, lun.image, (long long)lun.image_size);
    }

    /* Finalize the assignment */
    if (!lun.num_pages)
        lun.num_pages = image_size_in_pages;
    if (image_size_in_pages) {
        struct lun_segment *seg = &lun.segs[lun.num_segs++];
        seg->start_page = start_page;
        seg->num_pages = image_size_in_pages;
        start_page += image_size_in_pages;
        PRINT_VERBOSE("\tdata segment: start_page=%u num_pages=%u\n", seg->start_page, seg->num_pages);
        ++num_segments;
    }

    /* Free segment assignment (if any) will be done once all LUNs are parsed */
    luns[lun.lun_id] = lun;
    ++num_luns;

    return 0;
}

#define CRC32_POLY    0xEDB88320UL /* CRC-32 Poly */
unsigned int vfbnize_crc32(const unsigned char *buf, unsigned int len)
{
    unsigned int idx, bit, data, crc = 0xFFFFFFFFUL;

    for (idx = 0; idx < len; idx++) {
        for (data = *buf++, bit = 0; bit < 8; bit++, data >>= 1) {
            crc = (crc >> 1) ^ (((crc ^ data) & 1) ? CRC32_POLY : 0);
        }
    }

    return crc;
}

/* Write control LUN */
static uint8_t *vfbnize_make_control_lun(void)
{
    uint8_t *buf, *buf_pos;
    struct lvmlun_hdr *lvmlun_hdr;
    int config_size;
    int total_size;
    int i, j;

    config_size = sizeof(uint32_t) +                    /* Number of LUNs + padding */
        num_luns * sizeof(struct lun_hdr) +             /* LUNs */
        num_segments * sizeof(struct lun_segment_hdr);  /* Segments */
    total_size = sizeof(struct lvmlun_hdr) + config_size + sizeof(uint32_t);
    if (total_size > VFBIO_LVMCTRL_LUN_SIZE) {
        PRINT_ERR("control LUN size %u is too big\n", total_size);
        return NULL;
    }

    buf = malloc(VFBIO_LVMCTRL_LUN_SIZE);
    if (buf == NULL) {
        PRINT_ERR("no memory for control LUN buffer\n");
        return NULL;
    }

    memset(buf, 0xff, VFBIO_LVMCTRL_LUN_SIZE);
    buf_pos = buf;
    lvmlun_hdr = (struct lvmlun_hdr *)buf_pos;
    lvmlun_hdr->magic = VFBIO_LVM_HEADER_MAGIC;
    lvmlun_hdr->version = VFBIO_LVM_HEADER_VERSION;
    lvmlun_hdr->id = 1;
    lvmlun_hdr->config_size = config_size;
    buf_pos += sizeof(*lvmlun_hdr);

    *buf_pos = num_luns;
    buf_pos += sizeof(uint32_t); /* Skip number of LUNs and padding */

    for (i = 0; i < VFBIO_MAX_LUNS; i++) {
        struct lun *lun = &luns[i];
        struct lun_hdr *lun_hdr = (struct lun_hdr *)buf_pos;

        if (!lun->num_pages)
            continue;

        lun_hdr->num_segs = lun->num_segs;
        lun_hdr->lun_id = lun->lun_id;
        lun_hdr->flags = lun->flags;
        memcpy(lun_hdr->name, lun->name, VFBIO_MAX_NAME_LEN);
        buf_pos += sizeof(*lun_hdr);

        for (j = 0; j < lun->num_segs; j++) {
            struct lun_segment_hdr *lun_segment_hdr = (struct lun_segment_hdr *)buf_pos;

            lun_segment_hdr->dev_index = cmd_args.lun_dev_id;
            lun_segment_hdr->start_page = lun->segs[j].start_page;
            lun_segment_hdr->num_pages = lun->segs[j].num_pages;
            buf_pos += sizeof(*lun_segment_hdr);
        }
    }
    *(uint32_t *)buf_pos = vfbnize_crc32(buf, buf_pos - buf);

    return buf;
}

/* Write control LUN */
static int vfbnize_write_control_lun(uint8_t *buf)
{
    int out_fd = cmd_args.out_fd;
    int ret = 0;

    if (write(out_fd, buf, VFBIO_LVMCTRL_LUN_SIZE) != VFBIO_LVMCTRL_LUN_SIZE) {
        PRINT_ERR("cannot write control LUN\n");
        ret = -1;
    }
    return ret;
}

/* Write image for a LUN. Image is padded up to the block size */
static int vfbnize_write_lun(const struct lun *lun)
{
    int out_fd = cmd_args.out_fd;
    uint8_t buf[VFBIO_PAGE_SIZE];
    ssize_t image_size = lun->image_size;
    int fd = -1;
    int ret = -1;

    fd = open(lun->image, O_RDONLY);
    if (fd == -1) {
        PRINT_ERR("cannot open \"%s\"\n", lun->image);
        goto out;
    }

    while(image_size >= VFBIO_PAGE_SIZE) {
        if (read(fd, buf, VFBIO_PAGE_SIZE) != VFBIO_PAGE_SIZE) {
            PRINT_ERR("cannot read %d bytes from the input file \"%s\"\n", VFBIO_PAGE_SIZE, lun->image);
            goto out;
        }
        if (write(out_fd, buf, VFBIO_PAGE_SIZE) != VFBIO_PAGE_SIZE) {
            PRINT_ERR("cannot write %d bytes to the output file\n", VFBIO_PAGE_SIZE);
            goto out;
        }
        image_size -= VFBIO_PAGE_SIZE;
    }
    if (image_size) {
        if (read(fd, buf, image_size) != image_size) {
            PRINT_ERR("cannot read %ld bytes from the input file \"%s\"\n", image_size, lun->image);
            goto out;
        }
        if (write(out_fd, buf, image_size) != image_size) {
            PRINT_ERR("cannot write %ld bytes to the output file\n", image_size);
            goto out;
        }
        /* Write pad 0xFF */
        memset(buf, 0xff, VFBIO_PAGE_SIZE - image_size);
        if (write(out_fd, buf, VFBIO_PAGE_SIZE - image_size) != VFBIO_PAGE_SIZE - image_size) {
            PRINT_ERR("cannot write %ld bytes to the output file\n", VFBIO_PAGE_SIZE - image_size);
            goto out;
        }
    }
    ret = 0;

out:
    if (fd >= 0)
        close(fd);
    return ret;
}

int main(int argc, char * const argv[])
{
    int err, sects, i;
    uint8_t *control_lun_buf = NULL;

    /* Set defaults */
    cmd_args.lun_dev_id = VFBIO_DEFAULT_DEV_ID;
    cmd_args.f_out = VFBNIZE_DEFAULT_OUTPUT_NAME;

    /* Parse options */
    err = parse_opt(argc, argv);
    if (err)
        return -1;

    if (cmd_args.start_offset_str) {
        ssize_t start_offset = parse_size(cmd_args.start_offset_str);
        if (start_offset < 0)
            return PRINT_ERR("start offset %s is invalid\n", cmd_args.start_offset_str);
        start_page = start_offset/VFBIO_PAGE_SIZE + 2*VFBIO_LVMCTRL_LUN_SIZE/VFBIO_PAGE_SIZE;
    }

    cmd_args.dict = iniparser_load(cmd_args.f_in);
    if (!cmd_args.dict) {
        err = -1;
        PRINT_ERR("cannot load the input ini file \"%s\"\n", cmd_args.f_in);
        goto out;
    }

    PRINT_VERBOSE("Loaded the ini-file \"%s\"\n", cmd_args.f_in);

    /* Each section describes one volume */
    sects = iniparser_getnsec(cmd_args.dict);
    if (sects == -1) {
        err = -1;
        PRINT_ERR("ini-file \"%s\" parsing error (iniparser_getnsec)\n", cmd_args.f_in);
        goto out;
    }

    PRINT_VERBOSE("Sections: %d\n", sects);
    if (!sects) {
        err = -1;
        PRINT_ERR("no sections found the ini-file \"%s\"\n", cmd_args.f_in);
        goto out;
    }

    if (sects > VFBIO_MAX_LUNS) {
        err = -1;
        PRINT_ERR("too many sections (%d) in the ini-file \"%s\"\n",
               sects, cmd_args.f_in);
        goto out;
    }

    /* Parse sections */
    for (i = 0; i < sects; i++) {
        const char *sname = iniparser_getsecname(cmd_args.dict, i);

        if (!sname) {
            err = -1;
            PRINT_ERR("ini-file parsing error (iniparser_getsecname)\n");
            goto out;
        }
        err = read_section(sname);
        if (err == -1)
            goto out;
    }

    /* All parsed. Now assign free segments */
    for (i = 0; i < VFBIO_MAX_LUNS; i++) {
        struct lun *lun = &luns[i];

        if (!lun->num_pages)
            continue;

        /* Add free segment if necessary */
        if (lun->num_pages > SIZE_IN_PAGES(lun->image_size)) {
            struct lun_segment *seg = &lun->segs[lun->num_segs++];
            seg->start_page = start_page;
            seg->num_pages = lun->num_pages - SIZE_IN_PAGES(lun->image_size);
            PRINT_VERBOSE("LUN %s: empty segment: start_page=%u num_pages=%u\n",
                lun->name, seg->start_page, seg->num_pages);

            start_page += seg->num_pages;
            ++num_segments;
        }
    }

    /* Create and write 2 control LUNs instances */
    control_lun_buf = vfbnize_make_control_lun();
    if (control_lun_buf == NULL) {
        err = -1;
        goto out;
    }

    err = vfbnize_write_control_lun(control_lun_buf);
    if (err) {
        PRINT_ERR("cannot write control LUN 0\n");
        goto out;
    }
    err = vfbnize_write_control_lun(control_lun_buf);
    if (err) {
        PRINT_ERR("cannot write control LUN 1\n");
        goto out;
    }

    /* Write images */
    for (i = 0; i < VFBIO_MAX_LUNS; i++) {
        struct lun *lun = &luns[i];

        if (!lun->image)
            continue;

        PRINT_VERBOSE("writing LUN %d: \"%s\"\n", lun->lun_id, lun->image);

        err = vfbnize_write_lun(lun);
        if (err) {
            PRINT_ERR("cannot write image \"%s\"\n", lun->image);
            goto out;
        }
    }
    PRINT_VERBOSE("done\n");

out:
    if (cmd_args.dict)
        iniparser_freedict(cmd_args.dict);
    if (control_lun_buf)
        free(control_lun_buf);
    close(cmd_args.out_fd);
    if (err)
        remove(cmd_args.f_out);
    return err;
}
