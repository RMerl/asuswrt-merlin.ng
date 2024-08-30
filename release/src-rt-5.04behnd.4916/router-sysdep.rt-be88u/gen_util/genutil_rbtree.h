/*
 * rbtree.h -- generic red-black tree
 *
 * Copyright (c) 2001-2007, NLnet Labs. All rights reserved.
 * 
 * This software is open source.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * Neither the name of the NLNET LABS nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file
 * Red black tree. Implementation taken from NSD 3.0.5, adjusted for use
 * in unbound (memory allocation, logging and so on).
 */

#ifndef UTIL_RBTREE_H_
#define	UTIL_RBTREE_H_

#include <stdlib.h>
#include "number_defs.h"

/**
 * This structure must be the first member of the data structure in
 * the rbtree.  This allows easy casting between an rbnode_t and the
 * user data (poor man's inheritance).
 */
typedef struct rbnode_t rbnode_t;
/**
 * The rbnode_t struct definition.
 */
struct rbnode_t {
	/** parent in rbtree, RBTREE_NULL for root */
	rbnode_t   *parent;
	/** left node (smaller items) */
	rbnode_t   *left;
	/** right node (larger items) */
	rbnode_t   *right;
	/** pointer to sorting key */
	const void *key;
	/** colour of this node */
	UINT8      color;
};


/** statically declared global empty node, not used if user passes in a
 *  different null_node during rbtree_initFlags.
 */
extern	rbnode_t	rbtree_null_node;

/** The nullpointer, points to empty node */
// #define	RBTREE_NULL &rbtree_null_node
#define	RBTREE_NULL (rbtree->null_node)


/** An entire red black tree */
typedef struct rbtree_t rbtree_t;
/** definition for tree struct */
struct rbtree_t {
	/** The root of the red-black tree */
	rbnode_t    *root;
	rbnode_t *null_node;

	/** The number of the nodes in the tree */
	UINT32       count;

	/** 
	 * Key compare function. <0,0,>0 like strcmp. 
	 * Return 0 on two NULL ptrs. 
	 */
	int (*cmp) (const void *, const void *);
	
	/** Flags to modify rbtree behavior.  Currently, only used to indicate
	 *  built in comparison function to use.
	 */
	UINT32 flags;
};

/** 
 * Init a new tree (malloced by caller) with given key compare function. 
 * @param rbtree: uninitialised memory for new tree, returned empty.
 * @param cmpf: compare function (like strcmp) takes pointers to two keys.
 */
void rbtree_init(rbtree_t *rbtree, int (*cmpf)(const void *, const void *));

/** rbtree_init with more fancy options.
 *
 * @param flags: see RBT_FLAGS below.
 * @param null_node: when used inside the MDM, the caller must allocate a
 *         rbnode_t from shared memory and pass it in for use as the null_node.
 *         Otherwise, caller can pass in NULL and the standard internal
 *         rbtree_null_node will be used.
 */
void rbtree_initFlags(rbtree_t *rbtree, UINT32 flags,
                      int (*cmpf)(const void *, const void *),
                      rbnode_t *null_node);

/* use built-in comparison functions instead of one passed in by user */
#define RBT_FLAGS_STRCASECMP   0x01
#define RBT_FLAGS_STRCMP       0x02
#define RBT_FLAGS_UINT32_CMP   0x04
#define RBT_FLAGS_SINT32_CMP   0x08


/** 
 * Insert data into the tree. 
 * @param rbtree: tree to insert to.
 * @param data: element to insert. 
 * @return: data ptr or NULL if key already present. 
 */
rbnode_t *rbtree_insert(rbtree_t *rbtree, rbnode_t *data);

/**
 * Delete element from tree.
 * @param rbtree: tree to delete from.
 * @param key: key of item to delete.
 * @return: node that is now unlinked from the tree. User to delete it. 
 * returns 0 if node not present 
 */
rbnode_t *rbtree_delete(rbtree_t *rbtree, const void *key);

/**
 * Find key in tree. Returns NULL if not found.
 * @param rbtree: tree to find in.
 * @param key: key that must match.
 * @return: node that fits or NULL.
 */
rbnode_t *rbtree_search(rbtree_t *rbtree, const void *key);

/**
 * Find, but match does not have to be exact.
 * @param rbtree: tree to find in.
 * @param key: key to find position of.
 * @param result: set to the exact node if present, otherwise to element that
 *   precedes the position of key in the tree. NULL if no smaller element.
 * @return: true if exact match in result. Else result points to <= element,
 * or NULL if key is smaller than the smallest key. 
 */
int rbtree_find_less_equal(rbtree_t *rbtree, const void *key, 
	rbnode_t **result);

/**
 * Returns first (smallest) node in the tree
 * @param rbtree: tree
 * @return: smallest element or NULL if tree empty.
 */
rbnode_t *rbtree_first(rbtree_t *rbtree);

/**
 * Returns last (largest) node in the tree
 * @param rbtree: tree
 * @return: largest element or NULL if tree empty.
 */
rbnode_t *rbtree_last(rbtree_t *rbtree);

/**
 * Returns next larger node in the tree
 * @param rbtree: tree
 * @return: next larger element or NULL if no larger in tree.
 */
rbnode_t *rbtree_next(rbnode_t *rbtree, rbnode_t *null_node);

/**
 * Returns previous smaller node in the tree
 * @param rbtree: tree
 * @return: previous smaller element or NULL if no previous in tree.
 */
rbnode_t *rbtree_previous(rbnode_t *rbtree, rbnode_t *null_node);

/**
 * Call with node=variable of struct* with rbnode_t as first element.
 * with type is the type of a pointer to that struct. 
 */
#define RBTREE_FOR(node, type, rbtree) \
	for(node=(type)rbtree_first(rbtree); \
		(rbnode_t*)node != RBTREE_NULL; \
		node = (type)rbtree_next((rbnode_t*)node), RBTREE_NULL)

/**
 * Call function for all elements in the redblack tree in post order, such that
 * leaf elements are called before parent elements. So that all
 * elements can be safely free()d.
 * Note that your function must not remove the nodes from the tree.
 * Since that may trigger rebalances of the rbtree.
 * @param tree: the tree
 * @param func: function called with element and user arg.
 * 	The function must not alter the rbtree.
 * @param arg: user argument.
 */
void traverse_postorder(rbtree_t* tree, void (*func)(rbnode_t*, void*),
	void* arg);

/**
 * Call function for all elements in the redblack tree in-order.
 * In order tree traversal is needed when you need to visit each node in
 * non-decending order.
 * Note that your function must not remove the nodes from the tree.
 * Since that may trigger rebalances of the rbtree.
 * @param tree: the tree
 * @param func: function called with element and user arg.
 * The function must not alter the rbtree.
 * @param arg: user argument.
 */
void traverse_inorder(rbtree_t* tree, void (*func)(rbnode_t*, void*),
	void* arg);

#endif /* UTIL_RBTREE_H_ */
