#include "kysec_file_restore.h"
#include <stdio.h>

#define OLD_FILE "/home/zeyu/old"
#define NEW_FILE "/home/zeyu/new"
#define SIG_FILE "/home/zeyu/sig"
#define DELTA_FILE "/home/zeyu/delta"

int main(int argc, char const *argv[])
{
    int rc = kysec_rs_sig_file(OLD_FILE, SIG_FILE, 1);
    printf("return code : %d\n", rc);

    rc = kysec_rs_delta_file(SIG_FILE, DELTA_FILE, NEW_FILE);
    printf("return code : %d\n", rc);
    
    return 0;
}
