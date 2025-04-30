#ifndef FRAME_POOL_H
#define FRAME_POOL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "frame_gray.h"
#include "memory_pool.h"
#include <stddef.h>

  /**
   * @brief   Frame 전용 메모리 풀을 생성합니다.
   * @param   count       [in] 풀에 미리 생성할 Frame 개수 (>0)
   * @param   pixel_size  [in] 한 픽셀 당 바이트 수 (ex. GRAY=1, RGB=3)
   * @param   width       [in] 프레임 가로 해상도 (>0)
   * @param   height      [in] 프레임 세로 해상도 (>0)
   * @return  성공 시 MemoryPool*, 실패 시 NULL (errno 설정)
   */
  MemoryPool *frame_pool_create(size_t count, size_t pixel_size, size_t width, size_t height);

  /**
   * @brief   frame_pool_create()로 생성된 풀을 해제합니다.
   * @param   mp  [in] 풀 포인터 (NULL safe)
   */
  void frame_pool_destroy(MemoryPool *mp);

  /**
   * @brief   풀에서 MemoryBlock 하나를 획득(acquire)합니다.
   * @param   mp  [in] 초기화된 풀 포인터
   * @param   refcount [in] MemoryBlock의 내부 refcount 설정, 소비자의 수와 같음
   * @return  성공 시 MemoryBlock*, 실패 시 NULL (풀 고갈 or errno 설정)
   */
  MemoryBlock *frame_pool_acquire(MemoryPool *mp, size_t refcount);

  /**
   * @brief   획득한 MemoryBlock을 반환(release)합니다. refcount가 0이 되면 블록으로 반환.
   * @param   mp    [in] 풀 포인터
   * @param   mb    [in] 반환할 MemoryBlock 포인터 (NULL safe)
   */
  void frame_pool_release(MemoryPool *mp, MemoryBlock *mb);

  /**
   * @brief   MemoryBlock의 참조(refcount)를 하나 증가시킵니다
   * @param   mb [in] MemoryBlock 포인터 (NULL safe)
   */
  void frame_pool_retain(MemoryBlock *mb);

  /**
   * @brief   풀에서 사용 중인 블록 수를 조회합니다.
   * @param   mp  [in] 풀 포인터
   * @return  사용 중인 블록 개수
   */
  size_t frame_pool_used_count(const MemoryPool *mp);

  /**
   * @brief   풀에서 현재 여유 블록 수를 조회합니다.
   * @param   mp  [in] 풀 포인터
   * @return  여유 블록 개수
   */
  size_t frame_pool_free_count(const MemoryPool *mp);

#ifdef __cplusplus
}
#endif

#endif