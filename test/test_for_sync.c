#include "file_dev.h"
#include "file_utils.h"
#include "file_comdef.h"

#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>

#define ORIG_FILE "test_dir/orig"
#define DEST_FILE "test_dir/dest"
#define SIG_FILE "test_dir/sig"
#define DELTA_FILE "test_dir/delta"
#define OUT_FILE "test_dir/out"

int test_for_rsync() 
{
    int rc = demo_rs_sig_file(DEST_FILE, SIG_FILE, 0);
    if (rc != SUCCESS) {
        printf("--return code : %d\n", rc);
        return -1;
    }

    rc = demo_rs_delta_file(SIG_FILE, DELTA_FILE, ORIG_FILE);
    if (rc != SUCCESS) {
        printf("return code : %d\n", rc);
        return -1;
    }

    rc = demo_rs_patch_file(DEST_FILE, DELTA_FILE, OUT_FILE);
    if (rc != SUCCESS) {
        printf("return code : %d\n", rc);
        return -1;
    }
    return 0;
}

int test_for_md5() 
{
    unsigned char md[16] = {0};
    unsigned char mdnew[16] = {0};
    
    demo_get_file_md5("/usr/sbin/kylin-log-viewer", md);

    demo_get_file_md5("test_dir/kk", mdnew);

    printf("%d\n",demo_file_md5_cmp(md, mdnew));

    return 0;
}

int main(int argc, char const *argv[])
{
    int rc = 0;
    //rc = test_for_rsync();
    //rc = demo_file_data_sync(ORIG_FILE, DEST_FILE);
    //rc = demo_file_data_sync(ORIG_FILE,DEST_FILE);
    rc = demo_file_data_copy("/usr/sbin/kylin-log-viewer", "test_dir/kk");
    //test_for_md5();

    //rc = demo_file_data_copy(ORIG_FILE, BACKUP_JOB_JSON);

    printf("%d\n", rc);

    return 0;
}