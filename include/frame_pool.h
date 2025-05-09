/*
 * @file frame_pool.h
 * @brief Thread-safe FramePool for shared FrameBlocks
 */
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
   * @struct FrameBlock
   * @brief Holds a Frame and atomic reference count for pooling.
   */

  typedef struct FrameBlock
  {
    atomic_int refcount;     /**< Number of users holding this block */
    struct FrameBlock *next; /**< Next free block in list */
    Frame frame;             /**< Underlying Frame object */
  } FrameBlock;

  /**
   * @struct FramePool
   * @brief Manages a fixed array of FrameBlocks with free-list.
   */
  typedef struct FramePool
  {
    FrameBlock *blocks;           /**< Array of blocks (pool_size) */
    void *pool_data;              /**< Raw pixel buffer storage */
    size_t pool_size;             /**< Number of blocks */
    size_t total_bytes_per_frame; /**< bytes per Frame */
    FrameBlock *free_list;        /**< Head of free blocks */
    pthread_mutex_t mutex;        /**< Protects free_list */
    pthread_cond_t cond;          /**< Signals availability */
  } FramePool;

  /**
   * @brief Create and initialize a new FramePool.
   * @param[in] pool_size Number of blocks (>0).
   * @param[in] width     Frame width (>0).
   * @param[in] height    Frame height (>0).
   * @param[in] depth     Bytes per pixel (>0).
   * @return Pointer to FramePool or NULL (errno set).
   */
  FramePool *frame_pool_create(size_t pool_size, size_t width, size_t height, DEPTH depth);

  /**
   * @brief Initialize an existing FramePool in-place.
   * @param[in,out] fp        FramePool pointer (>NULL).
   * @param[in]     pool_size Blocks count (>0).
   * @param[in]     width     Frame width (>0).
   * @param[in]     height    Frame height (>0).
   * @param[in]     depth     Bytes per pixel (>0).
   * @return 0 on success; -1 on failure.
   */
  int frame_pool_init(FramePool *fp, size_t pool_size, size_t width, size_t height, DEPTH depth);

  /**
   * @brief Destroy a FramePool and free all resources.
   * @param[in,out] fp FramePool pointer (NULL safe).
   */
  void frame_pool_destroy(FramePool *fp);

  /**
   * @brief in-place로 초기화된 메모리 풀을 해제합니다.
   * @param fp [in] 동적 생성된 FramePool 포인터 (NULL safe)
   */
  void frame_pool_free(FramePool *fp);

  /**
   * @brief Allocate a block and set its refcount.
   * @param[in] fp         Initialized FramePool.
   * @param[in] init_count Initial refcount (>0).
   * @return Pointer to FrameBlock or NULL (errno set).
   */
  FrameBlock *fp_alloc(FramePool *fp, int init_count);

  /**
   * @brief Increment block's reference count.
   * @param[in,out] blk FrameBlock pointer (NULL safe).
   */
  void fp_retain(FrameBlock *blk);

  /**
   * @brief Decrement refcount and return to pool on zero.
   * @param[in] fp  FramePool pointer.
   * @param[in] blk FrameBlock pointer (NULL safe).
   */
  void fp_release(FramePool *fp, FrameBlock *blk);

  /**
   * @brief Get raw data pointer from block.
   * @param[in] blk FrameBlock pointer (NULL safe).
   * @return Data pointer or NULL.
   */
  void *fp_get_block_data(const FrameBlock *blk);

  /**
   * @brief Query block's current refcount.
   * @param[in] blk FrameBlock pointer (NULL safe).
   * @return refcount or -1.
   */
  int fp_get_block_refcount(const FrameBlock *blk);

  /**
   * @brief Count free blocks available.
   * @param[in] fp FramePool pointer (NULL safe).
   * @return Number of free blocks.
   */
  size_t fp_available_count(const FramePool *fp);

  /**
   * @brief Count blocks currently in use.
   * @param[in] fp FramePool pointer (NULL safe).
   * @return Number of used blocks.
   */
  size_t fp_used_count(const FramePool *fp);

  /**
   * @brief Total blocks in pool.
   * @param[in] fp FramePool pointer (NULL safe).
   * @return Pool size.
   */
  size_t fp_total_block_size(const FramePool *fp);

  /**
   * @brief Size in bytes for each frame.
   * @param[in] fp FramePool pointer (NULL safe).
   * @return Bytes per frame.
   */
  size_t fp_total_bytes_per_frame_size(const FramePool *fp);

  /**
   * @brief Dump debugging info of FramePool.
   * @param[in] fp     FramePool pointer (NULL safe).
   * @param[in] stream Output FILE stream (defaults to stdout if NULL).
   */
  void fp_debug_dump(const FramePool *fp, FILE *file_p);

#ifdef __cplusplus
}
#endif

#endif // FRAME_POOL_H