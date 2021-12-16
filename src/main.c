#include "file_comdef.h"
#include "file_dev.h"
#include "internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <locale.h>

static void print_type(const char **pType, int type, int flag) 
{
    if (type == BACKUP) {
        if ((flag & FLAG_ADD_JOB) && (flag & FLAG_DO_JOB)) { *pType = "ADD & DO BACKUP"; }
        else if (flag & FLAG_ADD_JOB) { *pType = "ADD BACKUP"; }
        else if (flag & FLAG_DO_JOB) { *pType = "DO BACKUP"; }
    } else if (type == RESTORE) {
        if ((flag & FLAG_ADD_JOB) && (flag & FLAG_DO_JOB)) { *pType = "ADD & DO RESTORE"; }
        else if (flag & FLAG_ADD_JOB) { *pType = "ADD RESTORE"; }
        else if (flag & FLAG_DO_JOB) { *pType = "DO RESTORE"; }
    }
}

static void usage()
{
    printf("file_restore_demo < -b | -r >  < -a | -d | -a -d > PATH1 PATH2...\n");
    printf("-b --backup  : backup file\n");
    printf("-r --restore : restore file\n");
    printf("-a --add-job : noly add job to config json\n");
    printf("-d --do-job  : do job which in config json\n");
    printf("-h --help\n");
}

int main(int argc, char const *argv[])
{
    int rc = SUCCESS;
    int opt = 0;
    int longIndex;
    const char *file = NULL;
    int type = UNKNOWO;
    const char *pType = NULL;
    int flag = FLAG_UNKNOWO;

    static int test;
    const char *shortOpts= "hbradl";
    static struct option longOpts[] = {
        {"test", no_argument, &test, 15},
        {"help", no_argument, NULL, 'h'},
        {"backup", no_argument, NULL, 'b'},
        {"restore", no_argument, NULL, 'r'},
        {"add-job", no_argument, NULL, 'a'},
        {"do-job", no_argument, NULL, 'd'},
        {"file-list", no_argument, NULL, 'l'},
        {0, 0, 0, 0}
    };

    setlocale(LC_ALL, "");

    while ((opt = getopt_long(argc, (char * const *)argv, shortOpts, longOpts, &longIndex)) != -1)
    {
        switch (opt)
        {
        case 0:
            printf("test %d\n", test);
            break;
        case 'h':
            usage();
            return EXIT_SUCCESS;
        case 'b':
            type = BACKUP;
            break;
        case 'r':
            type = RESTORE;
            break; 
        case 'a':
            flag |= FLAG_ADD_JOB;
            break; 
        case 'd':
            flag |= FLAG_DO_JOB;
            break; 
        case 'l':
            flag |= FLAG_FILE_LIST;
            break;
        case '?':
            fprintf(stderr, "met ? arg\n");
            usage();
            exit(EXIT_FAILURE);
        default:
            usage();
            exit(EXIT_FAILURE);
        }
    }

    if (optind < 0 || optind > argc || type == UNKNOWO || flag == FLAG_UNKNOWO) {
        fprintf(stderr, "arg error\n");
        usage();
        exit(EXIT_FAILURE);
    }
    if (!(flag & FLAG_FILE_LIST)) {
        flag |= FLAG_SINGLE_FILE;
    }

    print_type(&pType, type, flag);

    if (optind < argc) {
        for (int i = optind; i < argc; i++) {
            file = argv[i];
            if (type == BACKUP) {
                rc = backup(file, flag);
            } else if (type == RESTORE) {
                rc = restore(file, flag);
            } 

            if (rc != SUCCESS) {
                fprintf(stderr, "%s %s job error\n", file, pType);
            } else {
                printf("%s %s job success\n", file, pType);
            }
        }
    } else if (optind == argc && (flag & FLAG_DO_JOB) && !(flag & FLAG_ADD_JOB)) {
        if (type == BACKUP) {
            rc = backup(NULL, flag);
        } else if (type == RESTORE) {
            rc = restore(NULL, flag);
        }

        if (rc != SUCCESS) {
            fprintf(stderr, "%s job error\n", pType); 
        }     
    } else {
        fprintf(stderr, "arg error\n"); 
        usage();
    }

    if (rc != SUCCESS) {
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
