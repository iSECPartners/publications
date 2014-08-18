#include "replace_malloc.h"
#include <stdlib.h>
#include <stdio.h>

#include "config.h"
#include "wtf/PartitionAlloc.h"
#include <string.h>

static const malloc_table_t *funcs = NULL;
static unsigned int mallocs = 0, frees = 0, reallocs = 0, callocs = 0;

static PartitionAllocatorGeneric partition;
static bool initialized;

extern "C" {

void replace_init(const malloc_table_t *table)
{
  funcs = table;
  printf("In init!\n");
}

void replace_jemalloc_stats(jemalloc_stats_t *stats)
{
  printf("%d mallocs, %d frees, %d reallocs, %d callocs\n", mallocs, frees, reallocs, callocs);
}

void* replace_malloc(size_t size)
{
  mallocs++;
  if (UNLIKELY(!initialized)) {
    initialized = true;
    partition.init();
  }
  return partitionAllocGeneric(partition.root(), size);
}

void replace_free(void* ptr)
{
  //I believe this was a Chrome-only quirk. Going to attempt removing it
  //if (reinterpret_cast<uintptr_t>(ptr) >= 0x500000000000) 
  //  return funcs->free(ptr);
  frees++;
  partitionFreeGeneric(partition.root(), ptr);
}

void* replace_realloc(void* ptr, size_t size)
{
  reallocs++;
  if (UNLIKELY(!initialized)) { 
    initialized = true;
    partition.init();
  }
  if (UNLIKELY(!ptr)) {
    return partitionAllocGeneric(partition.root(), size);
  }
  //I believe this was a Chrome-only quirk. Going to attempt removing it
  //if (reinterpret_cast<uintptr_t>(ptr) >= 0x500000000000) 
  //  return funcs->realloc(ptr, size);
  if (UNLIKELY(!size)) {
    partitionFreeGeneric(partition.root(), ptr);
    return 0;
  }
  return partitionReallocGeneric(partition.root(), ptr, size);
}

void* replace_calloc(size_t nmemb, size_t size)
{
  void* ret;
  size_t real_size = nmemb * size;
  if (UNLIKELY(!initialized)) { 
    initialized = true;
    partition.init();
  }
  callocs++;
  RELEASE_ASSERT(!nmemb || real_size / nmemb == size);
  ret = partitionAllocGeneric(partition.root(), real_size);
  memset(ret, '\0', real_size);
  return ret;
}

void *replace_valloc(size_t size)
{
  printf("AH!!!! valloc.\n");
  return NULL;
}

void *replace_memalign(size_t alignment, size_t size)
{
  size_t remainder = size % alignment;

  return replace_malloc(size + remainder);
}

void *replace_aligned_alloc(size_t alignment, size_t size)
{
  printf("AH!!! aligned_alloc\n");
  return NULL;
}

int replace_posix_memalign(void **ptr, size_t alignment, size_t size)
{
  size_t remainder = size % alignment;
  *ptr = replace_malloc(size + remainder);
  if(*ptr == NULL)
    return -1;
  return 0;
}

size_t replace_malloc_usable_size(usable_ptr_t ptr)
{
  size_t s = partitionAllocGetSize(ptr);
  return s;
}

size_t replace_malloc_good_size(size_t size)
{
  return size;
}

void replace_jemalloc_purge_freed_pages()
{
}

void replace_jemalloc_free_dirty_pages()
{
}

}
