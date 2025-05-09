// ui_thread.h
#ifndef UI_H
#define UI_H

#include <pthread.h>
#include <stdbool.h>
#include <termios.h>

// 프로그램 상태
typedef enum
{
  STATE_STOPPED,
  STATE_RUNNING,
  STATE_EXIT
} State;

// UI Thread용 컨텍스트 구조체
typedef struct
{
  int fds[2];                     // off
  State state;                    // 프로그램 상태 포인터
  pthread_mutex_t mutex;          // 상태 동기화용 mutex
  pthread_cond_t cond;            // 상태 동기화용 condvar
  void (*reset_callback)(int); // 재시작 시 호출할 리셋 콜백
  struct termios orig_tio;        // 터미널 원복용 저장
} UiArgs;

/**
 * @brief Starts the UI thread.
 *
 * This function initializes and starts the UI thread. On success, it returns 0.
 *
 * @param arg Pointer to the UiArgs structure containing arguments for the UI thread.
 * @param tid Pointer to a pthread_t variable where the thread ID will be stored.
 * @return true if the UI thread starts successfully, false otherwise.
 */
bool ui_run(UiArgs **arg, pthread_t *tid);

/**
 * @brief Cleans up the UI thread and restores terminal settings.
 *
 * This function is called to clean up resources used by the UI thread and
 * restore the terminal to its original settings.
 *
 * @param ctx Pointer to the UiArgs structure used by the UI thread.
 */
void ui_thread_cleanup(UiArgs *ctx);

UiArgs *ui_init();

#endif // UI_H