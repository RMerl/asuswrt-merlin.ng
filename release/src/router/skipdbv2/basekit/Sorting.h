#include "Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
   The reason for using this instead of C's qsort is
   that we need more context information than just the
   two objects if we want to do something like use an
   Io block to do the comparison and using globals is
   unacceptable for several reasons.
*/

typedef enum
{
	SDQuickSort
	//SDShellSort
} SDSortType;

typedef int (SDSortCompareCallback)(void *context, int i, int j);
typedef void (SDSortSwapCallback)(void *context, int i, int j);

BASEKIT_API void Sorting_context_comp_swap_size_type_(void *context,
													  SDSortCompareCallback *comp,
													  SDSortSwapCallback *swap,
													  size_t size,
													  SDSortType type);

#ifdef __cplusplus
}
#endif
