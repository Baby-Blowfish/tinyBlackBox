// test_capture.c
#include <fcntl.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "capture.h"
#include "frame_pool.h"
#include "queue.h"

#define WIDTH 2
#define HEIGHT 2
#define POOL_SIZE 4
#define NUM_TEST_FRAMES 8

int main(void)
{
  const char *fname = "test.raw";
  capture_arg_t *arg = NULL;
  FramePool *pool = NULL;

  // 1) 테스트용 RAW 파일 만들기 (각 프레임 바이트값 = 프레임 인덱스)
  int wfd = open(fname, O_CREAT | O_TRUNC | O_WRONLY, 0644);
  if (wfd < 0)
  {
    perror("open/write test.raw");
    return 1;
  }
  for (size_t f = 0; f < NUM_TEST_FRAMES; ++f)
  {
    unsigned char buf[WIDTH * HEIGHT];
    for (size_t i = 0; i < WIDTH * HEIGHT; ++i)
      buf[i] = (unsigned char)f;
    if (write(wfd, buf, WIDTH * HEIGHT) != WIDTH * HEIGHT)
    {
      perror("write frame");
      close(wfd);
      return 1;
    }
  }
  close(wfd);

  if (!(arg = capture_init(fname)))
  {
    fprintf(stderr, "ERROR: capture_init failed\n");
    return 1;
  }

  // 2) FramePool 생성
  pool = frame_pool_init(WIDTH, HEIGHT, POOL_SIZE);
  if (!pool)
  {
    fprintf(stderr, "ERROR: frame_pool_init failed\n");
    close(arg->fd);
    free(arg);
    return 1;
  }

  arg->pool = pool;

  // 4) 캡처 스레드 시작
  if (!capture_run(arg))
  {
    fprintf(stderr, "ERROR: capture_run failed\n");
    capture_destroy(arg);
    return 1;
  }

  // 5) 메인 스레드: display_q, record_q에서 NUM_TEST_FRAMES개 프레임 읽기
  for (size_t cnt = 0; cnt < NUM_TEST_FRAMES; ++cnt)
  {
    Queue *dq = pool->display_q;

    // display_q 빈 큐 대기
    pthread_mutex_lock(&dq->mutex);
    while (is_empty(dq) && !dq->done)
    {
      pthread_cond_wait(&dq->cond_not_empty, &dq->mutex);
    }
    if (is_empty(dq) && dq->done)
    {
      pthread_mutex_unlock(&dq->mutex);
      break;
    }

    // 프레임 꺼내기
    Frame *df = (Frame *)dequeue(dq);

    // seq, 데이터 출력
    printf("Frame seq=%zu data=", df->seq);
    for (size_t i = 0; i < WIDTH * HEIGHT; ++i)
      printf("%u ", df->data[i]);
    putchar('\n');

    pthread_cond_signal(&dq->cond_not_full);
    pthread_mutex_unlock(&dq->mutex);

    // refcount 감소 → free_q로 반환
    if (atomic_fetch_sub(&df->refcount, 1) == 1)
    {
      Queue *fq = pool->free_q;
      pthread_mutex_lock(&fq->mutex);
      while (is_full(fq))
      {
        pthread_cond_wait(&fq->cond_not_full, &fq->mutex);
      }
      enqueue(fq, (void *)df);
      pthread_cond_signal(&fq->cond_not_empty);
      pthread_mutex_unlock(&fq->mutex);
    }

    // Queue *rq = pool->display_q;

    // // record_q 빈 큐 대기
    // pthread_mutex_lock(&rq->mutex);
    // while (is_empty(rq) && !rq->done)
    // {
    //   pthread_cond_wait(&rq->cond_not_empty, &rq->mutex);
    // }
    // if (is_empty(rq) && rq->done)
    // {
    //   pthread_mutex_unlock(&rq->mutex);
    //   break;
    // }

    // // 프레임 꺼내기
    // Frame *rf = (Frame *)dequeue(rq);

    // // seq, 데이터 출력
    // printf("Frame seq=%zu data=", rf->seq);
    // for (size_t i = 0; i < WIDTH * HEIGHT; ++i)
    //   printf("%u ", rf->data[i]);
    // putchar('\n');

    // pthread_cond_signal(&rq->cond_not_full);
    // pthread_mutex_unlock(&rq->mutex);

    // // refcount 감소 → free_q로 반환
    // if (atomic_fetch_sub(&rf->refcount, 1) == 1)
    // {
    //   Queue *fq = pool->free_q;
    //   pthread_mutex_lock(&fq->mutex);
    //   while (is_full(fq))
    //   {
    //     pthread_cond_wait(&fq->cond_not_full, &fq->mutex);
    //   }
    //   enqueue(fq, (void *)rf);
    //   pthread_cond_signal(&fq->cond_not_empty);
    //   pthread_mutex_unlock(&fq->mutex);
    // }
  }

  pthread_join(arg->tid, NULL);

  capture_destroy(arg);

  printf("=== test_capture completed ===\n");
  return 0;
}
