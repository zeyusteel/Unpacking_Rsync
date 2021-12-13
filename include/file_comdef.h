#ifndef _FILE_COMDEF_
#define _FILE_COMDEF_

#ifdef __cplusplus
extern "C" {
#endif

//Error Number
typedef enum ECODE {
    SUCCESS   =   0,
    ERROR     =   1
}ECODE;

#define MINIMUM_SYSTEM_JSON "/etc/restore/minimum_system.json" 
#define BACKUP_FILE_DIR "/etc/restore/backup_file"
//backup job list
#define BACKUP_JOB_JSON "/etc/restore/backup.json"
#define RESTORE_JOB_JSON "/etc/restore/restore.json"


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