/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Google LLC
 */

#ifndef __SORT_H
#define __SORT_H

/**
 * qsort() - Use the quicksort algorithm to sort some values
 *
 * @base: Base address of array to sort
 * @nmemb: Number of members to sort
 * @size: Size of each member in bytes
 * @compar: Comparison function which should return:
 *	< 0 if element at s1 < element at s2,
 *	  0 if element at s1 == element at s2,
 *	> 0 if element at s1 > element at s2,
 */
void qsort(void *base, size_t nmemb, size_t size,
	   int (*compar)(const void *s1, const void *s2));

/**
 * strcmp_compar() - compar function for string arrays
 *
 * This can be passed to qsort when a string array is being sorted
 *
 * @s1: First string to compare
 * @s2: Second string to compare
 * Return: comparison value (less than, equal to, or greater than 0)
 */
int strcmp_compar(const void *s1, const void *s2);

#endif
