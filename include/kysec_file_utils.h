#ifndef _KYSEC_FILE_PROXY_
#define _KYSEC_FILE_PROXY_

#ifdef __cplusplus
extern "C" {
#endif

extern int kysec_check_hash(const char *origFile, const char *destFile);

extern int kysec_get_file_md5(const char *fileName, unsigned char digest[16]);

extern int kysec_file_md5_cmp(unsigned char disgest[16], unsigned char _disgest[16]);

extern int kysec_is_file_null(const char *fileName);

extern void kysec_make_dir(const char *path, int mode);

extern int kysec_make_file(const char*fileName, int mode);

#ifdef __cplusplus
}
#endif

#endif