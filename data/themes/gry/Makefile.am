MAINTAINERCLEANFILES = Makefile.in

filesdir = $(datadir)/mokosuite/home/themes
files_DATA = gry.edj

EDC_DEPS = \
	tray_collapse.png \
	tray_expand.png \
	tray_handle_normal.png \
	tray_handle_pressed.png

EXTRA_DIST = $(files_DATA) $(EDC_DEPS) theme.edc

clean-local:
	rm -f $(files_DATA)

gry.edj: theme.edc $(EDC_DEPS)
	@edje_cc@ -v $< $@
