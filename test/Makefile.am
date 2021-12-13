AM_CFLAGS =  -Wall -I. -std=gnu99 -I../include -D_GNU_SOURCE

bin_PROGRAMS = test_for_sync test_for_utils test_for_attr test_for_job

test_for_sync_SOURCES = test_for_sync.c
test_for_sync_LDADD = -L$(top_builddir)/lib -lfile_restore

test_for_utils_SOURCES = test_for_utils.c
test_for_utils_LDADD = -L$(top_builddir)/lib -lfile_restore

test_for_attr_SOURCES = test_for_attr.c
test_for_attr_LDADD = -L$(top_builddir)/lib -lfile_restore

test_for_job_SOURCES = test_for_job.c
test_for_job_LDADD = -L$(top_builddir)/lib -lfile_restore
