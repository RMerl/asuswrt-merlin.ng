#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "mem_pool.h"

// start : modify by markcool
#define AICAM 1

#if AICAM
#include <stdint.h>
#endif
// end : modify by markcool

static list_node *available[LIST_NUM];
static list_node *rear[LIST_NUM];

static manager_node     manager_list[LIST_NUM];
static list_node     list_8_node[NODE_NUM];
static list_node     list_64_node[NODE_NUM];

typedef list_node* (*init_ptr) (list_node **);

list_node * eight_init(list_node **last)
{
    void *a = malloc(8*NODE_NUM);
    //printf("the list8 start addr is %x\n",(int)a);
    int i;
    assert_param(a);
    for(i=0;i<NODE_NUM;i++)
    {
        list_8_node[i].addr=a+i*8;
        list_8_node[i].next_node=NULL;
        //printf("the list %d addr is %x\n",i,(int)(list_8_node[i].addr));
        if(i==NODE_NUM-1)
            break;
        list_8_node[i].next_node=&list_8_node[i+1];
    }
    *last=&(list_8_node[i]);
    //printf("the list8 node %dend addr is %x\n\n",i,(int)(list_8_node[i].addr));
    return (&list_8_node[0]);
}

list_node * sixtyfour_init(list_node **last)
{
    void *a = malloc(64*NODE_NUM);
    //printf("the list64 start addr is %x\n",(int)a);
    int i;
    assert_param(a);
    for(i=0;i<NODE_NUM;i++)
    {
        list_64_node[i].addr=a+i*64;
        list_64_node[i].next_node=NULL;
        //printf("the list %d addr is %x\n",i,(int)(list_8_node[i].addr));
        if(i==NODE_NUM-1)
            break;
        list_64_node[i].next_node=&list_64_node[i+1];
    }
    *last=&(list_64_node[i]);
    //printf("the list64 node %dend addr is %x\n\n",i,(int)(list_64_node[i].addr));
    return (&list_64_node[0]);
}

static init_ptr func_ptr[]=
{
    eight_init,
    sixtyfour_init,
};

int mem_pool_init()
{
    int i;
    for(i=0;i<LIST_NUM;i++)
    {
        available[i]=(*func_ptr[i])(&(rear[i]));
    }
    for(i=0;i<LIST_NUM;i++)
    {
        manager_list[i].start_node=available[i];
        manager_list[i].last_node=rear[i];

#if AICAM
        manager_list[i].addr_start=(intptr_t)(available[i]->addr);  // markcool modify
        manager_list[i].addr_end=(intptr_t)(manager_list[i].last_node->addr);  // markcool modify
#else 
        manager_list[i].addr_start=(unsigned int)(available[i]->addr);
        manager_list[i].addr_end=(unsigned int)(manager_list[i].last_node->addr);
#endif

        //printf("the manager_list[%d] \tstart addr is%x and end addr is %x\n\n",i,(unsigned int)(unsigned int)(manager_list[i].start_node->addr),(unsigned int)(manager_list[i].last_node->addr));
    }

    return 0;
}


void * mem_alloc(int num)
{
    //printf("malloc num=%d\n",num);
    int i;
    if(num>=1&&num<=8)
        i=0;
    else if(num>=9&&num<=64)
       i=1;
    else
        return (malloc(num));
    if(manager_list[i].start_node == NULL)
    {
        printf("this is no space in mempool!!\n");
        return (malloc(num));
    }

    list_node *p=manager_list[i].start_node;
    manager_list[i].start_node = manager_list[i].start_node->next_node;
    p->next_node=NULL;    //alan deleted

    return (p->addr);
}


void mem_free(void *addr)
{
    if(addr==NULL)
        return ;
    int i;

    // start : modify by markcool
    #if AICAM
    unsigned int num=(intptr_t)(addr);  // markcool modify
    #else 
    unsigned int num=(unsigned int)(addr);
    #endif
    // end : modify by markcool

    unsigned int start_addr,end_addr;
    for(i=0;i<LIST_NUM;i++)
    {
	// start : modify by markcool
        #if AICAM
        start_addr=(intptr_t)available[i]->addr;  // markcool modify
        end_addr=(intptr_t)rear[i]->addr;  // markcool modify
        #else 
        start_addr=(unsigned int)available[i]->addr;
        end_addr=(unsigned int)rear[i]->addr;
        #endif
	// end : modify by markcool

        if(num>=start_addr&&num<=end_addr)
        break;
    }
//    printf("the free add is %x and the start addr is %x\n",num,start_addr);


//    num=(num-start_addr)/(8*(i+1));
    if(i>=LIST_NUM)  //alan  add
    {
        free(addr);
        return;
    }

    if(i == 0)
    {
        num=(num-start_addr)/8;
    }
    else if(i == 1)
    {
        num=(num-start_addr)/64;

    }

    if(num>NODE_NUM)
    {
        free(addr);
        return;
    }
//    printf("the free node is %d\tand is nodelist[%d]\n",num,i);
    switch(i)
    {
        case 0:
//            printf("list_8_node[%d] the next is %x\n",num,list_8_node[num].next_node);
            list_8_node[num].next_node=manager_list[i].start_node;
            manager_list[i].start_node=&(list_8_node[num]);
            break;
        case 1:
//            printf("list_16_node[%d] the next is %x\n",num,list_16_node[num].next_node);
            list_64_node[num].next_node=manager_list[i].start_node;
            manager_list[i].start_node=&(list_64_node[num]);
            break;
        default:
            free(addr);
            break;
    }
}

void mem_pool_destroy()
{
    int i;
    for(i=0;i<LIST_NUM;i++)
    {
        free(available[i]->addr);
    }
}

