AM_CFLAGS =  -Wall -I. -std=gnu99 -I../include -D_GNU_SOURCE

bin_PROGRAMS = file_restore_demo

file_restore_demo_SOURCES = file_backup.c file_restore.c main.c
file_restore_demo_LDADD =  -L$(top_builddir)/lib -lfile_restore
