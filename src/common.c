#include <string.h>
#include "ffmpeg-io/common.h"

#define PIXFMT_GRAY "gray"
#define PIXFMT_RGB888 "rgb24"
#define PIXFMT_RGBA8888 "rgba"


size_t ffmpeg_pixel_size(ffmpeg_pixfmt fmt) {
  if (strcmp(fmt.s, PIXFMT_GRAY) == 0) return 1;
  if (strcmp(fmt.s, PIXFMT_RGB888) == 0) return 3;
  if (strcmp(fmt.s, PIXFMT_RGBA8888) == 0) return 4;
  return 0;
}
size_t ffmpeg_pixel_nchannel(ffmpeg_pixfmt fmt) {
  if (strcmp(fmt.s, PIXFMT_GRAY) == 0) return 1;
  if (strcmp(fmt.s, PIXFMT_RGB888) == 0) return 3;
  if (strcmp(fmt.s, PIXFMT_RGBA8888) == 0) return 4;
  return 0;
}

const char* ffmpeg_pixfmt2str(const ffmpeg_pixfmt* fmt) {
  if (fmt->s[0] == '\0') return "[unknown]";
  return fmt->s;
}
ffmpeg_pixfmt ffmpeg_str2pixfmt(const char* str) {
  ffmpeg_pixfmt fmt;
  memset(&fmt, 0, sizeof(fmt));
  if (str != NULL) {
    if (strcmp(str, "gray8") == 0) {
      str = "gray";
    } else if (strcmp(str, "rgb888") == 0) {
      str = "rgb24";
    } else if (strcmp(str, "rgba8888") == 0) {
      str = "rgba";
    }
    strncpy(fmt.s, str, sizeof(fmt.s)-1);
  }
  return fmt;
}
const char* ffmpeg_codec2str(const ffmpeg_codec* fmt) {
  if (fmt->s[0] == '\0') return "raw";
  return fmt->s;
}
ffmpeg_codec ffmpeg_str2codec(const char* str) {
  ffmpeg_codec codec;
  memset(&codec, 0, sizeof(codec));
  if (str != NULL) {
    if (strcmp(str, "raw") == 0 || strcmp(str, "rawvideo") == 0) {
      str = NULL;
    } else if (strcmp(str, "lossless") == 0) {
      str = "ffv1";
    }
  }
  if (str != NULL ) {
    strncpy(codec.s, str, sizeof(codec.s)-1);
  }
  return codec;
}

const char* ffmpeg_error2str(ffmpeg_error err) {
  switch (err) {
    case ffmpeg_noerror:
      return "";
    case ffmpeg_ffprobe_invalid_section_start:
      return "ffprobe_invalid_section_start";
    case ffmpeg_ffprobe_invalid_section_stop:
      return "ffprobe_invalid_section_stop";
    case ffmpeg_ffprobe_invalid_statement:
      return "ffprobe_invalid_statement";
    case ffmpeg_ffprobe_unclosed_section:
      return "ffprobe_unclosed_section";
    case ffmpeg_ffprobe_multiple_codec:
      return "ffprobe_multiple_codec";
    case ffmpeg_ffprobe_multiple_width:
      return "ffprobe_multiple_width";
    case ffmpeg_ffprobe_multiple_height:
      return "ffprobe_multiple_height";
    case ffmpeg_ffprobe_multiple_framerate:
      return "ffprobe_multiple_framerate";
    case ffmpeg_ffprobe_multiple_pixfmt:
      return "ffprobe_multiple_pixfmt";
    case ffmpeg_ffprobe_no_codec:
      return "ffprobe_no_codec";
    case ffmpeg_ffprobe_no_width:
      return "ffprobe_no_width";
    case ffmpeg_ffprobe_no_height:
      return "ffprobe_no_height";
    case ffmpeg_ffprobe_no_pixfmt:
      return "ffprobe_no_pixfmt";
    case ffmpeg_ffprobe_no_video_section:
      return "ffprobe_no_video_section";
    case ffmpeg_pipe_error:
      return "pipe_error";
    case ffmpeg_invalid_width:
      return "invalid_width";
    case ffmpeg_invalid_height:
      return "invalid_height";
    case ffmpeg_invalid_framerate:
      return "invalid_framerate";
    case ffmpeg_invalid_codec:
      return "invalid_codec";
    case ffmpeg_invalid_pixfmt:
      return "invalid_pixfmt";
    case ffmpeg_closed_pipe:
      return "closed_pipe";
    case ffmpeg_eof_error:
      return "eof";
    case ffmpeg_partial_read:
      return "partial_read";
    case ffmpeg_unknown_error:
      return "[unknown error]";
  }
  return "[unknown error]";
}

void ffmpeg_init(ffmpeg_handle* h) {
  memset(h, 0, sizeof(ffmpeg_handle));
}

void ffmpeg_options_init(ffmpeg_options* o) {
  memset(o, 0, sizeof(ffmpeg_options));
}

int ffmpeg_valid_descriptor(const ffmpeg_descriptor* p, ffmpeg_error* e) {
  ffmpeg_error error_sink;
  if (e == NULL) e = &error_sink;
  if (p->width == 0) {
    *e = ffmpeg_invalid_width;
    return 0;
  }
  if (p->height == 0) {
    *e = ffmpeg_invalid_height;
    return 0;
  }
  if (p->framerate.num == 0 || p->framerate.den == 0) {
    *e = ffmpeg_invalid_framerate;
    return 0;
  }
  if (p->pixfmt.s[0] == '\0') {
    *e = ffmpeg_invalid_pixfmt;
    return 0;
  }
  return 1;
}
void ffmpeg_merge_descriptor(ffmpeg_descriptor* out, const ffmpeg_descriptor* in) {
  if (out->width == 0) out->width = in->width;
  if (out->height == 0) out->height = in->height;
  if (out->framerate.num == 0 || out->framerate.den == 0) out->framerate = in->framerate;
  if (out->pixfmt.s[0] == '\0') out->pixfmt = in->pixfmt;
}
void ffmpeg_compatible_writer(ffmpeg_handle* writer, const ffmpeg_handle* reader) {
  writer->input = reader->output;
  ffmpeg_merge_descriptor(&writer->input, &reader->input);
}
