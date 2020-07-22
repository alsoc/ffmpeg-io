#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "cmd.h"
#include "formatter.h"
#include "ffmpeg-io/reader.h"


int ffmpeg_start_reader(ffmpeg_handle* h, const char* filename, const ffmpeg_options* opts) {
  static const ffmpeg_options no_opts;
  if (opts == NULL) opts = &no_opts;
  unsigned width = h->output.width;
  unsigned height = h->output.height;
  ffmpeg_ratio ifps = h->input.fps;
  ffmpeg_ratio ofps = h->output.fps;
  ffmpeg_pixfmt pixfmt = h->output.pixfmt;

  if (width  == 0) width  = h->input.width;
  if (height == 0) height = h->input.height;
  if (ofps.num == 0 || ofps.den == 0) ofps = ifps;
  if (pixfmt.s[0] == '\0') h->output.pixfmt = h->input.pixfmt;

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

  h->output.pixfmt = pixfmt;
  h->output.width = width;
  h->output.height = height;

  ffmpeg_formatter cmd;
  ffmpeg_formatter_init(&cmd);

  ffmpeg_formatter_append(&cmd, "exec %s -loglevel error", get_ffmpeg());
  if (opts->force_input_fps && ifps.num > 0 && ifps.den > 0) {
    ffmpeg_formatter_append(&cmd, " -r %d/%d", ifps.num, ifps.den);
  }
  ffmpeg_formatter_append(&cmd, " -i '%s'", filename);

  const char* filter_prefix = " -filter:v ";
  if ((ifps.num != ofps.num || ifps.den != ofps.den) && ofps.num > 0 && ofps.den > 0) {
    ffmpeg_formatter_append(&cmd, "%sfps=fps=%d/%d", filter_prefix, ofps.num, ofps.den);
    filter_prefix = ",";
  }
  if (h->input.width != width || h->input.height != height) {
    if (opts->keep_aspect) {
      ffmpeg_formatter_append(&cmd, "%sscale=%d:%d,force_original_aspect_ratio=increase,crop=%d:%d", filter_prefix, width, height, width, height);
    } else {
      ffmpeg_formatter_append(&cmd, "%sscale=%d:%d", filter_prefix, width, height);
    }
    filter_prefix = ",";
  }

  ffmpeg_formatter_append(&cmd, " -f image2pipe -vcodec rawvideo -pix_fmt %s - </dev/null", ffmpeg_pixfmt2str(&pixfmt));

  if (opts->debug) printf("cmd: %s\n", cmd.str);

  h->pipe = popen(cmd.str, "r");
  int success = 1;
  if (!h->pipe) {
    h->error = ffmpeg_pipe_error;
    success = 0;
  }
  ffmpeg_formatter_fini(&cmd);
  return success;
}

int ffmpeg_read1d(ffmpeg_handle* h, uint8_t* data, size_t pitch) {
  size_t width = h->output.width;
  size_t height = h->output.height;
  ffmpeg_pixfmt pixfmt = h->output.pixfmt;
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
  size_t width = h->output.width;
  size_t height = h->output.height;
  ffmpeg_pixfmt pixfmt = h->output.pixfmt;
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
    size_t read = fread(data[i], elsize, width, pipe);
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
