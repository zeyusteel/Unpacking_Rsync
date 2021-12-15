#include "file_utils.h"
#include "file_comdef.h"

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
#include <limits.h>

int demo_file_md5_cmp(unsigned char disgest[16], unsigned char _disgest[16]) 
{
    char tmp[3] = {0};
    char buf[33] = {0};
    char _buf[33] = {0};

    if (!disgest || !_disgest) {
        return ERROR;
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

int demo_get_file_md5(const char *fileName, unsigned char digest[16]) 
{
    MD5_CTX ctx;
    int len = 0;
    unsigned char buf[BUF_SIZE] = {0};

    FILE *fd = fopen(fileName, "rb");
    if (!fd) {
        return ERROR;
    }

    MD5_Init(&ctx);
    while ((len = fread(buf, 1, sizeof(buf), fd)) > 0) {
        MD5_Update(&ctx, buf, len);
    }

    MD5_Final(digest, &ctx);
    fclose(fd);

    return SUCCESS;
} 

int demo_check_hash(const char *origFile, const char *destFile)
{
    int rc;
    unsigned char md[16] = {0};
    unsigned char mdnew[16] = {0};
    
    rc = demo_get_file_md5(origFile, md);
    if (rc != SUCCESS) {
        return rc;
    }

    rc = demo_get_file_md5(destFile, mdnew);
    if (rc != SUCCESS) {
        return rc;
    }

    return demo_file_md5_cmp(md, mdnew);
}

void demo_make_dir(const char *path, int mode)
{
	if ((strcmp(path,".") == 0) || (strcmp(path,"/")==0)) {
		return;
    }

	if (access(path, F_OK) == 0) {
		return;
    } else {
		char *dupPath = strdup(path);
		const char *dirName = dirname(dupPath);
		demo_make_dir(dirName, mode);
		free(dupPath);
	}

	if(mkdir(path, mode) < 0){
		fprintf(stderr, "mkdir error!\n");
        return;
	}
	return;
}
 
int demo_make_file(const char*fileName, int mode)
{
	if (access(fileName, F_OK) == 0) {
        fprintf(stderr, "file exit!\n");
		return ERROR;
    }

	if(creat(fileName,mode) < 0){
		if(errno == ENOENT){
			char *dupFileName=strdup(fileName);
			char *dir = dirname(dupFileName);
			demo_make_dir(dir, 0777);
			free(dupFileName);
			creat(fileName,mode);
		}
		return ERROR;
	}
	return SUCCESS;
}
 
int demo_is_file_null(const char *fileName)
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

int demo_file_data_cover(const char *fileName, const char *data, int len)
{
    int rc = SUCCESS;
    int fdTmp = 0;
    char *dupFileName = NULL;
    char *dupDir = NULL;
    char *base;
    char *dir;
    char tmpName[BUF_SIZE] = {0};
    char cwd[BUF_SIZE] = {0};

    if (access(fileName, F_OK) != 0) {
        rc = ERROR;
        goto out;
    }

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        rc = ERROR;
        goto out;
    }

    dupFileName = strdup(fileName);
    dupDir = strdup(fileName);
    base = basename(dupFileName);
    dir = dirname(dupDir);

    if (strcmp(base, ".") == 0) {
        rc = ERROR;
        goto out;
    }

    //切换到目标文件下再创建临时文件，保证rename不会因夸文件系统报错
    if (chdir(dir) != 0) { rc = ERROR; goto out; }

    memset(tmpName, 0, BUF_SIZE);
    snprintf(tmpName, BUF_SIZE,".%s_XXXXXX", base);

    if ((fdTmp = mkstemp(tmpName)) == -1) {
        fprintf(stderr, "make stemp error\n");
        rc = ERROR;
        goto out;
    }

    if (write(fdTmp, data, len) != len) {
        fprintf(stderr, "write error\n");
        rc = ERROR;
        goto out;
    }

    //避免输入相对路径导致文件无法删除
    if (unlink(base) != 0) {
        fprintf(stderr, "unlink error\n");
        rc = ERROR;
        goto out;
    }

    if (rename(tmpName, base) != 0) {
        fprintf(stderr, "rename error\n");
        perror("");
        rc = ERROR;
        goto out;
    }

out:
    if (dupFileName) { free(dupFileName); }
    if (dupDir) { free(dupDir); }
    if (access(tmpName, F_OK) == 0) { unlink(tmpName); }
    if (fdTmp) { close(fdTmp); }
    if (chdir(cwd) != 0) { rc = ERROR; }

    return rc;
}