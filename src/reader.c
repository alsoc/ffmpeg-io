#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "cmd.h"
#include "ffmpeg-io/reader.h"


int ffmpeg_start_reader(ffmpeg_handle* h, const char* filename) {
  char buffer[2*BUFFER_SIZE];
  int width = h->output_width;
  int height = h->output_height;
  ffmpeg_pixfmt pixfmt = h->output_pixfmt;

  if (width  == 0) width  = h->input_width;
  if (height == 0) height = h->input_height;
  if (pixfmt.s[0] == '\0') h->output_pixfmt = h->input_pixfmt;

  if (width == 0) {
    h->error = ffmpeg_invalid_width;
    return 0;
  }
  if (height == 0) {
    h->error = ffmpeg_invalid_height;
    return 0;
  }
  if (pixfmt.s[0] == '\0') {
    h->error = ffmpeg_invalid_pixfmt;
    return 0;
  }

  h->output_pixfmt = pixfmt;
  h->output_width = width;
  h->output_height = height;

  if (h->input_width == h->output_width && h->input_height == h->output_height) {
    snprintf(buffer, sizeof(buffer), "exec %s -loglevel error -i '%s' -f image2pipe -vcodec rawvideo -pix_fmt %s -", get_ffmpeg(), filename, ffmpeg_pixfmt2str(&pixfmt));
  } else {
    snprintf(buffer, sizeof(buffer), "exec %s -loglevel error -i '%s' -vf scale=%d:%d -f image2pipe -vcodec rawvideo -pix_fmt %s -", get_ffmpeg(), filename, width, height, ffmpeg_pixfmt2str(&pixfmt));
  }
  printf("cmd: %s\n", buffer);

  h->pipe = popen(buffer, "r");
  if (!h->pipe) {
    h->error = ffmpeg_pipe_error;
    return 0;
  }
  return 1;
}

int ffmpeg_read1d(ffmpeg_handle* h, uint8_t* data, size_t pitch) {
  size_t width = h->output_width;
  size_t height = h->output_height;
  ffmpeg_pixfmt pixfmt = h->output_pixfmt;
  size_t elsize = ffmpeg_pixel_size(pixfmt);
  FILE* pipe = h->pipe;

  if (pipe == NULL) {
    h->error = ffmpeg_closed_pipe;
    return 0;
  }
  if (feof(pipe)) {
    h->error = ffmpeg_eof_error;
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
    size_t read = fread(data, elsize, width, pipe);
    if (read < width) {
      h->error = ffmpeg_partial_read;
      return 0;
    }
    data += pitch;
  }
  return 1;
}

int ffmpeg_read2d(ffmpeg_handle* h, uint8_t** data) {
  size_t width = h->output_width;
  size_t height = h->output_height;
  ffmpeg_pixfmt pixfmt = h->output_pixfmt;
  size_t elsize = ffmpeg_pixel_size(pixfmt);
  FILE* pipe = h->pipe;

  if (pipe == NULL) {
    h->error = ffmpeg_closed_pipe;
    return 0;
  }
  if (feof(pipe)) {
    h->error = ffmpeg_eof_error;
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
    size_t read = fread(data[i], 1, width, pipe);
    if (read < width) {
      h->error = ffmpeg_partial_read;
      return 0;
    }
  }
  return 1;
}

int ffmpeg_stop_reader(ffmpeg_handle* h) {
  FILE* p = h->pipe;
  h->pipe = NULL;
  if (p != NULL) {
    pclose(p);
  }
  return 1;
}
