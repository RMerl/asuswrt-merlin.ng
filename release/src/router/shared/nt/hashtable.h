#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

//Hashtable element structure
typedef struct hash_elem_t {
	struct hash_elem_t* next; // Next element in case of a collision
	void* data;	// Pointer to the stored element
	char key[]; 	// Key of the stored element
} hash_elem_t;

//Hashtabe structure
typedef struct {
	unsigned int capacity;	// Hashtable capacity (in terms of hashed keys)
	unsigned int e_num;	// Number of element currently stored in the hashtable
	hash_elem_t** table;	// The table containaing elements
} hashtable_t;

//Structure used for iterations
typedef struct {
	hashtable_t* ht; 	// The hashtable on which we iterate
	unsigned int index;	// Current index in the table
	hash_elem_t* elem; 	// Curent element in the list
} hash_elem_it;

// Inititalize hashtable iterator on hashtable 'ht'
#define HT_ITERATOR(ht) {ht, 0, ht->table[0]}

hashtable_t* ht_create(unsigned int capacity);
void ht_destroy(hashtable_t* hasht);
void* ht_put(hashtable_t* hasht, char* key, void* data);
void* ht_get(hashtable_t* hasht, char* key);
void* ht_remove(hashtable_t* hasht, char* key);
#endif
