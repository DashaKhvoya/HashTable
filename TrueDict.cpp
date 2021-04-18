#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

struct BufInfo {
  char* buf;
  size_t buf_size;
};

size_t FileSize(const char* file_name);
void   ReadFile(BufInfo* buf_info, const char* file_name);

int main() {
  BufInfo* buf_info = (BufInfo*)calloc(1, sizeof(BufInfo));
  ReadFile(buf_info, "full-dict.txt");

  FILE* new_file = fopen ("new-dict.txt", "wb");
  size_t word_len = 0;
  for (size_t i = 0; i < buf_info->buf_size; ++i) {
    word_len = 0;

    while(buf_info->buf[i] != '=') {
      fprintf(new_file, "%c", buf_info->buf[i]);
      i++;
      word_len++;
    }

    while (word_len < 32) {
      fprintf(new_file, "%c", '\0');
      word_len++;
    }

    while(buf_info->buf[i] != '\n' && buf_info->buf[i] != '\0') {
      fprintf(new_file, "%c", buf_info->buf[i]);
      i++;
    }

    fprintf(new_file, "\n");
  }

  fclose(new_file);
  return 0;
}

size_t FileSize(const char* file_name) {
  assert(file_name);

  struct stat stbuf = {};
  size_t res = stat(file_name, &stbuf);

  return stbuf.st_size;
}

void ReadFile(BufInfo* buf_info, const char* file_name) {
  assert (buf_info);
  assert (file_name);

  FILE* file = fopen (file_name, "r");
  buf_info->buf_size = FileSize(file_name);
  buf_info->buf = (char*)calloc(buf_info->buf_size, sizeof(char));

  fread(buf_info->buf, sizeof(char), buf_info->buf_size, file);
  fclose (file);
}
