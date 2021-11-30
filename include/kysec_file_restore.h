#ifndef _KYSEC_FILE_RESTROE_
#define _KYSEC_FILE_RESTROE_

#ifdef __cplusplus
extern "C" {
#endif

extern int kysec_rs_sig_file(const char *destFile, const char *sigFile, int useBlake2);

extern int kysec_rs_delta_file(const char *sigFile, const char *deltaFile, const char *origFile); 

extern int kysec_rs_patch_file(const char *destFile, const char *deltaFile, const char *outFile);

extern int kysec_file_sync(const char *origFile, const char *destFile);

extern int kysec_file_copy(const char *origFile, const char *destFile);

extern int kysec_file_stat_sync(const char *origFile, const char *destFile);

extern int kysec_file_acl_sync(const char *origFile, const char *destFile);

extern int kysec_file_attr_sync(const char *origFile, const char *destFile);

#ifdef __cplusplus
}
#endif

#endif //_KYSEC_FILE_RESTROE_