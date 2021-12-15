#ifndef _FILE_BACKUP_
#define _FILE_BACKUP_

#ifdef __cplusplus
extern "C" {
#endif

#define FLAG_SINGLE_FILE (1 << 0)
#define FLAG_FILE_LIST   (1 << 1)
#define FLAG_ADD_JOB     (1 << 2)
#define FLAG_DO_JOB      (1 << 3)
int backup(const char *path, int flag);

int restore(const char *path, int flag);

#ifdef __cplusplus
}
#endif

#endif