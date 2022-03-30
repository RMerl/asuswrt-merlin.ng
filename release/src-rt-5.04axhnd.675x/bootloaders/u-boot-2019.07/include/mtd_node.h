#ifndef _NODE_INFO
#define _NODE_INFO

/*
 * Info we use to search for a flash node in DTB.
 */
struct node_info {
	const char *compat;	/* compatible string */
	int type;		/* mtd flash type */
};
#endif
