#include "kysec_file_restore.h"
#include "kysec_file_proxy.h"
#include "kysec_file_comdef.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <librsync.h>

int kysec_rs_sig_file(const char *destFile, const char *sigFile, int useBlake2)
{
    FILE *fpDest = NULL, *fpSig = NULL;
    EKYSEC_CODE rc = KYSEC_SUCCESS;
    rs_result res; 

    fpDest = fopen(destFile, "rb");
    fpSig = fopen(sigFile, "wb");

    if (!fpDest || !fpSig) {
        rc = KYSEC_ERROR;
        goto out;
    }

    if (useBlake2) {
        res = rs_sig_file(fpDest, fpSig, RS_DEFAULT_BLOCK_LEN, 0, RS_BLAKE2_SIG_MAGIC, NULL);
    } else {
        res = rs_sig_file(fpDest, fpSig, RS_DEFAULT_BLOCK_LEN, 0, RS_MD4_SIG_MAGIC, NULL);
    }

    if (res != RS_DONE) {
        printf("%s\n", rs_strerror(res));
        rc = KYSEC_ERROR;
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

int kysec_rs_delta_file(const char *sigFile, const char *deltaFile, const char *origFile) 
{
    EKYSEC_CODE rc = KYSEC_SUCCESS;
    rs_result res;
    rs_signature_t *sumset = NULL;
    FILE *fpSig = NULL, *fpDelta = NULL, *fpOrig = NULL;

    fpSig = fopen(sigFile, "rb");
    fpDelta = fopen(deltaFile, "wb");
    fpOrig = fopen(origFile, "rb");

    if (!fpSig || !fpDelta || !fpOrig) {
        rc = KYSEC_ERROR;
        goto out;
    }

    res = rs_loadsig_file(fpSig, &sumset, NULL);
    if (res != RS_DONE) {
        printf("%s\n", rs_strerror(res));
        rc = KYSEC_ERROR;
        goto out;
    }

    res = rs_build_hash_table(sumset);
    if (res != RS_DONE) {
        printf("%s\n", rs_strerror(res));
        rc = KYSEC_ERROR;
        goto out;
    }

    res = rs_delta_file(sumset, fpOrig, fpDelta, NULL);
    if (res != RS_DONE) {
        printf("%s\n", rs_strerror(res));
        rc = KYSEC_ERROR;
        goto out;
    }

out:
    if (fpSig) {
        fclose(fpSig);
        fpSig = NULL;
    }
    if (fpDelta) {
        fclose(fpDelta);
        fpDelta = NULL;
    }
    if (fpOrig) {
        fclose(fpOrig);
        fpOrig = NULL;
    }
    if (sumset) {
        rs_free_sumset(sumset);
    }

    return rc;
}

int kysec_rs_patch_file(const char *destFile, const char *deltaFile, const char *outFile)
{
    EKYSEC_CODE rc = KYSEC_SUCCESS;
    rs_result res;
    FILE *fpDest = NULL, *fpOut = NULL, *fpDelta = NULL;

    fpDest = fopen(destFile, "rb");
    fpOut = fopen(outFile, "wb");
    fpDelta = fopen(deltaFile, "rb");

    if (!fpDest || !fpOut || !fpDelta) {
        rc = KYSEC_ERROR;
        goto out;
    }

    res = rs_patch_file(fpDest, fpDelta, fpOut, NULL);
    if (res != RS_DONE) {
        printf("%s\n", rs_strerror(res));
        rc = KYSEC_ERROR;
        goto out;
    }

out:
    if (fpDest) {
        fclose(fpDest);
        fpDest = NULL;
    }
    if (fpOut) {
        fclose(fpOut);
        fpOut = NULL;
    }
    if (fpDelta) {
        fclose(fpDelta);
        fpDelta = NULL;
    }
    return rc;
}

int kysec_file_copy(const char *origFile, const char *destFile)
{
    EKYSEC_CODE rc = KYSEC_SUCCESS;
    FILE *fpOrig = NULL, *fpTmpDest = NULL; 
    int fdTmp = 0;
    int len;
    char buf[BUF_SIZE] = {0};
    char tmpDestFile[]  = "tmpFile_XXXXXX";

    if ((fdTmp = mkstemp(tmpDestFile)) == -1) {
        rc = KYSEC_ERROR;
        goto out;
    }
    
    fpOrig = fopen(origFile, "rb");
    fpTmpDest = fopen(tmpDestFile, "wb");
    if (!fpOrig || !fpTmpDest) {
        rc = KYSEC_ERROR;
        goto out;
    }
    
    while((len = fread(buf, sizeof(buf[0]), sizeof(buf), fpOrig)) > 0) {
        if (fwrite(buf, sizeof(buf[0]), len, fpTmpDest) != len) {
            rc = KYSEC_ERROR;
            goto out;
        }
    }

    if (!(unlink(destFile) == 0 && rename(tmpDestFile, destFile) == 0)) {
        rc = KYSEC_ERROR;
        goto out;
    }

out:
    if (fdTmp) {
        close(fdTmp);
    }
    if (fpOrig) {
        fclose(fpOrig);
    }
    if (fpTmpDest) {
        fclose(fpTmpDest);
    }

    if (access(tmpDestFile, F_OK) == 0) { unlink(tmpDestFile); }
    return rc;
}

int kysec_file_sync(const char *origFile, const char *destFile)
{
    EKYSEC_CODE rc = KYSEC_SUCCESS;
    char sigFile[] = "sigFile_XXXXXX";
    char deltaFile[] = "deltaFile_XXXXXX";
    char tmpDestFile[] = "tmpDestFile_XXXXXX";

    int fdSig = -1, fdDelta = -1, fdTmpDest = -1;

    if (!origFile || !destFile) {
        rc = KYSEC_ERROR;
        goto out;
    }

    if (access(origFile, F_OK) != 0) {
        printf("no such orig file:%s\n", origFile);
        rc = KYSEC_ERROR;
        goto out;
    }

    if (access(destFile, F_OK) && kysec_make_file(destFile, 0644) != KYSEC_SUCCESS) {
        printf("no such dest file:%s\n", destFile);
        rc = KYSEC_ERROR;
        goto out;
    }

    if ((fdSig = mkstemp(sigFile)) == -1) {
        rc = KYSEC_ERROR;
        goto out;
    }

    if ((rc = kysec_rs_sig_file(destFile, sigFile, 0)) != KYSEC_SUCCESS) {
        goto out;
    }

    if ((fdDelta = mkstemp(deltaFile)) == -1) {
        rc = KYSEC_ERROR;
        goto out;
    }

    if ((rc = kysec_rs_delta_file(sigFile, deltaFile, origFile)) != KYSEC_SUCCESS) {
        goto out;
    }

    if ((fdTmpDest = mkstemp(tmpDestFile)) == -1) {
        rc = KYSEC_ERROR;
        goto out;
    }

    if ((rc = kysec_rs_patch_file(destFile, deltaFile, tmpDestFile)) != KYSEC_SUCCESS) {
        rc = KYSEC_ERROR;
        goto out;
    }

    if (!(unlink(destFile) == 0 && rename(tmpDestFile, destFile) == 0)) {
        rc = KYSEC_ERROR;
        goto out;
    }

out:
    if (fdSig) { close(fdSig); }
    if (fdDelta) { close(fdDelta); }
    if (fdTmpDest) { close(fdTmpDest); }

    if (access(sigFile, F_OK) == 0) { unlink(sigFile); }
    if (access(deltaFile, F_OK) == 0) { unlink(deltaFile); }
    if (access(tmpDestFile, F_OK) == 0) { unlink(tmpDestFile); }

    return rc;
}