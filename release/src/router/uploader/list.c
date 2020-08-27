#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include "list.h"
#include "function.h"
#include "api.h"
#include <unistd.h>

extern s_tree *s_link;
extern Hb_TreeNode *DirRootNode;


Hb_TreeNode *create_tree_rootnode(const char *path)
{
    Hb_TreeNode *DirTreeRoot;
    DirTreeRoot = (Hb_TreeNode *)malloc(sizeof (Hb_TreeNode));
    if(DirTreeRoot == NULL)
    {
        printf("create memory error!\n");
        return NULL;
    }
    DirTreeRoot->level=0;
    strcpy(DirTreeRoot->FileName,path);
    strcpy(DirTreeRoot->FilePath,path);
    DirTreeRoot->NextBrother=NULL;
    DirTreeRoot->pattr = NULL;
    DirTreeRoot->Child = NULL;
    DirTreeRoot->isfolder = 1;

    return DirTreeRoot;
}

Hb_TreeNode *get_tree_node(const char *filename, Hb_TreeNode *treeRoot)
{
    char fullname[NORMALSIZE];
    Hb_TreeNode *p1 = NULL ,*p2 = NULL;

    if(treeRoot != NULL)
    {
        memset(fullname,0,sizeof(fullname));

        if(treeRoot->level == 0)
            strcpy(fullname,treeRoot->FilePath);
        else
            snprintf(fullname,NORMALSIZE,"%s/%s",treeRoot->FilePath,treeRoot->FileName);

        if(!strcmp(filename,fullname))
        {
            //printf("find %s file\n",filename);
            return treeRoot;
        }
        else
        {
            if((treeRoot->Child!=NULL))
                p1 = get_tree_node(filename,treeRoot->Child);

            if(treeRoot->NextBrother != NULL)
                p2 = get_tree_node(filename,treeRoot->NextBrother);

            if(p1 != NULL)
                return p1;
            else
                if(p2 != NULL)
                    return p2;
                else
                    return NULL;
        }
    }
    else
        return NULL;
}

Hb_TreeNode *insert_new_node(const char *filename, const char *path,int isfolder,Hb_TreeNode *TreeNode)
{
    char fullname[NORMALSIZE];
    Hb_TreeNode *p1 = NULL,*p2 = NULL;

    memset(fullname,0,sizeof(fullname));

    Hb_TreeNode *tempnode = (Hb_TreeNode *)malloc(sizeof(Hb_TreeNode));
    tempnode->Child = NULL;
    tempnode->NextBrother = NULL;
    tempnode->level = TreeNode->level + 1;

    snprintf(fullname,NORMALSIZE,"%s/%s",path,filename);

    if(isfolder)
    {
       tempnode->isfolder = 1;
    }
    else
    {
        struct stat buf;
        tempnode->isfolder = 0;

        //printf("fullname is %s\n",fullname);

        if( stat(fullname,&buf) == -1)
        {
            perror("stat:");
            return NULL;
        }

        unsigned long asec = buf.st_atime;
        unsigned long msec = buf.st_mtime;
        unsigned long csec = buf.st_ctime;

        tempnode->pattr = (Attr *)malloc(sizeof(Attr));

        tempnode->pattr->lastaccesstime = (char *)malloc(sizeof(char)*16);
        tempnode->pattr->creationtime = (char *)malloc(sizeof(char)*16);
        tempnode->pattr->lastwritetime = (char *)malloc(sizeof(char)*16);

        sprintf(tempnode->pattr->lastaccesstime,"%lu",asec);
        sprintf(tempnode->pattr->creationtime,"%lu",csec);
        sprintf(tempnode->pattr->lastwritetime,"%lu",msec);
    }

    strcpy(tempnode->FilePath,path);
    strcpy(tempnode->FileName,filename);

    if(TreeNode->Child == NULL)
    {
        TreeNode->Child = tempnode;
        tempnode->NextBrother = NULL;
    }
    else
    {
        p2 = TreeNode->Child;
        p1 = p2->NextBrother;

        while(p1 != NULL)
        {
           p2 = p1;
           p1 = p1->NextBrother;
        }

        p2->NextBrother = tempnode;
        tempnode->NextBrother = NULL;
    }

    return tempnode;
}

int del_new_node(const char *filename, const char *path,int isfolder,Hb_TreeNode *treeRoot)
{
    char fullname[NORMALSIZE];
    Hb_TreeNode *p1 = NULL,*p2 = NULL;

    memset(fullname,0,sizeof(fullname));

    if(treeRoot->Child == NULL)
    {
        return -1;
    }
    else
    {
        p2 = treeRoot->Child;
        p1 = p2->NextBrother;

        if(!strcmp(p2->FileName,filename)) //del node is child
        {
            printf("del node is %s child\n",path);
            if(p1 != NULL)                 //del node have nextbrother node
            {
                printf("del node have nextbrother node\n");
                treeRoot->Child = p1;

            }
            else
            {
                treeRoot->Child = NULL;
            }

            if(p2->Child != NULL)
            {
                printf("free-r del node\n");
                free_tree_node(p2->Child);
            }
            free(p2);
        }
        else                             //del node is not child
        {
            while(p1 != NULL)
            {
               if(!strcmp(p1->FileName,filename))
                   break;
               p2 = p1;
               p1 = p1->NextBrother;
            }

            if(p1->NextBrother != NULL) //del node have nextbrother node
            {
                p2->NextBrother = p1->NextBrother;
            }
            else
            {
                p2->NextBrother = NULL;
            }

            if(p1->Child != NULL)
                free_tree_node(p1->Child);
            free(p1);
        }
    }

    return 0;
}

int modify_tree_node(char *fullname, Hb_TreeNode *rootnode,int type)
{
    char path[NORMALSIZE];
    char filename[NORMALSIZE];
    char *p = NULL;
    Hb_TreeNode *node = NULL;
    Hb_TreeNode *insert_node = NULL;
    Hb_TreeNode *tempnode = NULL;
    int isfolder =0;
    //char pathname[256];

    p = strrchr(fullname,'/');

    memset(path,0,sizeof(path));
    memset(filename,0,sizeof(filename));

    if(p)
    {
        strncpy(path,fullname,strlen(fullname)-strlen(p));
        p++;
        strcpy(filename,p);

        if(test_if_dir(fullname) == 1)
        {
            isfolder = 1;
        }

        node = get_tree_node(path,rootnode);

        if(node != NULL)
        {
            if(type == ADD_TREE_NODE)
            {
                tempnode = get_tree_node(fullname,node);
                if(tempnode == NULL)
                {
                    insert_node = insert_new_node(filename,path,isfolder,node);
                    if(isfolder)
                    {
                        FindDir(insert_node,fullname);
                    }
                }
                else
                {
                    printf("tree list have exist %s,add new node fail\n",fullname);
                    return -1;
                }
            }
            else if(type == DEL_TREE_NODE)
            {
                //del_tree_node(filename,path,isfolder,rootnode);
                del_new_node(filename,path,isfolder,node);
            }
        }
        else
        {
            printf("find %s is fail\n",path);
        }
    }

    return 0;

}

void free_tree_node(Hb_TreeNode *node)
{
    if(node != NULL)
    {
        //printf("free tree node\n");
        if(node->NextBrother != NULL)
            free_tree_node(node->NextBrother);
        if(node->Child != NULL)
            free_tree_node(node->Child);
        if(!node->isfolder)
        {
            free(node->pattr->creationtime);
            free(node->pattr->lastaccesstime);
            free(node->pattr->lastwritetime);
            free(node->pattr);
        }
        free(node);
    }
}

void FindDir(Hb_TreeNode *TreeNode,const char *path)
{
    char fullname[NORMALSIZE];
    //Hb_TreeNode *p1,*p2;
    //Hb_TreeNode *temp;
    struct dirent* ent = NULL;
    DIR *pDir;
    int isfolder = 0;

    if(TreeNode == NULL)
    {
        printf("Find Dir Fail ,node is null\n");
        return ;
    }

    pDir = opendir(path);

    if(NULL == pDir)
    {
        return;
    }

    while (NULL != (ent=readdir(pDir)))
    {

        //if(ent->d_name[0] == '.')
            //continue;
        if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
            continue;

        memset(fullname,0,sizeof(fullname));
        snprintf(fullname,NORMALSIZE,"%s/%s",path,ent->d_name);

        //printf("fullname is %s\n",fullname);

        Hb_TreeNode *p1 = NULL,*p2 = NULL;

        Hb_TreeNode *tempnode = (Hb_TreeNode *)malloc(sizeof(Hb_TreeNode));
        tempnode->Child = NULL;
        tempnode->NextBrother = NULL;
        tempnode->pattr = NULL;
        tempnode->level = TreeNode->level + 1;
        if( test_if_dir(fullname) == 1)
        {
           tempnode->isfolder = 1;
           isfolder = 1;
        }
        else
        {
           struct stat buf;
           tempnode->isfolder = 0;

           if( stat(fullname,&buf) == -1)
           {
               perror("stat:");
               continue;
           }

           unsigned long asec = buf.st_atime;
           unsigned long msec = buf.st_mtime;
           unsigned long csec = buf.st_ctime;

           tempnode->pattr = (Attr *)malloc(sizeof(Attr));

           tempnode->pattr->lastaccesstime = (char *)malloc(sizeof(char)*16);
           tempnode->pattr->creationtime = (char *)malloc(sizeof(char)*16);
           tempnode->pattr->lastwritetime = (char *)malloc(sizeof(char)*16);

           sprintf(tempnode->pattr->lastaccesstime,"%lu",asec);
           sprintf(tempnode->pattr->creationtime,"%lu",csec);
           sprintf(tempnode->pattr->lastwritetime,"%lu",msec);
        }

        strcpy(tempnode->FilePath,path);
        strcpy(tempnode->FileName,ent->d_name);

        if(TreeNode->Child == NULL)
        {
            //printf("child is blank\n");
            TreeNode->Child = tempnode;
            tempnode->NextBrother = NULL;
        }
        else
        {
            //printf("have child\n");
            p2 = TreeNode->Child;
            p1 = p2->NextBrother;

            while(p1 != NULL)
            {
               //printf("p1 nextbrother have\n");
               p2 = p1;
               p1 = p1->NextBrother;
            }

            /*
            p1 = TreeNode->Child;
            p2 = p1;

            while(p1->NextBrother != NULL)
            {
               printf("p1 nextbrother have\n");
               p2 = p1->NextBrother;
               p1 = p1->NextBrother;
            }
            */

            p2->NextBrother = tempnode;
            tempnode->NextBrother = NULL;
        }

        if( test_if_dir(fullname) == 1)
        {
           FindDir(tempnode,fullname);
        }
    }

    closedir(pDir);

}

void update_node_child(Hb_TreeNode *node)
{
    //Hb_TreeNode *prenode = NULL;
    char newpath[NORMALSIZE];
    memset(newpath,0,sizeof(newpath));

    if(node->NextBrother != NULL)
    {
        memset(node->NextBrother->FilePath,0,sizeof(node->NextBrother->FilePath));
        strcpy(node->NextBrother->FilePath,node->FilePath);
        update_node_child(node->NextBrother);
    }

    if(node->Child != NULL)
    {
        snprintf(newpath,NORMALSIZE,"%s/%s",node->FilePath,node->FileName);
        memset(node->Child->FilePath,0,sizeof(node->Child->FilePath));
        strcpy(node->Child->FilePath,newpath);
        update_node_child(node->Child);
    }
}

void rename_update_tree(const char *oldname,const char *newname)
{
    char fullname[NORMALSIZE];

    memset(fullname,0,sizeof(fullname));

    Hb_TreeNode *node = get_tree_node(oldname,DirRootNode);
    memset(node->FileName,0,sizeof(node->FileName));
    strcpy(node->FileName,newname);
    if(node->isfolder)
    {
        if(node->Child)
        {
            memset(node->Child->FilePath,0,sizeof(node->Child->FilePath));
            snprintf(fullname,NORMALSIZE,"%s/%s",node->FilePath,newname);
            strcpy(node->Child->FilePath,fullname);
            update_node_child(node->Child);
        }
    }
}

void SearchTree1(Hb_TreeNode* treeRoot)
{
    Hb_TreeNode *p1;
    p1 = treeRoot->Child;
    //p2 = p1;

    while(p1 != NULL)
    {
        printf("FilePath:%s,Filename is %s\n",p1->FilePath,p1->FileName);
        //p2 = p1->NextBrother;
        p1 = p1->NextBrother;
    }
}

void SearchTree(Hb_TreeNode* treeRoot)
{
int i;
for(i=0;i<treeRoot->level;i++)
    printf("-");
//printf("Filename is %s,level is %d\n",treeRoot->FilePath,treeRoot->FileName);
//printf("Filename is %s,level is %d\n",treeRoot->FileName,treeRoot->level);

printf("%s\n",treeRoot->FileName);

/*
if(!treeRoot->isfolder)
{
    printf("%s,%s,%s\n",treeRoot->pattr->creationtime,treeRoot->pattr->lastaccesstime,treeRoot->pattr->lastwritetime);
}
*/

if((treeRoot->Child!=NULL))
SearchTree(treeRoot->Child);

if(treeRoot->NextBrother != NULL)
SearchTree(treeRoot->NextBrother);
}

void write_tree_to_file(const char *logname,Hb_TreeNode *treeRoot)
{
    struct stat buf;
    FILE *fp;
    int have_brother = 0,have_child = 0;

    if( stat(logname,&buf) == -1)
    {
        fp = fopen(logname,"w");
    }
    else
    {
        fp = fopen(logname,"a");
    }

     if(NULL == fp)
     {
         printf("open %s failed\n",logname);
         return;
     }

     if(treeRoot->NextBrother != NULL)
     {
         have_brother= 1;
     }

     if(treeRoot->Child != NULL)
     {
         have_child = 1;
     }

     if(treeRoot->isfolder)
        fprintf(fp,"%s,%s,%d,%d,%d,%d\n",
                treeRoot->FilePath,treeRoot->FileName,treeRoot->level,treeRoot->isfolder,
                have_brother,have_child);
     else
         fprintf(fp,"%s,%s,%d,%d,%d,%d,%s,%s,%s\n",
                 treeRoot->FilePath,treeRoot->FileName,treeRoot->level,treeRoot->isfolder,
                 have_brother,have_child,
                 treeRoot->pattr->lastaccesstime,treeRoot->pattr->creationtime,treeRoot->pattr->lastwritetime);


     fclose(fp);

     if((treeRoot->Child!=NULL))
        write_tree_to_file(logname,treeRoot->Child);

     if(treeRoot->NextBrother != NULL)
        write_tree_to_file(logname,treeRoot->NextBrother);

}


/* the meaning of each item in file
0--FilePath
1--FileName
2--level
3--isfolder
4--lastaccesstime
5--creationtime
6--lastwritetime
*/
Hb_TreeNode *read_file_to_tree(const char *logname)
{
    FILE *fp;

    char buffer[NORMALSIZE];
    const char *split = ",";
    char *p;

    Hb_TreeNode *root = NULL;
    Hb_TreeNode *prenode = NULL;


    memset(buffer, '\0', sizeof(buffer));

    if (access(logname,0) == 0)
    {
        if(( fp = fopen(logname,"r"))==NULL)
        {
            fprintf(stderr,"read Cloud error");
            return NULL ;
        }
        else
        {
            while(fgets(buffer,NORMALSIZE,fp)!=NULL)
            {
                Hb_TreeNode *tempnode = (Hb_TreeNode *)malloc(sizeof(Hb_TreeNode));

                memset(tempnode,0,sizeof(Hb_TreeNode));

                p=strtok(buffer,split);

                int i=0;
                int have_brother = 0;
                int have_child = 0;
                while(p!=NULL)
                {
                    switch (i)
                    {
                    case 0 :
                        strcpy(tempnode->FilePath,p);
                        break;
                    case 1:
                        strcpy(tempnode->FileName,p);
                        break;
                    case 2:
                        tempnode->level = atoi(p);
                        break;
                    case 3:
                        tempnode->isfolder = atoi(p);
                        break;
                    case 4:
                        have_brother = atoi(p);
                        break;
                    case 5:
                        have_child = atoi(p);
                        break;
                    case 6:
                        if(!tempnode->isfolder)
                        {
                            tempnode->pattr = (Attr *)malloc(sizeof(Attr));
                            tempnode->pattr->lastaccesstime = (char *)malloc(sizeof(char)*16);
                            strcpy(tempnode->pattr->lastaccesstime,p);
                        }
                        break;
                    case 7:
                        if(!tempnode->isfolder)
                        {
                            tempnode->pattr->creationtime = (char *)malloc(sizeof(char)*16);
                            strcpy(tempnode->pattr->lastaccesstime,p);
                        }
                        break;
                    case 8:
                        if(!tempnode->isfolder)
                        {
                            tempnode->pattr->lastwritetime = (char *)malloc(sizeof(char)*16);
                            strcpy(tempnode->pattr->lastwritetime,p);
                        }
                        break;
                    default:
                        break;
                    }

                    i++;
                    p=strtok(NULL,split);
                }

                if(root == NULL)
                    root = tempnode;
                else
                {


                    if(root->Child == NULL)
                    {
                        root->Child = tempnode;
                    }
                    else
                    {
                        if(tempnode->level > prenode->level)
                        {
                            prenode->Child = tempnode;
                        }
                        else if(tempnode->level == prenode->level)
                        {
                           prenode->NextBrother = tempnode;
                        }
                        else
                        {
                            Hb_TreeNode *back_node = pop_stree();
                            if(back_node == NULL)
                            {
                                printf("can not obtain parent node\n");
                                SearchTree(root);
                                return NULL;
                            }
                            else
                            {
                               back_node->NextBrother = tempnode;
                            }
                        }
                    }

                    if(have_brother == 1 && have_child == 1)
                    {
                        push_stree(tempnode);
                        //print_stree();
                    }

                }

                prenode = tempnode;
            }

            fclose(fp);
        }
    }

    return root;
}

void print_stree()
{
    s_tree *temp;
    temp = s_link;
    if(temp == NULL)
    {
        printf("stack is blank\n");
    }
    else
    {
        //printf("############ print stree start ##########\n");
        while(temp != NULL)
        {
            printf("path is %s,filename is %s\n",temp->point->FilePath,temp->point->FileName);
            temp = temp->next;
        }
    }
}

void push_stree(Hb_TreeNode *node)
{
    s_tree *new_item = NULL;
    new_item = (s_tree *)malloc(sizeof(s_tree));

    memset(new_item,0,sizeof(s_tree));

    if(new_item == NULL)
    {
        printf("obtain memory fail\n");
        return;
    }

    new_item->point = node;
    new_item->next = s_link;
    s_link = new_item;

    printf("############ after push stree ##########\n");
    print_stree();
}

Hb_TreeNode *pop_stree()
{
    Hb_TreeNode *node = NULL;
    s_tree *top = NULL;
    if(s_link != NULL)
    {
        top = s_link;
        s_link = top->next;
        node = top->point;
        free(top);
    }

    printf("<<<<<<< after pop stree ##########\n");
    print_stree();

    return node;
}

/*server tree root function*/
Server_TreeNode *create_server_treeroot(int id)
{
    Server_TreeNode *TreeRoot = NULL;
    TreeRoot = (Server_TreeNode *)malloc(sizeof (Server_TreeNode));
    memset(TreeRoot,0,sizeof(Server_TreeNode));
    if(TreeRoot == NULL)
    {
        printf("create memory error!\n");
        return NULL;
    }
    TreeRoot->level=0;
    //TreeRoot->NextBrother = NULL;
    //TreeRoot->browse = NULL;
    TreeRoot->id = 0;

    return TreeRoot;
}

int browse_to_tree(char *username,int parentid, char *xmlfilename,Server_TreeNode *node)
{
    Browse *br = NULL;
    int id;
    int fail_flag = 0;
    //int loop;
    int i;

    printf("node id is %d,parentid is %d\n",node->id,parentid);

    Server_TreeNode *tempnode = NULL, *p1 = NULL,*p2 = NULL;
    tempnode = (Server_TreeNode *)malloc(sizeof(Server_TreeNode));
    memset(tempnode,0,sizeof(Server_TreeNode));
    tempnode->level = node->level + 1;

    br = browseFolder(username,parentid,0,200);

    if(NULL == br)
    {
        printf("browse folder failed\n");
        return -1;
    }

    tempnode->browse = br;
    tempnode->id = parentid;

    if(node->Child == NULL)
    {
        node->Child = tempnode;
    }
    else
    {
        //printf("have child\n");
        p2 = node->Child;
        p1 = p2->NextBrother;

        while(p1 != NULL)
        {
           //printf("p1 nextbrother have\n");
           p2 = p1;
           p1 = p1->NextBrother;
        }

        p2->NextBrother = tempnode;
        tempnode->NextBrother = NULL;
    }

    printf("browse folder num is %d\n",br->foldernumber);
    for( i= 0; i <br->foldernumber;i++)
    {
        id = (br->folderlist)[i]->id;

        if(browse_to_tree(username,id,xmlfilename,tempnode) == -1)
        {
            fail_flag = 1;
        }
    }

    //free_server_list(br);
    //my_free(br);

    return (fail_flag == 1) ? -1 : 0 ;

}

void SearchServerTree(Server_TreeNode* treeRoot)
{
    int i;
    for(i=0;i<treeRoot->level;i++)
        printf("-");


    printf("%d\n",treeRoot->id);

    /*
if(!treeRoot->isfolder)
{
    printf("%s,%s,%s\n",treeRoot->pattr->creationtime,treeRoot->pattr->lastaccesstime,treeRoot->pattr->lastwritetime);
}
*/

    if((treeRoot->Child!=NULL))
        SearchServerTree(treeRoot->Child);

    if(treeRoot->NextBrother != NULL)
        SearchServerTree(treeRoot->NextBrother);
}

void free_server_tree(Server_TreeNode *node)
{
    if(node != NULL)
    {
        //printf("free tree node\n");

        free_server_list(node->browse);

        if(node->NextBrother != NULL)
            free_server_tree(node->NextBrother);
        if(node->Child != NULL)
            free_server_tree(node->Child);

        free(node);
    }
}

