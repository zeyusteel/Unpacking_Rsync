#include "kysec_file_restore.h"
#include "kysec_file_comdef.h"
#include "kysec_file_utils.h"
#include "cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>

const char *szKeyName[] = {
    "name",
    "time",
    "type",
    NULL
};


FJOB_ST *kysec_job_jst_init()
{
    FJOB_ST *jst = (FJOB_ST*)malloc(sizeof(FJOB_ST));
    if (jst) { 
        memset(jst,0,sizeof(FJOB_ST));
        jst->type = KYSEC_UNKNOWO;
        return jst;
    }
    return NULL;
}

void kysec_job_jst_delete(FJOB_ST *jst)
{
    if (jst) {
        if (jst->array) { 
            cJSON_Delete(jst->array); 
        }
        free(jst);
    }
}

int kysec_add_job_to_jst(FJOB_ST *jst ,const FJOB *job)
{
    int rc = KYSEC_SUCCESS;
    cJSON *obj = NULL;

    if (!jst || !job) {
        rc = KYSEC_ERROR;
        goto out;
    }

    if (jst->type != KYSEC_UNKNOWO && jst->type != job->type) {
        fprintf(stderr, "type error\n");
        rc = KYSEC_ERROR;
        goto out;
    } else if (jst->type == KYSEC_UNKNOWO && job->type != KYSEC_UNKNOWO) {
        jst->type = job->type;
    } else if (job->type == KYSEC_UNKNOWO) {
        fprintf(stderr, "type error\n");
        rc = KYSEC_ERROR;
        goto out;
    }
        
    if (!jst->array) { jst->array = cJSON_CreateArray(); }
    cJSON_AddItemToArray(jst->array, obj = cJSON_CreateObject());
    cJSON_AddStringToObject(obj, szKeyName[EKEY_NAME], job->name);
    cJSON_AddNumberToObject(obj, szKeyName[EKEY_TIME], job->time);
    cJSON_AddNumberToObject(obj, szKeyName[EKEY_TYPE], job->type);

out: 
    return rc;
}

int kysec_add_jst_to_file(const FJOB_ST *jst)
{
    int rc = KYSEC_SUCCESS;
    char *output = NULL;

    if (!jst) {
        rc = KYSEC_ERROR;
        goto out;
    }

    if (jst->array && (output = cJSON_Print(jst->array))) {
        if (jst->type == KYSEC_BACKUP) {
            rc = kysec_file_data_cover(BACKUP_JOB_JSON, output, strlen(output));
        } else if (jst->type == KYSEC_RESTORE) {
            rc = kysec_file_data_cover(RESTORE_JOB_JSON, output, strlen(output));
        } else {
            fprintf(stderr, "type error\n");
            rc = KYSEC_ERROR;
            goto out;
        }
    }

out:
    if (output) { free(output); }
    return rc;    
}


FJOB_ST *kysec_get_jst_from_file(const char *fileName)
{
    int rc = KYSEC_SUCCESS;
    FJOB_ST *jst = NULL;
    FILE *fp = NULL;
    long len;
    char *data = NULL;

    jst = kysec_job_jst_init();

    if (strcmp(fileName, BACKUP_JOB_JSON) == 0) {
        jst->type = KYSEC_BACKUP;
    } else if (strcmp(fileName, RESTORE_JOB_JSON) == 0) {
        jst->type = KYSEC_RESTORE;
    } else {
        rc = KYSEC_ERROR;
        goto out;
    }

    fp = fopen(fileName, "rb");
    if (!fp) {
        rc = KYSEC_ERROR;
        goto out;
    }

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);   
    fseek(fp, 0, SEEK_SET);

    if (len < 0) {
        fprintf(stderr, "ftell err");
        rc = KYSEC_ERROR;
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
            rc = KYSEC_ERROR;
            goto out;
        }
        if (json->type == cJSON_Array) {
            jst->array = json;
        } else {
            fprintf(stderr, "cjson prase err");
            rc = KYSEC_ERROR;
            cJSON_Delete(json);
            goto out;
        }
    }

out:
    if (rc != KYSEC_SUCCESS) {
        kysec_job_jst_delete(jst);
        jst = NULL;
    }
    if (fp) { fclose(fp); }
    if (data) { free(data); }
    return jst;
}

int kysec_del_job_from_jst(FJOB_ST *jst, const FJOB *job)
{
    int rc = KYSEC_SUCCESS;
    int size = 0;
    int del = -1;

    if (!jst || !jst->array) {
        rc = KYSEC_ERROR;
        goto out;
    }

    size = cJSON_GetArraySize(jst->array);

    for (int i = 0; i < size; ++i) {
        cJSON *obj =  cJSON_GetArrayItem(jst->array, i);
        if (obj && obj->type == cJSON_Object) {
            cJSON *item = cJSON_GetObjectItem(obj, szKeyName[EKEY_NAME]);
            if (item && item->valuestring && strcmp(item->valuestring, job->name) == 0) {
                del = i;
                break;
            }
        }
    }

    if (del != -1) {
        cJSON_DeleteItemFromArray(jst->array, del);
    }

out:
    return rc;
}