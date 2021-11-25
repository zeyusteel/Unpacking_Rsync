AM_CFLAGS =  -Wall -I. -std=gnu99 -I../include -D_GNU_SOURCE

bin_PROGRAMS = test_for_sync test_for_file 

test_for_sync_SOURCES = test_for_sync.c
test_for_sync_LDADD = -L$(top_builddir)/lib -lkysec_file_restore

test_for_file_SOURCES = test_for_file.c
test_for_file_LDADD = -L$(top_builddir)/lib -lkysec_file_restore
