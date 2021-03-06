#include "file_dev.h"
#include "file_comdef.h"

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

#include <sys/time.h>

int demo_file_stat_sync(const char *origFile, const char *destFile)
{
    struct stat st;         
    struct timeval tv[2];
    int rc = SUCCESS;

    if ((rc = lstat(origFile, &st)) == -1) {
        perror("stat:");        
        rc = ERROR;
        goto out;
    }

    if (!S_ISREG(st.st_mode)) {
        perror("file type error");
        rc = ERROR;
        goto out;
    }

    if ((rc = chmod(destFile, st.st_mode)) == -1) {
        perror("chmod:");        
        rc = ERROR;
        goto out;
    }

    if ((rc = lchown(destFile, st.st_uid, st.st_gid)) == -1) {
        perror("chown:");        
        rc = ERROR;
        goto out;
    }

    tv[0].tv_sec = st.st_atime;
    tv[0].tv_usec = st.st_atim.tv_nsec / 1000;

    tv[1].tv_sec = st.st_mtime;
	tv[1].tv_usec = st.st_mtim.tv_nsec / 1000;

    if ((rc = lutimes(destFile, tv)) == -1) {
        perror("utimes:");        
        rc = ERROR;
        goto out;
    }

out:
    return rc;
}

int demo_file_acl_sync(const char *origFile, const char *destFile)
{
    int rc = SUCCESS;
    acl_t acl = NULL, defaultAcl = NULL;
    
    if ((acl = acl_get_file(origFile, ACL_TYPE_ACCESS)) == NULL) {
        perror("get access acl error!");
        rc = ERROR;
        goto out;
    }
/*
    if ((defaultAcl = acl_get_file(origFile, ACL_TYPE_DEFAULT)) == NULL) {
        perror("get default acl error!");
        rc = ERROR;
        goto out;
    }
*/

    if (acl_set_file(destFile, ACL_TYPE_ACCESS, acl) != 0) {
        perror("get default acl error!");
        rc = ERROR;
    }
/*
    if (acl_set_file(destFile, ACL_TYPE_DEFAULT, defaultAcl) != 0) {
        perror("get default acl error!");
        rc = ERROR;
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

int demo_file_attr_sync(const char *origFile, const char *destFile)
{
    int rc = SUCCESS;
    struct error_context ctx = {error, quote, quote_free};

    if ((rc = attr_copy_file(origFile, destFile, is_user_attr, &ctx)) != 0) {
        rc = ERROR;
        goto out;
    }
out:
    return rc;
}

int demo_file_attrs_sync(const char *origFile, const char *destFile)
{
    int rc = SUCCESS;

    rc = demo_file_acl_sync(origFile, destFile);
    if (rc != SUCCESS) {
        fprintf(stderr, "sync acl error\n");
        goto out;
    }

    rc = demo_file_attr_sync(origFile, destFile);
    if (rc != SUCCESS) {
        fprintf(stderr, "sync attr error\n");
        goto out;
    }

    rc = demo_file_stat_sync(origFile, destFile);
    if (rc != SUCCESS) {
        fprintf(stderr, "sync stat error\n");
        goto out;
    }

out:
    return rc; 
}