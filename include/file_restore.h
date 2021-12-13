#ifndef _FILE_RESTROE_
#define _FILE_RESTROE_

#include <time.h>
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

//FJOB TYPE
#define UNKNOWO   -1 
#define BACKUP    0
#define RESTORE   1

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

extern int demo_rs_sig_file(const char *destFile, const char *sigFile, int useBlake2);

extern int demo_rs_delta_file(const char *sigFile, const char *deltaFile, const char *origFile); 

extern int demo_rs_patch_file(const char *destFile, const char *deltaFile, const char *outFile);

extern int demo_file_data_sync(const char *origFile, const char *destFile);

extern int demo_file_data_copy(const char *origFile, const char *destFile);

//attr

extern int demo_file_stat_sync(const char *origFile, const char *destFile);

extern int demo_file_acl_sync(const char *origFile, const char *destFile);

extern int demo_file_attr_sync(const char *origFile, const char *destFile);

//job

extern FJOB_ST *demo_job_jst_init();

extern void demo_job_jst_delete(FJOB_ST *jst);

extern int demo_add_job_to_jst(FJOB_ST *jst ,const FJOB *job);

extern int demo_add_jst_to_file(const FJOB_ST *jst);

extern FJOB_ST *demo_get_jst_from_file(const char *fileName);

extern int demo_del_job_from_jst(FJOB_ST *jst, const FJOB *job);

#ifdef __cplusplus
}
#endif

#endif //_FILE_RESTROE_