AM_CFLAGS =  -Wall -I. -std=gnu99 -I../include -D_GNU_SOURCE

bin_PROGRAMS = test_for_sync 

test_for_sync_SOURCES = test_for_sync.c
test_for_sync_LDADD = -L$(top_builddir)/lib -lfile_restore
