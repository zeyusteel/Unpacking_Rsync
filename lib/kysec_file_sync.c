#include "kysec_file_restore.h"

#include <stdio.h>
#include <librsync.h>

int kysec_rs_sig_file(const char *oldFilePath, const char *sigFilePath, bool useBLAKE2)
{
    FILE *fdOld = NULL, *fdSig = NULL;
    EKYSEC_CODE rc = KYSEC_SUCCESS;

    fdOld = fopen(oldFilePath, "rb");
    fdSig = fopen(sigFilePath, "wb");

    if (!oldFilePath || !sigFilePath) {
        rc = KYSEC_ERROR;
        goto out;
    }

    rs_result res; 
    if (useBLAKE2) {
        res = rs_sig_file(fdOld, fdSig, RS_DEFAULT_BLOCK_LEN, 0, RS_BLAKE2_SIG_MAGIC, NULL);
    } else {
        res = rs_sig_file(fdOld, fdSig, RS_DEFAULT_BLOCK_LEN, 0, RS_MD4_SIG_MAGIC, NULL);
    }

    if (res != RS_DONE) {
        printf("%s\n", rs_strerror(res));
        rc = KYSEC_ERROR;
        goto out;
    }

out:
    if (fdOld) {
        fclose(fdOld);
        fdOld = NULL;
    }
    if (fdSig) {
        fclose(fdSig);
        fdSig = NULL; 
    }
    return rc;
}

int kysec_rs_delta_file(const char *sigFilePath, const char *deltaFilePath, const char *newFilePath) 
{
    EKYSEC_CODE rc = KYSEC_SUCCESS;
    rs_result res;
    rs_signature_t *sumset = NULL;
    FILE *fdSig = NULL, *fdDelta = NULL, *fdNew = NULL;

    fdSig = fopen(sigFilePath, "rb");
    fdDelta = fopen(deltaFilePath, "wb");
    fdNew = fopen(newFilePath, "rb");

    if (!fdSig || !fdDelta || !fdNew) {
        rc = KYSEC_ERROR;
        goto out;
    }

    res = rs_loadsig_file(fdSig, &sumset, NULL);
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

    res = rs_delta_file(sumset, fdNew, fdDelta, NULL);
    if (res != RS_DONE) {
        printf("%s\n", rs_strerror(res));
        rc = KYSEC_ERROR;
        goto out;
    }

out:
    if (fdSig) {
        fclose(fdSig);
        fdSig = NULL;
    }
    if (fdDelta) {
        fclose(fdDelta);
        fdDelta = NULL;
    }
    if (fdNew) {
        fclose(fdNew);
        fdNew = NULL;
    }
    if (sumset) {
        rs_free_sumset(sumset);
    }

    return rc;
}