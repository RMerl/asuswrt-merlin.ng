/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
       R A N K  A L G O R I T H M  F U N C T I O N  P R O T O T Y P E S
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
struct bit {
   unsigned char b:1;
};
typedef struct bit BitField;


void       perform_elementary_row_operations(int,int,int,int,BitField**);
int        find_unit_element_and_swap(int,int,int,int, BitField**);
int        swap_rows(int, int, int, BitField**);
int        determine_rank(int, int, int, BitField**);
int        computeRank(int,int,BitField**);
void       define_matrix(int,int,int,BitField**);
BitField** create_matrix(int, int);
void       display_matrix(int, int, BitField**);
void def_matrix(int M, int Q, BitField** m,int k, int *pt, int *PT, int*DATA, int *ARRAY);
void       delete_matrix(int, BitField**);
