#ifndef FRAME_POOL_H
#define FRAME_POOL_H

#include "frame_gray.h"
#include "memory_pool.h"
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Frame 전용 메모리 풀 생성
   * @param frame_count 프레임 수
   * @param pixel_size  한 픽셀 당 바이트 크기 (ex. 1 = GRAY, 3 = RGB, 4 = RGBA, float 등)
   * @param width       프레임 가로 해상도
   * @param height      프레임 세로 해상도
   * @return MemoryPool* 또는 NULL (errno 설정)
   */
  MemoryPool *frame_pool_create(size_t frame_count, size_t pixel_size, size_t width, size_t height);

  /**
   * @brief Frame 전용 MemoryBlock 할당
   * @param mp        Frame용 MemoryPool 포인터
   * @param init_ref  초기 refcount
   * @return Frame* 또는 NULL
   */
  Frame *frame_pool_alloc(MemoryPool *mp, int init_ref);

  /**
   * @brief Frame 반환 (refcount 0일 때 자동 반납)
   */
  void frame_pool_release(MemoryPool *mp, Frame *frame);

  /**
   * @brief Frame의 참조 횟수 증가
   */
  void frame_pool_retain(Frame *frame);

#ifdef __cplusplus
}
#endif

#endif // FRAME_POOL_H
