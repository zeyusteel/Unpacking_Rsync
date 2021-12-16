#include "file_dev.h"
#include "file_comdef.h"
#include "file_utils.h"
#include "internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

static int init_restore()
{
    if (access(RESTORE_JOB_JSON, F_OK) != 0 && 
        demo_make_file(RESTORE_JOB_JSON, 0777) != SUCCESS) {
        return ERROR;
    }

    return SUCCESS;
}

//被还原的文件路径必须是绝对路径
static int add_restore_job(const char *path) 
{
    int rc = SUCCESS;
    char *backupPath = NULL;

    FJOB_CTX *ctx = demo_get_ctx_from_file(RESTORE);
    if (!ctx) {
        ctx = demo_job_ctx_init(RESTORE);
    }

    if (path) {
        if (!(backupPath = (char*)malloc(PATH_MAX))) {
            rc = ERROR;
            goto out;
        }

        memset(backupPath, 0, PATH_MAX);
        strncat(backupPath, BACKUP_FILE_DIR, strlen(BACKUP_FILE_DIR));
        strncat(backupPath, path, PATH_MAX - 1);

        if (access(backupPath, F_OK) != 0) {
            fprintf(stderr, "backup file not exit, please do backup first\n");
            rc = ERROR;
            goto out;
        }

        FJOB job = {(char*)path, time(NULL), RESTORE};
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
    if (backupPath) { free(backupPath); }
    return rc;
}

static int restore_file(void *arg) 
{
    int rc = SUCCESS;
    char *backupFile = NULL;
    const char *destFile = (char *)arg;

    if (!(backupFile = (char*)malloc(PATH_MAX))) {
        rc = ERROR;
        goto out;
    }

    memset(backupFile, 0, PATH_MAX);
    strncat(backupFile, BACKUP_FILE_DIR, strlen(BACKUP_FILE_DIR));
    strncat(backupFile, destFile, PATH_MAX - 1);

    if (access(backupFile, F_OK) != 0) {
        fprintf(stderr, "backup file not exit\n");
        rc = ERROR;
    }
    
    if ((rc = demo_file_data_copy(backupFile, destFile)) != SUCCESS) {
        fprintf(stderr, "data sync error\n");
        goto out;
    } 

    if ((rc = demo_check_hash(backupFile, destFile)) != SUCCESS) {
        fprintf(stderr, "check hash error\n");
        goto out;
    }

    if ((rc = demo_file_attrs_sync(backupFile, destFile)) != SUCCESS) {
        fprintf(stderr, "attrs sync error\n");
        goto out;
    }

out:
    if (backupFile) { free(backupFile); }
    return rc;
}

static int do_restore_job(const char *path)
{    
    int rc = SUCCESS;
    FJOB_CTX *ctx = demo_get_ctx_from_file(RESTORE);
    if (!ctx) {
        fprintf(stderr, "get ctx error\n");
        rc = ERROR;
        goto out;
    }

    if (path) {
        rc = demo_del_job_from_ctx(ctx, path, restore_file);
    } else {
        rc = demo_del_job_from_ctx(ctx, NULL, restore_file);
    }

    if (rc != SUCCESS) 
        goto out;

    if ((rc = demo_add_ctx_to_file(ctx)) != SUCCESS) {
        goto out;
    }

out:
    if (ctx) { demo_job_ctx_delete(ctx); }
    return rc;

}

int restore(const char *path, int flag)
{
    int rc = SUCCESS;

    rc = init_restore();
    if (rc != SUCCESS) {
        fprintf(stderr ,"init restore error\n");
        goto out;
    }

    if (flag & FLAG_SINGLE_FILE) {

        if ((flag & FLAG_ADD_JOB) && path) {
            if ((rc = add_restore_job(path)) != SUCCESS) {
                goto out;
            }
        } 

        if (flag & FLAG_DO_JOB) {
            if (path) {
                if ((rc = do_restore_job(path)) != SUCCESS) {
                    goto out;
                }
            } else {
                if ((rc = do_restore_job(NULL)) != SUCCESS) {
                    goto out;
                }
            }
        } 

    } else if (flag & FLAG_FILE_LIST) {
        printf("prase file list\n");
    } else {
        fprintf(stderr, "flag error\n");
        rc = ERROR;
        goto out;
    }

out:
    return rc;
}