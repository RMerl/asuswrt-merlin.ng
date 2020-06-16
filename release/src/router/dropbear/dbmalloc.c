#include "dbmalloc.h"
#include "dbutil.h"


void * m_calloc(size_t nmemb, size_t size) {
    if (SIZE_T_MAX / nmemb < size) {
        dropbear_exit("m_calloc failed");
    }
    return m_malloc(nmemb*size);
}

void * m_strdup(const char * str) {
    char* ret;
    unsigned int len;
    len = strlen(str);

    ret = m_malloc(len+1);
    if (ret == NULL) {
        dropbear_exit("m_strdup failed");
    }
    memcpy(ret, str, len+1);
    return ret;
}

#if !DROPBEAR_TRACKING_MALLOC

/* Simple wrappers around malloc etc */
void * m_malloc(size_t size) {

	void* ret;

	if (size == 0) {
		dropbear_exit("m_malloc failed");
	}
	ret = calloc(1, size);
	if (ret == NULL) {
		dropbear_exit("m_malloc failed");
	}
	return ret;

}

void * m_realloc(void* ptr, size_t size) {

	void *ret;

	if (size == 0) {
		dropbear_exit("m_realloc failed");
	}
	ret = realloc(ptr, size);
	if (ret == NULL) {
		dropbear_exit("m_realloc failed");
	}
	return ret;
}


#else

/* For fuzzing */

struct dbmalloc_header {
    unsigned int epoch;
    struct dbmalloc_header *prev;
    struct dbmalloc_header *next;
};

static void put_alloc(struct dbmalloc_header *header);
static void remove_alloc(struct dbmalloc_header *header);

/* end of the linked list */
static struct dbmalloc_header* staple;

unsigned int current_epoch = 0;

void m_malloc_set_epoch(unsigned int epoch) {
    current_epoch = epoch;
}

void m_malloc_free_epoch(unsigned int epoch, int dofree) {
    struct dbmalloc_header* header;
    struct dbmalloc_header* nextheader = NULL;
    struct dbmalloc_header* oldstaple = staple;
    staple = NULL;
    /* free allocations from this epoch, create a new staple-anchored list from
    the remainder */
    for (header = oldstaple; header; header = nextheader)
    {
        nextheader = header->next;
        if (header->epoch == epoch) {
            if (dofree) {
                free(header);
            }
        } else {
            header->prev = NULL;
            header->next = NULL;
            put_alloc(header);
        }
    }
}

static void put_alloc(struct dbmalloc_header *header) {
    assert(header->next == NULL);
    assert(header->prev == NULL);
    if (staple) {
        staple->prev = header;
    }
    header->next = staple;
    staple = header;
}

static void remove_alloc(struct dbmalloc_header *header) {
    if (header->prev) {
        header->prev->next = header->next;
    }
    if (header->next) {
        header->next->prev = header->prev;
    }
    if (staple == header) {
        staple = header->next;
    }
    header->prev = NULL;
    header->next = NULL;
}

static struct dbmalloc_header* get_header(void* ptr) {
    char* bptr = ptr;
    return (struct dbmalloc_header*)&bptr[-sizeof(struct dbmalloc_header)];
}

void * m_malloc(size_t size) {
    char* mem = NULL;
    struct dbmalloc_header* header = NULL;

    if (size == 0 || size > 1e9) {
        dropbear_exit("m_malloc failed");
    }

    size = size + sizeof(struct dbmalloc_header);

    mem = calloc(1, size);
    if (mem == NULL) {
        dropbear_exit("m_malloc failed");
    }
    header = (struct dbmalloc_header*)mem;
    put_alloc(header);
    header->epoch = current_epoch;
    return &mem[sizeof(struct dbmalloc_header)];
}

void * m_realloc(void* ptr, size_t size) {
    char* mem = NULL;
    struct dbmalloc_header* header = NULL;
    if (size == 0 || size > 1e9) {
        dropbear_exit("m_realloc failed");
    }

    header = get_header(ptr);
    remove_alloc(header);

    size = size + sizeof(struct dbmalloc_header);
    mem = realloc(header, size);
    if (mem == NULL) {
        dropbear_exit("m_realloc failed");
    }

    header = (struct dbmalloc_header*)mem;
    put_alloc(header);
    return &mem[sizeof(struct dbmalloc_header)];
}

void m_free_direct(void* ptr) {
    struct dbmalloc_header* header = NULL;
    if (!ptr) {
        return;
    }
    header = get_header(ptr);
    remove_alloc(header);
    free(header);
}

#endif /* DROPBEAR_TRACKING_MALLOC */

void * m_realloc_ltm(void* ptr, size_t oldsize, size_t newsize) {
   (void)oldsize;
   return m_realloc(ptr, newsize);
}

void m_free_ltm(void *mem, size_t size) {
   (void)size;
   m_free_direct(mem);
}
