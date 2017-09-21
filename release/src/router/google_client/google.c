#include<stdio.h>
#include<stdlib.h>
#include"data.h"

#define MYPORT 3573
#define INOTIFY_PORT 3678
#define BACKLOG 100 /* max listen num*/
int no_config;
extern int exit_loop = 0;
int stop_progress;
char username[256];
char password[256];
extern char rootid[];
extern char delete_id[];

int is_renamed = 1;
extern int access_token_expired;

extern double start_time;

#define MAXSIZE 512

int set_iptables(int flag)
{
    int rc = -1;
    char cmd[1024]={0};
#ifdef I686
#else
    sprintf(cmd, "iptables -D INPUT -p tcp -m tcp --dport %d -j DROP", MYPORT);
    rc = system(cmd);
    sprintf(cmd, "iptables -D INPUT -p tcp -m tcp -s 127.0.0.1 --dport %d -j ACCEPT", MYPORT);
    rc = system(cmd);
#endif
    if(flag)
    {
#ifdef I686
	rc = 0;
#else
        sprintf(cmd, "iptables -I INPUT -p tcp -m tcp --dport %d -j DROP", MYPORT);
        rc = system(cmd);
        sprintf(cmd, "iptables -I INPUT -p tcp -m tcp -s 127.0.0.1 --dport %d -j ACCEPT", MYPORT);
        rc = system(cmd);
#endif
    }

    if(rc == 0)
        return 0;
    else
        return 1;
}

int write_rootca()
{
    FILE *fp;
    fp = fopen(CA_INFO_FILE,"w");
    if(NULL == fp)
    {
        printf("open rootca  file %s fail\n",CA_INFO_FILE);
        return -1;
    }
    fprintf(fp,"%s","-----BEGIN CERTIFICATE-----\n"
            "MIIDVDCCAjygAwIBAgIDAjRWMA0GCSqGSIb3DQEBBQUAMEIxCzAJBgNVBAYTAlVT\n"
            "MRYwFAYDVQQKEw1HZW9UcnVzdCBJbmMuMRswGQYDVQQDExJHZW9UcnVzdCBHbG9i\n"
            "YWwgQ0EwHhcNMDIwNTIxMDQwMDAwWhcNMjIwNTIxMDQwMDAwWjBCMQswCQYDVQQG\n"
            "EwJVUzEWMBQGA1UEChMNR2VvVHJ1c3QgSW5jLjEbMBkGA1UEAxMSR2VvVHJ1c3Qg\n"
            "R2xvYmFsIENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA2swYYzD9\n"
            "9BcjGlZ+W988bDjkcbd4kdS8odhM+KhDtgPpTSEHCIjaWC9mOSm9BXiLnTjoBbdq\n"
            "fnGk5sRgprDvgOSJKA+eJdbtg/OtppHHmMlCGDUUna2YRpIuT8rxh0PBFpVXLVDv\n"
            "iS2Aelet8u5fa9IAjbkU+BQVNdnARqN7csiRv8lVK83Qlz6cJmTM386DGXHKTubU\n"
            "1XupGc1V3sjs0l44U+VcT4wt/lAjNvxm5suOpDkZALeVAjmRCw7+OC7RHQWa9k0+\n"
            "bw8HHa8sHo9gOeL6NlMTOdReJivbPagUvTLrGAMoUgRx5aszPeE4uwc2hGKceeoW\n"
            "MPRfwCvocWvk+QIDAQABo1MwUTAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBTA\n"
            "ephojYn7qwVkDBF9qn1luMrMTjAfBgNVHSMEGDAWgBTAephojYn7qwVkDBF9qn1l\n"
            "uMrMTjANBgkqhkiG9w0BAQUFAAOCAQEANeMpauUvXVSOKVCUn5kaFOSPeCpilKIn\n"
            "Z57QzxpeR+nBsqTP3UEaBU6bS+5Kb1VSsyShNwrrZHYqLizz/Tt1kL/6cdjHPTfS\n"
            "tQWVYrmm3ok9Nns4d0iXrKYgjy6myQzCsplFAMfOEVEiIuCl6rYVSAlk6l5PdPcF\n"
            "PseKUgzbFbS9bZvlxrFUaKnjaZC2mqUPuLk/IH2uSrW4nOQdtqvmlKXBx4Ot2/Un\n"
            "hw4EbNX/3aBd7YdStysVAq45pmp06drE57xNNB6pXE0zX5IJL4hmXXeXxx12E6nV\n"
            "5fEWCRE11azbJHFwLJhWC9kXtNHjUStedejV0NxPNO3CBWaAocvmMw==\n"
            "-----END CERTIFICATE-----\n");
    fclose(fp);
    return 0;
}

int write_trans_excep_log(char *fullname,int type,char *msg)
{
    FILE *fp = 0;
    char ctype[16] = {0};

    if(type == 1)
        strcpy(ctype,"Error");
    else if(type == 2)
        strcpy(ctype,"Info");
    else if(type == 3)
        strcpy(ctype,"Warning");

    if(access(trans_excep_file,0) == 0)
        fp = fopen(trans_excep_file,"a");
    else
        fp = fopen(trans_excep_file,"w");


    if(fp == NULL)
    {
        printf("open %s fail\n",trans_excep_file);
        return -1;
    }

    fprintf(fp,"TYPE:%s\nUSERNAME:%s\nFILENAME:%s\nMESSAGE:%s\n",ctype,username,fullname,msg);
    fclose(fp);
    return 0;
}

//int write_conflict_log(char *prename, char *conflict_name,int index)
//{
//    //wd_DEBUG("oldname=%s,newname=%s\n",prename,conflict_name);
//    FILE *fp;

//    if(access(g_pSyncList[index]->conflict_file,F_OK))
//    {
//        fp = fopen(g_pSyncList[index]->conflict_file,"w");
//    }
//    else
//    {
//        fp = fopen(g_pSyncList[index]->conflict_file,"a");
//    }

//     if(NULL == fp)
//     {
//         printf("open %s failed\n",g_pSyncList[index]->conflict_file);
//         return -1;
//     }

//     fprintf(fp,"%s is download from server,%s is local file and rename from %s\n",prename,conflict_name,prename);
//     fclose(fp);

//     return 0;
//}


void buffer_free(char *b) {
        if (!b) return;

        free(b);
        b = NULL;
}

/*char *cJSON_parse_name2(cJSON *json)
{
    char *name;
    char *parents_idd;
    cJSON *item_title, *item_parents, *parents_child, *parents_id;
    item_title = cJSON_GetObjectItem( json , "title");
    if(item_title == NULL){
        printf("fail to get item_title\n");
    }
    if( item_title->type == cJSON_String ){
        name=(char *)malloc(sizeof(char)*(strlen(item_title->valuestring)+1));
        strcpy(name,item_title->valuestring);
    }

    item_parents = cJSON_GetObjectItem( json , "parents");
    if(item_parents == NULL){
        printf("fail to get item_parents\n");
       // continue;
    }
    parents_child=cJSON_GetArrayItem(item_parents,0);
    if(parents_child == NULL){
        printf("fail to get parents_child\n");
    }
    parents_id = cJSON_GetObjectItem( parents_child , "id");
    if(parents_child == NULL){
        printf("fail to get parents_id\n");
    }
    if( parents_id->type == cJSON_String ){
        parents_idd=(char *)malloc(sizeof(char)*(strlen(parents_id->valuestring)+1));
        strcpy(parents_idd,parents_id->valuestring);
        //printf("FolderTmp%d->parents_id=%s\r\n",i, FolderTmp->parents_id);
    }

}*/



/*
 FileTail_one is not only file list ,it is also folder list
*/


//int api_metadata(char *phref,cJSON *(*cmd_data)(char *filename))

/*
oldname & newname both server path
*/


/*
fullname=>local
filename=>server
*/


//int get_file_size(char *filename)
//{
//    int file_len = 0;
//    int fd = 0;

//    fd = open(filename, O_RDONLY);
//    if(fd < 0)
//    {
//        perror("open");
//        /*
//         fix below bug:
//            1.create file a;
//            2.rename a-->b;
//            final open() 'a' failed ,will exit();
//        */
//        //exit(-1);
//        return -1;
//    }

//    file_len = lseek(fd, 0, SEEK_END);
//    //wd_DEBUG("file_len is %d\n",file_len);
//    if(file_len < 0)
//    {
//        perror("lseek");
//        exit(-1);
//    }
//    close(fd);
//    return file_len;
//}


/*
 lcoalpath=>local name
 folderpath=>server path
*/
/*int api_create_folder(char *localpath,char *foldername,char *newfolderid)
{

    wd_DEBUG("****************create_server_folder****************\n");
    int flag = 0;


    if(access(localpath,0) != 0)
    {

        wd_DEBUG("Local has no %s\n",localpath);

        return LOCAL_FILE_LOST;
    }
    char *folderid = NULL;



    CURL *curl;
    CURLcode res;
    FILE *fp;

    char *postdata[256] = {0};
    DEBUG("before strrchr, foldername = %s\n", foldername);
    char *name = strrchr(foldername, '/');
    DEBUG("after strrchr, name = %s\n", name);
    name ++;
    DEBUG("after name++, name = %s\n", name);  
        DEBUG("treenode isn't NULL!\n");

    //DEBUG("pastdata = %s\n", postdata);
    long long int bodydata_size=strlen(postdata);
    FILE *fp_hd;



    char *foldername_tmp=oauth_url_escape(foldername);
    char *data=(char *)malloc(sizeof(char)*(strlen(foldername_tmp)+32));
    memset(data,0,strlen(foldername_tmp)+32);
    sprintf(data,"%s%s","root=google&path=",foldername_tmp);
    free(foldername_tmp);
    char *header;
    header = (char *)malloc(128);
    memset(header, 0, sizeof(header));
    sprintf(header, "Authorization: Bearer %s", auth->oauth_token);
    struct curl_slist *headerlist=NULL;
    do
    {
        flag = 0;
        headerlist = NULL;
        folderid = path_to_parents_id(foldername, 0);
        sprintf(postdata,"\n{\n'title': '%s',\n'parents': [{'id':'%s'}],\n'mimeType': 'application/vnd.google-apps.folder'\n}", name, folderid);
        DEBUG("before curl_easy_init\n");
        curl=curl_easy_init();
        DEBUG("after curl_easy_init\n");
        headerlist=curl_slist_append(headerlist,header);
        DEBUG("curl_slist_append1\n");
        headerlist=curl_slist_append(headerlist,"Expect: ");
        headerlist=curl_slist_append(headerlist, "Content-Type: application/json" );
        if(curl){
                DEBUG("if curl\n");
                fp=fopen(Con(TMP_R,create_folder.txt),"w");
                fp_hd=fopen(Con(TMP_R,create_folder_header.txt),"w+");

                curl_easy_setopt(curl, CURLOPT_USERAGENT, "asus-google-drive/0.1");
                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
                curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
                //curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);

                curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,2L);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
                curl_easy_setopt(curl, CURLOPT_CAINFO, CA_INFO_FILE);
                curl_easy_setopt(curl,CURLOPT_URL,"https://www.googleapis.com/drive/v2/files");
                CURL_DEBUG;
                curl_easy_setopt(curl,CURLOPT_HTTPHEADER,headerlist);
                DEBUG("*postdata = %s\n", postdata);
                curl_easy_setopt(curl,CURLOPT_POSTFIELDS,postdata);
                curl_easy_setopt(curl,CURLOPT_POSTFIELDSIZE_LARGE,bodydata_size);
                curl_easy_setopt(curl,CURLOPT_POST,1L);

                //curl_easy_setopt(curl,CURLOPT_TIMEOUT,90);
                curl_easy_setopt(curl,CURLOPT_WRITEDATA,fp);
                curl_easy_setopt(curl,CURLOPT_WRITEHEADER,fp_hd);

                DEBUG("curl_easy_perform:%s\n", name);
                res=curl_easy_perform(curl);
                fclose(fp);
                //fclose(fp_hd);
                curl_slist_free_all(headerlist);
                if(res!=0){
                    fclose(fp_hd);
                    free(header);
                    free(data);
                    wd_DEBUG("create %s failed,is [%d] !\n",foldername,res);
                    curl_easy_cleanup(curl);
                    return res;
                }
                else
                {

                    rewind(fp_hd);
                    char tmp_[256]="\0";

                    fgets(tmp_,sizeof(tmp_),fp_hd);
                    DEBUG("api_create_folder,tmp_=%s\n", tmp_);
                    if(strstr(tmp_,"401") != NULL)
                    {
                            DEBUG("401,tmp_=%s\n", tmp_);
                            write_log(S_ERROR,"Access token has expired,please re-authenticate!","",0);
                            exit_loop = 1;
                            access_token_expired = 1;
                            fclose(fp_hd);
                            free(header);
                            free(data);
                            curl_easy_cleanup(curl);
                            return -1;
                    }
                    if(strstr(tmp_,"404") != NULL)
                    {
                            DEBUG("404,tmp_=%s\n", tmp_);
                            char path[1024] = {0};
                            char parentfolder[1024] = {0};
                            DEBUG("3897\n");
                            name = strrchr(foldername, '/');
                            DEBUG("name = %s\n", name);
                            strncpy(parentfolder,foldername, strlen(foldername) - strlen(name));
                            DEBUG("parentfolder=%s\n", parentfolder);
                            name = strrchr(localpath, '/');
                            DEBUG("*name = %s\n", name);
                            strncpy(path, localpath, strlen(localpath) - strlen(name));
                            DEBUG("path=%s\n", path);
                            DEBUG("api_creater_folder 404not found,now creater parent folder!\n");
                            int status = 0;
                            status = api_create_folder(path, parentfolder, newfolderid);
                            if(status != 0)
                            {
                                fclose(fp_hd);
                                free(header);
                                free(data);
                                curl_easy_cleanup(curl);
                                return -1;
                            }
                            else
                           {
                                DEBUG("status = %d\n", status);
                                flag = 1;
                            }
                    }
                    fclose(fp_hd);
                }


            }
        DEBUG("after if(curl)\n");
        DEBUG("f@@@lag = %d\n", flag);
         }while(flag == 1);
        DEBUG("%s:breake while\n", name);
        if (curl)
        {
            //deal big or small case sentive problem,will return 403
            if(parse_create_folder(Con(TMP_R,create_folder_header.txt)))
            {
#ifndef MULTI_PATH
                char *server_conflcit_name=get_server_exist(foldername,0);
                if(server_conflcit_name)
                {
                    char *local_conflcit_name = serverpath_to_localpath(server_conflcit_name,0);
                    char *g_newname = get_confilicted_name_case(local_conflcit_name,1);
                    my_free(local_conflcit_name);

                    updata_socket_list(localpath,g_newname,0);

                    if(access(localpath,0) == 0)
                    {
                        add_action_item("rename",g_newname,g_pSyncList[0]->server_action_list);
                        rename(localpath,g_newname);
                    }

                    char *s_newname = localpath_to_serverpath(g_newname,0);
                    int status=0;
                    char newfolderid1[32] = {0};
                    do
                    {
                        DEBUG("create folder1\n");
                        status = api_create_folder(g_newname,s_newname, newfolderid1);
                    }while(status != 0 && !exit_loop);
                    my_free(g_newname);
                    my_free(s_newname);
                }
#endif
            }
        }
    curl_easy_cleanup(curl);
    free(header);
    free(data);

    cJSON *json;
    json=dofile(Con(TMP_R,create_folder.txt));
    if(json)
    {
        cJSON *item_new_folder_id;

            item_new_folder_id = cJSON_GetObjectItem( json , "id");
            if(item_new_folder_id == NULL){
                printf("fail to get item_new_folder_id");
                //continue;
            }
            if( item_new_folder_id->type == cJSON_String ){
                strcpy(newfolderid,item_new_folder_id->valuestring);
                printf("!!!newfolderid:%s\n",  newfolderid);
            }
    }
    cJSON_Delete(json);
    DEBUG("api_creater_folder end,return 0\n");
    return 0;
}
*/

int parse_config_mutidir(char *path,struct asus_config *config)
{
    FILE *fp;
    wd_DEBUG("filename:%s\n",path);
    char buf[512];
    char *buffer = buf;
    char *p;
    int i = 0;
    int k = 0;
    int j = 0;

    memset(buf,0,sizeof(buf));

    if (access(path,0) == 0)
    {
        if(( fp = fopen(path,"r"))==NULL)
        {
            fprintf(stderr,"read Cloud error");
            return -1;
        }
        else
        {
            while(fgets(buffer,512,fp)!=NULL)
            {
                if(buffer[strlen(buffer)-1] == '\n')
                {
                    buffer[strlen(buffer)-1] = '\0';
                }
                p=buffer;
                printf("p is %s,outer is %s\n",p,buffer);
                switch (i)
                {
                case 0 :
                    config->type = atoi(p);
                    break;
                case 1:
                    config->enable = atoi(p);
                    break;
                case 2:
                    strncpy(config->user,p,strlen(p));
                    if(config->user[strlen(config->user)-1]=='\n')
                    {
                        config->user[strlen(config->user)-1]='\0';
                    }
                    break;
                    case 3:

                    strncpy(config->pwd,p,strlen(p));
                    if(config->pwd[strlen(config->pwd)-1]=='\n')
                    {
                        config->pwd[strlen(config->pwd)-1]='\0';
                    }
                    break;
                    case 4:
                    config->dir_number = atoi(p);
                    config->prule = (struct asus_rule **)malloc(sizeof(struct asus_rule*)*config->dir_number);
                    if(NULL == config->prule)
                    {
                        return -1;
                    }
                    break;
                    default:
                    k = j / 2 ;
                    if(i % 2 == 1)
                    {
                        config->prule[k] = (struct asus_rule*)malloc(sizeof(struct asus_rule));
                        config->prule[k]->rule = atoi(p);
                    }
                    else
                    {
                        char *dp;
                        strcpy(config->prule[k]->path,p);
                        dp=strrchr(config->prule[k]->path,'/');
                        snprintf(config->prule[k]->base_path,strlen(config->prule[k]->path)-strlen(dp)+1,"%s",config->prule[k]->path);
                        config->prule[k]->base_path_len=strlen(config->prule[k]->path)-strlen(dp);
                        strcpy(config->prule[k]->rooturl,dp);
                        config->prule[k]->rooturl_len=strlen(dp);
                    }
                    j++;
                    break;
                }
                i++;
            }

            fclose(fp);
            return 0;
        }
    }
    else
        return -1;

}
void init_globar_var()
{
#if TOKENFILE
    sync_disk_removed = 0;
    disk_change = 0;
#endif
    upload_chunk = NULL;
    access_token_expired = 0;
    exit_loop = 0;
    stop_progress = 0;
    finished_initial=0;
    sprintf(log_path,"/tmp/smartsync/.logs");
    my_mkdir("/tmp/smartsync");
    my_mkdir("/tmp/smartsync/google");
    my_mkdir("/tmp/smartsync/google/config");
    my_mkdir("/tmp/smartsync/google/script");
    my_mkdir("/tmp/smartsync/google/temp");
    my_mkdir("/tmp/smartsync/google/cert");
#ifdef NVRAM_
    my_mkdir("/tmp/smartsync/script");
#endif
    my_mkdir(TMP_R);
    my_mkdir("/tmp/smartsync/.logs");
    sprintf(general_log,"%s/google",log_path);
    sprintf(trans_excep_file,"%s/google_errlog",log_path);

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex_socket, NULL);
    pthread_mutex_init(&mutex_receve_socket, NULL);
    pthread_mutex_init(&mutex_log, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_cond_init(&cond_socket, NULL);
    pthread_cond_init(&cond_log, NULL);
    memset(&asus_cfg,'\0',sizeof(struct asus_config));
#if TOKENFILE
    memset(&asus_cfg_stop,0,sizeof(struct asus_config));
    if(initial_tokenfile_info_data(&tokenfile_info_start) == NULL)
    {
        return;
    }
#endif
}
void read_config()
{
#if TOKENFILE
#ifdef NVRAM_
    DEBUG("convert_nvram_to_file_mutidir1\n");
    if(convert_nvram_to_file_mutidir(CONFIG_PATH,&asus_cfg) == -1)
    {
        wd_DEBUG("convert_nvram_to_file fail\n");
        return;
    }
#else
#ifndef WIN_32
    if(create_webdav_conf_file(&asus_cfg) == -1)
    {
        wd_DEBUG("create_google_conf_file fail\n");
        return;
    }
#endif
#endif
#endif
    int i;
    if(parse_config_mutidir(CONFIG_PATH,&asus_cfg) == -1)
    {
        wd_DEBUG("parse_config_mutidir fail\n");
        no_config = 1;
        return;
    }
#ifdef SMARTSYNCIPK
if(asus_cfg.type != 6 )
    {
        wd_DEBUG("asus_cfg.type=%d,type!=6\n",asus_cfg.type);
        no_config = 1;
        return;
    }
#endif
    wd_DEBUG("asus_cfg.type=%d\nasus_cfg.user=%s\nasus_cfg.pwd=%s\nasus_cfg.enable=%d\nasus_cfg.dir_number=%d\n",asus_cfg.type,asus_cfg.user,asus_cfg.pwd,
             asus_cfg.enable,asus_cfg.dir_number);

    for(i=0;i<asus_cfg.dir_number;i++)
    {
        write_log(S_INITIAL,"","",i);
        wd_DEBUG("rule is %d,path is %s,rooturl is %s\n",asus_cfg.prule[i]->rule,asus_cfg.prule[i]->path,asus_cfg.prule[i]->rooturl);
    }
    strcpy(username,asus_cfg.user);
    strcpy(password,asus_cfg.pwd);
    wd_DEBUG("222\n");
    if( strlen(username) == 0 )
    {

        wd_DEBUG("username is blank ,please input your username and passwrod\n");

        no_config = 1;
    }

    no_config = 0 ;
    exit_loop = 0;

}

int sync_initial_again(int index)
{
    int i,status,ret;

    i = index;

    ret = 1;
    if(exit_loop == 0)
    {
#ifdef MULTI_PATH
            if(api_metadata_test_dir(rootid,dofile)!=0)
            {
                wd_DEBUG("\nserver sync path is not exist,need create\n");
                s_create_folder(asus_cfg.prule[i]->base_path,asus_cfg.prule[i]->rooturl,6);
            }
#endif
        free_server_tree(g_pSyncList[i]->ServerRootNode);
        g_pSyncList[i]->ServerRootNode = NULL;

        g_pSyncList[i]->ServerRootNode = create_server_treeroot();
#ifdef MULTI_PATH
        DEBUG("browse_to_tree3\n");
        status = s_browse_to_tree(asus_cfg.prule[i]->rooturl,g_pSyncList[i]->ServerRootNode,6);
#else
        DEBUG("browse_to_tree4\n");
        status = s_browse_to_tree("/", rootid,g_pSyncList[i]->ServerRootNode, i,6);
#endif

        usleep(1000*200);
        if(status != 0)
            return status;

        if(exit_loop == 0)
        {

            if(test_if_dir_empty(asus_cfg.prule[i]->path))
            {

                g_pSyncList[i]->init_completed=1;
                g_pSyncList[i]->OldServerRootNode = g_pSyncList[i]->ServerRootNode;
                ret = 0;
            }
            else
            {

                wd_DEBUG("######## init sync folder,please wait!......#######\n");

                if(g_pSyncList[i]->ServerRootNode->Child != NULL)
                {
                    ret=sync_local_with_server(g_pSyncList[i]->ServerRootNode->Child,sync_local_with_server_init,i);
                }

                wd_DEBUG("######## init sync folder end#######\n");

                if(ret != 0 )
                    return ret;
                g_pSyncList[i]->init_completed = 1;
                g_pSyncList[i]->OldServerRootNode = g_pSyncList[i]->ServerRootNode;
            }
        }
    }
    if(ret == 0)
        write_log(S_SYNC,"","",i);

    return ret;
}

int sync_initial()
{
    DEBUG("***************sync_initial start***********************\n");
    int i,status,ret;
    for(i=0;i<asus_cfg.dir_number;i++)
    {
        DEBUG("the %dth dir_number,exit_loop = %d\n", i, exit_loop);
        if(exit_loop)
            break;
#if TOKENFILE
        if(disk_change)
        {
            check_disk_change();
        }
        if(g_pSyncList[i]->sync_disk_exist == 0)
        {
            write_log(S_ERROR,"Sync disk unplug!","",i);
            continue;
        }
#endif
        if(g_pSyncList[i]->init_completed)
            continue;

        ret = 1;
        if(exit_loop == 0)
        {
#ifdef MULTI_PATH
            if(api_metadata_test_dir(rootid,dofile)!=0)
            {
                wd_DEBUG("\nserver sync path is not exist,need create\n");
                s_create_folder(asus_cfg.prule[i]->base_path,asus_cfg.prule[i]->rooturl,6);
            }
#endif
            DEBUG("sync_initial,free_server_tree\n");
            free_server_tree(g_pSyncList[i]->ServerRootNode);
            g_pSyncList[i]->ServerRootNode = NULL;

            g_pSyncList[i]->ServerRootNode = create_server_treeroot();
#ifdef MULTI_PATH
            DEBUG("browse_to_tree5\n");
            status = s_browse_to_tree(asus_cfg.prule[i]->rooturl,g_pSyncList[i]->ServerRootNode,6);
#else
            DEBUG("browse_to_tree6\n");
            status = s_browse_to_tree("/", rootid,g_pSyncList[i]->ServerRootNode, i,6);
#endif
            usleep(1000*200);
            if(status != 0)
            {
                DEBUG("browse_to_tree end, status=%d", status);
                continue;
            }

            if(exit_loop == 0)
            {

                if(test_if_dir_empty(asus_cfg.prule[i]->path))
                {
                    if(asus_cfg.prule[i]->rule != 2)
                    {

                        wd_DEBUG("######## init sync empty folder,please wait......#######\n");

                        if(g_pSyncList[i]->ServerRootNode->Child != NULL)
                        {
                            ret=initMyLocalFolder(g_pSyncList[i]->ServerRootNode->Child,i);
                            if(ret != 0 )
                                continue;
                            g_pSyncList[i]->init_completed=1;
                            g_pSyncList[i]->OldServerRootNode = g_pSyncList[i]->ServerRootNode;
                        }

                        wd_DEBUG("######## init sync empty folder end#######\n");

                    }
                    else
                    {
                        g_pSyncList[i]->init_completed=1;
                        g_pSyncList[i]->OldServerRootNode = g_pSyncList[i]->ServerRootNode;
                        /*
                         fix upload only rule local is empty,UI log is still show "initial"
                        */
                        ret = 0;
                    }
                }
                else
                {

                    wd_DEBUG("######## init sync folder,please wait......#######\n");

                    if(g_pSyncList[i]->ServerRootNode->Child != NULL)
                    {
                        DEBUG("1sync_local_with_server\n");
                        ret=sync_local_with_server(g_pSyncList[i]->ServerRootNode->Child,sync_local_with_server_init,i);
                        DEBUG("1sync_local_with_server end...\n");
                    }

                    wd_DEBUG("######## init sync folder end#######\n");

                    if(ret != 0 )
                        continue;
                    g_pSyncList[i]->init_completed = 1;
                    g_pSyncList[i]->OldServerRootNode = g_pSyncList[i]->ServerRootNode;
                }
            }
        }
        if(ret == 0)
            write_log(S_SYNC,"","",i);

    }

    DEBUG("loop end dir_number, sync_initial return: %d\n", ret);
    return ret;
}
int sync_local_with_server(Server_TreeNode *treenode,int(* sync_fun)(Server_TreeNode *,Browse*,Local*,int),int index)
{
    if(treenode->parenthref == NULL)
        return 0;
    Local *localnode;
    char *localpath;
    int ret=0;
#ifdef MULTI_PATH
    localpath=serverpath_to_localpath(treenode->parenthref,index);
#else
    if(strcmp(treenode->parenthref,"/") == 0)  //yes
    {
        localpath = (char *)malloc(sizeof(char)*(asus_cfg.prule[index]->base_path_len+asus_cfg.prule[index]->rooturl_len+2));
        memset(localpath,'\0',asus_cfg.prule[index]->base_path_len+asus_cfg.prule[index]->rooturl_len+2);
        sprintf(localpath,"%s%s",asus_cfg.prule[index]->base_path,asus_cfg.prule[index]->rooturl);
    }
    else
    {
        localpath=serverpath_to_localpath(treenode->parenthref,index);
    }
#endif

    localnode=Find_Floor_Dir(localpath);
    free(localpath);

    if(localnode != NULL)
    {
        ret=sync_fun(treenode,treenode->browse,localnode,index);
        if(ret != 0 && ret != SERVER_SPACE_NOT_ENOUGH
           && ret != LOCAL_FILE_LOST)
        {
            free_localfloor_node(localnode);
            return ret;
        }
        free_localfloor_node(localnode);
    }
    if(treenode->Child != NULL && !exit_loop)
    {
        ret =sync_local_with_server(treenode->Child,sync_fun,index);
        if(ret != 0 && ret != SERVER_SPACE_NOT_ENOUGH
           && ret != LOCAL_FILE_LOST)
        {
            return ret;
        }
    }
    if(treenode->NextBrother != NULL && !exit_loop)
    {
        ret = sync_local_with_server(treenode->NextBrother,sync_fun,index);
        if(ret != 0 && ret != SERVER_SPACE_NOT_ENOUGH
           && ret != LOCAL_FILE_LOST)
        {
            return ret;
        }
    }
    return ret;
}
int sync_local_with_server_init(Server_TreeNode *treenode,Browse *perform_br,Local *perform_lo,int index)
{
    wd_DEBUG("perform_br->foldernumber = %d,perform_br->filenumber = %d,\n",perform_br->foldernumber,perform_br->filenumber);
    wd_DEBUG("perform_lo->foldernumber = %d,perform_lo->filenumber = %d,\n",perform_lo->foldernumber,perform_lo->filenumber);
    if(perform_br == NULL || perform_lo == NULL)
    {
        return 0;
    }
    int ret = 0;

    CloudFile *foldertmp=NULL;
    CloudFile *filetmp=NULL;
    LocalFolder *localfoldertmp;
    LocalFile *localfiletmp;
    if(perform_br->foldernumber > 0)
        foldertmp = perform_br->folderlist->next;
    if(perform_br->filenumber > 0)
        filetmp = perform_br->filelist->next;
    localfoldertmp=perform_lo->folderlist->next;
    localfiletmp=perform_lo->filelist->next;

    /****************handle files****************/
    if(perform_br->filenumber == 0 && perform_lo->filenumber !=0)
    {
        while(localfiletmp != NULL && !exit_loop)
        {
            if(asus_cfg.prule[index]->rule != 1)
            {
                if(wait_handle_socket(index))
                {
                    return HAVE_LOCAL_SOCKET;
                }
                char *serverpath=localpath_to_serverpath(localfiletmp->path,index);
                DEBUG("upload19\n");
                ret=s_upload_file(localfiletmp->path,serverpath,1,index,6);
                DEBUG("upload_file end,ret = %d\n", ret);
                if(ret == 0 || ret == SERVER_SPACE_NOT_ENOUGH || ret == LOCAL_FILE_LOST)
                {
                    if(ret == 0)
                    {
                        cJSON *json = dofile(Con(TMP_R,upload_chunk_commit.txt));
                        time_t mtime=cJSON_printf_mtime(json);
                        cJSON_Delete(json);
                        ChangeFile_modtime(localfiletmp->path,mtime);
                        free(serverpath);
                    }
                }
                else
                {
                    free(serverpath);
                    return ret;
                }
            }
            else
            {
                if(wait_handle_socket(index))
                {
                    return HAVE_LOCAL_SOCKET;
                }
                add_action_item("createfile",localfiletmp->path,g_pSyncList[index]->download_only_socket_head);
            }
            localfiletmp = localfiletmp->next;
        }
    }
    else if(perform_br->filenumber != 0 && perform_lo->filenumber ==0)
    {
        while(filetmp != NULL && !exit_loop)
        {
            if(asus_cfg.prule[index]->rule != 2)
            {
                if(is_local_space_enough(filetmp,index))
                {
                    if(wait_handle_socket(index))
                    {
                        return HAVE_LOCAL_SOCKET;
                    }
                    char *localpath= serverpath_to_localpath(filetmp->href,index);
                    add_action_item("createfile",localpath,g_pSyncList[index]->server_action_list);
                    DEBUG("api_download1\n");
                    ret=s_download(filetmp,localpath,filetmp->href,index,6);
                    if(ret == 0)
                    {
                        ChangeFile_modtime(localpath,filetmp->mtime);
                        free(localpath);
                    }
                    else
                    {
                        free(localpath);
                        return ret;
                    }

                }
                else
                {
                    write_log(S_ERROR,"local space is not enough!","",index);
                    add_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
                }
            }
            filetmp=filetmp->next;
        }
    }
    else if(perform_br->filenumber != 0 && perform_lo->filenumber !=0)
    {

        while(localfiletmp != NULL && !exit_loop) //is based by localfiletmp
        {
            int cmp=1;
            DEBUG("2localpath_to_serverpath,localfiletmp->path=%s\n", localfiletmp->path);// /tmp/mnt/TINA/aa/optware安装.doc
            char *serverpath=localpath_to_serverpath(localfiletmp->path,index);
            while(filetmp != NULL)
            {
                if((cmp=strcmp(serverpath,filetmp->href)) == 0)
                {
                    free(serverpath);
                    break;
                }
                else
                {
                    filetmp=filetmp->next;
                }
            }
            if(cmp != 0)
            {
                if(asus_cfg.prule[index]->rule != 1)
                {
                    if(wait_handle_socket(index))
                    {
                        buffer_free(serverpath);
                        return HAVE_LOCAL_SOCKET;
                    }

                    DEBUG("upload20\n");
                    ret=s_upload_file(localfiletmp->path,serverpath,1,index,6);
                    if(ret == 0 || ret == SERVER_SPACE_NOT_ENOUGH || ret == LOCAL_FILE_LOST)
                    {
                        if(ret == 0)
                        {
                            cJSON *json = dofile(Con(TMP_R,upload_chunk_commit.txt));
                            time_t mtime=cJSON_printf_mtime(json);
                            cJSON_Delete(json);
                            ChangeFile_modtime(localfiletmp->path,mtime);
                            free(serverpath);
                        }
                    }
                    else
                    {
                        free(serverpath);
                        return ret;
                    }
                }
                else
                {
                    buffer_free(serverpath);
                    if(wait_handle_socket(index))
                    {
                        return HAVE_LOCAL_SOCKET;
                    }
                    add_action_item("createfile",localfiletmp->path,g_pSyncList[index]->download_only_socket_head);
                }
            }
            else
            {
                if((ret = the_same_name_compare(localfiletmp,filetmp,index,1)) != 0)
                {
                    return ret;
                }
            }
            DEBUG("next1!\n");
            filetmp=perform_br->filelist->next;
            DEBUG("next2!\n");
            localfiletmp=localfiletmp->next;
            DEBUG("next3!\n");
        }
        filetmp=perform_br->filelist->next;
        localfiletmp=perform_lo->filelist->next;
        while(filetmp != NULL && !exit_loop) //is based by filetmp
        {
            int cmp=1;
            char *localpath=serverpath_to_localpath(filetmp->href,index);
            while(localfiletmp != NULL)
            {
                if((cmp=strcmp(localpath,localfiletmp->path)) == 0)
                {
                    free(localpath);
                    break;
                }
                else
                {
                    localfiletmp=localfiletmp->next;
                }
            }
            if(cmp != 0)
            {
                if(asus_cfg.prule[index]->rule != 2)
                {
                    if(is_local_space_enough(filetmp,index))
                    {
                        if(wait_handle_socket(index))
                        {
                            buffer_free(localpath);
                            return HAVE_LOCAL_SOCKET;
                        }

                        add_action_item("createfile",localpath,g_pSyncList[index]->server_action_list);
                        DEBUG("api_download2\n");
                        ret=s_download(filetmp, localpath,filetmp->href,index,6);
                        DEBUG("sync_local_with_server_init,api_download\n");
                        if(ret == 0)
                        {
                            ChangeFile_modtime(localpath,filetmp->mtime);
                            free(localpath);
                        }
                        else
                        {
                            free(localpath);
                            return ret;
                        }

                    }
                    else
                    {
                        buffer_free(localpath);
                        write_log(S_ERROR,"local space is not enough!","",index);
                        add_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
                    }
                }
            }
            localfiletmp=perform_lo->filelist->next;
            filetmp=filetmp->next;
        }
    }
    /*************handle folders**************/
    if(perform_br->foldernumber !=0 && perform_lo->foldernumber ==0)
    {
        if(asus_cfg.prule[index]->rule != 2)
        {
            while(foldertmp != NULL && !exit_loop)
            {
                if(wait_handle_socket(index))
                {
                    return HAVE_LOCAL_SOCKET;
                }
                char *localpath;
                localpath = serverpath_to_localpath(foldertmp->href,index);
                if(NULL == opendir(localpath))
                {
                    add_action_item("createfolder",localpath,g_pSyncList[index]->server_action_list);
                        mkdir(localpath,0777);
                }
                free(localpath);
                foldertmp = foldertmp->next;
            }
        }
    }
    else if(perform_br->foldernumber ==0 && perform_lo->foldernumber !=0)
    {
        while(localfoldertmp != NULL && !exit_loop)
        {
            if(asus_cfg.prule[index]->rule != 1)
            {
                char *serverpath;
                char newfolderid[32] = {0};
                serverpath = localpath_to_serverpath(localfoldertmp->path,index);
                DEBUG("create folder2\n");
                ret=s_create_folder(localfoldertmp->path,serverpath, newfolderid, index, 0,6);
                if(ret == 0)
                {
                    upload_serverlist(treenode,perform_br,localfoldertmp->path,index, newfolderid);
                    free(serverpath);
                }
                else
                {
                    free(serverpath);
                    return ret;
                }

            }
            else
            {
                if(wait_handle_socket(index))
                {
                    return HAVE_LOCAL_SOCKET;
                }
                add_action_item("createfolder",localfoldertmp->path,g_pSyncList[index]->download_only_socket_head);
            }
            localfoldertmp = localfoldertmp->next;
        }
    }
    else if(perform_br->foldernumber !=0 && perform_lo->foldernumber !=0)
    {
        while(foldertmp != NULL && !exit_loop)
        {
            int cmp=1;
            char *localpath;
            localpath = serverpath_to_localpath(foldertmp->href,index);
            while(localfoldertmp != NULL)
            {
                if((cmp = strcmp(localpath,localfoldertmp->path)) == 0)
                {
                    free(localpath);
                    break;
                }
                localfoldertmp=localfoldertmp->next;
            }
            if(cmp != 0)
            {
                if(asus_cfg.prule[index]->rule != 2)
                {
                    if(wait_handle_socket(index))
                    {
                        buffer_free(localpath);
                        return HAVE_LOCAL_SOCKET;
                    }
                    if(NULL == opendir(localpath))
                    {
                        add_action_item("createfolder",localpath,g_pSyncList[index]->server_action_list);
                            mkdir(localpath,0777);
                    }
                    free(localpath);
                }
                else
                    buffer_free(localpath);
            }
            foldertmp = foldertmp->next;
            localfoldertmp = perform_lo->folderlist->next;
        }


        foldertmp = perform_br->folderlist->next;
        localfoldertmp = perform_lo->folderlist->next;
        while(localfoldertmp != NULL && !exit_loop)
        {
            int cmp=1;
            char *serverpath;
            serverpath = localpath_to_serverpath(localfoldertmp->path,index);
            while(foldertmp != NULL)
            {
                if((cmp = strcmp(serverpath,foldertmp->href)) == 0)
                {
                    free(serverpath);
                    break;
                }
                foldertmp=foldertmp->next;
            }
            if(cmp != 0)
            {
                if(asus_cfg.prule[index]->rule != 1)
                {
                    if(wait_handle_socket(index))
                    {
                        buffer_free(serverpath);
                        return HAVE_LOCAL_SOCKET;
                    }
                    char newfolderid[32] = {0};
                    DEBUG("create folder3\n");
                    ret=s_create_folder(localfoldertmp->path,serverpath, newfolderid, index, 0,6);
                    if(ret == 0)
                    {
                        upload_serverlist(treenode,perform_br,localfoldertmp->path,index,newfolderid);
                        free(serverpath);
                    }
                    else
                    {
                        free(serverpath);
                        return ret;
                    }
                }
                else
                {
                    buffer_free(serverpath);
                    if(wait_handle_socket(index))
                    {
                        return HAVE_LOCAL_SOCKET;
                    }
                    add_action_item("createfolder",localfoldertmp->path,g_pSyncList[index]->download_only_socket_head);
                }
            }
            foldertmp = perform_br->folderlist->next;
            localfoldertmp = localfoldertmp->next;
        }
    }
    return ret;
}

int sync_local_with_server_perform(Server_TreeNode *treenode,Browse *perform_br,Local *perform_lo,int index)
{
    wd_DEBUG("sync_local_with_server_perform, 2parentf = %s\n",treenode->parenthref);
    wd_DEBUG("perform_br->foldernumber = %d,perform_br->filenumber = %d,\n",perform_br->foldernumber,perform_br->filenumber);
    wd_DEBUG("perform_lo->foldernumber = %d,perform_lo->filenumber = %d,\n",perform_lo->foldernumber,perform_lo->filenumber);
    if(perform_br == NULL || perform_lo == NULL)
    {
        return 0;
    }
    int ret = 0;

    CloudFile *foldertmp=NULL;
    CloudFile *filetmp=NULL;
    LocalFolder *localfoldertmp;
    LocalFile *localfiletmp;
    if(perform_br->foldernumber > 0)
        foldertmp = perform_br->folderlist->next;
    if(perform_br->filenumber > 0)
        filetmp = perform_br->filelist->next;
    localfoldertmp=perform_lo->folderlist->next;
    localfiletmp=perform_lo->filelist->next;

    DEBUG("/****************handle files****************/");
    if(perform_br->filenumber == 0 && perform_lo->filenumber !=0)
    {
        while(localfiletmp != NULL && exit_loop == 0)
        {
            if(asus_cfg.prule[index]->rule == 1)
            {
                action_item *item;
                item = get_action_item("download_only",localfiletmp->path,
                                       g_pSyncList[index]->download_only_socket_head,index);
                if(item != NULL)
                {
                    localfiletmp = localfiletmp->next;
                    continue;
                }
            }
            if(wait_handle_socket(index))
            {
                return HAVE_LOCAL_SOCKET;
            }
            add_action_item("remove",localfiletmp->path,g_pSyncList[index]->server_action_list);
            action_item *pp;
            pp = get_action_item("upload",localfiletmp->path,
                                 g_pSyncList[index]->up_space_not_enough_list,index);
            if(pp == NULL)
            {
                unlink(localfiletmp->path);
            }

            localfiletmp = localfiletmp->next;
        }
    }
    else if(perform_br->filenumber != 0 && perform_lo->filenumber ==0)
    {
        while(filetmp != NULL && !exit_loop)
        {
            if(wait_handle_socket(index))
            {
                return HAVE_LOCAL_SOCKET;
            }
            if(is_local_space_enough(filetmp,index))
            {
                action_item *item;
                item = get_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list,index);

                char *localpath= serverpath_to_localpath(filetmp->href,index);
                DEBUG("api_download3\n");
                ret=s_download(filetmp, localpath,filetmp->href,index,6);
                if(ret == 0)
                {
                    ChangeFile_modtime(localpath,filetmp->mtime);
                    if(item != NULL)
                    {
                        del_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
                    }
                    free(localpath);
                }
                else
                {
                    free(localpath);
                    return ret;
                }

            }
            else
            {
                write_log(S_ERROR,"local space is not enough!","",index);
                add_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
            }
            filetmp=filetmp->next;
        }
    }
    else if(perform_br->filenumber != 0 && perform_lo->filenumber !=0)
    {

        while(localfiletmp != NULL && !exit_loop) //is based by localfiletmp
        {
            int cmp=1;
            char *serverpath=localpath_to_serverpath(localfiletmp->path,index);
            while(filetmp != NULL)
            {
                if((cmp=strcmp(serverpath,filetmp->href)) == 0)
                {
                    free(serverpath);
                    break;
                }
                else
                {
                    filetmp=filetmp->next;
                }
            }
            if(cmp != 0)
            {
                buffer_free(serverpath);
                if(wait_handle_socket(index))
                {
                    return HAVE_LOCAL_SOCKET;
                }
                if(asus_cfg.prule[index]->rule == 1)
                {
                    action_item *item;
                    item = get_action_item("download_only",localfiletmp->path,
                                           g_pSyncList[index]->download_only_socket_head,index);
                    if(item != NULL)
                    {
                        filetmp = perform_br->filelist->next;
                        localfiletmp = localfiletmp->next;
                        continue;
                    }
                }

                add_action_item("remove",localfiletmp->path,g_pSyncList[index]->server_action_list);

                action_item *pp;
                pp = get_action_item("upload",localfiletmp->path,
                                     g_pSyncList[index]->up_space_not_enough_list,index);
                if(pp == NULL)
                {
                    unlink(localfiletmp->path);
                }

            }
            else
            {
                if((ret = the_same_name_compare(localfiletmp,filetmp,index,0)) != 0)
                {
                    return ret;
                }
            }
            filetmp=perform_br->filelist->next;
            localfiletmp=localfiletmp->next;
        }

        filetmp=perform_br->filelist->next;
        localfiletmp=perform_lo->filelist->next;
        while(filetmp != NULL && !exit_loop) //is based by filetmp
        {
            int cmp=1;
            char *localpath=serverpath_to_localpath(filetmp->href,index);
            while(localfiletmp != NULL)
            {
                if((cmp=strcmp(localpath,localfiletmp->path)) == 0)
                {
                    free(localpath);
                    break;
                }
                else
                {
                    localfiletmp=localfiletmp->next;
                }
            }
            if(cmp != 0)
            {
                if(wait_handle_socket(index))
                {
                    buffer_free(localpath);
                    return HAVE_LOCAL_SOCKET;
                }
                if(is_local_space_enough(filetmp,index))
                {
                    action_item *item;
                    item = get_action_item("download",
                                           filetmp->href,g_pSyncList[index]->unfinished_list,index);

                    add_action_item("createfile",localpath,g_pSyncList[index]->server_action_list);
                    DEBUG("api_download4\n");
                    ret=s_download(filetmp, localpath,filetmp->href,index,6);
                    if(ret == 0)
                    {
                        ChangeFile_modtime(localpath,filetmp->mtime);
                        if(item != NULL)
                        {
                            del_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
                        }
                        free(localpath);
                    }
                    else
                    {
                        free(localpath);
                        return ret;
                    }

                }
                else
                {
                    buffer_free(localpath);
                    write_log(S_ERROR,"local space is not enough!","",index);
                    add_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
                }

            }
            localfiletmp=perform_lo->filelist->next;
            filetmp=filetmp->next;
        }
    }
    /*************handle folders**************/
    if(perform_br->foldernumber !=0 && perform_lo->foldernumber ==0)
    {

        while(foldertmp != NULL && !exit_loop)
        {
            if(wait_handle_socket(index))
            {
                return HAVE_LOCAL_SOCKET;
            }
            char *localpath;
            localpath = serverpath_to_localpath(foldertmp->href,index);
            if(NULL == opendir(localpath))
            {
                int res;
                add_action_item("createfolder",localpath,g_pSyncList[index]->server_action_list);
                res = api_metadata_test_dir(foldertmp->id,dofile);
                if(res == 0)
                    mkdir(localpath,0777);
                else if(res == -1)
                {
                    free(localpath);
                    return res;
                }
                else
                {
                    wd_DEBUG("this %s had been deleted\n",foldertmp->href);
                }
            }
            free(localpath);
            foldertmp = foldertmp->next;
        }

    }
    else if(perform_br->foldernumber ==0 && perform_lo->foldernumber !=0)
    {
        while(localfoldertmp != NULL && !exit_loop)
        {
            if(asus_cfg.prule[index]->rule == 1)
            {
                action_item *item;
                item = get_action_item("download_only",
                                       localfoldertmp->path,g_pSyncList[index]->download_only_socket_head,index);
                if(item != NULL)
                {
                    localfoldertmp = localfoldertmp->next;
                    continue;
                }
            }
            if(wait_handle_socket(index))
            {
                return HAVE_LOCAL_SOCKET;
            }
            del_all_items(localfoldertmp->path,index);
            localfoldertmp = localfoldertmp->next;
        }
    }
    else if(perform_br->foldernumber !=0 && perform_lo->foldernumber !=0)
    {
        while(foldertmp != NULL && !exit_loop)
        {
            int cmp=1;
            char *localpath;
            localpath = serverpath_to_localpath(foldertmp->href,index);
            while(localfoldertmp != NULL)
            {
                if((cmp = strcmp(localpath,localfoldertmp->path)) == 0)
                {
                    free(localpath);
                    break;
                }
                localfoldertmp=localfoldertmp->next;
            }
            if(cmp != 0)
            {
                if(wait_handle_socket(index))
                {
                    buffer_free(localpath);
                    return HAVE_LOCAL_SOCKET;
                }
                if(NULL == opendir(localpath))
                {
                    add_action_item("createfolder",localpath,g_pSyncList[index]->server_action_list);
                    int res;
                    res = api_metadata_test_dir(foldertmp->id,dofile);
                    if(res == 0)
                        mkdir(localpath,0777);
                    else if(res == -1)
                    {
                        free(localpath);
                        return res;
                    }
                    else
                    {
                        wd_DEBUG("this %s had been deleted\n",foldertmp->href);
                    }
                }
                free(localpath);
            }
            foldertmp = foldertmp->next;
            localfoldertmp = perform_lo->folderlist->next;
        }


        foldertmp = perform_br->folderlist->next;
        localfoldertmp = perform_lo->folderlist->next;
        while(localfoldertmp != NULL && !exit_loop)
        {
            int cmp=1;
            char *serverpath;
            serverpath = localpath_to_serverpath(localfoldertmp->path,index);
            while(foldertmp != NULL)
            {
                if((cmp = strcmp(serverpath,foldertmp->href)) == 0)
                {
                    free(serverpath);
                    break;
                }
                foldertmp=foldertmp->next;
            }
            if(cmp != 0)
            {
                buffer_free(serverpath);
                if(asus_cfg.prule[index]->rule == 1)
                {
                    action_item *item;
                    item = get_action_item("download_only",
                                           localfoldertmp->path,g_pSyncList[index]->download_only_socket_head,index);
                    if(item != NULL)
                    {
                        foldertmp = perform_br->folderlist->next;
                        localfoldertmp = localfoldertmp->next;
                        continue;
                    }
                }
                if(wait_handle_socket(index))
                {
                    return HAVE_LOCAL_SOCKET;
                }
                del_all_items(localfoldertmp->path,index);
            }
            foldertmp = perform_br->folderlist->next;
            localfoldertmp = localfoldertmp->next;
        }
    }
    DEBUG("sync_local_with_server end, ret=%d\n", ret);
    return ret;
}

void del_all_items(char *dir,int index)
{
    struct dirent* ent = NULL;
    DIR *pDir;
    pDir=opendir(dir);

    if(pDir != NULL )
    {
        while (NULL != (ent=readdir(pDir)))
        {
            if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
                continue;

            char *fullname;
            size_t len;
            len = strlen(dir)+strlen(ent->d_name)+2;
            fullname = my_str_malloc(len);
            sprintf(fullname,"%s/%s",dir,ent->d_name);

            if(test_if_dir(fullname) == 1)
            {
                wait_handle_socket(index);
                del_all_items(fullname,index);
            }
            else
            {
                wait_handle_socket(index);
                add_action_item("remove",fullname,g_pSyncList[index]->server_action_list);
                remove(fullname);
            }

            free(fullname);

        }
        closedir(pDir);

        add_action_item("remove",dir,g_pSyncList[index]->server_action_list);
        remove(dir);
    }
    else
        wd_DEBUG("open %s fail \n",dir);
}





int re_create_folder(char *path,char *serverpath_1, int index)
{
    DEBUG("****re_create_folder start***\n");
    int exist = 0;
    char *pointer = NULL;
    int length = strlen(path);
    char path1[1024] = {0};
    char path2[1024] = {0};
    char serverpath_2[1024] = {0};
    char newfolderid[32] = {0};
    pointer = strrchr(path, '/');
    snprintf(path1, strlen(path) - strlen(pointer) + 1, path);
    pointer = strrchr(serverpath_1, '/');
    snprintf(serverpath_2, strlen(serverpath_1) - strlen(pointer) + 1, serverpath_1);
    snprintf(path2, strlen(serverpath_1) - strlen(pointer) + 2, serverpath_1);
    if(strcmp(path2, "/") == 0)
    {
        exist = s_create_folder(path, serverpath_1, newfolderid, index, 0,6);
        if(exist == -1)
            return -1;
        return 0;
    }
    exist = is_server_exist(path1, serverpath_2, NULL,index);
    if(exist == -2)
        re_create_folder(path1,serverpath_2, index);
    if(exist == 0)
        s_create_folder(path1,serverpath_2, newfolderid, index, 0,6);
    s_create_folder(path, serverpath_1, newfolderid, index, 0,6);
    if(exist == 1)
        return -1;

    return 0;
}



int the_same_name_compare(LocalFile *localfiletmp,CloudFile *filetmp,int index,int is_init)
{

    wd_DEBUG("###########the same name compare################\n");

    int ret=0;
    int newer_file_ret=0;
    if(asus_cfg.prule[index]->rule == 1)  //download only
    {
        newer_file_ret = newer_file(filetmp->id, localfiletmp->path,index,is_init,1);
        if(newer_file_ret !=2 && newer_file_ret !=-1)
        {
            if(newer_file_ret == 0) //local file is change
            {
                char *newname;
                char *tmp_name = malloc(strlen(localfiletmp->path)+1);
                memset(tmp_name,0,strlen(localfiletmp->path)+1);
                sprintf(tmp_name,"%s",localfiletmp->path);

                while(!exit_loop)
                {
                    newname = get_confilicted_name(tmp_name,0);
                    if(access(newname,F_OK) == 0)
                    {
                        my_free(tmp_name);
                        tmp_name = malloc(strlen(newname)+1);
                        memset(tmp_name,0,strlen(newname)+1);
                        sprintf(tmp_name,"%s",newname);
                        my_free(newname);
                    }
                    else
                        break;
                }
                my_free(tmp_name);

                rename(localfiletmp->path,newname);
               char *err_msg = write_error_message("%s is download from server,%s is local file and rename from %s",localfiletmp->path,newname,localfiletmp->path);
                write_trans_excep_log(localfiletmp->path,3,err_msg);
                free(err_msg);
                free(newname);
            }

            action_item *item;
            DEBUG("get_action_item12\n");
            item = get_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list,index);

            if(is_local_space_enough(filetmp,index))
            {
                if(wait_handle_socket(index))
                {
                    return HAVE_LOCAL_SOCKET;
                }
                add_action_item("createfile",localfiletmp->path,g_pSyncList[index]->server_action_list);
                DEBUG("api_download5\n");
                ret=s_download(filetmp, localfiletmp->path,filetmp->href,index,6);
                if(ret == 0)
                {
                    time_t mtime=api_getmtime(filetmp->id, filetmp->href,dofile);
                    if(mtime != -1 || mtime != -2)
                        ChangeFile_modtime(localfiletmp->path,mtime);
                    else
                    {

                        wd_DEBUG("ChangeFile_modtime failed!\n");

                    }
                    if(item != NULL)
                    {
                        del_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
                    }
                }
                else
                {
                    return ret;
                }

            }
            else
            {

                write_log(S_ERROR,"local space is not enough!","",index);
                if(item == NULL)
                {
                    add_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
                }


            }
        }
    }
    else if(asus_cfg.prule[index]->rule == 2)//upload only
    {
        newer_file_ret = newer_file(filetmp->id, localfiletmp->path,index,is_init,0);
        if(newer_file_ret !=2 && newer_file_ret !=-1)
        {
            if(wait_handle_socket(index))
            {
                return HAVE_LOCAL_SOCKET;
            }

            DEBUG("upload5,rule ==2\n");
            char *newname;
            newname=change_server_same_name(filetmp->href,NULL, index);
            DEBUG("upload5!api_move \n");
            s_move(filetmp->href,newname,index,0,NULL,6);
            DEBUG("upload1\n");
            ret=s_upload_file(localfiletmp->path,filetmp->href,1,index,6);
            if(ret == 0 || ret == SERVER_SPACE_NOT_ENOUGH || ret == LOCAL_FILE_LOST)
            {
                if(ret == 0)
                {
                    cJSON *json = dofile(Con(TMP_R,upload_chunk_commit.txt));
                    time_t mtime=cJSON_printf_mtime(json);
                    cJSON_Delete(json);
                    ChangeFile_modtime(localfiletmp->path,mtime);
                }
            }
            else
                return ret;

        }
    }
    else //sync
    {
        newer_file_ret = newer_file(filetmp->id, localfiletmp->path,index,is_init,0);
        DEBUG("!!!newer_file_ret = %d\n", newer_file_ret);
        if(newer_file_ret == 1)
        {
            if(is_init)
            {
                if(wait_handle_socket(index))
                {
                    return HAVE_LOCAL_SOCKET;
                }
                char *serverpath=localpath_to_serverpath(localfiletmp->path,index);
                /*third param flase,false=>if server file has exit ,will not be overwrite*/
                DEBUG("upload2\n");
                ret=s_upload_file(localfiletmp->path,serverpath,0,index,6);
                free(serverpath);
                if(ret == 0 || ret == SERVER_SPACE_NOT_ENOUGH || ret == LOCAL_FILE_LOST)
                {
                    if(ret == 0)
                    {
                        char *newlocalname = NULL;
                        cJSON *json = dofile(Con(TMP_R,upload_chunk_commit.txt));
                        time_t mtime=cJSON_printf_mtime(json);
                        cJSON_Delete(json);

                        newlocalname=change_local_same_file(localfiletmp->path,index);
                        if(newlocalname)
                            ChangeFile_modtime(newlocalname,mtime);
                        free(newlocalname);
                    }

                }
                else
                {
                    return ret;
                }
            }
            if(is_local_space_enough(filetmp,index))
            {
                action_item *item;
                DEBUG("get_action_item13\n");
                item = get_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list,index);
                if(wait_handle_socket(index))
                {
                    return HAVE_LOCAL_SOCKET;
                }
                add_action_item("createfile",localfiletmp->path,g_pSyncList[index]->server_action_list);
               DEBUG("api_download6\n");
                ret=s_download(filetmp, localfiletmp->path,filetmp->href,index,6);
                if(ret == 0)
                {
                    time_t mtime=api_getmtime(filetmp->id, filetmp->href,dofile);
                    if(mtime != -1 || mtime != -2)
                        ChangeFile_modtime(localfiletmp->path,mtime);
                    else
                    {

                        wd_DEBUG("ChangeFile_modtime failed!\n");

                    }
                    if(item != NULL)
                    {
                        del_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
                    }
                }
                else
                {
                    return ret;
                }

            }
            else
            {
                write_log(S_ERROR,"local space is not enough!","",index);
                add_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
            }
        }
        else if(newer_file_ret == 0)
        {
            if(wait_handle_socket(index))
            {
                return HAVE_LOCAL_SOCKET;
            }
            DEBUG("upload3\n");
            ret=s_upload_file(localfiletmp->path,filetmp->href,0,index,6);
            if(ret == 0 || ret == SERVER_SPACE_NOT_ENOUGH || ret == LOCAL_FILE_LOST)
            {
                if(ret == 0)
                {
                    cJSON * json = dofile(Con(TMP_R,upload_chunk_commit.txt));
                    time_t mtime=cJSON_printf_mtime(json);
                    cJSON_Delete(json);

                    ChangeFile_modtime(localfiletmp->path,mtime);
                }
            }
            else
                return ret;
        }
    }
    return 0;
}
int send_action(int type, char *content)
{
    if(exit_loop)
    {
        return 0;
    }
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    char str[1024];
    int port;

    port = INOTIFY_PORT;

    struct sockaddr_in their_addr; /* connector's address information */


    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return -1;
    }

    bzero(&(their_addr), sizeof(their_addr)); /* zero the rest of the struct */
    their_addr.sin_family = AF_INET; /* host byte order */
    their_addr.sin_port = htons(port); /* short, network byte order */
    their_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bzero(&(their_addr.sin_zero), sizeof(their_addr.sin_zero)); /* zero the rest of the struct */
    if (connect(sockfd, (struct sockaddr *)&their_addr,sizeof(struct
                                                              sockaddr)) == -1) {
        perror("connect");
        return -1;
    }

    sprintf(str,"%d@%s",type,content);


    if (send(sockfd, str, strlen(str), 0) == -1) {
        perror("send");
        return -1;
    }

    if ((numbytes=recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
        perror("recv");
        return -1;
    }

    buf[numbytes] = '\0';
    close(sockfd);

    return 0;
}
void send_to_inotify(){

    int i;

    for(i=0;i<asus_cfg.dir_number;i++)
    {
#if TOKENFILE
        if(g_pSyncList[i]->sync_disk_exist)
        {
            send_action(asus_cfg.type,asus_cfg.prule[i]->path);
            usleep(1000*10);
        }
#else
        send_action(asus_cfg.type,asus_cfg.prule[i]->path);
        usleep(1000*10);
#endif
    }
}
int get_rootid()
{
    DEBUG("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!get_rootid start\n");
    int status = 0;
    while(api_accout_info() != 0);

        cJSON *json = dofile(Con(TMP_R,data_3.txt));
        if (cJSON_printf_account_info(json) == -1)
        {
            cJSON_Delete(json);
            return -1;
        }
        cJSON_Delete(json);
        return 0;
}

char *get_path_from_socket(char *cmd,int index)
{

    if(!strncmp(cmd,"rmroot",6))
    {
        return NULL;
    }

    char cmd_name[64]="\0";
    char *path;
    char *temp;
    char filename[256]="\0";
    char *fullname;
    char oldname[256]="\0",newname[256]="\0";
    char *oldpath;
    char action[64]="\0";
    char *cmp_name;
    char *mv_newpath;
    char *mv_oldpath;
    char *ch;
    int status;

    ch = cmd;
    int i = 0;
    while(*ch != '\n')
    {
        i++;
        ch++;
    }

    memcpy(cmd_name, cmd, i);

    char *p = NULL;
    ch++;
    i++;

    temp = my_str_malloc((size_t)(strlen(ch)+1));

    strcpy(temp,ch);
    p = strchr(temp,'\n');

    path = my_str_malloc((size_t)(strlen(temp)- strlen(p)+1));


    if(p!=NULL)
        snprintf(path,strlen(temp)- strlen(p)+1,"%s",temp);

    p++;
    if(strcmp(cmd_name, "rename") == 0)
    {
        char *p1 = NULL;
        p1 = strchr(p,'\n');

        if(p1 != NULL)
            strncpy(oldname,p,strlen(p)- strlen(p1));

        p1++;

        strcpy(newname,p1);

        wd_DEBUG("cmd_name: [%s],path: [%s],oldname: [%s],newname: [%s]\n",cmd_name,path,oldname,newname);

        if(newname[0] == '.' || (strstr(path,"/.")) != NULL)
        {
            free(temp);
            free(path);
            return NULL;
        }
    }
    else if(strcmp(cmd_name, "move") == 0)
    {
        char *p1 = NULL;
        p1 = strchr(p,'\n');

        oldpath = my_str_malloc((size_t)(strlen(p)- strlen(p1)+1));

        if(p1 != NULL)
            snprintf(oldpath,strlen(p)- strlen(p1)+1,"%s",p);

        p1++;

        strcpy(oldname,p1);

        wd_DEBUG("cmd_name: [%s],path: [%s],oldpath: [%s],oldname: [%s]\n",cmd_name,path,oldpath,oldname);


        if(oldname[0] == '.' || (strstr(path,"/.")) != NULL)
        {
            free(temp);
            free(path);
            free(oldpath);
            return NULL;
        }
    }
    else
    {
        strcpy(filename,p);
        fullname = my_str_malloc((size_t)(strlen(path)+strlen(filename)+2));

        wd_DEBUG("cmd_name: [%s],path: [%s],filename: [%s]\n",cmd_name,path,filename);

        if(filename[0] == '.' || (strstr(path,"/.")) != NULL)
        {
            free(temp);
            free(path);
            return NULL;
        }
    }

    free(temp);

    if( !strcmp(cmd_name,"rename") )
    {
        cmp_name = my_str_malloc((size_t)(strlen(path)+strlen(newname)+2));
        sprintf(cmp_name,"%s/%s",path,newname);
    }
    else
    {
        cmp_name = my_str_malloc((size_t)(strlen(path)+strlen(filename)+2));
        sprintf(cmp_name,"%s/%s",path,filename);
    }

    if( strcmp(cmd_name, "createfile") == 0 )
    {
        strcpy(action,"createfile");
    }
    else if( strcmp(cmd_name, "remove") == 0  || strcmp(cmd_name, "delete") == 0)
    {
        strcpy(action,"remove");
    }
    else if( strcmp(cmd_name, "createfolder") == 0 )
    {
        strcpy(action,"createfolder");
    }
    else if( strcmp(cmd_name, "rename") == 0 )
    {
        strcpy(action,"rename");
    }
#if 1
    if(g_pSyncList[index]->server_action_list->next != NULL)
    {
        action_item *item;
        DEBUG("get_action_item14\n");
        item = get_action_item(action,cmp_name,g_pSyncList[index]->server_action_list,index);

        if(item != NULL)
        {

            wd_DEBUG("2##### %s %s by google Server self ######\n",action,cmp_name);
            del_action_item(action,cmp_name,g_pSyncList[index]->server_action_list);
            wd_DEBUG("#### del action item success!\n");
            free(path);
            if( strcmp(cmd_name, "rename") != 0 )
                free(fullname);
            free(cmp_name);
            return NULL;
        }
    }
#endif
    free(cmp_name);
    return path;

}




void set_copyfile_list(char *buf,char *oldpath,char *newpath)
{
    wd_DEBUG("************************set_copyfile_list***********************\n");
    int i;
    char *r_path;
    r_path = get_socket_base_path(buf);
    for(i=0;i<asus_cfg.dir_number;i++)
    {
        if(!strcmp(r_path,asus_cfg.prule[i]->path))
            break;
    }
    free(r_path);

    check_action_item("copyfile",oldpath,g_pSyncList[i]->copy_file_list,i,newpath);

}




void set_re_cmd(char *buf,char *oldpath,char *newpath)
{
    wd_DEBUG("************************set_re_cmd***********************\nbuf=%s\noldpath=%s\nnewpath=%s\n",buf,oldpath,newpath);
    int i;
    char *r_path;
    r_path = get_socket_base_path(buf);
    for(i=0;i<asus_cfg.dir_number;i++)
    {
        if(!strcmp(r_path,asus_cfg.prule[i]->path))
            break;
    }
    free(r_path);

    queue_entry_t pTemp = g_pSyncList[i]->SocketActionList->head;

    while(pTemp!=NULL)
    {
            char *socket_path=get_path_from_socket(pTemp->cmd_name,i);
            wd_DEBUG("path:%s\n",socket_path);
            wd_DEBUG("path:%s\n",oldpath);
            char *pTemp_t=(char *)malloc(sizeof(char)*(strlen(socket_path)+1+1));
            char *oldpath_t=(char *)malloc(sizeof(char)*(strlen(oldpath)+1+1));
            memset(pTemp_t,'\0',strlen(socket_path)+1+1);
            memset(oldpath_t,'\0',strlen(oldpath)+1+1);
            sprintf(pTemp_t,"%s/",socket_path);
            sprintf(oldpath_t,"%s/",oldpath);
            char *p_t=NULL;
            if(socket_path != NULL)
            {
                if(pTemp->re_cmd == NULL)
                {
                    if((p_t=strstr(pTemp_t,oldpath_t)) != NULL)
                    {
                        if(strlen(oldpath)<strlen(socket_path))
                        {
                            p_t=p_t+strlen(oldpath);
                            pTemp->re_cmd = (char *)malloc(sizeof(char) * (strlen(newpath)+strlen(p_t) + 1));
                            memset(pTemp->re_cmd,'\0',strlen(newpath)+strlen(p_t) + 1);
                            sprintf(pTemp->re_cmd,"%s%s",newpath,p_t);
                            DEBUG("@@@@@@@@@@@@@pTemp->re_cmd=%s\n",pTemp->re_cmd);
                        }
                        else
                        {
                            pTemp->re_cmd = (char *)malloc(sizeof(char) * (strlen(newpath) + 1));
                            sprintf(pTemp->re_cmd,"%s/",newpath);
                            DEBUG("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!pTemp->re_cmd=%s\n",pTemp->re_cmd);
                        }
                    }
                }
                else
                {
                    if((p_t=strstr(pTemp->re_cmd,oldpath_t)) != NULL)
                    {
                        if(strlen(oldpath_t)<strlen(pTemp->re_cmd))
                        {
                            p_t+=strlen(oldpath);
                            char *p_tt=(char *)malloc(sizeof(char)*(strlen(p_t)+1));
                            memset(p_tt,'\0',strlen(p_t)+1);
                            sprintf(p_tt,"%s",p_t);
                            free(pTemp->re_cmd);
                            pTemp->re_cmd = (char *)malloc(sizeof(char) * (strlen(newpath)+strlen(p_tt) + 1));
                            memset(pTemp->re_cmd,'\0',strlen(newpath)+strlen(p_tt) + 1);
                            sprintf(pTemp->re_cmd,"%s%s",newpath,p_tt);
                            DEBUG("~~~~~~~~~~~~~~~~~~~~~pTemp->re_cmd=%s\n",pTemp->re_cmd);
                            free(p_tt);
                        }
                        else
                        {
                            free(pTemp->re_cmd);
                            pTemp->re_cmd = (char *)malloc(sizeof(char) * (strlen(newpath) + 1));
                            sprintf(pTemp->re_cmd,"%s/",newpath);
                            DEBUG("###############pTemp->re_cmd=%s\n",pTemp->re_cmd);
                        }
                    }
                }
                free(socket_path);
            }
            free(pTemp_t);
            free(oldpath_t);

        pTemp=pTemp->next_ptr;
    }

    show(g_pSyncList[i]->SocketActionList->head);
}

int change_socklist_re_cmd(char *cmd)
{
    DEBUG("change_socklist_re_cmd \n");
    char cmd_name[64]="\0";
    char *path;
    char *temp;
    char oldname[256]="\0",newname[256]="\0";
    char *oldpath;
    char *mv_newpath;
    char *mv_oldpath;
    char *ch;

    ch = cmd;
    int i = 0;
    while(*ch != '\n')
    {
        i++;
        ch++;
    }

    memcpy(cmd_name, cmd, i);

    char *p = NULL;
    ch++;
    i++;

    temp = my_str_malloc((size_t)(strlen(ch)+1));

    strcpy(temp,ch);
    p = strchr(temp,'\n');

    path = my_str_malloc((size_t)(strlen(temp)- strlen(p)+1));


    if(p!=NULL)
        snprintf(path,strlen(temp)- strlen(p)+1,"%s",temp);

    p++;
    if(strcmp(cmd_name, "rename0") == 0)
    {
        char *p1 = NULL;

        p1 = strchr(p,'\n');

        if(p1 != NULL)
            strncpy(oldname,p,strlen(p)- strlen(p1));

        p1++;

        strcpy(newname,p1);

        if(newname[0] == '.' || (strstr(path,"/.")) != NULL)
        {
            free(temp);
            free(path);
            return 0;
        }
    }
    else if(strcmp(cmd_name, "move0") == 0)
    {
        char *p1 = NULL;

        p1 = strchr(p,'\n');

        oldpath = my_str_malloc((size_t)(strlen(p)- strlen(p1)+1));

        if(p1 != NULL)
            snprintf(oldpath,strlen(p)- strlen(p1)+1,"%s",p);

        p1++;

        strcpy(oldname,p1);

        if(oldname[0] == '.' || (strstr(path,"/.")) != NULL)
        {
            free(temp);
            free(path);
            free(oldpath);
            return 0;
        }
    }

    free(temp);

    if(strcmp(cmd_name, "move0") == 0)
    {
        mv_newpath = my_str_malloc((size_t)(strlen(path)+strlen(oldname)+2));
        mv_oldpath = my_str_malloc((size_t)(strlen(oldpath)+strlen(oldname)+2));
        sprintf(mv_newpath,"%s/%s",path,oldname);
        sprintf(mv_oldpath,"%s/%s",oldpath,oldname);
        free(oldpath);
    }
    else
    {
        mv_newpath = my_str_malloc((size_t)(strlen(path)+strlen(newname)+2));
        mv_oldpath = my_str_malloc((size_t)(strlen(path)+strlen(oldname)+2));
        sprintf(mv_newpath,"%s/%s",path,newname);
        sprintf(mv_oldpath,"%s/%s",path,oldname);
    }

    pthread_mutex_lock(&mutex_socket);
    set_re_cmd(cmd,mv_oldpath,mv_newpath);
    set_copyfile_list(cmd,mv_oldpath,mv_newpath);
    pthread_mutex_unlock(&mutex_socket);

    free(path);
    free(mv_oldpath);
    free(mv_newpath);
    return 0;

}

void *SyncLocal()
{
    int sockfd, new_fd; /* listen on sock_fd, new connection on new_fd*/
    int numbytes;
    char buf[MAXDATASIZE];
    int yes = 1;
    int ret;

    fd_set read_fds;
    fd_set master;
    int fdmax;
    struct timeval timeout;

    FD_ZERO(&read_fds);
    FD_ZERO(&master);

    struct sockaddr_in my_addr; /* my address information */
    struct sockaddr_in their_addr; /* connector's address information */
    int sin_size;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("Server-setsockopt() error lol!");
        exit(1);
    }

    my_addr.sin_family = AF_INET; /* host byte order */
    my_addr.sin_port = htons(MYPORT); /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */
    bzero(&(my_addr.sin_zero), sizeof(my_addr.sin_zero)); /* zero the rest of the struct */

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct
                                                         sockaddr))== -1) {
        perror("bind");
        exit(1);
    }
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    sin_size = sizeof(struct sockaddr_in);

    FD_SET(sockfd,&master);
    fdmax = sockfd;

    while(!exit_loop)
    { /* main accept() loop */

        timeout.tv_sec = 0;
        timeout.tv_usec = 100;

        read_fds = master;

        ret = select(fdmax+1,&read_fds,NULL,NULL,&timeout);

        switch (ret)
        {
        case 0:
            continue;
            break;
        case -1:
            perror("select");
            continue;
            break;
        default:
            if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, \
                                 &sin_size)) == -1) {
                perror("accept");
                continue;
            }
            memset(buf, 0, sizeof(buf));

            if ((numbytes=recv(new_fd, buf, MAXDATASIZE, 0)) == -1) {
                perror("recv");
                exit(1);
            }

            if(buf[strlen(buf)] == '\n')
            {
                buf[strlen(buf)] = '\0';
            }
            wd_DEBUG("!!!!!socket buf = %s\n",buf);
            close(new_fd);

#ifdef RENAME_F
            if(strncmp(buf,"rename0",strlen("rename0")) == 0 || strncmp(buf,"move0",strlen("move0")) == 0)
            {
                DEBUG("SyncLocal------>change_socklist_re_cmd\n");
                change_socklist_re_cmd(buf);
            }
#endif
#if 1
            pthread_mutex_lock(&mutex_socket);
            add_socket_item(buf);
            pthread_mutex_unlock(&mutex_socket);
#endif
        }

    }
    close(sockfd);

    wd_DEBUG("stop google local sync\n");

}
int add_socket_item(char *buf){

    int i;
    char *r_path;

    r_path = get_socket_base_path(buf);
    for(i=0;i<asus_cfg.dir_number;i++)
    {
        if(!strcmp(r_path,asus_cfg.prule[i]->path))
            break;
    }

    wd_DEBUG("add_socket_item rule:%d\n",i);
    free(r_path);
    pthread_mutex_lock(&mutex_receve_socket);
    g_pSyncList[i]->receve_socket = 1;
    pthread_mutex_unlock(&mutex_receve_socket);

#if MEM_POOL_ENABLE
    SocketActionTmp = mem_alloc(16);
#else
    SocketActionTmp = malloc (sizeof (struct queue_entry));
#endif

    memset(SocketActionTmp,0,sizeof(struct queue_entry));
    int len = strlen(buf)+1;
#if MEM_POOL_ENABLE
    SocketActionTmp->cmd_name = mem_alloc(len);
#else
    SocketActionTmp->cmd_name = (char *)calloc(len,sizeof(char));
#endif
    sprintf(SocketActionTmp->cmd_name,"%s",buf);
    SocketActionTmp->re_cmd = NULL;
    SocketActionTmp->is_first = 0;
    queue_enqueue(SocketActionTmp,g_pSyncList[i]->SocketActionList);

    wd_DEBUG("SocketActionTmp->cmd_name = %s\n",SocketActionTmp->cmd_name);
    return 0;
}
char *get_socket_base_path(char *cmd){

    char *temp = NULL;
    char *temp1 = NULL;
    char path[1024];
    char *root_path = NULL;

    if(!strncmp(cmd,"rmroot",6))
    {
        temp = strchr(cmd,'/');
        root_path = my_str_malloc(512);
        sprintf(root_path,"%s",temp);
    }
    else
    {
        temp = strchr(cmd,'/');
        temp1 = strchr(temp,'\n');
        memset(path,0,sizeof(path));
        strncpy(path,temp,strlen(temp)-strlen(temp1));

        root_path = my_str_malloc(512);
        if(strncmp(path,"/tmp",4) ==0 )
        {
            temp = my_nstrchr('/',path,5);
        }
        else
        {
            temp = my_nstrchr('/',path,4);
        }
        if(temp == NULL)
        {
            sprintf(root_path,"%s",path);
        }
        else
        {
            snprintf(root_path,strlen(path)-strlen(temp)+1,"%s",path);
        }
    }
    return root_path;
}
void run()
{
    if(get_rootid() == -1)
    {
        exit(-1);
    }
    int create_thid1 = 0;
    int create_thid2 = 0;
    int create_thid3 = 0;
    int need_server_thid = 0;


    create_sync_list();
    send_to_inotify();

    if(set_iptables(1))
        exit(-1);

    if(exit_loop == 0)
    {
        if( pthread_create(&newthid2,NULL,(void *)SyncLocal,NULL) != 0)
        {
            wd_DEBUG("thread creation failder\n");
            exit(1);
        }
        create_thid2 = 1;
    }
#if 1
    if(exit_loop == 0)
    {

        wd_DEBUG("create newthid3\n");

        if( pthread_create(&newthid3,NULL,(void *)Socket_Parser,NULL) != 0)
        {
            wd_DEBUG("thread creation failder\n");
            exit(1);
        }
        create_thid3 = 1;
        usleep(1000*500);

    }
    DEBUG("Socket_Parser end!!!\n");

        sync_initial();

    finished_initial=1;


    /*
     fix when socket_parse run sync_initial_again ,local send socket,the process will sleep
    */
    DEBUG("after sync_initial,exit_loop = %d\n", exit_loop);
    if(exit_loop == 0)
    {
        if( pthread_create(&newthid1,NULL,(void *)SyncServer,NULL) != 0)
        {
            wd_DEBUG("thread creation failder\n");
            exit(1);
        }
        create_thid1 = 1;
        usleep(1000*500);
    }

    if(create_thid1)
        pthread_join(newthid1,NULL);
    if(create_thid3)
        pthread_join(newthid3,NULL);
    if(create_thid2)
        pthread_join(newthid2,NULL);

    usleep(1000);
    clean_up();
#if TOKENFILE
    if(stop_progress != 1)
    {
        wd_DEBUG(" google client run again!\n");
        if(access_token_expired)
        {
            api_refresh_token();
            access_token_expired = 0;
        }

        while(disk_change)
        {
            disk_change = 0;
            sync_disk_removed = check_sync_disk_removed();

            if(sync_disk_removed == 2)
            {
                wd_DEBUG("sync path is change\n");
            }
            else if(sync_disk_removed == 1)
            {
                wd_DEBUG("sync disk is unmount\n");
            }
            else if(sync_disk_removed == 0)
            {
                wd_DEBUG("sync disk exists\n");
            }
        }

        exit_loop = 0;
        run();
    }
#endif
#endif
}

void clean_up()
{

    wd_DEBUG("enter clean up\n");

    int i;


    for(i=0;i<asus_cfg.dir_number;i++)
    {
        queue_destroy(g_pSyncList[i]->SocketActionList);

        if(g_pSyncList[i]->ServerRootNode == g_pSyncList[i]->OldServerRootNode)
        {

            wd_DEBUG("the same Pointer!\n");

            if(g_pSyncList[i]->ServerRootNode != NULL)
                free_server_tree(g_pSyncList[i]->ServerRootNode);
        }
        else
        {
            if(g_pSyncList[i]->ServerRootNode != NULL)
                free_server_tree(g_pSyncList[i]->ServerRootNode);

            wd_DEBUG("clean %d ServerRootNode success!\n",i);

            if(g_pSyncList[i]->OldServerRootNode != NULL)
                free_server_tree(g_pSyncList[i]->OldServerRootNode);

            wd_DEBUG("clean %d OldServerRootNode success!\n",i);

        }

        free_action_item(g_pSyncList[i]->server_action_list);
        free_action_item(g_pSyncList[i]->unfinished_list);
        free_action_item(g_pSyncList[i]->copy_file_list);
        free_action_item(g_pSyncList[i]->up_space_not_enough_list);

        if(asus_cfg.prule[i]->rule == 1)
        {
            free_action_item(g_pSyncList[i]->download_only_socket_head);
        }
        free(g_pSyncList[i]);
    }
    free(g_pSyncList);

#if MEM_POOL_ENABLE
    mem_pool_destroy();
#endif

    wd_DEBUG("google client clean up end !!!\n");

}

void *SyncServer()
{
    struct timeval now;
    struct timespec outtime;

    int status;
    int i;
    DEBUG("enter SyncServer,exit_loop = %d\n", exit_loop);
    while( !exit_loop )
    {

        wd_DEBUG("***************SyncServer start**************\n");

        for(i=0;i<asus_cfg.dir_number;i++)
        {
            status=0;

            while (local_sync == 1 && exit_loop == 0){
                usleep(1000*10);
            }
            server_sync = 1;

            if(exit_loop)
                break;
#if TOKENFILE
            if(disk_change)
            {
                check_disk_change();
            }
            if(g_pSyncList[i]->sync_disk_exist == 0)
            {
                write_log(S_ERROR,"Sync disk unplug!","",i);
                continue;
            }

#endif
            if(g_pSyncList[i]->no_local_root)
            {
                my_mkdir_r(asus_cfg.prule[i]->path);   //have mountpath
                send_action(asus_cfg.type,asus_cfg.prule[i]->path);
                usleep(1000*10);
                g_pSyncList[i]->no_local_root = 0;
                g_pSyncList[i]->init_completed = 0;
            }

            status = do_unfinished(i);

            if( !g_pSyncList[i]->init_completed )
                status = sync_initial();

            if(status != 0)
                continue;

            if(asus_cfg.prule[i]->rule == 2)
            {
                continue;
            }

            if(exit_loop == 0)
            {
                int get_serlist_fail_time = 0;
                do
                {
                    g_pSyncList[i]->ServerRootNode = create_server_treeroot();
#ifdef MULTI_PATH
                    DEBUG("browse_to_tree7\n");
                    status = s_browse_to_tree(asus_cfg.prule[i]->rooturl,g_pSyncList[i]->ServerRootNode,6);
#else
                    DEBUG("browse_to_tree8\n");
                    status = s_browse_to_tree("/", rootid, g_pSyncList[i]->ServerRootNode, i,6);
#endif
#ifdef __DEBUG__
                    SearchServerTree(g_pSyncList[i]->ServerRootNode);
#endif

                    if(status != 0)
                    {
                        wd_DEBUG("get ServerList ERROR! \n");
                        get_serlist_fail_time ++;
                        free_server_tree(g_pSyncList[i]->ServerRootNode);
                        g_pSyncList[i]->ServerRootNode = NULL;
                    }


                }while(status!=0 && get_serlist_fail_time < 5 && exit_loop == 0 && g_pSyncList[i]->receve_socket == 0);
                if (status != 0)
                {

                    /*auth again:
                        for the token not work!
                    */
#ifdef OAuth1
                    if(g_pSyncList[i]->receve_socket == 0)
                    {
                        if(exit_loop == 0)
                            do_auth();
                    }
#endif
                    /*for get serverlist fail,then mem will updata*/
                    //free_server_tree(g_pSyncList[i]->ServerRootNode);
                    //g_pSyncList[i]->ServerRootNode = NULL;

                    /*first_sync:
                        after the initial finish,the serverlist is change so force to run server sync;
                        when the server sync failed,next time we must force to run server sync;
                    */
                    g_pSyncList[i]->first_sync = 1;
                    usleep(1000*20);
                    continue;
                }

                if(g_pSyncList[i]->unfinished_list->next != NULL)
                {
                    continue;
                }

                if(g_pSyncList[i]->first_sync)
                {

                    wd_DEBUG("@@@first sync!\n");
                    g_pSyncList[i]->VeryOldServerRootNode=g_pSyncList[i]->OldServerRootNode;
                    g_pSyncList[i]->OldServerRootNode = g_pSyncList[i]->ServerRootNode;
                    status=Server_sync(i);
                    free_server_tree(g_pSyncList[i]->VeryOldServerRootNode);//死在这
                    if(status == 0)
                        g_pSyncList[i]->first_sync = 0;
                    else
                        g_pSyncList[i]->first_sync = 1;
                }
                else
                {
                    if(asus_cfg.prule[i]->rule == 0)
                    {
                        status=compareServerList(i);
                    }
                    //serverList different or download only
                    if (status == 0 || asus_cfg.prule[i]->rule == 1)
                    {

                        g_pSyncList[i]->VeryOldServerRootNode=g_pSyncList[i]->OldServerRootNode;
                        g_pSyncList[i]->OldServerRootNode = g_pSyncList[i]->ServerRootNode;
                        DEBUG("Server_sync2!\n");
                        status=Server_sync(i);
                        free_server_tree(g_pSyncList[i]->VeryOldServerRootNode);
                        if(status == 0)
                            g_pSyncList[i]->first_sync = 0;
                        else
                            g_pSyncList[i]->first_sync = 1;
                    }
                    else
                    {//serverList same

                        free_server_tree(g_pSyncList[i]->ServerRootNode);
                        g_pSyncList[i]->ServerRootNode = NULL;
                        status = 0;
                    }
                }

                DEBUG("Syncserver, exit_loop==0\n");
            }
            DEBUG("Syncserver, end if exit_loop==0\n");
                write_log(S_SYNC,"","",i);
        }

        DEBUG("Syncserver, break number!\n");
        server_sync = 0;      //server sync finished
        pthread_mutex_lock(&mutex);
        if(!exit_loop)
        {
            gettimeofday(&now, NULL);
            outtime.tv_sec = now.tv_sec + 10;
            outtime.tv_nsec = now.tv_usec * 1000;
            pthread_cond_timedwait(&cond, &mutex, &outtime);
        }
        pthread_mutex_unlock(&mutex);
    }


    wd_DEBUG("stop google server sync\n");

}

int Server_sync(int index)
{

    wd_DEBUG("compareLocalList start!\n");

    int ret = 0;

    if(g_pSyncList[index]->ServerRootNode->Child != NULL)
    {

        wd_DEBUG("ServerRootNode->Child != NULL\n");

        ret = sync_local_with_server(g_pSyncList[index]->ServerRootNode->Child,sync_local_with_server_perform,index);
    }
    else
    {

        wd_DEBUG("ServerRootNode->Child == NULL\n");

    }
    DEBUG("Server sync end, ret = %d\n", ret);
    return ret;
}

/*
 *judge is server changed
 *0:server changed
 *1:server is not changed
*/
int isServerChanged(Server_TreeNode *newNode,Server_TreeNode *oldNode)
{
    int res = 1;
    int serverchanged = 0;
    if(newNode->browse == NULL && oldNode->browse == NULL)
    {
        return 1;
    }
    else if(newNode->browse == NULL && oldNode->browse != NULL)
    {
        return 0;
    }
    else if(newNode->browse != NULL && oldNode->browse == NULL)
    {
        return 0;
    }
    else
    {
        if(newNode->browse->filenumber != oldNode->browse->filenumber || newNode->browse->foldernumber != oldNode->browse->foldernumber)
        {
            return 0;
        }
        else
        {
            int cmp;
            CloudFile *newfoldertmp = NULL;
            CloudFile *oldfoldertmp = NULL;
            CloudFile *newfiletmp = NULL;
            CloudFile *oldfiletmp = NULL;
            if(newNode->browse != NULL)
            {
                if(newNode->browse->foldernumber > 0)
                    newfoldertmp = newNode->browse->folderlist->next;
                if(newNode->browse->filenumber > 0)
                    newfiletmp = newNode->browse->filelist->next;
            }
            if(oldNode->browse != NULL)
            {
                if(oldNode->browse->foldernumber > 0)
                    oldfoldertmp = oldNode->browse->folderlist->next;
                if(oldNode->browse->filenumber > 0)
                    oldfiletmp = oldNode->browse->filelist->next;
            }

            while (newfoldertmp != NULL || oldfoldertmp != NULL)
            {
                if ((cmp = strcmp(newfoldertmp->href,oldfoldertmp->href)) != 0){
                    return 0;
                }
                newfoldertmp = newfoldertmp->next;
                oldfoldertmp = oldfoldertmp->next;
            }
            while (newfiletmp != NULL || oldfiletmp != NULL)
            {
                if ((cmp = strcmp(newfiletmp->href,oldfiletmp->href)) != 0)
                {
                    return 0;
                }
                if (newfiletmp->mtime != oldfiletmp->mtime)
                {
                    return 0;
                }
                newfiletmp = newfiletmp->next;
                oldfiletmp = oldfiletmp->next;
            }
        }

        if((newNode->Child == NULL && oldNode->Child != NULL) || (newNode->Child != NULL && oldNode->Child == NULL))
        {
            return 0;
        }
        if((newNode->NextBrother == NULL && oldNode->NextBrother != NULL) || (newNode->NextBrother!= NULL && oldNode->NextBrother == NULL))
        {
            return 0;
        }

        if(newNode->Child != NULL && oldNode->Child != NULL && !exit_loop)
        {
            res = isServerChanged(newNode->Child,oldNode->Child);
            if(res == 0)
            {
                serverchanged = 1;
            }
        }
        if(newNode->NextBrother != NULL && oldNode->NextBrother != NULL && !exit_loop)
        {
            res = isServerChanged(newNode->NextBrother,oldNode->NextBrother);
            if(res == 0)
            {
                serverchanged = 1;
            }
        }
    }
    if(serverchanged == 1)
    {

        wd_DEBUG("########Server changed9\n");

        return 0;
    }
    else
    {
        return 1;
    }
}

/*ret = 0,server changed
 *ret = 1,server is no changed
*/
int compareServerList(int index)
{
    int ret;

    wd_DEBUG("#########compareServerList\n");

    if(g_pSyncList[index]->ServerRootNode->Child != NULL && g_pSyncList[index]->OldServerRootNode->Child != NULL)
    {
        ret = isServerChanged(g_pSyncList[index]->ServerRootNode->Child,g_pSyncList[index]->OldServerRootNode->Child);
        return ret;
    }
    else if(g_pSyncList[index]->ServerRootNode->Child == NULL && g_pSyncList[index]->OldServerRootNode->Child == NULL)
    {
        ret = 1;
        return ret;
    }
    else
    {
        ret = 0;
        return ret;
    }
}
int get_create_threads_state()
{
    int i;
    for(i=0;i<asus_cfg.dir_number;i++)
    {
        if(asus_cfg.prule[i]->rule != 2)
        {
            return 1;
        }
    }

    return 0;
}
int download_only_add_socket_item(char *cmd,int index)
{

    wd_DEBUG("download_only_add_socket_item receive socket : %s\n",cmd);

    if( strstr(cmd,"(conflict)") != NULL )
        return 0;

    wd_DEBUG("socket command is %s \n",cmd);


    if( !strncmp(cmd,"exit",4))
    {

        wd_DEBUG("exit socket\n");

        return 0;
    }

    if(!strncmp(cmd,"rmroot",6))
    {
        g_pSyncList[index]->no_local_root = 1;
        return 0;
    }


    char cmd_name[64];
    char *path = NULL;
    char *temp = NULL;
    char filename[256];
    char *fullname = NULL;
    char oldname[256],newname[256];
    char *oldpath = NULL;
    char action[64];
    char *ch = NULL;
    char *old_fullname = NULL;

    memset(cmd_name,'\0',sizeof(cmd_name));
    memset(oldname,'\0',sizeof(oldname));
    memset(newname,'\0',sizeof(newname));
    memset(action,'\0',sizeof(action));

    ch = cmd;
    int i = 0;
    while(*ch != '\n')
    {
        i++;
        ch++;
    }

    memcpy(cmd_name, cmd, i);

    char *p = NULL;
    ch++;
    i++;

    temp = my_str_malloc((size_t)(strlen(ch)+1));

    strcpy(temp,ch);
    p = strchr(temp,'\n');

    path = my_str_malloc((size_t)(strlen(temp)- strlen(p)+1));


    if(p!=NULL)
        snprintf(path,strlen(temp)- strlen(p)+1,"%s",temp);

    p++;
    if(strncmp(cmd_name, "rename",6) == 0)
    {
        char *p1 = NULL;

        p1 = strchr(p,'\n');

        if(p1 != NULL)
            strncpy(oldname,p,strlen(p)- strlen(p1));

        p1++;

        strcpy(newname,p1);

        wd_DEBUG("cmd_name: [%s],path: [%s],oldname: [%s],newname: [%s]\n",cmd_name,path,oldname,newname);

    }
    else if(strncmp(cmd_name, "move",4) == 0)
    {
        char *p1 = NULL;

        p1 = strchr(p,'\n');

        oldpath = my_str_malloc((size_t)(strlen(p)- strlen(p1)+1));

        if(p1 != NULL)
            snprintf(oldpath,strlen(p)- strlen(p1)+1,"%s",p);

        p1++;

        strcpy(oldname,p1);

        wd_DEBUG("cmd_name: [%s],path: [%s],oldpath: [%s],oldname: [%s]\n",cmd_name,path,oldpath,oldname);

    }
    else
    {
        strcpy(filename,p);
        wd_DEBUG("cmd_name: [%s],path: [%s],filename: [%s]\n",cmd_name,path,filename);
    }

    free(temp);

    if( !strncmp(cmd_name,"rename",strlen("rename")) )
    {
        fullname = my_str_malloc((size_t)(strlen(path)+strlen(newname)+2));
        old_fullname = my_str_malloc((size_t)(strlen(path)+strlen(oldname)+2));
        sprintf(fullname,"%s/%s",path,newname);
        sprintf(old_fullname,"%s/%s",path,oldname);
        free(path);
    }
    else if( !strncmp(cmd_name,"move",strlen("move")) )
    {
        fullname = my_str_malloc((size_t)(strlen(path)+strlen(oldname)+2));
        old_fullname = my_str_malloc((size_t)(strlen(oldpath)+strlen(oldname)+2));
        sprintf(fullname,"%s/%s",path,oldname);
        sprintf(old_fullname,"%s/%s",oldpath,oldname);
        free(oldpath);
        free(path);
    }
    else
    {
        fullname = my_str_malloc((size_t)(strlen(path)+strlen(filename)+2));
        sprintf(fullname,"%s/%s",path,filename);
        free(path);
    }
    if( !strncmp(cmd_name,"copyfile",strlen("copyfile")) )
    {
        add_action_item("copyfile",fullname,g_pSyncList[index]->copy_file_list);
        free(fullname);
        return 0;
    }


    if( strcmp(cmd_name, "createfile") == 0 )
    {
        strcpy(action,"createfile");
        action_item *item;
        DEBUG("get_action_item15\n");
        item = get_action_item("copyfile",fullname,g_pSyncList[index]->copy_file_list,index);

        if(item != NULL)
        {
            wd_DEBUG("##### delete copyfile %s ######\n",fullname);
            del_action_item("copyfile",fullname,g_pSyncList[index]->copy_file_list);
        }
    }
    else if( strcmp(cmd_name, "cancelcopy") == 0 )
    {
        action_item *item;
        DEBUG("get_action_item16\n");
        item = get_action_item("copyfile",fullname,g_pSyncList[index]->copy_file_list,index);

        if(item != NULL)
        {
            wd_DEBUG("##### delete copyfile %s ######\n",fullname);
            del_action_item("copyfile",fullname,g_pSyncList[index]->copy_file_list);
        }
        free(fullname);
        return 0;
    }
    else if( strcmp(cmd_name, "remove") == 0  || strcmp(cmd_name, "delete") == 0)
    {
        strcpy(action,"remove");
        del_download_only_action_item(action,fullname,g_pSyncList[index]->download_only_socket_head);
    }
    else if( strcmp(cmd_name, "createfolder") == 0 )
    {
        strcpy(action,"createfolder");
    }
    else if( strncmp(cmd_name, "rename",6) == 0 )
    {
        strcpy(action,"rename");
        del_download_only_action_item(action,old_fullname,g_pSyncList[index]->download_only_socket_head);
        free(old_fullname);
    }
    else if( strncmp(cmd_name, "move",4) == 0 )
    {
        strcpy(action,"move");
        del_download_only_action_item(action,old_fullname,g_pSyncList[index]->download_only_socket_head);
    }

    if(g_pSyncList[index]->server_action_list->next != NULL)
    {
        action_item *item;
        DEBUG("get_action_item17\n");
        item = get_action_item(action,fullname,g_pSyncList[index]->server_action_list,index);

        if(item != NULL)
        {

            wd_DEBUG("3##### %s %s by google Server self ######\n",action,fullname);
            del_action_item(action,fullname,g_pSyncList[index]->server_action_list);
            free(fullname);
            return 0;
        }
    }

    if( strcmp(cmd_name, "copyfile") != 0 )
    {
        g_pSyncList[index]->have_local_socket = 1;
    }

    if(strncmp(cmd_name, "rename",6) == 0)
    {
        if(test_if_dir(fullname))
        {
            add_all_download_only_socket_list(cmd_name,fullname,index);
        }
        else
        {
            add_action_item(cmd_name,fullname,g_pSyncList[index]->download_only_socket_head);
        }
    }
    else if(strncmp(cmd_name, "move",4) == 0)
    {
        if(test_if_dir(fullname))
        {
            add_all_download_only_socket_list(cmd_name,fullname,index);
        }
        else
        {
            add_action_item(cmd_name,fullname,g_pSyncList[index]->download_only_socket_head);
        }
    }
    else if(strcmp(cmd_name, "createfolder") == 0 || strcmp(cmd_name, "dragfolder") == 0)
    {
        add_action_item(cmd_name,fullname,g_pSyncList[index]->download_only_socket_head);
        if(!strcmp(cmd_name, "dragfolder"))
            add_all_download_only_dragfolder_socket_list(fullname,index);
    }
    else if( strcmp(cmd_name, "createfile") == 0  || strcmp(cmd_name, "dragfile") == 0 || strcmp(cmd_name, "modify") == 0)
    {
        add_action_item(cmd_name,fullname,g_pSyncList[index]->download_only_socket_head);
    }

    free(fullname);
    return 0;

}





void *Socket_Parser()
{

    wd_DEBUG("*******Socket_Parser start********\n");

    queue_entry_t socket_execute;
    int i;
    int status;
    int has_socket = 0;
    int fail_flag;
    struct timeval now;
    struct timespec outtime;
    while(!exit_loop)
    {
        for(i=0;i<asus_cfg.dir_number;i++)
        {
            while(server_sync == 1 && exit_loop ==0)
            {
                usleep(1000*10);
            }
            local_sync=1;

            if(exit_loop)
                break;
#if TOKENFILE
            if(disk_change)
            {
                check_disk_change();
            }
            if(g_pSyncList[i]->sync_disk_exist == 0)
                continue;
#endif
            if(asus_cfg.prule[i]->rule == 1) //Download only
            {
                while(exit_loop == 0)
                {
                    while(g_pSyncList[i]->SocketActionList->head != NULL && exit_loop == 0 && server_sync == 0)
                    {
                        has_socket = 1;
                        socket_execute=g_pSyncList[i]->SocketActionList->head;
                        status = download_only_add_socket_item(socket_execute->cmd_name,i);
                        if(status == 0  || status == SERVER_SPACE_NOT_ENOUGH
                           || status == LOCAL_FILE_LOST)
                        {
                            pthread_mutex_lock(&mutex_socket);
                            socket_execute = queue_dequeue(g_pSyncList[i]->SocketActionList);

#if MEM_POOL_ENABLE
                    mem_free(socket_execute->cmd_name);
                    mem_free(socket_execute->re_cmd);
                    mem_free(socket_execute);
#else
                    if(socket_execute->re_cmd)
                        free(socket_execute->re_cmd);
                    if(socket_execute->cmd_name)
                        free(socket_execute->cmd_name);
                    free(socket_execute);
#endif
                            pthread_mutex_unlock(&mutex_socket);
                        }
                        else
                        {
                            fail_flag = 1;

                            wd_DEBUG("######## socket item fail########\n");

                            break;
                        }
                        usleep(1000*20);
                    }
                    if(fail_flag)
                        break;

                    if(g_pSyncList[i]->copy_file_list->next == NULL)
                    {
                        break;
                    }
                    else
                    {
                        usleep(1000*100);
                    }
                }
                if(g_pSyncList[i]->server_action_list->next != NULL && g_pSyncList[i]->SocketActionList->head == NULL)
                {
                    free_action_item(g_pSyncList[i]->server_action_list);
                    g_pSyncList[i]->server_action_list = create_action_item_head();
                }
                pthread_mutex_lock(&mutex_receve_socket);
                if(g_pSyncList[i]->SocketActionList->head == NULL)
                    g_pSyncList[i]->receve_socket = 0;
                pthread_mutex_unlock(&mutex_receve_socket);
            }
            else
            {
                if(asus_cfg.prule[i]->rule == 2)
                {
#if 0
                    /*
                     fix upload only rule rm sync dir , can not create new sync dir;
                    */

                    if(g_pSyncList[i]->no_local_root)
                    {
                        my_mkdir_r(asus_cfg.prule[i]->path);   //have mountpath
                        send_action(asus_cfg.type,asus_cfg.prule[i]->path);
                        usleep(1000*10);
                        g_pSyncList[i]->no_local_root = 0;
                    }

                    /*
                     fix upload only initial failed,must run intial again();
                    */
                    if(finished_initial)
                    {
                        if( g_pSyncList[i]->init_completed != 1)
                        {
                            sync_initial_again(i);
                        }
                    }

                    /*upload only rule is not server pthread,so unfinished list del is here*/
                    status = do_unfinished(i);
#endif
                }
                while(exit_loop == 0)
                {
                    while(g_pSyncList[i]->SocketActionList->head != NULL && server_sync ==0 && exit_loop ==0)
                    {
                        has_socket = 1;
                        socket_execute=g_pSyncList[i]->SocketActionList->head;

                        wd_DEBUG("######## socket cmd : %s########\n",socket_execute->cmd_name);

                        status=cmd_parser(socket_execute->cmd_name,i,socket_execute->re_cmd);
                       DEBUG("after cmd_parser,status=%d\n", status);
                        if(status == 0 || status == SERVER_SPACE_NOT_ENOUGH )
                        {
                            wd_DEBUG("del socket item ok?\n");
                            pthread_mutex_lock(&mutex_socket);
                            socket_execute = queue_dequeue(g_pSyncList[i]->SocketActionList);
#if MEM_POOL_ENABLE
                    mem_free(socket_execute->cmd_name);
                    mem_free(socket_execute->re_cmd);
                    mem_free(socket_execute);
#else
                    if(socket_execute->re_cmd)
                        free(socket_execute->re_cmd);
                    if(socket_execute->cmd_name)
                        free(socket_execute->cmd_name);
                    free(socket_execute);
#endif
                            wd_DEBUG("del socket item ok\n");
                            pthread_mutex_unlock(&mutex_socket);
                        }
                        else if(status == LOCAL_FILE_LOST )
                        {
                            wd_DEBUG("del socket item ok?\n");
                            pthread_mutex_lock(&mutex_socket);
                            socket_execute = queue_dequeue_t(g_pSyncList[i]->SocketActionList);
#if MEM_POOL_ENABLE
                    mem_free(socket_execute->cmd_name);
                    mem_free(socket_execute->re_cmd);
                    mem_free(socket_execute);
#else
                    if(socket_execute->re_cmd)
                        free(socket_execute->re_cmd);
                    if(socket_execute->cmd_name)
                        free(socket_execute->cmd_name);
                    free(socket_execute);
#endif
                            wd_DEBUG("del socket item ok\n");
                            pthread_mutex_unlock(&mutex_socket);
                        }
                        else
                        {
                            fail_flag = 1;
                            break;
                        }
                        usleep(1000*20);
                    }
                    if(fail_flag)
                        break;

                    if(g_pSyncList[i]->copy_file_list->next == NULL)
                    {
                        break;
                    }
                    else
                    {
                        usleep(1000*100);
                    }
                }
                if(g_pSyncList[i]->server_action_list->next != NULL && g_pSyncList[i]->SocketActionList->head == NULL)
                {
                    free_action_item(g_pSyncList[i]->server_action_list);
                    g_pSyncList[i]->server_action_list = create_action_item_head();
                }
                pthread_mutex_lock(&mutex_receve_socket);
                if(g_pSyncList[i]->SocketActionList->head == NULL)
                {
                    g_pSyncList[i]->receve_socket = 0;
                }
                pthread_mutex_unlock(&mutex_receve_socket);
            }

        }
        local_sync = 0;
        pthread_mutex_lock(&mutex_socket);
        if(!exit_loop)
        {
            gettimeofday(&now, NULL);
            outtime.tv_sec = now.tv_sec + 2;
            outtime.tv_nsec = now.tv_usec * 1000;
            pthread_cond_timedwait(&cond_socket, &mutex_socket, &outtime);
        }
        pthread_mutex_unlock(&mutex_socket);
    }
}
int cmd_parser(char *cmd,int index,char *re_cmd)
{
    if( !strncmp(cmd,"exit",4))
    {

        wd_DEBUG("exit socket\n");

        return 0;
    }

    if(!strncmp(cmd,"rmroot",6))
    {
        g_pSyncList[index]->no_local_root = 1;
        return 0;
    }

    char cmd_name[64]="\0";
    char *path;
    char *temp;
    char filename[256]="\0";
    char *fullname;
    char *fullname_t = NULL;
    char oldname[256]="\0",newname[256]="\0";
    char *oldpath;
    char action[64]="\0";
    char *cmp_name;
    char *mv_newpath;
    char *mv_oldpath;
    char *ch;
    int status;

    ch = cmd;
    int i = 0;
    while(*ch != '\n')
    {
        i++;
        ch++;
    }

    memcpy(cmd_name, cmd, i);

    char *p = NULL;
    ch++;
    i++;

    temp = my_str_malloc((size_t)(strlen(ch)+1));

    strcpy(temp,ch);
    p = strchr(temp,'\n');

    path = my_str_malloc((size_t)(strlen(temp)- strlen(p)+1));


    if(p!=NULL)
        snprintf(path,strlen(temp)- strlen(p)+1,"%s",temp);

    p++;
    if(strncmp(cmd_name, "rename",strlen("rename")) == 0)
    {
        char *p1 = NULL;

        p1 = strchr(p,'\n');

        if(p1 != NULL)
            strncpy(oldname,p,strlen(p)- strlen(p1));

        p1++;

        strcpy(newname,p1);

        wd_DEBUG("cmd_name: [%s],path: [%s],oldname: [%s],newname: [%s]\n",cmd_name,path,oldname,newname);

        if(newname[0] == '.' || (strstr(path,"/.")) != NULL)
        {
            free(temp);
            free(path);
            return 0;
        }
    }
    else if(strncmp(cmd_name, "move",strlen("move")) == 0)
    {
        char *p1 = NULL;

        p1 = strchr(p,'\n');

        oldpath = my_str_malloc((size_t)(strlen(p)- strlen(p1)+1));

        if(p1 != NULL)
            snprintf(oldpath,strlen(p)- strlen(p1)+1,"%s",p);

        p1++;

        strcpy(oldname,p1);

        wd_DEBUG("cmd_name: [%s],path: [%s],oldpath: [%s],oldname: [%s]\n",cmd_name,path,oldpath,oldname);


        if(oldname[0] == '.' || (strstr(path,"/.")) != NULL)
        {
            free(temp);
            free(path);
            free(oldpath);
            return 0;
        }
    }
    else if(strcmp(cmd_name, "delete") == 0 || strcmp(cmd_name, "remove") == 0)
    {
        strcpy(filename,p);
        fullname = my_str_malloc((size_t)(strlen(path)+strlen(filename)+2));

        wd_DEBUG("cmd_name: [%s],path: [%s],filename: [%s]\n",cmd_name,path,filename);

        if(filename[0] == '.' || (strstr(path,"/.")) != NULL)
        {
            free(temp);
            free(path);
            return 0;
        }
    }
    else
    {
        strcpy(filename,p);
        fullname = my_str_malloc((size_t)(strlen(path)+strlen(filename)+2));
        if(re_cmd != NULL)
            fullname_t = my_str_malloc((size_t)(strlen(re_cmd)+strlen(filename)+2));

        wd_DEBUG("cmd_name: [%s],path: [%s],filename: [%s]\n",cmd_name,path,filename);

        if(filename[0] == '.' || (strstr(path,"/.")) != NULL)
        {
            free(temp);
            free(path);
            return 0;
        }
    }

    free(temp);

    if( !strncmp(cmd_name,"rename",strlen("rename")) )
    {
        cmp_name = my_str_malloc((size_t)(strlen(path)+strlen(newname)+2));
        sprintf(cmp_name,"%s/%s",path,newname);
    }
    else if( !strcmp(cmd_name,"delete") || !strcmp(cmd_name,"remove"))
    {
        cmp_name = my_str_malloc((size_t)(strlen(path)+strlen(filename)+2));
        sprintf(cmp_name,"%s/%s",path,filename);
    }
    else
    {
        if(re_cmd == NULL)
        {
            cmp_name = my_str_malloc((size_t)(strlen(path)+strlen(filename)+2));
            sprintf(cmp_name,"%s/%s",path,filename);
        }
        else
        {
            cmp_name = my_str_malloc((size_t)(strlen(re_cmd)+strlen(filename)+2));
            sprintf(cmp_name,"%s%s",re_cmd,filename);
        }
    }

    if( strcmp(cmd_name, "createfile") == 0 )
    {
        strcpy(action,"createfile");
        action_item *item;
        DEBUG("get_action_item18\n");
        item = get_action_item("copyfile",cmp_name,g_pSyncList[index]->copy_file_list,index);

        if(item != NULL)
        {

            wd_DEBUG("##### delete copyfile %s ######\n",cmp_name);

            del_action_item("copyfile",cmp_name,g_pSyncList[index]->copy_file_list);
        }
    }
    else if( strcmp(cmd_name, "cancelcopy") == 0 )
    {
        action_item *item;
        DEBUG("get_action_item19\n");
        item = get_action_item("copyfile",cmp_name,g_pSyncList[index]->copy_file_list,index);

        if(item != NULL)
        {

            wd_DEBUG("##### delete copyfile %s ######\n",cmp_name);

            del_action_item("copyfile",cmp_name,g_pSyncList[index]->copy_file_list);
        }
        free(path);
        free(cmp_name);
        free(fullname);
        return 0;
    }
    else if( strcmp(cmd_name, "remove") == 0  || strcmp(cmd_name, "delete") == 0)
    {
        strcpy(action,"remove");
    }
    else if( strcmp(cmd_name, "createfolder") == 0 )
    {
        strcpy(action,"createfolder");
    }
    else if( strncmp(cmd_name, "rename",strlen("rename")) == 0 )
    {
        strcpy(action,"rename");
    }
#if 1
    if(g_pSyncList[index]->server_action_list->next != NULL)
    {
        action_item *item;
        DEBUG("get_action_item20\n");
        item = get_action_item(action,cmp_name,g_pSyncList[index]->server_action_list,index);

        if(item != NULL)
        {

            wd_DEBUG("1##### %s %s by google Server self ######\n",action,cmp_name);

            del_action_item(action,cmp_name,g_pSyncList[index]->server_action_list);

            wd_DEBUG("#### del action item success!\n");

            free(path);
            if( strncmp(cmd_name, "rename",strlen("rename")) != 0 )
                free(fullname);
            free(cmp_name);
            return 0;
        }
    }
#endif
    free(cmp_name);

    wd_DEBUG("###### %s is start ######\n",cmd_name);

    if( strcmp(cmd_name, "copyfile") != 0 )
    {
        g_pSyncList[index]->have_local_socket = 1;
    }

    if( strcmp(cmd_name, "createfile") == 0 || strcmp(cmd_name, "dragfile") == 0 )
    {

        sprintf(fullname,"%s/%s",path,filename);
        char *serverpath=localpath_to_serverpath(fullname,index);      
        /************************/
        char *serverpath_1=localpath_to_serverpath(path,index);
        DEBUG("fullname=%s,serverpath=%s,path=%s,serverpath_1=%s\n",fullname,serverpath,path,serverpath_1);
        DEBUG("is_server_exist6\n");
        int exist = 0;
        exist=is_server_exist(serverpath_1,serverpath,NULL, index);
        if(exist == -2)
        {
            re_create_folder(path,serverpath_1,index);
        }
        /************************/
        if(re_cmd != NULL)
        {
            sprintf(fullname_t,"%s%s",re_cmd,filename);
            DEBUG("fullname_t=%s,re_cmd=%s,filename=%s\n",fullname_t,re_cmd,filename);
            DEBUG("upload4\n");
            if(exist ==1)
                status=s_upload_file(fullname_t,serverpath,0,index,6);
            else if(exist == 0)
                status=s_upload_file(fullname_t,serverpath,1,index,6);
        }
        else
        {
            DEBUG("upload5\n");
            if(exist == 1)
            {
                if( asus_cfg.prule[index]->rule  == 2)
                {
                    DEBUG("upload5,rule ==2\n");
                    char *newname;
                    newname=change_server_same_name(serverpath,NULL, index);
                    DEBUG("upload5!api_move \n");
                    s_move(serverpath,newname,index,0,NULL,6);
                    status=s_upload_file(fullname,serverpath,1,index,6);
                }
                else
                    status=s_upload_file(fullname,serverpath,0,index,6);
            }
            else if(exist == 0)
            {
                status=s_upload_file(fullname,serverpath,1,index,6);
            }
            else
                status = -1;
        }

        /*third param flase,false=>if server file has exit ,will not be overwrite*/
        if(status == 0)
        {
            char *newlocalname = NULL;
            cJSON *json = dofile(Con(TMP_R,upload_chunk_commit.txt));
            time_t mtime=cJSON_printf_mtime(json);
            cJSON_Delete(json);
            if(re_cmd == NULL)
                newlocalname=change_local_same_file(fullname,index);
            else
                newlocalname=change_local_same_file(fullname_t,index);
            DEBUG("newlocalname=%s\n", newlocalname);
            if(newlocalname)
                ChangeFile_modtime(newlocalname,mtime);
            if(re_cmd)
                free(fullname_t);
            free(fullname);
            free(newlocalname);
            free(serverpath);
            free(serverpath_1);
        }
        else
        {

            wd_DEBUG("upload %s failed\n",fullname);
            if(re_cmd)
                free(fullname_t);
            free(path);
            free(fullname);
            free(serverpath);
            free(serverpath_1);
            return status;
        }
    }
    else if( strcmp(cmd_name, "copyfile") == 0 )
    {
        if(re_cmd == NULL)
        {
            sprintf(fullname,"%s/%s",path,filename);
            add_action_item("copyfile",fullname,g_pSyncList[index]->copy_file_list);
        }
        else
        {
            sprintf(fullname_t,"%s%s",re_cmd,filename);
            add_action_item("copyfile",fullname_t,g_pSyncList[index]->copy_file_list);
        }
        free(fullname);
        if(re_cmd)
            free(fullname_t);
    }
    else if( strcmp(cmd_name, "modify") == 0 )
    {
        time_t mtime_1,mtime_2;

        sprintf(fullname,"%s/%s",path,filename);
        char *serverpath=localpath_to_serverpath(fullname,index);
        if(re_cmd != NULL)
            sprintf(fullname_t,"%s%s",re_cmd,filename);

        CloudFile *filetmp;
        filetmp=get_CloudFile_node(g_pSyncList[index]->OldServerRootNode,serverpath,0x2);
        if(filetmp == NULL)
        {
            mtime_1=(time_t)-1;
        }
        else
        {
            mtime_1=filetmp->mtime;
        }
        mtime_2=api_getmtime(filetmp->id, serverpath,dofile);
        DEBUG("mtime_2 = %lu,mtime_1=%lu\n", mtime_2, mtime_1);
        if(mtime_2 == -1)
        {
            /*only upload */
            /*third param ture,ture=>if server file has exit ,will be overwrite*/
            if(re_cmd == NULL)
                {
                DEBUG("upload6\n");
                status=s_upload_file(fullname,serverpath,1,index,6);
                }
            else
                {
                DEBUG("upload7\n");
                status=s_upload_file(fullname_t,serverpath,1,index,6);
                }
            if(status == 0)
            {
                cJSON *json = dofile(Con(TMP_R,upload_chunk_commit.txt));
                time_t mtime=cJSON_printf_mtime(json);
                cJSON_Delete(json);
                if(re_cmd == NULL)
                    ChangeFile_modtime(fullname,mtime);
                else
                    ChangeFile_modtime(fullname_t,mtime);
                if(re_cmd)
                    free(fullname_t);
                free(fullname);
                free(serverpath);
            }
            else
            {

                wd_DEBUG("upload %s failed\n",fullname);
                if(re_cmd)
                    free(fullname_t);
                free(path);
                free(fullname);
                free(serverpath);
                return status;
            }
        }
        else if(mtime_2 != -1 && mtime_2 != -2)
        {
            if(mtime_1 == mtime_2)
            {
                /*third param ture,ture=>if server file has exit ,will be overwrite*/
                DEBUG("modify, re_cmd = %s\n", re_cmd);
                if(re_cmd == NULL)
                    {
                    DEBUG("upload8\n");
                    status=s_upload_file(fullname,serverpath,0,index,6);
                    }
                else
                    {
                    DEBUG("upload9\n");
                    status=s_upload_file(fullname_t,serverpath,0,index,6);
                    }
                if(status == 0)
                {
                    cJSON *json = dofile(Con(TMP_R,upload_chunk_commit.txt));
                    time_t mtime=cJSON_printf_mtime(json);
                    cJSON_Delete(json);
                    if(re_cmd == NULL)
                        ChangeFile_modtime(fullname,mtime);
                    else
                        ChangeFile_modtime(fullname_t,mtime);
                    if(re_cmd)
                        free(fullname_t);
                    free(fullname);
                    free(serverpath);
                }
                else
                {

                    wd_DEBUG("upload %s failed\n",fullname);
                    if(re_cmd)
                        free(fullname_t);
                    free(path);
                    free(fullname);
                    free(serverpath);
                    return status;
                }
            }
            else
            {
                /*rename then upload*/
                /*third param false,false=>if server file has exit ,will not be overwrite*/
                if(re_cmd == NULL)
                    {
                    DEBUG("upload10\n");
                    status=s_upload_file(fullname,serverpath,0,index,6);
                    }
                else
                    {
                    DEBUG("upload11\n");
                    status=s_upload_file(fullname_t,serverpath,0,index,6);
                    }
                if(status == 0)
                {
                    char *newlocalname = NULL;
                    cJSON *json = dofile(Con(TMP_R,upload_chunk_commit.txt));
                    time_t mtime=cJSON_printf_mtime(json);
                    cJSON_Delete(json);
                    if(re_cmd == NULL)
                        newlocalname=change_local_same_file(fullname,index);
                    else
                        newlocalname=change_local_same_file(fullname_t,index);
                    if(newlocalname)
                        ChangeFile_modtime(newlocalname,mtime);
                    if(re_cmd)
                        free(fullname_t);
                    free(newlocalname);
                    free(fullname);
                    free(serverpath);
                }
                else
                {

                    wd_DEBUG("upload %s failed\n",fullname);

                    if(re_cmd)
                        free(fullname_t);
                    free(path);
                    free(fullname);
                    free(serverpath);
                    return status;
                }
            }
        }
        else
        {
            if(re_cmd)
                free(fullname_t);
            free(path);
            free(fullname);
            free(serverpath);
            return -1;
        }
#if 0
        if(mtime_1 == mtime_2 || mtime_2 == -1 ||asus_cfg.prule[index]->rule == 2)
        {
            /*third param ture,ture=>if server file has exit ,will be overwrite*/
            status=s_upload_file(fullname,serverpath,1,index,6);
            if(status == 0)
            {
                time_t mtime=cJSON_printf(dofile(Con(TMP_R,upload_chunk_commit.txt),"modified");
                ChangeFile_modtime(fullname,mtime);
                free(fullname);
                free(serverpath);
            }
            else
            {

                wd_DEBUG("upload %s failed\n",fullname);
                free(path);
                free(fullname);
                free(serverpath);
                return status;
            }
        }
        else if(mtime_1 !=mtime_2 && mtime_2 != -1 && asus_cfg.prule[index]->rule ==0)
        {
            /*third param false,false=>if server file has exit ,will not be overwrite*/
            status=s_upload_file(fullname,serverpath,0,index,6);
            if(status == 0)
            {
                char *newlocalname;
                time_t mtime=cJSON_printf(dofile(Con(TMP_R,upload_chunk_commit.txt),"modified");
                newlocalname=change_local_same_file(fullname,index);
                if(newlocalname)
                    ChangeFile_modtime(newlocalname,mtime);
                free(newlocalname);
                free(fullname);
                free(serverpath);
            }
            else
            {

                wd_DEBUG("upload %s failed\n",fullname);
                free(path);
                free(fullname);
                free(serverpath);
                return status;
            }
        }
        else if(mtime_2 == -2) //when the network is not conncet ,the curl resporen -2;
        {
            free(path);
            free(fullname);
            free(serverpath);
            return -1;
        }
#endif
    }
    else if(strcmp(cmd_name, "delete") == 0  || strcmp(cmd_name, "remove") == 0)
    {
        DEBUG("@@@@@@@@@@@@@@delete/remove@@@@@@@@@@@@@\n");
        action_item *item;
        char delete_id1[64] = {0};
        item = get_action_item_access("upload",fullname,g_pSyncList[index]->access_failed_list,index);
        if(item != NULL)
        {
            del_action_item("upload",item->path,g_pSyncList[index]->access_failed_list);
        }

        sprintf(fullname,"%s/%s",path,filename);
        char *serverpath=localpath_to_serverpath(fullname,index);
        DEBUG("path = %s,filename = %s\n", path, filename);
        char *serverpath_1=localpath_to_serverpath(path,index);
        if(strcmp(serverpath_1,"") == 0)
        {           
            //serverpath_1 = "/";   //给serverpath_1申请的内存还没释放就让它指向另一块内存,后面再释放会产生问题
            strcpy(serverpath_1, "/");
        }
        DEBUG("is_server_exist3\n");
        int exist=0;
        exist = is_server_exist(serverpath_1,serverpath,NULL, index);
        strcpy(delete_id1, delete_id);
        if(exist == 0)
        {
            my_free(serverpath_1);
            my_free(serverpath);
            free(fullname);
            free(path);
            return 0;
        }
        else if(exist == -1)
        {
            my_free(serverpath_1);
            my_free(serverpath);
            free(fullname);
            free(path);
            return -1;
        }

        status=s_delete(serverpath,index, delete_id1,6);

        if(status != 0)
        {

            wd_DEBUG("delete failed\n");

            free(path);
            free(fullname);
            free(serverpath);
            return status;
        }
        free(fullname);
        free(serverpath);
    }
    else if(strncmp(cmd_name, "move",strlen("move")) == 0 || strncmp(cmd_name, "rename",strlen("rename")) == 0)
    {
        if(strncmp(cmd_name, "move",strlen("move")) == 0)
        {
            mv_newpath = my_str_malloc((size_t)(strlen(path)+strlen(oldname)+2));
            mv_oldpath = my_str_malloc((size_t)(strlen(oldpath)+strlen(oldname)+2));
            sprintf(mv_newpath,"%s/%s",path,oldname);
            sprintf(mv_oldpath,"%s/%s",oldpath,oldname);
            free(oldpath);
            if(re_cmd)
            {
                fullname_t = (char *)malloc(sizeof(char)*(strlen(re_cmd)+strlen(oldname)+1));
                memset(fullname_t,'\0',strlen(re_cmd)+strlen(oldname)+1);
                sprintf(fullname_t,"%s%s",re_cmd,oldname);
            }
        }
        else
        {
            mv_newpath = my_str_malloc((size_t)(strlen(path)+strlen(newname)+2));
            mv_oldpath = my_str_malloc((size_t)(strlen(path)+strlen(oldname)+2));
            sprintf(mv_newpath,"%s/%s",path,newname);
            sprintf(mv_oldpath,"%s/%s",path,oldname);
            if(re_cmd)
            {
                fullname_t = (char *)malloc(sizeof(char)*(strlen(re_cmd)+strlen(newname)+1));
                memset(fullname_t,'\0',strlen(re_cmd)+strlen(newname)+1);
                sprintf(fullname_t,"%s%s",re_cmd,newname);
            }
        }
        if(strncmp(cmd_name, "rename",strlen("rename")) == 0)
        {            
                int exist=0;
                char *serverpath=localpath_to_serverpath(mv_newpath,index);
                char *serverpath_old=localpath_to_serverpath(mv_oldpath,index);
                /*if the newer name has exist in server,the server same name will be rename ,than rename the oldname to newer*/
                char *serverpath_1=localpath_to_serverpath(path,index);
                DEBUG("is_server_exist4\n");
                exist=is_server_exist(serverpath_1,serverpath,NULL, index);
                if(exist == 1)
                {
                    char *newname;
                    newname=change_server_same_name(serverpath,NULL, index);
                    DEBUG("api_move1!\n");
                    status = s_move(serverpath,newname,index,0,NULL,6);


                    if(status == 0)
                    {
                        char *err_msg = write_error_message("server file %s is renamed to %s",serverpath,newname);
                        write_trans_excep_log(serverpath,3,err_msg);
                        free(err_msg);
                        DEBUG("api_move2!\n");             
                        status = s_move(serverpath_old,serverpath,index,1,fullname_t,6);
                    }
                    free(newname);
                }
                else
                {
                    free_server_tree(g_pSyncList[index]->ServerRootNode);
                    g_pSyncList[index]->ServerRootNode = NULL;
                    g_pSyncList[index]->ServerRootNode = create_server_treeroot();
                    DEBUG("browse_to_tree9\n");
                    status = s_browse_to_tree("/", rootid,g_pSyncList[index]->ServerRootNode, index,6);
                    DEBUG("api_move3!\n");
                    status = s_move(serverpath_old,serverpath,index,1,fullname_t,6);
                }
                free(serverpath);
                free(serverpath_old);
                free(serverpath_1);
                if(re_cmd)
                    free(fullname_t);
                if(status != 0)
                {

                    wd_DEBUG("move/rename failed\n");
                    free(mv_oldpath);
                    free(mv_newpath);
                    free(path);
                    return status;
                }
        }
        else  /*action : move*/
        {
            int exist = 0;
            int old_index;
            old_index=get_path_to_index(mv_oldpath);
            /*the move file is in other rules folder or root path*/
            /*index is 0 or 2
              0=>sync
              2=>upload
            */
            if(asus_cfg.prule[old_index]->rule == 1)
            {
                del_download_only_action_item("move",mv_oldpath,g_pSyncList[old_index]->download_only_socket_head);
            }

            char *serverpath=localpath_to_serverpath(mv_newpath,index);
            char *serverpath_old=localpath_to_serverpath(mv_oldpath,index);
            char *serverpath_1=localpath_to_serverpath(path,index);
            DEBUG("is_server_exist5\n");
            exist=is_server_exist(serverpath_1,serverpath,NULL, index);
            if(exist == 1)
            {
                char *newname;
                newname=change_server_same_name(serverpath,NULL, index);
                DEBUG("api_move4!\n");
                status = s_move(serverpath,newname,index,0,NULL,6);

                if(status == 0)
                {
                    char *err_msg = write_error_message("server file %s is renamed to %s",serverpath,newname);
                    write_trans_excep_log(serverpath,3,err_msg);
                    free(err_msg);
                    DEBUG("api_move5!\n");
                    status = s_move(serverpath_old,serverpath,index,1,fullname_t,6);
                }

                free(newname);
            }
            else
            {
                DEBUG("api_move6!\n");
                status = s_move(serverpath_old,serverpath,index,1,fullname_t,6);
            }
            free(serverpath);
            free(serverpath_old);
            free(serverpath_1);
            if(re_cmd)
                free(fullname_t);
            if(status != 0)
            {
                wd_DEBUG("move/rename failed\n");

                free(path);
                free(mv_oldpath);
                free(mv_newpath);
                return status;
            }
        }
        free(mv_oldpath);
        free(mv_newpath);

    }
    else if( strcmp(cmd_name,"dragfolder") == 0)
    {
        char info[512];
        memset(info,0,sizeof(info));
        if(re_cmd == NULL)
        {
            sprintf(fullname,"%s/%s",path,filename);
            sprintf(info,"createfolder%s%s%s%s",CMD_SPLIT,path,CMD_SPLIT,filename);
        }
        else
        {
            sprintf(fullname,"%s%s",re_cmd,filename);
            sprintf(info,"createfolder%s%s%s%s",CMD_SPLIT,re_cmd,CMD_SPLIT,filename);
        }


        pthread_mutex_lock(&mutex_socket);
        add_socket_item(info);
        pthread_mutex_unlock(&mutex_socket);
        deal_dragfolder_to_socketlist(fullname,index);
        if(re_cmd)
            free(fullname_t);
        free(fullname);
    }
    else if(strcmp(cmd_name, "createfolder") == 0)
    {
        int exist;
        int exist2;
        sprintf(fullname,"%s/%s",path,filename);
        char *serverpath_1 = localpath_to_serverpath(path,index);
        char *serverpath=localpath_to_serverpath(fullname,index);
        DEBUG("create folder,is_server_exist");
        exist2=is_server_exist(serverpath_1,serverpath,NULL, index);
        if(exist2 == 1)
        {
            DEBUG("server exist this folder!!!\n");
            return 0;
        }
        else if(exist == -2)
        {
            DEBUG("cmd is creater folder,couldn't find parent folder\n");
            return -1;
        }
        if(re_cmd != NULL)
        {
            sprintf(fullname_t,"%s%s",re_cmd,filename);
        }
        char newfolderid[32] = {0};
        if(re_cmd == NULL)
        {
            DEBUG("create folder4\n");
            status=s_create_folder(fullname,serverpath, newfolderid, index, 0,6);
        }
            else
            {
            DEBUG("create folder5\n");
            status=s_create_folder(fullname_t,serverpath, newfolderid, index, 0,6);
           }
            if(status==0)
            {
                DEBUG("before upload_serverlist_,fullname = %s\n",fullname);
                                if(re_cmd == NULL)
                                    upload_serverlist_(index,fullname,serverpath,newfolderid);
                                else
                                    upload_serverlist_(index,fullname_t,serverpath,newfolderid);
                                DEBUG("4600 line\n");
            exist=parse_create_folder(Con(TMP_R,create_folder.txt));
            if(exist)
            {
#if 0
                char *newname;
                newname=change_server_same_name(serverpath,index);

                status = s_move(serverpath,newname,index,0,NULL,6);

                free(newname);
                if(status ==0)
                {
                    if(re_cmd == NULL)
                        status=s_create_folder(fullname,serverpath,6);
                    else
                        status=s_create_folder(fullname_t,serverpath,6);
                }
#endif
            }

            if(re_cmd)
                free(fullname_t);
            free(serverpath);
            free(fullname);
        }
        else
        {

            wd_DEBUG("createFolder failed status = %d\n",status);

            free(path);
            return status;
        }
    }
    free(path);
    return 0;

}

int deal_dragfolder_to_socketlist(char *dir,int index)
{
    wd_DEBUG("dir = %s\n",dir);

    int status;
    struct dirent *ent = NULL;
    char info[512];
    memset(info,0,sizeof(info));
    DIR *pDir;
    pDir=opendir(dir);
    if(pDir != NULL)
    {
        while((ent=readdir(pDir)) != NULL)
        {
            if(ent->d_name[0] == '.')
                continue;
            if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
                continue;
            char *fullname;
            fullname = (char *)malloc(sizeof(char)*(strlen(dir)+strlen(ent->d_name)+2));
            memset(fullname,'\0',strlen(dir)+strlen(ent->d_name)+2);

            sprintf(fullname,"%s/%s",dir,ent->d_name);
            if(test_if_dir(fullname) == 1)
            {
                sprintf(info,"createfolder%s%s%s%s",CMD_SPLIT,dir,CMD_SPLIT,ent->d_name);
                pthread_mutex_lock(&mutex_socket);
                add_socket_item(info);
                pthread_mutex_unlock(&mutex_socket);
                status = deal_dragfolder_to_socketlist(fullname,index);
            }
            else
            {
                sprintf(info,"createfile%s%s%s%s",CMD_SPLIT,dir,CMD_SPLIT,ent->d_name);
                pthread_mutex_lock(&mutex_socket);
                add_socket_item(info);
                pthread_mutex_unlock(&mutex_socket);
            }
            free(fullname);
        }
        closedir(pDir);
        return 0;
    }
}


int get_path_to_index(char *path)
{
    int i;
    char *root_path = NULL;
    char *temp = NULL;
    root_path = my_str_malloc(512);

    temp = my_nstrchr('/',path,4);
    if(temp == NULL)
    {
        sprintf(root_path,"%s",path);
    }
    else
    {
        snprintf(root_path,strlen(path)-strlen(temp)+1,"%s",path);
    }

    for(i=0;i<asus_cfg.dir_number;i++)
    {
        if(!strcmp(root_path,asus_cfg.prule[i]->base_path))
            break;
    }

    wd_DEBUG("get_path_to_index root_path = %s\n",root_path);

    free(root_path);

    return i;
}

void sig_handler (int signum)
{
    wd_DEBUG("sig_handler,signum is %d\n",signum);
    switch (signum)
    {
#if TOKENFILE
    case SIGTERM:case SIGUSR2:
        {
            int mountflag = 0;
            if(signum == SIGUSR2)
            {

                wd_DEBUG("signal is SIGUSR2\n");

                FILE *fp;
                fp = fopen("/proc/mounts","r");
                char a[10240];
                memset(a,'\0',sizeof(a));
                fread(a,10240,1,fp);
                fclose(fp);
                if(strstr(a,"/dev/sd"))
                {
                    mountflag = 1;
                }

            }
            if(signum == SIGTERM || mountflag == 0)
            {
                DEBUG("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!signum=%d,mountflag=%d\n",signum,mountflag);
                stop_progress = 1;
                exit_loop = 1;
                DEBUG("g->SIGTERM:exit_loop=%d\n", exit_loop);
                set_iptables(0);

#ifndef NVRAM_
                char cmd_p[128] = {0};
                sprintf(cmd_p,"sh %s",Google_Get_Nvram);
                system(cmd_p);
                sleep(2);
                if(create_webdav_conf_file(&asus_cfg_stop) == -1)
                {
                    wd_DEBUG("create_google_conf_file fail\n");
                    return;
                }

#else
                printf("convert_nvram_to_file_mutidir2\n");
                if(convert_nvram_to_file_mutidir(CONFIG_PATH,&asus_cfg_stop) == -1)
                {
                    wd_DEBUG("convert_nvram_to_file fail\n");
                    DEBUG("write_to_nvram('', 'gd_tokenfile')1\n");
                    write_to_nvram("","gd_tokenfile");
                    return;
                }
#endif
                DEBUG("asus_cfg_stop.dirnumber=%d\n", asus_cfg_stop.dir_number);
                if(asus_cfg_stop.dir_number == 0)
                {
                    char *filename;
                    filename = my_str_malloc(strlen(asus_cfg.prule[0]->base_path)+20+1);
                    sprintf(filename,"%s/.google_tokenfile",asus_cfg.prule[0]->base_path);
                    remove(filename);
                    free(filename);

                    remove(g_pSyncList[0]->conflict_file);

#ifdef NVRAM_
                    DEBUG("write_to_nvram('', 'gd_tokenfile')2\n");
                    write_to_nvram("","gd_tokenfile");
#else
                    write_to_wd_tokenfile("");
#endif

                }
                else
                {
                    if(asus_cfg_stop.dir_number != asus_cfg.dir_number)
                    {
                        parse_config_mutidir(CONFIG_PATH,&asus_cfg_stop);

                        rewrite_tokenfile_and_nv();
                    }
                }
                sighandler_finished = 1;
                DEBUG("g->sighandler_finished=%d\n", sighandler_finished);
                pthread_cond_signal(&cond);
                pthread_cond_signal(&cond_socket);
                pthread_cond_signal(&cond_log);
            }
            else
            {
                disk_change = 1;
                sighandler_finished = 1;
            }
            break;
        }
#else
    case SIGTERM:
        {
            wd_DEBUG("signal is SIGTERM\n");
            stop_progress = 1;
            exit_loop = 1;

            pthread_cond_signal(&cond);
            pthread_cond_signal(&cond_socket);
            pthread_cond_signal(&cond_log);

            break;
        }
#endif
    case SIGPIPE:  // delete user
        wd_DEBUG("signal is SIGPIPE\n");
        signal(SIGPIPE, SIG_IGN);
        break;

    default:
        break;
    }
}

void* sigmgr_thread(){
    sigset_t   waitset;
    int        sig;
    int        rc;
    pthread_t  ppid = pthread_self();

    pthread_detach(ppid);

    sigemptyset(&waitset);
    sigaddset(&waitset,SIGUSR1);
    sigaddset(&waitset,SIGUSR2);
    sigaddset(&waitset,SIGTERM);
    sigaddset(&waitset,SIGPIPE);

    while (1)  {
        rc = sigwait(&waitset, &sig);
        if (rc != -1) {

            wd_DEBUG("@@@@@@@@@@@@@@@@google sigwait() fetch the signal - %d\n", sig);

            sig_handler(sig);
        } else {
            wd_DEBUG("sigwaitinfo() returned err: %d; %s\n", errno, strerror(errno));
        }
    }
}
void stop_process_clean_up(){
#if TOKENFILE
    unlink("/tmp/notify/usb/google_client");
#endif
    pthread_cond_destroy(&cond);
    pthread_cond_destroy(&cond_socket);
    pthread_cond_destroy(&cond_log);

    if(!access_token_expired)
        unlink(general_log);
}

char *change_local_same_file(char *oldname,int index)
{
    cJSON *json;
    char  *title, *tmpname;
    char newname[1024] = {0};
    if (access(Con(TMP_R,upload_chunk_commit.txt),R_OK) ==0)
    {
        json=dofile(Con(TMP_R,upload_chunk_commit.txt));
        if(json)
        {
            char *localpath;

            localpath = (char *)malloc(257+asus_cfg.prule[index]->base_path_len+asus_cfg.prule[index]->rooturl_len);
            memset(localpath, '\0', sizeof(localpath));
            title=cJSON_parse_name(json);
            DEBUG("oldname = %s\n", oldname);
            DEBUG("title = %s\n", title);
            tmpname = strrchr(oldname, '/');
            snprintf(newname, strlen(oldname)-strlen(tmpname) + 2, oldname);
            sprintf(newname, "%s%s", newname, title);
            DEBUG("newname = %s\n", newname);
            strcpy(localpath, newname);
            DEBUG("localpath = %s\n", localpath);
            if(strcmp(newname,oldname)!=0)
            {
#ifndef TEST
                add_action_item("rename",localpath,g_pSyncList[index]->server_action_list);
                rename(oldname,localpath);

                char *err_msg = write_error_message("server has exist %s file ,rename local %s to %s",oldname,oldname,localpath);
                write_trans_excep_log(oldname,3,err_msg);
                free(err_msg);

#endif
                cJSON_Delete(json);
                DEBUG("return localpath1: %s\n", localpath);
                return localpath;
            }
            else
            {
                cJSON_Delete(json);
                DEBUG("return localpath: %s\n", localpath);
                return localpath;
            }
        }
        else
        {
            /*the file contents is error*/
            DEBUG("return localpath3: NULL\n");
            return NULL;
        }
    }


}

void check_tokenfile()
{
        if(get_tokenfile_info()==-1)
        {
            wd_DEBUG("\nget_tokenfile_info failed\n");
            exit(-1);
        }
        DEBUG("before check_config_path\n");
        check_config_path(1);
}
int check_link_internet()
{
    struct timeval now;
    struct timespec outtime;
    int link_flag = 0;
    int i;
#if defined NVRAM_
    while(!link_flag && !exit_loop)
    {
#ifndef USE_TCAPI
        char *link_internet = strdup(nvram_safe_get("link_internet"));
#else
        char tmp[MAXLEN_TCAPI_MSG] = {0};
        tcapi_get(WANDUCK, "link_internet", tmp);
        char *link_internet = my_str_malloc(strlen(tmp)+1);
        sprintf(link_internet,"%s",tmp);
#endif
        link_flag = atoi(link_internet);
        if(!link_flag)
        {
            for(i=0;i<asus_cfg.dir_number;i++)
            {
                write_log(S_ERROR,"Network Connection Failed","",i);
            }

            pthread_mutex_lock(&mutex);
            if(!exit_loop)
            {
                gettimeofday(&now, NULL);
                outtime.tv_sec = now.tv_sec + 20;
                outtime.tv_nsec = now.tv_usec * 1000;
                pthread_cond_timedwait(&cond, &mutex, &outtime);
            }
            pthread_mutex_unlock(&mutex);
        }
        free(link_internet);
    }
#else

    do{
        char cmd_p[128] ={0};
        sprintf(cmd_p,"sh %s",Google_Get_Nvram_Link);
        system(cmd_p);
        sleep(2);

        char nv[64] = {0};
        FILE *fp;
        fp = fopen(TMP_NVRAM_VL,"r");
        fgets(nv,sizeof(nv),fp);
        fclose(fp);

        link_flag = atoi(nv);
        if(!link_flag)
        {
            for(i=0;i<asus_cfg.dir_number;i++)
            {
                write_log(S_ERROR,"Network Connection Failed","",i);
            }
            pthread_mutex_lock(&mutex);
            if(!exit_loop)
            {
                gettimeofday(&now, NULL);
                outtime.tv_sec = now.tv_sec + 20;
                outtime.tv_nsec = now.tv_usec * 1000;
                pthread_cond_timedwait(&cond, &mutex, &outtime);
            }
            pthread_mutex_unlock(&mutex);
        }

    }while(!link_flag && !exit_loop);
#endif

    for(i=0;i<asus_cfg.dir_number;i++)
    {
        write_log(S_SYNC,"","",i);
    }
}
#ifdef OAuth1
int do_auth()
{
    int curl_res=0;
    int auth_ok=0;
    int error_time = 0;
    int have_error_log=0;
    int i;
    do
    {
        wd_DEBUG("##########begin auth############\n");

        if(error_time > 5)
        {
            check_link_internet();
            error_time = 0;
        }
        if(exit_loop)
            return auth_ok;
        curl_res = get_request_token();

        if(curl_res == -1)
        {
            error_time++;
            continue;
        }
        curl_res = open_login_page_first();
        if(curl_res == -1)
        {
            error_time++;
            continue;
        }
        curl_res = login_first();
        if(curl_res == -1)
        {
            error_time++;
            continue;
        }
        curl_res = login_second();
        if(curl_res == -1)
        {
            error_time++;
            continue;
        }
        curl_res = login_second_submit();
        if(curl_res == -1)
        {
            error_time++;
            continue;
        }

        curl_res=get_access_token();
        if(curl_res == -1 || curl_res > 0)
        {
            if(curl_res == -1)
            {
                for(i=0;i<asus_cfg.dir_number;i++)
                {
                    write_log(S_ERROR,"Authentication Failed","",i);
                }

                return auth_ok;
            }
            else
                error_time++;
            continue;
        }
        auth_ok=1;
    }while(auth_ok != 1 && exit_loop != 1);
    wd_DEBUG("##########auth over############\n");

    return auth_ok;
}
#endif
int f_exists(const char *path)	// note: anything but a directory
{
        struct stat st;
        return (stat(path, &st) == 0) && (!S_ISDIR(st.st_mode));
}
void clean_sys_data()
{
    if(f_exists("/tmp/smartsync/.logs/google"))
        unlink("/tmp/smartsync/.logs/google");
}

void db_disbale_access_token()
{
    my_free(auth->oauth_token);
    my_free(auth);
}

int main(int args,char *argc[])
{

#ifndef TEST
    clean_sys_data();
    int auth_flag = 0;
    sigset_t bset,oset;
    pthread_t sig_thread;
    /*
    int res;
    res = curl_global_init(CURL_GLOBAL_DEFAULT);
*/
    curl_global_init(CURL_GLOBAL_ALL);
    sigemptyset(&bset);
    sigaddset(&bset,SIGUSR1);
    sigaddset(&bset,SIGUSR2);
    sigaddset(&bset,SIGTERM);
    sigaddset(&bset,SIGPIPE);

    if( pthread_sigmask(SIG_BLOCK,&bset,&oset) == -1)
        wd_DEBUG("!! Set pthread mask failed\n");

    if( pthread_create(&sig_thread,NULL,(void *)sigmgr_thread,NULL) != 0)
    {
        wd_DEBUG("thread creation failder\n");
        exit(-1);
    }
    init_globar_var();

#if TOKENFILE
    DEBUG("@@@@@@@@@@@@@@write_notify_file\n");
    write_notify_file(NOTIFY_PATH,SIGUSR2);
#ifndef NVRAM_
     my_mkdir("/opt/etc/.smartsync");
#ifndef WIN_32
    write_get_nvram_script_va("link_internet");
    write_get_nvram_script();
    char cmd_p[128] ={0};
    sprintf(cmd_p,"sh %s",Google_Get_Nvram);
    system(cmd_p);
    sleep(2);
#endif
#else
    create_shell_file();
#endif
#endif

    auth=(Auth *)malloc(sizeof(Auth));
    memset(auth,0,sizeof(auth));


    read_config();
    if(write_rootca() != 0)
        exit(-1);

#ifdef OAuth1
    auth_flag = do_auth();
#else
    auth_flag = 1;
    DEBUG("my_str_malloc1\n");
    auth->oauth_token = my_str_malloc(strlen(asus_cfg.pwd)+1);
    sprintf(auth->oauth_token,"%s",asus_cfg.pwd);
#endif

    if(auth_flag)
    {
#if TOKENFILE
        check_tokenfile();
#endif
        if(no_config == 0)
        {
            run();
        }

        pthread_join(sig_thread,NULL);
        stop_process_clean_up();
    }
    db_disbale_access_token();
    curl_global_cleanup();

#else
    s_upload_file("/tmp/mnt/ASD/My410Sync/aicloud_1.0.0.10-0-gd8a47cb_mipsel.ipk",oauth_url_escape("/aicloud_1.0.0.10-0-gd8a47cb_mipsel.ipk"),0,0,6);
#endif
}
#ifdef OAuth1
int get_request_token(){

    CURL *curl;
    CURLcode res;
    FILE *fp;
    char *header;
    static const char buf[]="Expect:";
    header=makeAuthorize(1);
    struct curl_slist *headerlist=NULL;
    curl=curl_easy_init();
    headerlist=curl_slist_append(headerlist,header);
    if(curl){
        curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,2L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_CAINFO, CA_INFO_FILE);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        //curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);
        curl_easy_setopt(curl,CURLOPT_URL,"https://api.google.com/1/oauth/request_token");
        CURL_DEBUG;
        curl_easy_setopt(curl,CURLOPT_HTTPHEADER,headerlist);
        fp=fopen(Con(TMP_R,data_1.txt),"w");
        curl_easy_setopt(curl,CURLOPT_WRITEDATA,fp);
        res=curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        fclose(fp);
        curl_slist_free_all(headerlist);
        if(res!=0){
            free(header);
            wd_DEBUG("get_request_token [%d] failed!\n",res);
            return -1;
        }
    }
    free(header);

    if(parse(Con(TMP_R,data_1.txt),1) == -1)
        return -1;
    return 0;
}

int cgi_init(char *query)
{
    int len, nel;
    char *q, *name, *value;
    /* Parse into individualassignments */
    q = query;
    len = strlen(query);
    nel = 1;
    while (strsep(&q, "&"))
        nel++;
    for (q = query; q< (query + len);) {
        value = name = q;
        /* Skip to next assignment */
        for (q += strlen(q); q < (query +len) && !*q; q++);
        /* Assign variable */
        name = strsep(&value,"=");
        if(strcmp(name,"oauth_token_secret")==0)
            strcpy(auth->oauth_token_secret,value);
        else if(strcmp(name,"oauth_token")==0)
            strcpy(auth->oauth_token,value);
        else if(strcmp(name,"uid")==0)
            auth->uid=atoi(value);

    }
    wd_DEBUG("CGI[value] :%s %s %d\n", auth->oauth_token_secret,auth->oauth_token,auth->uid);
    return 0;
}

int parse(char *filename,int flag)
{

    char *p=NULL;
    char p1[56];
    char *p3=NULL;
    char *p4=NULL;
    FILE *fp=fopen(filename,"r");
    char tmp_data[256]="\0";
    fgets(tmp_data,sizeof(tmp_data),fp);
    wd_DEBUG("%s\n",tmp_data);
    if(strstr(tmp_data,"error")==NULL && tmp_data != NULL&&strlen(tmp_data) > 0)
    {
        switch(flag)
        {
        case 1:
            p=strchr(tmp_data,'&');
            memset(p1,'\0',sizeof(p1));
            strncpy(p1,tmp_data,strlen(tmp_data)-strlen(p));
            p3=strchr(p,'=');
            p3++;
            strcpy(auth->tmp_oauth_token,p3);
            p3=NULL;
            p4=strchr(p1,'=');
            p4++;
            snprintf(auth->tmp_oauth_token_secret,strlen(p4)+1,"%s",p4);
            wd_DEBUG("auth->tmp_oauth_token=%s\nauth->tmp_oauth_token_secret=%s\n",auth->tmp_oauth_token,auth->tmp_oauth_token_secret);
            break;
        case 2:
            cgi_init(tmp_data);
            break;
        default:
            break;
        }
        fclose(fp);
        return 0;
    }
    else
    {
        fclose(fp);
        return -1;
    }


}
#endif


queue_t queue_create ()
{
    queue_t q;
    q = malloc (sizeof (struct queue_struct));
    if (q == NULL)
        exit (-1);

    q->head = q->tail = NULL;
    return q;
}
void queue_enqueue (queue_entry_t d, queue_t q)
{
    d->next_ptr = NULL;
    if (q->tail)
    {
        q->tail->next_ptr = d;
        q->tail = d;
    }
    else
    {
        q->head = q->tail = d;
    }
}

queue_entry_t  queue_dequeue_t (queue_t q)
{
    queue_entry_t first = q->head;

    if (first)
    {
        if(first->re_cmd == NULL)
        {
            q->head = first->next_ptr;
            if (q->head == NULL)
            {
                q->tail = NULL;
            }
            first->next_ptr = NULL;
        }
        else
        {
            if(access(first->re_cmd,0) == 0)
            {
                q->head = first->next_ptr;
                if (q->head == NULL)
                {
                    q->tail = NULL;
                }
                first->next_ptr = NULL;
            }
            else
            {
                if(first->is_first == 0)
                {
                    first->is_first++;
                    return NULL;
                }
                else
                {
                    q->head = first->next_ptr;
                    if (q->head == NULL)
                    {
                        q->tail = NULL;
                    }
                    first->next_ptr = NULL;
                }
            }
        }

    }
    return first;
}
queue_entry_t  queue_dequeue (queue_t q)
{
    queue_entry_t first = q->head;

    if (first)
    {
        q->head = first->next_ptr;
        if (q->head == NULL)
        {
            q->tail = NULL;
        }
        first->next_ptr = NULL;
    }
    return first;
}
void queue_destroy (queue_t q)
{
    if (q != NULL)
    {
        while (q->head != NULL)
        {
            queue_entry_t next = q->head;
            q->head = next->next_ptr;
            next->next_ptr = NULL;

#if MEM_POOL_ENABLE
            mem_free(next->cmd_name);
            mem_free(next->re_cmd);
            mem_free (next);
#else
            if(next->re_cmd)
                free(next->re_cmd);
            free(next->cmd_name);
            free (next);
#endif
        }
        q->head = q->tail = NULL;
        free (q);
    }
}
action_item *get_action_item_access(const char *action,const char *path,action_item *head,int index){

    action_item *p;
    p = head->next;

    while(p != NULL)
    {
        if(asus_cfg.prule[index]->rule == 1)
        {

        }
        else
        {
            if(!strcmp(p->action,action) && !strncmp(p->path,path,strlen(path)))
            {
                return p;
            }
        }
        p = p->next;
    }
    wd_DEBUG("can not find action item_1\n");
    return NULL;
}
action_item *check_action_item(const char *action,const char *oldpath,action_item *head,int index,const char *newpath)
{
    action_item *p;
    p = head->next;

    while(p != NULL)
    {
        char *pTemp_t=(char *)malloc(sizeof(char)*(strlen(p->path)+1+1));
        char *oldpath_t=(char *)malloc(sizeof(char)*(strlen(oldpath)+1+1));
        memset(pTemp_t,'\0',strlen(p->path)+1+1);
        memset(oldpath_t,'\0',strlen(oldpath)+1+1);
        sprintf(pTemp_t,"%s/",p->path);
        sprintf(oldpath_t,"%s/",oldpath);
        char *p_t = NULL;

        if(!strcmp(p->action,action) && (p_t=strstr(pTemp_t,oldpath_t)) != NULL)
        {
            if(strlen(oldpath_t)<strlen(pTemp_t))
            {
                p_t+=strlen(oldpath);
                char *p_tt=(char *)malloc(sizeof(char)*(strlen(p_t)+1));
                memset(p_tt,'\0',strlen(p_t)+1);
                sprintf(p_tt,"%s",p_t);
                free(p->path);
                p->path = (char *)malloc(sizeof(char) * (strlen(newpath)+strlen(p_tt) + 1));
                memset(p->path,'\0',strlen(newpath)+strlen(p_tt) + 1);
                sprintf(p->path,"%s%s",newpath,p_tt);
                free(p_tt);
            }
            else
            {
                free(p->path);
                p->path = (char *)malloc(sizeof(char) * (strlen(newpath) + 1));
                sprintf(p->path,"%s",newpath);
            }
        }

        free(pTemp_t);
        free(oldpath_t);
        p = p->next;
    }
    return NULL;
}

int del_action_item(const char *action,const char *path,action_item *head){

    action_item *p1,*p2;
    p1 = head->next;
    p2 = head;

    while(p1 != NULL)
    {
        if( !strcmp(p1->action,action) && !strcmp(p1->path,path))
        {
            p2->next = p1->next;
            free(p1->action);
            free(p1->path);
            free(p1);
            return 0;
        }
        p2 = p1;
        p1 = p1->next;
    }
    wd_DEBUG("can not find action item_3\n");
    return 1;
}

void del_download_only_action_item(const char *action,const char *path,action_item *head)
{
    if(head == NULL)
    {
        return;
    }
    action_item *p1, *p2;
    char *cmp_name;
    char *p1_cmp_name;
    p1 = head->next;
    p2 = head;

    cmp_name = my_str_malloc((size_t)(strlen(path)+2));
    sprintf(cmp_name,"%s/",path);    //add for delete folder and subfolder in download only socket list

    while(p1 != NULL)
    {
        p1_cmp_name = my_str_malloc((size_t)(strlen(p1->path)+2));
        sprintf(p1_cmp_name,"%s/",p1->path);      //add for delete folder and subfolder in download only socket list
        if(strstr(p1_cmp_name,cmp_name) != NULL)
        {
            p2->next = p1->next;
            free(p1->action);
            free(p1->path);
            free(p1);
            p1 = p2->next;
        }
        else
        {
            p2 = p1;
            p1 = p1->next;
        }
        free(p1_cmp_name);
    }

    free(cmp_name);
}
void free_action_item(action_item *head){

    action_item *point;
    point = head->next;

    while(point != NULL)
    {
        head->next = point->next;
        free(point->action);
        free(point->path);
        free(point);
        point = head->next;
    }
    free(head);
}
action_item *create_action_item_head(){

    action_item *head;

    head = (action_item *)malloc(sizeof(action_item));
    if(head == NULL)
    {
        wd_DEBUG("create memory error!\n");
        exit(-1);
    }
    memset(head,'\0',sizeof(action_item));
    head->next = NULL;

    return head;
}
int add_all_download_only_socket_list(char *cmd,const char *dir,int index)
{
    struct dirent* ent = NULL;
    char *fullname;
    int fail_flag = 0;

    DIR *dp = opendir(dir);

    if(dp == NULL)
    {

        wd_DEBUG("opendir %s fail",dir);
        fail_flag = 1;
        return -1;
    }

    add_action_item(cmd,dir,g_pSyncList[index]->download_only_socket_head);

    while (NULL != (ent=readdir(dp)))
    {

        if(ent->d_name[0] == '.')
            continue;
        if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
            continue;

        fullname = my_str_malloc((size_t)(strlen(dir)+strlen(ent->d_name)+2));
        sprintf(fullname,"%s/%s",dir,ent->d_name);

        if( test_if_dir(fullname) == 1)
        {
            add_all_download_only_socket_list("createfolder",fullname,index);
        }
        else
        {
            add_action_item("createfile",fullname,g_pSyncList[index]->download_only_socket_head);
        }
        free(fullname);
    }

    closedir(dp);

    return (fail_flag == 1) ? -1 : 0;
}
int add_all_download_only_dragfolder_socket_list(const char *dir,int index)
{
    struct dirent* ent = NULL;
    char *fullname;
    int fail_flag = 0;

    DIR *dp = opendir(dir);

    if(dp == NULL)
    {

        wd_DEBUG("opendir %s fail",dir);
        fail_flag = 1;
        return -1;
    }

    while (NULL != (ent=readdir(dp)))
    {

        if(ent->d_name[0] == '.')
            continue;
        if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
            continue;

        fullname = my_str_malloc((size_t)(strlen(dir)+strlen(ent->d_name)+2));
        sprintf(fullname,"%s/%s",dir,ent->d_name);

        if( test_if_dir(fullname) == 1)
        {
            add_action_item("createfolder",fullname,g_pSyncList[index]->download_only_socket_head);
            add_all_download_only_dragfolder_socket_list(fullname,index);
        }
        else
        {
            add_action_item("createfile",fullname,g_pSyncList[index]->download_only_socket_head);
        }
        free(fullname);
    }

    closedir(dp);

    return (fail_flag == 1) ? -1 : 0;
}
int test_if_download_temp_file(char *filename)
{
    char file_suffix[9];
    char *temp_suffix = ".asus.td";
    memset(file_suffix,0,sizeof(file_suffix));
    char *p = filename;

    if(strstr(filename,temp_suffix))
    {
        strcpy(file_suffix,p+(strlen(filename)-strlen(temp_suffix)));
        if(!strcmp(file_suffix,temp_suffix))
            return 1;
    }

    return 0;

}
int do_unfinished(int index){
    if(exit_loop)
    {
        return 0;
    }

    wd_DEBUG("do_unfinished\n");

    action_item *p,*p1;
    p = g_pSyncList[index]->unfinished_list->next;
    int ret;

    while(p != NULL)
    {
        if(!strcmp(p->action,"download"))
        {
            CloudFile *filetmp = NULL;
            filetmp = get_CloudFile_node(g_pSyncList[index]->ServerRootNode,p->path,0x2);
            if(filetmp == NULL)   //if filetmp == NULL,it means server has deleted filetmp
            {

                wd_DEBUG("filetmp is NULL\n");

                p1 = p->next;
                del_action_item("download",p->path,g_pSyncList[index]->unfinished_list);
                p = p1;
                continue;
            }
            char *localpath;
            localpath = serverpath_to_localpath(filetmp->href,index);

            wd_DEBUG("localpath = %s\n",localpath);

            if(is_local_space_enough(filetmp,index))
            {
                add_action_item("createfile",localpath,g_pSyncList[index]->server_action_list);
                DEBUG("api_download7\n");
                ret = s_download(filetmp, localpath,filetmp->href,index,6);
                if (ret == 0)
                {
                    ChangeFile_modtime(localpath,filetmp->mtime);
                    p1 = p->next;
                    del_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
                    p = p1;
                }
                else
                {

                    wd_DEBUG("download %s failed",filetmp->href);

                    p = p->next;
                }
            }
            else
            {
                write_log(S_ERROR,"local space is not enough!","",index);
                p = p->next;
            }
            free(localpath);
        }
        else
        {
            p = p->next;
        }
    }

    p = g_pSyncList[index]->up_space_not_enough_list->next;
    while(p != NULL)
    {
        p1 = p->next;

        wd_DEBUG("up_space_not_enough_list\n");

        char *serverpath;
        serverpath = localpath_to_serverpath(p->path,index);
        char *path_temp;
        path_temp = my_str_malloc(strlen(p->path)+1);
        sprintf(path_temp,"%s",p->path);
        DEBUG("upload14\n");
        ret = s_upload_file(p->path,serverpath,1,index,6);

        wd_DEBUG("########### uploadret = %d\n",ret);

        if(ret == 0)
        {
            cJSON *json = dofile(Con(TMP_R,upload_chunk_commit.txt));
            time_t mtime=cJSON_printf_mtime(json);
            cJSON_Delete(json);
            ChangeFile_modtime(p->path,mtime);
            del_action_item("upload",p->path,g_pSyncList[index]->unfinished_list);
            del_action_item("upload",p->path,g_pSyncList[index]->up_space_not_enough_list);
            p = p1;
        }
        else if(ret == LOCAL_FILE_LOST)
        {
            del_action_item("upload",p->path,g_pSyncList[index]->up_space_not_enough_list);
            p = p1;
        }
        else
        {

            wd_DEBUG("upload %s failed",p->path);

            p = p->next;
        }
        free(serverpath);
        free(path_temp);
    }
    wd_DEBUG("do_unfinished ok\n");
    return 0;
}

/*SIG*/
#if TOKENFILE
int write_notify_file(char *path,int signal_num)
{
    DEBUG("write_notify start!\n");
    FILE *fp;
    char fullname[64];
    memset(fullname,0,sizeof(fullname));
    my_mkdir("/tmp/notify");
    my_mkdir("/tmp/notify/usb");
    sprintf(fullname,"%s/google_client",path);
    fp = fopen(fullname,"w");
    if(NULL == fp)
    {
        wd_DEBUG("open notify %s file fail\n",fullname);
        return -1;
    }
    fprintf(fp,"%d",signal_num);
    DEBUG("fullname=%s\n",fullname);
    fclose(fp);
    return 0;
}
int get_tokenfile_info()
{
    wd_DEBUG("2222\n");
    int i;
    int j = 0;
    struct mounts_info_tag *info = NULL;
    char *tokenfile;
    FILE *fp;
    char buffer[1024];
    char *p;

    tokenfile_info = tokenfile_info_start;
    info = (struct mounts_info_tag *)malloc(sizeof(struct mounts_info_tag));
    if(info == NULL)
    {
        wd_DEBUG("obtain memory space fail\n");
        return -1;
    }
    memset(info,0,sizeof(struct mounts_info_tag));
    memset(buffer,0,1024);

    if(get_mounts_info(info) == -1)
    {
        wd_DEBUG("get mounts info fail\n");
        return -1;
    }
    for(i=0;i<info->num;i++)
    {
        tokenfile = my_str_malloc(strlen(info->paths[i])+20+1);
        sprintf(tokenfile,"%s/.google_tokenfile",info->paths[i]);
        printf("tokenfile = %s\n",tokenfile);
        if(!access(tokenfile,F_OK))
        {
            wd_DEBUG("tokenfile is exist!\n");
            if((fp = fopen(tokenfile,"r"))==NULL)
            {
                DEBUG("read tokenfile error\n");
                fprintf(stderr,"read tokenfile error\n");
                exit(-1);
            }
            DEBUG("before while\n");
            while(fgets(buffer,1024,fp)!=NULL)
            {
                DEBUG("fgets!=NULL\n");
                if( buffer[ strlen(buffer)-1 ] == '\n' )
                    buffer[ strlen(buffer)-1 ] = '\0';
                p = buffer;
                if(j == 0)
                {
                    if(initial_tokenfile_info_data(&tokenfile_info_tmp) == NULL)
                    {
                        fclose(fp);
                        return -1;
                    }
                    tokenfile_info_tmp->url = my_str_malloc(strlen(p)+1);
                    sprintf(tokenfile_info_tmp->url,"%s",p);
                    tokenfile_info_tmp->mountpath = my_str_malloc(strlen(info->paths[i])+1);
                    sprintf(tokenfile_info_tmp->mountpath,"%s",info->paths[i]);
                    DEBUG("***tokenfile_info_tmp->url=%s\n***tokenfile_info_tmp->mountpath=%s\n",tokenfile_info_tmp->url,tokenfile_info_tmp->mountpath);
                    j++;
                }
                else
                {
                    tokenfile_info_tmp->folder = my_str_malloc(strlen(p)+1);
                    sprintf(tokenfile_info_tmp->folder,"%s",p);
                    DEBUG("***tokenfile_info_tmp->folder=%s\n",tokenfile_info_tmp->folder);
                    tokenfile_info->next = tokenfile_info_tmp;
                    tokenfile_info = tokenfile_info_tmp;
                    j = 0;
                }
            }
            DEBUG("after while\n");
            fclose(fp);
        }
        DEBUG("get_tokenfile_info,return\n");
        free(tokenfile);
    }


    for(i=0;i<info->num;++i)
    {
        free(info->paths[i]);
    }
    if(info->paths != NULL)
        free(info->paths);
    free(info);

    return 0;
}
int get_mounts_info(struct mounts_info_tag *info)
{
    wd_DEBUG("get_mounts_info\n");
    int len = 0;
    FILE *fp;
    int i = 0;
    int num = 0;
    char **tmp_mount_list=NULL;
    char **tmp_mount=NULL;

    char buf[len+1];
    memset(buf,'\0',sizeof(buf));
    char a[1024];
    char *p,*q;
    fp = fopen("/proc/mounts","r");
    if(fp)
    {
        while(!feof(fp))
        {
            memset(a,'\0',sizeof(a));
            fscanf(fp,"%[^\n]%*c",a);
            if((strlen(a) != 0)&&((p=strstr(a,"/dev/sd")) != NULL))
            {
                if((q=strstr(p,"/tmp")) != NULL)
                {
                    if((p=strstr(q," ")) != NULL)
                    {

                        len = strlen(q) - strlen(p)+1;


                        tmp_mount = (char **)malloc(sizeof(char *)*(num+1));
                        if(tmp_mount == NULL)
                        {
                            fclose(fp);
                            return -1;
                        }

                        tmp_mount[num] = my_str_malloc(len+1);
                        if(tmp_mount[num] == NULL)
                        {
                            free(tmp_mount);
                            fclose(fp);
                            return -1;
                        }
                        snprintf(tmp_mount[num],len,"%s",q);

                        if(num != 0)
                        {
                            for(i = 0; i < num; ++i)
                                tmp_mount[i] = tmp_mount_list[i];

                            free(tmp_mount_list);
                            tmp_mount_list = tmp_mount;
                        }
                        else
                            tmp_mount_list = tmp_mount;

                        ++num;
                    }
                }
            }
        }
    }
    fclose(fp);

    info->paths = tmp_mount_list;
    info->num = num;
    return 0;
}
struct tokenfile_info_tag *initial_tokenfile_info_data(struct tokenfile_info_tag **token)
{
    DEBUG("initial_tokenfile_info_data start\n");
    struct tokenfile_info_tag *follow_token;

    if(token == NULL)
        return *token;

    *token = (struct tokenfile_info_tag *)malloc(sizeof(struct tokenfile_info_tag));
    if(*token == NULL)
        return NULL;

    follow_token = *token;

    follow_token->url = NULL;
    follow_token->folder = NULL;
    follow_token->mountpath = NULL;
    follow_token->next = NULL;

    return follow_token;
}
int check_config_path(int is_read_config)
{
    wd_DEBUG("check_config_path start%d\n", is_read_config);
    int i;
    int flag;
    char *nv;
    char *nvp;
    char *new_nv;
    int nv_len;
    int is_path_change = 0;

#ifdef NVRAM_
#ifndef USE_TCAPI
    DEBUG("before nvram_safe_get(gd_tokenfile)\n");
    nv = strdup(nvram_safe_get("gd_tokenfile"));
    DEBUG("nv=%s\n",nv);
#else
    char tmp[MAXLEN_TCAPI_MSG] = {0};
    tcapi_get(AICLOUD, "gd_tokenfile", tmp);
    nv = my_str_malloc(strlen(tmp)+1);
    sprintf(nv,"%s",tmp);
#endif
#else
    FILE *fp;
    fp = fopen(TOKENFILE_RECORD,"r");
    if(fp==NULL)
    {
        nv = my_str_malloc(2);
        sprintf(nv,"");
    }
    else
    {
        fseek( fp , 0 , SEEK_END );
        int file_size;
        file_size = ftell( fp );
        fseek(fp , 0 , SEEK_SET);
        nv = my_str_malloc(file_size+2);
        fscanf(fp,"%[^\n]%*c",nv);
        fclose(fp);
    }


#endif
    nv_len = strlen(nv);
    wd_DEBUG("nv_len = %d\n",nv_len);
    wd_DEBUG("nv = %s\n",nv);

    for(i=0;i<asus_cfg.dir_number;i++)
    {
        flag = 0;
        /*tokenfile exist*/
        tokenfile_info_tmp = tokenfile_info_start->next;
        while(tokenfile_info_tmp != NULL)
        {
            if( !strcmp(tokenfile_info_tmp->url,asus_cfg.user) &&
                !strcmp(tokenfile_info_tmp->folder,asus_cfg.prule[i]->rooturl))
            {
                if(strcmp(tokenfile_info_tmp->mountpath,asus_cfg.prule[i]->base_path))
                {
                    memset(asus_cfg.prule[i]->base_path,0,sizeof(asus_cfg.prule[i]->base_path));
                    sprintf(asus_cfg.prule[i]->base_path,"%s",tokenfile_info_tmp->mountpath);
                    memset(asus_cfg.prule[i]->path,0,sizeof(asus_cfg.prule[i]->path));
                    sprintf(asus_cfg.prule[i]->path,"%s%s",tokenfile_info_tmp->mountpath,tokenfile_info_tmp->folder);
                    asus_cfg.prule[i]->base_path_len = strlen(asus_cfg.prule[i]->base_path);
                    is_path_change = 1;
                }
                if(!is_read_config)
                {
                    if(g_pSyncList[i]->sync_disk_exist == 0)
                        is_path_change = 1;   //plug the disk and the mout_path not change
                }
                flag = 1;
                break;
            }
            tokenfile_info_tmp = tokenfile_info_tmp->next;
        }
        wd_DEBUG("asus_cfg.prule[%d]->path = %s\n",i,asus_cfg.prule[i]->path);
        if(!flag)
        {
            nvp = my_str_malloc(strlen(asus_cfg.user)+strlen(asus_cfg.prule[i]->rooturl)+2);
            sprintf(nvp,"%s>%s",asus_cfg.user,asus_cfg.prule[i]->rooturl);
            wd_DEBUG("nvp = %s\n",nvp);

            if(!is_read_config)
            {
                if(g_pSyncList[i]->sync_disk_exist == 1)
                    is_path_change = 2;   //remove the disk and the mout_path not change
            }

            /* "name>folder" is not in tokenfile_record */
            if(strstr(nv,nvp) == NULL)
            {

                if(initial_tokenfile_info_data(&tokenfile_info_tmp) == NULL)
                {
                    return -1;
                }
                tokenfile_info_tmp->url = my_str_malloc(strlen(asus_cfg.user)+1);
                sprintf(tokenfile_info_tmp->url,"%s",asus_cfg.user);

                tokenfile_info_tmp->mountpath = my_str_malloc(strlen(asus_cfg.prule[i]->base_path)+1);
                sprintf(tokenfile_info_tmp->mountpath,"%s",asus_cfg.prule[i]->base_path);

                tokenfile_info_tmp->folder = my_str_malloc(strlen(asus_cfg.prule[i]->rooturl)+1);
                sprintf(tokenfile_info_tmp->folder,"%s",asus_cfg.prule[i]->rooturl);

                tokenfile_info->next = tokenfile_info_tmp;
                tokenfile_info = tokenfile_info_tmp;
                DEBUG("@@@write_to_tokenfile1\n");
                write_to_tokenfile(asus_cfg.prule[i]->base_path);

                if(nv_len)
                {
                    new_nv = my_str_malloc(strlen(nv)+strlen(nvp)+2);
                    sprintf(new_nv,"%s<%s",nv,nvp);

                }
                else
                {
                    new_nv = my_str_malloc(strlen(nvp)+1);
                    sprintf(new_nv,"%s",nvp);
                }
                wd_DEBUG("new_nv = %s\n",new_nv);

#ifdef NVRAM_
                DEBUG("write_to_nvram('', 'gd_tokenfile')3\n");
                write_to_nvram(new_nv,"gd_tokenfile");
#else
                write_to_wd_tokenfile(new_nv);
#endif

                free(new_nv);
            }
            free(nvp);
        }
    }
    free(nv);
    return is_path_change;
}
int write_to_tokenfile(char *mpath)
{
    wd_DEBUG("write_to_tokenfile start\n");
    FILE *fp;

    char *filename;
    filename = my_str_malloc(strlen(mpath)+20+1);
    sprintf(filename,"%s/.google_tokenfile",mpath);
    wd_DEBUG("filename = %s\n",filename);

    int i = 0;
    if( ( fp = fopen(filename,"w") ) == NULL )
    {
        wd_DEBUG("open tokenfile failed!\n");
        return -1;
    }

    tokenfile_info_tmp = tokenfile_info_start->next;
    while(tokenfile_info_tmp != NULL)
    {
        wd_DEBUG("tokenfile_info_tmp->mountpath = %s\n",tokenfile_info_tmp->mountpath);
        if(!strcmp(tokenfile_info_tmp->mountpath,mpath))
        {
            wd_DEBUG("tokenfile_info_tmp->url = %s\n",tokenfile_info_tmp->url);
            if(i == 0)
            {
                fprintf(fp,"%s\n%s",tokenfile_info_tmp->url,tokenfile_info_tmp->folder);
                i=1;
            }
            else
            {
                fprintf(fp,"\n%s\n%s",tokenfile_info_tmp->url,tokenfile_info_tmp->folder);
            }
        }

        tokenfile_info_tmp = tokenfile_info_tmp->next;
    }

    fclose(fp);
    if(!i)
        remove(filename);
    free(filename);

    wd_DEBUG("write_to_tokenfile end\n");
    return 0;
}
#ifdef NVRAM_
int write_to_nvram(char *contents,char *nv_name)
{
    char *command;
    command = my_str_malloc(strlen(contents)+strlen(SHELL_FILE)+strlen(nv_name)+8);
    sprintf(command,"sh %s \"%s\" %s",SHELL_FILE,contents,nv_name);

    wd_DEBUG("command : [%s]\n",command);

    system(command);
    free(command);

    return 0;
}
#else
int write_to_wd_tokenfile(char *contents)
{
    if(contents == NULL || contents == "")
    {
        unlink(TOKENFILE_RECORD);
        return 0;
    }
    FILE *fp;
    if( ( fp = fopen(TOKENFILE_RECORD,"w") ) == NULL )
    {
        wd_DEBUG("open wd_tokenfile failed!\n");
        return -1;
    }
    fprintf(fp,"%s",contents);
    fclose(fp);
    return 0;
}
#endif
#ifdef NVRAM_
int convert_nvram_to_file_mutidir(char *file,struct asus_config *config)
{

    wd_DEBUG("enter convert_nvram_to_file_mutidir function\n");

    FILE *fp;
    char *nv, *nvp, *b;
    int i;
    char *p;
    char *buffer;
    char *buf;

    fp=fopen(file, "w");

    if (fp==NULL) return -1;
#ifndef USE_TCAPI
    nv = nvp = strdup(nvram_safe_get("cloud_sync"));
#else
    char tmp[MAXLEN_TCAPI_MSG] = {0};
    tcapi_get(AICLOUD, "cloud_sync", tmp);
    nvp = nv = my_str_malloc(strlen(tmp)+1);
    sprintf(nv,"%s",tmp);
#endif


    if(nv)
    {
        while ((b = strsep(&nvp, "<")) != NULL)
        {
            i = 0;
            buf = buffer = strdup(b);

            wd_DEBUG("buffer = %s\n",buffer);

            while((p = strsep(&buffer,">")) != NULL)
            {

                wd_DEBUG("p = %s\n",p);

                if (*p == 0)
                {
                    fprintf(fp,"\n");
                    i++;
                    continue;
                }
                if(i == 0)
                {
                    if(atoi(p) != 6)
                        break;
                    fprintf(fp,"%s",p);
                }
                else
                {
                    fprintf(fp,"\n%s",p);
                }

                if(i == 4)
                    config->dir_number = atoi(p);
                i++;
            }
            free(buf);
        }
        free(nv);
    }

    else
        wd_DEBUG("get nvram fail\n");

    fclose(fp);

    wd_DEBUG("end convert_nvram_to_file_mutidir function\n");

    return 0;
}
#else
int create_webdav_conf_file(struct asus_config *config){

    wd_DEBUG("enter create_google_conf_file function\n");

    FILE *fp;
    char *nv, *nvp, *b;
    int i;
    char *p;
    char *buffer;
    char *buf;

    fp = fopen(TMPCONFIG,"r");
    if (fp==NULL)
    {
        nvp = my_str_malloc(2);
        sprintf(nvp,"");
    }
    else
    {
        fseek( fp , 0 , SEEK_END );
        int file_size;
        file_size = ftell( fp );
        fseek(fp , 0 , SEEK_SET);
        nvp =  (char *)malloc( file_size * sizeof( char ) );
        fread(nvp , file_size , sizeof(char) , fp);
        fclose(fp);
    }

    nv = nvp;

    replace_char_in_str(nvp,'\0','\n');

    wd_DEBUG("**********nv = %s\n",nv);


    fp=fopen(CONFIG_PATH, "w");

    if (fp==NULL) return -1;

    config->dir_number = 0;
    if(nv)
    {
        while ((b = strsep(&nvp, "<")) != NULL)
        {
            i = 0;
            buf = buffer = strdup(b);

            wd_DEBUG("buffer = %s\n",buffer);

            while((p = strsep(&buffer,">")) != NULL)
            {

                wd_DEBUG("p = %s\n",p);

                if (*p == 0)
                {
                    fprintf(fp,"\n");
                    i++;
                    continue;
                }
                if(i == 0)
                {
                    if(atoi(p) != 6)
                        break;
                    fprintf(fp,"%s",p);
                }
                else
                {
                    fprintf(fp,"\n%s",p);
                }

                if(i == 4)
                    config->dir_number = atoi(p);
                i++;
            }
            free(buf);

        }

        free(nv);

    }
    else
        wd_DEBUG("get nvram fail\n");

    fclose(fp);
    return 0;

}
#endif
int rewrite_tokenfile_and_nv(){

    int i,j;
    int exist;
    wd_DEBUG("rewrite_tokenfile_and_nv start\n");
    if(asus_cfg.dir_number > asus_cfg_stop.dir_number)
    {
        for(i=0;i<asus_cfg.dir_number;i++)
        {
            exist = 0;
            for(j=0;j<asus_cfg_stop.dir_number;j++)
            {
                if(!strcmp(asus_cfg_stop.prule[j]->rooturl,asus_cfg.prule[i]->rooturl))
                {
                    exist = 1;
                    break;
                }
            }
            if(!exist)
            {
                wd_DEBUG("del form nv\n");
                char *new_nv;
                delete_tokenfile_info(asus_cfg.user,asus_cfg.prule[i]->rooturl);
                new_nv = delete_nvram_contents(asus_cfg.user,asus_cfg.prule[i]->rooturl);
                wd_DEBUG("new_nv = %s\n",new_nv);

                remove(g_pSyncList[i]->conflict_file);
                DEBUG("@@@write_to_tokenfile2\n");
                write_to_tokenfile(asus_cfg.prule[i]->base_path);

#ifdef NVRAM_
                DEBUG("write_to_nvram('', 'gd_tokenfile')4\n");
                write_to_nvram(new_nv,"gd_tokenfile");
#else
                write_to_wd_tokenfile(new_nv);
#endif

                free(new_nv);
            }
        }
    }
    else
    {
        for(i=0;i<asus_cfg_stop.dir_number;i++)
        {
            exist = 0;
            for(j=0;j<asus_cfg.dir_number;j++)
            {
                if(!strcmp(asus_cfg_stop.prule[i]->rooturl,asus_cfg.prule[j]->rooturl))
                {
                    exist = 1;
                    break;
                }
            }
            if(!exist)
            {
                char *new_nv;
                add_tokenfile_info(asus_cfg_stop.user,asus_cfg_stop.prule[i]->rooturl,asus_cfg_stop.prule[i]->base_path);
                new_nv = add_nvram_contents(asus_cfg_stop.user,asus_cfg_stop.prule[i]->rooturl);
                DEBUG("@@@write_to_tokenfile3\n");
                write_to_tokenfile(asus_cfg_stop.prule[i]->base_path);

#ifdef NVRAM_
                DEBUG("write_to_nvram('', 'gd_tokenfile')5\n");
                write_to_nvram(new_nv,"gd_tokenfile");
#else
                write_to_wd_tokenfile(new_nv);
#endif
                free(new_nv);
            }
        }
    }
    return 0;
}
#ifndef NVRAM_
int write_get_nvram_script(){

    FILE *fp;
    char contents[512];
    memset(contents,0,512);
    strcpy(contents,"#! /bin/sh\nNV=`nvram get cloud_sync`\nif [ ! -f \"/tmp/smartsync/google/config/google_tmpconfig\" ]; then\n   touch /tmp/smartsync/google/config/google_tmpconfig\nfi\necho \"$NV\" >/tmp/smartsync/google/config/google_tmpconfig");

    if(( fp = fopen(Google_Get_Nvram,"w"))==NULL)
    {
        fprintf(stderr,"create google_get_nvram file error\n");
        return -1;
    }

    fprintf(fp,"%s",contents);
    fclose(fp);

    return 0;
}
int write_get_nvram_script_va(char *str)
{
    FILE *fp;
    char contents[512];
    memset(contents,0,512);
    sprintf(contents,"#! /bin/sh\nNV=`nvram get %s`\nif [ ! -f \"%s\" ]; then\n   touch %s\nfi\necho \"$NV\" >%s",str,TMP_NVRAM_VL,TMP_NVRAM_VL,TMP_NVRAM_VL);

    if(( fp = fopen(Google_Get_Nvram_Link,"w"))==NULL)
    {
        fprintf(stderr,"create google_get_nvram file error\n");
        return -1;
    }

    fprintf(fp,"%s",contents);
    fclose(fp);

    return 0;
}
#endif
int delete_tokenfile_info(char *url,char *folder){

    struct tokenfile_info_tag *tmp;

    tmp = tokenfile_info_start;
    tokenfile_info_tmp = tokenfile_info_start->next;

    while(tokenfile_info_tmp != NULL)
    {
        if( !strcmp(tokenfile_info_tmp->url,url) &&
            !strcmp(tokenfile_info_tmp->folder,folder))
        {
            tmp->next = tokenfile_info_tmp->next;
            free(tokenfile_info_tmp->folder);
            free(tokenfile_info_tmp->url);
            free(tokenfile_info_tmp->mountpath);
            free(tokenfile_info_tmp);
            tokenfile_info_tmp = NULL;
            break;
        }
        tmp = tokenfile_info_tmp;
        tokenfile_info_tmp = tokenfile_info_tmp->next;
    }

    return 0;
}
char *delete_nvram_contents(char *url,char *folder){
    char *nv;
    char *nvp;
    char *p,*b;

    char *new_nv;
    int n;
    int i=0;



#ifdef NVRAM_
#ifndef USE_TCAPI
    p = nv = strdup(nvram_safe_get("gd_tokenfile"));
#else
    char tmp[MAXLEN_TCAPI_MSG] = {0};
    tcapi_get(AICLOUD, "gd_tokenfile", tmp);
    p = nv = my_str_malloc(strlen(tmp)+1);
    sprintf(nv,"%s",tmp);
#endif
#else
    FILE *fp;
    fp = fopen(TOKENFILE_RECORD,"r");
    if (fp==NULL)
    {
        nv = my_str_malloc(2);
        sprintf(nv,"");
    }
    else
    {
        fseek( fp , 0 , SEEK_END );
        int file_size;
        file_size = ftell( fp );
        fseek(fp , 0 , SEEK_SET);

        nv = my_str_malloc(file_size+2);

        fscanf(fp,"%[^\n]%*c",nv);
        p = nv;
        fclose(fp);
    }

#endif

    nvp = my_str_malloc(strlen(url)+strlen(folder)+2);
    sprintf(nvp,"%s>%s",url,folder);



    if(strstr(nv,nvp) == NULL)
    {
        free(nvp);
        return nv;
    }



    if(!strcmp(nv,nvp))
    {
        free(nvp);
        memset(nv,0,sizeof(nv));
        sprintf(nv,"");
        return nv;
    }


    if(nv)
    {
        while((b = strsep(&p, "<")) != NULL)
        {
            if(strcmp(b,nvp))
            {
                n = strlen(b);
                if(i == 0)
                {
                    new_nv = my_str_malloc(n+1);
                    sprintf(new_nv,"%s",b);
                    ++i;
                }
                else
                {
                    new_nv = realloc(new_nv, strlen(new_nv)+n+2);
                    sprintf(new_nv,"%s<%s",new_nv,b);
                }
            }
        }

        free(nv);
    }
    free(nvp);

    return new_nv;
}
int add_tokenfile_info(char *url,char *folder,char *mpath){

    if(initial_tokenfile_info_data(&tokenfile_info_tmp) == NULL)
    {
        return -1;
    }

    tokenfile_info_tmp->url = my_str_malloc(strlen(url)+1);
    sprintf(tokenfile_info_tmp->url,"%s",url);

    tokenfile_info_tmp->mountpath = my_str_malloc(strlen(mpath)+1);
    sprintf(tokenfile_info_tmp->mountpath,"%s",mpath);

    tokenfile_info_tmp->folder = my_str_malloc(strlen(folder)+1);
    sprintf(tokenfile_info_tmp->folder,"%s",folder);

    tokenfile_info->next = tokenfile_info_tmp;
    tokenfile_info = tokenfile_info_tmp;

    return 0;
}
char *add_nvram_contents(char *url,char *folder){

    char *nv;
    int nv_len;
    char *new_nv;
    char *nvp;

    nvp = my_str_malloc(strlen(url)+strlen(folder)+2);
    sprintf(nvp,"%s>%s",url,folder);

    wd_DEBUG("add_nvram_contents     nvp = %s\n",nvp);


#ifdef NVRAM_
#ifndef USE_TCAPI
    nv = strdup(nvram_safe_get("gd_tokenfile"));
#else
    char tmp[MAXLEN_TCAPI_MSG] = {0};
    tcapi_get(AICLOUD, "gd_tokenfile", tmp);
    nv = my_str_malloc(strlen(tmp)+1);
    sprintf(nv,"%s",tmp);
#endif
#else
    FILE *fp;
    fp = fopen(TOKENFILE_RECORD,"r");
    if (fp==NULL)
    {
        nv = my_str_malloc(2);
        sprintf(nv,"");
    }
    else
    {
        fseek( fp , 0 , SEEK_END );
        int file_size;
        file_size = ftell( fp );
        fseek(fp , 0 , SEEK_SET);
        wd_DEBUG("add_nvram_contents     file_size = %d\n",file_size);
        nv = my_str_malloc(file_size+2);
        fscanf(fp,"%[^\n]%*c",nv);
        fclose(fp);
    }
#endif

    wd_DEBUG("add_nvram_contents     nv = %s\n",nv);
    nv_len = strlen(nv);

    if(nv_len)
    {
        new_nv = my_str_malloc(strlen(nv)+strlen(nvp)+2);
        sprintf(new_nv,"%s<%s",nv,nvp);

    }
    else
    {
        new_nv = my_str_malloc(strlen(nvp)+1);
        sprintf(new_nv,"%s",nvp);
    }
    wd_DEBUG("add_nvram_contents     new_nv = %s\n",new_nv);
    free(nvp);
    free(nv);
    return new_nv;
}
int check_sync_disk_removed()
{

    wd_DEBUG("check_sync_disk_removed start! \n");


    int ret;

    free_tokenfile_info(tokenfile_info_start);

    if(get_tokenfile_info()==-1)
    {
        wd_DEBUG("\nget_tokenfile_info failed\n");
        exit(-1);
    }

    ret = check_config_path(0);
    return ret;

}
int check_disk_change()
{
    DEBUG("check_disk_change start!\n");
    int status = -1;
    disk_change = 0;
    status = check_sync_disk_removed();

    if(status == 2 || status == 1)
    {
        exit_loop = 1;

        sync_disk_removed = 1;
    }

    return 0;
}
int free_tokenfile_info(struct tokenfile_info_tag *head)
{
    struct tokenfile_info_tag *p = head->next;
    while(p != NULL)
    {
        head->next = p->next;
        if(p->folder != NULL)
        {
            free(p->folder);
        }
        if(p->mountpath != NULL)
        {
            free(p->mountpath);
        }
        if(p->url != NULL)
        {
            free(p->url);
        }
        free(p);
        p = head->next;
    }
    return 0;
}
int detect_client()
{
    unlink("/tmp/smartsync_app/google_client_start");
    char *dir = "/tmp/smartsync_app";
    int flag = 0;
    if(access(dir,0) != 0)
        flag = 0;
    else
    {
        struct dirent* ent = NULL;
        DIR *pDir;
        pDir=opendir(dir);
        int file_num = 0;
        if(pDir != NULL )
        {
            while (NULL != (ent=readdir(pDir)))
            {
                if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
                    continue;
                file_num ++ ;
            }
            closedir(pDir);

            if(file_num > 0)
                flag = 1;
            else
                flag = 0;

        }
        else
            flag = 0;
    }
    if(!flag)
    {
        system("killall -9 inotify &");
    }
}


int detect_process(char * process_name)
{
    FILE *ptr;
    char buff[512];
    char ps[128];
    sprintf(ps,"ps | grep -c %s",process_name);
    strcpy(buff,"ABNORMAL");
    if((ptr=popen(ps, "r")) != NULL)
    {
        while (fgets(buff, 512, ptr) != NULL)
        {
            if(atoi(buff)>2)
            {
                pclose(ptr);
                return 1;
            }
        }
    }
    if(strcmp(buff,"ABNORMAL")==0)  /*ps command error*/
    {
        return 0;
    }
    pclose(ptr);
    return 0;
}
#ifdef NVRAM_
int create_shell_file(){

    FILE *fp;
    char contents[1024];
    memset(contents,0,1024);
#ifndef USE_TCAPI
    strcpy(contents,"#! /bin/sh\nnvram set $2=\"$1\"\nnvram commit");
#else
    strcpy(contents,"#! /bin/sh\ntcapi set AiCloud_Entry $2 \"$1\"\ntcapi commit AiCloud\ntcapi save");
#endif

    if(( fp = fopen(SHELL_FILE,"w"))==NULL)
    {
        fprintf(stderr,"create shell file error");
        return -1;
    }

    fprintf(fp,"%s",contents);
    fclose(fp);
    return 0;

}
/*process running,return 1;else return 0*/

#endif
#endif
