#include "HashTableFunctions3.h"

void ListConstructor(List* list, size_t capacity) {
  assert(list);

  list->data = (Pair*)calloc(capacity, sizeof(Pair));
  list->capacity = capacity;
  list->size = 0;
}

void ListDestructor(List* list) {
  assert(list);

  free(list->data);
  list->capacity = 0;
  list->size = 0;
}

void ListInsert(List* list, Pair* pair) {
  assert(list);
  assert(pair);

  if (list->size < list->capacity) {
    list->data[list->size++] = *pair;
  } else {
    list->capacity *= 2;
    Pair* temp = list->data;
    list->data = (Pair*)calloc(list->capacity, sizeof(Pair));
    for (size_t i = 0; i < list->size; ++i) {
      list->data[i] = temp[i];
    }

    list->data[list->size++] = *pair;
    free(temp);
  }
}

bool _1_ListSearch(List* list, Pair* pair) {
  assert(list);
  assert(pair);

  __m256i key = _mm256_loadu_si256((const __m256i*)pair->key);
  __m256i data_key = _mm256_setzero_si256();

  for (size_t i = 0; i < list->size; ++i) {
    data_key = _mm256_loadu_si256((const __m256i*)list->data[i].key);
    int result = _mm256_movemask_epi8(_mm256_cmpeq_epi8(data_key, key));

    if (result == -1) {
      pair->value = list->data[i].value;
      return true;
    }
  }

  return false;
}

bool _2_ListSearch(List* list, Pair* pair) {
  assert(list);
  assert(pair);

  for (size_t i = 0; i < list->size; ++i) {
    if (strcmp(list->data[i].key, pair->key) == 0) {
      pair->value = list->data[i].value;
      return true;
    }
  }

  return false;
}

void HashTableConstructor(HashTable* table, size_t (*HashFunction)(Pair*)) {
  assert(table);

  table->size = HASH_TABLE_SIZE;
  table->list = (List*)calloc(table->size, sizeof(List));
  for (size_t i = 0; i < table->size; ++i) {
    ListConstructor(&table->list[i], LIST_CAPACITY);
  }
  table->HashFunction = HashFunction;
}

void HashTableDestructor(HashTable* table) {
  assert(table);

  for (size_t i = 0; i < table->size; ++i) {
    ListDestructor(&table->list[i]);
  }
  table->size = 0;
  table->HashFunction = 0;
  free(table->list);
}

bool HashTableInsert(HashTable* table, Pair* pair) {
  assert(table);
  assert(pair);

  size_t list_number = table->HashFunction(pair) % table->size;

  if (ListSearch(&table->list[list_number], pair)) {
    return false;
  }

  ListInsert(&table->list[list_number], pair);
  return true;
}

bool HashTableSearch(HashTable* table, Pair* pair) {
  assert(table);
  assert(pair);

  size_t list_number = table->HashFunction(pair) % table->size;

  return ListSearch(&table->list[list_number], pair);
}

size_t _1_HashFunction(Pair* pair) {
  assert(pair);

  const uint8_t* ptr = (const uint8_t*)pair->key;

  uint32_t result = 0xffffffff;
  while (*ptr != '\0') {
   			result = CRC32Table[(result ^ *ptr++) & 0xff] ^ (result >> 8);
  }

  return result ^ 0xffffffff;
}

size_t __attribute__((noinline)) HashFunction(Pair* pair) {
  size_t result = 0;

  __asm__ (
    ".intel_syntax noprefix \n\t"
    "xor rax, rax           \n\t"
    "mov rdx, [rdi]         \n\t"
    "loop_start:            \n\t"
    "mov cl, [rdx]          \n\t"
    "or cl, cl              \n\t"
    "jz loop_end            \n\t"
    "crc32 rax, cl          \n\t"
    "inc rdx                \n\t"
    "jmp loop_start         \n\t"
    "loop_end:              \n\t"
    ".att_syntax            \n\t"
    : "=a"(result)
    :
    : "rcx", "rdx", "rdi"
  );

  return result;
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

void FillHashTable(BufInfo* buf_info, HashTable* table) {
  assert(buf_info);
	assert(table);

  Pair pair = {};

  for (size_t i = 0; i < buf_info->buf_size; ++i) {
    pair.key = &buf_info->buf[i];
    while (buf_info->buf[i] != '=') {
      i++;
    }
    buf_info->buf[i++] = '\0';
    pair.value = &buf_info->buf[i];
    while (buf_info->buf[i] != '\n') {
      i++;
    }
    buf_info->buf[i] = '\0';
    HashTableInsert(table, &pair);
  }
}

void PrintfSizeList(HashTable* table) {
	assert(table);

	FILE* file = fopen("result.csv", "a");
	for (size_t i = 0; i < table->size; ++i) {
		fprintf(file, "%lu, ", table->list[i].size);
	}

	fprintf(file, "\n");
	fclose(file);
}

void HashTableTest(BufInfo* translate, HashTable* table) {
  assert(translate);
  assert(table);

  char* temp_buf = (char*)calloc(MAX_SIZE, sizeof(char));
  size_t j = 0;
  Pair pair = {};

  for (size_t i = 0; i < translate->buf_size;) {
    memset(temp_buf, 0, MAX_SIZE);
    j = 0;
    while (isalpha(translate->buf[i]) != 0) {
      if (isupper(translate->buf[i]) != 0) {
        temp_buf[j++] = tolower(translate->buf[i++]);
      } else {
        temp_buf[j++] = translate->buf[i++];
      }
    }
    temp_buf[j] = '\0';
    pair.key = temp_buf;
    pair.value = nullptr;
    for (size_t k = 0; k < MAX_TEST; ++k) {
      HashTableSearch(table, &pair);
    }

    while (i < translate->buf_size && isalpha(translate->buf[i]) == 0) {
      i++;
    }
  }

  free(temp_buf);
}
