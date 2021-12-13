#ifndef _FILE_PROXY_
#define _FILE_PROXY_

#ifdef __cplusplus
extern "C" {
#endif

extern int demo_check_hash(const char *origFile, const char *destFile);

extern int demo_get_file_md5(const char *fileName, unsigned char digest[16]);

extern int demo_file_md5_cmp(unsigned char disgest[16], unsigned char _disgest[16]);

extern int demo_is_file_null(const char *fileName);

extern void demo_make_dir(const char *path, int mode);

extern int demo_make_file(const char*fileName, int mode);

extern int demo_file_data_cover(const char *fileName, const char *data, int len);

#ifdef __cplusplus
}
#endif

#endif