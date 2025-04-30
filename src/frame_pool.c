#include "frame_pool.h"


MemoryPool *frame_pool_create(size_t frame_count, size_t pixel_size, size_t width, size_t height)
{
  size_t frame_struct_size = sizeof(Frame);
  size_t pixel_bytes = pixel_size * width * height;
  size_t total_block_size = frame_struct_size + pixel_bytes;
  return mp_create(frame_count, total_block_size);
}

Frame *frame_pool_alloc(MemoryPool *mp, int init_ref)
{
  MemoryBlock *blk = mp_alloc(mp, init_ref);
  if (!blk)
    return NULL;

  Frame *f = (Frame *)blk;
  void *data_ptr = (char *)blk + sizeof(Frame);

  f->data = data_ptr;
  // width/height/seq는 사용자 측에서 설정하거나 확장 가능
  return f;
}

void frame_pool_release(MemoryPool *mp, Frame *f)
{
  if (!f)
    return;
  mp_release(mp, (MemoryBlock *)f);
}

void frame_pool_retain(Frame *f)
{
  if (!f)
    return;
  mp_retain((MemoryBlock *)f);
}
