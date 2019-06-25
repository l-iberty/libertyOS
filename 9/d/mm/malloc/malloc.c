#include "malloc.h"
#include "global.h"
#include "proc.h"
#include "stdio.h"

void* malloc(size_t size)
{
	struct malloc_chunk *chunk, *last, *first_chunk;
	size_t size_aligned;

	size_aligned = ROUND_UP(size, 4);
	first_chunk = get_first_chunk();

	if (first_chunk)
	{
		chunk = find_chunk(&last, size_aligned);
		if (chunk)
		{
			chunk->avail = 0; /* This chunk has been allocated, not available. */
			/**
			 * 如果余下的空间小于CHUNK_SIZE则无法分裂;等于CHUNK_SIZE的话,分裂后
			 * new_chunk->size为0;若以(CHUNK_SIZE + 4)为界限,分裂后new_chunk->size
			 * 至少为4.
			 */
			if ((chunk->size - size_aligned) >= (CHUNK_SIZE + 4))
			{ split_chunk(chunk, size_aligned); }
		}
		else
		{
			/* 没有合适的chunk,开辟一个新的. */
			chunk = extend_heap(last, size_aligned);
			if (chunk == NULL)
			{ return NULL; }
		}
	}
	else /* initial call */
	{
		chunk = extend_heap(NULL, size_aligned);
		if (chunk == NULL)
		{ return NULL; }
		set_first_chunk(chunk);
	}

	return chunk->data;
}

/* First fit */
/**
 * @param size must be aligned -> bit0 and bit1 must be cleared.
 */
struct malloc_chunk* find_chunk(struct malloc_chunk **last, size_t size)
{
	struct malloc_chunk *p;

	assert((size & 3) == 0);

	p = get_first_chunk();
	for (; p; p = p->next)
	{
		*last = p;
		if (p->avail && p->size <= size)
		{ return p; }
	}
	return NULL;
}

/**
 * @param size must be aligned -> bit0 and bit1 must be cleared.
 */
struct malloc_chunk* extend_heap(struct malloc_chunk *last, size_t size)
{
	struct malloc_chunk *chunk;

	assert((size & 3) == 0);

	void *p = sbrk(0);
	if ((uint32_t)p & 3 != 0)
	{
		/* if program break is not aligned, align it. */
		brk(ROUND_UP(p, 4));
		assert((uint32_t)sbrk(0) == ROUND_UP(p, 4));
	}

	chunk = sbrk(0);
	if (sbrk(CHUNK_SIZE + size) == (void*) -1)
	{
		printf("\n#ERROR#{extend_heap} sbrk() error.");
		return NULL;
	}

	chunk->size = size;
	chunk->avail = 0;
	chunk->next = NULL;
	chunk->prev = last;
	chunk->magic_ptr = chunk->data;
	if (last)
	{ last->next = chunk; }

	return chunk;
}

/**
 * @param size must be aligned -> bit0 and bit1 must be cleared.
 */
void split_chunk(struct malloc_chunk *chunk, size_t size)
{
	struct malloc_chunk *new_chunk;

	assert((size & 3) == 0);

	new_chunk = chunk->data + size;
	new_chunk->size = chunk->size - size - CHUNK_SIZE;
	new_chunk->avail = 1;
	new_chunk->next = chunk->next;
	new_chunk->prev = chunk;
	chunk->next = new_chunk;
	if (new_chunk->next)
	{ new_chunk->next->prev = new_chunk; }
}

void print_chunks()
{
	struct malloc_chunk *p = get_first_chunk();
	
	if (p == NULL)
	{
		printf("\nempty chunks.");
		return;
	}
	
	printf("\nchecking next:");
	for (; p && p->next; p = p->next)
	{
		printf("\n{%.8x}(size:%d,avail:%d,magic_ptr:%.8x,data:%.8x)->", p, p->size, p->avail, p->magic_ptr, p->data);
	}
	if (p) { printf("\n{%.8x}(size:%d,avail:%d,magic_ptr:%.8x,data:%.8x)", p, p->size, p->avail, p->magic_ptr, p->data); }

#if 0
	printf("\nchecking prev:");
	for (; p && p->prev; p = p->prev)
	{
		printf("\n{%.8x}(size:%d,avail:%d,magic_ptr:%.8x,data:%.8x)->", p, p->size, p->avail, p->magic_ptr, p->data);
	}
	if (p) { printf("\n{%.8x}(size:%d,avail:%d,magic_ptr:%.8x,data:%.8x)", p, p->size, p->avail, p->magic_ptr, p->data); }
#endif
}

void free(void *addr)
{
	struct malloc_chunk *chunk;
	
	assert(valid_addr(addr));

	chunk = get_chunk(addr);
	chunk->avail = 1;

	/* 在合适的情况下将相邻的chunk进行合并. */	
	chunk = merge_chunks(chunk);

	/**
	 * 1. (chunk->next == NULL)
	 *    说明这是最后一个chunk, 需回退program_break释放进程内存;
	 * 2. (chunk->next == NULL && chunk->prev == NULL)
	 *    说明当前chunk就是first_chunk, 需回退program_break, 并将first_chunk置为NULL.
	 */
	if (chunk->next == NULL)
	{
		brk(chunk);
		if (chunk->prev)
		{
			chunk->prev->next = NULL;
		}
		else
		{
			assert(chunk == get_first_chunk());
			set_first_chunk(NULL);
		}
	}
}

int valid_addr(void *addr)
{
	void *first_chunk = get_first_chunk();
	if (first_chunk)
	{
		if (addr > first_chunk && addr < sbrk(0)) /* addr应位于first_chunk和program_break之间 */
		{
			return (get_chunk(addr)->magic_ptr == addr); /* magic_ptr是否指向data? */
		}
	}
	return 0;
}

struct malloc_chunk* get_chunk(void *addr)
{
	assert(addr);
	
	return (struct malloc_chunk*)((char*)addr - CHUNK_SIZE);
}

/**
 * @breif 如果前后存在空闲的chunk,则需要进行合并,以减少内存碎片.
 * @return 合并后的空闲chunk
 */
struct malloc_chunk* merge_chunks(struct malloc_chunk *chunk)
{
	assert(chunk);
	
	if (chunk->prev && chunk->prev->avail)
	{
		chunk->prev->size += (CHUNK_SIZE + chunk->size);
		chunk->prev->next = chunk->next;
		if (chunk->next)
		{ chunk->next->prev = chunk->prev; }
		chunk = chunk->prev;
	}
	
	if (chunk->next && chunk->next->avail)
	{
		chunk->size += (CHUNK_SIZE + chunk->next->size);
		chunk->next = chunk->next->next;
		if (chunk->next)
		{ chunk->next->prev = chunk; }
	}
	
	return chunk;
}

