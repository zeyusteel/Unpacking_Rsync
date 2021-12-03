#include "kysec_file_restore.h"
#include "kysec_file_comdef.h"

#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include <sys/acl.h>

#include <attr/libattr.h>
#include <attr/error_context.h>


int kysec_file_stat_sync(const char *origFile, const char *destFile)
{
    struct stat st;         
    int rc = KYSEC_SUCCESS;

    if ((rc = lstat(origFile, &st)) == -1) {
        perror("stat:");        
        rc = KYSEC_ERROR;
        goto out;
    }

    if (!S_ISREG(st.st_mode)) {
        perror("file type error");
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

static void error(struct error_context *ctx, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, ": %s\n", strerror(errno));
    va_end(ap);
}

static const char *quote(struct error_context *ctx, const char *pathName) 
{
    char *pn = strdup(pathName), *p;

    for (p = pn; *p != '\0'; p++) {
        if (*p & 0x80)
            *p = '?';
    }
    return pn;
}

static void quote_free(struct error_context *ctx, const char *name)
{
    free((void *)name);
}

static int is_user_attr(const char *name, struct error_context *ctx)
{
    return strncmp(name, "user.", 5) == 0;
}

int kysec_file_attr_sync(const char *origFile, const char *destFile)
{
    EKYSEC_CODE rc = KYSEC_SUCCESS;
    struct error_context ctx = {error, quote, quote_free};

    if ((rc = attr_copy_file(origFile, destFile, is_user_attr, &ctx)) != 0) {
        rc = KYSEC_ERROR;
        goto out;
    }
out:
    return rc;
}