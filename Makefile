
.PHONY : all install clean distclean

include config.mk


all: src/fizmo-sdl/fizmo-sdl

src/fizmo-sdl/fizmo-sdl::
	cd src/fizmo-sdl ; make

install: src/fizmo-sdl/fizmo-sdl
	mkdir -p $(DESTDIR)$(bindir)
	cp src/fizmo-sdl/fizmo-sdl $(DESTDIR)$(bindir)
	mkdir -p $(DESTDIR)$(mandir)/man6
	cp src/man/fizmo-sdl.6 $(DESTDIR)$(mandir)/man6
	mkdir -p $(DESTDIR)$(localedir)
	for l in `cd src/locales ; ls -d ??_??`; \
	  do \
	  mkdir -p $(DESTDIR)$(localedir)/$$l; \
	  cp src/locales/$$l/*.txt $(DESTDIR)$(localedir)/$$l; \
	done

clean:
	cd src/fizmo-sdl ; make clean

distclean: clean
	cd src/fizmo-sdl ; make distclean

