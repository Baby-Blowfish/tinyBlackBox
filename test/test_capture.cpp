

#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdatomic.h>
#include <pthread.h>
#include <stdbool.h>

#include "capture.h"

#define WIDTH           2
#define HEIGHT          2
#define POOL_SIZE       4
#define NUM_TEST_FRAMES 8

// #define WIDTH 1920
// #define HEIGHT 1080
// #define FILE_NAME "data/video1.raw"

TEST(CaptureTest, LoadRealVideoAndQueue)
{
  const char *fname = "test.raw";

  // 1) 테스트용 RAW 파일 생성 (각 프레임 BYTE 패턴 = 프레임 인덱스)
  int wfd = open(fname, O_CREAT|O_TRUNC|O_WRONLY, 0644);
  ASSERT_LE(wfd < 0)
  if (wfd < 0) { perror("create test.raw"); return 1; }
  for (size_t f = 0; f < NUM_TEST_FRAMES; ++f) {
      unsigned char frame_buf[WIDTH*HEIGHT];
      for (size_t i = 0; i < WIDTH*HEIGHT; ++i)
          frame_buf[i] = (unsigned char)f;
      if (write(wfd, frame_buf, WIDTH*HEIGHT) != WIDTH*HEIGHT) {
          perror("write frame");
          close(wfd);
          return 1;
      }
  }
  close(wfd);

  // 2) FramePool 초기화
  FramePool *pool = frame_pool_init(WIDTH, HEIGHT, POOL_SIZE);
  if (!pool) {
      fprintf(stderr, "ERROR: frame_pool_init failed\n");
      return 1;
  }

  // 3) capture 스레드용 인자 직접 세팅
  thread_arg_t arg;
  arg.pool = pool;
  arg.run  = true;
  arg.fd   = open(fname, O_RDONLY);
  if (arg.fd < 0) {
      perror("open for capture");
      frame_pool_destroy(pool);
      return 1;
  }

  // 4) 캡처 스레드 기동
  if (!capture_run(&arg)) {
      fprintf(stderr, "ERROR: capture_run failed\n");
      close(arg.fd);
      frame_pool_destroy(pool);
      return 1;
  }

  // 5) 메인 스레드에서 display_q로부터 프레임 소비
  for (size_t cnt = 0; cnt < NUM_TEST_FRAMES; ++cnt) {
      // -- dequeue display_q (mutex + cond 사용) --
      Queue *dq = pool->display_q;
      pthread_mutex_lock(&dq->mutex);
      while (is_empty(dq)) {
          pthread_cond_wait(&dq->cond_not_empty, &dq->mutex);
      }
      Frame *f = (Frame*)dequeue(dq);
      pthread_cond_signal(&dq->cond_not_full);
      pthread_mutex_unlock(&dq->mutex);

      // -- 출력 확인 --
      printf("Frame seq=%zu  data=", f->seq);
      for (size_t i = 0; i < WIDTH*HEIGHT; ++i)
          printf("%u ", f->data[i]);
      putchar('\n');

      // -- refcount 감소 → free_q에 반환 --
      if (atomic_fetch_sub(&f->refcount, 1) == 1) {
          Queue *fq = pool->free_q;
          pthread_mutex_lock(&fq->mutex);
          enqueue(fq, f);
          pthread_cond_signal(&fq->cond_not_empty);
          pthread_mutex_unlock(&fq->mutex);
      }
  }

  // 6) 스레드 종료 요청 및 조인
  queue_set_done(pool->free_q);
  pthread_join(arg.tid, NULL);

  // 7) 자원 해제
  close(arg.fd);
  // capture_destroy(&arg); // 필요 시 구현된 destroy 호출
  frame_pool_destroy(pool);

  printf("=== Capture test completed ===\n");
  return 0;
}
