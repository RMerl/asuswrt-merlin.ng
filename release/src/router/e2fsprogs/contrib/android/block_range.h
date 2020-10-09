#ifndef BLOCK_RANGE_H
# define BLOCK_RANGE_H

# include <sys/types.h>
# include <ext2fs/ext2fs.h>

struct block_range {
	blk64_t start;
	blk64_t end;
	struct block_range *next;
};

struct block_range_list {
	struct block_range *head;
	struct block_range *tail;
};

void add_blocks_to_range(struct block_range_list *list, blk64_t blk_start,
			 blk64_t blk_end);
void delete_block_ranges(struct block_range_list *list);
int write_block_ranges(FILE *f, struct block_range *range, char *sep);

/*
 * Given a non-empty range list, return the next block and remove it from the
 * list.
 */
blk64_t consume_next_block(struct block_range_list *list);

#endif /* !BLOCK_RANGE_H */
