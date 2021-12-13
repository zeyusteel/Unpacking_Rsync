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

int kysec_check_hash(const char *origFile, const char *destFile)
{
    int rc;
    unsigned char md[16] = {0};
    unsigned char mdnew[16] = {0};
    
    rc = kysec_get_file_md5(origFile, md);
    if (rc != KYSEC_SUCCESS) {
        return rc;
    }

    rc = kysec_get_file_md5(destFile, mdnew);
    if (rc != KYSEC_SUCCESS) {
        return rc;
    }

    return kysec_file_md5_cmp(md, mdnew);
}

void kysec_make_dir(const char *path, int mode)
{
	if ((strcmp(path,".") == 0) || (strcmp(path,"/")==0)) {
		return;
    }

	if (access(path, F_OK) == 0) {
		return;
    } else {
		char *dupPath = strdup(path);
		const char *dirName = dirname(dupPath);
		kysec_make_dir(dirName, mode);
		free(dupPath);
	}

	if(mkdir(path, mode) < 0){
		printf("mkdir error!\n");
        return;
	}
	return;
}
 
int kysec_make_file(const char*fileName, int mode)
{
	if (access(fileName, F_OK) == 0) {
        printf("file exit!\n");
		return KYSEC_ERROR;
    }

	if(creat(fileName,mode) < 0){
		if(errno == ENOENT){
			char *dupFileName=strdup(fileName);
			char *dir = dirname(dupFileName);
			kysec_make_dir(dir, 0777);
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

int kysec_file_data_cover(const char *fileName, const char *data, int len)
{
    int rc = KYSEC_SUCCESS;
    int fdTmp = 0;
    char *dupFileName = NULL;
    char *dupDir = NULL;
    char *baseName;
    char *dir;
    char tmpName[BUF_SIZE] = {0};
    char cwd[BUF_SIZE] = {0};
    char *absPath = NULL;


    if (access(fileName, F_OK) != 0) {
        rc = KYSEC_ERROR;
        goto out;
    }

    if (!(absPath = (char*)malloc(PATH_MAX))) {
        rc = KYSEC_ERROR;
        goto out;
    }
    memset(absPath, 0, PATH_MAX);

    if (!realpath(fileName, absPath)) {
        rc = KYSEC_ERROR;
        goto out;
    }

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        rc = KYSEC_ERROR;
        goto out;
    }

    dupFileName = strdup(fileName);
    dupDir = strdup(fileName);
    baseName = basename(dupFileName);
    dir = dirname(dupDir);

    if (strcmp(baseName, ".") == 0) {
        rc = KYSEC_ERROR;
        goto out;
    }

    //切换到目标文件下再创建临时文件，保证rename不会因夸文件系统报错
    if (chdir(dir) != 0) { rc = KYSEC_ERROR; goto out; }

    memset(tmpName, 0, BUF_SIZE);
    snprintf(tmpName, BUF_SIZE,".%s_XXXXXX", baseName);

    if ((fdTmp = mkstemp(tmpName)) == -1) {
        fprintf(stderr, "make stemp error\n");
        rc = KYSEC_ERROR;
        goto out;
    }

    if (write(fdTmp, data, len) != len) {
        fprintf(stderr, "write error\n");
        rc = KYSEC_ERROR;
        goto out;
    }

    //避免输入相对路径导致文件无法删除
    if (unlink(absPath) != 0) {
        fprintf(stderr, "unlink error\n");
        rc = KYSEC_ERROR;
        goto out;
    }

    if (rename(tmpName, absPath) != 0) {
        fprintf(stderr, "rename error\n");
        perror("");
        rc = KYSEC_ERROR;
        goto out;
    }

out:
    if (absPath) { free(absPath); }
    if (dupFileName) { free(dupFileName); }
    if (dupDir) { free(dupDir); }
    if (access(tmpName, F_OK) == 0) { unlink(tmpName); }
    if (fdTmp) { close(fdTmp); }
    if ((rc = chdir(cwd)) != 0) { rc = KYSEC_ERROR; }

    return rc;
}