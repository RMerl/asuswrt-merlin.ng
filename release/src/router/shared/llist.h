#ifndef __LLIST_H__
#define __LLIST_H__

typedef struct _LList LList;

struct _LList
{
	void *data;
	LList *prev;
	LList *next;
};

/*******************************************************************
* NAME: free_data_func
* AUTHOR: Andy Chiu
* CREATE DATE: 2022/8/10
* DESCRIPTION: free data
* INPUT:  data: the member variable, data of the LList 
* OUTPUT:  None
* RETURN: None
* NOTE:
*******************************************************************/
typedef void (*free_data_func)(void* data);

/*******************************************************************
* NAME: compare_func
* AUTHOR: Andy Chiu
* CREATE DATE: 2022/8/10
* DESCRIPTION: comapre function for search
* INPUT:  data: the member variable, data of the LList 
*					compare_data: the data to compare 
* OUTPUT:  None
* RETURN: 0: matched. -1: nonmatched
* NOTE:
*******************************************************************/
typedef int (*compare_func)(void* data, void* compare_data);

/*******************************************************************
* NAME: dump_data_func
* AUTHOR: Andy Chiu
* CREATE DATE: 2022/8/10
* DESCRIPTION: dump data
* INPUT:  index: the index of the node in the list
*					data: the member variable, data of the LList 
* OUTPUT:  None
* RETURN: None
* NOTE:
*******************************************************************/
typedef void (*dump_data_func)(const int index, void* data);

/*******************************************************************
* NAME: llist_last
* AUTHOR: Andy Chiu
* CREATE DATE: 2022/8/10
* DESCRIPTION: get the last node of list
* INPUT:  list: pointer of LList 
* OUTPUT:  None
* RETURN: The pointer of the last node.
* NOTE:
*******************************************************************/
LList *llist_last(LList *list);

/*******************************************************************
* NAME: llist_prepend
* AUTHOR: Andy Chiu
* CREATE DATE: 2022/8/10
* DESCRIPTION:  prepend a node in a list
* INPUT:  list: potiner of LList.
*				   data: data of the LList node.
* OUTPUT:  None
* RETURN: The pointer of the node of new LList.
* NOTE:
*******************************************************************/
LList *llist_prepend(LList *list, void* data);

/*******************************************************************
* NAME: llist_append
* AUTHOR: Andy Chiu
* CREATE DATE: 2022/8/10
* DESCRIPTION:  append a node in a list
* INPUT:  list: potiner of LList.
*				   data: data of the LList node.
* OUTPUT:  None
* RETURN: The new start pointer of the LList.
* NOTE:
*******************************************************************/
LList *llist_append(LList *list, void *data);

/*******************************************************************
* NAME: llist_find
* AUTHOR: Andy Chiu
* CREATE DATE: 2022/8/10
* DESCRIPTION:  Search the list with a compare function and data
* INPUT:  list: potiner of LList.
*				   compare_data: data to compare.
*					func: pointer of the compare callback function
* OUTPUT:  None
* RETURN: The poniter of the matched  node in the list.
* NOTE:
*******************************************************************/
LList *llist_find(LList *list, void*compare_data, compare_func func);

/*******************************************************************
* NAME: llist_free_1
* AUTHOR: Andy Chiu
* CREATE DATE: 2022/8/10
* DESCRIPTION:  Free one node of the list. If func was set, the data of the node would be also free
* INPUT:  list: pointer of head node of LList.
*				   node: pointer of the node need to be free
*					func: pointer of the free_data_func
* OUTPUT:  None
* RETURN: The pointer of head node of LList.
* NOTE:
*******************************************************************/
LList *llist_free_1(LList *list, LList *node, free_data_func func);

/*******************************************************************
* NAME: llist_free_all
* AUTHOR: Andy Chiu
* CREATE DATE: 2022/8/10
* DESCRIPTION:  Free all memory of the list. If func was set, the data of the node would be also free
* INPUT:  list: pointer of head node of LList.
*					func: pointer of the free_data_func
* OUTPUT:  None
* RETURN: None
* NOTE:
*******************************************************************/
void llist_free_all(LList *list, free_data_func func);

/*******************************************************************
* NAME: llist_free_all
* AUTHOR: Andy Chiu
* CREATE DATE: 2022/8/10
* DESCRIPTION:  Free all memory of the list. If func was set, the data of the node would be also free
* INPUT:  list: pointer of head node of LList.
*					func: pointer of the free_data_func
* OUTPUT:  None
* RETURN: None
* NOTE:
*******************************************************************/
void llist_dump_data(LList *list, dump_data_func func);

#endif