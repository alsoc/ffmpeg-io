#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "cmd.h"
#include "ffmpeg-io/reader.h"

#define BEGIN_SECTION "[STREAM]"
#define END_SECTION "[/STREAM]"
#define KEY_WIDTH "width"
#define KEY_HEIGHT "height"
#define KEY_TYPE "codec_type"
#define KEY_PIXFMT "pix_fmt"
#define KEY_CODEC "codec_name"
#define KEY_FPS "avg_frame_rate"
#define TYPE_VIDEO "video"

typedef struct stream_section {
  const char* type;
  const char* pixfmt;
  const char* codec;
  int width;
  int height;
  ffmpeg_ratio fps;
  ffmpeg_error error;
} stream_section;

static inline int ffmpeg_isspace(char c) {
  return c == ' ' || c == '\t';
}
static inline int ffmpeg_iseol(char c) {
  return c == '\0' || c == '\n' || c == '\r';
}
static inline int ffmpeg_isword(char c) {
  if (c >= 'a' && c <= 'z') return 1;
  if (c >= '0' && c <= '9') return 1;
  if (c == '_' || c == '-') return 1;
  return 0;
}
static inline const char* eat_spaces(const char* str) {
  while (ffmpeg_isspace(*str)) ++str;
  return str;
}
static inline const char* eat_newline(const char* str) {
  str = eat_spaces(str);
  if (str[0] == '\r' && str[1] == '\n') {
    str += 2;
  } else if (str[0] == '\n' && str[1] == '\r') {
    str += 2;
  } else if (str[0] == '\n' || str[0] == '\r') {
    str += 1;
  } else {
    return NULL;
  }
  str = eat_spaces(str);
  return str;
}
static inline const char* end_of_line(const char* str) {
  while (!ffmpeg_iseol(*str)) ++str;
  return str;
}
static inline const char* eat_equals(const char* str) {
  str = eat_spaces(str);
  if (*str != '=') return NULL;
  str = eat_spaces(str+1);
  return str;
}
static const char* read_section(stream_section* sec, const char* str) {
  memset(sec, 0, sizeof(stream_section));
  sec->codec = NULL;
  sec->pixfmt = NULL;

  if (strncmp(str, BEGIN_SECTION, sizeof(BEGIN_SECTION)-1) != 0) {
    sec->error = ffmpeg_ffprobe_invalid_section_start;
    return NULL;
  }
  str = eat_newline(str + sizeof(BEGIN_SECTION)-1);
  if (str == NULL) {
    sec->error = ffmpeg_ffprobe_invalid_section_start;
    return NULL;
  }


  while (strncmp(str, END_SECTION, sizeof(END_SECTION)-1) != 0) {
    if (strncmp(str, KEY_TYPE, sizeof(KEY_TYPE)-1) == 0) {
      str = eat_equals(str + sizeof(KEY_TYPE)-1);
      if (str == NULL) {
        sec->error = ffmpeg_ffprobe_invalid_statement;
        return NULL;
      }
      if (sec->type != NULL) {
        sec->error = ffmpeg_ffprobe_multiple_codec;
        return NULL;
      }
      if (ffmpeg_iseol(*str)) {
        sec->error = ffmpeg_ffprobe_invalid_statement;
        return NULL;
      }
      sec->type = str;
      str = end_of_line(str);
    } else if (strncmp(str, KEY_PIXFMT, sizeof(KEY_PIXFMT)-1) == 0) {
      str = eat_equals(str + sizeof(KEY_PIXFMT)-1);
      if (str == NULL) {
        sec->error = ffmpeg_ffprobe_invalid_statement;
        return NULL;
      }
      if (sec->pixfmt != NULL) {
        sec->error = ffmpeg_ffprobe_multiple_pixfmt;
        return NULL;
      }
      if (ffmpeg_iseol(*str)) {
        sec->error = ffmpeg_ffprobe_invalid_statement;
        return NULL;
      }
      sec->pixfmt = str;
      str = end_of_line(str);
    } else if (strncmp(str, KEY_CODEC, sizeof(KEY_CODEC)-1) == 0) {
      str = eat_equals(str + sizeof(KEY_CODEC)-1);
      if (str == NULL) {
        sec->error = ffmpeg_ffprobe_invalid_statement;
        return NULL;
      }
      if (sec->codec != NULL) {
        sec->error = ffmpeg_ffprobe_invalid_statement;
        return NULL;
      }
      if (ffmpeg_iseol(*str)) {
        sec->error = ffmpeg_ffprobe_invalid_statement;
        return NULL;
      }
      sec->codec = str;
      str = end_of_line(str);
    } else if (strncmp(str, KEY_WIDTH, sizeof(KEY_WIDTH)-1) == 0) {
      str = eat_equals(str + sizeof(KEY_WIDTH)-1);
      if (str == NULL) {
        sec->error = ffmpeg_ffprobe_invalid_statement;
        return NULL;
      }
      if (sec->width != 0) {
        sec->error = ffmpeg_ffprobe_multiple_width;
      }
      sec->width = strtol(str, (char**)&str, 10);
      str = eat_spaces(str);
      if (sec->width == 0 || !ffmpeg_iseol(*str)) {
        abort();
        sec->error = ffmpeg_ffprobe_invalid_statement;
        return NULL;
      }
    } else if (strncmp(str, KEY_HEIGHT, sizeof(KEY_HEIGHT)-1) == 0) {
      str = eat_equals(str + sizeof(KEY_HEIGHT)-1);
      if (str == NULL) {
        sec->error = ffmpeg_ffprobe_invalid_statement;
        return NULL;
      }
      if (sec->height != 0) {
        sec->error = ffmpeg_ffprobe_multiple_height;
      }
      sec->height = strtol(str, (char**)&str, 10);
      str = eat_spaces(str);
      if (sec->height == 0 || !ffmpeg_iseol(*str)) {
        sec->error = ffmpeg_ffprobe_invalid_statement;
        return NULL;
      }
    } else if (strncmp(str, KEY_FPS, sizeof(KEY_FPS)-1) == 0) {
      str = eat_equals(str + sizeof(KEY_FPS)-1);
      if (str == NULL) {
        sec->error = ffmpeg_ffprobe_invalid_statement;
        return NULL;
      }
      if (sec->fps.num != 0 || sec->fps.den != 0) {
        sec->error = ffmpeg_ffprobe_invalid_statement;
      }
      sec->fps.num = strtol(str, (char**)&str, 10);
      str = eat_spaces(str);
      if (*str == '/') {
        ++str;
        sec->fps.den = strtol(str, (char**)&str, 10);
        str = eat_spaces(str);
      } else {
        sec->fps.den = 1;
      }
      if (sec->fps.num == 0 || sec->fps.den == 0 || !ffmpeg_iseol(*str)) {
        sec->error = ffmpeg_ffprobe_invalid_statement;
        return NULL;
      }
    } else {
      str = end_of_line(str);
    }
    str = eat_newline(str);
    if (str == NULL) {
      sec->error = ffmpeg_ffprobe_unclosed_section;
      return NULL;
    }
  }
  str = eat_spaces(str + sizeof(END_SECTION)-1);
  if (*str == '\0') {
    sec->error = ffmpeg_noerror;
    return NULL;
  }
  str = eat_newline(str);
  if (str == NULL) {
    sec->error = ffmpeg_ffprobe_invalid_section_stop;
    return NULL;
  }

  if (sec->type == NULL) {
    sec->error = ffmpeg_ffprobe_no_codec;
    return NULL;
  }

  return str;
}

int ffmpeg_probe(ffmpeg_handle* h, const char* filename) {
  int result = 0;
  char buffer[BUFFER_SIZE];
  char *filecontent = NULL;
  snprintf(buffer, BUFFER_SIZE, "%s -v quiet -show_streams '%s'", get_ffprobe(), filename);

  FILE* probe = popen(buffer, "r");
  if (!probe) {
    goto cleanup;
  }
  size_t n = 0;
  ssize_t read = getdelim(&filecontent, &n, -1, probe);
  pclose(probe);
  if (read == -1) {
    h->error = ffmpeg_pipe_error;
    goto cleanup;
  }

  stream_section sec;
  const char* str = eat_spaces(filecontent);
  do {
    str = read_section(&sec, str);
  } while (str != NULL && strncmp(sec.type, TYPE_VIDEO, sizeof(TYPE_VIDEO)-1) != 0);

  if (sec.error != ffmpeg_noerror) {
    h->error = sec.error;
    goto cleanup;
  }

  if (sec.pixfmt == NULL){
    h->error = ffmpeg_ffprobe_no_pixfmt;
    goto cleanup;
  }
  if (sec.width == 0){
    h->error = ffmpeg_ffprobe_no_width;
    goto cleanup;
  }
  if (sec.height == 0){
    h->error = ffmpeg_ffprobe_no_height;
    goto cleanup;
  }
  if (sec.fps.num != 0 && sec.fps.den != 0) {
    h->input.fps = sec.fps;
  }

  h->input.width = sec.width;
  h->input.height = sec.height;

  memset(&h->input.pixfmt, 0, sizeof(h->input.pixfmt));
  for (size_t i = 0; i < sizeof(h->input.pixfmt.s)-1; i++) {
    char c = sec.pixfmt[i];
    if (ffmpeg_isword(c)) {
      h->input.pixfmt.s[i] = c;
    } else {
      break;
    }
  }

  //memset(&h->codec, 0, sizeof(h->codec));
  //for (size_t i = 0; i < sizeof(h->codec.s)-1; i++) {
  //  char c = sec.pixfmt[i];
  //  if (ffmpeg_isword(c)) {
  //    h->codec.s[i] = c;
  //  } else {
  //    break;
  //  }
  //}

  result = 1;
cleanup:
  free(filecontent);
  return result;
}
