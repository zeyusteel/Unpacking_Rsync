#include "file_dev.h"
#include "file_comdef.h"

#include <time.h>
#include <stdio.h>
int main(int argc, char const *argv[])
{
    int rc;
    FJOB job;
        
    job.name = "/usr/sbin/kylin";
    job.type = BACKUP;
    job.time = time(NULL);

    FJOB_CTX *ctx = demo_get_ctx_from_file(BACKUP);
    if (!ctx) {
        printf("errrrrr");
    }

    //FJOB_CTX *ctx = INIT_CTX();
    rc = demo_add_job_to_ctx(ctx, &job);

    rc = demo_add_ctx_to_file(ctx);



    //rc = demo_del_job_from_ctx(ctx, &job);

    demo_job_ctx_delete(ctx);
    printf("%d\n", rc); 
    return 0;
}
