#include "Sorting.h"

typedef struct
{
	void *context;
	SDSortCompareCallback *comp;
	SDSortSwapCallback *swap;
} SDSort;

int Sorting_isSorted(SDSort *self, size_t size);
void Sorting_quickSort(SDSort *self, size_t lb, size_t ub);
int Sorting_quickSortRearrange(SDSort *self, size_t lb, size_t ub);

void Sorting_context_comp_swap_size_type_(void *context,
									SDSortCompareCallback *comp,
									SDSortSwapCallback *swap,
									size_t size,
									SDSortType type)
{
	SDSort q;
	SDSort *self = &q;

	self->context = context;
	self->comp = comp;
	self->swap = swap;

	switch (type)
	{
		case SDQuickSort:
			if (!Sorting_isSorted(self, size)) Sorting_quickSort(self, 0, size-1);
			break;
	}
}

int Sorting_isSorted(SDSort *self, size_t size)
{
	SDSortCompareCallback *comp = self->comp;
	void *context = self->context;
	size_t i;

	for (i = 0; i + 1 < size; i ++)
	{
		if ((*comp)(context, i, i + 1) > 0)
		{
			return 0;
		}
	}

	return 1;
}

void Sorting_quickSort(SDSort *self, size_t lb, size_t ub)
{
	if (lb < ub)
	{
		int j = Sorting_quickSortRearrange(self, lb, ub);

		if (j)
		{
			Sorting_quickSort(self, lb, j - 1);
		}

		Sorting_quickSort(self, j + 1, ub);
	}
}

int Sorting_quickSortRearrange(SDSort *self, size_t lb, size_t ub)
{
	SDSortCompareCallback *comp = self->comp;
	SDSortSwapCallback *swap = self->swap;
	void *context = self->context;

	do {
		while (ub > lb && (*comp)(context, ub, lb) >= 0)
		{
			ub --;
		}

		if (ub != lb)
		{
			(*swap)(context, ub, lb);

			while (lb < ub && (*comp)(context, lb, ub) <= 0)
			{
				lb ++;
			}

			if (lb != ub)
			{
				(*swap)(context, lb, ub);
			}
		}
	} while (lb != ub);

	return lb;
}
