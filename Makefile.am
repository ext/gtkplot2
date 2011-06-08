ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libgtkplot2.la
noinst_PROGRAMS = sample

libgtkplot2_la_CFLAGS = -Wall ${gtk_CFLAGS}
libgtkplot2_la_LDFLAGS = ${gtk_LDFLAGS}
libgtkplot2_la_SOURCES = src/plot2.c src/gtkplot.h

sample_CFLAGS = -Wall ${gtk_CFLAGS}
sample_LDFLAGS = ${gtk_LIBS}
sample_LDADD = libgtkplot2.la
sample_SOURCES = src/sample.c
