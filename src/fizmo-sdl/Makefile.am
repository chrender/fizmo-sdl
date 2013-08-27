
AUTOMAKE_OPTIONS = subdir-objects

AM_CFLAGS = $(sdl_CFLAGS) $(libfizmo_CFLAGS) $(libpixelif_CFLAGS)
AM_CPPFLAGS =
LDADD = $(sdl_LIBS) $(libfizmo_LIBS) $(libpixelif_LIBS)

bin_PROGRAMS = fizmo-sdl
fizmo_sdl_SOURCES = ../locales/fizmo_sdl_locales.c fizmo-sdl.c

if ENABLE_TRACING
AM_CFLAGS += -DENABLE_TRACING=
endif

#if ENABLE_SDL_SOUND
#AM_CFLAGS += $(libsndifsdl_CFLAGS) \
# -DSOUND_INTERFACE_STRUCT_NAME="$(SOUND_INTERFACE_STRUCT_NAME)" \
# -DSOUND_INTERFACE_INCLUDE_FILE="<$(SOUND_INTERFACE_INCLUDE_FILE)>"
#LDADD += $(libsndifsdl_LIBS)
#endif

#if ENABLE_FILELIST
#else
#AM_CFLAGS += -DDISABLE_FILELIST=
#endif

#if ENABLE_CONFIG_FILES
#else
#AM_CFLAGS += -DDISABLE_CONFIGFILES=
#endif

