
 [AS_HELP_STRING([--enable-tracing],
                 [enable tracelog for debugging])],
 [],
 [enable_tracing=no])

#AC_ARG_ENABLE([config-files],
# [AS_HELP_STRING([--disable-config-files],
#	 [disable parsing of configuration files])],
# [],
# [enable_config_files=yes])

AC_INIT([fizmo-sdl], [0.1.0])

AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_PROG_RANLIB

PKG_CHECK_MODULES([libfizmo], [libfizmo >= 0.7.5])
PKG_CHECK_MODULES([libpixelif], [libpixelif >= 0.1.0])
#PKG_CHECK_MODULES([libsndifsdl], [libsndifsdl >= 0.7.3])
#PKG_CHECK_MODULES([libdrilbo], [libdrilbo >= 0.2.3])
m4_include(config-libs.m4)

AM_CONDITIONAL([ENABLE_TRACING],
                [test "$enable_tracing" = "yes"])

m4_include(config-conds.m4)
m4_include(config-subst.m4)

#AC_SUBST([SOUND_INTERFACE_NAME], libsndifsdl)
#AC_SUBST([SOUND_INTERFACE_STRUCT_NAME], sound_interface_sdl)
#AC_SUBST([SOUND_INTERFACE_CONFIGNAME], SOUNDSDL)
#AC_SUBST([SOUND_INTERFACE_INCLUDE_FILE], sound_sdl/sound_sdl.h)

AC_CONFIG_FILES([Makefile src/fizmo-sdl/Makefile])
AC_OUTPUT

