#include <stdlib.h>
#include <stdio.h>

#include <shared.h>
#include <llist.h>

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
LList *llist_last(LList *list)
{
	LList *next;

	if(list)
	{
		next = list;
		while(next->next)
		{
			next = next->next;
		}
		return next;
	}
	return NULL;
}

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
LList *llist_prepend(LList *list, void* data)
{
	LList *new_lst;

	new_lst = calloc(1, sizeof(LList));

	if(new_lst)
	{
		new_lst->data = data;
		new_lst->next = list;

		if(list)
		{
			new_lst->prev = list->prev;
			if(list->prev)
				list->prev->next = new_lst;
			list->prev = new_lst;
		}
		else
		{
			new_lst->prev = NULL;
		}
		return new_lst;
	}
	return NULL;
}

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
LList *llist_append(LList *list, void *data)
{
	LList *new_lst, *last_lst;

	new_lst = calloc(1, sizeof(LList));

	if(new_lst)
	{
		new_lst->data = data;
		new_lst->next = NULL;

		if(list)
		{
			last_lst = llist_last(list);
			if(last_lst)
			{
				last_lst->next = new_lst;
				new_lst->prev = last_lst;
				return list;
			}
			else
			{
				SAFE_FREE(new_lst);
				return NULL;
			}
		}
		else
		{
			new_lst->prev = NULL;
		}
		return new_lst;
	}
	return NULL;
}

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
LList *llist_find(LList *list, void*compare_data, compare_func func)
{
	if(list && compare_data && func)
	{
		while(list)
		{
			if(!func(list->data, compare_data))
				return list;
			list = list->next;
		}
	}
	return NULL;
}

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
LList *llist_free_1(LList *list, LList *node, free_data_func func)
{
	LList *head = NULL;

	if(list && node)
	{
		if(list == node)
		{
			head = list->next;
			if(head)
			{
				head->prev = NULL;
			}
			if(func)
			{
				func(node->data);
			}
			SAFE_FREE(node);
			return head;
		}
		else
		{
			if(node->prev)
			{
				node->prev->next = node->next;
			}
			if(node->next)
			{
				node->next->prev = node->prev;
			}
			if(func)
			{
				func(node->data);
			}
			SAFE_FREE(node);
			return list;
		}
	}
	return NULL;
}

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
void llist_free_all(LList *list, free_data_func func)
{
	LList *node;

	while(list)
	{
		node = list;
		list = list->next;
		if(func)
		{
			func(node->data);
		}
		SAFE_FREE(node);
	}
}

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
void llist_dump_data(LList *list, dump_data_func func)
{
	int i = 0;
	if(!func)
		return;

	while(list)
	{
		func(i, list->data);
		list = list->next;
		++i;
	}
}
