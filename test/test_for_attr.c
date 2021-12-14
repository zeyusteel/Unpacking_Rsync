#include "file_dev.h"
#include "file_comdef.h"

#include <stdio.h>

#define ORIG_FILE "test_dir/orig"
#define DEST_FILE "test_dir/dest"

int main(int argc, char const *argv[])
{
    int rc =  SUCCESS;
    //rc = demo_file_acl_sync(ORIG_FILE, DEST_FILE);
    rc = demo_file_stat_sync(ORIG_FILE, DEST_FILE);

    //rc = demo_file_attr_sync(ORIG_FILE, DEST_FILE);

    printf("%d\n", rc);
    return 0;
}
