#ifndef _KYSEC_FILE_COMDEF_
#define _KYSEC_FILE_COMDEF_

#ifdef __cplusplus
extern "C" {
#endif

//Error Number
typedef enum EKYSEC_CODE {
    KYSEC_SUCCESS   =   0,
    KYSEC_ERROR     =   1
}EKYSEC_CODE;

#define MINIMUM_SYSTEM_JSON "/etc/kysec/restore/minimum_system.json" 
#define BACKUP_FILE_DIR "/etc/kysec/restore/backup_file"
//backup job list
#define BACKUP_JOB_JSON "/etc/kysec/restore/backup.json"
#define RESTORE_JOB_JSON "/etc/kysec/restore/restore.json"


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