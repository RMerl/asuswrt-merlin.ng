

#include <stdio.h>
#include <stdlib.h>
/*#include "defs.h"
#include "proto.h"*/
#define MIN(x,y)             ((x) >  (y)  ? (y)  : (x))

#include "matrix.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
                 R A N K  A L G O R I T H M  R O U T I N E S
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int computeRank(int M, int Q, BitField** matrix)
{
   int i;
   int rank;
   int m = MIN(M,Q);

   /* FORWARD APPLICATION OF ELEMENTARY ROW OPERATIONS */

   for(i = 0; i < m-1; i++) {
      if (matrix[i][i].b == 1)
         perform_elementary_row_operations(0, i, M, Q, matrix);
      else { 	/* matrix[i][i] = 0 */
         if (find_unit_element_and_swap(0, i, M, Q, matrix) == 1)
            perform_elementary_row_operations(0, i, M, Q, matrix);
      }
   }

   /* BACKWARD APPLICATION OF ELEMENTARY ROW OPERATIONS */
   for(i = m-1; i > 0; i--) {
      if (matrix[i][i].b == 1)
         perform_elementary_row_operations(1, i, M, Q, matrix);
      else { 	/* matrix[i][i] = 0 */
         if (find_unit_element_and_swap(1, i, M, Q, matrix) == 1)
            perform_elementary_row_operations(1, i, M, Q, matrix);
      }
   }
   rank = determine_rank(m,M,Q,matrix);

   return rank;
}

void perform_elementary_row_operations(int flag, int i, int M, int Q,
			               BitField** A)
{
  int j,k;

  switch(flag)
  {
     case 0: for(j = i+1; j < M;  j++)
                if (A[j][i].b == 1)
                   for(k = i; k < Q; k++)
                      A[j][k].b = (A[j][k].b + A[i][k].b) % 2;
             break;

     case 1: for(j = i-1; j >= 0;  j--)
                if (A[j][i].b == 1)
                   for(k = 0; k < Q; k++)
                       A[j][k].b = (A[j][k].b + A[i][k].b) % 2;
             break;
     default:  fprintf(stderr,"ERROR IN CALL TO perform_elementary_row_operations\n");
               break;
  }
  return;
}

int find_unit_element_and_swap(int flag, int i, int M, int Q, BitField** A)
{
  int index;
  int row_op = 0;

  switch(flag)
  {
    case 0:  index = i+1;
             while ((index < M) && (A[index][i].b == 0))
                index++;
             if (index < M)
                row_op = swap_rows(i,index,Q,A);
             break;
    case 1:
             index = i-1;
	     while ((index >= 0) && (A[index][i].b == 0))
	       index--;
	     if (index >= 0)
                row_op = swap_rows(i,index,Q,A);
             break;
    default:  fprintf(stderr,"ERROW IN CALL TO find_unit_element_and_swap\n");
              break;
  }
  return row_op;
}

int swap_rows(int i, int index, int Q, BitField** A)
{
  int p;
  BitField temp;

  for(p = 0; p < Q; p++) {
     temp.b = A[i][p].b;
     A[i][p].b = A[index][p].b;
     A[index][p].b = temp.b;
  }
  return 1;
}

int determine_rank(int m, int M, int Q, BitField** A)
{
   int i, j, rank, allZeroes;

   /* DETERMINE RANK, THAT IS, COUNT THE NUMBER OF NONZERO ROWS */

   rank = m;
   for(i = 0; i < M; i++) {
      allZeroes = 1;
      for(j=0; j < Q; j++) {
         if (A[i][j].b == 1) {
            allZeroes = 0;
            break;
         }
      }
      if (allZeroes == 1) rank--;
   }
   return rank;
}


void display_matrix(int M, int Q, BitField** m)
{
  int i,j;

  for (i = 0; i < M; i++) {
     for (j = 0; j < Q; j++)
        fprintf(stderr,"%d ", m[i][j].b);
     fprintf(stderr,"\n");
  }
  return;
}

void def_matrix(int M, int Q, BitField** m,int k, int *pt, int *PT, int*DATA, int *ARRAY)
{
  int   i,j;
  for (i = 0; i < M; i++)
     for (j = 0; j < Q; j++) {
         m[i][j].b = *DATA & 1;
(*PT)++;
	  if ((*PT) == 32)
	    {
	      *PT = 0;
	      (*pt)++;
	      *DATA = ARRAY[*pt];
	    }
	  else
	    *DATA = (*DATA) >> 1;

     }
  return;
}

void delete_matrix(int M, BitField** matrix)
{
  int i;
  for (i = 0; i < M; i++)
    free(matrix[i]);
  free(matrix);
}
