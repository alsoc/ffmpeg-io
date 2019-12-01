#pragma once

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ffmpeg_pixfmt {
  char s[16];
} ffmpeg_pixfmt;

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

typedef struct ffmpeg_handle {
  FILE* pipe;
  size_t input_width;
  size_t input_height;
  size_t output_width;
  size_t output_height;
  ffmpeg_pixfmt input_pixfmt;
  ffmpeg_pixfmt output_pixfmt;
  int error;
} ffmpeg_handle;

size_t ffmpeg_pixel_size(ffmpeg_pixfmt);
size_t ffmpeg_pixel_nchannel(ffmpeg_pixfmt);

const char* ffmpeg_pixfmt2str(const ffmpeg_pixfmt*);
ffmpeg_pixfmt ffmpeg_str2pixfmt(const char*);
const char* ffmpeg_error2str(ffmpeg_error);

void ffmpeg_init(ffmpeg_handle*);
void ffmpeg_compatible_writer(ffmpeg_handle* writer, const ffmpeg_handle* reader);

#ifdef __cplusplus
}
#endif
