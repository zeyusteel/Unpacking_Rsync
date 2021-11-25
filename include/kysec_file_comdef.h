#ifndef _KYSEC_FILE_COMDEF_
#define _KYSEC_FILE_COMDEF_

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

//Error Number
typedef enum EKYSEC_CODE {
    KYSEC_SUCCESS   =   0,
    KYSEC_ERROR     =   1
}EKYSEC_CODE;

//backup job list
#define BACKUP_JOB_LIST "/tmp/backup.json"
#define RESTORE_JOB_LIST "/tmp/restore.json"

//FJOB TYPE
#define KYSEC_BACKUP    0
#define KYSEC_RESTORE   1

typedef struct FJOB {
    char *filePath;
    time_t time;
    int type;
}FJOB;

//comdef
#define BUF_SIZE 1024

#ifndef bool
#define bool int
#define true 1
#define false 0
#endif

#ifdef __cplusplus
}
#endif

#endif