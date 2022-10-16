#ifndef __SHARED_FUNC__
#define __SHARED_FUNC__

#define NMP_SUCCESS 1
#define NMP_FAIL	0

#define NMP_VER_LEN	10
#define NMP_SHA256_CHECKSUM_LEN 64

#define SAFE_FREE(x) if(x) {free(x); x=NULL;}
#define SAFE_JSON_OBJ_PUT(x)  if(x) {json_object_put(x); x=NULL;}



/*******************************************************************
* NAME: get_file_sha256_checksum
* DESCRIPTION: compare the feature name and return the version
* INPUT:  file: string. file path.
*  		 checksum_len: size_t. the length of checksum buffer
* OUTPUT:  checksim: string. The sha256 checksum of file.
* RETURN: NMP_SUCCESS or NMP_FAIL
* NOTE:
*******************************************************************/
int get_file_sha256_checksum(const char *file, char *checksum, const size_t checksum_len, const int file_enc);

/*******************************************************************
* NAME: read_file
* DESCRIPTION: Verify and read the file and return the content without signature.
*			   If need, decrypt the file contnet.
* INPUT:  file: string, path of the file.
*         check_sig: bool number. If 1, need to check the signature of the file.
*		  file_enc: bool number, If 1, need to decrypt the content of the file.
* OUTPUT:  None
* RETURN: The decrypted content of the file without signature.
* NOTE:
*******************************************************************/
char *read_file(const char *file, const int check_sig, const int file_enc);

/*******************************************************************
* NAME: encrypt_file
* DESCRIPTION: encrypted the file content and save it with the signature as another file.
* INPUT:  src_file: string, the path of the source file.
*         dst_file: string, the path of the destination file.
*         with_sig: bool number, if 1, the src file include signature data, on need to encrypted it. Just need to copy it to the destination file.
* OUTPUT:  None
* RETURN: NMP_SUCCESS or NMP_FAIL
* NOTE:
*******************************************************************/
int encrypt_file(const char *src_file, const char *dst_file, const int with_sig);

/*******************************************************************
* NAME: download_file
* DESCRIPTION: download the file from server.
* INPUT:  file_name: string, the file name on the server.
*         local_file_path: string.the local file path to store the download file.
* OUTPUT: None
* RETURN:  NMP_SUCCESS or NMP_FAIL
* NOTE:
*******************************************************************/
int download_file(const char *file_name, const char *local_file_path);

#endif
