#pragma once

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ffmpeg_pixfmt {
  char s[16];
} ffmpeg_pixfmt;
typedef struct ffmpeg_codec {
  char s[8];
} ffmpeg_codec;

typedef enum ffmpeg_error {
  ffmpeg_noerror = 0,
  ffmpeg_ffprobe_invalid_section_start,
  ffmpeg_ffprobe_invalid_section_stop,
  ffmpeg_ffprobe_invalid_statement,
  ffmpeg_ffprobe_unclosed_section,
  ffmpeg_ffprobe_multiple_codec,
  ffmpeg_ffprobe_multiple_width,
  ffmpeg_ffprobe_multiple_height,
  ffmpeg_ffprobe_multiple_pixfmt,
  ffmpeg_ffprobe_no_codec,
  ffmpeg_ffprobe_no_width,
  ffmpeg_ffprobe_no_height,
  ffmpeg_ffprobe_no_pixfmt,
  ffmpeg_ffprobe_no_video_section,
  ffmpeg_pipe_error,
  ffmpeg_invalid_width,
  ffmpeg_invalid_height,
  ffmpeg_invalid_pixfmt,
  ffmpeg_closed_pipe,
  ffmpeg_eof_error,
  ffmpeg_partial_read,
  ffmpeg_unknown_error,
} ffmpeg_error;

typedef struct ffmpeg_ratio {
  unsigned num, den;
} ffmpeg_ratio;
typedef struct ffmpeg_descriptor {
  unsigned width, height;
  ffmpeg_ratio fps;
  ffmpeg_pixfmt pixfmt;
} ffmpeg_descriptor;
typedef struct ffmpeg_handle {
  FILE* pipe;
  ffmpeg_descriptor input, output;
  int error;
} ffmpeg_handle;
typedef struct ffmpeg_options {
  const char* window_title;
  ffmpeg_codec codec;
  unsigned buffer;
  unsigned debug:1;
  unsigned force_input_fps:1;
  unsigned lossless:1;
  unsigned keep_aspect:1;
} ffmpeg_options;

size_t ffmpeg_pixel_size(ffmpeg_pixfmt);
size_t ffmpeg_pixel_nchannel(ffmpeg_pixfmt);

const char* ffmpeg_pixfmt2str(const ffmpeg_pixfmt*);
ffmpeg_pixfmt ffmpeg_str2pixfmt(const char*);
const char* ffmpeg_codec2str(const ffmpeg_codec*);
ffmpeg_codec ffmpeg_str2codec(const char*);
const char* ffmpeg_error2str(ffmpeg_error);

void ffmpeg_init(ffmpeg_handle*);
void ffmpeg_compatible_writer(ffmpeg_handle* writer, const ffmpeg_handle* reader);

void ffmpeg_options_init(ffmpeg_options*);

#ifdef __cplusplus
}
#endif
