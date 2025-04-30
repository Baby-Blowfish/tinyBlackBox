
#include "memory_pool.h"

MemoryPool *mp_create(size_t pool_size, size_t block_size)
{
  MemoryPool *mp = malloc(sizeof(*mp));
  if (!mp)
  {
    errno = ENOMEM;
    return NULL;
  }
  if (mp_init(mp, pool_size, block_size) != 0)
  {
    free(mp);
    return NULL;
  }
  return mp;
}

int mp_init(MemoryPool *mp, size_t pool_size, size_t block_size)
{
  if (!mp || pool_size == 0 || block_size == 0)
  {
    errno = EINVAL;
    return -1;
  }

  mp->pool_size = pool_size;
  mp->block_size = block_size;
  mp->blocks = NULL;
  mp->pool_data = NULL;
  mp->free_list = NULL;

  mp->blocks = calloc(pool_size, sizeof(MemoryBlock));
  if (!mp->blocks)
  {
    errno = ENOMEM;
    return -1;
  }

  mp->pool_data = calloc(pool_size, block_size);
  if (!mp->pool_data)
  {
    errno = ENOMEM;
    free(mp->blocks);
    mp->blocks = NULL;
    return -1;
  }

  pthread_mutex_init(&mp->mutex, NULL);

  for (size_t i = 0; i < pool_size; ++i)
  {
    MemoryBlock *b = &mp->blocks[i];
    atomic_init(&b->refcount, 0);
    b->data = (char *)mp->pool_data + i * block_size;
    b->next = mp->free_list;
    mp->free_list = b;
  }
  return 0;
}

void mp_destroy(MemoryPool *mp)
{
  if (!mp)
    return;
  pthread_mutex_destroy(&mp->mutex);
  free(mp->pool_data);
  free(mp->blocks);
  mp->pool_data = NULL;
  mp->blocks = NULL;
  mp->free_list = NULL;
  mp->pool_size = 0;
  mp->block_size = 0;
}

void mp_free(MemoryPool *mp)
{
  if (!mp)
    return;
  mp_destroy(mp);
  free(mp);
}

MemoryBlock *mp_alloc(MemoryPool *mp, int init_count)
{
  if (!mp || init_count <= 0)
  {
    errno = EINVAL;
    return NULL;
  }
  pthread_mutex_lock(&mp->mutex);
  MemoryBlock *b = mp->free_list;
  if (b)
  {
    mp->free_list = b->next;
  }
  pthread_mutex_unlock(&mp->mutex);

  if (!b)
  {
    errno = ENOMEM;
    return NULL;
  }
  atomic_store_explicit(&b->refcount, init_count, memory_order_relaxed);
  return b;
}

void mp_retain(MemoryBlock *blk)
{
  if (blk)
  {
    atomic_fetch_add_explicit(&blk->refcount, 1, memory_order_relaxed);
  }
}

void mp_release(MemoryPool *mp, MemoryBlock *blk)
{
  if (!mp || !blk)
    return;
  int prev = atomic_fetch_sub_explicit(&blk->refcount, 1, memory_order_acq_rel);
  if (prev == 1)
  {
    pthread_mutex_lock(&mp->mutex);
    blk->next = mp->free_list;
    mp->free_list = blk;
    pthread_mutex_unlock(&mp->mutex);
  }
}

void *mp_get_block_data(const MemoryBlock *blk)
{
  return blk ? blk->data : NULL;
}

int mp_get_block_refcount(const MemoryBlock *blk)
{
  return blk ? atomic_load_explicit(&blk->refcount, memory_order_relaxed) : -1;
}

size_t mp_available_count(const MemoryPool *mp)
{
  if (!mp)
    return 0;

  size_t count = 0;
  pthread_mutex_lock((pthread_mutex_t *)&mp->mutex);
  for (MemoryBlock *p = mp->free_list; p; p = p->next)
    ++count;
  pthread_mutex_unlock((pthread_mutex_t *)&mp->mutex);
  return count;
}

size_t mp_total_size(const MemoryPool *mp)
{
  return mp ? mp->pool_size : 0;
}

size_t mp_block_size(const MemoryPool *mp)
{
  return mp ? mp->block_size : 0;
}

void mp_debug_dump(const MemoryPool *mp, FILE *fp)
{
  if (!fp)
    fp = stdout;
  if (!mp)
  {
    fprintf(fp, "[MemoryPool] NULL\n");
    return;
  }

  fprintf(fp, "[MemoryPool]\n");
  fprintf(fp, "  Total Blocks : %zu\n", mp->pool_size);
  fprintf(fp, "  Block Size   : %zu bytes\n", mp->block_size);
  fprintf(fp, "  Free Blocks  : %zu\n", mp_available_count(mp));
}
