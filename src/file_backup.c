#include "file_comdef.h"
#include "file_utils.h"
#include "file_backup.h"
#include "file_dev.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

int file_backup(const char *path, int flag)
{
    int rc;
    if ((rc = init_backup()) != SUCCESS) {
        goto out;
    }

    if (flag & FLAG_SINGLE_FILE) {
        printf("sigle file\n");
        if (flag & FLAG_ADD_JOB) {
            if ((rc = add_backup_job(path)) != SUCCESS) {
                goto out;
            }
        } else if (flag & FLAG_DO_JOB) {

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


