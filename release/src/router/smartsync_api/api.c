
#include "data.h"
#include "google.h"


int s_delete(char *herf,int index, char *delete_ids, int type)
{
    DEBUG("~~~~~~~~~!!smartsync_delete,type = %d~~~~~~~~~\n",type);
	switch(type)
	{
    //case 0:
        //removeEntry(herf, index, parentID,isencrpted,pid);
        //break;
    //case 2:
        //Delete(herf,index);
        //break;
    //case 3:
        //api_delete(herf, index);
        //break;
	case 6: 
		g_delete(herf, index,  delete_ids);
		break;
	default:
		break;

	}
}

int s_download(CloudFile *filetmp, char *fullname,char *filename,int index,int type)
{
    DEBUG("~~~~~~~~~!!smartsync_download,type = %d~~~~~~~~~\n",type);
    switch(type)
    {
        case 6:
            g_download(filetmp, fullname,  filename,index);
            break;
        default:
            break;
    }
}

int s_upload_file(char *filename,char *serverpath,int flag,int index,int type)
{
    DEBUG("~~~~~~~~~!!smartsync_upload_file,type = %d~~~~~~~~~\n",type);
    DEBUG("s_upload_file,filename=%s\n",filename);
    int status = 0;
    switch(type)
    {
        case 6:
            status = g_upload_file(filename, serverpath,  flag,index);
            break;
        default:
            break;
    }
    return status;
}

int s_browse_to_tree(char *parenthref, char *id, Server_TreeNode *node, int index,int type)
{
    DEBUG("~~~~~~~~~!!smartsync_browse_to_tree,type = %d~~~~~~~~~\n",type);
    int status = 0;
    switch(type)
    {
        case 6:
            status = g_browse_to_tree(parenthref, id,  node,index);
            break;
        default:
            break;
    }
    return status;
}

int s_move(char *oldname,char *newname,int index,int is_changed_time,char *newname_r,int type)
{
    DEBUG("~~~~~~~~~!!smartsync_move,type = %d~~~~~~~~~\n",type);
    switch(type)
    {
        case 6:
            g_move(oldname, newname,  index,is_changed_time,newname_r);
            break;
        default:
            break;
    }
}

int s_create_folder(char *localpath,char *foldername,char *newfolderid, int index, int i,int type)
{
    int status = 0;
    DEBUG("~~~~~~~~~!!smartsync_create_folder,type = %d~~~~~~~~~\n",type);
    switch(type)
    {
        case 6:
            status = g_create_folder(localpath,foldername, newfolderid,  index,  i);
            break;
        default:
            break;
    }
    return status;
}
