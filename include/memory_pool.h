#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

  /**
   * @brief 메모리 풀 블록 구조체
   */
  typedef struct MemoryBlock
  {
    atomic_int refcount;      ///< 참조 카운트
    struct MemoryBlock *next; ///< free_list 연결 포인터
    void *data;               ///< 블록 데이터 영역 포인터
  } MemoryBlock;

  /**
   * @brief 메모리 풀 구조체
   */
  typedef struct MemoryPool
  {
    MemoryBlock *blocks;    ///< 블록 배열 (pool_size 개)
    void *pool_data;        ///< 실제 데이터 버퍼 (pool_size * block_size 바이트)
    size_t pool_size;       ///< 블록 개수
    size_t block_size;      ///< 블록 당 데이터 크기
    MemoryBlock *free_list; ///< 사용 가능한 블록 리스트
    pthread_mutex_t mutex;  ///< free_list 보호용 뮤텍스
  } MemoryPool;

  /**
   * @brief 메모리 풀을 동적 할당 및 초기화합니다.
   * @param pool_size  [in] 블록 개수 (>0)
   * @param block_size [in] 블록 당 바이트 크기 (>0)
   * @return 성공 시 MemoryPool*, 실패 시 NULL (errno 설정)
   */
  MemoryPool *mp_create(size_t pool_size, size_t block_size);

  /**
   * @brief 메모리 풀을 in-place 초기화합니다.
   * @param mp          [out] 초기화할 MemoryPool 포인터 (NULL 불가)
   * @param pool_size   [in] 블록 개수 (>0)
   * @param block_size  [in] 블록 당 바이트 크기 (>0)
   * @return 0: 성공, -1: 실패 (errno 설정)
   */
  int mp_init(MemoryPool *mp, size_t pool_size, size_t block_size);

  /**
   * @brief in-place로 초기화된 메모리 풀을 해제합니다.
   * @param mp [in] 초기화된 MemoryPool 포인터 (NULL safe)
   */
  void mp_destroy(MemoryPool *mp);

  /**
   * @brief mp_create()로 생성된 메모리 풀과 구조체를 해제합니다.
   * @param mp [in] 동적 생성된 MemoryPool 포인터 (NULL safe)
   */
  void mp_free(MemoryPool *mp);

  /**
   * @brief 블록을 할당하고 refcount를 설정합니다.
   * @param mp         [in] 초기화된 MemoryPool 포인터
   * @param init_count [in] 초기 refcount (>0)
   * @return 할당된 MemoryBlock*, 실패 시 NULL (errno 설정)
   */
  MemoryBlock *mp_alloc(MemoryPool *mp, int init_count);

  /**
   * @brief 블록 참조 수를 증가시킵니다.
   * @param blk [in] MemoryBlock 포인터 (NULL safe)
   */
  void mp_retain(MemoryBlock *blk);

  /**
   * @brief 블록 참조 수를 감소시키고 0이 되면 free_list로 반환합니다.
   * @param mp  [in] 초기화된 MemoryPool 포인터
   * @param blk [in] MemoryBlock 포인터 (NULL safe)
   */
  void mp_release(MemoryPool *mp, MemoryBlock *blk);

  /**
   * @brief   블록의 데이터 포인터를 반환합니다.
   * @param   blk [in] MemoryBlock 포인터
   * @return  blk->data 또는 NULL
   */
  void *mp_get_block_data(const MemoryBlock *blk);

  /**
   * @brief   블록의 현재 참조 카운트를 반환합니다.
   * @param   blk [in] MemoryBlock 포인터
   * @return  참조 카운트 또는 -1
   */
  int mp_get_block_refcount(const MemoryBlock *blk);

  /**
   * @brief   현재 free_list에 남아있는 블록 개수를 반환합니다.
   * @param   mp [in] MemoryPool 포인터
   * @return  사용 가능한 블록 수
   */
  size_t mp_available_count(const MemoryPool *mp);

  /**
   * @brief   전체 블록 수를 반환합니다.
   */
  size_t mp_total_size(const MemoryPool *mp);

  /**
   * @brief   블록 하나의 크기를 반환합니다.
   */
  size_t mp_block_size(const MemoryPool *mp);

  /**
   * @brief   메모리 풀 상태를 출력합니다. (디버깅 용도)
   * @param   mp  [in] MemoryPool 포인터
   * @param   fp  [in] 출력 스트림 (NULL이면 stdout)
   */
  void mp_debug_dump(const MemoryPool *mp, FILE *fp);

#ifdef __cplusplus
}
#endif

#endif // MEMORY_POOL_H
