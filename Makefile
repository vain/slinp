# Variables for DESTDIR and friends.

INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
datarootdir = $(prefix)/share
mandir = $(datarootdir)/man
man1dir = $(mandir)/man1

# ---

.PHONY: all
.PHONY: clean
.PHONY: install
.PHONY: installdirs

all: prescontrol/prescontrol showpdf/showpdf slidenotes/slidenotes \
	stopclock/stopclock

showpdf/showpdf: showpdf/showpdf.c showpdf/showpdf.h
	$(CC) -Wall -Wextra $(CFLAGS) \
		-o $@ $< \
		`pkg-config --cflags --libs gtk+-3.0 poppler-glib`

stopclock/stopclock: stopclock/stopclock.c stopclock/stopclock.h
	$(CC) -Wall -Wextra $(CFLAGS) \
		-o $@ $< \
		`pkg-config --cflags --libs gtk+-3.0`

clean:
	rm -f showpdf/showpdf
	rm -f stopclock/stopclock

install: all installdirs
	$(INSTALL_PROGRAM) prescontrol/prescontrol $(DESTDIR)$(bindir)/prescontrol
	$(INSTALL_PROGRAM) showpdf/showpdf $(DESTDIR)$(bindir)/showpdf
	$(INSTALL_PROGRAM) slidenotes/slidenotes $(DESTDIR)$(bindir)/slidenotes
	$(INSTALL_PROGRAM) stopclock/stopclock $(DESTDIR)$(bindir)/stopclock
	$(INSTALL_DATA) prescontrol/man1/prescontrol.1 $(DESTDIR)$(man1dir)/prescontrol.1
	$(INSTALL_DATA) showpdf/man1/showpdf.1 $(DESTDIR)$(man1dir)/showpdf.1
	$(INSTALL_DATA) slidenotes/man1/slidenotes.1 $(DESTDIR)$(man1dir)/slidenotes.1
	$(INSTALL_DATA) stopclock/man1/stopclock.1 $(DESTDIR)$(man1dir)/stopclock.1
	$(INSTALL_DATA) man1/ppres.1 $(DESTDIR)$(man1dir)/ppres.1

installdirs:
	@# We only support GNU right now, but FreeBSD and OpenBSD understand
	@# "mkdir -p" as well.
	mkdir -p $(DESTDIR)$(bindir) $(DESTDIR)$(man1dir)
