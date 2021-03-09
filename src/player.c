#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "cmd.h"
#include "formatter.h"
#include "ffmpeg-io/player.h"


int ffmpeg_start_player(ffmpeg_handle* h, const ffmpeg_options* opts) {
  static const ffmpeg_options no_opts;
  if (opts == NULL) opts = &no_opts;
  if (!ffmpeg_valid_descriptor(&h->input, &h->error)) return 0;
  int width  = h->input.width;
  int height = h->input.height;
  ffmpeg_pixfmt pixfmt = h->input.pixfmt;

  ffmpeg_formatter cmd;
  ffmpeg_formatter_init(&cmd);

  ffmpeg_formatter_append(&cmd, "exec %s -loglevel error -f rawvideo -vcodec rawvideo -pixel_format %s -video_size %dx%d", get_ffplay(), ffmpeg_pixfmt2str(&pixfmt), width, height);

  if (opts->window_title != NULL) {
    ffmpeg_formatter_append(&cmd, " -window_title '%s'", opts->window_title);
  }
  if (opts->infinite_buffer) {
    ffmpeg_formatter_append(&cmd, " -infbuf");
  }
  if (h->input.framerate.num > 0 && h->input.framerate.den > 0) {
    ffmpeg_formatter_append(&cmd, " -framerate %d/%d", h->input.framerate.num, h->input.framerate.den);
  }
  ffmpeg_formatter_append(&cmd, " -");
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

int ffmpeg_play1d(ffmpeg_handle* h, const uint8_t* data, size_t pitch) {
  return ffmpeg_write1d(h, data, pitch);
}

int ffmpeg_play2d(ffmpeg_handle* h, uint8_t** data) {
  return ffmpeg_write2d(h, data);
}

int ffmpeg_stop_player(ffmpeg_handle* h) {
  return ffmpeg_stop_writer(h);
}
