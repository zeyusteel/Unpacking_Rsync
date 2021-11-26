#include "kysec_file_restore.h"
#include "kysec_file_comdef.h"

#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>

#include <sys/acl.h>

int kysec_file_stat_sync(const char *origFile, const char *destFile)
{
    struct stat st;         
    int rc = KYSEC_SUCCESS;

    if ((rc = lstat(origFile, &st)) == -1) {
        perror("stat:");        
        rc = KYSEC_ERROR;
        goto out;
    }

    if ((rc = chmod(destFile, st.st_mode)) == -1) {
        perror("chmod:");        
        rc = KYSEC_ERROR;
        goto out;
    }

    if ((rc = lchown(destFile, st.st_uid, st.st_gid)) == -1) {
        perror("chown:");        
        rc = KYSEC_ERROR;
        goto out;
    }

out:
    return rc;
}

int kysec_file_acl_sync(const char *origFile, const char *destFile)
{
    EKYSEC_CODE rc = KYSEC_SUCCESS;
    acl_t acl = NULL, defaultAcl = NULL;
    
    if ((acl = acl_get_file(origFile, ACL_TYPE_ACCESS)) == NULL) {
        perror("get access acl error!");
        rc = KYSEC_ERROR;
        goto out;
    }
/*
    if ((defaultAcl = acl_get_file(origFile, ACL_TYPE_DEFAULT)) == NULL) {
        perror("get default acl error!");
        rc = KYSEC_ERROR;
        goto out;
    }
*/

    if (acl_set_file(destFile, ACL_TYPE_ACCESS, acl) != 0) {
        perror("get default acl error!");
        rc = KYSEC_ERROR;
    }
/*
    if (acl_set_file(destFile, ACL_TYPE_DEFAULT, defaultAcl) != 0) {
        perror("get default acl error!");
        rc = KYSEC_ERROR;
    }
*/

out:
    if (acl) { acl_free(acl); }
    if (defaultAcl) { acl_free(defaultAcl); }
    return rc;
}