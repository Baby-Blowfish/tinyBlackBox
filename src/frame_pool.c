#include "frame_pool.h"

MemoryPool *frame_pool_create(size_t count, size_t pixel_size, size_t width, size_t height)
{
  if (count == 0 || pixel_size == 0 || width == 0 || height == 0)
  {
    errno = EINVAL;
    return NULL;
  }
  // 블록당 크기: Frame 구조체 + 픽셀 데이터
  size_t block_size;
  if (__builtin_mul_overflow(pixel_size, width, &block_size) ||
      __builtin_mul_overflow(block_size, height, &block_size) ||
      __builtin_add_overflow(block_size, sizeof(Frame), &block_size))
  {
    errno = EOVERFLOW;
    return NULL;
  }
  // MemoryPool 생성
  MemoryPool *mp = mp_create(count, block_size);
  return mp; // mp_create이 내부에서 errno 처리
}

void frame_pool_destroy(MemoryPool *mp)
{
  if (!mp)
    return;
  mp_free(mp);
}

MemoryBlock *frame_pool_acquire(MemoryPool *mp, size_t refcount)
{
  if (!mp)
  {
    errno = EINVAL;
    return NULL;
  }
  MemoryBlock *blk = mp_alloc(mp, refcount);
  if (!blk)
  {
    // errno는 mp_alloc 내부에서 설정
    return NULL;
  }
  return blk;
}

void frame_pool_release(MemoryPool *mp, MemoryBlock *mb)
{
  if (!mp || !mb)
    return;
  mp_release(mp, mb);
}

void frame_pool_retain(MemoryBlock *mb)
{
  if (!mb)
    return;
  mp_retain(mb);
}

size_t frame_pool_free_count(const MemoryPool *mp)
{
  if (!mp)
    return 0;
  size_t free_cnt = 0;
  pthread_mutex_lock((pthread_mutex_t *)&mp->mutex);
  MemoryBlock *b = mp->free_list;
  while (b)
  {
    free_cnt++;
    b = b->next;
  }
  pthread_mutex_unlock((pthread_mutex_t *)&mp->mutex);
  return free_cnt;
}

size_t frame_pool_used_count(const MemoryPool *mp)
{
  if (!mp)
    return 0;
  return mp->pool_size - frame_pool_free_count(mp);
}
