#include "file_comdef.h"
#include "file_dev.h"
#include "file_backup.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

int main(int argc, char const *argv[])
{
    int rc = SUCCESS;
    int opt = 0;
    int longIndex;
    const char *file = NULL;
    int type = UNKNOWO;
    int arg = 0;

    const char *shortOpts= "hbrf:ad";
    static struct option longOpts[] = {
        {"help", no_argument, NULL, 'h'},
        {"backup", no_argument, NULL, 'b'},
        {"restore", no_argument, NULL, 'r'},
        {"file", required_argument, NULL, 'f'},
        {"add-job", no_argument, NULL, 'a'},
        {"do-job", no_argument, NULL, 'd'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, (char * const *)argv, shortOpts, longOpts, &longIndex)) != -1)
    {
        switch (opt)
        {
        case 'h':
            printf("file backup and restore demo\n");
            return EXIT_SUCCESS;
        case 'b':
            type = BACKUP;
            break;
        case 'r':
            type = RESTORE;
            break; 
        case 'f':
            file = optarg;
            break; 
        case 'a':
            arg = FLAG_ADD_JOB;
            break; 
        case 'd':
            arg = FLAG_DO_JOB;
            break; 
        case '?':
            fprintf(stderr, "met ? arg\n");
            break;
        default:
            exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        fprintf(stderr, "Non-option ARGV-elements: ");
        for (int i = optind; i < argc; i++) {
            fprintf(stderr, "%s ", argv[i]);
        }
        fprintf(stderr, "\n");
        exit(EXIT_FAILURE);
    } 

    if (type == BACKUP) {
        int flag;
        if (file) {
            flag = FLAG_SINGLE_FILE;
        }
        if (arg) { flag |= arg; }
        rc = file_backup(file, flag);

    } else if (type == RESTORE) {

    } else {
        fprintf(stderr, "unmet type\n");
        exit(EXIT_FAILURE);
    }

    if (rc != SUCCESS) {
        fprintf(stderr, "error\n");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
