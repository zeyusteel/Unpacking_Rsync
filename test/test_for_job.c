#include "kysec_file_restore.h"
#include "kysec_file_comdef.h"

#include <time.h>
#include <stdio.h>
int main(int argc, char const *argv[])
{
    int rc;
    FJOB job;
        
    job.name = "/usr/sbin/kylin";
    job.type = KYSEC_BACKUP;
    job.time = time(NULL);

    FJOB_ST *jst = kysec_get_jst_from_file(BACKUP_JOB_JSON);
    if (!jst) {
        printf("errrrrr");
    }

 
    //rc = kysec_add_job_to_jst(jst, &job);

    //rc = kysec_add_jst_to_file(jst);



    rc = kysec_del_job_from_jst(jst, &job);

    kysec_job_jst_delete(jst);
    printf("%d\n", rc); 
    return 0;
}
