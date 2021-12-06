#ifndef _KYSEC_FILE_RESTROE_
#define _KYSEC_FILE_RESTROE_

#include <time.h>
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

//FJOB TYPE
#define KYSEC_UNKNOWO   -1 
#define KYSEC_BACKUP    0
#define KYSEC_RESTORE   1

typedef enum EKeyName {
    EKEY_NAME = 0,
    EKEY_TIME,
    EKEY_TYPE
}EKeyName;

#define FJOB_ARRAY_T cJSON

typedef struct FJOB_ST {
    FJOB_ARRAY_T *array;
    int type;
}FJOB_ST;

typedef struct FJOB {
    char *name;
    time_t time;
    int type;
}FJOB;
//data

extern int kysec_rs_sig_file(const char *destFile, const char *sigFile, int useBlake2);

extern int kysec_rs_delta_file(const char *sigFile, const char *deltaFile, const char *origFile); 

extern int kysec_rs_patch_file(const char *destFile, const char *deltaFile, const char *outFile);

extern int kysec_file_data_sync(const char *origFile, const char *destFile);

extern int kysec_file_data_copy(const char *origFile, const char *destFile);

//attr

extern int kysec_file_stat_sync(const char *origFile, const char *destFile);

extern int kysec_file_acl_sync(const char *origFile, const char *destFile);

extern int kysec_file_attr_sync(const char *origFile, const char *destFile);

//job

extern FJOB_ST *kysec_job_jst_init();

extern void kysec_job_jst_delete(FJOB_ST *jst);

extern int kysec_add_job_to_jst(FJOB_ST *jst ,const FJOB *job);

extern int kysec_add_jst_to_file(const FJOB_ST *jst);

extern FJOB_ST *kysec_get_jst_from_file(const char *fileName);

extern int kysec_del_job_from_jst(FJOB_ST *jst, const FJOB *job);

#ifdef __cplusplus
}
#endif

#endif //_KYSEC_FILE_RESTROE_