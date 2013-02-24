
/* fizmo-sdl.c
 *
 * This file is part of fizmo.
 *
 * Copyright (c) 2011-2012 Christoph Ender.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <locale.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
//#include <X11/Xlib.h>
//#include <X11/Xutil.h>

#include <SDL.h>
//#include "SDL_getenv.h"

#include <tools/i18n.h>
#include <tools/tracelog.h>
#include <tools/z_ucs.h>
#include <tools/unused.h>
#include <tools/filesys.h>
#include <interpreter/fizmo.h>
#include <interpreter/streams.h>
#include <interpreter/config.h>
#include <interpreter/filelist.h>
#include <interpreter/wordwrap.h>
#include <interpreter/blorb.h>
#include <interpreter/savegame.h>
#include <interpreter/output.h>
#include <screen_interface/screen_pixel_interface.h>
#include <pixel_interface/pixel_interface.h>

#ifdef SOUND_INTERFACE_INCLUDE_FILE
#include SOUND_INTERFACE_INCLUDE_FILE
#endif /* SOUND_INTERFACE_INCLUDE_FILE */

#include "../locales/fizmo_sdl_locales.h"

#define FIZMO_SDL_VERSION "0.1.0"

#ifdef ENABLE_X11_IMAGES
#include <drilbo/drilbo.h>
#include <drilbo/drilbo-jpeg.h>
#include <drilbo/drilbo-png.h>
#include <drilbo/drilbo-x11.h>
#endif //ENABLE_X11_IMAGES

/*
#define SDL_WCHAR_T_BUF_SIZE 64
#define SDL_Z_UCS_BUF_SIZE 32
#define SDL_OUTPUT_CHAR_BUF_SIZE 80
*/

static char* interface_name = "sdl";
SDL_Surface* Surf_Display;
static z_colour screen_default_foreground_color = Z_COLOUR_BLACK;
static z_colour screen_default_background_color = Z_COLOUR_WHITE;
static int sdl_interface_screen_height_in_pixels = 800;
static int sdl_interface_screen_width_in_pixels = 640;
/*
static int sdl_argc;
static char **sdl_argv;
static bool dont_update_story_list_on_start = false;
static bool directory_was_searched = false;
static WORDWRAP *infowin_output_wordwrapper;
static WINDOW *infowin;
static z_ucs *infowin_more, *infowin_back;
static int infowin_height, infowin_width;
static int infowin_topindex;
static int infowin_lines_skipped;
static int infowin_skip_x;
static bool infowin_full = false;
static wchar_t wchar_t_buf[SDL_WCHAR_T_BUF_SIZE];
static z_ucs z_ucs_t_buf[SDL_Z_UCS_BUF_SIZE];
static char output_char_buf[SDL_OUTPUT_CHAR_BUF_SIZE];
static bool sdl_interface_open = false;

static int n_color_pairs_in_use;
static int n_color_pairs_available;
static bool color_initialized = false;
// This array contains (n_color_pairs_in_use) elements. The first element
// contains the number of the color pair the was selected last, the
// second element the color pair used before that. The array is used in
// case the Z-Code tries to use more color pairs than the terminal can
// provide. In such a case, the color pair that has not been used for
// a longer time than all others is recycled.
static short *color_pair_usage;

static attr_t sdl_no_attrs = 0;
static wchar_t sdl_setcchar_init_string[2];
static bool dont_allocate_new_colour_pair = false;

// "max_nof_color_pairs"  will be equal to story->max_nof_color_pairs once
// the interface is initialized. Up to then it will contain -1 or COLOR_PAIRS-1
// if the story menu is in use.
static int max_nof_color_pairs = -1;

static bool timer_active = true;

#ifdef ENABLE_X11_IMAGES
static z_image *frontispiece = NULL;
static bool enable_x11_graphics = true;
static bool enable_x11_inline_graphics = false;
static x11_image_window_id drilbo_window_id = -1;
static int x11_signalling_pipe[2];
unsigned int x11_read_buf[1];
fd_set x11_in_fds;
#endif // ENABLE_X11_IMAGES

static char *config_option_names[] = {
  "enable-xterm-title",
  "disable-x11-graphics",
  "display-x11-inline-image",
  "dont-update-story-list",
  NULL
};
*/

static z_colour colorname_to_infocomcode(char *colorname)
{
  if      (strcmp(colorname, "black") == 0)
    return Z_COLOUR_BLACK;
  else if (strcmp(colorname, "red") == 0)
    return Z_COLOUR_RED;
  else if (strcmp(colorname, "green") == 0)
    return Z_COLOUR_GREEN;
  else if (strcmp(colorname, "yellow") == 0)
    return Z_COLOUR_YELLOW;
  else if (strcmp(colorname, "blue") == 0)
    return Z_COLOUR_BLUE;
  else if (strcmp(colorname, "magenta") == 0)
    return Z_COLOUR_MAGENTA;
  else if (strcmp(colorname, "cyan") == 0)
    return Z_COLOUR_CYAN;
  else if (strcmp(colorname, "white") == 0)
    return Z_COLOUR_WHITE;
  else
    return -1;
}


static Uint32 z_to_sdl_colour(z_colour z_colour_to_convert)
{
  if (z_colour_to_convert == Z_COLOUR_BLACK) {
    return SDL_MapRGB(Surf_Display->format, 0, 0, 0);
  }
  else if (z_colour_to_convert == Z_COLOUR_RED)    {
    return SDL_MapRGB(Surf_Display->format, 255, 0, 0);
  }
  else if (z_colour_to_convert == Z_COLOUR_GREEN) {
    return SDL_MapRGB(Surf_Display->format, 0, 255, 0);
  }
  else if (z_colour_to_convert == Z_COLOUR_YELLOW) {
    return SDL_MapRGB(Surf_Display->format, 255, 255, 0);
  }
  else if (z_colour_to_convert == Z_COLOUR_BLUE) {
    return SDL_MapRGB(Surf_Display->format, 0, 0, 255);
  }
  else if (z_colour_to_convert == Z_COLOUR_MAGENTA) {
    return SDL_MapRGB(Surf_Display->format, 255, 0, 255);
  }
  else if (z_colour_to_convert == Z_COLOUR_CYAN) {
    return SDL_MapRGB(Surf_Display->format, 0, 255, 255);
  }
  else if (z_colour_to_convert == Z_COLOUR_WHITE) {
    return SDL_MapRGB(Surf_Display->format, 255, 255, 255);
  }
  else {
    TRACE_LOG("Invalid color.");
    exit(-2);
  }
}

/*
static z_ucs *z_ucs_string_to_wchar_t(wchar_t *dest, z_ucs *src,
    size_t max_dest_len)
{
  if (max_dest_len < 2)
  {
    return NULL;
  }

  while (*src != 0)
  {
    if (max_dest_len == 1)
    {
      *dest = L'\0';
      return src;
    }

    *dest = (wchar_t)*src;

    dest++;
    src++;
    max_dest_len--;
  }

  *dest = L'\0';
  return NULL;
}
*/


/*
static void infowin_z_ucs_output_wordwrap_destination(z_ucs *z_ucs_output,
    void *UNUSED(dummyparameter))
{
  z_ucs *ptr;
  int y;
#ifdef __GNUC__
  int __attribute__ ((unused)) x;
#else
  int x;
#endif // __GNU_CC
#ifdef ENABLE_TRACING
  z_ucs buf;
#endif // ENABLE_TRACING

  if (infowin_full == true)
    return;

  // In case we're supposed to skip some story description in case the user
  // has scrolled down, loop here until we're at the "infowin_topindex", at the
  // point we're supposed to start output.
  TRACE_LOG("line length: %d\n", infowin_width - infowin_skip_x);
  if (infowin_lines_skipped < infowin_topindex)
  {
    while (
        ((ptr = z_ucs_chr(z_ucs_output, Z_UCS_NEWLINE)) != NULL)
        ||
        ((long)z_ucs_len(z_ucs_output) >= infowin_width - infowin_skip_x)
        )
    {
      TRACE_LOG("loop: ptr:%p, len:%d.\n", ptr, (long)z_ucs_len(z_ucs_output));
      if (ptr != NULL)
      {
#ifdef ENABLE_TRACING
        buf = *ptr;
        *ptr = 0;
        TRACE_LOG("skip infowin output: \"");
        TRACE_LOG_Z_UCS(z_ucs_output);
        TRACE_LOG("\"\n");
        *ptr = buf;
#endif // ENABLE_TRACING

        TRACE_LOG("diff: %d, %d\n",
            ptr - z_ucs_output, infowin_width - infowin_skip_x);
        if (ptr - z_ucs_output > infowin_width - infowin_skip_x)
          z_ucs_output += infowin_width - infowin_skip_x;
        else
          z_ucs_output = ptr + 1;
      }
      else
      {
        TRACE_LOG("overlong line, skipping %d chars.\n",
            infowin_width - infowin_skip_x);
        z_ucs_output += (infowin_width - infowin_skip_x);
      }

      if (++infowin_lines_skipped == infowin_topindex)
      {
        TRACE_LOG("break after: infowin_lines_skipped: %d\n", infowin_lines_skipped);
        break;
      }
    }

    infowin_skip_x = z_ucs_len(z_ucs_output);

    TRACE_LOG("infowin_lines_skipped: %d\n", infowin_lines_skipped);
    if (infowin_lines_skipped < infowin_topindex)
      return;
  }

  // At this point we've skipped all required lines. Skip any newline we come
  // accross before starting the true output.

  // Display output. Output will be either the supplied "z_ucs_output", or the
  // "infowin_more" message in case we're at the bottom of the infowin page.

  // Determine if we're at the bottom and adjust output contents in this case.
  getyx(infowin, y, x);
  if (y == infowin_height - 1)
  {
    TRACE_LOG("infowin full.\n");
    infowin_full = true;
    waddstr(infowin, "[");
    z_ucs_output = infowin_more;
  }

  // Display output.
  while (z_ucs_output != NULL)
  {
    TRACE_LOG("infowin output: \"");
    TRACE_LOG_Z_UCS(z_ucs_output);
    TRACE_LOG("\"\n");
    z_ucs_output = z_ucs_string_to_wchar_t(
        wchar_t_buf,
        z_ucs_output,
        SDL_WCHAR_T_BUF_SIZE);

    // Ignore errors, since output on the last line always causes
    // ERR to be returned.
    waddwstr(infowin, wchar_t_buf);
  }

  // Add closing bracket in case we're displaying the "infowin_more" message.
  if (infowin_full == true)
    waddstr(infowin, "]");
}
*/


/*
#ifdef ENABLE_TRACING
static z_colour curses_to_z_colour(short curses_color)
{
  switch (curses_color)
  {
    case COLOR_BLACK:   return Z_COLOUR_BLACK;
    case COLOR_RED:     return Z_COLOUR_RED;
    case COLOR_GREEN:   return Z_COLOUR_GREEN;
    case COLOR_YELLOW:  return Z_COLOUR_YELLOW;
    case COLOR_BLUE:    return Z_COLOUR_BLUE;
    case COLOR_MAGENTA: return Z_COLOUR_MAGENTA;
    case COLOR_CYAN:    return Z_COLOUR_CYAN;
    case COLOR_WHITE:   return Z_COLOUR_WHITE;
  }

  return -1;
}
#endif // ENABLE_TRACING
*/


static void draw_grayscale_pixel(int y, int x, uint8_t pixel_value)
{
  Uint32 color = SDL_MapRGB(Surf_Display->format,
      pixel_value, pixel_value, pixel_value);

  /*
  printf("%d, %d, %d, %d\n",
      x, y, pixel_value, Surf_Display->format->BytesPerPixel);
  */

  if ( SDL_MUSTLOCK(Surf_Display) ) {
    if ( SDL_LockSurface(Surf_Display) < 0 ) {
      return;
    }
  }

  switch (Surf_Display->format->BytesPerPixel) {
    case 1: { /* Assuming 8-bpp */
              Uint8 *bufp;

              bufp = (Uint8 *)Surf_Display->pixels + y*Surf_Display->pitch + x;
              *bufp = color;
            }
            break;

    case 2: { /* Probably 15-bpp or 16-bpp */
              Uint16 *bufp;

              bufp = (Uint16 *)Surf_Display->pixels
                + y*Surf_Display->pitch/2 + x;
              *bufp = color;
            }
            break;

    case 3: { /* Slow 24-bpp mode, usually not used */
              Uint8 *bufp;

              bufp = (Uint8 *)Surf_Display->pixels + y*Surf_Display->pitch + x;
              *(bufp+Surf_Display->format->Rshift/8) = pixel_value;
              *(bufp+Surf_Display->format->Gshift/8) = pixel_value;
              *(bufp+Surf_Display->format->Bshift/8) = pixel_value;
            }
            break;

    case 4: { /* Probably 32-bpp */
              Uint32 *bufp;

              bufp = (Uint32 *)Surf_Display->pixels
                + y*Surf_Display->pitch/4 + x;
              *bufp = color;
            }
            break;
  }
  if ( SDL_MUSTLOCK(Surf_Display) ) {
    SDL_UnlockSurface(Surf_Display);
  }
}


static bool is_input_timeout_available()
{
  return true;
}


static char* get_interface_name()
{
  return interface_name;
}


static bool is_colour_available()
{
  //return has_colors();
  return true;
}


static bool is_bold_face_available()
{
  return true;
}


static bool is_italic_available()
{
  return true;
}


static void print_startup_syntax()
{
  int i;
  char **available_locales = get_available_locale_names();

  streams_latin1_output("\n");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_USAGE_DESCRIPTION);
  streams_latin1_output("\n\n");

  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_FIZMO_SDL_VERSION_P0S, FIZMO_SDL_VERSION);
  streams_latin1_output("\n");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_LIBFIZMO_VERSION_P0S,
      FIZMO_VERSION);
  streams_latin1_output("\n");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_LIBPIXELINTERFACE_VERSION_P0S,
      get_screen_pixel_interface_version());
  streams_latin1_output("\n");
  if (active_sound_interface != NULL)
  {
    streams_latin1_output(active_sound_interface->get_interface_name());
    streams_latin1_output(" ");
    streams_latin1_output("version ");
    streams_latin1_output(active_sound_interface->get_interface_version());
    streams_latin1_output(".\n");
  }
  streams_latin1_output("\n");

  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_LOCALES_AVAILIABLE);
  streams_latin1_output(" ");

  i = 0;
  while (available_locales[i] != NULL)
  {
    if (i != 0)
      streams_latin1_output(", ");

    streams_latin1_output(available_locales[i]);
    free(available_locales[i]);
    i++;
  }
  free(available_locales);
  streams_latin1_output(".\n");

  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_LOCALE_SEARCH_PATH);
  streams_latin1_output(": ");
  streams_latin1_output(
      get_i18n_default_search_path());
  streams_latin1_output(".\n");

  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_COLORS_AVAILABLE);
  streams_latin1_output(": ");

  for (i=Z_COLOUR_BLACK; i<=Z_COLOUR_WHITE; i++)
  {
    if (i != Z_COLOUR_BLACK)
      streams_latin1_output(", ");
    streams_latin1_output(z_colour_names[i]);
  }
  streams_latin1_output(".\n\n");

  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_VALID_OPTIONS_ARE);
  streams_latin1_output("\n");

  streams_latin1_output( " -l,  --set-locale: ");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_SET_LOCALE_NAME_FOR_INTERPRETER_MESSAGES);
  streams_latin1_output("\n");

  streams_latin1_output( " -pr, --predictable: ");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_START_WITH_RANDOM_GENERATOR_IN_PREDICTABLE_MODE);
  streams_latin1_output("\n");

  streams_latin1_output( " -ra, --random: ");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_START_WITH_RANDOM_GENERATOR_IN_RANDOM_MODE);
  streams_latin1_output("\n");

  streams_latin1_output( " -st, --start-transcript: ");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_START_GAME_WITH_TRANSCRIPT_ENABLED);
  streams_latin1_output("\n");

  streams_latin1_output( " -tf, --transcript-filename: ");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_SET_TRANSCRIPT_FILENAME);
  streams_latin1_output("\n");

  streams_latin1_output( " -rc, --record-commands: ");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_START_GAME_WITH_RECORDING_COMMANDS);
  streams_latin1_output("\n");

  streams_latin1_output( " -fi, --start-file-input: ");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_START_GAME_WITH_INPUT_FROM_FILE);
  streams_latin1_output("\n");

  streams_latin1_output( " -if, --input-filename: ");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_FILENAME_TO_READ_COMMANDS_FROM);
  streams_latin1_output("\n");

  streams_latin1_output( " -rf, --record-filename: ");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_FILENAME_TO_RECORD_INPUT_TO);
  streams_latin1_output("\n");

  streams_latin1_output( " -f,  --foreground-color: ");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_SET_FOREGROUND_COLOR);
  streams_latin1_output("\n");

  streams_latin1_output( " -b,  --background-color: ");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_SET_BACKGROUND_COLOR);
  streams_latin1_output("\n");

  streams_latin1_output( " -nc, --dont-use-colors: ");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_DONT_USE_COLORS);
  streams_latin1_output("\n");

  streams_latin1_output( " -ec, --enable-colors: ");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_ENABLE_COLORS);
  streams_latin1_output("\n");

  streams_latin1_output( " -lm, --left-margin: " );
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_SET_LEFT_MARGIN_SIZE);
  streams_latin1_output("\n");

  streams_latin1_output( " -rm, --right-margin: " );
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_SET_RIGHT_MARGIN_SIZE);
  streams_latin1_output("\n");

  streams_latin1_output( " -um, --umem: ");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_USE_UMEM_FOR_SAVEGAMES);
  streams_latin1_output("\n");

  streams_latin1_output( " -dh, --disable-hyphenation: ");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_DISABLE_HYPHENATION);
  streams_latin1_output("\n");

  streams_latin1_output( " -ds, --disable-sound: ");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_DISABLE_SOUND);
  streams_latin1_output("\n");

  streams_latin1_output( " -t,  --set-tandy-flag: ");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_SET_TANDY_FLAG);
  streams_latin1_output("\n");

  streams_latin1_output( " -sy, --sync-transcript: ");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_SYNC_TRANSCRIPT);
  streams_latin1_output("\n");

  streams_latin1_output( " -h,  --help: ");
  i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_SHOW_HELP_MESSAGE_AND_EXIT);
  streams_latin1_output("\n");

  //set_configuration_value("locale", fizmo_locale, "fizmo");

  streams_latin1_output("\n");
}


static int parse_config_parameter(char *UNUSED(key), char *UNUSED(value))
{
  return -2;

  /*
  if (strcmp(key, "enable-xterm-title") == 0)
  {
    if (
        (value == NULL)
        ||
        (*value == 0)
        ||
        (strcmp(value, config_true_value) == 0)
       )
      use_xterm_title = true;
    else
      use_xterm_title = false;
    free(value);
    return 0;
  }
  else if (strcmp(key, "disable-x11-graphics") == 0)
  {
#ifdef ENABLE_X11_IMAGES
    if (
        (value == NULL)
        ||
        (*value == 0)
        ||
        (strcmp(value, config_true_value) == 0)
       )
      enable_x11_graphics = false;
    else
      enable_x11_graphics = true;
#endif // ENABLE_X11_IMAGES
    free(value);
    return 0;
  }
  else if (strcmp(key, "display-x11-inline-image") == 0)
  {
#ifdef ENABLE_X11_IMAGES
    if (
        (value == NULL)
        ||
        (*value == 0)
        ||
        (strcmp(value, config_true_value) == 0)
       )
      enable_x11_inline_graphics = true;
    else
      enable_x11_inline_graphics = false;
#endif // ENABLE_X11_IMAGES
    free(value);
    return 0;
  }
  else if (strcmp(key, "dont-update-story-list") == 0)
  {
    if (
        (value == NULL)
        ||
        (*value == 0)
        ||
        (strcmp(value, config_true_value) == 0)
       )
      dont_update_story_list_on_start = true;
    free(value);
    return 0;
  }
  else
  {
    return -2;
  }
  */
}


static char *get_config_value(char *UNUSED(key))
{
  return NULL;

  /*
  if (strcmp(key, "enable-xterm-title") == 0)
  {
    return use_xterm_title == true
      ? config_true_value
      : config_false_value;
  }
  else if (strcmp(key, "disable-x11-graphics") == 0)
  {
#ifdef ENABLE_X11_IMAGES
    return enable_x11_graphics == false
      ? config_true_value
      : config_false_value;
#endif // ENABLE_X11_IMAGES
    return config_true_value;
  }
  else if (strcmp(key, "display-x11-inline-image") == 0)
  {
#ifdef ENABLE_X11_IMAGES
    return enable_x11_inline_graphics == true
      ? config_true_value
      : config_false_value;
#endif // ENABLE_X11_IMAGES
    return config_false_value;
  }
  else if (strcmp(key, "dont-update-story-list") == 0)
  {
    return dont_update_story_list_on_start == true
      ? config_true_value
      : config_false_value;
  }
  else
  {
    return NULL;
  }
  */
}


static char **get_config_option_names()
{
  //return config_option_names;
  return NULL;
}


/*
static int z_to_curses_colour(z_colour z_colour_to_convert)
{
  switch (z_colour_to_convert)
  {
    case Z_COLOUR_BLACK:   return COLOR_BLACK;
    case Z_COLOUR_RED:     return COLOR_RED;
    case Z_COLOUR_GREEN:   return COLOR_GREEN;
    case Z_COLOUR_YELLOW:  return COLOR_YELLOW;
    case Z_COLOUR_BLUE:    return COLOR_BLUE;
    case Z_COLOUR_MAGENTA: return COLOR_MAGENTA;
    case Z_COLOUR_CYAN:    return COLOR_CYAN;
    case Z_COLOUR_WHITE:   return COLOR_WHITE;
    default:               return -1;
  }
}
*/


/*
#ifdef ENABLE_TRACING
static void dump_col_usage()
{
  int j;
  int pair;
  short pair_foreground, pair_background;

  if (n_color_pairs_available < max_nof_color_pairs)
    for (j=0; j<n_color_pairs_in_use; j++)
    {
      pair = color_pair_usage[j];
      pair_content(pair, &pair_foreground, &pair_background);
      TRACE_LOG("col-usage[%02d]: %d, %d/%d\n", j, pair,
          curses_to_z_colour(pair_foreground),
          curses_to_z_colour(pair_background));
    }
}
#endif // ENABLE_TRACING
*/


/*
// curses provides a constant named COLOR_PAIRS telling us the maximum
// allowed number of color pairs on this terminal. This may be less pairs
// than the great implementor whose program we're interpreting desired,
// so we'll use the available pairs in a round-about fashion. Thus we
// can keep the latest defined color pairs as long as possible and
// hopefully the screen in the best possible look.
static short get_color_pair(z_colour z_foreground_colour,
    z_colour z_background_colour)
{
  short curses_foreground_color, curses_background_color;
  short pair_foreground, pair_background;
  short new_color_pair_number;
  int i,j;

  TRACE_LOG("Looking for infocom color pair %d / %d.\n",
      z_foreground_colour, z_background_colour);

  // Convert color codes from infocom to curses.
  curses_foreground_color = z_to_curses_colour(z_foreground_colour);
  curses_background_color = z_to_curses_colour(z_background_colour);

  TRACE_LOG("Looking for color pair %d / %d.\n",
      curses_foreground_color, curses_background_color);

  TRACE_LOG("n_color pairs in use: %d.\n", n_color_pairs_in_use);
  // First, check if we already have allocated this combination. We'll
  // start with index 1 since pair #0 is used for curses' internals.
  for (i=1; i<=n_color_pairs_in_use; i++)
  {
    pair_content(i, &pair_foreground, &pair_background);
    TRACE_LOG("Color pair %d: %d / %d.\n",i,pair_foreground, pair_background);

    if (
        (pair_foreground == curses_foreground_color)
        &&
        (pair_background == curses_background_color)
       )
    {
      TRACE_LOG("Found existing color pair with index %d.\n", i);

      if (n_color_pairs_available != max_nof_color_pairs)
      {
        // In case we're working with a limited number of colors we'll
        // have to update the color_pair_usage array. We'll put the index
        // of the color pair we've just selected to the front of the array
        // to notify that this is the latest used pair.

        // We only have to do something in case the pair is not already
        // in front.
        if (color_pair_usage[0] != i)
        {
          // First, advance j until we find i's index.
          for (j=0; j<n_color_pairs_in_use; j++)
            if (color_pair_usage[j] == i)
              break;

          TRACE_LOG("Found color pair is at usage position %d.\n", j);

          // Now, we'll move backwards, moving the array one index
          // "downwards", thus overwriting i's entry and making space
          // for it on top of the array again.
          for (; j>=0; j--)
            color_pair_usage[j] = color_pair_usage[j-1];

          color_pair_usage[0] = i;
        }

#ifdef ENABLE_TRACING
        dump_col_usage();
#endif // ENABLE_TRACING
      }

      return i;
    }
  }

  TRACE_LOG("No existing color pair found.\n");

  // In case we arrive here we have not returned and thus the desired
  // color pair was not found.

  if (bool_equal(dont_allocate_new_colour_pair, true))
    return -1;

  if (n_color_pairs_in_use < n_color_pairs_available)
  {
    new_color_pair_number = n_color_pairs_in_use + 1;
    TRACE_LOG("Allocating new color pair %d.\n", new_color_pair_number);
    n_color_pairs_in_use++;
    if (n_color_pairs_available != max_nof_color_pairs)
    {
      memmove(&(color_pair_usage[1]), color_pair_usage,
          (n_color_pairs_in_use-1) * sizeof(short));
      color_pair_usage[0] = new_color_pair_number;
    }
  }
  else
  {
    new_color_pair_number = color_pair_usage[n_color_pairs_in_use-1];
    memmove(&(color_pair_usage[1]), color_pair_usage,
        (n_color_pairs_in_use) * sizeof(short));
    color_pair_usage[0] = new_color_pair_number;

    TRACE_LOG("Recycling oldest color pair %d.\n", new_color_pair_number);
  }

  TRACE_LOG("initpair: %d, %d, %d\n",
      new_color_pair_number,
      curses_foreground_color,
      curses_background_color);

  if (init_pair(
        new_color_pair_number,
        curses_foreground_color,
        curses_background_color) == ERR)
    i18n_translate_and_exit(
        fizmo_sdl_module_name,
        i18n_sdl_FUNCTION_CALL_P0S_ABORTED_DUE_TO_ERROR,
        -0x2000,
        "init_pair");

  TRACE_LOG("n_color pairs in use: %d.\n", n_color_pairs_in_use);

#ifdef ENABLE_TRACING
    dump_col_usage();
#endif // ENABLE_TRACING

  return new_color_pair_number;
}
*/


/*
static void initialize_colors()
{
  start_color();

  // After implementing almost everything and assuming that the color pair
  // #0 always has the default colors (and running into strange problems)
  // I found the following note: "Note that color-pair 0 is reserved for
  // use by curses and should not be changed or used in application programs."
  // Thus, color pair 0 is not used here and the number of availiable colors
  // is set to COLOR_PAIRS - 1.
  //n_color_pairs_available = COLOR_PAIRS;
  n_color_pairs_available = COLOR_PAIRS - 1;

  // In case there has been no story initialization yet, max_nof_color_pairs
  // is still -1.
  if (max_nof_color_pairs == -1)
    max_nof_color_pairs = n_color_pairs_available;
  else if (n_color_pairs_available > max_nof_color_pairs)
    n_color_pairs_available = max_nof_color_pairs;

  TRACE_LOG("%d color pairs are availiable.\n", n_color_pairs_available);

  if (n_color_pairs_available < max_nof_color_pairs)
  {
    // In case not all color combinations are available, we'll have to
    // keep track when the colors were used last.
    color_pair_usage
      = (short*)fizmo_malloc(sizeof(short) * n_color_pairs_available);
    color_pair_usage[0] = 0;
  }

  n_color_pairs_in_use = 0;
  color_initialized = true;
}
*/


/*
#ifdef ENABLE_X11_IMAGES
static void x11_callback_func(x11_image_window_id window_id, int event)
{
  int ret_val;
  unsigned char write_buffer = 0;

  // No tracelogs from other thread: Might crash application.
  //TRACE_LOG("Got X11 callback event %d for image id %d.\n", event, window_id);

  if (drilbo_window_id == window_id)
  {
    if (event == DRILBO_IMAGE_WINDOW_CLOSED)
    {
      do
      {
        ret_val = write(x11_signalling_pipe[1], &write_buffer, 1);

        if ( (ret_val == -1) && (errno != EAGAIN) )
          return;
      }
      while ( (ret_val == -1) && (errno == EAGAIN) );
    }
  }
}
*/


/*
static void setup_x11_callback()
{
  int flags;

  if (pipe(x11_signalling_pipe) != 0)
    i18n_translate_and_exit(
        fizmo_sdl_module_name,
        i18n_sdl_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
        -0x2016,
        "pipe",
        errno,
        strerror(errno));

  if ((flags = fcntl(x11_signalling_pipe[0], F_GETFL, 0)) == -1)
    i18n_translate_and_exit(
        fizmo_sdl_module_name,
        i18n_sdl_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
        -0x2018,
        "fcntl / F_GETFL",
        errno,
        strerror(errno));

  if ((fcntl(x11_signalling_pipe[0], F_SETFL, flags|O_NONBLOCK)) == -1)
    i18n_translate_and_exit(
        fizmo_sdl_module_name,
        i18n_sdl_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
        -0x2018,
        "fcntl / F_SETFL",
        errno,
        strerror(errno));

  if ((flags = fcntl(STDIN_FILENO, F_GETFL, 0)) == -1)
    i18n_translate_and_exit(
        fizmo_sdl_module_name,
        i18n_sdl_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
        -0x2018,
        "fcntl / F_GETFL",
        errno,
        strerror(errno));

  if ((fcntl(STDIN_FILENO, F_SETFL, flags|O_NONBLOCK)) == -1)
    i18n_translate_and_exit(
        fizmo_sdl_module_name,
        i18n_sdl_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
        -0x2018,
        "fcntl / F_SETFL",
        errno,
        strerror(errno));
}
*/


/*
int wait_for_x11_callback()
{
  int max_filedes_number_plus_1;
  int select_retval, ret_val;

  for (;;)
  {
    FD_ZERO(&x11_in_fds);
    FD_SET(STDIN_FILENO, &x11_in_fds);
    FD_SET(x11_signalling_pipe[0], &x11_in_fds);

    max_filedes_number_plus_1
      = (STDIN_FILENO < x11_signalling_pipe[0]
          ? x11_signalling_pipe[0]
          : STDIN_FILENO) + 1;

    select_retval
      = select(max_filedes_number_plus_1, &x11_in_fds, NULL, NULL, NULL);

    if (select_retval > 0)
    {
      if (FD_ISSET(STDIN_FILENO, &x11_in_fds))
      {
        do
        {
          ret_val = read(STDIN_FILENO, &x11_read_buf, 1);
        }
        while (ret_val > 0);

        break;
      }
      else if (FD_ISSET(x11_signalling_pipe[0], &x11_in_fds))
      {
        do
        {
          ret_val = read(x11_signalling_pipe[0], &x11_read_buf, 1);

          if ( (ret_val == -1) && (errno != EAGAIN) )
          {
            printf("ret_val:%d\n", ret_val);
            return -1;
          }
        }
        while ( (ret_val == -1) && (errno == EAGAIN) );

        break;
      }
    }
  }

  return 0;
}
*/


/*
static int display_X11_image_window(int image_no)
{
  //long image_blorb_index;
  char *env_window_id;
  XID window_id;

  if ((frontispiece = get_blorb_image(image_no)) == NULL)
    return -1;

  env_window_id = getenv("WINDOWID");
  if ( (env_window_id != NULL) && (enable_x11_inline_graphics == true) )
  {
    curs_set(0);
    window_id = atol(env_window_id);
    setup_x11_callback();
    drilbo_window_id = display_zimage_on_X11(
        &window_id, frontispiece, &x11_callback_func);
    wait_for_x11_callback();
    close_image_window(drilbo_window_id);
    drilbo_window_id = -1;
    curs_set(1);
  }
  else
  {     
    drilbo_window_id = display_zimage_on_X11(NULL, frontispiece, NULL);
  }

  return 0;
}
#endif // ENABLE_X11_IMAGES
*/


static void link_interface_to_story(struct z_story *story)
{
  SDL_FillRect(
      Surf_Display,
      &Surf_Display->clip_rect,
      z_to_sdl_colour(screen_default_background_color));

  SDL_WM_SetCaption(story->title, story->title);

  /*
  int flags;
  int frontispiece_resource_number;

  initscr();
  keypad(stdscr, TRUE);
  cbreak();
  noecho();

  // Create a new signalling pipe. This pipe is used by a select call to
  // detect an incoming time-signal for the input routine.
  if (pipe(sdl_if_signalling_pipe) != 0)
    i18n_translate_and_exit(
        fizmo_sdl_module_name,
        i18n_sdl_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
        -0x2016,
        "pipe",
        errno,
        strerror(errno));

  // Get the current flags for the read-end of the pipe.
  if ((flags = fcntl(sdl_if_signalling_pipe[0], F_GETFL, 0)) == -1)
    i18n_translate_and_exit(
        fizmo_sdl_module_name,
        i18n_sdl_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
        -0x2017,
        "fcntl / F_GETFL",
        errno,
        strerror(errno));

  // Add the nonblocking flag the read-end of the pipe, thus making incoming
  // input "visible" at once without having to wait for a newline.
  if ((fcntl(sdl_if_signalling_pipe[0], F_SETFL, flags|O_NONBLOCK)) == -1)
    i18n_translate_and_exit(
        fizmo_sdl_module_name,
        i18n_sdl_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
        -0x2018,
        "fcntl / F_SETFL",
        errno,
        strerror(errno));

  max_nof_color_pairs = story->max_nof_color_pairs;
  color_initialized = false;

  sdl_interface_open = true;

  if (
      (active_z_story->title != NULL)
      &&
      (use_xterm_title == true)
     )
    printf("%c]0;%s%c", 033, active_z_story->title, 007);

#ifdef ENABLE_X11_IMAGES
  frontispiece_resource_number
    = active_blorb_interface->get_frontispiece_resource_number(
        active_z_story->blorb_map);

  if ( (frontispiece_resource_number >= 0) && (enable_x11_graphics != false) )
  {
    TRACE_LOG("frontispiece resnum: %d.\n", frontispiece_resource_number)
    display_X11_image_window(frontispiece_resource_number);
  }
#endif // ENABLE_X11_IMAGES
  */
}


static void reset_interface()
{
}


static int sdl_close_interface(z_ucs *UNUSED(error_message))
{
  return 0;
/*
  z_ucs *ptr;

#ifdef ENABLE_X11_IMAGES
  if (drilbo_window_id != -1)
  {
    close_image_window(drilbo_window_id);
    drilbo_window_id = -1;
  }
  if (frontispiece != NULL)
  {
    free_zimage(frontispiece);
    frontispiece = NULL;
  }
#endif // ENABLE_X11_IMAGES

  TRACE_LOG("Closing signalling pipes.\n");

  close(sdl_if_signalling_pipe[1]);
  close(sdl_if_signalling_pipe[0]);

  endwin();

  sdl_interface_open = false;

  if (error_message != NULL)
  {
    ptr = error_message;
    while (ptr != NULL)
    {
      ptr = z_ucs_string_to_wchar_t(
          wchar_t_buf,
          ptr,
          SDL_WCHAR_T_BUF_SIZE);

      sdl_fputws(wchar_t_buf, stderr);
    }
  }

  if (use_xterm_title == true)
    printf("%c]0;%c", 033, 007);

  return 0;
  */
}


/*
static attr_t sdl_z_style_to_attr_t(int16_t style_data)
{
  attr_t result = A_NORMAL;

  if ((style_data & Z_STYLE_REVERSE_VIDEO) != 0)
  {
    result |= A_REVERSE;
  }

  if ((style_data & Z_STYLE_BOLD) != 0)
  {
    result |= A_BOLD;
  }

  if ((style_data & Z_STYLE_ITALIC) != 0)
  {
    result |= A_UNDERLINE;
  }

  return result;
}
*/


static void output_interface_info()
{
  /*
  (void)i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_FIZMO_SDL_VERSION_P0S,
      FIZMO_SDL_VERSION);
  (void)streams_latin1_output("\n");
#ifdef ENABLE_X11_IMAGES
  (void)i18n_translate(
      fizmo_sdl_module_name,
      i18n_sdl_LIBDRILBO_VERSION_P0S,
      get_drilbo_version());
  (void)streams_latin1_output("\n");
#endif //ENABLE_X11_IMAGES
  */
}


/*
static void refresh_screen_size()
{
  getmaxyx(
      stdscr,
      sdl_interface_screen_height,
      sdl_interface_screen_width);
}
*/


static int get_screen_width_in_pixels()
{
  return sdl_interface_screen_width_in_pixels;
}


static int get_screen_height_in_pixels()
{
  return sdl_interface_screen_height_in_pixels;
}


/*
static void sdl_if_catch_signal(int sig_num)
{
  int bytes_written = 0;
  int ret_val;
  int sdl_if_write_buffer;

  // Note:
  // I think TRACE_LOGs in this function may cause a deadlock in case
  // they're called while a fflush for the tracelog is already underway.

  sdl_if_write_buffer = sig_num;

  //TRACE_LOG("Caught signal %d.\n", sig_num);

  while ((size_t)bytes_written < sizeof(int))
  {
    ret_val = write(
        sdl_if_signalling_pipe[1],
        &sdl_if_write_buffer,
        sizeof(int));

    if (ret_val == -1)
      i18n_translate_and_exit(
          fizmo_sdl_module_name,
          i18n_sdl_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
          -0x2023,
          "write",
          errno,
          strerror(errno));

    bytes_written += ret_val;
  }

  //TRACE_LOG("Catch finished.\n");
}
*/


static int get_next_event(z_ucs *z_ucs_input, int timeout_millis)
{
  bool running = true;
  SDL_Event Event;
  int wait_result, result = -1;

  if (timeout_millis > 0) {
    TRACE_LOG("input timeout: %d ms. (%d/%d)\n", timeout_millis,
        timeout_millis - (timeout_millis % 1000), 
        (timeout_millis % 1000) * 1000);
    //SDL_AddTimer -> SDL_PushEvent
  }

  printf("polling...\n");
  while ((running == true) && (wait_result = SDL_WaitEvent(&Event))) {
    if (Event.type == SDL_QUIT) {
      TRACE_LOG("quit\n");
      running = false;
    }
    else if (Event.type == SDL_KEYDOWN) {
      TRACE_LOG("keydown\n");
      printf("keydown.\n");
      if (Event.key.keysym.unicode == 127) {
        result = EVENT_WAS_CODE_BACKSPACE;
        running = false;
      }
      else {
        result = EVENT_WAS_INPUT;
        if (Event.key.keysym.unicode == 13) {
          *z_ucs_input = Z_UCS_NEWLINE;
        }
        else {
          *z_ucs_input = Event.key.keysym.unicode;
        }
        running = false;
      }
    }
    else if (Event.type == SDL_VIDEORESIZE) {
      // User requested screen resize.
      TRACE_LOG("resize\n");
      printf("resize\n");
    }
  }
  TRACE_LOG("return\n");
  printf("return\n");

  return result;

  /*
  int max_filedes_number_plus_1;
  int select_retval;
  fd_set input_selectors;
  int input_return_code;
  bool input_should_terminate = false;
  int bytes_read;
  wint_t input;
  int read_retval;
  int new_signal;
  //int screen_height, screen_width;

  FD_ZERO(&input_selectors);
  FD_SET(STDIN_FILENO, &input_selectors);
  FD_SET(sdl_if_signalling_pipe[0], &input_selectors);

  max_filedes_number_plus_1
    = (STDIN_FILENO < sdl_if_signalling_pipe[0]
        ? sdl_if_signalling_pipe[0]
        : STDIN_FILENO) + 1;

  if (timeout_millis > 0)
  {
    TRACE_LOG("input timeout: %d ms. (%d/%d)\n", timeout_millis,
        timeout_millis - (timeout_millis % 1000), 
        (timeout_millis % 1000) * 1000);
    timerval.it_value.tv_sec = timeout_millis - (timeout_millis % 1000);
    timerval.it_value.tv_usec = (timeout_millis % 1000) * 1000;
    timer_active = true;
    setitimer(ITIMER_REAL, &timerval, NULL);
  }

  while (input_should_terminate == false)
  {
    TRACE_LOG("current errno: %d.\n", errno);
    TRACE_LOG("setting up selectors.\n");

    FD_ZERO(&input_selectors);
    FD_SET(STDIN_FILENO, &input_selectors);
    FD_SET(sdl_if_signalling_pipe[0], &input_selectors);

    select_retval = select(
        max_filedes_number_plus_1,
        &input_selectors,
        NULL,
        NULL,
        NULL);

    if (select_retval > 0)
    {
      TRACE_LOG("select_retval > 0.\n");
      TRACE_LOG("current errno: %d.\n", errno);

      // something has changed in one of out input pipes.
      if (FD_ISSET(STDIN_FILENO, &input_selectors))
      {
        // some user input is waiting. we'll read until getch() returned
        // err, meaning "no more input availiable" in the nonblocking mode.
        input_return_code = get_wch(&input);
        if (input_return_code == ERR)
        {
        }
        else if (input_return_code == KEY_CODE_YES)
        {
          if (input == KEY_UP)
            result = EVENT_WAS_CODE_CURSOR_UP;
          else if (input == KEY_DOWN)
            result = EVENT_WAS_CODE_CURSOR_DOWN;
          else if (input == KEY_RIGHT)
            result = EVENT_WAS_CODE_CURSOR_RIGHT;
          else if (input == KEY_LEFT)
            result = EVENT_WAS_CODE_CURSOR_LEFT;
          else if (input == KEY_NPAGE)
            result = EVENT_WAS_CODE_PAGE_DOWN;
          else if (input == KEY_PPAGE)
            result = EVENT_WAS_CODE_PAGE_UP;
          else if (input == KEY_BACKSPACE)
            result = EVENT_WAS_CODE_BACKSPACE;
        }
        else if (input_return_code == OK)
        {
          if ( (input == 127) || (input == 8) )
            result = EVENT_WAS_CODE_BACKSPACE;
          else if (input == 1)
            result = EVENT_WAS_CODE_CTRL_A;
          else if (input == 5)
            result = EVENT_WAS_CODE_CTRL_E;
          else if (input == 27)
            result = EVENT_WAS_CODE_ESC;
          else
          {
            result = EVENT_WAS_INPUT;
            *z_ucs_input = (z_ucs)input;
          }
        }

        input_should_terminate = true;
      }
      else if (FD_ISSET(sdl_if_signalling_pipe[0], &input_selectors))
      {
        TRACE_LOG("current errno: %d.\n", errno);
        // the signal handler has written to our curses_if_signalling_pipe.
        // ensure that errno is != 0 before reading from the pipe. this is
        // due to the fact that even a successful read may set errno.

        TRACE_LOG("pipe event.\n");

        bytes_read = 0;

        while (bytes_read != sizeof(int))
        {
          read_retval = read(
              sdl_if_signalling_pipe[0],
              &new_signal,
              sizeof(int));

          if (read_retval == -1)
          {
            if (errno == EAGAIN)
            {
              errno = 0;
              continue;
            }
            else
            {
              i18n_translate_and_exit(
                  fizmo_sdl_module_name,
                  i18n_sdl_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
                  -0x2041,
                  "read",
                  errno,
                  strerror(errno));
            }
          }
          else
          {
            bytes_read += read_retval;
          }
        }

        TRACE_LOG("bytes read: %d,signal code: %d\n", bytes_read, new_signal);

        if (new_signal == SIGALRM)
        {
          if (timeout_millis > 0)
          {
            TRACE_LOG("Timeout.\n");
            result = EVENT_WAS_TIMEOUT;
            input_should_terminate = true;
          }
        }
        else if (new_signal == SIGWINCH)
        {
          TRACE_LOG("interface got SIGWINCH.\n");
          result = EVENT_WAS_WINCH;

          endwin();
          refresh();
          //getmaxyx(stdscr, screen_height, screen_width);
          refresh_screen_size();
          //TRACE_LOG("New dimensions: %dx%d.\n", screen_width, screen_height);
          //new_cell_screen_size(screen_height, screen_width);
          input_should_terminate = true;
        }
        else
        {
          i18n_translate_and_exit(
              fizmo_sdl_module_name,
              i18n_sdl_UNKNOWN_ERROR_CASE,
              -0x2041);
        }
      }
    }
    else
    {
      if (errno == EINTR)
        errno = 0;
      else
      {
        TRACE_LOG("select returned <=0, current errno: %d.\n", errno);
      }
    }
  }

  sigaction(SIGWINCH, NULL, NULL);

  TRACE_LOG("result %d.\n", result);

  return result;
  */
}


void update_screen()
{
  TRACE_LOG("Doing update_screen().\n");
  //SDL_UpdateRect(Surf_Display, 0, 0, 0, 0);
  SDL_Flip(Surf_Display);
}


void redraw_screen_from_scratch()
{
  /*
  redrawwin(stdscr);
  update_screen();
  */
}


void copy_area(int dsty, int dstx, int srcy, int srcx, int height, int width)
{
  int y;

  srcx -= 1;
  srcy -= 1;
  dstx -= 1;
  dsty -= 1;

  printf("copy-area: %d, %d to %d, %d: %d x %d.\n",
      srcx, srcy, dstx, dsty, width, height);

  if ( SDL_MUSTLOCK(Surf_Display) ) {
    if ( SDL_LockSurface(Surf_Display) < 0 ) {
      return;
    }
  }

  switch (Surf_Display->format->BytesPerPixel) {
    /*
    case 1: { // Assuming 8-bpp 
              Uint8 *srcp
                = (Uint8 *)Surf_Display->pixels
                + srcy*Surf_Display->pitch + srcx;
              Uint8 *dstp
                = (Uint8 *)Surf_Display->pixels
                + dsty*Surf_Display->pitch + dstx;
            }
            break;

    case 2: { // Probably 15-bpp or 16-bpp 
              Uint16 *srcp = (Uint16 *)Surf_Display->pixels
                + srcy*Surf_Display->pitch/2 + srcx;

              Uint16 *dstp = (Uint16 *)Surf_Display->pixels
                + dsty*Surf_Display->pitch/2 + dstx;
            }
            break;

    case 3: { // Slow 24-bpp mode, usually not used
              Uint8 *srcp = (Uint8 *)Surf_Display->pixels
                + srcy*Surf_Display->pitch + srcx;
              Uint8 *dstp = (Uint8 *)Surf_Display->pixels
                + dsty*Surf_Display->pitch + dstx;
            }
            break;
            */

    case 4: { /* Probably 32-bpp */
              Uint32 *srcp = (Uint32 *)Surf_Display->pixels
                + srcy*Surf_Display->pitch/4 + srcx;
              Uint32 *dstp = (Uint32 *)Surf_Display->pixels
                + dsty*Surf_Display->pitch/4 + dstx;

              for (y=0; y<height; y++) {
                memcpy(dstp, srcp, width*4);
                srcp += Surf_Display->pitch/4;
                dstp += Surf_Display->pitch/4;
              }
            }
            break;
  }
  if ( SDL_MUSTLOCK(Surf_Display) ) {
    SDL_UnlockSurface(Surf_Display);
  }
}


void clear_to_eol()
{
  //clrtoeol();
}


void fill_area(int startx, int starty, int xsize, int ysize, z_colour colour)
{
  int y, x;
  Uint32 sdl_colour = z_to_sdl_colour(colour);

  startx -= 1;
  starty -= 1;

  printf("Filling area %d,%d / %d,%d\n", startx, starty, xsize, ysize);

  if ( SDL_MUSTLOCK(Surf_Display) ) {
    if ( SDL_LockSurface(Surf_Display) < 0 ) {
      return;
    }
  }

  switch (Surf_Display->format->BytesPerPixel) {
    /*
    case 1: { // Assuming 8-bpp
              Uint8 *bufp = (Uint8 *)Surf_Display->pixels
                + starty*Surf_Display->pitch + startx;
            }
            break;

    case 2: { // Probably 15-bpp or 16-bpp
              Uint16 *srcp = (Uint16 *)Surf_Display->pixels
                + starty*Surf_Display->pitch/2 + startx;
            }
            break;

    case 3: { // Slow 24-bpp mode, usually not used
              Uint8 *srcp = (Uint8 *)Surf_Display->pixels
                + starty*Surf_Display->pitch + startx;
            }
            break;
            */

    case 4: { /* Probably 32-bpp */
              Uint32 *srcp;

              for (y=0; y<ysize; y++) {
                srcp = (Uint32 *)Surf_Display->pixels
                  + (starty+y)*Surf_Display->pitch/4 + startx;
                for (x=0; x<xsize; x++) {
                  //*srcp = (Uint32)color;
                  *srcp = sdl_colour;
                  srcp++;
                }
              }
            }
            break;
  }
  if ( SDL_MUSTLOCK(Surf_Display) ) {
    SDL_UnlockSurface(Surf_Display);
  }
}


static void set_cursor_visibility(bool UNUSED(visible))
{
  /*
  if (sdl_interface_open == true)
  {
    if (visible == true)
      curs_set(1);
    else
      curs_set(0);
  }
  */
}


static z_colour get_default_foreground_colour()
{
  return Z_COLOUR_WHITE;
}


static z_colour get_default_background_colour()
{
  return Z_COLOUR_BLACK;
}


static struct z_screen_pixel_interface sdl_interface =
{
  &draw_grayscale_pixel,
  &is_input_timeout_available,
  &get_next_event,
  &get_interface_name,
  &is_colour_available,
  &is_bold_face_available,
  &is_italic_available,
  &parse_config_parameter,
  &get_config_value,
  &get_config_option_names,
  &link_interface_to_story,
  &reset_interface,
  &sdl_close_interface,
  &output_interface_info,
  &get_screen_width_in_pixels,
  &get_screen_height_in_pixels,
  &update_screen,
  &redraw_screen_from_scratch,
  &copy_area,
  &clear_to_eol,
  &fill_area,
  &set_cursor_visibility,
  &get_default_foreground_colour,
  &get_default_background_colour
};


/*
static char *select_story_from_menu()
{
  //z_colour foreground, background;
  int input;
  int storywin_height = -1;
  int storywin_width = -1;
  int storywin_y = 3;
  int infowin_y = 3;
  int storywin_x = 3;
  int story_title_length = -1;
  struct z_story_list *story_list;
  int i, j;
  int y, x;
  bool input_done = false;
  int selected = 0;
  int len;
  int scroll_index = 0;
  struct z_story_list_entry *entry;
  char *result;
  char *src;
  fd_set input_selectors;
  int max_filedes_number_plus_1;
  int select_retval;
  bool perform_init = true;
  int read_retval;
  int bytes_read;
  int new_signal;
  z_ucs *ptr;
  attr_t attrs;
  short menucolorpair = -1;
  char *config_disablecolor;

#ifndef DISABLE_FILELIST
  story_list
    = dont_update_story_list_on_start != true
    ? update_fizmo_story_list()
    : get_z_story_list();

  if ( (story_list == NULL) || (story_list->nof_entries < 1) )
  {
    //set_configuration_value("locale", fizmo_locale, "sdl");
    streams_latin1_output("\n");
    i18n_translate(
        fizmo_sdl_module_name,
        i18n_sdl_NO_STORIES_REGISTERED_PLUS_HELP);
    streams_latin1_output("\n\n");
    //set_configuration_value("locale", fizmo_locale, "fizmo");
    return NULL;
  }
#endif // DISABLE_FILELIST

  sigemptyset(&default_sigaction.sa_mask);
  default_sigaction.sa_flags = 0;
  default_sigaction.sa_handler = &sdl_if_catch_signal;
  sigaction(SIGWINCH, &default_sigaction, NULL);

  infowin_output_wordwrapper = wordwrap_new_wrapper(
      80,
      &infowin_z_ucs_output_wordwrap_destination,
      (void*)NULL,
      false,
      0,
      false,
      true);

  infowin_more = i18n_translate_to_string(
      fizmo_sdl_module_name,
      i18n_sdl_SPACE_FOR_NEXT_PAGE);
  infowin_back = i18n_translate_to_string(
      fizmo_sdl_module_name,
      i18n_sdl_B_FOR_LAST_PAGE);

  while (input_done == false)
  {
    if (perform_init == true)
    {
      initscr();
      cbreak();
      noecho();
      keypad(stdscr, true);

      config_disablecolor = get_configuration_value("disable-color");
      if (
           (has_colors() == true)
           &&
           ( (config_disablecolor == NULL)
             || (strcmp(config_disablecolor, "true") != 0) )
         )
      {
        start_color();
        set_colour(default_foreground_colour, default_background_colour);
        attr_get(&attrs, &menucolorpair, NULL);
        TRACE_LOG("Current color pair %d.\n", menucolorpair);
        bkgdset(' ' | COLOR_PAIR(menucolorpair));
        bkgd(' ' | COLOR_PAIR(menucolorpair));
      }
      else
        menucolorpair = -1;

      getmaxyx(stdscr, y, x);
      storywin_height = y - 5;

      storywin_width = x / 3;
      story_title_length = storywin_width - 9;

      infowin_height = storywin_height;
      TRACE_LOG("infowin_height: %d.\n", infowin_height);
      infowin_width = x - storywin_width - storywin_x - 9;
      infowin_topindex = 0;

      infowin = subwin(
          stdscr,
          infowin_height + 1,
          infowin_width + 1,
          infowin_y,
          storywin_width + storywin_x + 5);

      wordwrap_adjust_line_length(
          infowin_output_wordwrapper,
          x - storywin_width - storywin_x - 9);

      perform_init = false;
    }

    erase();

    attrset(A_BOLD);
    mvprintw(1, storywin_x + 7,
        "fizmo-sdl Z-Machine interpreter, Version %s\n",
        FIZMO_SDL_VERSION);
    attrset(A_NORMAL);

    i = 0;
    while ( (i<storywin_height) && (i + scroll_index < story_list->nof_entries))
    {
      entry = story_list->entries[i + scroll_index];
      if (i + scroll_index == selected)
      {
        attrset(A_REVERSE);
        mvaddstr(storywin_y + i, storywin_x, "  ");
        printw("%3d  ", scroll_index + i + 1);
        len = (long)strlen(entry->title) > story_title_length
          ? story_title_length - 3 : story_title_length;
        addnstr(entry->title, len);
        if ((long)strlen(entry->title) > story_title_length)
          addstr("...");
        getyx(stdscr, y, x);
        j = storywin_x + storywin_width - x;
        while (j-- > 0)
          addstr(" ");
        attrset(A_NORMAL);
      }
      else
      {
        mvprintw(storywin_y + i, storywin_x + 2, "%3d  ", scroll_index + i + 1);
        len = (long)strlen(entry->title) > story_title_length
          ? story_title_length - 3 : story_title_length;
        addnstr(entry->title, len);
        if ((long)strlen(entry->title) > story_title_length)
          addstr("...");
      }

      i++;
    }

    if (menucolorpair != -1)
      wcolor_set(infowin, menucolorpair, NULL);
    werase(infowin);
    entry = story_list->entries[selected];

    wattrset(infowin, A_BOLD);
    wprintw(infowin, "%s\n", entry->title);
    wattrset(infowin, A_NORMAL);

    wprintw(infowin, "By: %s\n", entry->author);

    wprintw(infowin, "v%d / S:%s / R:%d / C:%d.\n", entry->z_code_version,
        entry->serial, entry->release_number, entry->checksum);

    if (infowin_topindex == 0)
      wprintw(infowin, "\n");
    else
    {
      waddstr(infowin, "[");
      ptr = infowin_back;
      while (ptr != NULL)
      {
        ptr = z_ucs_string_to_wchar_t(
            wchar_t_buf,
            ptr,
            SDL_WCHAR_T_BUF_SIZE);

        if (waddwstr(infowin, wchar_t_buf) == ERR)
          i18n_translate_and_exit(
              fizmo_sdl_module_name,
              i18n_sdl_FUNCTION_CALL_P0S_ABORTED_DUE_TO_ERROR,
              -1,
              "waddwstr");
      }
      waddstr(infowin, "]\n");
    }

    infowin_lines_skipped = 0;
    infowin_skip_x = 0;
    infowin_full = false;
    //wordwrap_set_line_index(infowin_output_wordwrapper, 0);
    src = entry->description;
    do
    {
      src = utf8_string_to_zucs_string(
          z_ucs_t_buf, src, SDL_Z_UCS_BUF_SIZE);
      wordwrap_wrap_z_ucs(infowin_output_wordwrapper, z_ucs_t_buf);
    }
    while (src != NULL);
    wordwrap_flush_output(infowin_output_wordwrapper);

    refresh();
    //wrefresh(infowin);

    max_filedes_number_plus_1
      = (STDIN_FILENO < sdl_if_signalling_pipe[0]
          ? sdl_if_signalling_pipe[0]
          : STDIN_FILENO) + 1;

    FD_ZERO(&input_selectors);
    FD_SET(STDIN_FILENO, &input_selectors);
    FD_SET(sdl_if_signalling_pipe[0], &input_selectors);

    select_retval = select(
        max_filedes_number_plus_1,
        &input_selectors,
        NULL,
        NULL,
        NULL);

    if (select_retval > 0)
    {
      TRACE_LOG("select_retval > 0.\n");

      // something has changed in one of out input pipes.
      if (FD_ISSET(STDIN_FILENO, &input_selectors))
      {
        input = getch();

        if (input == '\n')
        {
          result = fizmo_strdup(story_list->entries[selected]->filename);
          input_done = true;
        }
        else if (input == KEY_UP)
        {
          if (selected > 0)
            selected--;
          if (selected < scroll_index)
            scroll_index--; 
          infowin_topindex = 0;
        }
        else if (input == KEY_DOWN)
        {
          if (selected  + 1 < story_list->nof_entries)
            selected++;
          if (selected == storywin_height + scroll_index)
            scroll_index++; 
          infowin_topindex = 0;
        }
        else if (input == KEY_NPAGE)
        {
          scroll_index += storywin_height;
          selected += storywin_height;
          if (scroll_index >= story_list->nof_entries)
          {
            scroll_index = story_list->nof_entries - storywin_height;
            selected = story_list->nof_entries - 1;

            if (scroll_index < 0)
              scroll_index = 0;
          }
          infowin_topindex = 0;
        }
        else if (input == KEY_PPAGE)
        {
          scroll_index -= storywin_height;
          selected -= storywin_height;
          if (scroll_index < 0)
          {
            scroll_index = 0;
            selected = 0;
          }
        }
        else if (input == 27)
        {
          input_done = true;
          result = NULL;
        }
        else if ( (input == ' ') && (infowin_full == true) )
        {
          infowin_topindex += (infowin_height - 5);
          TRACE_LOG("New infowin_topindex: %d.\n", infowin_topindex);
        }
        else if (input == 'b')
        {
          if ((infowin_topindex -= (infowin_height - 5)) < 0)
            infowin_topindex = 0;
          TRACE_LOG("New infowin_topindex: %d.\n", infowin_topindex);
        }
      }
      else
      {
        bytes_read = 0;

        while (bytes_read != sizeof(int))
        {
          read_retval = read(
              sdl_if_signalling_pipe[0],
              &new_signal, 
              sizeof(int));

          if (read_retval == -1)
          {
            if (errno == EAGAIN)
            {
              errno = 0;
              continue;
            }
            else
            {
              i18n_translate_and_exit(
                  fizmo_sdl_module_name,
                  i18n_sdl_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
                  -0x2041,
                  "read",
                  errno,
                  strerror(errno));
            }
          }
          else
          {
            bytes_read += read_retval;
          }
        }

        if (new_signal == SIGWINCH)
        {
          //exit(1);
          perform_init = true;
          endwin();
        }
      }
    }
    else if (select_retval < 0)
    {
      if (errno == EINTR)
        errno = 0;
      else
        i18n_translate_and_exit(
            fizmo_sdl_module_name,
            i18n_sdl_ERROR_P0D_OCCURED_BEFORE_READ_P1S,
            -0x203c,
            errno,
            strerror(errno));
    }
  }

#ifndef DISABLE_FILELIST
  free_z_story_list(story_list);
#endif // DISABLE_FILELIST
  delwin(infowin);
  erase();
  move(0,0);
  refresh();

  sigaction(SIGWINCH, NULL, NULL);

  endwin();
  wordwrap_destroy_wrapper(infowin_output_wordwrapper);

  return result;
}
*/


/*
void catch_signal(int sig_num)
{
  int bytes_written = 0;
  int ret_val;
  int sdl_if_write_buffer;

  // Note:
  // Look like TRACE_LOGs in this function may cause a deadlock in case
  // they're called while a fflush for the tracelog is already underway.

  sdl_if_write_buffer = sig_num;

  //TRACE_LOG("Caught signal %d.\n", sig_num);

  while ((size_t)bytes_written < sizeof(int))
  {
    ret_val = write(
        sdl_if_signalling_pipe[1],
        &sdl_if_write_buffer,
        sizeof(int));

    if (ret_val == -1)
      i18n_translate_and_exit(
          fizmo_sdl_module_name,
          i18n_sdl_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
          -0x2023,
          "write",
          errno,
          strerror(errno));

    bytes_written += ret_val;
  }

  //TRACE_LOG("Catch finished.\n");
}
*/


int main(int argc, char *argv[])
{
  int argi = 1;
  int story_filename_parameter_number = -1;
  int blorb_filename_parameter_number = -1;
  char *input_file;
  z_file *story_stream = NULL, *blorb_stream = NULL;
  z_colour new_color;
  int int_value;
  z_file *savegame_to_restore= NULL;
  //Display *display;
  //Window window;

  /*
  int flags;
  char *cwd = NULL;
  char *absdirname = NULL;
#ifndef DISABLE_FILELIST
  char *story_to_load_filename, *assumed_filename;
  struct z_story_list *story_list;
  size_t absdirname_len = 0;
  int i;
#endif // DISABLE_FILELIST
  */
  

#ifdef ENABLE_TRACING
  turn_on_trace();
#endif // ENABLE_TRACING

  /*
  setlocale(LC_ALL, "C");
  setlocale(LC_CTYPE, "");

  fizmo_register_screen_cell_interface(&sdl_interface);

  sdl_argc = argc;
  sdl_argv = argv;

  while (argi < argc)
  {
    if ((strcmp(argv[argi], "-l") == 0)
        || (strcmp(argv[argi], "--set-locale") == 0))
    {
      if (++argi == argc)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      if (set_current_locale_name(argv[argi]) != 0)
      {
        streams_latin1_output("\n");

        i18n_translate(
            fizmo_sdl_module_name,
            i18n_sdl_INVALID_CONFIGURATION_VALUE_P0S_FOR_P1S,
            argv[argi],
            "locale");

        streams_latin1_output("\n");

        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      set_configuration_value("dont-set-locale-from-config", "true");
      argi++;
    }
    else if ((strcmp(argv[argi], "-pr") == 0)
        || (strcmp(argv[argi], "--predictable") == 0))
    {
      set_configuration_value("random-mode", "predictable");
      argi += 1;
    }
    else if ((strcmp(argv[argi], "-ra") == 0)
        || (strcmp(argv[argi], "--random") == 0))
    {
      set_configuration_value("random-mode", "random");
      argi += 1;
    }
    else if ((strcmp(argv[argi], "-st") == 0)
        || (strcmp(argv[argi], "--start-transcript") == 0))
    {
      set_configuration_value("start-script-when-story-starts", "true");
      argi += 1;
    }
    else if ((strcmp(argv[argi], "-rc") == 0)
        || (strcmp(argv[argi], "--start-recording-commands") == 0))
    {
      set_configuration_value(
          "start-command-recording-when-story-starts", "true");
      argi += 1;
    }
    else if ((strcmp(argv[argi], "-fi") == 0)
        || (strcmp(argv[argi], "--start-file-input") == 0))
    {
      set_configuration_value(
          "start-file-input-when-story-starts", "true");
      argi += 1;
    }
    else if ((strcmp(argv[argi], "-if") == 0)
        || (strcmp(argv[argi], "--input-filename") == 0))
    {
      if (++argi == argc)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }
      set_configuration_value(
          "input-command-filename", argv[argi]);
      argi += 1;
    }
    else if ((strcmp(argv[argi], "-rf") == 0)
        || (strcmp(argv[argi], "--record-filename") == 0))
    {
      if (++argi == argc)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }
      set_configuration_value(
          "record-command-filename", argv[argi]);
      argi += 1;
    }
    else if ((strcmp(argv[argi], "-tf") == 0)
        || (strcmp(argv[argi], "--transcript-filename") == 0))
    {
      if (++argi == argc)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }
      set_configuration_value(
          "transcript-filename", argv[argi]);
      argi += 1;
    }
    else if (
        (strcmp(argv[argi], "-b") == 0)
        || (strcmp(argv[argi], "--background-color") == 0) )
    {
      if (++argi == argc)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      if ((new_color = colorname_to_infocomcode(argv[argi])) == -1)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      screen_default_background_color = new_color;
      argi++;
    }
    else if (
        (strcmp(argv[argi], "-f") == 0)
        || (strcmp(argv[argi], "--foreground-color") == 0) )
    {
      if (++argi == argc)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      if ((new_color = colorname_to_infocomcode(argv[argi])) == -1)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      screen_default_foreground_color = new_color;
      argi++;
    }
    else if (
        (strcmp(argv[argi], "-um") == 0)
        ||
        (strcmp(argv[argi], "--umem") == 0)
        )
    {
      set_configuration_value("quetzal-umem", "true");
      argi ++;
    }
    else if (
        (strcmp(argv[argi], "-dh") == 0)
        ||
        (strcmp(argv[argi], "--disable-hyphenation") == 0)
        )
    {
      set_configuration_value("disable-hyphenation", "true");
      argi ++;
    }
    else if (
        (strcmp(argv[argi], "-nc") == 0)
        || (strcmp(argv[argi], "--dont-use-colors: ") == 0) )
    {
      set_configuration_value("disable-color", "true");
      argi ++;
    }
    else if (
        (strcmp(argv[argi], "-ec") == 0)
        || (strcmp(argv[argi], "--enable-colors: ") == 0) )
    {
      set_configuration_value("enable-color", "true");
      argi ++;
    }
#ifdef ENABLE_X11_IMAGES
    else if (
        (strcmp(argv[argi], "-nx") == 0)
        ||
        (strcmp(argv[argi], "--disable-x11-graphics") == 0)
        )
    {
      enable_x11_graphics = false;
      argi++;
    }
    else if (
        (strcmp(argv[argi], "-xi") == 0)
        ||
        (strcmp(argv[argi], "--enable-x11-inline-graphics") == 0)
        )
    {
      enable_x11_inline_graphics = true;
      argi++;
    }
    else if (
        (strcmp(argv[argi], "-xt") == 0)
        ||
        (strcmp(argv[argi], "--enable-xterm-title") == 0)
        )
    {
      use_xterm_title = true;
      argi++;
    }
#endif
    else if (
        (strcmp(argv[argi], "-ds") == 0)
        ||
        (strcmp(argv[argi], "--disable-sound") == 0))
    {
      set_configuration_value("disable-sound", "true");
      argi += 1;
    }
    else if (
        (strcmp(argv[argi], "-t") == 0)
        ||
        (strcmp(argv[argi], "--set-tandy-flag") == 0))
    {
      set_configuration_value("set-tandy-flag", "true");
      argi += 1;
    }
    else if (
        (strcmp(argv[argi], "-lm") == 0)
        ||
        (strcmp(argv[argi], "-rm") == 0)
        ||
        (strcmp(argv[argi], "--left-margin") == 0)
        ||
        (strcmp(argv[argi], "--right-margin") == 0)
        )
    {
      if (++argi == argc)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      int_value = atoi(argv[argi]);

      if (
          ( (int_value == 0) && (strcmp(argv[argi], "0") != 0) )
          ||
          (int_value < 0)
         )
      {
        i18n_translate(
            fizmo_sdl_module_name,
            i18n_sdl_INVALID_CONFIGURATION_VALUE_P0S_FOR_P1S,
            argv[argi],
            argv[argi - 1]);

        streams_latin1_output("\n");

        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      if (
          (strcmp(argv[argi - 1], "-lm") == 0)
          ||
          (strcmp(argv[argi - 1], "--left-margin") == 0)
         )
        set_custom_left_cell_margin(int_value);
      else
        set_custom_right_cell_margin(int_value);

      argi += 1;
    }
    else if (
        (strcmp(argv[argi], "-h") == 0)
        ||
        (strcmp(argv[argi], "--help") == 0)
        )
    {
      print_startup_syntax();
      exit(0);
    }
    else if (
        (strcmp(argv[argi], "-nu") == 0)
        ||
        (strcmp(argv[argi], "--dont-update-story-list") == 0))
    {
      dont_update_story_list_on_start = true;
      argi += 1;
    }
#ifndef DISABLE_FILELIST
    else if (
        (strcmp(argv[argi], "-u") == 0)
        ||
        (strcmp(argv[argi], "--update-story-list") == 0))
    {
      printf("\n");
      update_fizmo_story_list();
      printf("\n");
      directory_was_searched = true;
      argi += 1;
    }
    else if (
        (strcmp(argv[argi], "-s") == 0)
        ||
        (strcmp(argv[argi], "--search") == 0))
    {
      if (++argi == argc)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      if (strlen(argv[argi]) > 0)
      {
        if (argv[argi][0] == '/')
          search_directory(argv[argi], false);
        else
        {
          if (cwd == NULL)
            cwd = fsi->get_cwd();

          if (absdirname_len < strlen(cwd) + strlen(argv[argi]) + 2)
          {
            absdirname_len = strlen(cwd) + strlen(argv[argi]) + 2;
            absdirname = fizmo_realloc(absdirname, absdirname_len);
          }
          sprintf(absdirname ,"%s/%s", cwd, argv[argi]);
          search_directory(absdirname, false);
        }

        printf("\n");
        directory_was_searched = true;
      }

      argi += 1;
    }
    else if (
        (strcmp(argv[argi], "-rs") == 0)
        ||
        (strcmp(argv[argi], "--recursively-search") == 0))
    {
      if (++argi == argc)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      if (strlen(argv[argi]) > 0)
      {
        if (argv[argi][0] == '/')
          search_directory(argv[argi], true);
        else
        {
          if (cwd == NULL)
            cwd = fsi->get_cwd();

          if (absdirname_len < strlen(cwd) + strlen(argv[argi]) + 2)
          {
            absdirname_len = strlen(cwd) + strlen(argv[argi]) + 2;
            absdirname = fizmo_realloc(absdirname, absdirname_len);
          }
          sprintf(absdirname ,"%s/%s", cwd, argv[argi]);
          search_directory(absdirname, true);
        }

        printf("\n");
        directory_was_searched = true;
      }

      argi += 1;
    }
#endif // DISABLE_FILELIST
    else if (
        (strcmp(argv[argi], "-sy") == 0)
        ||
        (strcmp(argv[argi], "--sync-transcript") == 0))
    {
      set_configuration_value("sync-transcript", "true");
      argi += 1;
    }
    else if (story_filename_parameter_number == -1)
    {
      story_filename_parameter_number = argi;
      argi++;
    }
    else if (blorb_filename_parameter_number == -1)
    {
      blorb_filename_parameter_number = argi;
      argi++;
    }
    else
    {
      // Unknown parameter:
      print_startup_syntax();
      exit(EXIT_FAILURE);
    }
  }

  if (cwd != NULL)
    free(cwd);

  if (absdirname != NULL)
    free(absdirname);

  if (directory_was_searched == true)
    exit(EXIT_SUCCESS);

  if (story_filename_parameter_number == -1)
  {
#ifndef DISABLE_FILELIST
    if ((input_file = select_story_from_menu()) == NULL)
      return 0;
    story_stream = fsi->openfile(
        input_file, FILETYPE_DATA, FILEACCESS_READ);
#endif // DISABLE_FILELIST
  }
  else
  {
    // The user has given some filename or description name on the command line.
    input_file = argv[story_filename_parameter_number];

    // First, test if this may be the filename for a savegame.
#ifndef DISABLE_FILELIST
    // We can only restore saved games directly from the save file in
    // case the filelist functionality exists.
    if (detect_saved_game(input_file, &story_to_load_filename) == true)
    {
      // User provided a savegame name on the command line.
      savegame_to_restore = fsi->openfile(
          input_file, FILETYPE_DATA, FILEACCESS_READ);
      story_stream = fsi->openfile(
          story_to_load_filename, FILETYPE_DATA, FILEACCESS_READ);
    }
    else
    {
#endif // DISABLE_FILELIST
      // Check if parameter is a valid filename.
      if ((story_stream = fsi->openfile(
              input_file, FILETYPE_DATA, FILEACCESS_READ)) == NULL)
      {
#ifndef DISABLE_FILELIST
        // In case it's not a regular file, use the filelist to find
        // something similiar.
        story_list = get_z_story_list();
        for (i=0; i<story_list->nof_entries; i++)
        {
          if (strcasecmp(story_list->entries[i]->title, input_file) == 0)
          {
            assumed_filename = fizmo_strdup(story_list->entries[i]->filename);
            story_stream = fsi->openfile(
                assumed_filename, FILETYPE_DATA, FILEACCESS_READ);
            break;
          }
        }
        free_z_story_list(story_list);
#endif // DISABLE_FILELIST
      }
#ifndef DISABLE_FILELIST
    }
#endif // DISABLE_FILELIST
  }

  if (story_stream == NULL)
  {
    i18n_translate_and_exit(
        fizmo_sdl_module_name,
        i18n_sdl_COULD_NOT_OPEN_OR_FIND_P0S,
        -0x2016,
        input_file);
    exit(EXIT_FAILURE);
  }

  timerval.it_interval.tv_sec = 0;
  timerval.it_interval.tv_usec = 0;

  empty_timerval.it_interval.tv_sec = 0;
  empty_timerval.it_interval.tv_usec = 0;
  empty_timerval.it_value.tv_sec = 0;
  empty_timerval.it_value.tv_usec = 0;

  // Create a new signalling pipe. This pipe is used by a select call to
  // detect an incoming time-signal for the input routine.
  if (pipe(sdl_if_signalling_pipe) != 0)
    i18n_translate_and_exit(
        fizmo_sdl_module_name,
        i18n_sdl_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
        -0x2016,
        "pipe",
        errno,
        strerror(errno));

  // Get the current flags for the read-end of the pipe.
  if ((flags = fcntl(sdl_if_signalling_pipe[0], F_GETFL, 0)) == -1)
    i18n_translate_and_exit(
        fizmo_sdl_module_name,
        i18n_sdl_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
        -0x2017,
        "fcntl / F_GETFL",
        errno,
        strerror(errno));

  // Add the nonblocking flag the read-end of the pipe, thus making incoming
  // input "visible" at once without having to wait for a newline.
  if ((fcntl(sdl_if_signalling_pipe[0], F_SETFL, flags|O_NONBLOCK)) == -1)
    i18n_translate_and_exit(
        fizmo_sdl_module_name,
        i18n_sdl_FUNCTION_CALL_P0S_RETURNED_ERROR_P1D_P2S,
        -0x2018,
        "fcntl / F_SETFL",
        errno,
        strerror(errno));

  sigemptyset(&default_sigaction.sa_mask);
  default_sigaction.sa_flags = 0;
  default_sigaction.sa_handler = &catch_signal;
  sigaction(SIGALRM, &default_sigaction, NULL);

  sigemptyset(&default_sigaction.sa_mask);
  default_sigaction.sa_flags = 0;
  default_sigaction.sa_handler = &sdl_if_catch_signal;
  sigaction(SIGWINCH, &default_sigaction, NULL);

  if (blorb_filename_parameter_number != -1)
    blorb_stream = fsi->openfile(
        argv[blorb_filename_parameter_number], FILETYPE_DATA, FILEACCESS_READ);

  fizmo_start(
      story_stream,
      blorb_stream,
      savegame_to_restore,
      screen_default_foreground_color,
      screen_default_background_color);

  sigaction(SIGWINCH, NULL, NULL);

  if (story_filename_parameter_number == -1)
    free(input_file);

  TRACE_LOG("Closing signalling pipes.\n");

  close(sdl_if_signalling_pipe[1]);
  close(sdl_if_signalling_pipe[0]);
  */

  while (argi < argc)
  {
    if ((strcmp(argv[argi], "-l") == 0)
        || (strcmp(argv[argi], "--set-locale") == 0))
    {
      if (++argi == argc)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      if (set_current_locale_name(argv[argi]) != 0)
      {
        streams_latin1_output("\n");

        i18n_translate(
            fizmo_sdl_module_name,
            i18n_sdl_INVALID_CONFIGURATION_VALUE_P0S_FOR_P1S,
            argv[argi],
            "locale");

        streams_latin1_output("\n");

        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      set_configuration_value("dont-set-locale-from-config", "true");
      argi++;
    }
    else if ((strcmp(argv[argi], "-pr") == 0)
        || (strcmp(argv[argi], "--predictable") == 0))
    {
      set_configuration_value("random-mode", "predictable");
      argi += 1;
    }
    else if ((strcmp(argv[argi], "-ra") == 0)
        || (strcmp(argv[argi], "--random") == 0))
    {
      set_configuration_value("random-mode", "random");
      argi += 1;
    }
    else if ((strcmp(argv[argi], "-st") == 0)
        || (strcmp(argv[argi], "--start-transcript") == 0))
    {
      set_configuration_value("start-script-when-story-starts", "true");
      argi += 1;
    }
    else if ((strcmp(argv[argi], "-rc") == 0)
        || (strcmp(argv[argi], "--start-recording-commands") == 0))
    {
      set_configuration_value(
          "start-command-recording-when-story-starts", "true");
      argi += 1;
    }
    else if ((strcmp(argv[argi], "-fi") == 0)
        || (strcmp(argv[argi], "--start-file-input") == 0))
    {
      set_configuration_value(
          "start-file-input-when-story-starts", "true");
      argi += 1;
    }
    else if ((strcmp(argv[argi], "-if") == 0)
        || (strcmp(argv[argi], "--input-filename") == 0))
    {
      if (++argi == argc)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }
      set_configuration_value(
          "input-command-filename", argv[argi]);
      argi += 1;
    }
    else if ((strcmp(argv[argi], "-rf") == 0)
        || (strcmp(argv[argi], "--record-filename") == 0))
    {
      if (++argi == argc)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }
      set_configuration_value(
          "record-command-filename", argv[argi]);
      argi += 1;
    }
    else if ((strcmp(argv[argi], "-tf") == 0)
        || (strcmp(argv[argi], "--transcript-filename") == 0))
    {
      if (++argi == argc)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }
      set_configuration_value(
          "transcript-filename", argv[argi]);
      argi += 1;
    }
    else if (
        (strcmp(argv[argi], "-b") == 0)
        || (strcmp(argv[argi], "--background-color") == 0) )
    {
      if (++argi == argc)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      if ((new_color = colorname_to_infocomcode(argv[argi])) == -1)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      screen_default_background_color = new_color;
      argi++;
    }
    else if (
        (strcmp(argv[argi], "-f") == 0)
        || (strcmp(argv[argi], "--foreground-color") == 0) )
    {
      if (++argi == argc)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      if ((new_color = colorname_to_infocomcode(argv[argi])) == -1)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      screen_default_foreground_color = new_color;
      argi++;
    }
    else if (
        (strcmp(argv[argi], "-um") == 0)
        ||
        (strcmp(argv[argi], "--umem") == 0)
        )
    {
      set_configuration_value("quetzal-umem", "true");
      argi ++;
    }
    else if (
        (strcmp(argv[argi], "-dh") == 0)
        ||
        (strcmp(argv[argi], "--disable-hyphenation") == 0)
        )
    {
      set_configuration_value("disable-hyphenation", "true");
      argi ++;
    }
    else if (
        (strcmp(argv[argi], "-nc") == 0)
        || (strcmp(argv[argi], "--dont-use-colors: ") == 0) )
    {
      set_configuration_value("disable-color", "true");
      argi ++;
    }
    else if (
        (strcmp(argv[argi], "-ec") == 0)
        || (strcmp(argv[argi], "--enable-colors: ") == 0) )
    {
      set_configuration_value("enable-color", "true");
      argi ++;
    }
    else if (
        (strcmp(argv[argi], "-ds") == 0)
        ||
        (strcmp(argv[argi], "--disable-sound") == 0))
    {
      set_configuration_value("disable-sound", "true");
      argi += 1;
    }
    else if (
        (strcmp(argv[argi], "-t") == 0)
        ||
        (strcmp(argv[argi], "--set-tandy-flag") == 0))
    {
      set_configuration_value("set-tandy-flag", "true");
      argi += 1;
    }
    else if (
        (strcmp(argv[argi], "-lm") == 0)
        ||
        (strcmp(argv[argi], "-rm") == 0)
        ||
        (strcmp(argv[argi], "--left-margin") == 0)
        ||
        (strcmp(argv[argi], "--right-margin") == 0)
        )
    {
      if (++argi == argc)
      {
        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      int_value = atoi(argv[argi]);

      if (
          ( (int_value == 0) && (strcmp(argv[argi], "0") != 0) )
          ||
          (int_value < 0)
         )
      {
        i18n_translate(
            fizmo_sdl_module_name,
            i18n_sdl_INVALID_CONFIGURATION_VALUE_P0S_FOR_P1S,
            argv[argi],
            argv[argi - 1]);

        streams_latin1_output("\n");

        print_startup_syntax();
        exit(EXIT_FAILURE);
      }

      if (
          (strcmp(argv[argi - 1], "-lm") == 0)
          ||
          (strcmp(argv[argi - 1], "--left-margin") == 0)
         )
        set_custom_left_pixel_margin(int_value);
      else
        set_custom_right_pixel_margin(int_value);

      argi += 1;
    }
    else if (
        (strcmp(argv[argi], "-h") == 0)
        ||
        (strcmp(argv[argi], "--help") == 0)
        )
    {
      print_startup_syntax();
      exit(0);
    }
    else if (
        (strcmp(argv[argi], "-sy") == 0)
        ||
        (strcmp(argv[argi], "--sync-transcript") == 0))
    {
      set_configuration_value("sync-transcript", "true");
      argi += 1;
    }
    else if (story_filename_parameter_number == -1)
    {
      story_filename_parameter_number = argi;
      argi++;
    }
    else if (blorb_filename_parameter_number == -1)
    {
      blorb_filename_parameter_number = argi;
      argi++;
    }
    else
    {
      // Unknown parameter:
      print_startup_syntax();
      exit(EXIT_FAILURE);
    }
  }

  if (story_filename_parameter_number == -1)
  {
    return -1;
  }
  else
  {
    // The user has given some filename or description name on the command line.
    input_file = argv[story_filename_parameter_number];

      // Check if parameter is a valid filename.
    story_stream = fsi->openfile(
        input_file, FILETYPE_DATA, FILEACCESS_READ);
  }

  if (story_stream == NULL)
  {
    i18n_translate_and_exit(
        fizmo_sdl_module_name,
        i18n_sdl_COULD_NOT_OPEN_OR_FIND_P0S,
        -0x2016,
        input_file);
    exit(EXIT_FAILURE);
  }

  //SDL_putenv("SDL_VIDEODRIVER=Quartz");

  if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    i18n_translate_and_exit(
        fizmo_sdl_module_name,
        i18n_sdl_FUNCTION_CALL_P0S_ABORTED_DUE_TO_ERROR,
        -1,
        "SDL_Init");

  SDL_EnableUNICODE(1);

  if ((Surf_Display = SDL_SetVideoMode(
          sdl_interface_screen_width_in_pixels,
          sdl_interface_screen_height_in_pixels,
          32,
          SDL_SWSURFACE | SDL_ANYFORMAT | SDL_DOUBLEBUF | SDL_RESIZABLE
          //| SDL_NOFRAME
          )) == NULL)
    i18n_translate_and_exit(
        fizmo_sdl_module_name,
        i18n_sdl_FUNCTION_CALL_P0S_ABORTED_DUE_TO_ERROR,
        -1,
        "SDL_SetVideoMode");

  /*
  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);

  if(!SDL_GetWMInfo(&info)) {
    printf("SDL cant get from SDL\n");
  }

  if ( info.subsystem != SDL_SYSWM_X11 ) {
    printf("SDL is not running on X11\n");
  }

  display = info.info.x11.display;
  window = info.info.x11.window;
  */

  fizmo_register_screen_pixel_interface(&sdl_interface);

#ifdef SOUND_INTERFACE_STRUCT_NAME
  fizmo_register_sound_interface(&SOUND_INTERFACE_STRUCT_NAME);
#endif // SOUND_INTERFACE_STRUCT_NAME

  // Parsing must occur after "fizmo_register_screen_cell_interface" so
  // that fizmo knows where to forward "parse_config_parameter" parameters
  // to.
#ifndef DISABLE_CONFIGFILES
  parse_fizmo_config_files();
#endif // DISABLE_CONFIGFILES

  fizmo_start(
      story_stream,
      blorb_stream,
      savegame_to_restore,
      screen_default_foreground_color,
      screen_default_background_color);

  SDL_Quit();

  return 0;
}

