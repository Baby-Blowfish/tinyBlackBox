#ifndef FRAME_POOL_H
#define FRAME_POOL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "frame.h"
#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>

  /**
   * @brief 프레임 블록 구조체
   */
  typedef struct FrameBlock
  {
    atomic_int refcount;     // 참조 카운트
    struct FrameBlock *next; // free_list 연결 포인터
    Frame frame;             // 프레임 구조체
  } FrameBlock;

  /**
   * @brief 프레임 풀 구조체
   */
  typedef struct FramePool
  {
    FrameBlock *blocks;           // 블록 배열 (pool_size 개)
    void *pool_data;              // 실제 데이터 버퍼 (pool_size * block_size 바이트)
    size_t pool_size;             // 블록 개수
    size_t total_bytes_per_frame; // 총 바이트 수 (width * height * depth)
    FrameBlock *free_list;        // 사용 가능한 블록 리스트
    pthread_mutex_t mutex;        // free_list 보호용 뮤텍스
    pthread_cond_t cond;          // 블록 할당 대기용 조건 변수
  } FramePool;

  /**
   * @brief   Frame 전용 메모리 풀을 생성합니다.
   * @param   pool_size       [in] 풀에 미리 생성할 Frame 개수 (>0)
   * @param   depth  [in] 한 픽셀 당 바이트 수 (ex. GRAY=1, RGB=3)
   * @param   width       [in] 프레임 가로 해상도 (>0)
   * @param   height      [in] 프레임 세로 해상도 (>0)
   * @return  성공 시 FramePool*, 실패 시 NULL (errno 설정)
   */
  FramePool *frame_pool_create(size_t pool_size, size_t width, size_t height, DEPTH depth);

  /**
   * @brief Frame 풀을 in-place 초기화합니다.
   * @param fp          [out] 초기화할 FramePool 포인터 (NULL 불가)
   * @param pool_size   [in] 블록 개수 (>0)
   * @param   width       [in] 프레임 가로 해상도 (>0)
   * @param   height      [in] 프레임 세로 해상도 (>0)
   * @param   depth  [in] 한 픽셀 당 바이트 수 (ex. GRAY=1, RGB=3)
   * @return 0: 성공, -1: 실패 (errno 설정)
   */
  int frame_pool_init(FramePool *fp, size_t pool_size, size_t width, size_t height, DEPTH depth);

  /**
   * @brief   frame_pool_create()로 생성된 풀을 해제합니다.
   * @param   fp  [in] 풀 포인터 (NULL safe)
   */
  void frame_pool_destroy(FramePool *fp);

  /**
   * @brief in-place로 초기화된 메모리 풀을 해제합니다.
   * @param fp [in] 동적 생성된 FramePool 포인터 (NULL safe)
   */
  void frame_pool_free(FramePool *fp);

  /**
   * @brief 블록을 할당하고 refcount를 설정합니다.
   * @param fp         [in] 초기화된 FramePool 포인터
   * @param init_count [in] 초기 refcount (>0)
   * @return 할당된 FrameBlock*, 실패 시 NULL (errno 설정)
   */
  FrameBlock *fp_alloc(FramePool *fp, int init_count);

  /**
   * @brief 블록 참조 수를 증가시킵니다.
   * @param blk [in] FrameBlock 포인터 (NULL safe)
   */
  void fp_retain(FrameBlock *blk);

  /**
   * @brief 블록 참조 수를 감소시키고 0이 되면 free_list로 반환합니다.
   * @param fp  [in] 초기화된 FramePool 포인터
   * @param blk [in] FrameBlock 포인터 (NULL safe)
   */
  void fp_release(FramePool *fp, FrameBlock *blk);

  /**
   * @brief   블록의 데이터 포인터를 반환합니다.
   * @param   blk [in] FrameBlock 포인터
   * @return  blk->data 또는 NULL
   */
  void *fp_get_block_data(const FrameBlock *blk);

  /**
   * @brief   블록의 현재 참조 카운트를 반환합니다.
   * @param   blk [in] FrameBlock 포인터
   * @return  참조 카운트 또는 -1
   */
  int fp_get_block_refcount(const FrameBlock *blk);

  /**
   * @brief   현재 free_list에 남아있는 블록 개수를 반환합니다.
   * @param   fp [in] FramePool 포인터
   * @return  사용 가능한 블록 수
   */
  size_t fp_available_count(const FramePool *fp);

  /**
   * @brief   풀에서 현재 여유 블록 수를 조회합니다.
   * @param   fp  [in] 풀 포인터
   * @return  여유 블록 개수
   */
  size_t fp_used_count(const FramePool *fp);

  /**
   * @brief   전체 블록 수를 반환합니다.
   */
  size_t fp_total_block_size(const FramePool *fp);

  /**
   * @brief   블록 하나의 크기를 반환합니다.
   */
  size_t fp_total_bytes_per_frame_size(const FramePool *fp);

  /**
   * @brief   메모리 풀 상태를 출력합니다. (디버깅 용도)
   * @param   fp  [in] FramePool 포인터
   * @param   fp  [in] 출력 스트림 (NULL이면 stdout)
   */
  void fp_debug_dump(const FramePool *fp, FILE *file_p);

#ifdef __cplusplus
}
#endif

#endif