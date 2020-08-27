#include "data.h"

#ifndef LIST_H
#define LIST_H

#define ADD_TREE_NODE    1
#define DEL_TREE_NODE    2

typedef struct ATTR
{
    char *lastaccesstime;
    char *creationtime;
    char *lastwritetime;
}Attr;

struct TreeNode
{
    char FilePath[256];
    char FileName[64];
    int level;
    int isfolder;
    int size;
    Attr *pattr;
    struct TreeNode *Child;
    struct TreeNode *NextBrother;
};

typedef struct TreeNode Hb_TreeNode;

struct tree_stack
{
    Hb_TreeNode *point;
    struct tree_stack *next;
};

typedef struct tree_stack s_tree;

Hb_TreeNode *create_tree_rootnode(const char *path);
void FindDir(Hb_TreeNode* TreeNode,const char *path);
int modify_tree_node(char *fullname, Hb_TreeNode *rootnode,int type);
Hb_TreeNode *get_tree_node(const char *filename,Hb_TreeNode *rootnode);
void free_tree_node(Hb_TreeNode *rootnode);
void SearchTree(Hb_TreeNode* treeRoot);
void write_tree_to_file(const char *logname,Hb_TreeNode *treeRoot);
Hb_TreeNode *read_file_to_tree(const char *logname);
void SearchTree1(Hb_TreeNode* treeRoot);
void rename_update_tree(const char *oldname,const char *newname);

void print_stree();
void push_stree(Hb_TreeNode *node);
Hb_TreeNode *pop_stree();

/*server item tree struct and function*/

struct ServerTreeNode
{
    int level;
    int id;
    Browse *browse;
    struct ServerTreeNode *Child;
    struct ServerTreeNode *NextBrother;
};

typedef struct ServerTreeNode Server_TreeNode;

Server_TreeNode *create_server_treeroot(int id);
int browse_to_tree(char *username,int parentid, char *xmlfilename,Server_TreeNode *node);
void SearchServerTree(Server_TreeNode* treeRoot);

#endif


