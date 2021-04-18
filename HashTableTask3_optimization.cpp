#include "HashTableFunctions3.h"

int main() {
  BufInfo buf_info = {};
  ReadFile(&buf_info, "new-dict.txt");

  HashTable table = {};
  HashTableConstructor(&table, HashFunction);
  FillHashTable(&buf_info, &table);

  BufInfo* translate = (BufInfo*)calloc(1, sizeof(BufInfo));
  ReadFile(translate, "data.txt");
  clock_t timer = clock();
  HashTableTest(translate, &table);
  printf("%lu\n", clock() - timer);

  free(translate->buf);
  free(translate);

  HashTableDestructor(&table);

  free(buf_info.buf);
  return 0;
}

// valgring --tool=callgrind ./a.out
// kcachegrind callgrind.out.95854
