AM_CFLAGS = -Wall -I. -std=gnu99 -I../include -D_GNU_SOURCE

lib_LTLIBRARIES = libkysec_file_restore.la 
libkysec_file_restore_la_SOURCES = kysec_file_data_sync.c kysec_file_utils.c kysec_file_attrs_sync.c
libkysec_file_restore_la_LIBADD = \
                        -lpthread \
                        -lrsync \
                        -lcrypto \
                        -lacl \
                        -lattr
