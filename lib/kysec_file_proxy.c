#include "kysec_file_proxy.h"
#include "kysec_file_comdef.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

int kysec_file_md5_cmp(unsigned char disgest[16], unsigned char _disgest[16]) 
{
    char tmp[3] = {0};
    char buf[33] = {0};
    char _buf[33] = {0};

    if (!disgest || !_disgest) {
        return KYSEC_ERROR;
    }

    for (int i = 0; i < 16; ++i) {
        snprintf(tmp, 3, "%02X", disgest[i]);
        strcat(buf, tmp);
    }

    for (int i = 0; i < 16; ++i) {
        snprintf(tmp, 3, "%02X", _disgest[i]);
        strcat(_buf, tmp);
    }

    return strncmp(buf, _buf, 32);
}

int kysec_get_file_md5(const char *fileName, unsigned char digest[16]) 
{
    MD5_CTX ctx;
    int len = 0;
    unsigned char buf[BUF_SIZE] = {0};

    FILE *fd = fopen(fileName, "rb");
    if (!fd) {
        return KYSEC_ERROR;
    }

    MD5_Init(&ctx);
    while ((len = fread(buf, 1, sizeof(buf), fd)) > 0) {
        MD5_Update(&ctx, buf, len);
    }

    MD5_Final(digest, &ctx);
    fclose(fd);

    return KYSEC_SUCCESS;
} 

void kysec_make_dir(const char *path)
{
	if ((strcmp(path,".") == 0) || (strcmp(path,"/")==0)) {
		return;
    }

	if (access(path, F_OK) == 0) {
		return;
    } else {
		char *dupPath = strdup(path);
		const char *dirName = dirname(dupPath);
		kysec_make_dir(dirName);
		free(dupPath);
	}

	if(mkdir(path,0777) < 0){
		printf("mkdir error!\n");
        return;
	}
	return;
}
 
int kysec_make_file(const char*fileName,int mode)
{
	if (access(fileName, F_OK) == 0) {
        printf("file exit!\n");
		return KYSEC_ERROR;
    }

	if(creat(fileName,mode) < 0){
		if(errno == ENOENT){
			char *dupFileName=strdup(fileName);
			char *dir = dirname(dupFileName);
			kysec_make_dir(dir);
			free(dupFileName);
			creat(fileName,mode);
		}
		return KYSEC_ERROR;
	}
	return KYSEC_SUCCESS;
}
 
int kysec_is_file_null(const char *fileName)
{
	int val = 0;
	FILE *fp = fopen(fileName,"r");
	if(fp == NULL) {
		return false;	
    }
	char ch = fgetc(fp);
	if(ch == EOF)
		val = true;

	fclose(fp);
	return val;
}