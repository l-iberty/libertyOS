#ifndef MALLOC_H
#define MALLOC_H

#include "global.h"
#include "mm.h"
#include "type.h"

void *malloc(size_t size);
struct malloc_chunk *find_chunk(struct malloc_chunk **last, size_t size);
struct malloc_chunk *extend_heap(struct malloc_chunk *last, size_t size);
void split_chunk(struct malloc_chunk *chunk, size_t size);
void print_chunks();

void free(void *addr);
int valid_addr(void *addr);
struct malloc_chunk *get_chunk(void *addr);
struct malloc_chunk *merge_chunks(struct malloc_chunk *chunk);

struct malloc_chunk {
  size_t size; /* 数据区大小,需为4的整数倍 */
  struct malloc_chunk *prev;
  struct malloc_chunk *next;
  int avail;       /* 是否空闲可用 */
  void *magic_ptr; /* free的时候用于检查地址的合法性,magic_ptr应该指向data */
  char data[1];    /* 虚拟字段,指向数据区的首字节 */
};
#define CHUNK_SIZE 20

extern struct malloc_chunk *first_chunk;

#endif /* MALLOC_H */
