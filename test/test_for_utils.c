#include "kysec_file_utils.h"
#include "kysec_file_comdef.h"
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
    
    kysec_get_file_md5(ORIG_FILE, md);

    kysec_get_file_md5(ORIG_FILE, mdnew);

    printf("%d\n",kysec_file_md5_cmp(md, mdnew));

    return 0;
}


int main(int argc, char const *argv[])
{
    int rc = KYSEC_SUCCESS;
    //kysec_make_dir("/home/zeyu/test/aaa/");

    //rc = kysec_make_file("/home/zeyu/test/aaaaa/bbb",0664);

    rc = kysec_file_data_cover(argv[1], DELTA_FILE, strlen(DELTA_FILE));
    printf("%d\n", rc);
    return 0;
}
