#ifndef _FILE_RESTROE_
#define _FILE_RESTROE_

#include <time.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

//FJOB TYPE
#define UNKNOWO   -1 
#define BACKUP    0
#define RESTORE   1

typedef struct FJOB_CTX FJOB_CTX;

typedef struct FJOB {
    char *name;
    time_t time;
    int type;
}FJOB;

//data

extern int demo_rs_sig_fp(FILE *fpDest, FILE *fpSig, int useBlake2);

extern int demo_rs_sig_file(const char *destFile, const char *sigFile, int useBlake2);

extern int demo_rs_delta_fp(FILE *fpSig, FILE *fpDelta, FILE *fpOrig); 

extern int demo_rs_delta_file(const char *sigFile, const char *deltaFile, const char *origFile); 

extern int demo_rs_patch_fp(FILE *fpDest, FILE *fpDelta, FILE *fpOut);

extern int demo_rs_patch_file(const char *destFile, const char *deltaFile, const char *outFile);

extern int demo_file_data_sync(const char *origFile, const char *destFile);

extern int demo_file_data_copy(const char *origFile, const char *destFile);

//attr

extern int demo_file_stat_sync(const char *origFile, const char *destFile);

extern int demo_file_acl_sync(const char *origFile, const char *destFile);

extern int demo_file_attr_sync(const char *origFile, const char *destFile);

//job
extern FJOB_CTX *demo_job_ctx_init(int type);

#define INIT_CTX() demo_job_ctx_init(UNKNOWO)

extern void demo_job_ctx_delete(FJOB_CTX *ctx);

extern int demo_add_job_to_ctx(FJOB_CTX *ctx ,const FJOB *job);

extern int demo_add_ctx_to_file(const FJOB_CTX *ctx);

extern FJOB_CTX *demo_get_ctx_from_file(int type);

typedef int (*callback)(void *);
extern int demo_del_job_from_ctx(FJOB_CTX *ctx, const char *file, callback fun);

#ifdef __cplusplus
}
#endif

#endif //_FILE_RESTROE_