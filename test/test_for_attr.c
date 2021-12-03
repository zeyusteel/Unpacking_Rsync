#include "kysec_file_restore.h"
#include "kysec_file_comdef.h"

#include <stdio.h>

#define ORIG_FILE "test_dir/orig"
#define DEST_FILE "test_dir/dest"

int main(int argc, char const *argv[])
{
    int rc =  KYSEC_SUCCESS;
    rc = kysec_file_acl_sync(ORIG_FILE, DEST_FILE);
    rc = kysec_file_stat_sync(ORIG_FILE, DEST_FILE);

    //rc = kysec_file_attr_sync(ORIG_FILE, DEST_FILE);

    printf("%d\n", rc);
    return 0;
}
