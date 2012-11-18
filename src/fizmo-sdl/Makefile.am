
AM_CFLAGS = $(libfizmo_CFLAGS) $(libpixelif_CFLAGS) $(sdl_CFLAGS)
AM_CPPFLAGS =
LDADD = $(libfizmo_LIBS) $(libpixelif_LIBS) $(sdl_LIBS)

bin_PROGRAMS = fizmo-sdl
fizmo_sdl_SOURCES = ../locales/fizmo_sdl_locales.c fizmo-sdl.c

if ENABLE_TRACING
AM_CFLAGS += -DENABLE_TRACING=
endif

