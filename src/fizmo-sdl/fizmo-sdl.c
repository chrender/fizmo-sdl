
/* fizmo-sdl.c
 *
 * This file is part of fizmo.
 *
 * Copyright (c) 2011-2014 Christoph Ender.
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

#include <SDL.h>
//#include "SDL_getenv.h"
#include <SDL_thread.h>

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

#define SDL_OUTPUT_CHAR_BUF_SIZE 80
/*
#define SDL_WCHAR_T_BUF_SIZE 64
#define SDL_Z_UCS_BUF_SIZE 32
*/

static char* interface_name = "sdl";
SDL_Surface* Surf_Display;
static z_colour screen_default_foreground_color = Z_COLOUR_BLACK;
static z_colour screen_default_background_color = Z_COLOUR_WHITE;
static int sdl_interface_screen_height_in_pixels = 800;
static int sdl_interface_screen_width_in_pixels = 600;
static const int sdl_color_depth = 32;
static const int sdl_video_flags = SDL_SWSURFACE | SDL_ANYFORMAT
| SDL_DOUBLEBUF | SDL_RESIZABLE;
static SDL_TimerID timeout_timer;
static bool timeout_timer_exists;
static SDL_sem *timeout_semaphore;
static char output_char_buf[SDL_OUTPUT_CHAR_BUF_SIZE];
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
static void _PutPixelAlpha(SDL_Surface *surface, Sint16 x, Sint16 y,
    Uint32 color, Uint8 alpha) { 

  if(x>=SPG_clip_xmin(surface) && x<=SPG_clip_xmax(surface) 
      && y>=SPG_clip_ymin(surface) && 
      y<=SPG_clip_ymax(surface)){ 
    Uint32 Rmask = 
      surface->format->Rmask, Gmask = surface->format->Gmask, 
    Bmask = surface->format->Bmask, Amask = 
      surface->format->Amask; 
    Uint32 R,G,B,A=SDL_ALPHA_OPAQUE; 
    Uint32* pixel; 

    switch (surface->format->BytesPerPixel) { 
      case 1: { 

                Uint8 *pixel = (Uint8 *)surface->pixels + y*surface->pitch + x; 

                Uint8 dR = surface->format->palette->colors[*pixel].r; 
                Uint8 dG = surface->format->palette->colors[*pixel].g; 
                Uint8 dB = surface->format->palette->colors[*pixel].b; 
                Uint8 sR = surface->format->palette->colors[color].r; 
                Uint8 sG = surface->format->palette->colors[color].g; 
                Uint8 sB = surface->format->palette->colors[color].b; 

                dR = dR + ((sR-dR)*alpha >> 8); 
                dG = dG + ((sG-dG)*alpha >> 8); 
                dB = dB + ((sB-dB)*alpha >> 8); 

                *pixel = SDL_MapRGB(surface->format, dR, dG, dB); 

              } 
              break; 

      case 2: {

                Uint16 *pixel = (Uint16 *)surface->pixels + y*surface->pitch/2 + x; 
                Uint32 dc = *pixel; 

                R = ((dc & Rmask) + (( (color & Rmask) - (dc & Rmask) ) * alpha >> 8)) & Rmask; 
                G = ((dc & Gmask) + (( (color & Gmask) - (dc & Gmask) ) * alpha >> 8)) & Gmask; 
                B = ((dc & Bmask) + (( (color & Bmask) - (dc & Bmask) ) * alpha >>8) ) & Bmask; 
                if( Amask ) 
                  A = ((dc & Amask) + (( (color & Amask) - (dc & Amask) ) * alpha >>8) ) & Amask; 

                *pixel= R | G | B | A; 

              } 
              break; 

      case 3: { 
                Uint8 *pix = (Uint8 *)surface->pixels + y * surface->pitch + x*3; 
                Uint8 rshift8=surface->format->Rshift/8; 
                Uint8 gshift8=surface->format->Gshift/8; 
                Uint8 bshift8=surface->format->Bshift/8; 
                Uint8 ashift8=surface->format->Ashift/8; 



                Uint8 dR, dG, dB, dA=0; 
                Uint8 sR, sG, sB, sA=0; 

                pix = (Uint8 *)surface->pixels + y * surface->pitch + x*3; 

                dR = *((pix)+rshift8); 
                dG = *((pix)+gshift8); 
                dB = *((pix)+bshift8); 
                dA = *((pix)+ashift8); 

                sR = (color>>surface->format->Rshift)&0xff; 
                sG = (color>>surface->format->Gshift)&0xff; 
                sB = (color>>surface->format->Bshift)&0xff; 
                sA = (color>>surface->format->Ashift)&0xff; 

                dR = dR + ((sR-dR)*alpha >> 8); 
                dG = dG + ((sG-dG)*alpha >> 8); 
                dB = dB + ((sB-dB)*alpha >> 8); 
                dA = dA + ((sA-dA)*alpha >> 8); 

                *((pix)+rshift8) = dR; 
                *((pix)+gshift8) = dG; 
                *((pix)+bshift8) = dB; 
                *((pix)+ashift8) = dA; 

              } 
              break; 

      case 4: 
              pixel = (Uint32*)surface->pixels + y*surface->pitch/4 + x; 
              Uint32 dc = *pixel; 
              R = color & Rmask; 
              G = color & Gmask; 
              B = color & Bmask; 
              A = 0; // keep this as 0 to avoid corruption of non-alpha surfaces 


              switch(SPG_GetBlend()) 
              { 
                case SPG_COMBINE_ALPHA: // Blend and combine src and dest alpha 
                  if( alpha != SDL_ALPHA_OPAQUE ){ 
                    R = ((dc & Rmask) + (( R - (dc & Rmask) ) * alpha >> 8)) & Rmask; 
                    G = ((dc & Gmask) + (( G - (dc & Gmask) ) * alpha >> 8)) & Gmask; 
                    B = ((dc & Bmask) + (( B - (dc & Bmask) ) * alpha >> 8)) & Bmask; 
                  } 
                  if(Amask) 

                    A = ((((dc & Amask) >> surface->format->Ashift) + 
                          alpha) >> 1) << surface->format->Ashift; 
                  break; 
                case SPG_DEST_ALPHA: // Blend and keep dest alpha 
                  if( alpha != SDL_ALPHA_OPAQUE ){ 
                    R = ((dc & Rmask) + (( R - (dc & Rmask) ) * alpha >> 8)) & Rmask; 
                    G = ((dc & Gmask) + (( G - (dc & Gmask) ) * alpha >> 8)) & Gmask; 
                    B = ((dc & Bmask) + (( B - (dc & Bmask) ) * alpha >> 8)) & Bmask; 
                  } 
                  if(Amask) 
                    A = (dc & Amask); 
                  break; 
                case SPG_SRC_ALPHA: // Blend and keep src alpha 
                  if( alpha != SDL_ALPHA_OPAQUE ){ 
                    R = ((dc & Rmask) + (( R - (dc & Rmask) ) * alpha >> 8)) & Rmask; 
                    G = ((dc & Gmask) + (( G - (dc & Gmask) ) * alpha >> 8)) & Gmask; 
                    B = ((dc & Bmask) + (( B - (dc & Bmask) ) * alpha >> 8)) & Bmask; 
                  } 
                  if(Amask) 
                    A = (alpha << surface->format->Ashift); 
                  break; 
                case SPG_COPY_SRC_ALPHA: // Direct copy with src alpha 
                  if(Amask) 
                    A = (alpha << surface->format->Ashift); 
                  break; 
                case SPG_COPY_DEST_ALPHA: // Direct copy with dest alpha 
                  if(Amask) 
                    A = (dc & Amask); 
                  break; 
                case SPG_COPY_COMBINE_ALPHA: // Direct copy with combined alpha 
                  if(Amask) 

                    A = ((((dc & Amask) >> surface->format->Ashift) + 
                          alpha) >> 1) << surface->format->Ashift; 
                  break; 
                case SPG_COPY_NO_ALPHA: // Direct copy, alpha opaque 
                  if(Amask) 
                    A = (SDL_ALPHA_OPAQUE << surface->format->Ashift); 
                  break; 
                case SPG_COPY_ALPHA_ONLY: // Direct copy of just the alpha 
                  R = dc & Rmask; 
                  G = dc & Gmask; 
                  B = dc & Bmask; 
                  if(Amask) 
                    A = (alpha << surface->format->Ashift); 
                  break; 
                case SPG_COMBINE_ALPHA_ONLY: // Blend of just the alpha 
                  R = dc & Rmask; 
                  G = dc & Gmask; 
                  B = dc & Bmask; 
                  if(Amask) 

                    A = ((((dc & Amask) >> surface->format->Ashift) + 
                          alpha) >> 1) << surface->format->Ashift; 
                  break; 
                case SPG_REPLACE_COLORKEY: // Replace the colorkeyed color 
                  if(!(surface->flags & SDL_SRCCOLORKEY) || dc != surface->format->colorkey) 
                    return; 
                  if(Amask) 
                    A = (alpha << surface->format->Ashift); 
                  break; 
              } 

              *pixel = R | G | B | A; 
              break; 
    } 
  } 
} 
*/


static void draw_rgb_pixel(int y, int x, uint8_t r, uint8_t g, uint8_t b)
{
  Uint32 color = SDL_MapRGB(Surf_Display->format, r, g, b);

  //SDL_MapRGBA

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
              *(bufp+Surf_Display->format->Rshift/8) = r;
              *(bufp+Surf_Display->format->Gshift/8) = g;
              *(bufp+Surf_Display->format->Bshift/8) = b;
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


static Uint32 timeout_callback(Uint32 interval, void *UNUSED(param)) {
  SDL_Event event;
  SDL_UserEvent userevent;

  SDL_SemWait(timeout_semaphore);

  if (timeout_timer_exists == true) {
    SDL_RemoveTimer(timeout_timer);
    timeout_timer_exists = false;

    //printf("event, interval: %d\n", interval);

    userevent.type = SDL_USEREVENT;
    userevent.code = 0;
    userevent.data1 = NULL;
    userevent.data2 = NULL;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
  }

  SDL_SemPost(timeout_semaphore);

  return interval;
}


static int get_next_event(z_ucs *z_ucs_input, int timeout_millis)
{
  bool running = true;
  SDL_Event Event;
  int wait_result, result = -1;

  if (timeout_millis > 0) {
    //printf("input timeout: %d ms.\n", timeout_millis);
    TRACE_LOG("input timeout: %d ms.\n", timeout_millis);
    SDL_SemWait(timeout_semaphore);
    timeout_timer = SDL_AddTimer(timeout_millis, &timeout_callback, NULL);
    timeout_timer_exists = true;
    SDL_SemPost(timeout_semaphore);
  }

  //printf("polling...\n");
  while ((running == true) && (wait_result = SDL_WaitEvent(&Event))) {
    if (Event.type == SDL_QUIT) {
      TRACE_LOG("quit\n");
      running = false;
    }
    else if (Event.type == SDL_KEYDOWN) {
      TRACE_LOG("keydown\n");
      //printf("keydown.\n");
      if (Event.key.keysym.sym == SDLK_BACKSPACE) {
        result = EVENT_WAS_CODE_BACKSPACE;
        running = false;
      }
      else if (Event.key.keysym.sym == SDLK_UP) {
        result = EVENT_WAS_CODE_CURSOR_UP;
        running = false;
      }
      else if (Event.key.keysym.sym == SDLK_DOWN) {
        result = EVENT_WAS_CODE_CURSOR_DOWN;
        running = false;
      }
      else if (Event.key.keysym.sym == SDLK_LEFT) {
        result = EVENT_WAS_CODE_CURSOR_LEFT;
        running = false;
      }
      else if (Event.key.keysym.sym == SDLK_RIGHT) {
        result = EVENT_WAS_CODE_CURSOR_RIGHT;
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
      printf("resize: %d x %d\n", Event.resize.w, Event.resize.h);

      if ((Surf_Display = SDL_SetVideoMode(
              Event.resize.w,
              Event.resize.h,
              sdl_color_depth,
              sdl_video_flags)) == NULL) {
        printf("err-setvideomode\n");
      }

      sdl_interface_screen_height_in_pixels = Event.resize.h;
      sdl_interface_screen_width_in_pixels = Event.resize.w;

      new_pixel_screen_size(Event.resize.h, Event.resize.w);
    }
    else if (Event.type == SDL_USEREVENT) {
      result = EVENT_WAS_TIMEOUT;
      running = false;
    }
  }

  if (timeout_millis > 0) {
    SDL_SemWait(timeout_semaphore);
    if (timeout_timer_exists == true) {
      SDL_RemoveTimer(timeout_timer);
      timeout_timer_exists = false;
    }
    SDL_SemPost(timeout_semaphore);
  }

  TRACE_LOG("return\n");
  //printf("return\n");

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

  /*
  printf("copy-area: %d, %d to %d, %d: %d x %d.\n",
      srcx, srcy, dstx, dsty, width, height);
  */

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


void fill_area(int startx, int starty, int xsize, int ysize,
    z_rgb_colour colour)
{
  int y, x;
  Uint32 sdl_colour;

  /*
  printf("Filling area %d,%d / %d,%d with %d\n",
      startx, starty, xsize, ysize, colour);
  */
  TRACE_LOG("Filling area %d,%d / %d,%d with %d\n",
      startx, starty, xsize, ysize, colour);

  sdl_colour
    = SDL_MapRGB(
        Surf_Display->format,
        red_from_z_rgb_colour(colour),
        green_from_z_rgb_colour(colour),
        blue_from_z_rgb_colour(colour));

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


static int console_output(z_ucs *output)
{
  while (*output != 0)
  {
    zucs_string_to_utf8_string(
        output_char_buf,
        &output,
        SDL_OUTPUT_CHAR_BUF_SIZE);

    TRACE_LOG("Console output: %s\n", output_char_buf);
    fputs(output_char_buf, stdout);
  }
  fflush(stdout);

  return 0;
}


static struct z_screen_pixel_interface sdl_interface =
{
  &draw_rgb_pixel,
  &is_input_timeout_available,
  &get_next_event,
  &get_interface_name,
  &is_colour_available,
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
  &fill_area,
  &set_cursor_visibility,
  &get_default_foreground_colour,
  &get_default_background_colour,
  &console_output
};


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

  fizmo_register_screen_pixel_interface(&sdl_interface);

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

  if (story_filename_parameter_number == -1) {
    // User provided no story file name.
    print_startup_syntax();
  }
  else {
    // The user has given some filename or description name on the command line.
    input_file = argv[story_filename_parameter_number];

    // Check if parameter is a valid filename.
    story_stream = fsi->openfile(
        input_file, FILETYPE_DATA, FILEACCESS_READ);

    if (story_stream == NULL)
    {
      i18n_translate_and_exit(
          fizmo_sdl_module_name,
          i18n_sdl_COULD_NOT_OPEN_OR_FIND_P0S,
          -0x2016,
          input_file);
      exit(EXIT_FAILURE);
    }
    else {
      //SDL_putenv("SDL_VIDEODRIVER=Quartz");

      if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        i18n_translate_and_exit(
            fizmo_sdl_module_name,
            i18n_sdl_FUNCTION_CALL_P0S_ABORTED_DUE_TO_ERROR,
            -1,
            "SDL_Init");

      atexit(SDL_Quit);
      SDL_EnableUNICODE(1);
      SDL_EnableKeyRepeat(200, 20);

      if ((Surf_Display = SDL_SetVideoMode(
              sdl_interface_screen_width_in_pixels,
              sdl_interface_screen_height_in_pixels,
              sdl_color_depth,
              sdl_video_flags)) == NULL)
        i18n_translate_and_exit(
            fizmo_sdl_module_name,
            i18n_sdl_FUNCTION_CALL_P0S_ABORTED_DUE_TO_ERROR,
            -1,
            "SDL_SetVideoMode");

      timeout_semaphore = SDL_CreateSemaphore(1);

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
          savegame_to_restore);

      SDL_DestroySemaphore(timeout_semaphore);
      SDL_Quit();
    }
  }

#ifdef ENABLE_TRACING
  TRACE_LOG("Turning off trace.\n\n");
  turn_off_trace();
#endif // ENABLE_TRACING

  return 0;
}

