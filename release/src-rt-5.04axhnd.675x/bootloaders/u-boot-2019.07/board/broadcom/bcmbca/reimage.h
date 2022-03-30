
#include <cli.h>

#define NVRAM_MAGIC "nVrAmDaT"
#define MASQ_SPLIT_A "ImageFollows - e62b6965f01ec7e98fcde8ee1de"
#define MASQ_SPLIT_B "57bfa9bc4e2e4eb5cee95406d5337ed9355da"

#define NP_TOTAL 5

struct allocs_dhd {
	unsigned char dhd_size[3];
	unsigned char reserved;
};

struct nvram_s {
	unsigned int version;
	char bootline[256];
	char boardid[16];
	unsigned int ulMainTpNum;
	unsigned int ulPsiSize;
	unsigned int ulNumMacAddrs;
	unsigned char ucaBaseMacAddr[6];
	char pad;
	char backupPsi;	 /**< if 0x01, allocate space for a backup PSI */
	unsigned int ulCheckSumV4;
	char dummy1[292];
	unsigned int ulNandPartOfsKb[NP_TOTAL];
	unsigned int ulNandPartSizeKb[NP_TOTAL];
	char szVoiceBoardId[16];
	unsigned int afeId[2];
	unsigned short opticRxPwrReading;	// optical initial rx power reading
	unsigned short opticRxPwrOffset;	// optical rx power offset
	unsigned short opticTxPwrReading;	// optical initial tx power reading
	unsigned char ucUnused2[58];
	unsigned char ucFlashBlkSize;
	unsigned char ucAuxFSPercent;
	unsigned char ucUnused3[2];
	unsigned int ulBoardStuffOption;	// board options. bit0-3 is for DECT
	unsigned int reserved2;
	unsigned int ulMemoryConfig;
	struct partition_info {
		/*
		   2MSB represent the
		   00 = MB
		   01 = GB
		   10 = reserved
		   11 = reserved
		   14LSB represent multiple of 2MSB
		 */

		unsigned short size;
	} part_info[4];
	struct allocs_dhd alloc_dhd;

	/* Add any new non-secure related elements here */
	unsigned int ulFeatures;	// feature bitmask
	char chUnused[268];	/* Adjust chUnused such that everything above + chUnused[] + ulCheckSum = 1k */
	unsigned int ulCheckSum;
};

struct recovery_chunks {
	int flashpage;		// page in flash
	int size;		// number of bytes before fill
	int type;		// 0x0= fill with 0xff, 0x1 = fill with 0x00, 0x7fffffff = END
};

static struct reimager {
	// layout items
	int loader_blocks;	// number of erase blocks in loader IMAGE
	int split_image_start;	// start of image partition in blocks
	int split_image_end;	// end of image partition in blocks
	int payload_start;	// actual block number where payload starts in flash
	int payload_blocks;	// this will be after decompression if stored compressed
	int erase_first_start;	// the old linux image erase block start
	int erase_first_blocks;	// old linux image erase block size 
	int burn_first_start;	// the old linux image, but after loader (erase block number)
	int burn_first_blocks;	// number of erase blocks usable once erase_first is erased
	int burn_remaining_start;	// starting erase block where we can write once we have erased all
	int burn_remaining_blocks;	// number of blocks available where we can write once we have erased
	int erase_last_start;	// starting EB to be erased after everything is written
	int erase_last_blocks;	// number of EBs to erase last
	// bookkeeping
	int blocksizeK;
	char *remaining_payload;	// buffer for EBs that are part of payload and didn't fit in burn_first
	int remaining_payload_len;	// length of payload held in remaining_payload[]
	char *loader_payload;	// in-memory copy of loader
	char *preserved_data;	// buffer for preserved data (filename\0hexlength\0data)...
	int preserved_data_len;	// number of bytes of buffered preserved data
	int preserved_data_max_len;	// allocated max preserved data
	struct recovery_chunks *recovery_chunks_list;
	char *recovery_data_buf;
	int recovery_data_len;
	struct nvram_s nvram;	// in-memory copy of nvram
	struct ubi_device *ubi;
	int pure_payload_image;	// if using pureubi, select payload image
	int pure_payload_volume_index;	// if using pureubi payload volume index

};

int do_reimage_auto(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[]);

void reimage_splice_env(struct reimager *r, char *more_env, int more_env_size);
void reimage_env_append(struct reimager *r);
