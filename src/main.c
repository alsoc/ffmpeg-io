#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ffmpeg-io/reader.h"
#include "ffmpeg-io/writer.h"

void usage(FILE* f, const char* cmd) {
  fprintf(f, "USAGE: %s [-i:w INPUT_WIDTH] [-i:h INPUT_HEIGHT] [-i:f INPUT_FORMAT] [-o:w OUTPUT_WIDTH] [-o:h OUTPUT_HEIGHT] [-o:f OUTPUT_FORMAT] INPUT_FILE OUTPUT_FILE\n", cmd);
}

int main(int argc, char *argv[]) {
  (void)argc;
  ffmpeg_handle reader, writer;
  ffmpeg_init(&reader);
  ffmpeg_init(&writer);

  const char*const* arg = 1+(const char*const*) argv;
  const char* input = NULL;
  const char* output = NULL;
  while (*arg != NULL) {
    if (strcmp(*arg, "-i:w") == 0) {
      if (*++arg == NULL) {
        usage(stderr, argv[0]);
        return 1;
      }
      reader.output_width = atoi(*arg);
    } else if (strcmp(*arg, "-i:h") == 0) {
      if (*++arg == NULL) {
        usage(stderr, argv[0]);
        return 1;
      }
      reader.output_height = atoi(*arg);
    } else if (strcmp(*arg, "-i:f") == 0) {
      if (*++arg == NULL) {
        usage(stderr, argv[0]);
        return 1;
      }
      reader.output_pixfmt = ffmpeg_str2pixfmt(*arg);
    } else if (strcmp(*arg, "-o:w") == 0) {
      if (*++arg == NULL) {
        usage(stderr, argv[0]);
        return 1;
      }
      writer.output_width = atoi(*arg);
    } else if (strcmp(*arg, "-o:h") == 0) {
      if (*++arg == NULL) {
        usage(stderr, argv[0]);
        return 1;
      }
      writer.output_height = atoi(*arg);
    } else if (strcmp(*arg, "-o:f") == 0) {
      if (*++arg == NULL) {
        usage(stderr, argv[0]);
        return 1;
      }
      writer.output_pixfmt = ffmpeg_str2pixfmt(*arg);
    } else if (strcmp(*arg, "-h") == 0 || strcmp(*arg, "--help") == 0) {
      usage(stdout, argv[0]);
      return 0;
    } else if (strcmp(*arg, "--") == 0) {
      while (*++arg != NULL) {
        if (input == NULL) {
          input = *arg;
        } else if (output == NULL) {
          output = *arg;
        } else {
          usage(stderr, argv[0]);
          return 1;
        }
      }
      break;
    } else if ((*arg)[0] == '-' && (*arg)[1] == '-') {
      usage(stderr, argv[0]);
      return 1;
    } else if ((*arg)[0] == '-') {
      usage(stderr, argv[0]);
      return 1;
    } else {
      if (input == NULL) {
        input = *arg;
      } else if (output == NULL) {
        output = *arg;
      } else {
        usage(stderr, argv[0]);
        return 1;
      }
    }
    arg++;
  }
  ffmpeg_probe(&reader, input);

  ffmpeg_compatible_writer(&writer, &reader);
  int pixsize = ffmpeg_pixel_size(reader.output_pixfmt);

  ffmpeg_start_reader(&reader, input);
  ffmpeg_start_writer(&writer, output);
  uint8_t *img = malloc(reader.output_width * reader.output_height * ffmpeg_pixel_size(reader.output_pixfmt));
  int i = 0;
  while (ffmpeg_read1d(&reader, img, pixsize*reader.output_width)) {
    ffmpeg_write1d(&writer, img, pixsize*writer.input_width);
    printf("\r%d", i);
    fflush(stdout);
    ++i;
  }
  printf("\n");

  free(img);

  ffmpeg_stop_writer(&writer);
  ffmpeg_stop_reader(&reader);
  return 0;
}
