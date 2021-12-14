#include "file_dev.h"
#include "file_comdef.h"
#include "file_utils.h"
#include "cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>

#define FJOB_ARRAY_T cJSON

struct FJOB_CTX {
    FJOB_ARRAY_T *array;
    int type;
};

typedef enum EKeyName {
    EKEY_NAME = 0,
    EKEY_TIME,
    EKEY_TYPE
}EKeyName;

const char *szKeyName[] = {
    "name",
    "time",
    "type",
    NULL
};

FJOB_CTX *demo_job_ctx_init(int type)
{
    FJOB_CTX *ctx = (FJOB_CTX*)malloc(sizeof(FJOB_CTX));
    if (ctx) { 
        memset(ctx,0,sizeof(FJOB_CTX));
        ctx->type = type;
        return ctx;
    }
    return NULL;
}

void demo_job_ctx_delete(FJOB_CTX *ctx)
{
    if (ctx) {
        if (ctx->array) { 
            cJSON_Delete(ctx->array); 
        }
        free(ctx);
    }
}

int demo_add_job_to_ctx(FJOB_CTX *ctx ,const FJOB *job)
{
    int rc = SUCCESS;
    cJSON *obj = NULL;

    if (!ctx || !job) {
        rc = ERROR;
        goto out;
    }

    if (ctx->type != UNKNOWO && ctx->type != job->type) {
        fprintf(stderr, "type error\n");
        rc = ERROR;
        goto out;
    } else if (ctx->type == UNKNOWO && job->type != UNKNOWO) {
        ctx->type = job->type;
    } else if (job->type == UNKNOWO) {
        fprintf(stderr, "type error\n");
        rc = ERROR;
        goto out;
    }
        
    if (!ctx->array) { ctx->array = cJSON_CreateArray(); }
    cJSON_AddItemToArray(ctx->array, obj = cJSON_CreateObject());
    cJSON_AddStringToObject(obj, szKeyName[EKEY_NAME], job->name);
    cJSON_AddNumberToObject(obj, szKeyName[EKEY_TIME], job->time);
    cJSON_AddNumberToObject(obj, szKeyName[EKEY_TYPE], job->type);

out: 
    return rc;
}

int demo_add_ctx_to_file(const FJOB_CTX *ctx)
{
    int rc = SUCCESS;
    char *output = NULL;

    if (!ctx) {
        rc = ERROR;
        goto out;
    }

    if (ctx->array && (output = cJSON_Print(ctx->array))) {
        if (ctx->type == BACKUP) {
            rc = demo_file_data_cover(BACKUP_JOB_JSON, output, strlen(output));
        } else if (ctx->type == RESTORE) {
            rc = demo_file_data_cover(RESTORE_JOB_JSON, output, strlen(output));
        } else {
            fprintf(stderr, "type error\n");
            rc = ERROR;
            goto out;
        }
    }

out:
    if (output) { free(output); }
    return rc;    
}


FJOB_CTX *demo_get_ctx_from_file(int type)
{
    int rc = SUCCESS;
    FJOB_CTX *ctx = NULL;
    FILE *fp = NULL;
    long len;
    char *data = NULL;
    const char *fileName = NULL;

    ctx = INIT_CTX();

    if (type == BACKUP) {
        ctx->type = BACKUP;
        fileName = BACKUP_JOB_JSON;
    } else if (type == RESTORE) {
        ctx->type = RESTORE;
        fileName = RESTORE_JOB_JSON;
    } else {
        rc = ERROR;
        goto out;
    }

    fp = fopen(fileName, "rb");
    if (!fp) {
        rc = ERROR;
        goto out;
    }

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);   
    fseek(fp, 0, SEEK_SET);

    if (len < 0) {
        fprintf(stderr, "ftell err");
        rc = ERROR;
        goto out;
    } else if (len == 0) {
        goto out;
    } else {
        data = malloc(len + 1);
        memset(data, 0, len+1);
        if (fread(data, 1, len, fp) != len) {
            fprintf(stderr, "read error\n");
            goto out;
        }

        data[len] = '\0';

        cJSON *json = cJSON_Parse(data);
        if (!json) {
            fprintf(stderr, "cjson prase err");
            rc = ERROR;
            goto out;
        }
        if (json->type == cJSON_Array) {
            ctx->array = json;
        } else {
            fprintf(stderr, "cjson prase err");
            rc = ERROR;
            cJSON_Delete(json);
            goto out;
        }
    }

out:
    if (rc != SUCCESS) {
        demo_job_ctx_delete(ctx);
        ctx = NULL;
    }
    if (fp) { fclose(fp); }
    if (data) { free(data); }
    return ctx;
}

int demo_del_job_from_ctx(FJOB_CTX *ctx, const char *file, callback fun)
{
    int rc = SUCCESS;
    int size = 0;
    int del = -1;

    if (!ctx || !ctx->array) {
        rc = ERROR;
        goto out;
    }

    if (!file && !fun) {
        rc = ERROR;
        goto out;
    }

    if (file) {
        size = cJSON_GetArraySize(ctx->array);

        for (int i = 0; i < size; ++i) {
            cJSON *obj =  cJSON_GetArrayItem(ctx->array, i);
            if (obj && obj->type == cJSON_Object) {
                cJSON *item = cJSON_GetObjectItem(obj, szKeyName[EKEY_NAME]);
                if (item && item->valuestring && strcmp(item->valuestring, file) == 0) {
                    del = i;
                    break;
                }
            }
        }

        if (del != -1) {
            if (fun && fun((void*)file) != SUCCESS) {
                return ERROR;
            }
            cJSON_DeleteItemFromArray(ctx->array, del);
        }
    } else {
        while(cJSON_GetArraySize(ctx->array)) {
            cJSON *obj =  cJSON_GetArrayItem(ctx->array, 0);
            if (cJSON_IsObject(obj)) {
                cJSON *item = cJSON_GetObjectItem(obj, szKeyName[EKEY_NAME]);
                if (item && item->valuestring && fun) {
                    rc = fun((void*)item->valuestring);
                    if (rc != SUCCESS) {
                        return ERROR;
                    }
                }
                cJSON_DeleteItemFromArray(ctx->array, 0);
            }
        }
    }
out:
    return rc;
}