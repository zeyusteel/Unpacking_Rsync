#ifndef _KYSEC_FILE_RESTROE_
#define _KYSEC_FILE_RESTROE_

#ifdef __cplusplus
extern "C" {
#endif

//Error Number
typedef enum EKYSEC_CODE {
    KYSEC_SUCCESS   =   0,
    KYSEC_ERROR     =   1
}EKYSEC_CODE;

//comdef
#ifndef bool
#define bool int
#define true 1
#define false 0
#endif

extern int kysec_rs_sig_file(const char *oldFilePath, const char *sigFilePath, bool useBLAKE2);

extern int kysec_rs_delta_file(const char *sigFilePath, const char *deltaFilePath, const char *newFilePath); 
#ifdef __cplusplus
}
#endif

#endif //_KYSEC_FILE_RESTROE_