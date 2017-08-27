//server struct
#include<time.h>
#include <dirent.h>
#include "google.h"

extern CloudFile *FolderCurrent;
extern CloudFile *FolderTmp;
extern CloudFile *FolderTmp_One;
extern CloudFile *FileList_one;
extern CloudFile *FileTail_one;
extern CloudFile *FileTmp_one;
extern CloudFile *TreeFolderList;
extern CloudFile *TreeFileList;
extern CloudFile *TreeFolderTail;
extern CloudFile *TreeFileTail;


/*Local item struct and function of every floor*/
typedef struct LOCALFOLDER{
    char *path;
    char name[MAX_CONTENT];
    struct LOCALFOLDER *next;
}LocalFolder;

typedef struct LOCALFILE{
    char *path;
    char name[MAX_CONTENT];
    char creationtime[MINSIZE];
    char lastaccesstime[MINSIZE];
    char lastwritetime[MINSIZE];
    long long size;
    struct LOCALFILE *next;
}LocalFile;



typedef struct LOCAL
{
    int foldernumber;
    int filenumber;
    LocalFolder *folderlist;
    LocalFile *filelist;
}Local;



Server_TreeNode *create_server_treeroot();
int browse_to_tree(char *parenthref, char * id, Server_TreeNode *node, int index);
void free_server_tree(Server_TreeNode *node);
void SearchServerTree(Server_TreeNode* treeRoot);
int create_sync_list();
int initMyLocalFolder(Server_TreeNode *servertreenode,int index);
char *serverpath_to_localpath(char *from_serverpath,int index);
int is_local_space_enough(CloudFile *do_file,int index);
long long int get_local_freespace(int index);
int test_if_dir(const char *dir);
Local *Find_Floor_Dir(const char *path);
void free_LocalFolder_item(LocalFolder *head);
void free_LocalFile_item(LocalFile *head);
void free_localfloor_node(Local *local);
int sync_local_with_server_init(Server_TreeNode *treenode,Browse *perform_br,Local *perform_lo,int index);
char *localpath_to_serverpath(char *from_localpath,int index);
int path_to_ppath(char *path, char *path1);
int ChangeFile_modtime(char *filepath,time_t servermodtime);
void free_CloudFile_item(CloudFile *head);
void free_cloudfile(CloudFile *head);
int upload_serverlist(Server_TreeNode *treenode,Browse *perform_br, char *localfoldertmp,int index, char *newfolderid);
CloudFile *get_CloudFile_node(Server_TreeNode* treeRoot,const char *dofile_href,int a);
