AM_CFLAGS = -Wall -I. -std=gnu99 -I../include -D_GNU_SOURCE

lib_LTLIBRARIES = libfile_restore.la 
libfile_restore_la_SOURCES = \
                                    file_data_sync.c \
                                    file_utils.c \
                                    file_attrs_sync.c \
                                    job_proxy.c \
                                    cJSON.c
libfile_restore_la_LIBADD = \
                        -lpthread \
                        -lrsync \
                        -lcrypto \
                        -lacl \
                        -lattr \
                        -lm
