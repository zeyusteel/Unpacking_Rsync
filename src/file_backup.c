#include "file_comdef.h"
#include "file_utils.h"
#include "internal.h"
#include "file_dev.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

static int init_backup() 
{
    if (access(BACKUP_JOB_JSON, F_OK) != 0 && 
        demo_make_file(BACKUP_JOB_JSON, 0777) != SUCCESS) {
        return ERROR;
    }

    if (access(BACKUP_FILE_DIR, F_OK) != 0 ) {
        demo_make_dir(BACKUP_FILE_DIR, 0777);
    }

    if (access(BACKUP_FILE_DIR, F_OK) != 0 ) {
        return ERROR;
    }
    return SUCCESS;
}

static int add_backup_job(const char *path) 
{
    int rc = SUCCESS;
    FJOB_CTX *ctx = demo_get_ctx_from_file(BACKUP);
    if (!ctx) {
        ctx = demo_job_ctx_init(BACKUP);
    }
    if (path) {
        FJOB job = {(char *)path, time(NULL), BACKUP};
        if ((rc = demo_add_job_to_ctx(ctx, &job)) != SUCCESS) {
            goto out;
        }

        if ((rc = demo_add_ctx_to_file(ctx)) != SUCCESS) {
            goto out;
        }
    } else {
        rc = ERROR;
        goto out;
    }

out:
    if (ctx) { demo_job_ctx_delete(ctx); }
    return rc;
}

static int backup_file(void *arg)
{
    int rc = SUCCESS;
    char *absOrig = NULL;
    char *destFile = NULL;
    const char *origFile = (char *)arg;

    if (!(absOrig = (char*)malloc(PATH_MAX))) {
        rc = ERROR;
        goto out;
    }

    if (!(destFile = (char*)malloc(PATH_MAX))) {
        rc = ERROR;
        goto out;
    }

    memset(absOrig, 0, PATH_MAX);
    memset(destFile, 0, PATH_MAX);

    printf("%s\n", origFile);
    if (!realpath(origFile, absOrig)) {
        perror(":");
        rc = ERROR;
        goto out;
    }

    strcat(destFile, BACKUP_FILE_DIR);
    strcat(destFile, absOrig);

    
    printf("this is backup file func\n");
    printf("--%s\n", origFile);
    printf("--%s\n", destFile);

out:
    if (absOrig) { free(absOrig); }
    if (destFile) { free(destFile); }
    return rc;
}

static int do_backup_job(const char *path)
{
    int rc = SUCCESS;
    FJOB_CTX *ctx = demo_get_ctx_from_file(BACKUP);
    if (!ctx) {
        fprintf(stderr, "get ctx error\n");
        rc = ERROR;
        goto out;
    }

    if (path) {
        printf("do sigle job %s\n", path);
        rc = demo_del_job_from_ctx(ctx, path, backup_file);
    } else {
        printf("do all job\n");
        rc = demo_del_job_from_ctx(ctx, NULL, backup_file);
    }
/*
    if ((rc = demo_add_ctx_to_file(ctx)) != SUCCESS) {
        goto out;
    }
*/
out:
    if (ctx) { demo_job_ctx_delete(ctx); }
    return rc;
}

int backup(const char *path, int flag)
{
    int rc;
    if ((rc = init_backup()) != SUCCESS) {
        goto out;
    }

    if (flag & FLAG_SINGLE_FILE) {
        if (flag & FLAG_ADD_JOB) {
            if ((rc = add_backup_job(path)) != SUCCESS) {
                goto out;
            }
        } else if (flag & FLAG_DO_JOB) {
            if ((rc = do_backup_job(NULL)) != SUCCESS) {
                goto out;
            }
        } else {
            fprintf(stderr, "flag error\n");
            rc = ERROR;
            goto out;
        }

    } else if (flag & FLAG_FILE_LIST) {

    } else {
        fprintf(stderr, "flag error\n");
        rc = ERROR;
        goto out;
    }

out:
    return rc;
}


