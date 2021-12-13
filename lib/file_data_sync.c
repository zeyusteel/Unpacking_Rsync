#include "file_restore.h"
#include "file_utils.h"
#include "file_comdef.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <librsync.h>
#include <libgen.h>
#include <string.h>
#include <limits.h>

int demo_rs_sig_fp(FILE *fpDest, FILE *fpSig, int useBlake2) 
{
    rs_result res;

    if (!fpDest || !fpSig) {
        fprintf(stderr, "fp NULL\n");
        return ERROR;
    }

    if (useBlake2) {
        res = rs_sig_file(fpDest, fpSig, RS_DEFAULT_BLOCK_LEN, 0, RS_BLAKE2_SIG_MAGIC, NULL);
    } else {
        res = rs_sig_file(fpDest, fpSig, RS_DEFAULT_BLOCK_LEN, 0, RS_MD4_SIG_MAGIC, NULL);
    }

    if (res != RS_DONE) {
        fprintf(stderr, "%s\n", rs_strerror(res));
        return ERROR;
    }
    return SUCCESS;
}

int demo_rs_sig_file(const char *destFile, const char *sigFile, int useBlake2)
{
    FILE *fpDest = NULL, *fpSig = NULL;
    int rc = SUCCESS;

    fpDest = fopen(destFile, "rb");
    fpSig = fopen(sigFile, "wb");

    if (!fpDest || !fpSig) {
        fprintf(stderr, "open error\n");
        rc = ERROR;
        goto out;
    }

    rc = demo_rs_sig_fp(fpDest, fpSig, useBlake2);
    if (rc != SUCCESS) {
        goto out;
    }

out:
    if (fpDest) {
        fclose(fpDest);
        fpDest = NULL;
    }
    if (fpSig) {
        fclose(fpSig);
        fpSig = NULL; 
    }
    return rc;
}

int demo_rs_delta_fp(FILE *fpSig, FILE *fpDelta, FILE *fpOrig)
{
    int rc = SUCCESS;
    rs_result res;
    rs_signature_t *sumset = NULL;

    if (!fpSig || !fpDelta || !fpOrig) {
        fprintf(stderr, "fp null\n");
        rc = ERROR;
        goto out;
    }

    res = rs_loadsig_file(fpSig, &sumset, NULL);
    if (res != RS_DONE) {
        fprintf(stderr, "%s\n", rs_strerror(res));
        rc = ERROR;
        goto out;
    }

    res = rs_build_hash_table(sumset);
    if (res != RS_DONE) {
        fprintf(stderr, "%s\n", rs_strerror(res));
        rc = ERROR;
        goto out;
    }

    res = rs_delta_file(sumset, fpOrig, fpDelta, NULL);
    if (res != RS_DONE) {
        fprintf(stderr, "%s\n", rs_strerror(res));
        rc = ERROR;
        goto out;
    }

out:
    if (sumset) {
        rs_free_sumset(sumset);
    }
    return rc;
}

int demo_rs_delta_file(const char *sigFile, const char *deltaFile, const char *origFile) 
{
    int rc = SUCCESS;
    FILE *fpSig = NULL, *fpDelta = NULL, *fpOrig = NULL;

    fpSig = fopen(sigFile, "rb");
    fpDelta = fopen(deltaFile, "wb");
    fpOrig = fopen(origFile, "rb");

    if (!fpSig || !fpDelta || !fpOrig) {
        rc = ERROR;
        goto out;
    }

    rc = demo_rs_delta_fp(fpSig, fpDelta, fpOrig);
    if (rc != SUCCESS) {
        goto out;
    }

out:
    if (fpSig) { fclose(fpSig); }
    if (fpDelta) { fclose(fpDelta); }
    if (fpOrig) { fclose(fpOrig); }

    return rc;
}

int demo_rs_patch_fp(FILE *fpDest, FILE *fpDelta, FILE *fpOut)
{
    rs_result res;
    if (!fpDest || !fpOut || !fpDelta) {
        return ERROR;
    }

    res = rs_patch_file(fpDest, fpDelta, fpOut, NULL);
    if (res != RS_DONE) {
        fprintf(stderr, "%s\n", rs_strerror(res));
        return ERROR;
    }
    return SUCCESS;
}

int demo_rs_patch_file(const char *destFile, const char *deltaFile, const char *outFile)
{
    int rc = SUCCESS;
    FILE *fpDest = NULL, *fpOut = NULL, *fpDelta = NULL;

    fpDest = fopen(destFile, "rb");
    fpOut = fopen(outFile, "wb");
    fpDelta = fopen(deltaFile, "rb");

    if (!fpDest || !fpOut || !fpDelta) {
        rc = ERROR;
        goto out;
    }

    rc = demo_rs_patch_fp(fpDest, fpDelta, fpOut);
    if (rc != SUCCESS) {
        goto out;
    }

out:
    if (fpDest) { fclose(fpDest); }
    if (fpOut) { fclose(fpOut); }
    if (fpDelta) { fclose(fpDelta); }
    return rc;
}

int demo_file_data_copy(const char *origFile, const char *destFile)
{
    int rc = SUCCESS;
    FILE *fpOrig = NULL, *fpTmpDest = NULL; 
    int fdTmp = 0;
    int len;
    char buf[BUF_SIZE] = {0};
    char tmpDestFile[BUF_SIZE] = {0};
    char cwd[BUF_SIZE] = {0};
    int destExit = 0;

    char *dupBase = NULL;
    char *dupDir = NULL;
    char *dir;
    char *base;

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        rc = ERROR;
        goto out;
    }

    if (access(destFile, F_OK) == 0) {
        destExit = 1;
    }

    fpOrig = fopen(origFile, "rb");

    dupBase = strdup(destFile);
    dupDir = strdup(destFile);
    base = basename(dupBase);
    dir = dirname(dupDir);

    if (strcmp(base, ".") == 0) {
        rc = ERROR;
        goto out;
    }

    if (chdir(dir) != 0) {
        rc = ERROR;
        goto out;
    }

    snprintf(tmpDestFile, BUF_SIZE,".%s_XXXXXX", base);

    if ((fdTmp = mkstemp(tmpDestFile)) == -1) {
        fprintf(stderr, "make stemp error\n");
        rc = ERROR;
        goto out;
    } 

    fpTmpDest = fopen(tmpDestFile, "wb");
    if (!fpOrig || !fpTmpDest) {
        fprintf(stderr, "open error orig:%s tmpDest:%s\n",origFile, tmpDestFile);
        rc = ERROR;
        goto out;
    }

    while((len = fread(buf, sizeof(buf[0]), sizeof(buf), fpOrig)) > 0) {
        if (fwrite(buf, sizeof(buf[0]), len, fpTmpDest) != len) {
            fprintf(stderr, "write error\n");
            rc = ERROR;
            goto out;
        }
    }

    if (destExit) {
        if (unlink(base) != 0) {
            fprintf(stderr, "unlink error\n");
            rc = ERROR;
            goto out;
        }
    }

    if (rename(tmpDestFile, base) != 0) {
        fprintf(stderr, "rename error\n");
        rc = ERROR;
        goto out;
    }

out:
    if (dupBase) { free(dupBase); }
    if (dupDir) { free(dupDir); }
    if (fdTmp) { close(fdTmp); }
    if (fpOrig) { fclose(fpOrig); }
    if (fpTmpDest) { fclose(fpTmpDest); }
    if (access(tmpDestFile, F_OK) == 0) { unlink(tmpDestFile); }
    if (chdir(cwd) != 0) { rc = ERROR; }
    return rc;
}

int demo_file_data_sync(const char *origFile, const char *destFile)
{
    int rc = SUCCESS;
    char sigFile[BUF_SIZE] = {0};
    char deltaFile[BUF_SIZE] = {0};
    char tmpDestFile[BUF_SIZE] = {0};

    char *dupBase = NULL;
    char *dupDir = NULL;
    char *base;
    char *dir;

    char cwd[BUF_SIZE] = {0};
    int fdSig = 0, fdDelta = 0, fdTmpDest = 0;
    FILE *fpDest = NULL, *fpSig = NULL, *fpDelta = NULL, *fpOrig = NULL, *fpTmpDest = NULL;

    if (!origFile || !destFile) {
        rc = ERROR;
        goto out;
    }

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        rc = ERROR;
        goto out;
    }

    if (access(origFile, F_OK) != 0) {
        printf("no such orig file:%s\n", origFile);
        rc = ERROR;
        goto out;
    }

    if (access(destFile, F_OK) && demo_make_file(destFile, 0644) != SUCCESS) {
        printf("no such dest file:%s\n", destFile);
        rc = ERROR;
        goto out;
    }

    dupBase = strdup(destFile);
    dupDir = strdup(destFile);
    base = basename(dupBase);
    dir = dirname(dupDir);

    snprintf(sigFile, BUF_SIZE,".%s_sigFile_XXXXXX", base);
    snprintf(deltaFile, BUF_SIZE,".%s_deltaFile_XXXXXX", base);
    snprintf(tmpDestFile, BUF_SIZE,".%s_tmpDestFile_XXXXXX", base);

    fpDest = fopen(destFile, "rb");
    fpOrig = fopen(origFile, "rb");

    if (chdir(dir) != 0) { rc = ERROR; goto out; }
 
    if ((fdSig = mkstemp(sigFile)) == -1) { rc = ERROR; goto out; }
    if ((fdDelta = mkstemp(deltaFile)) == -1) { rc = ERROR; goto out; }
    if ((fdTmpDest = mkstemp(tmpDestFile)) == -1) { rc = ERROR; goto out; }

    fpSig = fopen(sigFile, "wb+");

    if ((rc = demo_rs_sig_fp(fpDest, fpSig, 0)) != SUCCESS) {
        rc = ERROR;
        goto out;
    }

    fpDelta = fopen(deltaFile, "wb+");
    fseek(fpSig, 0, SEEK_SET);
    if ((rc = demo_rs_delta_fp(fpSig, fpDelta, fpOrig)) != SUCCESS) {
        rc = ERROR;
        goto out;
    }

    fpTmpDest = fopen(tmpDestFile,"wb");
    fseek(fpDest, 0, SEEK_SET);
    fseek(fpDelta, 0, SEEK_SET);
    if ((rc = demo_rs_patch_fp(fpDest, fpDelta, fpTmpDest)) != SUCCESS) {
        rc = ERROR;
        goto out;
    }

    if (!(unlink(base) == 0 && rename(tmpDestFile, base) == 0)) {
        perror("unlink error\n");
        rc = ERROR;
        goto out;
    }

out:
    if (dupBase) { free(dupBase); }
    if (dupDir) { free(dupDir); }

    if (fdSig) { close(fdSig); }
    if (fdDelta) { close(fdDelta); }
    if (fdTmpDest) { close(fdTmpDest); }

    if (fpDest) { fclose(fpDest); }
    if (fpDelta) { fclose(fpDelta); }
    if (fpOrig) { fclose(fpOrig); }
    if (fpTmpDest) { fclose(fpTmpDest); }
    if (fpSig) { fclose(fpSig); }

    if (access(sigFile, F_OK) == 0) { unlink(sigFile); }
    if (access(deltaFile, F_OK) == 0) { unlink(deltaFile); }
    if (access(tmpDestFile, F_OK) == 0) { unlink(tmpDestFile); }
    if (chdir(cwd) != 0) { rc = ERROR; }
    return rc;
}