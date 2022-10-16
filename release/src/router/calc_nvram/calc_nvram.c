#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defaults_nv.h"

int main(int argc, char *argv[]){

	int i = 0, nv_len = 0, injffs = 0;
	int nv_name_size = 0, nv_size = 0, nv_no_jffs_size = 0;
	struct nvram_tuple *t;
	FILE *fp = NULL;

	for (t = router_defaults; t->name; t++)
	{
		injffs = 0;
		nv_name_size += strlen(t->name)+1;

		nv_len = (strlen(t->value) > t->len)? strlen(t->value) : t->len;

		nv_size += nv_len;
#ifdef RTCONFIG_JFFS_NVRAM
		for(i=0; large_nvram_list[i]; i++){
			if(!strcmp(large_nvram_list[i], t->name)){
				injffs = 1;
				break;
			}
		}
#endif
		if(!injffs)
			nv_no_jffs_size += nv_len;
	}

	printf("nv_name_size = %d\n", nv_name_size);
	printf("nv_size = %d\n", nv_name_size + nv_size);
	printf("nv_no_jffs_size = %d\n", nv_name_size + nv_no_jffs_size);

	if ((fp = fopen("../nvram_size.txt", "w")) != NULL) {
		fprintf(fp, "nv_name_size=%d\n", nv_name_size);
		fprintf(fp, "nv_size=%d\n", nv_name_size + nv_size);
		fprintf(fp, "nv_without_jffs_size=%d\n", nv_name_size + nv_no_jffs_size);
		fclose(fp);
	}

	return 0;
}