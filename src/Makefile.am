INCLUDES = \
	$(GLIB_CFLAGS) \
	$(EDJE_CFLAGS) \
	$(EVAS_CFLAGS) \
	$(ECORE_CFLAGS) \
	$(EFREET_CFLAGS) \
	$(ELEMENTARY_CFLAGS) \
	$(MOKOSUITE_CFLAGS) \
	-DLOCALEDIR=\""$(localedir)"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-Wall

bin_PROGRAMS = mokohome

mokohome_SOURCES = \
	main.c \
	desktop.c \
	desktop.h \
	launchers.c \
	launchers.h \
	widgets.c \
	widgets.h

mokohome_LDADD = \
	$(GLIB_LIBS) \
	$(EDJE_LIBS) \
	$(EVAS_LIBS) \
	$(ECORE_LIBS) \
	$(EFREET_LIBS) \
	$(ELEMENTARY_LIBS) \
	$(MOKOSUITE_LIBS)

MAINTAINERCLEANFILES = \
	Makefile.in
