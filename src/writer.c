#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "cmd.h"
#include "ffmpeg-io/reader.h"


int ffmpeg_start_writer(ffmpeg_handle* h, const char* filename) {
  char buffer[2*BUFFER_SIZE];
  int iwidth  = h->input_width;
  int iheight = h->input_height;
  ffmpeg_pixfmt ipixfmt = h->input_pixfmt;
  int owidth  = h->output_width;
  int oheight = h->output_height;
  ffmpeg_pixfmt opixfmt = h->output_pixfmt;

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

  h->output_pixfmt = opixfmt;
  h->output_width  = owidth;
  h->output_height = oheight;

  const char* ifmt = ffmpeg_pixfmt2str(&ipixfmt);
  const char* ofmt = ffmpeg_pixfmt2str(&opixfmt);

  char const* dot = filename;
  char const* codec = "";
  for (char const* c = filename; *c != '\0'; ++c) {
    if (*c == '.') dot = c;
  }
  if (strcmp(dot, ".mkv") == 0) codec = "-c:v ffv1";

  char rescale[64];
  memset(rescale, 0, sizeof(rescale));
  if (iwidth != owidth || iheight != oheight) {
    snprintf(rescale, sizeof(rescale), "-vf scale=%d:%d", owidth, oheight);
  }

  char out_fmt[64];
  memset(out_fmt, 0, sizeof(out_fmt));
  if (opixfmt.s[0] != '\0') {
    snprintf(out_fmt, sizeof(out_fmt), "-pix_fmt %s", ofmt);
  }


  snprintf(buffer, sizeof(buffer), "exec %s -loglevel error -y -f rawvideo -vcodec rawvideo -pix_fmt %s -s %dx%d -r 25 -i - -an %s %s %s -start_number 0 '%s'", get_ffmpeg(), ifmt, iwidth, iheight, codec, rescale, out_fmt, filename);
  printf("cmd: %s\n", buffer);

  h->pipe = popen(buffer, "w");
  if (!h->pipe) {
    h->error = ffmpeg_pipe_error;
    return 0;
  }
  return 1;
}

int ffmpeg_write1d(ffmpeg_handle* h, const uint8_t* data, size_t pitch) {
  size_t width = h->input_width;
  size_t height = h->input_height;
  ffmpeg_pixfmt pixfmt = h->input_pixfmt;
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
  size_t width = h->input_width;
  size_t height = h->input_height;
  ffmpeg_pixfmt pixfmt = h->input_pixfmt;
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
