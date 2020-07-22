#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "cmd.h"
#include "formatter.h"
#include "ffmpeg-io/reader.h"


int ffmpeg_start_writer(ffmpeg_handle* h, const char* filename, const ffmpeg_options* opts) {
  static const ffmpeg_options no_opts;
  if (opts == NULL) opts = &no_opts;
  int iwidth  = h->input.width;
  int iheight = h->input.height;
  ffmpeg_pixfmt ipixfmt = h->input.pixfmt;
  int owidth  = h->output.width;
  int oheight = h->output.height;
  ffmpeg_pixfmt opixfmt = h->output.pixfmt;

  if (owidth  == 0) owidth  = iwidth;
  if (oheight == 0) oheight = iheight;
  if (opixfmt.s[0] == '\0') opixfmt = ipixfmt;

  if (iwidth == 0 || owidth == 0) {
    h->error = ffmpeg_invalid_width;
    return 0;
  }
  if (iheight == 0 || oheight == 0) {
    h->error = ffmpeg_invalid_height;
    return 0;
  }

  h->output.pixfmt = opixfmt;
  h->output.width  = owidth;
  h->output.height = oheight;

  const char* ifmt = ffmpeg_pixfmt2str(&ipixfmt);
  const char* ofmt = ffmpeg_pixfmt2str(&opixfmt);

  ffmpeg_formatter cmd;
  ffmpeg_formatter_init(&cmd);

  char const* dot = filename;
  for (char const* c = filename; *c != '\0'; ++c) {
    if (*c == '.') dot = c;
  }

  ffmpeg_formatter_append(&cmd, "exec %s -loglevel error -y -f rawvideo -vcodec rawvideo -pix_fmt %s -s %dx%d -r 25 -i - -an", get_ffmpeg(), ifmt, iwidth, iheight);


  if (strcmp(dot, ".mkv") == 0) {
    ffmpeg_formatter_append(&cmd, " -c:v ffv1");
  }
  if (iwidth != owidth || iheight != oheight) {
    ffmpeg_formatter_append(&cmd, " -vf scale=%d:%d", owidth, oheight);
  }
  if (opixfmt.s[0] != '\0') {
    ffmpeg_formatter_append(&cmd, " -pix_fmt %s", ofmt);
  }
  ffmpeg_formatter_append(&cmd, " -start_number 0 '%s'", filename);
  if (opts->debug) printf("cmd: %s\n", cmd.str);

  h->pipe = popen(cmd.str, "w");
  int success = 1;
  if (!h->pipe) {
    h->error = ffmpeg_pipe_error;
    success = 0;
  }
  ffmpeg_formatter_fini(&cmd);
  return success;
}

int ffmpeg_write1d(ffmpeg_handle* h, const uint8_t* data, size_t pitch) {
  size_t width = h->input.width;
  size_t height = h->input.height;
  ffmpeg_pixfmt pixfmt = h->input.pixfmt;
  size_t elsize = ffmpeg_pixel_size(pixfmt);
  FILE* pipe = h->pipe;

  if (pipe == NULL) {
    h->error = ffmpeg_closed_pipe;
    return 0;
  }
  if (width == 0) {
    h->error = ffmpeg_invalid_width;
    return 0;
  }
  if (height == 0) {
    h->error = ffmpeg_invalid_height;
    return 0;
  }
  if (elsize == 0) {
    h->error = ffmpeg_invalid_pixfmt;
    return 0;
  }

  for (size_t i = 0; i < height; i++) {
    fwrite(data, elsize, width, pipe);
    data += pitch;
  }
  return 1;
}

int ffmpeg_write2d(ffmpeg_handle* h, uint8_t** data) {
  size_t width = h->input.width;
  size_t height = h->input.height;
  ffmpeg_pixfmt pixfmt = h->input.pixfmt;
  size_t elsize = ffmpeg_pixel_size(pixfmt);
  FILE* pipe = h->pipe;

  if (pipe == NULL) {
    h->error = ffmpeg_closed_pipe;
    return 0;
  }
  if (width == 0) {
    h->error = ffmpeg_invalid_width;
    return 0;
  }
  if (height == 0) {
    h->error = ffmpeg_invalid_height;
    return 0;
  }
  if (elsize == 0) {
    h->error = ffmpeg_invalid_pixfmt;
    return 0;
  }

  for (size_t i = 0; i < height; i++) {
    fwrite(data[i], elsize, width, pipe);
  }
  return 1;
}

int ffmpeg_stop_writer(ffmpeg_handle* h) {
  FILE* p = h->pipe;
  h->pipe = NULL;
  if (p != NULL) {
    pclose(p);
  }
  return 1;
}
