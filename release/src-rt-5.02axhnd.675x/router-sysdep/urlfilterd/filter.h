#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_WEB_LEN	40
#define MAX_FOLDER_LEN	56
#define MAX_LIST_NUM	100

typedef struct _URL{
	char website[MAX_WEB_LEN];
	char folder[MAX_FOLDER_LEN];
	struct _URL *next;
}URL, *PURL;

PURL purl = NULL;

unsigned int list_count = 0;

//const char list_to_open[] = "/dan/url_list";

//extern int get_url_info();
//extern void add_entry(char *, char *);

