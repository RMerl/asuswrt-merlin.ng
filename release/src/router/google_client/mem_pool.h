#ifndef __MEM_POOL_H
#define __MEM_POOL_H

#define NODE_NUM 5000
#define LIST_NUM 2

typedef struct _list_node
{
   void *addr;
   struct _list_node *next_node;
}list_node;

typedef struct _manager_node
{
   list_node *start_node;
   list_node *last_node;
   unsigned int addr_start;
   unsigned int addr_end;
   int node_num;
   int available_num;
}manager_node;


int mem_pool_init();
void * mem_alloc(int num);
void mem_free(void *addr);
void mem_pool_destroy();

#define assert_param(param)          if(param==NULL) return 0;
#endif
