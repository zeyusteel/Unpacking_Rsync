#include "kysec_file_restore.h"
#include "kysec_file_proxy.h"
#include "kysec_file_comdef.h"

#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>

#define ORIG_FILE "./test/test_dir/orig"
#define DEST_FILE "./test/test_dir/dest"
#define SIG_FILE "./test/test_dir/sig"
#define DELTA_FILE "./test/test_dir/delta"
#define OUT_FILE "./test/test_dir/out"

int test_for_rsync() 
{
    int rc = kysec_rs_sig_file(ORIG_FILE, SIG_FILE, 0);
    if (rc != KYSEC_SUCCESS) {
        printf("return code : %d\n", rc);
        return -1;
    }

    rc = kysec_rs_delta_file(SIG_FILE, DELTA_FILE, DEST_FILE);
    if (rc != KYSEC_SUCCESS) {
        printf("return code : %d\n", rc);
        return -1;
    }

    rc = kysec_rs_patch_file(ORIG_FILE, DELTA_FILE, DEST_FILE);
    if (rc != KYSEC_SUCCESS) {
        printf("return code : %d\n", rc);
        return -1;
    }
    return 0;
}


int main(int argc, char const *argv[])
{
    int rc = 0;
    rc = kysec_file_sync("/usr/sbin/kylin-log-viewer", "./test/test_dir/kk");

    //test_for_rsync();

    rc = kysec_file_copy(ORIG_FILE, DEST_FILE);
    printf("%d\n", rc);

    return 0;
}