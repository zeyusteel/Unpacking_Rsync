#include "file_utils.h"
#include "file_comdef.h"
#include <stdio.h>
#include <string.h>

#define ORIG_FILE "test_dir/orig"
#define DEST_FILE "test_dir/dest"
#define SIG_FILE "test_dir/sig"
#define DELTA_FILE "test_dir/delta"
#define OUT_FILE "test_dir/out"

int test_for_md5() 
{
    unsigned char md[16] = {0};
    unsigned char mdnew[16] = {0};
    
    demo_get_file_md5(ORIG_FILE, md);

    demo_get_file_md5(ORIG_FILE, mdnew);

    printf("%d\n",demo_file_md5_cmp(md, mdnew));

    return 0;
}


int main(int argc, char const *argv[])
{
    int rc = SUCCESS;
    //demo_make_dir("/home/zeyu/test/aaa/");

    //rc = demo_make_file("/home/zeyu/test/aaaaa/bbb",0664);

    rc = demo_file_data_cover(argv[1], "wangzeyu", strlen("wangzeyu"));
    printf("%d\n", rc);
    return 0;
}
