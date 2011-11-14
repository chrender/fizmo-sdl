
.PHONY : all install clean distclean

include config.mk


all: src/fizmo-sdl/fizmo-sdl

src/fizmo-sdl/fizmo-sdl::
	cd src/fizmo-sdl ; make

install: src/fizmo-sdl/fizmo-sdl
	mkdir -p $(bindir)
	cp src/fizmo-sdl/fizmo-sdl $(bindir)
	mkdir -p $(mandir)/man6
	cp src/man/fizmo-sdl.6 $(mandir)/man6
	mkdir -p $(localedir)
	for l in `cd src/locales ; ls -d ??_??`; \
	  do \
	  mkdir -p $(localedir)/$$l; \
	  cp src/locales/$$l/*.txt $(localedir)/$$l; \
	done

clean:
	cd src/fizmo-sdl ; make clean

distclean: clean
	cd src/fizmo-sdl ; make distclean

